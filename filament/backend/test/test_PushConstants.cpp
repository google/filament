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
#include "backend/DriverEnums.h"
#include "backend/Handle.h"

#include <utils/Hash.h>

namespace test {

static ShaderGenerator::PushConstants gVertConstants;
static ShaderGenerator::PushConstants gFragConstants;

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
} pushConstantsV;

layout(location = 0) in vec4 mesh_position;
void main() {
    if (pushConstantsV.hideTriangle) {
        // Test that bools are written correctly. All bits must be 0 if the bool is false.
        gl_Position = vec4(0.0);
        return;
    }
    gl_Position = vec4(mesh_position.xy * pushConstantsV.triangleScale +
            vec2(pushConstantsV.triangleOffsetX, pushConstantsV.triangleOffsetY), 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
})";

static const char* const triangleFs = R"(#version 450 core

layout(push_constant) uniform Constants {
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // offset here accounts for the size of the push constants in the vertex stage.  Vulkan has one
    // block of memory for all stages to share.
    layout(offset=16) float red;
#else
    float red;
#endif
    bool padding;       // test correct bool padding
    float green;
    float blue;
} pushConstantsF;

precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
void main() {
    fragColor = vec4(pushConstantsF.red, pushConstantsF.green, pushConstantsF.blue, 1.0);
})";

void initPushConstants() {
    // TODO: move this initialization to a more appropriate place.
    gVertConstants.clear();
    gFragConstants.clear();

    gVertConstants.reserve(4);
    gVertConstants.resize(4);
    gVertConstants[pushConstantIndex.TRIANGLE_HIDE] = {"pushConstantsV.hideTriangle", backend::ConstantType::BOOL};
    gVertConstants[pushConstantIndex.TRIANGLE_SCALE] = {"pushConstantsV.triangleScale", backend::ConstantType::FLOAT};
    gVertConstants[pushConstantIndex.TRIANGLE_OFFSET_X] = {"pushConstantsV.triangleOffsetX", backend::ConstantType::FLOAT};
    gVertConstants[pushConstantIndex.TRIANGLE_OFFSET_Y] = {"pushConstantsV.triangleOffsetY", backend::ConstantType::FLOAT};

    gFragConstants.reserve(4);
    gFragConstants.resize(4);
    gFragConstants[pushConstantIndex.RED] = {"pushConstantsF.red", backend::ConstantType::FLOAT};
    gFragConstants[pushConstantIndex.GREEN] = {"pushConstantsF.green", backend::ConstantType::FLOAT};
    gFragConstants[pushConstantIndex.BLUE] = {"pushConstantsF.blue", backend::ConstantType::FLOAT};
}

TEST_F(BackendTest, PushConstants) {
    SKIP_IF(Backend::WEBGPU, "Push constants not supported on WebGPU");
    // Test is flaky on CI (but does not repro locally).
    SKIP_IF(SkipEnvironment(OperatingSystem::CI, Backend::VULKAN), "b/453776664");

    initPushConstants();

    auto& api = getDriverApi();

    api.startCapture(0);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a SwapChain and make it current.
        auto swapChain = addCleanup(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        // Create a program.
        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);
        Program p =
                shaderGen.getProgramWithPushConstants(api, { gVertConstants, gFragConstants, {} });
        ProgramHandle program = addCleanup(api.createProgram(std::move(p)));

        Handle<HwRenderTarget> renderTarget = addCleanup(api.createDefaultRenderTarget());

        TrianglePrimitive triangle(api);

        RenderPassParams params = getClearColorRenderPass();
        params.viewport = getFullViewport();

        PipelineState ps = {};
        ps.program = program;
        ps.vertexBufferInfo = triangle.getVertexBufferInfo();
        ps.rasterState.colorWrite = true;
        ps.rasterState.depthWrite = false;

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, 0);

        api.beginRenderPass(renderTarget, params);
        api.bindPipeline(ps);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());

        // Set the push constants to scale the triangle in half
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_HIDE, false);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_SCALE, 0.5f);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_X, 0.0f);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_Y, 0.0f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.RED, 0.25f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.GREEN, 0.5f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.BLUE, 1.0f);
        api.draw2(0, 3, 1);

        // Draw another triangle, transposed to the upper-right.
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_X, 0.5f);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_Y, 0.5f);

        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.RED, 1.00f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.GREEN, 0.5f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.BLUE, 0.25f);

        api.draw2(0, 3, 1);

        // Draw a final triangle, transposed to the lower-left.
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_X, -0.5f);
        api.setPushConstant(ShaderStage::VERTEX, pushConstantIndex.TRIANGLE_OFFSET_Y, -0.5f);

        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.RED, 0.5f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.GREEN, 0.25f);
        api.setPushConstant(ShaderStage::FRAGMENT, pushConstantIndex.BLUE, 1.00f);

        api.draw2(0, 3, 1);

        api.endRenderPass();

        EXPECT_IMAGE(renderTarget, ScreenshotParams(params.viewport.width, params.viewport.height,
                                           "pushConstants", 3575588741));

        api.commit(swapChain);
        api.endFrame(0);
    }

    api.stopCapture(0);
}

} // namespace test
