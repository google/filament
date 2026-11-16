/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "webgpu/WebGPURenderPassMipmapGenerator.h"
#include "webgpu/WebGPUTexture.h"
#include "webgpu/WebGPUTextureHelpers.h"

#include <backend/DriverEnums.h>

#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

namespace test {

using filament::backend::fToWGPUTextureUsage;
using filament::backend::isCompressedTextureFormat;
using filament::backend::TextureFormat;
using filament::backend::TextureUsage;
using filament::backend::toWGPUTextureFormat;
using filament::backend::WebGPURenderPassMipmapGenerator;
using filament::backend::WebGPUTexture;

namespace {

[[nodiscard]] bool hasUsage(const wgpu::TextureUsage usage, const wgpu::TextureUsage flag) {
    return (usage & flag) == flag;
}

} // namespace

// Regression test for https://github.com/google/filament/issues/10538
TEST(fToWGPUTextureUsage, compressedUploadableSampleableOmitsRenderAttachment) {
    const wgpu::TextureUsage usage{ fToWGPUTextureUsage(
            TextureUsage::UPLOADABLE | TextureUsage::SAMPLEABLE, TextureFormat::DXT5_SRGBA, 1,
            false, false, false) };
    EXPECT_TRUE(hasUsage(usage, wgpu::TextureUsage::CopyDst));
    EXPECT_TRUE(hasUsage(usage, wgpu::TextureUsage::TextureBinding));
    EXPECT_FALSE(hasUsage(usage, wgpu::TextureUsage::RenderAttachment));
    EXPECT_FALSE(hasUsage(usage, wgpu::TextureUsage::StorageBinding));
}

TEST(fToWGPUTextureUsage, compressedOmitsRenderAttachmentAndStorageEvenIfRequested) {
    for (TextureFormat const format : { TextureFormat::ETC2_EAC_RGBA8, TextureFormat::DXT1_RGBA,
                 TextureFormat::RED_RGTC1, TextureFormat::RGBA_BPTC_UNORM,
                 TextureFormat::RGBA_ASTC_4x4, TextureFormat::SRGB8_ALPHA8_ASTC_12x12 }) {
        const wgpu::TextureUsage usage{ fToWGPUTextureUsage(
                TextureUsage::UPLOADABLE | TextureUsage::SAMPLEABLE |
                        TextureUsage::GEN_MIPMAPPABLE,
                format, 1, true, true, false) };
        EXPECT_TRUE(hasUsage(usage, wgpu::TextureUsage::CopyDst));
        EXPECT_TRUE(hasUsage(usage, wgpu::TextureUsage::TextureBinding));
        EXPECT_FALSE(hasUsage(usage, wgpu::TextureUsage::RenderAttachment));
        EXPECT_FALSE(hasUsage(usage, wgpu::TextureUsage::StorageBinding));
    }
}

TEST(fToWGPUTextureUsage, uncompressedUploadableKeepsRenderAttachment) {
    // Uncompressed uploads may need a conversion blit, which requires RenderAttachment.
    const wgpu::TextureUsage usage{ fToWGPUTextureUsage(
            TextureUsage::UPLOADABLE | TextureUsage::SAMPLEABLE, TextureFormat::RGBA8, 1, false,
            false, false) };
    EXPECT_TRUE(hasUsage(usage, wgpu::TextureUsage::CopyDst));
    EXPECT_TRUE(hasUsage(usage, wgpu::TextureUsage::TextureBinding));
    EXPECT_TRUE(hasUsage(usage, wgpu::TextureUsage::RenderAttachment));
}

TEST(fToWGPUTextureUsage, uncompressedComputeStorageKeepsStorageBinding) {
    const wgpu::TextureUsage usage{ fToWGPUTextureUsage(
            TextureUsage::UPLOADABLE | TextureUsage::SAMPLEABLE, TextureFormat::RGBA8, 1, true,
            false, false) };
    EXPECT_TRUE(hasUsage(usage, wgpu::TextureUsage::StorageBinding));
}

TEST(fToWGPUTextureUsage, compressedAttachmentOmitsTransientAttachment) {
    const wgpu::TextureUsage compressedUsage{ fToWGPUTextureUsage(
            TextureUsage::COLOR_ATTACHMENT, TextureFormat::DXT5_SRGBA, 1, false, false, true) };
    EXPECT_FALSE(hasUsage(compressedUsage, wgpu::TextureUsage::RenderAttachment));
#if !defined(__EMSCRIPTEN__)
    EXPECT_FALSE(hasUsage(compressedUsage, wgpu::TextureUsage::TransientAttachment));
#endif

    const wgpu::TextureUsage uncompressedUsage{ fToWGPUTextureUsage(
            TextureUsage::COLOR_ATTACHMENT, TextureFormat::RGBA8, 1, false, false, true) };
    EXPECT_TRUE(hasUsage(uncompressedUsage, wgpu::TextureUsage::RenderAttachment));
#if !defined(__EMSCRIPTEN__)
    EXPECT_TRUE(hasUsage(uncompressedUsage, wgpu::TextureUsage::TransientAttachment));
#endif
}

TEST(isCompressedTextureFormat, distinguishesCompressedFromUncompressed) {
    for (wgpu::TextureFormat const format : {
                 wgpu::TextureFormat::BC1RGBAUnorm,
                 wgpu::TextureFormat::BC3RGBAUnormSrgb,
                 wgpu::TextureFormat::BC4RUnorm,
                 wgpu::TextureFormat::BC7RGBAUnorm,
                 wgpu::TextureFormat::ETC2RGB8Unorm,
                 wgpu::TextureFormat::ETC2RGBA8UnormSrgb,
                 wgpu::TextureFormat::EACR11Unorm,
                 wgpu::TextureFormat::ASTC4x4Unorm,
                 wgpu::TextureFormat::ASTC6x6UnormSrgb,
                 wgpu::TextureFormat::ASTC12x12Unorm,
         }) {
        EXPECT_TRUE(isCompressedTextureFormat(format));
    }

    for (wgpu::TextureFormat const format : {
                 wgpu::TextureFormat::Undefined,
                 wgpu::TextureFormat::RGBA8Unorm,
                 wgpu::TextureFormat::RGBA8UnormSrgb,
                 wgpu::TextureFormat::RGBA16Float,
                 wgpu::TextureFormat::Depth32Float,
             }) {
        EXPECT_FALSE(isCompressedTextureFormat(format));
    }
}

TEST(WebGPURenderPassMipmapGenerator, compressedFormatsAreNotMipmappable) {
    for (wgpu::TextureFormat const format : {
                 wgpu::TextureFormat::BC1RGBAUnorm,
                 wgpu::TextureFormat::BC3RGBAUnormSrgb,
                 wgpu::TextureFormat::BC7RGBAUnorm,
                 wgpu::TextureFormat::ETC2RGBA8UnormSrgb,
                 wgpu::TextureFormat::EACR11Unorm,
                 wgpu::TextureFormat::ASTC4x4Unorm,
                 wgpu::TextureFormat::ASTC6x6UnormSrgb,
             }) {
        auto const compat = WebGPURenderPassMipmapGenerator::getCompatibilityFor(
                format, wgpu::TextureDimension::e2D, 1, true);
        EXPECT_FALSE(compat.compatible);
    }

    auto const rgba8Compat = WebGPURenderPassMipmapGenerator::getCompatibilityFor(
            wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureDimension::e2D, 1, true);
    EXPECT_TRUE(rgba8Compat.compatible);

    for (wgpu::TextureFormat const format : {
                 wgpu::TextureFormat::BC1RGBAUnorm,
                 wgpu::TextureFormat::BC1RGBAUnormSrgb,
                 wgpu::TextureFormat::BC3RGBAUnormSrgb,
                 wgpu::TextureFormat::BC7RGBAUnormSrgb,
                 wgpu::TextureFormat::ETC2RGB8UnormSrgb,
                 wgpu::TextureFormat::ETC2RGBA8UnormSrgb,
                 wgpu::TextureFormat::ASTC4x4UnormSrgb,
                 wgpu::TextureFormat::ASTC6x6UnormSrgb,
             }) {
        EXPECT_FALSE(WebGPUTexture::supportsMultipleMipLevelsViaStorageBinding(format));
    }
    EXPECT_TRUE(WebGPUTexture::supportsMultipleMipLevelsViaStorageBinding(
            wgpu::TextureFormat::RGBA8UnormSrgb));
}

// WebGPU has no 3-channel BC1 format, and BC1RGBAUnorm decodes the 3-color block's selector 3 as
// transparent black, which violates the DXT1_RGB contract (opaque black). These must be rejected.
TEST(toWGPUTextureFormat, dxt1RgbIsUnsupported) {
    EXPECT_EQ(toWGPUTextureFormat(TextureFormat::DXT1_RGB), wgpu::TextureFormat::Undefined);
    EXPECT_EQ(toWGPUTextureFormat(TextureFormat::DXT1_SRGB), wgpu::TextureFormat::Undefined);
    EXPECT_EQ(toWGPUTextureFormat(TextureFormat::DXT1_RGBA), wgpu::TextureFormat::BC1RGBAUnorm);
    EXPECT_EQ(toWGPUTextureFormat(TextureFormat::DXT1_SRGBA),
            wgpu::TextureFormat::BC1RGBAUnormSrgb);
}

} // namespace test
