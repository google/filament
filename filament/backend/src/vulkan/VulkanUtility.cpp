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

#include "VulkanUtility.h"

#include <utils/algorithm.h>
#include <utils/debug.h>
#include <utils/Panic.h>

#include "private/backend/BackendUtils.h"

using namespace bluevk;

namespace filament::backend {

VkFormat getVkFormat(ElementType type, bool normalized, bool integer) {
    using ElementType = ElementType;
    if (normalized) {
        switch (type) {
            // Single Component Types
            case ElementType::BYTE: return VK_FORMAT_R8_SNORM;
            case ElementType::UBYTE: return VK_FORMAT_R8_UNORM;
            case ElementType::SHORT: return VK_FORMAT_R16_SNORM;
            case ElementType::USHORT: return VK_FORMAT_R16_UNORM;
            // Two Component Types
            case ElementType::BYTE2: return VK_FORMAT_R8G8_SNORM;
            case ElementType::UBYTE2: return VK_FORMAT_R8G8_UNORM;
            case ElementType::SHORT2: return VK_FORMAT_R16G16_SNORM;
            case ElementType::USHORT2: return VK_FORMAT_R16G16_UNORM;
            // Three Component Types
            case ElementType::BYTE3: return VK_FORMAT_R8G8B8_SNORM;      // NOT MINSPEC
            case ElementType::UBYTE3: return VK_FORMAT_R8G8B8_UNORM;     // NOT MINSPEC
            case ElementType::SHORT3: return VK_FORMAT_R16G16B16_SNORM;  // NOT MINSPEC
            case ElementType::USHORT3: return VK_FORMAT_R16G16B16_UNORM; // NOT MINSPEC
            // Four Component Types
            case ElementType::BYTE4: return VK_FORMAT_R8G8B8A8_SNORM;
            case ElementType::UBYTE4: return VK_FORMAT_R8G8B8A8_UNORM;
            case ElementType::SHORT4: return VK_FORMAT_R16G16B16A16_SNORM;
            case ElementType::USHORT4: return VK_FORMAT_R16G16B16A16_UNORM;
            default:
                FILAMENT_CHECK_POSTCONDITION(false) << "Normalized format does not exist.";
                return VK_FORMAT_UNDEFINED;
        }
    }
    switch (type) {
        // Single Component Types
        case ElementType::BYTE: return integer ? VK_FORMAT_R8_SINT : VK_FORMAT_R8_SSCALED;
        case ElementType::UBYTE: return integer ? VK_FORMAT_R8_UINT : VK_FORMAT_R8_USCALED;
        case ElementType::SHORT: return integer ? VK_FORMAT_R16_SINT : VK_FORMAT_R16_SSCALED;
        case ElementType::USHORT: return integer ? VK_FORMAT_R16_UINT : VK_FORMAT_R16_USCALED;
        case ElementType::HALF: return VK_FORMAT_R16_SFLOAT;
        case ElementType::INT: return VK_FORMAT_R32_SINT;
        case ElementType::UINT: return VK_FORMAT_R32_UINT;
        case ElementType::FLOAT: return VK_FORMAT_R32_SFLOAT;
        // Two Component Types
        case ElementType::BYTE2: return integer ? VK_FORMAT_R8G8_SINT : VK_FORMAT_R8G8_SSCALED;
        case ElementType::UBYTE2: return integer ? VK_FORMAT_R8G8_UINT : VK_FORMAT_R8G8_USCALED;
        case ElementType::SHORT2: return integer ? VK_FORMAT_R16G16_SINT : VK_FORMAT_R16G16_SSCALED;
        case ElementType::USHORT2: return integer ? VK_FORMAT_R16G16_UINT : VK_FORMAT_R16G16_USCALED;
        case ElementType::HALF2: return VK_FORMAT_R16G16_SFLOAT;
        case ElementType::FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
        // Three Component Types
        case ElementType::BYTE3: return VK_FORMAT_R8G8B8_SINT;      // NOT MINSPEC
        case ElementType::UBYTE3: return VK_FORMAT_R8G8B8_UINT;     // NOT MINSPEC
        case ElementType::SHORT3: return VK_FORMAT_R16G16B16_SINT;  // NOT MINSPEC
        case ElementType::USHORT3: return VK_FORMAT_R16G16B16_UINT; // NOT MINSPEC
        case ElementType::HALF3: return VK_FORMAT_R16G16B16_SFLOAT; // NOT MINSPEC
        case ElementType::FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
        // Four Component Types
        case ElementType::BYTE4: return integer ? VK_FORMAT_R8G8B8A8_SINT : VK_FORMAT_R8G8B8A8_SSCALED;
        case ElementType::UBYTE4: return integer ? VK_FORMAT_R8G8B8A8_UINT : VK_FORMAT_R8G8B8A8_USCALED;
        case ElementType::SHORT4: return integer ? VK_FORMAT_R16G16B16A16_SINT : VK_FORMAT_R16G16B16A16_SSCALED;
        case ElementType::USHORT4: return integer ? VK_FORMAT_R16G16B16A16_UINT : VK_FORMAT_R16G16B16A16_USCALED;
        case ElementType::HALF4: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case ElementType::FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    return VK_FORMAT_UNDEFINED;
}

VkFormat getVkFormat(TextureFormat format) {
    using TextureFormat = TextureFormat;
    switch (format) {
        // 8 bits per element.
        case TextureFormat::R8:                return VK_FORMAT_R8_UNORM;
        case TextureFormat::R8_SNORM:          return VK_FORMAT_R8_SNORM;
        case TextureFormat::R8UI:              return VK_FORMAT_R8_UINT;
        case TextureFormat::R8I:               return VK_FORMAT_R8_SINT;
        case TextureFormat::STENCIL8:          return VK_FORMAT_S8_UINT;

        // 16 bits per element.
        case TextureFormat::R16F:              return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::R16UI:             return VK_FORMAT_R16_UINT;
        case TextureFormat::R16I:              return VK_FORMAT_R16_SINT;
        case TextureFormat::RG8:               return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RG8_SNORM:         return VK_FORMAT_R8G8_SNORM;
        case TextureFormat::RG8UI:             return VK_FORMAT_R8G8_UINT;
        case TextureFormat::RG8I:              return VK_FORMAT_R8G8_SINT;
        case TextureFormat::RGB565:            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case TextureFormat::RGB5_A1:           return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        case TextureFormat::RGBA4:             return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case TextureFormat::DEPTH16:           return VK_FORMAT_D16_UNORM;

        // 24 bits per element. In practice, very few GPU vendors support these. So, we simply
        // always reshape them into 32-bit formats.
        case TextureFormat::RGB8:              return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::SRGB8:             return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::RGB8_SNORM:        return VK_FORMAT_R8G8B8A8_SNORM;
        case TextureFormat::RGB8UI:            return VK_FORMAT_R8G8B8A8_UINT;
        case TextureFormat::RGB8I:             return VK_FORMAT_R8G8B8A8_SINT;

        // A 32-bit format but 8 bits are unused.
        case TextureFormat::DEPTH24:           return VK_FORMAT_X8_D24_UNORM_PACK32;

        // 32 bits per element.
        case TextureFormat::R32F:              return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::R32UI:             return VK_FORMAT_R32_UINT;
        case TextureFormat::R32I:              return VK_FORMAT_R32_SINT;
        case TextureFormat::RG16F:             return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RG16UI:            return VK_FORMAT_R16G16_UINT;
        case TextureFormat::RG16I:             return VK_FORMAT_R16G16_SINT;
        case TextureFormat::R11F_G11F_B10F:    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case TextureFormat::RGB9_E5:           return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case TextureFormat::RGBA8:             return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::SRGB8_A8:          return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::RGBA8_SNORM:       return VK_FORMAT_R8G8B8A8_SNORM;
        case TextureFormat::RGB10_A2:          return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case TextureFormat::RGBA8UI:           return VK_FORMAT_R8G8B8A8_UINT;
        case TextureFormat::RGBA8I:            return VK_FORMAT_R8G8B8A8_SINT;
        case TextureFormat::DEPTH32F:          return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::DEPTH24_STENCIL8:  return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::DEPTH32F_STENCIL8: return VK_FORMAT_D32_SFLOAT_S8_UINT;

        // 48 bits per element. In practice, very few GPU vendors support these. So, we simply
        // always reshape them into 64-bit formats.
        case TextureFormat::RGB16F:            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RGB16UI:           return VK_FORMAT_R16G16B16A16_UINT;
        case TextureFormat::RGB16I:            return VK_FORMAT_R16G16B16A16_SINT;

        // 64 bits per element.
        case TextureFormat::RG32F:             return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::RG32UI:            return VK_FORMAT_R32G32_UINT;
        case TextureFormat::RG32I:             return VK_FORMAT_R32G32_SINT;
        case TextureFormat::RGBA16F:           return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RGBA16UI:          return VK_FORMAT_R16G16B16A16_UINT;
        case TextureFormat::RGBA16I:           return VK_FORMAT_R16G16B16A16_SINT;

        // 96-bits per element.
        case TextureFormat::RGB32F:            return VK_FORMAT_R32G32B32_SFLOAT;
        case TextureFormat::RGB32UI:           return VK_FORMAT_R32G32B32_UINT;
        case TextureFormat::RGB32I:            return VK_FORMAT_R32G32B32_SINT;

        // 128-bits per element.
        case TextureFormat::RGBA32F:           return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::RGBA32UI:          return VK_FORMAT_R32G32B32A32_UINT;
        case TextureFormat::RGBA32I:           return VK_FORMAT_R32G32B32A32_SINT;

        // Compressed textures.
        case TextureFormat::DXT1_RGB:          return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case TextureFormat::DXT1_SRGB:         return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        case TextureFormat::DXT1_RGBA:         return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case TextureFormat::DXT1_SRGBA:        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case TextureFormat::DXT3_RGBA:         return VK_FORMAT_BC2_UNORM_BLOCK;
        case TextureFormat::DXT3_SRGBA:        return VK_FORMAT_BC2_SRGB_BLOCK;
        case TextureFormat::DXT5_RGBA:         return VK_FORMAT_BC3_UNORM_BLOCK;
        case TextureFormat::DXT5_SRGBA:        return VK_FORMAT_BC3_SRGB_BLOCK;

        case TextureFormat::RED_RGTC1:              return VK_FORMAT_BC4_UNORM_BLOCK;
        case TextureFormat::SIGNED_RED_RGTC1:       return VK_FORMAT_BC4_SNORM_BLOCK;
        case TextureFormat::RED_GREEN_RGTC2:        return VK_FORMAT_BC5_UNORM_BLOCK;
        case TextureFormat::SIGNED_RED_GREEN_RGTC2: return VK_FORMAT_BC5_SNORM_BLOCK;

        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:      return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:    return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case TextureFormat::RGBA_BPTC_UNORM:            return VK_FORMAT_BC7_UNORM_BLOCK;
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:      return VK_FORMAT_BC7_SRGB_BLOCK;

        case TextureFormat::RGBA_ASTC_4x4:     return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_5x4:     return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_5x5:     return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_6x5:     return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_6x6:     return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_8x5:     return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_8x6:     return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_8x8:     return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_10x5:    return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_10x6:    return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_10x8:    return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_10x10:   return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_12x10:   return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case TextureFormat::RGBA_ASTC_12x12:   return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;

        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:   return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:   return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:   return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:   return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:   return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:   return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:   return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:   return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:  return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:  return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:  return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10: return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10: return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12: return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

        case TextureFormat::ETC2_RGB8:         return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case TextureFormat::ETC2_SRGB8:        return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case TextureFormat::ETC2_RGB8_A1:      return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case TextureFormat::ETC2_SRGB8_A1:     return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;

        case TextureFormat::ETC2_EAC_RGBA8:    return VK_FORMAT_UNDEFINED;
        case TextureFormat::ETC2_EAC_SRGBA8:   return VK_FORMAT_UNDEFINED;

        case TextureFormat::EAC_R11:           return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case TextureFormat::EAC_R11_SIGNED:    return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case TextureFormat::EAC_RG11:          return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case TextureFormat::EAC_RG11_SIGNED:   return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

        default:
            return VK_FORMAT_UNDEFINED;
    }
}

// Converts PixelBufferDescriptor format + type pair into a VkFormat.
//
// NOTE: This function only returns formats that support VK_FORMAT_FEATURE_BLIT_SRC_BIT as per
// "Required Format Support" in the Vulkan specification. These are the only formats that can be
// used for format conversion. If the requested format does not support this feature, then
// VK_FORMAT_UNDEFINED is returned.
VkFormat getVkFormat(PixelDataFormat format, PixelDataType type) {
    if (type == PixelDataType::USHORT_565) return VK_FORMAT_R5G6B5_UNORM_PACK16;
    if (type == PixelDataType::UINT_2_10_10_10_REV) return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    if (type == PixelDataType::UINT_10F_11F_11F_REV) return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

    #define CONVERT(FORMAT, TYPE, VK) \
    if (PixelDataFormat::FORMAT == format && PixelDataType::TYPE == type)  return VK_FORMAT_ ## VK;

    CONVERT(R, UBYTE, R8_UNORM);
    CONVERT(R, BYTE, R8_SNORM);
    CONVERT(R_INTEGER, UBYTE, R8_UINT);
    CONVERT(R_INTEGER, BYTE, R8_SINT);
    CONVERT(RG, UBYTE, R8G8_UNORM);
    CONVERT(RG, BYTE, R8G8_SNORM);
    CONVERT(RG_INTEGER, UBYTE, R8G8_UINT);
    CONVERT(RG_INTEGER, BYTE, R8G8_SINT);
    CONVERT(RGBA, UBYTE, R8G8B8A8_UNORM);
    CONVERT(RGBA, BYTE, R8G8B8A8_SNORM);
    CONVERT(RGBA_INTEGER, UBYTE, R8G8B8A8_UINT);
    CONVERT(RGBA_INTEGER, BYTE, R8G8B8A8_SINT);
    CONVERT(R_INTEGER, USHORT, R16_UINT);
    CONVERT(R_INTEGER, SHORT, R16_SINT);
    CONVERT(R, HALF, R16_SFLOAT);
    CONVERT(RG_INTEGER, USHORT, R16G16_UINT);
    CONVERT(RG_INTEGER, SHORT, R16G16_SINT);
    CONVERT(RG, HALF, R16G16_SFLOAT);
    CONVERT(RGBA_INTEGER, USHORT, R16G16B16A16_UINT);
    CONVERT(RGBA_INTEGER, SHORT, R16G16B16A16_SINT);
    CONVERT(RGBA, HALF, R16G16B16A16_SFLOAT);
    CONVERT(R_INTEGER, UINT, R32_UINT);
    CONVERT(R_INTEGER, INT, R32_SINT);
    CONVERT(R, FLOAT, R32_SFLOAT);
    CONVERT(RG_INTEGER, UINT, R32G32_UINT);
    CONVERT(RG_INTEGER, INT, R32G32_SINT);
    CONVERT(RG, FLOAT, R32G32_SFLOAT);
    CONVERT(RGBA_INTEGER, UINT, R32G32B32A32_UINT);
    CONVERT(RGBA_INTEGER, INT, R32G32B32A32_SINT);
    CONVERT(RGBA, FLOAT, R32G32B32A32_SFLOAT);
    #undef CONVERT

    return VK_FORMAT_UNDEFINED;
}

// Converts an SRGB Vulkan format identifier into its corresponding canonical UNORM format. If the
// given format is not an SRGB format, it is returned unmodified. This function is useful when
// determining if blit-based conversion is needed, since SRGB is orthogonal to the actual bit layout
// of color components.
VkFormat getVkFormatLinear(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_SRGB: return VK_FORMAT_R8_UNORM;
        case VK_FORMAT_R8G8_SRGB: return VK_FORMAT_R8G8_UNORM;
        case VK_FORMAT_R8G8B8_SRGB: return VK_FORMAT_R8G8B8_UNORM;
        case VK_FORMAT_B8G8R8_SRGB: return VK_FORMAT_B8G8R8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_UNORM;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case VK_FORMAT_BC2_SRGB_BLOCK: return VK_FORMAT_BC2_UNORM_BLOCK;
        case VK_FORMAT_BC3_SRGB_BLOCK: return VK_FORMAT_BC3_UNORM_BLOCK;
        case VK_FORMAT_BC7_SRGB_BLOCK: return VK_FORMAT_BC7_UNORM_BLOCK;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
        default: return format;
    }
    return format;
}

// See also FTexture::computeTextureDataSize, which takes a public-facing Texture format rather
// than a driver-level Texture format, and can account for a specified byte alignment.
uint32_t getBytesPerPixel(TextureFormat format) {
    return (uint32_t) getFormatSize(format);
}

VkCompareOp getCompareOp(SamplerCompareFunc func) {
    using Compare = SamplerCompareFunc;
    switch (func) {
        case Compare::LE: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Compare::GE: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Compare::L:  return VK_COMPARE_OP_LESS;
        case Compare::G:  return VK_COMPARE_OP_GREATER;
        case Compare::E:  return VK_COMPARE_OP_EQUAL;
        case Compare::NE: return VK_COMPARE_OP_NOT_EQUAL;
        case Compare::A:  return VK_COMPARE_OP_ALWAYS;
        case Compare::N:  return VK_COMPARE_OP_NEVER;
    }
}

VkBlendFactor getBlendFactor(BlendFunction mode) {
    switch (mode) {
        case BlendFunction::ZERO:                  return VK_BLEND_FACTOR_ZERO;
        case BlendFunction::ONE:                   return VK_BLEND_FACTOR_ONE;
        case BlendFunction::SRC_COLOR:             return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFunction::ONE_MINUS_SRC_COLOR:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFunction::DST_COLOR:             return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFunction::ONE_MINUS_DST_COLOR:   return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFunction::SRC_ALPHA:             return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFunction::ONE_MINUS_SRC_ALPHA:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFunction::DST_ALPHA:             return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFunction::ONE_MINUS_DST_ALPHA:   return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFunction::SRC_ALPHA_SATURATE:    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    }
}

VkCullModeFlags getCullMode(CullingMode mode) {
    switch (mode) {
        case CullingMode::NONE:           return VK_CULL_MODE_NONE;
        case CullingMode::FRONT:          return VK_CULL_MODE_FRONT_BIT;
        case CullingMode::BACK:           return VK_CULL_MODE_BACK_BIT;
        case CullingMode::FRONT_AND_BACK: return VK_CULL_MODE_FRONT_AND_BACK;
    }
}

VkFrontFace getFrontFace(bool inverseFrontFaces) {
    return inverseFrontFaces ?
            VkFrontFace::VK_FRONT_FACE_CLOCKWISE : VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

PixelDataType getComponentType(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT: return PixelDataType::UBYTE;
        case VK_FORMAT_R8_SINT: return PixelDataType::BYTE;
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT: return PixelDataType::UBYTE;
        case VK_FORMAT_R8G8_SINT: return PixelDataType::BYTE;
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT: return PixelDataType::UBYTE;
        case VK_FORMAT_R8G8B8_SINT: return PixelDataType::BYTE;
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM: return PixelDataType::UBYTE;
        case VK_FORMAT_B8G8R8_SNORM: return PixelDataType::BYTE;
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT: return PixelDataType::UBYTE;
        case VK_FORMAT_B8G8R8_SINT: return PixelDataType::BYTE;
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT: return PixelDataType::UBYTE;
        case VK_FORMAT_R8G8B8A8_SINT: return PixelDataType::BYTE;
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT: return PixelDataType::UBYTE;
        case VK_FORMAT_B8G8R8A8_SINT: return PixelDataType::BYTE;
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32: return PixelDataType::UBYTE;
        case VK_FORMAT_A8B8G8R8_SINT_PACK32: return PixelDataType::BYTE;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return PixelDataType::UBYTE;
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT: return PixelDataType::USHORT;
        case VK_FORMAT_R16_SINT: return PixelDataType::SHORT;
        case VK_FORMAT_R16_SFLOAT: return PixelDataType::HALF;
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT: return PixelDataType::USHORT;
        case VK_FORMAT_R16G16_SINT: return PixelDataType::SHORT;
        case VK_FORMAT_R16G16_SFLOAT: return PixelDataType::HALF;
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT: return PixelDataType::USHORT;
        case VK_FORMAT_R16G16B16_SINT: return PixelDataType::SHORT;
        case VK_FORMAT_R16G16B16_SFLOAT: return PixelDataType::HALF;
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT: return PixelDataType::USHORT;
        case VK_FORMAT_R16G16B16A16_SINT: return PixelDataType::SHORT;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return PixelDataType::HALF;
        case VK_FORMAT_R32_UINT: return PixelDataType::UINT;
        case VK_FORMAT_R32_SINT: return PixelDataType::INT;
        case VK_FORMAT_R32_SFLOAT: return PixelDataType::FLOAT;
        case VK_FORMAT_R32G32_UINT: return PixelDataType::UINT;
        case VK_FORMAT_R32G32_SINT: return PixelDataType::INT;
        case VK_FORMAT_R32G32_SFLOAT: return PixelDataType::FLOAT;
        case VK_FORMAT_R32G32B32_UINT: return PixelDataType::UINT;
        case VK_FORMAT_R32G32B32_SINT: return PixelDataType::INT;
        case VK_FORMAT_R32G32B32_SFLOAT: return PixelDataType::FLOAT;
        case VK_FORMAT_R32G32B32A32_UINT: return PixelDataType::UINT;
        case VK_FORMAT_R32G32B32A32_SINT: return PixelDataType::INT;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return PixelDataType::FLOAT;
        default: assert_invariant(false && "Unknown data type, conversion is not supported.");
    }
    return {};
}

uint32_t getComponentCount(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
            return 1;

        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
            return 2;

        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
            return 3;

        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return 4;
        default: assert_invariant(false && "Unknown data type, conversion is not supported.");
    }
    return {};
}

VkComponentMapping getSwizzleMap(TextureSwizzle const swizzle[4]) {
    VkComponentMapping map;
    VkComponentSwizzle* dst = &map.r;
    for (int i = 0; i < 4; ++i, ++dst) {
        switch (swizzle[i]) {
            case TextureSwizzle::SUBSTITUTE_ZERO: *dst = VK_COMPONENT_SWIZZLE_ZERO; break;
            case TextureSwizzle::SUBSTITUTE_ONE: *dst = VK_COMPONENT_SWIZZLE_ONE; break;
            // NOTE: In some cases, IDENTITY is equivalent to one of the other enums, in which
            // case we choose IDENTITY under the premise that it could be more efficient, or
            // allow for more state sharing. In practice this probably has no impact.
            case TextureSwizzle::CHANNEL_0:
                *dst = i == 0 ? VK_COMPONENT_SWIZZLE_IDENTITY : VK_COMPONENT_SWIZZLE_R;
                break;
            case TextureSwizzle::CHANNEL_1:
                *dst = i == 1 ? VK_COMPONENT_SWIZZLE_IDENTITY : VK_COMPONENT_SWIZZLE_G;
                break;
            case TextureSwizzle::CHANNEL_2:
                *dst = i == 2 ? VK_COMPONENT_SWIZZLE_IDENTITY : VK_COMPONENT_SWIZZLE_B;
                break;
            case TextureSwizzle::CHANNEL_3:
                *dst = i == 3 ? VK_COMPONENT_SWIZZLE_IDENTITY : VK_COMPONENT_SWIZZLE_A;
                break;
        }
    }
    return map;
}

VkShaderStageFlags getShaderStageFlags(ShaderStageFlags stageFlags) {
    VkShaderStageFlags flags = 0x0;
    if (any(stageFlags & ShaderStageFlags::VERTEX))     flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (any(stageFlags & ShaderStageFlags::FRAGMENT))   flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    return flags;
}

VkSamplerYcbcrModelConversion getYcbcrModelConversion(
    SamplerYcbcrModelConversion model) {
    switch (model) {
        case SamplerYcbcrModelConversion::RGB_IDENTITY:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
        case SamplerYcbcrModelConversion::YCBCR_IDENTITY:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY;
        case SamplerYcbcrModelConversion::YCBCR_709:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
        case SamplerYcbcrModelConversion::YCBCR_601:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
        case SamplerYcbcrModelConversion::YCBCR_2020:
            return VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020;
        default:
            assert_invariant(false &&
                             "Unknown data type, conversion is not supported.");
    }
}

VkSamplerYcbcrRange getYcbcrRange(SamplerYcbcrRange range) {
    switch (range) {
        case SamplerYcbcrRange::ITU_FULL:
            return VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
        case SamplerYcbcrRange::ITU_NARROW:
            return VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
        default:
            assert_invariant(false &&
                             "Unknown data type, conversion is not supported.");
    }
}

VkChromaLocation getChromaLocation(ChromaLocation loc) {
    switch (loc) {
        case ChromaLocation::COSITED_EVEN:
            return VK_CHROMA_LOCATION_COSITED_EVEN;
        case ChromaLocation::MIDPOINT:
            return VK_CHROMA_LOCATION_MIDPOINT;
        default:
            assert_invariant(false &&
                             "Unknown data type, conversion is not supported.");
    }
}

bool equivalent(const VkRect2D& a, const VkRect2D& b) {
    // These are all integers so there's no need for an epsilon.
    return a.extent.width == b.extent.width && a.extent.height == b.extent.height &&
            a.offset.x == b.offset.x && a.offset.y == b.offset.y;
}

bool equivalent(const VkExtent2D& a, const VkExtent2D& b) {
    return a.height == b.height && a.width == b.width;
}

VkImageAspectFlags getImageAspect(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

bool isVkDepthFormat(VkFormat format) {
    return (getImageAspect(format) & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
}

bool isVkStencilFormat(VkFormat format) {
    return (getImageAspect(format) & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;
}

static uint32_t mostSignificantBit(uint32_t x) { return 1ul << (31ul - utils::clz(x)); }

uint8_t reduceSampleCount(uint8_t sampleCount, VkSampleCountFlags mask) {
    if (sampleCount & mask) {
        return sampleCount;
    }
    return mostSignificantBit((sampleCount - 1) & mask);
}

} // namespace filament::backend
