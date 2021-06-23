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

#include "fg2/FrameGraph.h"
#include "fg2/FrameGraphResources.h"

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
    mEngine = nullptr;
    mData = nullptr;
    mMaterial = nullptr;            // aliased to mEngine
}

PostProcessManager::PostProcessMaterial::PostProcessMaterial(FEngine& engine,
        uint8_t const* data, int size) noexcept
        : PostProcessMaterial() {
    mEngine = &engine;
    mData = data;
    mSize = size;
}

PostProcessManager::PostProcessMaterial::PostProcessMaterial(
        PostProcessManager::PostProcessMaterial&& rhs) noexcept
        : PostProcessMaterial() {
    using namespace std;
    swap(mEngine, rhs.mEngine);
    swap(mData, rhs.mData);
    swap(mSize, rhs.mSize);
    swap(mHasMaterial, rhs.mHasMaterial);
}

PostProcessManager::PostProcessMaterial& PostProcessManager::PostProcessMaterial::operator=(
        PostProcessManager::PostProcessMaterial&& rhs) noexcept {
    using namespace std;
    swap(mEngine, rhs.mEngine);
    swap(mData, rhs.mData);
    swap(mSize, rhs.mSize);
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
        mEngine = nullptr;
        mData = nullptr;
#endif
    }
}

UTILS_NOINLINE
FMaterial* PostProcessManager::PostProcessMaterial::loadMaterial() const noexcept {
    // TODO: After all materials using this class have been converted to the post-process material
    //       domain, load both OPAQUE and TRANSPARENT variants here.
    mHasMaterial = true;
    mMaterial = upcast(Material::Builder().package(mData, mSize).build(*mEngine));
    return mMaterial;
}

FMaterial* PostProcessManager::PostProcessMaterial::assertMaterial() const noexcept {
    if (UTILS_UNLIKELY(!mHasMaterial)) {
        return loadMaterial();
    }
    return mMaterial;
}

PipelineState PostProcessManager::PostProcessMaterial::getPipelineState(uint8_t variant) const noexcept {
    FMaterial* const material = assertMaterial();
    return {
            .program = material->getProgram(variant),
            .rasterState = material->getRasterState(),
            .scissor = material->getDefaultInstance()->getScissor()
    };
}

FMaterial* PostProcessManager::PostProcessMaterial::getMaterial() const {
    assertMaterial();
    return mMaterial;
}

FMaterialInstance* PostProcessManager::PostProcessMaterial::getMaterialInstance() const {
    FMaterial* const material = assertMaterial();
    return material->getDefaultInstance();
}

// ------------------------------------------------------------------------------------------------

PostProcessManager::PostProcessManager(FEngine& engine) noexcept
        : mEngine(engine),
          mHaltonSamples{
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
                  { filament::halton(15, 2), filament::halton(15, 3) }}
{
}

UTILS_NOINLINE
void PostProcessManager::registerPostProcessMaterial(utils::StaticString name, uint8_t const* data, int size) {
    mMaterialRegistry.try_emplace(name, mEngine, data, size);
}

PostProcessManager::PostProcessMaterial& PostProcessManager::getPostProcessMaterial(utils::StaticString name) noexcept {
    assert_invariant(mMaterialRegistry.find(name) != mMaterialRegistry.end());
    return mMaterialRegistry[name];
}

#define MATERIAL(n) MATERIALS_ ## n ## _DATA, MATERIALS_ ## n ## _SIZE

struct MaterialInfo {
    utils::StaticString name;
    uint8_t const* data;
    int size;
};

static const MaterialInfo sMaterialList[] = {
        { "sao",                   MATERIAL(SAO) },
        { "mipmapDepth",           MATERIAL(MIPMAPDEPTH) },
        { "vsmMipmap",             MATERIAL(VSMMIPMAP) },
        { "bilateralBlur",         MATERIAL(BILATERALBLUR) },
        { "separableGaussianBlur", MATERIAL(SEPARABLEGAUSSIANBLUR) },
        { "bloomDownsample",       MATERIAL(BLOOMDOWNSAMPLE) },
        { "bloomUpsample",         MATERIAL(BLOOMUPSAMPLE) },
        { "flare",                 MATERIAL(FLARE) },
        { "blitLow",               MATERIAL(BLITLOW) },
        { "blitMedium",            MATERIAL(BLITMEDIUM) },
        { "blitHigh",              MATERIAL(BLITHIGH) },
        { "colorGrading",          MATERIAL(COLORGRADING) },
        { "colorGradingAsSubpass", MATERIAL(COLORGRADINGASSUBPASS) },
        { "fxaa",                  MATERIAL(FXAA) },
        { "taa",                   MATERIAL(TAA) },
        { "dofDownsample",         MATERIAL(DOFDOWNSAMPLE) },
        { "dofCoc",                MATERIAL(DOFCOC) },
        { "dofMipmap",             MATERIAL(DOFMIPMAP) },
        { "dofTiles",              MATERIAL(DOFTILES) },
        { "dofTilesSwizzle",       MATERIAL(DOFTILESSWIZZLE) },
        { "dofDilate",             MATERIAL(DOFDILATE) },
        { "dof",                   MATERIAL(DOF) },
        { "dofMedian",             MATERIAL(DOFMEDIAN) },
        { "dofCombine",            MATERIAL(DOFCOMBINE) },
};

void PostProcessManager::init() noexcept {
    auto& engine = mEngine;
    DriverApi& driver = engine.getDriverApi();

    #pragma nounroll
    for (auto const& info : sMaterialList) {
        registerPostProcessMaterial(info.name, info.data, info.size);
    }

    // UBO storage size.
    // The effective kernel size is (kMaxPositiveKernelSize - 1) * 4 + 1.
    // e.g.: 5 positive-side samples, give 4+1+4=9 samples both sides
    // taking advantage of linear filtering produces an effective kernel of 8+1+8=17 samples
    // and because it's a separable filter, the effective 2D filter kernel size is 17*17
    // The total number of samples needed over the two passes is 18.
    auto& separableGaussianBlur = getPostProcessMaterial("separableGaussianBlur");
    mSeparableGaussianBlurKernelStorageSize = separableGaussianBlur.getMaterial()->reflect("kernel")->size;

    mDummyOneTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

    mDummyOneTextureArray = driver.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

    mDummyZeroTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT);

    mStarburstTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::R8, 1, 256, 1, 1, TextureUsage::DEFAULT);

    PixelBufferDescriptor dataOne(driver.allocate(4), 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    PixelBufferDescriptor dataOneArray(driver.allocate(4), 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    PixelBufferDescriptor dataZero(driver.allocate(4), 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    PixelBufferDescriptor dataStarburst(driver.allocate(256), 256, PixelDataFormat::R, PixelDataType::UBYTE);
    *static_cast<uint32_t *>(dataOne.buffer) = 0xFFFFFFFF;
    *static_cast<uint32_t *>(dataOneArray.buffer) = 0xFFFFFFFF;
    *static_cast<uint32_t *>(dataZero.buffer) = 0;
    std::generate_n((uint8_t*)dataStarburst.buffer, 256,
            [&dist = mUniformDistribution, &gen = mEngine.getRandomEngine()]() {
        float r = 0.5 + 0.5 * dist(gen);
        return uint8_t(r * 255.0f);
    });
    driver.update2DImage(mDummyOneTexture, 0, 0, 0, 1, 1, std::move(dataOne));
    driver.update3DImage(mDummyOneTextureArray, 0, 0, 0, 0, 1, 1, 1, std::move(dataOneArray));
    driver.update2DImage(mDummyZeroTexture, 0, 0, 0, 1, 1, std::move(dataZero));
    driver.update2DImage(mStarburstTexture, 0, 0, 0, 256, 1, std::move(dataStarburst));
}

void PostProcessManager::terminate(DriverApi& driver) noexcept {
    FEngine& engine = mEngine;
    driver.destroyTexture(mDummyOneTexture);
    driver.destroyTexture(mDummyOneTextureArray);
    driver.destroyTexture(mDummyZeroTexture);
    driver.destroyTexture(mStarburstTexture);
    auto first = mMaterialRegistry.begin();
    auto last = mMaterialRegistry.end();
    while (first != last) {
        first.value().terminate(engine);
        ++first;
    }
}

UTILS_NOINLINE
void PostProcessManager::commitAndRender(FrameGraphResources::RenderPassInfo const& out,
        PostProcessMaterial const& material, uint8_t variant, DriverApi& driver) const noexcept {
    FMaterialInstance* const mi = material.getMaterialInstance();
    mi->commit(driver);
    mi->use(driver);
    driver.beginRenderPass(out.target, out.params);
    driver.draw(material.getPipelineState(variant), mEngine.getFullScreenRenderPrimitive());
    driver.endRenderPass();
}

UTILS_ALWAYS_INLINE
void PostProcessManager::commitAndRender(FrameGraphResources::RenderPassInfo const& out,
        PostProcessMaterial const& material, DriverApi& driver) const noexcept {
    commitAndRender(out, material, 0, driver);
}

// ------------------------------------------------------------------------------------------------

FrameGraphId<FrameGraphTexture> PostProcessManager::structure(FrameGraph& fg,
        const RenderPass& pass, uint32_t width, uint32_t height, float scale) noexcept {

    // structure pass -- automatically culled if not used, currently used by:
    //    - ssao
    //    - contact shadows
    // It consists of a mipmapped depth pass, tuned for SSAO
    struct StructurePassData {
        FrameGraphId<FrameGraphTexture> depth;
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

                data.depth = builder.write(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                builder.declareRenderPass("Structure Target", {
                        .attachments = { .depth = data.depth },
                        .clearFlags = TargetBufferFlags::DEPTH
                });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto out = resources.getRenderPassInfo();
                pass.execute(resources.getPassName(), out.target, out.params);
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
                    data.rt[i - 1] = builder.declareRenderPass("Structure mip target", {
                            .attachments = { .depth = out }
                    });
                }
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto in = resources.getTexture(data.depth);
                // The first mip already exists, so we process n-1 lods
                for (size_t level = 0; level < levelCount - 1; level++) {
                    auto out = resources.getRenderPassInfo(level);
                    driver.setMinMaxLevels(in, level, level);
                    auto& material = getPostProcessMaterial("mipmapDepth");
                    FMaterialInstance* const mi = material.getMaterialInstance();
                    mi->setParameter("depth", in, { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                    mi->setParameter("level", uint32_t(level));
                    commitAndRender(out, material, driver);
                }
                driver.setMinMaxLevels(in, 0, levelCount - 1);
            });

    fg.getBlackboard().put("structure", depth);
    return depth;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::screenSpaceAmbientOcclusion(
        FrameGraph& fg, RenderPass& pass,
        filament::Viewport const& svp, const CameraInfo& cameraInfo,
        View::AmbientOcclusionOptions options) noexcept {

    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    FrameGraphId<FrameGraphTexture> depth = fg.getBlackboard().get<FrameGraphTexture>("structure");
    assert_invariant(depth);

    const size_t levelCount = fg.getDescriptor(depth).levels;

    // With q the standard deviation,
    // A gaussian filter requires 6q-1 values to keep its gaussian nature
    // (see en.wikipedia.org/wiki/Gaussian_filter)
    // More intuitively, 2q is the width of the filter in pixels.
    BilateralPassConfig config = {
            // TODO: "bilateralThreshold" should be a user-settable parameter
            //       z-distance that constitute an edge for bilateral filtering
            .bilateralThreshold = 0.0625f
    };

    float sampleCount{};
    float spiralTurns{};
    switch (options.quality) {
        default:
        case View::QualityLevel::LOW:
            sampleCount = 7.0f;
            spiralTurns = 1.0f;
            break;
        case View::QualityLevel::MEDIUM:
            sampleCount = 11.0f;
            spiralTurns = 9.0f;
            break;
        case View::QualityLevel::HIGH:
            sampleCount = 16.0f;
            spiralTurns = 13.0f;
            break;
        case View::QualityLevel::ULTRA:
            sampleCount = 32.0f;
            spiralTurns = 14.0f;
            break;
    }

    switch (options.lowPassFilter) {
        default:
        case View::QualityLevel::LOW:
            // no filtering, values don't matter
            config.kernelSize = 1;
            config.standardDeviation = 1.0f;
            config.scale = 1.0f;
            break;
        case View::QualityLevel::MEDIUM:
            config.kernelSize = 11;
            config.standardDeviation = 4.0f;
            config.scale = 2.0f;
            break;
        case View::QualityLevel::HIGH:
        case View::QualityLevel::ULTRA:
            config.kernelSize = 23;
            config.standardDeviation = 8.0f;
            config.scale = 1.0f;
            break;
    }

    /*
     * Our main SSAO pass
     */

    struct SSAOPassData {
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> ssao;
    };

    const bool highQualityUpsampling =
            options.upsampling >= View::QualityLevel::HIGH && options.resolution < 1.0f;

    const bool lowPassFilterEnabled = options.lowPassFilter != View::QualityLevel::LOW;

    auto& SSAOPass = fg.addPass<SSAOPassData>("SSAO Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const& desc = builder.getDescriptor(depth);

                data.depth = builder.sample(depth);
                data.ssao = builder.createTexture("SSAO Buffer", {
                        .width = desc.width,
                        .height = desc.height,
                        .format = (lowPassFilterEnabled || highQualityUpsampling) ? TextureFormat::RGB8 : TextureFormat::R8
                });

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // Note that we have to clear the SAO buffer because blended objects will end-up
                // reading into it even though they were not written in the depth buffer.
                // The bilateral filter in the blur pass will ignore pixels at infinity.

                data.depth = builder.read(data.depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                data.ssao = builder.write(data.ssao, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass("SSAO Target", {
                        .attachments = { .color = { data.ssao }, .depth = data.depth },
                        .clearColor = { 1.0f },
                        .clearFlags = TargetBufferFlags::COLOR
                });
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto depth = resources.getTexture(data.depth);
                auto ssao = resources.getRenderPassInfo();
                auto const& desc = resources.getDescriptor(data.ssao);

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

                auto& material = getPostProcessMaterial("sao");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("depth", depth, {
                        .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
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
                mi->setParameter("depthParams",
                        cameraInfo.projection[3][2] * 0.5f);
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
                // (note: this is actually equivalent to using the camera view matrix -- before the
                // world matrix is accounted for)
                auto m = cameraInfo.view * cameraInfo.worldOrigin;
                const float3 l = normalize(
                        mat3f::getTransformForNormals(m.upperLeft())
                                * options.ssct.lightDirection);
                mi->setParameter("ssctIntensity",
                        options.ssct.enabled ? options.ssct.intensity : 0.0f);
                mi->setParameter("ssctVsLightDirection", -l);
                mi->setParameter("ssctDepthBias",
                        float2{ options.ssct.depthBias, options.ssct.depthSlopeBias });
                mi->setParameter("ssctSampleCount", uint32_t(options.ssct.sampleCount));
                mi->setParameter("ssctRayCount",
                        float2{ options.ssct.rayCount, 1.0 / options.ssct.rayCount });

                mi->commit(driver);
                mi->use(driver);

                PipelineState pipeline(material.getPipelineState());
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::L;

                driver.beginRenderPass(ssao.target, ssao.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    FrameGraphId<FrameGraphTexture> ssao = SSAOPass->ssao;

    /*
     * Final separable bilateral blur pass
     */

    if (lowPassFilterEnabled) {
        ssao = bilateralBlurPass(fg, ssao, { config.scale, 0 }, cameraInfo.zf,
                TextureFormat::RGB8,
                config);

        ssao = bilateralBlurPass(fg, ssao, { 0, config.scale }, cameraInfo.zf,
                highQualityUpsampling ? TextureFormat::RGB8 : TextureFormat::R8,
                config);
    }

    fg.getBlackboard().put("ssao", ssao);
    return ssao;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::bilateralBlurPass(
        FrameGraph& fg, FrameGraphId<FrameGraphTexture> input, math::int2 axis, float zf,
        TextureFormat format, BilateralPassConfig config) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    struct BlurPassData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> blurred;
    };

    auto& blurPass = fg.addPass<BlurPassData>("Separable Blur Pass",
            [&](FrameGraph::Builder& builder, auto& data) {

                auto const& desc = builder.getDescriptor(input);

                data.input = builder.sample(input);

                data.blurred = builder.createTexture("Blurred output", {
                        .width = desc.width, .height = desc.height, .format = format });

                FrameGraphId<FrameGraphTexture> depth = fg.getBlackboard().get<FrameGraphTexture>("structure");
                assert_invariant(depth);
                depth = builder.read(depth, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // We need to clear the buffers because we are skipping pixels at infinity (skybox)
                data.blurred = builder.write(data.blurred, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass("Blurred target", {
                        .attachments = { .color = { data.blurred }, . depth = depth },
                        .clearColor = { 1.0f },
                        .clearFlags = TargetBufferFlags::COLOR
                });
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto ssao = resources.getTexture(data.input);
                auto blurred = resources.getRenderPassInfo();
                auto const& desc = resources.getDescriptor(data.blurred);

                // unnormalized gaussian half-kernel of a given standard deviation
                // returns number of samples stored in array (max 32)
                auto gaussianKernel =
                        [](float* outKernel, size_t gaussianWidth, float stdDev) -> uint32_t {
                    const size_t gaussianSampleCount = std::min(size_t(32), (gaussianWidth + 1u) / 2u);
                    for (size_t i = 0; i < gaussianSampleCount; i++) {
                        float x = i;
                        float g = std::exp(-(x * x) / (2.0f * stdDev * stdDev));
                        outKernel[i] = g;
                    }
                    return uint32_t(gaussianSampleCount);
                };

                float kGaussianSamples[32];
                uint32_t kGaussianCount = gaussianKernel(kGaussianSamples,
                        config.kernelSize, config.standardDeviation);

                auto& material = getPostProcessMaterial("bilateralBlur");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("ssao", ssao, { /* only reads level 0 */ });
                mi->setParameter("axis", axis / float2{desc.width, desc.height});
                mi->setParameter("kernel", kGaussianSamples, kGaussianCount);
                mi->setParameter("sampleCount", kGaussianCount);
                mi->setParameter("farPlaneOverEdgeDistance", -zf / config.bilateralThreshold);

                mi->commit(driver);
                mi->use(driver);

                PipelineState pipeline(material.getPipelineState());
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::L;

                driver.beginRenderPass(blurred.target, blurred.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return blurPass->blurred;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::generateGaussianMipmap(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, size_t roughnessLodCount,
        bool reinhard, size_t kernelWidth, float sigmaRatio) noexcept {
    for (size_t i = 1; i < roughnessLodCount; i++) {
        input = gaussianBlurPass(fg, input, i - 1, input, i, reinhard, kernelWidth, sigmaRatio);
        reinhard = false; // only do the reinhard filtering on the first level
    }
    return input;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::gaussianBlurPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, uint8_t srcLevel,
        FrameGraphId<FrameGraphTexture> output, uint8_t dstLevel,
        bool reinhard, size_t kernelWidth, float sigmaRatio) noexcept {

    const float sigma = (kernelWidth + 1.0f) / sigmaRatio;

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    auto computeGaussianCoefficients = [kernelWidth, sigma](float2* kernel, size_t size) -> size_t {
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
            float x0 = i * 2 - 1;
            float x1 = i * 2;
            float k0 = std::exp(-alpha * x0 * x0);
            float k1 = std::exp(-alpha * x1 * x1);
            float k = k0 + k1;
            float o = k0 / k;
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

    const size_t kernelStorageSize = mSeparableGaussianBlurKernelStorageSize;
    fg.addPass<BlurPassData>("Gaussian Blur Passes",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto desc = builder.getDescriptor(input);

                if (!output) {
                    output = builder.createTexture("Blurred texture", desc);
                }

                data.in = builder.sample(input);

                // width of the destination level (b/c we're blurring horizontally)
                desc.width = FTexture::valueForLevel(dstLevel, desc.width);
                // height of the source level (b/c it's not blurred in this pass)
                desc.height = FTexture::valueForLevel(srcLevel, desc.height);
                // only one level
                desc.levels = 1;

                data.temp = builder.createTexture("Horizontal temporary buffer", desc);
                data.temp = builder.sample(data.temp);
                data.temp = builder.declareRenderPass(data.temp);

                data.out = builder.createSubresource(output, "Blurred texture mip",{ .level = dstLevel });
                data.out = builder.declareRenderPass(data.out);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto const& separableGaussianBlur = getPostProcessMaterial("separableGaussianBlur");
                FMaterialInstance* const mi = separableGaussianBlur.getMaterialInstance();

                float2 kernel[64];
                size_t m = computeGaussianCoefficients(kernel,
                        std::min(sizeof(kernel) / sizeof(*kernel), kernelStorageSize));

                // horizontal pass
                auto hwTempRT = resources.getRenderPassInfo(0);
                auto hwOutRT = resources.getRenderPassInfo(1);
                auto hwTemp = resources.getTexture(data.temp);
                auto hwIn = resources.getTexture(data.in);
                auto const& inDesc = resources.getDescriptor(data.in);
                auto const& outDesc = resources.getDescriptor(data.out);
                auto const& tempDesc = resources.getDescriptor(data.temp);

                mi->setParameter("source", hwIn, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });
                mi->setParameter("level", (float)srcLevel);
                mi->setParameter("reinhard", reinhard ? uint32_t(1) : uint32_t(0));
                mi->setParameter("resolution", float4{
                        tempDesc.width, tempDesc.height,
                        1.0f / tempDesc.width, 1.0f / tempDesc.height });
                mi->setParameter("axis",
                        float2{ 1.0f / FTexture::valueForLevel(srcLevel, inDesc.width), 0 });
                mi->setParameter("count", (int32_t)m);
                mi->setParameter("kernel", kernel, m);

                // The framegraph only computes discard flags at FrameGraphPass boundaries
                hwTempRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                commitAndRender(hwTempRT, separableGaussianBlur, driver);

                // vertical pass
                auto width = outDesc.width;
                auto height = outDesc.height;
                assert_invariant(width == hwOutRT.params.viewport.width);
                assert_invariant(height == hwOutRT.params.viewport.height);

                mi->setParameter("source", hwTemp, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR /* level is always 0 */
                });
                mi->setParameter("level", 0.0f);
                mi->setParameter("resolution",
                        float4{ width, height, 1.0f / width, 1.0f / height });
                mi->setParameter("axis", float2{ 0, 1.0f / tempDesc.height });
                mi->commit(driver);
                // we don't need to call use() here, since it's the same material

                driver.beginRenderPass(hwOutRT.target, hwOutRT.params);
                driver.draw(separableGaussianBlur.getPipelineState(), fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::dof(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, const View::DepthOfFieldOptions& dofOptions,
        bool translucent, const CameraInfo& cameraInfo, float2 scale) noexcept {

    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

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
     *  (note: our z-buffer is encoded as reversed-z, so we use 1-z below)
     *
     *          screen-space -> clip-space -> view-space -> distance (x-1)
     *
     *   v_s = { x, y, 1 - z, 1 }                 // screen space (reversed-z)
     *   v_c = 2 * v_s - 1                        // clip space
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
     *          -2C         C - A
     *    1/d = --- . z  + -------
     *           B            B
     *
     * note: Here the result doesn't depend on {x, y}. This wouldn't be the case with a
     *       tilt-shift lens.
     *
     * Mathematica code:
     *      p = {{a, 0, b, 0}, {0, c, d, 0}, {0, 0, m22, m32}, {0, 0, m23, 0}};
     *      v = {x, y, (1 - z)*2 - 1, 1};
     *      f = Inverse[p].v;
     *      Simplify[f[[4]]/f[[3]]]
     *
     * Plugging this back into the expression of: coc(z) = Kc . Ks . (1 - S / d)
     * We get that:  coc(z) = C0 * z + C1
     * With: C0 = - Kc * Ks * S * 2 * C / B
     *       C1 =   Kc * Ks * (1 - S * (C - A) / B)
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
              -K * focusDistance * 2.0 * p[2][3] / p[3][2],
               K * (1.0 - focusDistance * (p[2][3] - p[2][2]) / p[3][2])
    };

    Blackboard& blackboard = fg.getBlackboard();
    auto depth = blackboard.get<FrameGraphTexture>("depth");
    assert_invariant(depth);

    /*
     * dofResolution is used (at compile time for now) to chose between full- or quarter-resolution
     * for the DoF calculations. Set to [1] for full resolution or [2] for quarter-resolution.
     */
    const uint32_t dofResolution = dofOptions.nativeResolution ? 1u : 2u;

    constexpr const uint32_t maxMipLevels = 4u; // mip levels at full-resolution
    constexpr const uint32_t maxMipLevelsMask = (1u << maxMipLevels) - 1u;
    auto const& colorDesc = fg.getDescriptor(input);
    const uint32_t width  = ((colorDesc.width  + maxMipLevelsMask) & ~maxMipLevelsMask) / dofResolution;
    const uint32_t height = ((colorDesc.height + maxMipLevelsMask) & ~maxMipLevelsMask) / dofResolution;
    const uint8_t maxLevelCount = FTexture::maxLevelCount(width, height);
    uint8_t mipmapCount = min(maxLevelCount, uint8_t(maxMipLevels));

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

                data.outColor = builder.createTexture("dof downsample output", {
                        .width  = width, .height = height, .levels = mipmapCount, .format = format
                });
                data.outCoc = builder.createTexture("dof CoC output", {
                        .width  = width, .height = height, .levels = mipmapCount,
                        .format = TextureFormat::R16F,
                        .swizzle = {
                                // the next stage expects min/max CoC in the red/green channel
                                .r = backend::TextureSwizzle::CHANNEL_0,
                                .g = backend::TextureSwizzle::CHANNEL_0 },
                });
                data.outColor = builder.write(data.outColor, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.outCoc   = builder.write(data.outCoc,   FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass("DoF Target", { .attachments = {
                                .color = { data.outColor, data.outCoc }
                        }
                });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto const& out = resources.getRenderPassInfo();
                auto color = resources.getTexture(data.color);
                auto depth = resources.getTexture(data.depth);
                auto const& material = (dofResolution == 1) ?
                        getPostProcessMaterial("dofCoc") :
                        getPostProcessMaterial("dofDownsample");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("color", color, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("depth", depth, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("cocParams", cocParams);
                mi->setParameter("cocClamp", float2{
                    -(dofOptions.maxForegroundCOC ? dofOptions.maxForegroundCOC : DOF_DEFAULT_MAX_COC),
                      dofOptions.maxBackgroundCOC ? dofOptions.maxBackgroundCOC : DOF_DEFAULT_MAX_COC});
                mi->setParameter("uvscale", float4{ width, height,
                        1.0f / colorDesc.width, 1.0f / colorDesc.height });
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

                    data.rp[i] = builder.declareRenderPass("DoF Target", { .attachments = {
                                .color = { inOutColor, inOutCoc  }
                        }
                    });
                }
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto inOutColor = resources.getTexture(data.inOutColor);
                auto inOutCoc   = resources.getTexture(data.inOutCoc);

                auto const& material = getPostProcessMaterial("dofMipmap");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("color", inOutColor, { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("coc",   inOutCoc,   { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->use(driver);

                const PipelineState pipeline(material.getPipelineState(variant));

                for (size_t level = 0 ; level < mipmapCount - 1u ; level++) {
                    auto const& out = resources.getRenderPassInfo(data.rp[level]);
                    mi->setParameter("mip", uint32_t(level));
                    mi->setParameter("weightScale", 0.5f / float(1u<<level));   // FIXME: halfres?
                    mi->commit(driver);
                    driver.beginRenderPass(out.target, out.params);
                    driver.draw(pipeline, fullScreenRenderPrimitive);
                    driver.endRenderPass();
                }
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
    // round the width/height to 16 (tile size), divide by scale (1 or 2) for halfres
    const uint32_t tileBufferWidth  = ((colorDesc.width  + (tileSize - 1u)) & ~(tileSize - 1u)) / dofResolution;
    const uint32_t tileBufferHeight = ((colorDesc.height + (tileSize - 1u)) & ~(tileSize - 1u)) / dofResolution;
    const size_t tileReductionCount = std::log2(tileSize / dofResolution);

    struct PostProcessDofTiling1 {
        FrameGraphId<FrameGraphTexture> inCocMinMax;
        FrameGraphId<FrameGraphTexture> outTilesCocMinMax;
    };

    const bool textureSwizzleSupported = Texture::isTextureSwizzleSupported(mEngine);
    for (size_t i = 0; i < tileReductionCount; i++) {
        auto& ppDoFTiling = fg.addPass<PostProcessDofTiling1>("DoF Tiling",
                [&](FrameGraph::Builder& builder, auto& data) {
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
                    auto const& outputDesc = resources.getDescriptor(data.outTilesCocMinMax);
                    auto const& out = resources.getRenderPassInfo();
                    auto inCocMinMax = resources.getTexture(data.inCocMinMax);
                    auto const& material = (!textureSwizzleSupported && (i == 0)) ?
                            getPostProcessMaterial("dofTilesSwizzle") :
                            getPostProcessMaterial("dofTiles");
                    FMaterialInstance* const mi = material.getMaterialInstance();
                    mi->setParameter("cocMinMax", inCocMinMax, { .filterMin = SamplerMinFilter::NEAREST });
                    mi->setParameter("uvscale", float4{
                        outputDesc.width, outputDesc.height,
                        1.0f / inputDesc.width, 1.0f / inputDesc.height });
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
                    FMaterialInstance* const mi = material.getMaterialInstance();
                    mi->setParameter("tiles", inTilesCocMinMax, { .filterMin = SamplerMinFilter::NEAREST });
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

                // The DoF buffer (output) doesn't need to be a multiple of 8 because it's not
                // mipmapped. We just need to adjust the uv properly.
                data.outColor = builder.createTexture("dof color output", {
                        .width  = (colorDesc.width  + (dofResolution / 2u)) / dofResolution,
                        .height = (colorDesc.height + (dofResolution / 2u)) / dofResolution,
                        .format = fg.getDescriptor(data.color).format
                });
                data.outAlpha = builder.createTexture("dof alpha output", {
                        .width  = builder.getDescriptor(data.outColor).width,
                        .height = builder.getDescriptor(data.outColor).height,
                        .format = TextureFormat::R8
                });
                data.outColor  = builder.write(data.outColor, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.outAlpha  = builder.write(data.outAlpha, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass("DoF Target", {
                        .attachments = { .color = { data.outColor, data.outAlpha }}
                });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto const& out = resources.getRenderPassInfo();

                auto color          = resources.getTexture(data.color);
                auto coc            = resources.getTexture(data.coc);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& inputDesc = resources.getDescriptor(data.coc);
                auto const& outputDesc = resources.getDescriptor(data.outColor);
                auto const& tilesDesc = resources.getDescriptor(data.tilesCocMinMax);

                auto const& material = getPostProcessMaterial("dof");
                FMaterialInstance* const mi = material.getMaterialInstance();
                // it's not safe to use bilinear filtering in the general case (causes artifacts around edges)
                mi->setParameter("color", color,
                        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("colorLinear", color,
                        { .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST });
                mi->setParameter("coc", coc,
                        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("tiles", tilesCocMinMax,
                        { .filterMin = SamplerMinFilter::NEAREST });

                // The bokeh height is always correct regardless of the dynamic resolution scaling.
                // (because the CoC is calculated w.r.t. the height), so we only need to adjust
                // the width.
                const float aspectRatio = scale.x / scale.y;
                mi->setParameter("cocToTexelScale", float2{
                        aspectRatio / (inputDesc.width  * dofResolution),
                                1.0 / (inputDesc.height * dofResolution)
                });

                mi->setParameter("cocToPixelScale", (1.0f / dofResolution));
                mi->setParameter("uvscale", float4{
                    outputDesc.width  / float(inputDesc.width),
                    outputDesc.height / float(inputDesc.height),
                    outputDesc.width  / float(tileSize / dofResolution * tilesDesc.width),
                    outputDesc.height / float(tileSize / dofResolution * tilesDesc.height)
                });
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
                builder.declareRenderPass("DoF Target", {
                        .attachments = { .color = { data.outColor, data.outAlpha }}
                });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto const& out = resources.getRenderPassInfo();

                auto inColor        = resources.getTexture(data.inColor);
                auto inAlpha        = resources.getTexture(data.inAlpha);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& outputDesc = resources.getDescriptor(data.outColor);
                auto const& tilesDesc = resources.getDescriptor(data.tilesCocMinMax);

                auto const& material = getPostProcessMaterial("dofMedian");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("dof",   inColor,        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("alpha", inAlpha,        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("tiles", tilesCocMinMax, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("uvscale", float2{
                        outputDesc.width  / float(tileSize / dofResolution * tilesDesc.width),
                        outputDesc.height / float(tileSize / dofResolution * tilesDesc.height)
                });
                commitAndRender(out, material, driver);
            });


    /*
     * DoF recombine
     */

    auto outColor = ppDoFMedian->outColor;
    auto outAlpha = ppDoFMedian->outAlpha;
    if (dofOptions.filter == View::DepthOfFieldOptions::Filter::NONE) {
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
                auto const& dofDesc = resources.getDescriptor(data.dof);
                auto const& tilesDesc = resources.getDescriptor(data.tilesCocMinMax);
                auto const& out = resources.getRenderPassInfo();

                auto color      = resources.getTexture(data.color);
                auto dof        = resources.getTexture(data.dof);
                auto alpha      = resources.getTexture(data.alpha);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& material = getPostProcessMaterial("dofCombine");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("color", color, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("dof",   dof,   { .filterMag = SamplerMagFilter::NEAREST });
                mi->setParameter("alpha", alpha, { .filterMag = SamplerMagFilter::NEAREST });
                mi->setParameter("tiles", tilesCocMinMax, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("uvscale", float4{
                    colorDesc.width  / (dofDesc.width    * float(dofResolution)),
                    colorDesc.height / (dofDesc.height   * float(dofResolution)),
                    colorDesc.width  / (tilesDesc.width  * float(tileSize)),
                    colorDesc.height / (tilesDesc.height * float(tileSize))
                });
                commitAndRender(out, material, driver);
            });

    return ppDoFCombine->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::bloom(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, TextureFormat outFormat,
        View::BloomOptions& bloomOptions, float2 scale) noexcept {

    FrameGraphId<FrameGraphTexture> bloom = bloomPass(fg, input,
            outFormat, bloomOptions, scale);

    fg.getBlackboard().put("bloom", bloom);

    return bloom;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::bloomPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, TextureFormat outFormat,
        View::BloomOptions& bloomOptions, float2 scale) noexcept {
    // Chrome does not support feedback loops in WebGL 2.0. See also:
    // https://bugs.chromium.org/p/chromium/issues/detail?id=1066201
#if defined(__EMSCRIPTEN__)
    constexpr bool isWebGL = true;
#else
    constexpr bool isWebGL = false;
#endif

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    // Figure out a good size for the bloom buffer.
    auto const& desc = fg.getDescriptor(input);

    // width and height after dynamic resolution upscaling
    const float aspect = (desc.width * scale.y) / (desc.height * scale.x);

    // compute the desired bloom buffer size
    float bloomHeight = bloomOptions.resolution;
    float bloomWidth  = bloomHeight * aspect;

    // Anamorphic bloom by always scaling down one of the dimension -- we do this (as opposed
    // to scaling up) so that the amount of blooming doesn't decrease. However, the resolution
    // decreases, meaning that the user might need to adjust the BloomOptions::resolution and
    // BloomOptions::levels.
    if (bloomOptions.anamorphism >= 1.0f) {
        bloomWidth *= 1.0 / bloomOptions.anamorphism;
    } else {
        bloomHeight *= bloomOptions.anamorphism;
    }

    // convert back to integer width/height
    const uint32_t width  = std::max(1.0f, std::floor(bloomWidth));
    const uint32_t height = std::max(1.0f, std::floor(bloomHeight));

    // we might need to adjust the max # of levels
    const uint32_t major = std::max(bloomWidth,  bloomHeight);
    const uint8_t maxLevels = FTexture::maxLevelCount(major);
    bloomOptions.levels = std::min(bloomOptions.levels, maxLevels);
    bloomOptions.levels = std::min(bloomOptions.levels, kMaxBloomLevels);

    if (2 * width < desc.width || 2 * height < desc.height) {
        // if we're scaling down by more than 2x, prescale the image with a blit to improve
        // performance. This is important on mobile/tilers.
        input = opaqueBlit(fg, input, {
                .width = desc.width / 2,
                .height = desc.height / 2,
                .format = outFormat
        });
    }

    if constexpr(!isWebGL) {

        struct BloomPassData {
            FrameGraphId<FrameGraphTexture> in;
            FrameGraphId<FrameGraphTexture> out;
        };

        // downsample phase
        auto& bloomDownsamplePass = fg.addPass<BloomPassData>("Bloom Downsample",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.in = builder.sample(input);
                    data.out = builder.createTexture("Bloom Texture", {
                            .width = width,
                            .height = height,
                            .levels = bloomOptions.levels,
                            .format = outFormat
                    });

                    data.out = builder.sample(data.out);
                    for (size_t i = 0; i < bloomOptions.levels; i++) {
                        auto out = builder.createSubresource(data.out, "Bloom Texture mip",
                                { .level = uint8_t(i) });
                        builder.declareRenderPass(out);
                    }
                },
                [=](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {

                    auto const& material = getPostProcessMaterial("bloomDownsample");
                    FMaterialInstance* mi = material.getMaterialInstance();

                    const PipelineState pipeline(material.getPipelineState());

                    auto hwIn = resources.getTexture(data.in);
                    auto hwOut = resources.getTexture(data.out);
                    auto const& outDesc = resources.getDescriptor(data.out);

                    mi->use(driver);
                    mi->setParameter("source", hwIn, {
                            .filterMag = SamplerMagFilter::LINEAR,
                            .filterMin = SamplerMinFilter::LINEAR /* level is always 0 */
                    });
                    mi->setParameter("level", 0.0f);
                    mi->setParameter("threshold", bloomOptions.threshold ? 1.0f : 0.0f);
                    mi->setParameter("invHighlight",
                            std::isinf(bloomOptions.highlight) ? 0.0f : 1.0f
                                                                        / bloomOptions.highlight);

                    for (size_t i = 0; i < bloomOptions.levels; i++) {
                        auto hwOutRT = resources.getRenderPassInfo(i);

                        auto w = FTexture::valueForLevel(i, outDesc.width);
                        auto h = FTexture::valueForLevel(i, outDesc.height);
                        mi->setParameter("resolution", float4{ w, h, 1.0f / w, 1.0f / h });
                        mi->commit(driver);

                        hwOutRT.params.flags.discardStart = TargetBufferFlags::COLOR;
                        hwOutRT.params.flags.discardEnd = TargetBufferFlags::NONE;
                        driver.beginRenderPass(hwOutRT.target, hwOutRT.params);
                        driver.draw(pipeline, fullScreenRenderPrimitive);
                        driver.endRenderPass();

                        // prepare the next level
                        mi->setParameter("source", hwOut, {
                                .filterMag = SamplerMagFilter::LINEAR,
                                .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                        });
                        mi->setParameter("level", float(i));
                        driver.setMinMaxLevels(hwOut, i,
                                i); // safe because we're using LINEAR_MIPMAP_NEAREST
                    }
                    driver.setMinMaxLevels(hwOut, 0, bloomOptions.levels - 1);
                });

        input = bloomDownsamplePass->out;

        // flare pass
        auto& flarePass = fg.addPass<BloomPassData>("Flare",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.in = builder.sample(input);
                    data.out = builder.createTexture("Flare Texture", {
                            .width  = width  / 2,
                            .height = height / 2,
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
                    FMaterialInstance* mi = material.getMaterialInstance();

                    mi->setParameter("color", in, {
                            .filterMag = SamplerMagFilter::LINEAR,
                            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                    });

                    mi->setParameter("level", 1.0f);    // adjust with resolution
                    mi->setParameter("aspectRatio",
                            float2{ aspectRatio, 1.0f / aspectRatio });
                    mi->setParameter("threshold",
                            float2{ bloomOptions.ghostThreshold, bloomOptions.haloThreshold });
                    mi->setParameter("chromaticAberration",
                            bloomOptions.chromaticAberration);
                    mi->setParameter("ghostCount", (float)bloomOptions.ghostCount);
                    mi->setParameter("ghostSpacing", bloomOptions.ghostSpacing);
                    mi->setParameter("haloRadius", bloomOptions.haloRadius);
                    mi->setParameter("haloThickness", bloomOptions.haloThickness);

                    commitAndRender(out, material, driver);
                });

        auto flare = gaussianBlurPass(fg, flarePass->out, 0,
                {}, 0, false, 9);

        fg.getBlackboard().put("flare", flare);

        // upsample phase
        auto& bloomUpsamplePass = fg.addPass<BloomPassData>("Bloom Upsample",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.in = builder.sample(input);
                    data.out = input;
                    for (size_t i = 0; i < bloomOptions.levels; i++) {
                        auto out = builder.createSubresource(data.out, "Bloom Texture mip",
                                { .level = uint8_t(i) });
                        builder.declareRenderPass(out);
                    }
                },
                [=](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {

                    auto hwIn = resources.getTexture(data.in);
                    auto const& outDesc = resources.getDescriptor(data.out);

                    auto const& material = getPostProcessMaterial("bloomUpsample");
                    FMaterialInstance* mi = material.getMaterialInstance();
                    PipelineState pipeline(material.getPipelineState());
                    pipeline.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionDstRGB = BlendFunction::ONE;

                    mi->use(driver);

                    for (size_t i = bloomOptions.levels - 1; i >= 1; i--) {
                        auto hwDstRT = resources.getRenderPassInfo(i - 1);
                        hwDstRT.params
                               .flags
                               .discardStart = TargetBufferFlags::NONE; // because we'll blend
                        hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                        auto w = FTexture::valueForLevel(i - 1, outDesc.width);
                        auto h = FTexture::valueForLevel(i - 1, outDesc.height);
                        mi->setParameter("resolution", float4{ w, h, 1.0f / w, 1.0f / h });
                        mi->setParameter("source", hwIn, {
                                .filterMag = SamplerMagFilter::LINEAR,
                                .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                        });
                        mi->setParameter("level", float(i));
                        driver.setMinMaxLevels(hwIn, i, i);
                        mi->commit(driver);

                        driver.beginRenderPass(hwDstRT.target, hwDstRT.params);
                        driver.draw(pipeline, fullScreenRenderPrimitive);
                        driver.endRenderPass();
                    }

                    driver.setMinMaxLevels(hwIn, 0, bloomOptions.levels - 1);
                });

        return bloomUpsamplePass->out;

    } else { // !isWebGL

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
                            .levels = bloomOptions.levels,
                            .format = outFormat
                    });
                    data.out = builder.sample(data.out);

                    data.stage = builder.createTexture("Bloom Stage Texture", {
                            .width = width,
                            .height = height,
                            .levels = bloomOptions.levels,
                            .format = outFormat
                    });
                    data.stage = builder.sample(data.stage);

                    for (size_t i = 0; i < bloomOptions.levels; i++) {
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

                    auto const& material = getPostProcessMaterial("bloomDownsample");
                    FMaterialInstance* mi = material.getMaterialInstance();

                    const PipelineState pipeline(material.getPipelineState());

                    auto hwIn = resources.getTexture(data.in);
                    auto hwOut = resources.getTexture(data.out);
                    auto hwStage = resources.getTexture(data.stage);
                    auto const& outDesc = resources.getDescriptor(data.out);

                    mi->use(driver);
                    mi->setParameter("source", hwIn, {
                            .filterMag = SamplerMagFilter::LINEAR,
                            .filterMin = SamplerMinFilter::LINEAR /* level is always 0 */
                    });
                    mi->setParameter("level", 0.0f);
                    mi->setParameter("threshold", bloomOptions.threshold ? 1.0f : 0.0f);
                    mi->setParameter("invHighlight",
                            std::isinf(bloomOptions.highlight) ? 0.0f : 1.0f
                                                                        / bloomOptions.highlight);

                    for (size_t i = 0; i < bloomOptions.levels; i++) {
                        const bool parity = (i % 2) == 0;
                        auto hwDstRT = resources.getRenderPassInfo(
                                parity ? data.outRT[i] : data.stageRT[i]);

                        auto w = FTexture::valueForLevel(i, outDesc.width);
                        auto h = FTexture::valueForLevel(i, outDesc.height);
                        mi->setParameter("resolution", float4{ w, h, 1.0f / w, 1.0f / h });
                        mi->commit(driver);

                        hwDstRT.params.flags.discardStart = TargetBufferFlags::COLOR;
                        hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;
                        driver.beginRenderPass(hwDstRT.target, hwDstRT.params);
                        driver.draw(pipeline, fullScreenRenderPrimitive);
                        driver.endRenderPass();

                        // prepare the next level
                        mi->setParameter("source", parity ? hwOut : hwStage, {
                                .filterMag = SamplerMagFilter::LINEAR,
                                .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                        });
                        mi->setParameter("level", float(i));
                    }
                });

        FrameGraphId<FrameGraphTexture> output = bloomDownsamplePass->out;
        FrameGraphId<FrameGraphTexture> stage = bloomDownsamplePass->stage;

        // upsample phase
        auto& bloomUpsamplePass = fg.addPass<BloomPassData>("Bloom Upsample",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.out = builder.sample(output);
                    data.stage = builder.sample(stage);
                    for (size_t i = 0; i < bloomOptions.levels; i++) {
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
                    FMaterialInstance* mi = material.getMaterialInstance();
                    PipelineState pipeline(material.getPipelineState());
                    pipeline.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionDstRGB = BlendFunction::ONE;

                    mi->use(driver);

                    for (size_t j = bloomOptions.levels, i = j - 1; i >= 1; i--, j++) {
                        const bool parity = (j % 2) == 0;

                        auto hwDstRT = resources.getRenderPassInfo(
                                parity ? data.outRT[i - 1] : data.stageRT[i - 1]);
                        hwDstRT.params
                               .flags
                               .discardStart = TargetBufferFlags::NONE; // because we'll blend
                        hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                        auto w = FTexture::valueForLevel(i - 1, outDesc.width);
                        auto h = FTexture::valueForLevel(i - 1, outDesc.height);
                        mi->setParameter("resolution", float4{ w, h, 1.0f / w, 1.0f / h });
                        mi->setParameter("source", parity ? hwStage : hwOut, {
                                .filterMag = SamplerMagFilter::LINEAR,
                                .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                        });
                        mi->setParameter("level", float(i));
                        mi->commit(driver);

                        driver.beginRenderPass(hwDstRT.target, hwDstRT.params);
                        driver.draw(pipeline, fullScreenRenderPrimitive);
                        driver.endRenderPass();
                    }

                    // Every other level is missing from the out texture, so we need to do
                    // blits to complete the chain.
                    const SamplerMagFilter filter = SamplerMagFilter::NEAREST;
                    for (size_t i = 1; i < bloomOptions.levels; i += 2) {
                        auto in = resources.getRenderPassInfo(data.stageRT[i]);
                        auto out = resources.getRenderPassInfo(data.outRT[i]);
                        driver.blit(TargetBufferFlags::COLOR, out.target, out.params.viewport,
                                in.target, in.params.viewport, filter);
                    }
                });
        return bloomUpsamplePass->out;
    }
}

static float4 getVignetteParameters(View::VignetteOptions options, uint32_t width, uint32_t height) {
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
        View::VignetteOptions vignetteOptions, uint32_t width, uint32_t height) noexcept {

    float4 vignetteParameters = getVignetteParameters(vignetteOptions, width, height);

    auto const& material = getPostProcessMaterial("colorGradingAsSubpass");
    FMaterialInstance* mi = material.getMaterialInstance();
    mi->setParameter("lut", colorGrading->getHwHandle(), {
            .filterMag = SamplerMagFilter::LINEAR,
            .filterMin = SamplerMinFilter::LINEAR
    });

    const float temporalNoise = mUniformDistribution(mEngine.getRandomEngine());

    mi->setParameter("vignette", vignetteParameters);
    mi->setParameter("vignetteColor", vignetteOptions.color);
    mi->setParameter("dithering", colorGradingConfig.dithering);
    mi->setParameter("fxaa", colorGradingConfig.fxaa);
    mi->setParameter("temporalNoise", temporalNoise);
    mi->commit(driver);
}

void PostProcessManager::colorGradingSubpass(DriverApi& driver,
        ColorGradingConfig const& colorGradingConfig) noexcept {
    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    auto const& material = getPostProcessMaterial("colorGradingAsSubpass");
    // the UBO has been set and committed in colorGradingPrepareSubpass()
    FMaterialInstance* mi = material.getMaterialInstance();
    mi->use(driver);
    const uint8_t variant = uint8_t(colorGradingConfig.translucent ?
            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

    driver.nextSubpass();
    driver.draw(material.getPipelineState(variant), fullScreenRenderPrimitive);
}

FrameGraphId<FrameGraphTexture> PostProcessManager::colorGrading(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, float2 scale,
        const FColorGrading* colorGrading, ColorGradingConfig const& colorGradingConfig,
        View::BloomOptions bloomOptions, View::VignetteOptions vignetteOptions) noexcept
{
    Blackboard& blackboard = fg.getBlackboard();

    FrameGraphId<FrameGraphTexture> bloomDirt;
    FrameGraphId<FrameGraphTexture> starburst;
    FrameGraphId<FrameGraphTexture> bloom = blackboard.get<FrameGraphTexture>("bloom");
    FrameGraphId<FrameGraphTexture> flare = blackboard.get<FrameGraphTexture>("flare");

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
                auto const& inputDesc = fg.getDescriptor(input);
                data.input = builder.sample(input);
                data.output = builder.createTexture("colorGrading output", {
                        .width = inputDesc.width,
                        .height = inputDesc.height,
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
                FMaterialInstance* mi = material.getMaterialInstance();
                mi->setParameter("lut", colorGrading->getHwHandle(), {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                mi->setParameter("colorBuffer", colorTexture, { /* shader uses texelFetch */ });
                mi->setParameter("bloomBuffer", bloomTexture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR /* always read base level in shader */
                });
                mi->setParameter("flareBuffer", flareTexture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                mi->setParameter("dirtBuffer", dirtTexture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                mi->setParameter("starburstBuffer", starburstTexture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR,
                        .wrapS = SamplerWrapMode::REPEAT,
                        .wrapT = SamplerWrapMode::REPEAT
                });

                // Bloom params
                float4 bloomParameters{
                    bloomStrength / float(bloomOptions.levels),
                    1.0f,
                    (bloomOptions.enabled && bloomOptions.dirt) ? bloomOptions.dirtStrength : 0.0f,
                    bloomOptions.lensFlare ? bloomStrength : 0.0f
                };
                if (bloomOptions.blendMode == View::BloomOptions::BlendMode::INTERPOLATE) {
                    bloomParameters.y = 1.0f - bloomParameters.x;
                }

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

                const uint8_t variant = uint8_t(colorGradingConfig.translucent ?
                            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                commitAndRender(out, material, variant, driver);
            }
    );

    return ppColorGrading->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::fxaa(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        TextureFormat outFormat, bool translucent) noexcept {

    struct PostProcessFXAA {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto& ppFXAA = fg.addPass<PostProcessFXAA>("fxaa",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const& inputDesc = fg.getDescriptor(input);
                data.input = builder.sample(input);
                data.output = builder.createTexture("fxaa output", {
                        .width = inputDesc.width,
                        .height = inputDesc.height,
                        .format = outFormat
                });
                data.output = builder.declareRenderPass(data.output);
            },
            [=](FrameGraphResources const& resources,
                auto const& data, DriverApi& driver) {
                auto const& texture = resources.getTexture(data.input);
                auto const& out = resources.getRenderPassInfo();

                auto const& material = getPostProcessMaterial("fxaa");
                FMaterialInstance* mi = material.getMaterialInstance();
                mi->setParameter("colorBuffer", texture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });

                const uint8_t variant = uint8_t(translucent ?
                    PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                commitAndRender(out, material, variant, driver);
            });

    return ppFXAA->output;
}

void PostProcessManager::prepareTaa(FrameHistory& frameHistory,
        CameraInfo const& cameraInfo,
        View::TemporalAntiAliasingOptions const& taaOptions) const noexcept {
    auto const& previous = frameHistory[0];
    auto& current = frameHistory.getCurrent();
    // get sample position within a pixel [-0.5, 0.5]
    const float2 jitter = halton(previous.frameId) - 0.5f;
    // compute the world-space to clip-space matrix for this frame
    current.projection = cameraInfo.projection * (cameraInfo.view * cameraInfo.worldOrigin);
    // save this frame's sample position
    current.jitter = jitter;
    // update frame id
    current.frameId = previous.frameId + 1;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::taa(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, FrameHistory& frameHistory,
        View::TemporalAntiAliasingOptions taaOptions,
        ColorGradingConfig colorGradingConfig) noexcept {

    FrameHistoryEntry const& entry = frameHistory[0];
    FrameGraphId<FrameGraphTexture> colorHistory;
    mat4f const* historyProjection = nullptr;
    if (UTILS_UNLIKELY(!entry.color.handle)) {
        // if we don't have a history yet, just use the current color buffer as history
        colorHistory = input;
        historyProjection = &frameHistory.getCurrent().projection;
    } else {
        colorHistory = fg.import("TAA history", entry.colorDesc,
                FrameGraphTexture::Usage::SAMPLEABLE, entry.color);
        historyProjection = &entry.projection;
    }

    Blackboard& blackboard = fg.getBlackboard();
    auto depth = blackboard.get<FrameGraphTexture>("depth");
    assert_invariant(depth);

    struct TAAData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> history;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> tonemappedOutput;
    };
    auto& taa = fg.addPass<TAAData>("TAA",
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
                builder.declareRenderPass("TAA target", {
                        .attachments = { .color = { data.output, data.tonemappedOutput }}
                });
            },
            [=, &frameHistory](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {

                constexpr mat4f normalizedToClip = {
                        float4{  2,  0,  0, 0 },
                        float4{  0,  2,  0, 0 },
                        float4{  0,  0,  -2, 0 },
                        float4{ -1, -1, 1, 1 },
                };

                constexpr float2 sampleOffsets[9] = {
                        { -1.0f, -1.0f }, {  0.0f, -1.0f }, {  1.0f, -1.0f },
                        { -1.0f,  0.0f }, {  0.0f,  0.0f }, {  1.0f,  0.0f },
                        { -1.0f,  1.0f }, {  0.0f,  1.0f }, {  1.0f,  1.0f },
                };

                FrameHistoryEntry& current = frameHistory.getCurrent();

                float sum = 0.0;
                float weights[9];

                // this doesn't get vectorized (probably because of exp()), so don't bother
                // unrolling it.
                #pragma nounroll
                for (size_t i = 0; i < 9; i++) {
                    float2 d = sampleOffsets[i] - current.jitter;
                    d *= 1.0f / taaOptions.filterWidth;
                    // this is a gaussian fit of a 3.3 Blackman Harris window
                    // see: "High Quality Temporal Supersampling" by Bruan Karis
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
                FMaterialInstance* mi = material.getMaterialInstance();
                mi->setParameter("color",  color, {});  // nearest
                mi->setParameter("depth",  depth, {});  // nearest
                mi->setParameter("alpha", taaOptions.feedback);
                mi->setParameter("history", history, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                mi->setParameter("filterWeights",  weights, 9);
                mi->setParameter("reprojection",
                        *historyProjection *
                        inverse(current.projection) *
                        normalizedToClip);

                mi->commit(driver);
                mi->use(driver);

                const uint8_t variant = uint8_t(colorGradingConfig.translucent ?
                        PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                if (colorGradingConfig.asSubpass) {
                    out.params.subpassMask = 1;
                }
                driver.beginRenderPass(out.target, out.params);
                driver.draw(material.getPipelineState(variant), mEngine.getFullScreenRenderPrimitive());
                if (colorGradingConfig.asSubpass) {
                    colorGradingSubpass(driver, colorGradingConfig);
                }
                driver.endRenderPass();

                // perform TAA here using colorHistory + input -> output
                resources.detach(data.output, &current.color, &current.colorDesc);
            });
    return colorGradingConfig.asSubpass ? taa->tonemappedOutput : taa->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::opaqueBlit(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Descriptor outDesc,
        SamplerMagFilter filter) noexcept {

    struct PostProcessScaling {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto& ppBlit = fg.addPass<PostProcessScaling>("opaque blit",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const& inputDesc = fg.getDescriptor(input);

                // we currently have no use for this case, so we just assert. This is better for now to trap
                // cases that we might not intend.
                assert_invariant(inputDesc.samples <= 1);

                data.output = builder.declareRenderPass(
                        builder.createTexture("opaque blit output", outDesc));

                // FIXME: here we use sample() instead of read() because this forces the
                //      backend to use a texture (instead of a renderbuffer). We need this because
                //      "implicit resolve" renderbuffers are currently not supported -- and
                //      implicit resolves are needed when taking the blit path.
                //      (we do this only when the texture does not request multisampling, since
                //      these are not sampleable).
                data.input = (inputDesc.samples > 1) ? builder.read(input) : builder.sample(input);

                // We use a RenderPass for the source here, instead of just creating a render
                // target from data.input in the execute closure, because data.input may refer to
                // an imported render target and in this case data.input won't resolve to an actual
                // HwTexture handle. Using a RenderPass works because data.input will resolve
                // to the actual imported render target and will have the correct viewport.
                builder.declareRenderPass("opaque blit input", {
                        .attachments = { .color = { data.input }}});
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

FrameGraphId<FrameGraphTexture> PostProcessManager::blendBlit(
        FrameGraph& fg, bool translucent, View::QualityLevel quality,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphTexture::Descriptor outDesc) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto& ppQuadBlit = fg.addPass<QuadBlitData>("quad scaling",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("scaled output", outDesc);
                data.output = builder.declareRenderPass(data.output);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto color = resources.getTexture(data.input);
                auto out = resources.getRenderPassInfo();
                auto const& desc = resources.getDescriptor(data.input);

                const StaticString blitterNames[3] = { "blitLow", "blitMedium", "blitHigh" };
                unsigned index = std::min(2u, (unsigned)quality);
                auto& material = getPostProcessMaterial(blitterNames[index]);
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("color", color, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                mi->setParameter("resolution",
                        float4{ desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height });
                mi->commit(driver);
                mi->use(driver);

                PipelineState pipeline(material.getPipelineState());
                if (translucent) {
                    pipeline.rasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionDstRGB   = BlendFunction::ONE_MINUS_SRC_ALPHA;
                    pipeline.rasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
                }
                driver.beginRenderPass(out.target, out.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    // we rely on automatic culling of unused render passes
    return ppQuadBlit->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::resolve(FrameGraph& fg,
        const char* outputBufferName, FrameGraphId<FrameGraphTexture> input) noexcept {

    // Don't do anything if we're not a MSAA buffer
    auto desc = fg.getDescriptor(input);
    if (desc.samples <= 1) {
        return input;
    }

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
                                   rpDesc.attachments.depth :
                                   rpDesc.attachments.color[0];

                data.usage = isDepthFormat(desc.format) ?
                             FrameGraphTexture::Usage::DEPTH_ATTACHMENT :
                             FrameGraphTexture::Usage::COLOR_ATTACHMENT;

                data.inFlags = isDepthFormat(desc.format) ?
                               backend::TargetBufferFlags::DEPTH :
                               backend::TargetBufferFlags::COLOR0;

                data.input = builder.read(input, data.usage);

                auto outputDesc = desc;
                outputDesc.levels = 1;
                outputDesc.samples = 0;

                rpDescAttachment = builder.createTexture(outputBufferName, outputDesc);
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
                            inDesc.samples, { in }, {}, {});
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
        FrameGraphId<FrameGraphTexture> input, uint8_t layer, size_t level, bool finalize) noexcept {

    struct VsmMipData {
        FrameGraphId<FrameGraphTexture> in;
    };

    auto& depthMipmapPass = fg.addPass<VsmMipData>("VSM Generate Mipmap Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                const char* name = builder.getName(input);
                data.in = builder.sample(input);

                auto out = builder.createSubresource(data.in, "Mip level", {
                        .level = uint8_t(level + 1), .layer = layer });

                out = builder.write(out, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass(name, {
                    .attachments = { .color = { out }},
                    .clearColor = { 1.0f, 1.0f, 1.0f, 1.0f },
                    .clearFlags = TargetBufferFlags::COLOR
                });
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
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("color", in, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });
                mi->setParameter("level", uint32_t(level));
                mi->setParameter("layer", uint32_t(layer));
                mi->setParameter("uvscale", 1.0f / dim);

                // When generating shadow map mip levels, we want to preserve the 1 texel border.
                auto vpWidth = (uint32_t) std::max(0, dim - 2);
                out.params.viewport = { 1, 1, vpWidth, vpWidth };

                commitAndRender(out, material, driver);

                if (finalize) {
                   driver.setMinMaxLevels(in, 0, level);
                }
            });

    return depthMipmapPass->in;
}

} // namespace filament
