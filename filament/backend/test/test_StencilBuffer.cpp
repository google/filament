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

#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "TrianglePrimitive.h"

using namespace filament;
using namespace filament::backend;

namespace test {

// 1. Clear the stencil buffer to all zeroes.
// 2. Render a small triangle only to the stencil buffer and set the operation to write ones.
// 3. Enable a "== 0" stencil test and enable color writes.
// 4. Render a larger triangle to the SwapChain.
// 5. The larger triangle should have a "hole" in the middle.
class BasicStencilBufferTest : public BackendTest {
public:

    Handle <HwSwapChain> mSwapChain;
    ProgramHandle mProgram;
    Cleanup mCleanup;

    BasicStencilBufferTest() : mCleanup(getDriverApi()) {}

    void SetUp() override {
        auto& api = getDriverApi();

        // Create a platform-specific SwapChain and make it current.
        mSwapChain = mCleanup.add(createSwapChain());
        api.makeCurrent(mSwapChain, mSwapChain);

        Shader shader = SharedShaders::makeShader(api, mCleanup, ShaderRequest{
                .mVertexType = VertexShaderType::Noop,
                .mFragmentType = FragmentShaderType::White,
                .mUniformType = ShaderUniformType::None
        });
        mProgram = shader.getProgram();
    }

    void RunTest(Handle <HwRenderTarget> renderTarget) {
        auto& api = getDriverApi();

        // We'll be using a triangle as geometry.
        TrianglePrimitive smallTriangle(api);
        static filament::math::float2 vertices[3] = {
                { -0.5, -0.5 },
                { 0.5,  -0.5 },
                { -0.5, 0.5 }
        };
        smallTriangle.updateVertices(vertices);
        TrianglePrimitive triangle(api);

        // Step 1: Clear the stencil buffer to all zeroes and the color buffer to blue.
        // Render a small triangle only to the stencil buffer, increasing the stencil buffer to 1.
        RenderPassParams params = {};
        params.flags.clear = TargetBufferFlags::COLOR0 | TargetBufferFlags::STENCIL;
        params.viewport = { 0, 0, 512, 512 };
        params.clearColor = math::float4(0.0f, 0.0f, 1.0f, 1.0f);
        params.clearStencil = 0u;
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState ps = {};
        ps.program = mProgram;
        ps.rasterState.colorWrite = false;
        ps.rasterState.depthWrite = false;
        ps.stencilState.stencilWrite = true;
        ps.stencilState.front.stencilOpDepthStencilPass = StencilOperation::INCR;

        api.makeCurrent(mSwapChain, mSwapChain);
        RenderFrame frame(api);

        api.beginRenderPass(renderTarget, params);
        api.draw(ps, smallTriangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        // Step 2: Render a larger triangle with the stencil test enabled.
        params.flags.clear = TargetBufferFlags::NONE;
        params.flags.discardStart = TargetBufferFlags::NONE;
        ps.rasterState.colorWrite = true;
        ps.stencilState.stencilWrite = false;
        ps.stencilState.front.stencilOpDepthStencilPass = StencilOperation::KEEP;
        ps.stencilState.front.stencilFunc = StencilState::StencilFunction::E;
        ps.stencilState.front.ref = 0u;

        api.beginRenderPass(renderTarget, params);
        api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        api.commit(mSwapChain);
    }

    void TearDown() override {
        flushAndWait();
    }
};

TEST_F(BasicStencilBufferTest, StencilBuffer) {
    auto& api = getDriverApi();
    Cleanup cleanup(api);

    // Create two textures: a color and a stencil, and an associated RenderTarget.
    auto colorTexture = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 512, 512, 1, TextureUsage::COLOR_ATTACHMENT));
    auto stencilTexture = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::STENCIL8, 1, 512, 512, 1, TextureUsage::STENCIL_ATTACHMENT));
    auto renderTarget = cleanup.add(api.createRenderTarget(
            TargetBufferFlags::COLOR0 | TargetBufferFlags::STENCIL, 512, 512, 1, 0,
            {{ colorTexture }}, {}, {{ stencilTexture }}));

    RunTest(renderTarget);

    readPixelsAndAssertHash("StencilBuffer", 512, 512, renderTarget, 0x3B1AEF0F, true);
}

TEST_F(BasicStencilBufferTest, DepthAndStencilBuffer) {
    auto& api = getDriverApi();
    Cleanup cleanup(api);

    // Create two textures: a color and a stencil, and an associated RenderTarget.
    auto colorTexture = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 512, 512, 1, TextureUsage::COLOR_ATTACHMENT));
    auto depthStencilTexture = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::DEPTH24_STENCIL8, 1, 512, 512, 1,
            TextureUsage::STENCIL_ATTACHMENT | TextureUsage::DEPTH_ATTACHMENT));
    auto renderTarget = cleanup.add(api.createRenderTarget(
            TargetBufferFlags::COLOR0 | TargetBufferFlags::STENCIL, 512, 512, 1, 0,
            {{ colorTexture }}, { depthStencilTexture }, {{ depthStencilTexture }}));

    RunTest(renderTarget);

    readPixelsAndAssertHash("DepthAndStencilBuffer", 512, 512, renderTarget, 0x3B1AEF0F, true);
}

TEST_F(BasicStencilBufferTest, StencilBufferMSAA) {
    auto& api = getDriverApi();
    Cleanup cleanup(api);

    // Create two textures: a single-sampled color and a MSAA stencil texture.
    // We also create two RenderTargets, one for each pass:
    // Pass 0: Render a triangle only into the MSAA stencil buffer.
    // Pass 1: Render a triangle into (an auto-created) MSAA color buffer using the stencil test.
    //         Performs an auto-resolve on the color.
    auto colorTexture = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 512, 512, 1,
            TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE));
    auto depthStencilTextureMSAA = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::DEPTH24_STENCIL8, 4, 512, 512, 1,
            TextureUsage::STENCIL_ATTACHMENT | TextureUsage::DEPTH_ATTACHMENT));
    auto renderTarget0 = cleanup.add(api.createRenderTarget(
            TargetBufferFlags::DEPTH_AND_STENCIL, 512, 512, 4, 0,
            {{}}, { depthStencilTextureMSAA }, { depthStencilTextureMSAA }));
    auto renderTarget1 = cleanup.add(api.createRenderTarget(
            TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH_AND_STENCIL, 512, 512, 4, 0,
            {{ colorTexture }}, { depthStencilTextureMSAA }, { depthStencilTextureMSAA }));

    api.startCapture(0);
    cleanup.addPostCall([&]() { api.stopCapture(0); });

    // We'll be using a triangle as geometry.
    TrianglePrimitive smallTriangle(api);
    static filament::math::float2 vertices[3] = {
            { -0.5, -0.5 },
            { 0.5,  -0.5 },
            { -0.5, 0.5 }
    };
    smallTriangle.updateVertices(vertices);
    TrianglePrimitive triangle(api);

    // Step 1: Clear the stencil buffer to all zeroes.
    // Render a small triangle only to the stencil buffer, increasing the stencil buffer to 1.
    RenderPassParams params = {};
    params.flags.clear = TargetBufferFlags::STENCIL;
    params.viewport = { 0, 0, 512, 512 };
    params.clearStencil = 0u;
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;

    PipelineState ps = {};
    ps.program = mProgram;
    ps.rasterState.colorWrite = false;
    ps.rasterState.depthWrite = false;
    ps.stencilState.stencilWrite = true;
    ps.stencilState.front.stencilOpDepthStencilPass = StencilOperation::INCR;

    api.makeCurrent(mSwapChain, mSwapChain);
    {
        RenderFrame frame(api);

        api.beginRenderPass(renderTarget0, params);
        api.draw(ps, smallTriangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        // Step 2: Render a larger triangle with the stencil test enabled.
        params.flags.clear = TargetBufferFlags::COLOR0;
        params.flags.discardStart = TargetBufferFlags::COLOR0;
        params.flags.discardEnd = TargetBufferFlags::STENCIL;
        params.clearColor = math::float4(0.0f, 0.0f, 1.0f, 1.0f);
        ps.rasterState.colorWrite = true;
        ps.stencilState.stencilWrite = false;
        ps.stencilState.front.stencilOpDepthStencilPass = StencilOperation::KEEP;
        ps.stencilState.front.stencilFunc = StencilState::StencilFunction::E;
        ps.stencilState.front.ref = 0u;

        api.beginRenderPass(renderTarget1, params);
        api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        api.commit(mSwapChain);
    }
    readPixelsAndAssertHash("StencilBufferAutoResolve", 512, 512, renderTarget1, 0x6CEFAC8F, true);
}

} // namespace test
