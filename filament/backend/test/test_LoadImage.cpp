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

#include <math/half.h>

#include <fstream>
#include <vector>

#ifndef IOS
#include <imageio/BlockCompression.h>
using namespace image;
#endif

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
    //gl_Position.y = 1.0f - gl_Position.y;
    gl_Position.y *= -1.0f;
#endif
}
)");

std::string fragmentTemplate (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
// This parameter is ignored for other backends.
layout(location = 0, set = 1) uniform {samplerType} tex;

void main() {
    vec2 fbsize = vec2(512);
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    fragColor = vec4(texture(tex, uv).rgb, 1.0f);
}

)");

std::string fragmentUpdateImage3DTemplate (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
// This parameter is ignored for other backends.
layout(location = 0, set = 1) uniform {samplerType} tex;

float getLayer(in sampler3D s) { return 2.5f / 4.0f; }
float getLayer(in sampler2DArray s) { return 2.0f; }

void main() {
    vec2 fbsize = vec2(512);
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    fragColor = vec4(texture(tex, vec3(uv, getLayer(tex))).rgb, 1.0f);
}

)");

std::string fragmentUpdateImageMip (R"(#version 450 core

layout(location = 0) out vec4 fragColor;

// Filament's Vulkan backend requires a descriptor set index of 1 for all samplers.
// This parameter is ignored for other backends.
layout(location = 0, set = 1) uniform sampler2D tex;

void main() {
    vec2 fbsize = vec2(512);
    vec2 uv = gl_FragCoord.xy / fbsize;
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT)
    uv.y = 1.0 - uv.y;
#endif
    fragColor = vec4(textureLod(tex, uv, 1.0f).rgb, 1.0f);
}

)");

}

namespace test {

template<typename componentType> inline componentType getMaxValue();

template<typename ComponentType>
static void fillCheckerboard(void* buffer, size_t size, size_t stride, size_t components,
        ComponentType value) {
    ComponentType* row = (ComponentType*)buffer;
    int p = 0;
    for (int r = 0; r < size; r++) {
        ComponentType* pixel = row;
        for (int col = 0; col < size; col++) {
            // Generate a checkerboard pattern.
            if ((p & 0x0010) ^ ((p / size) & 0x0010)) {
                // Turn on the first component (red).
                pixel[0] = value;
            }
            pixel += components;
            p++;
        }
        row += stride * components;
    }
}

#ifndef IOS
static PixelBufferDescriptor compressedCheckerboardPixelBuffer(size_t size) {
    LinearImage uncompressed(size, size, 4);
    fillCheckerboard<float>(uncompressed.getPixelRef(), size, size, 4, 1.0f);

    S3tcConfig config {
        .format = CompressedFormat::RGBA_S3TC_DXT1,
        .srgb = false
    };
    CompressedTexture compressed = s3tcCompress(uncompressed, config);

    void* buffer = malloc(compressed.size);
    memcpy(buffer, compressed.data.get(), compressed.size);

    PixelBufferDescriptor descriptor(buffer, compressed.size, CompressedPixelDataType::DXT1_RGBA,
            compressed.size, [](void* buffer, size_t size, void* user) {
                free(buffer);
            }, nullptr);
    return descriptor;
}
#endif

static void getPixelInfo(PixelDataFormat format, PixelDataType type, size_t& outComponents, int& outBpp) {
    switch (format) {
        case PixelDataFormat::UNUSED:
        case PixelDataFormat::R:
        case PixelDataFormat::R_INTEGER:
        case PixelDataFormat::DEPTH_COMPONENT:
        case PixelDataFormat::ALPHA:
            outComponents = 1;
            break;
        case PixelDataFormat::RG:
        case PixelDataFormat::RG_INTEGER:
        case PixelDataFormat::DEPTH_STENCIL:
            outComponents = 2;
            break;
        case PixelDataFormat::RGB:
        case PixelDataFormat::RGB_INTEGER:
            outComponents = 3;
            break;
        case PixelDataFormat::RGBA:
        case PixelDataFormat::RGBA_INTEGER:
            outComponents = 4;
            break;
    }

    outBpp = outComponents;
    switch (type) {
        case PixelDataType::COMPRESSED:
        case PixelDataType::UBYTE:
        case PixelDataType::BYTE:
            // nothing to do
            break;
        case PixelDataType::USHORT:
        case PixelDataType::SHORT:
        case PixelDataType::HALF:
            outBpp *= 2;
            break;
        case PixelDataType::UINT:
        case PixelDataType::INT:
        case PixelDataType::FLOAT:
            outBpp *= 4;
            break;
        case PixelDataType::UINT_10F_11F_11F_REV:
            // Special case, format must be RGB and uses 4 bytes
            assert_invariant(format == PixelDataFormat::RGB);
            outBpp = 4;
            break;
        case PixelDataType::UINT_2_10_10_10_REV:
            // Special case, format must be RGBA and uses 4 bytes
            assert_invariant(format == PixelDataFormat::RGBA);
            outBpp = 4;
            break;
        case PixelDataType::USHORT_565:
            // Special case, format must be RGB and uses 2 bytes
            assert_invariant(format == PixelDataFormat::RGB);
            outBpp = 2;
            break;
    }
}

static PixelBufferDescriptor checkerboardPixelBuffer(PixelDataFormat format, PixelDataType type,
        size_t size, size_t bufferPadding = 0) {
    size_t components; int bpp;
    getPixelInfo(format, type, components, bpp);

    size_t bufferSize = size + bufferPadding * 2;
    uint8_t* buffer = (uint8_t*) calloc(1, bufferSize * bufferSize * bpp);

    uint8_t* ptr = buffer + (bufferSize * bufferPadding * bpp) + (bufferPadding * bpp);

    switch (type) {
        case PixelDataType::BYTE:
            fillCheckerboard<int8_t>(ptr, size, bufferSize, components, 1);
            break;

        case PixelDataType::UBYTE:
            fillCheckerboard<uint8_t>(ptr, size, bufferSize, components, 0xFF);
            break;

        case PixelDataType::SHORT:
            fillCheckerboard<int16_t>(ptr, size, bufferSize, components, 1);
            break;

        case PixelDataType::USHORT:
            fillCheckerboard<uint16_t>(ptr, size, bufferSize, components, 1u);
            break;

        case PixelDataType::UINT:
            fillCheckerboard<uint32_t>(ptr, size, bufferSize, components, 1u);
            break;

        case PixelDataType::INT:
            fillCheckerboard<int32_t>(ptr, size, bufferSize, components, 1);
            break;

        case PixelDataType::FLOAT:
            fillCheckerboard<float>(ptr, size, bufferSize, components, 1.0f);
            break;

        case PixelDataType::HALF:
            fillCheckerboard<math::half>(ptr, size, bufferSize, components, math::half(1.0f));
            break;

        case PixelDataType::UINT_2_10_10_10_REV:
            fillCheckerboard<uint32_t>(ptr, size, bufferSize, 1, 0xC00003FF /* red */);
            break;

        case PixelDataType::USHORT_565:
            fillCheckerboard<uint16_t>(ptr, size, bufferSize, 1, 0xF800 /* red */);
            break;

        case PixelDataType::UINT_10F_11F_11F_REV:
            fillCheckerboard<uint32_t>(ptr, size, bufferSize, 1, 0x000003C0 /* red */);
            break;

        case PixelDataType::COMPRESSED:
            break;
    }

    PixelBufferDescriptor descriptor(buffer, bufferSize * bufferSize * bpp, format, type,
            1, bufferPadding, bufferPadding, bufferSize, [](void* buffer, size_t size, void* user) {
                free(buffer);
            }, nullptr);
    return descriptor;
}

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

    /* // Test integer format uploads. */
    // TODO: These cases fail on OpenGL and Vulkan.
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

    // Test compresseed format upload.
#ifndef IOS
    testCases.emplace_back("RGBA, DXT1_RGBA -> DXT1_RGBA", PixelDataFormat::RGBA, CompressedPixelDataType::DXT1_RGBA, TextureFormat::DXT1_RGBA);
#endif

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
        ProgramHandle program;
        std::string fragment = stringReplace("{samplerType}",
                getSamplerTypeName(t.textureFormat), fragmentTemplate);
        ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
        Program prog = shaderGen.getProgram();
        Program::Sampler psamplers[] = { utils::CString("tex"), 0, false };
        prog.setSamplerGroup(0, psamplers, sizeof(psamplers) / sizeof(psamplers[0]));
        program = api.createProgram(std::move(prog));

        // Create a Texture.
        auto usage = TextureUsage::SAMPLEABLE;
        Handle<HwTexture> texture = api.createTexture(SamplerType::SAMPLER_2D, 1,
                t.textureFormat, 1, 512, 512, 1u, usage);

        // Upload some pixel data.
        if (t.compressed) {
#ifdef IOS
            assert_invariant(false);
#else
            assert_invariant(!t.uploadSubregions);
            PixelBufferDescriptor descriptor = compressedCheckerboardPixelBuffer(512);
            api.update2DImage(texture, 0, 0, 0, 512, 512, std::move(descriptor));
#endif
        } else {
            if (t.uploadSubregions) {
                const auto& pf = t.pixelFormat;
                const auto& pt = t.pixelType;
                PixelBufferDescriptor subregion1 = checkerboardPixelBuffer(pf, pt, 256, t.bufferPadding);
                PixelBufferDescriptor subregion2 = checkerboardPixelBuffer(pf, pt, 256, t.bufferPadding);
                PixelBufferDescriptor subregion3 = checkerboardPixelBuffer(pf, pt, 256, t.bufferPadding);
                PixelBufferDescriptor subregion4 = checkerboardPixelBuffer(pf, pt, 256, t.bufferPadding);
                api.update2DImage(texture, 0, 0, 0, 256, 256, std::move(subregion1));
                api.update2DImage(texture, 0, 256, 0, 256, 256, std::move(subregion2));
                api.update2DImage(texture, 0, 0, 256, 256, 256, std::move(subregion3));
                api.update2DImage(texture, 0, 256, 256, 256, 256, std::move(subregion4));
            } else {
                PixelBufferDescriptor descriptor
                    = checkerboardPixelBuffer(t.pixelFormat, t.pixelType, 512, t.bufferPadding);
                api.update2DImage(texture, 0, 0, 0, 512, 512, std::move(descriptor));
            }
        }

        SamplerGroup samplers(1);
        SamplerParams sparams = {};
        sparams.filterMag = SamplerMagFilter::LINEAR;
        sparams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
        samplers.setSampler(0, texture, sparams);
        auto sgroup = api.createSamplerGroup(samplers.getSize());
        api.updateSamplerGroup(sgroup, std::move(samplers.toCommandStream()));

        api.bindSamplers(0, sgroup);

        renderTriangle(defaultRenderTarget, swapChain, program);

        readPixelsAndAssertHash(t.name, 512, 512, defaultRenderTarget, expectedHash);

        api.flush();
        api.commit(swapChain);
        api.endFrame(0);

        api.destroyProgram(program);
        api.destroySwapChain(swapChain);
        api.destroyRenderTarget(defaultRenderTarget);
        api.destroyTexture(texture);

        // This ensures all driver commands have finished before exiting the test.
        api.finish();
    }

    api.stopCapture();

    executeCommands();

    getDriver().purge();
}

TEST_F(BackendTest, UpdateImageSRGB) {
    auto& api = getDriverApi();
    api.startCapture();

    PixelDataFormat pixelFormat = PixelDataFormat::RGB;
    PixelDataType pixelType = PixelDataType::UBYTE;
    TextureFormat textureFormat = TextureFormat::SRGB8_A8;

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = createSwapChain();
    api.makeCurrent(swapChain, swapChain);
    auto defaultRenderTarget = api.createDefaultRenderTarget(0);

    // Create a program.
    std::string fragment = stringReplace("{samplerType}",
            getSamplerTypeName(textureFormat), fragmentTemplate);
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
    Program prog = shaderGen.getProgram();
    Program::Sampler psamplers[] = { utils::CString("tex"), 0, false };
    prog.setSamplerGroup(0, psamplers, sizeof(psamplers) / sizeof(psamplers[0]));
    ProgramHandle program = api.createProgram(std::move(prog));

    // Create a texture.
    Handle<HwTexture> texture = api.createTexture(SamplerType::SAMPLER_2D, 1,
            textureFormat, 1, 512, 512, 1, TextureUsage::SAMPLEABLE);

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

    api.update2DImage(texture, 0, 0, 0, 512, 512, std::move(descriptor));

    api.beginFrame(0, 0);

    // Update samplers.
    SamplerGroup samplers(1);
    SamplerParams sparams = {};
    sparams.filterMag = SamplerMagFilter::LINEAR;
    sparams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
    samplers.setSampler(0, texture, sparams);
    auto sgroup = api.createSamplerGroup(samplers.getSize());
    api.updateSamplerGroup(sgroup, std::move(samplers.toCommandStream()));

    api.bindSamplers(0, sgroup);

    renderTriangle(defaultRenderTarget, swapChain, program);

    static const uint32_t expectedHash = 519370995;
    readPixelsAndAssertHash("UpdateImageSRGB", 512, 512, defaultRenderTarget, expectedHash);

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    api.destroySamplerGroup(sgroup);
    api.destroyProgram(program);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(defaultRenderTarget);

    // This ensures all driver commands have finished before exiting the test.
    api.finish();

    api.stopCapture();

    executeCommands();

    getDriver().purge();
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
    std::string fragment = stringReplace("{samplerType}",
            getSamplerTypeName(textureFormat), fragmentUpdateImageMip);
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
    Program prog = shaderGen.getProgram();
    Program::Sampler psamplers[] = { utils::CString("tex"), 0, false };
    prog.setSamplerGroup(0, psamplers, sizeof(psamplers) / sizeof(psamplers[0]));
    ProgramHandle program = api.createProgram(std::move(prog));

    // Create a texture with 3 mip levels.
    // Base level: 1024
    // Level 1:     512     <-- upload data and sample from this level
    // Level 2:     256
    Handle<HwTexture> texture = api.createTexture(SamplerType::SAMPLER_2D, 3,
            textureFormat, 1, 1024, 1024, 1, TextureUsage::SAMPLEABLE);

    // Create image data.
    PixelBufferDescriptor descriptor = checkerboardPixelBuffer(pixelFormat, pixelType, 512);
    api.update2DImage(texture, 1, 0, 0, 512, 512, std::move(descriptor));

    api.beginFrame(0, 0);

    // Update samplers.
    SamplerGroup samplers(1);
    SamplerParams sparams = {};
    sparams.filterMag = SamplerMagFilter::LINEAR;
    sparams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
    samplers.setSampler(0, texture, sparams);
    auto sgroup = api.createSamplerGroup(samplers.getSize());
    api.updateSamplerGroup(sgroup, std::move(samplers.toCommandStream()));

    api.bindSamplers(0, sgroup);

    renderTriangle(defaultRenderTarget, swapChain, program);

    static const uint32_t expectedHash = 3644679986;
    readPixelsAndAssertHash("UpdateImageMipLevel", 512, 512, defaultRenderTarget, expectedHash);

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    api.destroySamplerGroup(sgroup);
    api.destroyProgram(program);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(defaultRenderTarget);

    // This ensures all driver commands have finished before exiting the test.
    api.finish();

    api.stopCapture();

    executeCommands();

    getDriver().purge();
}

TEST_F(BackendTest, UpdateImage3D) {
    auto& api = getDriverApi();
    api.startCapture();

    PixelDataFormat pixelFormat = PixelDataFormat::RGBA;
    PixelDataType pixelType = PixelDataType::FLOAT;
    TextureFormat textureFormat = TextureFormat::RGBA16F;
    SamplerType samplerType = SamplerType::SAMPLER_2D_ARRAY;
    TextureUsage usage = TextureUsage::SAMPLEABLE;

    // Create a platform-specific SwapChain and make it current.
    auto swapChain = createSwapChain();
    api.makeCurrent(swapChain, swapChain);
    auto defaultRenderTarget = api.createDefaultRenderTarget(0);

    // Create a program.
    std::string fragment = stringReplace("{samplerType}",
            getSamplerTypeName(samplerType), fragmentUpdateImage3DTemplate);
    ShaderGenerator shaderGen(vertex, fragment, sBackend, sIsMobilePlatform);
    Program prog = shaderGen.getProgram();
    Program::Sampler psamplers[] = { utils::CString("tex"), 0, false };
    prog.setSamplerGroup(0, psamplers, sizeof(psamplers) / sizeof(psamplers[0]));
    ProgramHandle program = api.createProgram(std::move(prog));

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

    api.beginFrame(0, 0);

    // Update samplers.
    SamplerGroup samplers(1);
    SamplerParams sparams = {};
    sparams.filterMag = SamplerMagFilter::LINEAR;
    sparams.filterMin = SamplerMinFilter::LINEAR_MIPMAP_NEAREST;
    samplers.setSampler(0, texture, sparams);
    auto sgroup = api.createSamplerGroup(samplers.getSize());
    api.updateSamplerGroup(sgroup, std::move(samplers.toCommandStream()));

    api.bindSamplers(0, sgroup);

    renderTriangle(defaultRenderTarget, swapChain, program);

    static const uint32_t expectedHash = 3644679986;
    readPixelsAndAssertHash("UpdateImage3D", 512, 512, defaultRenderTarget, expectedHash);

    api.flush();
    api.commit(swapChain);
    api.endFrame(0);

    api.destroySamplerGroup(sgroup);
    api.destroyProgram(program);
    api.destroySwapChain(swapChain);
    api.destroyRenderTarget(defaultRenderTarget);

    // This ensures all driver commands have finished before exiting the test.
    api.finish();

    api.stopCapture();

    executeCommands();

    getDriver().purge();
}

} // namespace test
