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

#ifndef SRC_DAWN_NATIVE_VULKAN_MEMORYTYPESELECTOR_H_
#define SRC_DAWN_NATIVE_VULKAN_MEMORYTYPESELECTOR_H_

#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/EnumClassBitmasks.h"
#include "dawn/native/vulkan/VulkanInfo.h"

namespace dawn::native::vulkan {

// Each bit of MemoryKind represents a kind of memory that influence the result of the allocation.
// For example, to take into account mappability and Vulkan's bufferImageGranularity.
enum class MemoryKind : uint8_t {
    LazilyAllocated = 1,
    Linear = 2,
    DeviceLocal = 4,
    ReadMappable = 8,
    WriteMappable = 16,
    HostCached = 32,
};

bool IsMemoryKindMappable(MemoryKind memoryKind);

bool SupportsBufferMapExtendedUsages(const VulkanDeviceInfo& deviceInfo);

// Encapsulates logic to select best memory type based on VkMemoryRequirements plus MemoryKind.
class MemoryTypeSelector {
  public:
    explicit MemoryTypeSelector(const VulkanDeviceInfo& info);
    MemoryTypeSelector(std::vector<VkMemoryType> memoryTypes,
                       std::vector<VkMemoryHeap> memoryHeaps);

    int FindBestTypeIndex(VkMemoryRequirements requirements, MemoryKind kind);

  private:
    VkMemoryPropertyFlags GetRequiredMemoryPropertyFlags(MemoryKind memoryKind) const;

    const std::vector<VkMemoryType> mMemoryTypes;
    const std::vector<VkMemoryHeap> mMemoryHeaps;
};

}  // namespace dawn::native::vulkan

namespace wgpu {
template <>
struct IsWGPUBitmask<dawn::native::vulkan::MemoryKind> {
    static constexpr bool enable = true;
};

}  // namespace wgpu

#endif  // SRC_DAWN_NATIVE_VULKAN_MEMORYTYPESELECTOR_H_
