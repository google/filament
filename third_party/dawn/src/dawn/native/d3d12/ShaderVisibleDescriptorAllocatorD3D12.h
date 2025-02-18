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

#ifndef SRC_DAWN_NATIVE_D3D12_SHADERVISIBLEDESCRIPTORALLOCATORD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_SHADERVISIBLEDESCRIPTORALLOCATORD3D12_H_

#include <list>
#include <memory>

#include "dawn/common/MutexProtected.h"
#include "dawn/native/Error.h"
#include "dawn/native/RingBufferAllocator.h"
#include "dawn/native/d3d12/IntegerTypes.h"
#include "dawn/native/d3d12/PageableD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

// |ShaderVisibleDescriptorAllocator| allocates a variable-sized block of descriptors from a GPU
// descriptor heap pool.
// Internally, it manages a list of heaps using a ringbuffer block allocator. The heap is in one
// of two states: switched in or out. Only a switched in heap can be bound to the pipeline. If
// the heap is full, the caller must switch-in a new heap before re-allocating and the old one
// is returned to the pool.
namespace dawn::native::d3d12 {

class Device;
class GPUDescriptorHeapAllocation;

class ShaderVisibleDescriptorHeap : public Pageable {
  public:
    ShaderVisibleDescriptorHeap(ComPtr<ID3D12DescriptorHeap> d3d12DescriptorHeap, uint64_t size);
    ID3D12DescriptorHeap* GetD3D12DescriptorHeap() const;

  private:
    ComPtr<ID3D12DescriptorHeap> mD3d12DescriptorHeap;
};

class ShaderVisibleDescriptorAllocator {
  public:
    static ResultOrError<std::unique_ptr<MutexProtected<ShaderVisibleDescriptorAllocator>>> Create(
        Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE heapType);

    ShaderVisibleDescriptorAllocator(Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType);

    // Returns true if the allocation was successful, when false is returned the current heap is
    // full and AllocateAndSwitchShaderVisibleHeap() must be called.
    bool AllocateGPUDescriptors(uint32_t descriptorCount,
                                ExecutionSerial pendingSerial,
                                D3D12_CPU_DESCRIPTOR_HANDLE* baseCPUDescriptor,
                                GPUDescriptorHeapAllocation* allocation);

    void Tick(ExecutionSerial completedSerial);

    ID3D12DescriptorHeap* GetShaderVisibleHeap() const;
    MaybeError AllocateAndSwitchShaderVisibleHeap();

    // For testing purposes only.
    HeapVersionID GetShaderVisibleHeapSerialForTesting() const;
    uint64_t GetShaderVisibleHeapSizeForTesting() const;
    uint64_t GetShaderVisiblePoolSizeForTesting() const;
    bool IsShaderVisibleHeapLockedResidentForTesting() const;
    bool IsLastShaderVisibleHeapInLRUForTesting() const;

    bool IsAllocationStillValid(const GPUDescriptorHeapAllocation& allocation) const;

    Device* GetDevice() const;

  private:
    struct SerialDescriptorHeap {
        ExecutionSerial heapSerial;
        std::unique_ptr<ShaderVisibleDescriptorHeap> heap;
    };

    ResultOrError<std::unique_ptr<ShaderVisibleDescriptorHeap>> AllocateHeap(
        uint32_t descriptorCount) const;

    std::unique_ptr<ShaderVisibleDescriptorHeap> mHeap;
    RingBufferAllocator mAllocator;
    std::list<SerialDescriptorHeap> mPool;
    D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;

    raw_ptr<Device> mDevice;

    // The serial value of 0 means the shader-visible heaps have not been allocated.
    // This value is never returned in the GPUDescriptorHeapAllocation after
    // AllocateGPUDescriptors() is called.
    HeapVersionID mHeapSerial = HeapVersionID(0);

    uint32_t mSizeIncrement;

    // The descriptor count is the current size of the heap in number of descriptors.
    // This is stored on the allocator to avoid extra conversions.
    uint32_t mDescriptorCount = 0;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_SHADERVISIBLEDESCRIPTORALLOCATORD3D12_H_
