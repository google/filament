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

#include "VulkanConstants.h"
#include "VulkanMemory.h"
#include "VulkanUtility.h"

#include <utils/Panic.h>

static constexpr uint32_t TIME_BEFORE_EVICTION = VK_MAX_COMMAND_BUFFERS;

namespace filament {
namespace backend {

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
    VmaAllocationCreateInfo allocInfo { .pool = mContext.vmaPoolCPU };
    vmaCreateBuffer(mContext.allocator, &bufferInfo, &allocInfo, &stage->buffer, &stage->memory,
            nullptr);

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

    const UTILS_UNUSED VkResult result = vmaCreateImage(mContext.allocator, &imageInfo, &allocInfo,
            &image->image, &image->memory, nullptr);

    assert_invariant(result == VK_SUCCESS);

    return image;
}

void VulkanStagePool::gc() noexcept {
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
            vmaDestroyBuffer(mContext.allocator, pair.second->buffer, pair.second->memory);
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
            vmaDestroyImage(mContext.allocator, image->image, image->memory);
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
}

void VulkanStagePool::reset() noexcept {
    for (auto stage : mUsedStages) {
        vmaDestroyBuffer(mContext.allocator, stage->buffer, stage->memory);
        delete stage;
    }
    mUsedStages.clear();

    for (auto pair : mFreeStages) {
        vmaDestroyBuffer(mContext.allocator, pair.second->buffer, pair.second->memory);
        delete pair.second;
    }
    mFreeStages.clear();

    for (auto image : mUsedImages) {
        vmaDestroyImage(mContext.allocator, image->image, image->memory);
        delete image;
    }
    mUsedStages.clear();

    for (auto image : mFreeImages) {
        vmaDestroyImage(mContext.allocator, image->image, image->memory);
        delete image;
    }
    mFreeStages.clear();
}

} // namespace filament
} // namespace backend
