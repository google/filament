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

#include "driver/vulkan/VulkanStagePool.h"

#include <utils/Panic.h>

namespace filament {
namespace driver {

VulkanStage const* VulkanStagePool::acquireStage(uint32_t numBytes) noexcept {
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
        .lastAccessed = mCurrentFrame,
        .capacity = numBytes,
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
    vmaCreateBuffer(mContext.allocator, &bufferInfo, &allocInfo, &stage->buffer, &stage->memory, nullptr);
    return stage;
}

void VulkanStagePool::releaseStage(VulkanStage const* stage) noexcept {
    auto iter = mUsedStages.find(stage);
    if (iter == mUsedStages.end()) {
        utils::slog.e << "Unknown stage: " << stage->capacity << " bytes" << utils::io::endl;
        return;
    }
    stage->lastAccessed = mCurrentFrame;
    mUsedStages.erase(iter);
    mFreeStages.insert(std::make_pair(stage->capacity, stage));
}

void VulkanStagePool::gc() noexcept {
    mCurrentFrame++;
    decltype(mFreeStages) stages;
    stages.swap(mFreeStages);
    const uint64_t evictionTime = mCurrentFrame - TIME_BEFORE_EVICTION;
    for (auto pair : stages) {
        if (pair.second->lastAccessed < evictionTime) {
            vmaDestroyBuffer(mContext.allocator, pair.second->buffer, pair.second->memory);
        } else {
            mFreeStages.insert(pair);
        }
    }
}

void VulkanStagePool::reset() noexcept {
    assert(mUsedStages.empty());
    for (auto pair : mFreeStages) {
        vmaDestroyBuffer(mContext.allocator, pair.second->buffer, pair.second->memory);
    }
    mFreeStages.clear();
}

} // namespace filament
} // namespace driver
