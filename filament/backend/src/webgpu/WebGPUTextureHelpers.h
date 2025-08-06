/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUTEXTUREHELPERS_H
#define TNT_FILAMENT_BACKEND_WEBGPUTEXTUREHELPERS_H

#include "webgpu/WebGPUConstants.h"

#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <string_view>

namespace filament::backend {

[[nodiscard]] constexpr bool hasStencil(const wgpu::TextureFormat textureFormat) {
    return textureFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
           textureFormat == wgpu::TextureFormat::Depth32FloatStencil8 ||
           textureFormat == wgpu::TextureFormat::Stencil8;
}

[[nodiscard]] constexpr bool hasDepth(const wgpu::TextureFormat textureFormat) {
    return textureFormat == wgpu::TextureFormat::Depth16Unorm ||
           textureFormat == wgpu::TextureFormat::Depth32Float ||
           textureFormat == wgpu::TextureFormat::Depth24Plus ||
           textureFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
           textureFormat == wgpu::TextureFormat::Depth32FloatStencil8;
}

[[nodiscard]] constexpr std::string_view toString(const PixelDataFormat format) {
    switch (format) {
        case PixelDataFormat::R:               return "R";
        case PixelDataFormat::R_INTEGER:       return "R_INTEGER";
        case PixelDataFormat::RG:              return "RG";
        case PixelDataFormat::RG_INTEGER:      return "RG_INTEGER";
        case PixelDataFormat::RGB:             return "RGB";
        case PixelDataFormat::RGB_INTEGER:     return "RGB_INTEGER";
        case PixelDataFormat::RGBA:            return "RGBA";
        case PixelDataFormat::RGBA_INTEGER:    return "RGBA_INTEGER";
        case PixelDataFormat::UNUSED:          return "UNUSED";
        case PixelDataFormat::DEPTH_COMPONENT: return "DEPTH_COMPONENT";
        case PixelDataFormat::DEPTH_STENCIL:   return "DEPTH_STENCIL";
        case PixelDataFormat::ALPHA:           return "ALPHA";
    };
}

[[nodiscard]] constexpr std::string_view toString(const PixelDataType type) {
    switch (type) {
        case PixelDataType::UBYTE:                 return "UBYTE";
        case PixelDataType::BYTE:                  return "BYTE";
        case PixelDataType::USHORT:                return "USHORT";
        case PixelDataType::SHORT:                 return "SHORT";
        case PixelDataType::UINT:                  return "UINT";
        case PixelDataType::INT:                   return "INT";
        case PixelDataType::HALF:                  return "HALF";
        case PixelDataType::FLOAT:                 return "FLOAT";
        case PixelDataType::COMPRESSED:            return "COMPRESSED";
        case PixelDataType::UINT_10F_11F_11F_REV:  return "UINT_10F_11F_11F_REV";
        case PixelDataType::USHORT_565:            return "USHORT_565";
        case PixelDataType::UINT_2_10_10_10_REV:   return "UINT_2_10_10_10_REV";
    };
}

[[nodiscard]] constexpr wgpu::TextureFormat toWebGPUFormat(const PixelDataFormat format,
        const PixelDataType type) {
    if (type == PixelDataType::UINT_2_10_10_10_REV)                                 return wgpu::TextureFormat::RGB10A2Unorm;
    if (type == PixelDataType::UINT_10F_11F_11F_REV)                                return wgpu::TextureFormat::RG11B10Ufloat;
    if (PixelDataFormat::R == format && PixelDataType::UBYTE == type)               return wgpu::TextureFormat::R8Unorm;
    if (PixelDataFormat::R == format && PixelDataType::BYTE == type)                return wgpu::TextureFormat::R8Snorm;
    if (PixelDataFormat::R_INTEGER == format && PixelDataType::UBYTE == type)       return wgpu::TextureFormat::R8Uint;
    if (PixelDataFormat::R_INTEGER == format && PixelDataType::BYTE == type)        return wgpu::TextureFormat::R8Sint;
    if (PixelDataFormat::RG == format && PixelDataType::UBYTE == type)              return wgpu::TextureFormat::RG8Unorm;
    if (PixelDataFormat::RG == format && PixelDataType::BYTE == type)               return wgpu::TextureFormat::RG8Snorm;
    if (PixelDataFormat::RG_INTEGER == format && PixelDataType::UBYTE == type)      return wgpu::TextureFormat::RG8Uint;
    if (PixelDataFormat::RG_INTEGER == format && PixelDataType::BYTE == type)       return wgpu::TextureFormat::RG8Sint;
    if (PixelDataFormat::RGBA == format && PixelDataType::UBYTE == type)            return wgpu::TextureFormat::RGBA8Unorm;
    if (PixelDataFormat::RGBA == format && PixelDataType::BYTE == type)             return wgpu::TextureFormat::RGBA8Snorm;
    if (PixelDataFormat::RGBA_INTEGER == format && PixelDataType::UBYTE == type)    return wgpu::TextureFormat::RGBA8Uint;
    if (PixelDataFormat::RGBA_INTEGER == format && PixelDataType::BYTE == type)     return wgpu::TextureFormat::RGBA8Sint;
    if (PixelDataFormat::R_INTEGER == format && PixelDataType::USHORT == type)      return wgpu::TextureFormat::R16Uint;
    if (PixelDataFormat::R_INTEGER == format && PixelDataType::SHORT == type)       return wgpu::TextureFormat::R16Sint;
    if (PixelDataFormat::R == format && PixelDataType::HALF == type)                return wgpu::TextureFormat::R16Float;
    if (PixelDataFormat::RG_INTEGER == format && PixelDataType::USHORT == type)     return wgpu::TextureFormat::RG16Uint;
    if (PixelDataFormat::RG_INTEGER == format && PixelDataType::SHORT == type)      return wgpu::TextureFormat::RG16Sint;
    if (PixelDataFormat::RG == format && PixelDataType::HALF == type)               return wgpu::TextureFormat::RG16Float;
    if (PixelDataFormat::RGBA_INTEGER == format && PixelDataType::USHORT == type)   return wgpu::TextureFormat::RGBA16Uint;
    if (PixelDataFormat::RGBA_INTEGER == format && PixelDataType::SHORT == type)    return wgpu::TextureFormat::RGBA16Sint;
    if (PixelDataFormat::RGBA == format && PixelDataType::HALF == type)             return wgpu::TextureFormat::RGBA16Float;
    if (PixelDataFormat::R_INTEGER == format && PixelDataType::UINT == type)        return wgpu::TextureFormat::R32Uint;
    if (PixelDataFormat::R_INTEGER == format && PixelDataType::INT == type)         return wgpu::TextureFormat::R32Sint;
    if (PixelDataFormat::R == format && PixelDataType::FLOAT == type)               return wgpu::TextureFormat::R32Float;
    if (PixelDataFormat::RG_INTEGER == format && PixelDataType::UINT == type)       return wgpu::TextureFormat::RG32Uint;
    if (PixelDataFormat::RG_INTEGER == format && PixelDataType::INT == type)        return wgpu::TextureFormat::RG32Sint;
    if (PixelDataFormat::RG == format && PixelDataType::FLOAT == type)              return wgpu::TextureFormat::RG32Float;
    if (PixelDataFormat::RGBA_INTEGER == format && PixelDataType::UINT == type)     return wgpu::TextureFormat::RGBA32Uint;
    if (PixelDataFormat::RGBA_INTEGER == format && PixelDataType::INT == type)      return wgpu::TextureFormat::RGBA32Sint;
    if (PixelDataFormat::RGBA == format && PixelDataType::FLOAT == type)            return wgpu::TextureFormat::RGBA32Float;
    if (PixelDataFormat::DEPTH_COMPONENT == format && PixelDataType::FLOAT == type) return wgpu::TextureFormat::Depth32Float;
    return wgpu::TextureFormat::Undefined;
}

[[nodiscard]] constexpr wgpu::TextureFormat toWGPUTextureFormat(const TextureFormat fFormat) {
    switch (fFormat) {
        case TextureFormat::R8:                      return wgpu::TextureFormat::R8Unorm;
        case TextureFormat::R8_SNORM:                return wgpu::TextureFormat::R8Snorm;
        case TextureFormat::R8UI:                    return wgpu::TextureFormat::R8Uint;
        case TextureFormat::R8I:                     return wgpu::TextureFormat::R8Sint;
        case TextureFormat::STENCIL8:                return wgpu::TextureFormat::Stencil8;
        case TextureFormat::R16F:                    return wgpu::TextureFormat::R16Float;
        case TextureFormat::R16UI:                   return wgpu::TextureFormat::R16Uint;
        case TextureFormat::R16I:                    return wgpu::TextureFormat::R16Sint;
        case TextureFormat::RG8:                     return wgpu::TextureFormat::RG8Unorm;
        case TextureFormat::RG8_SNORM:               return wgpu::TextureFormat::RG8Snorm;
        case TextureFormat::RG8UI:                   return wgpu::TextureFormat::RG8Uint;
        case TextureFormat::RG8I:                    return wgpu::TextureFormat::RG8Sint;
        case TextureFormat::R32F:                    return wgpu::TextureFormat::R32Float;
        case TextureFormat::R32UI:                   return wgpu::TextureFormat::R32Uint;
        case TextureFormat::R32I:                    return wgpu::TextureFormat::R32Sint;
        case TextureFormat::RG16F:                   return wgpu::TextureFormat::RG16Float;
        case TextureFormat::RG16UI:                  return wgpu::TextureFormat::RG16Uint;
        case TextureFormat::RG16I:                   return wgpu::TextureFormat::RG16Sint;
        case TextureFormat::RGBA8:                   return wgpu::TextureFormat::RGBA8Unorm;
        case TextureFormat::SRGB8_A8:                return wgpu::TextureFormat::RGBA8UnormSrgb;
        case TextureFormat::RGBA8_SNORM:             return wgpu::TextureFormat::RGBA8Snorm;
        case TextureFormat::RGBA8UI:                 return wgpu::TextureFormat::RGBA8Uint;
        case TextureFormat::RGBA8I:                  return wgpu::TextureFormat::RGBA8Sint;
        case TextureFormat::DEPTH16:                 return wgpu::TextureFormat::Depth16Unorm;
        case TextureFormat::DEPTH24:                 return wgpu::TextureFormat::Depth24Plus;
        case TextureFormat::DEPTH32F:                return wgpu::TextureFormat::Depth32Float;
        case TextureFormat::DEPTH24_STENCIL8:        return wgpu::TextureFormat::Depth24PlusStencil8;
        case TextureFormat::DEPTH32F_STENCIL8:       return wgpu::TextureFormat::Depth32FloatStencil8;
        case TextureFormat::RG32F:                   return wgpu::TextureFormat::RG32Float;
        case TextureFormat::RG32UI:                  return wgpu::TextureFormat::RG32Uint;
        case TextureFormat::RG32I:                   return wgpu::TextureFormat::RG32Sint;
        case TextureFormat::RGBA16F:                 return wgpu::TextureFormat::RGBA16Float;
        case TextureFormat::RGBA16UI:                return wgpu::TextureFormat::RGBA16Uint;
        case TextureFormat::RGBA16I:                 return wgpu::TextureFormat::RGBA16Sint;
        case TextureFormat::RGBA32F:                 return wgpu::TextureFormat::RGBA32Float;
        case TextureFormat::RGBA32UI:                return wgpu::TextureFormat::RGBA32Uint;
        case TextureFormat::RGBA32I:                 return wgpu::TextureFormat::RGBA32Sint;
        case TextureFormat::EAC_R11:                 return wgpu::TextureFormat::EACR11Unorm;
        case TextureFormat::EAC_R11_SIGNED:          return wgpu::TextureFormat::EACR11Snorm;
        case TextureFormat::EAC_RG11:                return wgpu::TextureFormat::EACRG11Unorm;
        case TextureFormat::EAC_RG11_SIGNED:         return wgpu::TextureFormat::EACRG11Snorm;
        case TextureFormat::ETC2_RGB8:               return wgpu::TextureFormat::ETC2RGB8Unorm;
        case TextureFormat::ETC2_SRGB8:              return wgpu::TextureFormat::ETC2RGB8UnormSrgb;
        case TextureFormat::ETC2_RGB8_A1:            return wgpu::TextureFormat::ETC2RGB8A1Unorm;
        case TextureFormat::ETC2_SRGB8_A1:           return wgpu::TextureFormat::ETC2RGB8A1UnormSrgb;
        case TextureFormat::ETC2_EAC_RGBA8:          return wgpu::TextureFormat::ETC2RGBA8Unorm;
        case TextureFormat::ETC2_EAC_SRGBA8:         return wgpu::TextureFormat::ETC2RGBA8UnormSrgb;
        case TextureFormat::RGBA_ASTC_4x4:           return wgpu::TextureFormat::ASTC4x4Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_4x4:   return wgpu::TextureFormat::ASTC4x4UnormSrgb;
        case TextureFormat::RGBA_ASTC_5x4:           return wgpu::TextureFormat::ASTC5x4Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x4:   return wgpu::TextureFormat::ASTC5x4UnormSrgb;
        case TextureFormat::RGBA_ASTC_5x5:           return wgpu::TextureFormat::ASTC5x5Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_5x5:   return wgpu::TextureFormat::ASTC5x5UnormSrgb;
        case TextureFormat::RGBA_ASTC_6x5:           return wgpu::TextureFormat::ASTC6x5Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x5:   return wgpu::TextureFormat::ASTC6x5UnormSrgb;
        case TextureFormat::RGBA_ASTC_6x6:           return wgpu::TextureFormat::ASTC6x6Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_6x6:   return wgpu::TextureFormat::ASTC6x6UnormSrgb;
        case TextureFormat::RGBA_ASTC_8x5:           return wgpu::TextureFormat::ASTC8x5Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x5:   return wgpu::TextureFormat::ASTC8x5UnormSrgb;
        case TextureFormat::RGBA_ASTC_8x6:           return wgpu::TextureFormat::ASTC8x6Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x6:   return wgpu::TextureFormat::ASTC8x6UnormSrgb;
        case TextureFormat::RGBA_ASTC_8x8:           return wgpu::TextureFormat::ASTC8x8Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_8x8:   return wgpu::TextureFormat::ASTC8x8UnormSrgb;
        case TextureFormat::RGBA_ASTC_10x5:          return wgpu::TextureFormat::ASTC10x5Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x5:  return wgpu::TextureFormat::ASTC10x5UnormSrgb;
        case TextureFormat::RGBA_ASTC_10x6:          return wgpu::TextureFormat::ASTC10x6Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x6:  return wgpu::TextureFormat::ASTC10x6UnormSrgb;
        case TextureFormat::RGBA_ASTC_10x8:          return wgpu::TextureFormat::ASTC10x8Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x8:  return wgpu::TextureFormat::ASTC10x8UnormSrgb;
        case TextureFormat::RGBA_ASTC_10x10:         return wgpu::TextureFormat::ASTC10x10Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_10x10: return wgpu::TextureFormat::ASTC10x10UnormSrgb;
        case TextureFormat::RGBA_ASTC_12x10:         return wgpu::TextureFormat::ASTC12x10Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x10: return wgpu::TextureFormat::ASTC12x10UnormSrgb;
        case TextureFormat::RGBA_ASTC_12x12:         return wgpu::TextureFormat::ASTC12x12Unorm;
        case TextureFormat::SRGB8_ALPHA8_ASTC_12x12: return wgpu::TextureFormat::ASTC12x12UnormSrgb;
        case TextureFormat::RED_RGTC1:               return wgpu::TextureFormat::BC4RUnorm;
        case TextureFormat::SIGNED_RED_RGTC1:        return wgpu::TextureFormat::BC4RSnorm;
        case TextureFormat::RED_GREEN_RGTC2:         return wgpu::TextureFormat::BC5RGUnorm;
        case TextureFormat::SIGNED_RED_GREEN_RGTC2:  return wgpu::TextureFormat::BC5RGSnorm;
        case TextureFormat::RGB_BPTC_UNSIGNED_FLOAT: return wgpu::TextureFormat::BC6HRGBUfloat;
        case TextureFormat::RGB_BPTC_SIGNED_FLOAT:   return wgpu::TextureFormat::BC6HRGBFloat;
        case TextureFormat::RGBA_BPTC_UNORM:         return wgpu::TextureFormat::BC7RGBAUnorm;
        case TextureFormat::SRGB_ALPHA_BPTC_UNORM:   return wgpu::TextureFormat::BC7RGBAUnormSrgb;
        case TextureFormat::RGB565:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and discard the alpha and lower precision.
            FWGPU_LOGW << "Requested Filament texture format RGB565 but getting "
                          "wgpu::TextureFormat::Undefined (no direct mapping in wgpu)";
            return wgpu::TextureFormat::Undefined;
        case TextureFormat::RGB9_E5: return wgpu::TextureFormat::RGB9E5Ufloat;
        case TextureFormat::RGB5_A1:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and handle the packing/unpacking in shaders.
            FWGPU_LOGW << "Requested Filament texture format RGB5_A1 but getting "
                          "wgpu::TextureFormat::Undefined (no direct mapping in wgpu)";
            return wgpu::TextureFormat::Undefined;
        case TextureFormat::RGBA4:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and handle the packing/unpacking in shaders.
            FWGPU_LOGW << "Requested Filament texture format RGBA4 but getting "
                          "wgpu::TextureFormat::Undefined (no direct mapping in wgpu)";
            return wgpu::TextureFormat::Undefined;
        case TextureFormat::RGB8:
            FWGPU_LOGW << "Requested Filament texture format RGB8 but getting "
                          "wgpu::TextureFormat::RGBA8Unorm (no direct sRGB equivalent in wgpu "
                          "without alpha)";
            return wgpu::TextureFormat::RGBA8Unorm;
        case TextureFormat::SRGB8:
            FWGPU_LOGW << "Requested Filament texture format SRGB8 but getting "
                          "wgpu::TextureFormat::RGBA8UnormSrgb (no direct sRGB equivalent in wgpu "
                          "without alpha)";
            return wgpu::TextureFormat::RGBA8UnormSrgb;
        case TextureFormat::RGB8_SNORM:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB8_SNORM but getting "
                       "wgpu::TextureFormat::RGBA8Snorm (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA8Snorm;
        case TextureFormat::RGB8UI:
            FWGPU_LOGW << "Requested Filament texture format RGB8UI but getting "
                          "wgpu::TextureFormat::RGBA8Uint (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA8Uint;
        case TextureFormat::RGB8I:
            FWGPU_LOGW << "Requested Filament texture format RGB8I but getting "
                          "wgpu::TextureFormat::RGBA8Sint (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA8Sint;
        case TextureFormat::R11F_G11F_B10F:          return wgpu::TextureFormat::RG11B10Ufloat;
        case TextureFormat::UNUSED:                  return wgpu::TextureFormat::Undefined;
        case TextureFormat::RGB10_A2:                return wgpu::TextureFormat::RGB10A2Unorm;
        case TextureFormat::RGB16F:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB16F but getting "
                       "wgpu::TextureFormat::RGBA16Float (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA16Float;
        case TextureFormat::RGB16UI:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB16UI but getting "
                       "wgpu::TextureFormat::RGBA16Uint (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA16Uint;
        case TextureFormat::RGB16I:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB16I but getting "
                       "wgpu::TextureFormat::RGBA16Sint (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA16Sint;
        case TextureFormat::RGB32F:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB32F but getting "
                       "wgpu::TextureFormat::RGBA32Float (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA32Float;
        case TextureFormat::RGB32UI:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB32UI but getting "
                       "wgpu::TextureFormat::RGBA32Uint (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA32Uint;
        case TextureFormat::RGB32I:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB32I but getting "
                       "wgpu::TextureFormat::RGBA32Sint (no direct mapping in wgpu without alpha)";
            return wgpu::TextureFormat::RGBA32Sint;
        case TextureFormat::DXT1_RGB:                return wgpu::TextureFormat::BC1RGBAUnorm;
        case TextureFormat::DXT1_RGBA:               return wgpu::TextureFormat::BC1RGBAUnorm;
        case TextureFormat::DXT3_RGBA:               return wgpu::TextureFormat::BC2RGBAUnorm;
        case TextureFormat::DXT5_RGBA:               return wgpu::TextureFormat::BC3RGBAUnorm;
        case TextureFormat::DXT1_SRGB:               return wgpu::TextureFormat::BC1RGBAUnormSrgb;
        case TextureFormat::DXT1_SRGBA:              return wgpu::TextureFormat::BC1RGBAUnormSrgb;
        case TextureFormat::DXT3_SRGBA:              return wgpu::TextureFormat::BC2RGBAUnormSrgb;
        case TextureFormat::DXT5_SRGBA:              return wgpu::TextureFormat::BC3RGBAUnormSrgb;
    }
}

[[nodiscard]] constexpr wgpu::TextureViewDimension toWebGPUTextureViewDimension(
        const SamplerType samplerType) {
    switch (samplerType) {
        case SamplerType::SAMPLER_2D:            return wgpu::TextureViewDimension::e2D;
        case SamplerType::SAMPLER_2D_ARRAY:      return wgpu::TextureViewDimension::e2DArray;
        case SamplerType::SAMPLER_CUBEMAP:       return wgpu::TextureViewDimension::Cube;
        case SamplerType::SAMPLER_EXTERNAL:      return wgpu::TextureViewDimension::e2D;
        case SamplerType::SAMPLER_3D:            return wgpu::TextureViewDimension::e3D;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY: return wgpu::TextureViewDimension::CubeArray;
    }
}

[[nodiscard]] constexpr wgpu::TextureDimension toWebGPUTextureDimension(
        const SamplerType samplerType) {
    switch (samplerType) {
        case SamplerType::SAMPLER_2D:            return wgpu::TextureDimension::e2D;
        case SamplerType::SAMPLER_2D_ARRAY:      return wgpu::TextureDimension::e2D;
        case SamplerType::SAMPLER_CUBEMAP:       return wgpu::TextureDimension::e2D;
        case SamplerType::SAMPLER_EXTERNAL:      return wgpu::TextureDimension::e2D;
        case SamplerType::SAMPLER_3D:            return wgpu::TextureDimension::e3D;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY: return wgpu::TextureDimension::e2D;
    }
}

/**
 * @return the linear version of the format (removing the Srgb part) or the original format if it
 * is already linear
 */
[[nodiscard]] constexpr wgpu::TextureFormat toLinearFormat(const wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8UnormSrgb:      return wgpu::TextureFormat::RGBA8Unorm;
        case wgpu::TextureFormat::BGRA8UnormSrgb:      return wgpu::TextureFormat::BGRA8Unorm;
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:    return wgpu::TextureFormat::BC1RGBAUnorm;
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:    return wgpu::TextureFormat::BC2RGBAUnorm;
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:    return wgpu::TextureFormat::BC3RGBAUnorm;
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:    return wgpu::TextureFormat::BC7RGBAUnorm;
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:    return wgpu::TextureFormat::ASTC4x4Unorm;
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:    return wgpu::TextureFormat::ASTC5x4Unorm;
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:    return wgpu::TextureFormat::ASTC5x5Unorm;
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:    return wgpu::TextureFormat::ASTC6x5Unorm;
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:    return wgpu::TextureFormat::ASTC6x6Unorm;
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:    return wgpu::TextureFormat::ASTC8x5Unorm;
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:    return wgpu::TextureFormat::ASTC8x6Unorm;
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:    return wgpu::TextureFormat::ASTC8x8Unorm;
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:   return wgpu::TextureFormat::ASTC10x5Unorm;
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:   return wgpu::TextureFormat::ASTC10x6Unorm;
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:   return wgpu::TextureFormat::ASTC10x8Unorm;
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:  return wgpu::TextureFormat::ASTC10x10Unorm;
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:  return wgpu::TextureFormat::ASTC12x10Unorm;
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:  return wgpu::TextureFormat::ASTC12x12Unorm;
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:   return wgpu::TextureFormat::ETC2RGB8Unorm;
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb: return wgpu::TextureFormat::ETC2RGB8A1Unorm;
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:  return wgpu::TextureFormat::ETC2RGBA8Unorm;
        default:                                       return format;
    }
}

/**
 * @return true if https://www.w3.org/TR/webgpu/#copy-compatible which states:
 *  Two GPUTextureFormats format1 and format2 are copy-compatible if:
 *  - format1 equals format2, or
 *  - format1 and format2 differ only in whether they are srgb formats (have the -srgb suffix).
 */
[[nodiscard]] constexpr bool areCopyCompatible(const wgpu::TextureFormat sourceFormat,
        const wgpu::TextureFormat destinationFormat) {
    return sourceFormat == destinationFormat ||
           (toLinearFormat(sourceFormat) == toLinearFormat(destinationFormat));
}

[[nodiscard]] constexpr wgpu::Extent2D getBlockSize(const wgpu::TextureFormat format) {
    // see https://www.w3.org/TR/webgpu/#texture-formats
    //  and https://www.w3.org/TR/webgpu/#texture-format-caps
    switch (format) {
        // BC compressed formats
        case wgpu::TextureFormat::BC1RGBAUnorm:
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:
        case wgpu::TextureFormat::BC2RGBAUnorm:
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:
        case wgpu::TextureFormat::BC3RGBAUnorm:
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:
        case wgpu::TextureFormat::BC4RUnorm:
        case wgpu::TextureFormat::BC4RSnorm:
        case wgpu::TextureFormat::BC5RGUnorm:
        case wgpu::TextureFormat::BC5RGSnorm:
        case wgpu::TextureFormat::BC6HRGBUfloat:
        case wgpu::TextureFormat::BC6HRGBFloat:
        case wgpu::TextureFormat::BC7RGBAUnorm:
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:
        // ETC2 compressed formats
        case wgpu::TextureFormat::ETC2RGB8Unorm:
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
        case wgpu::TextureFormat::ETC2RGBA8Unorm:
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
        case wgpu::TextureFormat::EACR11Unorm:
        case wgpu::TextureFormat::EACR11Snorm:
        case wgpu::TextureFormat::EACRG11Unorm:
        case wgpu::TextureFormat::EACRG11Snorm:
        // ASTC compressed formats
        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
            return { .width = 4, .height = 4 };
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
            return { .width = 5, .height = 4 };
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
            return { .width = 5, .height = 5 };
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
            return { .width = 6, .height = 5 };
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
            return { .width = 6, .height = 6 };
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
            return { .width = 8, .height = 5 };
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
            return { .width = 8, .height = 6 };
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
            return { .width = 8, .height = 8 };
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
            return { .width = 10, .height = 5 };
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
            return { .width = 10, .height = 6 };
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
            return { .width = 10, .height = 8 };
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
            return { .width = 10, .height = 10 };
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
            return { .width = 12, .height = 10 };
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
            return { .width = 12, .height = 12 };
        default:
            // All other formats are uncompressed and have a 1x1 block size.
            return { .width = 1, .height = 1 };
    }
}

[[nodiscard]] constexpr size_t getWGPUTextureFormatPixelSize(const wgpu::TextureFormat format) {
    switch (format) {
        // 1 byte
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R8Sint:
        case wgpu::TextureFormat::Stencil8:
            return 1;

        // 2 bytes
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG8Sint:
        case wgpu::TextureFormat::Depth16Unorm:
            return 2;

        // 4 bytes
        // -- 32-bit single-channel formats
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::R32Sint:
        // -- 16-bit two-channel formats
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RG16Float:
        // -- 8-bit four-channel formats
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        // -- Packed 32-bit formats
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RG11B10Ufloat:
        case wgpu::TextureFormat::RGB9E5Ufloat:
        // -- Depth/Stencil formats
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth24PlusStencil8:
            return 4;

        // 8 bytes
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Float:
            return 8;

        // 16 bytes
        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA32Sint:
            return 16;

        // Non-copyable or compressed formats
        case wgpu::TextureFormat::Depth32FloatStencil8: // Not copyable from texture to buffer
        default:
            return 0;
    }
}

/**
 * @param format texture format to potentially be used for storage binding
 * @return true if the format is compatible with wgpu::TextureUsage::StorageBinding
 */
[[nodiscard]] constexpr bool isFormatStorageCompatible(const wgpu::TextureFormat format) {
    switch (format) {
        // List of formats that support storage binding
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA32Float:
        case wgpu::TextureFormat::RGBA32Sint:
        case wgpu::TextureFormat::RGBA32Uint:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGBA8Sint:
            return true;
        default:
            // All other formats, including packed floats (RG11B10Ufloat),
            // depth/stencil, and sRGB formats do not support storage.
            return false;
    }
}

/**
 * Todo: should this take into account sRGB/linear when determining if conversion is necessary?
 *       For instance, if the output format is the same as the input, except the output is sRGB
 *       do we really need to do a conversion? If the answer is no, we should do that check here
 *       and return conversionNecessary false in that case.
 *       However, doing a straight-forward comparison is the safest most conservative thing to do
 *       for functional correctness and NOT doing a conversion in such cases could be considered
 *       an optimization. Thus, consider the optimization when we have better test coverage to
 *       experiment with such a refactor.
 * @return True if theres a format mismatch
 */
[[nodiscard]] constexpr bool conversionNecessary(const wgpu::TextureFormat source,
        const wgpu::TextureFormat destination, const PixelDataType pixelDataType) {
    return source != destination && pixelDataType != PixelDataType::COMPRESSED;
}

/**
 * @param fUsage Filament's requested texture usage
 * @param samples How many samples to use for MSAA
 * @param needsComputeStorageSupport if we need to use this texture as storage binding in something
 *                                   like a compute shader
 * @param needsRenderAttachmentSupport if we need to use this texture as a render pass attachment
 *                                     in something like a render pass blit (e.g. mipmap generation)
 * @param deviceSupportsTransientAttachments if the device itself supports Render Attachments
 * @return The appropriate texture usage flags for the underlying texture
 */
[[nodiscard]] wgpu::TextureUsage fToWGPUTextureUsage(TextureUsage const& fUsage,
        const uint8_t samples, const bool needsComputeStorageSupport,
        const bool needsRenderAttachmentSupport, const bool deviceSupportsTransientAttachments) {
    wgpu::TextureUsage retUsage{ wgpu::TextureUsage::None };

    if (any(TextureUsage::BLIT_SRC & fUsage)) {
        retUsage |= (wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    }
    if (any(TextureUsage::BLIT_DST & fUsage)) {
        retUsage |= (wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);
    }
    if (any(TextureUsage::UPLOADABLE & fUsage)) {
        retUsage |= wgpu::TextureUsage::CopyDst;
    }
    if (any(TextureUsage::GEN_MIPMAPPABLE & fUsage)) {
        retUsage |= (wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding);
    }
    if (any(TextureUsage::SAMPLEABLE & fUsage)) {
        retUsage |= wgpu::TextureUsage::TextureBinding;
    }
    // if needsComputeStorageSupport we need to read and write to the texture in a shader and thus
    // require CopySrc, CopyDst, TextureBinding, & StorageBinding
    if (needsComputeStorageSupport) {
        retUsage |= (wgpu::TextureUsage::StorageBinding |
                     wgpu::TextureUsage::CopySrc |
                     wgpu::TextureUsage::CopyDst |
                     wgpu::TextureUsage::TextureBinding);
    }

    wgpu::TextureUsage transientAttachmentNeeded{ wgpu::TextureUsage::None };
    const bool useTransientAttachment {
            deviceSupportsTransientAttachments &&
            // Usage consists of attachment flags only.
            none(fUsage & ~TextureUsage::ALL_ATTACHMENTS) &&
            // Usage contains at least one attachment flag.
            any(fUsage & TextureUsage::ALL_ATTACHMENTS) &&
            // Depth resolve cannot use transient attachment because it uses a custom shader.
            // TODO: see VulkanDriver::isDepthStencilResolveSupported() to know when to remove this
            // restriction.
            // Note that the custom shader does not resolve stencil. We do need to move to vk 1.2
            // and above to be able to support stencil resolve (along with depth).
            !(any(fUsage & TextureUsage::DEPTH_ATTACHMENT) && samples > 1)};
    if (useTransientAttachment) {
        transientAttachmentNeeded |= wgpu::TextureUsage::TransientAttachment;
    }

    // A texture that is a blit destination or render attachment will often need to be
    // a copy source for subsequent operations (e.g., mipmap generation, readbacks).
    // However, we dont need to add the CopySrc IF its a transientAttachment
    if (any((TextureUsage::BLIT_DST | TextureUsage::COLOR_ATTACHMENT |
                    TextureUsage::DEPTH_ATTACHMENT) &
                fUsage)) {
        if (!useTransientAttachment) {
            retUsage |= wgpu::TextureUsage::CopySrc;
        }
    }

    if (needsRenderAttachmentSupport) {
        retUsage |= wgpu::TextureUsage::RenderAttachment;
    }
    // WGPU Render attachment covers either color or stencil situation dependant
    if (any((TextureUsage::COLOR_ATTACHMENT | TextureUsage::STENCIL_ATTACHMENT |
                    TextureUsage::DEPTH_ATTACHMENT) &
                fUsage)) {
        retUsage |= wgpu::TextureUsage::RenderAttachment;
        retUsage |= transientAttachmentNeeded;
    }

    // NOTE: Unused wgpu flags:
    //  StorageAttachment

    // NOTE: Unused Filament flags:
    //  SUBPASS_INPUT: VK goes to input attachment which we don't support right now
    //  PROTECTED
    return retUsage;
}

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUTEXTUREHELPERS_H
