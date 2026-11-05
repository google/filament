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
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "Skip.h"
#include "TrianglePrimitive.h"

#include <private/filament/SamplerInterfaceBlock.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PixelBufferDescriptor.h>

#include <math/vec2.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;

// Binds a regular 2D texture to a SAMPLER_EXTERNAL descriptor. This is what happens when a Filament
// external Texture is sampled before setExternalImage() is called: the frontend binds a 1x1 RGBA8
// placeholder texture. In WebGPU, the descriptor is a `texture_external` binding, which accepts a
// single-mip 2D RGBA8Unorm texture view in place of a wgpu::ExternalTexture.
TEST_F(BackendTest, WebGPUExternalSamplerAcceptsTextureView) {
    SKIP_IF_NOT(Backend::WEBGPU, "texture_external bindings are specific to WebGPU");

    constexpr size_t kRenderTargetSize = 16;
    constexpr uint32_t kGreen = 0xFF00FF00;

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

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

    // The shared textured shader, with its sampler declared as SAMPLER_EXTERNAL. The WGSL
    // generator declares it as `texture_external` and samples it with
    // textureSampleBaseClampToEdge.
    SamplerInterfaceBlock::SamplerInfo const samplerInfo{ "backend_test", "test_tex", 0,
        SamplerType::SAMPLER_EXTERNAL, SamplerFormat::FLOAT, Precision::HIGH,
        /*filterable=*/true, /*multisample=*/false };
    Shader shader(api, *mCleanup,
            ShaderConfig{
                .vertexLanguage = filament::backend::ShaderLanguage::ESSL3,
                .vertexShader = SharedShaders::getVertexShaderText(VertexShaderType::Textured,
                        ShaderUniformType::Sampler),
                .fragmentLanguage = filament::backend::ShaderLanguage::ESSL3,
                .fragmentShader = SharedShaders::getFragmentShaderText(
                        FragmentShaderType::Textured, ShaderUniformType::Sampler),
                .uniforms = { { "test_tex", DescriptorType::SAMPLER_EXTERNAL, samplerInfo } },
            });

    TextureHandle colorTexture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, kRenderTargetSize, kRenderTargetSize, 1,
            TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE));
    RenderTargetHandle renderTarget = addCleanup(api.createRenderTarget(TargetBufferFlags::COLOR,
            kRenderTargetSize, kRenderTargetSize, 1, 0, { { colorTexture } }, {}, {}));

    TrianglePrimitive triangle(api);
    float2 const fullScreen[3] = { { -1.0, -1.0 }, { 3.0, -1.0 }, { -1.0, 3.0 } };
    triangle.updateVertices(fullScreen);

    PipelineState ps = getColorWritePipelineState();
    shader.addProgramToPipelineState(ps);
    ps.primitiveType = PrimitiveType::TRIANGLES;
    ps.vertexBufferInfo = triangle.getVertexBufferInfo();

    RenderPassParams params = getClearColorDepthRenderPass();
    params.viewport.width = kRenderTargetSize;
    params.viewport.height = kRenderTargetSize;

    // Same as Filament's placeholder for external textures: a 1x1 RGBA8 SAMPLER_2D texture.
    TextureHandle const texture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, 1, 1, 1, TextureUsage::DEFAULT));
    uint32_t* texel = static_cast<uint32_t*>(malloc(sizeof(uint32_t)));
    *texel = kGreen;
    api.update3DImage(texture, 0, 0, 0, 0, 1, 1, 1,
            PixelBufferDescriptor(texel, sizeof(uint32_t), PixelDataFormat::RGBA,
                    PixelDataType::UBYTE, [](void* buffer, size_t, void*) { free(buffer); }));

    // Sampler parameters that are not clamped by the backend, since the texture's target is not
    // SAMPLER_EXTERNAL. They must still be valid for the external texture's filtering sampler.
    SamplerParams samplerParams{};
    samplerParams.filterMag = SamplerMagFilter::LINEAR;
    samplerParams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_LINEAR;
    samplerParams.wrapS = SamplerWrapMode::REPEAT;
    samplerParams.wrapT = SamplerWrapMode::REPEAT;

    DescriptorSetHandle const descriptorSet = shader.createDescriptorSet(api);
    api.updateDescriptorSetTexture(descriptorSet, 0, texture, samplerParams);

    std::vector<uint8_t> pixels(kRenderTargetSize * kRenderTargetSize * 4);
    bool readPixelsDone = false;
    api.bindDescriptorSet(descriptorSet, 0, {});
    {
        RenderFrame frame(api);
        api.beginRenderPass(renderTarget, params);
        api.bindPipeline(ps);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();
        api.readPixels(renderTarget, 0, 0, kRenderTargetSize, kRenderTargetSize,
                PixelBufferDescriptor(pixels.data(), pixels.size(), PixelDataFormat::RGBA,
                        PixelDataType::UBYTE,
                        [](void*, size_t, void* user) { *static_cast<bool*>(user) = true; },
                        &readPixelsDone));
        api.commit(swapChain);
    }
    waitFor(readPixelsDone);

    size_t const center = (kRenderTargetSize / 2 * kRenderTargetSize + kRenderTargetSize / 2) * 4;
    uint32_t color;
    memcpy(&color, pixels.data() + center, sizeof(color));
    EXPECT_EQ(kGreen, color) << "the 2D texture bound to the texture_external slot was not sampled";
}

} // namespace test
