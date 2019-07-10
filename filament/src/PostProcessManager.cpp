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
    mPostProcessUb = UniformBuffer(PostProcessingUib::getUib().getSize());

    // TODO: load materials lazily as to reduce start-up time and memory usage
    mSSAO = PostProcessMaterial(mEngine, MATERIALS_SAO_DATA, MATERIALS_SAO_SIZE);
    mMipmapDepth = PostProcessMaterial(mEngine, MATERIALS_MIPMAPDEPTH_DATA, MATERIALS_MIPMAPDEPTH_SIZE);
    mBlur = PostProcessMaterial(mEngine, MATERIALS_BLUR_DATA, MATERIALS_BLUR_SIZE);

    // create sampler for post-process FBO
    DriverApi& driver = mEngine.getDriverApi();
    mPostProcessSbh = driver.createSamplerGroup(PostProcessSib::SAMPLER_COUNT);
    mPostProcessUbh = driver.createUniformBuffer(mPostProcessUb.getSize(),
            backend::BufferUsage::DYNAMIC);
    driver.bindSamplers(BindingPoints::POST_PROCESS, mPostProcessSbh);
    driver.bindUniformBuffer(BindingPoints::POST_PROCESS, mPostProcessUbh);

    mNoSSAOTexture = driver.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::R8, 0, 1, 1, 1, TextureUsage::DEFAULT);


    PixelBufferDescriptor data(driver.allocate(1), 1, PixelDataFormat::R, PixelDataType::UBYTE);
    auto p = static_cast<uint8_t *>(data.buffer);
    *p = 0xFFu;
    driver.update2DImage(mNoSSAOTexture, 0, 0, 0, 1, 1, std::move(data));
}

void PostProcessManager::terminate(backend::DriverApi& driver) noexcept {
    driver.destroySamplerGroup(mPostProcessSbh);
    driver.destroyUniformBuffer(mPostProcessUbh);
    driver.destroyTexture(mNoSSAOTexture);
    mSSAO.terminate(mEngine);
    mMipmapDepth.terminate(mEngine);
    mBlur.terminate(mEngine);
}

void PostProcessManager::setSource(uint32_t viewportWidth, uint32_t viewportHeight,
        backend::Handle<backend::HwTexture> color,
        backend::Handle<backend::HwTexture> depth,
        uint32_t textureWidth, uint32_t textureHeight) const noexcept {
    FEngine& engine = mEngine;
    DriverApi& driver = engine.getDriverApi();

    // FXAA requires linear filtering. The post-processing stage however, doesn't
    // use samplers.
    backend::SamplerParams params;
    params.filterMag = SamplerMagFilter::LINEAR;
    params.filterMin = SamplerMinFilter::LINEAR;
    SamplerGroup group(PostProcessSib::SAMPLER_COUNT);
    group.setSampler(PostProcessSib::COLOR_BUFFER, color, params);
    group.setSampler(PostProcessSib::DEPTH_BUFFER, depth, {});

    auto duration = engine.getEngineTime();
    float fraction = (duration.count() % 1000000000) / 1000000000.0f;

    float2 uvScale = float2{ viewportWidth, viewportHeight } / float2{ textureWidth, textureHeight };

    UniformBuffer& ub = mPostProcessUb;
    ub.setUniform(offsetof(PostProcessingUib, time), fraction);
    ub.setUniform(offsetof(PostProcessingUib, uvScale), uvScale);

    driver.updateSamplerGroup(mPostProcessSbh, std::move(group));
    driver.loadUniformBuffer(mPostProcessUbh, ub.toBufferDescriptor(driver));
}

// ------------------------------------------------------------------------------------------------

FrameGraphResource PostProcessManager::toneMapping(FrameGraph& fg, FrameGraphResource input,
        backend::TextureFormat outFormat, bool dithering, bool translucent) noexcept {

    FEngine& engine = mEngine;
    backend::Handle<backend::HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    struct PostProcessToneMapping {
        FrameGraphResource input;
        FrameGraphResource output;
    };
    backend::Handle<backend::HwProgram> toneMappingProgram = engine.getPostProcessProgram(
            translucent ? PostProcessStage::TONE_MAPPING_TRANSLUCENT
                        : PostProcessStage::TONE_MAPPING_OPAQUE);

    auto& ppToneMapping = fg.addPass<PostProcessToneMapping>("tonemapping",
            [&](FrameGraph::Builder& builder, PostProcessToneMapping& data) {
                auto const* inputDesc = fg.getDescriptor(input);
                data.input = builder.read(input);

                FrameGraphResource::Descriptor outputDesc{
                        .width = inputDesc->width,
                        .height = inputDesc->height,
                        .format = outFormat
                };
                data.output = builder.createTexture("tonemapping output", outputDesc);
                data.output = builder.useRenderTarget(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessToneMapping const& data, DriverApi& driver) {
                PipelineState pipeline;
                pipeline.rasterState.culling = RasterState::CullingMode::NONE;
                pipeline.rasterState.colorWrite = true;
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::A;
                pipeline.program = toneMappingProgram;

                auto const& textureDesc = resources.getDescriptor(data.input);
                auto const& color = resources.getTexture(data.input);
                // TODO: the first parameters below are the *actual viewport* size
                //       (as opposed to the size of the source texture). Currently we don't allow
                //       the texture to be resized, so they match. We'll need something more
                //       sophisticated in the future.

                mPostProcessUb.setUniform(offsetof(PostProcessingUib, dithering), dithering);
                setSource(textureDesc.width, textureDesc.height,
                        color, {}, textureDesc.width, textureDesc.height);

                auto const& target = resources.getRenderTarget(data.output);
                driver.beginRenderPass(target.target, target.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return ppToneMapping.getData().output;
}

FrameGraphResource PostProcessManager::fxaa(FrameGraph& fg,
        FrameGraphResource input, backend::TextureFormat outFormat, bool translucent) noexcept {

    FEngine& engine = mEngine;
    backend::Handle<backend::HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    struct PostProcessFXAA {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    backend::Handle<backend::HwProgram> antiAliasingProgram = engine.getPostProcessProgram(
            translucent ? PostProcessStage::ANTI_ALIASING_TRANSLUCENT
                        : PostProcessStage::ANTI_ALIASING_OPAQUE);

    auto& ppFXAA = fg.addPass<PostProcessFXAA>("fxaa",
            [&](FrameGraph::Builder& builder, PostProcessFXAA& data) {
                auto* inputDesc = fg.getDescriptor(input);
                data.input = builder.read(input);

                FrameGraphResource::Descriptor outputDesc{
                        .width = inputDesc->width,
                        .height = inputDesc->height,
                        .format = outFormat
                };
                data.output = builder.createTexture("fxaa output", outputDesc);
                data.output = builder.useRenderTarget(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessFXAA const& data, DriverApi& driver) {
                PipelineState pipeline;
                pipeline.rasterState.culling = RasterState::CullingMode::NONE;
                pipeline.rasterState.colorWrite = true;
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::A;
                pipeline.program = antiAliasingProgram;

                auto const& textureDesc = resources.getDescriptor(data.input);
                auto const& texture = resources.getTexture(data.input);
                // TODO: the first parameters below are the *actual viewport* size
                //       (as opposed to the size of the source texture). Currently we don't allow
                //       the texture to be resized, so they match. We'll need something more
                //       sophisticated in the future.
                setSource(textureDesc.width, textureDesc.height,
                        texture, {}, textureDesc.width, textureDesc.height);

                auto const& target = resources.getRenderTarget(data.output);
                driver.beginRenderPass(target.target, target.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return ppFXAA.getData().output;
}

FrameGraphResource PostProcessManager::resolve(
        FrameGraph& fg, FrameGraphResource input) noexcept {
    struct PostProcessResolve {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& ppResolve = fg.addPass<PostProcessResolve>("resolve",
            [&](FrameGraph::Builder& builder, PostProcessResolve& data) {
                auto* inputDesc = fg.getDescriptor(input);
                data.input = builder.useRenderTarget(builder.getName(input),
                        { .attachments.color = { input, FrameGraphRenderTarget::Attachments::READ },
                          .samples = builder.getSamples(input)
                        }).color;

                FrameGraphResource::Descriptor outputDesc{
                        .width = inputDesc->width,
                        .height = inputDesc->height,
                        .format = inputDesc->format
                };
                data.output = builder.createTexture("resolve output", outputDesc);
                data.output = builder.useRenderTarget(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessResolve const& data, DriverApi& driver) {
                auto in = resources.getRenderTarget(data.input);
                auto out = resources.getRenderTarget(data.output);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, out.params.viewport, in.target, in.params.viewport,
                        SamplerMagFilter::LINEAR);
            });

    return ppResolve.getData().output;
}

FrameGraphResource PostProcessManager::dynamicScaling(FrameGraph& fg,
        FrameGraphResource input, backend::TextureFormat outFormat) noexcept {

    struct PostProcessScaling {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& ppScaling = fg.addPass<PostProcessScaling>("scaling",
            [&](FrameGraph::Builder& builder, PostProcessScaling& data) {
                auto* inputDesc = fg.getDescriptor(input);
                data.input = builder.useRenderTarget(builder.getName(input),
                        { .attachments.color = { input, FrameGraphRenderTarget::Attachments::READ },
                          .samples = builder.getSamples(input)
                        }).color;

                FrameGraphResource::Descriptor outputDesc{
                        .width = inputDesc->width,
                        .height = inputDesc->height,
                        .format = outFormat
                };
                data.output = builder.createTexture("scale output", outputDesc);
                data.output = builder.useRenderTarget(data.output);
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessScaling const& data, DriverApi& driver) {
                auto in = resources.getRenderTarget(data.input);
                auto out = resources.getRenderTarget(data.output);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, out.params.viewport, in.target, in.params.viewport,
                        SamplerMagFilter::LINEAR);
            });

    return ppScaling.getData().output;
}


FrameGraphResource PostProcessManager::ssao(FrameGraph& fg, RenderPass& pass,
        filament::Viewport const& svp,
        CameraInfo const& cameraInfo, View::AmbientOcclusionOptions const& options) noexcept {

    FEngine& engine = mEngine;
    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();

    /*
     * SSAO depth pass -- automatically culled if not used
     */

    FrameGraphResource depth = depthPass(fg, pass, svp.width, svp.height, options);

    /*
     * create depth mipmap chain
     */

    // The first mip already exists, so we process n-1 lods
    const size_t levelCount = fg.getDescriptor(depth)->levels;
    for (size_t level = 0; level < levelCount - 1; level++) {
        depth = mipmapPass(fg, depth, level);
    }

    /*
     * Our main SSAO pass
     */

    struct SSAOPassData {
        FrameGraphResource depth;
        FrameGraphResource ssao;
        View::AmbientOcclusionOptions options;
    };

    auto& SSAOPass = fg.addPass<SSAOPassData>("SSAO Pass",
            [&](FrameGraph::Builder& builder, SSAOPassData& data) {

                data.options = options;

                auto const& desc = builder.getDescriptor(depth);
                data.depth = builder.read(depth);

                data.ssao = builder.createTexture("SSAO Buffer", {
                        .width = desc.width, .height = desc.height,
                        .format = TextureFormat::R8 });

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // Note that we're not clearing the SAO buffer, which will leave skipped pixels
                // in an undefined state -- this doesn't matter because the skybox material
                // doesn't use SSAO and the bilateral filter in the blur pass will ignore those
                // pixels at infinity.
                data.ssao = builder.useRenderTarget("SSAO Target",
                        { .attachments.color = { data.ssao, FrameGraphRenderTarget::Attachments::WRITE },
                          .attachments.depth = { data.depth, FrameGraphRenderTarget::Attachments::READ }
                        }, TargetBufferFlags::NONE).color;
            },
            [=](FrameGraphPassResources const& resources,
                    SSAOPassData const& data, DriverApi& driver) {
                auto depth = resources.getTexture(data.depth);
                auto ssao = resources.getRenderTarget(data.ssao);
                auto const& desc = resources.getDescriptor(data.ssao);

                // estimate of the size in pixel of a 1m tall/wide object viewed from 1m away (i.e. at z=-1)
                const float projectionScale = std::min(
                        0.5f * cameraInfo.projection[0].x * desc.width,
                        0.5f * cameraInfo.projection[1].y * desc.height);

                SamplerParams params;
                FMaterialInstance* const pInstance = mSSAO.getMaterialInstance();
                pInstance->setParameter("depth", depth, params);
                pInstance->setParameter("resolution",
                        float4{ desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height });
                pInstance->setParameter("radius", data.options.radius);
                pInstance->setParameter("invRadiusSquared", 1.0f / (data.options.radius * data.options.radius));
                pInstance->setParameter("projectionScaleRadius", projectionScale * data.options.radius);
                pInstance->setParameter("bias", data.options.bias);
                pInstance->setParameter("power", data.options.power);
                pInstance->setParameter("maxLevel", uint32_t(levelCount - 1));
                pInstance->commit(driver);
                pInstance->use(driver);

                PipelineState pipeline;
                pipeline.program = mSSAO.getProgram();
                pipeline.rasterState = mSSAO.getMaterial()->getRasterState();
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::G;

                driver.beginRenderPass(ssao.target, ssao.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    FrameGraphResource ssao = SSAOPass.getData().ssao;

    /*
     * Final separable blur pass
     */

    // horizontal separable blur pass
    ssao = blurPass(fg, ssao, depth, {1, 0});

    // vertical separable blur pass
    ssao = blurPass(fg, ssao, depth, {0, 1});
    return ssao;
}

FrameGraphResource PostProcessManager::depthPass(FrameGraph& fg, RenderPass& pass,
        uint32_t width, uint32_t height,
        View::AmbientOcclusionOptions const& options) noexcept {

    // SSAO depth pass -- automatically culled if not used
    struct DepthPassData {
        FrameGraphResource depth;
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

                data.depth = builder.useRenderTarget("SSAO Depth Target",
                        { .attachments.depth = data.depth },
                        TargetBufferFlags::DEPTH).depth;
            },
            [=, &pass](FrameGraphPassResources const& resources,
                    DepthPassData const& data, DriverApi& driver) {
                auto out = resources.getRenderTarget(data.depth);
                pass.execute(resources.getPassName(), out.target, out.params, first, last);
            });

    return ssaoDepthPass.getData().depth;
}

FrameGraphResource PostProcessManager::mipmapPass(FrameGraph& fg,
        FrameGraphResource input, size_t level) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    struct DepthMipData {
        FrameGraphResource in;
        FrameGraphResource out;
    };

    auto& depthMipmapPass = fg.addPass<DepthMipData>("Depth Mipmap Pass",
            [&](FrameGraph::Builder& builder, DepthMipData& data) {
                const char* name = builder.getName(input);
                data.in = builder.useRenderTarget(name, {
                        .attachments.depth = {
                                input, uint8_t(level), FrameGraphRenderTarget::Attachments::READ
                        }}).depth;

                data.out = builder.useRenderTarget(name, {
                        .attachments.depth = {
                                input, uint8_t(level + 1), FrameGraphRenderTarget::Attachments::WRITE
                        }}).depth;
            },
            [=](FrameGraphPassResources const& resources,
                    DepthMipData const& data, DriverApi& driver) {

                auto in = resources.getTexture(data.in);
                auto out = resources.getRenderTarget(data.out, level + 1u);

                SamplerParams params;
                FMaterialInstance* const pInstance = mMipmapDepth.getMaterialInstance();
                pInstance->setParameter("depth", in, params);
                pInstance->setParameter("level", uint32_t(level));
                pInstance->commit(driver);
                pInstance->use(driver);

                PipelineState pipeline;
                pipeline.program = mMipmapDepth.getProgram();
                pipeline.rasterState = mMipmapDepth.getMaterial()->getRasterState();

                driver.beginRenderPass(out.target, out.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return depthMipmapPass.getData().out;
}

FrameGraphResource PostProcessManager::blurPass(FrameGraph& fg, FrameGraphResource input,
        FrameGraphResource depth, math::int2 axis) noexcept {

    Handle<HwRenderPrimitive> fullScreenRenderPrimitive = mEngine.getFullScreenRenderPrimitive();

    struct BlurPassData {
        FrameGraphResource input;
        FrameGraphResource depth;
        FrameGraphResource blurred;
    };

    auto& blurPass = fg.addPass<BlurPassData>("Separable Blur Pass",
            [&](FrameGraph::Builder& builder, BlurPassData& data) {

                auto const& desc = builder.getDescriptor(input);

                data.input = builder.read(input);
                data.depth = builder.read(depth);

                data.blurred = builder.createTexture("Blurred output", {
                        .width = desc.width, .height = desc.height, .format = desc.format });

                // Here we use the depth test to skip pixels at infinity (i.e. the skybox)
                // Note that we're not clearing the SAO buffer, which will leave skipped pixels
                // in an undefined state -- this doesn't matter because the skybox material
                // doesn't use SSAO.
                data.blurred = builder.useRenderTarget("Blurred target",
                        { .attachments.color = { data.blurred, FrameGraphRenderTarget::Attachments::WRITE },
                          .attachments.depth = { depth, FrameGraphRenderTarget::Attachments::READ }
                        }, TargetBufferFlags::NONE).color;
            },
            [=](FrameGraphPassResources const& resources,
                    BlurPassData const& data, DriverApi& driver) {
                auto ssao = resources.getTexture(data.input);
                auto depth = resources.getTexture(data.depth);
                auto blurred = resources.getRenderTarget(data.blurred);
                auto const& desc = resources.getDescriptor(data.blurred);

                SamplerParams params;
                FMaterialInstance* const pInstance = mBlur.getMaterialInstance();
                pInstance->setParameter("ssao", ssao, params);
                pInstance->setParameter("depth", depth, params);
                pInstance->setParameter("axis", axis);
                pInstance->setParameter("resolution",
                        float4{ desc.width, desc.height, 1.0f / desc.width, 1.0f / desc.height });
                pInstance->commit(driver);
                pInstance->use(driver);

                PipelineState pipeline;
                pipeline.program = mBlur.getProgram();
                pipeline.rasterState = mBlur.getMaterial()->getRasterState();
                pipeline.rasterState.depthFunc = RasterState::DepthFunc::G;

                driver.beginRenderPass(blurred.target, blurred.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return blurPass.getData().blurred;
}

} // namespace filament
