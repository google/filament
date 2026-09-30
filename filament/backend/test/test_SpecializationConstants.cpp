/*
 * Copyright (C) 2026 The Android Open Source Project
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
#include "TrianglePrimitive.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/Program.h>

#include <utility>

#include <stdint.h>

namespace test {

using namespace filament;
using namespace filament::backend;

namespace {

static const char* const triangleVs = R"(#version 450 core

layout(constant_id = 0) const bool kHideTriangle = false;
layout(constant_id = 1) const float kTriangleScale = 1.0;
layout(constant_id = 2) const int kTriangleOffsetSteps = 0;

layout(location = 0) in vec4 mesh_position;
void main() {
    bool hide = kHideTriangle;
    float scale = kTriangleScale;
    int offsetSteps = kTriangleOffsetSteps;
    if (hide) {
        gl_Position = vec4(0.0);
        return;
    }
    vec2 offset = vec2(float(offsetSteps) * 0.5);
    gl_Position = vec4(mesh_position.xy * scale + offset, 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
})";

static const char* const triangleFs = R"(#version 450 core

layout(constant_id = 3) const bool kSwapRedBlue = false;
layout(constant_id = 4) const float kRed = 0.0;
layout(constant_id = 5) const int kGreenQuarterSteps = 0;

precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
void main() {
    bool swapRB = kSwapRedBlue;
    float red = kRed;
    int greenSteps = kGreenQuarterSteps;
    float green = float(greenSteps) * 0.25;
    float blue = 1.0;
    if (swapRB) {
        fragColor = vec4(blue, green, red, 1.0);
    } else {
        fragColor = vec4(red, green, blue, 1.0);
    }
})";

Program::SpecializationConstantsInfo makeSpecializationConstants(bool hideTriangle,
        float triangleScale, int32_t triangleOffsetSteps, bool swapRedBlue, float red,
        int32_t greenQuarterSteps) {
    Program::SpecializationConstantsInfo constants =
            Program::SpecializationConstantsInfo::with_capacity(6);
    constants.push_back(hideTriangle);
    constants.push_back(triangleScale);
    constants.push_back(triangleOffsetSteps);
    constants.push_back(swapRedBlue);
    constants.push_back(red);
    constants.push_back(greenQuarterSteps);
    return constants;
}

} // anonymous namespace

TEST_F(BackendTest, SpecializationConstants) {
    auto& api = getDriverApi();

    api.startCapture(0);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a SwapChain and make it current.
        auto swapChain = addCleanup(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);

        // Center triangle: scale = 0.5, offset = (0, 0), color = (0.25, 0.5, 1.0).
        ProgramHandle centerProgram =
                addCleanup(api.createProgram(shaderGen.getProgramWithSpecializationConstants(api,
                        makeSpecializationConstants(false, 0.5f, 0, false, 0.25f, 2))));

        // Upper-right triangle: scale = 0.5, offset = (0.5, 0.5), color = (1.0, 0.5, 0.25)
        // via kSwapRedBlue = true.
        ProgramHandle upperRightProgram =
                addCleanup(api.createProgram(shaderGen.getProgramWithSpecializationConstants(api,
                        makeSpecializationConstants(false, 0.5f, 1, true, 0.25f, 2))));

        // Lower-left triangle: scale = 0.5, offset = (-0.5, -0.5), color = (0.5, 0.25, 1.0).
        ProgramHandle lowerLeftProgram =
                addCleanup(api.createProgram(shaderGen.getProgramWithSpecializationConstants(api,
                        makeSpecializationConstants(false, 0.5f, -1, false, 0.5f, 1))));

        // Hidden triangle: kHideTriangle = true should discard the triangle in the vertex stage.
        ProgramHandle hiddenProgram =
                addCleanup(api.createProgram(shaderGen.getProgramWithSpecializationConstants(api,
                        makeSpecializationConstants(true, 1.0f, 0, false, 1.0f, 4))));

        Handle<HwRenderTarget> renderTarget = addCleanup(api.createDefaultRenderTarget());

        TrianglePrimitive triangle(api);

        RenderPassParams params = getClearColorDepthRenderPass();
        params.viewport = getFullViewport();

        PipelineState ps = {};
        ps.vertexBufferInfo = triangle.getVertexBufferInfo();
        ps.rasterState.colorWrite = true;
        ps.rasterState.depthWrite = false;

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, 0);

        api.beginRenderPass(renderTarget, params);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());

        for (ProgramHandle const program:
                { centerProgram, upperRightProgram, lowerLeftProgram, hiddenProgram }) {
            ps.program = program;
            api.bindPipeline(ps);
            api.draw2(0, 3, 1);
        }

        api.endRenderPass();

        // The specialization constants are chosen so that this renders exactly the same image as
        // the PushConstants test, but the golden is kept separate: the file name also drives the
        // "_actual" and "_diff" output paths, which must not collide between tests.
        EXPECT_IMAGE(renderTarget, ScreenshotParams(params.viewport.width, params.viewport.height,
                                           "specializationConstants", 3575588741));

        api.commit(swapChain);
        api.endFrame(0);
    }

    api.stopCapture(0);
}

} // namespace test
