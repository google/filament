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

#include <utils/Panic.h>

#include "private/backend/BackendUtils.h"

namespace filament {
namespace backend {

void createSemaphore(VkDevice device, VkSemaphore *semaphore) {
    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = vkCreateSemaphore(device, &createInfo, nullptr, semaphore);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateSemaphore error.");
}

VkFormat getVkFormat(ElementType type, bool normalized) {
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
            case ElementType::BYTE3: return VK_FORMAT_R8G8B8_SNORM;
            case ElementType::UBYTE3: return VK_FORMAT_R8G8B8_UNORM;
            case ElementType::SHORT3: return VK_FORMAT_R16G16B16_SNORM;
            case ElementType::USHORT3: return VK_FORMAT_R16G16B16_UNORM;
            // Four Component Types
            case ElementType::BYTE4: return VK_FORMAT_R8G8B8A8_SNORM;
            case ElementType::UBYTE4: return VK_FORMAT_R8G8B8A8_UNORM;
            case ElementType::SHORT4: return VK_FORMAT_R16G16B16A16_SNORM;
            case ElementType::USHORT4: return VK_FORMAT_R16G16B16A16_UNORM;
            default:
                ASSERT_POSTCONDITION(false, "Normalized format does not exist.");
                return VK_FORMAT_UNDEFINED;
        }
    }
    switch (type) {
        // Single Component Types
        case ElementType::BYTE: return VK_FORMAT_R8_SINT;
        case ElementType::UBYTE: return VK_FORMAT_R8_UINT;
        case ElementType::SHORT: return VK_FORMAT_R16_SINT;
        case ElementType::USHORT: return VK_FORMAT_R16_UINT;
        case ElementType::HALF: return VK_FORMAT_R16_SFLOAT;
        case ElementType::INT: return VK_FORMAT_R32_SINT;
        case ElementType::UINT: return VK_FORMAT_R32_UINT;
        case ElementType::FLOAT: return VK_FORMAT_R32_SFLOAT;
        // Two Component Types
        case ElementType::BYTE2: return VK_FORMAT_R8G8_SINT;
        case ElementType::UBYTE2: return VK_FORMAT_R8G8_UINT;
        case ElementType::SHORT2: return VK_FORMAT_R16G16_SINT;
        case ElementType::USHORT2: return VK_FORMAT_R16G16_UINT;
        case ElementType::HALF2: return VK_FORMAT_R16G16_SFLOAT;
        case ElementType::FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
        // Three Component Types
        case ElementType::BYTE3: return VK_FORMAT_R8G8B8_SINT;
        case ElementType::UBYTE3: return VK_FORMAT_R8G8B8_UINT;
        case ElementType::SHORT3: return VK_FORMAT_R16G16B16_SINT;
        case ElementType::USHORT3: return VK_FORMAT_R16G16B16_UINT;
        case ElementType::HALF3: return VK_FORMAT_R16G16B16_SFLOAT;
        case ElementType::FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
        // Four Component Types
        case ElementType::BYTE4: return VK_FORMAT_R8G8B8A8_SINT;
        case ElementType::UBYTE4: return VK_FORMAT_R8G8B8A8_UINT;
        case ElementType::SHORT4: return VK_FORMAT_R16G16B16A16_SINT;
        case ElementType::USHORT4: return VK_FORMAT_R16G16B16A16_UINT;
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

        case TextureFormat::DEPTH24:
            return VK_FORMAT_UNDEFINED;

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
        case TextureFormat::RGB10_A2:          return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
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

// See also FTexture::computeTextureDataSize, which takes a public-facing Texture format rather
// than a driver-level Texture format, and can account for a specified byte alignment.
uint32_t getBytesPerPixel(TextureFormat format) {
    return (uint32_t) getFormatSize(format);
}

VkCompareOp getCompareOp(SamplerCompareFunc func) {
    using Compare = backend::SamplerCompareFunc;
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
    using BlendFunction = filament::backend::BlendFunction;
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
    using CullingMode = filament::backend::CullingMode;
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

} // namespace filament
} // namespace backend
