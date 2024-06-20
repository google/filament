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

#include <DataReshaper.h>
#include <backend/DriverEnums.h>
#include <private/backend/BackendUtils.h>

#include <utils/Panic.h>

using namespace bluevk;

namespace filament::backend {

VulkanTexture::VulkanTexture(VkDevice device, VmaAllocator allocator, VulkanCommands* commands,
        VkImage image, VkFormat format, uint8_t samples, uint32_t width, uint32_t height,
        TextureUsage tusage, VulkanStagePool& stagePool, bool heapAllocated)
    : HwTexture(SamplerType::SAMPLER_2D, 1, samples, width, height, 1, TextureFormat::UNUSED,
            tusage),
      VulkanResource(
              heapAllocated ? VulkanResourceType::HEAP_ALLOCATED : VulkanResourceType::TEXTURE),
      mVkFormat(format),
      mViewType(imgutil::getViewType(target)),
      mSwizzle({}),
      mTextureImage(image),
      mFullViewRange{
              .aspectMask = getImageAspect(),
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
      },
      mPrimaryViewRange(mFullViewRange),
      mStagePool(stagePool),
      mDevice(device),
      mAllocator(allocator),
      mCommands(commands) {}

VulkanTexture::VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice,
        VulkanContext const& context, VmaAllocator allocator, VulkanCommands* commands,
        SamplerType target, uint8_t levels, TextureFormat tformat, uint8_t samples, uint32_t w,
        uint32_t h, uint32_t depth, TextureUsage tusage, VulkanStagePool& stagePool,
        bool heapAllocated, VkComponentMapping swizzle)
    : HwTexture(target, levels, samples, w, h, depth, tformat, tusage),
      VulkanResource(
              heapAllocated ? VulkanResourceType::HEAP_ALLOCATED : VulkanResourceType::TEXTURE),
      mVkFormat(backend::getVkFormat(tformat)),
      mViewType(imgutil::getViewType(target)),
      mSwizzle(swizzle),
      mStagePool(stagePool),
      mDevice(device),
      mAllocator(allocator),
      mCommands(commands) {

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
    constexpr VkImageUsageFlags blittable = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (any(usage & (TextureUsage::BLIT_DST | TextureUsage::BLIT_SRC))) {
        imageInfo.usage |= blittable;
    }

    if (any(usage & TextureUsage::SAMPLEABLE)) {

#if FVK_ENABLED(FVK_DEBUG_TEXTURE)
        // Validate that the format is actually sampleable.
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, mVkFormat, &props);
        if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
            FVK_LOGW << "Texture usage is SAMPLEABLE but format " << mVkFormat << " is not "
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
        samples = reduceSampleCount(samples, isVkDepthFormat(mVkFormat)
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
    if (error || FVK_ENABLED(FVK_DEBUG_TEXTURE)) {
        FVK_LOGD << "vkCreateImage: "
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
    FILAMENT_CHECK_POSTCONDITION(!error) << "Unable to create image.";

    // Allocate memory for the VkImage and bind it.
    VkMemoryRequirements memReqs = {};
    vkGetImageMemoryRequirements(mDevice, mTextureImage, &memReqs);

    uint32_t memoryTypeIndex
            = context.selectMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    FILAMENT_CHECK_POSTCONDITION(memoryTypeIndex < VK_MAX_MEMORY_TYPES)
            << "VulkanTexture: unable to find a memory type that meets requirements.";

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = memoryTypeIndex,
    };
    error = vkAllocateMemory(mDevice, &allocInfo, nullptr, &mTextureImageMemory);
    FILAMENT_CHECK_POSTCONDITION(!error) << "Unable to allocate image memory.";
    error = vkBindImageMemory(mDevice, mTextureImage, mTextureImageMemory, 0);
    FILAMENT_CHECK_POSTCONDITION(!error) << "Unable to bind image.";

    uint32_t layerCount = 0;
    if (target == SamplerType::SAMPLER_CUBEMAP) {
        layerCount = 6;
    } else if (target == SamplerType::SAMPLER_CUBEMAP_ARRAY) {
        layerCount = depth * 6;
    } else if (target == SamplerType::SAMPLER_2D_ARRAY) {
        layerCount = depth;
    } else if (target == SamplerType::SAMPLER_3D) {
        layerCount = 1;
    } else {
        layerCount = 1;
    }

    mFullViewRange = {
        .aspectMask = getImageAspect(),
        .baseMipLevel = 0,
        .levelCount = levels,
        .baseArrayLayer = 0,
        .layerCount = layerCount,
    };

    // Spec out the "primary" VkImageView that shaders use to sample from the image.
    mPrimaryViewRange = mFullViewRange;

    // Go ahead and create the primary image view.
    getImageView(mPrimaryViewRange, mViewType, mSwizzle);

    VulkanCommandBuffer& commandsBuf = mCommands->get();
    commandsBuf.acquire(this);
    transitionLayout(&commandsBuf, mFullViewRange, imgutil::getDefaultLayout(imageInfo.usage));
}


VulkanTexture::VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice,
        VulkanContext const& context, VmaAllocator allocator, VulkanCommands* commands,
        VulkanTexture const* src, uint8_t baseLevel, uint8_t levelCount,
        VulkanStagePool& stagePool)
        : HwTexture(src->target, src->levels, src->samples, src->width, src->height, src->depth,
                src->format, src->usage),
          VulkanResource(VulkanResourceType::TEXTURE),
          mVkFormat(src->mVkFormat),
          mViewType(src->mViewType),
          mSwizzle(src->mSwizzle),
          mStagePool(stagePool),
          mDevice(device),
          mAllocator(allocator),
          mCommands(commands) {
    // TODO: implement me
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

    assert_invariant(hostData->size > 0 && "Data is empty");

    // Otherwise, use vkCmdCopyBufferToImage.
    void* mapped = nullptr;
    VulkanStage const* stage = mStagePool.acquireStage(hostData->size);
    assert_invariant(stage->memory);
    vmaMapMemory(mAllocator, stage->memory, &mapped);
    memcpy(mapped, hostData->buffer, hostData->size);
    vmaUnmapMemory(mAllocator, stage->memory);
    vmaFlushAllocation(mAllocator, stage->memory, 0, hostData->size);

    VulkanCommandBuffer& commands = mCommands->get();
    VkCommandBuffer const cmdbuf = commands.buffer();
    commands.acquire(this);

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
    VulkanLayout nextLayout = getLayout(transitionRange.baseArrayLayer, miplevel);
    VkImageLayout const newVkLayout = imgutil::getVkLayout(newLayout);

    if (nextLayout == VulkanLayout::UNDEFINED) {
        nextLayout = imgutil::getDefaultLayout(this->usage);
    }

    transitionLayout(&commands, transitionRange, newLayout);

    vkCmdCopyBufferToImage(cmdbuf, stage->buffer, mTextureImage, newVkLayout, 1, &copyRegion);

    transitionLayout(&commands, transitionRange, nextLayout);
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

    VulkanCommandBuffer& commands = mCommands->get();
    VkCommandBuffer const cmdbuf = commands.buffer();
    commands.acquire(this);

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

    const VkImageSubresourceRange range = { aspect, miplevel, 1, layer, 1 };

    VulkanLayout const newLayout = VulkanLayout::TRANSFER_DST;
    VulkanLayout const oldLayout = getLayout(layer, miplevel);
    transitionLayout(&commands, range, newLayout);

    vkCmdBlitImage(cmdbuf, stage->image, imgutil::getVkLayout(VulkanLayout::TRANSFER_SRC),
            mTextureImage, imgutil::getVkLayout(newLayout), 1, blitRegions, VK_FILTER_NEAREST);

    transitionLayout(&commands, range, oldLayout);
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

VkImageView VulkanTexture::getViewForType(VkImageSubresourceRange const& range, VkImageViewType type) {
    return getImageView(range, type, mSwizzle);
}

VkImageView VulkanTexture::getImageView(VkImageSubresourceRange range, VkImageViewType viewType,
        VkComponentMapping swizzle) {
    ImageViewKey const key {range, viewType, swizzle};
    auto iter = mCachedImageViews.find(key);
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
    mCachedImageViews.emplace(key, imageView);
    return imageView;
}

VkImageAspectFlags VulkanTexture::getImageAspect() const {
    // Helper function in VulkanUtility
    return filament::backend::getImageAspect(mVkFormat);
}

void VulkanTexture::transitionLayout(VulkanCommandBuffer* commands,
        const VkImageSubresourceRange& range, VulkanLayout newLayout) {
    transitionLayout(commands->buffer(), commands->fence, range, newLayout);
}

void VulkanTexture::transitionLayout(
        VkCommandBuffer cmdbuf, std::shared_ptr<VulkanCmdFence> fence,
        const VkImageSubresourceRange& range,
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
                hasTransitions = hasTransitions || imgutil::transitionLayout(cmdbuf, {
                    .image = mTextureImage,
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
        hasTransitions = imgutil::transitionLayout(cmdbuf, {
            .image = mTextureImage,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .subresources = range,
        });
    }

    if (hasTransitions) {
        mTransitionFence = fence;
        setLayout(range, newLayout);

#if FVK_ENABLED(FVK_DEBUG_LAYOUT_TRANSITION)
        FVK_LOGD << "transition texture=" << mTextureImage << " (" << range.baseArrayLayer
                       << "," << range.baseMipLevel << ")" << " count=(" << range.layerCount << ","
                       << range.levelCount << ")" << " from=" << oldLayout << " to=" << newLayout
                       << " format=" << mVkFormat << " depth=" << isVkDepthFormat(mVkFormat)
                       << " slice-by-slice=" << transitionSliceBySlice << utils::io::endl;
#endif
    } else {
#if FVK_ENABLED(FVK_DEBUG_LAYOUT_TRANSITION)
        FVK_LOGD << "transition texture=" << mTextureImage << " (" << range.baseArrayLayer
                      << "," << range.baseMipLevel << ")" << " count=(" << range.layerCount << ","
                      << range.levelCount << ")" << " to=" << newLayout
                      << " is skipped because of no change in layout" << utils::io::endl;
#endif
    }
}

void VulkanTexture::setLayout(const VkImageSubresourceRange& range, VulkanLayout newLayout) {
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

#if FVK_ENABLED(FVK_DEBUG_TEXTURE)
void VulkanTexture::print() const {
    uint32_t const firstLayer = 0;
    uint32_t const lastLayer = firstLayer + mFullViewRange.layerCount;
    uint32_t const firstLevel = 0;
    uint32_t const lastLevel = firstLevel + mFullViewRange.levelCount;

    for (uint32_t layer = firstLayer; layer < lastLayer; ++layer) {
        for (uint32_t level = firstLevel; level < lastLevel; ++level) {
            bool primary =
                layer >= mPrimaryViewRange.baseArrayLayer &&
                layer < (mPrimaryViewRange.baseArrayLayer + mPrimaryViewRange.layerCount) &&
                level >= mPrimaryViewRange.baseMipLevel &&
                level < (mPrimaryViewRange.baseMipLevel + mPrimaryViewRange.levelCount);
            FVK_LOGD << "[" << mTextureImage << "]: (" << layer << "," << level
                          << ")=" << getLayout(layer, level)
                          << " primary=" << primary
                          << utils::io::endl;
        }
    }

    for (auto view: mCachedImageViews) {
        auto& range = view.first.range;
        FVK_LOGD << "[" << mTextureImage << ", imageView=" << view.second << "]=>"
                      << " (" << range.baseArrayLayer << "," << range.baseMipLevel << ")"
                      << " count=(" << range.layerCount << "," << range.levelCount << ")"
                      << " aspect=" << range.aspectMask << " viewType=" << view.first.type
                      << utils::io::endl;
    }
}
#endif

} // namespace filament::backend
