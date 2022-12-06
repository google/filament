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

#include "MetalEnums.h"

#include "MetalContext.h"

namespace filament {
namespace backend {

MTLPixelFormat getMetalFormat(MetalContext* context, TextureFormat format) noexcept {
    switch (format) {
        // 8-bits per element
        case TextureFormat::R8: return MTLPixelFormatR8Unorm;
        case TextureFormat::R8_SNORM: return MTLPixelFormatR8Snorm;
        case TextureFormat::R8UI: return MTLPixelFormatR8Uint;
        case TextureFormat::R8I: return MTLPixelFormatR8Sint;
        case TextureFormat::STENCIL8: return MTLPixelFormatStencil8;

        // 16-bits per element
        case TextureFormat::R16F: return MTLPixelFormatR16Float;
        case TextureFormat::R16UI: return MTLPixelFormatR16Uint;
        case TextureFormat::R16I: return MTLPixelFormatR16Sint;
        case TextureFormat::RG8: return MTLPixelFormatRG8Unorm;
        case TextureFormat::RG8_SNORM: return MTLPixelFormatRG8Snorm;
        case TextureFormat::RG8UI: return MTLPixelFormatRG8Uint;
        case TextureFormat::RG8I: return MTLPixelFormatRG8Sint;

        // 24-bits per element, not supported by Metal.
        case TextureFormat::RGB8:
        case TextureFormat::SRGB8:
        case TextureFormat::RGB8_SNORM:
        case TextureFormat::RGB8UI:
        case TextureFormat::RGB8I:
            return MTLPixelFormatInvalid;

        // 32-bits per element
        case TextureFormat::R32F: return MTLPixelFormatR32Float;
        case TextureFormat::R32UI: return MTLPixelFormatR32Uint;
        case TextureFormat::R32I: return MTLPixelFormatR32Sint;
        case TextureFormat::RG16F: return MTLPixelFormatRG16Float;
        case TextureFormat::RG16UI: return MTLPixelFormatRG16Uint;
        case TextureFormat::RG16I: return MTLPixelFormatRG16Sint;
        case TextureFormat::R11F_G11F_B10F: return MTLPixelFormatRG11B10Float;
        case TextureFormat::RGB9_E5: return MTLPixelFormatRGB9E5Float;
        case TextureFormat::RGBA8: return MTLPixelFormatRGBA8Unorm;
        case TextureFormat::SRGB8_A8: return MTLPixelFormatRGBA8Unorm_sRGB;
        case TextureFormat::RGBA8_SNORM: return MTLPixelFormatRGBA8Snorm;
        case TextureFormat::RGB10_A2: return MTLPixelFormatRGB10A2Unorm;
        case TextureFormat::RGBA8UI: return MTLPixelFormatRGBA8Uint;
        case TextureFormat::RGBA8I: return MTLPixelFormatRGBA8Sint;
        case TextureFormat::DEPTH32F: return MTLPixelFormatDepth32Float;
        case TextureFormat::DEPTH32F_STENCIL8: return MTLPixelFormatDepth32Float_Stencil8;

        // 48-bits per element
        case TextureFormat::RGB16F:
        case TextureFormat::RGB16UI:
        case TextureFormat::RGB16I:
            return MTLPixelFormatInvalid;

        // 64-bits per element
        case TextureFormat::RG32F: return MTLPixelFormatRG32Float;
        case TextureFormat::RG32UI: return MTLPixelFormatRG32Uint;
        case TextureFormat::RG32I: return MTLPixelFormatRG32Sint;
        case TextureFormat::RGBA16F: return MTLPixelFormatRGBA16Float;
        case TextureFormat::RGBA16UI: return MTLPixelFormatRGBA16Uint;
        case TextureFormat::RGBA16I: return MTLPixelFormatRGBA16Sint;

        // 96-bits per element
        case TextureFormat::RGB32F:
        case TextureFormat::RGB32UI:
        case TextureFormat::RGB32I:
            return MTLPixelFormatInvalid;

        // 128-bits per element
        case TextureFormat::RGBA32F: return MTLPixelFormatRGBA32Float;
        case TextureFormat::RGBA32UI: return MTLPixelFormatRGBA32Uint;
        case TextureFormat::RGBA32I: return MTLPixelFormatRGBA32Sint;

        case TextureFormat::UNUSED:
            return MTLPixelFormatInvalid;

        default: break;
    }

    // Packed 16 bit formats are only available on Apple GPUs.
    if (context->highestSupportedGpuFamily.apple >= 1) {
        if (@available(macOS 11.0, *)) {
            switch (format) {
                case TextureFormat::RGB565: return MTLPixelFormatB5G6R5Unorm;
                case TextureFormat::RGB5_A1: return MTLPixelFormatA1BGR5Unorm;
                case TextureFormat::RGBA4: return MTLPixelFormatABGR4Unorm;
                default: break;
            }
        }
    }

    if (@available(iOS 13.0, *)) {
        if (format == TextureFormat::DEPTH16) return MTLPixelFormatDepth16Unorm;
    }

#if TARGET_OS_OSX
    if (context->highestSupportedGpuFamily.mac >= 1 &&
            context->device.depth24Stencil8PixelFormatSupported) {
        if (@available(macOS 11.0, *)) {
            if (format == TextureFormat::DEPTH24_STENCIL8) {
                return MTLPixelFormatDepth24Unorm_Stencil8;
            }
        }
    }
#endif

    // Only iOS 13.0 and Apple Silicon support the ASTC HDR profile. Older OS versions fallback to
    // LDR. The HDR profile is a superset of the LDR profile.
    if (context->highestSupportedGpuFamily.apple >= 2) {
        if (@available(iOS 13, macOS 11.0, *)) {
            switch (format) {
                case TextureFormat::RGBA_ASTC_4x4: return MTLPixelFormatASTC_4x4_HDR;
                case TextureFormat::RGBA_ASTC_5x4: return MTLPixelFormatASTC_5x4_HDR;
                case TextureFormat::RGBA_ASTC_5x5: return MTLPixelFormatASTC_5x5_HDR;
                case TextureFormat::RGBA_ASTC_6x5: return MTLPixelFormatASTC_6x5_HDR;
                case TextureFormat::RGBA_ASTC_6x6: return MTLPixelFormatASTC_6x6_HDR;
                case TextureFormat::RGBA_ASTC_8x5: return MTLPixelFormatASTC_8x5_HDR;
                case TextureFormat::RGBA_ASTC_8x6: return MTLPixelFormatASTC_8x6_HDR;
                case TextureFormat::RGBA_ASTC_8x8: return MTLPixelFormatASTC_8x8_HDR;
                case TextureFormat::RGBA_ASTC_10x5: return MTLPixelFormatASTC_10x5_HDR;
                case TextureFormat::RGBA_ASTC_10x6: return MTLPixelFormatASTC_10x6_HDR;
                case TextureFormat::RGBA_ASTC_10x8: return MTLPixelFormatASTC_10x8_HDR;
                case TextureFormat::RGBA_ASTC_10x10: return MTLPixelFormatASTC_10x10_HDR;
                case TextureFormat::RGBA_ASTC_12x10: return MTLPixelFormatASTC_12x10_HDR;
                case TextureFormat::RGBA_ASTC_12x12: return MTLPixelFormatASTC_12x12_HDR;
                default: break;
            }
        } else if (@available(macOS 11.0, *)) {
            switch (format) {
                case TextureFormat::RGBA_ASTC_4x4: return MTLPixelFormatASTC_4x4_LDR;
                case TextureFormat::RGBA_ASTC_5x4: return MTLPixelFormatASTC_5x4_LDR;
                case TextureFormat::RGBA_ASTC_5x5: return MTLPixelFormatASTC_5x5_LDR;
                case TextureFormat::RGBA_ASTC_6x5: return MTLPixelFormatASTC_6x5_LDR;
                case TextureFormat::RGBA_ASTC_6x6: return MTLPixelFormatASTC_6x6_LDR;
                case TextureFormat::RGBA_ASTC_8x5: return MTLPixelFormatASTC_8x5_LDR;
                case TextureFormat::RGBA_ASTC_8x6: return MTLPixelFormatASTC_8x6_LDR;
                case TextureFormat::RGBA_ASTC_8x8: return MTLPixelFormatASTC_8x8_LDR;
                case TextureFormat::RGBA_ASTC_10x5: return MTLPixelFormatASTC_10x5_LDR;
                case TextureFormat::RGBA_ASTC_10x6: return MTLPixelFormatASTC_10x6_LDR;
                case TextureFormat::RGBA_ASTC_10x8: return MTLPixelFormatASTC_10x8_LDR;
                case TextureFormat::RGBA_ASTC_10x10: return MTLPixelFormatASTC_10x10_LDR;
                case TextureFormat::RGBA_ASTC_12x10: return MTLPixelFormatASTC_12x10_LDR;
                case TextureFormat::RGBA_ASTC_12x12: return MTLPixelFormatASTC_12x12_LDR;
                default: break;
            }
        }
    }

    // EAC / ETC2 formats are only available on Apple GPUs.
    if (context->highestSupportedGpuFamily.apple >= 1) {
        if (@available(macOS 11.0, *)) {
            switch (format) {
                case TextureFormat::EAC_R11: return MTLPixelFormatEAC_R11Unorm;
                case TextureFormat::EAC_R11_SIGNED: return MTLPixelFormatEAC_R11Snorm;
                case TextureFormat::EAC_RG11: return MTLPixelFormatEAC_RG11Unorm;
                case TextureFormat::EAC_RG11_SIGNED: return MTLPixelFormatEAC_RG11Snorm;
                case TextureFormat::ETC2_RGB8: return MTLPixelFormatETC2_RGB8;
                case TextureFormat::ETC2_SRGB8: return MTLPixelFormatETC2_RGB8_sRGB;
                case TextureFormat::ETC2_RGB8_A1: return MTLPixelFormatETC2_RGB8A1;
                case TextureFormat::ETC2_SRGB8_A1: return MTLPixelFormatETC2_RGB8A1_sRGB;
                case TextureFormat::ETC2_EAC_RGBA8: return MTLPixelFormatEAC_RGBA8;
                case TextureFormat::ETC2_EAC_SRGBA8: return MTLPixelFormatEAC_RGBA8_sRGB;
                default: break;
            }
        }
    }

    // DXT (BC) formats are only available on macOS desktop.
    // See https://en.wikipedia.org/wiki/S3_Texture_Compression#S3TC_format_comparison
#if TARGET_OS_OSX
    if (context->highestSupportedGpuFamily.mac >= 1) {
        switch (format) {
            case TextureFormat::DXT1_RGBA: return MTLPixelFormatBC1_RGBA;
            case TextureFormat::DXT1_SRGBA: return MTLPixelFormatBC1_RGBA_sRGB;
            case TextureFormat::DXT3_RGBA: return MTLPixelFormatBC2_RGBA;
            case TextureFormat::DXT3_SRGBA: return MTLPixelFormatBC2_RGBA_sRGB;
            case TextureFormat::DXT5_RGBA: return MTLPixelFormatBC3_RGBA;
            case TextureFormat::DXT5_SRGBA: return MTLPixelFormatBC3_RGBA_sRGB;

            case TextureFormat::DXT1_RGB: return MTLPixelFormatInvalid;
            case TextureFormat::DXT1_SRGB: return MTLPixelFormatInvalid;

            case TextureFormat::RED_RGTC1:              return MTLPixelFormatBC4_RUnorm;
            case TextureFormat::SIGNED_RED_RGTC1:       return MTLPixelFormatBC4_RSnorm;
            case TextureFormat::RED_GREEN_RGTC2:        return MTLPixelFormatBC5_RGUnorm;
            case TextureFormat::SIGNED_RED_GREEN_RGTC2: return MTLPixelFormatBC5_RGSnorm;

            case TextureFormat::RGB_BPTC_SIGNED_FLOAT:      return MTLPixelFormatBC6H_RGBFloat;
            case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:    return MTLPixelFormatBC6H_RGBUfloat;
            case TextureFormat::RGBA_BPTC_UNORM:            return MTLPixelFormatBC7_RGBAUnorm;
            case TextureFormat::SRGB_ALPHA_BPTC_UNORM:      return MTLPixelFormatBC7_RGBAUnorm_sRGB;
            default: break;
        }
    }
#endif

    return MTLPixelFormatInvalid;
}

} // namespace backend
} // namespace filament
