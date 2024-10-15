/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "VulkanStagePool.h"

#include "VulkanCommands.h"
#include "VulkanConstants.h"
#include "VulkanImageUtility.h"
#include "VulkanMemory.h"
#include "VulkanUtility.h"

#include <utils/Panic.h>

static constexpr uint32_t TIME_BEFORE_EVICTION = FVK_MAX_COMMAND_BUFFERS;

namespace filament::backend {

VulkanStagePool::VulkanStagePool(VmaAllocator allocator, VulkanCommands* commands)
    : mAllocator(allocator),
      mCommands(commands) {}

VulkanStage const* VulkanStagePool::acquireStage(uint32_t numBytes) {
    // First check if a stage exists whose capacity is greater than or equal to the requested size.
    auto iter = mFreeStages.lower_bound(numBytes);
    if (iter != mFreeStages.end()) {
        auto stage = iter->second;
        mFreeStages.erase(iter);
        mUsedStages.insert(stage);
        return stage;
    }
    // We were not able to find a sufficiently large stage, so create a new one.
    VulkanStage* stage = new VulkanStage({
        .memory = VK_NULL_HANDLE,
        .buffer = VK_NULL_HANDLE,
        .capacity = numBytes,
        .lastAccessed = mCurrentFrame,
    });

    // Create the VkBuffer.
    mUsedStages.insert(stage);
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = numBytes,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_CPU_ONLY };
    UTILS_UNUSED_IN_RELEASE VkResult result = vmaCreateBuffer(mAllocator, &bufferInfo,
            &allocInfo, &stage->buffer, &stage->memory, nullptr);

#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
    if (result != VK_SUCCESS) {
        FVK_LOGE << "Allocation error: " << result << utils::io::endl;
    }
#endif

    return stage;
}

VulkanStageImage const* VulkanStagePool::acquireImage(PixelDataFormat format, PixelDataType type,
        uint32_t width, uint32_t height) {
    const VkFormat vkformat = getVkFormat(format, type);
    for (auto image : mFreeImages) {
        if (image->format == vkformat && image->width == width && image->height == height) {
            mFreeImages.erase(image);
            mUsedImages.insert(image);
            return image;
        }
    }

    VulkanStageImage* image = new VulkanStageImage({
        .format = vkformat,
        .width = width,
        .height = height,
        .lastAccessed = mCurrentFrame,
    });

    mUsedImages.insert(image);

    const VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = vkformat,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    };

    const VmaAllocationCreateInfo allocInfo {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    const UTILS_UNUSED VkResult result = vmaCreateImage(mAllocator, &imageInfo, &allocInfo,
            &image->image, &image->memory, nullptr);

    assert_invariant(result == VK_SUCCESS);

    VkImageAspectFlags const aspectFlags = getImageAspect(vkformat);
    VkCommandBuffer const cmdbuffer = mCommands->get().buffer();

    // We use VK_IMAGE_LAYOUT_GENERAL here because the spec says:
    // "Host access to image memory is only well-defined for linear images and for image
    // subresources of those images which are currently in either the
    // VK_IMAGE_LAYOUT_PREINITIALIZED or VK_IMAGE_LAYOUT_GENERAL layout. Calling
    // vkGetImageSubresourceLayout for a linear image returns a subresource layout mapping that is
    // valid for either of those image layouts."
    imgutil::transitionLayout(cmdbuffer, {
            .image = image->image,
            .oldLayout = VulkanLayout::UNDEFINED,
            .newLayout = VulkanLayout::READ_WRITE, // (= VK_IMAGE_LAYOUT_GENERAL)
            .subresources = { aspectFlags, 0, 1, 0, 1 },
        });
    return image;
}

void VulkanStagePool::gc() noexcept {
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("stagepool::gc");

    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    if (++mCurrentFrame <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint64_t evictionTime = mCurrentFrame - TIME_BEFORE_EVICTION;

    // Destroy buffers that have not been used for several frames.
    decltype(mFreeStages) freeStages;
    freeStages.swap(mFreeStages);
    for (auto pair : freeStages) {
        if (pair.second->lastAccessed < evictionTime) {
            vmaDestroyBuffer(mAllocator, pair.second->buffer, pair.second->memory);
            delete pair.second;
        } else {
            mFreeStages.insert(pair);
        }
    }

    // Reclaim buffers that are no longer being used by any command buffer.
    decltype(mUsedStages) usedStages;
    usedStages.swap(mUsedStages);
    for (auto stage : usedStages) {
        if (stage->lastAccessed < evictionTime) {
            stage->lastAccessed = mCurrentFrame;
            mFreeStages.insert(std::make_pair(stage->capacity, stage));
        } else {
            mUsedStages.insert(stage);
        }
    }

    // Destroy images that have not been used for several frames.
    decltype(mFreeImages) freeImages;
    freeImages.swap(mFreeImages);
    for (auto image : freeImages) {
        if (image->lastAccessed < evictionTime) {
            vmaDestroyImage(mAllocator, image->image, image->memory);
            delete image;
        } else {
            mFreeImages.insert(image);
        }
    }

    // Reclaim images that are no longer being used by any command buffer.
    decltype(mUsedImages) usedImages;
    usedImages.swap(mUsedImages);
    for (auto image : usedImages) {
        if (image->lastAccessed < evictionTime) {
            image->lastAccessed = mCurrentFrame;
            mFreeImages.insert(image);
        } else {
            mUsedImages.insert(image);
        }
    }
    FVK_SYSTRACE_END();
}

void VulkanStagePool::terminate() noexcept {
    for (auto stage : mUsedStages) {
        vmaDestroyBuffer(mAllocator, stage->buffer, stage->memory);
        delete stage;
    }
    mUsedStages.clear();

    for (auto pair : mFreeStages) {
        vmaDestroyBuffer(mAllocator, pair.second->buffer, pair.second->memory);
        delete pair.second;
    }
    mFreeStages.clear();

    for (auto image : mUsedImages) {
        vmaDestroyImage(mAllocator, image->image, image->memory);
        delete image;
    }
    mUsedStages.clear();

    for (auto image : mFreeImages) {
        vmaDestroyImage(mAllocator, image->image, image->memory);
        delete image;
    }
    mFreeStages.clear();
}

} // namespace filament::backend
