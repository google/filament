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
class BasicStencilBufferTest : public BackendTest {
public:

    Handle<HwSwapChain> swapChain;
    ProgramHandle program;

    void SetUp() override {
        auto& api = getDriverApi();

        // Create a platform-specific SwapChain and make it current.
        swapChain = createSwapChain();
        api.makeCurrent(swapChain, swapChain);

        // Create a program.
        ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
        Program p = shaderGen.getProgram();
        program = api.createProgram(std::move(p));
    }

    void RunTest(Handle<HwRenderTarget> renderTarget) {
        auto& api = getDriverApi();

        // We'll be using a triangle as geometry.
        TrianglePrimitive smallTriangle(api);
        static filament::math::float2 vertices[3] = {
                { -0.5, -0.5 },
                {  0.5, -0.5 },
                { -0.5,  0.5 }
        };
        smallTriangle.updateVertices(vertices);
        TrianglePrimitive triangle(api);

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
        ps.rasterState.stencilOpDepthStencilPass = StencilOperation::INCR;

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
        ps.rasterState.stencilOpDepthStencilPass = StencilOperation::KEEP;
        ps.rasterState.stencilFunc = RasterState::StencilFunction::E;
        ps.rasterState.stencilRef = 0u;

        api.beginRenderPass(renderTarget, params);
        api.draw(ps, triangle.getRenderPrimitive(), 1);
        api.endRenderPass();

        api.commit(swapChain);
        api.endFrame(0);
    }

    void TearDown() override {
        auto& api = getDriverApi();
        api.destroyProgram(program);
        api.destroySwapChain(swapChain);
        flushAndWait();
    }
};

TEST_F(BasicStencilBufferTest, StencilBuffer) {
    auto& api = getDriverApi();

    // Create two textures: a color and a stencil, and an associated RenderTarget.
    auto colorTexture = api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 512, 512, 1, TextureUsage::COLOR_ATTACHMENT);
    auto stencilTexture = api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::STENCIL8, 1, 512, 512, 1, TextureUsage::STENCIL_ATTACHMENT);
    auto renderTarget = getDriverApi().createRenderTarget(
            TargetBufferFlags::COLOR0 | TargetBufferFlags::STENCIL, 512, 512, 1,
            {{colorTexture}}, {}, {{stencilTexture}});

    RunTest(renderTarget);

    readPixelsAndAssertHash("StencilBuffer", 512, 512, renderTarget, 0x3B1AEF0F, true);

    flushAndWait();
    getDriver().purge();

    api.destroyTexture(colorTexture);
    api.destroyTexture(stencilTexture);
    api.destroyRenderTarget(renderTarget);
}

TEST_F(BasicStencilBufferTest, DepthAndStencilBuffer) {
    auto& api = getDriverApi();

    // Create two textures: a color and a stencil, and an associated RenderTarget.
    auto colorTexture = api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 512, 512, 1, TextureUsage::COLOR_ATTACHMENT);
    auto depthStencilTexture = api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::DEPTH24_STENCIL8, 1, 512, 512, 1, TextureUsage::STENCIL_ATTACHMENT | TextureUsage::DEPTH_ATTACHMENT);
    auto renderTarget = getDriverApi().createRenderTarget(
            TargetBufferFlags::COLOR0 | TargetBufferFlags::STENCIL, 512, 512, 1,
            {{colorTexture}}, {depthStencilTexture}, {{depthStencilTexture}});

    RunTest(renderTarget);

    readPixelsAndAssertHash("DepthAndStencilBuffer", 512, 512, renderTarget, 0x3B1AEF0F, true);

    flushAndWait();
    getDriver().purge();

    api.destroyTexture(colorTexture);
    api.destroyTexture(depthStencilTexture);
    api.destroyRenderTarget(renderTarget);
}

} // namespace test