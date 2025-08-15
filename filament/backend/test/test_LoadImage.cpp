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

#include "BackendTestUtils.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "Skip.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include "TrianglePrimitive.h"
#include "private/filament/SamplerInterfaceBlock.h"

#include <vector>

#include <stddef.h>
#include <stdint.h>

using namespace filament;
using namespace filament::backend;

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string fragmentTemplate (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
// This parameter is ignored for other backends.
layout(binding = 0, set = 0) uniform {samplerType} test_tex;

void main() {
    vec2 fbsize = vec2({texSize});
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_WEBGPU_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    fragColor = vec4(texture(test_tex, uv).rgb, 1.0f);
}

)");

std::string fragmentUpdateImage3DTemplate (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
layout(binding = 0, set = 0) uniform {samplerType} test_tex;

float getLayer(in sampler3D s) { return 2.5f / 4.0f; }
float getLayer(in sampler2DArray s) { return 2.0f; }

void main() {
    vec2 fbsize = vec2({texSize});
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_WEBGPU_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    fragColor = vec4(texture(test_tex, vec3(uv, getLayer(test_tex))).rgb, 1.0f);
}

)");

std::string fragmentUpdateImageMip (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
layout(binding = 0, set = 0) uniform sampler2D test_tex;

void main() {
    vec2 fbsize = vec2({texSize});
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_WEBGPU_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    fragColor = vec4(textureLod(test_tex, uv, 1.0f).rgb, 1.0f);
}

)");

}

namespace test {

template<typename componentType> inline componentType getMaxValue();

inline std::string stringReplace(const std::string& find, const std::string& replace,
        std::string source) {
    std::string::size_type pos = source.find(find);
    if (pos != std::string::npos) {
        source.replace(pos, find.length(), replace);
    }
    return source;
}

static const char* getSamplerTypeName(SamplerType samplerType) {
    switch (samplerType) {
        case SamplerType::SAMPLER_EXTERNAL:
        case SamplerType::SAMPLER_2D:
            return "sampler2D";
        case SamplerType::SAMPLER_2D_ARRAY:
            return "sampler2DArray";
        case SamplerType::SAMPLER_CUBEMAP:
            return "samplerCube";
        case SamplerType::SAMPLER_3D:
            return "sampler3D";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return "samplerCubeArray";
    }
}

static const char* getSamplerTypeName(TextureFormat textureFormat) {
    switch (textureFormat) {
        case TextureFormat::R8UI:
        case TextureFormat::R16UI:
        case TextureFormat::RG8UI:
        case TextureFormat::RGB8UI:
        case TextureFormat::R32UI:
        case TextureFormat::RG16UI:
        case TextureFormat::RGBA8UI:
        case TextureFormat::RGB16UI:
        case TextureFormat::RG32UI:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGBA32UI:
            return  "usampler2D";

        case TextureFormat::R8I:
        case TextureFormat::R16I:
        case TextureFormat::RG8I:
        case TextureFormat::RGB8I:
        case TextureFormat::R32I:
        case TextureFormat::RG16I:
        case TextureFormat::RGBA8I:
        case TextureFormat::RGB16I:
        case TextureFormat::RG32I:
        case TextureFormat::RGBA16I:
        case TextureFormat::RGB32I:
        case TextureFormat::RGBA32I:
            return "isampler2D";

        default:
            return "sampler2D";
    }

}
static SamplerFormat getSamplerFormat(TextureFormat textureFormat) {
    switch (textureFormat) {
        case TextureFormat::R8UI:
        case TextureFormat::R16UI:
        case TextureFormat::RG8UI:
        case TextureFormat::RGB8UI:
        case TextureFormat::R32UI:
        case TextureFormat::RG16UI:
        case TextureFormat::RGBA8UI:
        case TextureFormat::RGB16UI:
        case TextureFormat::RG32UI:
        case TextureFormat::RGBA16UI:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGBA32UI:
            return SamplerFormat::UINT;

        case TextureFormat::R8I:
        case TextureFormat::R16I:
        case TextureFormat::RG8I:
        case TextureFormat::RGB8I:
        case TextureFormat::R32I:
        case TextureFormat::RG16I:
        case TextureFormat::RGBA8I:
        case TextureFormat::RGB16I:
        case TextureFormat::RG32I:
        case TextureFormat::RGBA16I:
        case TextureFormat::RGB32I:
        case TextureFormat::RGBA32I:
            return SamplerFormat::INT;

        default:
            return SamplerFormat::FLOAT;
    }
}

class LoadImageTest : public BackendTest {
public:
    LoadImageTest() : mTriangle(getDriverApi()) {
        mVertexShader = SharedShaders::getVertexShaderText(VertexShaderType::Noop,
                ShaderUniformType::None);

        // Checkerboard utils require square textures
        EXPECT_THAT(screenWidth(), testing::Eq(screenWidth()));
    }

    uint32_t kTexSize = screenWidth();
    uint32_t kHalfTexSize = kTexSize / 2;
    uint32_t kDoubleTexSize = kTexSize * 2;
    std::string mVertexShader;
    TrianglePrimitive mTriangle;

    std::string getFormattedFragment(const std::string& fragment, TextureFormat textureFormat) {
        std::string withSampler =
                stringReplace("{samplerType}", getSamplerTypeName(textureFormat), fragment);
        return stringReplace("{texSize}", std::to_string(kTexSize), withSampler);
    }

    std::string getFormattedFragment(const std::string& fragment, SamplerType samplerType) {
        std::string withSampler =
                stringReplace("{samplerType}", getSamplerTypeName(samplerType), fragment);
        return stringReplace("{texSize}", std::to_string(kTexSize), withSampler);
    }
};

TEST_F(LoadImageTest, UpdateImage2D) {
    FAIL_IF(Backend::VULKAN, "Multiple test cases crash, see b/417481434");

    // All of these test cases should result in the same rendered image, and thus the same hash.
    static const uint32_t expectedHash = 1875922935;

    struct TestCase {
        const char* name;
        PixelDataFormat pixelFormat;
        TextureFormat textureFormat;

        bool compressed;

        // Add a border of padding to the pixel buffer.
        size_t bufferPadding;

        // Upload subregions of the texture separately.
        bool uploadSubregions;

        union {
            CompressedPixelDataType compressedPixelType;
            PixelDataType pixelType;
        };

        TestCase(const char* name, PixelDataFormat pformat, PixelDataType ptype,
                TextureFormat tformat, size_t bpadding = 0, bool subregions = false) : name(name),
                pixelFormat(pformat), pixelType(ptype), textureFormat(tformat), compressed(false),
                bufferPadding(bpadding), uploadSubregions(subregions) { }

        TestCase(const char* name, PixelDataFormat pformat, CompressedPixelDataType ctype,
                TextureFormat tformat) : name(name), pixelFormat(pformat), compressedPixelType(ctype),
                textureFormat(tformat), compressed(true), bufferPadding(0), uploadSubregions(false) { }
    };

    std::vector<TestCase> testCases;

    // Test basic upload.
    testCases.emplace_back("RGBA UBYTE to RGBA8", PixelDataFormat::RGBA, PixelDataType::UBYTE, TextureFormat::RGBA8);

    // Test format conversion.
    // TODO: Vulkan crashes with `Texture at colorAttachment[0] has usage (0x01) which doesn't specify MTLTextureUsageRenderTarget (0x04)'
    // TODO: WebGPU Crashes with "destination texture usage doesn't have wgpu::TextureUsage::RenderAttachment"
    testCases.emplace_back("RGBA FLOAT to RGBA16F", PixelDataFormat::RGBA, PixelDataType::FLOAT, TextureFormat::RGBA16F);

    // Test texture formats not all backends support natively.
    // TODO: Vulkan crashes with "VK_FORMAT_R32G32B32_SFLOAT is not supported"
    // TODO: WebGPU Crashes with "destination texture usage doesn't have wgpu::TextureUsage::RenderAttachment"
    testCases.emplace_back("RGB FLOAT to RGB32F", PixelDataFormat::RGB, PixelDataType::FLOAT, TextureFormat::RGB32F);
    testCases.emplace_back("RGB FLOAT to RGB16F", PixelDataFormat::RGB, PixelDataType::FLOAT, TextureFormat::RGB16F);

    // Test packed format uploads.
    // TODO: Vulkan crashes with "Texture at colorAttachment[0] has usage (0x01) which doesn't specify MTLTextureUsageRenderTarget (0x04)"
    // TODO: WebGPU Crashes with "destination texture usage doesn't have wgpu::TextureUsage::RenderAttachment"
    testCases.emplace_back("RGBA UINT_2_10_10_10_REV to RGB10_A2", PixelDataFormat::RGBA, PixelDataType::UINT_2_10_10_10_REV, TextureFormat::RGB10_A2);
    testCases.emplace_back("RGB UINT_10F_11F_11F_REV to R11F_G11F_B10F", PixelDataFormat::RGB, PixelDataType::UINT_10F_11F_11F_REV, TextureFormat::R11F_G11F_B10F);
    testCases.emplace_back("RGB HALF to R11F_G11F_B10F", PixelDataFormat::RGB, PixelDataType::HALF, TextureFormat::R11F_G11F_B10F);

    // Test integer format uploads.
    // TODO: These cases fail on OpenGL and Vulkan.
    // TODO: These cases now also fail on Metal, but at some point previously worked.
    // TODO: These cases fail for WebGPU. However, leaving them causes
    //       Tint Reader Error: error: sampled image must have float component type
    //       Beginning SpirV-output dump with ret 0
//     testCases.emplace_back("RGB_INTEGER UBYTE to RGB8UI", PixelDataFormat::RGB_INTEGER, PixelDataType::UBYTE, TextureFormat::RGB8UI);
//     testCases.emplace_back("RGB_INTEGER USHORT to RGB16UI", PixelDataFormat::RGB_INTEGER, PixelDataType::USHORT, TextureFormat::RGB16UI);
//     testCases.emplace_back("RGB_INTEGER INT to RGB32I", PixelDataFormat::RGB_INTEGER, PixelDataType::INT, TextureFormat::RGB32I);

    // Test uploads with buffer padding.
    // TODO: Vulkan crashes with "Assertion failed: (offset + size <= allocationSize)"
    testCases.emplace_back("RGBA UBYTE to RGBA8 (with buffer padding)", PixelDataFormat::RGBA, PixelDataType::UBYTE, TextureFormat::RGBA8, 64u);
    testCases.emplace_back("RGBA FLOAT to RGBA16F (with buffer padding)", PixelDataFormat::RGBA, PixelDataType::FLOAT, TextureFormat::RGBA16F, 64u);
    testCases.emplace_back("RGB FLOAT to RGB32F (with buffer padding)", PixelDataFormat::RGB, PixelDataType::FLOAT, TextureFormat::RGB32F, 64u);

    // Upload subregions separately.
    // TODO: Vulkan crashes with "Offsets not yet supported"
    testCases.emplace_back("RGBA UBYTE to RGBA8 (subregions)", PixelDataFormat::RGBA, PixelDataType::UBYTE, TextureFormat::RGBA8, 0u, true);
    testCases.emplace_back("RGBA FLOAT to RGBA16F (subregions)", PixelDataFormat::RGBA, PixelDataType::FLOAT, TextureFormat::RGBA16F, 0u, true);
    testCases.emplace_back("RGBA UBYTE to RGBA8 (subregions and buffer padding)", PixelDataFormat::RGBA, PixelDataType::UBYTE, TextureFormat::RGBA8, 64u, true);
    testCases.emplace_back("RGB FLOAT to RGB32F (subregions and buffer padding)", PixelDataFormat::RGB, PixelDataType::FLOAT, TextureFormat::RGB32F, 64u, true);

    auto& api = getDriverApi();

    api.startCapture();

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    for (const auto& t : testCases) {
        Cleanup cleanup(api);

        // Create a platform-specific SwapChain and make it current.
        auto swapChain = cleanup.add(createSwapChain());
        api.makeCurrent(swapChain, swapChain);
        auto defaultRenderTarget = cleanup.add(api.createDefaultRenderTarget(0));

        // Create a program.
        filament::SamplerInterfaceBlock::SamplerInfo samplerInfo { "test", "tex", 0,
            SamplerType::SAMPLER_2D, getSamplerFormat(t.textureFormat), Precision::HIGH, false };

        std::string const fragment = getFormattedFragment(fragmentTemplate, t.textureFormat);
        Shader shader(api, cleanup, ShaderConfig{
           .vertexShader = mVertexShader,
           .fragmentShader= fragment,
           .uniforms = {{"test_tex", DescriptorType::SAMPLER_2D_FLOAT, samplerInfo}}
        });

        // Create a Texture.
        auto usage = TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE;
        Handle<HwTexture> const texture = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1,
                t.textureFormat, 1, kTexSize, kTexSize, 1u, usage));

        // Upload some pixel data.
        if (t.uploadSubregions) {
            api.update3DImage(texture, 0, 0, 0, 0, kHalfTexSize, kHalfTexSize, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, kHalfTexSize,
                            t.bufferPadding));
            api.update3DImage(texture, 0, kHalfTexSize, 0, 0, kHalfTexSize, kHalfTexSize, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, kHalfTexSize,
                            t.bufferPadding));
            api.update3DImage(texture, 0, 0, kHalfTexSize, 0, kHalfTexSize, kHalfTexSize, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, kHalfTexSize,
                            t.bufferPadding));
            api.update3DImage(texture, 0, kHalfTexSize, kHalfTexSize, 0, kHalfTexSize, kHalfTexSize,
                    1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, kHalfTexSize,
                            t.bufferPadding));
        } else {
            api.update3DImage(texture, 0, 0, 0, 0, kTexSize, kTexSize, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, kTexSize, t.bufferPadding));
        }

        DescriptorSetHandle  descriptorSet = shader.createDescriptorSet(api);
        api.updateDescriptorSetTexture(descriptorSet, 0, texture, {
                .filterMag = SamplerMagFilter::NEAREST,
                .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });

        api.bindDescriptorSet(descriptorSet, 0, {});

        RenderPassParams params = getClearColorRenderPass();
        params.viewport.width = kTexSize;
        params.viewport.height = kTexSize;
        PipelineState state = getColorWritePipelineState();
        shader.addProgramToPipelineState(state);
        state.primitiveType = PrimitiveType::TRIANGLES;
        state.vertexBufferInfo = mTriangle.getVertexBufferInfo();
        api.beginRenderPass(defaultRenderTarget, params);
        api.bindPipeline(state);
        api.bindRenderPrimitive(mTriangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();

        EXPECT_IMAGE(defaultRenderTarget,
                ScreenshotParams(kTexSize, kTexSize, t.name, expectedHash));

        api.commit(swapChain);
        api.endFrame(0);
    }

    api.stopCapture();
}

TEST_F(LoadImageTest, UpdateImageSRGB) {
    FAIL_IF(SkipEnvironment(OperatingSystem::APPLE, Backend::VULKAN),
            "Crashing when reading pixels without a redundant call to makeCurrent right before the"
            "render pass. b/422798473");

    auto& api = getDriverApi();
    Cleanup cleanup(api);
    api.startCapture();

    PixelDataFormat const pixelFormat = PixelDataFormat::RGBA;
    PixelDataType const pixelType = PixelDataType::UBYTE;
    TextureFormat const textureFormat = TextureFormat::SRGB8_A8;

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = cleanup.add(createSwapChain());
    api.makeCurrent(swapChain, swapChain);
    auto defaultRenderTarget = cleanup.add(api.createDefaultRenderTarget(0));

    // Create a program.
    filament::SamplerInterfaceBlock::SamplerInfo samplerInfo { "test", "tex", 0,
        SamplerType::SAMPLER_2D, getSamplerFormat(textureFormat), Precision::HIGH, false };
    std::string const fragment = getFormattedFragment(fragmentTemplate, textureFormat);
    Shader shader(api, cleanup, ShaderConfig{
        .vertexShader = mVertexShader, .fragmentShader = fragment, .uniforms = {{
            "test_tex", DescriptorType::SAMPLER_2D_FLOAT, samplerInfo
    }}});

    // Create a texture.
    Handle<HwTexture> const texture =
            cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1, textureFormat, 1, kTexSize,
                    kTexSize, 1, TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE));

    // Create image data.
    size_t components; int bpp;
    getPixelInfo(pixelFormat, pixelType, components, bpp);
    size_t bpl = kTexSize * kTexSize * bpp;
    size_t bufferSize = bpl;
    void* buffer = calloc(1, bufferSize);
    PixelBufferDescriptor descriptor(buffer, bufferSize, pixelFormat, pixelType,
            1, 0, 0, kTexSize, [](void* buffer, size_t size, void* user) {
                free(buffer);
            }, nullptr);

    // Add a gradient.
    uint8_t* pixel = (uint8_t*) buffer;
    for (int r = 0; r < kTexSize; r++) {
        for (int c = 0; c < kTexSize; c++) {
            for (int n = 0; n < components; n++) {
                *pixel++ = (c / static_cast<float>(kTexSize)) * 255;
            }
        }
    }

    api.update3DImage(texture, 0, 0, 0, 0, kTexSize, kTexSize, 1, std::move(descriptor));

    api.beginFrame(0, 0, 0);

    // Update samplers.
    DescriptorSetHandle descriptorSet = shader.createDescriptorSet(api);
    api.updateDescriptorSetTexture(descriptorSet, 0, texture, {
            .filterMag = SamplerMagFilter::LINEAR,
            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
    });

    api.bindDescriptorSet(descriptorSet, 0, {});

    RenderPassParams params = getClearColorRenderPass();
    params.viewport.width = kTexSize;
    params.viewport.height = kTexSize;
    PipelineState state = getColorWritePipelineState();
    shader.addProgramToPipelineState(state);
    state.primitiveType = PrimitiveType::TRIANGLES;
    state.vertexBufferInfo = mTriangle.getVertexBufferInfo();
    api.beginRenderPass(defaultRenderTarget, params);
    api.bindPipeline(state);
    api.bindRenderPrimitive(mTriangle.getRenderPrimitive());
    api.draw2(0, 3, 1);
    api.endRenderPass();

    EXPECT_IMAGE(defaultRenderTarget,
            ScreenshotParams(kTexSize, kTexSize, "UpdateImageSRGB", 3300305265));

    api.commit(swapChain);
    api.endFrame(0);

    api.stopCapture();
}

TEST_F(LoadImageTest, UpdateImageMipLevel) {
    FAIL_IF(SkipEnvironment(OperatingSystem::APPLE, Backend::VULKAN),
            "Crashing when reading pixels without a redundant call to makeCurrent right before the"
            "render pass. b/422798473");

    auto& api = getDriverApi();
    Cleanup cleanup(api);
    api.startCapture();

    PixelDataFormat pixelFormat = PixelDataFormat::RGBA;
    PixelDataType pixelType = PixelDataType::HALF;
    TextureFormat textureFormat = TextureFormat::RGBA32F;

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = cleanup.add(createSwapChain());
    api.makeCurrent(swapChain, swapChain);
    auto defaultRenderTarget = cleanup.add(api.createDefaultRenderTarget(0));

    // Create a program.
    filament::SamplerInterfaceBlock::SamplerInfo samplerInfo { "test", "tex", 0,
        SamplerType::SAMPLER_2D, getSamplerFormat(textureFormat), Precision::HIGH, false };
    std::string const fragment = getFormattedFragment(fragmentUpdateImageMip, textureFormat);
    Shader shader(api, cleanup, ShaderConfig {
        .vertexShader = mVertexShader,
        .fragmentShader = fragment,
        .uniforms = {{"test_tex", DescriptorType::SAMPLER_2D_FLOAT, samplerInfo}}
    });

    // Create a texture with 3 mip levels.
    // Base level: 1024
    // Level 1:     512     <-- upload data and sample from this level
    // Level 2:     256
    Handle<HwTexture> texture = cleanup.add(
            api.createTexture(SamplerType::SAMPLER_2D, 3, textureFormat, 1, kDoubleTexSize,
                    kDoubleTexSize, 1, TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE));

    // Create image data.
    PixelBufferDescriptor descriptor = checkerboardPixelBuffer(pixelFormat, pixelType, kTexSize);
    api.update3DImage(texture, /* level*/ 1, 0, 0, 0, kTexSize, kTexSize, 1, std::move(descriptor));

    api.beginFrame(0, 0, 0);

    // Update samplers.
    DescriptorSetHandle descriptorSet = shader.createDescriptorSet(api);
    api.updateDescriptorSetTexture(descriptorSet, 0, texture, {
            .filterMag = SamplerMagFilter::LINEAR,
            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
    });

    api.bindDescriptorSet(descriptorSet, 0, {});

    {
        RenderFrame frame(api);
        RenderPassParams params = getClearColorRenderPass();
        params.viewport.width = kTexSize;
        params.viewport.height = kTexSize;
        PipelineState state = getColorWritePipelineState();
        shader.addProgramToPipelineState(state);
        state.primitiveType = PrimitiveType::TRIANGLES;
        state.vertexBufferInfo = mTriangle.getVertexBufferInfo();
        api.beginRenderPass(defaultRenderTarget, params);
        api.bindPipeline(state);
        api.bindRenderPrimitive(mTriangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();
    }

    EXPECT_IMAGE(defaultRenderTarget,
            ScreenshotParams(kTexSize, kTexSize, "UpdateImageMipLevel", 1875922935));

    api.commit(swapChain);
    api.endFrame(0);

    api.stopCapture();
}

TEST_F(LoadImageTest, UpdateImage3D) {
    FAIL_IF(SkipEnvironment(OperatingSystem::APPLE, Backend::VULKAN),
            "Crashing when reading pixels without a redundant call to makeCurrent right before the"
            "render pass. b/422798473");
    NONFATAL_FAIL_IF(SkipEnvironment(OperatingSystem::APPLE, Backend::VULKAN),
            "Checkerboard not drawn, possibly due to using wrong z value of 3d texture, "
            "see b/417254499");
    auto& api = getDriverApi();
    Cleanup cleanup(api);
    api.startCapture();

    PixelDataFormat pixelFormat = PixelDataFormat::RGBA;
    PixelDataType pixelType = PixelDataType::FLOAT;
    TextureFormat textureFormat = TextureFormat::RGBA16F;
    SamplerType samplerType = SamplerType::SAMPLER_2D_ARRAY;
    TextureUsage usage = TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE;

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = cleanup.add(createSwapChain());
    api.makeCurrent(swapChain, swapChain);
    auto defaultRenderTarget = cleanup.add(api.createDefaultRenderTarget(0));

    // Create a program.
    filament::SamplerInterfaceBlock::SamplerInfo samplerInfo { "test", "tex", 0,
        SamplerType::SAMPLER_2D_ARRAY, getSamplerFormat(textureFormat), Precision::HIGH, false };
    std::string fragment = getFormattedFragment(fragmentUpdateImage3DTemplate, samplerType);
    Shader shader(api, cleanup, ShaderConfig {
        .vertexShader = mVertexShader,
        .fragmentShader = fragment,
        .uniforms = {{"test_tex", DescriptorType::SAMPLER_2D_ARRAY_FLOAT, samplerInfo}}
    });

    // Create a texture.
    Handle<HwTexture> texture = cleanup.add(api.createTexture(samplerType, 1,
            textureFormat, 1, kTexSize, kTexSize, 4, usage));

    // Create image data for all 4 layers.
    size_t components; int bpp;
    getPixelInfo(pixelFormat, pixelType, components, bpp);
    size_t bpl = kTexSize * kTexSize * bpp;
    size_t bufferSize = bpl * 4;
    void* buffer = calloc(1, bufferSize);
    PixelBufferDescriptor descriptor(buffer, bufferSize, pixelFormat, pixelType,
            1, 0, 0, kTexSize, [](void* buffer, size_t size, void* user) {
                free(buffer);
            }, nullptr);

    // Only add checkerboard data to the 3rd layer, which we'll sample from.
    uint8_t* thirdLayer = (uint8_t*) buffer + (bpl * 2);
    fillCheckerboard<float>(thirdLayer, kTexSize, kTexSize, components, 1.0f);

    api.update3DImage(texture, 0, 0, 0, 0, kTexSize, kTexSize, 4, std::move(descriptor));

    {
        RenderFrame frame(api);

        // Update samplers.
        DescriptorSetHandle descriptorSet = shader.createDescriptorSet(api);
        api.updateDescriptorSetTexture(descriptorSet, 0, texture,
                { .filterMag = SamplerMagFilter::LINEAR,
                    .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST });

        api.bindDescriptorSet(descriptorSet, 0, {});

        RenderPassParams params = getClearColorRenderPass();
        params.viewport.width = kTexSize;
        params.viewport.height = kTexSize;
        PipelineState state = getColorWritePipelineState();
        shader.addProgramToPipelineState(state);
        state.primitiveType = PrimitiveType::TRIANGLES;
        state.vertexBufferInfo = mTriangle.getVertexBufferInfo();
        api.beginRenderPass(defaultRenderTarget, params);
        api.bindPipeline(state);
        api.bindRenderPrimitive(mTriangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();

        EXPECT_IMAGE(defaultRenderTarget,
                ScreenshotParams(kTexSize, kTexSize, "UpdateImage3D", 1875922935));
    }

    api.stopCapture();
}

} // namespace test
