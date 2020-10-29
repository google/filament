/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "WriteVideo.h"

#include <CoreVideo/CoreVideo.h>

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string vertex (R"(#version 450 core

layout(location = 0) in vec4 mesh_position;

layout(location = 0) out uvec4 indices;

void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
}
)");

std::string fragment (R"(#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragColor1;

void main() {
    fragColor = vec4(1.0);
    fragColor1 = vec4(1.0, 0.0, 0.0, 1.0);
}

)");

}

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, CVPixelBufferSwapChain) {
    // Create a CVPixelBuffer that will be rendered into.
    CFStringRef keys[4];
    keys[0] = kCVPixelBufferCGBitmapContextCompatibilityKey;
    keys[1] = kCVPixelBufferCGImageCompatibilityKey;
    keys[2] = kCVPixelBufferOpenGLCompatibilityKey;
    keys[3] = kCVPixelBufferMetalCompatibilityKey;
    CFTypeRef values[4];
    int yes = 1;
    values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &yes);
    values[1] = values[0];
    values[2] = values[0];
    values[3] = values[0];
    CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault, (const void**) keys,
            (const void**) values, 4, nullptr, nullptr);
    CVPixelBufferRef pixelBuffer = nullptr;
    CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault, 512, 512,
            kCVPixelFormatType_32BGRA, options, &pixelBuffer);
    assert(status == kCVReturnSuccess);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a SwapChain from the CVPixelBuffer.
        // These two driver calls, createSwapChain and setupExternalImage, are called from
        // engine->createSwapChain.
        auto swapChain = getDriverApi().createSwapChain(pixelBuffer,
                backend::SWAP_CHAIN_CONFIG_APPLE_CVPIXELBUFFER);
        getDriverApi().setupExternalImage(pixelBuffer);
        getDriverApi().makeCurrent(swapChain, swapChain);

        // Create a program.
        ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
        Program p = shaderGen.getProgram();
        auto program = getDriverApi().createProgram(std::move(p));

        TrianglePrimitive triangle(getDriverApi());

        auto defaultRenderTarget = getDriverApi().createDefaultRenderTarget(0);

        RenderPassParams params = {};
        params.viewport.left = 0;
        params.viewport.bottom = 0;
        params.viewport.width = 512;
        params.viewport.height = 512;
        params.flags.clear = TargetBufferFlags::COLOR;
        params.clearColor = {0.f, 1.f, 0.f, 1.f};
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState state;
        state.program = program;
        state.rasterState.colorWrite = true;
        state.rasterState.depthWrite = false;
        state.rasterState.depthFunc = RasterState::DepthFunc::A;
        state.rasterState.culling = CullingMode::NONE;

        getDriverApi().startCapture(0);

        getDriverApi().makeCurrent(swapChain, swapChain);
        getDriverApi().beginFrame(0, 0, nullptr, nullptr);

        // Draw a triangle.
        getDriverApi().beginRenderPass(defaultRenderTarget, params);
        getDriverApi().draw(state, triangle.getRenderPrimitive());
        getDriverApi().endRenderPass();

        getDriverApi().flush();
        getDriverApi().commit(swapChain);
        getDriverApi().endFrame(0);

        getDriverApi().stopCapture(0);

        getDriverApi().destroyProgram(program);
        getDriverApi().destroySwapChain(swapChain);
        getDriverApi().destroyRenderTarget(defaultRenderTarget);
    }

    executeCommands();

    // Do something with pixel buffer and then release it.
    encode(pixelBuffer);

    CVPixelBufferRelease(pixelBuffer);
}

} // namespace test
