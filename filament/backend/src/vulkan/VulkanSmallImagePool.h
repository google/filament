/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANSMALLIMAGEPOOL_H
#define TNT_FILAMENT_BACKEND_VULKANSMALLIMAGEPOOL_H

#include "VulkanMemory.h"

#include <stdint.h>

namespace filament::backend {

struct VulkanContext;

// Owns a VmaPool with small blocks for suballocating small device-local texture images.
//
// VMA's default pool starts at 32 MiB per block on large heaps (256 MiB / 8), which wastes tens of
// megabytes when an app only allocates a few MiB of small textures on GPUs that do not request
// dedicated image allocations (e.g. PowerVR). Using 4 MiB blocks keeps slack small while still
// suballocating multiple small textures per VkDeviceMemory.
//
// The pool is only created when the image memory type differs from the memory type used by
// VulkanBufferCache. When they match (e.g. UMA GPUs whose DEVICE_LOCAL type is also
// HOST_VISIBLE | HOST_COHERENT), small images are better off sharing VMA's default-pool buffer
// blocks, and getPool() always returns VK_NULL_HANDLE.
//
// This class is not thread-safe; it is expected to be used from the backend thread only.
class VulkanSmallImagePool {
public:
    static constexpr VkDeviceSize BLOCK_SIZE = 4u * 1024u * 1024u;

    VulkanSmallImagePool(VmaAllocator allocator, VulkanContext const& context);
    ~VulkanSmallImagePool();

    VulkanSmallImagePool(VulkanSmallImagePool const&) = delete;
    VulkanSmallImagePool& operator=(VulkanSmallImagePool const&) = delete;

    // Returns the pool that an image should be suballocated from, or VK_NULL_HANDLE if the image
    // should go through VMA's default pools. `memoryTypeBits` comes from the image's
    // VkMemoryRequirements. Callers are responsible for only asking on behalf of small,
    // non-dedicated, non-transient, non-protected images with VK_IMAGE_TILING_OPTIMAL (the pool
    // ignores bufferImageGranularity).
    VmaPool getPool(uint32_t memoryTypeBits) const noexcept;

    // Destroys the underlying VmaPool. All allocations made from the pool must have been freed,
    // and this must be called before the VmaAllocator is destroyed.
    void terminate() noexcept;

private:
    VmaAllocator const mAllocator;
    VmaPool mPool = VK_NULL_HANDLE;
    uint32_t mMemoryTypeIndex = VK_MAX_MEMORY_TYPES;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANSMALLIMAGEPOOL_H
