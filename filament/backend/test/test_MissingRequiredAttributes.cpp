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

#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "TrianglePrimitive.h"

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string vertex(R"(#version 450 core

layout(location = 0) in vec4 mesh_position;

// these attributes won't be set in the renderable
layout(location = 5) in uvec4 mesh_bone_indices;
layout(location = 6) in vec4 mesh_bone_weights;

layout(location = 0) out uvec4 indices;
layout(location = 1) out vec4 weights;

void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);

    // for a valid test, we must read from the attributes, otherwise they're optimized away
    indices = mesh_bone_indices;
    weights = mesh_bone_weights;
}
)");

}

namespace test {

using namespace filament;
using namespace filament::backend;

/**
 * This test case checks that a backend can handle missing vertex attributes and render a frame
 * successfully.
 */
TEST_F(BackendTest, MissingRequiredAttributes) {
    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        DriverApi& api = getDriverApi();
        Cleanup cleanup(api);
        // Create a platform-specific SwapChain and make it current.
        auto swapChain = cleanup.add(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        // Create a program.
        Shader shader(api, cleanup, ShaderConfig{
                .vertexShader = vertex,
                .fragmentShader = SharedShaders::getFragmentShaderText(FragmentShaderType::White,
                        ShaderUniformType::None),
        });

        auto defaultRenderTarget = cleanup.add(api.createDefaultRenderTarget(0));

        TrianglePrimitive triangle(api);

        PipelineState state = getColorWritePipelineState();
        shader.addProgramToPipelineState(state);

        RenderPassParams params = getClearColorRenderPass();
        params.viewport = getFullViewport();

        api.startCapture(0);

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, 0);

        // Render a triangle.
        api.beginRenderPass(defaultRenderTarget, params);
        state.primitiveType = PrimitiveType::TRIANGLES;
        state.vertexBufferInfo = triangle.getVertexBufferInfo();
        api.bindPipeline(state);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();

        api.flush();
        api.commit(swapChain);
        api.endFrame(0);

        api.stopCapture(0);
    }

    executeCommands();
}

} // namespace test
