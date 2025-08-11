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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUSTRINGS_H
#define TNT_FILAMENT_BACKEND_WEBGPUSTRINGS_H

#include "WebGPUConstants.h"

#include <backend/DriverEnums.h>

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>

/**
 * Reusable set of convenience functions for strings -- generally string views, literals, &
 * strings -- used in the WebGPU backend
 */

namespace filament::backend {

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM) \
        || FWGPU_ENABLED(FWGPU_DEBUG_UPDATE_IMAGE) \
        || FWGPU_ENABLED(FWGPU_DEBUG_BLIT)\
        || FWGPU_ENABLED(FWGPU_DEBUG_BIND_GROUPS)
template<typename WebGPUPrintable>
[[nodiscard]] inline std::string webGPUPrintableToString(const WebGPUPrintable printable) {
    std::stringstream out;
    out << printable;
    return out.str();
}
#endif

[[nodiscard]] constexpr std::string_view errorTypeToString(const wgpu::ErrorType errorType) {
    switch (errorType) {
        case wgpu::ErrorType::NoError:     return "NO_ERROR";
        case wgpu::ErrorType::Validation:  return "VALIDATION";
        case wgpu::ErrorType::OutOfMemory: return "OUT_OF_MEMORY";
        case wgpu::ErrorType::Internal:    return "INTERNAL";
        case wgpu::ErrorType::Unknown:     return "UNKNOWN";
    }
}

[[nodiscard]] constexpr std::string_view powerPreferenceToString(
        const wgpu::PowerPreference powerPreference) {
    switch (powerPreference) {
        case wgpu::PowerPreference::Undefined:       return "UNDEFINED";
        case wgpu::PowerPreference::LowPower:        return "LOW_POWER";
        case wgpu::PowerPreference::HighPerformance: return "HIGH_PERFORMANCE";
    }
}

[[nodiscard]] static inline std::string_view powerPreferenceToString(
        const wgpu::DawnAdapterPropertiesPowerPreference powerPreference) {
    return powerPreferenceToString(powerPreference.powerPreference);
}

[[nodiscard]] constexpr std::string_view backendTypeToString(const wgpu::BackendType backendType) {
    switch (backendType) {
        case wgpu::BackendType::Undefined: return "UNDEFINED";
        case wgpu::BackendType::Null:      return "NULL";
        case wgpu::BackendType::WebGPU:    return "WEBGPU";
        case wgpu::BackendType::D3D11:     return "D3D11";
        case wgpu::BackendType::D3D12:     return "D3D12";
        case wgpu::BackendType::Metal:     return "METAL";
        case wgpu::BackendType::Vulkan:    return "VULKAN";
        case wgpu::BackendType::OpenGL:    return "OPENGL";
        case wgpu::BackendType::OpenGLES:  return "OPENGLES";
    }
}

[[nodiscard]] constexpr std::string_view adapterTypeToString(const wgpu::AdapterType adapterType) {
    switch (adapterType) {
        case wgpu::AdapterType::DiscreteGPU:   return "DISCRETE_GPU";
        case wgpu::AdapterType::IntegratedGPU: return "INTEGRATED_GPU";
        case wgpu::AdapterType::CPU:           return "CPU";
        case wgpu::AdapterType::Unknown:       return "UNKNOWN";
    }
}

[[nodiscard]] inline std::string adapterOptionsToString(
        wgpu::RequestAdapterOptions const& options) {
    std::stringstream out;
    out << "power preference " << powerPreferenceToString(options.powerPreference)
        << " force fallback adapter " << bool(options.forceFallbackAdapter) << " backend type "
        << backendTypeToString(options.backendType);
    return out.str();
}

[[nodiscard]] inline std::string adapterInfoToString(wgpu::AdapterInfo const& info) {
    std::stringstream out;
    out << "vendor (" << info.vendorID << ") '" << info.vendor
        << "' device (" << info.deviceID << ") '" << info.device
        << "' adapter " << adapterTypeToString(info.adapterType)
        << " backend " << backendTypeToString(info.backendType)
        << " architecture '" << info.architecture
        << "' subgroupMinSize " << info.subgroupMinSize
        << " subgroupMaxSize " << info.subgroupMaxSize;
    return out.str();
}

[[nodiscard]] constexpr std::string_view deviceLostReasonToString(
        const wgpu::DeviceLostReason reason) {
    switch (reason) {
        case wgpu::DeviceLostReason::Unknown:           return "UNKNOWN";
        case wgpu::DeviceLostReason::Destroyed:         return "DESTROYED";
        case wgpu::DeviceLostReason::CallbackCancelled: return "CALLBACK_CANCELLED";
        case wgpu::DeviceLostReason::FailedCreation:    return "FAILED_CREATION";
    }
}

[[nodiscard]] constexpr std::string_view webGPUTextureFormatToString(
        const wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Undefined:                   return "Undefined";
        case wgpu::TextureFormat::R8Unorm:                     return "R8Unorm";
        case wgpu::TextureFormat::R8Snorm:                     return "R8Snorm";
        case wgpu::TextureFormat::R8Uint:                      return "R8Uint";
        case wgpu::TextureFormat::R8Sint:                      return "R8Sint";
        case wgpu::TextureFormat::R16Uint:                     return "R16Uint";
        case wgpu::TextureFormat::R16Sint:                     return "R16Sint";
        case wgpu::TextureFormat::R16Float:                    return "R16Float";
        case wgpu::TextureFormat::RG8Unorm:                    return "RG8Unorm";
        case wgpu::TextureFormat::RG8Snorm:                    return "RG8Snorm";
        case wgpu::TextureFormat::RG8Uint:                     return "RG8Uint";
        case wgpu::TextureFormat::RG8Sint:                     return "RG8Sint";
        case wgpu::TextureFormat::R32Float:                    return "R32Float";
        case wgpu::TextureFormat::R32Uint:                     return "R32Uint";
        case wgpu::TextureFormat::R32Sint:                     return "R32Sint";
        case wgpu::TextureFormat::RG16Uint:                    return "RG16Uint";
        case wgpu::TextureFormat::RG16Sint:                    return "RG16Sint";
        case wgpu::TextureFormat::RG16Float:                   return "RG16Float";
        case wgpu::TextureFormat::RGBA8Unorm:                  return "RGBA8Unorm";
        case wgpu::TextureFormat::RGBA8UnormSrgb:              return "RGBA8UnormSrgb";
        case wgpu::TextureFormat::RGBA8Snorm:                  return "RGBA8Snorm";
        case wgpu::TextureFormat::RGBA8Uint:                   return "RGBA8Uint";
        case wgpu::TextureFormat::RGBA8Sint:                   return "RGBA8Sint";
        case wgpu::TextureFormat::BGRA8Unorm:                  return "BGRA8Unorm";
        case wgpu::TextureFormat::BGRA8UnormSrgb:              return "BGRA8UnormSrgb";
        case wgpu::TextureFormat::RGB10A2Uint:                 return "RGB10A2Uint";
        case wgpu::TextureFormat::RGB10A2Unorm:                return "RGB10A2Unorm";
        case wgpu::TextureFormat::RG11B10Ufloat:               return "RG11B10Ufloat";
        case wgpu::TextureFormat::RGB9E5Ufloat:                return "RGB9E5Ufloat";
        case wgpu::TextureFormat::RG32Float:                   return "RG32Float";
        case wgpu::TextureFormat::RG32Uint:                    return "RG32Uint";
        case wgpu::TextureFormat::RG32Sint:                    return "RG32Sint";
        case wgpu::TextureFormat::RGBA16Uint:                  return "RGBA16Uint";
        case wgpu::TextureFormat::RGBA16Sint:                  return "RGBA16Sint";
        case wgpu::TextureFormat::RGBA16Float:                 return "RGBA16Float";
        case wgpu::TextureFormat::RGBA32Float:                 return "RGBA32Float";
        case wgpu::TextureFormat::RGBA32Uint:                  return "RGBA32Uint";
        case wgpu::TextureFormat::RGBA32Sint:                  return "RGBA32Sint";
        case wgpu::TextureFormat::Stencil8:                    return "Stencil8";
        case wgpu::TextureFormat::Depth16Unorm:                return "Depth16Unorm";
        case wgpu::TextureFormat::Depth24Plus:                 return "Depth24Plus";
        case wgpu::TextureFormat::Depth24PlusStencil8:         return "Depth24PlusStencil8";
        case wgpu::TextureFormat::Depth32Float:                return "Depth32Float";
        case wgpu::TextureFormat::Depth32FloatStencil8:        return "Depth32FloatStencil8";
        case wgpu::TextureFormat::BC1RGBAUnorm:                return "BC1RGBAUnorm";
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:            return "BC1RGBAUnormSrgb";
        case wgpu::TextureFormat::BC2RGBAUnorm:                return "BC2RGBAUnorm";
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:            return "BC2RGBAUnormSrgb";
        case wgpu::TextureFormat::BC3RGBAUnorm:                return "BC3RGBAUnorm";
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:            return "BC3RGBAUnormSrgb";
        case wgpu::TextureFormat::BC4RUnorm:                   return "BC4RUnorm";
        case wgpu::TextureFormat::BC4RSnorm:                   return "BC4RSnorm";
        case wgpu::TextureFormat::BC5RGUnorm:                  return "BC5RGUnorm";
        case wgpu::TextureFormat::BC5RGSnorm:                  return "BC5RGSnorm";
        case wgpu::TextureFormat::BC6HRGBUfloat:               return "BC6HRGBUfloat";
        case wgpu::TextureFormat::BC6HRGBFloat:                return "BC6HRGBFloat";
        case wgpu::TextureFormat::BC7RGBAUnorm:                return "BC7RGBAUnorm";
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:            return "BC7RGBAUnormSrgb";
        case wgpu::TextureFormat::ETC2RGB8Unorm:               return "ETC2RGB8Unorm";
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:           return "ETC2RGB8UnormSrgb";
        case wgpu::TextureFormat::ETC2RGB8A1Unorm:             return "ETC2RGB8A1Unorm";
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:         return "ETC2RGB8A1UnormSrgb";
        case wgpu::TextureFormat::ETC2RGBA8Unorm:              return "ETC2RGBA8Unorm";
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:          return "ETC2RGBA8UnormSrgb";
        case wgpu::TextureFormat::EACR11Unorm:                 return "EACR11Unorm";
        case wgpu::TextureFormat::EACR11Snorm:                 return "EACR11Snorm";
        case wgpu::TextureFormat::EACRG11Unorm:                return "EACRG11Unorm";
        case wgpu::TextureFormat::EACRG11Snorm:                return "EACRG11Snorm";
        case wgpu::TextureFormat::ASTC4x4Unorm:                return "ASTC4x4Unorm";
        case wgpu::TextureFormat::ASTC4x4UnormSrgb:            return "ASTC4x4UnormSrgb";
        case wgpu::TextureFormat::ASTC5x4Unorm:                return "ASTC5x4Unorm";
        case wgpu::TextureFormat::ASTC5x4UnormSrgb:            return "ASTC5x4UnormSrgb";
        case wgpu::TextureFormat::ASTC5x5Unorm:                return "ASTC5x5Unorm";
        case wgpu::TextureFormat::ASTC5x5UnormSrgb:            return "ASTC5x5UnormSrgb";
        case wgpu::TextureFormat::ASTC6x5Unorm:                return "ASTC6x5Unorm";
        case wgpu::TextureFormat::ASTC6x5UnormSrgb:            return "ASTC6x5UnormSrgb";
        case wgpu::TextureFormat::ASTC6x6Unorm:                return "ASTC6x6Unorm";
        case wgpu::TextureFormat::ASTC6x6UnormSrgb:            return "ASTC6x6UnormSrgb";
        case wgpu::TextureFormat::ASTC8x5Unorm:                return "ASTC8x5Unorm";
        case wgpu::TextureFormat::ASTC8x5UnormSrgb:            return "ASTC8x5UnormSrgb";
        case wgpu::TextureFormat::ASTC8x6Unorm:                return "ASTC8x6Unorm";
        case wgpu::TextureFormat::ASTC8x6UnormSrgb:            return "ASTC8x6UnormSrgb";
        case wgpu::TextureFormat::ASTC8x8Unorm:                return "ASTC8x8Unorm";
        case wgpu::TextureFormat::ASTC8x8UnormSrgb:            return "ASTC8x8UnormSrgb";
        case wgpu::TextureFormat::ASTC10x5Unorm:               return "ASTC10x5Unorm";
        case wgpu::TextureFormat::ASTC10x5UnormSrgb:           return "ASTC10x5UnormSrgb";
        case wgpu::TextureFormat::ASTC10x6Unorm:               return "ASTC10x6Unorm";
        case wgpu::TextureFormat::ASTC10x6UnormSrgb:           return "ASTC10x6UnormSrgb";
        case wgpu::TextureFormat::ASTC10x8Unorm:               return "ASTC10x8Unorm";
        case wgpu::TextureFormat::ASTC10x8UnormSrgb:           return "ASTC10x8UnormSrgb";
        case wgpu::TextureFormat::ASTC10x10Unorm:              return "ASTC10x10Unorm";
        case wgpu::TextureFormat::ASTC10x10UnormSrgb:          return "ASTC10x10UnormSrgb";
        case wgpu::TextureFormat::ASTC12x10Unorm:              return "ASTC12x10Unorm";
        case wgpu::TextureFormat::ASTC12x10UnormSrgb:          return "ASTC12x10UnormSrgb";
        case wgpu::TextureFormat::ASTC12x12Unorm:              return "ASTC12x12Unorm";
        case wgpu::TextureFormat::ASTC12x12UnormSrgb:          return "ASTC12x12UnormSrgb";
        case wgpu::TextureFormat::R16Unorm:                    return "R16Unorm";
        case wgpu::TextureFormat::RG16Unorm:                   return "RG16Unorm";
        case wgpu::TextureFormat::RGBA16Unorm:                 return "RGBA16Unorm";
        case wgpu::TextureFormat::R16Snorm:                    return "R16Snorm";
        case wgpu::TextureFormat::RG16Snorm:                   return "RG16Snorm";
        case wgpu::TextureFormat::RGBA16Snorm:                 return "RGBA16Snorm";
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:       return "R8BG8Biplanar420Unorm";
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm: return "R10X6BG10X6Biplanar420Unorm";
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:    return "R8BG8A8Triplanar420Unorm";
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:       return "R8BG8Biplanar422Unorm";
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:       return "R8BG8Biplanar444Unorm";
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm: return "R10X6BG10X6Biplanar422Unorm";
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm: return "R10X6BG10X6Biplanar444Unorm";
        case wgpu::TextureFormat::External:                    return "External";
    }
}

[[nodiscard]] constexpr std::string_view webGPUTextureDimensionToString(
        const wgpu::TextureDimension dimension) {
    switch (dimension) {
        case wgpu::TextureDimension::Undefined: return "Undefined";
        case wgpu::TextureDimension::e1D:       return "e1D";
        case wgpu::TextureDimension::e2D:       return "e2D";
        case wgpu::TextureDimension::e3D:       return "e3D";
    }
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM) \
        || FWGPU_ENABLED(FWGPU_DEBUG_UPDATE_IMAGE) \
        || FWGPU_ENABLED(FWGPU_DEBUG_BLIT)         \
        || FWGPU_ENABLED(FWGPU_DEBUG_BIND_GROUPS)
[[nodiscard]] inline std::string webGPUTextureToString(wgpu::Texture const& texture) {
    if (texture == nullptr) {
        return "nullptr (no wgpu::Texture)";
    }
    std::stringstream out;
    out << "wgpu::Texture("
        << " format:" << webGPUTextureFormatToString(texture.GetFormat())
        << " dimension:" << webGPUTextureDimensionToString(texture.GetDimension())
        << " " << webGPUPrintableToString(texture.GetUsage())
        << " mipLevelCount:" << texture.GetMipLevelCount()
        << " sampleCount:" << texture.GetSampleCount() << " width:" << texture.GetWidth()
        << " height:" << texture.GetHeight()
        << " depthOrArrayLayers:" << texture.GetDepthOrArrayLayers() << ")";
    return out.str();
}
#endif

[[nodiscard]] constexpr std::string_view filamentShaderStageToString(const ShaderStage stage) {
    switch (stage) {
        case ShaderStage::VERTEX:   return "vertex";
        case ShaderStage::FRAGMENT: return "fragment";
        case ShaderStage::COMPUTE:  return "compute";
    }
}

#if FWGPU_ENABLED(FWGPU_DEBUG_BIND_GROUPS)
[[nodiscard]] constexpr std::string_view filamentDescriptorTypeToString(const DescriptorType type) {
    switch (type) {
     case DescriptorType::SAMPLER_2D_FLOAT:          return "SAMPLER_2D_FLOAT";
     case DescriptorType::SAMPLER_2D_INT:            return "SAMPLER_2D_INT";
     case DescriptorType::SAMPLER_2D_UINT:           return "SAMPLER_2D_UINT";
     case DescriptorType::SAMPLER_2D_DEPTH:          return "SAMPLER_2D_DEPTH";
     case DescriptorType::SAMPLER_2D_ARRAY_FLOAT:    return "SAMPLER_2D_ARRAY_FLOAT";
     case DescriptorType::SAMPLER_2D_ARRAY_INT:      return "SAMPLER_2D_ARRAY_INT";
     case DescriptorType::SAMPLER_2D_ARRAY_UINT:     return "SAMPLER_2D_ARRAY_UINT";
     case DescriptorType::SAMPLER_2D_ARRAY_DEPTH:    return "SAMPLER_2D_ARRAY_DEPTH";
     case DescriptorType::SAMPLER_CUBE_FLOAT:        return "SAMPLER_CUBE_FLOAT";
     case DescriptorType::SAMPLER_CUBE_INT:          return "SAMPLER_CUBE_INT";
     case DescriptorType::SAMPLER_CUBE_UINT:         return "SAMPLER_CUBE_UINT";
     case DescriptorType::SAMPLER_CUBE_DEPTH:        return "SAMPLER_CUBE_DEPTH";
     case DescriptorType::SAMPLER_CUBE_ARRAY_FLOAT:  return "SAMPLER_CUBE_ARRAY_FLOAT";
     case DescriptorType::SAMPLER_CUBE_ARRAY_INT:    return "SAMPLER_CUBE_ARRAY_INT";
     case DescriptorType::SAMPLER_CUBE_ARRAY_UINT:   return "SAMPLER_CUBE_ARRAY_UINT";
     case DescriptorType::SAMPLER_CUBE_ARRAY_DEPTH:  return "SAMPLER_CUBE_ARRAY_DEPTH";
     case DescriptorType::SAMPLER_3D_FLOAT:          return "SAMPLER_3D_FLOAT";
     case DescriptorType::SAMPLER_3D_INT:            return "SAMPLER_3D_INT";
     case DescriptorType::SAMPLER_3D_UINT:           return "SAMPLER_3D_UINT";
     case DescriptorType::SAMPLER_2D_MS_FLOAT:       return "SAMPLER_2D_MS_FLOAT";
     case DescriptorType::SAMPLER_2D_MS_INT:         return "SAMPLER_2D_MS_INT";
     case DescriptorType::SAMPLER_2D_MS_UINT:        return "SAMPLER_2D_MS_UINT";
     case DescriptorType::SAMPLER_2D_MS_ARRAY_FLOAT: return "SAMPLER_2D_MS_ARRAY_FLOAT";
     case DescriptorType::SAMPLER_2D_MS_ARRAY_INT:   return "SAMPLER_2D_MS_ARRAY_INT";
     case DescriptorType::SAMPLER_2D_MS_ARRAY_UINT:  return "SAMPLER_2D_MS_ARRAY_UINT";
     case DescriptorType::SAMPLER_EXTERNAL:          return "SAMPLER_EXTERNAL";
     case DescriptorType::UNIFORM_BUFFER:            return "UNIFORM_BUFFER";
     case DescriptorType::SHADER_STORAGE_BUFFER:     return "SHADER_STORAGE_BUFFER";
     case DescriptorType::INPUT_ATTACHMENT:          return "INPUT_ATTACHMENT";
    }
}

[[nodiscard]] constexpr std::string_view filamentStageFlagsToString(const ShaderStageFlags flags) {
    if (flags == ShaderStageFlags::FRAGMENT) {
        return "FRAGMENT";
    }
    if (flags == (ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT)) {
        return "VERTEX | FRAGMENT";
    }
    if (flags == ShaderStageFlags::VERTEX) {
        return "VERTEX";
    }
    if (flags == ShaderStageFlags::COMPUTE) {
        return "COMPUTE";
    }
    if (flags == ShaderStageFlags::ALL_SHADER_STAGE_FLAGS) {
        return "VERTEX | FRAGMENT | COMPUTE";
    }
    if (flags == (ShaderStageFlags::FRAGMENT | ShaderStageFlags::COMPUTE)) {
        return "FRAGMENT | COMPUTE";
    }
    if (flags == (ShaderStageFlags::VERTEX | ShaderStageFlags::COMPUTE)) {
        return "VERTEX | COMPUTE";
    }
    if (flags == ShaderStageFlags::NONE) {
        return "NONE";
    }
    return "UNKNOWN/UNHANDLED"; // should not be possible
}

[[nodiscard]] constexpr std::string_view filamentDescriptorFlagsToString(const DescriptorFlags flags) {
    if (flags == DescriptorFlags::NONE) {
        return "NONE";
    }
    if (flags == DescriptorFlags::DYNAMIC_OFFSET) {
        return "DYNAMIC_OFFSET";
    }
    if (flags == DescriptorFlags::UNFILTERABLE) {
        return "UNFILTERABLE";
    }
    if (flags == (DescriptorFlags::DYNAMIC_OFFSET | DescriptorFlags::UNFILTERABLE)) {
        return "DYNAMIC_OFFSET | UNFILTERABLE";
    }
    return "UNKNOWN/UNHANDLED"; // should not be possible
}

[[nodiscard]] constexpr std::string_view filamentSamplerMagFilterToString(
        const SamplerMagFilter filter) {
    switch (filter) {
        case SamplerMagFilter::NEAREST: return "NEAREST";
        case SamplerMagFilter::LINEAR:  return "LINEAR";
    }
}

[[nodiscard]] constexpr std::string_view filamentSamplerMinFilterToString(
        const SamplerMinFilter filter) {
    switch (filter) {
        case SamplerMinFilter::NEAREST:                return "NEAREST";
        case SamplerMinFilter::LINEAR:                 return "LINEAR";
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST: return "NEAREST_MIPMAP_NEAREST";
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST:  return "LINEAR_MIPMAP_NEAREST";
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR:  return "NEAREST_MIPMAP_LINEAR";
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR:   return "LINEAR_MIPMAP_LINEAR";
    }
}

[[nodiscard]] constexpr std::string_view filamentSamplerWrapModeToString(
        const SamplerWrapMode mode) {
    switch (mode) {
        case SamplerWrapMode::CLAMP_TO_EDGE:   return "CLAMP_TO_EDGE";
        case SamplerWrapMode::REPEAT:          return "REPEAT";
        case SamplerWrapMode::MIRRORED_REPEAT: return "MIRRORED_REPEAT";
    }
}

[[nodiscard]] constexpr std::string_view filamentSamplerCompareModeToString(
        const SamplerCompareMode mode) {
    switch (mode) {
        case SamplerCompareMode::NONE:               return "NONE";
        case SamplerCompareMode::COMPARE_TO_TEXTURE: return "COMPARE_TO_TEXTURE";
    }
}

[[nodiscard]] constexpr std::string_view filamentSamplerCompareFuncToString(
        const SamplerCompareFunc func) {
    switch (func) {
        case SamplerCompareFunc::LE: return "LE";
        case SamplerCompareFunc::GE: return "GE";
        case SamplerCompareFunc::L:  return "L";
        case SamplerCompareFunc::G:  return "G";
        case SamplerCompareFunc::E:  return "E";
        case SamplerCompareFunc::NE: return "NE";
        case SamplerCompareFunc::A:  return "A";
        case SamplerCompareFunc::N:  return "N";
    }
}

[[nodiscard]] inline std::string filamentTextureUsageFlagsToString(const TextureUsage flags) {
    if (flags == TextureUsage::NONE) {
        return "NONE";
    }
    bool first{ true };
    std::stringstream out;
    if (any(flags & TextureUsage::COLOR_ATTACHMENT)) {
        first = false;
        out << "COLOR_ATTACHMENT";
    }
    if (any(flags & TextureUsage::DEPTH_ATTACHMENT)) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << "DEPTH_ATTACHMENT";
    }
    if (any(flags & TextureUsage::STENCIL_ATTACHMENT)) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << "STENCIL_ATTACHMENT";
    }
    if (any(flags & TextureUsage::UPLOADABLE)) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << "UPLOADABLE";
    }
    if (any(flags & TextureUsage::SAMPLEABLE)) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << "SAMPLEABLE";
    }
    if (any(flags & TextureUsage::SUBPASS_INPUT)) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << "SUBPASS_INPUT";
    }
    if (any(flags & TextureUsage::BLIT_SRC)) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << "BLIT_SRC";
    }
    if (any(flags & TextureUsage::BLIT_DST)) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << "BLIT_DST";
    }
    if (any(flags & TextureUsage::PROTECTED)) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << "PROTECTED";
    }
    return out.str();
}
#endif // FWGPU_ENABLED(FWGPU_DEBUG_BIND_GROUPS)

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_WEBGPUSTRINGS_H
