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

#include "VulkanBufferCache.h"

#include "VulkanBuffer.h"
#include "VulkanConstants.h"
#include "VulkanMemory.h"
#include "memory/Resource.h"
#include "memory/ResourceManager.h"

#include <utility>

namespace filament::backend {

namespace {

VkBufferUsageFlags getVkBufferUsage(VulkanBufferUsage usage) {
    switch (usage) {
        case VulkanBufferUsage::VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case VulkanBufferUsage::INDEX:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case VulkanBufferUsage::UNIFORM:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case VulkanBufferUsage::SHADER_STORAGE:
            return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case VulkanBufferUsage::UNKNOWN:
            return 0;
    }

    return 0;
}

}// namespace

VulkanBufferCache::VulkanBufferCache(VulkanContext const& context,
        fvkmemory::ResourceManager& resourceManager, VmaAllocator allocator)
    : mContext(context),
      mResourceManager(resourceManager),
      mAllocator(allocator) {}

fvkmemory::resource_ptr<VulkanBuffer> VulkanBufferCache::acquire(VulkanBufferUsage usage,
        uint32_t numBytes) noexcept {
    assert_invariant(usage != VulkanBufferUsage::UNKNOWN);

    BufferPool& bufferPool = getPool(usage);

    // First check if an allocation exists whose capacity is greater than or equal to the requested
    // size.
    auto iter = bufferPool.lower_bound(numBytes);
    if (iter != bufferPool.end()) {
        VulkanGpuBuffer const* gpuBuffer = iter->second.gpuBuffer;
        bufferPool.erase(iter);
        return fvkmemory::resource_ptr<VulkanBuffer>::construct(&mResourceManager, gpuBuffer,
                [this](VulkanGpuBuffer const* gpuBuffer) { this->release(gpuBuffer); });
    }

    // We were not able to find a sufficiently large allocation, so create a new one that is
    // recycled after being yielded.
    VulkanGpuBuffer const* gpuBuffer = allocate(usage, numBytes);
    return fvkmemory::resource_ptr<VulkanBuffer>::construct(&mResourceManager, gpuBuffer,
            [this](VulkanGpuBuffer const* gpuBuffer) { this->release(gpuBuffer); });
}

void VulkanBufferCache::gc() noexcept {
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("VulkanBufferCache::gc");

    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    constexpr uint32_t TIME_BEFORE_EVICTION = 3;
    if (++mCurrentFrame <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint64_t evictionTime = mCurrentFrame - TIME_BEFORE_EVICTION;

    // Destroy buffers that have not been used for several frames.
    for (auto& bufferPool: mGpuBufferPools) {
        for (auto poolIter = bufferPool.begin(); poolIter != bufferPool.end();) {
            if (poolIter->second.lastAccessed < evictionTime) {
#if FVK_ENABLED(FVK_DEBUG_VULKAN_BUFFER_CACHE)
                FVK_LOGD << "VulkanBufferCache - Destroyed vkBuffer "
                         << poolIter->second.gpuBuffer->vkbuffer << " with usage "
                         << static_cast<int>(poolIter->second.gpuBuffer->usage) << utils::io::endl;
#endif// FVK_DEBUG_VULKAN_BUFFER_CACHE

                destroy(poolIter->second.gpuBuffer);
                poolIter = bufferPool.erase(poolIter);
            } else {
                ++poolIter;
            }
        }
    }

    FVK_SYSTRACE_END();
}

void VulkanBufferCache::terminate() noexcept {
    for (auto& bufferPool: mGpuBufferPools) {
        for (auto& poolEntry: bufferPool) {
            destroy(poolEntry.second.gpuBuffer);
        }
        bufferPool.clear();
    }
}

void VulkanBufferCache::release(VulkanGpuBuffer const* gpuBuffer) noexcept {
    assert_invariant(gpuBuffer != nullptr);

    BufferPool& bufferPool = getPool(gpuBuffer->usage);
    bufferPool.insert(std::make_pair(gpuBuffer->numBytes, UnusedGpuBuffer{
                                                              .lastAccessed = mCurrentFrame,
                                                              .gpuBuffer = gpuBuffer,
                                                          }));
}

VulkanGpuBuffer const* VulkanBufferCache::allocate(VulkanBufferUsage usage,
        uint32_t numBytes) noexcept {
    VkBufferCreateInfo const bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = numBytes,
        // `VK_BUFFER_USAGE_TRANSFER_DST_BIT` is needed to allow updating the buffer through
        // a staging using `vkCmdCopyBuffer`.
        .usage = getVkBufferUsage(usage) | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };

    VmaAllocationCreateFlags vmaFlags = 0;
    if (usage == VulkanBufferUsage::UNIFORM) {
        // In the case of UMA, the uniform buffers will always be mappable
        if (mContext.isUnifiedMemoryArchitecture()) {
            vmaFlags |= VMA_ALLOCATION_CREATE_MAPPED_BIT |
                        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        }
    }

    VulkanGpuBuffer* gpuBuffer = new VulkanGpuBuffer{
        .numBytes = numBytes,
        .usage = usage,
    };
    VmaAllocationCreateInfo const allocInfo{
        .flags = vmaFlags,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };
    UTILS_UNUSED_IN_RELEASE VkResult result = vmaCreateBuffer(mAllocator, &bufferInfo, &allocInfo,
            &gpuBuffer->vkbuffer, &gpuBuffer->vmaAllocation, &gpuBuffer->allocationInfo);

#if FVK_ENABLED(FVK_DEBUG_VULKAN_BUFFER_CACHE)
    if (result != VK_SUCCESS) {
        FVK_LOGE << "VulkanBufferCache - failed to allocate a new vkBuffer of size " << numBytes
                 << " and usage " << static_cast<int>(usage) << ", error: " << result
                 << utils::io::endl;
    } else {
        FVK_LOGD << "VulkanBufferCache - allocated a vkBuffer " << gpuBuffer->vkbuffer
                 << " of size " << numBytes << " and usage = " << static_cast<int>(usage)
                 << "  successfully" << utils::io::endl;
    }
#endif// FVK_DEBUG_VULKAN_BUFFER_CACHE

    return gpuBuffer;
}

void VulkanBufferCache::destroy(VulkanGpuBuffer const* gpuBuffer) noexcept {
    vmaDestroyBuffer(mAllocator, gpuBuffer->vkbuffer, gpuBuffer->vmaAllocation);
    delete gpuBuffer;
    gpuBuffer = nullptr;
}

VulkanBufferCache::BufferPool& VulkanBufferCache::getPool(VulkanBufferUsage usage) noexcept {

    int poolIndex = -1;
    switch (usage) {
        case VulkanBufferUsage::VERTEX:
            poolIndex = 0;
            break;
        case VulkanBufferUsage::INDEX:
            poolIndex = 1;
            break;
        case VulkanBufferUsage::UNIFORM:
            poolIndex = 2;
            break;
        case VulkanBufferUsage::SHADER_STORAGE:
            poolIndex = 3;
            break;
        case VulkanBufferUsage::UNKNOWN:
            PANIC_LOG("There's no pool for buffers with unkown usage.");
            break;
    }

    assert_invariant(poolIndex >= 0 && poolIndex < MAX_POOL_COUNT);
    return mGpuBufferPools[poolIndex];
}

}// namespace filament::backend
