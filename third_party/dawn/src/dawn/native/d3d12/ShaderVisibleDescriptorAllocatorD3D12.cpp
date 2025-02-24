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

#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/GPUDescriptorHeapAllocationD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"

namespace dawn::native::d3d12 {

// Limits the min/max heap size to always be some known value for testing.
// Thresholds should be adjusted (lower == faster) to avoid tests taking too long to complete.
// We change the value from {1024, 512} to {32, 16} because we use blending
// for D3D12DescriptorHeapTests.EncodeManyUBO and R16Float has limited range
// and low precision at big integer.
static constexpr const uint32_t kShaderVisibleSmallHeapSizes[] = {32, 16};

uint32_t GetD3D12ShaderVisibleHeapMinSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool useSmallSize) {
    if (useSmallSize) {
        return kShaderVisibleSmallHeapSizes[heapType];
    }

    // Minimum heap size must be large enough to satisfy the largest descriptor allocation
    // request and to amortize the cost of sub-allocation. But small enough to avoid wasting
    // memory should only a tiny fraction ever be used.
    // TODO(dawn:155): Figure out these values.
    switch (heapType) {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return 4096;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return 256;
        default:
            DAWN_UNREACHABLE();
    }
}

uint32_t GetD3D12ShaderVisibleHeapMaxSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool useSmallSize) {
    if (useSmallSize) {
        return kShaderVisibleSmallHeapSizes[heapType];
    }

    switch (heapType) {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
        default:
            DAWN_UNREACHABLE();
    }
}

D3D12_DESCRIPTOR_HEAP_FLAGS GetD3D12HeapFlags(D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
    switch (heapType) {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        default:
            DAWN_UNREACHABLE();
    }
}

// static
ResultOrError<std::unique_ptr<MutexProtected<ShaderVisibleDescriptorAllocator>>>
ShaderVisibleDescriptorAllocator::Create(Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
    std::unique_ptr<MutexProtected<ShaderVisibleDescriptorAllocator>> allocator =
        std::make_unique<MutexProtected<ShaderVisibleDescriptorAllocator>>(device, heapType);
    DAWN_TRY((*allocator)->AllocateAndSwitchShaderVisibleHeap());
    return std::move(allocator);
}

ShaderVisibleDescriptorAllocator::ShaderVisibleDescriptorAllocator(
    Device* device,
    D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : mHeapType(heapType),
      mDevice(device),
      mSizeIncrement(device->GetD3D12Device()->GetDescriptorHandleIncrementSize(heapType)),
      mDescriptorCount(GetD3D12ShaderVisibleHeapMinSize(
          heapType,
          mDevice->IsToggleEnabled(Toggle::UseD3D12SmallShaderVisibleHeapForTesting))) {
    DAWN_ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
                heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

bool ShaderVisibleDescriptorAllocator::AllocateGPUDescriptors(
    uint32_t descriptorCount,
    ExecutionSerial pendingSerial,
    D3D12_CPU_DESCRIPTOR_HANDLE* baseCPUDescriptor,
    GPUDescriptorHeapAllocation* allocation) {
    DAWN_ASSERT(mHeap != nullptr);
    const uint64_t startOffset = mAllocator.Allocate(descriptorCount, pendingSerial);
    if (startOffset == RingBufferAllocator::kInvalidOffset) {
        return false;
    }

    ID3D12DescriptorHeap* descriptorHeap = mHeap->GetD3D12DescriptorHeap();

    const uint64_t heapOffset = mSizeIncrement * startOffset;

    // Check for 32-bit overflow since CPU heap start handle uses size_t.
    const size_t cpuHeapStartPtr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;

    DAWN_ASSERT(heapOffset <= std::numeric_limits<size_t>::max() - cpuHeapStartPtr);

    *baseCPUDescriptor = {cpuHeapStartPtr + static_cast<size_t>(heapOffset)};

    const D3D12_GPU_DESCRIPTOR_HANDLE baseGPUDescriptor = {
        descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + heapOffset};

    // Record both the device and heap serials to determine later if the allocations are
    // still valid.
    *allocation = GPUDescriptorHeapAllocation{baseGPUDescriptor, pendingSerial, mHeapSerial};

    return true;
}

ID3D12DescriptorHeap* ShaderVisibleDescriptorAllocator::GetShaderVisibleHeap() const {
    return mHeap->GetD3D12DescriptorHeap();
}

void ShaderVisibleDescriptorAllocator::Tick(ExecutionSerial completedSerial) {
    mAllocator.Deallocate(completedSerial);
}

ResultOrError<std::unique_ptr<ShaderVisibleDescriptorHeap>>
ShaderVisibleDescriptorAllocator::AllocateHeap(uint32_t descriptorCount) const {
    // The size in bytes of a descriptor heap is best calculated by the increment size
    // multiplied by the number of descriptors. In practice, this is only an estimate and
    // the actual size may vary depending on the driver.
    const uint64_t kSize = mSizeIncrement * descriptorCount;

    DAWN_TRY(mDevice->GetResidencyManager()->EnsureCanAllocate(kSize, MemorySegment::Local));

    ComPtr<ID3D12DescriptorHeap> d3d12DescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC heapDescriptor;
    heapDescriptor.Type = mHeapType;
    heapDescriptor.NumDescriptors = descriptorCount;
    heapDescriptor.Flags = GetD3D12HeapFlags(mHeapType);
    heapDescriptor.NodeMask = 0;
    DAWN_TRY(CheckOutOfMemoryHRESULT(mDevice->GetD3D12Device()->CreateDescriptorHeap(
                                         &heapDescriptor, IID_PPV_ARGS(&d3d12DescriptorHeap)),
                                     "ID3D12Device::CreateDescriptorHeap"));

    std::unique_ptr<ShaderVisibleDescriptorHeap> descriptorHeap =
        std::make_unique<ShaderVisibleDescriptorHeap>(std::move(d3d12DescriptorHeap), kSize);

    // We must track the allocation in the LRU when it is created, otherwise the residency
    // manager will see the allocation as non-resident in the later call to LockAllocation.
    mDevice->GetResidencyManager()->TrackResidentAllocation(descriptorHeap.get());

    return std::move(descriptorHeap);
}

// Creates a GPU descriptor heap that manages descriptors in a FIFO queue.
MaybeError ShaderVisibleDescriptorAllocator::AllocateAndSwitchShaderVisibleHeap() {
    std::unique_ptr<ShaderVisibleDescriptorHeap> descriptorHeap;
    // Dynamically allocate using a two-phase allocation strategy.
    // The first phase increasingly grows a small heap in binary sizes for light users while the
    // second phase pool-allocates largest sized heaps for heavy users.
    if (mHeap != nullptr) {
        mDevice->GetResidencyManager()->UnlockAllocation(mHeap.get());

        const uint32_t maxDescriptorCount = GetD3D12ShaderVisibleHeapMaxSize(
            mHeapType, mDevice->IsToggleEnabled(Toggle::UseD3D12SmallShaderVisibleHeapForTesting));
        if (mDescriptorCount < maxDescriptorCount) {
            // Phase #1. Grow the heaps in powers-of-two.
            mDevice->ReferenceUntilUnused(mHeap->GetD3D12DescriptorHeap());
            mDescriptorCount = std::min(mDescriptorCount * 2, maxDescriptorCount);
        } else {
            // Phase #2. Pool-allocate heaps.
            // Return the switched out heap to the pool and retrieve the oldest heap that is no
            // longer used by GPU. This maintains a heap buffer to avoid frequently re-creating
            // heaps for heavy users.
            // TODO(dawn:256): Consider periodically triming to avoid OOM.
            mPool.push_back({mDevice->GetQueue()->GetPendingCommandSerial(), std::move(mHeap)});
            if (mPool.front().heapSerial <= mDevice->GetQueue()->GetCompletedCommandSerial()) {
                descriptorHeap = std::move(mPool.front().heap);
                mPool.pop_front();
            }
        }
    }

    if (descriptorHeap == nullptr) {
        DAWN_TRY_ASSIGN(descriptorHeap, AllocateHeap(mDescriptorCount));
    }

    DAWN_TRY(mDevice->GetResidencyManager()->LockAllocation(descriptorHeap.get()));

    // Create a FIFO buffer from the recently created heap.
    mHeap = std::move(descriptorHeap);
    mAllocator = RingBufferAllocator(mDescriptorCount);

    // Invalidate all bindgroup allocations on previously bound heaps by incrementing the heap
    // serial. When a bindgroup attempts to re-populate, it will compare with its recorded
    // heap serial.
    mHeapSerial++;

    return {};
}

Device* ShaderVisibleDescriptorAllocator::GetDevice() const {
    return mDevice;
}

HeapVersionID ShaderVisibleDescriptorAllocator::GetShaderVisibleHeapSerialForTesting() const {
    return mHeapSerial;
}

uint64_t ShaderVisibleDescriptorAllocator::GetShaderVisibleHeapSizeForTesting() const {
    return mAllocator.GetSize();
}

uint64_t ShaderVisibleDescriptorAllocator::GetShaderVisiblePoolSizeForTesting() const {
    return mPool.size();
}

bool ShaderVisibleDescriptorAllocator::IsShaderVisibleHeapLockedResidentForTesting() const {
    return mHeap->IsResidencyLocked();
}

bool ShaderVisibleDescriptorAllocator::IsLastShaderVisibleHeapInLRUForTesting() const {
    DAWN_ASSERT(!mPool.empty());
    return mPool.back().heap->IsInResidencyLRUCache();
}

bool ShaderVisibleDescriptorAllocator::IsAllocationStillValid(
    const GPUDescriptorHeapAllocation& allocation) const {
    // Descriptor allocations are only valid for the serial they were created for and are
    // re-allocated every submit. For this reason, we view any descriptors allocated prior to the
    // pending submit as invalid. We must also verify the descriptor heap has not switched (because
    // a larger descriptor heap was needed).
    return (allocation.GetLastUsageSerial() == mDevice->GetQueue()->GetPendingCommandSerial() &&
            allocation.GetHeapSerial() == mHeapSerial);
}

ShaderVisibleDescriptorHeap::ShaderVisibleDescriptorHeap(
    ComPtr<ID3D12DescriptorHeap> d3d12DescriptorHeap,
    uint64_t size)
    : Pageable(d3d12DescriptorHeap, MemorySegment::Local, size),
      mD3d12DescriptorHeap(std::move(d3d12DescriptorHeap)) {}

ID3D12DescriptorHeap* ShaderVisibleDescriptorHeap::GetD3D12DescriptorHeap() const {
    return mD3d12DescriptorHeap.Get();
}
}  // namespace dawn::native::d3d12
