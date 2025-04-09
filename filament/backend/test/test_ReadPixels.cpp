/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "BackendTestUtils.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "TrianglePrimitive.h"

#include <utils/Hash.h>

#include <fstream>
#include <algorithm>

using namespace filament;
using namespace filament::backend;

#ifndef FILAMENT_IOS

#include <imageio/ImageEncoder.h>
#include <image/ColorTransform.h>

using namespace image;
#endif

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string fragmentFloat(R"(#version 450 core

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(1.0);
}

)");

std::string fragmentUint(R"(#version 450 core

layout(location = 0) out uvec4 fragColor;

void main() {
    fragColor = uvec4(1);
}

)");

}

namespace test {

class ReadPixelsTest : public BackendTest {
public:
    bool readPixelsFinished = false;
};

TEST_F(ReadPixelsTest, ReadPixels) {
    // These test scenarios use a known hash of the result pixel buffer to decide pass / fail,
    // asserting an exact pixel-for-pixel match. So far, rendering on macOS and iPhone have had
    // deterministic results. Take this test with a grain of salt, however, as other platform / GPU
    // combinations may vary ever-so-slightly, which would cause this test to fail.

    const size_t renderTargetBaseSize = 512;

    struct TestCase {
        const char* testName = "readPixels_normal";

        // The murmur3 hash of the read pixel buffer result, used to determine success.
        uint32_t hash = 0x899fb4a9;

        // The number of mip levels in the texture we're rendering into.
        size_t mipLevels = 1;

        // The mip level to render into. Must be < mipLevels.
        size_t mipLevel = 0;

        // The number of samples for MSAA rendering.
        size_t samples = 1;

        // The size of the actual render target, taking mip level into account;
        size_t getRenderTargetSize() const {
            return std::max(size_t(1), renderTargetBaseSize >> mipLevel);
        }

        // The rect that read pixels will read from.
        struct Rect {
            size_t x, y, width, height;
        } readRect = { 0, 0, renderTargetBaseSize, renderTargetBaseSize };

        // The size of the pixel buffer read pixels will write into.
        size_t bufferDimension = getRenderTargetSize();

        size_t getBufferSizeBytes() const {
            size_t components;
            int bpp;
            getPixelInfo(format, type, components, bpp);
            return bufferDimension * bufferDimension * bpp;
        }

        // The offset and stride set on the pixel buffer.
        size_t left = 0, top = 0, alignment = 1;

        // If true, use the default RT to render into the SwapChain as opposed to a texture.
        bool useDefaultRT = false;

        size_t getPixelBufferStride() const {
            return bufferDimension;
        }

        void exportScreenshot(void* pixelData) const {
#ifndef FILAMENT_IOS
            const size_t width = readRect.width, height = readRect.height;
            LinearImage image(width, height, 4);
            if (format == PixelDataFormat::RGBA && type == PixelDataType::UBYTE) {
                image = toLinearWithAlpha<uint8_t>(width, height, width * 4, (uint8_t*)pixelData);
            }
            if (format == PixelDataFormat::RGBA && type == PixelDataType::FLOAT) {
                memcpy(image.getPixelRef(), pixelData, width * height * sizeof(math::float4));
            }
            std::string png = std::string(testName) + ".png";
            std::ofstream outputStream(png.c_str(), std::ios::binary | std::ios::trunc);
            ImageEncoder::encode(outputStream, ImageEncoder::Format::PNG, image, "",
                    png.c_str());
#endif
        }

        void exportRawBytes(void* pixelData) const {
            std::string out = std::string(testName) + ".raw";
            std::ofstream outputStream(out.c_str(), std::ios::binary | std::ios::trunc);
            outputStream.write((char*)pixelData, getBufferSizeBytes());
            outputStream.close();
        }

        // The format and type for the readPixels call.
        PixelDataFormat format = PixelDataFormat::RGBA;
        PixelDataType type = PixelDataType::UBYTE;

        // The texture format of the render target.
        TextureFormat textureFormat = TextureFormat::RGBA8;
    };

    // The normative read pixels test case. Render a white triangle over a blue background and read
    // the full viewport into a pixel buffer.
    TestCase const t0;

    // Check that a subregion of the render target can be read into a pixel buffer.
    TestCase t2;
    t2.testName = "readPixels_subregion";
    t2.readRect.x = 90;
    t2.readRect.y = 403;
    t2.readRect.width = 64;
    t2.readRect.height = 64;
    t2.bufferDimension = 64;
    t2.hash = 0xcba7675a;

    // Check that readPixels works when rendering into and reading from a mip level.
    TestCase t3;
    t3.testName = "readPixels_mip";
    t3.mipLevels = 4;
    t3.mipLevel = 2;
    t3.bufferDimension = t3.getRenderTargetSize();
    t3.readRect.width = t3.getRenderTargetSize();
    t3.readRect.height = t3.getRenderTargetSize();
    t3.hash = 0xe6fa6c55;

    // Check that readPixels can return pixels in floating point RGBA format.
    TestCase t4;
    t4.testName = "readPixels_float";
    t4.format = PixelDataFormat::RGBA;
    t4.type = PixelDataType::FLOAT;
    t4.hash = 0xd8f5a7df;

    // Check that readPixels can read a region of the render target into a subregion of a large
    // buffer.
    TestCase t5;
    t5.testName = "readPixels_subbuffer";
    t5.readRect.x = 90;
    t5.readRect.y = 403;
    t5.readRect.width = 64;
    t5.readRect.height = 64;
    t5.bufferDimension = 512;
    t5.left = 64;
    t5.top = 64;
    t5.hash = 0xbaefdb54;

    // Check that readPixels works with integer formats.
    TestCase t6;
    t6.testName = "readPixels_UINT";
    t6.format = PixelDataFormat::R_INTEGER;
    t6.type = PixelDataType::UINT;
    t6.textureFormat = TextureFormat::R32UI;
    t6.hash = 0x9d91227;

    // Check that readPixels works with half formats.
    TestCase t7;
    t7.testName = "readPixels_half";
    t7.format = PixelDataFormat::RG;
    t7.type = PixelDataType::HALF;
    t7.textureFormat = TextureFormat::RG16F;
    t7.hash = 3726805703;

    // Check that readPixels works when rendering into the SwapChain.
    // This requires that the test runner's native window size is 512x512.
    TestCase t8;
    t8.testName = "readPixels_swapchain";
    t8.useDefaultRT = true;

    TestCase const testCases[] = { t0, t2, t3, t4, t5, t6, t7, t8 };

    DriverApi& api = getDriverApi();
    Cleanup cleanup(api);

    std::string vertexShader = SharedShaders::getVertexShaderText(VertexShaderType::Noop,
            ShaderUniformType::None);
    Shader floatShader(api, cleanup, ShaderConfig{
            .vertexShader = vertexShader,
            .fragmentShader = fragmentFloat,
            .uniforms = {}
    });
    Shader uintShader(api, cleanup, ShaderConfig{
            .vertexShader = vertexShader,
            .fragmentShader = fragmentUint,
            .uniforms = {}
    });

    for (const auto& t: testCases) {
        // Create a platform-specific SwapChain and make it current.
        Handle<HwSwapChain> swapChain;
        if (t.useDefaultRT) {
            swapChain = cleanup.add(createSwapChain());
        } else {
            swapChain = cleanup.add(api.createSwapChainHeadless(t.getRenderTargetSize(),
                    t.getRenderTargetSize(), 0));
        }

        api.makeCurrent(swapChain, swapChain);

        // Create a Texture and RenderTarget to render into.
        auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
        Handle<HwTexture> const texture = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D,
                t.mipLevels, t.textureFormat, 1, renderTargetBaseSize, renderTargetBaseSize, 1,
                usage));

        Handle<HwRenderTarget> renderTarget;
        if (t.useDefaultRT) {
            // The width and height must match the width and height of the respective mip
            // level (at least for OpenGL).
            renderTarget = cleanup.add(api.createDefaultRenderTarget());
        } else {
            // The width and height must match the width and height of the respective mip
            // level (at least for OpenGL).
            renderTarget = cleanup.add(api.createRenderTarget(
                    TargetBufferFlags::COLOR, t.getRenderTargetSize(),
                    t.getRenderTargetSize(), t.samples, 0, {{ texture, uint8_t(t.mipLevel) }}, {},
                    {}));
        }

        TrianglePrimitive const triangle(api);

        RenderPassParams params = {};
        fullViewport(params);
        params.flags.clear = TargetBufferFlags::COLOR;
        params.clearColor = { 0.f, 0.f, 1.f, 1.f };
        params.flags.discardStart = TargetBufferFlags::ALL;
        params.flags.discardEnd = TargetBufferFlags::NONE;
        params.viewport.height = t.getRenderTargetSize();
        params.viewport.width = t.getRenderTargetSize();

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, 0);

        // Render a white triangle over blue.
        api.beginRenderPass(renderTarget, params);

        PipelineState state;
        state.program = floatShader.getProgram();
        if (isUnsignedIntFormat(t.textureFormat)) {
            state.program = uintShader.getProgram();
        }
        state.rasterState.colorWrite = true;
        state.rasterState.depthWrite = false;
        state.rasterState.depthFunc = RasterState::DepthFunc::A;
        state.rasterState.culling = CullingMode::NONE;
        api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);

        api.endRenderPass();

        if (t.mipLevel > 0) {
            Cleanup localCleanup(api);
            // Render red to the first mip level to check that the backend is actually reading the
            // correct mip.
            RenderPassParams p = params;
            Handle<HwRenderTarget> mipLevelOneRT = localCleanup.add(api.createRenderTarget(
                    TargetBufferFlags::COLOR, renderTargetBaseSize, renderTargetBaseSize, 1, 0,
                    {{ texture }}, {}, {}));
            p.clearColor = { 1.f, 0.f, 0.f, 1.f };
            api.beginRenderPass(mipLevelOneRT, p);
            api.endRenderPass();
        }

        // Read pixels.
        void* buffer = calloc(1, t.getBufferSizeBytes());

        PixelBufferDescriptor descriptor(buffer, t.getBufferSizeBytes(), t.format, t.type,
                t.alignment, t.left, t.top, t.getPixelBufferStride(), [](void* buffer, size_t size,
                        void* user) {
                    const auto* test = (const TestCase*)user;
                    assert_invariant(test);

                    test->exportScreenshot(buffer);
                    //test->exportRawBytes(buffer);

                    // Hash the contents of the buffer and check that they match.
                    uint32_t hash = utils::hash::murmur3((const uint32_t*)buffer, size / 4, 0);

                    ASSERT_EQ(test->hash, hash) << test->testName <<
                                                " failed: hashes do not match." << std::endl;

                    free(buffer);
                }, (void*)&t);

        api.readPixels(renderTarget, t.readRect.x, t.readRect.y, t.readRect.width,
                t.readRect.height, std::move(descriptor));

        // Now render red over what was just rendered. This ensures that readPixels captures the
        // state of rendering between render passes.
        params.clearColor = { 1.f, 0.f, 0.f, 1.f };
        api.beginRenderPass(renderTarget, params);
        api.endRenderPass();

        api.commit(swapChain);
        api.endFrame(0);
    }

    // This ensures all driver commands have finished before exiting the test.
    flushAndWait();
}

TEST_F(ReadPixelsTest, ReadPixelsPerformance) {
    const size_t renderTargetSize = 2000;
    const int iterationCount = 100;

    DriverApi& api = getDriverApi();
    Cleanup cleanup(api);

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = cleanup.add(
            api.createSwapChainHeadless(renderTargetSize, renderTargetSize, 0));
    api.makeCurrent(swapChain, swapChain);

    Shader shader = SharedShaders::makeShader(api, cleanup, ShaderRequest{
            .mVertexType = VertexShaderType::Noop,
            .mFragmentType = FragmentShaderType::White,
            .mUniformType = ShaderUniformType::None
    });

    // Create a Texture and RenderTarget to render into.
    auto usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
    Handle<HwTexture> texture = cleanup.add(api.createTexture(
            SamplerType::SAMPLER_2D,            // target
            1,                                  // levels
            TextureFormat::RGBA8,               // format
            1,                                  // samples
            renderTargetSize,                   // width
            renderTargetSize,                   // height
            1,                                  // depth
            usage));                             // usage

    Handle<HwRenderTarget> renderTarget = cleanup.add(api.createRenderTarget(
            TargetBufferFlags::COLOR,
            renderTargetSize,                          // width
            renderTargetSize,                          // height
            1,                                         // samples
            0,                                         // layerCount
            {{ texture }},                             // color
            {},                                        // depth
            {}));                                       // stencil

    TrianglePrimitive triangle(api);

    RenderPassParams params = {};
    fullViewport(params);
    params.flags.clear = TargetBufferFlags::COLOR;
    params.clearColor = { 0.f, 0.f, 1.f, 1.f };
    params.flags.discardStart = TargetBufferFlags::ALL;
    params.flags.discardEnd = TargetBufferFlags::NONE;
    params.viewport.height = renderTargetSize;
    params.viewport.width = renderTargetSize;

    void* buffer = calloc(1, renderTargetSize * renderTargetSize * 4);

    PipelineState state;
    state.program = shader.getProgram();
    state.rasterState.colorWrite = true;
    state.rasterState.depthWrite = false;
    state.rasterState.depthFunc = RasterState::DepthFunc::A;
    state.rasterState.culling = CullingMode::NONE;

    for (int iteration = 0; iteration < iterationCount; ++iteration) {
        readPixelsFinished = false;

        if (0 == iteration % 10) {
            printf("Executing test %d / %d\n", iteration, iterationCount);
        }

        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, 0);

        // Render some content, just so we don't read back uninitialized data.
        api.beginRenderPass(renderTarget, params);
        api.draw(state, triangle.getRenderPrimitive(), 0, 3, 1);
        api.endRenderPass();

        PixelBufferDescriptor descriptor(buffer, renderTargetSize * renderTargetSize * 4,
                PixelDataFormat::RGBA, PixelDataType::UBYTE, 1, 0, 0, renderTargetSize,
                [](void* buffer, size_t size, void* user) {
                    ReadPixelsTest* test = (ReadPixelsTest*)user;
                    test->readPixelsFinished = true;
                }, this);

        api.readPixels(renderTarget, 0, 0, renderTargetSize, renderTargetSize,
                std::move(descriptor));
        api.commit(swapChain);
        api.endFrame(0);

        flushAndWait();
        getDriver().purge();

        ASSERT_TRUE(readPixelsFinished);
    }

    free(buffer);

    api.finish();
    executeCommands();
}

} // namespace test
