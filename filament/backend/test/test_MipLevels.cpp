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

#include "ShaderGenerator.h"
#include "TrianglePrimitive.h"
#include "BackendTestUtils.h"

#include "private/backend/SamplerGroup.h"

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string vertex (R"(#version 450 core

layout(location = 0) in vec4 mesh_position;
layout(location = 0) out vec2 uv;

void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
    uv = (mesh_position.xy * 0.5 + 0.5);
}
)");

std::string fragment (R"(#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 uv;

layout(location = 0, set = 1) uniform sampler2D backend_test_sib_tex;

void main() {
    fragColor = textureLod(backend_test_sib_tex, uv, 1);
}
)");

std::string whiteFragment (R"(#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 uv;

layout(location = 0, set = 1) uniform sampler2D backend_test_sib_tex;

void main() {
    fragColor = vec4(1.0);
}
)");

}

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, SetMinMaxLevel) {
    auto& api = getDriverApi();
    api.startCapture(0);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a SwapChain and make it current.
        auto swapChain = createSwapChain();
        api.makeCurrent(swapChain, swapChain);

        // Create a program that draws only white.
        Handle<HwProgram> whiteProgram;
        {
            ShaderGenerator shaderGen(vertex, whiteFragment, sBackend, sIsMobilePlatform);
            Program p = shaderGen.getProgram(api);
            Program::Sampler sampler{utils::CString("backend_test_sib_tex"), 0};
            p.setSamplerGroup(0, ShaderStageFlags::FRAGMENT, &sampler, 1);
            whiteProgram = api.createProgram(std::move(p));
        }

        // Create a program that samples a texture.
        Handle<HwProgram> textureProgram;
        {
            SamplerInterfaceBlock sib = filament::SamplerInterfaceBlock::Builder()
                    .name("backend_test_sib")
                    .stageFlags(backend::ShaderStageFlags::FRAGMENT)
                    .add( {{"tex", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, Precision::HIGH }} )
                    .build();
            ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform, &sib);
            Program p = shaderGen.getProgram(api);
            Program::Sampler sampler{utils::CString("backend_test_sib_tex"), 0};
            p.setSamplerGroup(0, ShaderStageFlags::FRAGMENT, &sampler, 1);
            textureProgram = api.createProgram(std::move(p));
        }

        // Create a texture that has 4 mip levels. Each level is a different color.
        // Level 0: 128x128 (red)
        // Level 1:   64x64 (green)
        // Level 2:   32x32 (blue)
        // Level 3:   16x16 (yellow)
        const size_t kTextureSize = 128;
        const size_t kMipLevels = 4;
        Handle<HwTexture> texture = api.createTexture(SamplerType::SAMPLER_2D, kMipLevels,
                TextureFormat::RGBA8, 1, kTextureSize, kTextureSize, 1,
                TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT | TextureUsage::UPLOADABLE);

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

        api.beginFrame(0, 0);

        // We set the base mip to 1, and the max mip to 3
        // Level 0: 128x128 (red)
        // Level 1:   64x64 (green)             <-- base
        // Level 2:   32x32 (blue)              <--- white triangle rendered
        // Level 3:   16x16 (yellow)            <-- max
        api.setMinMaxLevels(texture, 1, 3);

        // Render a white triangle into level 2.
        // We specify mip level 2, because minMaxLevels has no effect when rendering into a texture.
        Handle<HwRenderTarget> renderTarget = api.createRenderTarget(
                TargetBufferFlags::COLOR, 32, 32, 1, 0,
                {texture, 2 /* level */, 0 /* layer */}, {}, {});
        {
            RenderPassParams params = {};
            fullViewport(params);
            params.flags.clear = TargetBufferFlags::NONE;
            params.flags.discardStart = TargetBufferFlags::NONE;
            params.flags.discardEnd = TargetBufferFlags::NONE;
            PipelineState ps = {};
            ps.program = whiteProgram;
            ps.rasterState.colorWrite = true;
            ps.rasterState.depthWrite = false;
            api.beginRenderPass(renderTarget, params);
            api.draw(ps, triangle.getRenderPrimitive(), 0, 3, 1);
            api.endRenderPass();
        }

        backend::Handle<HwRenderTarget> defaultRenderTarget = api.createDefaultRenderTarget(0);

        RenderPassParams params = {};
        fullViewport(params);
        params.flags.clear = TargetBufferFlags::COLOR;
        params.clearColor = {0.f, 0.f, 0.5f, 1.f};
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState state;
        state.program = textureProgram;
        state.rasterState.colorWrite = true;
        state.rasterState.depthWrite = false;
        state.rasterState.depthFunc = SamplerCompareFunc::A;
        state.rasterState.culling = CullingMode::NONE;

        SamplerGroup samplers(1);
        SamplerParams samplerParams {};
        samplerParams.filterMag = SamplerMagFilter::NEAREST;
        samplerParams.filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST;
        samplers.setSampler(0, { texture, samplerParams });
        backend::Handle<HwSamplerGroup> samplerGroup =
                api.createSamplerGroup(1, utils::FixedSizeString<32>("Test"));
        api.updateSamplerGroup(samplerGroup, samplers.toBufferDescriptor(api));
        api.bindSamplers(0, samplerGroup);

        // Render a triangle to the screen, sampling from mip level 1.
        // Because the min level is 1, the result color should be the white triangle drawn in the
        // previous pass.
        api.beginRenderPass(defaultRenderTarget, params);
        api.scissor(params.viewport);
        api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        // Adjust the base mip to 2.
        // Note that this is done without another call to updateSamplerGroup.
        api.setMinMaxLevels(texture, 2, 3);

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

        // Cleanup.
        api.destroySwapChain(swapChain);
        api.destroyRenderTarget(renderTarget);
        api.destroyTexture(texture);
        api.destroyProgram(whiteProgram);
        api.destroyProgram(textureProgram);
    }

    api.finish();

    executeCommands();
    getDriver().purge();
}

} // namespace test
