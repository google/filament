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

#include "WebGPUHandles.h"

#include <backend/DriverEnums.h>

#include <private/backend/BackendUtils.h>
#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace {

wgpu::StringView getUserTextureLabel(filament::backend::SamplerType target) {
    // TODO will be helpful to get more useful info than this
    using filament::backend::SamplerType;
    switch (target) {
        case SamplerType::SAMPLER_2D:
            return "a_2D_user_texture";
        case SamplerType::SAMPLER_2D_ARRAY:
            return "a_2D_array_user_texture";
        case SamplerType::SAMPLER_CUBEMAP:
            return "a_cube_map_user_texture";
        case SamplerType::SAMPLER_EXTERNAL:
            return "an_external_user_texture";
        case SamplerType::SAMPLER_3D:
            return "a_3D_user_texture";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return "a_cube_map_array_user_texture";
    }
}

wgpu::StringView getUserTextureViewLabel(filament::backend::SamplerType target) {
    // TODO will be helpful to get more useful info than this
    using filament::backend::SamplerType;
    switch (target) {
        case SamplerType::SAMPLER_2D:
            return "a_2D_user_texture_view";
        case SamplerType::SAMPLER_2D_ARRAY:
            return "a_2D_array_user_texture_view";
        case SamplerType::SAMPLER_CUBEMAP:
            return "a_cube_map_user_texture_view";
        case SamplerType::SAMPLER_EXTERNAL:
            return "an_external_user_texture_view";
        case SamplerType::SAMPLER_3D:
            return "a_3D_user_texture_view";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return "a_cube_map_array_user_texture_view";
    }
}

}// namespace

namespace filament::backend {

WGPUTexture::WGPUTexture(SamplerType samplerType, uint8_t levels, TextureFormat format,
        uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
        wgpu::Device const& device) noexcept {
    assert_invariant(
            samples == 1 ||
            samples == 4 &&
                    "An invalid number of samples were requested, as WGPU requires the sample "
                    "count to either be 1 (no multisampling) or 4, at least as of April 2025 of "
                    "the spec. See https://www.w3.org/TR/webgpu/#texture-creation or "
                    "https://gpuweb.github.io/gpuweb/#multisample-state");

    mFormat = fToWGPUTextureFormat(format);
    mUsage = fToWGPUTextureUsage(usage);
    mAspect = fToWGPUTextureViewAspect(usage, format);
    mSamplerType = samplerType;
    mBlockWidth = filament::backend::getBlockWidth(format);
    mBlockHeight = filament::backend::getBlockHeight(format);

    wgpu::TextureDescriptor textureDescriptor{
        .label = getUserTextureLabel(samplerType),
        .usage = mUsage,
        .dimension = samplerType == SamplerType::SAMPLER_3D ? wgpu::TextureDimension::e3D
                                                       : wgpu::TextureDimension::e2D,
        .size = { .width = width, .height = height, .depthOrArrayLayers = depth },
        .format = mFormat,
        .mipLevelCount = levels,
        .sampleCount = samples,
        // TODO Is this fine? Could do all-the-things, a naive mapping or get something from
        // Filament
        .viewFormatCount = 0,
        .viewFormats = nullptr,
    };

    switch (samplerType) {
        case SamplerType::SAMPLER_2D:
            mArrayLayerCount = 1;
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            mArrayLayerCount = textureDescriptor.size.depthOrArrayLayers;
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            textureDescriptor.size.depthOrArrayLayers = 6;
            mArrayLayerCount = textureDescriptor.size.depthOrArrayLayers;
            break;
        case SamplerType::SAMPLER_EXTERNAL:
        case SamplerType::SAMPLER_3D:
            mArrayLayerCount = 1;
            break;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            textureDescriptor.size.depthOrArrayLayers = depth * 6;
            mArrayLayerCount = textureDescriptor.size.depthOrArrayLayers;
            break;
    }
    assert_invariant(textureDescriptor.format != wgpu::TextureFormat::Undefined &&
                     "Could not find appropriate WebGPU format");
    mTexture = device.CreateTexture(&textureDescriptor);
    FILAMENT_CHECK_POSTCONDITION(mTexture)
            << "Failed to create texture for " << textureDescriptor.label;

    mDefaultTextureView = makeTextureView(mDefaultMipLevel, levels, mDefaultBaseArrayLayer,
            mArrayLayerCount, samplerType);
}

WGPUTexture::WGPUTexture(WGPUTexture* src, uint8_t baseLevel, uint8_t levelCount) noexcept
    : mTexture(src->mTexture),
      mAspect(src->mAspect),
      mArrayLayerCount(src->mArrayLayerCount),
      mBlockWidth(src->mBlockWidth),
      mBlockHeight(src->mBlockHeight),
      mSamplerType(src->mSamplerType),
      mDefaultTextureView(
              makeTextureView(baseLevel, levelCount, 0, src->mArrayLayerCount, mSamplerType)) {}

wgpu::TextureUsage WGPUTexture::fToWGPUTextureUsage(TextureUsage const& fUsage) {
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

wgpu::TextureFormat WGPUTexture::fToWGPUTextureFormat(TextureFormat const& fFormat) {
    switch (fFormat) {
        case filament::backend::TextureFormat::R8:
            return wgpu::TextureFormat::R8Unorm;
        case filament::backend::TextureFormat::R8_SNORM:
            return wgpu::TextureFormat::R8Snorm;
        case filament::backend::TextureFormat::R8UI:
            return wgpu::TextureFormat::R8Uint;
        case filament::backend::TextureFormat::R8I:
            return wgpu::TextureFormat::R8Sint;
        case filament::backend::TextureFormat::STENCIL8:
            return wgpu::TextureFormat::Stencil8;
        case filament::backend::TextureFormat::R16F:
            return wgpu::TextureFormat::R16Float;
        case filament::backend::TextureFormat::R16UI:
            return wgpu::TextureFormat::R16Uint;
        case filament::backend::TextureFormat::R16I:
            return wgpu::TextureFormat::R16Sint;
        case filament::backend::TextureFormat::RG8:
            return wgpu::TextureFormat::RG8Unorm;
        case filament::backend::TextureFormat::RG8_SNORM:
            return wgpu::TextureFormat::RG8Snorm;
        case filament::backend::TextureFormat::RG8UI:
            return wgpu::TextureFormat::RG8Uint;
        case filament::backend::TextureFormat::RG8I:
            return wgpu::TextureFormat::RG8Sint;
        case filament::backend::TextureFormat::R32F:
            return wgpu::TextureFormat::R32Float;
        case filament::backend::TextureFormat::R32UI:
            return wgpu::TextureFormat::R32Uint;
        case filament::backend::TextureFormat::R32I:
            return wgpu::TextureFormat::R32Sint;
        case filament::backend::TextureFormat::RG16F:
            return wgpu::TextureFormat::RG16Float;
        case filament::backend::TextureFormat::RG16UI:
            return wgpu::TextureFormat::RG16Uint;
        case filament::backend::TextureFormat::RG16I:
            return wgpu::TextureFormat::RG16Sint;
        case filament::backend::TextureFormat::RGBA8:
            return wgpu::TextureFormat::RGBA8Unorm;
        case filament::backend::TextureFormat::SRGB8_A8:
            return wgpu::TextureFormat::RGBA8UnormSrgb;
        case filament::backend::TextureFormat::RGBA8_SNORM:
            return wgpu::TextureFormat::RGBA8Snorm;
        case filament::backend::TextureFormat::RGBA8UI:
            return wgpu::TextureFormat::RGBA8Uint;
        case filament::backend::TextureFormat::RGBA8I:
            return wgpu::TextureFormat::RGBA8Sint;
        case filament::backend::TextureFormat::DEPTH16:
            return wgpu::TextureFormat::Depth16Unorm;
        case filament::backend::TextureFormat::DEPTH24:
            return wgpu::TextureFormat::Depth24Plus;
        case filament::backend::TextureFormat::DEPTH32F:
            return wgpu::TextureFormat::Depth32Float;
        case filament::backend::TextureFormat::DEPTH24_STENCIL8:
            return wgpu::TextureFormat::Depth24PlusStencil8;
        case filament::backend::TextureFormat::DEPTH32F_STENCIL8:
            return wgpu::TextureFormat::Depth32FloatStencil8;
        case filament::backend::TextureFormat::RG32F:
            return wgpu::TextureFormat::RG32Float;
        case filament::backend::TextureFormat::RG32UI:
            return wgpu::TextureFormat::RG32Uint;
        case filament::backend::TextureFormat::RG32I:
            return wgpu::TextureFormat::RG32Sint;
        case filament::backend::TextureFormat::RGBA16F:
            return wgpu::TextureFormat::RGBA16Float;
        case filament::backend::TextureFormat::RGBA16UI:
            return wgpu::TextureFormat::RGBA16Uint;
        case filament::backend::TextureFormat::RGBA16I:
            return wgpu::TextureFormat::RGBA16Sint;
        case filament::backend::TextureFormat::RGBA32F:
            return wgpu::TextureFormat::RGBA32Float;
        case filament::backend::TextureFormat::RGBA32UI:
            return wgpu::TextureFormat::RGBA32Uint;
        case filament::backend::TextureFormat::RGBA32I:
            return wgpu::TextureFormat::RGBA32Sint;
        case filament::backend::TextureFormat::EAC_R11:
            return wgpu::TextureFormat::EACR11Unorm;
        case filament::backend::TextureFormat::EAC_R11_SIGNED:
            return wgpu::TextureFormat::EACR11Snorm;
        case filament::backend::TextureFormat::EAC_RG11:
            return wgpu::TextureFormat::EACRG11Unorm;
        case filament::backend::TextureFormat::EAC_RG11_SIGNED:
            return wgpu::TextureFormat::EACRG11Snorm;
        case filament::backend::TextureFormat::ETC2_RGB8:
            return wgpu::TextureFormat::ETC2RGB8Unorm;
        case filament::backend::TextureFormat::ETC2_SRGB8:
            return wgpu::TextureFormat::ETC2RGB8UnormSrgb;
        case filament::backend::TextureFormat::ETC2_RGB8_A1:
            return wgpu::TextureFormat::ETC2RGB8A1Unorm;
        case filament::backend::TextureFormat::ETC2_SRGB8_A1:
            return wgpu::TextureFormat::ETC2RGB8A1UnormSrgb;
        case filament::backend::TextureFormat::ETC2_EAC_RGBA8:
            return wgpu::TextureFormat::ETC2RGBA8Unorm;
        case filament::backend::TextureFormat::ETC2_EAC_SRGBA8:
            return wgpu::TextureFormat::ETC2RGBA8UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_4x4:
            return wgpu::TextureFormat::ASTC4x4Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
            return wgpu::TextureFormat::ASTC4x4UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_5x4:
            return wgpu::TextureFormat::ASTC5x4Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
            return wgpu::TextureFormat::ASTC5x4UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_5x5:
            return wgpu::TextureFormat::ASTC5x5Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
            return wgpu::TextureFormat::ASTC5x5UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_6x5:
            return wgpu::TextureFormat::ASTC6x5Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
            return wgpu::TextureFormat::ASTC6x5UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_6x6:
            return wgpu::TextureFormat::ASTC6x6Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
            return wgpu::TextureFormat::ASTC6x6UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_8x5:
            return wgpu::TextureFormat::ASTC8x5Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
            return wgpu::TextureFormat::ASTC8x5UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_8x6:
            return wgpu::TextureFormat::ASTC8x6Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
            return wgpu::TextureFormat::ASTC8x6UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_8x8:
            return wgpu::TextureFormat::ASTC8x8Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
            return wgpu::TextureFormat::ASTC8x8UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_10x5:
            return wgpu::TextureFormat::ASTC10x5Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
            return wgpu::TextureFormat::ASTC10x5UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_10x6:
            return wgpu::TextureFormat::ASTC10x6Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
            return wgpu::TextureFormat::ASTC10x6UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_10x8:
            return wgpu::TextureFormat::ASTC10x8Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
            return wgpu::TextureFormat::ASTC10x8UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_10x10:
            return wgpu::TextureFormat::ASTC10x10Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
            return wgpu::TextureFormat::ASTC10x10UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_12x10:
            return wgpu::TextureFormat::ASTC12x10Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
            return wgpu::TextureFormat::ASTC12x10UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_12x12:
            return wgpu::TextureFormat::ASTC12x12Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            return wgpu::TextureFormat::ASTC12x12UnormSrgb;
        case filament::backend::TextureFormat::RED_RGTC1:
            return wgpu::TextureFormat::BC4RUnorm;
        case filament::backend::TextureFormat::SIGNED_RED_RGTC1:
            return wgpu::TextureFormat::BC4RSnorm;
        case filament::backend::TextureFormat::RED_GREEN_RGTC2:
            return wgpu::TextureFormat::BC5RGUnorm;
        case filament::backend::TextureFormat::SIGNED_RED_GREEN_RGTC2:
            return wgpu::TextureFormat::BC5RGSnorm;
        case filament::backend::TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:
            return wgpu::TextureFormat::BC6HRGBUfloat;
        case filament::backend::TextureFormat::RGB_BPTC_SIGNED_FLOAT:
            return wgpu::TextureFormat::BC6HRGBFloat;
        case filament::backend::TextureFormat::RGBA_BPTC_UNORM:
            return wgpu::TextureFormat::BC7RGBAUnorm;
        case filament::backend::TextureFormat::SRGB_ALPHA_BPTC_UNORM:
            return wgpu::TextureFormat::BC7RGBAUnormSrgb;
        case filament::backend::TextureFormat::RGB565:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and discard the alpha and lower precision.
            return wgpu::TextureFormat::Undefined;
        case filament::backend::TextureFormat::RGB9_E5:
            return wgpu::TextureFormat::RGB9E5Ufloat;
        case filament::backend::TextureFormat::RGB5_A1:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and handle the packing/unpacking in shaders.
            return wgpu::TextureFormat::Undefined;
        case filament::backend::TextureFormat::RGBA4:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and handle the packing/unpacking in shaders.
            return wgpu::TextureFormat::Undefined;
        case filament::backend::TextureFormat::RGB8:
            // No direct sRGB equivalent in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8Unorm;
        case filament::backend::TextureFormat::SRGB8:
            // No direct sRGB equivalent in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8UnormSrgb;
        case filament::backend::TextureFormat::RGB8_SNORM:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8Snorm;
        case filament::backend::TextureFormat::RGB8UI:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8Uint;
        case filament::backend::TextureFormat::RGB8I:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8Sint;
        case filament::backend::TextureFormat::R11F_G11F_B10F:
            return wgpu::TextureFormat::RG11B10Ufloat;
        case filament::backend::TextureFormat::UNUSED:
            return wgpu::TextureFormat::Undefined;
        case filament::backend::TextureFormat::RGB10_A2:
            return wgpu::TextureFormat::RGB10A2Unorm;
        case filament::backend::TextureFormat::RGB16F:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA16Float;
        case filament::backend::TextureFormat::RGB16UI:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA16Uint;
        case filament::backend::TextureFormat::RGB16I:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA16Sint;
        case filament::backend::TextureFormat::RGB32F:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA32Float;
        case filament::backend::TextureFormat::RGB32UI:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA32Uint;
        case filament::backend::TextureFormat::RGB32I:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA32Sint;
        case filament::backend::TextureFormat::DXT1_RGB:
            return wgpu::TextureFormat::BC1RGBAUnorm;
        case filament::backend::TextureFormat::DXT1_RGBA:
            return wgpu::TextureFormat::BC1RGBAUnorm;
        case filament::backend::TextureFormat::DXT3_RGBA:
            return wgpu::TextureFormat::BC2RGBAUnorm;
        case filament::backend::TextureFormat::DXT5_RGBA:
            return wgpu::TextureFormat::BC3RGBAUnorm;
        case filament::backend::TextureFormat::DXT1_SRGB:
            return wgpu::TextureFormat::BC1RGBAUnormSrgb;
        case filament::backend::TextureFormat::DXT1_SRGBA:
            return wgpu::TextureFormat::BC1RGBAUnormSrgb;
        case filament::backend::TextureFormat::DXT3_SRGBA:
            return wgpu::TextureFormat::BC2RGBAUnormSrgb;
        case filament::backend::TextureFormat::DXT5_SRGBA:
            return wgpu::TextureFormat::BC3RGBAUnormSrgb;
    }
}

wgpu::TextureAspect WGPUTexture::fToWGPUTextureViewAspect(TextureUsage const& fUsage,
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

wgpu::TextureView WGPUTexture::getOrMakeTextureView(uint8_t mipLevel, uint32_t arrayLayer) const {
    //TODO: there's an optimization to be made here to return mDefaultTextureView.
    // Problem: mDefaultTextureView is a view of the entire texture,
    // but this function (and its callers) expects a single-slice view.
    // Returning the whole texture view for a single-slice request seems wrong.

    return makeTextureView(mipLevel, 1, arrayLayer, 1, mSamplerType);
}

wgpu::TextureView WGPUTexture::makeTextureView(const uint8_t& baseLevel, const uint8_t& levelCount,
        const uint32_t& baseArrayLayer, const uint32_t& arrayLayerCount,
        SamplerType samplerType) const noexcept{

    wgpu::TextureViewDescriptor textureViewDescriptor{
        .label = getUserTextureViewLabel(target),
        .format = mFormat,
        .baseMipLevel = baseLevel,
        .mipLevelCount = levelCount,
        .baseArrayLayer = baseArrayLayer,
        .arrayLayerCount = arrayLayerCount,
        .aspect = mAspect,
        .usage = mUsage
    };

    switch (samplerType) {
        case SamplerType::SAMPLER_2D:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2DArray;
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::Cube;
            break;
        case SamplerType::SAMPLER_EXTERNAL:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
            break;
        case SamplerType::SAMPLER_3D:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e3D;
            break;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::CubeArray;
            break;
    }
    wgpu::TextureView textureView = mTexture.CreateView(&textureViewDescriptor);
    FILAMENT_CHECK_POSTCONDITION(textureView)
            << "Failed to create texture view " << textureViewDescriptor.label;
    return textureView;
}

WGPURenderTarget::WGPURenderTarget(uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount,
        const MRT& colorAttachmentsMRT,
        const Attachment& depthAttachmentInfo,
        const Attachment& stencilAttachmentInfo)
    : HwRenderTarget(width, height),
      mDefaultRenderTarget(false),
      mSamples(samples),
      mLayerCount(layerCount),
      mColorAttachments(colorAttachmentsMRT),
      mDepthAttachment(depthAttachmentInfo),
      mStencilAttachment(stencilAttachmentInfo) {
    // TODO Make this an array
    mColorAttachmentDescriptors.reserve(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
}

wgpu::LoadOp WGPURenderTarget::getLoadOperation(RenderPassParams const& params,
                                                TargetBufferFlags bufferToOperateOn) {
    if (any(params.flags.clear & bufferToOperateOn)) {
        return wgpu::LoadOp::Clear;
    }
    if (any(params.flags.discardStart & bufferToOperateOn)) {
        return wgpu::LoadOp::Clear; // Or wgpu::LoadOp::Undefined if clear is not desired on discard
    }
    return wgpu::LoadOp::Load;
}

wgpu::StoreOp WGPURenderTarget::getStoreOperation(RenderPassParams const& params,
                                                  TargetBufferFlags bufferToOperateOn) {
    if (any(params.flags.discardEnd & bufferToOperateOn)) {
        return wgpu::StoreOp::Discard;
    }
    return wgpu::StoreOp::Store;
}

void WGPURenderTarget::setUpRenderPassAttachments(wgpu::RenderPassDescriptor& descriptor,
        RenderPassParams const& params, wgpu::TextureView const& defaultColorTextureView,
        wgpu::TextureView const& defaultDepthStencilTextureView,
        wgpu::TextureView const* customColorTextureViews, uint32_t customColorTextureViewCount,
        wgpu::TextureView const& customDepthTextureView,
        wgpu::TextureView const& customStencilTextureView, wgpu::TextureFormat customDepthFormat,
        wgpu::TextureFormat customStencilFormat) {
    mColorAttachmentDescriptors.clear();
    mHasDepthStencilAttachment = false;

    if (mDefaultRenderTarget) {
        assert_invariant(defaultColorTextureView);
        mColorAttachmentDescriptors.push_back({ .view = defaultColorTextureView,
            .resolveTarget = nullptr,
            .loadOp = WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::COLOR0),
            .storeOp = WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::COLOR0),
            .clearValue = { params.clearColor.r, params.clearColor.g, params.clearColor.b,
                params.clearColor.a } });

        if (defaultDepthStencilTextureView) {
            mDepthStencilAttachmentDescriptor = {
                .view = defaultDepthStencilTextureView,
                .depthLoadOp = WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::DEPTH),
                .depthStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::DEPTH),
                .depthClearValue = static_cast<float>(params.clearDepth),
                .depthReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0,
                .stencilLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::STENCIL),
                .stencilStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::STENCIL),
                .stencilClearValue = params.clearStencil,
                .stencilReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0,
            };
            mHasDepthStencilAttachment = true;
        }
    } else {// Custom Render Target
        for (uint32_t i = 0; i < customColorTextureViewCount; ++i) {
            if (customColorTextureViews[i]) {
                mColorAttachmentDescriptors.push_back({ .view = customColorTextureViews[i],
                    // .resolveTarget = nullptr; // TODO: MSAA resolve for custom RT
                    .loadOp = WGPURenderTarget::getLoadOperation(params, getTargetBufferFlagsAt(i)),
                    .storeOp =
                            WGPURenderTarget::getStoreOperation(params, getTargetBufferFlagsAt(i)),
                    .clearValue = { .r = params.clearColor.r,
                        .g = params.clearColor.g,
                        .b = params.clearColor.b,
                        .a = params.clearColor.a } });
            }
        }

        FILAMENT_CHECK_POSTCONDITION(!(customDepthTextureView && customStencilTextureView))
                << "WebGPU CANNOT support separate texture views for depth + stencil. depth + "
                   "stencil needs to be in one texture view";

        const bool hasStencil =
                customStencilTextureView ||
                (customDepthFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
                        customDepthFormat == wgpu::TextureFormat::Depth32FloatStencil8);

        const bool hasDepth =
                customDepthTextureView ||
                (customStencilFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
                        customDepthFormat == wgpu::TextureFormat::Depth32FloatStencil8);

        if (customDepthTextureView || customStencilTextureView) {
            assert_invariant((hasDepth || hasStencil) &&
                             "Depth or Texture view without a valid texture format");
            mDepthStencilAttachmentDescriptor = {};
            mDepthStencilAttachmentDescriptor.view =
                    customDepthTextureView ? customDepthTextureView : customStencilTextureView;

            if (hasDepth) {
                mDepthStencilAttachmentDescriptor.depthLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDescriptor.depthStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDescriptor.depthClearValue =
                        static_cast<float>(params.clearDepth);
                mDepthStencilAttachmentDescriptor.depthReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0;
            } else {
                mDepthStencilAttachmentDescriptor.depthLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDescriptor.depthStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDescriptor.depthReadOnly = true;
            }

            if (hasStencil) {
                mDepthStencilAttachmentDescriptor.stencilLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDescriptor.stencilStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDescriptor.stencilClearValue = params.clearStencil;
                mDepthStencilAttachmentDescriptor.stencilReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0;
            } else {
                mDepthStencilAttachmentDescriptor.stencilLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDescriptor.stencilStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDescriptor.stencilReadOnly = true;
            }
            mHasDepthStencilAttachment = true;
        }
    }

    descriptor.colorAttachmentCount = mColorAttachmentDescriptors.size();
    descriptor.colorAttachments = mColorAttachmentDescriptors.data();
    descriptor.depthStencilAttachment =
            mHasDepthStencilAttachment ? &mDepthStencilAttachmentDescriptor : nullptr;

    // descriptor.sampleCount was removed from the core spec. If your webgpu.h still has it,
    // and your Dawn version expects it, you might need to set it here based on this->samples.
    // e.g., descriptor.sampleCount = this->samples;
}

}// namespace filament::backend
