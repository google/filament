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

uniform Params {
    highp vec4 padding[4];  // offset of 64 bytes

    highp vec4 color;
    highp vec4 offset;
} params;

void main() {
    gl_Position = vec4(mesh_position.xy + params.offset.xy, 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
}
)");

std::string fragment (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

uniform Params {
    highp vec4 padding[4];  // offset of 64 bytes

    highp vec4 color;
    highp vec4 offset;
} params;

void main() {
    fragColor = vec4(params.color.rgb, 1.0f);
}

)");

}

namespace test {

using namespace filament;
using namespace filament::backend;

// In the shader, these MaterialParams are offset by 64 bytes into the uniform buffer to test buffer
// updates with offset.
struct MaterialParams {
    math::float4 color;
    math::float4 offset;
};
static_assert(sizeof(MaterialParams) == 8 * sizeof(float));

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
        Program p = shaderGen.getProgram(getDriverApi());
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

        // Create a uniform buffer.
        // We use STATIC here, even though the buffer is updated, to force the Metal backend to use a
        // GPU buffer, which is more interesting to test.
        auto ubuffer = getDriverApi().createBufferObject(sizeof(MaterialParams) + 64,
                BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
        getDriverApi().bindUniformBuffer(0, ubuffer);

        getDriverApi().startCapture(0);

        // Upload uniforms.
        {
            MaterialParams params {
                    .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                    .offset = { 0.0f, 0.0f, 0.0f, 0.0f }
            };
            auto* tmp = new MaterialParams(params);
            auto cb = [](void* buffer, size_t size, void* user) {
                auto* sp = (MaterialParams*) buffer;
                delete sp;
            };
            BufferDescriptor bd(tmp, sizeof(MaterialParams), cb);
            getDriverApi().updateBufferObject(ubuffer, std::move(bd), 64);
        }

        getDriverApi().makeCurrent(swapChain, swapChain);
        getDriverApi().beginFrame(0, 0);

        // Draw 10 triangles, updating the vertex buffer / index buffer each time.
        size_t triangleIndex = 0;
        for (float i = -1.0f; i < 1.0f; i += 0.2f) {
            const float low = i, high = i + 0.2;
            const filament::math::float2 v[3] {{low, low}, {high, low}, {low, high}};
            triangle.updateVertices(v);

            if (updateIndices) {
                if (triangleIndex % 2 == 0) {
                    // Upload each index separately, to test offsets.
                    const TrianglePrimitive::index_type i[3] {0, 1, 2};
                    triangle.updateIndices(i + 0, 1, 0);
                    triangle.updateIndices(i + 1, 1, 1);
                    triangle.updateIndices(i + 2, 1, 2);
                } else {
                    // This effectively hides this triangle.
                    const TrianglePrimitive::index_type i[3] {0, 0, 0};
                    triangle.updateIndices(i);
                }
            }

            if (triangleIndex > 0) {
                params.flags.clear = TargetBufferFlags::NONE;
                params.flags.discardStart = TargetBufferFlags::NONE;
            }

            getDriverApi().beginRenderPass(defaultRenderTarget, params);
            getDriverApi().draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
            getDriverApi().endRenderPass();

            triangleIndex++;
        }

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

// This test renders two triangles in two separate draw calls. Between the draw calls, a uniform
// buffer object is partially updated.
TEST_F(BackendTest, BufferObjectUpdateWithOffset) {
    // Create a platform-specific SwapChain and make it current.
    auto swapChain = createSwapChain();
    getDriverApi().makeCurrent(swapChain, swapChain);

    // Create a program.
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
    Program p = shaderGen.getProgram(getDriverApi());
    p.uniformBlockBindings({{"params", 1}});
    auto program = getDriverApi().createProgram(std::move(p));

    // Create a uniform buffer.
    // We use STATIC here, even though the buffer is updated, to force the Metal backend to use a
    // GPU buffer, which is more interesting to test.
    auto ubuffer = getDriverApi().createBufferObject(sizeof(MaterialParams) + 64,
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
    getDriverApi().bindUniformBuffer(0, ubuffer);

    // Create a render target.
    auto colorTexture = getDriverApi().createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 512, 512, 1, TextureUsage::COLOR_ATTACHMENT);
    auto renderTarget = getDriverApi().createRenderTarget(
            TargetBufferFlags::COLOR0, 512, 512, 1, {{colorTexture}}, {}, {});

    // Upload uniforms for the first triangle.
    {
        MaterialParams params {
            .color = { 1.0f, 0.0f, 0.5f, 1.0f },
            .offset = { 0.0f, 0.0f, 0.0f, 0.0f }
        };
        auto* tmp = new MaterialParams(params);
        auto cb = [](void* buffer, size_t size, void* user) {
            auto* sp = (MaterialParams*) buffer;
            delete sp;
        };
        BufferDescriptor bd(tmp, sizeof(MaterialParams), cb);
        getDriverApi().updateBufferObject(ubuffer, std::move(bd), 64);
    }

    RenderPassParams params = {};
    params.flags.clear = TargetBufferFlags::COLOR;
    params.clearColor = {0.f, 0.f, 1.f, 1.f};
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;
    params.viewport.height = 512;
    params.viewport.width = 512;
    renderTriangle(renderTarget, swapChain, program, params);

    // Upload uniforms for the second triangle. To test partial buffer updates, we'll only update
    // color.b, color.a, offset.x, and offset.y.
    {
        MaterialParams params {
                .color = { 1.0f, 0.0f, 1.0f, 1.0f },
                .offset = { 0.5f, 0.5f, 0.0f, 0.0f }
        };
        auto* tmp = new MaterialParams(params);
        auto cb = [](void* buffer, size_t size, void* user) {
            auto* sp = (MaterialParams*) ((char*)buffer - offsetof(MaterialParams, color.b));
            delete sp;
        };
        BufferDescriptor bd((char*)tmp + offsetof(MaterialParams, color.b), sizeof(float) * 4, cb);
        getDriverApi().updateBufferObject(ubuffer, std::move(bd), 64 + offsetof(MaterialParams, color.b));
    }

    params.flags.clear = TargetBufferFlags::NONE;
    params.flags.discardStart = TargetBufferFlags::NONE;
    renderTriangle(renderTarget, swapChain, program, params);

    static const uint32_t expectedHash = 91322442;
    readPixelsAndAssertHash(
            "BufferObjectUpdateWithOffset", 512, 512, renderTarget, expectedHash, true);

    getDriverApi().flush();
    getDriverApi().commit(swapChain);
    getDriverApi().endFrame(0);

    getDriverApi().destroyProgram(program);
    getDriverApi().destroySwapChain(swapChain);
    getDriverApi().destroyBufferObject(ubuffer);
    getDriverApi().destroyRenderTarget(renderTarget);
    getDriverApi().destroyTexture(colorTexture);

    // This ensures all driver commands have finished before exiting the test.
    getDriverApi().finish();

    executeCommands();

    getDriver().purge();
}

} // namespace test
