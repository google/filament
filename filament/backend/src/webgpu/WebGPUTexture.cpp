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

#include "WebGPUTexture.h"

#include "WebGPUConstants.h"

#include "DriverBase.h"
#include "private/backend/BackendUtils.h"
#include <backend/DriverEnums.h>

#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

namespace {

[[nodiscard]] constexpr wgpu::StringView getUserTextureLabel(const SamplerType target) {
    // TODO will be helpful to get more useful info than this
    switch (target) {
        case SamplerType::SAMPLER_2D:            return "a_2D_user_texture";
        case SamplerType::SAMPLER_2D_ARRAY:      return "a_2D_array_user_texture";
        case SamplerType::SAMPLER_CUBEMAP:       return "a_cube_map_user_texture";
        case SamplerType::SAMPLER_EXTERNAL:      return "an_external_user_texture";
        case SamplerType::SAMPLER_3D:            return "a_3D_user_texture";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY: return "a_cube_map_array_user_texture";
    }
}

[[nodiscard]] constexpr wgpu::StringView getUserTextureViewLabel(const SamplerType target) {
    // TODO will be helpful to get more useful info than this
    switch (target) {
        case SamplerType::SAMPLER_2D:            return "a_2D_user_texture_view";
        case SamplerType::SAMPLER_2D_ARRAY:      return "a_2D_array_user_texture_view";
        case SamplerType::SAMPLER_CUBEMAP:       return "a_cube_map_user_texture_view";
        case SamplerType::SAMPLER_EXTERNAL:      return "an_external_user_texture_view";
        case SamplerType::SAMPLER_3D:            return "a_3D_user_texture_view";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY: return "a_cube_map_array_user_texture_view";
    }
}

[[nodiscard]] wgpu::TextureUsage fToWGPUTextureUsage(TextureUsage const& fUsage,
        const uint8_t samples) {
    wgpu::TextureUsage retUsage = wgpu::TextureUsage::None;

    // Basing this mapping off of VulkanTexture.cpp's getUsage func and suggestions from Gemini
    // TODO Validate assumptions, revisit if issues.
    if (any(TextureUsage::BLIT_SRC & fUsage)) {
        retUsage |= wgpu::TextureUsage::CopySrc;
    }
    if (any((TextureUsage::BLIT_DST | TextureUsage::UPLOADABLE) & fUsage)) {
        retUsage |= wgpu::TextureUsage::CopyDst;
    }
    if (any(TextureUsage::SAMPLEABLE & fUsage)) {
        retUsage |= wgpu::TextureUsage::TextureBinding;
    }
    // WGPU Render attachment covers either color or stencil situation dependant
    // NOTE: Depth attachment isn't used this way in Vulkan but logically maps to WGPU docs. If
    // issues, investigate here.
    if (any((TextureUsage::COLOR_ATTACHMENT | TextureUsage::STENCIL_ATTACHMENT |
                    TextureUsage::DEPTH_ATTACHMENT) &
                fUsage)) {
        retUsage |= wgpu::TextureUsage::RenderAttachment;
    }

    // This is from Vulkan logic- if there are any issues try disabling this first, allows perf
    // benefit though
    const bool useTransientAttachment =
            // Usage consists of attachment flags only.
            none(fUsage & ~TextureUsage::ALL_ATTACHMENTS) &&
            // Usage contains at least one attachment flag.
            any(fUsage & TextureUsage::ALL_ATTACHMENTS) &&
            // Depth resolve cannot use transient attachment because it uses a custom shader.
            // TODO: see VulkanDriver::isDepthStencilResolveSupported() to know when to remove this
            // restriction.
            // Note that the custom shader does not resolve stencil. We do need to move to vk 1.2
            // and above to be able to support stencil resolve (along with depth).
            !(any(fUsage & TextureUsage::DEPTH_ATTACHMENT) && samples > 1);
    if (useTransientAttachment) {
        retUsage |= wgpu::TextureUsage::TransientAttachment;
    }
    // NOTE: Unused wgpu flags:
    //  StorageBinding
    //  StorageAttachment

    // NOTE: Unused Filament flags:
    //  SUBPASS_INPUT VK goes to input attachment which we don't support right now
    //  PROTECTED
    return retUsage;
}

[[nodiscard]] wgpu::TextureAspect fToWGPUTextureViewAspect(TextureUsage const& fUsage,
        TextureFormat const& fFormat) {

    const bool isDepth = any(fUsage & TextureUsage::DEPTH_ATTACHMENT);
    const bool isStencil = any(fUsage & TextureUsage::STENCIL_ATTACHMENT);
    const bool isColor = any(fUsage & TextureUsage::COLOR_ATTACHMENT);
    const bool isSample = any(fUsage & TextureUsage::SAMPLEABLE);

    if (isDepth && !isColor && !isStencil) {
        return wgpu::TextureAspect::DepthOnly;
    }

    if (isStencil && !isColor && !isDepth) {
        return wgpu::TextureAspect::StencilOnly;
    }

    if (fFormat == filament::backend::TextureFormat::DEPTH32F ||
            fFormat == filament::backend::TextureFormat::DEPTH24 ||
            fFormat == filament::backend::TextureFormat::DEPTH16) {
        return wgpu::TextureAspect::DepthOnly;
    }

    if (fFormat == filament::backend::TextureFormat::STENCIL8) {
        return wgpu::TextureAspect::StencilOnly;
    }

    if (fFormat == filament::backend::TextureFormat::DEPTH24_STENCIL8 ||
            fFormat == filament::backend::TextureFormat::DEPTH32F_STENCIL8) {
        if (isSample) {
            return wgpu::TextureAspect::DepthOnly;
        }
    }

    return wgpu::TextureAspect::All;
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

[[nodiscard]] constexpr wgpu::Extent3D toTextureSize(const SamplerType samplerType,
        const uint32_t width, const uint32_t height, const uint32_t depth) {
    switch (samplerType) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_2D_ARRAY:
        case SamplerType::SAMPLER_EXTERNAL:
        case SamplerType::SAMPLER_3D:
            return { .width = width, .height = height, .depthOrArrayLayers = depth };
        case SamplerType::SAMPLER_CUBEMAP:
            return { .width = width, .height = height, .depthOrArrayLayers = 6 };
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return { .width = width, .height = height, .depthOrArrayLayers = depth * 6 };
    }
}

[[nodiscard]] constexpr uint32_t toArrayLayerCount(const SamplerType samplerType,
        const uint32_t depthOrArrayLayers) {
    switch (samplerType) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_EXTERNAL:
        case SamplerType::SAMPLER_3D:
            return 1;
        case SamplerType::SAMPLER_2D_ARRAY:
        case SamplerType::SAMPLER_CUBEMAP:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return depthOrArrayLayers;
    }
}

} // namespace

WebGPUTexture::WebGPUTexture(const SamplerType samplerType, const uint8_t levels,
        const TextureFormat format, const uint8_t samples, const uint32_t width,
        const uint32_t height, const uint32_t depth, const TextureUsage usage,
        wgpu::Device const& device) noexcept
    : HwTexture{ samplerType, levels, samples, width, height, depth, format, usage },
      mWebGPUFormat{ fToWGPUTextureFormat(format) },
      mAspect{ fToWGPUTextureViewAspect(usage, format) },
      mWebGPUUsage{ fToWGPUTextureUsage(usage, samples) },
      mBlockWidth{ filament::backend::getBlockWidth(format) },
      mBlockHeight{ filament::backend::getBlockHeight(format) } {
    assert_invariant(
            samples == 1 ||
            samples == 4 &&
                    "An invalid number of samples were requested, as WGPU requires the sample "
                    "count to either be 1 (no multisampling) or 4, at least as of April 2025 of "
                    "the spec. See https://www.w3.org/TR/webgpu/#texture-creation or "
                    "https://gpuweb.github.io/gpuweb/#multisample-state");
    const wgpu::TextureDescriptor textureDescriptor{
        .label = getUserTextureLabel(samplerType),
        .usage = mWebGPUUsage,
        .dimension = toWebGPUTextureDimension(samplerType),
        .size = toTextureSize(samplerType, width, height, depth),
        .format = mWebGPUFormat,
        .mipLevelCount = levels,
        .sampleCount = samples,
        // TODO Is this fine? Could do all-the-things, a naive mapping or get something from
        // Filament
        .viewFormatCount = 0,
        .viewFormats = nullptr,
    };
    mArrayLayerCount = toArrayLayerCount(samplerType, textureDescriptor.size.depthOrArrayLayers);
    assert_invariant(textureDescriptor.format != wgpu::TextureFormat::Undefined &&
                     "Could not find appropriate WebGPU format");
    mTexture = device.CreateTexture(&textureDescriptor);
    FILAMENT_CHECK_POSTCONDITION(mTexture)
            << "Failed to create texture for " << textureDescriptor.label;
    mDefaultTextureView = makeTextureView(mDefaultMipLevel, levels, mDefaultBaseArrayLayer,
            mArrayLayerCount, samplerType);
    FILAMENT_CHECK_POSTCONDITION(mDefaultTextureView)
            << "Failed to create default texture view for " << textureDescriptor.label;
}

WebGPUTexture::WebGPUTexture(WebGPUTexture const* src, const uint8_t baseLevel,
        const uint8_t levelCount) noexcept
    : HwTexture{ src->target, levelCount, src->samples, src->width, src->height, src->depth,
          src->format, src->usage },
      mTexture{ src->mTexture },
      mWebGPUFormat{ src->mWebGPUFormat },
      mAspect{ src->mAspect },
      mWebGPUUsage{ src->mWebGPUUsage },
      mArrayLayerCount{ src->mArrayLayerCount },
      mBlockWidth{ src->mBlockWidth },
      mBlockHeight{ src->mBlockHeight },
      mDefaultTextureView{ makeTextureView(baseLevel, levelCount, 0, src->mArrayLayerCount,
              src->target) },
      mDefaultMipLevel{ baseLevel },
      mDefaultBaseArrayLayer{ src->mArrayLayerCount } {}

wgpu::TextureView WebGPUTexture::getOrMakeTextureView(const uint8_t mipLevel,
        const uint32_t arrayLayer) {
    // TODO: there's an optimization to be made here to return mDefaultTextureView.
    //  Problem: mDefaultTextureView is a view of the entire texture,
    //  but this function (and its callers) expects a single-slice view.
    //  Returning the whole texture view for a single-slice request seems wrong.
    return makeTextureView(mipLevel, 1, arrayLayer, 1, target);
}

wgpu::TextureFormat WebGPUTexture::fToWGPUTextureFormat(TextureFormat const& fFormat) {
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
                          "wgpu::TextureFormat::Undefined (no direct mapping in wgpu)"
                       << utils::io::endl;
            return wgpu::TextureFormat::Undefined;
        case TextureFormat::RGB9_E5: return wgpu::TextureFormat::RGB9E5Ufloat;
        case TextureFormat::RGB5_A1:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and handle the packing/unpacking in shaders.
            FWGPU_LOGW << "Requested Filament texture format RGB5_A1 but getting "
                          "wgpu::TextureFormat::Undefined (no direct mapping in wgpu)"
                       << utils::io::endl;
            return wgpu::TextureFormat::Undefined;
        case TextureFormat::RGBA4:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and handle the packing/unpacking in shaders.
            FWGPU_LOGW << "Requested Filament texture format RGBA4 but getting "
                          "wgpu::TextureFormat::Undefined (no direct mapping in wgpu)"
                       << utils::io::endl;
            return wgpu::TextureFormat::Undefined;
        case TextureFormat::RGB8:
            FWGPU_LOGW << "Requested Filament texture format RGB8 but getting "
                          "wgpu::TextureFormat::RGBA8Unorm (no direct sRGB equivalent in wgpu "
                          "without alpha)"
                       << utils::io::endl;
            return wgpu::TextureFormat::RGBA8Unorm;
        case TextureFormat::SRGB8:
            FWGPU_LOGW << "Requested Filament texture format SRGB8 but getting "
                          "wgpu::TextureFormat::RGBA8UnormSrgb (no direct sRGB equivalent in wgpu "
                          "without alpha)"
                       << utils::io::endl;
            return wgpu::TextureFormat::RGBA8UnormSrgb;
        case TextureFormat::RGB8_SNORM:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB8_SNORM but getting "
                       "wgpu::TextureFormat::RGBA8Snorm (no direct mapping in wgpu without alpha)"
                    << utils::io::endl;
            return wgpu::TextureFormat::RGBA8Snorm;
        case TextureFormat::RGB8UI:
            FWGPU_LOGW << "Requested Filament texture format RGB8UI but getting "
                          "wgpu::TextureFormat::RGBA8Uint (no direct mapping in wgpu without alpha)"
                       << utils::io::endl;
            return wgpu::TextureFormat::RGBA8Uint;
        case TextureFormat::RGB8I:
            FWGPU_LOGW << "Requested Filament texture format RGB8I but getting "
                          "wgpu::TextureFormat::RGBA8Sint (no direct mapping in wgpu without alpha)"
                       << utils::io::endl;
            return wgpu::TextureFormat::RGBA8Sint;
        case TextureFormat::R11F_G11F_B10F:          return wgpu::TextureFormat::RG11B10Ufloat;
        case TextureFormat::UNUSED:                  return wgpu::TextureFormat::Undefined;
        case TextureFormat::RGB10_A2:                return wgpu::TextureFormat::RGB10A2Unorm;
        case TextureFormat::RGB16F:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB16F but getting "
                       "wgpu::TextureFormat::RGBA16Float (no direct mapping in wgpu without alpha)"
                    << utils::io::endl;
            return wgpu::TextureFormat::RGBA16Float;
        case TextureFormat::RGB16UI:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB16UI but getting "
                       "wgpu::TextureFormat::RGBA16Uint (no direct mapping in wgpu without alpha)"
                    << utils::io::endl;
            return wgpu::TextureFormat::RGBA16Uint;
        case TextureFormat::RGB16I:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB16I but getting "
                       "wgpu::TextureFormat::RGBA16Sint (no direct mapping in wgpu without alpha)"
                    << utils::io::endl;
            return wgpu::TextureFormat::RGBA16Sint;
        case TextureFormat::RGB32F:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB32F but getting "
                       "wgpu::TextureFormat::RGBA32Float (no direct mapping in wgpu without alpha)"
                    << utils::io::endl;
            return wgpu::TextureFormat::RGBA32Float;
        case TextureFormat::RGB32UI:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB32UI but getting "
                       "wgpu::TextureFormat::RGBA32Uint (no direct mapping in wgpu without alpha)"
                    << utils::io::endl;
            return wgpu::TextureFormat::RGBA32Uint;
        case TextureFormat::RGB32I:
            FWGPU_LOGW
                    << "Requested Filament texture format RGB32I but getting "
                       "wgpu::TextureFormat::RGBA32Sint (no direct mapping in wgpu without alpha)"
                    << utils::io::endl;
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

wgpu::TextureView WebGPUTexture::makeTextureView(const uint8_t& baseLevel,
        const uint8_t& levelCount, const uint32_t& baseArrayLayer, const uint32_t& arrayLayerCount,
        const SamplerType samplerType) const noexcept {
    const wgpu::TextureViewDescriptor textureViewDescriptor{
        .label = getUserTextureViewLabel(target),
        .format = mWebGPUFormat,
        .dimension = toWebGPUTextureViewDimension(samplerType),
        .baseMipLevel = baseLevel,
        .mipLevelCount = levelCount,
        .baseArrayLayer = baseArrayLayer,
        .arrayLayerCount = arrayLayerCount,
        .aspect = mAspect,
        .usage = mWebGPUUsage };
    wgpu::TextureView textureView = mTexture.CreateView(&textureViewDescriptor);
    FILAMENT_CHECK_POSTCONDITION(textureView)
            << "Failed to create texture view " << textureViewDescriptor.label;
    return textureView;
}

}// namespace filament::backend
