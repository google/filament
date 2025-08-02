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
#include "WebGPURenderPassMipmapGenerator.h"
#include "WebGPUStrings.h"
#include "WebGPUTextureHelpers.h"

#include "DriverBase.h"
#include "private/backend/BackendUtils.h"
#include <backend/DriverEnums.h>

#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
#include <webgpu/webgpu_cpp_print.h>
#include <sstream>
#endif

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

/**
 * @param viewFormat potential view format to a texture
 * @return an underlying texture format that is storage binding compatible for which this view
 *         texture format can be used. If no such format exists, Undefined is returned.
 *         If the view texture is already storage binding compatible then the view texture itself
 *         is returned.
 */
[[nodiscard]] constexpr wgpu::TextureFormat storageBindingCompatibleFormatForViewFormat(
        const wgpu::TextureFormat viewFormat) {
    switch (viewFormat) {
        case wgpu::TextureFormat::RGBA8UnormSrgb:      return wgpu::TextureFormat::RGBA8Unorm;
        case wgpu::TextureFormat::BGRA8UnormSrgb:      return wgpu::TextureFormat::BGRA8Unorm;
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:    return wgpu::TextureFormat::BC1RGBAUnorm;
        case wgpu::TextureFormat::BC2RGBAUnormSrgb:    return wgpu::TextureFormat::BC2RGBAUnorm;
        case wgpu::TextureFormat::BC3RGBAUnormSrgb:    return wgpu::TextureFormat::BC3RGBAUnorm;
        case wgpu::TextureFormat::BC7RGBAUnormSrgb:    return wgpu::TextureFormat::BC7RGBAUnorm;
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:   return wgpu::TextureFormat::ETC2RGB8Unorm;
        case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb: return wgpu::TextureFormat::ETC2RGB8A1Unorm;
        case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:  return wgpu::TextureFormat::ETC2RGBA8Unorm;
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
        default:
            if (isFormatStorageCompatible(viewFormat)) {
                return viewFormat; // view format is itself already storage binding compatible
            } else {
                return wgpu::TextureFormat::Undefined; // no valid format in this case
            }
    }
}

/**
 * @param fUsage Filament's requested texture usage
 * @param samples How many samples to use for MSAA
 * @param needsComputeStorageSupport if we need to use this texture as storage binding in something
 *                                   like a compute shader
 * @param needsRenderAttachmentSupport if we need to use this texture as a render pass attachment
 *                                     in something like a render pass blit (e.g. mipmap generation)
 * @return The appropriate texture usage flags for the underlying texture
 */
[[nodiscard]] wgpu::TextureUsage fToWGPUTextureUsage(TextureUsage const& fUsage,
        const uint8_t samples, const bool needsComputeStorageSupport,
        const bool needsRenderAttachmentSupport) {
    wgpu::TextureUsage retUsage = wgpu::TextureUsage::None;

    // if needsComputeStorageSupport we need to read and write to the texture in a shader and thus
    // require CopySrc, CopyDst, TextureBinding, & StorageBinding

    // Basing this mapping off of VulkanTexture.cpp's getUsage func and suggestions from Gemini
    // TODO Validate assumptions, revisit if issues.
    if (needsComputeStorageSupport || any(TextureUsage::BLIT_SRC & fUsage)) {
        retUsage |= wgpu::TextureUsage::CopySrc;
    }
    if (needsComputeStorageSupport ||
            any((TextureUsage::BLIT_DST | TextureUsage::UPLOADABLE) & fUsage)) {
        retUsage |= wgpu::TextureUsage::CopyDst;
    }
    if (needsComputeStorageSupport || any(TextureUsage::SAMPLEABLE & fUsage)) {
        retUsage |= wgpu::TextureUsage::TextureBinding;
    }
    if (needsComputeStorageSupport) {
        retUsage |= wgpu::TextureUsage::StorageBinding;
    }
    if (needsRenderAttachmentSupport) {
        retUsage |= wgpu::TextureUsage::RenderAttachment;
    }
    if (any(TextureUsage::BLIT_SRC & fUsage)) {
        retUsage |= wgpu::TextureUsage::RenderAttachment;
    }
    if (any(TextureUsage::BLIT_DST & fUsage)) {
        retUsage |= wgpu::TextureUsage::RenderAttachment;
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

[[nodiscard]] WebGPUTexture::MipmapGenerationStrategy determineMipmapGenerationStrategy(
        const wgpu::TextureFormat format, const SamplerType samplerType, const uint8_t sampleCount,
        const uint8_t mipmapLevels) {
    if (mipmapLevels <= 1) {
        return WebGPUTexture::MipmapGenerationStrategy::NONE; // no mipmap generation needed
    }
    const WebGPURenderPassMipmapGenerator::FormatCompatibility renderPassCompatibility{
        WebGPURenderPassMipmapGenerator::getCompatibilityFor(format,
                toWebGPUTextureDimension(samplerType), sampleCount)
    };
    if (renderPassCompatibility.compatible) {
        return WebGPUTexture::MipmapGenerationStrategy::RENDER_PASS;
    }
    if (WebGPUTexture::supportsMultipleMipLevelsViaStorageBinding(format)) {
        return WebGPUTexture::MipmapGenerationStrategy::SPD_COMPUTE_PASS;
    }
    // we don't have a way to generate mipmaps for this texture, at least at this time
    return WebGPUTexture::MipmapGenerationStrategy::NONE;
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
      mViewFormat{ toWGPUTextureFormat(format) },
      mMipmapGenerationStrategy{ determineMipmapGenerationStrategy(mViewFormat, samplerType,
              samples, levels) },
      mWebGPUFormat{ mMipmapGenerationStrategy == MipmapGenerationStrategy::SPD_COMPUTE_PASS
                             ? storageBindingCompatibleFormatForViewFormat(mViewFormat)
                             : mViewFormat },
      mAspect{ fToWGPUTextureViewAspect(usage, format) },
      mWebGPUUsage{ fToWGPUTextureUsage(usage, samples,
              mMipmapGenerationStrategy == MipmapGenerationStrategy::SPD_COMPUTE_PASS,
              mMipmapGenerationStrategy == MipmapGenerationStrategy::RENDER_PASS) },
      mViewUsage{ fToWGPUTextureUsage(usage, samples, false, false) },
      mDimension{toWebGPUTextureViewDimension(samplerType)},
      mBlockWidth{ filament::backend::getBlockWidth(format) },
      mBlockHeight{ filament::backend::getBlockHeight(format) },
      mDefaultMipLevel{ 0 },
      mDefaultBaseArrayLayer{ 0 } {
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
        .viewFormatCount = 1,
        .viewFormats = &mViewFormat,
    };
    mArrayLayerCount = toArrayLayerCount(samplerType, textureDescriptor.size.depthOrArrayLayers);
    assert_invariant(textureDescriptor.format != wgpu::TextureFormat::Undefined &&
                     "Could not find appropriate WebGPU format");
    mTexture = device.CreateTexture(&textureDescriptor);
    FILAMENT_CHECK_POSTCONDITION(mTexture)
            << "Failed to create texture for " << textureDescriptor.label;
    mDefaultTextureView = makeTextureView(mDefaultMipLevel, levels, mDefaultBaseArrayLayer,
            mArrayLayerCount, toWebGPUTextureViewDimension(samplerType));
    FILAMENT_CHECK_POSTCONDITION(mDefaultTextureView)
            << "Failed to create default texture view for " << textureDescriptor.label;
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    if (mViewFormat != mWebGPUFormat) {
        std::stringstream viewFormatStream;
        viewFormatStream << mViewFormat;
        std::stringstream textureFormatStream;
        textureFormatStream << mWebGPUFormat;
        FWGPU_LOGD << "Texture '" << textureDescriptor.label << "' has view format "
                   << viewFormatStream.str() << " and texture format " << textureFormatStream.str();
    }
#endif
}

WebGPUTexture::WebGPUTexture(WebGPUTexture const* src, const uint8_t baseLevel,
        const uint8_t levelCount) noexcept
    : HwTexture{ src->target, levelCount, src->samples, src->width, src->height, src->depth,
          src->format, src->usage },
      mViewFormat{ src->mViewFormat },
      mMipmapGenerationStrategy{ src->mMipmapGenerationStrategy },
      mWebGPUFormat{ src->mWebGPUFormat },
      mAspect{ src->mAspect },
      mWebGPUUsage{ src->mWebGPUUsage },
      mViewUsage{ src->mViewUsage },
      mBlockWidth{ src->mBlockWidth },
      mBlockHeight{ src->mBlockHeight },
      mArrayLayerCount{ src->mArrayLayerCount },
      mTexture{ src->mTexture },
      mDefaultMipLevel{ baseLevel },
      mDefaultBaseArrayLayer{ src->mArrayLayerCount },
      mDefaultTextureView{ makeTextureView(mDefaultMipLevel, levelCount, 0, mDefaultBaseArrayLayer,
              toWebGPUTextureViewDimension(src->target)) },
      mMsaaSidecarTexture{ src->mMsaaSidecarTexture } {}

wgpu::Texture const& WebGPUTexture::getMsaaSidecarTexture(const uint8_t sampleCount) const {
    if (mMsaaSidecarTexture == nullptr) {
        return mMsaaSidecarTexture; // nullptr (no such sidecar)
    }
    FILAMENT_CHECK_PRECONDITION(sampleCount == mMsaaSidecarTexture.GetSampleCount())
            << "The MSAA sidecar texture has a different sample count ("
            << mMsaaSidecarTexture.GetSampleCount() << ") than requested (" << +sampleCount
            << "). Note that this restriction was written when WebGPU only supported msaa "
               "textures with 4 samples. If that has changed, this implementation should be "
               "updated (e.g. map of sidecar textures by sampleCount or something).";
    return mMsaaSidecarTexture;
}

bool WebGPUTexture::supportsMultipleMipLevelsViaStorageBinding(const wgpu::TextureFormat format) {
    return storageBindingCompatibleFormatForViewFormat(format) != wgpu::TextureFormat::Undefined;
}

wgpu::TextureView WebGPUTexture::makeAttachmentTextureView(const uint8_t mipLevel,
        const uint32_t arrayLayer, const uint32_t layerCount) {
    // TODO: there's an optimization to be made here to return mDefaultTextureView.
    //  Problem: mDefaultTextureView is a view of the entire texture,
    //  but this function (and its callers) expects a single-slice view.
    //  Returning the whole texture view for a single-slice request seems wrong.
    return makeTextureView(mipLevel, 1, arrayLayer, layerCount,
            layerCount > 1 ? wgpu::TextureViewDimension::e2DArray
                           : wgpu::TextureViewDimension::e2D);
}

void WebGPUTexture::createMsaaSidecarTextureIfNotAlreadyCreated(const uint8_t samples,
        wgpu::Device const& device) {
    FILAMENT_CHECK_PRECONDITION(samples > 1) << "Requesting to create a MSAA sidecar texture for "
                                             << +samples << " samples? Invalid request.";
    if (mMsaaSidecarTexture) {
        FILAMENT_CHECK_PRECONDITION(mMsaaSidecarTexture.GetSampleCount() == samples)
                << "An MSAA sidecar texture has already been created for this texture, but with a "
                   "different sample count ("
                << mMsaaSidecarTexture.GetSampleCount() << ") than requested (" << +samples
                << "). Note that this restriction was written when WebGPU only supported msaa "
                   "textures with 4 samples. If that has changed, this implementation should be "
                   "updated (e.g. map of sidecar textures by sampleCount or something).";
        return; // we already have the sidecar created
    }
    const wgpu::TextureDescriptor descriptor{
        .label = "msaa_sidecar_texture",
        .usage = mTexture.GetUsage(),
        .dimension = mTexture.GetDimension(),
        .size = {
            .width = mTexture.GetWidth(),
            .height = mTexture.GetHeight(),
            .depthOrArrayLayers = mTexture.GetDepthOrArrayLayers(),
        },
        .format = mTexture.GetFormat(),
        .mipLevelCount = mTexture.GetMipLevelCount(),
        .sampleCount = samples,
        .viewFormatCount = 1,
        .viewFormats = &mViewFormat,
    };
    mMsaaSidecarTexture = device.CreateTexture(&descriptor);
    FILAMENT_CHECK_POSTCONDITION(mMsaaSidecarTexture) << "Failed to create MSAA sidecar texture";
}

wgpu::TextureView WebGPUTexture::makeMsaaSidecarTextureViewIfTextureSidecarExists(
        const uint8_t samples, const uint8_t mipLevel, const uint32_t arrayLayer) const {
    if (mMsaaSidecarTexture == nullptr) {
        return nullptr;
    }
    FILAMENT_CHECK_PRECONDITION(mMsaaSidecarTexture.GetSampleCount() == samples)
            << "An MSAA sidecar texture has already been created for this texture, but with a "
               "different sample count ("
            << mMsaaSidecarTexture.GetSampleCount() << ") than requested for view (" << +samples
            << "). Note that this restriction was written when WebGPU only supported msaa "
               "textures with 4 samples. If that has changed, this implementation should be "
               "updated (e.g. map of sidecar textures by sampleCount or something).";
    const wgpu::TextureViewDescriptor descriptor{
        .label = "msaa_sidecar_texture_view",
        .format = mViewFormat,
        .dimension = mDimension,
        .baseMipLevel = mipLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = arrayLayer,
        .arrayLayerCount = 1,
        .aspect = mAspect,
        .usage = mViewUsage,
    };
    const wgpu::TextureView textureView{ mMsaaSidecarTexture.CreateView(&descriptor) };
    FILAMENT_CHECK_POSTCONDITION(mMsaaSidecarTexture)
            << "Failed to create MSAA sidecar texture view (" << +samples << " samples)";
    return textureView;
}

wgpu::TextureView WebGPUTexture::makeTextureView(const uint8_t& baseLevel,
        const uint8_t& levelCount, const uint32_t& baseArrayLayer, const uint32_t& arrayLayerCount,
        const wgpu::TextureViewDimension dimension) const noexcept {

    const wgpu::TextureViewDescriptor textureViewDescriptor{
        .label = getUserTextureViewLabel(target),
        .format = mViewFormat,
        .dimension = dimension,
        .baseMipLevel = baseLevel,
        .mipLevelCount = levelCount,
        .baseArrayLayer = baseArrayLayer,
        .arrayLayerCount = arrayLayerCount,
        .aspect = mAspect,
        .usage = mViewUsage };
    wgpu::TextureView textureView = mTexture.CreateView(&textureViewDescriptor);
    FILAMENT_CHECK_POSTCONDITION(textureView)
            << "Failed to create texture view " << textureViewDescriptor.label;
    return textureView;
}

}// namespace filament::backend
