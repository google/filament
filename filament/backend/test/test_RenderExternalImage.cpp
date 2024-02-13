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

#include "private/backend/SamplerGroup.h"

#include <CoreVideo/CoreVideo.h>

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
    TrianglePrimitive triangle(getDriverApi());

    auto swapChain = createSwapChain();

    SamplerInterfaceBlock sib = filament::SamplerInterfaceBlock::Builder()
            .name("Test")
            .stageFlags(backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS)
            .add( {{"tex", SamplerType::SAMPLER_EXTERNAL, SamplerFormat::FLOAT, Precision::HIGH }} )
            .build();
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform, &sib);

    // Create a program that samples a texture.
    Program p = shaderGen.getProgram(getDriverApi());
    Program::Sampler sampler { utils::CString("test_tex"), 0 };
    p.setSamplerGroup(0, ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, &sampler, 1);
    backend::Handle<HwProgram> program = getDriverApi().createProgram(std::move(p));

    backend::Handle<HwRenderTarget> defaultRenderTarget = getDriverApi().createDefaultRenderTarget(0);

    // Create a texture that will be backed by an external image.
    auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
    const NativeView& view = getNativeView();
    backend::Handle<HwTexture> texture = getDriverApi().createTexture(
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
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    getDriverApi().startCapture(0);
    getDriverApi().makeCurrent(swapChain, swapChain);
    getDriverApi().beginFrame(0, 0);

    SamplerGroup samplers(1);
    samplers.setSampler(0, { texture, {} });
    backend::Handle<HwSamplerGroup> samplerGroup =
            getDriverApi().createSamplerGroup(1, utils::FixedSizeString<32>("Test"));
    getDriverApi().updateSamplerGroup(samplerGroup, samplers.toBufferDescriptor(getDriverApi()));
    getDriverApi().bindSamplers(0, samplerGroup);

    // Render a triangle.
    getDriverApi().beginRenderPass(defaultRenderTarget, params);
    getDriverApi().draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
    getDriverApi().endRenderPass();

    getDriverApi().flush();
    getDriverApi().commit(swapChain);
    getDriverApi().endFrame(0);

    getDriverApi().stopCapture(0);

    // Delete our resources.
    getDriverApi().destroyTexture(texture);
    getDriverApi().destroySamplerGroup(samplerGroup);

    // Destroy frame resources.
    getDriverApi().destroyProgram(program);
    getDriverApi().destroyRenderTarget(defaultRenderTarget);

    executeCommands();
}

TEST_F(BackendTest, RenderExternalImage) {
    TrianglePrimitive triangle(getDriverApi());

    auto swapChain = createSwapChain();

    SamplerInterfaceBlock sib = filament::SamplerInterfaceBlock::Builder()
            .name("Test")
            .stageFlags(backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS)
            .add( {{"tex", SamplerType::SAMPLER_EXTERNAL, SamplerFormat::FLOAT, Precision::HIGH }} )
            .build();
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform, &sib);

    // Create a program that samples a texture.
    Program p = shaderGen.getProgram(getDriverApi());
    Program::Sampler sampler { utils::CString("test_tex"), 0 };
    p.setSamplerGroup(0, ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, &sampler, 1);
    auto program = getDriverApi().createProgram(std::move(p));

    backend::Handle<HwRenderTarget> defaultRenderTarget = getDriverApi().createDefaultRenderTarget(0);

    // require users to create two Filament textures and have two material parameters
    // add a "plane" parameter to setExternalImage

    // Create a texture that will be backed by an external image.
    auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
    const NativeView& view = getNativeView();
    backend::Handle<HwTexture> texture = getDriverApi().createTexture(
                SamplerType::SAMPLER_EXTERNAL,      // target
                1,                                  // levels
                TextureFormat::RGBA8,               // format
                1,                                  // samples
                view.width,                         // width
                view.height,                        // height
                1,                                  // depth
                usage);                             // usage

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

    getDriverApi().setupExternalImage(pixBuffer);
    getDriverApi().setExternalImage(texture, pixBuffer);

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
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    getDriverApi().startCapture(0);
    getDriverApi().makeCurrent(swapChain, swapChain);
    getDriverApi().beginFrame(0, 0);

    SamplerGroup samplers(1);
    samplers.setSampler(0, { texture, {} });
    backend::Handle<HwSamplerGroup> samplerGroup =
            getDriverApi().createSamplerGroup(1, utils::FixedSizeString<32>("Test"));
    getDriverApi().updateSamplerGroup(samplerGroup, samplers.toBufferDescriptor(getDriverApi()));
    getDriverApi().bindSamplers(0, samplerGroup);

    // Render a triangle.
    getDriverApi().beginRenderPass(defaultRenderTarget, params);
    getDriverApi().draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
    getDriverApi().endRenderPass();

    getDriverApi().flush();
    getDriverApi().commit(swapChain);
    getDriverApi().endFrame(0);

    getDriverApi().stopCapture(0);

    // Delete our resources.
    getDriverApi().destroyTexture(texture);
    getDriverApi().destroySamplerGroup(samplerGroup);

    // Destroy frame resources.
    getDriverApi().destroyProgram(program);
    getDriverApi().destroyRenderTarget(defaultRenderTarget);

    executeCommands();
}

} // namespace test
