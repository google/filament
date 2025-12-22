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

#include <private/utils/Tracing.h>
#include <utils/Panic.h>
#include <utils/debug.h>

namespace filament {
namespace {

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
    mFreeList.clear();

    // Resize mNodes to the number of slots
    size_t slotCount = mTotalSize / mSlotSize;
    mNodes.clear();
    mNodes.reserve(slotCount);
    mNodes.resize(slotCount);

    // Initialize the single free block covering the entire buffer
    InternalSlotNode* node = &mNodes[0];
    node->slot.offset = 0;
    node->slot.slotSize = mTotalSize;
    node->slot.isAllocated = false;
    node->slot.gpuUseCount = 0;
    node->freeListIterator = mFreeList.emplace(mTotalSize, node);

    // Set the tail tag
    mNodes[slotCount - 1] = *node;
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
    const allocation_size_t originalSlotSize = targetNode->slot.slotSize;

    mFreeList.erase(bestFitIter);

    const allocation_size_t remainingSize = targetNode->slot.slotSize - alignedSize;
    const allocation_size_t offset = targetNode->slot.offset;
    assert_invariant(remainingSize % mSlotSize == 0);
    assert_invariant((offset + alignedSize) % mSlotSize == 0);

    // Split the slot if it is larger than what we need.
    if (originalSlotSize > alignedSize) {
        // Update Head block
        targetNode->slot.slotSize = alignedSize;
        targetNode->slot.isAllocated = true;

        // Update Tail block
        size_t endSlotIndex = (offset + alignedSize) / mSlotSize - 1;
        mNodes[endSlotIndex] = *targetNode; // Copy Head to Tail

        // The Head of remaining free block
        size_t nextSlotIndex = endSlotIndex + 1;
        InternalSlotNode* nextNode = &mNodes[nextSlotIndex];
        nextNode->slot.offset = offset + alignedSize;
        nextNode->slot.slotSize = remainingSize;
        nextNode->slot.isAllocated = false;
        nextNode->slot.gpuUseCount = 0;
        nextNode->freeListIterator = mFreeList.emplace(remainingSize, nextNode);

        // Update the Tail of remaining free block
        size_t nextEndSlotIndex = (nextNode->slot.offset + nextNode->slot.slotSize) / mSlotSize - 1;
        mNodes[nextEndSlotIndex] = *nextNode; // Copy Head to Tail
    } else {
        // Allocate the whole block
        targetNode->slot.isAllocated = true;

        // Update Tail block
        size_t endSlotIndex = (offset + targetNode->slot.slotSize) / mSlotSize - 1;
        mNodes[endSlotIndex] = *targetNode;
    }

    return { calculateIdByOffset(offset), offset };
}

BufferAllocator::InternalSlotNode* BufferAllocator::getNodeById(AllocationId id) {
    assert_invariant(id > 0);
    assert_invariant(id <= mNodes.size());
    return &mNodes[id - 1];
}

const BufferAllocator::InternalSlotNode* BufferAllocator::getNodeById(
        AllocationId id) const {
    assert_invariant(id > 0);
    assert_invariant(id <= mNodes.size());
    return &mNodes[id - 1];
}

void BufferAllocator::retire(AllocationId id) {
    InternalSlotNode* targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);

    Slot& slot = targetNode->slot;
    slot.isAllocated = false;
    if (slot.gpuUseCount == 0) {
        freeSlot(targetNode);
    }
}

void BufferAllocator::acquireGpu(AllocationId id) {
    InternalSlotNode* targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);

    // Should not happen, just for safety
    if (UTILS_UNLIKELY(targetNode->slot.isFree())) {
        mFreeList.erase(targetNode->freeListIterator);
        targetNode->freeListIterator = mFreeList.end();
    }

    targetNode->slot.gpuUseCount++;
}

void BufferAllocator::releaseGpu(AllocationId id) {
    InternalSlotNode* targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);
    assert_invariant(targetNode->slot.gpuUseCount > 0);

    Slot& slot = targetNode->slot;
    slot.gpuUseCount--;
    if (slot.gpuUseCount == 0 && !slot.isAllocated) {
        freeSlot(targetNode);
    }
}

void BufferAllocator::freeSlot(InternalSlotNode* node) {
    size_t currentStartIdx = node->slot.offset / mSlotSize;
    size_t currentEndIdx = (node->slot.offset + node->slot.slotSize) / mSlotSize - 1;

    // Check Previous (Left Neighbor)
    if (currentStartIdx > 0) {
        InternalSlotNode* prev = &mNodes[currentStartIdx - 1];
        size_t prevHeadIdx = prev->slot.offset / mSlotSize;
        InternalSlotNode* prevHead = &mNodes[prevHeadIdx];

        if (prevHead->slot.isFree()) {
            // Merge with prev
            // prev is the TAIL of the left block.

            assert_invariant(prevHead->slot.offset == prev->slot.offset);
            assert_invariant(prevHead->slot.slotSize == prev->slot.slotSize);

            mFreeList.erase(prevHead->freeListIterator);
            prevHead->slot.slotSize += node->slot.slotSize;
            assert_invariant(prevHead->slot.slotSize % mSlotSize == 0);

            // Switch current node pointer to prevHead for potential next merge
            node = prevHead;
            currentStartIdx = prevHeadIdx;
        }
    }

    // Check Next (Right Neighbor)
    size_t nextStartIdx = currentEndIdx + 1;
    if (nextStartIdx < mNodes.size()) {
        InternalSlotNode* next = &mNodes[nextStartIdx];
        if (next->slot.isFree()) {
            // Merge with next

            mFreeList.erase(next->freeListIterator);
            node->slot.slotSize += next->slot.slotSize;
            assert_invariant(node->slot.slotSize % mSlotSize == 0);

            // currentEndIdx increases
            currentEndIdx = (node->slot.offset + node->slot.slotSize) / mSlotSize - 1;
        }
    }

    assert_invariant(node->slot.offset / mSlotSize == currentStartIdx);

    // Push merged free block to the list
    node->freeListIterator = mFreeList.emplace(node->slot.slotSize, node);

    // Copy Head to Tail
    mNodes[currentEndIdx] = *node;
}

BufferAllocator::allocation_size_t BufferAllocator::getTotalSize() const noexcept {
    return mTotalSize;
}

BufferAllocator::allocation_size_t
    BufferAllocator::getAllocationOffset(AllocationId id) const {
    return getNodeById(id)->slot.offset;
}

bool BufferAllocator::isLockedByGpu(AllocationId id) const {
    const InternalSlotNode* targetNode = getNodeById(id);
    return targetNode->slot.gpuUseCount > 0;
}

BufferAllocator::AllocationId BufferAllocator::calculateIdByOffset(
        allocation_size_t offset) const {
    assert_invariant(offset % mSlotSize == 0);

    // The ID is 1-based since we use 0 for UNALLOCATED.
    return (offset / mSlotSize) + 1;
}

BufferAllocator::allocation_size_t BufferAllocator::getAllocationSize(AllocationId id) const {
    return getNodeById(id)->slot.slotSize;
}

bool BufferAllocator::isValid(AllocationId id) {
    return id != UNALLOCATED && id != REALLOCATION_REQUIRED;
}

BufferAllocator::allocation_size_t BufferAllocator::alignUp(
        allocation_size_t size) const noexcept {
    if (size == 0) return 0;

    return (size + mSlotSize - 1) & ~(mSlotSize - 1);
}

} // namespace filament
