/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <private/filament/EngineEnums.h>

#include "ShaderGenerator.h"
#include "TrianglePrimitive.h"

#include <fstream>

#ifndef IOS
#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>

using namespace image;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string downsampleVs = R"(#version 450 core
layout(location = 0) in vec4 mesh_position;
void main() {
    // Hack: move and scale triangle so that it covers entire viewport.
    gl_Position = vec4((mesh_position.xy + 0.5) * 5.0, 0.0, 1.0);
})";

static std::string downsampleFs = R"(#version 450 core
layout(location = 0) out vec4 fragColor;
uniform sampler2D tex;
void main() {
    float lod = 0.0;
    vec2 uv = gl_FragCoord.xy / 256.0 + 0.5 / 256.0;
    fragColor = textureLod(tex, uv, lod);
})";

static uint32_t goldenPixelValue = 0;

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, FeedbackLoops) {
    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a platform-specific SwapChain and make it current.
        auto swapChain = createSwapChain();
        getDriverApi().makeCurrent(swapChain, swapChain);

        // Create a program.
        ProgramHandle downsampleProgram;
        {
            ShaderGenerator shaderGen(downsampleVs, downsampleFs, sBackend, sIsMobilePlatform);
            Program prog = shaderGen.getProgram();
            Program::Sampler psamplers[1];
            psamplers[0].binding = 0;
            psamplers[0].name = utils::CString("tex");
            psamplers[0].strict = false;
            prog.setSamplerGroup(0, psamplers, sizeof(psamplers) / sizeof(psamplers[0]));
            downsampleProgram = getDriverApi().createProgram(std::move(prog));
        }

        TrianglePrimitive triangle(getDriverApi());

        auto defaultRenderTarget = getDriverApi().createDefaultRenderTarget(0);

        // Create one texture with two miplevels.
        auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
        Handle<HwTexture> texture = getDriverApi().createTexture(
                    SamplerType::SAMPLER_2D,            // target
                    2,                                  // levels
                    TextureFormat::RGBA8,               // format
                    1,                                  // samples
                    512,                                // width
                    512,                                // height
                    1,                                  // depth
                    usage);                             // usage

        // Create a RenderTarget for miplevel 1.
        Handle<HwRenderTarget> renderTarget = getDriverApi().createRenderTarget(
                TargetBufferFlags::COLOR,
                256,                                       // width of miplevel
                256,                                       // height of miplevel
                1,                                         // samples
                { texture, 1, 0 },                         // color level = 1
                {},                                        // depth
                {});                                       // stencil

        // Fill the texture with interesting colors.
        const size_t size = 512 * 512 * 4;
        uint8_t* buffer = (uint8_t*) malloc(size);
        for (int r = 0, i = 0; r < 512; r++) {
            for (int c = 0; c < 512; c++, i += 4) {
                buffer[i + 0] = 0x10;
                buffer[i + 1] = 0xff * r / 511;
                buffer[i + 2] = 0xff * c / 511;
                buffer[i + 3] = 0xf0;
            }
         }
        auto cb = [](void* buffer, size_t size, void* user) { free(buffer); };
        PixelBufferDescriptor pb(buffer, size, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb);

        // Upload texture data.
        getDriverApi().update2DImage(texture, 0, 0, 0, 512, 512, std::move(pb));

        RenderPassParams params = {};
        params.viewport.left = 0;
        params.viewport.bottom = 0;
        params.viewport.width = 256;
        params.viewport.height = 256;
        params.flags.clear = TargetBufferFlags::COLOR;
        params.clearColor = {1.f, 0.f, 0.f, 1.f};
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;

        PipelineState state;
        state.program = downsampleProgram;
        state.rasterState.disableBlending();
        state.rasterState.colorWrite = true;
        state.rasterState.depthWrite = false;
        state.rasterState.depthFunc = RasterState::DepthFunc::A;
        state.rasterState.culling = CullingMode::NONE;

        backend::SamplerGroup samplers(1);
        backend::SamplerParams sparams = {};
        sparams.filterMag = SamplerMagFilter::LINEAR;
        sparams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;

        samplers.setSampler(0, texture, sparams);

        auto sgroup = getDriverApi().createSamplerGroup(samplers.getSize());
        getDriverApi().updateSamplerGroup(sgroup, std::move(samplers.toCommandStream()));

        getDriverApi().makeCurrent(swapChain, swapChain);
        getDriverApi().beginFrame(0, 0);
        getDriverApi().bindSamplers(0, sgroup);

        // Draw a triangle.
        getDriverApi().beginRenderPass(renderTarget, params);
        getDriverApi().draw(state, triangle.getRenderPrimitive());
        getDriverApi().endRenderPass();

        // Read back the current render target.
        const size_t size2 = 256 * 256 * 4;
        void* buffer2 = calloc(1, size2);
        auto cb2 = [](void* buffer, size_t size, void* user) {
            uint32_t* texels = (uint32_t*) buffer;
            goldenPixelValue = texels[0]; // <-- First column, first row of pixels.
            #ifndef IOS
            const size_t width = 256, height = 256;
            LinearImage image(width, height, 4);
            image = toLinearWithAlpha<uint8_t>(width, height, width * 4, (uint8_t*) buffer);
            std::ofstream pngstrm("feedback.png", std::ios::binary | std::ios::trunc);
            ImageEncoder::encode(pngstrm, ImageEncoder::Format::PNG, image, "", "feedback.png");
            #endif
        };
        PixelBufferDescriptor pb2(buffer2, size2, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb2);

        getDriverApi().readPixels(renderTarget, 0, 0, 256, 256, std::move(pb2));

        getDriverApi().flush();
        getDriverApi().commit(swapChain);
        getDriverApi().endFrame(0);

        getDriverApi().destroyProgram(downsampleProgram);
        getDriverApi().destroySwapChain(swapChain);
        getDriverApi().destroyRenderTarget(defaultRenderTarget);
    }

    getDriverApi().finish();
    executeCommands();
    getDriver().purge();

    const uint32_t expected = 0xf000ff10;
    printf("Pixel value is %8.8x, Expected %8.8x\n", goldenPixelValue, expected);
    EXPECT_EQ(goldenPixelValue, expected);
}

} // namespace test
