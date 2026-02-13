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

#include "BackendTestUtils.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "Skip.h"
#include "TrianglePrimitive.h"

#include <utils/Hash.h>
#include <utils/debug.h>

#include <algorithm>
#include <fstream>

using namespace filament;
using namespace filament::backend;

namespace {

std::string fragmentFloat(R"(#version 450 core
layout(location = 0) out vec4 fragColor;
void main() {
    fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red
}
)");

std::string fragmentCoord(R"(#version 450 core
layout(location = 0) out vec4 fragColor;
void main() {
    // gl_FragCoord.xy is in pixels. (0.5, 0.5) is the center of the bottom-left pixel.
    // We want to map this to a color to verify orientation.
    // Red varies with X, Green varies with Y.
    // We assume a 64x64 texture for this test.
    vec2 coord = gl_FragCoord.xy / 64.0;
    fragColor = vec4(coord.x, coord.y, 0.0, 1.0);
}
)");
}

namespace test {

class ReadTextureTest : public BackendTest {
public:
    ReadTextureTest() { EXPECT_THAT(screenWidth(), ::testing::Eq(screenHeight())); }
};

TEST_F(ReadTextureTest, ReadTexture2D) {
    DriverApi& api = getDriverApi();
    const size_t textureSize = 64;

    // Create a headless SwapChain
    Handle<HwSwapChain> swapChain =
            addCleanup(api.createSwapChainHeadless(textureSize, textureSize, 0));
    api.makeCurrent(swapChain, swapChain);

    // Create a Texture and RenderTarget to render into.
    auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE | TextureUsage::BLIT_SRC;
    Handle<HwTexture> const texture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, textureSize, textureSize, 1, usage));

    Handle<HwRenderTarget> renderTarget = addCleanup(api.createRenderTarget(
            TargetBufferFlags::COLOR, textureSize, textureSize, 1, 0, { { texture, 0 } }, {}, {}));

    TrianglePrimitive const triangle(api);

    std::string vertexShader =
            SharedShaders::getVertexShaderText(VertexShaderType::Noop, ShaderUniformType::None);
    Shader shader(api, *mCleanup,
            ShaderConfig{
                .vertexShader = vertexShader,
                .fragmentShader = fragmentFloat,
                .uniforms = {},
            });

    RenderPassParams params = getClearColorRenderPass(math::float4(0, 0, 1, 1)); // Blue background
    params.viewport.width = textureSize;
    params.viewport.height = textureSize;

    api.beginFrame(0, 0, 0);
    api.beginRenderPass(renderTarget, params);

    PipelineState state = getColorWritePipelineState();
    shader.addProgramToPipelineState(state);
    state.primitiveType = PrimitiveType::TRIANGLES;
    state.vertexBufferInfo = triangle.getVertexBufferInfo();
    api.bindPipeline(state);
    api.bindRenderPrimitive(triangle.getRenderPrimitive());
    api.draw2(0, 3, 1);

    api.endRenderPass();

    // Read texture back
    size_t bufferSize = textureSize * textureSize * 4;
    void* buffer = calloc(1, bufferSize);

    struct UserData {
        bool finished = false;
        uint32_t expectedHash;
    } userData;

    // We'll calculate the expected hash later or just check some pixels.
    // For now, let's just check that the callback is called.

    PixelBufferDescriptor descriptor(
            buffer, bufferSize, PixelDataFormat::RGBA, PixelDataType::UBYTE, 1, 0, 0, textureSize,
            [](void* buffer, size_t size, void* user) {
                UserData* data = (UserData*) user;
                data->finished = true;

                // Basic verification: check if we have some red pixels from the triangle
                // and blue pixels from the background.
                uint8_t* pixels = (uint8_t*) buffer;
                bool foundRed = false;
                bool foundBlue = false;
                for (size_t i = 0; i < size; i += 4) {
                    if (pixels[i] == 255 && pixels[i + 1] == 0 && pixels[i + 2] == 0) {
                        foundRed = true;
                    }
                    if (pixels[i] == 0 && pixels[i + 1] == 0 && pixels[i + 2] == 255) {
                        foundBlue = true;
                    }
                }
                EXPECT_TRUE(foundRed);
                EXPECT_TRUE(foundBlue);

                free(buffer);
            },
            &userData);

    api.readTexture(texture, 0, 0, 0, 0, textureSize, textureSize, std::move(descriptor));

    api.commit(swapChain);
    api.endFrame(0);

    flushAndWait();

    EXPECT_TRUE(userData.finished);
}

TEST_F(ReadTextureTest, ReadTextureArray) {
    DriverApi& api = getDriverApi();
    const size_t textureSize = 64;
    const uint16_t layers = 2;

    Handle<HwSwapChain> swapChain =
            addCleanup(api.createSwapChainHeadless(textureSize, textureSize, 0));
    api.makeCurrent(swapChain, swapChain);

    auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE | TextureUsage::BLIT_SRC;
    Handle<HwTexture> const texture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D_ARRAY, 1,
            TextureFormat::RGBA8, 1, textureSize, textureSize, layers, usage));

    TrianglePrimitive const triangle(api);
    std::string vertexShader =
            SharedShaders::getVertexShaderText(VertexShaderType::Noop, ShaderUniformType::None);
    Shader shader(api, *mCleanup,
            ShaderConfig{ .vertexShader = vertexShader,
                .fragmentShader = fragmentFloat,
                .uniforms = {} });

    for (uint16_t layer = 0; layer < layers; ++layer) {
        Handle<HwRenderTarget> renderTarget =
                addCleanup(api.createRenderTarget(TargetBufferFlags::COLOR, textureSize,
                        textureSize, 1, 0, { { texture, 0, layer } }, {}, {}));

        RenderPassParams params =
                getClearColorRenderPass(math::float4(float(layer), 0, 1.0f - float(layer), 1));
        params.viewport.width = textureSize;
        params.viewport.height = textureSize;

        api.beginFrame(0, 0, 0);
        api.beginRenderPass(renderTarget, params);
        PipelineState state = getColorWritePipelineState();
        shader.addProgramToPipelineState(state);
        state.primitiveType = PrimitiveType::TRIANGLES;
        state.vertexBufferInfo = triangle.getVertexBufferInfo();
        api.bindPipeline(state);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();
        api.commit(swapChain);
        api.endFrame(0);
        flushAndWait();
    }

    for (uint16_t layer = 0; layer < layers; ++layer) {
        size_t bufferSize = textureSize * textureSize * 4;
        void* buffer = calloc(1, bufferSize);
        bool finished = false;

        PixelBufferDescriptor descriptor(
                buffer, bufferSize, PixelDataFormat::RGBA, PixelDataType::UBYTE, 1, 0, 0,
                textureSize,
                [](void* buffer, size_t size, void* user) {
                    bool* f = (bool*) user;
                    *f = true;
                    free(buffer);
                },
                &finished);

        api.readTexture(texture, 0, layer, 0, 0, textureSize, textureSize, std::move(descriptor));
        flushAndWait();
        EXPECT_TRUE(finished);
    }
}

TEST_F(ReadTextureTest, ReadTextureXCoordinates) {
    SKIP_IF(Backend::OPENGL, "readTexture not implemented for OpenGL");

    DriverApi& api = getDriverApi();
    const size_t textureSize = 64;

    Handle<HwSwapChain> swapChain =
            addCleanup(api.createSwapChainHeadless(textureSize, textureSize, 0));
    api.makeCurrent(swapChain, swapChain);

    auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE | TextureUsage::BLIT_SRC;
    Handle<HwTexture> const texture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, textureSize, textureSize, 1, usage));

    Handle<HwRenderTarget> renderTarget = addCleanup(api.createRenderTarget(
            TargetBufferFlags::COLOR, textureSize, textureSize, 1, 0, { { texture, 0 } }, {}, {}));

    // Draw a full-screen quad (using a large triangle)
    TrianglePrimitive triangle(api);
    const math::float2 fsVertices[3] = { { -1.0f, -1.0f }, { 3.0f, -1.0f }, { -1.0f, 3.0f } };
    triangle.updateVertices(fsVertices);

    std::string vertexShader =
            SharedShaders::getVertexShaderText(VertexShaderType::Noop, ShaderUniformType::None);
    Shader shader(api, *mCleanup,
            ShaderConfig{
                .vertexShader = vertexShader,
                .fragmentShader = fragmentCoord,
                .uniforms = {},
            });

    RenderPassParams params = getClearColorRenderPass(math::float4(0));
    params.viewport.width = textureSize;
    params.viewport.height = textureSize;

    api.beginFrame(0, 0, 0);
    api.beginRenderPass(renderTarget, params);

    PipelineState state = getColorWritePipelineState();
    shader.addProgramToPipelineState(state);
    state.primitiveType = PrimitiveType::TRIANGLES;
    state.vertexBufferInfo = triangle.getVertexBufferInfo();
    api.bindPipeline(state);
    api.bindRenderPrimitive(triangle.getRenderPrimitive());
    api.draw2(0, 3, 1);

    api.endRenderPass();

    // Read texture back
    size_t bufferSize = textureSize * textureSize * 4;
    void* buffer = calloc(1, bufferSize);

    struct UserData {
        bool finished = false;
    } userData;

    PixelBufferDescriptor descriptor(
            buffer, bufferSize, PixelDataFormat::RGBA, PixelDataType::UBYTE, 1, 0, 0, textureSize,
            [](void* buffer, size_t size, void* user) {
                UserData* data = (UserData*) user;
                data->finished = true;

                uint8_t* pixels = (uint8_t*) buffer;
                const size_t width = 64;
                const size_t height = 64;

                // Check Bottom-Left (0, 0) -> Should be black (0, 0, 0)
                // In buffer, this is at index 0
                // Y=0, X=0
                EXPECT_NEAR(pixels[0], 0, 5);
                EXPECT_NEAR(pixels[1], 0, 5);

                // Check Bottom-Right (1, 0) -> Should be Red (255, 0, 0)
                // Y=0, X=63
                size_t br_idx = (63) * 4;
                EXPECT_NEAR(pixels[br_idx], 255, 5);
                EXPECT_NEAR(pixels[br_idx + 1], 0, 5);

                // Check Top-Left (0, 1) -> Should be Green (0, 255, 0)
                // Y=63, X=0
                size_t tl_idx = (63 * width) * 4;
                EXPECT_NEAR(pixels[tl_idx], 0, 5);
                EXPECT_NEAR(pixels[tl_idx + 1], 255, 5);

                // Check Top-Right (1, 1) -> Should be Yellow (255, 255, 0)
                // Y=63, X=63
                size_t tr_idx = (63 * width + 63) * 4;
                EXPECT_NEAR(pixels[tr_idx], 255, 5);
                EXPECT_NEAR(pixels[tr_idx + 1], 255, 5);

                free(buffer);
            },
            &userData);

    api.readTexture(texture, 0, 0, 0, 0, textureSize, textureSize, std::move(descriptor));

    api.commit(swapChain);
    api.endFrame(0);

    flushAndWait();

    EXPECT_TRUE(userData.finished);
}

} // namespace test
