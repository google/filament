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
#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "details/Texture.h"
#include "generated/resources/materials.h"

#include <private/filament/SibGenerator.h>

#include <filament/MaterialEnums.h>

#include <utils/Log.h>

namespace filament {

using namespace utils;
using namespace math;
using namespace backend;
using namespace filament::details;

// The blue noise texture data below is converted from the images provided for
// download by Christoph Peters under the Creative Commons CC0 Public
// Domain Dedication (CC0 1.0) license.
// See http://momentsingraphics.de/BlueNoise.html
const uint8_t kBlueNoise[] = {
    0x6f, 0x31, 0x8e, 0xa2, 0x71, 0xc3, 0x47, 0xb1, 0xc9, 0x32, 0x97, 0x5e, 0x42, 0x25, 0x55, 0xfc,
    0x19, 0x63, 0xef, 0xde, 0x20, 0xfa, 0x94, 0x13, 0x26, 0x6a, 0xdc, 0xaa, 0xc2, 0x8a, 0x0d, 0xa7,
    0x7d, 0xb2, 0x4f, 0x0f, 0x41, 0xad, 0x7b, 0x57, 0xd5, 0x83, 0xf7, 0x17, 0x74, 0x36, 0xe5, 0xd4,
    0x29, 0xca, 0x98, 0x84, 0xbd, 0x68, 0x35, 0xec, 0xa1, 0x3e, 0x01, 0xb5, 0x4d, 0xf1, 0x93, 0x44,
    0x02, 0xf4, 0x38, 0x5b, 0xe6, 0x05, 0xcc, 0x1c, 0xbb, 0x65, 0x90, 0xce, 0x21, 0x5c, 0xbe, 0x6b,
    0xdf, 0xa4, 0x72, 0x24, 0xd6, 0x9c, 0x8b, 0x46, 0xf5, 0x54, 0xe2, 0x30, 0x7e, 0x9e, 0x11, 0x87,
    0x53, 0xc4, 0x15, 0xfe, 0x4c, 0x2d, 0xb3, 0x73, 0x0c, 0x28, 0xa9, 0x69, 0xfd, 0xb0, 0xd3, 0x3b,
    0x64, 0xb4, 0x91, 0x7a, 0xac, 0x61, 0xeb, 0x81, 0xd7, 0x95, 0xc7, 0x08, 0x48, 0x1a, 0xee, 0x2c,
    0xe8, 0x1f, 0x45, 0x0b, 0xcd, 0x3a, 0x12, 0xc1, 0x58, 0x3c, 0x70, 0xdd, 0x8c, 0x56, 0x78, 0x99,
    0xd0, 0x82, 0xf3, 0xa0, 0xe0, 0x6e, 0x22, 0xf8, 0xa5, 0x18, 0xea, 0xb8, 0x34, 0xc6, 0xab, 0x06,
    0x6c, 0xbc, 0x33, 0x59, 0x89, 0xba, 0x9a, 0x4e, 0x2f, 0x86, 0x62, 0x9d, 0x23, 0xf9, 0x5f, 0x3f,
    0x10, 0x4b, 0xdb, 0x27, 0x00, 0x43, 0xe4, 0x79, 0xc5, 0xf0, 0x03, 0x4a, 0x7f, 0x14, 0xe3, 0x8f,
    0xf6, 0xaf, 0x77, 0xc8, 0xfb, 0x67, 0x92, 0x0e, 0xd1, 0xae, 0x6d, 0xda, 0xc0, 0x52, 0xcb, 0xa3,
    0x1d, 0x5d, 0x96, 0x16, 0xa6, 0xb6, 0x37, 0x1e, 0x5a, 0x40, 0x2a, 0x8d, 0xa8, 0x39, 0x75, 0x2e,
    0xd8, 0xe9, 0x3d, 0x80, 0x51, 0xed, 0xd9, 0x76, 0x9f, 0xff, 0xb9, 0x1b, 0xf2, 0x66, 0x04, 0x85,
    0x49, 0xbf, 0x09, 0xd2, 0x2b, 0x60, 0x07, 0x88, 0xe7, 0x50, 0x0a, 0x7c, 0xe1, 0xcf, 0x9b, 0xb7
};

static constexpr uint8_t kMaxBloomLevels = 12u;
static_assert(kMaxBloomLevels >= 3, "We require at least 3 bloom levels");

// ------------------------------------------------------------------------------------------------

PostProcessManager::PostProcessMaterial::PostProcessMaterial(FEngine& engine,
        uint8_t const* data, size_t size) noexcept {
    mMaterial = upcast(Material::Builder().package(data, size).build(engine));
    mMaterialInstance = mMaterial->getDefaultInstance();
    // TODO: After all materials using this class have been converted to the post-process material
    // domain, load both OPAQUE and TRANSPARENt variants here.
    mProgram = mMaterial->getProgram(0);
}

PostProcessManager::PostProcessMaterial::PostProcessMaterial(
        PostProcessManager::PostProcessMaterial&& rhs) noexcept {
    using namespace std;
    swap(mMaterial, rhs.mMaterial);
    swap(mMaterialInstance, rhs.mMaterialInstance);
    swap(mProgram, rhs.mProgram);
}

PostProcessManager::PostProcessMaterial& PostProcessManager::PostProcessMaterial::operator=(
        PostProcessManager::PostProcessMaterial&& rhs) noexcept {
    using namespace std;
    swap(mMaterial, rhs.mMaterial);
    swap(mMaterialInstance, rhs.mMaterialInstance);
    swap(mProgram, rhs.mProgram);
    return *this;
}

PostProcessManager::PostProcessMaterial::~PostProcessMaterial() {
    assert(mMaterial == nullptr);
}

void PostProcessManager::PostProcessMaterial::terminate(FEngine& engine) noexcept {
    engine.destroy(mMaterial);
    mMaterial = nullptr;
    mMaterialInstance = nullptr;
    mProgram.clear();
}

// ------------------------------------------------------------------------------------------------

PostProcessManager::PostProcessManager(FEngine& engine) noexcept : mEngine(engine) {
}

void PostProcessManager::init() noexcept {
    // TODO: load materials lazily as to reduce start-up time and memory usage
    mSSAO = PostProcessMaterial(mEngine, MATERIALS_SAO_DATA, MATERIALS_SAO_SIZE);
    mMipmapDepth = PostProcessMaterial(mEngine, MATERIALS_MIPMAPDEPTH_DATA, MATERIALS_MIPMAPDEPTH_SIZE);
    mBilateralBlur = PostProcessMaterial(mEngine, MATERIALS_BILATERALBLUR_DATA, MATERIALS_BILATERALBLUR_SIZE);
    mSeparableGaussianBlur = PostProcessMaterial(mEngine, MATERIALS_SEPARABLEGAUSSIANBLUR_DATA, MATERIALS_SEPARABLEGAUSSIANBLUR_SIZE);
    mBloomDownsample = PostProcessMaterial(mEngine, MATERIALS_BLOOMDOWNSAMPLE_DATA, MATERIALS_BLOOMDOWNSAMPLE_SIZE);
    mBloomUpsample = PostProcessMaterial(mEngine, MATERIALS_BLOOMUPSAMPLE_DATA, MATERIALS_BLOOMUPSAMPLE_SIZE);
    mBlit = PostProcessMaterial(mEngine, MATERIALS_BLIT_DATA, MATERIALS_BLIT_SIZE);
    mTonemapping = PostProcessMaterial(mEngine, MATERIALS_TONEMAPPING_DATA, MATERIALS_TONEMAPPING_SIZE);
    mFxaa = PostProcessMaterial(mEngine, MATERIALS_FXAA_DATA, MATERIALS_FXAA_SIZE);

    // UBO storage size.
    // The effective kernel size is (kMaxPositiveKernelSize - 1) * 4 + 1.
    // e.g.: 5 positive-side samples, give 4+1+4=9 samples both sides
    // taking advantage of linear filtering produces a effective kernel of 8+1+8=17 samples
    // and because it's a separable filter, the effective 2D filter kernel size is 17*17
    // The total number of samples needed over the two passes is 18.
    mSeparableGaussianBlurKernelStorageSize = mSeparableGaussianBlur.getMaterial()->reflect("kernel")->size;

    DriverApi& driver = mEngine.getDriverApi();
    mNoSSAOTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::R8, 0, 1, 1, 1, TextureUsage::DEFAULT);

    mNoiseTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGB16F, 0, 16, 16, 1, TextureUsage::DEFAULT);

    FMaterialInstance* const pInstance = mSSAO.getMaterialInstance();
    pInstance->setParameter("noise", mNoiseTexture, {});

    // Initialize the blue noise texture for SSAO
    // These should match the parameters used in ssaogen. We recompute the noise texture from the
    // blue noise data (instead of using ssaogen) to save space in this binary, as we only
    // need to store 256 bytes instead of 3 KiB.
    const float kSpiralTurns = 7.0;
    const size_t spiralSampleCount = 7;
    const size_t trigNoiseSampleCount = 256;
    const float dalpha = 1.0f / (spiralSampleCount - 0.5f);
    const size_t size = trigNoiseSampleCount * sizeof(float3);
    PixelBufferDescriptor noiseData(driver.allocate(size), size,
            PixelDataFormat::RGB, PixelDataType::HALF);
    half3* const noise = static_cast<half3 *>(noiseData.buffer);
    for (size_t i = 0; i < trigNoiseSampleCount; i++) {
        float phi = kBlueNoise[i] / 255.0f;
        float dr = phi * dalpha;
        float dphi = float(2.0 * F_PI * kSpiralTurns * phi * phi * dalpha * dalpha
                           + phi * 2.0 * F_PI * (1.0 + kSpiralTurns * dalpha * dalpha)
                           + phi * 4.0 * F_PI * kSpiralTurns * dalpha * dalpha);
        noise[i] = half3{ std::cos(dphi), std::sin(dphi), dr };
    }
    driver.update2DImage(mNoiseTexture, 0, 0, 0, 16, 16, std::move(noiseData));


    PixelBufferDescriptor data(driver.allocate(1), 1, PixelDataFormat::R, PixelDataType::UBYTE);
    auto p = static_cast<uint8_t *>(data.buffer);
    *p = 0xFFu;
    driver.update2DImage(mNoSSAOTexture, 0, 0, 0, 1, 1, std::move(data));
}

void PostProcessManager::terminate(DriverApi& driver) noexcept {
    driver.destroyTexture(mNoSSAOTexture);
    driver.destroyTexture(mNoiseTexture);
    FEngine& engine = mEngine;
    mSSAO.terminate(engine);
    mMipmapDepth.terminate(engine);
    mBilateralBlur.terminate(engine);
    mSeparableGaussianBlur.terminate(engine);
    mBloomDownsample.terminate(engine);
    mBloomUpsample.terminate(engine);
    mBlit.terminate(engine);
    mTonemapping.terminate(engine);
    mFxaa.terminate(engine);
}

// ------------------------------------------------------------------------------------------------

FrameGraphId<FrameGraphTexture> PostProcessManager::toneMapping(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        backend::TextureFormat outFormat, bool translucent, bool fxaa, float2 scale,
        View::BloomOptions bloomOptions, bool dithering) noexcept {

    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    struct PostProcessToneMapping {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphId<FrameGraphTexture> bloom;
        FrameGraphRenderTargetHandle rt;
    };

    FrameGraphId<FrameGraphTexture> bloomBlur;

    float bloom = 0.0f;
    if (bloomOptions.enabled) {
        bloom = clamp(bloomOptions.strength, 0.0f, 1.0f);
        bloomBlur = bloomPass(fg, input, TextureFormat::R11F_G11F_B10F, bloomOptions, scale);
    }

    auto& ppToneMapping = fg.addPass<PostProcessToneMapping>("tonemapping",
            [&](FrameGraph::Builder& builder, PostProcessToneMapping& data) {
                auto const& inputDesc = fg.getDescriptor(input);
                data.input = builder.sample(input);
                data.output = builder.createTexture("tonemapping output", {
                        .width = inputDesc.width,
                        .height = inputDesc.height,
                        .format = outFormat
                });
                data.rt = builder.createRenderTarget(data.output);

                if (!bloomBlur.isValid()) {
                    // we need a dummy texture
                    bloomBlur = builder.createTexture("dummy", {});
                }
                data.bloom = builder.sample(bloomBlur);
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessToneMapping const& data, DriverApi& driver) {
                auto const& colorTexture = resources.getTexture(data.input);
                auto const& bloomTexture = resources.getTexture(data.bloom);

                FMaterialInstance* pInstance = mTonemapping.getMaterialInstance();
                pInstance->setParameter("colorBuffer", colorTexture, { /* shader uses texelFetch */ });
                pInstance->setParameter("bloomBuffer", bloomTexture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR /* always read base level in shader */
                });

                float2 bloomParameter{ bloom / float(bloomOptions.levels), 1.0f };
                if (bloomOptions.blendMode == View::BloomOptions::BlendMode::INTERPOLATE) {
                    bloomParameter.y = 1.0f - bloomParameter.x;
                }

                pInstance->setParameter("dithering", dithering);
                pInstance->setParameter("bloom", bloomParameter);
                pInstance->setParameter("fxaa", fxaa);
                pInstance->commit(driver);

                const uint8_t variant = uint8_t(translucent ?
                            PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                PipelineState pipeline{
                        .program = mTonemapping.getMaterial()->getProgram(variant),
                        .rasterState = mTonemapping.getMaterial()->getRasterState(),
                        .scissor = pInstance->getScissor()
                };

                auto const& target = resources.getRenderTarget(data.rt);
                driver.beginRenderPass(target.target, target.params);
                pInstance->use(driver);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return ppToneMapping.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::fxaa(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        TextureFormat outFormat, bool translucent) noexcept {

    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    struct PostProcessFXAA {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle rt;
    };

    auto& ppFXAA = fg.addPass<PostProcessFXAA>("fxaa",
            [&](FrameGraph::Builder& builder, PostProcessFXAA& data) {
                auto const& inputDesc = fg.getDescriptor(input);
                data.input = builder.sample(input);
                data.output = builder.createTexture("fxaa output", {
                        .width = inputDesc.width,
                        .height = inputDesc.height,
                        .format = outFormat
                });
                data.rt = builder.createRenderTarget(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessFXAA const& data, DriverApi& driver) {
                auto const& texture = resources.getTexture(data.input);

                FMaterialInstance* pInstance = mFxaa.getMaterialInstance();
                pInstance->setParameter("colorBuffer", texture, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });

                pInstance->commit(driver);

                const uint8_t variant = uint8_t(translucent ?
                    PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE);

                PipelineState pipeline{
                        .program = mFxaa.getMaterial()->getProgram(variant),
                        .rasterState = mFxaa.getMaterial()->getRasterState(),
                        .scissor = pInstance->getScissor()
                };

                auto const& target = resources.getRenderTarget(data.rt);
                driver.beginRenderPass(target.target, target.params);
                pInstance->use(driver);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return ppFXAA.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::dynamicScaling(FrameGraph& fg, bool blend,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphTexture::Descriptor outDesc) noexcept {

    if (UTILS_UNLIKELY(blend)) {
        return quadBlit(fg, blend, input, outDesc.format);
    }

    struct PostProcessScaling {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle srt;
        FrameGraphRenderTargetHandle drt;
    };

    auto& ppBlitScaling = fg.addPass<PostProcessScaling>("blit scaling",
            [&](FrameGraph::Builder& builder, PostProcessScaling& data) {
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
                        .attachments = { data.input, {}},
                        // We must set the sample count (as opposed to leaving to 0) to express
                        // the fact that we want a new rendertarget (as opposed to match one
                        // that might exist with multisample enabled). This is because sample
                        // count is only matched if specified.
                        .samples = std::max(uint8_t(1), inputDesc.samples)
                });

                data.output = builder.createTexture("scaled output", outDesc);
                data.drt = builder.createRenderTarget(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessScaling const& data, DriverApi& driver) {
                auto in = resources.getRenderTarget(data.srt);
                auto out = resources.getRenderTarget(data.drt);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, out.params.viewport, in.target, in.params.viewport,
                        SamplerMagFilter::LINEAR);
            });

    // we rely on automatic culling of unused render passes
    return ppBlitScaling.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::quadBlit(FrameGraph& fg,
        bool blend, FrameGraphId<FrameGraphTexture> input, TextureFormat outFormat) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    struct QuadBlitData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle drt;
    };

    auto& ppQuadScaling = fg.addPass<QuadBlitData>("quad scaling",
            [&](FrameGraph::Builder& builder, auto& data) {
                auto const& inputDesc = fg.getDescriptor(input);
                data.input = builder.sample(input);
                data.output = builder.createTexture("scaled output", {
                        .width = inputDesc.width,
                        .height = inputDesc.height,
                        .format = outFormat
                });
                data.drt = builder.createRenderTarget(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {

                auto color = resources.getTexture(data.input);
                auto out = resources.getRenderTarget(data.drt);

                FMaterialInstance* const pInstance = mBlit.getMaterialInstance();
                pInstance->setParameter("color", color, SamplerParams{
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR
                });
                pInstance->commit(driver);

                PipelineState pipeline;
                pipeline.program = mBlit.getProgram();
                pipeline.rasterState = mBlit.getMaterial()->getRasterState();
                pipeline.scissor = pInstance->getScissor();
                if (blend) {
                    pipeline.rasterState.blendFunctionSrcRGB   = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionSrcAlpha = BlendFunction::ONE;
                    pipeline.rasterState.blendFunctionDstRGB   = BlendFunction::ONE_MINUS_SRC_ALPHA;
                    pipeline.rasterState.blendFunctionDstAlpha = BlendFunction::ONE_MINUS_SRC_ALPHA;
                }
                driver.beginRenderPass(out.target, out.params);
                pInstance->use(driver);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    // we rely on automatic culling of unused render passes
    return ppQuadScaling.getData().output;
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
                auto outputDesc = builder.getDescriptor(input);
                outputDesc.levels = 1;
                outputDesc.samples = 0;
                input = builder.read(input);
                FrameGraphRenderTarget::Descriptor d;
                d.attachments.color = { input };
                data.srt = builder.createRenderTarget(builder.getName(input), d);
                data.output = builder.createTexture(outputBufferName, outputDesc);
                data.drt = builder.createRenderTarget(data.output);
            },
            [](FrameGraphPassResources const& resources, auto const& data, DriverApi& driver) {
                auto in = resources.getRenderTarget(data.srt);
                auto out = resources.getRenderTarget(data.drt);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, out.params.viewport, in.target, in.params.viewport,
                        SamplerMagFilter::NEAREST);
            });
    return ppResolve.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::ssao(FrameGraph& fg, RenderPass& pass,
        filament::Viewport const& svp, CameraInfo const& cameraInfo,
        View::AmbientOcclusionOptions const& options) noexcept {

    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    /*
     * SSAO depth pass -- automatically culled if not used
     */

    FrameGraphId<FrameGraphTexture> depth = depthPass(fg, pass, svp.width, svp.height, options);

    /*
     * create depth mipmap chain
     */

    // The first mip already exists, so we process n-1 lods
    const size_t levelCount = fg.getDescriptor(depth).levels;
    for (size_t level = 0; level < levelCount - 1; level++) {
        depth = mipmapPass(fg, depth, level);
    }

    /*
     * Our main SSAO pass
     */

    struct SSAOPassData {
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> ssao;
        View::AmbientOcclusionOptions options;
        FrameGraphRenderTargetHandle rt;
    };

    auto& SSAOPass = fg.addPass<SSAOPassData>("SSAO Pass",
            [&](FrameGraph::Builder& builder, SSAOPassData& data) {

                data.options = options;

                auto const& desc = builder.getDescriptor(depth);
                data.depth = builder.sample(depth);

                data.ssao = builder.createTexture("SSAO Buffer", {
                        .width = desc.width, .height = desc.height,
                        .format = TextureFormat::R8 });

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // Note that we have to clear the SAO buffer because blended objects will end-up
                // reading into it even though they were not written in the depth buffer.
                // The bilateral filter in the blur pass will ignore pixels at infinity.

                data.ssao = builder.write(data.ssao);
                data.depth = builder.sample(data.depth);

                data.rt = builder.createRenderTarget("SSAO Target",
                        { .attachments = { data.ssao, data.depth }
                        }, TargetBufferFlags::COLOR);
            },
            [=](FrameGraphPassResources const& resources,
                    SSAOPassData const& data, DriverApi& driver) {
                auto depth = resources.getTexture(data.depth);
                auto ssao = resources.getRenderTarget(data.rt);
                auto const& desc = resources.getDescriptor(data.ssao);

                // estimate of the size in pixel of a 1m tall/wide object viewed from 1m away (i.e. at z=-1)
                const float projectionScale = std::min(
                        0.5f * cameraInfo.projection[0].x * desc.width,
                        0.5f * cameraInfo.projection[1].y * desc.height);

                FMaterialInstance* const pInstance = mSSAO.getMaterialInstance();
                pInstance->setParameter("depth", depth, {});
                pInstance->setParameter("resolution",
                        float4{ desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height });
                pInstance->setParameter("radius", data.options.radius);
                pInstance->setParameter("invRadiusSquared", 1.0f / (data.options.radius * data.options.radius));
                pInstance->setParameter("projectionScaleRadius", projectionScale * data.options.radius);
                pInstance->setParameter("bias", data.options.bias);
                pInstance->setParameter("power", data.options.power);
                pInstance->setParameter("intensity", std::max(0.0f, data.options.intensity));
                pInstance->setParameter("maxLevel", uint32_t(levelCount - 1));
                pInstance->commit(driver);

                PipelineState pipeline;
                pipeline.program = mSSAO.getProgram();
                pipeline.rasterState = mSSAO.getMaterial()->getRasterState();
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::G;
                pipeline.scissor = pInstance->getScissor();

                ssao.params.clearColor = 1.0f;
                driver.beginRenderPass(ssao.target, ssao.params);
                pInstance->use(driver);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    FrameGraphId<FrameGraphTexture> ssao = SSAOPass.getData().ssao;

    /*
     * Final separable blur pass
     */

    // horizontal separable blur pass
    ssao = bilateralBlurPass(fg, ssao, depth, { 1, 0 });

    // vertical separable blur pass
    ssao = bilateralBlurPass(fg, ssao, depth, { 0, 1 });
    return ssao;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::depthPass(FrameGraph& fg, RenderPass const& pass,
        uint32_t width, uint32_t height,
        View::AmbientOcclusionOptions const& options) noexcept {

    // SSAO depth pass -- automatically culled if not used
    struct DepthPassData {
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphRenderTargetHandle rt;
    };

    // sanitize a bit the user provided scaling factor
    const float scale = std::min(std::abs(options.resolution), 1.0f);
    width  = std::ceil(width * scale);
    height = std::ceil(height * scale);

    // We limit the level size to 32 pixels (which is where the -5 comes from)
    const size_t levelCount = std::max(1, std::ilogbf(std::max(width, height)) + 1 - 5);

    // SSAO generates its own depth pass at the requested resolution
    auto& ssaoDepthPass = fg.addPass<DepthPassData>("SSAO Depth Pass",
            [&](FrameGraph::Builder& builder, DepthPassData& data) {
                data.depth = builder.createTexture("Depth Buffer", {
                        .width = width, .height = height,
                        .levels = uint8_t(levelCount),
                        .format = TextureFormat::DEPTH24 });

                data.depth = builder.write(builder.read(data.depth));

                // nested designated initializers not in C++ standard: https://tinyurl.com/y6krwocx
                FrameGraphRenderTarget::Descriptor d;
                d.attachments.depth = data.depth;
                data.rt = builder.createRenderTarget("SSAO Depth Target", d,
                                                     TargetBufferFlags::DEPTH);
            },
            [pass](FrameGraphPassResources const& resources,
                    DepthPassData const& data, DriverApi& driver) {
                auto out = resources.getRenderTarget(data.rt);
                pass.execute(resources.getPassName(), out.target, out.params);
            });

    return ssaoDepthPass.getData().depth;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::mipmapPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, size_t level) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    struct DepthMipData {
        FrameGraphId<FrameGraphTexture> in;
        FrameGraphId<FrameGraphTexture> out;
        FrameGraphRenderTargetHandle rt;

    };

    auto& depthMipmapPass = fg.addPass<DepthMipData>("Depth Mipmap Pass",
            [&](FrameGraph::Builder& builder, DepthMipData& data) {
                const char* name = builder.getName(input);
                data.in = builder.sample(input);
                data.out = builder.write(data.in);
                FrameGraphRenderTarget::Descriptor d;
                d.attachments.depth = { data.out, uint8_t(level + 1) };
                data.rt = builder.createRenderTarget(name, d);
            },
            [=](FrameGraphPassResources const& resources,
                    DepthMipData const& data, DriverApi& driver) {

                auto in = resources.getTexture(data.in);
                auto out = resources.getRenderTarget(data.rt, level + 1u);

                FMaterialInstance* const pInstance = mMipmapDepth.getMaterialInstance();
                pInstance->setParameter("depth", in, { /* uses texelFetch */ });
                pInstance->setParameter("level", uint32_t(level));
                pInstance->commit(driver);

                PipelineState pipeline{
                    .program = mMipmapDepth.getProgram(),
                    .rasterState = mMipmapDepth.getMaterial()->getRasterState(),
                    .scissor = pInstance->getScissor()
                };

                driver.beginRenderPass(out.target, out.params);
                pInstance->use(driver);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return depthMipmapPass.getData().out;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::bilateralBlurPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input,
        FrameGraphId<FrameGraphTexture> depth, math::int2 axis) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    struct BlurPassData {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphId<FrameGraphTexture> blurred;
        FrameGraphRenderTargetHandle rt;
    };

    auto& blurPass = fg.addPass<BlurPassData>("Separable Blur Pass",
            [&](FrameGraph::Builder& builder, BlurPassData& data) {

                auto const& desc = builder.getDescriptor(input);

                data.input = builder.sample(input);
                data.depth = builder.sample(depth);

                data.blurred = builder.createTexture("Blurred output", {
                        .width = desc.width, .height = desc.height, .format = desc.format });

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // Note that we're not clearing the SAO buffer, which will leave skipped pixels
                // in an undefined state -- this doesn't matter because the skybox material
                // doesn't use SSAO.
                depth = builder.read(depth);
                data.blurred = builder.write(data.blurred);
                data.rt = builder.createRenderTarget("Blurred target",
                        { .attachments = { data.blurred, depth }
                        }, TargetBufferFlags::NONE);
            },
            [=](FrameGraphPassResources const& resources,
                    BlurPassData const& data, DriverApi& driver) {
                auto ssao = resources.getTexture(data.input);
                auto depth = resources.getTexture(data.depth);
                auto blurred = resources.getRenderTarget(data.rt);
                auto const& desc = resources.getDescriptor(data.blurred);

                // TODO: "oneOverEdgeDistance" should be a user-settable parameter
                //       z-distance that constitute an edge for bilateral filtering
                FMaterialInstance* const pInstance = mBilateralBlur.getMaterialInstance();
                pInstance->setParameter("ssao", ssao, {});
                pInstance->setParameter("depth", depth, {});
                pInstance->setParameter("axis", axis);
                pInstance->setParameter("oneOverEdgeDistance", 1.0f / 0.1f);
                pInstance->setParameter("resolution",
                        float4{ desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height });
                pInstance->commit(driver);

                PipelineState pipeline;
                pipeline.program = mBilateralBlur.getProgram();
                pipeline.rasterState = mBilateralBlur.getMaterial()->getRasterState();
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::G;
                pipeline.scissor = pInstance->getScissor();

                driver.beginRenderPass(blurred.target, blurred.params);
                pInstance->use(driver);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return blurPass.getData().blurred;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::generateGaussianMipmap(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, size_t roughnessLodCount,
        size_t kernelWidth, float sigmaRatio) noexcept {
    for (size_t i = 1; i < roughnessLodCount; i++) {
        input = gaussianBlurPass(fg, input, i - 1, input, i, kernelWidth, sigmaRatio);
    }
    return input;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::gaussianBlurPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, uint8_t srcLevel,
        FrameGraphId<FrameGraphTexture> output, uint8_t dstLevel,
        size_t kernelWidth, float sigmaRatio) noexcept {

    const float sigma = (kernelWidth + 1) / sigmaRatio;

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
                        .attachments = { data.temp, {}}
                }, TargetBufferFlags::NONE);

                data.outRT = builder.createRenderTarget("Blurred target", {
                        .attachments = {{ data.out, dstLevel }, {}}
                }, TargetBufferFlags::NONE);
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {

                PostProcessMaterial const& separableGaussianBlur = mSeparableGaussianBlur;
                FMaterialInstance* const mi = separableGaussianBlur.getMaterialInstance();

                PipelineState pipeline{
                        .program = separableGaussianBlur.getProgram(),
                        .rasterState = separableGaussianBlur.getMaterial()->getRasterState(),
                        .scissor = mi->getScissor()
                };

                float2 kernel[64];
                size_t m = computeGaussianCoefficients(kernel,
                        std::min(sizeof(kernel) / sizeof(*kernel), kernelStorageSize));

                // horizontal pass
                auto hwTempRT = resources.getRenderTarget(data.tempRT);
                auto hwOutRT = resources.getRenderTarget(data.outRT);
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
                mi->setParameter("resolution", float4{
                        tempDesc.width, tempDesc.height,
                        1.0f / tempDesc.width, 1.0f / tempDesc.height });
                mi->setParameter("axis",
                        float2{ 1.0f / FTexture::valueForLevel(srcLevel, inDesc.width), 0 });
                mi->setParameter("count", (int32_t)m);
                mi->setParameter("kernel", kernel, m);
                mi->commit(driver);
                mi->use(driver);

                // FIXME: see if the framegraph could figure this out
                hwTempRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                driver.beginRenderPass(hwTempRT.target, hwTempRT.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();

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

                driver.beginRenderPass(hwOutRT.target, hwOutRT.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return gaussianBlurPasses.getData().out;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::bloomPass(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
        View::BloomOptions& bloomOptions, float2 scale) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    // Figure out a good size for the bloom buffer. We pick the major axis lower
    // power of two, and scale the minor axis accordingly.
    auto const& desc = fg.getDescriptor(input);
    uint32_t width = desc.width / scale.x;
    uint32_t height = desc.height / scale.y;
    if (bloomOptions.anamorphism >= 1.0) {
        height *= bloomOptions.anamorphism;
    } else if (bloomOptions.anamorphism < 1.0) {
        width *= 1.0f / std::max(bloomOptions.anamorphism, 1.0f / 4096.0f);
    }
    // FIXME: compensate for dynamic scaling
    uint32_t& major = width > height ? width : height;
    uint32_t& minor = width < height ? width : height;
    uint32_t newMinor = clamp(bloomOptions.resolution,
            1u << bloomOptions.levels, std::min(minor, 1u << kMaxBloomLevels));
    major = major * uint64_t(newMinor) / minor;
    minor = newMinor;

    // we might need to adjust the max # of levels
    const uint8_t maxLevels = static_cast<uint8_t>(std::ilogbf(major) + 1);
    bloomOptions.levels = std::min(bloomOptions.levels, maxLevels);
    bloomOptions.levels = std::min(bloomOptions.levels, kMaxBloomLevels);

//    slog.d << desc.width << "x" << desc.height << " -> " << width << "x" << height
//           << ", levels=" << +bloomOptions.levels << io::endl;

    struct BloomPassData {
        FrameGraphId<FrameGraphTexture> in;
        FrameGraphId<FrameGraphTexture> out;
        FrameGraphRenderTargetHandle outRT[kMaxBloomLevels];
    };

    auto& bloomPass = fg.addPass<BloomPassData>("Gaussian Blur Passes",
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
                            .attachments = {{ data.out, uint8_t(i) }, {}}
                    }, TargetBufferFlags::NONE);
                }
            },
            [=](FrameGraphPassResources const& resources,
                    auto const& data, DriverApi& driver) {

                PostProcessMaterial const& bloomDownsample = mBloomDownsample;
                FMaterialInstance* mi = bloomDownsample.getMaterialInstance();

                PipelineState pipeline{
                        .program = bloomDownsample.getProgram(),
                        .rasterState = bloomDownsample.getMaterial()->getRasterState(),
                        .scissor = mi->getScissor(),
                };

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

                // downsample phase
                for (size_t i = 0; i < bloomOptions.levels; i++) {
                    auto hwOutRT = resources.getRenderTarget(data.outRT[i]);

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

                // upsample phase
                PostProcessMaterial const& bloomUpsample = mBloomUpsample;
                mi = bloomUpsample.getMaterialInstance();
                pipeline.program = bloomUpsample.getProgram();
                pipeline.rasterState = bloomUpsample.getMaterial()->getRasterState();
                pipeline.scissor = mi->getScissor();
                pipeline.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
                pipeline.rasterState.blendFunctionDstRGB = BlendFunction::ONE;

                mi->use(driver);

                for (size_t i = bloomOptions.levels - 1; i >= 1; i--) {
                    auto hwDstRT = resources.getRenderTarget(data.outRT[i - 1]);
                    hwDstRT.params.flags.discardStart = TargetBufferFlags::NONE; // because we'll blend
                    hwDstRT.params.flags.discardEnd = TargetBufferFlags::NONE;

                    auto w = FTexture::valueForLevel(i - 1, outDesc.width);
                    auto h = FTexture::valueForLevel(i - 1, outDesc.height);
                    mi->setParameter("resolution", float4{ w, h, 1.0f / w, 1.0f / h });
                    mi->setParameter("source", hwOut, {
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

    return bloomPass.getData().out;
}

} // namespace filament
