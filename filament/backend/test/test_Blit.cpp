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
    Handle<HwTexture> const srcTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a destination texture.
    Handle<HwTexture> const dstTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    // Create a RenderTarget for each texture's miplevel.
    Handle<HwRenderTarget> srcRenderTargets[kNumLevels];
    Handle<HwRenderTarget> dstRenderTargets[kNumLevels];
    for (uint8_t level = 0; level < kNumLevels; level++) {
        srcRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, 0, { srcTexture, level, 0 }, {}, {});
        dstRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kDstTexWidth >> level, kDstTexHeight >> level, 1, 0, { dstTexture, level, 0 }, {}, {});
    }

    // Do a "magnify" blit from level 1 of the source RT to the level 0 of the destination RT.
    const int srcLevel = 1;
    api.blitDEPRECATED(TargetBufferFlags::COLOR0, dstRenderTargets[0],
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTargets[srcLevel],
            {0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel}, SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    api.beginFrame(0, 0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "ColorMagnify.png" };
    api.beginFrame(0, 0, 0);
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
    Handle<HwTexture> const srcTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a destination texture.
    Handle<HwTexture> const dstTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    // Create a RenderTarget for each texture's miplevel.
    Handle<HwRenderTarget> srcRenderTargets[kNumLevels];
    Handle<HwRenderTarget> dstRenderTargets[kNumLevels];
    for (uint8_t level = 0; level < kNumLevels; level++) {
        srcRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, 0, { srcTexture, level, 0 }, {}, {});
        dstRenderTargets[level] = api.createRenderTarget( TargetBufferFlags::COLOR,
                kDstTexWidth >> level, kDstTexHeight >> level, 1, 0, { dstTexture, level, 0 }, {}, {});
    }

    // Do a "minify" blit from level 1 of the source RT to the level 0 of the destination RT.
    const int srcLevel = 1;

    api.blitDEPRECATED(TargetBufferFlags::COLOR0,
            dstRenderTargets[0], {0, 0, kDstTexWidth, kDstTexHeight},
            srcRenderTargets[srcLevel], {0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel},
            SamplerMagFilter::LINEAR);

    // Grab a screenshot.
    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "ColorMinify.png" };
    dumpScreenshot(api, dstRenderTargets[0], &params);

    // Wait for the ReadPixels result to come back.
    flushAndWait();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0xf3d9c53f;
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

TEST_F(BackendTest, ColorResolve) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 256;
    constexpr int kSrcTexHeight = 256;
    constexpr int kDstTexWidth = 256;
    constexpr int kDstTexHeight = 256;
    constexpr auto kColorTexFormat = TextureFormat::RGBA8;
    constexpr int kSampleCount = 4;

    // Create a program.
    ProgramHandle program;
    {
        ShaderGenerator shaderGen(triangleVs, triangleFs, sBackend, sIsMobilePlatform);
        Program prog = shaderGen.getProgram(api);
        prog.uniformBlockBindings({{"params", 1}});
        program = api.createProgram(std::move(prog));
    }

    // Create a VertexBuffer, IndexBuffer, and RenderPrimitive.
    TrianglePrimitive const triangle(api);

    // Create 4-sample texture.
    Handle<HwTexture> const srcColorTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kColorTexFormat, kSampleCount, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::COLOR_ATTACHMENT);

    // Create 1-sample texture.
    Handle<HwTexture> const dstColorTexture = api.createTexture(
        SamplerType::SAMPLER_2D, 1, kColorTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT);

    // Create a 4-sample render target with the 4-sample texture.
    Handle<HwRenderTarget> const srcRenderTarget = api.createRenderTarget(
            TargetBufferFlags::COLOR, kSrcTexWidth, kSrcTexHeight, kSampleCount, 0,
            {{ srcColorTexture }}, {}, {});

    // Create a 1-sample render target with the 1-sample texture.
    Handle<HwRenderTarget> const dstRenderTarget = api.createRenderTarget(
            TargetBufferFlags::COLOR, kDstTexWidth, kDstTexHeight, 1, 0,
            {{ dstColorTexture }}, {}, {});

    // Prep for rendering.
    RenderPassParams params = {};
    params.flags.clear = TargetBufferFlags::COLOR;
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;
    params.clearColor = float4(1, 1, 0, 1);
    params.viewport.width = kSrcTexWidth;
    params.viewport.height = kSrcTexHeight;

    PipelineState state = {};
    state.program = program;
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    auto ubuffer = api.createBufferObject(sizeof(MaterialParams),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
    // Draw red triangle into srcRenderTarget.
    uploadUniforms(api, ubuffer, {
        .color = float4(1, 0, 0, 1),
        .scale = float4(1, 1, 0.5, 0),
    });

    // FIXME: on Metal this triangle is not drawn. Can't understand why.
    api.beginFrame(0, 0, 0);
        api.beginRenderPass(srcRenderTarget, params);
            api.bindUniformBuffer(0, ubuffer);
            api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();
    api.endFrame(0);

    // Resolve the MSAA render target into the single-sample render target.
    api.blitDEPRECATED(TargetBufferFlags::COLOR,
            dstRenderTarget, {0, 0, kDstTexWidth, kDstTexHeight},
            srcRenderTarget, {0, 0, kSrcTexWidth, kSrcTexHeight},
            SamplerMagFilter::NEAREST);

    // Grab a screenshot.
    ScreenshotParams sparams{ kDstTexWidth, kDstTexHeight, "ColorResolve.png" };
    dumpScreenshot(api, dstRenderTarget, &sparams);

    // Wait for the ReadPixels result to come back.
    flushAndWait();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0xebfac2ef;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", sparams.pixelHashResult, expected);
    EXPECT_TRUE(sparams.pixelHashResult == expected);

    // Cleanup.
    api.destroyBufferObject(ubuffer);
    api.destroyProgram(program);
    api.destroyTexture(srcColorTexture);
    api.destroyTexture(dstColorTexture);
    api.destroyRenderTarget(srcRenderTarget);
    api.destroyRenderTarget(dstRenderTarget);
    flushAndWait();
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
            kSrcTexWidth >> level, kSrcTexHeight >> level, 1, 0, { srcTexture, level, kSrcTexLayer }, {}, {});
    Handle<HwRenderTarget> dstRenderTarget = api.createRenderTarget( TargetBufferFlags::COLOR,
            kDstTexWidth >> level, kDstTexHeight >> level, 1, 0, { dstTexture, level, kDstTexLayer }, {}, {});

    // Do a blit from kSrcTexLayer of the source RT to kDstTexLayer of the destination RT.
    const int srcLevel = 0;
    api.blitDEPRECATED(TargetBufferFlags::COLOR0, dstRenderTarget,
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTarget,
            {0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel}, SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    api.beginFrame(0, 0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "Blit2DTextureArray.png" };
    api.beginFrame(0, 0, 0);
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
                    srcRect.height, 1, 0, {srcTexture, kSrcLevel, 0}, {}, {});
    Handle<HwRenderTarget> dstRenderTarget =
            api.createRenderTarget(TargetBufferFlags::COLOR, kDstTexWidth >> kDstLevel,
                    kDstTexHeight >> kDstLevel, 1, 0, {dstTexture, kDstLevel, 0}, {}, {});

    api.blitDEPRECATED(TargetBufferFlags::COLOR0, dstRenderTarget, dstRect, srcRenderTarget, srcRect,
            SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    api.beginFrame(0, 0, 0);
    api.commit(swapChain);
    api.endFrame(0);

    // Grab a screenshot.
    ScreenshotParams params { kDstTexWidth, kDstTexHeight, "BlitRegion.png" };
    api.beginFrame(0, 0, 0);
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
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, 0, { srcTexture, level, 0 }, {}, {});
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

    api.beginFrame(0, 0, 0);

    api.blitDEPRECATED(TargetBufferFlags::COLOR0, dstRenderTarget,
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
