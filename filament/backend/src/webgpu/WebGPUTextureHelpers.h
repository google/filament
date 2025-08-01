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

#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <string_view>

namespace filament::backend {

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

[[nodiscard]] constexpr wgpu::TextureFormat toWebGPULinearFormat(const wgpu::TextureFormat format) {
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

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUTEXTUREHELPERS_H
