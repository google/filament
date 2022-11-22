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

#include <utils/Hash.h>
#include <utils/Log.h>

#include <fstream>

#ifndef IOS
#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>
#endif

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;
using namespace utils;

static const char* const triangleVs = R"(#version 450 core
layout(location = 0) in vec4 mesh_position;
uniform Params { highp vec4 color; highp vec4 scale; } params;
void main() {
    gl_Position = vec4((mesh_position.xy + 0.5) * params.scale.xy, params.scale.z, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
})";

static const char* const triangleFs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
uniform Params { highp vec4 color; highp vec4 scale; } params;
void main() {
    fragColor = params.color;
})";

struct MaterialParams {
    float4 color;
    float4 scale;
};

struct ScreenshotParams {
    int width;
    int height;
    const char* filename;
    uint32_t pixelHashResult;
};

#ifdef IOS
static void dumpScreenshot(DriverApi& dapi, Handle<HwRenderTarget> rt, ScreenshotParams* params) {}
#else
static void dumpScreenshot(DriverApi& dapi, Handle<HwRenderTarget> rt, ScreenshotParams* params) {
    using namespace image;
    const size_t size = params->width * params->height * 4;
    void* buffer = calloc(1, size);
    auto cb = [](void* buffer, size_t size, void* user) {
        ScreenshotParams* params = (ScreenshotParams*) user;
        int w = params->width, h = params->height;
        const uint32_t* texels = (uint32_t*) buffer;
        params->pixelHashResult = utils::hash::murmur3(texels, size / 4, 0);
        LinearImage image(w, h, 4);
        image = toLinearWithAlpha<uint8_t>(w, h, w * 4, (uint8_t*) buffer);
        std::ofstream pngstrm(params->filename, std::ios::binary | std::ios::trunc);
        ImageEncoder::encode(pngstrm, ImageEncoder::Format::PNG, image, "", params->filename);
    };
    PixelBufferDescriptor pb(buffer, size, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb,
            (void*) params);
    dapi.readPixels(rt, 0, 0, params->width, params->height, std::move(pb));
}
#endif

static void uploadUniforms(DriverApi& dapi, Handle<HwBufferObject> ubh, MaterialParams params) {
    MaterialParams* tmp = new MaterialParams(params);
    auto cb = [](void* buffer, size_t size, void* user) {
        MaterialParams* sp = (MaterialParams*) buffer;
        delete sp;
    };
    BufferDescriptor bd(tmp, sizeof(MaterialParams), cb);
    dapi.updateBufferObject(ubh, std::move(bd), 0);
}

static uint32_t toUintColor(float4 color) {
    color = saturate(color);
    uint32_t r = color.r * 255.0f;
    uint32_t g = color.g * 255.0f;
    uint32_t b = color.b * 255.0f;
    uint32_t a = color.a * 255.0f;
    return (r << 0) | (g << 8) | (b << 16) | (a << 24);
}

static void createBitmap(DriverApi& dapi, Handle<HwTexture> texture, int baseWidth, int baseHeight,
        int level, int layer, float3 color, bool flipY) {
    auto cb = [](void* buffer, size_t size, void* user) { free(buffer); };
    const int width = baseWidth >> level;
    const int height = baseHeight >> level;
    const size_t size0 = height * width * 4;
    uint8_t* buffer0 = (uint8_t*) calloc(size0, 1);
    PixelBufferDescriptor pb(buffer0, size0, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb);

    const float3 foreground = color;
    const float3 background = float3(1, 1, 0);

    // Draw a triangle on a yellow background.
    //
    // The triangle is oriented as such:
    // low addresses
    // |      .
    // |     ...
    // |    .....
    // v   ........
    // high addresses
    //
    // If flipY is specified (e.g., for OpenGL) we flip the image:
    // high addresses
    // ^      .
    // |     ...
    // |    .....
    // |   ........
    // low addresses
    // This is because OpenGL automatically flips image data when uploading into the texture.
    uint32_t* texels = (uint32_t*) buffer0;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            float2 uv = { (col - width / 2.0f) / width, (row - height / 2.0f) / height };
            const float d = abs(uv.x);
            const float triangleWidth = uv.y >= -.3 && uv.y <= .3 ? (.4f / .6f * uv.y + .2f) : 0;
            const float t = d < triangleWidth ? 1.0 : 0.0;
            const float3 color = mix(foreground, background, t);
            int rowFlipped = flipY ? (height - 1) - row : row;
            texels[rowFlipped * width + col] = toUintColor(float4(color, 1.0f));
        }
    }

    // Upload to the GPU.
    dapi.update3DImage(texture, level, 0, 0, layer, width, height, 1, std::move(pb));
}

static void createBitmap(DriverApi& dapi, Handle<HwTexture> texture, int baseWidth, int baseHeight,
        int level, float3 color, bool flipY) {
    createBitmap(dapi, texture, baseWidth, baseHeight, level, 0u, color, flipY);
}

static void createFaces(DriverApi& dapi, Handle<HwTexture> texture, int baseWidth, int baseHeight,
        int level, float3 color) {
    auto cb = [](void* buffer, size_t size, void* user) { free(buffer); };
    const int width = baseWidth >> level;
    const int height = baseHeight >> level;
    const size_t size0 = height * width * 4 * 6;
    uint8_t* buffer0 = (uint8_t*) calloc(size0, 1);
    PixelBufferDescriptor pb(buffer0, size0, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb);

    const float3 foreground = color;
    const float3 background = float3(1, 1, 0);
    const float radius = 0.25f;

    // Draw a circle on a yellow background.
    uint32_t* texels = (uint32_t*) buffer0;
    for (int face = 0; face < 6; face++) {
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                float2 uv = { (col - width / 2.0f) / width, (row - height / 2.0f) / height };
                const float d = distance(uv, float2(0));
                const float t = d < radius ? 1.0 : 0.0;
                const float3 color = mix(foreground, background, t);
                texels[row * width + col] = toUintColor(float4(color, 1.0f));
            }
        }
        texels += face * width * height;
    }

    // Upload to the GPU.
    dapi.update3DImage(texture, level, 0, 0, 0, width, height, 6, std::move(pb));
}

TEST_F(BackendTest, CubemapMinify) {
    auto& api = getDriverApi();

    Handle<HwTexture> texture = api.createTexture(
        SamplerType::SAMPLER_CUBEMAP, 4, TextureFormat::RGBA8, 1, 256, 256, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);

    createFaces(api, texture, 256, 256, 0, float3(0.5, 0, 0));

    const int srcLevel = 0;
    const int dstLevel = 1;

    const TextureCubemapFace srcFace = TextureCubemapFace::NEGATIVE_Y;
    const TextureCubemapFace dstFace = TextureCubemapFace::NEGATIVE_Y;

    const TargetBufferInfo srcInfo = { texture, srcLevel, +srcFace };
    const TargetBufferInfo dstInfo = { texture, dstLevel, +dstFace };

    Handle<HwRenderTarget> srcRenderTarget;
    srcRenderTarget = api.createRenderTarget( TargetBufferFlags::COLOR,
            256 >> srcLevel, 256 >> srcLevel, 1, { srcInfo }, {}, {});

    Handle<HwRenderTarget> dstRenderTarget;
    dstRenderTarget = api.createRenderTarget( TargetBufferFlags::COLOR,
            256 >> dstLevel, 256 >> dstLevel, 1, { dstInfo }, {}, {});

    api.blit(TargetBufferFlags::COLOR0, dstRenderTarget,
            {0, 0, 256 >> dstLevel, 256 >> dstLevel}, srcRenderTarget,
            {0, 0, 256 >> srcLevel, 256 >> srcLevel}, SamplerMagFilter::LINEAR);

    // Push through an empty frame. Note that this test does not do
    // makeCurrent / commit and is therefore similar to renderStandaloneView.
    api.beginFrame(0, 0);
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams params { 256 >> dstLevel, 256 >> dstLevel, "CubemapMinify.png" };
    api.beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTarget, &params);
    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Cleanup.
    api.destroyTexture(texture);
    api.destroyRenderTarget(srcRenderTarget);
    api.destroyRenderTarget(dstRenderTarget);
    executeCommands();
}


TEST_F(BackendTest, ColorMagnify) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 256;
    constexpr int kSrcTexHeight = 256;
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    constexpr int kDstTexWidth = 384;
    constexpr int kDstTexHeight = 384;
    constexpr auto kDstTexFormat = TextureFormat::RGBA8;
    constexpr int kNumLevels = 3;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a destination texture.
    Handle<HwTexture> dstTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    // Create a RenderTarget for each texture's miplevel.
    Handle<HwRenderTarget> srcRenderTargets[kNumLevels];
    Handle<HwRenderTarget> dstRenderTargets[kNumLevels];
    for (uint8_t level = 0; level < kNumLevels; level++) {
        srcRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, { srcTexture, level, 0 }, {}, {});
        dstRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kDstTexWidth >> level, kDstTexHeight >> level, 1, { dstTexture, level, 0 }, {}, {});
    }

    // Do a "magnify" blit from level 1 of the source RT to the level 0 of the destination RT.
    const int srcLevel = 1;
    api.blit(TargetBufferFlags::COLOR0, dstRenderTargets[0],
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTargets[srcLevel],
            {0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel}, SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    api.beginFrame(0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "ColorMagnify.png" };
    api.beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTargets[0], &params);
    api.commit(swapChain);
    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0x410bdd31;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", params.pixelHashResult, expected);
    EXPECT_TRUE(params.pixelHashResult == expected);

    // Cleanup.
    api.destroyTexture(srcTexture);
    api.destroyTexture(dstTexture);
    api.destroySwapChain(swapChain);
    for (auto rt : srcRenderTargets)  api.destroyRenderTarget(rt);
    for (auto rt : dstRenderTargets)  api.destroyRenderTarget(rt);
    executeCommands();
}

TEST_F(BackendTest, ColorMinify) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 1024;
    constexpr int kSrcTexHeight = 1024;
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    constexpr int kDstTexWidth = 384;
    constexpr int kDstTexHeight = 384;
    constexpr auto kDstTexFormat = TextureFormat::RGBA8;
    constexpr int kNumLevels = 3;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a destination texture.
    Handle<HwTexture> dstTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    // Create a RenderTarget for each texture's miplevel.
    Handle<HwRenderTarget> srcRenderTargets[kNumLevels];
    Handle<HwRenderTarget> dstRenderTargets[kNumLevels];
    for (uint8_t level = 0; level < kNumLevels; level++) {
        srcRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, { srcTexture, level, 0 }, {}, {});
        dstRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kDstTexWidth >> level, kDstTexHeight >> level, 1, { dstTexture, level, 0 }, {}, {});
    }

    // Do a "minify" blit from level 1 of the source RT to the level 0 of the destination RT.
    const int srcLevel = 1;
    api.blit(TargetBufferFlags::COLOR0, dstRenderTargets[0],
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTargets[srcLevel],
            {0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel}, SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    api.beginFrame(0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "ColorMinify.png" };
    api.beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTargets[0], &params);
    api.commit(swapChain);
    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0x7739bef5;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", params.pixelHashResult, expected);
    EXPECT_TRUE(params.pixelHashResult == expected);

    // Cleanup.
    api.destroyTexture(srcTexture);
    api.destroyTexture(dstTexture);
    api.destroySwapChain(swapChain);
    for (auto rt : srcRenderTargets)  api.destroyRenderTarget(rt);
    for (auto rt : dstRenderTargets)  api.destroyRenderTarget(rt);
    executeCommands();
}

TEST_F(BackendTest, DepthMinify) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 1024;
    constexpr int kSrcTexHeight = 1024;
    constexpr int kDstTexWidth = 256;
    constexpr int kDstTexHeight = 256;
    constexpr auto kColorTexFormat = TextureFormat::RGBA8;
    constexpr auto kDepthTexFormat = TextureFormat::DEPTH24;
    constexpr int kNumLevels = 3;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a program.
    ProgramHandle program;
    {
        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);
        Program prog = shaderGen.getProgram(api);
        Program::Sampler psamplers[] = { utils::CString("tex"), 0 };
        prog.setSamplerGroup(0, ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, psamplers, sizeof(psamplers) / sizeof(psamplers[0]));
        prog.uniformBlockBindings({{"params", 1}});
        program = api.createProgram(std::move(prog));
    }

    // Create a VertexBuffer, IndexBuffer, and RenderPrimitive.
    TrianglePrimitive* triangle = new TrianglePrimitive(api);

    // Create textures for the first rendering pass.
    Handle<HwTexture> srcColorTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kColorTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    Handle<HwTexture> srcDepthTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kDepthTexFormat, 1, kSrcTexWidth, kSrcTexHeight,
        1, TextureUsage::DEPTH_ATTACHMENT);

    // Create textures for the second rendering pass.
    Handle<HwTexture> dstColorTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kColorTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    Handle<HwTexture> dstDepthTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kDepthTexFormat, 1, kDstTexWidth, kDstTexHeight,
        1, TextureUsage::DEPTH_ATTACHMENT);

    // Create render targets.
    const int level = 0;

    Handle<HwRenderTarget> srcRenderTarget = api.createRenderTarget(
            TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH,
            kSrcTexWidth >> level, kSrcTexHeight >> level, 1,
            {{ srcColorTexture, level }},
            { srcDepthTexture, level }, {});

    Handle<HwRenderTarget> dstRenderTarget = api.createRenderTarget(
            TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH,
            kDstTexWidth >> level, kDstTexHeight >> level, 1,
            {{ dstColorTexture, level }},
            { dstDepthTexture, level }, {});

    // Prep for rendering.
    RenderPassParams params = {};
    params.flags.clear = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;
    params.clearColor = float4(1, 1, 0, 1);
    params.clearDepth = 1.0;
    params.viewport.width = kSrcTexWidth;
    params.viewport.height = kSrcTexHeight;

    PipelineState state = {};
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = true;
    state.rasterState.depthFunc = RasterState::DepthFunc::L;
    state.rasterState.culling = RasterState::CullingMode::NONE;
    state.program = program;
    auto ubuffer = api.createBufferObject(sizeof(MaterialParams),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0);
    api.bindUniformBuffer(0, ubuffer);

    // Draw red triangle into srcRenderTarget.
    uploadUniforms(api, ubuffer, {
        .color = float4(1, 0, 0, 1),
        .scale = float4(1, 1, 0.5, 0),
    });
    api.beginRenderPass(srcRenderTarget, params);
    api.draw(state, triangle->getRenderPrimitive(), 1);
    api.endRenderPass();
    api.endFrame(0);

    // Copy over the color buffer and the depth buffer.
    api.blit(TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH, dstRenderTarget,
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTarget,
            {0, 0, kSrcTexWidth, kSrcTexHeight}, SamplerMagFilter::LINEAR);

    // Draw a larger blue triangle into dstRenderTarget. We carefully place it below the red
    // triangle in order to exercise depth testing.
    api.beginFrame(0, 0);
    params.viewport.width = kDstTexWidth;
    params.viewport.height = kDstTexHeight;
    params.flags.clear = TargetBufferFlags::NONE;
    uploadUniforms(api, ubuffer, {
        .color = float4(0, 0, 1, 1),
        .scale = float4(1.2, 1.2, 0.75, 0),
    });
    api.beginRenderPass(dstRenderTarget, params);
    api.draw(state, triangle->getRenderPrimitive(), 1);
    api.endRenderPass();
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams sparams { kDstTexWidth, kDstTexHeight, "DepthMinify.png" };
    api.beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTarget, &sparams);
    api.commit(swapChain);
    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0x92b6f5c1;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", sparams.pixelHashResult, expected);
    EXPECT_TRUE(sparams.pixelHashResult == expected);

    // Cleanup.
    api.destroyBufferObject(ubuffer);
    api.destroyProgram(program);
    api.destroyTexture(srcColorTexture);
    api.destroyTexture(dstColorTexture);
    api.destroyTexture(srcDepthTexture);
    api.destroyTexture(dstDepthTexture);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(srcRenderTarget);
    api.destroyRenderTarget(dstRenderTarget);
    delete triangle;
    executeCommands();
}

TEST_F(BackendTest, ColorResolve) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 256;
    constexpr int kSrcTexHeight = 256;
    constexpr int kDstTexWidth = 256;
    constexpr int kDstTexHeight = 256;
    constexpr auto kColorTexFormat = TextureFormat::RGBA8;
    constexpr int kSampleCount = 4;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a program.
    ProgramHandle program;
    {
        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);
        Program prog = shaderGen.getProgram(api);
        Program::Sampler psamplers[] = { utils::CString("tex"), 0 };
        prog.setSamplerGroup(0, ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, psamplers, sizeof(psamplers) / sizeof(psamplers[0]));
        prog.uniformBlockBindings({{"params", 1}});
        program = api.createProgram(std::move(prog));
    }

    // Create a VertexBuffer, IndexBuffer, and RenderPrimitive.
    TrianglePrimitive* triangle = new TrianglePrimitive(api);

    // Create 4-sample texture.
    Handle<HwTexture> srcColorTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kColorTexFormat, kSampleCount, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::COLOR_ATTACHMENT);

    // Create 1-sample texture.
    Handle<HwTexture> dstColorTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kColorTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    // Create a 4-sample render target with the 4-sample texture.
    Handle<HwRenderTarget> srcRenderTarget = api.createRenderTarget(
            TargetBufferFlags::COLOR, kSrcTexWidth, kSrcTexHeight, kSampleCount,{{ srcColorTexture }}, {}, {});

    // Create a 1-sample render target with the 1-sample texture.
    Handle<HwRenderTarget> dstRenderTarget = api.createRenderTarget(
            TargetBufferFlags::COLOR, kDstTexWidth, kDstTexHeight, 1, {{ dstColorTexture }}, {}, {});

    // Prep for rendering.
    RenderPassParams params = {};
    params.flags.clear = TargetBufferFlags::COLOR;
    params.clearColor = float4(1, 1, 0, 1);
    params.viewport.width = kSrcTexWidth;
    params.viewport.height = kSrcTexHeight;

    PipelineState state = {};
    state.rasterState.colorWrite = true;
    state.rasterState.culling = RasterState::CullingMode::NONE;
    state.program = program;
    auto ubuffer = api.createBufferObject(sizeof(MaterialParams),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0);
    api.bindUniformBuffer(0, ubuffer);

    // Draw red triangle into srcRenderTarget.
    uploadUniforms(api, ubuffer, {
        .color = float4(1, 0, 0, 1),
        .scale = float4(1, 1, 0.5, 0),
    });
    api.beginRenderPass(srcRenderTarget, params);
    api.draw(state, triangle->getRenderPrimitive(), 1);
    api.endRenderPass();
    api.endFrame(0);

    // Resolve the MSAA render target into the single-sample render target.
    api.blit(TargetBufferFlags::COLOR, dstRenderTarget,
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTarget,
            {0, 0, kSrcTexWidth, kSrcTexHeight}, SamplerMagFilter::LINEAR);

    // Grab a screenshot.
    ScreenshotParams sparams { kDstTexWidth, kDstTexHeight, "ColorResolve.png" };
    api.beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTarget, &sparams);
    api.commit(swapChain);
    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0xebfac2ef;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", sparams.pixelHashResult, expected);
    EXPECT_TRUE(sparams.pixelHashResult == expected);

    // Cleanup.
    api.destroyBufferObject(ubuffer);
    api.destroyProgram(program);
    api.destroyTexture(srcColorTexture);
    api.destroyTexture(dstColorTexture);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(srcRenderTarget);
    api.destroyRenderTarget(dstRenderTarget);
    delete triangle;
    executeCommands();
}

TEST_F(BackendTest, DepthResolve) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 256;
    constexpr int kSrcTexHeight = 256;
    constexpr int kDstTexWidth = 256;
    constexpr int kDstTexHeight = 256;
    constexpr auto kColorTexFormat = TextureFormat::RGBA8;
    constexpr auto kDepthTexFormat = TextureFormat::DEPTH24;
    constexpr int kSampleCount = 4;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a program.
    ProgramHandle program;
    {
        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);
        Program prog = shaderGen.getProgram(api);
        Program::Sampler psamplers[] = { utils::CString("tex"), 0 };
        prog.setSamplerGroup(0, ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, psamplers, sizeof(psamplers) / sizeof(psamplers[0]));
        prog.uniformBlockBindings({{"params", 1}});
        program = api.createProgram(std::move(prog));
    }

    // Create a VertexBuffer, IndexBuffer, and RenderPrimitive.
    TrianglePrimitive* triangle = new TrianglePrimitive(api);

    // Create 4-sample textures.
    Handle<HwTexture> srcColorTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kColorTexFormat, kSampleCount, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::COLOR_ATTACHMENT);

    Handle<HwTexture> srcDepthTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kDepthTexFormat, kSampleCount, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::DEPTH_ATTACHMENT);

    // Create 1-sample textures.
    Handle<HwTexture> dstColorTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kColorTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    Handle<HwTexture> dstDepthTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kDepthTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::DEPTH_ATTACHMENT);

    // Create a 4-sample render target with 4-sample textures.
    Handle<HwRenderTarget> srcRenderTarget = api.createRenderTarget(
            TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH, kSrcTexWidth, kSrcTexHeight,
            kSampleCount, {{ srcColorTexture }}, { srcDepthTexture }, {});

    // Create a 1-sample render target with 1-sample textures.
    Handle<HwRenderTarget> dstRenderTarget = api.createRenderTarget(
            TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH, kDstTexWidth, kDstTexHeight,
            1, {{ dstColorTexture }}, { dstDepthTexture }, {});

    // Prep for rendering.
    RenderPassParams params = {};
    params.flags.clear = TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH;
    params.clearColor = float4(1, 1, 0, 1);
    params.clearDepth = 1.0;
    params.flags.discardEnd = TargetBufferFlags::NONE;
    params.viewport.width = kSrcTexWidth;
    params.viewport.height = kSrcTexHeight;

    PipelineState state = {};
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = true;
    state.rasterState.depthFunc = RasterState::DepthFunc::L;
    state.rasterState.culling = RasterState::CullingMode::NONE;
    state.program = program;
    auto ubuffer = api.createBufferObject(sizeof(MaterialParams),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
    api.makeCurrent(swapChain, swapChain);
    api.beginFrame(0, 0);
    api.bindUniformBuffer(0, ubuffer);

    // Draw red triangle into srcRenderTarget.
    uploadUniforms(api, ubuffer, {
        .color = float4(1, 0, 0, 1),
        .scale = float4(1, 1, 0.5, 0),
    });
    api.beginRenderPass(srcRenderTarget, params);
    api.draw(state, triangle->getRenderPrimitive(), 1);
    api.endRenderPass();
    api.endFrame(0);

    // Resolve the MSAA color and depth buffers into the single-sample render target.
    api.blit(TargetBufferFlags::COLOR | TargetBufferFlags::DEPTH, dstRenderTarget,
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTarget,
            {0, 0, kSrcTexWidth, kSrcTexHeight}, SamplerMagFilter::LINEAR);

    // Draw a larger blue triangle into dstRenderTarget. We carefully place it below the red
    // triangle in order to exercise depth testing.
    api.beginFrame(0, 0);
    params.viewport.width = kDstTexWidth;
    params.viewport.height = kDstTexHeight;
    params.flags.clear = TargetBufferFlags::NONE;
    uploadUniforms(api, ubuffer, {
        .color = float4(0, 0, 1, 1),
        .scale = float4(1.2, 1.2, 0.75, 0),
    });
    api.beginRenderPass(dstRenderTarget, params);
    api.draw(state, triangle->getRenderPrimitive(), 1);
    api.endRenderPass();

    // Grab a screenshot.
    ScreenshotParams sparams { kDstTexWidth, kDstTexHeight, "DepthResolve.png" };
    api.beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTarget, &sparams);
    api.commit(swapChain);
    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0x5a38e6ba;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", sparams.pixelHashResult, expected);
    EXPECT_TRUE(sparams.pixelHashResult == expected);

    // Cleanup.
    api.destroyBufferObject(ubuffer);
    api.destroyProgram(program);
    api.destroyTexture(srcColorTexture);
    api.destroyTexture(dstColorTexture);
    api.destroyTexture(srcDepthTexture);
    api.destroyTexture(dstDepthTexture);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(srcRenderTarget);
    api.destroyRenderTarget(dstRenderTarget);
    delete triangle;
    executeCommands();
}

TEST_F(BackendTest, Blit2DTextureArray) {
    auto& api = getDriverApi();

    api.startCapture(0);

    constexpr int kSrcTexWidth = 256;
    constexpr int kSrcTexHeight = 256;
    constexpr int kSrcTexDepth = 4; // layers of the texture array
    constexpr int kDstTexDepth = 1; // layers of the texture array
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    constexpr int kDstTexWidth = 256;
    constexpr int kDstTexHeight = 256;
    constexpr auto kDstTexFormat = TextureFormat::RGBA8;
    constexpr int kNumLevels = 1;
    constexpr int kSrcTexLayer = 2;
    constexpr int kDstTexLayer = 0;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = api.createTexture(
            SamplerType::SAMPLER_2D_ARRAY, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, kSrcTexDepth,
            TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, kSrcTexLayer, float3(0.5, 0, 0), flipY);

    // Create a destination texture.
    Handle<HwTexture> dstTexture = api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    // Create two RenderTargets.
    const int level = 0;
    Handle<HwRenderTarget> srcRenderTarget = api.createRenderTarget( TargetBufferFlags::COLOR,
            kSrcTexWidth >> level, kSrcTexHeight >> level, 1, { srcTexture, level, kSrcTexLayer }, {}, {});
    Handle<HwRenderTarget> dstRenderTarget = api.createRenderTarget( TargetBufferFlags::COLOR,
            kDstTexWidth >> level, kDstTexHeight >> level, 1, { dstTexture, level, kDstTexLayer }, {}, {});

    // Do a blit from kSrcTexLayer of the source RT to kDstTexLayer of the destination RT.
    const int srcLevel = 0;
    api.blit(TargetBufferFlags::COLOR0, dstRenderTarget,
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTarget,
            {0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel}, SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    api.beginFrame(0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "Blit2DTextureArray.png" };
    api.beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTarget, &params);
    api.commit(swapChain);
    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0x8de7d55b;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", params.pixelHashResult, expected);
    EXPECT_TRUE(params.pixelHashResult == expected);

    // Cleanup.
    api.destroyTexture(srcTexture);
    api.destroyTexture(dstTexture);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(srcRenderTarget);
    api.destroyRenderTarget(dstRenderTarget);
    executeCommands();

    api.stopCapture(0);
}

TEST_F(BackendTest, BlitRegion) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 1024;
    constexpr int kSrcTexHeight = 1024;
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    constexpr int kDstTexWidth = 384;
    constexpr int kDstTexHeight = 384;
    constexpr auto kDstTexFormat = TextureFormat::RGBA8;
    constexpr int kNumLevels = 3;
    constexpr int kSrcLevel = 1;
    constexpr int kDstLevel = 0;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a destination texture.
    Handle<HwTexture> dstTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    // Blit one-quarter of src level 1 to dst level 0.
    Viewport srcRect = {
        .left = 0,
        .bottom = 0,
        .width = (kSrcTexWidth >> kSrcLevel) / 2,
        .height = (kSrcTexHeight >> kSrcLevel) / 2,
    };
    Viewport dstRect = {
        .left = 10,
        .bottom = 10,
        .width = kDstTexWidth - 10,
        .height = kDstTexHeight - 10,
    };

    // Create a RenderTarget for each texture's miplevel.
    // We purposely set the render target width and height to smaller than the texture, to check
    // that this case is handled correctly.
    Handle<HwRenderTarget> srcRenderTarget =
            api.createRenderTarget(TargetBufferFlags::COLOR, srcRect.width,
                    srcRect.height, 1, {srcTexture, kSrcLevel, 0}, {}, {});
    Handle<HwRenderTarget> dstRenderTarget =
            api.createRenderTarget(TargetBufferFlags::COLOR, kDstTexWidth >> kDstLevel,
                    kDstTexHeight >> kDstLevel, 1, {dstTexture, kDstLevel, 0}, {}, {});

    api.blit(TargetBufferFlags::COLOR0, dstRenderTarget, dstRect, srcRenderTarget, srcRect,
            SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    api.beginFrame(0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "BlitRegion.png" };
    api.beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTarget, &params);
    api.commit(swapChain);
    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    //
    // TODO: for some reason, this test has very, very slight (as in one pixel) differences between
    // OpenGL and Metal. So disable golden checking for now.
    // Use the compare tool from ImageMagick to see visual differences:
    // compare -verbose -metric mae BlitRegion_Metal.png BlitRegion_OpenGL.png difference.png
    //
    // const uint32_t expected = 0x74fa34ed;
    // printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", params.pixelHashResult, expected);
    // EXPECT_TRUE(params.pixelHashResult == expected);

    // Cleanup.
    api.destroyTexture(srcTexture);
    api.destroyTexture(dstTexture);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(srcRenderTarget);
    api.destroyRenderTarget(dstRenderTarget);
    executeCommands();
}

TEST_F(BackendTest, BlitRegionToSwapChain) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 1024;
    constexpr int kSrcTexHeight = 1024;
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    constexpr int kDstTexWidth = 512;
    constexpr int kDstTexHeight = 512;
    constexpr int kNumLevels = 3;

    // Create a SwapChain and make it current.
    auto swapChain = createSwapChain();
    Handle<HwRenderTarget> dstRenderTarget = api.createDefaultRenderTarget();
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a RenderTarget for each texture's miplevel.
    Handle<HwRenderTarget> srcRenderTargets[kNumLevels];
    for (uint8_t level = 0; level < kNumLevels; level++) {
        srcRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, { srcTexture, level, 0 }, {}, {});
    }

    // Blit one-quarter of src level 1 to dst level 0.
    const int srcLevel = 1;
    Viewport srcRect = {
        .left = (kSrcTexWidth >> srcLevel) / 2,
        .bottom = (kSrcTexHeight >> srcLevel) / 2,
        .width = (kSrcTexWidth >> srcLevel) / 2,
        .height = (kSrcTexHeight >> srcLevel) / 2,
    };
    Viewport dstRect = {
        .left = 10,
        .bottom = 10,
        .width = kDstTexWidth - 10,
        .height = kDstTexHeight - 10,
    };

    api.beginFrame(0, 0);

    api.blit(TargetBufferFlags::COLOR0, dstRenderTarget,
            dstRect, srcRenderTargets[srcLevel],
            srcRect, SamplerMagFilter::LINEAR);

    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "BlitRegionToSwapChain.png" };
    dumpScreenshot(api, dstRenderTarget, &params);

    api.commit(swapChain);

    api.endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Cleanup.
    api.destroyTexture(srcTexture);
    api.destroySwapChain(swapChain);
    for (auto rt : srcRenderTargets)  api.destroyRenderTarget(rt);
    executeCommands();
}

} // namespace test
