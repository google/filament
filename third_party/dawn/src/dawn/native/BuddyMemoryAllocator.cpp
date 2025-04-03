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

#include "dawn/native/BuddyMemoryAllocator.h"

#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/ResourceHeapAllocator.h"

namespace dawn::native {

BuddyMemoryAllocator::BuddyMemoryAllocator(uint64_t maxSystemSize,
                                           uint64_t memoryBlockSize,
                                           ResourceHeapAllocator* heapAllocator)
    : mMemoryBlockSize(memoryBlockSize),
      mBuddyBlockAllocator(maxSystemSize),
      mHeapAllocator(heapAllocator) {
    DAWN_ASSERT(memoryBlockSize <= maxSystemSize);
    DAWN_ASSERT(IsPowerOfTwo(mMemoryBlockSize));
    DAWN_ASSERT(maxSystemSize % mMemoryBlockSize == 0);

    mTrackedSubAllocations.resize(maxSystemSize / mMemoryBlockSize);
}

BuddyMemoryAllocator::~BuddyMemoryAllocator() = default;

uint64_t BuddyMemoryAllocator::GetMemoryIndex(uint64_t offset) const {
    DAWN_ASSERT(offset != BuddyAllocator::kInvalidOffset);
    return offset / mMemoryBlockSize;
}

ResultOrError<ResourceMemoryAllocation> BuddyMemoryAllocator::Allocate(uint64_t allocationSize,
                                                                       uint64_t alignment) {
    ResourceMemoryAllocation invalidAllocation = ResourceMemoryAllocation{};

    if (allocationSize == 0) {
        return std::move(invalidAllocation);
    }

    // Check the unaligned size to avoid overflowing NextPowerOfTwo.
    if (allocationSize > mMemoryBlockSize) {
        return std::move(invalidAllocation);
    }

    uint64_t originalAllocationSize = allocationSize;
    // Round allocation size to nearest power-of-two.
    allocationSize = NextPowerOfTwo(allocationSize);

    // Allocation cannot exceed the memory size.
    if (allocationSize > mMemoryBlockSize) {
        return std::move(invalidAllocation);
    }

    // Attempt to sub-allocate a block of the requested size.
    const uint64_t blockOffset = mBuddyBlockAllocator.Allocate(allocationSize, alignment);
    if (blockOffset == BuddyAllocator::kInvalidOffset) {
        return std::move(invalidAllocation);
    }

    const uint64_t memoryIndex = GetMemoryIndex(blockOffset);
    if (mTrackedSubAllocations[memoryIndex].refcount == 0) {
        // Transfer ownership to this allocator
        std::unique_ptr<ResourceHeapBase> memory;
        DAWN_TRY_ASSIGN(memory, mHeapAllocator->AllocateResourceHeap(mMemoryBlockSize));
        mTrackedSubAllocations[memoryIndex] = {/*refcount*/ 0, std::move(memory)};
    }

    mTrackedSubAllocations[memoryIndex].refcount++;

    AllocationInfo info;
    info.mBlockOffset = blockOffset;
    info.mRequestedSize = originalAllocationSize;
    info.mMethod = AllocationMethod::kSubAllocated;

    // Allocation offset is always local to the memory.
    const uint64_t memoryOffset = blockOffset % mMemoryBlockSize;

    return ResourceMemoryAllocation{info, memoryOffset,
                                    mTrackedSubAllocations[memoryIndex].mMemoryAllocation.get()};
}

void BuddyMemoryAllocator::Deallocate(const ResourceMemoryAllocation& allocation) {
    const AllocationInfo info = allocation.GetInfo();

    DAWN_ASSERT(info.mMethod == AllocationMethod::kSubAllocated);

    const uint64_t memoryIndex = GetMemoryIndex(info.mBlockOffset);

    DAWN_ASSERT(mTrackedSubAllocations[memoryIndex].refcount > 0);
    mTrackedSubAllocations[memoryIndex].refcount--;

    if (mTrackedSubAllocations[memoryIndex].refcount == 0) {
        mHeapAllocator->DeallocateResourceHeap(
            std::move(mTrackedSubAllocations[memoryIndex].mMemoryAllocation));
    }

    mBuddyBlockAllocator.Deallocate(info.mBlockOffset);
}

uint64_t BuddyMemoryAllocator::GetMemoryBlockSize() const {
    return mMemoryBlockSize;
}

uint64_t BuddyMemoryAllocator::ComputeTotalNumOfHeapsForTesting() const {
    uint64_t count = 0;
    for (const TrackedSubAllocations& allocation : mTrackedSubAllocations) {
        if (allocation.refcount > 0) {
            count++;
        }
    }
    return count;
}

}  // namespace dawn::native
