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
#include "RenderTargetPool.h"

#include "details/Engine.h"

#include "fg/FrameGraph.h"

#include <private/filament/SibGenerator.h>

#include <utils/Log.h>

namespace filament {

using namespace utils;
using namespace driver;
using namespace details;

void PostProcessManager::init(FEngine& engine) noexcept {
    mEngine = &engine;

    mCommands.reserve(8);

    mPostProcessUb = UniformBuffer(engine.getPerPostProcessUib());

    // create sampler for post-process FBO
    DriverApi& driver = engine.getDriverApi();
    mPostProcessSbh = driver.createSamplerBuffer(engine.getPostProcessSib().getSize());
    mPostProcessUbh = driver.createUniformBuffer(engine.getPerPostProcessUib().getSize(),
            driver::BufferUsage::DYNAMIC);
    driver.bindSamplers(BindingPoints::POST_PROCESS, mPostProcessSbh);
    driver.bindUniformBuffer(BindingPoints::POST_PROCESS, mPostProcessUbh);
}

void PostProcessManager::terminate(driver::DriverApi& driver) noexcept {
    driver.destroySamplerBuffer(mPostProcessSbh);
    driver.destroyUniformBuffer(mPostProcessUbh);
}

void PostProcessManager::setSource(uint32_t viewportWidth, uint32_t viewportHeight,
        Handle<HwTexture> texture, uint32_t textureWidth, uint32_t textureHeight) const noexcept {
    FEngine& engine = *mEngine;
    DriverApi& driver = engine.getDriverApi();

    // FXAA requires linear filtering. The post-processing stage however, doesn't
    // use samplers.
    driver::SamplerParams params;
    params.filterMag = SamplerMagFilter::LINEAR;
    params.filterMin = SamplerMinFilter::LINEAR;
    SamplerBuffer sb(engine.getPostProcessSib());
    sb.setSampler(PostProcessSib::COLOR_BUFFER, texture, params);

    auto duration = engine.getEngineTime();
    float fraction = (duration.count() % 1000000000) / 1000000000.0f;

    UniformBuffer& ub = mPostProcessUb;
    ub.setUniform(offsetof(PostProcessingUib, time), fraction);
    ub.setUniform(offsetof(PostProcessingUib, uvScale),
            filament::math::float2{ viewportWidth, viewportHeight } / filament::math::float2{ textureWidth, textureHeight });

    // The shader may need to know the offset between the top of the texture and the top
    // of the rectangle that it actually needs to sample from.
    const float yOffset = textureHeight - viewportHeight;
    ub.setUniform(offsetof(PostProcessingUib, yOffset), yOffset);

    driver.updateSamplerBuffer(mPostProcessSbh, std::move(sb));
    driver.updateUniformBuffer(mPostProcessUbh, ub.toBufferDescriptor(driver));
}

void PostProcessManager::blit(driver::TextureFormat format) noexcept {
    mCommands.push_back({{}, format});
}

void PostProcessManager::pass(driver::TextureFormat format, Handle<HwProgram> program) noexcept {
    mCommands.push_back({program, format});
}

void PostProcessManager::finish(driver::TargetBufferFlags discarded,
        Handle<HwRenderTarget> viewRenderTarget,
        Viewport const& vp,
        RenderTargetPool::Target const* previous,
        Viewport const& svp) {

    assert(viewRenderTarget);
    assert(previous);

    FEngine& engine = *mEngine;
    DriverApi& driver = engine.getDriverApi();
    RenderTargetPool& rtp = engine.getRenderTargetPool();
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine.getFullScreenRenderPrimitive();
    std::vector<Command>& commands = mCommands;

    if (UTILS_UNLIKELY(commands.empty())) {
        rtp.put(previous);
        return;
    }

    Driver::PipelineState pipeline;

    pipeline.rasterState.culling = Driver::RasterState::CullingMode::NONE;
    pipeline.rasterState.colorWrite = true;
    pipeline.rasterState.depthFunc = Driver::RasterState::DepthFunc::A;

    RenderPassParams params = {};
    params.discardStart = TargetBufferFlags::ALL;
    params.discardEnd = TargetBufferFlags::DEPTH_AND_STENCIL;
    params.left = 0;
    params.bottom = 0;
    params.width = svp.width;
    params.height = svp.height;
    params.dependencies = RenderPassParams::DEPENDENCY_BY_REGION;

    for (size_t i = 0, c = commands.size() - 1; i < c; i++) {
        // if the next command is a blit, it we don't need a texture
        uint8_t flags = !commands[i + 1].program ? RenderTargetPool::Target::NO_TEXTURE : uint8_t();

        // create a render target for this pass
        RenderTargetPool::Target const* target = rtp.get(
                TargetBufferFlags::COLOR, svp.width, svp.height, 1, commands[i].format, flags);

        assert(target);

        if (commands[i].program) {
            // set the source for this pass (i.e. previous target)
            setSource(params.width, params.height, previous->texture, previous->w, previous->h);

            // draw a full screen triangle
            pipeline.program = commands[i].program;
            driver.beginRenderPass(target->target, params);
            driver.draw(pipeline, fullScreenRenderPrimitive);
            driver.endRenderPass();
        } else {
            driver.blit(TargetBufferFlags::COLOR,
                    target->target, 0, 0, svp.width, svp.height,
                    previous->target, 0, 0, svp.width, svp.height);
        }
        // return the previous target to the pool
        rtp.put(previous);
        previous = target;
    }

    assert(!commands.empty());
    assert(previous);

    // The last command is special, it always draw to the viewRenderTarget and uses
    // the non scaled viewport.
    if (commands.back().program) {
        params.discardStart = discarded;
        params.discardEnd = TargetBufferFlags::DEPTH_AND_STENCIL;
        params.left = vp.left;
        params.bottom = vp.bottom;
        params.width = vp.width;
        params.height = vp.height;

        setSource(params.width, params.height, previous->texture, previous->w, previous->h);
        pipeline.program = commands.back().program;
        driver.beginRenderPass(viewRenderTarget, params);
        driver.draw(pipeline, fullScreenRenderPrimitive);
        driver.endRenderPass();

    } else {
        driver.blit(TargetBufferFlags::COLOR,
                viewRenderTarget, vp.left, vp.bottom, vp.width, vp.height,
                previous->target, 0, 0, svp.width, svp.height);
    }

    rtp.put(previous);

    // clear our command buffer
    commands.clear();
}


// ------------------------------------------------------------------------------------------------

FrameGraphResource PostProcessManager::msaa(FrameGraph& fg,
        FrameGraphResource input, driver::TextureFormat outFormat) noexcept {

    struct PostProcessMSAA {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& ppMSAA = fg.addPass<PostProcessMSAA>("msaa",
            [&](FrameGraph::Builder& builder, PostProcessMSAA& data) {
                auto const* inputDesc = fg.getDescriptor(input);
                data.input = builder.blit(input);

                FrameGraphResource::Descriptor outputDesc{
                        .width = inputDesc->width,
                        .height = inputDesc->height,
                        .format = outFormat
                };
                data.output = builder.write(builder.createResource("msaa output", outputDesc));
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessMSAA const& data, DriverApi& driver) {
                auto in = resources.getRenderTarget(data.input);
                auto out = resources.getRenderTarget(data.output);
                auto const& desc = resources.getDescriptor(data.input);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, 0, 0, desc.width, desc.height,
                        in.target, 0, 0, desc.width, desc.height);
            });

    return ppMSAA.getData().output;
}

FrameGraphResource PostProcessManager::toneMapping(FrameGraph& fg,
        FrameGraphResource input, driver::TextureFormat outFormat, bool translucent) noexcept {

    FEngine* engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine->getFullScreenRenderPrimitive();

    struct PostProcessToneMapping {
        FrameGraphResource input;
        FrameGraphResource output;
    };
    Handle<HwProgram> toneMappingProgram = engine->getPostProcessProgram(
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
                data.output = builder.write(
                        builder.createResource("tonemapping output", outputDesc));
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessToneMapping const& data, DriverApi& driver) {
                Driver::PipelineState pipeline;
                pipeline.rasterState.culling = Driver::RasterState::CullingMode::NONE;
                pipeline.rasterState.colorWrite = true;
                pipeline.rasterState.depthFunc = Driver::RasterState::DepthFunc::A;
                pipeline.program = toneMappingProgram;

                auto const& targetDesc = resources.getDescriptor(data.output);
                auto const& textureDesc = resources.getDescriptor(data.input);
                auto const& texture = resources.getTexture(data.input, TextureUsage::COLOR_ATTACHMENT);
                setSource(targetDesc.width, targetDesc.height, texture, textureDesc.width, textureDesc.height);

                auto const& target = resources.getRenderTarget(data.output);
                driver.beginRenderPass(target.target, target.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return ppToneMapping.getData().output;
}

FrameGraphResource PostProcessManager::fxaa(FrameGraph& fg,
        FrameGraphResource input, driver::TextureFormat outFormat, bool translucent) noexcept {

    FEngine* engine = mEngine;
    Handle<HwRenderPrimitive> const& fullScreenRenderPrimitive = engine->getFullScreenRenderPrimitive();

    struct PostProcessFXAA {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    Handle<HwProgram> antiAliasingProgram = engine->getPostProcessProgram(
            translucent ? PostProcessStage::ANTI_ALIASING_TRANSLUCENT
                        : PostProcessStage::ANTI_ALIASING_OPAQUE);

    auto& ppFXAA = fg.addPass<PostProcessFXAA>("fxaa",
            [&](FrameGraph::Builder& builder, PostProcessFXAA& data) {
                auto* inputDesc = fg.getDescriptor(input);
                inputDesc->format = TextureFormat::RGBA8;
                data.input = builder.read(input);

                FrameGraphResource::Descriptor outputDesc{
                        .width = inputDesc->width,
                        .height = inputDesc->height,
                        .format = outFormat
                };
                data.output = builder.write(builder.createResource("fxaa output", outputDesc));
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessFXAA const& data, DriverApi& driver) {
                Driver::PipelineState pipeline;
                pipeline.rasterState.culling = Driver::RasterState::CullingMode::NONE;
                pipeline.rasterState.colorWrite = true;
                pipeline.rasterState.depthFunc = Driver::RasterState::DepthFunc::A;
                pipeline.program = antiAliasingProgram;

                auto const& targetDesc = resources.getDescriptor(data.output);
                auto const& textureDesc = resources.getDescriptor(data.input);
                auto const& texture = resources.getTexture(data.input, TextureUsage::COLOR_ATTACHMENT);
                setSource(targetDesc.width, targetDesc.height, texture, textureDesc.width, textureDesc.height);

                auto const& target = resources.getRenderTarget(data.output);
                driver.beginRenderPass(target.target, target.params);
                driver.draw(pipeline, fullScreenRenderPrimitive);
                driver.endRenderPass();
            });

    return ppFXAA.getData().output;
}

FrameGraphResource PostProcessManager::dynamicScaling(FrameGraph& fg,
        FrameGraphResource input, driver::TextureFormat outFormat,
        Viewport const& outViewport) noexcept {

    struct PostProcessScaling {
        FrameGraphResource input;
        FrameGraphResource output;
    };

    auto& ppScaling = fg.addPass<PostProcessScaling>("scaling",
            [&](FrameGraph::Builder& builder, PostProcessScaling& data) {
                auto* inputDesc = fg.getDescriptor(input);
                data.input = builder.blit(input);

                FrameGraphResource::Descriptor outputDesc{
                        .width = inputDesc->width,
                        .height = inputDesc->height,
                        .format = outFormat
                };
                data.output = builder.write(builder.createResource("scale output", outputDesc));
            },
            [=](FrameGraphPassResources const& resources,
                    PostProcessScaling const& data, DriverApi& driver) {
                auto in = resources.getRenderTarget(data.input);
                auto out = resources.getRenderTarget(data.output);
                auto const& inDesc = resources.getDescriptor(data.input);
                driver.blit(TargetBufferFlags::COLOR,
                        out.target, outViewport.left, outViewport.bottom, outViewport.width,
                        outViewport.height,
                        in.target, 0, 0, inDesc.width, inDesc.height);
            });

    return ppScaling.getData().output;
}


} // namespace filament
