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

// Various kinds of memory that influence the result of the allocation. For example, to take
// into account mappability and Vulkan's bufferImageGranularity.
enum class MemoryKind {
    LazilyAllocated,
    Linear,
    LinearReadMappable,
    LinearWriteMappable,
    Opaque,
};

class ResourceMemoryAllocator {
  public:
    explicit ResourceMemoryAllocator(Device* device);
    ~ResourceMemoryAllocator();

    ResultOrError<ResourceMemoryAllocation> Allocate(const VkMemoryRequirements& requirements,
                                                     MemoryKind kind,
                                                     bool forceDisableSubAllocation = false);
    void Deallocate(ResourceMemoryAllocation* allocation);

    void DestroyPool();

    void Tick(ExecutionSerial completedSerial);

    int FindBestTypeIndex(VkMemoryRequirements requirements, MemoryKind kind);

  private:
    raw_ptr<Device> mDevice;

    class SingleTypeAllocator;
    std::vector<std::unique_ptr<SingleTypeAllocator>> mAllocatorsPerType;

    SerialQueue<ExecutionSerial, ResourceMemoryAllocation> mSubAllocationsToDelete;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RESOURCEMEMORYALLOCATORVK_H_
