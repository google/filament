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

#include "dawn/native/d3d12/ResidencyManagerD3D12.h"

#include <algorithm>
#include <vector>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/Forward.h"
#include "dawn/native/d3d12/HeapD3D12.h"
#include "dawn/native/d3d12/PhysicalDeviceD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"

namespace dawn::native::d3d12 {

ResidencyManager::ResidencyManager(Device* device)
    : mDevice(device),
      mResidencyManagementEnabled(device->IsToggleEnabled(Toggle::UseD3D12ResidencyManagement)) {
    UpdateVideoMemoryInfo();
}

// Increments number of locks on a heap to ensure the heap remains resident.
MaybeError ResidencyManager::LockAllocation(Pageable* pageable) {
    if (!mResidencyManagementEnabled) {
        return {};
    }

    // If the heap isn't already resident, make it resident.
    if (!pageable->IsInResidencyLRUCache() && !pageable->IsResidencyLocked()) {
        ID3D12Pageable* d3d12Pageable = pageable->GetD3D12Pageable();
        uint64_t size = pageable->GetSize();

        DAWN_TRY(MakeAllocationsResident(GetMemorySegmentInfo(pageable->GetMemorySegment()), size,
                                         1, &d3d12Pageable));
    }

    // Since we can't evict the heap, it's unnecessary to track the heap in the LRU Cache.
    if (pageable->IsInResidencyLRUCache()) {
        pageable->RemoveFromList();
    }

    pageable->IncrementResidencyLock();

    return {};
}

// Decrements number of locks on a heap. When the number of locks becomes zero, the heap is
// inserted into the LRU cache and becomes eligible for eviction.
void ResidencyManager::UnlockAllocation(Pageable* pageable) {
    if (!mResidencyManagementEnabled) {
        return;
    }

    DAWN_ASSERT(pageable->IsResidencyLocked());
    DAWN_ASSERT(!pageable->IsInResidencyLRUCache());
    pageable->DecrementResidencyLock();

    // If another lock still exists on the heap, nothing further should be done.
    if (pageable->IsResidencyLocked()) {
        return;
    }

    // When all locks have been removed, the resource remains resident and becomes tracked in
    // the corresponding LRU.
    TrackResidentAllocation(pageable);
}

// Returns the appropriate MemorySegmentInfo for a given MemorySegment.
ResidencyManager::MemorySegmentInfo* ResidencyManager::GetMemorySegmentInfo(
    MemorySegment memorySegment) {
    switch (memorySegment) {
        case MemorySegment::Local:
            return &mVideoMemoryInfo.local;
        case MemorySegment::NonLocal:
            DAWN_ASSERT(!mDevice->GetDeviceInfo().isUMA);
            return &mVideoMemoryInfo.nonLocal;
        default:
            DAWN_UNREACHABLE();
    }
}

// Allows an application component external to Dawn to cap Dawn's residency budgets to prevent
// competition for device memory. Returns the amount of memory reserved, which may be less
// that the requested reservation when under pressure.
uint64_t ResidencyManager::SetExternalMemoryReservation(MemorySegment segment,
                                                        uint64_t requestedReservationSize) {
    MemorySegmentInfo* segmentInfo = GetMemorySegmentInfo(segment);

    segmentInfo->externalRequest = requestedReservationSize;

    UpdateMemorySegmentInfo(segmentInfo);

    return segmentInfo->externalReservation;
}

void ResidencyManager::UpdateVideoMemoryInfo() {
    UpdateMemorySegmentInfo(&mVideoMemoryInfo.local);
    if (!mDevice->GetDeviceInfo().isUMA) {
        UpdateMemorySegmentInfo(&mVideoMemoryInfo.nonLocal);
    }
}

void ResidencyManager::UpdateMemorySegmentInfo(MemorySegmentInfo* segmentInfo) {
    DXGI_QUERY_VIDEO_MEMORY_INFO queryVideoMemoryInfo;

    ToBackend(mDevice->GetPhysicalDevice())
        ->GetHardwareAdapter()
        ->QueryVideoMemoryInfo(0, segmentInfo->dxgiSegment, &queryVideoMemoryInfo);

    // The video memory budget provided by QueryVideoMemoryInfo is defined by the operating
    // system, and may be lower than expected in certain scenarios. Under memory pressure, we
    // cap the external reservation to half the available budget, which prevents the external
    // component from consuming a disproportionate share of memory and ensures that Dawn can
    // continue to make forward progress. Note the choice to halve memory is arbitrarily chosen
    // and subject to future experimentation.
    segmentInfo->externalReservation =
        std::min(queryVideoMemoryInfo.Budget / 2, segmentInfo->externalRequest);

    segmentInfo->usage = queryVideoMemoryInfo.CurrentUsage - segmentInfo->externalReservation;

    // If we're restricting the budget for testing, leave the budget as is.
    if (mRestrictBudgetForTesting) {
        return;
    }

    // We cap Dawn's budget to 95% of the provided budget. Leaving some budget unused
    // decreases fluctuations in the operating-system-defined budget, which improves stability
    // for both Dawn and other applications on the system. Note the value of 95% is arbitrarily
    // chosen and subject to future experimentation.
    static constexpr float kBudgetCap = 0.95f;
    segmentInfo->budget = static_cast<uint64_t>(
        (queryVideoMemoryInfo.Budget - segmentInfo->externalReservation) * kBudgetCap);
}

// Removes a heap from the LRU and returns the least recently used heap when possible. Returns
// nullptr when nothing further can be evicted.
ResultOrError<Pageable*> ResidencyManager::RemoveSingleEntryFromLRU(
    MemorySegmentInfo* memorySegment) {
    // If the LRU is empty, return nullptr to allow execution to continue. Note that fully
    // emptying the LRU is undesirable, because it can mean either 1) the LRU is not accurately
    // accounting for Dawn's GPU allocations, or 2) a component external to Dawn is using all of
    // the process budget and starving Dawn, which will cause thrash.
    if (memorySegment->lruCache.empty()) {
        return nullptr;
    }

    Pageable* pageable = memorySegment->lruCache.head()->value();

    ExecutionSerial lastSubmissionSerial = pageable->GetLastSubmission();

    // If the next candidate for eviction was inserted into the LRU during the current serial,
    // it is because more memory is being used in a single command list than is available.
    // In this scenario, we cannot make any more resources resident and thrashing must occur.
    if (lastSubmissionSerial == mDevice->GetQueue()->GetPendingCommandSerial()) {
        return nullptr;
    }

    // We must ensure that any previous use of a resource has completed before the resource can
    // be evicted.
    DAWN_TRY(ToBackend(mDevice->GetQueue())->WaitForSerial(lastSubmissionSerial));

    pageable->RemoveFromList();
    return pageable;
}

MaybeError ResidencyManager::EnsureCanAllocate(uint64_t allocationSize,
                                               MemorySegment memorySegment) {
    if (!mResidencyManagementEnabled) {
        return {};
    }

    [[maybe_unused]] uint64_t bytesEvicted;
    DAWN_TRY_ASSIGN(bytesEvicted,
                    EnsureCanMakeResident(allocationSize, GetMemorySegmentInfo(memorySegment)));
    return {};
}

// Any time we need to make something resident, we must check that we have enough free memory to
// make the new object resident while also staying within budget. If there isn't enough
// memory, we should evict until there is. Returns the number of bytes evicted.
ResultOrError<uint64_t> ResidencyManager::EnsureCanMakeResident(uint64_t sizeToMakeResident,
                                                                MemorySegmentInfo* memorySegment) {
    DAWN_ASSERT(mResidencyManagementEnabled);

    UpdateMemorySegmentInfo(memorySegment);

    uint64_t memoryUsageAfterMakeResident = sizeToMakeResident + memorySegment->usage;

    // Return when we can call MakeResident and remain under budget.
    if (memoryUsageAfterMakeResident < memorySegment->budget) {
        return 0;
    }

    std::vector<ID3D12Pageable*> resourcesToEvict;
    uint64_t sizeNeededToBeUnderBudget = memoryUsageAfterMakeResident - memorySegment->budget;
    uint64_t sizeEvicted = 0;
    while (sizeEvicted < sizeNeededToBeUnderBudget) {
        Pageable* pageable;
        DAWN_TRY_ASSIGN(pageable, RemoveSingleEntryFromLRU(memorySegment));

        // If no heap was returned, then nothing more can be evicted.
        if (pageable == nullptr) {
            break;
        }

        sizeEvicted += pageable->GetSize();
        resourcesToEvict.push_back(pageable->GetD3D12Pageable());
    }

    if (resourcesToEvict.size() > 0) {
        DAWN_TRY(CheckHRESULT(
            mDevice->GetD3D12Device()->Evict(static_cast<uint32_t>(resourcesToEvict.size()),
                                             resourcesToEvict.data()),
            "Evicting resident heaps to free memory"));
    }

    return sizeEvicted;
}

// Given a list of heaps that are pending usage, this function will estimate memory needed,
// evict resources until enough space is available, then make resident any heaps scheduled for
// usage.
MaybeError ResidencyManager::EnsureHeapsAreResident(Heap** heaps, size_t heapCount) {
    if (!mResidencyManagementEnabled) {
        return {};
    }

    std::vector<ID3D12Pageable*> localHeapsToMakeResident;
    std::vector<ID3D12Pageable*> nonLocalHeapsToMakeResident;
    uint64_t localSizeToMakeResident = 0;
    uint64_t nonLocalSizeToMakeResident = 0;

    ExecutionSerial pendingCommandSerial = mDevice->GetQueue()->GetPendingCommandSerial();
    for (size_t i = 0; i < heapCount; i++) {
        Heap* heap = heaps[i];

        // Heaps that are locked resident are not tracked in the LRU cache.
        if (heap->IsResidencyLocked()) {
            continue;
        }

        if (heap->IsInResidencyLRUCache()) {
            // If the heap is already in the LRU, we must remove it and append again below to
            // update its position in the LRU.
            heap->RemoveFromList();
        } else {
            if (heap->GetMemorySegment() == MemorySegment::Local) {
                localSizeToMakeResident += heap->GetSize();
                localHeapsToMakeResident.push_back(heap->GetD3D12Pageable());
            } else {
                nonLocalSizeToMakeResident += heap->GetSize();
                nonLocalHeapsToMakeResident.push_back(heap->GetD3D12Pageable());
            }
        }

        // If we submit a command list to the GPU, we must ensure that heaps referenced by that
        // command list stay resident at least until that command list has finished execution.
        // Setting this serial unnecessarily can leave the LRU in a state where nothing is
        // eligible for eviction, even though some evictions may be possible.
        heap->SetLastSubmission(pendingCommandSerial);

        // Insert the heap into the appropriate LRU.
        TrackResidentAllocation(heap);
    }

    if (localSizeToMakeResident > 0) {
        return MakeAllocationsResident(&mVideoMemoryInfo.local, localSizeToMakeResident,
                                       localHeapsToMakeResident.size(),
                                       localHeapsToMakeResident.data());
    }

    if (nonLocalSizeToMakeResident > 0) {
        DAWN_ASSERT(!mDevice->GetDeviceInfo().isUMA);
        return MakeAllocationsResident(&mVideoMemoryInfo.nonLocal, nonLocalSizeToMakeResident,
                                       nonLocalHeapsToMakeResident.size(),
                                       nonLocalHeapsToMakeResident.data());
    }

    return {};
}

MaybeError ResidencyManager::MakeAllocationsResident(MemorySegmentInfo* segment,
                                                     uint64_t sizeToMakeResident,
                                                     uint64_t numberOfObjectsToMakeResident,
                                                     ID3D12Pageable** allocations) {
    [[maybe_unused]] uint64_t bytesEvicted;
    DAWN_TRY_ASSIGN(bytesEvicted, EnsureCanMakeResident(sizeToMakeResident, segment));

    // Note that MakeResident is a synchronous function and can add a significant
    // overhead to command recording. In the future, it may be possible to decrease this
    // overhead by using MakeResident on a secondary thread, or by instead making use of
    // the EnqueueMakeResident function (which is not available on all Windows 10
    // platforms).
    HRESULT hr = mDevice->GetD3D12Device()->MakeResident(
        static_cast<uint32_t>(numberOfObjectsToMakeResident), allocations);

    // A MakeResident call can fail if there's not enough available memory. This
    // could occur when there's significant fragmentation or if the allocation size
    // estimates are incorrect. We may be able to continue execution by evicting some
    // more memory and calling MakeResident again.
    while (FAILED(hr)) {
        constexpr uint32_t kAdditonalSizeToEvict = 50000000;  // 50MB

        uint64_t sizeEvicted = 0;

        DAWN_TRY_ASSIGN(sizeEvicted, EnsureCanMakeResident(kAdditonalSizeToEvict, segment));

        // If nothing can be evicted after MakeResident has failed, we cannot continue
        // execution and must throw a fatal error.
        if (sizeEvicted == 0) {
            return DAWN_OUT_OF_MEMORY_ERROR(
                "MakeResident has failed due to excessive video memory usage.");
        }

        hr = mDevice->GetD3D12Device()->MakeResident(
            static_cast<uint32_t>(numberOfObjectsToMakeResident), allocations);
    }

    return {};
}

// Inserts a heap at the bottom of the LRU. The passed heap must be resident or scheduled to
// become resident within the current serial. Failing to call this function when an allocation
// is implicitly made resident will cause the residency manager to view the allocation as
// non-resident and call MakeResident - which will make D3D12's internal residency refcount on
// the allocation out of sync with Dawn.
void ResidencyManager::TrackResidentAllocation(Pageable* pageable) {
    if (!mResidencyManagementEnabled) {
        return;
    }

    DAWN_ASSERT(pageable->IsInList() == false);
    GetMemorySegmentInfo(pageable->GetMemorySegment())->lruCache.Append(pageable);
}

// Places an artifical cap on Dawn's budget so we can test in a predictable manner. If used,
// this function must be called before any resources have been created.
void ResidencyManager::RestrictBudgetForTesting(uint64_t artificialBudgetCap) {
    DAWN_ASSERT(mVideoMemoryInfo.nonLocal.lruCache.empty());
    DAWN_ASSERT(!mRestrictBudgetForTesting);

    mRestrictBudgetForTesting = true;
    UpdateVideoMemoryInfo();

    // Dawn has a non-zero memory usage even before any resources have been created, and this
    // value can vary depending on the environment Dawn is running in. By adding this in
    // addition to the artificial budget cap, we can create a predictable and reproducible
    // budget for testing.
    mVideoMemoryInfo.local.budget = mVideoMemoryInfo.local.usage + artificialBudgetCap;
    if (!mDevice->GetDeviceInfo().isUMA) {
        mVideoMemoryInfo.nonLocal.budget = mVideoMemoryInfo.nonLocal.usage + artificialBudgetCap;
    }
}

}  // namespace dawn::native::d3d12
