// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_RESOURCEMEMORYALLOCATORVK_H_
#define SRC_DAWN_NATIVE_VULKAN_RESOURCEMEMORYALLOCATORVK_H_

#include <map>
#include <memory>
#include <vector>

#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PooledResourceMemoryAllocator.h"
#include "dawn/native/ResourceMemoryAllocation.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

class Device;
class ResourceHeap;
struct VulkanDeviceInfo;

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

bool SupportsBufferMapExtendedUsages(const VulkanDeviceInfo& deviceInfo);

class ResourceMemoryAllocator {
  public:
    explicit ResourceMemoryAllocator(Device* device);
    ~ResourceMemoryAllocator();

    ResultOrError<ResourceMemoryAllocation> Allocate(const VkMemoryRequirements& requirements,
                                                     MemoryKind kind,
                                                     bool forceDisableSubAllocation = false);
    void Deallocate(ResourceMemoryAllocation* allocation);

    void DestroyPool();

    // Returns the last serial that an object is pending deletion after or
    // kBeginningOfGPUTime if no objects are pending deletion.
    ExecutionSerial GetLastPendingDeletionSerial();

    void Tick(ExecutionSerial completedSerial);

    int FindBestTypeIndex(VkMemoryRequirements requirements, MemoryKind kind);

    uint64_t GetTotalUsedMemory() const;
    uint64_t GetTotalAllocatedMemory() const;

  protected:
    void RecordHeapAllocation(VkDeviceSize size);
    void DeallocateResourceHeap(ResourceHeap* heap);

  private:
    raw_ptr<Device> mDevice;

    class SingleTypeAllocator;
    std::vector<std::unique_ptr<SingleTypeAllocator>> mAllocatorsPerType;

    SerialQueue<ExecutionSerial, ResourceMemoryAllocation> mSubAllocationsToDelete;
    std::map<ExecutionSerial, VkDeviceSize> mUsedMemoryToDecrement;
    std::map<ExecutionSerial, VkDeviceSize> mAllocatedMemoryToDecrement;

    VkDeviceSize mTotalAllocatedMemory = 0;
    VkDeviceSize mTotalUsedMemory = 0;
};

}  // namespace dawn::native::vulkan

namespace wgpu {
template <>
struct IsWGPUBitmask<dawn::native::vulkan::MemoryKind> {
    static constexpr bool enable = true;
};

}  // namespace wgpu

#endif  // SRC_DAWN_NATIVE_VULKAN_RESOURCEMEMORYALLOCATORVK_H_
