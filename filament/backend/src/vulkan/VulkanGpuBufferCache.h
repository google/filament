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

#ifndef TNT_FILAMENT_BACKEND_VULKANGPUBUFFERCACHE_H
#define TNT_FILAMENT_BACKEND_VULKANGPUBUFFERCACHE_H

#include "VulkanContext.h"
#include "VulkanMemory.h"
#include "memory/Resource.h"
#include "memory/ResourceManager.h"

#include <map>

namespace filament::backend {

class VulkanGpuBufferCache {
public:
    VulkanGpuBufferCache(VulkanContext const& context, fvkmemory::ResourceManager& resourceManager,
            VmaAllocator allocator);

    // `VulkanGpuBufferCache` is not copyable.
    VulkanGpuBufferCache(const VulkanGpuBufferCache&) = delete;
    VulkanGpuBufferCache& operator=(const VulkanGpuBufferCache&) = delete;

    // Allocates or reuse a new VkBuffer that is device local.
    // In the case of Unified memory architecture, uniform buffers are also host visible.
    fvkmemory::resource_ptr<VulkanGpuBufferHolder> acquire(VulkanBufferUsage usage,
            uint32_t numBytes) noexcept;

    // Return a `VulkanGpuBuffer` back to its corresponding pool
    void yield(VulkanGpuBuffer const* gpuBuffer) noexcept;

    // Evicts old unused `VulkanGpuBuffer` and bumps the current frame number
    void gc() noexcept;

    // Destroys all unused `VulkanGpuBuffer`.
    // This should be called while the context's VkDevice is still alive.
    void terminate() noexcept;

private:
    struct UnusedGpuBuffer {
        uint64_t lastAccessed;
        VulkanGpuBuffer const* gpuBuffer;
    };

    using BufferPool = std::multimap<uint32_t, UnusedGpuBuffer>;

    // Allocate a new VkBuffer from the VMA pool of the corresponding `numBytes` and `usage`.
    VulkanGpuBuffer const* allocate(VulkanBufferUsage usage, uint32_t numBytes) noexcept;

    // Destroy the corresponding VkBuffer and return the VkDeviceMemory to the VMA pool.
    void destroy(VulkanGpuBuffer const* gpuBuffer) noexcept;

    BufferPool& getPool(VulkanBufferUsage usage) noexcept;

    VulkanContext const& mContext;
    fvkmemory::ResourceManager& mResourceManager;
    VmaAllocator mAllocator;

    // Buffers can be recycled, after they are released. Each type of buffer have its own pool
    static constexpr int MAX_POOL_COUNT = 4;
    BufferPool mGpuBufferPools[MAX_POOL_COUNT];

    // Store the current "time" (really just a frame count) and LRU eviction parameters.
    uint64_t mCurrentFrame = 0;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANGPUBUFFERCACHE_H
