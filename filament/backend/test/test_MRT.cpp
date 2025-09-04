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

#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "TrianglePrimitive.h"

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

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

TEST_F(BackendTest, MRT) {
    DriverApi& api = getDriverApi();
    Cleanup cleanup(api);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a platform-specific SwapChain and make it current.
        auto swapChain = cleanup.add(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        Shader shader(api, cleanup, ShaderConfig{
                .vertexShader = SharedShaders::getVertexShaderText(VertexShaderType::Noop,
                        ShaderUniformType::None),
                .fragmentShader = fragment,
                .uniforms = {}
        });

        TrianglePrimitive triangle(api);

        auto defaultRenderTarget = cleanup.add(api.createDefaultRenderTarget());

        // Create two Textures.
        auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
        Handle<HwTexture> textureA = cleanup.add(api.createTexture(
                    SamplerType::SAMPLER_2D,            // target
                    1,                                  // levels
                    TextureFormat::RGBA8,               // format
                    1,                                  // samples
                    screenWidth(),                      // width
                    screenHeight(),                     // height
                    1,                                  // depth
                    usage));                            // usage
        Handle<HwTexture> textureB = cleanup.add(api.createTexture(
                    SamplerType::SAMPLER_2D,            // target
                    1,                                  // levels
                    TextureFormat::RGBA8,               // format
                    1,                                  // samples
                    screenWidth(),                      // width
                    screenHeight(),                     // height
                    1,                                  // depth
                    usage));                            // usage

        // Create a RenderTarget with two attachments.
        Handle<HwRenderTarget> renderTarget = cleanup.add(api.createRenderTarget(
                TargetBufferFlags::COLOR0 | TargetBufferFlags::COLOR1,
                // The width and height must match the width and height of the respective mip
                // level (at least for OpenGL).
                screenWidth(),                          // width
                screenHeight(),                         // height
                1,                                      // samples
                0,                                      // layerCount
                {{textureA },{textureB }},              // color
                {},                                     // depth
                {}));                                   // stencil

        PipelineState state = getColorWritePipelineState();
        shader.addProgramToPipelineState(state);

        RenderPassParams params = getClearColorRenderPass();
        params.viewport = getFullViewport();

        api.startCapture(0);

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, 0);

        // Draw a triangle.
        api.beginRenderPass(renderTarget, params);
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
