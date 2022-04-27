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

using namespace filament;
using namespace filament::backend;

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string vertex (R"(#version 450 core

layout(location = 0) in vec4 mesh_position;

void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
}
)");

std::string fragment (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(1.0);
}

)");

}

namespace test {

// 1. Clear the stencil buffer to all zeroes.
// 2. Render a small triangle only to the stencil buffer and set the operation to write ones.
// 3. Enable a "== 0" stencil test and enable color writes.
// 4. Render a larger triangle to the SwapChain.
// 5. The larger triangle should have a "hole" in the middle.
TEST_F(BackendTest, StencilBuffer) {
    auto& api = getDriverApi();

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a platform-specific SwapChain and make it current.
        auto swapChain = createSwapChain();
        api.makeCurrent(swapChain, swapChain);

        // Create a program.
        ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
        Program p = shaderGen.getProgram();
        ProgramHandle program = api.createProgram(std::move(p));

        // We'll be using a triangle as geometry.
        TrianglePrimitive smallTriangle(api);
        static filament::math::float2 vertices[3] = {
                { -0.5, -0.5 },
                {  0.5, -0.5 },
                { -0.5,  0.5 }
        };
        smallTriangle.updateVertices(vertices);
        TrianglePrimitive triangle(api);

        // Create two textures: a color and a stencil, and an associated RenderTarget.
        auto colorTexture = getDriverApi().createTexture(SamplerType::SAMPLER_2D, 1,
                TextureFormat::RGBA8, 1, 512, 512, 1, TextureUsage::COLOR_ATTACHMENT);
        auto stencilTexture = getDriverApi().createTexture(SamplerType::SAMPLER_2D, 1,
                TextureFormat::STENCIL8, 1, 512, 512, 1, TextureUsage::STENCIL_ATTACHMENT);
        auto renderTarget = getDriverApi().createRenderTarget(
                TargetBufferFlags::COLOR0 | TargetBufferFlags::STENCIL, 512, 512, 1,
                {{colorTexture}}, {}, {{stencilTexture}});

        // Step 1: Clear the stencil buffer to all zeroes and the color buffer to blue.
        // Render a small triangle only to the stencil buffer, increasing the stencil buffer to 1.
        RenderPassParams params = {};
        params.flags.clear = TargetBufferFlags::COLOR0 | TargetBufferFlags::STENCIL;
        params.viewport = {0, 0, 512, 512};
        params.clearColor = math::float4(0.0f, 0.0f, 1.0f, 1.0f);
        params.clearStencil = 0u;
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState ps = {};
        ps.program = program;
        ps.rasterState.colorWrite = false;
        ps.rasterState.depthWrite = false;
        ps.rasterState.stencilWrite = true;
        ps.rasterState.stencilOpDpPass = StencilOperation::INCR;

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0);

        api.beginRenderPass(renderTarget, params);
        api.draw(ps, smallTriangle.getRenderPrimitive(), 1);
        api.endRenderPass();

        // Step 2: Render a larger triangle with the stencil test enabled.
        params.flags.clear = TargetBufferFlags::NONE;
        params.flags.discardStart = TargetBufferFlags::NONE;
        ps.rasterState.colorWrite = true;
        ps.rasterState.stencilWrite = false;
        ps.rasterState.stencilOpDpPass = StencilOperation::KEEP;
        ps.rasterState.stencilFunc = RasterState::StencilFunction::E;
        ps.rasterState.stencilRef = 0u;

        api.beginRenderPass(renderTarget, params);
        api.draw(ps, triangle.getRenderPrimitive(), 1);
        api.endRenderPass();

        readPixelsAndAssertHash("StencilBuffer", 512, 512, renderTarget, 0x3B1AEF0F, true);

        api.commit(swapChain);
        api.endFrame(0);

        api.destroyProgram(program);
        api.destroySwapChain(swapChain);
        api.destroyTexture(colorTexture);
        api.destroyTexture(stencilTexture);
        api.destroyRenderTarget(renderTarget);
    }

    flushAndWait();
    getDriver().purge();
}

} // namespace test