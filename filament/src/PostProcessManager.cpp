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

#include "materials/antiAliasing/fxaa/fxaa.h"
#include "materials/antiAliasing/taa/taa.h"
#include "materials/bloom/bloom.h"
#include "materials/colorGrading/colorGrading.h"
#include "materials/dof/dof.h"
#include "materials/flare/flare.h"
#include "materials/fsr/fsr.h"
#include "materials/sgsr/sgsr.h"
#include "materials/ssao/ssao.h"

#include "details/Engine.h"

#include "ds/SsrPassDescriptorSet.h"
#include "ds/TypedUniformBuffer.h"

#include "fg/FrameGraph.h"
#include "fg/FrameGraphId.h"
#include "fg/FrameGraphResources.h"
#include "fg/FrameGraphTexture.h"

#include "fsr.h"
#include "FrameHistory.h"
#include "RenderPass.h"

#include "details/Camera.h"
#include "details/ColorGrading.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "details/Texture.h"
#include "details/VertexBuffer.h"

#include "generated/resources/materials.h"

#include <filament/Material.h>
#include <filament/MaterialEnums.h>
#include <filament/Options.h>
#include <filament/Viewport.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/UibStructs.h>

#include <backend/DriverEnums.h>
#include <backend/DriverApiForward.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>
#include <backend/PixelBufferDescriptor.h>

#include <private/backend/BackendUtils.h>

#include <math/half.h>
#include <math/mat2.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/scalar.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/algorithm.h>
#include <utils/BitmaskEnum.h>
#include <utils/debug.h>
#include <utils/compiler.h>
#include <utils/FixedCapacityVector.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>
#include <variant>
#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament {

using namespace utils;
using namespace math;
using namespace backend;

static constexpr uint8_t kMaxBloomLevels = 12u;
static_assert(kMaxBloomLevels >= 3, "We require at least 3 bloom levels");

namespace {

constexpr float halton(unsigned int i, unsigned int const b) noexcept {
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

template <typename ValueType>
void setConstantParameter(FMaterial* const material, std::string_view const name,
        ValueType value, bool& dirty) noexcept {
    auto id = material->getSpecializationConstantId(name);
    if (id.has_value()) {
        if (material->setConstant(id.value(), value)) {
            dirty = true;
        }
    }
}

} // anonymous

// ------------------------------------------------------------------------------------------------

PostProcessManager::PostProcessMaterial::PostProcessMaterial(StaticMaterialInfo const& info) noexcept
    : mConstants(info.constants.begin(), info.constants.size()) {
    mData = info.data; // aliased to mMaterial
    mSize = info.size;
}

PostProcessManager::PostProcessMaterial::PostProcessMaterial(
        PostProcessMaterial&& rhs) noexcept
        : mData(nullptr), mSize(0), mConstants(rhs.mConstants) {
    using namespace std;
    swap(mData, rhs.mData); // aliased to mMaterial
    swap(mSize, rhs.mSize);
}

PostProcessManager::PostProcessMaterial& PostProcessManager::PostProcessMaterial::operator=(
        PostProcessMaterial&& rhs) noexcept {
    if (this != &rhs) {
        using namespace std;
        swap(mData, rhs.mData); // aliased to mMaterial
        swap(mSize, rhs.mSize);
        swap(mConstants, rhs.mConstants);
    }
    return *this;
}

PostProcessManager::PostProcessMaterial::~PostProcessMaterial() noexcept {
    assert_invariant((!mSize && !mMaterial) || (mSize && mData));
}

void PostProcessManager::PostProcessMaterial::terminate(FEngine& engine) noexcept {
    if (!mSize) {
        engine.destroy(mMaterial);
        mMaterial = nullptr;
    }
}

UTILS_NOINLINE
void PostProcessManager::PostProcessMaterial::loadMaterial(FEngine& engine) const noexcept {
    // TODO: After all materials using this class have been converted to the post-process material
    //       domain, load both OPAQUE and TRANSPARENT variants here.
    auto builder = Material::Builder();
    builder.package(mData, mSize);
    for (auto const& constant: mConstants) {
        std::visit([&](auto&& arg) {
            builder.constant(constant.name.data(), constant.name.size(), arg);
        }, constant.value);
    }
    mMaterial = downcast(builder.build(engine));
    mSize = 0; // material loaded
}

UTILS_NOINLINE
FMaterial* PostProcessManager::PostProcessMaterial::getMaterial(FEngine& engine,
        PostProcessVariant variant) const noexcept {
    if (UTILS_UNLIKELY(mSize)) {
        loadMaterial(engine);
    }
    mMaterial->prepareProgram(Variant{ Variant::type_t(variant) });
    return mMaterial;
}

UTILS_NOINLINE
FMaterialInstance* PostProcessManager::PostProcessMaterial::getMaterialInstance(
        FMaterial const* const ma) noexcept {
    // TODO: we need to move away from the default instance
    return const_cast<FMaterial *>(ma)->getDefaultInstance();
}

UTILS_NOINLINE
FMaterialInstance* PostProcessManager::PostProcessMaterial::getMaterialInstance(FEngine& engine,
        PostProcessMaterial const& material, PostProcessVariant const variant) noexcept {
    FMaterial const* ma = material.getMaterial(engine, variant);
    return getMaterialInstance(ma);
}

// ------------------------------------------------------------------------------------------------

const PostProcessManager::JitterSequence<4> PostProcessManager::sRGSS4 = {{{
        { 0.625f, 0.125f },
        { 0.125f, 0.375f },
        { 0.875f, 0.625f },
        { 0.375f, 0.875f }
}}};

const PostProcessManager::JitterSequence<4> PostProcessManager::sUniformHelix4 = {{{
        { 0.25f, 0.25f },
        { 0.75f, 0.75f },
        { 0.25f, 0.75f },
        { 0.75f, 0.25f },
}}};

template<size_t COUNT>
static constexpr auto halton() {
    std::array<float2, COUNT> h;
    for (size_t i = 0; i < COUNT; i++) {
        h[i] = {
                halton(i, 2),
                halton(i, 3) };
    }
    return h;
}

const PostProcessManager::JitterSequence<32>
        PostProcessManager::sHaltonSamples = { halton<32>() };

PostProcessManager::PostProcessManager(FEngine& engine) noexcept
        : mEngine(engine),
          mWorkaroundSplitEasu(false),
          mWorkaroundAllowReadOnlyAncillaryFeedbackLoop(false) {
    // don't use Engine here, it's not fully initialized yet
}

PostProcessManager::~PostProcessManager() noexcept = default;

void PostProcessManager::setFrameUniforms(DriverApi& driver,
        TypedUniformBuffer<PerViewUib>& uniforms) noexcept {
    mPostProcessDescriptorSet.setFrameUniforms(driver, uniforms);
    mSsrPassDescriptorSet.setFrameUniforms(mEngine, uniforms);
}

void PostProcessManager::bindPostProcessDescriptorSet(DriverApi& driver) const noexcept {
    mPostProcessDescriptorSet.bind(driver);
}

UTILS_NOINLINE
void PostProcessManager::registerPostProcessMaterial(std::string_view const name,
        StaticMaterialInfo const& info) {
    mMaterialRegistry.try_emplace(name, info);
}

UTILS_NOINLINE
PostProcessManager::PostProcessMaterial& PostProcessManager::getPostProcessMaterial(
        std::string_view const name) noexcept {
    auto pos = mMaterialRegistry.find(name);
    assert_invariant(pos != mMaterialRegistry.end());
    return pos.value();
}

// StaticMaterialInfo::ConstantInfo destructor is called during shut-down, to avoid side-effect
// we ensure it's trivially destructible
static_assert(std::is_trivially_destructible_v<PostProcessManager::StaticMaterialInfo::ConstantInfo>);

#define MATERIAL(p, n) p ## _ ## n ## _DATA, size_t(p ## _ ## n ## _SIZE)

static const PostProcessManager::StaticMaterialInfo sMaterialListFeatureLevel0[] = {
        { "blitLow",                    MATERIAL(MATERIALS, BLITLOW) },
};

static const PostProcessManager::StaticMaterialInfo sMaterialList[] = {
        { "blitArray",                  MATERIAL(MATERIALS, BLITARRAY) },
        { "blitDepth",                  MATERIAL(MATERIALS, BLITDEPTH) },
        { "separableGaussianBlur1",     MATERIAL(MATERIALS, SEPARABLEGAUSSIANBLUR),
                { {"arraySampler", false}, {"componentCount", 1} } },
        { "separableGaussianBlur1L",    MATERIAL(MATERIALS, SEPARABLEGAUSSIANBLUR),
                { {"arraySampler", true }, {"componentCount", 1} } },
        { "separableGaussianBlur2",     MATERIAL(MATERIALS, SEPARABLEGAUSSIANBLUR),
                { {"arraySampler", false}, {"componentCount", 2} } },
        { "separableGaussianBlur2L",    MATERIAL(MATERIALS, SEPARABLEGAUSSIANBLUR),
                { {"arraySampler", true }, {"componentCount", 2} } },
        { "separableGaussianBlur3",     MATERIAL(MATERIALS, SEPARABLEGAUSSIANBLUR),
                { {"arraySampler", false}, {"componentCount", 3} } },
        { "separableGaussianBlur3L",    MATERIAL(MATERIALS, SEPARABLEGAUSSIANBLUR),
                { {"arraySampler", true }, {"componentCount", 3} } },
        { "separableGaussianBlur4",     MATERIAL(MATERIALS, SEPARABLEGAUSSIANBLUR),
                { {"arraySampler", false}, {"componentCount", 4} } },
        { "separableGaussianBlur4L",    MATERIAL(MATERIALS, SEPARABLEGAUSSIANBLUR),
                { {"arraySampler", true }, {"componentCount", 4} } },
        { "vsmMipmap",                  MATERIAL(MATERIALS, VSMMIPMAP) },
        { "debugShadowCascades",        MATERIAL(MATERIALS, DEBUGSHADOWCASCADES) },
        { "resolveDepth",               MATERIAL(MATERIALS, RESOLVEDEPTH) },
        { "shadowmap",                  MATERIAL(MATERIALS, SHADOWMAP) },
};

void PostProcessManager::init() noexcept {
    auto& engine = mEngine;
    DriverApi& driver = engine.getDriverApi();

    //FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    //debugRegistry.registerProperty("d.ssao.sampleCount", &engine.debug.ssao.sampleCount);
    //debugRegistry.registerProperty("d.ssao.spiralTurns", &engine.debug.ssao.spiralTurns);
    //debugRegistry.registerProperty("d.ssao.kernelSize", &engine.debug.ssao.kernelSize);
    //debugRegistry.registerProperty("d.ssao.stddev", &engine.debug.ssao.stddev);

    mFullScreenQuadRph = engine.getFullScreenRenderPrimitive();
    mFullScreenQuadVbih = engine.getFullScreenVertexBuffer()->getVertexBufferInfoHandle();
    mPerRenderableDslh = engine.getPerRenderableDescriptorSetLayout().getHandle();

    mSsrPassDescriptorSet.init(engine);
    mPostProcessDescriptorSet.init(engine);
    mStructureDescriptorSet.init(engine);

    mWorkaroundSplitEasu =
            driver.isWorkaroundNeeded(Workaround::SPLIT_EASU);
    mWorkaroundAllowReadOnlyAncillaryFeedbackLoop =
            driver.isWorkaroundNeeded(Workaround::ALLOW_READ_ONLY_ANCILLARY_FEEDBACK_LOOP);

    UTILS_NOUNROLL
    for (auto const& info: sMaterialListFeatureLevel0) {
        registerPostProcessMaterial(info.name, info);
    }

    if (mEngine.getActiveFeatureLevel() >= FeatureLevel::FEATURE_LEVEL_1) {
        UTILS_NOUNROLL
        for (auto const& info: sMaterialList) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getBloomMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getFlareMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getDofMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getColorGradingMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getFsrMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getSgsrMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getFxaaMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getTaaMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
        for (auto const& info: getSsaoMaterialList()) {
            registerPostProcessMaterial(info.name, info);
        }
    }

    if (engine.hasFeatureLevel(FeatureLevel::FEATURE_LEVEL_1)) {
        mStarburstTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
                TextureFormat::R8, 1, 256, 1, 1, TextureUsage::DEFAULT);

        PixelBufferDescriptor dataStarburst(driver.allocate(256), 256,
                PixelDataFormat::R, PixelDataType::UBYTE);
        std::generate_n((uint8_t*)dataStarburst.buffer, 256,
                [&dist = mUniformDistribution, &gen = mEngine.getRandomEngine()]() {
                    float const r = 0.5f + 0.5f * dist(gen);
                    return uint8_t(r * 255.0f);
                });

        driver.update3DImage(mStarburstTexture,
                0, 0, 0, 0, 256, 1, 1,
                std::move(dataStarburst));
    }
}

void PostProcessManager::terminate(DriverApi& driver) noexcept {
    FEngine& engine = mEngine;
    driver.destroyTexture(mStarburstTexture);

    auto first = mMaterialRegistry.begin();
    auto const last = mMaterialRegistry.end();
    while (first != last) {
        first.value().terminate(engine);
        ++first;
    }

    mPostProcessDescriptorSet.terminate(engine.getDescriptorSetLayoutFactory(), driver);
    mSsrPassDescriptorSet.terminate(driver);
    mStructureDescriptorSet.terminate(driver);
}

Handle<HwTexture> PostProcessManager::getOneTexture() const {
    return mEngine.getOneTexture();
}

Handle<HwTexture> PostProcessManager::getZeroTexture() const {
    return mEngine.getZeroTexture();
}

Handle<HwTexture> PostProcessManager::getOneTextureArray() const {
    return mEngine.getOneTextureArray();
}

Handle<HwTexture> PostProcessManager::getZeroTextureArray() const {
    return mEngine.getZeroTextureArray();
}

UTILS_NOINLINE
PipelineState PostProcessManager::getPipelineState(
        FMaterial const* const ma, PostProcessVariant variant) const noexcept {
    return {
            .program = ma->getProgram(Variant{ Variant::type_t(variant) }),
            .vertexBufferInfo = mFullScreenQuadVbih,
            .pipelineLayout = {
                    .setLayout = {
                            ma->getPerViewDescriptorSetLayout().getHandle(),
                            mPerRenderableDslh,
                            ma->getDescriptorSetLayout().getHandle()
                    }},
            .rasterState = ma->getRasterState()
    };
}

UTILS_NOINLINE
void PostProcessManager::renderFullScreenQuad(
        FrameGraphResources::RenderPassInfo const& out,
        PipelineState const& pipeline,
        DriverApi& driver) const noexcept {
    assert_invariant(
            ((out.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH)
                    && !pipeline.rasterState.depthWrite)
            || !(out.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH));
    driver.beginRenderPass(out.target, out.params);
    driver.draw(pipeline, mFullScreenQuadRph, 0, 3, 1);
    driver.endRenderPass();
}

UTILS_NOINLINE
void PostProcessManager::renderFullScreenQuadWithScissor(
        FrameGraphResources::RenderPassInfo const& out,
        PipelineState const& pipeline,
        backend::Viewport const scissor,
        DriverApi& driver) const noexcept {
    assert_invariant(
            ((out.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH)
                    && !pipeline.rasterState.depthWrite)
            || !(out.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH));
    driver.beginRenderPass(out.target, out.params);
    driver.scissor(scissor);
    driver.draw(pipeline, mFullScreenQuadRph, 0, 3, 1);
    driver.endRenderPass();
}

UTILS_NOINLINE
void PostProcessManager::commitAndRenderFullScreenQuad(DriverApi& driver,
        FrameGraphResources::RenderPassInfo const& out, FMaterialInstance const* mi,
        PostProcessVariant const variant) const noexcept {
    mi->commit(driver);
    mi->use(driver);
    FMaterial const* const ma = mi->getMaterial();
    PipelineState const pipeline = getPipelineState(ma, variant);

    assert_invariant(
            ((out.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH)
             && !pipeline.rasterState.depthWrite)
            || !(out.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH));

    driver.beginRenderPass(out.target, out.params);
    driver.draw(pipeline, mFullScreenQuadRph, 0, 3, 1);
    driver.endRenderPass();
}

// ------------------------------------------------------------------------------------------------

PostProcessManager::StructurePassOutput PostProcessManager::structure(FrameGraph& fg,
        RenderPassBuilder const& passBuilder, uint8_t const structureRenderFlags,
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
    width  = std::max(32u, uint32_t(std::ceil(float(width) * scale)));
    height = std::max(32u, uint32_t(std::ceil(float(height) * scale)));

    // We limit the lowest lod size to 32 pixels (which is where the -5 comes from)
    const size_t levelCount = std::min(8, FTexture::maxLevelCount(width, height) - 5);
    assert_invariant(levelCount >= 1);

    // generate depth pass at the requested resolution
    auto const& structurePass = fg.addPass<StructurePassData>("Structure Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                bool const isES2 = mEngine.getDriverApi().getFeatureLevel() == FeatureLevel::FEATURE_LEVEL_0;
                data.depth = builder.createTexture("Structure Buffer", {
                        .width = width, .height = height,
                        .levels = uint8_t(levelCount),
                        .format = isES2 ? TextureFormat::DEPTH24 : TextureFormat::DEPTH32F });

                data.depth = builder.write(data.depth,
                        FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                if (config.picking) {
                    // FIXME: the DescriptorSetLayout must specify SAMPLER_FLOAT
                    data.picking = builder.createTexture("Picking Buffer", {
                            .width = width, .height = height,
                            .format = isES2 ? TextureFormat::RGBA8 : TextureFormat::RG32F });

                    data.picking = builder.write(data.picking,
                            FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                }

                builder.declareRenderPass("Structure Target", {
                        .attachments = { .color = { data.picking }, .depth = data.depth },
                        .clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH
                });
            },
            [=, passBuilder = passBuilder](FrameGraphResources const& resources,
                    auto const&, DriverApi& driver) mutable {
                Variant structureVariant(Variant::DEPTH_VARIANT);
                structureVariant.setPicking(config.picking);

                // bind the per-view descriptorSet that is used for the structure pass
                getStructureDescriptorSet().bind(driver);

                passBuilder.renderFlags(structureRenderFlags);
                passBuilder.variant(structureVariant);
                passBuilder.commandTypeFlags(RenderPass::CommandTypeFlags::SSAO);

                RenderPass const pass{ passBuilder.build(mEngine, driver) };
                auto const out = resources.getRenderPassInfo();
                driver.beginRenderPass(out.target, out.params);
                pass.getExecutor().execute(mEngine, driver);
                driver.endRenderPass();
            });

    auto const depth = structurePass->depth;

    /*
     * create depth mipmap chain
    */

    struct StructureMipmapData {
        FrameGraphId<FrameGraphTexture> depth;
    };

    fg.addPass<StructureMipmapData>("StructureMipmap",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.depth = builder.sample(depth);
                for (size_t i = 1; i < levelCount; i++) {
                    auto out = builder.createSubresource(data.depth, "Structure mip", {
                            .level = uint8_t(i)
                    });
                    out = builder.write(out, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                    builder.declareRenderPass("Structure mip target", {
                            .attachments = { .depth = out }
                    });
                }
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

                auto in = resources.getTexture(data.depth);
                auto& material = getPostProcessMaterial("mipmapDepth");
                FMaterial const* const ma = material.getMaterial(mEngine);
                auto pipeline = getPipelineState(ma);

                FMaterialInstance* const mi = PostProcessMaterial::getMaterialInstance(ma);

                // The first mip already exists, so we process n-1 lods
                for (size_t level = 0; level < levelCount - 1; level++) {
                    auto out = resources.getRenderPassInfo(level);

                    auto th = driver.createTextureView(in, level, 1);
                    mi->setParameter("depth", th, {
                        .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                    mi->commit(driver);
                    mi->use(driver);
                    renderFullScreenQuad(out, pipeline, driver);
                    driver.destroyTexture(th);
                }
            });

    return { depth, structurePass->picking };
}

FrameGraphId<FrameGraphTexture> PostProcessManager::transparentPicking(FrameGraph& fg,
        RenderPassBuilder const& passBuilder, uint8_t const structureRenderFlags,
        uint32_t width, uint32_t height, float const scale) noexcept {

            struct PickingRenderPassData {
                FrameGraphId<FrameGraphTexture> depth;
                FrameGraphId<FrameGraphTexture> picking;
            };
            auto const& pickingRenderPass = fg.addPass<PickingRenderPassData>("Picking Render Pass",
                [&](FrameGraph::Builder& builder, auto& data) {
                    bool const isFL0 = mEngine.getDriverApi().getFeatureLevel() ==
                        FeatureLevel::FEATURE_LEVEL_0;

                    // TODO: Specify the precision for picking pass
                     width  = std::max(32u, uint32_t(std::ceil(float(width) * scale)));
                     height = std::max(32u, uint32_t(std::ceil(float(height) * scale)));
                    data.depth = builder.createTexture("Depth Buffer", {
                            .width = width, .height = height,
                            .format = isFL0 ? TextureFormat::DEPTH24 : TextureFormat::DEPTH32F });

                    data.depth = builder.write(data.depth,
                        FrameGraphTexture::Usage::DEPTH_ATTACHMENT);

                    data.picking = builder.createTexture("Picking Buffer", {
                            .width = width, .height = height,
                            .format = isFL0 ? TextureFormat::RGBA8 : TextureFormat::RG32F });

                    data.picking = builder.write(data.picking,
                        FrameGraphTexture::Usage::COLOR_ATTACHMENT);

                    builder.declareRenderPass("Picking Render Target", {
                            .attachments = {.color = { data.picking }, .depth = data.depth },
                            .clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH
                        });
                },
                [=, passBuilder = passBuilder](FrameGraphResources const& resources,
                    auto const&, DriverApi& driver) mutable {
                        Variant pickingVariant(Variant::DEPTH_VARIANT);
                        pickingVariant.setPicking(true);

                        // bind the per-view descriptorSet that is used for the structure pass
                        getStructureDescriptorSet().bind(driver);

                        auto [target, params] = resources.getRenderPassInfo();
                        passBuilder.renderFlags(structureRenderFlags);
                        passBuilder.variant(pickingVariant);
                        passBuilder.commandTypeFlags(RenderPass::CommandTypeFlags::DEPTH);

                        RenderPass const pass{ passBuilder.build(mEngine, driver) };
                        driver.beginRenderPass(target, params);
                        pass.getExecutor().execute(mEngine, driver);
                        driver.endRenderPass();
                });

    return pickingRenderPass->picking;
}

// ------------------------------------------------------------------------------------------------

FrameGraphId<FrameGraphTexture> PostProcessManager::ssr(FrameGraph& fg,
        RenderPassBuilder const& passBuilder,
        FrameHistory const& frameHistory,
        CameraInfo const& cameraInfo,
        FrameGraphId<FrameGraphTexture> const structure,
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

    auto const& previous = frameHistory.getPrevious().ssr;
    if (!previous.color.handle) {
        return {};
    }

    FrameGraphId<FrameGraphTexture> const history = fg.import("SSR history", previous.desc,
            FrameGraphTexture::Usage::SAMPLEABLE, previous.color);
    mat4 const& historyProjection = previous.projection;
    auto const& uvFromClipMatrix = mEngine.getUvFromClipMatrix();

    auto const& ssrPass = fg.addPass<SSRPassData>("SSR Pass",
            [&](FrameGraph::Builder& builder, auto& data) {

                // Create our reflection buffer. We need an alpha channel, so we have to use RGBA16F
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
                builder.declareRenderPass("Reflections Target", {
                        .attachments = { .color = { data.reflections }, .depth = data.depth },
                        .clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH });

                // get the structure buffer
                assert_invariant(structure);
                data.structure = builder.sample(structure);

                if (history) {
                    data.history = builder.sample(history);
                }
            },
            [this, projection = cameraInfo.projection,
                    userViewMatrix = cameraInfo.getUserViewMatrix(), uvFromClipMatrix, historyProjection,
                    options, passBuilder = passBuilder]
            (FrameGraphResources const& resources, auto const& data, DriverApi& driver) mutable {
                // set structure sampler
                mSsrPassDescriptorSet.prepareStructure(mEngine, data.structure ?
                        resources.getTexture(data.structure) : getOneTexture());

                // set screen-space reflections and screen-space refractions
                mat4f const uvFromViewMatrix = uvFromClipMatrix * projection;
                mat4f const reprojection = uvFromClipMatrix *
                        mat4f{ historyProjection * inverse(userViewMatrix) };

                // the history sampler is a regular texture2D
                TextureHandle const history = data.history ?
                        resources.getTexture(data.history) : getZeroTexture();
                mSsrPassDescriptorSet.prepareHistorySSR(mEngine, history, reprojection, uvFromViewMatrix, options);

                mSsrPassDescriptorSet.commit(mEngine);

                mSsrPassDescriptorSet.bind(driver);

                auto const out = resources.getRenderPassInfo();

                // Remove the HAS_SHADOWING RenderFlags, since it's irrelevant when rendering reflections
                passBuilder.renderFlags(RenderPass::HAS_SHADOWING, 0);

                // use our special SSR variant, it can only be applied to object that have
                // the SCREEN_SPACE ReflectionMode.
                passBuilder.variant(Variant{ Variant::SPECIAL_SSR });

                // generate all our drawing commands, except blended objects.
                passBuilder.commandTypeFlags(RenderPass::CommandTypeFlags::SCREEN_SPACE_REFLECTIONS);

                RenderPass const pass{ passBuilder.build(mEngine, driver) };
                driver.beginRenderPass(out.target, out.params);
                pass.getExecutor().execute(mEngine, driver);
                driver.endRenderPass();
            });

    return ssrPass->reflections;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::screenSpaceAmbientOcclusion(FrameGraph& fg,
        filament::Viewport const&, const CameraInfo& cameraInfo,
        FrameGraphId<FrameGraphTexture> const depth,
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
     *
     * This is also needed in Vulkan for a similar reason.
     */
    FrameGraphId<FrameGraphTexture> duplicateDepthOutput = {};
    if (!mWorkaroundAllowReadOnlyAncillaryFeedbackLoop) {
        duplicateDepthOutput = blitDepth(fg, depth);
    }

    auto const& SSAOPass = fg.addPass<SSAOPassData>(
            "SSAO Pass",
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

                auto depthAttachment = duplicateDepthOutput ? duplicateDepthOutput : data.depth;

                depthAttachment = builder.read(depthAttachment,
                        FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                builder.declareRenderPass("SSAO Target", {
                        .attachments = { .color = { data.ao, data.bn }, .depth = depthAttachment },
                        .clearColor = { 1.0f },
                        .clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::COLOR1
                });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                // bind the per-view descriptorSet that is used for the structure pass
                getStructureDescriptorSet().bind(driver);

                auto depth = resources.getTexture(data.depth);
                auto ssao = resources.getRenderPassInfo();
                auto const& desc = resources.getDescriptor(data.depth);

                // Estimate of the size in pixel units of a 1m tall/wide object viewed from 1m away (i.e. at z=-1)
                const float projectionScale = std::min(
                        0.5f * cameraInfo.projection[0].x * desc.width,
                        0.5f * cameraInfo.projection[1].y * desc.height);

                const auto invProjection = inverse(cameraInfo.projection);
                const float inc = (1.0f / (sampleCount - 0.5f)) * spiralTurns * f::TAU;

                const mat4 screenFromClipMatrix{ mat4::row_major_init{
                        0.5 * desc.width, 0.0, 0.0, 0.5 * desc.width,
                        0.0, 0.5 * desc.height, 0.0, 0.5 * desc.height,
                        0.0, 0.0, 0.5, 0.5,
                        0.0, 0.0, 0.0, 1.0
                }};

                std::string_view materialName;
                auto aoType = options.aoType;
#ifdef FILAMENT_DISABLE_GTAO
                materialName = computeBentNormals ? "saoBentNormals" : "sao";
                aoType = AmbientOcclusionOptions::AmbientOcclusionType::SAO;
#else
                if (aoType ==
                        AmbientOcclusionOptions::AmbientOcclusionType::GTAO) {
                    materialName = computeBentNormals ? "gtaoBentNormals" : "gtao";
                } else {
                    materialName = computeBentNormals ? "saoBentNormals" : "sao";
                }
#endif
                auto& material = getPostProcessMaterial(materialName);

                FMaterial const * const ma = material.getMaterial(mEngine);
                FMaterialInstance* const mi = PostProcessMaterial::getMaterialInstance(ma);

                // Set AO type specific material parameters
                switch (aoType) {
                    case AmbientOcclusionOptions::AmbientOcclusionType::SAO: {
                        // Where the falloff function peaks
                        const float peak = 0.1f * options.radius;
                        const float intensity = (f::TAU * peak) * options.intensity;

                        // always square AO result, as it looks much better
                        const float power = options.power * 2.0f;

                        mi->setParameter("minHorizonAngleSineSquared",
                                std::pow(std::sin(options.minHorizonAngleRad), 2.0f));
                        mi->setParameter("intensity", intensity / sampleCount);
                        mi->setParameter("power", power);
                        mi->setParameter("peak2", peak * peak);
                        mi->setParameter("bias", options.bias);
                        mi->setParameter("sampleCount",
                                float2{ sampleCount, 1.0f / (sampleCount - 0.5f) });
                        mi->setParameter("spiralTurns", spiralTurns);
                        mi->setParameter("angleIncCosSin", float2{ std::cos(inc), std::sin(inc) });
                        break;
                    }
                    case AmbientOcclusionOptions::AmbientOcclusionType::GTAO: {
                        const auto sliceCount = static_cast<float>(options.gtao.sampleSliceCount);
                        mi->setParameter("stepsPerSlice",
                                static_cast<float>(options.gtao.sampleStepsPerSlice));
                        mi->setParameter("sliceCount",
                                float2{ sliceCount, 1.0f / sliceCount });
                        mi->setParameter("power", options.power);
                        mi->setParameter("radius", options.radius);
                        mi->setParameter("intensity", options.intensity);
                        mi->setParameter("thicknessHeuristic", options.gtao.thicknessHeuristic);

                        break;
                    }
                }

                // Set common material parameters
                mi->setParameter("invRadiusSquared", 1.0f / (options.radius * options.radius));
                mi->setParameter("depth", depth,
                        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("screenFromViewMatrix",
                        mat4f(screenFromClipMatrix * cameraInfo.projection));
                mi->setParameter("resolution",
                        float4{ desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height });
                mi->setParameter("projectionScale", projectionScale);
                mi->setParameter("projectionScaleRadius", projectionScale * options.radius);
                mi->setParameter("positionParams",
                        float2{ invProjection[0][0], invProjection[1][1] } * 2.0f);
                mi->setParameter("maxLevel", uint32_t(levelCount - 1));
                mi->setParameter("invFarPlane", 1.0f / -cameraInfo.zf);
                mi->setParameter("ssctShadowDistance", options.ssct.shadowDistance);
                mi->setParameter("ssctConeAngleTangeant",
                        std::tan(options.ssct.lightConeRad * 0.5f));
                mi->setParameter("ssctContactDistanceMaxInv",
                        1.0f / options.ssct.contactDistanceMax);
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
                        float2{ options.ssct.rayCount, 1.0f / float(options.ssct.rayCount) });

                mi->commit(driver);
                mi->use(driver);

                auto pipeline = getPipelineState(ma);
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::L;
                assert_invariant(ssao.params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH);
                renderFullScreenQuad(ssao, pipeline, driver);
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
        FrameGraphId<FrameGraphTexture> const input,
        FrameGraphId<FrameGraphTexture> depth,
        int2 const axis, float const zf, TextureFormat const format,
        BilateralPassConfig const& config) noexcept {
    assert_invariant(depth);

    struct BlurPassData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> blurred;
        FrameGraphId<FrameGraphTexture> ao;
        FrameGraphId<FrameGraphTexture> bn;
    };

    auto const& blurPass = fg.addPass<BlurPassData>("Separable Blur Pass",
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

                builder.declareRenderPass("Blurred target", {
                        .attachments = { .color = { data.ao, data.bn }, .depth = depth },
                        .clearColor = { 1.0f },
                        .clearFlags = TargetBufferFlags::COLOR0 | TargetBufferFlags::COLOR1
                });
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

                auto ssao = resources.getTexture(data.input);
                auto blurred = resources.getRenderPassInfo();
                auto const& desc = resources.getDescriptor(data.blurred);

                // unnormalized gaussian half-kernel of a given standard deviation
                // returns number of samples stored in the array (max 16)
                constexpr size_t kernelArraySize = 16; // limited by bilateralBlur.mat
                auto gaussianKernel =
                        [kernelArraySize](float* outKernel, size_t const gaussianWidth, float const stdDev) -> uint32_t {
                    const size_t gaussianSampleCount = std::min(kernelArraySize, (gaussianWidth + 1u) / 2u);
                    for (size_t i = 0; i < gaussianSampleCount; i++) {
                        float const x = float(i);
                        float const g = std::exp(-(x * x) / (2.0f * stdDev * stdDev));
                        outKernel[i] = g;
                    }
                    return uint32_t(gaussianSampleCount);
                };

                float kGaussianSamples[kernelArraySize];
                uint32_t const kGaussianCount = gaussianKernel(kGaussianSamples,
                        config.kernelSize, config.standardDeviation);

                auto& material = config.bentNormals ?
                        getPostProcessMaterial("bilateralBlurBentNormals") :
                        getPostProcessMaterial("bilateralBlur");
                FMaterial const* const ma = material.getMaterial(mEngine);
                FMaterialInstance* const mi = PostProcessMaterial::getMaterialInstance(ma);
                mi->setParameter("ssao", ssao, { /* only reads level 0 */ });
                mi->setParameter("axis", axis / float2{desc.width, desc.height});
                mi->setParameter("kernel", kGaussianSamples, kGaussianCount);
                mi->setParameter("sampleCount", kGaussianCount);
                mi->setParameter("farPlaneOverEdgeDistance", -zf / config.bilateralThreshold);

                mi->commit(driver);
                mi->use(driver);

                auto pipeline = getPipelineState(ma);
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::L;
                renderFullScreenQuad(blurred, pipeline, driver);
            });

    return blurPass->blurred;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::generateGaussianMipmap(FrameGraph& fg,
        const FrameGraphId<FrameGraphTexture> input, size_t const levels,
        bool reinhard, size_t const kernelWidth, float const sigma) noexcept {

    auto const subResourceDesc = fg.getSubResourceDescriptor(input);

    // Create one subresource per level to be generated from the input. These will be our
    // destinations.
    struct MipmapPassData {
        FixedCapacityVector<FrameGraphId<FrameGraphTexture>> out;
    };
    auto const& mipmapPass = fg.addPass<MipmapPassData>("Mipmap Pass",
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
        auto const output = mipmapPass->out[i];
        from = gaussianBlurPass(fg, from, output, reinhard, kernelWidth, sigma);
        reinhard = false; // only do the reinhard filtering on the first level
    }

    // return our original input (we only wrote into sub resources)
    return input;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::gaussianBlurPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const input,
        FrameGraphId<FrameGraphTexture> output,
        bool const reinhard, size_t kernelWidth, const float sigma) noexcept {

    auto computeGaussianCoefficients =
            [kernelWidth, sigma](float2* kernel, size_t const size) -> size_t {
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
            float const x0 = float(i * 2 - 1);
            float const x1 = float(i * 2);
            float const k0 = std::exp(-alpha * x0 * x0);
            float const k1 = std::exp(-alpha * x1 * x1);

            // k * textureLod(..., o) with bilinear sampling is equivalent to:
            //      k * (s[0] * (1 - o) + s[1] * o)
            // solve:
            //      k0 = k * (1 - o)
            //      k1 = k * o

            float const k = k0 + k1;
            float const o = k1 / k;
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

    auto const& blurPass = fg.addPass<BlurPassData>("Gaussian Blur Pass (separable)",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const inDesc = builder.getDescriptor(input);

                if (!output) {
                    output = builder.createTexture("Blurred texture", inDesc);
                }

                auto const outDesc = builder.getDescriptor(output);
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
                bindPostProcessDescriptorSet(driver);

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
                switch (getFormatComponentCount(outDesc.format)) {
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
                FMaterialInstance* const mi = PostProcessMaterial::getMaterialInstance(
                        mEngine, separableGaussianBlur);

                const size_t kernelStorageSize = mi->getMaterial()->reflect("kernel")->size;


                // Initialize the samplers with dummy textures because vulkan requires a sampler to
                // be bound to a texture even if sampler might be unused.
                mi->setParameter("sourceArray"sv, getZeroTextureArray(), {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });
                mi->setParameter("source"sv, getZeroTexture(), {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });


                float2 kernel[64];
                size_t const m = computeGaussianCoefficients(kernel,
                        std::min(std::size(kernel), kernelStorageSize));

                std::string_view const sourceParameterName = is2dArray ? "sourceArray"sv : "source"sv;
                // horizontal pass
                mi->setParameter(sourceParameterName, hwIn, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });
                mi->setParameter("level", float(inSubDesc.level));
                mi->setParameter("layer", float(inSubDesc.layer));
                mi->setParameter("reinhard", reinhard ? uint32_t(1) : uint32_t(0));
                mi->setParameter("axis",float2{ 1.0f / inDesc.width, 0 });
                mi->setParameter("count", int32_t(m));
                mi->setParameter("kernel", kernel, m);

                // The framegraph only computes discard flags at FrameGraphPass boundaries
                hwTempRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                commitAndRenderFullScreenQuad(driver, hwTempRT, mi);

                // vertical pass
                UTILS_UNUSED_IN_RELEASE auto width = outDesc.width;
                UTILS_UNUSED_IN_RELEASE auto height = outDesc.height;
                assert_invariant(width == hwOutRT.params.viewport.width);
                assert_invariant(height == hwOutRT.params.viewport.height);

                mi->setParameter(sourceParameterName, hwTemp, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR /* level is always 0 */
                });
                mi->setParameter("level", 0.0f);
                mi->setParameter("layer", 0.0f);
                mi->setParameter("axis", float2{ 0, 1.0f / tempDesc.height });

                commitAndRenderFullScreenQuad(driver, hwOutRT, mi);
            });

    return blurPass->out;
}

PostProcessManager::ScreenSpaceRefConfig PostProcessManager::prepareMipmapSSR(FrameGraph& fg,
        uint32_t const width, uint32_t const height, TextureFormat const format,
        float const verticalFieldOfView, float2 const scale) noexcept {

    // The kernel-size was determined empirically so that we don't get too many artifacts
    // due to the down-sampling with a box filter (which happens implicitly).
    // Requires only 6 stored coefficients and 11 tap/pass
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
    auto const& pass = fg.addPass<PrepareMipmapSSRPassData>("Prepare MipmapSSR Pass",
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
        bool const needInputDuplication, ScreenSpaceRefConfig const& config) noexcept {

    // Descriptor of our actual input image (e.g. reflection buffer or refraction framebuffer)
    auto const& desc = fg.getDescriptor(input);

    // Descriptor of the destination. `output` is a subresource (i.e. a layer of a 2D array)
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
            // Resolve directly into the destination. This guarantees a blit/resolve will be
            // performed (i.e.: the source is copied) and we also guarantee that format/scaling
            // is the same after the forwardResource call below.
            input = ppm.resolve(fg, "ssr", input, outDesc);
        } else {
            // First resolve (if needed), may be a no-op. Guarantees that format/size is unchanged
            // by construction.
            input = ppm.resolve(fg, "ssr", input, { .levels = 1 });
            // Then blit into an appropriate texture, this handles scaling and format conversion.
            // The input/output sizes may differ when non-homogenous DSR is enabled.
            input = ppm.blit(fg, false, input, { 0, 0, desc.width, desc.height }, outDesc,
                    SamplerMagFilter::LINEAR, SamplerMinFilter::LINEAR);
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
        FrameGraphId<FrameGraphTexture> const input,
        FrameGraphId<FrameGraphTexture> const depth,
        const CameraInfo& cameraInfo,
        bool const translucent,
        float2 bokehScale,
        const DepthOfFieldOptions& dofOptions) noexcept {

    assert_invariant(depth);

    PostProcessVariant const variant =
            translucent ? PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE;

    const TextureFormat format = translucent ? TextureFormat::RGBA16F
                                             : TextureFormat::R11F_G11F_B10F;

    // Rotate the bokeh based on the aperture diameter (i.e. angle of the blades)
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
    const float Ks = float(desc.height) / FCamera::SENSOR_SIZE;
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
    constexpr uint32_t maxMipLevels = 4u;

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

    auto const& ppDoFDownsample = fg.addPass<PostProcessDofDownsample>("DoF Downsample",
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
                                .r = TextureSwizzle::CHANNEL_0,
                                .g = TextureSwizzle::CHANNEL_0 },
                });
                data.outColor = builder.write(data.outColor, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                data.outCoc   = builder.write(data.outCoc,   FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass("DoF Target", { .attachments = {
                                .color = { data.outColor, data.outCoc }
                        }
                });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto const& out = resources.getRenderPassInfo();
                auto color = resources.getTexture(data.color);
                auto depth = resources.getTexture(data.depth);
                auto const& material = (dofResolution == 1) ?
                        getPostProcessMaterial("dofCoc") :
                        getPostProcessMaterial("dofDownsample");
                FMaterialInstance* const mi = PostProcessMaterial::getMaterialInstance(
                        mEngine, material);

                mi->setParameter("color", color, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("depth", depth, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("cocParams", cocParams);
                mi->setParameter("cocClamp", float2{
                    -(dofOptions.maxForegroundCOC ? dofOptions.maxForegroundCOC : DOF_DEFAULT_MAX_COC),
                      dofOptions.maxBackgroundCOC ? dofOptions.maxBackgroundCOC : DOF_DEFAULT_MAX_COC});
                mi->setParameter("texelSize", float2{
                        1.0f / float(colorDesc.width),
                        1.0f / float(colorDesc.height) });
                commitAndRenderFullScreenQuad(driver, out, mi);
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

    auto const& ppDoFMipmap = fg.addPass<PostProcessDofMipmap>("DoF Mipmap",
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
                bindPostProcessDescriptorSet(driver);

                auto desc       = resources.getDescriptor(data.inOutColor);
                auto inOutColor = resources.getTexture(data.inOutColor);
                auto inOutCoc   = resources.getTexture(data.inOutCoc);

                auto const& material = getPostProcessMaterial("dofMipmap");
                FMaterial const* const ma = material.getMaterial(mEngine);
                FMaterialInstance* const mi = PostProcessMaterial::getMaterialInstance(ma);

                auto const pipeline = getPipelineState(ma, variant);

                for (size_t level = 0 ; level < mipmapCount - 1u ; level++) {
                    const float w = FTexture::valueForLevel(level, desc.width);
                    const float h = FTexture::valueForLevel(level, desc.height);
                    auto const& out = resources.getRenderPassInfo(data.rp[level]);
                    auto inColor = driver.createTextureView(inOutColor, level, 1);
                    auto inCoc = driver.createTextureView(inOutCoc, level, 1);
                    mi->setParameter("color", inColor, { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                    mi->setParameter("coc",   inCoc,   { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                    mi->setParameter("weightScale", 0.5f / float(1u << level));   // FIXME: halfres?
                    mi->setParameter("texelSize", float2{ 1.0f / w, 1.0f / h });
                    mi->commit(driver);
                    mi->use(driver);

                    renderFullScreenQuad(out, pipeline, driver);

                    driver.destroyTexture(inColor);
                    driver.destroyTexture(inCoc);
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
    constexpr size_t tileSize = 16;

    // we assume the width/height is already multiple of 16
    assert_invariant(!(colorDesc.width  & 0xF) && !(colorDesc.height & 0xF));
    const uint32_t tileBufferWidth  = width;
    const uint32_t tileBufferHeight = height;
    const size_t tileReductionCount = ctz(tileSize / dofResolution);

    struct PostProcessDofTiling1 {
        FrameGraphId<FrameGraphTexture> inCocMinMax;
        FrameGraphId<FrameGraphTexture> outTilesCocMinMax;
    };

    const bool textureSwizzleSupported = Texture::isTextureSwizzleSupported(mEngine);
    for (size_t i = 0; i < tileReductionCount; i++) {
        auto const& ppDoFTiling = fg.addPass<PostProcessDofTiling1>("DoF Tiling",
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
                    bindPostProcessDescriptorSet(driver);
                    auto const& inputDesc = resources.getDescriptor(data.inCocMinMax);
                    auto const& out = resources.getRenderPassInfo();
                    auto inCocMinMax = resources.getTexture(data.inCocMinMax);
                    auto const& material = (!textureSwizzleSupported && (i == 0)) ?
                            getPostProcessMaterial("dofTilesSwizzle") :
                            getPostProcessMaterial("dofTiles");
                    FMaterialInstance* const mi =
                            PostProcessMaterial::getMaterialInstance(mEngine, material);
                    mi->setParameter("cocMinMax", inCocMinMax, { .filterMin = SamplerMinFilter::NEAREST });
                    mi->setParameter("texelSize", float2{ 1.0f / inputDesc.width, 1.0f / inputDesc.height });
                    commitAndRenderFullScreenQuad(driver, out, mi);
                });
        inTilesCocMinMax = ppDoFTiling->outTilesCocMinMax;
    }

    /*
     * Dilate tiles
     */

    // This is a small helper that does one round of dilate
    auto dilate = [&](FrameGraphId<FrameGraphTexture> const input) -> FrameGraphId<FrameGraphTexture> {

        struct PostProcessDofDilate {
            FrameGraphId<FrameGraphTexture> inTilesCocMinMax;
            FrameGraphId<FrameGraphTexture> outTilesCocMinMax;
        };

        auto const& ppDoFDilate = fg.addPass<PostProcessDofDilate>("DoF Dilate",
                [&](FrameGraph::Builder& builder, auto& data) {
                    auto const& inputDesc = fg.getDescriptor(input);
                    data.inTilesCocMinMax = builder.sample(input);
                    data.outTilesCocMinMax = builder.createTexture("dof dilated tiles output", inputDesc);
                    data.outTilesCocMinMax = builder.declareRenderPass(data.outTilesCocMinMax );
                },
                [=](FrameGraphResources const& resources,
                        auto const& data, DriverApi& driver) {
                    bindPostProcessDescriptorSet(driver);
                    auto const& out = resources.getRenderPassInfo();
                    auto inTilesCocMinMax = resources.getTexture(data.inTilesCocMinMax);
                    auto const& material = getPostProcessMaterial("dofDilate");
                    FMaterialInstance* const mi =
                            PostProcessMaterial::getMaterialInstance(mEngine, material);
                    mi->setParameter("tiles", inTilesCocMinMax, { .filterMin = SamplerMinFilter::NEAREST });
                    commitAndRenderFullScreenQuad(driver, out, mi);
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

    auto const& ppDoF = fg.addPass<PostProcessDof>("DoF",
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
                builder.declareRenderPass("DoF Target", {
                        .attachments = { .color = { data.outColor, data.outAlpha }}
                });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto const& out = resources.getRenderPassInfo();

                auto color          = resources.getTexture(data.color);
                auto coc            = resources.getTexture(data.coc);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& inputDesc = resources.getDescriptor(data.coc);

                auto const& material = getPostProcessMaterial("dof");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);
                // it's not safe to use bilinear filtering in the general case (causes artifacts around edges)
                mi->setParameter("color", color,
                        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("colorLinear", color,
                        { .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST });
                mi->setParameter("coc", coc,
                        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("tiles", tilesCocMinMax,
                        { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("cocToTexelScale", float2{
                        bokehScale.x / (inputDesc.width * dofResolution),
                        bokehScale.y / (inputDesc.height * dofResolution)
                });
                mi->setParameter("cocToPixelScale", (1.0f / float(dofResolution)));
                mi->setParameter("ringCounts", float4{
                    dofOptions.foregroundRingCount ? dofOptions.foregroundRingCount : DOF_DEFAULT_RING_COUNT,
                    dofOptions.backgroundRingCount ? dofOptions.backgroundRingCount : DOF_DEFAULT_RING_COUNT,
                    dofOptions.fastGatherRingCount ? dofOptions.fastGatherRingCount : DOF_DEFAULT_RING_COUNT,
                    0.0 // unused for now
                });
                mi->setParameter("bokehAngle",  bokehAngle);
                commitAndRenderFullScreenQuad(driver, out, mi);
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

    auto const& ppDoFMedian = fg.addPass<PostProcessDofMedian>("DoF Median",
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
                bindPostProcessDescriptorSet(driver);
                auto const& out = resources.getRenderPassInfo();

                auto inColor        = resources.getTexture(data.inColor);
                auto inAlpha        = resources.getTexture(data.inAlpha);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& material = getPostProcessMaterial("dofMedian");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);
                mi->setParameter("dof",   inColor,        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("alpha", inAlpha,        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("tiles", tilesCocMinMax, { .filterMin = SamplerMinFilter::NEAREST });
                commitAndRenderFullScreenQuad(driver, out, mi);
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

    auto const& ppDoFCombine = fg.addPass<PostProcessDofCombine>("DoF combine",
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
                bindPostProcessDescriptorSet(driver);
                auto const& out = resources.getRenderPassInfo();

                auto color      = resources.getTexture(data.color);
                auto dof        = resources.getTexture(data.dof);
                auto alpha      = resources.getTexture(data.alpha);
                auto tilesCocMinMax = resources.getTexture(data.tilesCocMinMax);

                auto const& material = getPostProcessMaterial("dofCombine");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);
                mi->setParameter("color", color, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("dof",   dof,   { .filterMag = SamplerMagFilter::NEAREST });
                mi->setParameter("alpha", alpha, { .filterMag = SamplerMagFilter::NEAREST });
                mi->setParameter("tiles", tilesCocMinMax, { .filterMin = SamplerMinFilter::NEAREST });
                commitAndRenderFullScreenQuad(driver, out, mi);
            });

    return ppDoFCombine->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::downscalePass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const input,
        FrameGraphTexture::Descriptor const& outDesc,
        bool const threshold, float const highlight, bool const fireflies) noexcept {
    struct DownsampleData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };
    auto const& downsamplePass = fg.addPass<DownsampleData>("Downsample",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("Downsample-output", outDesc);
                builder.declareRenderPass(data.output);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto const& out = resources.getRenderPassInfo();
                auto const& material = getPostProcessMaterial("bloomDownsample2x");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);
                mi->setParameter("source", resources.getTexture(data.input), {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                mi->setParameter("level", 0);
                mi->setParameter("threshold", threshold ? 1.0f : 0.0f);
                mi->setParameter("fireflies", fireflies ? 1.0f : 0.0f);
                mi->setParameter("invHighlight", std::isinf(highlight) ? 0.0f : 1.0f / highlight);
                commitAndRenderFullScreenQuad(driver, out, mi);

            });
    return downsamplePass->output;
}

PostProcessManager::BloomPassOutput PostProcessManager::bloom(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, TextureFormat const outFormat,
        BloomOptions& inoutBloomOptions,
        TemporalAntiAliasingOptions const& taaOptions,
        float2 const scale) noexcept {

    // Figure out a good size for the bloom buffer. We must use a fixed bloom buffer size so
    // that the size/strength of the bloom doesn't vary much with the resolution, otherwise
    // dynamic resolution would affect the bloom effect too much.
    auto desc = fg.getDescriptor(input);

    // width and height after dynamic resolution upscaling
    const float aspect = (float(desc.width) * scale.y) / (float(desc.height) * scale.x);

    // FIXME: don't allow inoutBloomOptions.resolution to be larger than input's resolution
    //        (avoid upscale) but how does this affect dynamic resolution
    // FIXME: check what happens on WebGL and intel's processors

    // compute the desired bloom buffer size
    float bloomHeight = float(inoutBloomOptions.resolution);
    float bloomWidth  = bloomHeight * aspect;

    // we might need to adjust the max # of levels
    const uint32_t major = uint32_t(std::max(bloomWidth,  bloomHeight));
    const uint8_t maxLevels = FTexture::maxLevelCount(major);
    inoutBloomOptions.levels = std::min(inoutBloomOptions.levels, maxLevels);
    inoutBloomOptions.levels = std::min(inoutBloomOptions.levels, kMaxBloomLevels);

    if (inoutBloomOptions.quality == QualityLevel::LOW) {
        // In low quality mode, we adjust the bloom buffer size so that both dimensions
        // have enough exact mip levels. This can slightly affect the aspect ratio causing
        // some artifacts:
        // - add some anamorphism (experimentally not visible)
        // - visible bloom size changes with dynamic resolution in non-homogenous mode
        // This allows us to use the 9 sample downsampling filter (instead of 13)
        // for at least 4 levels.
        uint32_t width  = std::max(16u, uint32_t(std::floor(bloomWidth)));
        uint32_t height = std::max(16u, uint32_t(std::floor(bloomHeight)));
        width  &= ~((1 << 4) - 1);  // at least 4 levels
        height &= ~((1 << 4) - 1);
        bloomWidth  = float(width);
        bloomHeight = float(height);
    }

    bool threshold = inoutBloomOptions.threshold;

    // we don't need to do the fireflies reduction if we have TAA (it already does it)
    bool fireflies = threshold && !taaOptions.enabled;

    assert_invariant(bloomWidth && bloomHeight);

    while (2 * bloomWidth < float(desc.width) || 2 * bloomHeight < float(desc.height)) {
        if (inoutBloomOptions.quality == QualityLevel::LOW ||
            inoutBloomOptions.quality == QualityLevel::MEDIUM) {
            input = downscalePass(fg, input, {
                            .width  = (desc.width  = std::max(1u, desc.width  / 2)),
                            .height = (desc.height = std::max(1u, desc.height / 2)),
                            .format = outFormat
                    },
                    threshold, inoutBloomOptions.highlight, fireflies);
            threshold = false; // we do the thresholding only once during down sampling
            fireflies = false; // we do the fireflies reduction only once during down sampling
        } else if (inoutBloomOptions.quality == QualityLevel::HIGH ||
                   inoutBloomOptions.quality == QualityLevel::ULTRA) {
            // In high quality mode, we increase the size of the bloom buffer such that the
            // first scaling is less than 2x, and we increase the number of levels accordingly.
            if (bloomWidth * 2.0f > 2048.0f || bloomHeight * 2.0f > 2048.0f) {
                // but we can't scale above the h/w guaranteed minspec
                break;
            }
            bloomWidth  *= 2.0f;
            bloomHeight *= 2.0f;
            inoutBloomOptions.levels++;
        }
    }

    // convert back to integer width/height
    uint32_t const width  = std::max(1u, uint32_t(std::floor(bloomWidth)));
    uint32_t const height = std::max(1u, uint32_t(std::floor(bloomHeight)));

    input = downscalePass(fg, input,
            { .width = width, .height = height, .format = outFormat },
            threshold, inoutBloomOptions.highlight, fireflies);

    struct BloomPassData {
        FrameGraphId<FrameGraphTexture> out;
        uint32_t outRT[kMaxBloomLevels];
    };

    // Creating a mip-chain poses a "feedback" loop problem on some GPU. We will disable
    // Bloom on these.
    // See: https://github.com/google/filament/issues/2338

    auto const& bloomDownsamplePass = fg.addPass<BloomPassData>("Bloom Downsample",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.out = builder.createTexture("Bloom Out Texture", {
                        .width = width,
                        .height = height,
                        .levels = inoutBloomOptions.levels,
                        .format = outFormat
                });
                data.out = builder.sample(data.out);

                for (size_t i = 0; i < inoutBloomOptions.levels; i++) {
                    auto out = builder.createSubresource(data.out, "Bloom Out Texture mip",
                            { .level = uint8_t(i) });
                    if (i == 0) {
                        // this causes the last blit above to render into this mip
                       fg.forwardResource(out, input);
                    }
                    builder.declareRenderPass(out, &data.outRT[i]);
                }
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

                // TODO: if downsampling is not exactly a multiple of two, use the 13 samples
                //       filter. This is generally the accepted solution, however, the 13 samples
                //       filter is not correct either when we don't sample at integer coordinates,
                //       but it seems ot create less artifacts.
                //       A better solution might be to use the filter described in
                //       Castaño, 2013, "Shadow Mapping Summary Part 1", which is 5x5 filter with
                //       9 samples, but works at all coordinates.

                auto hwOut = resources.getTexture(data.out);

                auto const& material9 = getPostProcessMaterial("bloomDownsample9");
                auto* mi9 = PostProcessMaterial::getMaterialInstance(mEngine, material9);

                auto const& material13 = getPostProcessMaterial("bloomDownsample");
                auto* mi13 = PostProcessMaterial::getMaterialInstance(mEngine, material13);

                for (size_t i = 1; i < inoutBloomOptions.levels; i++) {
                    auto hwDstRT = resources.getRenderPassInfo(data.outRT[i]);
                    hwDstRT.params.flags.discardStart = TargetBufferFlags::COLOR;
                    hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                    // if downsampling is a multiple of 2 in each dimension we can use the
                    // 9 samples filter.
                    auto vp = resources.getRenderPassInfo(data.outRT[i-1]).params.viewport;
                    auto* const mi = (vp.width & 1 || vp.height & 1) ? mi13 : mi9;
                    auto hwOutView = driver.createTextureView(hwOut, i - 1, 1);
                    mi->setParameter("source", hwOutView, {
                            .filterMag = SamplerMagFilter::LINEAR,
                            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST });
                    commitAndRenderFullScreenQuad(driver, hwDstRT, mi);
                    driver.destroyTexture(hwOutView);
                }
            });

    // output of bloom downsample pass becomes input of next (flare) pass
    input = bloomDownsamplePass->out;

    // flare pass
    auto const flare = flarePass(fg, input, width, height, outFormat, inoutBloomOptions);

    auto const& bloomUpsamplePass = fg.addPass<BloomPassData>("Bloom Upsample",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.out = builder.sample(input);
                for (size_t i = 0; i < inoutBloomOptions.levels; i++) {
                    auto out = builder.createSubresource(data.out, "Bloom Out Texture mip",
                            { .level = uint8_t(i) });
                    builder.declareRenderPass(out, &data.outRT[i]);
                }
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto hwOut = resources.getTexture(data.out);
                auto const& outDesc = resources.getDescriptor(data.out);

                auto const& material = getPostProcessMaterial("bloomUpsample");
                FMaterial const* const ma = material.getMaterial(mEngine);
                FMaterialInstance* mi = PostProcessMaterial::getMaterialInstance(ma);

                auto pipeline = getPipelineState(ma);
                pipeline.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
                pipeline.rasterState.blendFunctionDstRGB = BlendFunction::ONE;

                for (size_t j = inoutBloomOptions.levels, i = j - 1; i >= 1; i--, j++) {
                    auto hwDstRT = resources.getRenderPassInfo(data.outRT[i - 1]);
                    hwDstRT.params.flags.discardStart = TargetBufferFlags::NONE; // b/c we'll blend
                    hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;
                    auto w = FTexture::valueForLevel(i - 1, outDesc.width);
                    auto h = FTexture::valueForLevel(i - 1, outDesc.height);
                    auto hwOutView = driver.createTextureView(hwOut, i, 1);
                    mi->setParameter("resolution", float4{ w, h, 1.0f / w, 1.0f / h });
                    mi->setParameter("source", hwOutView, {
                            .filterMag = SamplerMagFilter::LINEAR,
                            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST});
                    mi->commit(driver);
                    mi->use(driver);
                    renderFullScreenQuad(hwDstRT, pipeline, driver);
                    driver.destroyTexture(hwOutView);
                }
            });

    return { bloomUpsamplePass->out, flare };
}

UTILS_NOINLINE
FrameGraphId<FrameGraphTexture> PostProcessManager::flarePass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const input,
        uint32_t const width, uint32_t const height,
        TextureFormat const outFormat,
        BloomOptions const& bloomOptions) noexcept {

    struct FlarePassData {
        FrameGraphId<FrameGraphTexture> in;
        FrameGraphId<FrameGraphTexture> out;
    };
    auto const& flarePass = fg.addPass<FlarePassData>("Flare",
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
                bindPostProcessDescriptorSet(driver);
                auto in = resources.getTexture(data.in);
                auto const out = resources.getRenderPassInfo(0);
                const float aspectRatio = float(width) / float(height);

                auto const& material = getPostProcessMaterial("flare");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);

                mi->setParameter("color", in, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });

                mi->setParameter("level", 0.0f);    // adjust with resolution
                mi->setParameter("aspectRatio", float2{ aspectRatio, 1.0f / aspectRatio });
                mi->setParameter("threshold",
                        float2{ bloomOptions.ghostThreshold, bloomOptions.haloThreshold });
                mi->setParameter("chromaticAberration", bloomOptions.chromaticAberration);
                mi->setParameter("ghostCount", float(bloomOptions.ghostCount));
                mi->setParameter("ghostSpacing", bloomOptions.ghostSpacing);
                mi->setParameter("haloRadius", bloomOptions.haloRadius);
                mi->setParameter("haloThickness", bloomOptions.haloThickness);

                commitAndRenderFullScreenQuad(driver, out, mi);
            });

    constexpr float kernelWidth = 9;
    constexpr float sigma = (kernelWidth + 1.0f) / 6.0f;
    auto const flare = gaussianBlurPass(fg, flarePass->out, {}, false, kernelWidth, sigma);
    return flare;
}

UTILS_NOINLINE
static float4 getVignetteParameters(VignetteOptions const& options,
        uint32_t const width, uint32_t const height) noexcept {
    if (options.enabled) {
        // Vignette params
        // From 0.0 to 0.5 the vignette is a rounded rect that turns into an oval
        // From 0.5 to 1.0 the vignette turns from oval to circle
        float const oval = min(options.roundness, 0.5f) * 2.0f;
        float const circle = (max(options.roundness, 0.5f) - 0.5f) * 2.0f;
        float const roundness = (1.0f - oval) * 6.0f + oval;

        // Mid point varies during the oval/rounded section of roundness
        // We also modify it to emphasize feathering
        float const midPoint = (1.0f - options.midPoint) * mix(2.2f, 3.0f, oval)
                         * (1.0f - 0.1f * options.feather);

        // Radius of the rounded corners as a param to pow()
        float const radius = roundness *
                mix(1.0f + 4.0f * (1.0f - options.feather), 1.0f, std::sqrt(oval));

        // Factor to transform oval into circle
        float const aspect = mix(1.0f, float(width) / float(height), circle);

        return float4{ midPoint, radius, aspect, options.feather };
    }

    // Set half-max to show disabled
    return float4{ std::numeric_limits<half>::max() };
}

void PostProcessManager::colorGradingPrepareSubpass(DriverApi& driver,
        const FColorGrading* colorGrading, ColorGradingConfig const& colorGradingConfig,
        VignetteOptions const& vignetteOptions, uint32_t const width, uint32_t const height) noexcept {

    auto& material = getPostProcessMaterial("colorGradingAsSubpass");
    FMaterialInstance* const mi =
            configureColorGradingMaterial(material, colorGrading, colorGradingConfig,
                    vignetteOptions, width, height);
    mi->commit(driver);
}

void PostProcessManager::colorGradingSubpass(DriverApi& driver,
        ColorGradingConfig const& colorGradingConfig) noexcept {

    bindPostProcessDescriptorSet(driver);

    PostProcessVariant const variant = colorGradingConfig.translucent ?
            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE;

    auto const& material = getPostProcessMaterial("colorGradingAsSubpass");
    FMaterial const* const ma = material.getMaterial(mEngine, variant);
    // the UBO has been set and committed in colorGradingPrepareSubpass()
    FMaterialInstance const* mi = PostProcessMaterial::getMaterialInstance(ma);
    mi->use(driver);
    auto const pipeline = getPipelineState(ma, variant);
    driver.nextSubpass();
    driver.scissor(mi->getScissor());
    driver.draw(pipeline, mFullScreenQuadRph, 0, 3, 1);
}

void PostProcessManager::customResolvePrepareSubpass(DriverApi& driver, CustomResolveOp const op) noexcept {
    auto const& material = getPostProcessMaterial("customResolveAsSubpass");
    FMaterialInstance* const mi = PostProcessMaterial::getMaterialInstance(mEngine, material);
    mi->setParameter("direction", op == CustomResolveOp::COMPRESS ? 1.0f : -1.0f),
    mi->commit(driver);
    material.getMaterial(mEngine);
}

void PostProcessManager::customResolveSubpass(DriverApi& driver) noexcept {

    bindPostProcessDescriptorSet(driver);

    FEngine const& engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();
    auto const& material = getPostProcessMaterial("customResolveAsSubpass");
    FMaterial const* const ma = material.getMaterial(mEngine);
    // the UBO has been set and committed in colorGradingPrepareSubpass()
    FMaterialInstance const* mi = PostProcessMaterial::getMaterialInstance(ma);
    mi->use(driver);
    auto const pipeline = getPipelineState(ma);
    driver.nextSubpass();
    driver.scissor(mi->getScissor());
    driver.draw(pipeline, fullScreenRenderPrimitive, 0, 3, 1);
}

FrameGraphId<FrameGraphTexture> PostProcessManager::customResolveUncompressPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const inout) noexcept {
    struct UncompressData {
        FrameGraphId<FrameGraphTexture> inout;
    };
    auto const& detonemapPass = fg.addPass<UncompressData>("Uncompress Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.inout = builder.read(inout, FrameGraphTexture::Usage::SUBPASS_INPUT);
                data.inout = builder.write(data.inout, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass("Uncompress target", {
                        .attachments = { .color = { data.inout }}
                });
            },
            [=](FrameGraphResources const& resources, auto const&, DriverApi& driver) {
                customResolvePrepareSubpass(driver, CustomResolveOp::UNCOMPRESS);
                auto out = resources.getRenderPassInfo();
                out.params.subpassMask = 1;
                bindPostProcessDescriptorSet(driver);
                driver.beginRenderPass(out.target, out.params);
                customResolveSubpass(driver);
                driver.endRenderPass();
            });
    return detonemapPass->inout;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::colorGrading(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const input, filament::Viewport const& vp,
        FrameGraphId<FrameGraphTexture> const bloom,
        FrameGraphId<FrameGraphTexture> const flare,
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
            FTexture const* fdirt = downcast(bloomOptions.dirt);
            FrameGraphTexture const frameGraphTexture{ .handle = fdirt->getHwHandleForSampling() };
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

    auto const& ppColorGrading = fg.addPass<PostProcessColorGrading>("colorGrading",
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
                bindPostProcessDescriptorSet(driver);
                auto colorTexture = resources.getTexture(data.input);

                auto bloomTexture =
                        data.bloom ? resources.getTexture(data.bloom) : getZeroTexture();

                auto flareTexture =
                        data.flare ? resources.getTexture(data.flare) : getZeroTexture();

                auto dirtTexture =
                        data.dirt ? resources.getTexture(data.dirt) : getOneTexture();

                auto starburstTexture =
                        data.starburst ? resources.getTexture(data.starburst) : getOneTexture();

                auto const& out = resources.getRenderPassInfo();

                auto const& input = resources.getDescriptor(data.input);
                auto const& output = resources.getDescriptor(data.output);

                auto& material = getPostProcessMaterial("colorGrading");
                FMaterialInstance* const mi =
                        configureColorGradingMaterial(material, colorGrading, colorGradingConfig,
                                vignetteOptions, output.width, output.height);

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
                if (bloomOptions.blendMode == BloomOptions::BlendMode::INTERPOLATE) {
                    bloomParameters.y = 1.0f - bloomParameters.x;
                }

                mi->setParameter("bloom", bloomParameters);
                mi->setParameter("viewport", float4{
                        float(vp.left)   / input.width,
                        float(vp.bottom) / input.height,
                        float(vp.width)  / input.width,
                        float(vp.height) / input.height
                });

                commitAndRenderFullScreenQuad(driver, out, mi,
                        colorGradingConfig.translucent
                        ? PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);
            }
    );

    return ppColorGrading->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::fxaa(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const input, filament::Viewport const& vp,
        TextureFormat const outFormat, bool const preserveAlphaChannel) noexcept {

    struct PostProcessFXAA {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto const& ppFXAA = fg.addPass<PostProcessFXAA>("fxaa",
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
                bindPostProcessDescriptorSet(driver);
                auto const& inDesc = resources.getDescriptor(data.input);
                auto const& texture = resources.getTexture(data.input);
                auto const& out = resources.getRenderPassInfo();

                auto const& material = getPostProcessMaterial("fxaa");

                PostProcessVariant const variant = preserveAlphaChannel ?
                        PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE;

                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material, variant);

                mi->setParameter("colorBuffer", texture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                mi->setParameter("viewport", float4{
                        float(vp.left)   / inDesc.width,
                        float(vp.bottom) / inDesc.height,
                        float(vp.width)  / inDesc.width,
                        float(vp.height) / inDesc.height
                });
                mi->setParameter("texelSize", 1.0f / float2{ inDesc.width, inDesc.height });

                commitAndRenderFullScreenQuad(driver, out, mi, variant);
            });

    return ppFXAA->output;
}

void PostProcessManager::TaaJitterCamera(
        filament::Viewport const& svp,
        TemporalAntiAliasingOptions const& taaOptions,
        FrameHistory& frameHistory,
        FrameHistoryEntry::TemporalAA FrameHistoryEntry::*pTaa,
        CameraInfo* inoutCameraInfo) const noexcept {
    auto const& previous = frameHistory.getPrevious().*pTaa;
    auto& current = frameHistory.getCurrent().*pTaa;

    // compute projection
    current.projection = inoutCameraInfo->projection * inoutCameraInfo->getUserViewMatrix();
    current.frameId = previous.frameId + 1;

    auto jitterPosition = [pattern = taaOptions.jitterPattern](size_t const frameIndex) -> float2 {
        using JitterPattern = TemporalAntiAliasingOptions::JitterPattern;
        switch (pattern) {
            case JitterPattern::RGSS_X4:
                return sRGSS4(frameIndex);
            case JitterPattern::UNIFORM_HELIX_X4:
                return sUniformHelix4(frameIndex);
            case JitterPattern::HALTON_23_X8:
                return sHaltonSamples(frameIndex % 8);
            case JitterPattern::HALTON_23_X16:
                return sHaltonSamples(frameIndex % 16);
            case JitterPattern::HALTON_23_X32:
                return sHaltonSamples(frameIndex);
        }
        return { 0.0f, 0.0f };
    };

    // sample position within a pixel [-0.5, 0.5]
    // for metal/vulkan we need to reverse the y-offset
    current.jitter = jitterPosition(previous.frameId);
    float2 jitter = current.jitter;
    switch (mEngine.getBackend()) {
        case Backend::METAL:
        case Backend::VULKAN:
        case Backend::WEBGPU:
            jitter.y = -jitter.y;
            UTILS_FALLTHROUGH;
        case Backend::OPENGL:
        default:
            break;
    }

    float2 const jitterInClipSpace = jitter * (2.0f / float2{ svp.width, svp.height });

    // update projection matrix
    inoutCameraInfo->projection[2].xy -= jitterInClipSpace;
    // VERTEX_DOMAIN_DEVICE doesn't apply the projection, but it still needs this
    // clip transform, so we apply it separately (see surface_main.vs)
    inoutCameraInfo->clipTransform.zw -= jitterInClipSpace;
}

void PostProcessManager::configureTemporalAntiAliasingMaterial(
        TemporalAntiAliasingOptions const& taaOptions) noexcept {

    FMaterial* const ma = getPostProcessMaterial("taa").getMaterial(mEngine);
    bool dirty = false;

    setConstantParameter(ma, "upscaling", taaOptions.upscaling, dirty);
    setConstantParameter(ma, "historyReprojection", taaOptions.historyReprojection, dirty);
    setConstantParameter(ma, "filterHistory", taaOptions.filterHistory, dirty);
    setConstantParameter(ma, "filterInput", taaOptions.filterInput, dirty);
    setConstantParameter(ma, "useYCoCg", taaOptions.useYCoCg, dirty);
    setConstantParameter(ma, "preventFlickering", taaOptions.preventFlickering, dirty);
    setConstantParameter(ma, "boxType", int32_t(taaOptions.boxType), dirty);
    setConstantParameter(ma, "boxClipping", int32_t(taaOptions.boxClipping), dirty);
    setConstantParameter(ma, "varianceGamma", taaOptions.varianceGamma, dirty);
    if (dirty) {
        ma->invalidate();
        // TODO: call Material::compile(), we can't do that now because it works only
        //       with surface materials
    }
}

FMaterialInstance* PostProcessManager::configureColorGradingMaterial(
        PostProcessMaterial& material, FColorGrading const* colorGrading,
        ColorGradingConfig const& colorGradingConfig, VignetteOptions const& vignetteOptions,
        uint32_t const width, uint32_t const height) noexcept {
    FMaterial* const ma = material.getMaterial(mEngine);
    bool dirty = false;

    setConstantParameter(ma, "isOneDimensional", colorGrading->isOneDimensional(), dirty);
    setConstantParameter(ma, "isLDR", colorGrading->isLDR(), dirty);

    if (dirty) {
        ma->invalidate();
        // TODO: call Material::compile(), we can't do that now because it works only
        //       with surface materials
    }

    PostProcessVariant const variant = colorGradingConfig.translucent ?
            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE;
    FMaterialInstance* const mi =
            PostProcessMaterial::getMaterialInstance(mEngine, material, variant);

    const SamplerParams params = {
            .filterMag = SamplerMagFilter::LINEAR,
            .filterMin = SamplerMinFilter::LINEAR,
            .wrapS = SamplerWrapMode::CLAMP_TO_EDGE,
            .wrapT = SamplerWrapMode::CLAMP_TO_EDGE,
            .wrapR = SamplerWrapMode::CLAMP_TO_EDGE,
            .anisotropyLog2 = 0
    };

    mi->setParameter("lut", colorGrading->getHwHandle(), params);

    const float lutDimension = float(colorGrading->getDimension());
    mi->setParameter("lutSize", float2{
        0.5f / lutDimension, (lutDimension - 1.0f) / lutDimension,
    });

    const float temporalNoise = mUniformDistribution(mEngine.getRandomEngine());

    float4 const vignetteParameters = getVignetteParameters(vignetteOptions, width, height);
    mi->setParameter("vignette", vignetteParameters);
    mi->setParameter("vignetteColor", vignetteOptions.color);
    mi->setParameter("dithering", colorGradingConfig.dithering);
    mi->setParameter("outputLuminance", colorGradingConfig.outputLuminance);
    mi->setParameter("temporalNoise", temporalNoise);

    return mi;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::taa(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphId<FrameGraphTexture> const depth,
        FrameHistory& frameHistory,
        FrameHistoryEntry::TemporalAA FrameHistoryEntry::*pTaa,
        TemporalAntiAliasingOptions const& taaOptions,
        ColorGradingConfig const& colorGradingConfig) noexcept {
    assert_invariant(depth);

    auto const& previous = frameHistory.getPrevious().*pTaa;
    auto& current = frameHistory.getCurrent().*pTaa;

    // if we don't have a history yet, just use the current color buffer as history
    FrameGraphId<FrameGraphTexture> colorHistory = input;
    if (UTILS_LIKELY(previous.color.handle)) {
        colorHistory = fg.import("TAA history", previous.desc,
                FrameGraphTexture::Usage::SAMPLEABLE, previous.color);
    }

    mat4 const& historyProjection = previous.color.handle ?
            previous.projection : current.projection;

    struct TAAData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> history;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> tonemappedOutput;
    };
    auto const& taaPass = fg.addPass<TAAData>("TAA",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto desc = fg.getDescriptor(input);
                if (taaOptions.upscaling) {
                    desc.width *= 2;
                    desc.height *= 2;
                }
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
            [=, &current](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

                constexpr mat4f normalizedToClip{mat4f::row_major_init{
                        2, 0, 0, -1,
                        0, 2, 0, -1,
                        0, 0, 1,  0,
                        0, 0, 0,  1
                }};

                constexpr float2 sampleOffsets[9] = {
                        { -1.0f, -1.0f }, {  0.0f, -1.0f }, {  1.0f, -1.0f }, { -1.0f,  0.0f },
                        {  0.0f,  0.0f },
                        {  1.0f,  0.0f }, { -1.0f,  1.0f }, {  0.0f,  1.0f }, {  1.0f,  1.0f },
                };

                constexpr float2 subSampleOffsets[4] = {
                        { -0.25f,  0.25f },
                        {  0.25f,  0.25f },
                        {  0.25f, -0.25f },
                        { -0.25f, -0.25f }
                };

                UTILS_UNUSED
                auto const lanczos = [](float const x, float const a) -> float {
                    if (x <= std::numeric_limits<float>::epsilon()) {
                        return 1.0f;
                    }
                    if (std::abs(x) <= a) {
                        return (a * std::sin(f::PI * x) * std::sin(f::PI * x / a))
                               / ((f::PI * f::PI) * (x * x));
                    }
                    return 0.0f;
                };

                float const filterWidth = std::clamp(taaOptions.filterWidth, 1.0f, 2.0f);
                float4 sum = 0.0;
                float4 weights[9];

                // this doesn't get vectorized (probably because of exp()), so don't bother
                // unrolling it.
                UTILS_NOUNROLL
                for (size_t i = 0; i < 9; i++) {
                    float2 const o = sampleOffsets[i];
                    for (size_t j = 0; j < 4; j++) {
                        float2 const subPixelOffset = taaOptions.upscaling ? subSampleOffsets[j] : float2{ 0 };
                        float2 const d = (o - (current.jitter - subPixelOffset)) / filterWidth;
                        weights[i][j] = lanczos(length(d), filterWidth);
                    }
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

                PostProcessVariant const variant = colorGradingConfig.translucent ?
                        PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE;

                FMaterial const* const ma = material.getMaterial(mEngine, variant);

                FMaterialInstance* mi = PostProcessMaterial::getMaterialInstance(ma);
                mi->setParameter("color",  color, {});  // nearest
                mi->setParameter("depth",  depth, {});  // nearest
                mi->setParameter("alpha", taaOptions.feedback);
                mi->setParameter("history", history, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                mi->setParameter("filterWeights",  weights, 9);
                mi->setParameter("jitter",  current.jitter);
                mi->setParameter("reprojection",
                        mat4f{ historyProjection * inverse(current.projection) } *
                        normalizedToClip);

                mi->commit(driver);
                mi->use(driver);

                if (colorGradingConfig.asSubpass) {
                    out.params.subpassMask = 1;
                }
                auto const pipeline = getPipelineState(ma, variant);

                driver.beginRenderPass(out.target, out.params);
                driver.draw(pipeline, mFullScreenQuadRph, 0, 3, 1);
                if (colorGradingConfig.asSubpass) {
                    colorGradingSubpass(driver, colorGradingConfig);
                }
                driver.endRenderPass();
            });

    input = colorGradingConfig.asSubpass ? taaPass->tonemappedOutput : taaPass->output;
    auto const history = input;

    // optional sharpen pass from FSR1
    if (taaOptions.sharpness > 0.0f) {
        input = rcas(fg, taaOptions.sharpness,
                input, fg.getDescriptor(input), colorGradingConfig.translucent);
    }

    struct ExportColorHistoryData {
        FrameGraphId<FrameGraphTexture> color;
    };
    fg.addPass<ExportColorHistoryData>("Export TAA history",
            [&](FrameGraph::Builder& builder, auto& data) {
                // We need to use sideEffect here to ensure this pass won't be culled.
                // The "output" of this pass is going to be used during the next frame as
                // an "import".
                builder.sideEffect();
                data.color = builder.sample(history); // FIXME: an access must be declared for detach(), why?
            }, [&current](FrameGraphResources const& resources, auto const& data, auto&) {
                resources.detach(data.color, &current.color, &current.desc);
            });

    return input;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::rcas(
        FrameGraph& fg,
        float const sharpness,
        FrameGraphId<FrameGraphTexture> const input,
        FrameGraphTexture::Descriptor const& outDesc,
        bool const translucent) {

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto const& ppFsrRcas = fg.addPass<QuadBlitData>("FidelityFX FSR1 Rcas",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("FFX FSR1 Rcas output", outDesc);
                data.output = builder.declareRenderPass(data.output);
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

                auto const input = resources.getTexture(data.input);
                auto const out = resources.getRenderPassInfo();
                auto const& outputDesc = resources.getDescriptor(data.input);

                PostProcessVariant const variant = translucent ?
                        PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE;

                auto const& material = getPostProcessMaterial("fsr_rcas");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material, variant);

                FSRUniforms uniforms;
                FSR_SharpeningSetup(&uniforms, { .sharpness = 2.0f - 2.0f * sharpness });
                mi->setParameter("RcasCon", uniforms.RcasCon);
                mi->setParameter("color", input, {}); // uses texelFetch
                mi->setParameter("resolution", float4{
                        outputDesc.width, outputDesc.height,
                        1.0f / outputDesc.width, 1.0f / outputDesc.height });
                commitAndRenderFullScreenQuad(driver, out, mi, variant);
            });

    return ppFsrRcas->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::upscale(FrameGraph& fg, bool translucent,
    bool sourceHasLuminance, DynamicResolutionOptions dsrOptions,
    FrameGraphId<FrameGraphTexture> const input, filament::Viewport const& vp,
    FrameGraphTexture::Descriptor const& outDesc, SamplerMagFilter filter) noexcept {
    // The code below cannot handle sub-resources
    assert_invariant(fg.getSubResourceDescriptor(input).layer == 0);
    assert_invariant(fg.getSubResourceDescriptor(input).level == 0);

    const bool lowQualityFallback = translucent;
    if (lowQualityFallback) {
        // FidelityFX-FSR nor SGSR support the source alpha channel currently
        dsrOptions.quality = QualityLevel::LOW;
    }

    if (dsrOptions.quality == QualityLevel::LOW) {
        return upscaleBilinear(fg, dsrOptions, input, vp, outDesc, filter);
    }
    if (dsrOptions.quality == QualityLevel::MEDIUM) {
        return upscaleSGSR1(fg, sourceHasLuminance, dsrOptions, input, vp, outDesc);
    }
    return upscaleFSR1(fg, dsrOptions, input, vp, outDesc);
}

FrameGraphId<FrameGraphTexture> PostProcessManager::upscaleBilinear(FrameGraph& fg,
        DynamicResolutionOptions dsrOptions, FrameGraphId<FrameGraphTexture> const input,
        filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc,
        SamplerMagFilter filter) noexcept {

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto const& ppQuadBlit = fg.addPass<QuadBlitData>(dsrOptions.enabled ? "upscaling" : "compositing",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("upscaled output", outDesc);
                data.output = builder.write(data.output, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass(builder.getName(data.output), {
                        .attachments = { .color = { data.output } },
                        .clearFlags = TargetBufferFlags::DEPTH });
            },
            [this, vp, filter](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

                auto color = resources.getTexture(data.input);
                auto const& inputDesc = resources.getDescriptor(data.input);

                // --------------------------------------------------------------------------------
                // set uniforms

                auto& material = getPostProcessMaterial("blitLow");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);
                mi->setParameter("color", color, {
                    .filterMag = filter
                });

                mi->setParameter("levelOfDetail", 0.0f);

                mi->setParameter("viewport", float4{
                        float(vp.left)   / inputDesc.width,
                        float(vp.bottom) / inputDesc.height,
                        float(vp.width)  / inputDesc.width,
                        float(vp.height) / inputDesc.height
                });
                mi->commit(driver);
                mi->use(driver);

                auto out = resources.getRenderPassInfo();

                auto pipeline = getPipelineState(material.getMaterial(mEngine));
                renderFullScreenQuad(out, pipeline, driver);
            });

    auto output = ppQuadBlit->output;

    // if we had to take the low quality fallback, we still do the "sharpen pass"
    if (dsrOptions.sharpness > 0.0f) {
        output = rcas(fg, dsrOptions.sharpness, output, outDesc,
                false /* translucent=false, because dst buffer is allocated */);
    }

    // we rely on automatic culling of unused render passes
    return output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::upscaleSGSR1(FrameGraph& fg, bool sourceHasLuminance,
    DynamicResolutionOptions dsrOptions, FrameGraphId<FrameGraphTexture> const input,
    filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc) noexcept {

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto const& ppQuadBlit = fg.addPass<QuadBlitData>(dsrOptions.enabled ? "upscaling" : "compositing",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("upscaled output", outDesc);
                data.output = builder.write(data.output, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass(builder.getName(data.output), {
                        .attachments = { .color = { data.output } },
                        .clearFlags = TargetBufferFlags::DEPTH });
            },
            [this, vp, sourceHasLuminance](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

                auto color = resources.getTexture(data.input);
                auto const& inputDesc = resources.getDescriptor(data.input);

                // --------------------------------------------------------------------------------
                // set uniforms

                auto const& material = getPostProcessMaterial("sgsr1");

                PostProcessVariant const variant = sourceHasLuminance ?
                        PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE;

                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material, variant);

                mi->setParameter("color", color, {
                    // The SGSR documentation doesn't clarify if LINEAR or NEAREST should be used. The
                    // sample code uses NEAREST, but that doesn't seem right, since it would mean the
                    // LERP mode would not be a LERP, and the non-edges would be sampled as NEAREST.
                    .filterMag = SamplerMagFilter::LINEAR
                });

                mi->setParameter("viewport", float4{
                        float(vp.left)   / inputDesc.width,
                        float(vp.bottom) / inputDesc.height,
                        float(vp.width)  / inputDesc.width,
                        float(vp.height) / inputDesc.height
                });

                mi->setParameter("viewportInfo", float4{
                        1.0f / inputDesc.width,
                        1.0f / inputDesc.height,
                        float(inputDesc.width),
                        float(inputDesc.height)
                });

                mi->commit(driver);
                mi->use(driver);

                auto out = resources.getRenderPassInfo();
                commitAndRenderFullScreenQuad(driver, out, mi, variant);

            });

    auto output = ppQuadBlit->output;

    // we rely on automatic culling of unused render passes
    return output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::upscaleFSR1(FrameGraph& fg,
    DynamicResolutionOptions dsrOptions, FrameGraphId<FrameGraphTexture> const input,
    filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc) noexcept {

    const bool twoPassesEASU = mWorkaroundSplitEasu &&
            (dsrOptions.quality == QualityLevel::MEDIUM
                || dsrOptions.quality == QualityLevel::HIGH);

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> depth;
    };

    auto const& ppQuadBlit = fg.addPass<QuadBlitData>(dsrOptions.enabled ? "upscaling" : "compositing",
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
                builder.declareRenderPass(builder.getName(data.output), {
                        .attachments = { .color = { data.output }, .depth = { data.depth }},
                        .clearFlags = TargetBufferFlags::DEPTH });
            },
            [this, twoPassesEASU, dsrOptions, vp](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

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

                auto color = resources.getTexture(data.input);
                auto const& inputDesc = resources.getDescriptor(data.input);
                auto const& outputDesc = resources.getDescriptor(data.output);

                // --------------------------------------------------------------------------------
                // set uniforms

                PostProcessMaterial* splitEasuMaterial = nullptr;
                PostProcessMaterial* easuMaterial = nullptr;

                if (twoPassesEASU) {
                    splitEasuMaterial = &getPostProcessMaterial("fsr_easu_mobileF");
                    FMaterialInstance* const mi =
                            PostProcessMaterial::getMaterialInstance(mEngine, *splitEasuMaterial);
                    setEasuUniforms(mi, inputDesc, outputDesc);
                    mi->setParameter("color", color, {
                        .filterMag = SamplerMagFilter::LINEAR
                    });
                    mi->setParameter("resolution",
                            float4{ outputDesc.width, outputDesc.height,
                                    1.0f / outputDesc.width, 1.0f / outputDesc.height });
                    mi->commit(driver);
                    mi->use(driver);
                }

                { // just a scope to not leak local variables
                    const std::string_view blitterNames[2] = { "fsr_easu_mobile", "fsr_easu" };
                    unsigned const index = std::min(1u, unsigned(dsrOptions.quality) - 2);
                    easuMaterial = &getPostProcessMaterial(blitterNames[index]);
                    FMaterialInstance* const mi =
                            PostProcessMaterial::getMaterialInstance(mEngine, *easuMaterial);

                    setEasuUniforms(mi, inputDesc, outputDesc);

                    mi->setParameter("color", color, {
                        .filterMag = SamplerMagFilter::LINEAR
                    });

                    mi->setParameter("resolution",
                            float4{outputDesc.width, outputDesc.height, 1.0f / outputDesc.width,
                                1.0f / outputDesc.height});

                    mi->setParameter("viewport", float4{
                            float(vp.left)   / inputDesc.width,
                            float(vp.bottom) / inputDesc.height,
                            float(vp.width)  / inputDesc.width,
                            float(vp.height) / inputDesc.height
                    });
                    mi->commit(driver);
                    mi->use(driver);
                }

                // --------------------------------------------------------------------------------
                // render pass with draw calls

                auto out = resources.getRenderPassInfo();

                if (UTILS_UNLIKELY(twoPassesEASU)) {
                    auto pipeline0 = getPipelineState(splitEasuMaterial->getMaterial(mEngine));
                    auto pipeline1 = getPipelineState(easuMaterial->getMaterial(mEngine));
                    pipeline1.rasterState.depthFunc = SamplerCompareFunc::NE;
                    driver.beginRenderPass(out.target, out.params);
                    driver.draw(pipeline0, mFullScreenQuadRph, 0, 3, 1);
                    driver.draw(pipeline1, mFullScreenQuadRph, 0, 3, 1);
                    driver.endRenderPass();
                } else {
                    auto pipeline = getPipelineState(easuMaterial->getMaterial(mEngine));
                    renderFullScreenQuad(out, pipeline, driver);
                }
            });

    auto output = ppQuadBlit->output;
    if (dsrOptions.sharpness > 0.0f) {
        output = rcas(fg, dsrOptions.sharpness, output, outDesc, false);
    }

    // we rely on automatic culling of unused render passes
    return output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::blit(FrameGraph& fg, bool const translucent,
        FrameGraphId<FrameGraphTexture> const input,
        filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc,
        SamplerMagFilter filterMag,
        SamplerMinFilter filterMin) noexcept {

    uint32_t const layer = fg.getSubResourceDescriptor(input).layer;
    float const levelOfDetail = fg.getSubResourceDescriptor(input).level;

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto const& ppQuadBlit = fg.addPass<QuadBlitData>("blitting",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("blit output", outDesc);
                data.output = builder.write(data.output,
                        FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass(builder.getName(data.output), {
                        .attachments = { .color = { data.output }},
                        .clearFlags = TargetBufferFlags::DEPTH });
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto color = resources.getTexture(data.input);
                auto const& inputDesc = resources.getDescriptor(data.input);
                auto out = resources.getRenderPassInfo();

                // --------------------------------------------------------------------------------
                // set uniforms

                PostProcessMaterial const& material =
                        getPostProcessMaterial(layer ? "blitArray" : "blitLow");
                FMaterial const* const ma = material.getMaterial(mEngine);
                auto* mi = PostProcessMaterial::getMaterialInstance(ma);
                mi->setParameter("color", color, {
                        .filterMag = filterMag,
                        .filterMin = filterMin
                });
                mi->setParameter("viewport", float4{
                        float(vp.left)   / inputDesc.width,
                        float(vp.bottom) / inputDesc.height,
                        float(vp.width)  / inputDesc.width,
                        float(vp.height) / inputDesc.height
                });
                mi->setParameter("levelOfDetail", levelOfDetail);
                if (layer) {
                    mi->setParameter("layerIndex", layer);
                }
                mi->commit(driver);
                mi->use(driver);

                auto pipeline = getPipelineState(ma);
                if (translucent) {
                    pipeline.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionDstRGB = BlendFunction::ONE_MINUS_SRC_ALPHA;
                    pipeline.rasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
                }
                renderFullScreenQuad(out, pipeline, driver);
            });

    return ppQuadBlit->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::blitDepth(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const input) noexcept {
    auto const& inputDesc = fg.getDescriptor(input);
    filament::Viewport const vp = {0, 0, inputDesc.width, inputDesc.height};
    bool const hardwareBlitSupported =
            mEngine.getDriverApi().isDepthStencilBlitSupported(inputDesc.format);

    struct BlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    if (hardwareBlitSupported) {
        auto const& depthPass = fg.addPass<BlitData>(
                "Depth Blit",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.input = builder.read(input, FrameGraphTexture::Usage::BLIT_SRC);

                    auto desc = builder.getDescriptor(data.input);
                    desc.levels = 1;// only copy the base level

                    // create a new buffer for the copy
                    data.output = builder.createTexture("depth blit output", desc);

                    // output is an attachment
                    data.output = builder.write(data.output, FrameGraphTexture::Usage::BLIT_DST);
                },
                [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                    auto const& src = resources.getTexture(data.input);
                    auto const& dst = resources.getTexture(data.output);
                    auto const& srcSubDesc = resources.getSubResourceDescriptor(data.input);
                    auto const& dstSubDesc = resources.getSubResourceDescriptor(data.output);
                    auto const& desc = resources.getDescriptor(data.output);
                    assert_invariant(desc.samples == resources.getDescriptor(data.input).samples);
                    // here we can guarantee that src and dst format and size match, by
                    // construction.
                    driver.blit(
                            dst, dstSubDesc.level, dstSubDesc.layer, { 0, 0 },
                            src, srcSubDesc.level, srcSubDesc.layer, { 0, 0 },
                            { desc.width, desc.height });
                });
        return depthPass->output;
    }
    // Otherwise, we would do a shader-based blit.

    auto const& ppQuadBlit = fg.addPass<BlitData>(
            "Depth Blit (Shader)",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                // Note that this is a same size/format blit.
                auto const& outputDesc = inputDesc;
                data.output = builder.createTexture("depth blit output", outputDesc);
                data.output =
                        builder.write(data.output, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                builder.declareRenderPass(builder.getName(data.output),
                        {.attachments = {.depth = {data.output}}});
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto depth = resources.getTexture(data.input);
                auto const& inputDesc = resources.getDescriptor(data.input);
                auto const out = resources.getRenderPassInfo();

                // --------------------------------------------------------------------------------
                // set uniforms
                PostProcessMaterial const& material = getPostProcessMaterial("blitDepth");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);
                mi->setParameter("depth", depth,
                        {
                            .filterMag = SamplerMagFilter::NEAREST,
                            .filterMin = SamplerMinFilter::NEAREST,
                        });
                mi->setParameter("viewport",
                        float4{float(vp.left) / inputDesc.width,
                            float(vp.bottom) / inputDesc.height, float(vp.width) / inputDesc.width,
                            float(vp.height) / inputDesc.height});
                commitAndRenderFullScreenQuad(driver, out, mi);
            });

    return ppQuadBlit->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::resolve(FrameGraph& fg,
        const char* outputBufferName, FrameGraphId<FrameGraphTexture> const input,
        FrameGraphTexture::Descriptor outDesc) noexcept {

    // Don't do anything if we're not a MSAA buffer
    auto const& inDesc = fg.getDescriptor(input);
    if (inDesc.samples <= 1) {
        return input;
    }

    // The Metal / Vulkan backends currently don't support depth/stencil resolve.
    if (isDepthFormat(inDesc.format) && (!mEngine.getDriverApi().isDepthStencilResolveSupported())) {
        return resolveDepth(fg, outputBufferName, input, outDesc);
    }

    outDesc.width = inDesc.width;
    outDesc.height = inDesc.height;
    outDesc.format = inDesc.format;
    outDesc.samples = 0;

    struct ResolveData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto const& ppResolve = fg.addPass<ResolveData>("resolve",
            [&](FrameGraph::Builder& builder, auto& data) {
                // we currently don't support stencil resolve.
                assert_invariant(!isStencilFormat(inDesc.format));

                data.input = builder.read(input, FrameGraphTexture::Usage::BLIT_SRC);
                data.output = builder.createTexture(outputBufferName, outDesc);
                data.output = builder.write(data.output, FrameGraphTexture::Usage::BLIT_DST);
            },
            [](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                auto const& src = resources.getTexture(data.input);
                auto const& dst = resources.getTexture(data.output);
                auto const& srcSubDesc = resources.getSubResourceDescriptor(data.input);
                auto const& dstSubDesc = resources.getSubResourceDescriptor(data.output);
                UTILS_UNUSED_IN_RELEASE auto const& srcDesc = resources.getDescriptor(data.input);
                UTILS_UNUSED_IN_RELEASE auto const& dstDesc = resources.getDescriptor(data.output);
                assert_invariant(src);
                assert_invariant(dst);
                assert_invariant(srcDesc.format == dstDesc.format);
                assert_invariant(srcDesc.width == dstDesc.width && srcDesc.height == dstDesc.height);
                assert_invariant(srcDesc.samples > 1 && dstDesc.samples <= 1);
                driver.resolve(
                        dst, dstSubDesc.level, dstSubDesc.layer,
                        src, srcSubDesc.level, srcSubDesc.layer);
            });

    return ppResolve->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::resolveDepth(FrameGraph& fg,
        const char* outputBufferName, FrameGraphId<FrameGraphTexture> const input,
        FrameGraphTexture::Descriptor outDesc) noexcept {

    // Don't do anything if we're not a MSAA buffer
    auto const& inDesc = fg.getDescriptor(input);
    if (inDesc.samples <= 1) {
        return input;
    }

    UTILS_UNUSED_IN_RELEASE auto const& inSubDesc = fg.getSubResourceDescriptor(input);
    assert_invariant(isDepthFormat(inDesc.format));
    assert_invariant(inSubDesc.layer == 0);
    assert_invariant(inSubDesc.level == 0);

    outDesc.width = inDesc.width;
    outDesc.height = inDesc.height;
    outDesc.format = inDesc.format;
    outDesc.samples = 0;

    struct ResolveData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto const& ppResolve = fg.addPass<ResolveData>("resolveDepth",
            [&](FrameGraph::Builder& builder, auto& data) {
                // we currently don't support stencil resolve
                assert_invariant(!isStencilFormat(inDesc.format));

                data.input = builder.sample(input);
                data.output = builder.createTexture(outputBufferName, outDesc);
                data.output = builder.write(data.output, FrameGraphTexture::Usage::DEPTH_ATTACHMENT);
                builder.declareRenderPass(builder.getName(data.output), {
                        .attachments = { .depth = { data.output }},
                        .clearFlags = TargetBufferFlags::DEPTH });
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto const& input = resources.getTexture(data.input);
                auto const& material = getPostProcessMaterial("resolveDepth");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);
                mi->setParameter("depth", input, {}); // NEAREST
                commitAndRenderFullScreenQuad(driver, resources.getRenderPassInfo(), mi);
            });

    return ppResolve->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::vsmMipmapPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const input, uint8_t layer, size_t const level,
        float4 clearColor) noexcept {

    struct VsmMipData {
        FrameGraphId<FrameGraphTexture> in;
    };

    auto const& depthMipmapPass = fg.addPass<VsmMipData>("VSM Generate Mipmap Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                const char* name = builder.getName(input);
                data.in = builder.sample(input);

                auto out = builder.createSubresource(data.in, "Mip level", {
                        .level = uint8_t(level + 1), .layer = layer });

                out = builder.write(out, FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                builder.declareRenderPass(name, {
                    .attachments = { .color = { out }},
                    .clearColor = clearColor,
                    .clearFlags = TargetBufferFlags::COLOR
                });
            },
            [=](FrameGraphResources const& resources,
                    auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);

                auto in = driver.createTextureView(resources.getTexture(data.in), level, 1);
                auto out = resources.getRenderPassInfo();

                auto const& inDesc = resources.getDescriptor(data.in);
                auto width = inDesc.width;
                assert_invariant(width == inDesc.height);
                int const dim = width >> (level + 1);

                auto& material = getPostProcessMaterial("vsmMipmap");
                FMaterial const* const ma = material.getMaterial(mEngine);

                // When generating shadow map mip levels, we want to preserve the 1 texel border.
                // (note clearing never respects the scissor in Filament)
                auto const pipeline = getPipelineState(ma);
                backend::Viewport const scissor = { 1u, 1u, dim - 2u, dim - 2u };

                FMaterialInstance* const mi = PostProcessMaterial::getMaterialInstance(ma);
                mi->setParameter("color", in, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });
                mi->setParameter("layer", uint32_t(layer));
                mi->setParameter("uvscale", 1.0f / float(dim));
                mi->commit(driver);
                mi->use(driver);

                renderFullScreenQuadWithScissor(out, pipeline, scissor, driver);

                driver.destroyTexture(in); // `in` is just a view on `data.in`
            });

    return depthMipmapPass->in;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::debugShadowCascades(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> const input,
        FrameGraphId<FrameGraphTexture> const depth) noexcept {

    // new pass for showing the cascades
    struct DebugShadowCascadesData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> output;
    };
    auto const& debugShadowCascadePass = fg.addPass<DebugShadowCascadesData>("ShadowCascades",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const desc = builder.getDescriptor(input);
                data.color = builder.sample(input);
                data.depth = builder.sample(depth);
                data.output = builder.createTexture("Shadow Cascade Debug", desc);
                builder.declareRenderPass(data.output);
            },
            [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto color = resources.getTexture(data.color);
                auto depth = resources.getTexture(data.depth);
                auto const out = resources.getRenderPassInfo();
                auto const& material = getPostProcessMaterial("debugShadowCascades");
                FMaterialInstance* const mi =
                        PostProcessMaterial::getMaterialInstance(mEngine, material);
                mi->setParameter("color",  color, {});  // nearest
                mi->setParameter("depth",  depth, {});  // nearest
                commitAndRenderFullScreenQuad(driver, out, mi);
            });

    return debugShadowCascadePass->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::debugCombineArrayTexture(FrameGraph& fg,
    bool const translucent, FrameGraphId<FrameGraphTexture> const input,
    filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc,
    SamplerMagFilter filterMag,
    SamplerMinFilter filterMin) noexcept {

    auto& inputTextureDesc = fg.getDescriptor(input);
    assert_invariant(inputTextureDesc.depth > 1);
    assert_invariant(inputTextureDesc.type == SamplerType::SAMPLER_2D_ARRAY);

    // TODO: add support for sub-resources
    assert_invariant(fg.getSubResourceDescriptor(input).layer == 0);
    assert_invariant(fg.getSubResourceDescriptor(input).level == 0);

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
    };

    auto const& ppQuadBlit = fg.addPass<QuadBlitData>("combining array tex",
        [&](FrameGraph::Builder& builder, auto& data) {
            data.input = builder.sample(input);
            data.output = builder.createTexture("upscaled output", outDesc);
            data.output = builder.write(data.output,
                FrameGraphTexture::Usage::COLOR_ATTACHMENT);
            builder.declareRenderPass(builder.getName(data.output), {
                    .attachments = {.color = { data.output }},
                    .clearFlags = TargetBufferFlags::DEPTH });
        },
        [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                bindPostProcessDescriptorSet(driver);
                auto color = resources.getTexture(data.input);
                auto const& inputDesc = resources.getDescriptor(data.input);
                auto out = resources.getRenderPassInfo();

                // --------------------------------------------------------------------------------
                // set uniforms

                PostProcessMaterial const& material = getPostProcessMaterial("blitArray");
                FMaterial const* const ma = material.getMaterial(mEngine);
                auto* mi = PostProcessMaterial::getMaterialInstance(ma);
                mi->setParameter("color", color, {
                        .filterMag = filterMag,
                        .filterMin = filterMin
                    });
                mi->setParameter("viewport", float4{
                        float(vp.left) / inputDesc.width,
                        float(vp.bottom) / inputDesc.height,
                        float(vp.width) / inputDesc.width,
                        float(vp.height) / inputDesc.height
                    });
                mi->commit(driver);
                mi->use(driver);

                auto pipeline = getPipelineState(ma);
                if (translucent) {
                    pipeline.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionDstRGB = BlendFunction::ONE_MINUS_SRC_ALPHA;
                    pipeline.rasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
                }

                // The width of each view takes up 1/depth of the screen width.
                out.params.viewport.width /= inputTextureDesc.depth;

                // Render all layers of the texture to the screen side-by-side.
                for (uint32_t i = 0; i < inputTextureDesc.depth; ++i) {
                    mi->setParameter("layerIndex", i);
                    mi->commit(driver);
                    renderFullScreenQuad(out, pipeline, driver);
                    // From the second draw, don't clear the targetbuffer.
                    out.params.flags.clear = TargetBufferFlags::NONE;
                    out.params.flags.discardStart = TargetBufferFlags::NONE;
                    out.params.viewport.left += out.params.viewport.width;
                }
        });

    return ppQuadBlit->output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::debugDisplayShadowTexture(
        FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphId<FrameGraphTexture> const shadowmap, float const scale,
        uint8_t const layer, uint8_t const level, uint8_t const channel, float const power) noexcept {
    if (shadowmap) {
        struct ShadowMapData {
            FrameGraphId<FrameGraphTexture> color;
            FrameGraphId<FrameGraphTexture> depth;
        };

        auto const& desc = fg.getDescriptor(input);
        float const ratio = float(desc.height) / float(desc.width);
        float const screenScale = float(fg.getDescriptor(shadowmap).height) / float(desc.height);
        float2 const s = { screenScale * scale * ratio, screenScale * scale };

        auto const& shadomapDebugPass = fg.addPass<ShadowMapData>("shadowmap debug pass",
                [&](FrameGraph::Builder& builder, auto& data) {
                    data.color = builder.read(input,
                            FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                    data.color = builder.write(data.color,
                            FrameGraphTexture::Usage::COLOR_ATTACHMENT);
                    data.depth = builder.sample(shadowmap);
                    builder.declareRenderPass("color target", {
                            .attachments = { .color = { data.color }}
                    });
                },
                [=](FrameGraphResources const& resources, auto const& data, DriverApi& driver) {
                    bindPostProcessDescriptorSet(driver);
                    auto const out = resources.getRenderPassInfo();
                    auto in = resources.getTexture(data.depth);
                    auto const& material = getPostProcessMaterial("shadowmap");
                    FMaterialInstance* const mi =
                            PostProcessMaterial::getMaterialInstance(mEngine, material);
                    mi->setParameter("shadowmap", in, {
                        .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                    mi->setParameter("scale", s);
                    mi->setParameter("layer", (uint32_t)layer);
                    mi->setParameter("level", (uint32_t)level);
                    mi->setParameter("channel", (uint32_t)channel);
                    mi->setParameter("power", power);
                    commitAndRenderFullScreenQuad(driver, out, mi);
                });
        input = shadomapDebugPass->color;
    }
    return input;
}

} // namespace filament
