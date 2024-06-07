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
#include "BackendTestUtils.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include "private/filament/SamplerInterfaceBlock.h"

#include <math/half.h>

#include <vector>

#include <stddef.h>
#include <stdint.h>

using namespace filament;
using namespace filament::backend;

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string vertex (R"(#version 450 core

layout(location = 0) in vec4 mesh_position;

void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    gl_Position.y *= -1.0f;
#endif
}
)");

std::string fragmentTemplate (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
// This parameter is ignored for other backends.
layout(location = 0, set = 1) uniform {samplerType} test_tex;

void main() {
    vec2 fbsize = vec2(512);
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    fragColor = vec4(texture(test_tex, uv).rgb, 1.0f);
}

)");

std::string fragmentUpdateImage3DTemplate (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
layout(location = 0, set = 1) uniform {samplerType} test_tex;

float getLayer(in sampler3D s) { return 2.5f / 4.0f; }
float getLayer(in sampler2DArray s) { return 2.0f; }

void main() {
    vec2 fbsize = vec2(512);
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    fragColor = vec4(texture(test_tex, vec3(uv, getLayer(test_tex))).rgb, 1.0f);
}

)");

std::string fragmentUpdateImageMip (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
layout(location = 0, set = 1) uniform sampler2D test_tex;

void main() {
    vec2 fbsize = vec2(512);
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
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

TEST_F(BackendTest, UpdateImage2D) {

    // All of these test cases should result in the same rendered image, and thus the same hash.
    static const uint32_t expectedHash = 3644679986;

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
    testCases.emplace_back("RGBA, UBYTE -> RGBA8", PixelDataFormat::RGBA, PixelDataType::UBYTE, TextureFormat::RGBA8);

    // Test format conversion.
    // TODO: Vulkan crashes with `Texture at colorAttachment[0] has usage (0x01) which doesn't specify MTLTextureUsageRenderTarget (0x04)'
    testCases.emplace_back("RGBA, FLOAT -> RGBA16F", PixelDataFormat::RGBA, PixelDataType::FLOAT, TextureFormat::RGBA16F);

    // Test texture formats not all backends support natively.
    // TODO: Vulkan crashes with "VK_FORMAT_R32G32B32_SFLOAT is not supported"
    testCases.emplace_back("RGB, FLOAT -> RGB32F", PixelDataFormat::RGB, PixelDataType::FLOAT, TextureFormat::RGB32F);
    testCases.emplace_back("RGB, FLOAT -> RGB16F", PixelDataFormat::RGB, PixelDataType::FLOAT, TextureFormat::RGB16F);

    // Test packed format uploads.
    // TODO: Vulkan crashes with "Texture at colorAttachment[0] has usage (0x01) which doesn't specify MTLTextureUsageRenderTarget (0x04)"
    testCases.emplace_back("RGBA, UINT_2_10_10_10_REV -> RGB10_A2", PixelDataFormat::RGBA, PixelDataType::UINT_2_10_10_10_REV, TextureFormat::RGB10_A2);
    testCases.emplace_back("RGB, UINT_10F_11F_11F_REV -> R11F_G11F_B10F", PixelDataFormat::RGB, PixelDataType::UINT_10F_11F_11F_REV, TextureFormat::R11F_G11F_B10F);
    testCases.emplace_back("RGB, HALF -> R11F_G11F_B10F", PixelDataFormat::RGB, PixelDataType::HALF, TextureFormat::R11F_G11F_B10F);

    // Test integer format uploads.
    // TODO: These cases fail on OpenGL and Vulkan.
    // TODO: These cases now also fail on Metal, but at some point previously worked.
     testCases.emplace_back("RGB_INTEGER, UBYTE -> RGB8UI", PixelDataFormat::RGB_INTEGER, PixelDataType::UBYTE, TextureFormat::RGB8UI);
     testCases.emplace_back("RGB_INTEGER, USHORT -> RGB16UI", PixelDataFormat::RGB_INTEGER, PixelDataType::USHORT, TextureFormat::RGB16UI);
     testCases.emplace_back("RGB_INTEGER, INT -> RGB32I", PixelDataFormat::RGB_INTEGER, PixelDataType::INT, TextureFormat::RGB32I);

    // Test uploads with buffer padding.
    // TODO: Vulkan crashes with "Assertion failed: (offset + size <= allocationSize)"
    testCases.emplace_back("RGBA, UBYTE -> RGBA8 (with buffer padding)", PixelDataFormat::RGBA, PixelDataType::UBYTE, TextureFormat::RGBA8, 64u);
    testCases.emplace_back("RGBA, FLOAT -> RGBA16F (with buffer padding)", PixelDataFormat::RGBA, PixelDataType::FLOAT, TextureFormat::RGBA16F, 64u);
    testCases.emplace_back("RGB, FLOAT -> RGB32F (with buffer padding)", PixelDataFormat::RGB, PixelDataType::FLOAT, TextureFormat::RGB32F, 64u);

    // Upload subregions separately.
    // TODO: Vulkan crashes with "Offsets not yet supported"
    testCases.emplace_back("RGBA, UBYTE -> RGBA8 (subregions)", PixelDataFormat::RGBA, PixelDataType::UBYTE, TextureFormat::RGBA8, 0u, true);
    testCases.emplace_back("RGBA, FLOAT -> RGBA16F (subregions)", PixelDataFormat::RGBA, PixelDataType::FLOAT, TextureFormat::RGBA16F, 0u, true);
    testCases.emplace_back("RGBA, UBYTE -> RGBA8 (subregions, buffer padding)", PixelDataFormat::RGBA, PixelDataType::UBYTE, TextureFormat::RGBA8, 64u, true);
    testCases.emplace_back("RGB, FLOAT -> RGB32F (subregions, buffer padding)", PixelDataFormat::RGB, PixelDataType::FLOAT, TextureFormat::RGB32F, 64u, true);

    auto& api = getDriverApi();

    api.startCapture();

    // The test is executed within this block scope to force destructors to run before
    // executeCommands().
    for (const auto& t : testCases) {
        // Create a platform-specific SwapChain and make it current.
        auto swapChain = createSwapChain();
        api.makeCurrent(swapChain, swapChain);
        auto defaultRenderTarget = api.createDefaultRenderTarget(0);

        // Create a program.
        SamplerInterfaceBlock const sib = filament::SamplerInterfaceBlock::Builder()
                .name("Test")
                .stageFlags(backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS)
                .add( {{"tex", 0, SamplerType::SAMPLER_2D, getSamplerFormat(t.textureFormat), Precision::HIGH }} )
                .build();

        std::string const fragment = stringReplace("{samplerType}",
                getSamplerTypeName(t.textureFormat), fragmentTemplate);
        ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform, &sib);

        Program prog = shaderGen.getProgram(api);
        prog.descriptorBindings(0, {{"test_tex", DescriptorType::SAMPLER, 0}});

        ProgramHandle const program = api.createProgram(std::move(prog));

        DescriptorSetLayoutHandle descriptorSetLayout = api.createDescriptorSetLayout({
                {{
                         DescriptorType::SAMPLER,
                         ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, 0,
                         DescriptorFlags::NONE, 0
                 }}});

        DescriptorSetHandle descriptorSet = api.createDescriptorSet(descriptorSetLayout);

        // Create a Texture.
        auto usage = TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE;
        Handle<HwTexture> const texture = api.createTexture(SamplerType::SAMPLER_2D, 1,
                t.textureFormat, 1, 512, 512, 1u, usage);

        // Upload some pixel data.
        if (t.uploadSubregions) {
            api.update3DImage(texture, 0,   0,   0, 0, 256, 256, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, 256, t.bufferPadding));
            api.update3DImage(texture, 0, 256,   0, 0, 256, 256, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, 256, t.bufferPadding));
            api.update3DImage(texture, 0,   0, 256, 0, 256, 256, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, 256, t.bufferPadding));
            api.update3DImage(texture, 0, 256, 256, 0, 256, 256, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, 256, t.bufferPadding));
        } else {
            api.update3DImage(texture, 0, 0, 0, 0, 512, 512, 1,
                    checkerboardPixelBuffer(t.pixelFormat, t.pixelType, 512, t.bufferPadding));
        }

        api.updateDescriptorSetTexture(descriptorSet, 0, texture, {
                .filterMag = SamplerMagFilter::NEAREST,
                .filterMin = SamplerMinFilter::NEAREST_MIPMAP_NEAREST });

        api.bindDescriptorSet(descriptorSet, 0, {});

        renderTriangle({ descriptorSetLayout }, defaultRenderTarget, swapChain, program);

        readPixelsAndAssertHash(t.name, 512, 512, defaultRenderTarget, expectedHash);

        api.commit(swapChain);
        api.endFrame(0);

        api.destroyDescriptorSet(descriptorSet);
        api.destroyDescriptorSetLayout(descriptorSetLayout);
        api.destroyProgram(program);
        api.destroySwapChain(swapChain);
        api.destroyRenderTarget(defaultRenderTarget);
        api.destroyTexture(texture);
    }

    api.finish();
    api.stopCapture();

    flushAndWait();
}

TEST_F(BackendTest, UpdateImageSRGB) {
    auto& api = getDriverApi();
    api.startCapture();

    PixelDataFormat const pixelFormat = PixelDataFormat::RGBA;
    PixelDataType const pixelType = PixelDataType::UBYTE;
    TextureFormat const textureFormat = TextureFormat::SRGB8_A8;

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = createSwapChain();
    api.makeCurrent(swapChain, swapChain);
    auto defaultRenderTarget = api.createDefaultRenderTarget(0);

    // Create a program.
    SamplerInterfaceBlock const sib = filament::SamplerInterfaceBlock::Builder()
            .name("Test")
            .stageFlags(backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS)
            .add( {{"tex", 0, SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, Precision::HIGH }} )
            .build();
    std::string const fragment = stringReplace("{samplerType}",
            getSamplerTypeName(textureFormat), fragmentTemplate);
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform, &sib);
    Program prog = shaderGen.getProgram(api);
    prog.descriptorBindings(0, {{"test_tex", DescriptorType::SAMPLER, 0}});
    ProgramHandle const program = api.createProgram(std::move(prog));
    DescriptorSetLayoutHandle descriptorSetLayout = api.createDescriptorSetLayout({
            {{
                     DescriptorType::SAMPLER,
                     ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, 0,
                     DescriptorFlags::NONE, 0
             }}});

    DescriptorSetHandle descriptorSet = api.createDescriptorSet(descriptorSetLayout);

    // Create a texture.
    Handle<HwTexture> const texture = api.createTexture(SamplerType::SAMPLER_2D, 1,
            textureFormat, 1, 512, 512, 1, TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE);

    // Create image data.
    size_t components; int bpp;
    getPixelInfo(pixelFormat, pixelType, components, bpp);
    size_t bpl = 512 * 512 * bpp;
    size_t bufferSize = bpl;
    void* buffer = calloc(1, bufferSize);
    PixelBufferDescriptor descriptor(buffer, bufferSize, pixelFormat, pixelType,
            1, 0, 0, 512, [](void* buffer, size_t size, void* user) {
                free(buffer);
            }, nullptr);

    // Add a gradient.
    uint8_t* pixel = (uint8_t*) buffer;
    for (int r = 0; r < 512; r++) {
        for (int c = 0; c < 512; c++) {
            for (int n = 0; n < components; n++) {
                *pixel++ = (c / 512.0f) * 255;
            }
        }
    }

    api.update3DImage(texture, 0, 0, 0, 0, 512, 512, 1, std::move(descriptor));

    api.beginFrame(0, 0, 0);

    // Update samplers.
    api.updateDescriptorSetTexture(descriptorSet, 0, texture, {
            .filterMag = SamplerMagFilter::LINEAR,
            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
    });

    api.bindDescriptorSet(descriptorSet, 0, {});

    renderTriangle({ descriptorSetLayout }, defaultRenderTarget, swapChain, program);

    static const uint32_t expectedHash = 359858623;
    readPixelsAndAssertHash("UpdateImageSRGB", 512, 512, defaultRenderTarget, expectedHash);

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    api.destroyDescriptorSet(descriptorSet);
    api.destroyDescriptorSetLayout(descriptorSetLayout);
    api.destroyProgram(program);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(defaultRenderTarget);

    // This ensures all driver commands have finished before exiting the test.
    api.finish();
    api.stopCapture();

    flushAndWait();
}

TEST_F(BackendTest, UpdateImageMipLevel) {
    auto& api = getDriverApi();
    api.startCapture();

    PixelDataFormat pixelFormat = PixelDataFormat::RGBA;
    PixelDataType pixelType = PixelDataType::HALF;
    TextureFormat textureFormat = TextureFormat::RGBA32F;

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = createSwapChain();
    api.makeCurrent(swapChain, swapChain);
    auto defaultRenderTarget = api.createDefaultRenderTarget(0);

    // Create a program.
    SamplerInterfaceBlock sib = filament::SamplerInterfaceBlock::Builder()
            .name("Test")
            .stageFlags(backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS)
            .add( {{"tex", 0, SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, Precision::HIGH }} )
            .build();
    std::string const fragment = stringReplace("{samplerType}",
            getSamplerTypeName(textureFormat), fragmentUpdateImageMip);
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform, &sib);
    Program prog = shaderGen.getProgram(api);
    prog.descriptorBindings(0, {{"test_tex", DescriptorType::SAMPLER, 0}});
    ProgramHandle const program = api.createProgram(std::move(prog));
    DescriptorSetLayoutHandle descriptorSetLayout = api.createDescriptorSetLayout({
            {{
                     DescriptorType::SAMPLER,
                     ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, 0,
                     DescriptorFlags::NONE, 0
             }}});

    DescriptorSetHandle descriptorSet = api.createDescriptorSet(descriptorSetLayout);

    // Create a texture with 3 mip levels.
    // Base level: 1024
    // Level 1:     512     <-- upload data and sample from this level
    // Level 2:     256
    Handle<HwTexture> texture = api.createTexture(SamplerType::SAMPLER_2D, 3,
            textureFormat, 1, 1024, 1024, 1, TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE);

    // Create image data.
    PixelBufferDescriptor descriptor = checkerboardPixelBuffer(pixelFormat, pixelType, 512);
    api.update3DImage(texture, /* level*/ 1, 0, 0, 0, 512, 512, 1, std::move(descriptor));

    api.beginFrame(0, 0, 0);

    // Update samplers.
    api.updateDescriptorSetTexture(descriptorSet, 0, texture, {
            .filterMag = SamplerMagFilter::LINEAR,
            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
    });

    api.bindDescriptorSet(descriptorSet, 0, {});

    renderTriangle({ descriptorSetLayout }, defaultRenderTarget, swapChain, program);

    static const uint32_t expectedHash = 3644679986;
    readPixelsAndAssertHash("UpdateImageMipLevel", 512, 512, defaultRenderTarget, expectedHash);

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    api.destroyDescriptorSet(descriptorSet);
    api.destroyDescriptorSetLayout(descriptorSetLayout);
    api.destroyProgram(program);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(defaultRenderTarget);

    // This ensures all driver commands have finished before exiting the test.
    api.finish();
    api.stopCapture();

    flushAndWait();
}

TEST_F(BackendTest, UpdateImage3D) {
    auto& api = getDriverApi();
    api.startCapture();

    PixelDataFormat pixelFormat = PixelDataFormat::RGBA;
    PixelDataType pixelType = PixelDataType::FLOAT;
    TextureFormat textureFormat = TextureFormat::RGBA16F;
    SamplerType samplerType = SamplerType::SAMPLER_2D_ARRAY;
    TextureUsage usage = TextureUsage::SAMPLEABLE | TextureUsage::UPLOADABLE;

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = createSwapChain();
    api.makeCurrent(swapChain, swapChain);
    auto defaultRenderTarget = api.createDefaultRenderTarget(0);

    // Create a program.
    SamplerInterfaceBlock sib = filament::SamplerInterfaceBlock::Builder()
            .name("Test")
            .stageFlags(backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS)
            .add( {{"tex", 0, SamplerType::SAMPLER_2D_ARRAY, SamplerFormat::FLOAT, Precision::HIGH }} )
            .build();
    std::string fragment = stringReplace("{samplerType}",
            getSamplerTypeName(samplerType), fragmentUpdateImage3DTemplate);
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform, &sib);
    Program prog = shaderGen.getProgram(api);
    prog.descriptorBindings(0, {{ "test_tex", DescriptorType::SAMPLER, 0 }});
    ProgramHandle const program = api.createProgram(std::move(prog));
    DescriptorSetLayoutHandle descriptorSetLayout = api.createDescriptorSetLayout({
            {{
                     DescriptorType::SAMPLER,
                     ShaderStageFlags::ALL_SHADER_STAGE_FLAGS, 0,
                     DescriptorFlags::NONE, 0
             }}});

    DescriptorSetHandle descriptorSet = api.createDescriptorSet(descriptorSetLayout);

    // Create a texture.
    Handle<HwTexture> texture = api.createTexture(samplerType, 1,
            textureFormat, 1, 512, 512, 4, usage);

    // Create image data for all 4 layers.
    size_t components; int bpp;
    getPixelInfo(pixelFormat, pixelType, components, bpp);
    size_t bpl = 512 * 512 * bpp;
    size_t bufferSize = bpl * 4;
    void* buffer = calloc(1, bufferSize);
    PixelBufferDescriptor descriptor(buffer, bufferSize, pixelFormat, pixelType,
            1, 0, 0, 512, [](void* buffer, size_t size, void* user) {
                free(buffer);
            }, nullptr);

    // Only add checkerboard data to the 3rd layer, which we'll sample from.
    uint8_t* thirdLayer = (uint8_t*) buffer + (bpl * 2);
    fillCheckerboard<float>(thirdLayer, 512, 512, components, 1.0f);

    api.update3DImage(texture, 0, 0, 0, 0, 512, 512, 4, std::move(descriptor));

    api.beginFrame(0, 0, 0);

    // Update samplers.
    api.updateDescriptorSetTexture(descriptorSet, 0, texture, {
            .filterMag = SamplerMagFilter::LINEAR,
            .filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST
    });

    api.bindDescriptorSet(descriptorSet, 0, {});

    renderTriangle({ descriptorSetLayout }, defaultRenderTarget, swapChain, program);

    static const uint32_t expectedHash = 3644679986;
    readPixelsAndAssertHash("UpdateImage3D", 512, 512, defaultRenderTarget, expectedHash);

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    api.destroyDescriptorSet(descriptorSet);
    api.destroyDescriptorSetLayout(descriptorSetLayout);
    api.destroyProgram(program);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(defaultRenderTarget);

    // This ensures all driver commands have finished before exiting the test.
    api.finish();
    api.stopCapture();

    flushAndWait();
}

} // namespace test
