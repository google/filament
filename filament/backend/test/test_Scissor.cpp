/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "BackendTest.h"

#include "ShaderGenerator.h"
#include "TrianglePrimitive.h"

#include <utils/Hash.h>

namespace test {

using namespace filament;
using namespace filament::backend;

static const char* const triangleVs = R"(#version 450 core
layout(location = 0) in vec4 mesh_position;
void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
})";

static const char* const triangleFs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
void main() {
    fragColor = vec4(1.0f);
})";

TEST_F(BackendTest, ScissorViewportRegion) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 1024;
    constexpr int kSrcTexHeight = 1024;
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    constexpr int kNumLevels = 3;
    constexpr int kSrcLevel = 1;
    constexpr int kSrcRtWidth = 384;
    constexpr int kSrcRtHeight = 384;

    api.startCapture(0);

    //    color texture (mip level 1) 512x512           depth texture (mip level 0) 512x512
    // +----------------------------------------+   +------------------------------------------+
    // |                                        |   |                                          |
    // |                                        |   |                                          |
    // |     RenderTarget (384x384)             |   |   RenderTarget (384x384)                 |
    // +------------------------------+         |   +------------------------------+           |
    // |                              |         |   |                              |           |
    // |     +-------------------+    |         |   |                              |           |
    // |     |    viewport       |    |         |   |                              |           |
    // |     |                   |    |         |   |                              |           |
    // | +---+---------------+   |    |         |   |                              |           |
    // | |   |               |   |    |         |   |                              |           |
    // | |   |               |   |    |         |   |                              |           |
    // | |   | (64,64)       |   |    |         |   |                              |           |
    // | |   +---------------+---+    |         |   |                              |           |
    // | |  scissor          |        |         |   |                              |           |
    // | +-------------------+        |         |   |                              |           |
    // |  (32, 32)                    |         |   |                              |           |
    // +------------------------------+---------+   +------------------------------+-----------+

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
        auto swapChain = api.createSwapChainHeadless(256, 256, 0);
        api.makeCurrent(swapChain, swapChain);

        // Create a program.
        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);
        Program p = shaderGen.getProgram(api);
        ProgramHandle program = api.createProgram(std::move(p));

        // Create source color and depth textures.
        Handle<HwTexture> srcTexture = api.createTexture(SamplerType::SAMPLER_2D, kNumLevels,
                kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
                TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);
        Handle<HwTexture> depthTexture = api.createTexture(SamplerType::SAMPLER_2D, 1,
                TextureFormat::DEPTH16, 1, 512, 512, 1,
                TextureUsage::DEPTH_ATTACHMENT);

        // Render into the bottom-left quarter of the texture.
        Viewport srcRect = {
                .left = 64,
                .bottom = 64,
                .width = kSrcRtWidth - 64 * 2,
                .height = kSrcRtHeight - 64 * 2
        };
        Viewport scissor = {
                .left = 32,
                .bottom = 32,
                .width = kSrcRtWidth - 64 * 2,
                .height = kSrcRtHeight - 64 * 2
        };

        // We purposely set the render target width and height to smaller than the texture, to check
        // that this case is handled correctly.
        Handle<HwRenderTarget> srcRenderTarget = api.createRenderTarget(
                TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH, kSrcRtHeight, kSrcRtHeight, 1,
                {srcTexture, kSrcLevel, 0}, {depthTexture, 0, 0}, {});

        Handle<HwRenderTarget> fullRenderTarget = api.createRenderTarget(TargetBufferFlags::COLOR,
                kSrcTexHeight >> kSrcLevel, kSrcTexWidth >> kSrcLevel, 1,
                {srcTexture, kSrcLevel, 0}, {}, {});

        TrianglePrimitive triangle(api);

        // Render a white triangle over blue.
        RenderPassParams params = {};
        params.flags.clear = TargetBufferFlags::COLOR0;
        params.viewport = srcRect;
        params.clearColor = math::float4(0.0f, 0.0f, 1.0f, 1.0f);
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState ps = {};
        ps.program = program;
        ps.rasterState.colorWrite = true;
        ps.rasterState.depthWrite = false;

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0);

        api.beginRenderPass(srcRenderTarget, params);
        api.scissor(scissor);
        api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        readPixelsAndAssertHash("scissor", kSrcTexWidth >> 1, kSrcTexHeight >> 1, fullRenderTarget,
                0xAB3D1C53, true);

        api.commit(swapChain);
        api.endFrame(0);

        api.stopCapture(0);

        // Cleanup.
        api.destroyTexture(srcTexture);
        api.destroySwapChain(swapChain);
        api.destroyRenderTarget(srcRenderTarget);
    }

    // Wait for the ReadPixels result to come back.
    api.finish();

    executeCommands();
    getDriver().purge();
}

// Verify that a negative Viewport origin works with scissor.
TEST_F(BackendTest, ScissorViewportEdgeCases) {
    auto& api = getDriverApi();

    api.startCapture(0);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
        auto swapChain = api.createSwapChainHeadless(256, 256, 0);
        api.makeCurrent(swapChain, swapChain);

        // Create a program.
        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);
        Program p = shaderGen.getProgram(api);
        ProgramHandle program = api.createProgram(std::move(p));

        // Create a source color textures.
        Handle<HwTexture> srcTexture = api.createTexture(SamplerType::SAMPLER_2D, 1,
                TextureFormat::RGBA8, 1, 512, 512, 1,
                TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

        // Render into the bottom-left quarter of the texture, checking 3 special cases.
        // 1. negative viewport left/bottom
        Viewport bottomLeftViewport = {
                .left = -64,
                .bottom = -64,
                .width = 256,
                .height = 256
        };
        // 2. viewport that extends beyond the top of the render target.
        Viewport topLeftViewport = {
                .left = -64,
                .bottom = 320,
                .width = 256,
                .height = 256
        };
        // 3. A scissor rect > render target size.
        // This is precisely what Filament does to essentially convey "no scissor".
        Viewport scissor = {0, 0, (uint32_t)std::numeric_limits<int32_t>::max(),
                (uint32_t)std::numeric_limits<int32_t>::max()};

        Handle<HwRenderTarget> renderTarget = api.createRenderTarget(
                TargetBufferFlags::COLOR, 512, 512, 1,
                {srcTexture, 0, 0}, {}, {});

        TrianglePrimitive triangle(api);

        // Render a white triangle over blue.
        RenderPassParams params = {};
        params.flags.clear = TargetBufferFlags::COLOR0;
        params.viewport = bottomLeftViewport;
        params.clearColor = math::float4(0.0f, 0.0f, 1.0f, 1.0f);
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState ps = {};
        ps.program = program;
        ps.rasterState.colorWrite = true;
        ps.rasterState.depthWrite = false;

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0);

        api.beginRenderPass(renderTarget, params);
        api.scissor(scissor);
        api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        params.viewport = topLeftViewport;
        params.flags.clear = TargetBufferFlags::NONE;
        params.flags.discardStart = TargetBufferFlags::NONE;
        api.beginRenderPass(renderTarget, params);
        api.scissor(scissor);
        api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        readPixelsAndAssertHash(
                "ScissorViewportEdgeCases", 512, 512, renderTarget, 0x6BF00F31, true);

        api.commit(swapChain);
        api.endFrame(0);

        api.stopCapture(0);

        // Cleanup.
        api.destroyTexture(srcTexture);
        api.destroySwapChain(swapChain);
        api.destroyRenderTarget(renderTarget);
    }

    // Wait for the ReadPixels result to come back.
    api.finish();

    executeCommands();
    getDriver().purge();
}

} // namespace test
