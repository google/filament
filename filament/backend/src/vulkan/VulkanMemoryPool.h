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

#ifndef TNT_FILAMENT_BACKEND_VULKANMEMORYPOOL_H
#define TNT_FILAMENT_BACKEND_VULKANMEMORYPOOL_H

#include "VulkanContext.h"
#include "VulkanMemory.h"
#include "memory/Resource.h"
#include "memory/ResourceManager.h"

#include <map>

namespace filament::backend {

class VulkanMemoryPool {
public:
    VulkanMemoryPool(VulkanContext const& context, fvkmemory::ResourceManager& resourceManager,
            VmaAllocator allocator);

    // `VulkanMemoryPool` is not copyable.
    VulkanMemoryPool(const VulkanMemoryPool&) = delete;
    VulkanMemoryPool& operator=(const VulkanMemoryPool&) = delete;

    fvkmemory::resource_ptr<VulkanBufferMemory> acquire(VulkanBufferUsage usage,
            uint32_t numBytes) noexcept;

    // Return an allocation back to the pool
    void yield(VulkanMemoryPoolAllocation const* poolAllocation) noexcept;

    // Evicts old unused allocations and bumps the current frame number
    void gc() noexcept;

    // Destroys all unused allocations.
    // This should be called while the context's VkDevice is still alive.
    void terminate() noexcept;

private:
    VulkanMemoryPoolAllocation const* allocateMemory(VulkanBufferUsage usage,
            uint32_t numBytes) noexcept;

    void releaseMemory(VulkanMemoryPoolAllocation const* poolAllocation) noexcept;

    struct FreeMemoryPoolAllocation {
        uint64_t lastAccessed;
        VulkanMemoryPoolAllocation const* poolAllocation;
    };

    VulkanContext const& mContext;
    fvkmemory::ResourceManager& mResourceManager;
    VmaAllocator mAllocator;

    // Allocation for uniform buffers are kept for a few of frames so they can be recycled,
    // otherwise they are destroyed.
    std::multimap<uint32_t, FreeMemoryPoolAllocation> mFreeUniformAllocations;

    // Store the current "time" (really just a frame count) and LRU eviction parameters.
    uint64_t mCurrentFrame = 0;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANMEMORYPOOL_H
