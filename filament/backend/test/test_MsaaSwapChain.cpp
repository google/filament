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

#include "ImageExpectations.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "Skip.h"
#include "SharedShaders.h"
#include "TrianglePrimitive.h"

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;

// Copied from filament/SwapChain.h
static constexpr uint64_t CONFIG_MSAA_4_SAMPLES = backend::SWAP_CHAIN_CONFIG_MSAA_4_SAMPLES;

// Tests are parameterized by headless versus native SwapChain.
struct MsaaSwapChainTest : public BackendTest, public testing::WithParamInterface<bool> {};

TEST_P(MsaaSwapChainTest, Basic) {
    SKIP_IF(Backend::OPENGL, "OpenGL does not support MSAA SwapChain on all platforms.");
    SKIP_IF(Backend::VULKAN, "Vulkan does not support MSAA SwapChain on all platforms.");

    constexpr int kRenderTargetSize = 512;

    auto useHeadlessSwapChain = GetParam();

    auto& api = getDriverApi();
    api.startCapture(0);
    Cleanup cleanup(api);

    auto swapChain = [&]() {
        auto flags = CONFIG_MSAA_4_SAMPLES;
        if (useHeadlessSwapChain) {
            return cleanup.add(
                    api.createSwapChainHeadless(kRenderTargetSize, kRenderTargetSize, flags));
        }
        return cleanup.add(createSwapChain(flags));
    }();

    api.makeCurrent(swapChain, swapChain);
    RenderTargetHandle renderTarget = cleanup.add(api.createDefaultRenderTarget());

    Shader shader = SharedShaders::makeShader(api, cleanup, ShaderRequest{
            .mVertexType = VertexShaderType::Noop,
            .mFragmentType = FragmentShaderType::White,
    });
    TrianglePrimitive triangle(api);

    RenderPassParams params = getClearColorRenderPass();
    params.viewport = getFullViewport();

    PipelineState ps = getColorWritePipelineState();
    shader.addProgramToPipelineState(ps);

    {
        RenderFrame frame(api);

        api.beginRenderPass(renderTarget, params);
        ps.primitiveType = PrimitiveType::TRIANGLES;
        ps.vertexBufferInfo = triangle.getVertexBufferInfo();
        api.bindPipeline(ps);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();

        EXPECT_IMAGE(renderTarget,
                ScreenshotParams(kRenderTargetSize, kRenderTargetSize, "MsaaSwapChain", 0));

        api.commit(swapChain);
    }

    api.stopCapture(0);
}

// Helper to give names to the tests.
static std::string testNameGenerator(const testing::TestParamInfo<bool>& info) {
    const bool useHeadlessSwapChain = info.param;
    return useHeadlessSwapChain ? "HeadlessSwapChain" : "NativeSwapChain";
}

INSTANTIATE_TEST_SUITE_P(MsaaSwapChainTests, MsaaSwapChainTest, testing::Values(true, false),
        testNameGenerator);

} // namespace test
