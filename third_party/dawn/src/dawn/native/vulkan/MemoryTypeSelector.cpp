// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/MemoryTypeSelector.h"

#include <utility>

namespace dawn::native::vulkan {
namespace {

// On Vulkan the memory type of the mappable buffers with extended usages must have all below memory
// property flags.
constexpr VkMemoryPropertyFlags kMapExtendedUsageMemoryPropertyFlags =
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

bool HasMemoryTypeWithFlags(const std::vector<VkMemoryType>& memoryTypes,
                            VkMemoryPropertyFlags flags) {
    for (auto& memoryType : memoryTypes) {
        if ((memoryType.propertyFlags & flags) == flags) {
            return true;
        }
    }
    return false;
}

}  // namespace

bool IsMemoryKindMappable(MemoryKind memoryKind) {
    return memoryKind & (MemoryKind::ReadMappable | MemoryKind::WriteMappable);
}

bool SupportsBufferMapExtendedUsages(const VulkanDeviceInfo& deviceInfo) {
    // TODO(crbug.com/422798184): Only enable BufferMapExtendedUsages if heap size for device local
    // + host visible is greater than 256MB aka it's not Resizable BAR.
    return HasMemoryTypeWithFlags(deviceInfo.memoryTypes, kMapExtendedUsageMemoryPropertyFlags);
}

MemoryTypeSelector::MemoryTypeSelector(const VulkanDeviceInfo& info)
    : MemoryTypeSelector(info.memoryTypes, info.memoryHeaps) {}

MemoryTypeSelector::MemoryTypeSelector(std::vector<VkMemoryType> memoryTypes,
                                       std::vector<VkMemoryHeap> memoryHeaps)
    : mMemoryTypes(std::move(memoryTypes)), mMemoryHeaps(std::move(memoryHeaps)) {}

int MemoryTypeSelector::FindBestTypeIndex(VkMemoryRequirements requirements, MemoryKind kind) {
    bool mappable = IsMemoryKindMappable(kind);
    VkMemoryPropertyFlags vkRequiredFlags = GetRequiredMemoryPropertyFlags(kind);

    // Find a suitable memory type for this allocation
    int bestType = -1;
    for (size_t i = 0; i < mMemoryTypes.size(); ++i) {
        // Resource must support this memory type
        if ((requirements.memoryTypeBits & (1 << i)) == 0) {
            continue;
        }

        // Memory type must have all the required memory properties.
        if ((mMemoryTypes[i].propertyFlags & vkRequiredFlags) != vkRequiredFlags) {
            continue;
        }

        // Found the first candidate memory type
        if (bestType == -1) {
            bestType = static_cast<int>(i);
            continue;
        }

        // For non-mappable resources that can be lazily allocated, favor lazy
        // allocation (note: this is a more important property than that of
        // device local memory and hence is checked first).
        bool currentLazilyAllocated =
            (mMemoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0u;
        bool bestLazilyAllocated =
            (mMemoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0u;
        if ((kind == MemoryKind::LazilyAllocated) &&
            (currentLazilyAllocated != bestLazilyAllocated)) {
            if (currentLazilyAllocated) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // For non-mappable, non-lazily-allocated resources, favor device local
        // memory.
        bool currentDeviceLocal =
            (mMemoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0u;
        bool bestDeviceLocal =
            (mMemoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0u;
        if (!mappable && (currentDeviceLocal != bestDeviceLocal)) {
            if (currentDeviceLocal) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // Cached memory is optimal for read access from CPU as host memory reads to uncached memory
        // are slower than to cached memory.
        bool currentHostCached =
            (mMemoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0u;
        bool bestHostCached =
            (mMemoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0u;
        if ((kind & MemoryKind::ReadMappable) && currentHostCached != bestHostCached) {
            if (currentHostCached) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // Coherent memory is optimal for write access from CPU as host memory writes to uncoherent
        // memory must be explicitly flushed. This is less important than HOST_CACHED above.
        bool currentHostCoherent =
            (mMemoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0u;
        bool bestHostCoherent =
            (mMemoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0u;
        if ((kind & MemoryKind::WriteMappable) && currentHostCoherent != bestHostCoherent) {
            if (currentHostCoherent) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // All things equal favor the memory in the biggest heap
        VkDeviceSize bestTypeHeapSize = mMemoryHeaps[mMemoryTypes[bestType].heapIndex].size;
        VkDeviceSize candidateHeapSize = mMemoryHeaps[mMemoryTypes[i].heapIndex].size;
        if (candidateHeapSize > bestTypeHeapSize) {
            bestType = static_cast<int>(i);
            continue;
        }
    }

    return bestType;
}

VkMemoryPropertyFlags MemoryTypeSelector::GetRequiredMemoryPropertyFlags(
    MemoryKind memoryKind) const {
    VkMemoryPropertyFlags vkFlags = 0;

    // HOST_VISIBLE_BIT must be set for mappable memory.
    if (IsMemoryKindMappable(memoryKind)) {
        vkFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }

    // DEVICE_LOCAL_BIT must be set when MemoryKind::DeviceLocal is required.
    if (memoryKind & MemoryKind::DeviceLocal) {
        vkFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    // HOST_CACHED_BIT must be set when MemoryKind::HostCached is required.
    if (memoryKind & MemoryKind::HostCached) {
        vkFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

    return vkFlags;
}

}  // namespace dawn::native::vulkan
