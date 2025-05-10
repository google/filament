/*
 * Copyright (C) 2023 The Android Open Source Project
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
 *
 */

#include "BackendTest.h"

#include "BackendTestUtils.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "TrianglePrimitive.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <stddef.h>
#include <stdint.h>

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string fragmentTexturedLod (R"(#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 uv;

layout(location = 0, set = 0) uniform sampler2D backend_test_sib_tex;

void main() {
    fragColor = textureLod(backend_test_sib_tex, uv, 1);
}
)");

}

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, TextureViewLod) {
    auto& api = getDriverApi();
    api.startCapture(0);
    Cleanup cleanup(api);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a SwapChain and make it current.
        auto swapChain = cleanup.add(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        Shader whiteShader = SharedShaders::makeShader(api, cleanup, ShaderRequest {
            .mVertexType = VertexShaderType::Textured,
            .mFragmentType = FragmentShaderType::White,
            .mUniformType = ShaderUniformType::Sampler
        });

        // Create a program that samples a texture.
        std::string vertexShader = SharedShaders::getVertexShaderText(
                VertexShaderType::Textured, ShaderUniformType::Sampler);
        filament::SamplerInterfaceBlock::SamplerInfo samplerInfo {
            "backend_test", "sib_tex", 0,
            SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, Precision::HIGH, false };
        Shader texturedShader(api, cleanup, ShaderConfig {
           .vertexShader = vertexShader,
           .fragmentShader = fragmentTexturedLod,
           .uniforms = {{
               "backend_test_sib_tex", DescriptorType::SAMPLER_2D_FLOAT, samplerInfo
           }}
        });

        // Create a texture that has 4 mip levels. Each level is a different color.
        // Level 0: 128x128 (red)
        // Level 1:   64x64 (green)
        // Level 2:   32x32 (blue)
        // Level 3:   16x16 (yellow)
        const size_t kTextureSize = 128;
        const size_t kMipLevels = 4;
        Handle<HwTexture> texture = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D,
                kMipLevels, TextureFormat::RGBA8, 1, kTextureSize, kTextureSize, 1,
                TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT
                | TextureUsage::UPLOADABLE));

        // Create image data.
        auto pixelFormat = PixelDataFormat::RGBA;
        auto pixelType = PixelDataType::UBYTE;
        size_t components; int bpp;
        getPixelInfo(pixelFormat, pixelType, components, bpp);
        uint32_t colors[] = {
                0xFF0000FF, /* red */
                0xFF00FF00, /* green */
                0xFFFF0000, /* blue */
                0xFF00FFFF, /* yellow */
        };
        for (int l = 0; l < kMipLevels; l++) {
            size_t mipSize = kTextureSize >> l;
            auto* buffer = (uint8_t*)calloc(1, mipSize * mipSize * bpp);
            fillCheckerboard<uint32_t>(buffer, mipSize, mipSize, 1, colors[l]);
            PixelBufferDescriptor descriptor(
                    buffer, mipSize * mipSize * bpp, pixelFormat, pixelType, 1, 0, 0,
                    mipSize, [](void* buffer, size_t size, void* user) { free(buffer); },
                    nullptr);
            api.update3DImage(
                    texture, l, 0, 0, 0, mipSize, mipSize, 1, std::move(descriptor));
        }

        TrianglePrimitive triangle(api);

        api.beginFrame(0, 0, 0);

        // We set the base mip to 1, and the max mip to 3
        // Level 0: 128x128 (red)
        // Level 1:   64x64 (green)             <-- base
        // Level 2:   32x32 (blue)              <--- white triangle rendered
        // Level 3:   16x16 (yellow)            <-- max
        auto texture13 = cleanup.add(api.createTextureView(texture, 1, 3));

        // Render a white triangle into level 2.
        // We specify mip level 2, because minMaxLevels has no effect when rendering into a texture.
        Handle<HwRenderTarget> renderTarget = cleanup.add(api.createRenderTarget(
                TargetBufferFlags::COLOR, 32, 32, 1, 0,
                {texture, 2 /* level */, 0 /* layer */}, {}, {}));
        {
            RenderPassParams params = {};
            fullViewport(params);
            params.flags.clear = TargetBufferFlags::NONE;
            params.flags.discardStart = TargetBufferFlags::NONE;
            params.flags.discardEnd = TargetBufferFlags::NONE;
            PipelineState ps = {};
            ps.program = whiteShader.getProgram();
            ps.rasterState.colorWrite = true;
            ps.rasterState.depthWrite = false;
            api.beginRenderPass(renderTarget, params);
            api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);
            api.endRenderPass();
        }

        backend::Handle<HwRenderTarget> defaultRenderTarget =
                cleanup.add(api.createDefaultRenderTarget(0));

        RenderPassParams params = {};
        fullViewport(params);
        params.flags.clear = TargetBufferFlags::COLOR;
        params.clearColor = {0.f, 0.f, 0.5f, 1.f};
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState state;
        state.program = texturedShader.getProgram();
        state.pipelineLayout.setLayout = { texturedShader.getDescriptorSetLayout() };
        state.rasterState.colorWrite = true;
        state.rasterState.depthWrite = false;
        state.rasterState.depthFunc = SamplerCompareFunc::A;
        state.rasterState.culling = CullingMode::NONE;

        DescriptorSetHandle descriptorSet13 = texturedShader.createDescriptorSet(api);
        api.updateDescriptorSetTexture(descriptorSet13, 0, texture13, {
                .filterMag = SamplerMagFilter::NEAREST,
                .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });

        api.bindDescriptorSet(descriptorSet13, 0, {});

        // Render a triangle to the screen, sampling from mip level 1.
        // Because the min level is 1, the result color should be the white triangle drawn in the
        // previous pass.
        api.beginRenderPass(defaultRenderTarget, params);
        api.scissor(params.viewport);
        api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        // Adjust the base mip to 2.
        auto texture22 = cleanup.add(api.createTextureView(texture, 2, 2));

        DescriptorSetHandle descriptorSet22 = texturedShader.createDescriptorSet(api);
        api.updateDescriptorSetTexture(descriptorSet22, 0, texture22, {
                .filterMag = SamplerMagFilter::NEAREST,
                .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });

        api.bindDescriptorSet(descriptorSet22, 0, {});

        // Render a second, smaller, triangle, again sampling from mip level 1.
        // This triangle should be yellow striped.
        static filament::math::float2 vertices[3] = {
                { -0.5, -0.5 },
                {  0.5, -0.5 },
                { -0.5,  0.5 }
        };
        triangle.updateVertices(vertices);
        params.flags.clear = TargetBufferFlags::NONE;
        params.flags.discardStart = TargetBufferFlags::NONE;
        api.beginRenderPass(defaultRenderTarget, params);
        api.scissor(params.viewport);
        api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        api.commit(swapChain);
        api.endFrame(0);

        api.stopCapture(0);
    }

    api.finish();

    executeCommands();
    getDriver().purge();
}

} // namespace test
