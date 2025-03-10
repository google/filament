// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_STAGINGDESCRIPTORALLOCATORD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_STAGINGDESCRIPTORALLOCATORD3D12_H_

#include <vector>

#include "dawn/common/SerialQueue.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d12/CPUDescriptorHeapAllocationD3D12.h"
#include "partition_alloc/pointers/raw_ptr.h"

// |StagingDescriptorAllocator| allocates a fixed-size block of descriptors from a CPU
// descriptor heap pool.
// Internally, it manages a list of heaps using a fixed-size block allocator. The fixed-size
// block allocator is backed by a list of free blocks (free-list). The heap is in one of two
// states: AVAILABLE or not. To allocate, the next free block is removed from the free-list
// and the corresponding heap offset is returned. The AVAILABLE heap always has room for
// at-least one free block. If no AVAILABLE heap exists, a new heap is created and inserted
// back into the pool to be immediately used. To deallocate, the block corresponding to the
// offset is inserted back into the free-list.
namespace dawn::native::d3d12 {

class Device;

class StagingDescriptorAllocator {
  public:
    StagingDescriptorAllocator() = default;
    StagingDescriptorAllocator(Device* device,
                               uint32_t descriptorCount,
                               uint32_t heapSize,
                               D3D12_DESCRIPTOR_HEAP_TYPE heapType);
    ~StagingDescriptorAllocator();

    ResultOrError<CPUDescriptorHeapAllocation> AllocateCPUDescriptors();

    // Will call Deallocate when the serial is passed.
    ResultOrError<CPUDescriptorHeapAllocation> AllocateTransientCPUDescriptors();

    void Deallocate(CPUDescriptorHeapAllocation* allocation);

    uint32_t GetSizeIncrement() const;

    void Tick(ExecutionSerial completedSerial);

  private:
    using Index = uint16_t;

    struct NonShaderVisibleBuffer {
        ComPtr<ID3D12DescriptorHeap> heap;
        std::vector<Index> freeBlockIndices;
    };

    MaybeError AllocateCPUHeap();

    Index GetFreeBlockIndicesSize() const;

    std::vector<uint32_t> mAvailableHeaps;  // Indices into the pool.
    std::vector<NonShaderVisibleBuffer> mPool;

    raw_ptr<const Device> mDevice = nullptr;

    const uint32_t mSizeIncrement = 0;  // Size of the descriptor (in bytes).
    const uint32_t mBlockSize = 0;      // Size of the block of descriptors (in bytes).
    const uint32_t mHeapSize = 0;       // Size of the heap (in number of descriptors).

    D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;

    SerialQueue<ExecutionSerial, CPUDescriptorHeapAllocation> mAllocationsToDelete;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_STAGINGDESCRIPTORALLOCATORD3D12_H_
