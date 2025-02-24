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

#ifndef SRC_DAWN_NATIVE_D3D12_RESIDENCYMANAGERD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_RESIDENCYMANAGERD3D12_H_

#include "dawn/common/LinkedList.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/Error.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;
class Heap;
class Pageable;

class ResidencyManager {
  public:
    explicit ResidencyManager(Device* device);

    MaybeError LockAllocation(Pageable* pageable);
    void UnlockAllocation(Pageable* pageable);

    MaybeError EnsureCanAllocate(uint64_t allocationSize, MemorySegment memorySegment);
    MaybeError EnsureHeapsAreResident(Heap** heaps, size_t heapCount);

    uint64_t SetExternalMemoryReservation(MemorySegment segment, uint64_t requestedReservationSize);

    void TrackResidentAllocation(Pageable* pageable);

    void RestrictBudgetForTesting(uint64_t artificialBudgetCap);

  private:
    struct MemorySegmentInfo {
        const DXGI_MEMORY_SEGMENT_GROUP dxgiSegment;
        LinkedList<Pageable> lruCache = {};
        uint64_t budget = 0;
        uint64_t usage = 0;
        uint64_t externalReservation = 0;
        uint64_t externalRequest = 0;
    };

    struct VideoMemoryInfo {
        MemorySegmentInfo local = {DXGI_MEMORY_SEGMENT_GROUP_LOCAL};
        MemorySegmentInfo nonLocal = {DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL};
    };

    MemorySegmentInfo* GetMemorySegmentInfo(MemorySegment memorySegment);
    ResultOrError<uint64_t> EnsureCanMakeResident(uint64_t allocationSize,
                                                  MemorySegmentInfo* memorySegment);
    ResultOrError<Pageable*> RemoveSingleEntryFromLRU(MemorySegmentInfo* memorySegment);
    MaybeError MakeAllocationsResident(MemorySegmentInfo* segment,
                                       uint64_t sizeToMakeResident,
                                       uint64_t numberOfObjectsToMakeResident,
                                       ID3D12Pageable** allocations);
    void UpdateVideoMemoryInfo();
    void UpdateMemorySegmentInfo(MemorySegmentInfo* segmentInfo);

    raw_ptr<Device> mDevice;
    bool mResidencyManagementEnabled = false;
    bool mRestrictBudgetForTesting = false;
    VideoMemoryInfo mVideoMemoryInfo = {};
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_RESIDENCYMANAGERD3D12_H_
