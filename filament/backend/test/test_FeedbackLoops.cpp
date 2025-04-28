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

#include "Lifetimes.h"
#include "Shader.h"
#include "Skip.h"
#include "TrianglePrimitive.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/Hash.h>
#include <utils/Log.h>

#include <fstream>
#include <string>

#include <stddef.h>
#include <stdint.h>

#ifndef FILAMENT_IOS
#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>

using namespace image;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string const fullscreenVs = R"(#version 450 core
layout(location = 0) in vec4 mesh_position;
void main() {
    // Hack: move and scale triangle so that it covers entire viewport.
    gl_Position = vec4((mesh_position.xy + 0.5) * 5.0, 0.0, 1.0);
})";

static std::string const fullscreenFs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
// This parameter is ignored for other backends.
layout(binding = 0, set = 0) uniform sampler2D test_tex;

layout(binding = 1, set = 0) uniform Params {
    highp float fbWidth;
    highp float fbHeight;
    highp float sourceLevel;
    highp float unused;
} params;

void main() {
    vec2 fbsize = vec2(params.fbWidth, params.fbHeight);
    vec2 uv = (gl_FragCoord.xy + 0.5) / fbsize;
    fragColor = textureLod(test_tex, uv, params.sourceLevel);
})";

// Selecting a NPOT texture size seems to exacerbate the bug seen with Intel GPU's.
// Note that Filament uses a higher precision format (R11F_G11F_B10F) but this does not seem
// necessary to trigger the bug.

static constexpr int kTexWidth = 360;
static constexpr int kTexHeight = 180;
static constexpr int kNumLevels = 3;
static constexpr int kNumFrames = 2;
static constexpr auto kTexFormat = filament::backend::TextureFormat::RGB8;

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace utils;

struct MaterialParams {
    float fbWidth;
    float fbHeight;
    float sourceLevel;
    float unused;
};

// TODO: This test needs work to get Metal and OpenGL to agree on results.
// The problems are caused by both uploading and rendering into the same texture, since the OpenGL
// backend's readPixels does not work correctly with textures that have image data uploaded.
TEST_F(BackendTest, FeedbackLoops) {
    NONFATAL_FAIL_IF(SkipEnvironment(OperatingSystem::APPLE, Backend::VULKAN),
            "Image is unexpectedly darker, see b/417226296");
    SKIP_IF(SkipEnvironment(OperatingSystem::APPLE, Backend::OPENGL),
            "OpenGL image is upside down due to readPixels failing for texture with uploaded image "
            "data");
    auto& api = getDriverApi();
    Cleanup cleanup(api);

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    {
        // Create a platform-specific SwapChain and make it current.
        auto swapChain = cleanup.add(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        // Create a program.
        filament::SamplerInterfaceBlock::SamplerInfo samplerInfo { "test", "tex", 0,
                SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, Precision::HIGH, false };
        Shader shader = Shader(api, cleanup, ShaderConfig {
            .vertexShader = fullscreenVs,
            .fragmentShader = fullscreenFs,
            .uniforms = {{"test_tex", DescriptorType::SAMPLER_FLOAT, samplerInfo}, {"Params"}}
        });

        TrianglePrimitive const triangle(getDriverApi());

        // Create a texture.
        auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
        Handle<HwTexture> const texture = cleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kTexFormat, 1, kTexWidth, kTexHeight, 1, usage));

        // Create ubo
        auto ubuffer = cleanup.add(api.createBufferObject(sizeof(MaterialParams),
                BufferObjectBinding::UNIFORM, BufferUsage::STATIC));

        // Create a RenderTarget for each miplevel.
        Handle<HwRenderTarget> renderTargets[kNumLevels];
        for (uint8_t level = 0; level < kNumLevels; level++) {
            slog.i << "Level " << int(level) << ": " <<
                    (kTexWidth >> level) << "x" << (kTexHeight >> level) << io::endl;
            renderTargets[level] = cleanup.add(api.createRenderTarget( TargetBufferFlags::COLOR,
                    kTexWidth >> level, kTexHeight >> level, 1, 0, { texture, level, 0 }, {}, {}));
        }

        // Fill the base level of the texture with interesting colors.
        const size_t size = kTexHeight * kTexWidth * 4;
        uint8_t* buffer = (uint8_t*) malloc(size);
        for (int r = 0, i = 0; r < kTexHeight; r++) {
            for (int c = 0; c < kTexWidth; c++, i += 4) {
                buffer[i + 0] = 0x10;
                buffer[i + 1] = 0x1f * r / (kTexHeight - 1);
                buffer[i + 2] = 0x1f * c / (kTexWidth - 1);
                buffer[i + 3] = 0xf0;
            }
         }
        auto cb = [](void* buffer, size_t size, void* user) { free(buffer); };
        PixelBufferDescriptor pb(buffer, size, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb);
        api.update3DImage(texture, 0, 0, 0, 0, kTexWidth, kTexHeight, 1, std::move(pb));

        for (int frame = 0; frame < kNumFrames; frame++) {

            // Prep for rendering.
            RenderPassParams params = {};
            params.flags.clear = TargetBufferFlags::NONE;
            params.flags.discardEnd = TargetBufferFlags::NONE;
            PipelineState state;
            state.rasterState.colorWrite = true;
            state.rasterState.depthWrite = false;
            state.rasterState.depthFunc = RasterState::DepthFunc::A;
            state.program = shader.getProgram();
            state.pipelineLayout.setLayout[0] = { shader.getDescriptorSetLayout() };

            api.makeCurrent(swapChain, swapChain);
            api.beginFrame(0, 0, 0);

            // Downsample passes
            params.flags.discardStart = TargetBufferFlags::ALL;
            state.rasterState.disableBlending();
            for (int targetLevel = 1; targetLevel < kNumLevels; targetLevel++) {
                Cleanup passCleanup(api);
                const uint32_t sourceLevel = targetLevel - 1;
                params.viewport.width = kTexWidth >> targetLevel;
                params.viewport.height = kTexHeight >> targetLevel;

                auto descriptorSet = shader.createDescriptorSet(api);
                auto textureView = passCleanup.add(api.createTextureView(texture, sourceLevel, 1));
                api.updateDescriptorSetTexture(descriptorSet, 0, textureView, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });

                UniformBindingConfig uniformBinding{
                    .binding = 1,
                    .descriptorSet = descriptorSet
                };
                shader.bindUniform<MaterialParams>(api, ubuffer, uniformBinding);
                shader.uploadUniform(api, ubuffer, uniformBinding, MaterialParams{
                    .fbWidth = float(params.viewport.width),
                    .fbHeight = float(params.viewport.height),
                    .sourceLevel = float(sourceLevel),
                });

                api.beginRenderPass(renderTargets[targetLevel], params);
                api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
                api.endRenderPass();
            }

            // Upsample passes
            params.flags.discardStart = TargetBufferFlags::NONE;
            state.rasterState.blendFunctionSrcRGB = BlendFunction::ONE;
            state.rasterState.blendFunctionDstRGB = BlendFunction::ONE;
            for (int targetLevel = kNumLevels - 2; targetLevel >= 0; targetLevel--) {
                Cleanup passCleanup(api);
                const uint32_t sourceLevel = targetLevel + 1;
                params.viewport.width = kTexWidth >> targetLevel;
                params.viewport.height = kTexHeight >> targetLevel;

                auto descriptorSet = shader.createDescriptorSet(api);
                auto textureView = passCleanup.add(api.createTextureView(texture, sourceLevel, 1));
                api.updateDescriptorSetTexture(descriptorSet, 0, textureView, {
                        .filterMag = SamplerMagFilter::LINEAR,
                        .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
                });

                UniformBindingConfig uniformBinding{
                        .binding = 1,
                        .descriptorSet = descriptorSet
                };
                shader.bindUniform<MaterialParams>(api, ubuffer, uniformBinding);
                shader.uploadUniform(api, ubuffer, uniformBinding, MaterialParams{
                        .fbWidth = float(params.viewport.width),
                        .fbHeight = float(params.viewport.height),
                        .sourceLevel = float(sourceLevel),
                });

                api.beginRenderPass(renderTargets[targetLevel], params);
                api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
                api.endRenderPass();
            }

            // Read back the render target corresponding to the base level.
            //
            // NOTE: Calling glReadPixels on any miplevel other than the base level
            // seems to be un-reliable on some GPU's.
            if (frame == kNumFrames - 1) {
                EXPECT_IMAGE(renderTargets[0], getExpectations(),
                        ScreenshotParams(kTexWidth, kTexHeight, "FeedbackLoops", 4192780705));
            }

            api.flush();
            api.commit(swapChain);
            api.endFrame(0);
            api.finish();
            executeCommands();
            getDriver().purge();
        }
    }
}

} // namespace test
