/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef FILAMENT_TARGET_MOBILE
#   define DOF_DEFAULT_RING_COUNT 3
#   define DOF_DEFAULT_MAX_COC    24
#else
#   define DOF_DEFAULT_RING_COUNT 5
#   define DOF_DEFAULT_MAX_COC    32
#endif

#include "PostProcessManager.h"

#include "details/Engine.h"

#include "fg/FrameGraph.h"
#include "fg/FrameGraphResources.h"

#include "fsr.h"
#include "PerViewUniforms.h"
#include "RenderPass.h"

#include "details/Camera.h"
#include "details/ColorGrading.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "details/Texture.h"

#include "generated/resources/materials.h"

#include <filament/MaterialEnums.h>

#include <math/half.h>
#include <math/mat2.h>

#include <algorithm>
#include <limits>

namespace filament {

using namespace utils;
using namespace math;
using namespace backend;

static constexpr uint8_t kMaxBloomLevels = 12u;
static_assert(kMaxBloomLevels >= 3, "We require at least 3 bloom levels");

constexpr static float halton(unsigned int i, unsigned int b) noexcept {
    // skipping a bunch of entries makes the average of the sequence closer to 0.5
    i += 409;
    float f = 1.0f;
    float r = 0.0f;
    while (i > 0u) {
        f /= float(b);
        r += f * float(i % b);
        i /= b;
    }
    return r;
}

// ------------------------------------------------------------------------------------------------

PostProcessManager::PostProcessMaterial::PostProcessMaterial() noexcept {
    mMaterial = nullptr; // aliased to mData
    mData = nullptr;
}

PostProcessManager::PostProcessMaterial::PostProcessMaterial(FEngine& engine,
        uint8_t const* data, int size) noexcept
        : PostProcessMaterial() {
    mData = data; // aliased to mMaterial
    mSize = size;
}

PostProcessManager::PostProcessMaterial::PostProcessMaterial(
        PostProcessManager::PostProcessMaterial&& rhs) noexcept
        : PostProcessMaterial() {
    using namespace std;
    swap(mData, rhs.mData); // aliased to mMaterial
    swap(mSize, rhs.mSize);
    swap(mHasMaterial, rhs.mHasMaterial);
}

PostProcessManager::PostProcessMaterial& PostProcessManager::PostProcessMaterial::operator=(
        PostProcessManager::PostProcessMaterial&& rhs) noexcept {
    if (this != &rhs) {
        using namespace std;
        swap(mData, rhs.mData); // aliased to mMaterial
        swap(mSize, rhs.mSize);
        swap(mHasMaterial, rhs.mHasMaterial);
    }
    return *this;
}

PostProcessManager::PostProcessMaterial::~PostProcessMaterial() {
    assert_invariant(!mHasMaterial || mMaterial == nullptr);
}

void PostProcessManager::PostProcessMaterial::terminate(FEngine& engine) noexcept {
    if (mHasMaterial) {
        engine.destroy(mMaterial);
// this is only needed for validation in the dtor in debug builds
#ifndef NDEBUG
        mMaterial = nullptr;
        mHasMaterial = false;
    } else {
        mData = nullptr;
#endif
    }
}

UTILS_NOINLINE
void PostProcessManager::PostProcessMaterial::loadMaterial(FEngine& engine) const noexcept {
    // TODO: After all materials using this class have been converted to the post-process material
    //       domain, load both OPAQUE and TRANSPARENT variants here.
    mHasMaterial = true;
    mMaterial = upcast(Material::Builder().package(mData, mSize).build(engine));
}

UTILS_NOINLINE
FMaterial* PostProcessManager::PostProcessMaterial::getMaterial(FEngine& engine) const noexcept {
    if (UTILS_UNLIKELY(!mHasMaterial)) {
        loadMaterial(engine);
    }
    return mMaterial;
}

UTILS_NOINLINE
PipelineState PostProcessManager::PostProcessMaterial::getPipelineState(
        FEngine& engine, Variant::type_t variantKey) const noexcept {
    FMaterial* const material = getMaterial(engine);
    material->prepareProgram(Variant{ variantKey });
    return {
            .program = material->getProgram(Variant{variantKey}),
            .rasterState = material->getRasterState(),
            .scissor = material->getDefaultInstance()->getScissor()
    };
}

UTILS_NOINLINE
FMaterialInstance* PostProcessManager::PostProcessMaterial::getMaterialInstance(FEngine& engine) const noexcept {
    FMaterial* const material = getMaterial(engine);
    return material->getDefaultInstance();
}

// ------------------------------------------------------------------------------------------------

const math::float2 PostProcessManager::sHaltonSamples[16] = {
        { filament::halton( 0, 2), filament::halton( 0, 3) },
        { filament::halton( 1, 2), filament::halton( 1, 3) },
        { filament::halton( 2, 2), filament::halton( 2, 3) },
        { filament::halton( 3, 2), filament::halton( 3, 3) },
        { filament::halton( 4, 2), filament::halton( 4, 3) },
        { filament::halton( 5, 2), filament::halton( 5, 3) },
        { filament::halton( 6, 2), filament::halton( 6, 3) },
        { filament::halton( 7, 2), filament::halton( 7, 3) },
        { filament::halton( 8, 2), filament::halton( 8, 3) },
        { filament::halton( 9, 2), filament::halton( 9, 3) },
        { filament::halton(10, 2), filament::halton(10, 3) },
        { filament::halton(11, 2), filament::halton(11, 3) },
        { filament::halton(12, 2), filament::halton(12, 3) },
        { filament::halton(13, 2), filament::halton(13, 3) },
        { filament::halton(14, 2), filament::halton(14, 3) },
        { filament::halton(15, 2), filament::halton(15, 3) }
};

PostProcessManager::PostProcessManager(FEngine& engine) noexcept
        : mEngine(engine),
         mWorkaroundSplitEasu(false),
         mWorkaroundAllowReadOnlyAncillaryFeedbackLoop(false) {
}

PostProcessManager::~PostProcessManager() noexcept = default;

UTILS_NOINLINE
void PostProcessManager::registerPostProcessMaterial(std::string_view name, uint8_t const* data, int size) {
    mMaterialRegistry.try_emplace(name, mEngine, data, size);
}

UTILS_NOINLINE
PostProcessManager::PostProcessMaterial& PostProcessManager::getPostProcessMaterial(std::string_view name) noexcept {
    assert_invariant(mMaterialRegistry.find(name) != mMaterialRegistry.end());
    return mMaterialRegistry[name];
}

#define MATERIAL(n) MATERIALS_ ## n ## _DATA, MATERIALS_ ## n ## _SIZE

struct MaterialInfo {
    std::string_view name;
    uint8_t const* data;
    int size;
};

static const MaterialInfo sMaterialList[] = {
        { "bilateralBlur",              MATERIAL(BILATERALBLUR) },
        { "bilateralBlurBentNormals",   MATERIAL(BILATERALBLURBENTNORMALS) },
        { "blitLow",                    MATERIAL(BLITLOW) },
        { "bloomDownsample",            MATERIAL(BLOOMDOWNSAMPLE) },
        { "bloomUpsample",              MATERIAL(BLOOMUPSAMPLE) },
        { "colorGrading",               MATERIAL(COLORGRADING) },
        { "colorGradingAsSubpass",      MATERIAL(COLORGRADINGASSUBPASS) },
        { "customResolveAsSubpass",     MATERIAL(CUSTOMRESOLVEASSUBPASS) },
        { "dof",                        MATERIAL(DOF) },
        { "dofCoc",                     MATERIAL(DOFCOC) },
        { "dofCombine",                 MATERIAL(DOFCOMBINE) },
        { "dofDilate",                  MATERIAL(DOFDILATE) },
        { "dofDownsample",              MATERIAL(DOFDOWNSAMPLE) },
        { "dofMedian",                  MATERIAL(DOFMEDIAN) },
        { "dofMipmap",                  MATERIAL(DOFMIPMAP) },
        { "dofTiles",                   MATERIAL(DOFTILES) },
        { "dofTilesSwizzle",            MATERIAL(DOFTILESSWIZZLE) },
        { "flare",                      MATERIAL(FLARE) },
        { "fxaa",                       MATERIAL(FXAA) },
        { "mipmapDepth",                MATERIAL(MIPMAPDEPTH) },
        { "sao",                        MATERIAL(SAO) },
        { "saoBentNormals",             MATERIAL(SAOBENTNORMALS) },
        { "separableGaussianBlur1",     MATERIAL(SEPARABLEGAUSSIANBLUR1) },
        { "separableGaussianBlur2",     MATERIAL(SEPARABLEGAUSSIANBLUR2) },
        { "separableGaussianBlur3",     MATERIAL(SEPARABLEGAUSSIANBLUR3) },
        { "separableGaussianBlur4",     MATERIAL(SEPARABLEGAUSSIANBLUR4) },
        { "separableGaussianBlur1L",    MATERIAL(SEPARABLEGAUSSIANBLUR1L) },
        { "separableGaussianBlur2L",    MATERIAL(SEPARABLEGAUSSIANBLUR2L) },
        { "separableGaussianBlur3L",    MATERIAL(SEPARABLEGAUSSIANBLUR3L) },
        { "separableGaussianBlur4L",    MATERIAL(SEPARABLEGAUSSIANBLUR4L) },
        { "taa",                        MATERIAL(TAA) },
        { "vsmMipmap",                  MATERIAL(VSMMIPMAP) },
        { "fsr_easu",                   MATERIAL(FSR_EASU) },
        { "fsr_easu_mobile",            MATERIAL(FSR_EASU_MOBILE) },
        { "fsr_easu_mobileF",           MATERIAL(FSR_EASU_MOBILEF) },
        { "fsr_rcas",                   MATERIAL(FSR_RCAS) },
};

void PostProcessManager::init() noexcept {
    auto& engine = mEngine;
    DriverApi& driver = engine.getDriverApi();

    //FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    //debugRegistry.registerProperty("d.ssao.sampleCount", &engine.debug.ssao.sampleCount);
    //debugRegistry.registerProperty("d.ssao.spiralTurns", &engine.debug.ssao.spiralTurns);
    //debugRegistry.registerProperty("d.ssao.kernelSize", &engine.debug.ssao.kernelSize);
    //debugRegistry.registerProperty("d.ssao.stddev", &engine.debug.ssao.stddev);

    mWorkaroundSplitEasu =
            driver.isWorkaroundNeeded(Workaround::SPLIT_EASU);
    mWorkaroundAllowReadOnlyAncillaryFeedbackLoop =
            driver.isWorkaroundNeeded(Workaround::ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP);

    #pragma nounroll
    for (auto const& info : sMaterialList) {
        registerPostProcessMaterial(info.name, info.data, info.size);
    }

    mStarburstTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::R8, 1, 256, 1, 1, TextureUsage::DEFAULT);

    PixelBufferDescriptor dataStarburst(driver.allocate(256), 256, PixelDataFormat::R, PixelDataType::UBYTE);
    std::generate_n((uint8_t*)dataStarburst.buffer, 256,
            [&dist = mUniformDistribution, &gen = mEngine.getRandomEngine()]() {
        float r = 0.5f + 0.5f * dist(gen);
        return uint8_t(r * 255.0f);
    });

    driver.update3DImage(mStarburstTexture,
            0, 0, 0, 0, 256, 1, 1,
            std::move(dataStarburst));
}

void PostProcessManager::terminate(DriverApi& driver) noexcept {
    FEngine& engine = mEngine;
    driver.destroyTexture(mStarburstTexture);
    auto first = mMaterialRegistry.begin();
    auto last = mMaterialRegistry.end();
    while (first != last) {
        first.value().terminate(engine);
        ++first;
    }
}

backend::Handle<backend::HwTexture> PostProcessManager::getOneTexture() const {
    return mEngine.getOneTexture();
}

backend::Handle<backend::HwTexture> PostProcessManager::getZeroTexture() const {
    return mEngine.getZeroTexture();
}

backend::Handle<backend::HwTexture> PostProcessManager::getOneTextureArray() const {
    return mEngine.getOneTextureArray();
}

backend::Handle<backend::HwTexture> PostProcessManager::getZeroTextureArray() const {
    return mEngine.getZeroTextureArray();
}

UTILS_NOINLINE
void PostProcessManager::render(FrameGraphResources::RenderPassInfo const& out,
        backend::PipelineState const& pipeline,
        DriverApi& driver) const noexcept {

    assert_invariant(
            ((out.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH)
             && !pipeline.rasterState.depthWrite)
            || !(out.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH));

    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();
    driver.beginRenderPass(out.target, out.params);
    driver.draw(pipeline, fullScreenRenderPrimitive, 1);
    driver.endRenderPass();
}

UTILS_NOINLINE
void PostProcessManager::commitAndRender(FrameGraphResources::RenderPassInfo const& out,
        PostProcessMaterial const& material, uint8_t variant, DriverApi& driver) const noexcept {
    FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
    mi->commit(driver);
    mi->use(driver);
    render(out, material.getPipelineState(mEngine, variant), driver);
}

UTILS_ALWAYS_INLINE
void PostProcessManager::commitAndRender(FrameGraphResources::RenderPassInfo const& out,
        PostProcessMaterial const& material, DriverApi& driver) const noexcept {
    commitAndRender(out, material, 0, driver);
}

// ------------------------------------------------------------------------------------------------

PostProcessManager::StructurePassOutput PostProcessManager::structure(FrameGraph& fg,
        RenderPass const& pass, uint8_t structureRenderFlags,
        uint32_t width, uint32_t height,
        StructurePassConfig const& config) noexcept {

    const float scale = config.scale;

    // structure pass -- automatically culled if not used, currently used by:
    //    - ssao
    //    - contact shadows
    // It consists of a mipmapped depth pass, tuned for SSAO
    struct StructurePassData {
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> picking;
    };

    // sanitize a bit the user provided scaling factor
    width  = std::max(32u, (uint32_t)std::ceil(width * scale));
    height = std::max(32u, (uint32_t)std::ceil(height * scale));

    // We limit the lowest lod size to 32 pixels (which is where the -5 comes from)
    const size_t levelCount = std::min(8, FTexture::maxLevelCount(width, height) - 5);
    assert_invariant(levelCount >= 1);

    // generate depth pass at the requested resolution
    auto& structurePass = fg.addPass<StructurePassData>("Structure Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.depth = builder.createTexture("Structure Buffer", {
                        .width = width, .height = height,
                        .levels = uint8_t(levelCount),
                        .format = TextureFormat::DEPTH32F });

                // workaround: since we have levels, this implies SAMPLEABLE (because of the gl
                // backend, which implements non-sampleables with renderbuffers, which don't have levels).
                // (should the gl driver revert to textures, in that case?)
                data.depth = builder.write(data.depth,
                        FrameGraphTexture::Usage::DEPTH_ATTACHMENT | FrameGraphTexture::Usage::SAMPLEABLE);

                if (config.picking) {
                    data.picking = builder.createTexture("Picking Buffer", {
                            .width = width, .height = height,
                            .format = TextureFormat::RG32UI });

                    data.picking = builder.write(data.picking,
                            FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                }

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.picking;
                descr.attachments.content.depth = data.depth;
                descr.clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH;
                builder.declareRenderPass("Structure Target", descr);
            },
            [=, renderPass = pass](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) mutable {
                Variant structureVariant(Variant::DEPTH_VARIANT);
                structureVariant.setPicking(config.picking);

                auto out = resources.getRenderPassInfo();
                renderPass.setRenderFlags(structureRenderFlags);
                renderPass.setVariant(structureVariant);
                renderPass.appendCommands(RenderPass::CommandTypeFlags::SSAO);
                renderPass.sortCommands();
                renderPass.execute(resources.getPassName(), out.target, out.params);
            });

    auto depth = structurePass->depth;

    /*
     * create depth mipmap chain
    */

    struct StructureMipmapData {
        FrameGraphId<FrameGraphTexture> depth;
        uint32_t rt[8];
    };

    fg.addPass<StructureMipmapData>("StructureMipmap",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.depth = builder.sample(depth);
                for (size_t i = 1; i < levelCount; i++) {
                    auto out = builder.createSubresource(data.depth, "Structure mip", {
                            .level = uint8_t(i)
                    });
                    out = builder.write(out, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                    FrameGraphRenderPass::Descriptor descr;
                    descr.attachments.content.depth = out;
                    data.rt[i - 1] = builder.declareRenderPass("Structure mip target", descr);
                }
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto in = resources.getTexture(data.depth);
                auto& material = getPostProcessMaterial("mipmapDepth");
                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
                mi->setParameter("depth", in, { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                // The first mip already exists, so we process n-1 lods
                for (size_t level = 0; level < levelCount - 1; level++) {
                    auto out = resources.getRenderPassInfo(level);
                    driver.setMinMaxLevels(in, level, level);

                    auto& material = getPostProcessMaterial("mipmapDepth");
                    FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
                    SamplerParams depthSamplerParams{};
                    depthSamplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                    mi->setParameter("depth", in, depthSamplerParams);
                    mi->setParameter("level", uint32_t(level));
                    commitAndRender(out, material, driver);
                }
                driver.setMinMaxLevels(in, 0, levelCount - 1);
            });

    return { depth, structurePass->picking };
}

// ------------------------------------------------------------------------------------------------

FrameGraphId<FrameGraphTexture> PostProcessManager::ssr(FrameGraph& fg,
        RenderPass const& pass,
        FrameHistory const& frameHistory,
        CameraInfo const& cameraInfo,
        PerViewUniforms& uniforms,
        FrameGraphId<FrameGraphTexture> structure,
        ScreenSpaceReflectionsOptions const& options,
        FrameGraphTexture::Descriptor const& desc) noexcept {

    struct SSRPassData {
        // our output, the reflection map
        FrameGraphId<FrameGraphTexture> reflections;
        // we need a depth buffer for culling
        FrameGraphId<FrameGraphTexture> depth;
        // we also need the structure buffer for ray-marching
        FrameGraphId<FrameGraphTexture> structure;
        // and the history buffer for fetching the reflections
        FrameGraphId<FrameGraphTexture> history;
    };

    mat4f historyProjection;
    FrameGraphId<FrameGraphTexture> history;

    FrameHistoryEntry const& entry = frameHistory[0];
    if (entry.ssr.color.handle) {
        // the first time around we may not have a history buffer
        history = fg.import("SSR history", entry.ssr.desc,
                FrameGraphTexture::Usage::SAMPLEABLE, entry.ssr.color);
        historyProjection = entry.ssr.projection;
    }

    auto const& uvFromClipMatrix = mEngine.getUvFromClipMatrix();

    auto& ssrPass = fg.addPass<SSRPassData>("SSR Pass",
            [&](FrameGraph::Builder& builder, auto& data) {

                // create our reflection buffer. We need an alpha channel, so we have to use RGBA16F
                data.reflections = builder.createTexture("Reflections Texture", {
                        .width = desc.width, .height = desc.height,
                        .format = TextureFormat::RGBA16F });

                // create our depth buffer, the depth buffer is never written to memory
                data.depth = builder.createTexture("Reflections Texture Depth", {
                        .width = desc.width, .height = desc.height,
                        .format = TextureFormat::DEPTH32F });

                // we're writing to both these buffers
                data.reflections = builder.write(data.reflections,
                        FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.depth = builder.write(data.depth,
                        FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                // finally declare our render target
                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.reflections;
                descr.attachments.content.depth = data.depth;
                descr.clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH;
                builder.declareRenderPass("Reflections Target", descr);

                // get the structure buffer
                assert_invariant(structure);
                data.structure = builder.sample(structure);

                if (history) {
                    data.history = builder.sample(history);
                }
            },
            [this, projection = cameraInfo.projection,
                    userViewMatrix = cameraInfo.getUserViewMatrix(), uvFromClipMatrix, historyProjection,
                    options, &uniforms, renderPass = pass]
            (FrameGraphResources const& resources, auto const& data, DriverApi& driver) mutable {
                // set structure sampler
                uniforms.prepareStructure(data.structure ?
                        resources.getTexture(data.structure) : getOneTexture());

                // set screen-space reflections and screen-space refractions
                mat4f uvFromViewMatrix = uvFromClipMatrix * projection;
                mat4f reprojection = mat4f{ uvFromClipMatrix * historyProjection
                        * inverse(userViewMatrix) };

                // the history sampler is a regular texture2D
                TextureHandle history = data.history ?
                        resources.getTexture(data.history) : getZeroTexture();
                uniforms.prepareHistorySSR(history, reprojection, uvFromViewMatrix, options);

                uniforms.commit(driver);

                auto out = resources.getRenderPassInfo();

                // Remove the HAS_SHADOWING RenderFlags, since it's irrelevant when rendering reflections
                RenderPass::RenderFlags flags = renderPass.getRenderFlags();
                flags &= ~RenderPass::HAS_SHADOWING;
                renderPass.setRenderFlags(flags);

                // use our special SSR variant, it can only be applied to object that have
                // the SCREEN_SPACE ReflectionMode.
                renderPass.setVariant(Variant{Variant::SPECIAL_SSR});
                // generate all our drawing commands, except blended objects.
                renderPass.appendCommands(RenderPass::CommandTypeFlags::SCREEN_SPACE_REFLECTIONS);
                renderPass.sortCommands();
                renderPass.execute(resources.getPassName(), out.target, out.params);
            });

    return ssrPass->reflections;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::screenSpaceAmbientOcclusion(FrameGraph& fg,
        filament::Viewport const& svp, const CameraInfo& cameraInfo,
        FrameGraphId<FrameGraphTexture> depth,
        AmbientOcclusionOptions const& options) noexcept {
    assert_invariant(depth);

    const size_t levelCount = fg.getDescriptor(depth).levels;

    // With q the standard deviation,
    // A gaussian filter requires 6q-1 values to keep its gaussian nature
    // (see en.wikipedia.org/wiki/Gaussian_filter)
    // More intuitively, 2q is the width of the filter in pixels.
    BilateralPassConfig config = {
            .bentNormals = options.bentNormals,
            .bilateralThreshold = options.bilateralThreshold,
    };

    float sampleCount{};
    float spiralTurns{};
    float standardDeviation{};
    switch (options.quality) {
        default:
        case QualityLevel::LOW:
            sampleCount = 7.0f;
            spiralTurns = 3.0f;
            standardDeviation = 8.0;
            break;
        case QualityLevel::MEDIUM:
            sampleCount = 11.0f;
            spiralTurns = 6.0f;
            standardDeviation = 8.0;
            break;
        case QualityLevel::HIGH:
            sampleCount = 16.0f;
            spiralTurns = 7.0f;
            standardDeviation = 6.0;
            break;
        case QualityLevel::ULTRA:
            sampleCount = 32.0f;
            spiralTurns = 14.0f;
            standardDeviation = 4.0;
            break;
    }

    switch (options.lowPassFilter) {
        default:
        case QualityLevel::LOW:
            // no filtering, values don't matter
            config.kernelSize = 1;
            config.standardDeviation = 1.0f;
            config.scale = 1.0f;
            break;
        case QualityLevel::MEDIUM:
            config.kernelSize = 11;
            config.standardDeviation = standardDeviation * 0.5f;
            config.scale = 2.0f;
            break;
        case QualityLevel::HIGH:
        case QualityLevel::ULTRA:
            config.kernelSize = 23;
            config.standardDeviation = standardDeviation;
            config.scale = 1.0f;
            break;
    }

    // for debugging
    //config.kernelSize = engine.debug.ssao.kernelSize;
    //config.standardDeviation = engine.debug.ssao.stddev;
    //sampleCount = engine.debug.ssao.sampleCount;
    //spiralTurns = engine.debug.ssao.spiralTurns;

    /*
     * Our main SSAO pass
     */

    struct SSAOPassData {
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> ssao;
        FrameGraphId<FrameGraphTexture> ao;
        FrameGraphId<FrameGraphTexture> bn;
    };

    const bool computeBentNormals = options.bentNormals;

    const bool highQualityUpsampling =
            options.upsampling >= QualityLevel::HIGH && options.resolution < 1.0f;

    const bool lowPassFilterEnabled = options.lowPassFilter != QualityLevel::LOW;

    /*
     * GLES considers there is a feedback loop when a buffer is used as both a texture and
     * attachment, even if writes are not enabled. This restriction is lifted on desktop GL and
     * Vulkan. The Metal situation is unclear.
     * In this case, we need to duplicate the depth texture to use it as an attachment.
     * The pass below that does this is automatically culled if not needed, which is decided by
     * each backend.
     */

    struct DuplicateDepthPassData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto& duplicateDepthPass = fg.addPass<DuplicateDepthPassData>("Duplicate Depth Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                // read the depth as an attachment
                data.input = builder.read(depth,
                        FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                auto desc = builder.getDescriptor(data.input);
                desc.levels = 1; // only copy the base level
                // create a new buffer for the copy
                data.output = builder.createTexture("Depth Texture Copy", desc);
                data.output = builder.write(data.output,
                        FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.depth = data.output;
                builder.declareRenderPass("Depth Copy RenderTarget", descr);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto const& desc = resources.getDescriptor(data.input);
                auto out = resources.getRenderPassInfo();
                // create a temporary render target for source, needed for the blit.
                auto inTarget = driver.createRenderTarget(TargetBufferFlags::DEPTH,
                        desc.width, desc.height, desc.samples, {},
                        { resources.getTexture(data.input) }, {});
                driver.blit(TargetBufferFlags::DEPTH,
                        out.target, out.params.viewport,
                        inTarget, out.params.viewport,
                        SamplerMagFilter::NEAREST);
                driver.destroyRenderTarget(inTarget);
            });

    auto& SSAOPass = fg.addPass<SSAOPassData>("SSAO Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const& desc = builder.getDescriptor(depth);

                data.depth = builder.sample(depth);
                data.ssao = builder.createTexture("SSAO Buffer", {
                        .width = desc.width,
                        .height = desc.height,
                        .depth = computeBentNormals ? 2u : 1u,
                        .type = Texture::Sampler::SAMPLER_2D_ARRAY,
                        .format = (lowPassFilterEnabled || highQualityUpsampling || computeBentNormals) ?
                                TextureFormat::RGB8 : TextureFormat::R8
                });

                if (computeBentNormals) {
                    data.ao = builder.createSubresource(data.ssao, "SSAO attachment", { .layer = 0 });
                    data.bn = builder.createSubresource(data.ssao, "Bent Normals attachment", { .layer = 1 });
                    data.ao = builder.write(data.ao, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                    data.bn = builder.write(data.bn, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                } else {
                    data.ao = data.ssao;
                    data.ao = builder.write(data.ao, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                }

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // Note that we have to clear the SAO buffer because blended objects will end-up
                // reading into it even though they were not written in the depth buffer.
                // The bilateral filter in the blur pass will ignore pixels at infinity.

                auto depthAttachment = data.depth;
                if (!mWorkaroundAllowReadOnlyAncillaryFeedbackLoop) {
                    depthAttachment = duplicateDepthPass->output;
                }

                depthAttachment = builder.read(depthAttachment,
                        FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.ao;
                descr.attachments.content.color[1] = data.bn;
                descr.attachments.content.depth = depthAttachment;
                descr.clearColor = { 1.0f };
                descr.clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::COLOR1;
                builder.declareRenderPass("SSAO Target", descr);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto depth = resources.getTexture(data.depth);
                auto ssao = resources.getRenderPassInfo();
                auto const& desc = resources.getDescriptor(data.depth);

                // estimate of the size in pixel of a 1m tall/wide object viewed from 1m away (i.e. at z=-1)
                const float projectionScale = std::min(
                        0.5f * cameraInfo.projection[0].x * desc.width,
                        0.5f * cameraInfo.projection[1].y * desc.height);

                // Where the falloff function peaks
                const float peak = 0.1f * options.radius;
                const float intensity = (f::TAU * peak) * options.intensity;
                // always square AO result, as it looks much better
                const float power = options.power * 2.0f;

                const auto invProjection = inverse(cameraInfo.projection);
                const float inc = (1.0f / (sampleCount - 0.5f)) * spiralTurns * f::TAU;

                const mat4 screenFromClipMatrix{ mat4::row_major_init{
                        0.5 * desc.width, 0.0, 0.0, 0.5 * desc.width,
                        0.0, 0.5 * desc.height, 0.0, 0.5 * desc.height,
                        0.0, 0.0, 0.5, 0.5,
                        0.0, 0.0, 0.0, 1.0
                }};

                auto& material = computeBentNormals ?
                            getPostProcessMaterial("saoBentNormals") :
                            getPostProcessMaterial("sao");

                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
                SamplerParams depthSamplerParams{};
                depthSamplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                mi->setParameter("depth", depth, depthSamplerParams);
                mi->setParameter("screenFromViewMatrix",
                        mat4f(screenFromClipMatrix * cameraInfo.projection));
                mi->setParameter("resolution",
                        float4{ desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height });
                mi->setParameter("invRadiusSquared",
                        1.0f / (options.radius * options.radius));
                mi->setParameter("minHorizonAngleSineSquared",
                        std::pow(std::sin(options.minHorizonAngleRad), 2.0f));
                mi->setParameter("projectionScale",
                        projectionScale);
                mi->setParameter("projectionScaleRadius",
                        projectionScale * options.radius);
                mi->setParameter("positionParams", float2{
                        invProjection[0][0], invProjection[1][1] } * 2.0f);
                mi->setParameter("peak2", peak * peak);
                mi->setParameter("bias", options.bias);
                mi->setParameter("power", power);
                mi->setParameter("intensity", intensity / sampleCount);
                mi->setParameter("maxLevel", uint32_t(levelCount - 1));
                mi->setParameter("sampleCount", float2{ sampleCount, 1.0f / (sampleCount - 0.5f) });
                mi->setParameter("spiralTurns", spiralTurns);
                mi->setParameter("angleIncCosSin", float2{ std::cos(inc), std::sin(inc) });
                mi->setParameter("invFarPlane", 1.0f / -cameraInfo.zf);

                mi->setParameter("ssctShadowDistance", options.ssct.shadowDistance);
                mi->setParameter("ssctConeAngleTangeant", std::tan(options.ssct.lightConeRad * 0.5f));
                mi->setParameter("ssctContactDistanceMaxInv", 1.0f / options.ssct.contactDistanceMax);
                // light direction in view space
                const mat4f view{ cameraInfo.getUserViewMatrix() };
                const float3 l = normalize(
                        mat3f::getTransformForNormals(view.upperLeft())
                                * options.ssct.lightDirection);
                mi->setParameter("ssctIntensity",
                        options.ssct.enabled ? options.ssct.intensity : 0.0f);
                mi->setParameter("ssctVsLightDirection", -l);
                mi->setParameter("ssctDepthBias",
                        float2{ options.ssct.depthBias, options.ssct.depthSlopeBias });
                mi->setParameter("ssctSampleCount", uint32_t(options.ssct.sampleCount));
                mi->setParameter("ssctRayCount",
                        float2{ options.ssct.rayCount, 1.0f / options.ssct.rayCount });

                mi->commit(driver);
                mi->use(driver);

                PipelineState pipeline(material.getPipelineState(mEngine));
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::L;
                assert_invariant(ssao.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH);
                render(ssao, pipeline, driver);
            });

    FrameGraphId<FrameGraphTexture> ssao = SSAOPass->ssao;

    /*
     * Final separable bilateral blur pass
     */

    if (lowPassFilterEnabled) {
        ssao = bilateralBlurPass(fg, ssao, depth, { config.scale, 0 },
                cameraInfo.zf,
                TextureFormat::RGB8,
                config);

        ssao = bilateralBlurPass(fg, ssao, depth, { 0, config.scale },
                cameraInfo.zf,
                (highQualityUpsampling || computeBentNormals) ? TextureFormat::RGB8
                                                              : TextureFormat::R8,
                config);
    }

    return ssao;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::bilateralBlurPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphId<FrameGraphTexture> depth,
        math::int2 axis, float zf, TextureFormat format,
        BilateralPassConfig const& config) noexcept {
    assert_invariant(depth);

    struct BlurPassData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> blurred;
        FrameGraphId<FrameGraphTexture> ao;
        FrameGraphId<FrameGraphTexture> bn;
    };

    auto& blurPass = fg.addPass<BlurPassData>("Separable Blur Pass",
            [&](FrameGraph::Builder& builder, auto& data) {

                auto const& desc = builder.getDescriptor(input);

                data.input = builder.sample(input);

                data.blurred = builder.createTexture("Blurred output", {
                        .width = desc.width,
                        .height = desc.height,
                        .depth = desc.depth,
                        .type = desc.type,
                        .format = format });

                depth = builder.read(depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                if (config.bentNormals) {
                    data.ao = builder.createSubresource(data.blurred, "SSAO attachment", { .layer = 0 });
                    data.bn = builder.createSubresource(data.blurred, "Bent Normals attachment", { .layer = 1 });
                    data.ao = builder.write(data.ao, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                    data.bn = builder.write(data.bn, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                } else {
                    data.ao = data.blurred;
                    data.ao = builder.write(data.ao, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                }

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // We need to clear the buffers because we are skipping pixels at infinity (skybox)
                data.blurred = builder.write(data.blurred, FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.ao;
                descr.attachments.content.color[1] = data.bn;
                descr.attachments.content.depth = depth;
                descr.clearColor = { 1.0f };
                descr.clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::COLOR1;
                builder.declareRenderPass("Blurred target", descr);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto ssao = resources.getTexture(data.input);
                auto blurred = resources.getRenderPassInfo();
                auto const& desc = resources.getDescriptor(data.blurred);

                // unnormalized gaussian half-kernel of a given standard deviation
                // returns number of samples stored in array (max 16)
                constexpr size_t kernelArraySize = 16; // limited by bilateralBlur.mat
                auto gaussianKernel =
                        [kernelArraySize](float* outKernel, size_t gaussianWidth, float stdDev) -> uint32_t {
                    const size_t gaussianSampleCount = std::min(kernelArraySize, (gaussianWidth + 1u) / 2u);
                    for (size_t i = 0; i < gaussianSampleCount; i++) {
                        float x = i;
                        float g = std::exp(-(x * x) / (2.0f * stdDev * stdDev));
                        outKernel[i] = g;
                    }
                    return uint32_t(gaussianSampleCount);
                };

                float kGaussianSamples[kernelArraySize];
                uint32_t kGaussianCount = gaussianKernel(kGaussianSamples,
                        config.kernelSize, config.standardDeviation);

                auto& material = config.bentNormals ?
                        getPostProcessMaterial("bilateralBlurBentNormals") :
                        getPostProcessMaterial("bilateralBlur");
                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
                mi->setParameter("ssao", ssao, { /* only reads level 0 */ });
                mi->setParameter("axis", axis / float2{desc.width, desc.height});
                mi->setParameter("kernel", kGaussianSamples, kGaussianCount);
                mi->setParameter("sampleCount", kGaussianCount);
                mi->setParameter("farPlaneOverEdgeDistance", -zf / config.bilateralThreshold);

                mi->commit(driver);
                mi->use(driver);

                PipelineState pipeline(material.getPipelineState(mEngine));
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::L;
                render(blurred, pipeline, driver);
            });

    return blurPass->blurred;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::generateGaussianMipmap(FrameGraph& fg,
        const FrameGraphId<FrameGraphTexture> input, size_t levels,
        bool reinhard, size_t kernelWidth, float sigma) noexcept {

    auto const subResourceDesc = fg.getSubResourceDescriptor(input);

    // create one subresource per level to be generated from the input. These will be our
    // destinations.
    struct MipmapPassData {
        FixedCapacityVector<FrameGraphId<FrameGraphTexture>> out;
    };
    auto& mipmapPass = fg.addPass<MipmapPassData>("Mipmap Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.out.reserve(levels - 1);
                for (size_t i = 1; i < levels; i++) {
                    data.out.push_back(builder.createSubresource(input,
                            "Mipmap output", {
                                    .level = uint8_t(subResourceDesc.level + i),
                                    .layer = subResourceDesc.layer }));
                }
            });


    // Then generate a blur pass for each level, using the previous level as source
    auto from = input;
    for (size_t i = 0; i < levels - 1; i++) {
        auto output = mipmapPass->out[i];
        from = gaussianBlurPass(fg, from, output, reinhard, kernelWidth, sigma);
        reinhard = false; // only do the reinhard filtering on the first level
    }

    // return our original input (we only wrote into sub resources)
    return input;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::gaussianBlurPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphId<FrameGraphTexture> output,
        bool reinhard, size_t kernelWidth, const float sigma) noexcept {

    auto computeGaussianCoefficients =
            [kernelWidth, sigma](float2* kernel, size_t size) -> size_t {
        const float alpha = 1.0f / (2.0f * sigma * sigma);

        // number of positive-side samples needed, using linear sampling
        size_t m = (kernelWidth - 1) / 4 + 1;
        // clamp to what we have
        m = std::min(size, m);

        // How the kernel samples are stored:
        //  *===*---+---+---+---+---+---+
        //  | 0 | 1 | 2 | 3 | 4 | 5 | 6 |       Gaussian coefficients (right size)
        //  *===*-------+-------+-------+
        //  | 0 |   1   |   2   |   3   |       stored coefficients (right side)

        kernel[0].x = 1.0;
        kernel[0].y = 0.0;
        float totalWeight = kernel[0].x;

        for (size_t i = 1; i < m; i++) {
            float x0 = float(i * 2 - 1);
            float x1 = float(i * 2);
            float k0 = std::exp(-alpha * x0 * x0);
            float k1 = std::exp(-alpha * x1 * x1);

            // k * textureLod(..., o) with bilinear sampling is equivalent to:
            //      k * (s[0] * (1 - o) + s[1] * o)
            // solve:
            //      k0 = k * (1 - o)
            //      k1 = k * o

            float k = k0 + k1;
            float o = k1 / k;
            kernel[i].x = k;
            kernel[i].y = o;
            totalWeight += (k0 + k1) * 2.0f;
        }
        for (size_t i = 0; i < m; i++) {
            kernel[i].x *= 1.0f / totalWeight;
        }
        return m;
    };

    struct BlurPassData {
        FrameGraphId<FrameGraphTexture> in;
        FrameGraphId<FrameGraphTexture> out;
        FrameGraphId<FrameGraphTexture> temp;
    };

    // The effective kernel size is (kMaxPositiveKernelSize - 1) * 4 + 1.
    // e.g.: 5 positive-side samples, give 4+1+4=9 samples both sides
    // taking advantage of linear filtering produces an effective kernel of 8+1+8=17 samples
    // and because it's a separable filter, the effective 2D filter kernel size is 17*17
    // The total number of samples needed over the two passes is 18.

    auto& blurPass = fg.addPass<BlurPassData>("Gaussian Blur Pass (separable)",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto inDesc = builder.getDescriptor(input);

                if (!output) {
                    output = builder.createTexture("Blurred texture", inDesc);
                }

                auto outDesc = builder.getDescriptor(output);
                auto tempDesc = inDesc;
                tempDesc.width = outDesc.width; // width of the destination level (b/c we're blurring horizontally)
                tempDesc.levels = 1;
                tempDesc.depth = 1;
                // note: we don't systematically use a Sampler2D for the temp buffer because
                // this could force us to use two different programs below

                data.in = builder.sample(input);
                data.temp = builder.createTexture("Horizontal temporary buffer", tempDesc);
                data.temp = builder.sample(data.temp);
                data.temp = builder.declareRenderPass(data.temp);
                data.out = builder.declareRenderPass(output);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {

                // don't use auto for those, b/c the ide can't resolve them
                using FGTD = FrameGraphTexture::Descriptor;
                using FGTSD = FrameGraphTexture::SubResourceDescriptor;

                auto hwTempRT = resources.getRenderPassInfo(0);
                auto hwOutRT = resources.getRenderPassInfo(1);
                auto hwTemp = resources.getTexture(data.temp);
                auto hwIn = resources.getTexture(data.in);
                FGTD const& inDesc = resources.getDescriptor(data.in);
                FGTSD const& inSubDesc = resources.getSubResourceDescriptor(data.in);
                FGTD const& outDesc = resources.getDescriptor(data.out);
                FGTD const& tempDesc = resources.getDescriptor(data.temp);

                using namespace std::literals;
                std::string_view materialName;
                const bool is2dArray = inDesc.type == SamplerType::SAMPLER_2D_ARRAY;
                switch (backend::getFormatComponentCount(outDesc.format)) {
                    case 1: materialName  = is2dArray ?
                            "separableGaussianBlur1L"sv : "separableGaussianBlur1"sv;   break;
                    case 2: materialName  = is2dArray ?
                            "separableGaussianBlur2L"sv : "separableGaussianBlur2"sv;   break;
                    case 3: materialName  = is2dArray ?
                            "separableGaussianBlur3L"sv : "separableGaussianBlur3"sv;   break;
                    default: materialName = is2dArray ?
                            "separableGaussianBlur4L"sv : "separableGaussianBlur4"sv;   break;
                }

                auto const& separableGaussianBlur = getPostProcessMaterial(materialName);
                FMaterialInstance* const mi = separableGaussianBlur.getMaterialInstance(mEngine);
                const size_t kernelStorageSize = mi->getMaterial()->reflect("kernel")->size;

                float2 kernel[64];
                size_t m = computeGaussianCoefficients(kernel,
                        std::min(sizeof(kernel) / sizeof(*kernel), kernelStorageSize));

                // horizontal pass

                SamplerParams sourceSamplerParams{};
                sourceSamplerParams.filterMag = SamplerMagFilter::LINEAR;
                sourceSamplerParams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
                mi->setParameter("source", hwIn, sourceSamplerParams);
                mi->setParameter("level", float(inSubDesc.level));
                mi->setParameter("layer", float(inSubDesc.layer));
                mi->setParameter("reinhard", reinhard ? uint32_t(1) : uint32_t(0));
                mi->setParameter("axis",float2{ 1.0f / inDesc.width, 0 });
                mi->setParameter("count", (int32_t)m);
                mi->setParameter("kernel", kernel, m);

                // The framegraph only computes discard flags at FrameGraphPass boundaries
                hwTempRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                commitAndRender(hwTempRT, separableGaussianBlur, driver);

                // vertical pass
                UTILS_UNUSED_IN_RELEASE auto width = outDesc.width;
                UTILS_UNUSED_IN_RELEASE auto height = outDesc.height;
                assert_invariant(width == hwOutRT.params.viewport.width);
                assert_invariant(height == hwOutRT.params.viewport.height);

                SamplerParams sourceSamplerParams2{};
                sourceSamplerParams2.filterMin = SamplerMinFilter::LINEAR;
                sourceSamplerParams2.filterMag = SamplerMagFilter::LINEAR; /* level is always 0 */
                mi->setParameter("source", hwTemp, sourceSamplerParams2);
                mi->setParameter("level", 0.0f);
                mi->setParameter("layer", 0.0f);
                mi->setParameter("axis", float2{ 0, 1.0f / tempDesc.height });
                mi->commit(driver);
                // we don't need to call use() here, since it's the same material

                render(hwOutRT, separableGaussianBlur.getPipelineState(mEngine), driver);
            });

    return blurPass->out;
}

PostProcessManager::ScreenSpaceRefConfig PostProcessManager::prepareMipmapSSR(FrameGraph& fg,
        uint32_t width, uint32_t height, backend::TextureFormat format,
        float verticalFieldOfView, float2 scale) noexcept {

    // The kernel-size was determined empirically so that we don't get too many artifacts
    // due to the down-sampling with a box filter (which happens implicitly).
    // requires only 6 stored coefficients and 11 tap/pass
    // e.g.: size of 13 (4 stored coefficients)
    //      +-------+-------+-------*===*-------+-------+-------+
    //  ... | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | ...
    //      +-------+-------+-------*===*-------+-------+-------+
    constexpr size_t kernelSize = 21u;

    // The relation between the kernel size N and sigma (standard deviation) is 6*sigma - 1 = N
    // and is designed so the filter keeps its "gaussian-ness".
    // The standard deviation sigma0 is expressed in texels.
    constexpr float sigma0 = (kernelSize + 1u) / 6.0f;

    static_assert(kernelSize & 1u,
            "kernel size must be odd");

    static_assert((((kernelSize - 1u) / 2u) & 1u) == 0,
            "kernel positive side size must be even");

    // texel size of the reflection buffer in world units at 1 meter
    constexpr float d = 1.0f; // 1m
    const float texelSizeAtOneMeter = d * std::tan(verticalFieldOfView) / float(height);

    /*
     * 1. Relation between standard deviation and LOD
     * ----------------------------------------------
     *
     * The standard deviation doubles at each level (i.e. variance quadruples), however,
     * the mip-chain is constructed by successively blurring each level, which causes the
     * variance of a given level to increase by the variance of the previous level (i.e. variances
     * add under convolution). This results in a scaling of 2.23 (instead of 2) of the standard
     * deviation for each level: sqrt( 1^2 + 2^2 ) = sqrt(5) = 2.23;
     *
     *  The standard deviation is scaled by 2.23 each time we go one mip down,
     *  and our mipmap chain is built such that lod 0 is not blurred and lod 1 is blurred with
     *  sigma0 * 2 (because of the smaller resolution of lod 1). To simplify things a bit, we
     *  replace this factor by 2.23 (i.e. we pretend that lod 0 is blurred by sigma0).
     *  We then get:
     *      sigma = sigma0 * 2.23^lod
     *      lod   = log2(sigma / sigma0) / log2(2.23)
     *
     *      +------------------------------------------------+
     *      |  lod = [ log2(sigma) - log2(sigma0) ] * 0.8614 |
     *      +------------------------------------------------+
     *
     * 2. Relation between standard deviation and roughness
     * ----------------------------------------------------
     *
     *  The spherical gaussian approximation of the GGX distribution is given by:
     *
     *           1         2(cos(theta)-1)
     *         ------ exp(  --------------- )
     *         pi*a^2           a^2
     *
     *
     *  Which is equivalent to:
     *
     *      sqrt(2)
     *      ------- Gaussian(2 * sqrt(1 - cos(theta)), a)
     *       pi*a
     *
     *  But when we filter a frame, we're actually calculating:
     *
     *      Gaussian(d * tan(theta), sigma)
     *
     *  With d the distance from the eye to the center sample, theta the angle, and it turns out
     *  that sqrt(2) * tan(theta) is very close to 2 * sqrt(1 - cos(theta)) for small angles, we
     *  can make that assumption because our filter is not wide.
     *  The above can be rewritten as:
     *
     *      Gaussian(d * tan(theta), a * d / sqrt(2))
     *    = Gaussian(    tan(theta), a     / sqrt(2))
     *
     *  Which now matches the SG approximation (we don't mind the scale factor because it's
     *  calculated automatically in the shader).
     *
     *  We finally get that:
     *
     *      +---------------------+
     *      | sigma = a / sqrt(2) |
     *      +---------------------+
     *
     *
     * 3. Taking the resolution into account
     * -------------------------------------
     *
     *  sigma0 above is expressed in texels, but we're interested in world units. The texel
     *  size in world unit is given by:
     *
     *      +--------------------------------+
     *      |  s = d * tan(fov) / resolution |      with d distance to camera plane
     *      +--------------------------------+
     *
     * 4. Roughness to lod mapping
     * ---------------------------
     *
     *  Putting it all together we get:
     *
     *      lod   = [ log2(sigma)       - log2(           sigma0 * s ) ] * 0.8614
     *      lod   = [ log2(a / sqrt(2)) - log2(           sigma0 * s ) ] * 0.8614
     *      lod   = [ log2(a)           - log2( sqrt(2) * sigma0 * s ) ] * 0.8614
     *
     *   +-------------------------------------------------------------------------------------+
     *   | lod   = [ log2(a / d) - log2(sqrt(2) * sigma0 * (tan(fov) / resolution)) ] * 0.8614 |
     *   +-------------------------------------------------------------------------------------+
     */

    const float refractionLodOffset =
            -std::log2(f::SQRT2 * sigma0 * texelSizeAtOneMeter);

    // LOD count, we don't go lower than 16 texel in one dimension
    uint8_t roughnessLodCount = FTexture::maxLevelCount(width, height);
    roughnessLodCount = std::max(std::min(4, +roughnessLodCount), +roughnessLodCount - 4);

    // Make sure we keep the original buffer aspect ratio (this is because dynamic-resolution is
    // not necessarily homogenous).
    uint32_t w = width;
    uint32_t h = height;
    if (scale.x != scale.y) {
        // dynamic resolution wasn't homogenous, which would affect the blur, so make sure to
        // keep an intermediary buffer that has the same aspect-ratio as the original.
        const float homogenousScale = std::sqrt(scale.x * scale.y);
        w = uint32_t((homogenousScale / scale.x) * float(width));
        h = uint32_t((homogenousScale / scale.y) * float(height));
    }

    const FrameGraphTexture::Descriptor outDesc{
            .width = w, .height = h, .depth = 2,
            .levels = roughnessLodCount,
            .type = SamplerType::SAMPLER_2D_ARRAY,
            .format = format,
    };

    struct PrepareMipmapSSRPassData {
        FrameGraphId<FrameGraphTexture> ssr;
        FrameGraphId<FrameGraphTexture> refraction;
        FrameGraphId<FrameGraphTexture> reflection;
    };
    auto& pass = fg.addPass<PrepareMipmapSSRPassData>("Prepare MipmapSSR Pass",
            [&](FrameGraph::Builder& builder, auto& data){
                // create the SSR 2D array
                data.ssr = builder.createTexture("ssr", outDesc);
                // create the refraction subresource at layer 0
                data.refraction = builder.createSubresource(data.ssr, "refraction", {.layer = 0 });
                // create the reflection subresource at layer 1
                data.reflection = builder.createSubresource(data.ssr, "reflection", {.layer = 1 });
            });

    return {
            .ssr = pass->ssr,
            .refraction = pass->refraction,
            .reflection = pass->reflection,
            .lodOffset = refractionLodOffset,
            .roughnessLodCount = roughnessLodCount,
            .kernelSize = kernelSize,
            .sigma0 = sigma0
    };
}

FrameGraphId<FrameGraphTexture> PostProcessManager::generateMipmapSSR(
        PostProcessManager& ppm, FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphId<FrameGraphTexture> output,
        bool needInputDuplication, ScreenSpaceRefConfig const& config) noexcept {

    // descriptor of our actual input image (e.g. reflection buffer or refraction framebuffer)
    auto const& desc = fg.getDescriptor(input);

    // descriptor of the destination. output is a subresource (i.e. a layer of a 2D array)
    auto const& outDesc = fg.getDescriptor(output);

    /*
     * Resolve if needed + copy the image into first LOD
     */

    // needInputDuplication:
    // In some situations it's not possible to use the FrameGraph's forwardResource(),
    // as an optimization because the SSR buffer must be distinct from the color buffer
    // (input here), because we can't read and write into the same buffer (e.g. for refraction).

    if (needInputDuplication || outDesc.width != desc.width || outDesc.height != desc.height) {
        if (desc.samples > 1 &&
                outDesc.width == desc.width && outDesc.height == desc.height &&
                desc.format == outDesc.format) {
            // resolve directly into the destination
            input = ppm.resolveBaseLevelNoCheck(fg, "ssr", input, outDesc);
        } else {
            // first resolve (if needed)
            input = ppm.resolveBaseLevel(fg, "ssr", input);
            // then blit into an appropriate texture
            // this handles scaling, format conversion and mipmaping
            input = ppm.opaqueBlit(fg, input, { 0, 0, desc.width, desc.height }, outDesc);
        }
    }

    // A lot of magic happens right here. This forward call replaces 'input' (which is either
    // the actual input we received when entering this function, or, a resolved version of it) by
    // our output. Effectively, forcing the methods *above* to render into our output.
    output = fg.forwardResource(output, input);

    /*
     * Generate mipmap chain
     */

    return ppm.generateGaussianMipmap(fg, output, config.roughnessLodCount,
            true, config.kernelSize, config.sigma0);
}

FrameGraphId<FrameGraphTexture> PostProcessManager::dof(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphId<FrameGraphTexture> depth,
        const CameraInfo& cameraInfo,
        bool translucent,
        float bokehAspectRatio,
        const DepthOfFieldOptions& dofOptions) noexcept {

    assert_invariant(depth);

    const uint8_t variant = uint8_t(
            translucent ? PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

    const TextureFormat format = translucent ? TextureFormat::RGBA16F
                                             : TextureFormat::R11F_G11F_B10F;

    // rotate the bokeh based on the aperture diameter (i.e. angle of the blades)
    float bokehAngle = f::PI / 6.0f;
    if (dofOptions.maxApertureDiameter > 0.0f) {
        bokehAngle += f::PI_2 * saturate(cameraInfo.A / dofOptions.maxApertureDiameter);
    }

    /*
     * Circle-of-confusion
     * -------------------
     *
     * (see https://en.wikipedia.org/wiki/Circle_of_confusion)
     *
     * Ap: aperture [m]
     * f: focal length [m]
     * S: focus distance [m]
     * d: distance to the focal plane [m]
     *
     *            f      f     |      S  |
     * coc(d) =  --- . ----- . | 1 - --- |      in meters (m)
     *           Ap    S - f   |      d  |
     *
     *  This can be rewritten as:
     *
     *  coc(z) = Kc . Ks . (1 - S / d)          in pixels [px]
     *
     *                A.f
     *          Kc = -----          with: A = f / Ap
     *               S - f
     *
     *          Ks = height [px] / SensorSize [m]        pixel conversion
     *
     *
     *  We also introduce a "cocScale" factor for artistic reasons (see code below).
     *
     *
     *  Object distance computation (d)
     *  -------------------------------
     *
     *  1/d is computed from the depth buffer value as:
     *  (note: our Z clip space is 1 to 0 (inverted DirectX NDC))
     *
     *          screen-space -> clip-space -> view-space -> distance (*-1)
     *
     *   v_s = { x, y, z, 1 }                     // screen space (reversed-z)
     *   v_c = v_s                                // clip space (matches screen space)
     *   v   = inverse(projection) * v_c          // view space
     *   d   = -v.z / v.w                         // view space distance to camera
     *   1/d = -v.w / v.z
     *
     * Assuming a generic projection matrix of the form:
     *
     *    a 0 x 0
     *    0 b y 0
     *    0 0 A B
     *    0 0 C 0
     *
     * It comes that:
     *
     *           C          A
     *    1/d = --- . z  - ---
     *           B          B
     *
     * note: Here the result doesn't depend on {x, y}. This wouldn't be the case with a
     *       tilt-shift lens.
     *
     * Mathematica code:
     *      p = {{a, 0, b, 0}, {0, c, d, 0}, {0, 0, m22, m32}, {0, 0, m23, 0}};
     *      v = {x, y, z, 1};
     *      f = Inverse[p].v;
     *      Simplify[f[[4]]/f[[3]]]
     *
     * Plugging this back into the expression of: coc(z) = Kc . Ks . (1 - S / d)
     * We get that:  coc(z) = C0 * z + C1
     * With: C0 = - Kc * Ks * S * -C / B
     *       C1 =   Kc * Ks * (1 + S * A / B)
     *
     * It's just a madd!
     */
    const float focusDistance = cameraInfo.d;
    auto const& desc = fg.getDescriptor<FrameGraphTexture>(input);
    const float Kc = (cameraInfo.A * cameraInfo.f) / (focusDistance - cameraInfo.f);
    const float Ks = ((float)desc.height) / FCamera::SENSOR_SIZE;
    const float K  = dofOptions.cocScale * Ks * Kc;

    auto const& p = cameraInfo.projection;
    const float2 cocParams = {
              K * focusDistance * p[2][3] / p[3][2],
              K * (1.0 + focusDistance * p[2][2] / p[3][2])
    };

    /*
     * dofResolution is used to chose between full- or quarter-resolution
     * for the DoF calculations. Set to [1] for full resolution or [2] for quarter-resolution.
     */
    const uint32_t dofResolution = dofOptions.nativeResolution ? 1u : 2u;

    auto const& colorDesc = fg.getDescriptor(input);
    const uint32_t width  = colorDesc.width  / dofResolution;
    const uint32_t height = colorDesc.height / dofResolution;

    // at full resolution, 4 "safe" levels are guaranteed
    constexpr const uint32_t maxMipLevels = 4u;

    // compute numbers of "safe" levels (should be 4, but can be 3 at half res)
    const uint8_t mipmapCount = std::min(maxMipLevels, ctz(width | height));
    assert_invariant(mipmapCount == maxMipLevels || mipmapCount == maxMipLevels-1);

    /*
     * Setup:
     *      - Downsample of color buffer
     *      - Separate near & far field
     *      - Generate Circle Of Confusion buffer
     */

    struct PostProcessDofDownsample {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> outColor;
        FrameGraphId<FrameGraphTexture> outCoc;
    };

    auto& ppDoFDownsample = fg.addPass<PostProcessDofDownsample>("DoF Downsample",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.color = builder.sample(input);
                data.depth = builder.sample(depth);

                FrameGraphTexture::Descriptor dofTexDescr;
                dofTexDescr.width = width;
                dofTexDescr.height = height;
                dofTexDescr.levels = mipmapCount;
                dofTexDescr.format = format;
                data.outColor = builder.createTexture("dof downsample output", dofTexDescr);

                FrameGraphTexture::Descriptor cocTexDescr;
                cocTexDescr.width = width;
                cocTexDescr.height = height;
                cocTexDescr.levels = mipmapCount;
                cocTexDescr.format = TextureFormat::R16F;
                // the next stage expects min/max CoC in the red/green channel
                cocTexDescr.swizzle.r = backend::TextureSwizzle::CHANNEL_0;
                cocTexDescr.swizzle.g = backend::TextureSwizzle::CHANNEL_0;
                data.outCoc = builder.createTexture("dof CoC output", cocTexDescr);
                data.outColor = builder.write(data.outColor, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.outCoc   = builder.write(data.outCoc,   FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.outColor;
                descr.attachments.content.color[1] = data.outCoc;
                builder.declareRenderPass("DoF Target", descr);
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto const& out = resources.getRenderPassInfo();
                auto color = resources.getTexture(data.color);
                auto depth = resources.getTexture(data.depth);
                auto const& material = (dofResolution == 1) ?
                        getPostProcessMaterial("dofCoc") :
                        getPostProcessMaterial("dofDownsample");
                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
                SamplerParams colorSamplerParams{};
                colorSamplerParams.filterMin = SamplerMinFilter::NEAREST;
                mi->setParameter("color", color, colorSamplerParams);
                SamplerParams depthSamplerParams{};
                depthSamplerParams.filterMin = SamplerMinFilter::NEAREST;
                mi->setParameter("depth", depth, depthSamplerParams);
                mi->setParameter("cocParams", cocParams);
                mi->setParameter("cocClamp", float2{
                    -(dofOptions.maxForegroundCOC ? dofOptions.maxForegroundCOC : DOF_DEFAULT_MAX_COC),
                      dofOptions.maxBackgroundCOC ? dofOptions.maxBackgroundCOC : DOF_DEFAULT_MAX_COC});
                mi->setParameter("texelSize", float2{ 1.0f / colorDesc.width, 1.0f / colorDesc.height });
                commitAndRender(out, material, driver);
            });

    /*
     * Setup (Continued)
     *      - Generate mipmaps
     */

    struct PostProcessDofMipmap {
        FrameGraphId<FrameGraphTexture> inOutColor;
        FrameGraphId<FrameGraphTexture> inOutCoc;
        uint32_t rp[maxMipLevels];
    };

    assert_invariant(mipmapCount - 1 <= sizeof(PostProcessDofMipmap::rp) / sizeof(uint32_t));

    auto& ppDoFMipmap = fg.addPass<PostProcessDofMipmap>("DoF Mipmap",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.inOutColor = builder.sample(ppDoFDownsample->outColor);
                data.inOutCoc   = builder.sample(ppDoFDownsample->outCoc);
                for (size_t i = 0; i < mipmapCount - 1u; i++) {
                    // make sure inputs are always multiple of two (should be true by construction)
                    // (this is so that we can compute clean mip levels)
                    assert_invariant((FTexture::valueForLevel(uint8_t(i), fg.getDescriptor(data.inOutColor).width ) & 0x1u) == 0);
                    assert_invariant((FTexture::valueForLevel(uint8_t(i), fg.getDescriptor(data.inOutColor).height) & 0x1u) == 0);

                    auto inOutColor = builder.createSubresource(data.inOutColor, "Color mip", { .level = uint8_t(i + 1) });
                    auto inOutCoc   = builder.createSubresource(data.inOutCoc, "Coc mip", { .level = uint8_t(i + 1) });

                    inOutColor = builder.write(inOutColor, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                    inOutCoc   = builder.write(inOutCoc,   FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                    FrameGraphRenderPass::Descriptor descr;
                    descr.attachments.content.color[0] = data.inOutColor;
                    descr.attachments.content.color[1] = data.inOutCoc;

                    data.rp[i] = builder.declareRenderPass("DoF Target", descr);
                }
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto desc       = resources.getDescriptor(data.inOutColor);
                auto inOutColor = resources.getTexture(data.inOutColor);
                auto inOutCoc   = resources.getTexture(data.inOutCoc);

                auto const& material = getPostProcessMaterial("dofMipmap");
                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);

                SamplerParams colorSamplerParams{};
                colorSamplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                mi->setParameter("color", inOutColor, colorSamplerParams);
                SamplerParams cocSamplerParams{};
                cocSamplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                mi->setParameter("coc",   inOutCoc,   cocSamplerParams);
                mi->use(driver);

                const PipelineState pipeline(material.getPipelineState(mEngine, variant));

                for (size_t level = 0 ; level < mipmapCount - 1u ; level++) {
                    const float w = FTexture::valueForLevel(level, desc.width);
                    const float h = FTexture::valueForLevel(level, desc.height);

                    auto const& out = resources.getRenderPassInfo(data.rp[level]);
                    driver.setMinMaxLevels(inOutColor, level, level);
                    driver.setMinMaxLevels(inOutCoc, level, level);
                    mi->setParameter("mip", uint32_t(level));
                    mi->setParameter("weightScale", 0.5f / float(1u<<level));   // FIXME: halfres?
                    mi->setParameter("texelSize", float2{ 1.0f / w, 1.0f / h });
                    mi->commit(driver);
                    render(out, pipeline, driver);
                }
                driver.setMinMaxLevels(inOutColor, 0, mipmapCount - 1u);
                driver.setMinMaxLevels(inOutCoc, 0, mipmapCount - 1u);
            });

    /*
     * Setup (Continued)
     *      - Generate min/max tiles for far/near fields (continued)
     */

    auto inTilesCocMinMax = ppDoFDownsample->outCoc;

    // TODO: Should the tile size be in real pixels? i.e. always 16px instead of being dependant on
    //       the DoF effect resolution?
    // Size of a tile in full-resolution pixels -- must match TILE_SIZE in dofDilate.mat
    const size_t tileSize = 16;

    // we assume the width/height is already multiple of 16
    assert_invariant(!(colorDesc.width  & 0xF) && !(colorDesc.height & 0xF));
    const uint32_t tileBufferWidth  = colorDesc.width  / dofResolution;
    const uint32_t tileBufferHeight = colorDesc.height / dofResolution;
    const size_t tileReductionCount = ctz(tileSize / dofResolution);

    struct PostProcessDofTiling1 {
        FrameGraphId<FrameGraphTexture> inCocMinMax;
        FrameGraphId<FrameGraphTexture> outTilesCocMinMax;
    };

    const bool textureSwizzleSupported = Texture::isTextureSwizzleSupported(mEngine);
    for (size_t i = 0; i < tileReductionCount; i++) {
        auto& ppDoFTiling = fg.addPass<PostProcessDofTiling1>("DoF Tiling",
                [&](FrameGraph::Builder& builder, auto& data) {

                    // this must be true by construction
                    assert_invariant(((tileBufferWidth  >> i) & 1u) == 0);
                    assert_invariant(((tileBufferHeight >> i) & 1u) == 0);

                    data.inCocMinMax = builder.sample(inTilesCocMinMax);
                    data.outTilesCocMinMax = builder.createTexture("dof tiles output", {
                            .width  = tileBufferWidth  >> (i + 1u),
                            .height = tileBufferHeight >> (i + 1u),
                            .format = TextureFormat::RG16F
                    });
                    data.outTilesCocMinMax = builder.declareRenderPass(data.outTilesCocMinMax);
                },
                [=](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {
                    auto const& inputDesc = resources.getDescriptor(data.inCocMinMax);
                    auto const& out = resources.getRenderPassInfo();
                    auto inCocMinMax = resources.getTexture(data.inCocMinMax);
                    auto const& material = (!textureSwizzleSupported && (i == 0)) ?
                            getPostProcessMaterial("dofTilesSwizzle") :
                            getPostProcessMaterial("dofTiles");
                    FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
                    SamplerParams cocMinMaxSamplerParams{};
                    cocMinMaxSamplerParams.filterMin = SamplerMinFilter::NEAREST;
                    mi->setParameter("cocMinMax", inCocMinMax, cocMinMaxSamplerParams);
                    mi->setParameter("texelSize", float2{ 1.0f / inputDesc.width, 1.0f / inputDesc.height });
                    commitAndRender(out, material, driver);
                });
        inTilesCocMinMax = ppDoFTiling->outTilesCocMinMax;
    }

    /*
     * Dilate tiles
     */

    // This is a small helper that does one round of dilate
    auto dilate = [&](FrameGraphId<FrameGraphTexture> input) -> FrameGraphId<FrameGraphTexture> {

        struct PostProcessDofDilate {
            FrameGraphId<FrameGraphTexture> inTilesCocMinMax;
            FrameGraphId<FrameGraphTexture> outTilesCocMinMax;
        };

        auto& ppDoFDilate = fg.addPass<PostProcessDofDilate>("DoF Dilate",
                [&](FrameGraph::Builder& builder, auto& data) {
                    auto const& inputDesc = fg.getDescriptor(input);
                    data.inTilesCocMinMax = builder.sample(input);
                    data.outTilesCocMinMax = builder.createTexture("dof dilated tiles output", inputDesc);
                    data.outTilesCocMinMax = builder.declareRenderPass(data.outTilesCocMinMax );
                },
                [=](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {
                    auto const& out = resources.getRenderPassInfo();
                    auto inTilesCocMinMax = resources.getTexture(data.inTilesCocMinMax);
                    auto const& material = getPostProcessMaterial("dofDilate");
                    FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
                    SamplerParams tilesSamplerParams{};
                    tilesSamplerParams.filterMin = SamplerMinFilter::NEAREST;
                    mi->setParameter("tiles", inTilesCocMinMax, tilesSamplerParams);
                    commitAndRender(out, material, driver);
                });
        return ppDoFDilate->outTilesCocMinMax;
    };

    // Tiles of 16 full-resolution pixels requires two dilate rounds to accommodate our max Coc of 32 pixels
    // (note: when running at half-res, the tiles are 8 half-resolution pixels, and still need two
    //  dilate rounds to accommodate the mac CoC pf 16 half-resolution pixels)
    auto dilated = dilate(inTilesCocMinMax);
    dilated = dilate(dilated);

    /*
     * DoF blur pass
     */

    struct PostProcessDof {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> coc;
        FrameGraphId<FrameGraphTexture> tilesCocMinMax;
        FrameGraphId<FrameGraphTexture> outColor;
        FrameGraphId<FrameGraphTexture> outAlpha;
    };

    auto& ppDoF = fg.addPass<PostProcessDof>("DoF",
            [&](FrameGraph::Builder& builder, auto& data) {

                data.color          = builder.sample(ppDoFMipmap->inOutColor);
                data.coc            = builder.sample(ppDoFMipmap->inOutCoc);
                data.tilesCocMinMax = builder.sample(dilated);

                data.outColor = builder.createTexture("dof color output", {
                        .width  = colorDesc.width  / dofResolution,
                        .height = colorDesc.height / dofResolution,
                        .format = fg.getDescriptor(data.color).format
                });
                data.outAlpha = builder.createTexture("dof alpha output", {
                        .width  = colorDesc.width  / dofResolution,
                        .height = colorDesc.height / dofResolution,
                        .format = TextureFormat::R8
                });
                data.outColor  = builder.write(data.outColor, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.outAlpha  = builder.write(data.outAlpha, FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.outColor;
                descr.attachments.content.color[1] = data.outAlpha;

                builder.declareRenderPass("DoF Target", descr);
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto const& out = resources.getRenderPassInfo();

                auto color          = resources.getTexture(data.color);
                auto coc            = resources.getTexture(data.coc);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& inputDesc = resources.getDescriptor(data.coc);

                auto const& material = getPostProcessMaterial("dof");
                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);
                // it's not safe to use bilinear filtering in the general case (causes artifacts around edges)

                SamplerParams colorSamplerParams{};
                colorSamplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                mi->setParameter("color", color,
                        colorSamplerParams);

                SamplerParams colorLinearSamplerParams{};
                colorLinearSamplerParams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
                mi->setParameter("colorLinear", color,
                        colorLinearSamplerParams);

                SamplerParams cocSamplerParams{};
                cocSamplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                mi->setParameter("coc", coc,
                        cocSamplerParams);

                SamplerParams tilesSamplerParams{};
                tilesSamplerParams.filterMin = SamplerMinFilter::NEAREST;
                mi->setParameter("tiles", tilesCocMinMax,
                        tilesSamplerParams);
                mi->setParameter("cocToTexelScale", float2{
                        bokehAspectRatio / (inputDesc.width  * dofResolution),
                                     1.0 / (inputDesc.height * dofResolution)
                });
                mi->setParameter("cocToPixelScale", (1.0f / dofResolution));
                mi->setParameter("ringCounts", float4{
                    dofOptions.foregroundRingCount ? dofOptions.foregroundRingCount : DOF_DEFAULT_RING_COUNT,
                    dofOptions.backgroundRingCount ? dofOptions.backgroundRingCount : DOF_DEFAULT_RING_COUNT,
                    dofOptions.fastGatherRingCount ? dofOptions.fastGatherRingCount : DOF_DEFAULT_RING_COUNT,
                    0.0 // unused for now
                });
                mi->setParameter("bokehAngle",  bokehAngle);
                commitAndRender(out, material, driver);
            });

    /*
     * DoF median
     */

    struct PostProcessDofMedian {
        FrameGraphId<FrameGraphTexture> inColor;
        FrameGraphId<FrameGraphTexture> inAlpha;
        FrameGraphId<FrameGraphTexture> tilesCocMinMax;
        FrameGraphId<FrameGraphTexture> outColor;
        FrameGraphId<FrameGraphTexture> outAlpha;
    };

    auto& ppDoFMedian = fg.addPass<PostProcessDofMedian>("DoF Median",
            [&](FrameGraph::Builder& builder, auto& data) {

                data.inColor        = builder.sample(ppDoF->outColor);
                data.inAlpha        = builder.sample(ppDoF->outAlpha);
                data.tilesCocMinMax = builder.sample(dilated);

                data.outColor = builder.createTexture("dof color output", fg.getDescriptor(data.inColor));
                data.outAlpha = builder.createTexture("dof alpha output", fg.getDescriptor(data.inAlpha));
                data.outColor = builder.write(data.outColor, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.outAlpha = builder.write(data.outAlpha, FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.outColor;
                descr.attachments.content.color[1] = data.outAlpha;

                builder.declareRenderPass("DoF Target", descr);
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto const& out = resources.getRenderPassInfo();

                auto inColor        = resources.getTexture(data.inColor);
                auto inAlpha        = resources.getTexture(data.inAlpha);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& material = getPostProcessMaterial("dofMedian");
                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);

                SamplerParams dofSamplerParams{};
                dofSamplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                mi->setParameter("dof",   inColor,        dofSamplerParams);

                SamplerParams alphaSamplerParams{};
                alphaSamplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
                mi->setParameter("alpha", inAlpha,        alphaSamplerParams);

                SamplerParams tilesSamplerParams{};
                tilesSamplerParams.filterMin = SamplerMinFilter::NEAREST;
                mi->setParameter("tiles", tilesCocMinMax, tilesSamplerParams);

                commitAndRender(out, material, driver);
            });


    /*
     * DoF recombine
     */

    auto outColor = ppDoFMedian->outColor;
    auto outAlpha = ppDoFMedian->outAlpha;
    if (dofOptions.filter == DepthOfFieldOptions::Filter::NONE) {
        outColor = ppDoF->outColor;
        outAlpha = ppDoF->outAlpha;
    }

    struct PostProcessDofCombine {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> dof;
        FrameGraphId<FrameGraphTexture> alpha;
        FrameGraphId<FrameGraphTexture> tilesCocMinMax;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto& ppDoFCombine = fg.addPass<PostProcessDofCombine>("DoF combine",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.color      = builder.sample(input);
                data.dof        = builder.sample(outColor);
                data.alpha      = builder.sample(outAlpha);
                data.tilesCocMinMax = builder.sample(dilated);
                auto const& inputDesc = fg.getDescriptor(data.color);
                data.output = builder.createTexture("DoF output", inputDesc);
                data.output = builder.declareRenderPass(data.output);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto const& out = resources.getRenderPassInfo();

                auto color      = resources.getTexture(data.color);
                auto dof        = resources.getTexture(data.dof);
                auto alpha      = resources.getTexture(data.alpha);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& material = getPostProcessMaterial("dofCombine");
                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);

                SamplerParams colorSamplerParams{};
                colorSamplerParams.filterMin = SamplerMinFilter::NEAREST;
                mi->setParameter("color", color, colorSamplerParams);

                SamplerParams dofSamplerParams{};
                dofSamplerParams.filterMag = SamplerMagFilter::NEAREST;
                mi->setParameter("dof",   dof,   dofSamplerParams);

                SamplerParams alphaSamplerParams{};
                alphaSamplerParams.filterMag = SamplerMagFilter::NEAREST;
                mi->setParameter("alpha", alpha, alphaSamplerParams);

                SamplerParams tilesSamplerParams{};
                tilesSamplerParams.filterMin = SamplerMinFilter::NEAREST;
                mi->setParameter("tiles", tilesCocMinMax, tilesSamplerParams);

                commitAndRender(out, material, driver);
            });

    return ppDoFCombine->output;
}

PostProcessManager::BloomPassOutput PostProcessManager::bloom(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        BloomOptions& inoutBloomOptions,
        backend::TextureFormat outFormat,
        math::float2 scale) noexcept {
    return bloomPass(fg, input, outFormat, inoutBloomOptions, scale);
}

PostProcessManager::BloomPassOutput PostProcessManager::bloomPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, TextureFormat outFormat,
        BloomOptions& inoutBloomOptions, float2 scale) noexcept {
    // Figure out a good size for the bloom buffer.
    auto const& desc = fg.getDescriptor(input);

    // width and height after dynamic resolution upscaling
    const float aspect = (float(desc.width) * scale.y) / (float(desc.height) * scale.x);

    // compute the desired bloom buffer size
    float bloomHeight = float(inoutBloomOptions.resolution);
    float bloomWidth  = bloomHeight * aspect;

    // Anamorphic bloom by always scaling down one of the dimension -- we do this (as opposed
    // to scaling up) so that the amount of blooming doesn't decrease. However, the resolution
    // decreases, meaning that the user might need to adjust the BloomOptions::resolution and
    // BloomOptions::levels.
    if (inoutBloomOptions.anamorphism >= 1.0f) {
        bloomWidth *= 1.0f / inoutBloomOptions.anamorphism;
    } else {
        bloomHeight *= inoutBloomOptions.anamorphism;
    }

    // convert back to integer width/height
    const uint32_t width  = std::max(1u, uint32_t(std::floor(bloomWidth)));
    const uint32_t height = std::max(1u, uint32_t(std::floor(bloomHeight)));

    // we might need to adjust the max # of levels
    const uint32_t major = uint32_t(std::max(bloomWidth,  bloomHeight));
    const uint8_t maxLevels = FTexture::maxLevelCount(major);
    inoutBloomOptions.levels = std::min(inoutBloomOptions.levels, maxLevels);
    inoutBloomOptions.levels = std::min(inoutBloomOptions.levels, kMaxBloomLevels);

    if (2 * width < desc.width || 2 * height < desc.height) {
        // if we're scaling down by more than 2x, prescale the image with a blit to improve
        // performance. This is important on mobile/tilers.
        input = opaqueBlit(fg, input, { 0, 0, desc.width, desc.height }, {
                .width = std::max(1u, desc.width / 2),
                .height = std::max(1u, desc.height / 2),
                .format = outFormat
        });
    }

    struct BloomPassData {
        FrameGraphId<FrameGraphTexture> in;
        FrameGraphId<FrameGraphTexture> out;
        FrameGraphId<FrameGraphTexture> stage;
        uint32_t outRT[kMaxBloomLevels];
        uint32_t stageRT[kMaxBloomLevels];
    };

    // downsample phase
    auto& bloomDownsamplePass = fg.addPass<BloomPassData>("Bloom Downsample",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.in = builder.sample(input);
                data.out = builder.createTexture("Bloom Out Texture", {
                        .width = width,
                        .height = height,
                        .levels = inoutBloomOptions.levels,
                        .format = outFormat
                });
                data.out = builder.sample(data.out);

                data.stage = builder.createTexture("Bloom Stage Texture", {
                        .width = width,
                        .height = height,
                        .levels = inoutBloomOptions.levels,
                        .format = outFormat
                });
                data.stage = builder.sample(data.stage);

                for (size_t i = 0; i < inoutBloomOptions.levels; i++) {
                    auto out = builder.createSubresource(data.out, "Bloom Out Texture mip",
                            { .level = uint8_t(i) });
                    auto stage = builder.createSubresource(data.stage,
                            "Bloom Stage Texture mip", { .level = uint8_t(i) });
                    builder.declareRenderPass(out, &data.outRT[i]);
                    builder.declareRenderPass(stage, &data.stageRT[i]);
                }
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto hwIn = resources.getTexture(data.in);
                auto hwOut = resources.getTexture(data.out);
                auto hwStage = resources.getTexture(data.stage);

                mi->use(driver);

                SamplerParams samplerParams{};
                samplerParams.filterMag = SamplerMagFilter::LINEAR;
                samplerParams.filterMin = SamplerMinFilter::LINEAR;

                mi->setParameter("source", hwIn, samplerParams);
                mi->setParameter("level", 0.0f);
                mi->setParameter("threshold", inoutBloomOptions.threshold ? 1.0f : 0.0f);
                mi->setParameter("invHighlight",
                        std::isinf(inoutBloomOptions.highlight) ? 0.0f : 1.0f
                                                                            / inoutBloomOptions.highlight);
                mis[1]->setParameter("source", hwStage, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });
                mis[2]->setParameter("source", hwIn, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });

                for (auto* mi : mis) {
                    mi->setParameter("level", 0.0f);
                    mi->setParameter("threshold", inoutBloomOptions.threshold ? 1.0f : 0.0f);
                    mi->setParameter("invHighlight", std::isinf(inoutBloomOptions.highlight)
                            ? 0.0f : 1.0f / inoutBloomOptions.highlight);
                    mi->commit(driver);
                }

                const PipelineState pipeline(material.getPipelineState(mEngine));

                { // first iteration
                    auto hwDstRT = resources.getRenderPassInfo(data.outRT[0]);
                    hwDstRT.params.flags.discardStart = TargetBufferFlags::COLOR;
                    hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;
                    mis[2]->use(driver);
                    render(hwDstRT, pipeline, driver);
                }


                    // prepare the next level
                    SamplerParams samplerParamsi{};
                    samplerParamsi.filterMag = SamplerMagFilter::LINEAR;
                    samplerParamsi.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
                    mi->setParameter("source", parity ? hwOut : hwStage, samplerParamsi);
                    mi->setParameter("level", float(i));
                }
            });

    FrameGraphId<FrameGraphTexture> output = bloomDownsamplePass->out;
    FrameGraphId<FrameGraphTexture> stage = bloomDownsamplePass->stage;

    // flare pass
    auto flare = flarePass(fg, bloomDownsamplePass->out, width, height, outFormat, inoutBloomOptions);

    // upsample phase
    auto& bloomUpsamplePass = fg.addPass<BloomPassData>("Bloom Upsample",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.out = builder.sample(output);
                data.stage = builder.sample(stage);
                for (size_t i = 0; i < inoutBloomOptions.levels; i++) {
                    auto out = builder.createSubresource(data.out, "Bloom Out Texture mip",
                            { .level = uint8_t(i) });
                    auto stage = builder.createSubresource(data.stage,
                            "Bloom Stage Texture mip", { .level = uint8_t(i) });
                    builder.declareRenderPass(out, &data.outRT[i]);
                    builder.declareRenderPass(stage, &data.stageRT[i]);
                }
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {

                auto hwOut = resources.getTexture(data.out);
                auto hwStage = resources.getTexture(data.stage);
                auto const& outDesc = resources.getDescriptor(data.out);

                auto const& material = getPostProcessMaterial("bloomUpsample");
                auto const* ma = material.getMaterial(mEngine);

                FMaterialInstance* mis[] = {
                        ma->createInstance("bloomUpsample-ping"),
                        ma->createInstance("bloomUpsample-pong"),
                };

                mis[0]->setParameter("source", hwOut, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });

                mis[1]->setParameter("source", hwStage, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });

                PipelineState pipeline(material.getPipelineState(mEngine));
                pipeline.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
                pipeline.rasterState.blendFunctionDstRGB = BlendFunction::ONE;

                for (size_t j = inoutBloomOptions.levels, i = j - 1; i >= 1; i--, j++) {
                    const size_t parity = 1u - (j % 2u);

                    auto hwDstRT = resources.getRenderPassInfo(
                            parity ? data.outRT[i - 1] : data.stageRT[i - 1]);
                    hwDstRT.params.flags.discardStart = TargetBufferFlags::NONE; // b/c we'll blend
                    hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                    auto w = FTexture::valueForLevel(i - 1, outDesc.width);
                    auto h = FTexture::valueForLevel(i - 1, outDesc.height);

                    mi->setParameter("resolution", float4{ w, h, 1.0f / w, 1.0f / h });
                    SamplerParams sourceSamplerParams{};
                    sourceSamplerParams.filterMag = SamplerMagFilter::LINEAR;
                    sourceSamplerParams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
                    mi->setParameter("source", parity ? hwStage : hwOut, sourceSamplerParams);
                    mi->setParameter("level", float(i));
                    mi->commit(driver);
                    render(hwDstRT, pipeline, driver);
                }

                for (auto& mi : mis) {
                    mEngine.destroy(mi);
                }

                // Every other level is missing from the out texture, so we need to do
                // blits to complete the chain.
                const SamplerMagFilter filter = SamplerMagFilter::NEAREST;
                for (size_t i = 1; i < inoutBloomOptions.levels; i += 2) {
                    auto in = resources.getRenderPassInfo(data.stageRT[i]);
                    auto out = resources.getRenderPassInfo(data.outRT[i]);
                    driver.blit(TargetBufferFlags::COLOR, out.target, out.params.viewport,
                            in.target, in.params.viewport, filter);
                }
            });

    return { bloomUpsamplePass->out, flare };
}

UTILS_NOINLINE
FrameGraphId<FrameGraphTexture> PostProcessManager::flarePass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        uint32_t width, uint32_t height,
        backend::TextureFormat outFormat,
        BloomOptions const& bloomOptions) noexcept {

    struct FlarePassData {
        FrameGraphId<FrameGraphTexture> in;
        FrameGraphId<FrameGraphTexture> out;
    };
    auto& flarePass = fg.addPass<FlarePassData>("Flare",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.in = builder.sample(input);
                data.out = builder.createTexture("Flare Texture", {
                        .width  = std::max(1u, width  / 2),
                        .height = std::max(1u, height / 2),
                        .format = outFormat
                });
                data.out = builder.declareRenderPass(data.out);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto in = resources.getTexture(data.in);
                auto out = resources.getRenderPassInfo(0);
                const float aspectRatio = float(width) / height;

                auto const& material = getPostProcessMaterial("flare");
                FMaterialInstance* mi = material.getMaterialInstance(mEngine);

                SamplerParams colorSamplerParams{};
                colorSamplerParams.filterMag = SamplerMagFilter::LINEAR;
                colorSamplerParams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
                mi->setParameter("color", in, colorSamplerParams);

                mi->setParameter("level", 1.0f);    // adjust with resolution
                mi->setParameter("aspectRatio", float2{ aspectRatio, 1.0f / aspectRatio });
                mi->setParameter("threshold",
                        float2{ bloomOptions.ghostThreshold, bloomOptions.haloThreshold });
                mi->setParameter("chromaticAberration", bloomOptions.chromaticAberration);
                mi->setParameter("ghostCount", (float)bloomOptions.ghostCount);
                mi->setParameter("ghostSpacing", bloomOptions.ghostSpacing);
                mi->setParameter("haloRadius", bloomOptions.haloRadius);
                mi->setParameter("haloThickness", bloomOptions.haloThickness);

                commitAndRender(out, material, driver);
            });

    constexpr float kernelWidth = 9;
    constexpr float sigma = (kernelWidth + 1.0f) / 6.0f;
    auto flare = gaussianBlurPass(fg, flarePass->out, {}, false, kernelWidth, sigma);
    return flare;
}

UTILS_NOINLINE
static float4 getVignetteParameters(VignetteOptions const& options,
        uint32_t width, uint32_t height) noexcept {
    if (options.enabled) {
        // Vignette params
        // From 0.0 to 0.5 the vignette is a rounded rect that turns into an oval
        // From 0.5 to 1.0 the vignette turns from oval to circle
        float oval = min(options.roundness, 0.5f) * 2.0f;
        float circle = (max(options.roundness, 0.5f) - 0.5f) * 2.0f;
        float roundness = (1.0f - oval) * 6.0f + oval;

        // Mid point varies during the oval/rounded section of roundness
        // We also modify it to emphasize feathering
        float midPoint = (1.0f - options.midPoint) * mix(2.2f, 3.0f, oval)
                         * (1.0f - 0.1f * options.feather);

        // Radius of the rounded corners as a param to pow()
        float radius = roundness *
                mix(1.0f + 4.0f * (1.0f - options.feather), 1.0f, std::sqrt(oval));

        // Factor to transform oval into circle
        float aspect = mix(1.0f, float(width) / float(height), circle);

        return float4{ midPoint, radius, aspect, options.feather };
    }

    // Set half-max to show disabled
    return float4{ std::numeric_limits<half>::max() };
}

void PostProcessManager::colorGradingPrepareSubpass(DriverApi& driver,
        const FColorGrading* colorGrading, ColorGradingConfig const& colorGradingConfig,
        VignetteOptions const& vignetteOptions, uint32_t width, uint32_t height) noexcept {

    float4 vignetteParameters = getVignetteParameters(vignetteOptions, width, height);

    auto const& material = getPostProcessMaterial("colorGradingAsSubpass");
    FMaterialInstance* mi = material.getMaterialInstance(mEngine);

    SamplerParams samplerParams{};
    samplerParams.filterMag = SamplerMagFilter::LINEAR,
    samplerParams.filterMin = SamplerMinFilter::LINEAR,
    samplerParams.wrapS = SamplerWrapMode::CLAMP_TO_EDGE,
    samplerParams.wrapT = SamplerWrapMode::CLAMP_TO_EDGE,
    samplerParams.wrapR = SamplerWrapMode::CLAMP_TO_EDGE,
        samplerParams.anisotropyLog2 = 0;

    mi->setParameter("lut", colorGrading->getHwHandle(), samplerParams);

    const float lutDimension = float(colorGrading->getDimension());
    mi->setParameter("lutSize", float2{
        0.5f / lutDimension, (lutDimension - 1.0f) / lutDimension,
    });

    const float temporalNoise = mUniformDistribution(mEngine.getRandomEngine());

    mi->setParameter("vignette", vignetteParameters);
    mi->setParameter("vignetteColor", vignetteOptions.color);
    mi->setParameter("dithering", colorGradingConfig.dithering);
    mi->setParameter("fxaa", colorGradingConfig.fxaa);
    mi->setParameter("temporalNoise", temporalNoise);
    mi->commit(driver);

    // load both variants
    material.getMaterial(mEngine)->prepareProgram(Variant{Variant::type_t(PostProcessVariant::TRANSLUCENT)});
    material.getMaterial(mEngine)->prepareProgram(Variant{Variant::type_t(PostProcessVariant::OPAQUE)});
}

void PostProcessManager::colorGradingSubpass(DriverApi& driver,
        ColorGradingConfig const& colorGradingConfig) noexcept {
    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    auto const& material = getPostProcessMaterial("colorGradingAsSubpass");
    // the UBO has been set and committed in colorGradingPrepareSubpass()
    FMaterialInstance* mi = material.getMaterialInstance(mEngine);
    mi->use(driver);
    const Variant::type_t variant = Variant::type_t(colorGradingConfig.translucent ?
            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

    driver.nextSubpass();
    driver.draw(material.getPipelineState(mEngine, variant), fullScreenRenderPrimitive, 1);
}

void PostProcessManager::customResolvePrepareSubpass(DriverApi& driver, CustomResolveOp op) noexcept {
    auto const& material = getPostProcessMaterial("customResolveAsSubpass");
    FMaterialInstance* mi = material.getMaterialInstance(mEngine);
    mi->setParameter("direction", op == CustomResolveOp::COMPRESS ? 1.0f : -1.0f),
    mi->commit(driver);
    material.getMaterial(mEngine)->prepareProgram(Variant{});
}

void PostProcessManager::customResolveSubpass(DriverApi& driver) noexcept {
    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();
    auto const& material = getPostProcessMaterial("customResolveAsSubpass");
    // the UBO has been set and committed in colorGradingPrepareSubpass()
    FMaterialInstance* mi = material.getMaterialInstance(mEngine);
    mi->use(driver);
    driver.nextSubpass();
    driver.draw(material.getPipelineState(mEngine), fullScreenRenderPrimitive, 1);
}

FrameGraphId<FrameGraphTexture> PostProcessManager::customResolveUncompressPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> inout) noexcept {
    struct UncompressData {
        FrameGraphId<FrameGraphTexture> inout;
    };
    auto& detonemapPass = fg.addPass<UncompressData>("Uncompress Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.inout = builder.read(inout, FrameGraphTexture::Usage::SUBPASS_INPUT);
                data.inout = builder.write(data.inout, FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.inout;

                builder.declareRenderPass("Uncompress target", descr);
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                customResolvePrepareSubpass(driver, CustomResolveOp::UNCOMPRESS);
                auto out = resources.getRenderPassInfo();
                out.params.subpassMask = 1;
                driver.beginRenderPass(out.target, out.params);
                customResolveSubpass(driver);
                driver.endRenderPass();
            });
    return detonemapPass->inout;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::colorGrading(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, filament::Viewport const& vp,
        FrameGraphId<FrameGraphTexture> bloom,
        FrameGraphId<FrameGraphTexture> flare,
        FColorGrading const* colorGrading,
        ColorGradingConfig const& colorGradingConfig,
        BloomOptions const& bloomOptions,
        VignetteOptions const& vignetteOptions) noexcept
{
    FrameGraphId<FrameGraphTexture> bloomDirt;
    FrameGraphId<FrameGraphTexture> starburst;

    float bloomStrength = 0.0f;
    if (bloomOptions.enabled) {
        bloomStrength = clamp(bloomOptions.strength, 0.0f, 1.0f);
        if (bloomOptions.dirt) {
            FTexture* fdirt = upcast(bloomOptions.dirt);
            FrameGraphTexture frameGraphTexture{ .handle = fdirt->getHwHandle() };
            bloomDirt = fg.import("dirt", {
                    .width = (uint32_t)fdirt->getWidth(0u),
                    .height = (uint32_t)fdirt->getHeight(0u),
                    .format = fdirt->getFormat()
            }, FrameGraphTexture::Usage::SAMPLEABLE, frameGraphTexture);
        }

        if (bloomOptions.lensFlare && bloomOptions.starburst) {
            starburst = fg.import("starburst", {
                    .width = 256, .height = 1, .format = TextureFormat::R8
            }, FrameGraphTexture::Usage::SAMPLEABLE,
                    FrameGraphTexture{ .handle = mStarburstTexture });
        }
    }

    struct PostProcessColorGrading {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> bloom;
        FrameGraphId<FrameGraphTexture> flare;
        FrameGraphId<FrameGraphTexture> dirt;
        FrameGraphId<FrameGraphTexture> starburst;
    };

    auto& ppColorGrading = fg.addPass<PostProcessColorGrading>("colorGrading",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("colorGrading output", {
                        .width = vp.width,
                        .height = vp.height,
                        .format = colorGradingConfig.ldrFormat
                });
                data.output = builder.declareRenderPass(data.output);

                if (bloom) {
                    data.bloom = builder.sample(bloom);
                }
                if (bloomDirt) {
                    data.dirt = builder.sample(bloomDirt);
                }
                if (bloomOptions.lensFlare && flare) {
                    data.flare = builder.sample(flare);
                    if (starburst) {
                        data.starburst = builder.sample(starburst);
                    }
                }
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                Handle<HwTexture> colorTexture = resources.getTexture(data.input);

                Handle<HwTexture> bloomTexture =
                        data.bloom ? resources.getTexture(data.bloom) : getZeroTexture();

                Handle<HwTexture> flareTexture =
                        data.flare ? resources.getTexture(data.flare) : getZeroTexture();

                Handle<HwTexture> dirtTexture =
                        data.dirt ? resources.getTexture(data.dirt) : getOneTexture();

                Handle<HwTexture> starburstTexture =
                        data.starburst ? resources.getTexture(data.starburst) : getOneTexture();

                auto const& out = resources.getRenderPassInfo();

                auto const& material = getPostProcessMaterial("colorGrading");
                FMaterialInstance* mi = material.getMaterialInstance(mEngine);

                SamplerParams samplerParams{};
                samplerParams.filterMag = SamplerMagFilter::LINEAR;
                samplerParams.filterMin = SamplerMinFilter::LINEAR;
                mi->setParameter("lut", colorGrading->getHwHandle(), samplerParams);

                const float lutDimension = float(colorGrading->getDimension());
                mi->setParameter("lutSize", float2{
                        0.5f / lutDimension, (lutDimension - 1.0f) / lutDimension,
                });
                mi->setParameter("colorBuffer", colorTexture, { /* shader uses texelFetch */ });
                mi->setParameter("bloomBuffer", bloomTexture, samplerParams);
                mi->setParameter("flareBuffer", flareTexture, samplerParams);
                mi->setParameter("dirtBuffer", dirtTexture, samplerParams);

                SamplerParams samplerParams2{};
                samplerParams2.filterMag = SamplerMagFilter::LINEAR;
                samplerParams2.filterMag = SamplerMagFilter::LINEAR;
                samplerParams2.wrapS = SamplerWrapMode::REPEAT;
                samplerParams2.wrapT = SamplerWrapMode::REPEAT;

                mi->setParameter("starburstBuffer", starburstTexture, samplerParams2);

                // Bloom params
                float4 bloomParameters{
                    bloomStrength / float(bloomOptions.levels),
                    1.0f,
                    (bloomOptions.enabled && bloomOptions.dirt) ? bloomOptions.dirtStrength : 0.0f,
                    bloomOptions.lensFlare ? bloomStrength : 0.0f
                };
                if (bloomOptions.blendMode == BloomOptions::BlendMode::INTERPOLATE) {
                    bloomParameters.y = 1.0f - bloomParameters.x;
                }

                auto const& input = resources.getDescriptor(data.input);
                auto const& output = resources.getDescriptor(data.output);
                float4 vignetteParameters = getVignetteParameters(
                        vignetteOptions, output.width, output.height);

                const float temporalNoise = mUniformDistribution(mEngine.getRandomEngine());

                mi->setParameter("dithering", colorGradingConfig.dithering);
                mi->setParameter("bloom", bloomParameters);
                mi->setParameter("vignette", vignetteParameters);
                mi->setParameter("vignetteColor", vignetteOptions.color);
                mi->setParameter("fxaa", colorGradingConfig.fxaa);
                mi->setParameter("temporalNoise", temporalNoise);
                mi->setParameter("viewport", float4{
                        (float)vp.left   / input.width,
                        (float)vp.bottom / input.height,
                        (float)vp.width  / input.width,
                        (float)vp.height / input.height
                });

                const uint8_t variant = uint8_t(colorGradingConfig.translucent ?
                            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                commitAndRender(out, material, variant, driver);
            }
    );

    return ppColorGrading->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::fxaa(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, filament::Viewport const& vp,
        TextureFormat outFormat, bool translucent) noexcept {

    struct PostProcessFXAA {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto& ppFXAA = fg.addPass<PostProcessFXAA>("fxaa",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("fxaa output", {
                        .width = vp.width,
                        .height = vp.height,
                        .format = outFormat
                });
                data.output = builder.declareRenderPass(data.output);
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto const& inDesc = resources.getDescriptor(data.input);
                auto const& texture = resources.getTexture(data.input);
                auto const& out = resources.getRenderPassInfo();

                auto const& material = getPostProcessMaterial("fxaa");
                FMaterialInstance* mi = material.getMaterialInstance(mEngine);

                SamplerParams colorSamplerParams{};
                colorSamplerParams.filterMag = SamplerMagFilter::LINEAR;
                colorSamplerParams.filterMin = SamplerMinFilter::LINEAR;
                mi->setParameter("colorBuffer", texture, colorSamplerParams);
                mi->setParameter("viewport", float4{
                        (float)vp.left   / inDesc.width,
                        (float)vp.bottom / inDesc.height,
                        (float)vp.width  / inDesc.width,
                        (float)vp.height / inDesc.height
                });
                mi->setParameter("texelSize", 1.0f / float2{ inDesc.width, inDesc.height });

                const uint8_t variant = uint8_t(translucent ?
                    PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                commitAndRender(out, material, variant, driver);
            });

    return ppFXAA->output;
}

void PostProcessManager::prepareTaa(FrameGraph& fg, filament::Viewport const& svp,
        FrameHistory& frameHistory,
        FrameHistoryEntry::TemporalAA FrameHistoryEntry::*pTaa,
        CameraInfo* inoutCameraInfo,
        PerViewUniforms& uniforms) const noexcept {
    auto const& previous = frameHistory.getPrevious().*pTaa;
    auto& current = frameHistory.getCurrent().*pTaa;

    // compute projection
    current.projection = mat4f{ inoutCameraInfo->projection * inoutCameraInfo->getUserViewMatrix() };
    current.frameId = previous.frameId + 1;

    // sample position within a pixel [-0.5, 0.5]
    const float2 jitter = halton(previous.frameId) - 0.5f;
    current.jitter = jitter;

    float2 jitterInClipSpace = jitter * (2.0f / float2{ svp.width, svp.height });

    // update projection matrix
    inoutCameraInfo->projection[2].xy -= jitterInClipSpace;
    // VERTEX_DOMAIN_DEVICE doesn't apply the projection, but it still needs this
    // clip transform, so we apply it separately (see main.vs)
    inoutCameraInfo->clipTransfrom.zw -= jitterInClipSpace;

    fg.addTrivialSideEffectPass("Jitter Camera",
            [=, &uniforms] (DriverApi& driver) {
        uniforms.prepareCamera(*inoutCameraInfo);
        uniforms.commit(driver);
    });
}

FrameGraphId<FrameGraphTexture> PostProcessManager::taa(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphId<FrameGraphTexture> depth,
        FrameHistory& frameHistory,
        FrameHistoryEntry::TemporalAA FrameHistoryEntry::*pTaa,
        TemporalAntiAliasingOptions const& taaOptions,
        ColorGradingConfig const& colorGradingConfig) noexcept {
    assert_invariant(depth);

    auto const& previous = frameHistory.getPrevious().*pTaa;
    auto& current = frameHistory.getCurrent().*pTaa;

    FrameGraphId<FrameGraphTexture> colorHistory;
    mat4f historyProjection;
    if (UTILS_UNLIKELY(!previous.color.handle)) {
        // if we don't have a history yet, just use the current color buffer as history
        colorHistory = input;
        historyProjection = current.projection;
    } else {
        colorHistory = fg.import("TAA history", previous.desc,
                FrameGraphTexture::Usage::SAMPLEABLE, previous.color);
        historyProjection = previous.projection;
    }

    struct TAAData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> history;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> tonemappedOutput;
    };
    auto& taaPass = fg.addPass<TAAData>("TAA",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto desc = fg.getDescriptor(input);
                data.color = builder.sample(input);
                data.depth = builder.sample(depth);
                data.history = builder.sample(colorHistory);
                data.output = builder.createTexture("TAA output", desc);
                data.output = builder.write(data.output);
                if (colorGradingConfig.asSubpass) {
                    data.tonemappedOutput = builder.createTexture("Tonemapped Buffer", {
                            .width = desc.width,
                            .height = desc.height,
                            .format = colorGradingConfig.ldrFormat
                    });
                    data.tonemappedOutput = builder.write(data.tonemappedOutput, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                    data.output = builder.read(data.output, FrameGraphTexture::Usage::SUBPASS_INPUT);
                }
                data.output = builder.write(data.output, FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.output;
                descr.attachments.content.color[1] = data.tonemappedOutput;
                builder.declareRenderPass("TAA target", descr);
            },
            [=, &current](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {

                constexpr mat4f normalizedToClip{mat4f::row_major_init{
                        2, 0, 0, -1,
                        0, 2, 0, -1,
                        0, 0, 1,  0,
                        0, 0, 0,  1
                }};

                constexpr float2 sampleOffsets[9] = {
                        { -1.0f, -1.0f }, {  0.0f, -1.0f }, {  1.0f, -1.0f },
                        { -1.0f,  0.0f }, {  0.0f,  0.0f }, {  1.0f,  0.0f },
                        { -1.0f,  1.0f }, {  0.0f,  1.0f }, {  1.0f,  1.0f },
                };

                float sum = 0.0;
                float weights[9];

                // this doesn't get vectorized (probably because of exp()), so don't bother
                // unrolling it.
                #pragma nounroll
                for (size_t i = 0; i < 9; i++) {
                    float2 d = sampleOffsets[i] - current.jitter;
                    d *= 1.0f / taaOptions.filterWidth;
                    // this is a gaussian fit of a 3.3 Blackman Harris window
                    // see: "High Quality Temporal Supersampling" by Brian Karis
                    weights[i] = std::exp2(-3.3f * (d.x * d.x + d.y * d.y));
                    sum += weights[i];
                }
                for (auto& w : weights) {
                    w /= sum;
                }

                auto out = resources.getRenderPassInfo();
                auto color = resources.getTexture(data.color);
                auto depth = resources.getTexture(data.depth);
                auto history = resources.getTexture(data.history);

                auto const& material = getPostProcessMaterial("taa");
                FMaterialInstance* mi = material.getMaterialInstance(mEngine);
                mi->setParameter("color",  color, {});  // nearest
                mi->setParameter("depth",  depth, {});  // nearest
                mi->setParameter("alpha", taaOptions.feedback);

                SamplerParams historySamplerParams{};
                historySamplerParams.filterMag = SamplerMagFilter::LINEAR;
                historySamplerParams.filterMin = SamplerMinFilter::LINEAR;
                mi->setParameter("history", history, historySamplerParams);
                mi->setParameter("filterWeights",  weights, 9);
                mi->setParameter("reprojection",
                        historyProjection *
                        inverse(current.projection) *
                        normalizedToClip);

                mi->commit(driver);
                mi->use(driver);

                const uint8_t variant = uint8_t(colorGradingConfig.translucent ?
                        PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                if (colorGradingConfig.asSubpass) {
                    out.params.subpassMask = 1;
                }
                PipelineState pipeline(material.getPipelineState(mEngine, variant));
                driver.beginRenderPass(out.target, out.params);
                driver.draw(pipeline, mEngine.getFullScreenRenderPrimitive(), 1);
                if (colorGradingConfig.asSubpass) {
                    colorGradingSubpass(driver, colorGradingConfig);
                }
                driver.endRenderPass();
            });

    input = colorGradingConfig.asSubpass ? taaPass->tonemappedOutput : taaPass->output;

    struct ExportColorHistoryData {
        FrameGraphId<FrameGraphTexture> color;
    };
    auto& exportHistoryPass = fg.addPass<ExportColorHistoryData>("Export TAA history",
            [&](FrameGraph::Builder& builder, auto& data) {
                // We need to use sideEffect here to ensure this pass won't be culled.
                // The "output" of this pass is going to be used during the next frame as
                // an "import".
                builder.sideEffect();
                data.color = builder.sample(input); // FIXME: an access must be declared for detach(), why?
            }, [&current](FrameGraphResources const& resources, auto const& data,
                    backend::DriverApi&) {
                resources.detach(data.color,
                        &current.color, &current.desc);
            });

    return exportHistoryPass->color;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::opaqueBlit(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, filament::Viewport const& vp,
        FrameGraphTexture::Descriptor const& outDesc,
        SamplerMagFilter filter) noexcept {

    struct PostProcessScaling {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto& ppBlit = fg.addPass<PostProcessScaling>("opaque blit",
            [&](FrameGraph::Builder& builder, auto& data) {

                // we currently have no use for this case, so we just assert. This is better for now to trap
                // cases that we might not intend.
                assert_invariant(fg.getDescriptor(input).samples <= 1);

                data.output = builder.declareRenderPass(
                        builder.createTexture("opaque blit output", outDesc));

                data.input =  builder.read(input);

                // We use a RenderPass for the source here, instead of just creating a render
                // target from data.input in the execute closure, because data.input may refer to
                // an imported render target and in this case data.input won't resolve to an actual
                // HwTexture handle. Using a RenderPass works because data.input will resolve
                // to the actual imported render target and will have the correct viewport
                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.input;
                descr.viewport = vp;

                builder.declareRenderPass("opaque blit input", descr);
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto out = resources.getRenderPassInfo(0);
                auto in = resources.getRenderPassInfo(1);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, out.params.viewport,
                        in.target, in.params.viewport,
                        filter);
            });

    // we rely on automatic culling of unused render passes
    return ppBlit->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::upscale(FrameGraph& fg, bool translucent,
        DynamicResolutionOptions dsrOptions, FrameGraphId<FrameGraphTexture> input,
        filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc,
        backend::SamplerMagFilter filter) noexcept {

    if (UTILS_LIKELY(!translucent && dsrOptions.quality == QualityLevel::LOW)) {
        return opaqueBlit(fg, input, vp, outDesc, filter);
    }

    // The code below cannot handle sub-resources
    assert_invariant(fg.getSubResourceDescriptor(input).layer == 0);
    assert_invariant(fg.getSubResourceDescriptor(input).level == 0);

    const bool lowQualityFallback = translucent && dsrOptions.quality != QualityLevel::LOW;
    if (lowQualityFallback) {
        // FidelityFX-FSR doesn't support the alpha channel currently
        dsrOptions.quality = QualityLevel::LOW;
    }

    const bool twoPassesEASU = mWorkaroundSplitEasu &&
            (dsrOptions.quality == QualityLevel::MEDIUM
                || dsrOptions.quality == QualityLevel::HIGH);

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> depth;
    };

    auto& ppQuadBlit = fg.addPass<QuadBlitData>(dsrOptions.enabled ? "upscaling" : "compositing",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("upscaled output", outDesc);

                if (twoPassesEASU) {
                    // FIXME: it would be better to use the stencil buffer in this case (less bandwidth)
                    data.depth = builder.createTexture("upscaled output depth", {
                        .width = outDesc.width,
                        .height = outDesc.height,
                        .format = TextureFormat::DEPTH16
                    });
                    data.depth = builder.write(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                }

                data.output = builder.write(data.output, FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = data.output;
                descr.attachments.content.depth = { data.depth };
                descr.clearFlags = TargetBufferFlags::DEPTH;

                builder.declareRenderPass(builder.getName(data.output), descr);
            },
            [this, twoPassesEASU, dsrOptions, vp, translucent, filter](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {

                // helper to set the EASU uniforms
                auto setEasuUniforms = [vp, backend = mEngine.getBackend()](FMaterialInstance* mi,
                        FrameGraphTexture::Descriptor const& inputDesc,
                        FrameGraphTexture::Descriptor const& outputDesc) {
                    FSRUniforms uniforms{};
                    FSR_ScalingSetup(&uniforms, {
                            .backend = backend,
                            .input = vp,
                            .inputWidth = inputDesc.width,
                            .inputHeight = inputDesc.height,
                            .outputWidth = outputDesc.width,
                            .outputHeight = outputDesc.height,
                    });
                    mi->setParameter("EasuCon0", uniforms.EasuCon0);
                    mi->setParameter("EasuCon1", uniforms.EasuCon1);
                    mi->setParameter("EasuCon2", uniforms.EasuCon2);
                    mi->setParameter("EasuCon3", uniforms.EasuCon3);
                    mi->setParameter("textureSize",
                            float2{ inputDesc.width, inputDesc.height });
                };

                // helper to enable blending
                auto enableTranslucentBlending = [](PipelineState& pipeline) {
                    pipeline.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionDstRGB = BlendFunction::ONE_MINUS_SRC_ALPHA;
                    pipeline.rasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
                };

                auto color = resources.getTexture(data.input);
                auto const& inputDesc = resources.getDescriptor(data.input);
                auto const& outputDesc = resources.getDescriptor(data.output);

                // --------------------------------------------------------------------------------
                // set uniforms

                PostProcessMaterial* splitEasuMaterial = nullptr;
                PostProcessMaterial* easuMaterial = nullptr;

                if (twoPassesEASU) {
                    splitEasuMaterial = &getPostProcessMaterial("fsr_easu_mobileF");
                    auto* mi = splitEasuMaterial->getMaterialInstance(mEngine);
                    setEasuUniforms(mi, inputDesc, outputDesc);

                    SamplerParams colorSamplerParams{};
                    colorSamplerParams.filterMag = SamplerMagFilter::LINEAR;
                    mi->setParameter("color", color, colorSamplerParams);
                    mi->setParameter("resolution",
                            float4{ outputDesc.width, outputDesc.height,
                                    1.0f / outputDesc.width, 1.0f / outputDesc.height });
                    mi->commit(driver);
                    mi->use(driver);
                }

                { // just a scope to not leak local variables
                    const std::string_view blitterNames[4] = {
                            "blitLow", "fsr_easu_mobile", "fsr_easu_mobile", "fsr_easu" };
                    unsigned index = std::min(3u, (unsigned)dsrOptions.quality);
                    easuMaterial = &getPostProcessMaterial(blitterNames[index]);
                    auto* mi = easuMaterial->getMaterialInstance(mEngine);
                    if (dsrOptions.quality != QualityLevel::LOW) {
                        setEasuUniforms(mi, inputDesc, outputDesc);
                    }

                    SamplerParams samplerParams{};
                    samplerParams.filterMag = (dsrOptions.quality == QualityLevel::LOW) ?
                                                  filter : SamplerMagFilter::LINEAR;
                    mi->setParameter("color", color, samplerParams);
                    mi->setParameter("resolution",
                            float4{ outputDesc.width, outputDesc.height,
                                    1.0f / outputDesc.width, 1.0f / outputDesc.height });
                    mi->setParameter("viewport", float4{
                            (float)vp.left   / inputDesc.width,
                            (float)vp.bottom / inputDesc.height,
                            (float)vp.width  / inputDesc.width,
                            (float)vp.height / inputDesc.height
                    });
                    mi->commit(driver);
                    mi->use(driver);
                }

                // --------------------------------------------------------------------------------
                // render pass with draw calls

                auto fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

                auto out = resources.getRenderPassInfo();

                if (UTILS_UNLIKELY(twoPassesEASU)) {
                    PipelineState pipeline0(splitEasuMaterial->getPipelineState(mEngine));
                    PipelineState pipeline1(easuMaterial->getPipelineState(mEngine));
                    pipeline1.rasterState.depthFunc = backend::SamplerCompareFunc::NE;
                    if (translucent) {
                        enableTranslucentBlending(pipeline0);
                        enableTranslucentBlending(pipeline1);
                    }
                    driver.beginRenderPass(out.target, out.params);
                    driver.draw(pipeline0, fullScreenRenderPrimitive, 1);
                    driver.draw(pipeline1, fullScreenRenderPrimitive, 1);
                    driver.endRenderPass();
                } else {
                    PipelineState pipeline(easuMaterial->getPipelineState(mEngine));
                    if (translucent) {
                        enableTranslucentBlending(pipeline);
                    }
                    driver.beginRenderPass(out.target, out.params);
                    driver.draw(pipeline, fullScreenRenderPrimitive, 1);
                    driver.endRenderPass();
                }
            });

    auto output = ppQuadBlit->output;

    // if we had to take the low quality fallback, we still do the "sharpen pass"
    if (dsrOptions.sharpness > 0.0f && (dsrOptions.quality != QualityLevel::LOW || lowQualityFallback)) {
        auto& ppFsrRcas = fg.addPass<QuadBlitData>("FidelityFX FSR1 Rcas",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.input = builder.sample(output);
                    data.output = builder.createTexture("FFX FSR1 Rcas output", outDesc);
                    data.output = builder.declareRenderPass(data.output);
                },
                [=](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {

                    auto color = resources.getTexture(data.input);
                    auto out = resources.getRenderPassInfo();
                    auto const& outputDesc = resources.getDescriptor(data.output);

                    auto& material = getPostProcessMaterial("fsr_rcas");
                    FMaterialInstance* const mi = material.getMaterialInstance(mEngine);

                    FSRUniforms uniforms;
                    FSR_SharpeningSetup(&uniforms, { .sharpness = 2.0f - 2.0f * dsrOptions.sharpness });
                    mi->setParameter("RcasCon", uniforms.RcasCon);
                    mi->setParameter("color", color, { }); // uses texelFetch
                    mi->setParameter("resolution", float4{
                            outputDesc.width, outputDesc.height,
                            1.0f / outputDesc.width, 1.0f / outputDesc.height });
                    mi->commit(driver);
                    mi->use(driver);

                    const uint8_t variant = uint8_t(translucent ?
                            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                    PipelineState pipeline(material.getPipelineState(mEngine, variant));
                    render(out, pipeline, driver);
                });

        output = ppFsrRcas->output;
    }

    // we rely on automatic culling of unused render passes
    return output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::blit(FrameGraph& fg, bool translucent,
        FrameGraphId<FrameGraphTexture> input, filament::Viewport const& vp,
        FrameGraphTexture::Descriptor const& outDesc,
        backend::SamplerMagFilter filter) noexcept {
    // for now, we implement this by calling upscale() with the low-quality setting.
    return upscale(fg, translucent, { .quality = QualityLevel::LOW }, input, vp, outDesc, filter);
}

FrameGraphId<FrameGraphTexture> PostProcessManager::resolveBaseLevel(FrameGraph& fg,
        const char* outputBufferName, FrameGraphId<FrameGraphTexture> input) noexcept {
    // Don't do anything if we're not a MSAA buffer
    auto desc = fg.getDescriptor(input);
    if (desc.samples <= 1) {
        return input;
    }
    desc.samples = 0;
    desc.levels = 1;
    return resolveBaseLevelNoCheck(fg, outputBufferName, input, desc);
}

FrameGraphId<FrameGraphTexture> PostProcessManager::resolveBaseLevelNoCheck(FrameGraph& fg,
        const char* outputBufferName, FrameGraphId<FrameGraphTexture> input,
        FrameGraphTexture::Descriptor const& desc) noexcept {

    struct ResolveData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        backend::TargetBufferFlags inFlags;
        FrameGraphTexture::Usage usage;
    };

    auto& ppResolve = fg.addPass<ResolveData>("resolve",
            [&](FrameGraph::Builder& builder, auto& data) {
                FrameGraphRenderPass::Descriptor rpDesc;

                auto& rpDescAttachment = isDepthFormat(desc.format) ?
                                   rpDesc.attachments.content.depth :
                                   rpDesc.attachments.content.color[0];

                data.usage = isDepthFormat(desc.format) ?
                             FrameGraphTexture::Usage::DEPTH_ATTACHMENT :
                             FrameGraphTexture::Usage::COLOR_ATTACHMENT;

                data.inFlags = isDepthFormat(desc.format) ?
                               backend::TargetBufferFlags::DEPTH :
                               backend::TargetBufferFlags::COLOR0;

                data.input = builder.read(input, data.usage);

                rpDescAttachment = builder.createTexture(outputBufferName, desc);
                rpDescAttachment = builder.write(rpDescAttachment, data.usage);
                data.output = rpDescAttachment;
                builder.declareRenderPass("Resolve Pass", rpDesc);
            },
            [](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto inDesc = resources.getDescriptor(data.input);
                auto in = resources.getTexture(data.input);
                auto out = resources.getRenderPassInfo();

                assert_invariant(in);

                Handle<HwRenderTarget> inRt;
                if (data.usage == FrameGraphTexture::Usage::COLOR_ATTACHMENT) {
                    inRt = driver.createRenderTarget(data.inFlags,
                            out.params.viewport.width, out.params.viewport.height,
                            inDesc.samples, {{ in }}, {}, {});
                }
                if (data.usage == FrameGraphTexture::Usage::DEPTH_ATTACHMENT) {
                    inRt = driver.createRenderTarget(data.inFlags,
                            out.params.viewport.width, out.params.viewport.height,
                            inDesc.samples, {}, { in }, {});
                }

                driver.blit(data.inFlags,
                        out.target, out.params.viewport, inRt, out.params.viewport,
                        SamplerMagFilter::NEAREST);

                driver.destroyRenderTarget(inRt);
            });

    return ppResolve->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::vsmMipmapPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, uint8_t layer, size_t level,
        math::float4 clearColor, bool finalize) noexcept {

    struct VsmMipData {
        FrameGraphId<FrameGraphTexture> in;
    };

    auto& depthMipmapPass = fg.addPass<VsmMipData>("VSM Generate Mipmap Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                const char* name = builder.getName(input);
                data.in = builder.sample(input);

                FrameGraphTexture::SubResourceDescriptor subResDescr;
                subResDescr.level = uint8_t(level + 1);
                subResDescr.layer = layer;

                auto out = builder.createSubresource(data.in, "Mip level", subResDescr);

                out = builder.write(out, FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                FrameGraphRenderPass::Descriptor descr;
                descr.attachments.content.color[0] = out;
                descr.clearColor = clearColor;
                descr.clearFlags = TargetBufferFlags::COLOR;

                builder.declareRenderPass(name, descr);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto in = resources.getTexture(data.in);
                auto out = resources.getRenderPassInfo();

                auto const& inDesc = resources.getDescriptor(data.in);
                auto width = inDesc.width;
                assert_invariant(width == inDesc.height);
                int dim = width >> (level + 1);

                driver.setMinMaxLevels(in, level, level);

                auto& material = getPostProcessMaterial("vsmMipmap");

                // When generating shadow map mip levels, we want to preserve the 1 texel border.
                // (note clearing never respects the scissor in filament)
                PipelineState pipeline(material.getPipelineState(mEngine));
                pipeline.scissor = { 1u, 1u, dim - 2u, dim - 2u };

                FMaterialInstance* const mi = material.getMaterialInstance(mEngine);

                SamplerParams samplerParams{};
                samplerParams.filterMag = SamplerMagFilter::LINEAR;
                samplerParams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
                mi->setParameter("color", in, samplerParams);
                mi->setParameter("level", uint32_t(level));
                mi->setParameter("layer", uint32_t(layer));
                mi->setParameter("uvscale", 1.0f / dim);
                mi->commit(driver);
                mi->use(driver);
                render(out, pipeline, driver);

                if (finalize) {
                   driver.setMinMaxLevels(in, 0, level);
                }
            });

    return depthMipmapPass->in;
}

} // namespace filament
