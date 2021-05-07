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
    VmaAllocationCreateInfo allocInfo {
        .usage = VMA_MEMORY_USAGE_CPU_ONLY
    };
    vmaCreateBuffer(mContext.allocator, &bufferInfo, &allocInfo, &stage->buffer, &stage->memory,
            nullptr);

    return stage;
}

void VulkanStagePool::gc() noexcept {
    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    if (++mCurrentFrame <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint64_t evictionTime = mCurrentFrame - TIME_BEFORE_EVICTION;

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
}

} // namespace filament
} // namespace backend
