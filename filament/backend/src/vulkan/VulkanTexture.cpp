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

#include "VulkanMemory.h"
#include "VulkanTexture.h"
#include "VulkanUtility.h"

#include <private/backend/BackendUtils.h>

#include "DataReshaper.h"

#include <utils/Panic.h>

using namespace bluevk;

namespace filament::backend {

using ImgUtil = VulkanImageUtility;
VulkanTexture::VulkanTexture(VkDevice device, VmaAllocator allocator,
        VulkanCommands* commands, VkImage image, VkFormat format, uint8_t samples,
        uint32_t width, uint32_t height, TextureUsage tusage, VulkanStagePool& stagePool)
    : HwTexture(SamplerType::SAMPLER_2D, 1, samples, width, height, 1, TextureFormat::UNUSED,
              tusage),
      mVkFormat(format), mViewType(ImgUtil::getViewType(target)), mSwizzle({}),
      mTextureImage(image),
      mPrimaryViewRange{
              .aspectMask = getImageAspect(),
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
      },
      mStagePool(stagePool), mDevice(device), mAllocator(allocator), mCommands(commands) {}

VulkanTexture::VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice,
        VulkanContext const& context, VmaAllocator allocator,
        VulkanCommands* commands, SamplerType target, uint8_t levels,
        TextureFormat tformat, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage tusage, VulkanStagePool& stagePool, VkComponentMapping swizzle)
    : HwTexture(target, levels, samples, w, h, depth, tformat, tusage),
      // Vulkan does not support 24-bit depth, use the official fallback format.
      mVkFormat(tformat == TextureFormat::DEPTH24 ? context.getDepthFormat()
                                                  : backend::getVkFormat(tformat)),
      mViewType(ImgUtil::getViewType(target)), mSwizzle(swizzle), mStagePool(stagePool),
      mDevice(device), mAllocator(allocator), mCommands(commands) {

    // Create an appropriately-sized device-only VkImage, but do not fill it yet.
    VkImageCreateInfo imageInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = target == SamplerType::SAMPLER_3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
            .format = mVkFormat,
            .extent = {w, h, depth},
            .mipLevels = levels,
            .arrayLayers = 1,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = 0};
    if (target == SamplerType::SAMPLER_CUBEMAP) {
        imageInfo.arrayLayers = 6;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    if (target == SamplerType::SAMPLER_2D_ARRAY) {
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

    // Filament expects blit() to work with any texture, so we almost always set these usage flags.
    // TODO: investigate performance implications of setting these flags.
    const VkImageUsageFlags blittable = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (any(usage & TextureUsage::SAMPLEABLE)) {

#if VK_ENABLE_VALIDATION
        // Validate that the format is actually sampleable.
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, mVkFormat, &props);
        if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
            utils::slog.w << "Texture usage is SAMPLEABLE but format " << mVkFormat << " is not "
                    "sampleable with optimal tiling." << utils::io::endl;
        }
#endif

        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | blittable;
        if (any(usage & TextureUsage::SUBPASS_INPUT)) {
            imageInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
    }
    if (any(usage & TextureUsage::STENCIL_ATTACHMENT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (any(usage & TextureUsage::UPLOADABLE)) {
        imageInfo.usage |= blittable;
    }
    if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
        imageInfo.usage |= blittable;
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        // Depth resolves uses a custom shader and therefore needs to be sampleable.
        if (samples > 1) {
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
    }

    // Constrain the sample count according to the sample count masks in VkPhysicalDeviceProperties.
    // Note that VulkanRenderTarget holds a single MSAA count, so we play it safe if this is used as
    // any kind of attachment (color or depth).
    const auto& limits = context.getPhysicalDeviceLimits();
    if (imageInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        samples = reduceSampleCount(samples, isDepthFormat(mVkFormat)
                                                     ? limits.sampledImageDepthSampleCounts
                                                     : limits.sampledImageColorSampleCounts);
    }
    if (imageInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        samples = reduceSampleCount(samples, limits.framebufferColorSampleCounts);
    }

    if (imageInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        samples = reduceSampleCount(samples, limits.sampledImageDepthSampleCounts);
    }
    this->samples = samples;
    imageInfo.samples = (VkSampleCountFlagBits) samples;

    VkResult error = vkCreateImage(mDevice, &imageInfo, VKALLOC, &mTextureImage);
    if (error || FILAMENT_VULKAN_VERBOSE) {
        utils::slog.d << "vkCreateImage: "
            << "image = " << mTextureImage << ", "
            << "result = " << error << ", "
            << "handle = " << utils::io::hex << mTextureImage << utils::io::dec << ", "
            << "extent = " << w << "x" << h << "x"<< depth << ", "
            << "mipLevels = " << int(levels) << ", "
            << "TextureUsage = " << static_cast<int>(usage) << ", "            
            << "usage = " << imageInfo.usage << ", "
            << "samples = " << imageInfo.samples << ", "
            << "type = " << imageInfo.imageType << ", "
            << "flags = " << imageInfo.flags << ", "
            << "target = " << static_cast<int>(target) <<", "
            << "format = " << mVkFormat << utils::io::endl;
    }
    ASSERT_POSTCONDITION(!error, "Unable to create image.");

    // Allocate memory for the VkImage and bind it.
    VkMemoryRequirements memReqs = {};
    vkGetImageMemoryRequirements(mDevice, mTextureImage, &memReqs);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = context.selectMemoryType(memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    error = vkAllocateMemory(mDevice, &allocInfo, nullptr, &mTextureImageMemory);
    ASSERT_POSTCONDITION(!error, "Unable to allocate image memory.");
    error = vkBindImageMemory(mDevice, mTextureImage, mTextureImageMemory, 0);
    ASSERT_POSTCONDITION(!error, "Unable to bind image.");

    // Spec out the "primary" VkImageView that shaders use to sample from the image.
    mPrimaryViewRange.aspectMask = getImageAspect();
    mPrimaryViewRange.baseMipLevel = 0;
    mPrimaryViewRange.levelCount = levels;
    mPrimaryViewRange.baseArrayLayer = 0;
    if (target == SamplerType::SAMPLER_CUBEMAP) {
        mPrimaryViewRange.layerCount = 6;
    } else if (target == SamplerType::SAMPLER_CUBEMAP_ARRAY) {
        mPrimaryViewRange.layerCount = depth * 6;
    } else if (target == SamplerType::SAMPLER_2D_ARRAY) {
        mPrimaryViewRange.layerCount = depth;
    } else if (target == SamplerType::SAMPLER_3D) {
        mPrimaryViewRange.layerCount = 1;
    } else {
        mPrimaryViewRange.layerCount = 1;
    }

    // Go ahead and create the primary image view, no need to do it lazily.
    getImageView(mPrimaryViewRange, mViewType, mSwizzle);

    // Transition the layout of each image slice that might be used as a render target.
    // We do not transition images that are merely SAMPLEABLE, this is deferred until upload time
    // because we do not know how many layers and levels will actually be used.
    if (imageInfo.usage
        & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
           | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        const uint32_t layers = mPrimaryViewRange.layerCount;
        VkImageSubresourceRange range = { getImageAspect(), 0, levels, 0, layers };
        VkCommandBuffer cmdbuf = mCommands->get().cmdbuffer;
        transitionLayout(cmdbuf, range, ImgUtil::getDefaultLayout(imageInfo.usage));
    }
}

VulkanTexture::~VulkanTexture() {
    if (mTextureImageMemory != VK_NULL_HANDLE) {
        vkDestroyImage(mDevice, mTextureImage, VKALLOC);
        vkFreeMemory(mDevice, mTextureImageMemory, VKALLOC);
    }
    for (auto entry : mCachedImageViews) {
        vkDestroyImageView(mDevice, entry.second, VKALLOC);
    }
}

void VulkanTexture::updateImage(const PixelBufferDescriptor& data, uint32_t width, uint32_t height,
        uint32_t depth, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset, uint32_t miplevel) {
    assert_invariant(width <= this->width && height <= this->height);
    assert_invariant(depth <= this->depth * ((target == SamplerType::SAMPLER_CUBEMAP ||
                        target == SamplerType::SAMPLER_CUBEMAP_ARRAY) ? 6 : 1));

    const PixelBufferDescriptor* hostData = &data;
    PixelBufferDescriptor reshapedData;

    // First, reshape 3-component data into 4-component data. The fourth component is usually
    // set to 1 (one exception is when type = HALF). In practice, alpha is just a dummy channel.
    // Note that the reshaped data is freed at the end of this method due to the callback.
    if (reshape(data, reshapedData)) {
        hostData = &reshapedData;
    }

    // If format conversion is both required and supported, use vkCmdBlitImage.
    const VkFormat hostFormat = backend::getVkFormat(hostData->format, hostData->type);
    const VkFormat deviceFormat = getVkFormatLinear(mVkFormat);
    if (hostFormat != deviceFormat && hostFormat != VK_FORMAT_UNDEFINED) {
        assert_invariant(xoffset == 0 && yoffset == 0 && zoffset == 0 &&
                "Offsets not yet supported when format conversion is required.");
        updateImageWithBlit(*hostData, width, height, depth, miplevel);
        return;
    }

    // Otherwise, use vkCmdCopyBufferToImage.
    void* mapped = nullptr;
    VulkanStage const* stage = mStagePool.acquireStage(hostData->size);
    assert_invariant(stage->memory);
    vmaMapMemory(mAllocator, stage->memory, &mapped);
    memcpy(mapped, hostData->buffer, hostData->size);
    vmaUnmapMemory(mAllocator, stage->memory);
    vmaFlushAllocation(mAllocator, stage->memory, 0, hostData->size);

    const VkCommandBuffer cmdbuf = mCommands->get(true).cmdbuffer;

    VkBufferImageCopy copyRegion = {
        .bufferOffset = {},
        .bufferRowLength = {},
        .bufferImageHeight = {},
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = miplevel,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = { int32_t(xoffset), int32_t(yoffset), int32_t(zoffset) },
        .imageExtent = { width, height, depth }
    };

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
    VulkanLayout nextLayout = getLayout(0, miplevel);
    VkImageLayout const newVkLayout = ImgUtil::getVkLayout(newLayout);

    if (nextLayout == VulkanLayout::UNDEFINED) {
        nextLayout = VulkanLayout::READ_WRITE;
    }

    transitionLayout(cmdbuf, transitionRange, newLayout);

    vkCmdCopyBufferToImage(cmdbuf, stage->buffer, mTextureImage, newVkLayout, 1, &copyRegion);

    transitionLayout(cmdbuf, transitionRange, nextLayout);
}

void VulkanTexture::updateImageWithBlit(const PixelBufferDescriptor& hostData, uint32_t width,
        uint32_t height, uint32_t depth, uint32_t miplevel) {
    void* mapped = nullptr;
    VulkanStageImage const* stage
            = mStagePool.acquireImage(hostData.format, hostData.type, width, height);
    vmaMapMemory(mAllocator, stage->memory, &mapped);
    memcpy(mapped, hostData.buffer, hostData.size);
    vmaUnmapMemory(mAllocator, stage->memory);
    vmaFlushAllocation(mAllocator, stage->memory, 0, hostData.size);

    const VkCommandBuffer cmdbuf = mCommands->get().cmdbuffer;

    // TODO: support blit-based format conversion for 3D images and cubemaps.
    const int layer = 0;

    const VkOffset3D rect[2] { {0, 0, 0}, {int32_t(width), int32_t(height), 1} };

    const VkImageAspectFlags aspect = getImageAspect();

    const VkImageBlit blitRegions[1] = {{
        .srcSubresource = { aspect, 0, 0, 1 },
        .srcOffsets = { rect[0], rect[1] },
        .dstSubresource = { aspect, uint32_t(miplevel), layer, 1 },
        .dstOffsets = { rect[0], rect[1] }
    }};

    const VkImageSubresourceRange range = { aspect, miplevel, 1, 0, 1 };

    VulkanLayout const newLayout = VulkanLayout::TRANSFER_DST;
    VulkanLayout const oldLayout = getLayout(0, miplevel);
    transitionLayout(cmdbuf, range, newLayout);

    vkCmdBlitImage(cmdbuf, stage->image, ImgUtil::getVkLayout(VulkanLayout::TRANSFER_SRC),
            mTextureImage, ImgUtil::getVkLayout(newLayout), 1, blitRegions, VK_FILTER_NEAREST);

    transitionLayout(cmdbuf, range, oldLayout);
}

void VulkanTexture::setPrimaryRange(uint32_t minMiplevel, uint32_t maxMiplevel) {
    maxMiplevel = filament::math::min(int(maxMiplevel), int(this->levels - 1));
    mPrimaryViewRange.baseMipLevel = minMiplevel;
    mPrimaryViewRange.levelCount = maxMiplevel - minMiplevel + 1;
    getImageView(mPrimaryViewRange, mViewType, mSwizzle);
}

VkImageView VulkanTexture::getAttachmentView(VkImageSubresourceRange range) {
    // Attachments should only have one mipmap level and one layer.
    range.levelCount = 1;
    range.layerCount = 1;
    return getImageView(range, VK_IMAGE_VIEW_TYPE_2D, {});
}

VkImageView VulkanTexture::getImageView(VkImageSubresourceRange range, VkImageViewType viewType,
        VkComponentMapping swizzle) {
    auto iter = mCachedImageViews.find(range);
    if (iter != mCachedImageViews.end()) {
        return iter->second;
    }
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = mTextureImage,
        .viewType = viewType,
        .format = mVkFormat,
        .components = swizzle,
        .subresourceRange = range,
    };
    VkImageView imageView;
    vkCreateImageView(mDevice, &viewInfo, VKALLOC, &imageView);
    mCachedImageViews.emplace(range, imageView);
    return imageView;
}

VkImageAspectFlags VulkanTexture::getImageAspect() const {
    // Helper function in VulkanUtility
    return filament::backend::getImageAspect(mVkFormat);
}

void VulkanTexture::transitionLayout(VkCommandBuffer cmdbuf, const VkImageSubresourceRange& range,
        VulkanLayout newLayout) {
    VulkanLayout oldLayout = getLayout(range.baseArrayLayer, range.baseMipLevel);

    #if FILAMENT_VULKAN_VERBOSE
    utils::slog.i << "transition layout of " << mTextureImage << ",layer=" << range.baseArrayLayer
                  << ",level=" << range.baseMipLevel << " from=" << oldLayout << " to=" << newLayout
                  << " format=" << mVkFormat
                  << " depth=" << isDepthFormat(mVkFormat) << utils::io::endl;
    #endif

    ImgUtil::transitionLayout(cmdbuf, {
            .image = mTextureImage,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .subresources = range,
    });

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
            mSubresourceLayouts.clear(first, last);
        }
    } else {
        for (uint32_t layer = firstLayer; layer < lastLayer; ++layer) {
            uint32_t const first = (layer << 16) | firstLevel;
            uint32_t const last = (layer << 16) | lastLevel;
            mSubresourceLayouts.add(first, last, newLayout);
        }
    }
}

VulkanLayout VulkanTexture::getLayout(uint32_t layer, uint32_t level) const {
    assert_invariant(level <= 0xffff && layer <= 0xffff);
    const uint32_t key = (layer << 16) | level;
    if (!mSubresourceLayouts.has(key)) {
        return VulkanLayout::UNDEFINED;
    }
    return mSubresourceLayouts.get(key);
}

#if FILAMENT_VULKAN_VERBOSE
void VulkanTexture::print() const {
    const uint32_t firstLayer = 0;
    const uint32_t lastLayer = firstLayer + mPrimaryViewRange.layerCount;
    const uint32_t firstLevel = 0;
    const uint32_t lastLevel = firstLevel + mPrimaryViewRange.levelCount;

    for (uint32_t layer = firstLayer; layer < lastLayer; ++layer) {
        for (uint32_t level = firstLevel; level < lastLevel; ++level) {
            utils::slog.d << "[" << mTextureImage << "]: (" << layer << "," << level
                          << ")=" << getLayout(layer, level) << utils::io::endl;
        }
    }
}
#endif

} // namespace filament::backend
