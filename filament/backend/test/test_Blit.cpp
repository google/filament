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

#include "ImageExpectations.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "Skip.h"
#include "TrianglePrimitive.h"

#include <utils/Hash.h>
#include <utils/Log.h>

#include <fstream>

#ifndef FILAMENT_IOS

#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>

#endif

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;
using namespace utils;

class BlitTest : public BackendTest {
public:
    BlitTest() : mCleanup(getDriverApi()) {}

protected:
    Cleanup mCleanup;
};

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
    uint8_t* buffer0 = (uint8_t*)calloc(size0, 1);
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
    uint32_t* texels = (uint32_t*)buffer0;
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
    uint8_t* buffer0 = (uint8_t*)calloc(size0, 1);
    PixelBufferDescriptor pb(buffer0, size0, PixelDataFormat::RGBA, PixelDataType::UBYTE, cb);

    const float3 foreground = color;
    const float3 background = float3(1, 1, 0);
    const float radius = 0.25f;

    // Draw a circle on a yellow background.
    uint32_t* texels = (uint32_t*)buffer0;
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

TEST_F(BlitTest, ColorMagnify) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 256;
    constexpr int kSrcTexHeight = 256;
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    constexpr int kDstTexWidth = 384;
    constexpr int kDstTexHeight = 384;
    constexpr auto kDstTexFormat = TextureFormat::RGBA8;
    constexpr int kNumLevels = 3;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = mCleanup.add(api.createSwapChainHeadless(256, 256, 0));
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> const srcTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT));
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a destination texture.
    Handle<HwTexture> const dstTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT));

    // Create a RenderTarget for each texture's miplevel.
    Handle<HwRenderTarget> srcRenderTargets[kNumLevels];
    Handle<HwRenderTarget> dstRenderTargets[kNumLevels];
    for (uint8_t level = 0; level < kNumLevels; level++) {
        srcRenderTargets[level] = mCleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR,
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, 0, { srcTexture, level, 0 }, {},
                {}));
        dstRenderTargets[level] = mCleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR,
                kDstTexWidth >> level, kDstTexHeight >> level, 1, 0, { dstTexture, level, 0 }, {},
                {}));
    }

    // Do a "magnify" blit from level 1 of the source RT to the level 0 of the destination RT.
    const int srcLevel = 1;
    api.blitDEPRECATED(TargetBufferFlags::COLOR0, dstRenderTargets[0],
            { 0, 0, kDstTexWidth, kDstTexHeight }, srcRenderTargets[srcLevel],
            { 0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel },
            SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    {
        RenderFrame frame(api);
        api.commit(swapChain);
    }

    {
        RenderFrame frame(api);
        EXPECT_IMAGE(dstRenderTargets[0],
                ScreenshotParams(kDstTexWidth, kDstTexHeight, "ColorMagnify", 0x410bdd31));
        api.commit(swapChain);
    }
}

TEST_F(BlitTest, ColorMinify) {
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 1024;
    constexpr int kSrcTexHeight = 1024;
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    constexpr int kDstTexWidth = 384;
    constexpr int kDstTexHeight = 384;
    constexpr auto kDstTexFormat = TextureFormat::RGBA8;
    constexpr int kNumLevels = 3;

    // Create a SwapChain and make it current. We don't really use it so the res doesn't matter.
    auto swapChain = mCleanup.add(api.createSwapChainHeadless(256, 256, 0));
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> const srcTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT));
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a destination texture.
    Handle<HwTexture> const dstTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT));

    // Create a RenderTarget for each texture's miplevel.
    Handle<HwRenderTarget> srcRenderTargets[kNumLevels];
    Handle<HwRenderTarget> dstRenderTargets[kNumLevels];
    for (uint8_t level = 0; level < kNumLevels; level++) {
        srcRenderTargets[level] = mCleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR,
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, 0, { srcTexture, level, 0 }, {},
                {}));
        dstRenderTargets[level] = mCleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR,
                kDstTexWidth >> level, kDstTexHeight >> level, 1, 0, { dstTexture, level, 0 }, {},
                {}));
    }

    // Do a "minify" blit from level 1 of the source RT to the level 0 of the destination RT.
    const int srcLevel = 1;

    api.blitDEPRECATED(TargetBufferFlags::COLOR0,
            dstRenderTargets[0], { 0, 0, kDstTexWidth, kDstTexHeight },
            srcRenderTargets[srcLevel],
            { 0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel },
            SamplerMagFilter::LINEAR);

    EXPECT_IMAGE(dstRenderTargets[0],
            ScreenshotParams(kDstTexWidth, kDstTexHeight, "ColorMinify", 0xf3d9c53f));
}

TEST_F(BlitTest, ColorResolve) {
    NONFATAL_FAIL_IF(SkipEnvironment(OperatingSystem::APPLE, Backend::VULKAN),
            "Nothing is drawn, see b/417229577");
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 256;
    constexpr int kSrcTexHeight = 256;
    constexpr int kDstTexWidth = 256;
    constexpr int kDstTexHeight = 256;
    constexpr auto kColorTexFormat = TextureFormat::RGBA8;
    constexpr int kSampleCount = 4;

    Shader shader = SharedShaders::makeShader(api, mCleanup, ShaderRequest{
            .mVertexType = VertexShaderType::Simple,
            .mFragmentType = FragmentShaderType::SolidColored,
            .mUniformType = ShaderUniformType::Simple,
    });

    // Create a VertexBuffer, IndexBuffer, and RenderPrimitive.
    TrianglePrimitive const triangle(api);

    // Create 4-sample texture.
    Handle<HwTexture> const srcColorTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, 1, kColorTexFormat, kSampleCount, kSrcTexWidth, kSrcTexHeight,
            1,
            TextureUsage::COLOR_ATTACHMENT));

    // Create 1-sample texture.
    Handle<HwTexture> const dstColorTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, 1, kColorTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT));

    // Create a 4-sample render target with the 4-sample texture.
    Handle<HwRenderTarget> const srcRenderTarget = mCleanup.add(api.createRenderTarget(
            TargetBufferFlags::COLOR, kSrcTexWidth, kSrcTexHeight, kSampleCount, 0,
            {{ srcColorTexture }}, {}, {}));

    // Create a 1-sample render target with the 1-sample texture.
    Handle<HwRenderTarget> const dstRenderTarget = mCleanup.add(api.createRenderTarget(
            TargetBufferFlags::COLOR, kDstTexWidth, kDstTexHeight, 1, 0,
            {{ dstColorTexture }}, {}, {}));

    // Prep for rendering.
    PipelineState state = getColorWritePipelineState();
    shader.addProgramToPipelineState(state);

    RenderPassParams params = getClearColorRenderPass();
    params.viewport.width = kSrcTexWidth;
    params.viewport.height = kSrcTexHeight;

    auto ubuffer = mCleanup.add(api.createBufferObject(sizeof(SimpleMaterialParams),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    // Draw red triangle into srcRenderTarget.
    shader.uploadUniform(api, ubuffer, SimpleMaterialParams{
        .color = float4(1, 0, 0, 1),
        .scaleMinusOne = float4(0, 0, -0.5, 0),
        .offset = float4(0.5, 0.5, 0, 0),
    });
    shader.bindUniform<SimpleMaterialParams>(api, ubuffer);

    {
        RenderFrame frame(api);
        api.beginRenderPass(srcRenderTarget, params);
        state.primitiveType = PrimitiveType::TRIANGLES;
        state.vertexBufferInfo = triangle.getVertexBufferInfo();
        api.bindPipeline(state);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();
    }

    // Resolve the MSAA render target into the single-sample render target.
    api.blitDEPRECATED(TargetBufferFlags::COLOR,
            dstRenderTarget, { 0, 0, kDstTexWidth, kDstTexHeight },
            srcRenderTarget, { 0, 0, kSrcTexWidth, kSrcTexHeight },
            SamplerMagFilter::NEAREST);

    EXPECT_IMAGE(dstRenderTarget,
            ScreenshotParams(kDstTexWidth, kDstTexHeight, "ColorResolve", 531759687));
}

TEST_F(BlitTest, Blit2DTextureArray) {
    auto& api = getDriverApi();

    api.startCapture(0);
    mCleanup.addPostCall([&]() { api.stopCapture(0); });

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
    auto swapChain = mCleanup.add(api.createSwapChainHeadless(256, 256, 0));
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D_ARRAY, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth,
            kSrcTexHeight, kSrcTexDepth,
            TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT));
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, kSrcTexLayer, float3(0.5, 0, 0),
            flipY);

    // Create a destination texture.
    Handle<HwTexture> dstTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT));

    // Create two RenderTargets.
    const int level = 0;
    Handle<HwRenderTarget> srcRenderTarget = mCleanup.add(
            api.createRenderTarget(TargetBufferFlags::COLOR,
                    kSrcTexWidth >> level, kSrcTexHeight >> level, 1, 0,
                    { srcTexture, level, kSrcTexLayer }, {}, {}));
    Handle<HwRenderTarget> dstRenderTarget = mCleanup.add(
            api.createRenderTarget(TargetBufferFlags::COLOR,
                    kDstTexWidth >> level, kDstTexHeight >> level, 1, 0,
                    { dstTexture, level, kDstTexLayer }, {}, {}));

    // Do a blit from kSrcTexLayer of the source RT to kDstTexLayer of the destination RT.
    const int srcLevel = 0;
    api.blitDEPRECATED(TargetBufferFlags::COLOR0, dstRenderTarget,
            { 0, 0, kDstTexWidth, kDstTexHeight }, srcRenderTarget,
            { 0, 0, kSrcTexWidth >> srcLevel, kSrcTexHeight >> srcLevel },
            SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    {
        RenderFrame frame(api);
        api.commit(swapChain);
    }

    {
        RenderFrame frame(api);
        EXPECT_IMAGE(dstRenderTarget,
                ScreenshotParams(kDstTexWidth, kDstTexHeight, "Blit2DTextureArray",
                        0x8de7d55b));
        api.commit(swapChain);
    }
}

TEST_F(BlitTest, BlitRegion) {
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
    auto swapChain = mCleanup.add(api.createSwapChainHeadless(256, 256, 0));
    api.makeCurrent(swapChain, swapChain);

    // Create a source texture.
    Handle<HwTexture> srcTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT));
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a destination texture.
    Handle<HwTexture> dstTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kDstTexFormat, 1, kDstTexWidth, kDstTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT));

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
            mCleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR, srcRect.width,
                    srcRect.height, 1, 0, { srcTexture, kSrcLevel, 0 }, {}, {}));
    Handle<HwRenderTarget> dstRenderTarget =
            mCleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR, kDstTexWidth >> kDstLevel,
                    kDstTexHeight >> kDstLevel, 1, 0, { dstTexture, kDstLevel, 0 }, {}, {}));

    api.blitDEPRECATED(TargetBufferFlags::COLOR0, dstRenderTarget, dstRect, srcRenderTarget,
            srcRect,
            SamplerMagFilter::LINEAR);

    // Push through an empty frame to allow the texture to upload and the blit to execute.
    {
        RenderFrame frame(api);
        api.commit(swapChain);
    }

    {
        RenderFrame frame(api);
        // TODO: for some reason, this test has very, very slight (as in one pixel) differences
        // between OpenGL and Metal. So disable golden checking for now.
        // EXPECT_IMAGE(dstRenderTarget, ScreenshotParams(kDstTexWidth,
        //         kDstTexHeight, "BlitRegion", 0x74fa34ed));
        api.commit(swapChain);
    }
}

TEST_F(BlitTest, BlitRegionToSwapChain) {
    FAIL_IF(Backend::VULKAN, "Crashes due to not finding color attachment, see b/417481493");
    auto& api = getDriverApi();

    constexpr int kSrcTexWidth = 1024;
    constexpr int kSrcTexHeight = 1024;
    constexpr auto kSrcTexFormat = TextureFormat::RGBA8;
    const uint32_t kDstTexWidth = screenWidth();
    const uint32_t kDstTexHeight = screenHeight();
    constexpr int kNumLevels = 3;

    // Create a SwapChain and make it current.
    auto swapChain = mCleanup.add(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    Handle<HwRenderTarget> dstRenderTarget = mCleanup.add(api.createDefaultRenderTarget());

    // Create a source texture.
    Handle<HwTexture> srcTexture = mCleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D, kNumLevels, kSrcTexFormat, 1, kSrcTexWidth, kSrcTexHeight, 1,
            TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE | TextureUsage::COLOR_ATTACHMENT));
    const bool flipY = sBackend == Backend::OPENGL;
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 0, float3(0.5, 0, 0), flipY);
    createBitmap(api, srcTexture, kSrcTexWidth, kSrcTexHeight, 1, float3(0, 0, 0.5), flipY);

    // Create a RenderTarget for each texture's miplevel.
    Handle<HwRenderTarget> srcRenderTargets[kNumLevels];
    for (uint8_t level = 0; level < kNumLevels; level++) {
        srcRenderTargets[level] = mCleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR,
                kSrcTexWidth >> level, kSrcTexHeight >> level, 1, 0, { srcTexture, level, 0 }, {},
                {}));
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

    {
        RenderFrame frame(api);

        api.blitDEPRECATED(TargetBufferFlags::COLOR0, dstRenderTarget,
                dstRect, srcRenderTargets[srcLevel],
                srcRect, SamplerMagFilter::LINEAR);

                api.commit(swapChain);

        // TODO: for some reason, this test has been disabled. It needs to be tested on all
        // machines.
        // EXPECT_IMAGE(dstRenderTarget,
        //         ScreenshotParams(kDstTexWidth, kDstTexHeight, "BlitRegionToSwapChain", 0x0));
    }
}

} // namespace test
