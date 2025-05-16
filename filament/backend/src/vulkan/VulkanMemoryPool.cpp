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

#include "VulkanMemoryPool.h"

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

} // namespace

VulkanMemoryPool::VulkanMemoryPool(VulkanContext const& context,
        fvkmemory::ResourceManager& resourceManager, VmaAllocator allocator)
    : mContext(context),
      mResourceManager(resourceManager),
      mAllocator(allocator) {}

fvkmemory::resource_ptr<VulkanBufferMemory> VulkanMemoryPool::acquire(VulkanBufferUsage usage,
        uint32_t numBytes) noexcept {
    assert_invariant(usage != VulkanBufferUsage::UNKNOWN);

    if (usage != VulkanBufferUsage::UNIFORM) {
        // Allocate memory for vertex, index or storage buffer won't be recycled.
        VulkanMemoryPoolAllocation const* poolAllocation = allocateMemory(usage, numBytes);
        return fvkmemory::resource_ptr<VulkanBufferMemory>::construct(&mResourceManager,
                poolAllocation, [this](VulkanMemoryPoolAllocation const* poolAllocation) {
                    this->yield(poolAllocation);
                });
    }

    // First check if an allocation exists whose capacity is greater than or equal to the requested
    // size.
    auto iter = mFreeUniformAllocations.lower_bound(numBytes);
    if (iter != mFreeUniformAllocations.end()) {
        VulkanMemoryPoolAllocation const* poolAllocation = iter->second.poolAllocation;
        mFreeUniformAllocations.erase(iter);
        return fvkmemory::resource_ptr<VulkanBufferMemory>::construct(&mResourceManager,
                poolAllocation, [this](VulkanMemoryPoolAllocation const* poolAllocation) {
                    this->yield(poolAllocation);
                });
    }

    // We were not able to find a sufficiently large allocation, so create a new one that is
    // recycled after being yielded.
    VulkanMemoryPoolAllocation const* uniformPoolAllocation = allocateMemory(usage, numBytes);
    return fvkmemory::resource_ptr<VulkanBufferMemory>::construct(&mResourceManager,
            uniformPoolAllocation, [this](VulkanMemoryPoolAllocation const* poolAllocation) {
                this->yield(poolAllocation);
            });
}

void VulkanMemoryPool::yield(VulkanMemoryPoolAllocation const* poolAllocation) noexcept {
    if (!poolAllocation) {
        return;
    }

    if (poolAllocation->usage == VulkanBufferUsage::UNIFORM) {
        mFreeUniformAllocations.insert(
                std::make_pair(poolAllocation->numBytes, FreeMemoryPoolAllocation{
                                                             .lastAccessed = mCurrentFrame,
                                                             .poolAllocation = poolAllocation,
                                                         }));
        return;
    }

    releaseMemory(poolAllocation);
}

void VulkanMemoryPool::gc() noexcept {
    FVK_SYSTRACE_CONTEXT();
    FVK_SYSTRACE_START("VulkanMemoryPool::gc");

    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    constexpr uint32_t TIME_BEFORE_EVICTION = 3;
    if (++mCurrentFrame <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint64_t evictionTime = mCurrentFrame - TIME_BEFORE_EVICTION;

    // Destroy buffers that have not been used for several frames.
    for (auto iter = mFreeUniformAllocations.begin(); iter != mFreeUniformAllocations.end();) {
        if (iter->second.lastAccessed < evictionTime) {
#if FVK_ENABLED(FVK_DEBUG_MEMORY_POOL_ALLOCATION)
            FVK_LOGD << "Memory pool destroyed vkBuffer " << iter->second.poolAllocation->vkbuffer
                     << utils::io::endl;
#endif // FVK_DEBUG_MEMORY_POOL_ALLOCATION

            releaseMemory(iter->second.poolAllocation);
            iter = mFreeUniformAllocations.erase(iter);
        } else {
            ++iter;
        }
    }

    FVK_SYSTRACE_END();
}

void VulkanMemoryPool::terminate() noexcept {
    for (auto& entry: mFreeUniformAllocations) {
        releaseMemory(entry.second.poolAllocation);
    }
    mFreeUniformAllocations.clear();
}

VulkanMemoryPoolAllocation const* VulkanMemoryPool::allocateMemory(VulkanBufferUsage usage,
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

    VulkanMemoryPoolAllocation* poolAllocation = new VulkanMemoryPoolAllocation{
        .numBytes = numBytes,
        .usage = usage,
    };
    VmaAllocationCreateInfo const allocInfo{
        .flags = vmaFlags,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };
    UTILS_UNUSED_IN_RELEASE VkResult result =
            vmaCreateBuffer(mAllocator, &bufferInfo, &allocInfo, &poolAllocation->vkbuffer,
                    &poolAllocation->vmaAllocation, &poolAllocation->allocationInfo);

#if FVK_ENABLED(FVK_DEBUG_MEMORY_POOL_ALLOCATION)
    if (result != VK_SUCCESS) {
        FVK_LOGE << "Memory pool failed to allocate a new vkBuffer, error: " << result
                 << utils::io::endl;
    } else {
        FVK_LOGD << "Memory pool allocated a vkBuffer " << poolAllocation->vkbuffer << " of size "
                 << numBytes << "and usage = " << (int) usage << "  successfully"
                 << utils::io::endl;
    }
#endif // FVK_DEBUG_MEMORY_POOL_ALLOCATION

    return poolAllocation;
}

void VulkanMemoryPool::releaseMemory(VulkanMemoryPoolAllocation const* poolAllocation) noexcept {
    vmaDestroyBuffer(mAllocator, poolAllocation->vkbuffer, poolAllocation->vmaAllocation);
    delete poolAllocation;
    poolAllocation = nullptr;
}

} // namespace filament::backend
