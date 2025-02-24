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

#include "dawn/native/d3d12/HeapAllocatorD3D12.h"

#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/HeapD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

namespace dawn::native::d3d12 {

HeapAllocator::HeapAllocator(Device* device,
                             ResourceHeapKind resourceHeapKind,
                             D3D12_HEAP_FLAGS heapFlags,
                             MemorySegment memorySegment)
    : mDevice(device),
      mResourceHeapKind(resourceHeapKind),
      mHeapFlags(heapFlags),
      mMemorySegment(memorySegment) {}

ResultOrError<std::unique_ptr<ResourceHeapBase>> HeapAllocator::AllocateResourceHeap(
    uint64_t size) {
    D3D12_HEAP_DESC heapDesc;
    heapDesc.SizeInBytes = size;
    heapDesc.Properties = GetD3D12HeapProperties(mResourceHeapKind);
    heapDesc.Alignment = mDevice->IsToggleEnabled(Toggle::D3D12Use64KBAlignedMSAATexture)
                             ? D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT
                             : D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Flags = mHeapFlags;

    // CreateHeap will implicitly make the created heap resident. We must ensure enough free
    // memory exists before allocating to avoid an out-of-memory error when overcommitted.
    DAWN_TRY(mDevice->GetResidencyManager()->EnsureCanAllocate(size, mMemorySegment));

    ComPtr<ID3D12Heap> d3d12Heap;
    DAWN_TRY(CheckOutOfMemoryHRESULT(
        mDevice->GetD3D12Device()->CreateHeap(&heapDesc, IID_PPV_ARGS(&d3d12Heap)),
        "ID3D12Device::CreateHeap"));

    std::unique_ptr<ResourceHeapBase> heapBase =
        std::make_unique<Heap>(std::move(d3d12Heap), mMemorySegment, size);

    // Calling CreateHeap implicitly calls MakeResident on the new heap. We must track this to
    // avoid calling MakeResident a second time.
    mDevice->GetResidencyManager()->TrackResidentAllocation(ToBackend(heapBase.get()));
    return std::move(heapBase);
}

void HeapAllocator::DeallocateResourceHeap(std::unique_ptr<ResourceHeapBase> heap) {
    mDevice->ReferenceUntilUnused(static_cast<Heap*>(heap.get())->GetD3D12Heap());
}

}  // namespace dawn::native::d3d12
