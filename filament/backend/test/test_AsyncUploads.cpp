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
#include "Shader.h"
#include "SharedShaders.h"
#include "Skip.h"

#include <backend/PixelBufferDescriptor.h>

#include <chrono>
#include <thread>

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;

struct AsyncState {
    bool indexBufferCreated = false;
    bool bufferObjectCreated = false;
    bool textureCreated = false;
    bool textureViewSwizzledCreated = false;
    bool indexBufferUpdated = false;
    bool bufferObjectUpdated = false;
    bool textureUpdated = false;
    bool vertexBufferSet = false;
    bool commandQueued = false;
};

static void signalCallback(void* user) {
    bool* flag = static_cast<bool*>(user);
    *flag = true;
}

TEST_F(BackendTest, BasicAsyncFlow) {
    SKIP_IF(Backend::VULKAN, "Vulkan does not support asynchronous resource uploading");
    SKIP_IF(Backend::WEBGPU, "WebGPU does not support asynchronous resource uploading");

    constexpr int kRenderTargetSize = 512;

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);
    RenderTargetHandle renderTarget = addCleanup(api.createDefaultRenderTarget());

    Shader shader = SharedShaders::makeShader(api, *mCleanup,
            ShaderRequest{
                .mVertexType = VertexShaderType::Textured,
                .mFragmentType = FragmentShaderType::Textured,
                .mUniformType = ShaderUniformType::Sampler,
            });

    RenderPassParams params = getClearColorRenderPass();
    params.viewport = getFullViewport();

    PipelineState ps = getColorWritePipelineState();
    shader.addProgramToPipelineState(ps);

    AsyncState state;

    auto waitFor = [&](const bool& flag) {
        int attempts = 0;
        while (!flag && attempts < 1000) {
            api.finish();
            executeCommands();
            getDriver().purge();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            attempts++;
        }
        EXPECT_TRUE(flag);
    };

    // 1. Asynchronous Creation

    // Create Index Buffer Asynchronously
    IndexBufferHandle ibh = addCleanup(api.createIndexBufferAsync(ElementType::UINT, 3,
            BufferUsage::STATIC, nullptr, signalCallback, &state.indexBufferCreated));

    // Create Buffer Object Asynchronously (for vertices)
    BufferObjectHandle boh =
            addCleanup(api.createBufferObjectAsync(sizeof(float2) * 3, BufferObjectBinding::VERTEX,
                    BufferUsage::STATIC, nullptr, signalCallback, &state.bufferObjectCreated));

    // Create Texture Asynchronously
    TextureHandle th = addCleanup(
            api.createTextureAsync(SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1, 2, 2, 1,
                    TextureUsage::DEFAULT, nullptr, signalCallback, &state.textureCreated));

    // Wait for creation callbacks.
    waitFor(state.indexBufferCreated);
    waitFor(state.bufferObjectCreated);
    waitFor(state.textureCreated);

    // Create Texture View Swizzle Asynchronously (now that th is created)
    TextureHandle tsv = addCleanup(api.createTextureViewSwizzleAsync(th, TextureSwizzle::CHANNEL_0,
            TextureSwizzle::CHANNEL_1, TextureSwizzle::CHANNEL_2, TextureSwizzle::CHANNEL_3,
            nullptr, signalCallback, &state.textureViewSwizzledCreated));

    waitFor(state.textureViewSwizzledCreated);

    // 2. Asynchronous Data Updates

    // Update Index Buffer Asynchronously
    uint32_t* indices = (uint32_t*) malloc(sizeof(uint32_t) * 3);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    BufferDescriptor indexData(indices, sizeof(uint32_t) * 3,
            [](void* buffer, size_t, void*) { free(buffer); });
    api.updateIndexBufferAsync(ibh, std::move(indexData), 0, nullptr, signalCallback,
            &state.indexBufferUpdated);

    // Update Buffer Object Asynchronously
    float2* vertices = (float2*) malloc(sizeof(float2) * 3);
    vertices[0] = { -1.0, -1.0 };
    vertices[1] = { 1.0, -1.0 };
    vertices[2] = { -1.0, 1.0 };
    BufferDescriptor vertexData(vertices, sizeof(float2) * 3,
            [](void* buffer, size_t, void*) { free(buffer); });
    api.updateBufferObjectAsync(boh, std::move(vertexData), 0, nullptr, signalCallback,
            &state.bufferObjectUpdated);

    // Update 3D Image Asynchronously (Texture)
    uint32_t* texData = (uint32_t*) malloc(sizeof(uint32_t) * 4);
    texData[0] = 0xFF0000FF; // Red
    texData[1] = 0xFF00FF00; // Green
    texData[2] = 0xFFFF0000; // Blue
    texData[3] = 0xFFFFFFFF; // White
    PixelBufferDescriptor pixelData(texData, sizeof(uint32_t) * 4, PixelDataFormat::RGBA,
            PixelDataType::UBYTE, [](void* buffer, size_t, void*) { free(buffer); });
    api.update3DImageAsync(th, 0, 0, 0, 0, 2, 2, 1, std::move(pixelData), nullptr, signalCallback,
            &state.textureUpdated);

    // 3. Asynchronous Setup

    AttributeArray attributes = { Attribute{ .offset = 0,
        .stride = sizeof(float2),
        .buffer = 0,
        .type = ElementType::FLOAT2,
        .flags = 0 } };
    VertexBufferInfoHandle vbih = addCleanup(api.createVertexBufferInfo(1, 1, attributes));
    VertexBufferHandle vbh = addCleanup(api.createVertexBuffer(3, vbih));

    // Set Vertex Buffer Object Asynchronously
    api.setVertexBufferObjectAsync(vbh, 0, boh, nullptr, signalCallback, &state.vertexBufferSet);

    // Queue Command Asynchronously
    api.queueCommandAsync(
            []() {
                // This command runs asynchronously.
            },
            nullptr, signalCallback, &state.commandQueued);

    // Wait for all updates and setups to complete.
    waitFor(state.indexBufferUpdated);
    waitFor(state.bufferObjectUpdated);
    waitFor(state.textureUpdated);
    waitFor(state.vertexBufferSet);
    waitFor(state.commandQueued);

    // 4. Render using the asynchronously loaded resources

    RenderPrimitiveHandle rph =
            addCleanup(api.createRenderPrimitive(vbh, ibh, PrimitiveType::TRIANGLES));

    // Bind texture
    SamplerParams samplerParams {};
    samplerParams.filterMin = SamplerMinFilter::NEAREST;
    samplerParams.filterMag = SamplerMagFilter::NEAREST;

    DescriptorSetHandle descriptorSet = shader.createDescriptorSet(api);
    // Use the swizzled view just to test it too!
    api.updateDescriptorSetTexture(descriptorSet, 0, tsv, samplerParams);
    api.bindDescriptorSet(descriptorSet, 0, {});

    {
        RenderFrame frame(api);

        api.beginRenderPass(renderTarget, params);
        ps.primitiveType = PrimitiveType::TRIANGLES;
        ps.vertexBufferInfo = vbih;
        api.bindPipeline(ps);
        api.bindRenderPrimitive(rph);
        api.draw2(0, 3, 1);
        api.endRenderPass();

        EXPECT_IMAGE(renderTarget,
                ScreenshotParams(kRenderTargetSize, kRenderTargetSize, "BasicAsyncFlow", 1));

        api.commit(swapChain);
    }
}

} // namespace test
