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

#ifndef TNT_FILAMENT_DRIVER_VULKANSTAGEPOOL_H
#define TNT_FILAMENT_DRIVER_VULKANSTAGEPOOL_H

#include "VulkanContext.h"

#include "VulkanDisposer.h"

#include <map>
#include <unordered_set>

namespace filament {
namespace backend {

// Immutable POD representing a shared CPU-GPU staging area.
struct VulkanStage {
    VmaAllocation memory;
    VkBuffer buffer;
    uint32_t capacity;
    mutable uint64_t lastAccessed;
};

// Manages a pool of stages, periodically releasing stages that have been unused for a while.
class VulkanStagePool {
public:
    explicit VulkanStagePool(VulkanContext& context, VulkanDisposer& disposer) noexcept :
            mContext(context), mDisposer(disposer) {}

    // Finds or creates a stage whose capacity is at least the given number of bytes.
    VulkanStage const* acquireStage(uint32_t numBytes);

    // Returns the given stage back to the pool.
    void releaseStage(VulkanStage const* stage) noexcept;
    void releaseStage(VulkanStage const* stage, VulkanCommandBuffer& cmd) noexcept;

    // Evicts old unused stages and bumps the current frame number.
    void gc() noexcept;

    // Destroys all unused stages and asserts that there are no stages currently in use.
    // This should be called while the context's VkDevice is still alive.
    void reset() noexcept;

private:
    VulkanContext& mContext;
    VulkanDisposer& mDisposer;

    // Use an ordered multimap for quick (capacity => stage) lookups using lower_bound().
    std::multimap<uint32_t, VulkanStage const*> mFreeStages;

    // Simple unordered set for stashing a list of in-use stages that can be reclaimed later.
    // In theory this need not exist, but is useful for validation and ensuring no leaks.
    std::unordered_set<VulkanStage const*> mUsedStages;

    // Store the current "time" (really just a frame count) and LRU eviction parameters.
    uint64_t mCurrentFrame = 0;
    static constexpr uint32_t TIME_BEFORE_EVICTION = 2;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANSTAGEPOOL_H
