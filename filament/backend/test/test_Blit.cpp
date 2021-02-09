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

#include <utils/Hash.h>
#include <utils/Log.h>

#include <fstream>

#ifndef IOS
#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>
#endif

static uint32_t sPixelHashResult = 0;

static constexpr int kDstTexWidth = 384;
static constexpr int kDstTexHeight = 384;
static constexpr auto kDstTexFormat = filament::backend::TextureFormat::RGBA8;

static constexpr int kNumLevels = 3;

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;
using namespace utils;

struct MaterialParams {
    float fbWidth;
    float fbHeight;
    float sourceLevel;
    float unused;
};

static void uploadUniforms(DriverApi& dapi, Handle<HwUniformBuffer> ubh, MaterialParams params) {
    MaterialParams* tmp = new MaterialParams(params);
    auto cb = [](void* buffer, size_t size, void* user) {
        MaterialParams* sp = (MaterialParams*) buffer;
        delete sp;
    };
    BufferDescriptor bd(tmp, sizeof(MaterialParams), cb);
    dapi.loadUniformBuffer(ubh, std::move(bd));
}

#ifdef IOS
static void dumpScreenshot(DriverApi& dapi, Handle<HwRenderTarget> rt, const char* filename) {}
#else
static void dumpScreenshot(DriverApi& dapi, Handle<HwRenderTarget> rt, const char* filename) {
    using namespace image;
    const size_t size = kDstTexWidth * kDstTexHeight * 4;
    void* buffer = calloc(1, size);
    auto cb = [](void* buffer, size_t size, void* user) {
        const char* file = (char*) user;
        int w = kDstTexWidth, h = kDstTexHeight;
        const uint32_t* texels = (uint32_t*) buffer;
        sPixelHashResult = utils::hash::murmur3(texels, size / 4, 0);
        LinearImage image(w, h, 4);
        image = toLinearWithAlpha<uint8_t>(w, h, w * 4, (uint8_t*) buffer);
        std::ofstream pngstrm(file, std::ios::binary | std::ios::trunc);
        ImageEncoder::encode(pngstrm, ImageEncoder::Format::PNG, image, "", file);
    };
    PixelBufferDescriptor pb(buffer, size, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb,
            (void*) filename);
    dapi.readPixels(rt, 0, 0, kDstTexWidth, kDstTexHeight, std::move(pb));
}
#endif

static uint32_t toUintColor(float4 color) {
    color = saturate(color);
    uint32_t r = color.r * 255.0;
    uint32_t g = color.g * 255.0;
    uint32_t b = color.b * 255.0;
    uint32_t a = color.a * 255.0;
    return (r << 0) | (g << 8) | (b << 16) | (a << 24);
}

static void createBitmap(DriverApi& dapi, Handle<HwTexture> texture, int baseWidth, int baseHeight,
        int level, float3 color) {
    auto cb = [](void* buffer, size_t size, void* user) { free(buffer); };
    const int width = baseWidth >> level;
    const int height = baseHeight >> level;
    const size_t size0 = height * width * 4;
    uint8_t* buffer0 = (uint8_t*) calloc(size0, 1);
    PixelBufferDescriptor pb(buffer0, size0, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb);

    const float3 foreground = color;
    const float3 background = float3(1, 1, 0);
    const float radius = 0.25f;

    // Draw a circle on a yellow background.
    uint32_t* texels = (uint32_t*) buffer0;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            float2 uv = { (col - width / 2.0f) / width, (row - height / 2.0f) / height };
            const float d = distance(uv, float2(0));
            const float t = d < radius ? 1.0 : 0.0;
            const float3 color = mix(foreground, background, t);
            texels[row * width + col] = toUintColor(float4(color, 1.0f));
        }
    }

    // Upload to the GPU.
    dapi.update2DImage(texture, level, 0, 0, width, height, std::move(pb));
}

TEST_F(BackendTest, ColorMagnify) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 256;
    constexpr int kSrcTexHeight = 256;
    constexpr auto kSrcTexFormat = filament::backend::TextureFormat::RGBA8;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0));
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5));

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

    // Do a "magnify" blit from level 1 of the source RT to the level 0 of the desination RT.
    const int srcLevel = 1;
    api.blit(TargetBufferFlags::COLOR0, dstRenderTargets[0],
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTargets[srcLevel],
            {0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel}, SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to exectue.
    getDriverApi().beginFrame(0, 0);
    getDriverApi().commit(swapChain);
    getDriverApi().endFrame(0);

    // Grab a screenshot.
    getDriverApi().beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTargets[0], "ColorMagnify.png");
    getDriverApi().commit(swapChain);
    getDriverApi().endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0xb830a36a;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", sPixelHashResult, expected);
    EXPECT_TRUE(sPixelHashResult == expected);

    // Cleanup.
    api.destroyTexture(srcTexture);
    api.destroyTexture(dstTexture);
    api.destroySwapChain(swapChain);
    for (auto rt : srcRenderTargets)  api.destroyRenderTarget(rt);
    for (auto rt : dstRenderTargets)  api.destroyRenderTarget(rt);
}

TEST_F(BackendTest, ColorMinify) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 1024;
    constexpr int kSrcTexHeight = 1024;
    constexpr auto kSrcTexFormat = filament::backend::TextureFormat::RGBA8;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = api.createSwapChainHeadless(256, 256, 0);
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = api.createTexture(
        SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
        TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0));
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5));

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

    // Do a "magnify" blit from level 1 of the source RT to the level 0 of the desination RT.
    const int srcLevel = 1;
    api.blit(TargetBufferFlags::COLOR0, dstRenderTargets[0],
            {0, 0, kDstTexWidth, kDstTexHeight}, srcRenderTargets[srcLevel],
            {0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel}, SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to exectue.
    getDriverApi().beginFrame(0, 0);
    getDriverApi().commit(swapChain);
    getDriverApi().endFrame(0);

    // Grab a screenshot.
    getDriverApi().beginFrame(0, 0);
    dumpScreenshot(api, dstRenderTargets[0], "ColorMinify.png");
    getDriverApi().commit(swapChain);
    getDriverApi().endFrame(0);

    // Wait for the ReadPixels result to come back.
    api.finish();
    executeCommands();
    getDriver().purge();

    // Check if the image matches perfectly to our golden run.
    const uint32_t expected = 0xe2353ca6;
    printf("Computed hash is 0x%8.8x, Expected 0x%8.8x\n", sPixelHashResult, expected);
    EXPECT_TRUE(sPixelHashResult == expected);

    // Cleanup.
    api.destroyTexture(srcTexture);
    api.destroyTexture(dstTexture);
    api.destroySwapChain(swapChain);
    for (auto rt : srcRenderTargets)  api.destroyRenderTarget(rt);
    for (auto rt : dstRenderTargets)  api.destroyRenderTarget(rt);
}

} // namespace test
