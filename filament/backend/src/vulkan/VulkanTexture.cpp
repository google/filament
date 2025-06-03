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

#include "VulkanCommands.h"
#include "VulkanContext.h"
#include "VulkanMemory.h"
#include "VulkanTexture.h"
#include "vulkan/memory/ResourcePointer.h"
#include "vulkan/utils/Conversion.h"

#include <DataReshaper.h>
#include <backend/DriverEnums.h>
#include <private/backend/BackendUtils.h>

#include <utils/Panic.h>

using namespace bluevk;

namespace filament::backend {

namespace {

inline uint8_t getLayerCount(SamplerType const target, uint32_t const depth) {
    switch (target) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_3D:
        case SamplerType::SAMPLER_EXTERNAL:
            return 1;
        case SamplerType::SAMPLER_CUBEMAP:
            return 6;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return depth * 6;
        case SamplerType::SAMPLER_2D_ARRAY:
            return depth;
    }
}

VkComponentMapping composeSwizzle(VkComponentMapping const& prev, VkComponentMapping const& next) {
    static constexpr VkComponentSwizzle IDENTITY[] = {
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A,
    };
    auto const compose = [](VkComponentMapping const& prev,
                                 VkComponentMapping const& next) -> VkComponentMapping {
        VkComponentSwizzle vals[4] = { next.r, next.g, next.b, next.a };
        for (auto& out: vals) {
            switch (out) {
                case VK_COMPONENT_SWIZZLE_R:
                    out = prev.r;
                    break;
                case VK_COMPONENT_SWIZZLE_G:
                    out = prev.g;
                    break;
                case VK_COMPONENT_SWIZZLE_B:
                    out = prev.b;
                    break;
                case VK_COMPONENT_SWIZZLE_A:
                    out = prev.a;
                    break;
                // Do not modify the value
                case VK_COMPONENT_SWIZZLE_IDENTITY:
                case VK_COMPONENT_SWIZZLE_ZERO:
                case VK_COMPONENT_SWIZZLE_ONE:
                // Below is not exposed in Vulkan's API, but needs to be there for compilation.
                case VK_COMPONENT_SWIZZLE_MAX_ENUM:
                    break;
            }
        }
        return { vals[0], vals[1], vals[2], vals[3] };
    };

    auto const identityToChannel = [](VkComponentMapping const& mapping) -> VkComponentMapping {
        VkComponentSwizzle vals[4] = { mapping.r, mapping.g, mapping.b, mapping.a };
        for (uint8_t i = 0; i < 4; i++) {
            if (vals[i] != VK_COMPONENT_SWIZZLE_IDENTITY) {
                continue;
            }
            vals[i] = IDENTITY[i];
        }
        return { vals[0], vals[1], vals[2], vals[3] };
    };
    auto const channelToIdentity = [](VkComponentMapping const& mapping) -> VkComponentMapping {
        VkComponentSwizzle vals[4] = { mapping.r, mapping.g, mapping.b, mapping.a };
        for (uint8_t i = 0; i < 4; i++) {
            if (IDENTITY[i] != vals[i]) {
                continue;
            }
            vals[i] = VK_COMPONENT_SWIZZLE_IDENTITY;
        }
        return { vals[0], vals[1], vals[2], vals[3] };
    };

    // We make sure all all identities are mapped into respective channels so that actual channel
    // mapping will be passed onto the output.
    VkComponentMapping const prevExplicit = identityToChannel(prev);
    VkComponentMapping const nextExplicit = identityToChannel(next);
    return channelToIdentity(compose(prevExplicit, nextExplicit));
}

inline VulkanLayout getDefaultLayoutImpl(TextureUsage usage) {
    if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
        if (any(usage & TextureUsage::SAMPLEABLE)) {
            return VulkanLayout::DEPTH_SAMPLER;
        } else {
            return VulkanLayout::DEPTH_ATTACHMENT;
        }
    }

    if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
        return VulkanLayout::COLOR_ATTACHMENT;
    }
    // Finally, the layout for an immutable texture is optimal read-only.
    return VulkanLayout::FRAG_READ;
}

inline VulkanLayout getDefaultLayoutImpl(VkImageUsageFlags vkusage) {
    TextureUsage usage{};
    if (vkusage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        usage = usage | TextureUsage::DEPTH_ATTACHMENT;
    }
    if (vkusage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        usage = usage | TextureUsage::COLOR_ATTACHMENT;
    }
    if (vkusage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        usage = usage | TextureUsage::SAMPLEABLE;
    }
    return getDefaultLayoutImpl(usage);
}

SamplerType getSamplerTypeFromDepth(uint32_t const depth) {
  return depth > 1 ? SamplerType::SAMPLER_2D_ARRAY
                                  : SamplerType::SAMPLER_2D;
}

uint8_t getLayerCountFromDepth(uint32_t const depth) {
    return getLayerCount(getSamplerTypeFromDepth(depth), depth);
}

VkImageUsageFlags getUsage(VulkanContext const& context, uint8_t samples,
        VkPhysicalDevice physicalDevice, VkFormat vkFormat, TextureUsage tusage) {
    VkImageUsageFlags usage = {};
    if (any(tusage & TextureUsage::BLIT_SRC)) {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (any(tusage & TextureUsage::BLIT_DST)) {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    // Determine if we can use the transient usage flag combined with lazily allocated memory.
    const bool useTransientAttachment =
            // Lazily allocated memory is available.
            context.isLazilyAllocatedMemorySupported() &&
            // Usage consists of attachment flags only.
            none(tusage & ~TextureUsage::ALL_ATTACHMENTS) &&
            // Usage contains at least one attachment flag.
            any(tusage & TextureUsage::ALL_ATTACHMENTS) &&
            // Depth resolve cannot use transient attachment because it uses a custom shader.
            // TODO: see VulkanDriver::isDepthStencilResolveSupported() to know when to remove this
            // restriction.
            // Note that the custom shader does not resolve stencil. We do need to move to vk 1.2
            // and above to be able to support stencil resolve (along with depth).
            !(any(tusage & TextureUsage::DEPTH_ATTACHMENT) && samples > 1);

    const VkImageUsageFlags transientFlag =
       useTransientAttachment ? VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT : 0U;

    if (any(tusage & TextureUsage::SAMPLEABLE)) {

#if FVK_ENABLED(FVK_DEBUG_TEXTURE)
        if (physicalDevice != VK_NULL_HANDLE) {
            // Validate that the format is actually sampleable.
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, vkFormat, &props);
            if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
                FVK_LOGW << "Texture usage is SAMPLEABLE but format " << vkFormat << " is not "
                        "sampleable with optimal tiling." << utils::io::endl;
            }
        }
#endif

        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (any(tusage & TextureUsage::COLOR_ATTACHMENT)) {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | transientFlag;
        if (any(tusage & TextureUsage::SUBPASS_INPUT)) {
            usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
    }
    if (any(tusage & TextureUsage::STENCIL_ATTACHMENT)) {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | transientFlag;
    }
    if (any(tusage & TextureUsage::UPLOADABLE)) {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (any(tusage & TextureUsage::DEPTH_ATTACHMENT)) {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | transientFlag;

        // Depth resolves uses a custom shader and therefore needs to be sampleable.
        if (samples > 1) {
            usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
    }
    return usage;
}

} // anonymous namespace

VulkanTextureState::VulkanTextureState(VulkanStagePool& stagePool, VulkanCommands* commands,
        VmaAllocator allocator, VkDevice device, VkImage image, VkDeviceMemory deviceMemory,
        VkFormat format, VkImageViewType viewType, uint8_t levels, uint8_t layerCount,
        VkSamplerYcbcrConversion ycbcrConversion, VkImageUsageFlags usage, bool isProtected)
    : mStagePool(stagePool),
      mCommands(commands),
      mAllocator(allocator),
      mDevice(device),
      mTextureImage(image),
      mTextureImageMemory(deviceMemory),
      mVkFormat(format),
      mViewType(viewType),
      mFullViewRange{ fvkutils::getImageAspect(format), 0, levels, 0, layerCount },
      mYcbcr{ ycbcrConversion },
      mDefaultLayout(getDefaultLayoutImpl(usage)),
      mUsage(usage),
      mIsProtected(isProtected) {}

VulkanTextureState::~VulkanTextureState() {
    clearCachedImageViews();
    if (mTextureImageMemory != VK_NULL_HANDLE) {
        vkDestroyImage(mDevice, mTextureImage, VKALLOC);
        vkFreeMemory(mDevice, mTextureImageMemory, VKALLOC);
    }
}

void VulkanTextureState::clearCachedImageViews() noexcept {
    for (auto entry: mCachedImageViews) {
        vkDestroyImageView(mDevice, entry.second, VKALLOC);
    }
    mCachedImageViews.clear();
}

VkImageView VulkanTextureState::getImageView(VkImageSubresourceRange range, VkImageViewType viewType,
        VkComponentMapping swizzle) {
    ImageViewKey const key{ range, viewType, swizzle };
    if (auto iter = mCachedImageViews.find(key); iter != mCachedImageViews.end()) {
        return iter->second;
    }

    VkSamplerYcbcrConversionInfo conversionInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO,
        .conversion = mYcbcr.conversion,
    };

    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = mYcbcr.conversion != VK_NULL_HANDLE ? &conversionInfo : nullptr,
        .flags = 0,
        .image = mTextureImage,
        .viewType = viewType,
        .format = mYcbcr.conversion != VK_NULL_HANDLE ? VK_FORMAT_UNDEFINED : mVkFormat,
        .components = swizzle,
        .subresourceRange = range,
    };
    VkImageView imageView;
    vkCreateImageView(mDevice, &viewInfo, VKALLOC, &imageView);
    mCachedImageViews.emplace(key, imageView);
    return imageView;
}

// Constructor for internally passed VkImage - including swapchain images and external images.
VulkanTexture::VulkanTexture(VulkanContext const& context, VkDevice device, VmaAllocator allocator,
        fvkmemory::ResourceManager* resourceManager, VulkanCommands* commands, VkImage image,
        VkDeviceMemory memory, VkFormat format, VkSamplerYcbcrConversion conversion,
        uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage tusage,
        VulkanStagePool& stagePool)
    : HwTexture(getSamplerTypeFromDepth(depth), 1, samples, width, height, depth,
              TextureFormat::UNUSED, tusage),
      mState(fvkmemory::resource_ptr<VulkanTextureState>::construct(resourceManager, stagePool,
              commands, allocator, device, image, memory, format,
              fvkutils::getViewType(SamplerType::SAMPLER_2D),
              /*mipLevels=*/1, getLayerCountFromDepth(depth), conversion,
              getUsage(context, samples, VK_NULL_HANDLE, format, tusage),
              any(usage & TextureUsage::PROTECTED))) {
    mPrimaryViewRange = mState->mFullViewRange;
}

// Constructor for user facing texture
VulkanTexture::VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice,
        VulkanContext const& context, VmaAllocator allocator,
        fvkmemory::ResourceManager* resourceManager, VulkanCommands* commands, SamplerType target,
        uint8_t levels, TextureFormat tformat, uint8_t samples, uint32_t w, uint32_t h,
        uint32_t depth, TextureUsage tusage, VulkanStagePool& stagePool)
     : HwTexture(target, levels, samples, w, h, depth, tformat, tusage) {
    // Create an appropriately-sized device-only VkImage, but do not fill it yet.
    VkFormat const vkFormat = fvkutils::getVkFormat(tformat);
    bool const isProtected = any(tusage & TextureUsage::PROTECTED);
    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = target == SamplerType::SAMPLER_3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
        .format = vkFormat,
        .extent = {w, h, depth},
        .mipLevels = levels,
        .arrayLayers = 1,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = getUsage(context, samples, physicalDevice, vkFormat, tusage),
    };
    if (target == SamplerType::SAMPLER_3D && any(tusage & TextureUsage::ALL_ATTACHMENTS)) {
        if (context.isImageView2DOn3DImageSupported()) {
            // Note that VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT is only meant to create 2D views of
            // a 3D image in the case where the image is the render target.  So, for example, it's
            // not meant to allow for 2D views that can be used with a sampler.
            imageInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        } else {
            FVK_LOGW << "Note: creating 2D views on 3D image is not available on this platform. "
                     << "i.e. we cannot render to slices of a 3D image" << utils::io::endl;
        }
    } else if (target == SamplerType::SAMPLER_CUBEMAP) {
        imageInfo.arrayLayers = 6;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    } else if (target == SamplerType::SAMPLER_2D_ARRAY) {
        imageInfo.arrayLayers = depth;
        imageInfo.extent.depth = 1;
        // NOTE: We do not use VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT here because:
        //
        //  (a) MoltenVK does not support it, and
        //  (b) it is necessary only when 3D textures need to support array-style access
        //
        // In other words, the "arrayness" of the texture is an aspect of the VkImageView,
        // not the VkImage.
    }
    if (target == SamplerType::SAMPLER_CUBEMAP_ARRAY) {
        imageInfo.arrayLayers = depth * 6;
        imageInfo.extent.depth = 1;
    }
    if (isProtected) {
        imageInfo.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }

    // Constrain the sample count according to the sample count masks in VkPhysicalDeviceProperties.
    // Note that VulkanRenderTarget holds a single MSAA count, so we play it safe if this is used as
    // any kind of attachment (color or depth).
    auto const& limits = context.getPhysicalDeviceLimits();
    if (imageInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        samples = fvkutils::reduceSampleCount(samples,
                fvkutils::isVkDepthFormat(vkFormat)
                        ? limits.sampledImageDepthSampleCounts
                        : limits.sampledImageColorSampleCounts);
    }
    if (imageInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        samples = fvkutils::reduceSampleCount(samples, limits.framebufferColorSampleCounts);
    }

    if (imageInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        samples = fvkutils::reduceSampleCount(samples, limits.sampledImageDepthSampleCounts);
    }
    this->samples = samples;
    imageInfo.samples = (VkSampleCountFlagBits) samples;

    VkImage textureImage;
    VkResult result = vkCreateImage(device, &imageInfo, VKALLOC, &textureImage);
    if (result != VK_SUCCESS || FVK_ENABLED(FVK_DEBUG_TEXTURE)) {
        FVK_LOGD << "vkCreateImage: "
            << "image = " << textureImage << ", "
            << "result = " << result << ", "
            << "handle = " << utils::io::hex << textureImage << utils::io::dec << ", "
            << "extent = " << w << "x" << h << "x"<< depth << ", "
            << "mipLevels = " << int(levels) << ", "
            << "TextureUsage = " << static_cast<int>(tusage) << ", "
            << "usage = " << imageInfo.usage << ", "
            << "samples = " << imageInfo.samples << ", "
            << "type = " << imageInfo.imageType << ", "
            << "flags = " << imageInfo.flags << ", "
            << "target = " << static_cast<int>(target) <<", "
            << "format = " << vkFormat << utils::io::endl;
    }
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "Unable to create image."
                                                       << " error=" << static_cast<int32_t>(result);

    // Allocate memory for the VkImage and bind it.
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, textureImage, &memReqs);

    bool const useTransientAttachment = imageInfo.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    VkFlags const requiredMemoryFlags =
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
        (useTransientAttachment ? VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : 0U) |
        (isProtected ? VK_MEMORY_PROPERTY_PROTECTED_BIT : 0U);
    uint32_t memoryTypeIndex
            = context.selectMemoryType(memReqs.memoryTypeBits, requiredMemoryFlags);

    FILAMENT_CHECK_POSTCONDITION(memoryTypeIndex < VK_MAX_MEMORY_TYPES)
            << "VulkanTexture: unable to find a memory type that meets requirements.";

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = memoryTypeIndex,
    };
    VkDeviceMemory textureImageMemory;
    result = vkAllocateMemory(device, &allocInfo, nullptr, &textureImageMemory);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "Unable to allocate image memory."
                                                       << " error=" << static_cast<int32_t>(result);
    result = vkBindImageMemory(device, textureImage, textureImageMemory, 0);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "Unable to bind image."
                                                       << " error=" << static_cast<int32_t>(result);

    mState = fvkmemory::resource_ptr<VulkanTextureState>::construct(resourceManager, stagePool,
            commands, allocator, device, textureImage, textureImageMemory, vkFormat,
            fvkutils::getViewType(target), levels, getLayerCount(target, depth),
            VK_NULL_HANDLE /* ycbcrConversion */, imageInfo.usage, isProtected);

    // Spec out the "primary" VkImageView that shaders use to sample from the image.
    mPrimaryViewRange = mState->mFullViewRange;
}

// Constructor for creating a texture view
VulkanTexture::VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice,
        VulkanContext const& context, VmaAllocator allocator, VulkanCommands* commands,
        fvkmemory::resource_ptr<VulkanTexture> src, uint8_t baseLevel,
        uint8_t levelCount)
    : HwTexture(src->target, src->levels, src->samples, src->width, src->height, src->depth,
            src->format, src->usage),
      mState(src->mState) {
    mPrimaryViewRange = src->mPrimaryViewRange;
    mPrimaryViewRange.baseMipLevel = src->mPrimaryViewRange.baseMipLevel + baseLevel;
    mPrimaryViewRange.levelCount = levelCount;
}

// Constructor for creating a texture view with swizzle
VulkanTexture::VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice,
        VulkanContext const& context, VmaAllocator allocator, VulkanCommands* commands,
        fvkmemory::resource_ptr<VulkanTexture> src, VkComponentMapping swizzle)
    : HwTexture(src->target, src->levels, src->samples, src->width, src->height, src->depth,
              src->format, src->usage),
      mState(src->mState),
      mPrimaryViewRange(src->mPrimaryViewRange),
      mSwizzle(composeSwizzle(src->mSwizzle, swizzle)) {}

void VulkanTexture::updateImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
        uint32_t depth, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset, uint32_t miplevel) {
    assert_invariant(width <= this->width && height <= this->height);
    assert_invariant(depth <= this->depth * ((target == SamplerType::SAMPLER_CUBEMAP ||
                        target == SamplerType::SAMPLER_CUBEMAP_ARRAY) ? 6 : 1));
    assert_invariant(!mState->mIsProtected);
    const PixelBufferDescriptor* hostData = &data;
    PixelBufferDescriptor reshapedData;

    // First, reshape 3-component data into 4-component data. The fourth component is usually
    // set to 1 (one exception is when type = HALF). In practice, alpha is just a dummy channel.
    // Note that the reshaped data is freed at the end of this method due to the callback.
    if (reshape(data, reshapedData)) {
        hostData = &reshapedData;
    }

    // If format conversion is both required and supported, use vkCmdBlitImage.
    const VkFormat hostFormat = fvkutils::getVkFormat(hostData->format, hostData->type);
    const VkFormat deviceFormat = fvkutils::getVkFormatLinear(mState->mVkFormat);
    if (hostFormat != deviceFormat && hostFormat != VK_FORMAT_UNDEFINED) {
        assert_invariant(xoffset == 0 && yoffset == 0 && zoffset == 0 &&
                "Offsets not yet supported when format conversion is required.");
        updateImageWithBlit(*hostData, width, height, depth, miplevel);
        return;
    }

    assert_invariant(hostData->size > 0 && "Data is empty");

    // Otherwise, use vkCmdCopyBufferToImage.
    // Note: the following stageSegment must be stored within the command buffer
    // before going out of scope, to ensure proper bookkeeping within the
    // staging buffer pool.
    fvkmemory::resource_ptr<VulkanStage::Segment> stageSegment =
            mState->mStagePool.acquireStage(hostData->size);
    assert_invariant(stageSegment->memory());
    memcpy(stageSegment->mapping(), hostData->buffer, hostData->size);
    vmaFlushAllocation(mState->mAllocator, stageSegment->memory(), stageSegment->offset(),
            hostData->size);

    VulkanCommandBuffer& commands = mState->mCommands->get();
    VkCommandBuffer const cmdbuf = commands.buffer();
    commands.acquire(stageSegment);
    commands.acquire(fvkmemory::resource_ptr<VulkanTexture>::cast(this));

    VkBufferImageCopy copyRegion = { .bufferOffset = stageSegment->offset(),
        .bufferRowLength = {},
        .bufferImageHeight = {},
        .imageSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = miplevel,
            .baseArrayLayer = 0,
            .layerCount = 1 },
        .imageOffset = { int32_t(xoffset), int32_t(yoffset), int32_t(zoffset) },
        .imageExtent = { width, height, depth } };

    VkImageSubresourceRange transitionRange = {
        .aspectMask = getImageAspect(),
        .baseMipLevel = miplevel,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    // Vulkan specifies subregions for 3D textures differently than from 2D arrays.
    if (    target == SamplerType::SAMPLER_2D_ARRAY ||
            target == SamplerType::SAMPLER_CUBEMAP ||
            target == SamplerType::SAMPLER_CUBEMAP_ARRAY) {
        copyRegion.imageOffset.z = 0;
        copyRegion.imageExtent.depth = 1;
        copyRegion.imageSubresource.baseArrayLayer = zoffset;
        copyRegion.imageSubresource.layerCount = depth;
        transitionRange.baseArrayLayer = zoffset;
        transitionRange.layerCount = depth;
    }

    VulkanLayout const newLayout = VulkanLayout::TRANSFER_DST;
    VulkanLayout nextLayout = getLayout(transitionRange.baseArrayLayer, miplevel);
    VkImageLayout const newVkLayout = fvkutils::getVkLayout(newLayout);

    if (nextLayout == VulkanLayout::UNDEFINED) {
        nextLayout = getDefaultLayout();
    }

    transitionLayout(&commands, transitionRange, newLayout);

    vkCmdCopyBufferToImage(cmdbuf, stageSegment->buffer(), mState->mTextureImage, newVkLayout, 1,
            &copyRegion);

    transitionLayout(&commands, transitionRange, nextLayout);
}

void VulkanTexture::updateImageWithBlit(const PixelBufferDescriptor& hostData, uint32_t width,
        uint32_t height, uint32_t depth, uint32_t miplevel) {
    void* mapped = nullptr;
    VulkanStageImage const* stage
            = mState->mStagePool.acquireImage(hostData.format, hostData.type, width, height);
    vmaMapMemory(mState->mAllocator, stage->memory, &mapped);
    memcpy(mapped, hostData.buffer, hostData.size);
    vmaUnmapMemory(mState->mAllocator, stage->memory);
    vmaFlushAllocation(mState->mAllocator, stage->memory, 0, hostData.size);

    VulkanCommandBuffer& commands = mState->mCommands->get();
    VkCommandBuffer const cmdbuf = commands.buffer();
    commands.acquire(fvkmemory::resource_ptr<VulkanTexture>::cast(this));

    // TODO: support blit-based format conversion for 3D images and cubemaps.
    constexpr int layer = 0;

    VkOffset3D const rect[2] { {0, 0, 0}, {int32_t(width), int32_t(height), 1} };

    VkImageAspectFlags const aspect = getImageAspect();

    VkImageBlit const blitRegions[1] = {{
        .srcSubresource = { aspect, 0, 0, 1 },
        .srcOffsets = { rect[0], rect[1] },
        .dstSubresource = { aspect, uint32_t(miplevel), layer, 1 },
        .dstOffsets = { rect[0], rect[1] }
    }};

    VkImageSubresourceRange const range = { aspect, miplevel, 1, layer, 1 };

    VulkanLayout const newLayout = VulkanLayout::TRANSFER_DST;
    VulkanLayout const oldLayout = getLayout(layer, miplevel);
    transitionLayout(&commands, range, newLayout);

    vkCmdBlitImage(cmdbuf, stage->image, fvkutils::getVkLayout(VulkanLayout::TRANSFER_SRC),
            mState->mTextureImage, fvkutils::getVkLayout(newLayout), 1, blitRegions, VK_FILTER_NEAREST);

    transitionLayout(&commands, range, oldLayout);
}

VulkanLayout VulkanTexture::getDefaultLayout() const {
    return mState->mDefaultLayout;
}

VkImageView VulkanTexture::getAttachmentView(VkImageSubresourceRange const& range, VkImageViewType type) {
    assert_invariant(mState->mYcbcr.conversion == VK_NULL_HANDLE &&
                     "We are not yet supporting external image as attachments.");
    if (type == VK_IMAGE_VIEW_TYPE_2D) {
        VkImageSubresourceRange copy = range;
        copy.levelCount = 1;
        copy.layerCount = 1;
        return mState->getImageView(copy, type, {});
    } else {
        return mState->getImageView(range, type, {});
    }
}

VkImageView VulkanTexture::getView(VkImageSubresourceRange const& range) {
    return mState->getImageView(range, mState->mViewType, mSwizzle);
}

VkImageAspectFlags VulkanTexture::getImageAspect() const {
    // Helper function in VulkanUtility
    return fvkutils::getImageAspect(mState->mVkFormat);
}

bool VulkanTexture::transitionLayout(VulkanCommandBuffer* commands,
        VkImageSubresourceRange const& range, VulkanLayout newLayout) {
    if (transitionLayout(commands->buffer(), range, newLayout)) {
        commands->acquire(fvkmemory::resource_ptr<VulkanTexture>::cast(this));
        return true;
    }
    return false;
}

bool VulkanTexture::transitionLayout(VkCommandBuffer cmdbuf, VkImageSubresourceRange const& range,
        VulkanLayout newLayout) {
    VulkanLayout const oldLayout = getLayout(range.baseArrayLayer, range.baseMipLevel);

    uint32_t const firstLayer = range.baseArrayLayer;
    uint32_t const lastLayer = firstLayer + range.layerCount;
    uint32_t const firstLevel = range.baseMipLevel;
    uint32_t const lastLevel = firstLevel + range.levelCount;

    // If we are transitioning more than one layer/level (slice), we need to know whether they are
    // all of the same layer.  If not, we need to transition slice-by-slice. Otherwise it would
    // trigger the validation layer saying that the `oldLayout` provided is incorrect.
    // TODO: transition by multiple slices with more sophisticated range finding.
    bool transitionSliceBySlice = false;
    for (uint32_t i = firstLayer; i < lastLayer; ++i) {
        for (uint32_t j = firstLevel; j < lastLevel; ++j) {
            if (oldLayout != getLayout(i, j)) {
                transitionSliceBySlice = true;
                break;
            }
        }
    }

    bool hasTransitions = false;
    if (transitionSliceBySlice) {
        for (uint32_t i = firstLayer; i < lastLayer; ++i) {
            for (uint32_t j = firstLevel; j < lastLevel; ++j) {
                VulkanLayout const layout = getLayout(i, j);
                if (layout == newLayout) {
                    continue;
                }
                hasTransitions = hasTransitions || fvkutils::transitionLayout(cmdbuf, {
                    .image = mState->mTextureImage,
                    .oldLayout = layout,
                    .newLayout = newLayout,
                    .subresources = {
                        .aspectMask = range.aspectMask,
                        .baseMipLevel = j,
                        .levelCount = 1,
                        .baseArrayLayer = i,
                        .layerCount = 1,
                    },
                });
            }
        }
    } else if (newLayout != oldLayout) {
        hasTransitions = fvkutils::transitionLayout(cmdbuf, {
            .image = mState->mTextureImage,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .subresources = range,
        });
    }

    // Even if we didn't carry out the transition, we should assume that the new layout is defined
    // through this call.
    setLayout(range, newLayout);

    if (hasTransitions) {
#if FVK_ENABLED(FVK_DEBUG_LAYOUT_TRANSITION)
        FVK_LOGD << "transition texture=" << mState->mTextureImage << " (" << range.baseArrayLayer
                 << "," << range.baseMipLevel << ")" << " count=(" << range.layerCount << ","
                 << range.levelCount << ")" << " from=" << oldLayout << " to=" << newLayout
                 << " format=" << mState->mVkFormat << " depth="
                 << fvkutils::isVkDepthFormat(mState->mVkFormat)
                 << " slice-by-slice=" << transitionSliceBySlice << utils::io::endl;
#endif
    } else {
#if FVK_ENABLED(FVK_DEBUG_LAYOUT_TRANSITION)
        FVK_LOGD << "transition texture=" << mState->mTextureImage << " (" << range.baseArrayLayer
                 << "," << range.baseMipLevel << ")" << " count=(" << range.layerCount << ","
                 << range.levelCount << ")" << " to=" << newLayout
                 << " is skipped because of no change in layout" << utils::io::endl;
#endif
    }

    return hasTransitions;
}

void VulkanTexture::samplerToAttachmentBarrier(VulkanCommandBuffer* commands,
        VkImageSubresourceRange const& range) {
    VkCommandBuffer const cmdbuf = commands->buffer();
    VkImageLayout const layout =
            fvkutils::getVkLayout(getLayout(range.baseArrayLayer, range.baseMipLevel));
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = layout,
        .newLayout = layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = mState->mTextureImage,
        .subresourceRange = range,
    };
    vkCmdPipelineBarrier(cmdbuf, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanTexture::attachmentToSamplerBarrier(VulkanCommandBuffer* commands,
        VkImageSubresourceRange const& range) {
    VkCommandBuffer const cmdbuf = commands->buffer();
    VkImageLayout const layout
            = fvkutils::getVkLayout(getLayout(range.baseArrayLayer, range.baseMipLevel));
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = layout,
        .newLayout = layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = mState->mTextureImage,
        .subresourceRange = range,
    };
    vkCmdPipelineBarrier(cmdbuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanTexture::setLayout(VkImageSubresourceRange const& range, VulkanLayout newLayout) {
    uint32_t const firstLayer = range.baseArrayLayer;
    uint32_t const lastLayer = firstLayer + range.layerCount;
    uint32_t const firstLevel = range.baseMipLevel;
    uint32_t const lastLevel = firstLevel + range.levelCount;

    assert_invariant(firstLevel <= 0xffff && lastLevel <= 0xffff);
    assert_invariant(firstLayer <= 0xffff && lastLayer <= 0xffff);

    if (newLayout == VulkanLayout::UNDEFINED) {
        for (uint32_t layer = firstLayer; layer < lastLayer; ++layer) {
            uint32_t const first = (layer << 16) | firstLevel;
            uint32_t const last = (layer << 16) | lastLevel;
            mState->mSubresourceLayouts.clear(first, last);
        }
    } else {
        for (uint32_t layer = firstLayer; layer < lastLayer; ++layer) {
            uint32_t const first = (layer << 16) | firstLevel;
            uint32_t const last = (layer << 16) | lastLevel;
            mState->mSubresourceLayouts.add(first, last, newLayout);
        }
    }
}

void VulkanTexture::setYcbcrConversion(VkSamplerYcbcrConversion conversion) {
    // Note that this comparison is valid because we only ever create VkSamplerYcbcrConversion from
    // a cache.  So for each set of parameters, there is exactly one conversion (similar to
    // samplers).
    VulkanTextureState::Ycbcr ycbcr = {
        .conversion = conversion,
    };
    if (mState->mYcbcr != ycbcr) {
        mState->mYcbcr = ycbcr;
        mState->clearCachedImageViews();
    }
}

VulkanLayout VulkanTexture::getLayout(uint32_t layer, uint32_t level) const {
    assert_invariant(level <= 0xffff && layer <= 0xffff);
    const uint32_t key = (layer << 16) | level;
    if (!mState->mSubresourceLayouts.has(key)) {
        return VulkanLayout::UNDEFINED;
    }
    return mState->mSubresourceLayouts.get(key);
}

#if FVK_ENABLED(FVK_DEBUG_TEXTURE)
void VulkanTexture::print() const {
    uint32_t const firstLayer = 0;
    uint32_t const lastLayer = firstLayer + mState->mFullViewRange.layerCount;
    uint32_t const firstLevel = 0;
    uint32_t const lastLevel = firstLevel + mState->mFullViewRange.levelCount;

    for (uint32_t layer = firstLayer; layer < lastLayer; ++layer) {
        for (uint32_t level = firstLevel; level < lastLevel; ++level) {
            bool primary =
                layer >= mPrimaryViewRange.baseArrayLayer &&
                layer < (mPrimaryViewRange.baseArrayLayer + mPrimaryViewRange.layerCount) &&
                level >= mPrimaryViewRange.baseMipLevel &&
                level < (mPrimaryViewRange.baseMipLevel + mPrimaryViewRange.levelCount);
            FVK_LOGD << "[" << mState->mTextureImage << "]: (" << layer << "," << level
                          << ")=" << getLayout(layer, level)
                          << " primary=" << primary
                          << utils::io::endl;
        }
    }

    for (auto view: mState->mCachedImageViews) {
        auto& range = view.first.range;
        FVK_LOGD << "[" << mState->mTextureImage << ", imageView=" << view.second << "]=>"
                      << " (" << range.baseArrayLayer << "," << range.baseMipLevel << ")"
                      << " count=(" << range.layerCount << "," << range.levelCount << ")"
                      << " aspect=" << range.aspectMask << " viewType=" << view.first.type
                      << utils::io::endl;
    }
}
#endif

} // namespace filament::backend
