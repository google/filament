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
        const RenderTargetPool::Target* pos) const noexcept {
    FEngine& engine = *mEngine;
    DriverApi& driver = engine.getDriverApi();

    // FXAA requires linear filtering. The post-processing stage however, doesn't
    // use samplers.
    driver::SamplerParams params;
    params.filterMag = SamplerMagFilter::LINEAR;
    params.filterMin = SamplerMinFilter::LINEAR;
    SamplerBuffer sb(engine.getPostProcessSib());
    sb.setSampler(PostProcessSib::COLOR_BUFFER, pos->texture, params);

    auto duration = engine.getEngineTime();
    float fraction = (duration.count() % 1000000000) / 1000000000.0f;

    UniformBuffer& ub = mPostProcessUb;
    ub.setUniform(offsetof(PostProcessingUib, time), fraction);
    ub.setUniform(offsetof(PostProcessingUib, uvScale),
            math::float2{ viewportWidth, viewportHeight } / math::float2{ pos->w, pos->h });

    // The shader may need to know the offset between the top of the texture and the top
    // of the rectangle that it actually needs to sample from.
    const float yOffset = pos->h - viewportHeight;
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
            setSource(params.width, params.height, previous);

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

        setSource(params.width, params.height, previous);
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

} // namespace filament
