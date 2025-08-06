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

#include "ImageExpectations.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "Skip.h"
#include "TrianglePrimitive.h"

namespace test {

using namespace filament;
using namespace filament::backend;

// Uniform config for writing MaterialParams to the shader uniform with 64 bytes of padding.
const UniformBindingConfig kBindingConfig = {
        .dataSize = sizeof(SimpleMaterialParams),
        .bufferSize = sizeof(SimpleMaterialParams) + 64,
        .byteOffset = 64
};

class BufferUpdatesTest : public BackendTest {
public:
    BufferUpdatesTest() : mCleanup(getDriverApi()) {}

protected:
    Shader createShader() {
        return SharedShaders::makeShader(getDriverApi(), mCleanup, ShaderRequest{
                .mVertexType = VertexShaderType::Simple,
                .mFragmentType = FragmentShaderType::SolidColored,
                .mUniformType = ShaderUniformType::SimpleWithPadding
        });
    }

    Cleanup mCleanup;
};

TEST_F(BufferUpdatesTest, VertexBufferUpdate) {
    const bool largeBuffers = false;

    // If updateIndices is true, then even-numbered triangles will have their indices set to
    // {0, 0, 0}, effectively "hiding" every other triangle.
    const bool updateIndices = true;

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        auto& api = getDriverApi();
        Cleanup cleanup(api);

        // Create a platform-specific SwapChain and make it current.
        auto swapChain = cleanup.add(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        Shader shader = createShader();

        auto defaultRenderTarget = cleanup.add(api.createDefaultRenderTarget());

        // To test large buffers (which exercise a different code path) create an extra large
        // buffer. Only the first 3 vertices will be used.
        TrianglePrimitive triangle(api, largeBuffers);

        PipelineState state = getColorWritePipelineState();
        shader.addProgramToPipelineState(state);

        RenderPassParams params = getClearColorRenderPass();
        params.viewport = getFullViewport();

        // Create a uniform buffer.
        // We use STATIC here, even though the buffer is updated, to force the Metal backend to use
        // a GPU buffer, which is more interesting to test.
        auto ubuffer = cleanup.add(api.createBufferObject(sizeof(SimpleMaterialParams) + 64,
                BufferObjectBinding::UNIFORM, BufferUsage::STATIC));

        shader.bindUniform<SimpleMaterialParams>(api, ubuffer, kBindingConfig);

        api.startCapture(0);

        // Upload the uniform, but with an offset to accommodate the padding in the shader's
        // uniform definition.
        shader.uploadUniform(api, ubuffer, kBindingConfig, SimpleMaterialParams{
                .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                .scaleMinusOne = { 0.0, 0.0, 0.0, 0.0 },
                .offset = { 0.0f, 0.0f, 0.0f, 0.0f }
        });

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, 0);

        // Draw 10 triangles, updating the vertex buffer / index buffer each time.
        size_t triangleIndex = 0;
        for (float i = -1.0f; i < 1.0f; i += 0.2f) {
            const float low = i, high = i + 0.2;
            const filament::math::float2 v[3]{{ low,  low },
                                              { high, low },
                                              { low,  high }};
            triangle.updateVertices(v);

            if (updateIndices) {
                if (triangleIndex % 2 == 0) {
                    // Upload each index separately, to test offsets.
                    const TrianglePrimitive::index_type i[3]{ 0, 1, 2 };
                    triangle.updateIndices(i + 0, 1, 0);
                    triangle.updateIndices(i + 1, 1, 1);
                    triangle.updateIndices(i + 2, 1, 2);
                } else {
                    // This effectively hides this triangle.
                    const TrianglePrimitive::index_type i[3]{ 0, 0, 0 };
                    triangle.updateIndices(i);
                }
            }

            if (triangleIndex > 0) {
                params.flags.clear = TargetBufferFlags::NONE;
                params.flags.discardStart = TargetBufferFlags::NONE;
            }

            api.beginRenderPass(defaultRenderTarget, params);
            state.primitiveType = PrimitiveType::TRIANGLES;
            state.vertexBufferInfo = triangle.getVertexBufferInfo();
            api.bindPipeline(state);
            api.bindRenderPrimitive(triangle.getRenderPrimitive());
            api.draw2(0, 3, 1);
            api.endRenderPass();

            triangleIndex++;
        }

        api.flush();
        api.commit(swapChain);
        api.endFrame(0);

        api.stopCapture(0);
    }

    executeCommands();
}

// This test renders two triangles in two separate draw calls. Between the draw calls, a uniform
// buffer object is partially updated.
TEST_F(BufferUpdatesTest, BufferObjectUpdateWithOffset) {
    NONFATAL_FAIL_IF(SkipEnvironment(OperatingSystem::APPLE, Backend::VULKAN),
            "All values including alpha are written as 0, see b/417254943");

    auto& api = getDriverApi();
    Cleanup cleanup(api);

    const TrianglePrimitive triangle(api);

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = cleanup.add(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    // Create a program.
    Shader shader = createShader();

    // Create a uniform buffer.
    // We use STATIC here, even though the buffer is updated, to force the Metal backend to use a
    // GPU buffer, which is more interesting to test.
    auto ubuffer = cleanup.add(api.createBufferObject(sizeof(SimpleMaterialParams) + 64,
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC));

    shader.bindUniform<SimpleMaterialParams>(api, ubuffer, kBindingConfig);

    // Create a render target.
    auto colorTexture =
            cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1,
                    screenWidth(), screenHeight(), 1, TextureUsage::COLOR_ATTACHMENT));
    auto renderTarget = cleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR0, screenWidth(),
            screenHeight(), 1, 0, { { colorTexture } }, {}, {}));

    // Upload uniforms for the first triangle.
    // Upload the uniform, but with an offset to accommodate the padding in the shader's
    // uniform definition.
    shader.uploadUniform(api, ubuffer, kBindingConfig, SimpleMaterialParams{
            .color = { 1.0f, 0.0f, 0.5f, 1.0f },
            .scaleMinusOne = { 0.0f, 0.0f, 0.0f, 0.0f },
            .offset = { 0.0f, 0.0f, 0.0f, 0.0f }
    });

    PipelineState state = getColorWritePipelineState();
    shader.addProgramToPipelineState(state);
    state.primitiveType = PrimitiveType::TRIANGLES;
    state.vertexBufferInfo = triangle.getVertexBufferInfo();

    {
        RenderFrame frame(api);

        RenderPassParams clearParams = getClearColorRenderPass();
        clearParams.viewport.height = screenWidth();
        clearParams.viewport.width = screenHeight();

        api.beginRenderPass(renderTarget, clearParams);
        api.bindPipeline(state);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();

        // Upload uniforms for the second triangle. To test partial buffer updates, we'll only
        // update color.b, color.a, scaleMinusOne, offset.x, and offset.y.
        const UniformBindingConfig partialBindingConfig = {
                .dataSize = sizeof(float) * 8,
                .bufferSize = sizeof(SimpleMaterialParams) + 64,
                .byteOffset = 64 + offsetof(SimpleMaterialParams, color.b) };
        shader.uploadUniform(api, ubuffer, partialBindingConfig,
                std::array<float, 8>{
                        1.0f, 1.0f, // color.b, color.a
                        0.0f, 0.0f, 0.0f, 0.0f, // scale
                        0.5f, 0.5f // offset.x, offset.y
                });

        RenderPassParams noClearParams = getNoClearRenderPass();
        noClearParams.viewport.height = screenWidth();
        noClearParams.viewport.width = screenHeight();

        api.beginRenderPass(renderTarget, noClearParams);
        api.bindPipeline(state);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();
    }


    EXPECT_IMAGE(renderTarget, getExpectations(),
            ScreenshotParams(screenWidth(), screenHeight(), "BufferObjectUpdateWithOffset",
                    2320747245));

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    // This ensures all driver commands have finished before exiting the test.
    api.finish();

    executeCommands();

    getDriver().purge();
}

} // namespace test
