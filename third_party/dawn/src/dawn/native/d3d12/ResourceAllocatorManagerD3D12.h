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

#ifndef SRC_DAWN_NATIVE_D3D12_RESOURCEALLOCATORMANAGERD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_RESOURCEALLOCATORMANAGERD3D12_H_

#include <array>
#include <memory>

#include "dawn/common/SerialQueue.h"
#include "dawn/native/BuddyMemoryAllocator.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PooledResourceMemoryAllocator.h"
#include "dawn/native/d3d12/d3d12_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::d3d12 {

class Device;
class HeapAllocator;
class ResourceHeapAllocation;

// Resource heap types + flags combinations are named after the D3D constants.
// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_heap_flags
// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_heap_type
enum ResourceHeapKind {
    // Resource heap tier 2
    // Allows resource heaps to contain all buffer and textures types.
    // This enables better heap reuse by avoiding the need for separate heaps and
    // also reduces fragmentation.
    Readback_AllBuffersAndTextures,
    Upload_AllBuffersAndTextures,
    Default_AllBuffersAndTextures,

    // Resource heap tier 1
    // Resource heaps only support types from a single resource category.
    Readback_OnlyBuffers,
    Upload_OnlyBuffers,
    Default_OnlyBuffers,

    Default_OnlyNonRenderableOrDepthTextures,
    Default_OnlyRenderableOrDepthTextures,

    // Allows custom resource heaps to contain the buffers that support write-back CPU property.
    Custom_WriteBack_OnlyBuffers,

    EnumCount,
    InvalidEnum = EnumCount,
};

// Manages a list of resource allocators used by the device to create resources using
// multiple allocation methods.
class ResourceAllocatorManager {
  public:
    explicit ResourceAllocatorManager(Device* device);
    ~ResourceAllocatorManager();

    ResultOrError<ResourceHeapAllocation> AllocateMemory(
        ResourceHeapKind resourceHeapKind,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage,
        uint32_t colorFormatBytesPerBlock,
        bool forceAllocateAsCommittedResource = false);

    void DeallocateMemory(ResourceHeapAllocation& allocation);

    void Tick(ExecutionSerial lastCompletedSerial);

  private:
    void FreeSubAllocatedMemory(ResourceHeapAllocation& allocation);

    ResultOrError<ResourceHeapAllocation> CreatePlacedResource(
        ResourceHeapKind resourceHeapKind,
        const D3D12_RESOURCE_DESC& requestedResourceDescriptor,
        const D3D12_CLEAR_VALUE* optimizedClearValue,
        D3D12_RESOURCE_STATES initialUsage);

    ResultOrError<ResourceHeapAllocation> CreateCommittedResource(
        ResourceHeapKind resourceHeapKind,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        const D3D12_CLEAR_VALUE* optimizedClearValue,
        D3D12_RESOURCE_STATES initialUsage);

    void DestroyPool();

    raw_ptr<Device> mDevice;

    static constexpr uint64_t kMaxHeapSize = 32ll * 1024ll * 1024ll * 1024ll;  // 32GB
    static constexpr uint64_t kMinHeapSize = 4ll * 1024ll * 1024ll;            // 4MB

    std::array<std::unique_ptr<BuddyMemoryAllocator>, ResourceHeapKind::EnumCount>
        mSubAllocatedResourceAllocators;
    std::array<std::unique_ptr<HeapAllocator>, ResourceHeapKind::EnumCount> mHeapAllocators;

    std::array<std::unique_ptr<PooledResourceMemoryAllocator>, ResourceHeapKind::EnumCount>
        mPooledHeapAllocators;

    SerialQueue<ExecutionSerial, ResourceHeapAllocation> mAllocationsToDelete;
    SerialQueue<ExecutionSerial, std::unique_ptr<ResourceHeapBase>> mHeapsToDelete;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_RESOURCEALLOCATORMANAGERD3D12_H_
