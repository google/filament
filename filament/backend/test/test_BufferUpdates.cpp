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

uniform Params {
    highp vec4 padding[4];  // offset of 64 bytes

    highp float red;
    highp float green;
    highp float blue;
} params;

void main() {
    fragColor = vec4(params.red, params.green, params.blue, 1.0f);
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
        getDriverApi().beginFrame(0, 0);

        // Draw 10 triangles, updating the vertex buffer / index buffer each time.
        getDriverApi().beginRenderPass(defaultRenderTarget, params);
        size_t triangleIndex = 0;
        for (float i = -1.0f; i < 1.0f; i += 0.2f) {
            const float low = i, high = i + 0.2;
            const filament::math::float2 v[3] {{low, low}, {high, low}, {low, high}};
            triangle.updateVertices(v);

            if (updateIndices) {
                if (triangleIndex % 2 == 0) {
                    // Upload each index separately, to test offsets.
                    const short i[3] {0, 1, 2};
                    triangle.updateIndices(i + 0, 1, 0);
                    triangle.updateIndices(i + 1, 1, 1);
                    triangle.updateIndices(i + 2, 1, 2);
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

struct MaterialParams {
    float red;
    float green;
    float blue;
};

static void uploadUniforms(DriverApi& dapi, Handle<HwBufferObject> ubh, MaterialParams params) {
    MaterialParams* tmp = new MaterialParams(params);
    auto cb = [](void* buffer, size_t size, void* user) {
        MaterialParams* sp = (MaterialParams*) buffer;
        delete sp;
    };
    BufferDescriptor bd(tmp, sizeof(MaterialParams), cb);
    dapi.updateBufferObject(ubh, std::move(bd), 0);
}

TEST_F(BackendTest, BufferObjectUpdateWithOffset) {
    // Create a platform-specific SwapChain and make it current.
    auto swapChain = createSwapChain();
    getDriverApi().makeCurrent(swapChain, swapChain);

    // Create a program.
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
    Program p = shaderGen.getProgram();
    p.setUniformBlock(1, utils::CString("params"));
    auto program = getDriverApi().createProgram(std::move(p));

    // Create a uniform buffer.
    auto ubuffer = getDriverApi().createBufferObject(sizeof(MaterialParams) + 64,
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
    getDriverApi().bindUniformBuffer(0, ubuffer);

    // Upload uniforms.
    {
        MaterialParams params {
            .red = 1.0f,
            .green = 0.0f,
            .blue = 0.5f
        };
        MaterialParams* tmp = new MaterialParams(params);
        auto cb = [](void* buffer, size_t size, void* user) {
            MaterialParams* sp = (MaterialParams*) buffer;
            delete sp;
        };
        BufferDescriptor bd(tmp, sizeof(MaterialParams), cb);
        getDriverApi().updateBufferObject(ubuffer, std::move(bd), 64);
    }

    auto defaultRenderTarget = getDriverApi().createDefaultRenderTarget(0);

    renderTriangle(defaultRenderTarget, swapChain, program);

    static const uint32_t expectedHash = 2000773999;
    readPixelsAndAssertHash("BufferObjectUpdateWithOffset", 512, 512, defaultRenderTarget, expectedHash);

    getDriverApi().flush();
    getDriverApi().commit(swapChain);
    getDriverApi().endFrame(0);

    getDriverApi().destroyProgram(program);
    getDriverApi().destroySwapChain(swapChain);
    getDriverApi().destroyRenderTarget(defaultRenderTarget);

    // This ensures all driver commands have finished before exiting the test.
    getDriverApi().finish();

    executeCommands();

    getDriver().purge();
}

} // namespace test
