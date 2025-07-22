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

#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

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

enum class ScalarSampleType : uint8_t {
    F32,
    I32,
    U32,
};

[[nodiscard]] constexpr ScalarSampleType getScalarSampleTypeFrom(const wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8Uint:
        case wgpu::TextureFormat::R16Uint:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::Stencil8:
        case wgpu::TextureFormat::RG8Uint:
        case wgpu::TextureFormat::RG16Uint:
        case wgpu::TextureFormat::RGBA8Uint:
        case wgpu::TextureFormat::RGB10A2Uint:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RGBA16Uint:
        case wgpu::TextureFormat::RGBA32Uint:
            return ScalarSampleType::U32;
        case wgpu::TextureFormat::R8Sint:
        case wgpu::TextureFormat::R16Sint:
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::RG8Sint:
        case wgpu::TextureFormat::RG16Sint:
        case wgpu::TextureFormat::RGBA8Sint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RGBA16Sint:
        case wgpu::TextureFormat::RGBA32Sint:
            return ScalarSampleType::I32;
        case wgpu::TextureFormat::R8Snorm:
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG8Snorm:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::RGBA8Snorm:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RG11B10Ufloat:
        case wgpu::TextureFormat::RGB9E5Ufloat:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA32Float:
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
        case wgpu::TextureFormat::ASTC4x4Unorm:
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x4Unorm:
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:
        case wgpu::TextureFormat::ASTC5x5Unorm:
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x5Unorm:
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:
        case wgpu::TextureFormat::ASTC6x6Unorm:
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x5Unorm:
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:
        case wgpu::TextureFormat::ASTC8x6Unorm:
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:
        case wgpu::TextureFormat::ASTC8x8Unorm:
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x5Unorm:
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:
        case wgpu::TextureFormat::ASTC10x6Unorm:
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:
        case wgpu::TextureFormat::ASTC10x8Unorm:
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:
        case wgpu::TextureFormat::ASTC10x10Unorm:
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x10Unorm:
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:
        case wgpu::TextureFormat::ASTC12x12Unorm:
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:
        case wgpu::TextureFormat::R16Unorm:
        case wgpu::TextureFormat::RG16Unorm:
        case wgpu::TextureFormat::RGBA16Unorm:
        case wgpu::TextureFormat::R16Snorm:
        case wgpu::TextureFormat::RG16Snorm:
        case wgpu::TextureFormat::RGBA16Snorm:
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return ScalarSampleType::F32;
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            PANIC_POSTCONDITION("A depth texture format requires special sampler treatment and "
                                "does not generally support linear filtering (needed for mipmap "
                                "generation). Thus no applicable scalar sample type for %d",
                    format);
            break;
        case wgpu::TextureFormat::Undefined:
        case wgpu::TextureFormat::External:
            PANIC_POSTCONDITION("No scalar sample type for texture format %d", format);
            break;
    }
}

[[nodiscard]] constexpr std::string_view toWGSLString(const ScalarSampleType type) {
    switch (type) {
        case ScalarSampleType::F32: return "f32";
        case ScalarSampleType::I32: return "i32";
        case ScalarSampleType::U32: return "u32";
    }
}

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUTEXTUREHELPERS_H
