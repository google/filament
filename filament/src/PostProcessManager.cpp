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

#include "PostProcessManager.h"

#include "details/Engine.h"

#include "fg/FrameGraph.h"
#include "fg/FrameGraphPassResources.h"

#include "RenderPass.h"

#include "details/Camera.h"
#include "details/ColorGrading.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "details/Texture.h"

#include "generated/resources/materials.h"

#include <private/filament/SibGenerator.h>

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
    assert(!mHasMaterial || mMaterial == nullptr);
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
    assert(mMaterialRegistry.find(name) != mMaterialRegistry.end());
    return mMaterialRegistry[name];
}

#define MATERIAL(n) MATERIALS_ ## n ## _DATA, MATERIALS_ ## n ## _SIZE

void PostProcessManager::init() noexcept {
    auto& engine = mEngine;
    DriverApi& driver = engine.getDriverApi();

    registerPostProcessMaterial("sao", MATERIAL(SAO));
    registerPostProcessMaterial("mipmapDepth", MATERIAL(MIPMAPDEPTH));
    registerPostProcessMaterial("bilateralBlur", MATERIAL(BILATERALBLUR));
    registerPostProcessMaterial("separableGaussianBlur", MATERIAL(SEPARABLEGAUSSIANBLUR));
    registerPostProcessMaterial("bloomDownsample", MATERIAL(BLOOMDOWNSAMPLE));
    registerPostProcessMaterial("bloomUpsample", MATERIAL(BLOOMUPSAMPLE));
    registerPostProcessMaterial("blitLow", MATERIAL(BLITLOW));
    registerPostProcessMaterial("blitMedium", MATERIAL(BLITMEDIUM));
    registerPostProcessMaterial("blitHigh", MATERIAL(BLITHIGH));
    registerPostProcessMaterial("colorGrading", MATERIAL(COLORGRADING));
    registerPostProcessMaterial("colorGradingAsSubpass", MATERIAL(COLORGRADINGASSUBPASS));
    registerPostProcessMaterial("fxaa", MATERIAL(FXAA));
    registerPostProcessMaterial("taa", MATERIAL(TAA));
    registerPostProcessMaterial("dofDownsample", MATERIAL(DOFDOWNSAMPLE));
    registerPostProcessMaterial("dofMipmap", MATERIAL(DOFMIPMAP));
    registerPostProcessMaterial("dofTiles", MATERIAL(DOFTILES));
    registerPostProcessMaterial("dofDilate", MATERIAL(DOFDILATE));
    registerPostProcessMaterial("dof", MATERIAL(DOF));
    registerPostProcessMaterial("dofMedian", MATERIAL(DOFMEDIAN));
    registerPostProcessMaterial("dofCombine", MATERIAL(DOFCOMBINE));

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

    PixelBufferDescriptor dataOne(driver.allocate(4), 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    PixelBufferDescriptor dataOneArray(driver.allocate(4), 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    PixelBufferDescriptor dataZero(driver.allocate(4), 4, PixelDataFormat::RGBA, PixelDataType::UBYTE);
    *static_cast<uint32_t *>(dataOne.buffer) = 0xFFFFFFFF;
    *static_cast<uint32_t *>(dataOneArray.buffer) = 0xFFFFFFFF;
    *static_cast<uint32_t *>(dataZero.buffer) = 0;
    driver.update2DImage(mDummyOneTexture, 0, 0, 0, 1, 1, std::move(dataOne));
    driver.update3DImage(mDummyOneTextureArray, 0, 0, 0, 0, 1, 1, 1, std::move(dataOneArray));
    driver.update2DImage(mDummyZeroTexture, 0, 0, 0, 1, 1, std::move(dataZero));
}

void PostProcessManager::terminate(DriverApi& driver) noexcept {
    FEngine& engine = mEngine;
    driver.destroyTexture(mDummyOneTexture);
    driver.destroyTexture(mDummyOneTextureArray);
    driver.destroyTexture(mDummyZeroTexture);
    auto first = mMaterialRegistry.begin();
    auto last = mMaterialRegistry.end();
    while (first != last) {
        first.value().terminate(engine);
        ++first;
    }
}

UTILS_NOINLINE
void PostProcessManager::commitAndRender(FrameGraphRenderTarget const& out,
        PostProcessMaterial const& material, uint8_t variant, DriverApi& driver) const noexcept {
    FMaterialInstance* const mi = material.getMaterialInstance();
    mi->commit(driver);
    mi->use(driver);
    driver.beginRenderPass(out.target, out.params);
    driver.draw(material.getPipelineState(variant), mEngine.getFullScreenRenderPrimitive());
    driver.endRenderPass();
}

UTILS_ALWAYS_INLINE
void PostProcessManager::commitAndRender(FrameGraphRenderTarget const& out,
        PostProcessMaterial const& material, DriverApi& driver) const noexcept {
    commitAndRender(out, material, 0, driver);
}

// ------------------------------------------------------------------------------------------------

FrameGraphId<FrameGraphTexture> PostProcessManager::structure(FrameGraph& fg,
        const RenderPass& pass, uint32_t width, uint32_t height, float scale) noexcept {

    // structure pass -- automatically culled if not used, currently used by:
    //    - ssao
    //    - contact shadows
    //    - depth-of-field
    // It consists of a mipmapped depth pass, tuned for SSAO
    struct StructurePassData {
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphRenderTargetHandle rt;
    };

    // sanitize a bit the user provided scaling factor
    width  = std::max(32u, (uint32_t)std::ceil(width * scale));
    height = std::max(32u, (uint32_t)std::ceil(height * scale));

    // We limit the lowest lod size to 32 pixels (which is where the -5 comes from)
    const size_t levelCount = FTexture::maxLevelCount(width, height) - 5;
    assert(levelCount >= 1);

    // generate depth pass at the requested resolution
    auto& structurePass = fg.addPass<StructurePassData>("Structure Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.depth = builder.createTexture("Structure Buffer", {
                        .width = width, .height = height,
                        .levels = uint8_t(levelCount),
                        .format = TextureFormat::DEPTH32F });

                data.depth = builder.write(builder.read(data.depth));

                data.rt = builder.createRenderTarget("Structure Target", {
                        .attachments = {{}, data.depth },
                        .clearFlags = TargetBufferFlags::DEPTH
                });
            },
            [=](FrameGraphPassResources const& resources, auto const& data, DriverApi& driver) {
                auto out = resources.get(data.rt);
                pass.execute(resources.getPassName(), out.target, out.params);
            });

    auto depth = structurePass.getData().depth;

    /*
     * create depth mipmap chain
    */

    // The first mip already exists, so we process n-1 lods
    for (size_t level = 0; level < levelCount - 1; level++) {
        depth = mipmapPass(fg, depth, level);
    }

    fg.getBlackboard().put("structure", depth);
    return depth;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::mipmapPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, size_t level) noexcept {

    struct DepthMipData {
        FrameGraphId<FrameGraphTexture> in;
        FrameGraphId<FrameGraphTexture> out;
        FrameGraphRenderTargetHandle rt;

    };

    auto& depthMipmapPass = fg.addPass<DepthMipData>("Depth Mipmap Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                const char* name = builder.getName(input);
                data.in = builder.sample(input);
                data.out = builder.write(data.in);
                data.rt = builder.createRenderTarget(name, {
                        .attachments = {{}, { data.out, uint8_t(level + 1) }}});
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto in = resources.getTexture(data.in);
                auto out = resources.get(data.rt);

                auto& material = getPostProcessMaterial("mipmapDepth");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("depth", in, { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("level", uint32_t(level));

                commitAndRender(out, material, driver);
            });

    return depthMipmapPass.getData().out;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::screenSpaceAmbientOcclusion(
        FrameGraph& fg, RenderPass& pass,
        filament::Viewport const& svp, const CameraInfo& cameraInfo,
        View::AmbientOcclusionOptions options) noexcept {

    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    FrameGraphId<FrameGraphTexture> depth = fg.getBlackboard().get<FrameGraphTexture>("structure");
    assert(depth.isValid());

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
            spiralTurns = 5.0f;
            break;
        case View::QualityLevel::MEDIUM:
            sampleCount = 11.0f;
            spiralTurns = 9.0f;
            break;
        case View::QualityLevel::HIGH:
            sampleCount = 16.0f;
            spiralTurns = 10.0f;
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
        FrameGraphRenderTargetHandle rt;
    };

    auto& SSAOPass = fg.addPass<SSAOPassData>("SSAO Pass",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const& desc = builder.getDescriptor(depth);

                data.depth = builder.sample(depth);
                data.ssao = builder.createTexture("SSAO Buffer", {
                        .width = desc.width,
                        .height = desc.height,
                        .format = (options.lowPassFilter == View::QualityLevel::LOW) ? TextureFormat::R8 : TextureFormat::RGB8
                });

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // Note that we have to clear the SAO buffer because blended objects will end-up
                // reading into it even though they were not written in the depth buffer.
                // The bilateral filter in the blur pass will ignore pixels at infinity.

                data.ssao = builder.write(data.ssao);
                data.rt = builder.createRenderTarget("SSAO Target", {
                        .attachments = { data.ssao, data.depth },
                        .clearColor = { 1.0f },
                        .clearFlags = TargetBufferFlags::COLOR
                });
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto depth = resources.getTexture(data.depth);
                auto ssao = resources.get(data.rt);
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

    FrameGraphId<FrameGraphTexture> ssao = SSAOPass.getData().ssao;

    /*
     * Final separable bilateral blur pass
     */

    if (options.lowPassFilter != View::QualityLevel::LOW) {
        const bool highQualitySampling =
                options.upsampling >= View::QualityLevel::HIGH && options.resolution < 1.0f;

        ssao = bilateralBlurPass(fg, ssao, { config.scale, 0 }, cameraInfo.zf,
                TextureFormat::RGB8,
                config);

        ssao = bilateralBlurPass(fg, ssao, { 0, config.scale }, cameraInfo.zf,
                highQualitySampling ? TextureFormat::RGB8 : TextureFormat::R8,
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
        FrameGraphRenderTargetHandle rt;
    };

    auto& blurPass = fg.addPass<BlurPassData>("Separable Blur Pass",
            [&](FrameGraph::Builder& builder, auto& data) {

                auto const& desc = builder.getDescriptor(input);

                data.input = builder.sample(input);

                data.blurred = builder.createTexture("Blurred output", {
                        .width = desc.width, .height = desc.height, .format = format });

                auto depth = fg.getBlackboard().get<FrameGraphTexture>("structure");
                assert(depth.isValid());
                builder.read(depth);

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // We need to clear the buffers because we are skipping pixels at infinity (skybox)
                data.blurred = builder.write(data.blurred);
                data.rt = builder.createRenderTarget("Blurred target", {
                        .attachments = { data.blurred, depth },
                        .clearColor = { 1.0f },
                        .clearFlags = TargetBufferFlags::COLOR
                });
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto ssao = resources.getTexture(data.input);
                auto blurred = resources.get(data.rt);
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

    return blurPass.getData().blurred;
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
        FrameGraphRenderTargetHandle outRT;
        FrameGraphRenderTargetHandle tempRT;
    };

    const size_t kernelStorageSize = mSeparableGaussianBlurKernelStorageSize;
    auto& gaussianBlurPasses = fg.addPass<BlurPassData>("Gaussian Blur Passes",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto desc = builder.getDescriptor(input);

                if (!output.isValid()) {
                    output = builder.createTexture("Blurred texture", desc);
                }

                data.in = builder.sample(input);

                data.out = builder.write(output);

                // width of the destination level (b/c we're blurring horizontally)
                desc.width = FTexture::valueForLevel(dstLevel, desc.width);
                // height of the source level (b/c it's not blurred in this pass)
                desc.height = FTexture::valueForLevel(srcLevel, desc.height);
                // only one level
                desc.levels = 1;

                data.temp = builder.createTexture("Horizontal temporary buffer", desc);
                data.temp = builder.write(builder.sample(data.temp));

                data.tempRT = builder.createRenderTarget("Horizontal temporary target", {
                        .attachments = { data.temp } });

                data.outRT = builder.createRenderTarget("Blurred target", {
                        .attachments = {{ data.out, dstLevel }} });
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto const& separableGaussianBlur = getPostProcessMaterial("separableGaussianBlur");
                FMaterialInstance* const mi = separableGaussianBlur.getMaterialInstance();

                float2 kernel[64];
                size_t m = computeGaussianCoefficients(kernel,
                        std::min(sizeof(kernel) / sizeof(*kernel), kernelStorageSize));

                // horizontal pass
                auto hwTempRT = resources.get(data.tempRT);
                auto hwOutRT = resources.get(data.outRT);
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
                auto width = FTexture::valueForLevel(dstLevel, outDesc.width);
                auto height = FTexture::valueForLevel(dstLevel, outDesc.height);
                assert(width == hwOutRT.params.viewport.width);
                assert(height == hwOutRT.params.viewport.height);

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

    return gaussianBlurPasses.getData().out;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::dof(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        const View::DepthOfFieldOptions& dofOptions,
        bool translucent,
        const CameraInfo& cameraInfo) noexcept {

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

    const float focusDistance = std::max(cameraInfo.zn, dofOptions.focusDistance);
    auto const& desc = fg.getDescriptor<FrameGraphTexture>(input);
    const float Kc = (cameraInfo.A * cameraInfo.f) / (focusDistance - cameraInfo.f);
    const float Ks = ((float)desc.height) / FCamera::SENSOR_SIZE;
    float2 cocParams{
            // we use 1/zn instead of (zf - zn) / (zf * zn), because in reality we're using
            // a projection with an infinite far plane
            (dofOptions.cocScale * Ks * Kc) * focusDistance / cameraInfo.zn,
            (dofOptions.cocScale * Ks * Kc) * (1.0f - focusDistance / cameraInfo.zn)
    };
    // handle reversed z
    cocParams = float2{ -cocParams.x, cocParams.x + cocParams.y };

    Blackboard& blackboard = fg.getBlackboard();
    auto depth = blackboard.get<FrameGraphTexture>("depth");
    assert(depth.isValid());

    // the downsampled target is multiple of 8, so we can have 4 clean mipmap levels
    constexpr const uint32_t maxMipLevels = 4u;
    constexpr const uint32_t maxMipLevelsMask = (1u << maxMipLevels) - 1u;
    auto const& colorDesc = fg.getDescriptor(input);
    const uint32_t width  = ((colorDesc.width  + maxMipLevelsMask) & ~maxMipLevelsMask) / 2;
    const uint32_t height = ((colorDesc.height + maxMipLevelsMask) & ~maxMipLevelsMask) / 2;
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
        FrameGraphId<FrameGraphTexture> outForeground;
        FrameGraphId<FrameGraphTexture> outBackground;
        FrameGraphId<FrameGraphTexture> outCocFgBg;
        FrameGraphRenderTargetHandle rt;
    };

    auto& ppDoFDownsample = fg.addPass<PostProcessDofDownsample>("DoF Downsample",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.color = builder.sample(input);
                data.depth = builder.sample(depth);

                data.outForeground = builder.createTexture("dof foreground output", {
                        .width  = width,
                        .height = height,
                        .levels = mipmapCount,
                        .format = format
                });
                data.outBackground = builder.createTexture("dof background output", {
                        .width  = width,
                        .height = height,
                        .levels = mipmapCount,
                        .format = format
                });
                data.outCocFgBg = builder.createTexture("dof CoC output", {
                        .width  = width,
                        .height = height,
                        .levels = mipmapCount,
                        .format = TextureFormat::RG16F
                });
                data.outForeground = builder.write(data.outForeground);
                data.outBackground = builder.write(data.outBackground);
                data.outCocFgBg    = builder.write(data.outCocFgBg);
                data.rt = builder.createRenderTarget("DoF Target", {
                        .attachments = {
                                { data.outForeground, data.outBackground, data.outCocFgBg }, {}, {}
                        }
                });
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto const& out = resources.get(data.rt);
                auto color = resources.getTexture(data.color);
                auto depth = resources.getTexture(data.depth);
                auto const& material = getPostProcessMaterial("dofDownsample");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("color", color, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("depth", depth, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("cocParams", cocParams);
                mi->setParameter("uvscale", float4{ width, height, 1.0f / colorDesc.width, 1.0f / colorDesc.height });
                commitAndRender(out, material, driver);
            });

    /*
     * Setup (Continued)
     *      - Generate mipmaps
     */

    struct PostProcessDofMipmap {
        FrameGraphId<FrameGraphTexture> inOutForeground;
        FrameGraphId<FrameGraphTexture> inOutBackground;
        FrameGraphId<FrameGraphTexture> inOutCocFgBg;
        FrameGraphRenderTargetHandle rt[3];
    };

    assert(mipmapCount - 1
           <= sizeof(PostProcessDofMipmap::rt) / sizeof(FrameGraphRenderTargetHandle));

    auto& ppDoFMipmap = fg.addPass<PostProcessDofMipmap>("DoF Mipmap",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.inOutForeground = builder.sample(ppDoFDownsample.getData().outForeground);
                data.inOutBackground = builder.sample(ppDoFDownsample.getData().outBackground);
                data.inOutCocFgBg    = builder.sample(ppDoFDownsample.getData().outCocFgBg);
                data.inOutForeground = builder.write(data.inOutForeground);
                data.inOutBackground = builder.write(data.inOutBackground);
                data.inOutCocFgBg    = builder.write(data.inOutCocFgBg);
                for (size_t i = 0; i < mipmapCount - 1u; i++) {
                    // make sure inputs are always multiple of two (should be true by construction)
                    // (this is so that we can compute clean mip levels)
                    assert((FTexture::valueForLevel(uint8_t(i), fg.getDescriptor(data.inOutForeground).width ) & 0x1u) == 0);
                    assert((FTexture::valueForLevel(uint8_t(i), fg.getDescriptor(data.inOutForeground).height) & 0x1u) == 0);
                    using Attachment = FrameGraphRenderTarget::Attachments::AttachmentInfo;
                    data.rt[i] = builder.createRenderTarget("DoF Target", {
                            .attachments = {
                                    {
                                        Attachment{ data.inOutForeground, uint8_t(i + 1) },
                                        Attachment{ data.inOutBackground, uint8_t(i + 1) },
                                        Attachment{ data.inOutCocFgBg,    uint8_t(i + 1) }
                                    },
                                    {},
                                    {}
                            }
                    });
                }
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto inOutForeground = resources.getTexture(data.inOutForeground);
                auto inOutBackground = resources.getTexture(data.inOutBackground);
                auto inOutCocFgBg    = resources.getTexture(data.inOutCocFgBg);

                auto const& material = getPostProcessMaterial("dofMipmap");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("foreground", inOutForeground, { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("background", inOutBackground, { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("cocFgBg",    inOutCocFgBg,    { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->use(driver);

                const PipelineState pipeline(material.getPipelineState(variant));

                for (size_t level = 0 ; level < mipmapCount - 1u ; level++) {
                    auto const& out = resources.get(data.rt[level]);
                    mi->setParameter("mip", uint32_t(level));
                    mi->setParameter("weightScale", 0.5f / float(1u<<level));
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

    auto inTilesCocMaxMin = ppDoFDownsample.getData().outCocFgBg;

    // Match this with TILE_SIZE in dofDilate.mat
    const size_t tileSize = 16; // size of the tile in full resolution pixel
    const uint32_t tileBufferWidth  = ((colorDesc.width  + (tileSize - 1u)) & ~(tileSize - 1u)) / 4u;
    const uint32_t tileBufferHeight = ((colorDesc.height + (tileSize - 1u)) & ~(tileSize - 1u)) / 4u;
    const size_t tileReductionCount = std::log2(tileSize) - 1.0; // -1 because we start from half-resolution

    struct PostProcessDofTiling1 {
        FrameGraphId<FrameGraphTexture> inCocMaxMin;
        FrameGraphId<FrameGraphTexture> outTilesCocMaxMin;
        FrameGraphRenderTargetHandle rt;
    };

    for (size_t i = 0; i < tileReductionCount; i++) {
        auto& ppDoFTiling = fg.addPass<PostProcessDofTiling1>("DoF Tiling",
                [&](FrameGraph::Builder& builder, auto& data) {
                    assert((tileBufferWidth  & 1u) == 0);
                    assert((tileBufferHeight & 1u) == 0);
                    data.inCocMaxMin = builder.sample(inTilesCocMaxMin);
                    data.outTilesCocMaxMin = builder.createTexture("dof tiles output", {
                            .width  = tileBufferWidth  >> i,
                            .height = tileBufferHeight >> i,
                            .format = TextureFormat::RG16F
                    });
                    data.outTilesCocMaxMin = builder.write(data.outTilesCocMaxMin);
                    data.rt = builder.createRenderTarget("DoF Tiles Target", {
                            .attachments = { data.outTilesCocMaxMin }
                    });
                },
                [=](FrameGraphPassResources const& resources,
                        auto const& data, DriverApi& driver) {
                    auto const& inputDesc = resources.getDescriptor(data.inCocMaxMin);
                    auto const& outputDesc = resources.getDescriptor(data.outTilesCocMaxMin);
                    auto const& out = resources.get(data.rt);
                    auto inCocMaxMin = resources.getTexture(data.inCocMaxMin);
                    auto const& material = getPostProcessMaterial("dofTiles");
                    FMaterialInstance* const mi = material.getMaterialInstance();
                    mi->setParameter("cocMaxMin", inCocMaxMin, { .filterMin = SamplerMinFilter::NEAREST });
                    mi->setParameter("uvscale", float4{
                        outputDesc.width, outputDesc.height,
                        1.0f / inputDesc.width, 1.0f / inputDesc.height });
                    commitAndRender(out, material, driver);
                });
        inTilesCocMaxMin = ppDoFTiling.getData().outTilesCocMaxMin;
    }

    /*
     * Dilate tiles
     */

    // This is a small helper that does one round of dilate
    auto dilate = [&](FrameGraphId<FrameGraphTexture> input) -> FrameGraphId<FrameGraphTexture> {

        struct PostProcessDofDilate {
            FrameGraphId<FrameGraphTexture> inTilesCocMaxMin;
            FrameGraphId<FrameGraphTexture> outTilesCocMaxMin;
            FrameGraphRenderTargetHandle rt;
        };

        auto& ppDoFDilate = fg.addPass<PostProcessDofDilate>("DoF Dilate",
                [&](FrameGraph::Builder& builder, auto& data) {
                    auto const& inputDesc = fg.getDescriptor(input);
                    data.inTilesCocMaxMin = builder.sample(input);
                    data.outTilesCocMaxMin = builder.createTexture("dof dilated tiles output", inputDesc);
                    data.outTilesCocMaxMin = builder.write(data.outTilesCocMaxMin);
                    data.rt = builder.createRenderTarget("DoF Dilated Tiles Target", {
                            .attachments = { data.outTilesCocMaxMin }
                    });
                },
                [=](FrameGraphPassResources const& resources,
                        auto const& data, DriverApi& driver) {
                    auto const& out = resources.get(data.rt);
                    auto inTilesCocMaxMin = resources.getTexture(data.inTilesCocMaxMin);
                    auto const& material = getPostProcessMaterial("dofDilate");
                    FMaterialInstance* const mi = material.getMaterialInstance();
                    mi->setParameter("tiles", inTilesCocMaxMin, { .filterMin = SamplerMinFilter::NEAREST });
                    commitAndRender(out, material, driver);
                });
        return ppDoFDilate.getData().outTilesCocMaxMin;
    };

    // Tiles of 16 pixels requires two dilate rounds to accommodate our max Coc of 32 pixels
    auto dilated = dilate(inTilesCocMaxMin);
    dilated = dilate(dilated);

    /*
     * DoF blur pass
     */

    struct PostProcessDof {
        FrameGraphId<FrameGraphTexture> foreground;
        FrameGraphId<FrameGraphTexture> background;
        FrameGraphId<FrameGraphTexture> cocFgBg;
        FrameGraphId<FrameGraphTexture> tilesCocMaxMin;
        FrameGraphId<FrameGraphTexture> outForeground;
        FrameGraphId<FrameGraphTexture> outAlpha;
        FrameGraphRenderTargetHandle rt;
    };

    auto& ppDoF = fg.addPass<PostProcessDof>("DoF",
            [&](FrameGraph::Builder& builder, auto& data) {

                data.foreground     = builder.sample(ppDoFMipmap.getData().inOutForeground);
                data.background     = builder.sample(ppDoFMipmap.getData().inOutBackground);
                data.cocFgBg        = builder.sample(ppDoFMipmap.getData().inOutCocFgBg);
                data.tilesCocMaxMin = builder.sample(dilated);

                // The DoF buffer (output) doesn't need to be a multiple of 8 because it's not
                // mipmapped. We just need to adjust the uv properly.
                data.outForeground = builder.createTexture("dof color output", {
                        .width  = (colorDesc.width  + 1) / 2,
                        .height = (colorDesc.height + 1) / 2,
                        .format = fg.getDescriptor(data.foreground).format
                });
                data.outAlpha = builder.createTexture("dof alpha output", {
                        .width  = (colorDesc.width  + 1) / 2,
                        .height = (colorDesc.height + 1) / 2,
                        .format = TextureFormat::R8
                });
                data.outForeground  = builder.write(data.outForeground);
                data.outAlpha       = builder.write(data.outAlpha);
                data.rt = builder.createRenderTarget("DoF Target", {
                        .attachments = { { data.outForeground,
                                           data.outAlpha, {}, {} }, {}, {} }
                });
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto const& out = resources.get(data.rt);

                auto foreground     = resources.getTexture(data.foreground);
                auto background     = resources.getTexture(data.background);
                auto cocFgBg        = resources.getTexture(data.cocFgBg);
                auto tilesCocMaxMin = resources.getTexture(data.tilesCocMaxMin);

                auto const& inputDesc = resources.getDescriptor(data.cocFgBg);
                auto const& outputDesc = resources.getDescriptor(data.outForeground);
                auto const& tilesDesc = resources.getDescriptor(data.tilesCocMaxMin);

                auto const& material = getPostProcessMaterial("dof");
                FMaterialInstance* const mi = material.getMaterialInstance();
                // it's not safe to use bilinear filtering in the general case (causes artifacts around edges)
                mi->setParameter("foreground", foreground,
                        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("foregroundLinear", foreground,
                        { .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST });
                mi->setParameter("background", background,
                        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("cocFgBg", cocFgBg,
                        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("tiles", tilesCocMaxMin,
                        { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("cocToTexelOffset", 0.5f / float2{ inputDesc.width, inputDesc.height });
                mi->setParameter("uvscale", float4{
                    outputDesc.width  / float(inputDesc.width),
                    outputDesc.height / float(inputDesc.height),
                    outputDesc.width  / (tileSize * 0.5f * tilesDesc.width),
                    outputDesc.height / (tileSize * 0.5f * tilesDesc.height)
                });
                mi->setParameter("bokehAngle",  bokehAngle);
                commitAndRender(out, material, driver);
            });

    /*
     * DoF median
     */


    struct PostProcessDofMedian {
        FrameGraphId<FrameGraphTexture> inForeground;
        FrameGraphId<FrameGraphTexture> inAlpha;
        FrameGraphId<FrameGraphTexture> tilesCocMaxMin;
        FrameGraphId<FrameGraphTexture> outForeground;
        FrameGraphId<FrameGraphTexture> outAlpha;
        FrameGraphRenderTargetHandle rt;
    };

    auto& ppDoFMedian = fg.addPass<PostProcessDofMedian>("DoF Median",
            [&](FrameGraph::Builder& builder, auto& data) {

                data.inForeground   = builder.sample(ppDoF.getData().outForeground);
                data.inAlpha        = builder.sample(ppDoF.getData().outAlpha);
                data.tilesCocMaxMin = builder.sample(dilated);

                data.outForeground  = builder.createTexture("dof color output", fg.getDescriptor(data.inForeground));
                data.outAlpha       = builder.createTexture("dof alpha output", fg.getDescriptor(data.inAlpha));
                data.outForeground  = builder.write(data.outForeground);
                data.outAlpha       = builder.write(data.outAlpha);
                data.rt = builder.createRenderTarget("DoF Target", {
                        .attachments = { { data.outForeground,
                                                 data.outAlpha, {}, {} }, {}, {} }
                });
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto const& out = resources.get(data.rt);

                auto inForeground   = resources.getTexture(data.inForeground);
                auto inAlpha        = resources.getTexture(data.inAlpha);
                auto tilesCocMaxMin = resources.getTexture(data.tilesCocMaxMin);

                auto const& outputDesc = resources.getDescriptor(data.outForeground);
                auto const& tilesDesc = resources.getDescriptor(data.tilesCocMaxMin);

                auto const& material = getPostProcessMaterial("dofMedian");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("dof",   inForeground,   { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("alpha", inAlpha,        { .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });
                mi->setParameter("tiles", tilesCocMaxMin, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("uvscale", float2{
                        outputDesc.width  / (tileSize * 0.5f * tilesDesc.width),
                        outputDesc.height / (tileSize * 0.5f * tilesDesc.height)
                });
                commitAndRender(out, material, driver);
            });


    /*
     * DoF recombine
     */

    auto outForeground = ppDoFMedian.getData().outForeground;
    auto outAlpha = ppDoFMedian.getData().outAlpha;
    if (false) { // TODO: make this a quality setting
        outForeground = ppDoF.getData().outForeground;
        outAlpha = ppDoF.getData().outAlpha;
    }

    struct PostProcessDofCombine {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> dof;
        FrameGraphId<FrameGraphTexture> alpha;
        FrameGraphId<FrameGraphTexture> tilesCocMaxMin;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle rt;
    };

    auto& ppDoFCombine = fg.addPass<PostProcessDofCombine>("DoF combine",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.color      = builder.sample(input);
                data.dof        = builder.sample(outForeground);
                data.alpha      = builder.sample(outAlpha);
                data.tilesCocMaxMin = builder.sample(dilated);
                auto const& inputDesc = fg.getDescriptor(data.color);
                data.output = builder.createTexture("dof output", inputDesc);
                data.output = builder.write(data.output);
                data.rt = builder.createRenderTarget("DoF Target", {
                        .attachments = { data.output }
                });
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {
                auto const& dofDesc = resources.getDescriptor(data.dof);
                auto const& tilesDesc = resources.getDescriptor(data.tilesCocMaxMin);
                auto const& out = resources.get(data.rt);

                auto color      = resources.getTexture(data.color);
                auto dof        = resources.getTexture(data.dof);
                auto alpha      = resources.getTexture(data.alpha);
                auto tilesCocMaxMin = resources.getTexture(data.tilesCocMaxMin);

                auto const& material = getPostProcessMaterial("dofCombine");
                FMaterialInstance* const mi = material.getMaterialInstance();
                mi->setParameter("color", color, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("dof",   dof,   { .filterMag = SamplerMagFilter::NEAREST });
                mi->setParameter("alpha", alpha, { .filterMag = SamplerMagFilter::NEAREST });
                mi->setParameter("tiles", tilesCocMaxMin, { .filterMin = SamplerMinFilter::NEAREST });
                mi->setParameter("uvscale", float4{
                    colorDesc.width  / (dofDesc.width    *  2.0f),
                    colorDesc.height / (dofDesc.height   *  2.0f),
                    colorDesc.width  / (tilesDesc.width  * float(tileSize)),
                    colorDesc.height / (tilesDesc.height * float(tileSize))
                });
                commitAndRender(out, material, driver);
            });

    return ppDoFCombine.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::bloomPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, TextureFormat outFormat,
        View::BloomOptions& bloomOptions, float2 scale) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    // Figure out a good size for the bloom buffer. We pick the major axis lower
    // power of two, and scale the minor axis accordingly taking dynamic scaling into account.
    auto const& desc = fg.getDescriptor(input);
    uint32_t width = desc.width / scale.x;
    uint32_t height = desc.height / scale.y;
    if (bloomOptions.anamorphism >= 1.0) {
        height *= bloomOptions.anamorphism;
    } else if (bloomOptions.anamorphism < 1.0) {
        width *= 1.0f / std::max(bloomOptions.anamorphism, 1.0f / 4096.0f);
    }
    uint32_t& major = width > height ? width : height;
    uint32_t& minor = width < height ? width : height;
    uint32_t newMinor = clamp(bloomOptions.resolution,
            1u << bloomOptions.levels, std::min(minor, 1u << kMaxBloomLevels));
    major = major * uint64_t(newMinor) / minor;
    minor = newMinor;

    // we might need to adjust the max # of levels
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

    struct BloomPassData {
        FrameGraphId<FrameGraphTexture> in;
        FrameGraphId<FrameGraphTexture> out;
        FrameGraphRenderTargetHandle outRT[kMaxBloomLevels];
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
                data.out = builder.write(builder.sample(data.out));

                for (size_t i = 0; i < bloomOptions.levels; i++) {
                    data.outRT[i] = builder.createRenderTarget("Bloom target", {
                            .attachments = {{ data.out, uint8_t(i) }} });
                }
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto const& material = getPostProcessMaterial("bloomDownsample");
                FMaterialInstance* mi = material.getMaterialInstance();

                const PipelineState pipeline(material.getPipelineState());

                auto hwIn = resources.getTexture(data.in);
                auto hwOut = resources.getTexture(data.out);
                auto const& outDesc = resources.getDescriptor(data.out);

                mi->use(driver);
                mi->setParameter("source", hwIn,  {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR /* level is always 0 */
                });
                mi->setParameter("level", 0.0f);
                mi->setParameter("threshold", bloomOptions.threshold ? 1.0f : 0.0f);
                mi->setParameter("invHighlight", std::isinf(bloomOptions.highlight) ? 0.0f : 1.0f / bloomOptions.highlight);

                for (size_t i = 0; i < bloomOptions.levels; i++) {
                    auto hwOutRT = resources.get(data.outRT[i]);

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
                    mi->setParameter("source", hwOut,  {
                            .filterMag = SamplerMagFilter::LINEAR,
                            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                    });
                    mi->setParameter("level", float(i));
                }
            });

    input = bloomDownsamplePass.getData().out;

    // upsample phase
    auto& bloomUpsamplePass = fg.addPass<BloomPassData>("Bloom Upsample",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.in = builder.sample(input);
                data.out = builder.write(input);

                for (size_t i = 0; i < bloomOptions.levels; i++) {
                    data.outRT[i] = builder.createRenderTarget("Bloom target", {
                            .attachments = {{ data.out, uint8_t(i) }} });
                }
            },
            [=](FrameGraphPassResources const& resources,
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
                    auto hwDstRT = resources.get(data.outRT[i - 1]);
                    hwDstRT.params.flags.discardStart = TargetBufferFlags::NONE; // because we'll blend
                    hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                    auto w = FTexture::valueForLevel(i - 1, outDesc.width);
                    auto h = FTexture::valueForLevel(i - 1, outDesc.height);
                    mi->setParameter("resolution", float4{ w, h, 1.0f / w, 1.0f / h });
                    mi->setParameter("source", hwIn, {
                            .filterMag = SamplerMagFilter::LINEAR,
                            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                    });
                    mi->setParameter("level", float(i));
                    mi->commit(driver);

                    driver.beginRenderPass(hwDstRT.target, hwDstRT.params);
                    driver.draw(pipeline, fullScreenRenderPrimitive);
                    driver.endRenderPass();
                }
            });

    return bloomUpsamplePass.getData().out;
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
                mix(1.0f + 4.0f * (1.0f - options.feather), 1.0f, std::sqrtf(oval));

        // Factor to transform oval into circle
        float aspect = mix(1.0f, float(width) / float(height), circle);

        return float4{ midPoint, radius, aspect, options.feather };
    }

    // Set half-max to show disabled
    return float4{ std::numeric_limits<half>::max() };
}

void PostProcessManager::colorGradingPrepareSubpass(DriverApi& driver,
        const FColorGrading* colorGrading, View::VignetteOptions vignetteOptions, bool fxaa,
        bool dithering, uint32_t width, uint32_t height) noexcept {

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
    mi->setParameter("dithering", dithering);
    mi->setParameter("fxaa", fxaa);
    mi->setParameter("temporalNoise", temporalNoise);
    mi->commit(driver);
}

void PostProcessManager::colorGradingSubpass(DriverApi& driver,  bool translucent) noexcept {
    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    auto const& material = getPostProcessMaterial("colorGradingAsSubpass");
    material.getMaterialInstance()->use(driver);
    const uint8_t variant = uint8_t(translucent ?
            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

    driver.nextSubpass();
    driver.draw(material.getPipelineState(variant), fullScreenRenderPrimitive);
}

FrameGraphId<FrameGraphTexture> PostProcessManager::colorGrading(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, const FColorGrading* colorGrading,
        TextureFormat outFormat, bool translucent, bool fxaa, float2 scale,
        View::BloomOptions bloomOptions, View::VignetteOptions vignetteOptions, bool dithering) noexcept {

    struct PostProcessColorGrading {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> bloom;
        FrameGraphId<FrameGraphTexture> dirt;
        FrameGraphRenderTargetHandle rt;
    };

    FrameGraphId<FrameGraphTexture> bloomBlur;
    FrameGraphId<FrameGraphTexture> bloomDirt;

    float bloom = 0.0f;
    if (bloomOptions.enabled) {
        bloom = clamp(bloomOptions.strength, 0.0f, 1.0f);
        bloomBlur = bloomPass(fg, input, TextureFormat::R11F_G11F_B10F, bloomOptions, scale);
        if (bloomOptions.dirt) {
            FTexture* fdirt = upcast(bloomOptions.dirt);
            FrameGraphTexture frameGraphTexture { .texture = fdirt->getHwHandle() };
            bloomDirt = fg.import("dirt", {
                    .width = (uint32_t)fdirt->getWidth(0u),
                    .height = (uint32_t)fdirt->getHeight(0u),
                    .format = fdirt->getFormat()
            }, frameGraphTexture);
        }
    }

    auto& ppColorGrading = fg.addPass<PostProcessColorGrading>("colorGrading",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const& inputDesc = fg.getDescriptor(input);
                data.input = builder.sample(input);
                data.output = builder.createTexture("colorGrading output", {
                        .width = inputDesc.width,
                        .height = inputDesc.height,
                        .format = outFormat
                });
                data.output = builder.write(data.output);
                data.rt = builder.createRenderTarget("colorGrading Target", {
                        .attachments = { data.output } });

                if (bloomBlur.isValid()) {
                    data.bloom = builder.sample(bloomBlur);
                }
                if (bloomDirt.isValid()) {
                    data.dirt = builder.sample(bloomDirt);
                }
            },
            [=](FrameGraphPassResources const& resources, auto const& data, DriverApi& driver) {
                Handle<HwTexture> colorTexture = resources.getTexture(data.input);

                Handle<HwTexture> bloomTexture =
                        data.bloom.isValid() ? resources.getTexture(data.bloom) : getZeroTexture();

                Handle<HwTexture> dirtTexture =
                        data.dirt.isValid() ? resources.getTexture(data.dirt) : getOneTexture();

                auto const& out = resources.get(data.rt);

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
                mi->setParameter("dirtBuffer", dirtTexture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });

                // Bloom params
                float4 bloomParameters{
                    bloom / float(bloomOptions.levels),
                    1.0f,
                    (bloomOptions.enabled && bloomOptions.dirt) ? bloomOptions.dirtStrength : 0.0f,
                    0.0f
                };
                if (bloomOptions.blendMode == View::BloomOptions::BlendMode::INTERPOLATE) {
                    bloomParameters.y = 1.0f - bloomParameters.x;
                }

                auto const& output = resources.getDescriptor(data.output);
                float4 vignetteParameters = getVignetteParameters(
                        vignetteOptions, output.width, output.height);

                const float temporalNoise = mUniformDistribution(mEngine.getRandomEngine());

                mi->setParameter("dithering", dithering);
                mi->setParameter("bloom", bloomParameters);
                mi->setParameter("vignette", vignetteParameters);
                mi->setParameter("vignetteColor", vignetteOptions.color);
                mi->setParameter("fxaa", fxaa);
                mi->setParameter("temporalNoise", temporalNoise);

                const uint8_t variant = uint8_t(translucent ?
                            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                commitAndRender(out, material, variant, driver);
            }
    );

    return ppColorGrading.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::fxaa(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        TextureFormat outFormat, bool translucent) noexcept {

    struct PostProcessFXAA {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle rt;
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
                data.output = builder.write(data.output);
                data.rt = builder.createRenderTarget("FXAA Target", {
                        .attachments = { data.output } });
            },
            [=](FrameGraphPassResources const& resources,
                auto const& data, DriverApi& driver) {
                auto const& texture = resources.getTexture(data.input);
                auto const& out = resources.get(data.rt);

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

    return ppFXAA.getData().output;
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
    if (UTILS_UNLIKELY(!entry.color.texture)) {
        // if we don't have a history yet, just use the current color buffer as history
        colorHistory = input;
        historyProjection = &frameHistory.getCurrent().projection;
    } else {
        colorHistory = fg.import("TAA history", entry.colorDesc, entry.color);
        historyProjection = &entry.projection;
    }

    Blackboard& blackboard = fg.getBlackboard();
    auto depth = blackboard.get<FrameGraphTexture>("depth");
    assert(depth.isValid());

    struct TAAData {
        FrameGraphId<FrameGraphTexture> color;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> history;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> tonemappedOutput;
        FrameGraphRenderTargetHandle rt;
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
                    data.tonemappedOutput = builder.write(data.tonemappedOutput);
                    data.output = builder.read(data.output);
                }
                data.rt = builder.createRenderTarget("TAA target", {
                        .attachments = {{ data.output, data.tonemappedOutput, {}, {}}, {}, {}}
                });
            },
            [=, &frameHistory](FrameGraphPassResources const& resources, auto const& data, DriverApi& driver) {

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

                auto out = resources.get(data.rt);
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
                    colorGradingSubpass(driver, colorGradingConfig.translucent);
                }
                driver.endRenderPass();

                // perform TAA here using colorHistory + input -> output
                resources.detach(data.output, &current.color, &current.colorDesc);
            });
    return colorGradingConfig.asSubpass ? taa.getData().tonemappedOutput : taa.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::opaqueBlit(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Descriptor outDesc,
        SamplerMagFilter filter) noexcept {

    struct PostProcessScaling {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle srt;
        FrameGraphRenderTargetHandle drt;
    };

    auto& ppBlit = fg.addPass<PostProcessScaling>("blit scaling",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const& inputDesc = fg.getDescriptor(input);

                // we currently have no use for this case, so we just assert. This is better for now to trap
                // cases that we might not intend.
                assert(inputDesc.samples <= 1);

                // FIXME: here we use sample() instead of read() because this forces the
                //      backend to use a texture (instead of a renderbuffer). We need this because
                //      "implicit resolve" renderbuffers are currently not supported -- and
                //      implicit resolves are needed when taking the blit path.
                //      (we do this only when the texture does not request multisampling, since
                //      these are not sampleable).
                data.input = (inputDesc.samples > 1) ? builder.read(input) : builder.sample(input);

                data.srt = builder.createRenderTarget(builder.getName(data.input), {
                        .attachments = { data.input },
                        // We must set the sample count (as opposed to leaving to 0) to express
                        // the fact that we want a new rendertarget (as opposed to match one
                        // that might exist with multisample enabled). This is because sample
                        // count is only matched if specified.
                        .samples = std::max(uint8_t(1), inputDesc.samples)
                });

                data.output = builder.createTexture("scaled output", outDesc);
                data.output = builder.write(data.output);
                data.drt = builder.createRenderTarget("Scaled Target", {
                        .attachments = { data.output } });
            },
            [=](FrameGraphPassResources const& resources, auto const& data, DriverApi& driver) {
                auto in = resources.get(data.srt);
                auto out = resources.get(data.drt);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, out.params.viewport, in.target, in.params.viewport, filter);
            });

    // we rely on automatic culling of unused render passes
    return ppBlit.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::blendBlit(
        FrameGraph& fg, bool translucent, View::QualityLevel quality,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphTexture::Descriptor outDesc) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle drt;
    };

    auto& ppQuadBlit = fg.addPass<QuadBlitData>("quad scaling",
            [&](FrameGraph::Builder& builder, auto& data) {
                data.input = builder.sample(input);
                data.output = builder.createTexture("scaled output", outDesc);
                data.output = builder.write(data.output);
                data.drt = builder.createRenderTarget("Scaled Target",{
                        .attachments = { data.output } });
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto color = resources.getTexture(data.input);
                auto out = resources.get(data.drt);
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
    return ppQuadBlit.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::resolve(FrameGraph& fg,
        const char* outputBufferName, FrameGraphId<FrameGraphTexture> input) noexcept {

    // Don't do anything if we're not a MSAA buffer
    auto desc = fg.getDescriptor(input);
    if (desc.samples <= 1) {
        return input;
    }

    struct ResolveData {
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle srt;
        FrameGraphRenderTargetHandle drt;
    };

    auto& ppResolve = fg.addPass<ResolveData>("resolve",
            [&](FrameGraph::Builder& builder, auto& data) {
                FrameGraphId<FrameGraphTexture> colorAttachmentSrc{};
                FrameGraphId<FrameGraphTexture> depthAttachmentSrc{};
                FrameGraphId<FrameGraphTexture> colorAttachmentDst{};
                FrameGraphId<FrameGraphTexture> depthAttachmentDst{};

                auto outputDesc = desc;
                input = builder.read(input);

                (isDepthFormat(desc.format) ? depthAttachmentSrc : colorAttachmentSrc) = input;
                data.srt = builder.createRenderTarget(builder.getName(input), {
                        .attachments = { colorAttachmentSrc, depthAttachmentSrc }, .samples = desc.samples });

                outputDesc.levels = 1;
                outputDesc.samples = 0;
                data.output = builder.createTexture(outputBufferName, outputDesc);
                data.output = builder.write(data.output);

                (isDepthFormat(desc.format) ? depthAttachmentDst : colorAttachmentDst) = data.output;
                data.drt = builder.createRenderTarget(outputBufferName, {
                        .attachments = { colorAttachmentDst, depthAttachmentDst } });
            },
            [](FrameGraphPassResources const& resources, auto const& data, DriverApi& driver) {
                auto in = resources.get(data.srt);
                auto out = resources.get(data.drt);
                driver.blit(TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH,
                        out.target, out.params.viewport, in.target, in.params.viewport,
                        SamplerMagFilter::NEAREST);
            });
    return ppResolve.getData().output;
}

} // namespace filament
