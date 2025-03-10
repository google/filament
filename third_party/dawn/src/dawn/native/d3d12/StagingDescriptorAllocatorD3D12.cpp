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

#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"

#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/Queue.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"

namespace dawn::native::d3d12 {

StagingDescriptorAllocator::StagingDescriptorAllocator(Device* device,
                                                       uint32_t descriptorCount,
                                                       uint32_t heapSize,
                                                       D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : mDevice(device),
      mSizeIncrement(device->GetD3D12Device()->GetDescriptorHandleIncrementSize(heapType)),
      mBlockSize(descriptorCount * mSizeIncrement),
      mHeapSize(RoundUp(heapSize, descriptorCount)),
      mHeapType(heapType) {
    DAWN_ASSERT(descriptorCount <= heapSize);
}

StagingDescriptorAllocator::~StagingDescriptorAllocator() {
    const Index freeBlockIndicesSize = GetFreeBlockIndicesSize();
    for (auto& buffer : mPool) {
        DAWN_ASSERT(buffer.freeBlockIndices.size() == freeBlockIndicesSize);
    }
    DAWN_ASSERT(mAvailableHeaps.size() == mPool.size());
}

ResultOrError<CPUDescriptorHeapAllocation> StagingDescriptorAllocator::AllocateCPUDescriptors() {
    if (mAvailableHeaps.empty()) {
        DAWN_TRY(AllocateCPUHeap());
    }

    DAWN_ASSERT(!mAvailableHeaps.empty());

    const uint32_t heapIndex = mAvailableHeaps.back();
    NonShaderVisibleBuffer& buffer = mPool[heapIndex];

    DAWN_ASSERT(!buffer.freeBlockIndices.empty());

    const Index blockIndex = buffer.freeBlockIndices.back();

    buffer.freeBlockIndices.pop_back();

    if (buffer.freeBlockIndices.empty()) {
        mAvailableHeaps.pop_back();
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor = {
        buffer.heap->GetCPUDescriptorHandleForHeapStart().ptr + (blockIndex * mBlockSize)};

    return CPUDescriptorHeapAllocation{baseCPUDescriptor, heapIndex};
}

MaybeError StagingDescriptorAllocator::AllocateCPUHeap() {
    D3D12_DESCRIPTOR_HEAP_DESC heapDescriptor;
    heapDescriptor.Type = mHeapType;
    heapDescriptor.NumDescriptors = mHeapSize;
    heapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDescriptor.NodeMask = 0;

    ComPtr<ID3D12DescriptorHeap> heap;
    DAWN_TRY(CheckHRESULT(
        mDevice->GetD3D12Device()->CreateDescriptorHeap(&heapDescriptor, IID_PPV_ARGS(&heap)),
        "ID3D12Device::CreateDescriptorHeap"));

    NonShaderVisibleBuffer newBuffer;
    newBuffer.heap = std::move(heap);

    const Index freeBlockIndicesSize = GetFreeBlockIndicesSize();
    newBuffer.freeBlockIndices.reserve(freeBlockIndicesSize);

    for (Index blockIndex = 0; blockIndex < freeBlockIndicesSize; blockIndex++) {
        newBuffer.freeBlockIndices.push_back(blockIndex);
    }

    mAvailableHeaps.push_back(mPool.size());
    mPool.emplace_back(std::move(newBuffer));

    return {};
}

void StagingDescriptorAllocator::Deallocate(CPUDescriptorHeapAllocation* allocation) {
    DAWN_ASSERT(allocation->IsValid());

    const uint32_t heapIndex = allocation->GetHeapIndex();

    DAWN_ASSERT(heapIndex < mPool.size());

    // Insert the deallocated block back into the free-list. Order does not matter. However,
    // having blocks be non-contigious could slow down future allocations due to poor cache
    // locality.
    // TODO(dawn:155): Consider more optimization.
    std::vector<Index>& freeBlockIndices = mPool[heapIndex].freeBlockIndices;
    if (freeBlockIndices.empty()) {
        mAvailableHeaps.emplace_back(heapIndex);
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE heapStart =
        mPool[heapIndex].heap->GetCPUDescriptorHandleForHeapStart();

    const D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor = allocation->OffsetFrom(0, 0);

    const Index blockIndex = (baseDescriptor.ptr - heapStart.ptr) / mBlockSize;

    freeBlockIndices.emplace_back(blockIndex);

    // Invalidate the handle in case the developer accidentally uses it again.
    allocation->Invalidate();
}

uint32_t StagingDescriptorAllocator::GetSizeIncrement() const {
    return mSizeIncrement;
}

StagingDescriptorAllocator::Index StagingDescriptorAllocator::GetFreeBlockIndicesSize() const {
    return ((mHeapSize * mSizeIncrement) / mBlockSize);
}

ResultOrError<CPUDescriptorHeapAllocation>
StagingDescriptorAllocator::AllocateTransientCPUDescriptors() {
    CPUDescriptorHeapAllocation allocation;
    DAWN_TRY_ASSIGN(allocation, AllocateCPUDescriptors());
    mAllocationsToDelete.Enqueue(allocation, mDevice->GetQueue()->GetPendingCommandSerial());
    return allocation;
}

void StagingDescriptorAllocator::Tick(ExecutionSerial completedSerial) {
    for (CPUDescriptorHeapAllocation& allocation :
         mAllocationsToDelete.IterateUpTo(completedSerial)) {
        Deallocate(&allocation);
    }

    mAllocationsToDelete.ClearUpTo(completedSerial);
}

}  // namespace dawn::native::d3d12
