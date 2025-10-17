/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "details/BufferAllocator.h"

#include <utils/Panic.h>
#include <utils/debug.h>

namespace filament {
namespace {

static bool isValidId(BufferAllocator::AllocationId id) {
    return id != BufferAllocator::UNALLOCATED && id != BufferAllocator::REALLOCATION_REQUIRED;
}

#ifndef NDEBUG
constexpr static bool isPowerOfTwo(uint32_t n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}
#endif

} // anonymous namespace

BufferAllocator::BufferAllocator(allocation_size_t totalSize, allocation_size_t slotSize)
    : mTotalSize(totalSize),
      mSlotSize(slotSize) {
    assert_invariant(mSlotSize > 0);
    assert_invariant(isPowerOfTwo(mSlotSize));

    reset(mTotalSize);
}

void BufferAllocator::reset(allocation_size_t newTotalSize) {
    assert_invariant(newTotalSize % mSlotSize == 0);

    mTotalSize = newTotalSize;

    mSlotPool.clear();
    mFreeList.clear();
    mOffsetMap.clear();

    // Initialize the pool with a single large free slot.
    mSlotPool.emplace_back(InternalSlotNode{
        .mSlot = {
            .offset = 0,
            .slotSize = mTotalSize,
            .isAllocated = false,
            .gpuUseCount = 0
        }
    });

    InternalSlotNode* firstNode = &mSlotPool.front();

    // Add this initial free slot to the free list and offset map.
    auto freeListIter = mFreeList.emplace(newTotalSize, firstNode);
    auto offsetMapIter = mOffsetMap.emplace(0, firstNode);

    firstNode->mSlotPoolIterator = mSlotPool.begin();
    firstNode->mFreeListIterator = freeListIter;
    firstNode->mOffsetMapIterator = offsetMapIter.first;
}

std::pair<BufferAllocator::AllocationId, BufferAllocator::allocation_size_t>
    BufferAllocator::allocate(allocation_size_t size) noexcept {
    if (size == 0) {
        return { UNALLOCATED, 0 };
    }

    const allocation_size_t alignedSize = alignUp(size);
    auto bestFitIter = mFreeList.lower_bound(alignedSize);

    if (bestFitIter == mFreeList.end()) {
        return { REALLOCATION_REQUIRED, 0 };
    }

    InternalSlotNode* targetNode = bestFitIter->second;
    const allocation_size_t originalSlotSize = targetNode->mSlot.slotSize;

    mFreeList.erase(bestFitIter);
    targetNode->mFreeListIterator = mFreeList.end();
    targetNode->mSlot.isAllocated = true;

    // Split the slot if it is larger than what we need.
    if (originalSlotSize > alignedSize) {
        targetNode->mSlot.slotSize = alignedSize;

        allocation_size_t remainingSize = originalSlotSize - alignedSize;
        allocation_size_t newSlotOffset = targetNode->mSlot.offset + alignedSize;
        assert_invariant(remainingSize % mSlotSize == 0);
        assert_invariant(newSlotOffset % mSlotSize == 0);

        // Create a new node for the remaining free space.
        auto insertPos = std::next(targetNode->mSlotPoolIterator);
        auto newNodeIter = mSlotPool.emplace(insertPos, InternalSlotNode{
            .mSlot = {
                .offset = newSlotOffset,
                .slotSize = remainingSize,
                .isAllocated = false,
                .gpuUseCount = 0
            }
        });
        InternalSlotNode* newNode = &(*newNodeIter);

        // Add the new free slot to our tracking maps.
        auto freeListIter = mFreeList.emplace(remainingSize, newNode);
        auto offsetMapIter = mOffsetMap.emplace(newSlotOffset, newNode);
        newNode->mSlotPoolIterator = newNodeIter;
        newNode->mFreeListIterator = freeListIter;
        newNode->mOffsetMapIterator = offsetMapIter.first;
    }

    auto allocationId = calculateIdByOffset(targetNode->mSlot.offset);
    return { allocationId, targetNode->mSlot.offset };
}

BufferAllocator::InternalSlotNode* BufferAllocator::getNodeById(
        AllocationId id) const noexcept {
    if (!isValidId(id)) {
        return nullptr;
    }

    auto offset = getAllocationOffset(id);
    auto iter = mOffsetMap.find(offset);

    // We cannot find the corresponding node in the map.
    if (iter == mOffsetMap.end()) {
        return nullptr;
    }
    return iter->second;
}

void BufferAllocator::retire(AllocationId id) {
    auto targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);

    targetNode->mSlot.isAllocated = false;
}

void BufferAllocator::acquireGpu(AllocationId id) {
    auto targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);

    targetNode->mSlot.gpuUseCount++;
}

void BufferAllocator::releaseGpu(AllocationId id) {
    auto targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);
    assert_invariant(targetNode->mSlot.gpuUseCount > 0);

    targetNode->mSlot.gpuUseCount--;
}

void BufferAllocator::releaseFreeSlots() {
    auto curr = mSlotPool.begin();
    while (curr != mSlotPool.end()) {
        if (!curr->mSlot.isFree()) {
            ++curr;
            continue;
        }

        auto next = std::next(curr);
        bool merged = false;
        while (next != mSlotPool.end() && next->mSlot.isFree()) {
            merged = true;
            // Combine the size of free slots
            curr->mSlot.slotSize += next->mSlot.slotSize;
            assert_invariant(curr->mSlot.slotSize % mSlotSize == 0);

            // Erase the merged slot from all maps
            if (next->mFreeListIterator != mFreeList.end()) {
                mFreeList.erase(next->mFreeListIterator);
            }
            mOffsetMap.erase(next->mOffsetMapIterator);
            next = mSlotPool.erase(next);
        }

        // If we performed any merge, the current block's size has changed.
        // We need to update its position in the mFreeList.
        if (curr->mFreeListIterator != mFreeList.end()) {
            // If it's already in the free list and we merged, we need to update it.
            if (merged) {
                mFreeList.erase(curr->mFreeListIterator);
                curr->mFreeListIterator = mFreeList.emplace(curr->mSlot.slotSize, &(*curr));
            }
        } else {
            // If it's not in the free list, it must be a newly freed block. Add it.
            curr->mFreeListIterator = mFreeList.emplace(curr->mSlot.slotSize, &(*curr));
        }

        curr = next;
    }
}

BufferAllocator::allocation_size_t BufferAllocator::getTotalSize() const noexcept {
    return mTotalSize;
}

BufferAllocator::allocation_size_t
    BufferAllocator::getAllocationOffset(AllocationId id) const {
    assert_invariant(isValidId(id));

    return (id - 1) * mSlotSize;
}

bool BufferAllocator::isLockedByGpu(AllocationId id) const {
    auto targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);

    return targetNode->mSlot.gpuUseCount > 0;
}

BufferAllocator::AllocationId BufferAllocator::calculateIdByOffset(
        allocation_size_t offset) const {
    assert_invariant(offset % mSlotSize == 0);

    // The ID is 1-based since we use 0 for UNALLOCATED.
    return (offset / mSlotSize) + 1;
}

BufferAllocator::allocation_size_t BufferAllocator::getAllocationSize(AllocationId id) const {
    auto targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);

    return targetNode->mSlot.slotSize;
}

BufferAllocator::allocation_size_t BufferAllocator::alignUp(
        allocation_size_t size) const noexcept {
    if (size == 0) return 0;

    return (size + mSlotSize - 1) & ~(mSlotSize - 1);
}

} // namespace filament
