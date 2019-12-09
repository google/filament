/*
 * Copyright (C) 2019 The Android Open Source Project
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

void main() {
    fragColor = vec4(1.0);
}

)");

}

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, VertexBufferUpdate) {
    const bool largeBuffers = false;

    // If updateIndices is true, then even-numbered triangles will have their indices set to
    // {0, 0, 0}, effectively "hiding" every other triangle.
    const bool updateIndices = true;

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a platform-specific SwapChain and make it current.
        auto swapChain = createSwapChain();
        getDriverApi().makeCurrent(swapChain, swapChain);

        // Create a program.
        ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
        Program p = shaderGen.getProgram();
        auto program = getDriverApi().createProgram(std::move(p));

        auto defaultRenderTarget = getDriverApi().createDefaultRenderTarget(0);

        // To test large buffers (which exercise a different code path) create an extra large
        // buffer. Only the first 3 vertices will be used.
        TrianglePrimitive triangle(getDriverApi(), largeBuffers);

        RenderPassParams params = {};
        fullViewport(params);
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

        // Draw 10 triangles, updating the vertex buffer / index buffer each time.
        getDriverApi().beginRenderPass(defaultRenderTarget, params);
        size_t triangleIndex = 0;
        for (float i = -1.0f; i < 1.0f; i += 0.2f) {
            const float low = i, high = i + 0.2;
            const filament::math::float2 v[3] {{low, low}, {high, low}, {low, high}};
            triangle.updateVertices(v);

            if (updateIndices) {
                if (triangleIndex % 2 == 0) {
                    const short i[3] {0, 1, 2};
                    triangle.updateIndices(i);
                } else {
                    // This effectively hides this triangle.
                    const short i[3] {0, 0, 0};
                    triangle.updateIndices(i);
                }
            }
            getDriverApi().draw(state, triangle.getRenderPrimitive());

            triangleIndex++;
        }
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
}

} // namespace test
