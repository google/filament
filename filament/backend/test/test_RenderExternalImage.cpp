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

#include "ShaderGenerator.h"
#include "TrianglePrimitive.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <CoreVideo/CoreVideo.h>

#include <stddef.h>
#include <stdint.h>

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

layout(location = 0, set = 1) uniform sampler2D test_tex;

void main() {
    fragColor = texture(test_tex, uv);
}
)");

}

namespace test {

using namespace filament;
using namespace filament::backend;

// Rendering an external image without setting any data should not crash.
TEST_F(BackendTest, RenderExternalImageWithoutSet) {
    auto& api = getDriverApi();

    TrianglePrimitive triangle(api);

    auto swapChain = createSwapChain();

    filament::SamplerInterfaceBlock::SamplerInfo samplerInfo { "test", "tex", 0,
        SamplerType::SAMPLER_EXTERNAL, SamplerFormat::FLOAT, Precision::HIGH, false };
    filamat::DescriptorSets descriptors;
    descriptors[1] = { { "test_tex", { DescriptorType::SAMPLER, ShaderStageFlags::FRAGMENT, 0 },
            samplerInfo } };
    ShaderGenerator shaderGen(
            vertex, fragment, sBackend, sIsMobilePlatform, std::move(descriptors));

    // Create a program that samples a texture.
    Program p = shaderGen.getProgram(api);
    p.descriptorBindings(1, {{"test_tex", DescriptorType::SAMPLER, 0}});
    backend::Handle<HwProgram> program = api.createProgram(std::move(p));
    DescriptorSetLayoutHandle descriptorSetLayout = api.createDescriptorSetLayout({
            {{
                     DescriptorType::SAMPLER,
                     ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, 0,
                     DescriptorFlags::NONE, 0
             }}});

    DescriptorSetHandle descriptorSet = api.createDescriptorSet(descriptorSetLayout);

    backend::Handle<HwRenderTarget> defaultRenderTarget = api.createDefaultRenderTarget(0);

    // Create a texture that will be backed by an external image.
    auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
    const NativeView& view = getNativeView();
    backend::Handle<HwTexture> texture = api.createTexture(
                SamplerType::SAMPLER_EXTERNAL,      // target
                1,                                  // levels
                TextureFormat::RGBA8,               // format
                1,                                  // samples
                view.width,                         // width
                view.height,                        // height
                1,                                  // depth
                usage);                             // usage

    RenderPassParams params = {};
    fullViewport(params);
    params.flags.clear = TargetBufferFlags::COLOR;
    params.clearColor = {0.f, 1.f, 0.f, 1.f};
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;

    PipelineState state;
    state.program = program;
    state.pipelineLayout.setLayout[1] = { descriptorSetLayout };
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    api.startCapture(0);
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0, 0);

    api.updateDescriptorSetTexture(descriptorSet, 0, texture, {});
    api.bindDescriptorSet(descriptorSet, 1, {});

    // Render a triangle.
    api.beginRenderPass(defaultRenderTarget, params);
    api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
    api.endRenderPass();

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    api.stopCapture(0);

    // Delete our resources.
    api.destroyDescriptorSet(descriptorSet);
    api.destroyDescriptorSetLayout(descriptorSetLayout);
    api.destroyTexture(texture);

    // Destroy frame resources.
    api.destroyProgram(program);
    api.destroyRenderTarget(defaultRenderTarget);

    api.finish();

    executeCommands();
}

TEST_F(BackendTest, RenderExternalImage) {
    auto& api = getDriverApi();

    TrianglePrimitive triangle(api);

    auto swapChain = createSwapChain();

    filament::SamplerInterfaceBlock::SamplerInfo samplerInfo { "test", "tex", 0,
        SamplerType::SAMPLER_EXTERNAL, SamplerFormat::FLOAT, Precision::HIGH, false };
    filamat::DescriptorSets descriptors;
    descriptors[1] = { { "test_tex", { DescriptorType::SAMPLER, ShaderStageFlags::FRAGMENT, 0 },
            samplerInfo } };
    ShaderGenerator shaderGen(
            vertex, fragment, sBackend, sIsMobilePlatform, std::move(descriptors));

    // Create a program that samples a texture.
    Program p = shaderGen.getProgram(api);
    p.descriptorBindings(1, {{"test_tex", DescriptorType::SAMPLER, 0}});
    auto program = api.createProgram(std::move(p));

    DescriptorSetLayoutHandle descriptorSetLayout = api.createDescriptorSetLayout({
            {{
                     DescriptorType::SAMPLER,
                     ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, 0,
                     DescriptorFlags::NONE, 0
             }}});

    DescriptorSetHandle descriptorSet = api.createDescriptorSet(descriptorSetLayout);

    backend::Handle<HwRenderTarget> defaultRenderTarget = api.createDefaultRenderTarget(0);

    // require users to create two Filament textures and have two material parameters
    // add a "plane" parameter to setExternalImage

    // Create a texture that will be backed by an external image.
    auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
    const NativeView& view = getNativeView();

    // Create an external image.
    CFStringRef keys[4];
    keys[0] = kCVPixelBufferCGBitmapContextCompatibilityKey;
    keys[1] = kCVPixelBufferCGImageCompatibilityKey;
    keys[2] = kCVPixelBufferOpenGLCompatibilityKey;
    keys[3] = kCVPixelBufferMetalCompatibilityKey;
    CFTypeRef values[4];
    int yes = 1;
    values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &yes);
    values[1] = values[0];
    values[2] = values[0];
    values[3] = values[0];
    CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault, (const void**) keys, (const void**) values, 4, nullptr, nullptr);
    CVPixelBufferRef pixBuffer = nullptr;
    CVReturn status =
            CVPixelBufferCreate(kCFAllocatorDefault, 1024, 1024, kCVPixelFormatType_32BGRA, options, &pixBuffer);
    assert(status == kCVReturnSuccess);

    // Fill image with checker-pattern.
    const size_t tileSize = 64;
    const uint32_t blue = 0xFF0000FF;   // BGRA format
    const uint32_t black = 0xFF000000;
    CVReturn lockStatus = CVPixelBufferLockBaseAddress(pixBuffer, 0);
    assert(lockStatus == kCVReturnSuccess);
    uint32_t* pix = (uint32_t*) CVPixelBufferGetBaseAddressOfPlane(pixBuffer, 0);
    assert(pix);
    for (size_t r = 0; r < 1024; r++) {
        for (size_t c = 0; c < 1024; c++) {
            size_t idx = r * 1024 + c;
            pix[idx] = (idx + tileSize * (r / tileSize % 2)) / tileSize % 2 == 0 ? blue : black;
        }
    }

    api.setupExternalImage(pixBuffer);
    backend::Handle<HwTexture> texture =
            api.createTextureExternalImage(TextureFormat::RGBA8, 1024, 1024, usage, pixBuffer);

    // We're now free to release the buffer.
    CVBufferRelease(pixBuffer);

    RenderPassParams params = {};
    fullViewport(params);
    params.flags.clear = TargetBufferFlags::COLOR;
    params.clearColor = {0.f, 1.f, 0.f, 1.f};
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;

    PipelineState state;
    state.program = program;
    state.pipelineLayout.setLayout[1] = { descriptorSetLayout };
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    api.startCapture(0);
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0, 0);

    api.updateDescriptorSetTexture(descriptorSet, 0, texture, {});

    api.bindDescriptorSet(descriptorSet, 1, {});

    // Render a triangle.
    api.beginRenderPass(defaultRenderTarget, params);
    api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
    api.endRenderPass();

    readPixelsAndAssertHash("RenderExternalImage", 512, 512, defaultRenderTarget, 267229901, true);

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    api.stopCapture(0);

    // Delete our resources.
    api.destroyDescriptorSet(descriptorSet);
    api.destroyDescriptorSetLayout(descriptorSetLayout);
    api.destroyTexture(texture);

    // Destroy frame resources.
    api.destroyProgram(program);
    api.destroyRenderTarget(defaultRenderTarget);

    api.finish();

    executeCommands();
}

} // namespace test
