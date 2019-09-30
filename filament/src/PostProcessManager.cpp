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

#include "RenderPass.h"

#include "details/Camera.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "generated/resources/materials.h"

#include <private/filament/SibGenerator.h>

#include <filament/MaterialEnums.h>

#include <utils/Log.h>

namespace filament {

using namespace utils;
using namespace math;
using namespace backend;
using namespace filament::details;

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
    mBlur = PostProcessMaterial(mEngine, MATERIALS_BLUR_DATA, MATERIALS_BLUR_SIZE);
    mTonemapping = PostProcessMaterial(mEngine, MATERIALS_TONEMAPPING_DATA, MATERIALS_TONEMAPPING_SIZE);
    mFxaa = PostProcessMaterial(mEngine, MATERIALS_FXAA_DATA, MATERIALS_FXAA_SIZE);

    DriverApi& driver = mEngine.getDriverApi();
    mNoSSAOTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::R8, 0, 1, 1, 1, TextureUsage::DEFAULT);


    PixelBufferDescriptor data(driver.allocate(1), 1, PixelDataFormat::R, PixelDataType::UBYTE);
    auto p = static_cast<uint8_t *>(data.buffer);
    *p = 0xFFu;
    driver.update2DImage(mNoSSAOTexture, 0, 0, 0, 1, 1, std::move(data));
}

void PostProcessManager::terminate(backend::DriverApi& driver) noexcept {
    driver.destroyTexture(mNoSSAOTexture);
    mSSAO.terminate(mEngine);
    mMipmapDepth.terminate(mEngine);
    mBlur.terminate(mEngine);
    mTonemapping.terminate(mEngine);
    mFxaa.terminate(mEngine);
}

// ------------------------------------------------------------------------------------------------

FrameGraphId<FrameGraphTexture> PostProcessManager::toneMapping(FrameGraph& fg, FrameGraphId<FrameGraphTexture> input,
        backend::TextureFormat outFormat, bool dithering, bool translucent) noexcept {

    FEngine& engine = mEngine;
    backend::Handle<backend::HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    struct PostProcessToneMapping {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle rt;
    };

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
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessToneMapping const& data, DriverApi& driver) {
                auto const& color = resources.getTexture(data.input);

                FMaterialInstance* pInstance = mTonemapping.getMaterialInstance();
                pInstance->setParameter("dithering", dithering);
                pInstance->setParameter("colorBuffer", color, {});

                pInstance->commit(driver);

                const uint8_t variant = static_cast<uint8_t> (
                        translucent ?
                        PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE );

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
        backend::TextureFormat outFormat, bool translucent) noexcept {

    FEngine& engine = mEngine;
    backend::Handle<backend::HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

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

                const uint8_t variant = static_cast<uint8_t> (
                        translucent ?
                        PostProcessVariant::TRANSLUCENT : PostProcessVariant::OPAQUE );

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

FrameGraphId<FrameGraphTexture> PostProcessManager::resolve(
        FrameGraph& fg, FrameGraphId<FrameGraphTexture> input) noexcept {
    struct PostProcessResolve {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle srt;
        FrameGraphRenderTargetHandle drt;
    };

    auto& ppResolve = fg.addPass<PostProcessResolve>("resolve",
            [&](FrameGraph::Builder& builder, PostProcessResolve& data) {
                auto const& inputDesc = fg.getDescriptor(input);

                data.input = builder.read(input);
                data.srt = builder.createRenderTarget(builder.getName(data.input),
                           { .attachments = {data.input, {}}});

                data.output = builder.createTexture("resolve output", {
                        .width = inputDesc.width,
                        .height = inputDesc.height,
                        .format = inputDesc.format
                });
                data.drt = builder.createRenderTarget(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessResolve const& data, DriverApi& driver) {
                auto in = resources.getRenderTarget(data.srt);
                auto out = resources.getRenderTarget(data.drt);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, out.params.viewport, in.target, in.params.viewport,
                        SamplerMagFilter::LINEAR);
            });

    return ppResolve.getData().output;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::dynamicScaling(FrameGraph& fg,
        FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat) noexcept {

    struct PostProcessScaling {
        FrameGraphId<FrameGraphTexture> input;
        FrameGraphId<FrameGraphTexture> output;
        FrameGraphRenderTargetHandle srt;
        FrameGraphRenderTargetHandle drt;
    };

    auto& ppScaling = fg.addPass<PostProcessScaling>("scaling",
            [&](FrameGraph::Builder& builder, PostProcessScaling& data) {
                auto const& inputDesc = fg.getDescriptor(input);

                data.input = builder.read(input);
                FrameGraphRenderTarget::Descriptor d;
                d.attachments.color = { data.input };
                data.srt = builder.createRenderTarget(builder.getName(data.input), d);

                data.output = builder.createTexture("scale output", {
                        .width = inputDesc.width,
                        .height = inputDesc.height,
                        .format = outFormat
                });
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

    return ppScaling.getData().output;
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
                // Note that we're not clearing the SAO buffer, which will leave skipped pixels
                // in an undefined state -- this doesn't matter because the skybox material
                // doesn't use SSAO and the bilateral filter in the blur pass will ignore those
                // pixels at infinity.

                data.ssao = builder.write(data.ssao);
                data.depth = builder.sample(data.depth);

                data.rt = builder.createRenderTarget("SSAO Target",
                        { .attachments = { data.ssao, data.depth }
                        }, TargetBufferFlags::NONE);
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
                pInstance->setParameter("maxLevel", uint32_t(levelCount - 1));
                pInstance->commit(driver);

                PipelineState pipeline;
                pipeline.program = mSSAO.getProgram();
                pipeline.rasterState = mSSAO.getMaterial()->getRasterState();
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::G;
                pipeline.scissor = pInstance->getScissor();

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
    ssao = blurPass(fg, ssao, depth, {1, 0});

    // vertical separable blur pass
    ssao = blurPass(fg, ssao, depth, {0, 1});
    return ssao;
}

FrameGraphId<FrameGraphTexture> PostProcessManager::depthPass(FrameGraph& fg, RenderPass& pass,
        uint32_t width, uint32_t height,
        View::AmbientOcclusionOptions const& options) noexcept {

    // SSAO depth pass -- automatically culled if not used
    struct DepthPassData {
        FrameGraphId<FrameGraphTexture> depth;
        FrameGraphRenderTargetHandle rt;
    };

    RenderPass::Command const* first = pass.getCommands().begin();
    RenderPass::Command const* last = pass.getCommands().end();

    // sanitize a bit the user provided scaling factor
    const float scale = std::min(std::abs(options.resolution), 1.0f);
    width  = std::ceil(width * scale);
    height = std::ceil(height * scale);

    // We limit the level size to 32 pixels (which is where the -5 comes from)
    const size_t levelCount = std::max(1, std::ilogbf(std::max(width, height)) + 1 - 5);

    // SSAO generates its own depth path at 1/4 resolution
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
                data.rt = builder.createRenderTarget("SSAO Depth Target",
                                                     d,
                                                     TargetBufferFlags::DEPTH);
            },
            [=, &pass](FrameGraphPassResources const& resources,
                    DepthPassData const& data, DriverApi& driver) {
                auto out = resources.getRenderTarget(data.rt);
                pass.execute(resources.getPassName(), out.target, out.params, first, last);
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
                pInstance->setParameter("depth", in, {});
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

FrameGraphId<FrameGraphTexture> PostProcessManager::blurPass(FrameGraph& fg,
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

                FMaterialInstance* const pInstance = mBlur.getMaterialInstance();
                pInstance->setParameter("ssao", ssao, {});
                pInstance->setParameter("depth", depth, {});
                pInstance->setParameter("axis", axis);
                pInstance->setParameter("resolution",
                        float4{ desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height });
                pInstance->commit(driver);

                PipelineState pipeline;
                pipeline.program = mBlur.getProgram();
                pipeline.rasterState = mBlur.getMaterial()->getRasterState();
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::G;
                pipeline.scissor = pInstance->getScissor();

                driver.beginRenderPass(blurred.target, blurred.params);
                pInstance->use(driver);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return blurPass.getData().blurred;
}

} // namespace filament
