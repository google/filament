/*
 * Copyright (C) 2024 The Android Open Source Project
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
#include "ShaderGenerator.h"
#include "Skip.h"
#include "TrianglePrimitive.h"

#include <utils/Hash.h>

namespace test {

using namespace filament;
using namespace filament::backend;

static constexpr struct {
    size_t TRIANGLE_HIDE = 0;
    size_t TRIANGLE_SCALE = 1;
    size_t TRIANGLE_OFFSET_X = 2;
    size_t TRIANGLE_OFFSET_Y = 3;

    size_t RED = 0;
    size_t GREEN = 2;
    size_t BLUE = 3;
} pushConstantIndex;

static const char* const triangleVs = R"(#version 450 core

layout(push_constant) uniform Constants {
    bool hideTriangle;
    float triangleScale;
    float triangleOffsetX;
    float triangleOffsetY;
} pushConstants;

layout(location = 0) in vec4 mesh_position;
void main() {
    if (pushConstants.hideTriangle) {
        // Test that bools are written correctly. All bits must be 0 if the bool is false.
        gl_Position = vec4(0.0);
        return;
    }
    gl_Position = vec4(mesh_position.xy * pushConstants.triangleScale +
            vec2(pushConstants.triangleOffsetX, pushConstants.triangleOffsetY), 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
})";

static const char* const triangleFs = R"(#version 450 core

layout(push_constant) uniform Constants {
    float red;
    bool padding;       // test correct bool padding
    float green;
    float blue;
} pushConstants;

precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
void main() {
    fragColor = vec4(pushConstants.red, pushConstants.green, pushConstants.blue, 1.0);
})";

TEST_F(BackendTest, PushConstants) {
    SKIP_IF(Backend::OPENGL, "Push constants not supported on OpenGL");

    auto& api = getDriverApi();

    api.startCapture(0);
    Cleanup cleanup(api);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a SwapChain and make it current.
        auto swapChain = cleanup.add(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        // Create a program.
        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);
        Program p = shaderGen.getProgram(api);
        ProgramHandle program = cleanup.add(api.createProgram(std::move(p)));

        Handle<HwRenderTarget> renderTarget = cleanup.add(api.createDefaultRenderTarget());

        TrianglePrimitive triangle(api);

        RenderPassParams params = {};
        params.flags.clear = TargetBufferFlags::COLOR0;
        params.viewport = { 0, 0, 512, 512 };
        params.clearColor = math::float4(0.0f, 0.0f, 1.0f, 1.0f);
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState ps = {};
        ps.program = program;
        ps.rasterState.colorWrite = true;
        ps.rasterState.depthWrite = false;

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, 0);

        api.beginRenderPass(renderTarget, params);

        // Set the push constants to scale the triangle in half
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_HIDE, false);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_SCALE, 0.5f);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_X, 0.0f);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_Y, 0.0f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.RED, 0.25f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.GREEN, 0.5f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.BLUE, 1.0f);
        api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);

        // Draw another triangle, transposed to the upper-right.
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_X, 0.5f);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_Y, 0.5f);

        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.RED, 1.00f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.GREEN, 0.5f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.BLUE, 0.25f);

        api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);

        // Draw a final triangle, transposed to the lower-left.
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_X, -0.5f);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_Y, -0.5f);

        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.RED, 0.5f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.GREEN, 0.25f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.BLUE, 1.00f);

        api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);

        api.endRenderPass();

        EXPECT_IMAGE(renderTarget, getExpectations(),
                ScreenshotParams(512, 512, "pushConstants", 1957275826));

        api.commit(swapChain);
        api.endFrame(0);
    }

    api.stopCapture(0);
}

} // namespace test
