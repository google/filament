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

#include "VulkanSmallImagePool.h"

#include "VulkanContext.h"

#include <utils/debug.h>

namespace filament::backend {

namespace {

// Returns the memory type index that small sampled images will be allocated from, or
// VK_MAX_MEMORY_TYPES if a dedicated small-image pool should not be used.
uint32_t findSmallImageMemoryTypeIndex(VmaAllocator allocator, VulkanContext const& context) {
    VkImageCreateInfo const sampleImageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = { 1, 1, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VmaAllocationCreateInfo const sampleAllocInfo = {
        .usage = VMA_MEMORY_USAGE_UNKNOWN,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };
    uint32_t imageMemoryTypeIndex = VK_MAX_MEMORY_TYPES;
    VkResult result = vmaFindMemoryTypeIndexForImageInfo(allocator, &sampleImageInfo,
            &sampleAllocInfo, &imageMemoryTypeIndex);
    if (result != VK_SUCCESS) {
        return VK_MAX_MEMORY_TYPES;
    }

    // Check whether GPU buffers (VulkanBufferCache::allocate) already land on the same memory
    // type as small images. When they match (e.g., UMA GPUs whose DEVICE_LOCAL type is also
    // HOST_VISIBLE | HOST_COHERENT), VMA's default pool lets small images suballocate into spare
    // space inside existing buffer blocks; creating a separate pool would force an extra
    // VkDeviceMemory block. When they differ (e.g., GPUs with separate DEVICE_LOCAL-only vs.
    // HOST_VISIBLE types), a dedicated small-block pool avoids opening a 32 MiB default block just
    // for small images.
    VkBufferCreateInfo const sampleBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = 1024,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    VmaAllocationCreateInfo const sampleBufferAllocInfo = {
        .flags = context.isUnifiedMemoryArchitecture()
                ? (VMA_ALLOCATION_CREATE_MAPPED_BIT |
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
                : VmaAllocationCreateFlags(0),
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };
    uint32_t bufferMemoryTypeIndex = VK_MAX_MEMORY_TYPES;
    result = vmaFindMemoryTypeIndexForBufferInfo(allocator, &sampleBufferInfo,
            &sampleBufferAllocInfo, &bufferMemoryTypeIndex);
    if (result == VK_SUCCESS && bufferMemoryTypeIndex == imageMemoryTypeIndex) {
        return VK_MAX_MEMORY_TYPES;
    }
    return imageMemoryTypeIndex;
}

} // anonymous namespace

VulkanSmallImagePool::VulkanSmallImagePool(VmaAllocator allocator, VulkanContext const& context)
    : mAllocator(allocator) {
    uint32_t const memoryTypeIndex = findSmallImageMemoryTypeIndex(allocator, context);
    if (memoryTypeIndex >= VK_MAX_MEMORY_TYPES) {
        return;
    }

    VmaPoolCreateInfo const poolCreateInfo = {
        .memoryTypeIndex = memoryTypeIndex,
        // Only VK_IMAGE_TILING_OPTIMAL images are allocated from this pool, so there is never a
        // buffer/linear-image neighbor and bufferImageGranularity padding is unnecessary.
        .flags = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT,
        .blockSize = BLOCK_SIZE,
        .minBlockCount = 0,
    };
    VmaPool pool = VK_NULL_HANDLE;
    if (vmaCreatePool(allocator, &poolCreateInfo, &pool) != VK_SUCCESS) {
        return;
    }
    mPool = pool;
    mMemoryTypeIndex = memoryTypeIndex;
}

VulkanSmallImagePool::~VulkanSmallImagePool() {
    assert_invariant(mPool == VK_NULL_HANDLE);
}

VmaPool VulkanSmallImagePool::getPool(uint32_t memoryTypeBits) const noexcept {
    if (mPool == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    assert_invariant(mMemoryTypeIndex < VK_MAX_MEMORY_TYPES);
    if ((memoryTypeBits & (1u << mMemoryTypeIndex)) == 0) {
        return VK_NULL_HANDLE;
    }
    return mPool;
}

void VulkanSmallImagePool::terminate() noexcept {
    if (mPool != VK_NULL_HANDLE) {
        vmaDestroyPool(mAllocator, mPool);
        mPool = VK_NULL_HANDLE;
    }
    mMemoryTypeIndex = VK_MAX_MEMORY_TYPES;
}

} // namespace filament::backend
