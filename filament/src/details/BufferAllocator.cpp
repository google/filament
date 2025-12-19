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

    mNodePool.clear();
    mFreeList.clear();
    mSlotMap.clear();
    mFreeNodeIndices.clear();

    const size_t maxSlots = mTotalSize / mSlotSize;
    mNodePool.resize(maxSlots);
    // Initialize the pool with a single large free slot at index 0.
    mNodePool[0] = InternalSlotNode{
        .slot = { .offset = 0, .slotSize = mTotalSize, .isAllocated = false, .gpuUseCount = 0 }
    };

    mFreeNodeIndices.clear();
    mFreeNodeIndices.reserve(maxSlots - 1);
    // Fill free node indices with 1..maxSlots-1 (0 is taken)
    for (size_t i = 0; i < maxSlots - 1; ++i) {
        mFreeNodeIndices.push_back(maxSlots - 1 - i);
    }

    InternalSlotNode* firstNode = &mNodePool[0];

    // Add the initial free slot to the free list.
    auto freeListIter = mFreeList.emplace(newTotalSize, firstNode);

    mSlotMap.resize(mTotalSize / mSlotSize, nullptr);
    mSlotMap[0] = firstNode;
    mSlotMap[mSlotMap.size() - 1] = firstNode;

    firstNode->freeListIterator = freeListIter;
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
    targetNode->freeListIterator = mFreeList.end();
    targetNode->slot.isAllocated = true;

    // Split the slot if it is larger than what we need.
    if (originalSlotSize > alignedSize) {
        targetNode->slot.slotSize = alignedSize;

        allocation_size_t remainingSize = originalSlotSize - alignedSize;
        allocation_size_t newSlotOffset = targetNode->slot.offset + alignedSize;
        assert_invariant(remainingSize % mSlotSize == 0);
        assert_invariant(newSlotOffset % mSlotSize == 0);

        // Get a free node from the pool
        assert_invariant(!mFreeNodeIndices.empty());
        uint32_t nodeIndex = mFreeNodeIndices.back();
        mFreeNodeIndices.pop_back();

        InternalSlotNode* newNode = &mNodePool[nodeIndex];
        *newNode = InternalSlotNode{ .slot = { .offset = newSlotOffset,
                                         .slotSize = remainingSize,
                                         .isAllocated = false,
                                         .gpuUseCount = 0 } };

        // Add the new free slot to our tracking maps.
        auto freeListIter = mFreeList.emplace(remainingSize, newNode);

        // Update mSlotMap for the new node (start and end)
        size_t newStartIdx = newSlotOffset / mSlotSize;
        size_t newEndIdx = (newSlotOffset + remainingSize) / mSlotSize - 1;
        mSlotMap[newStartIdx] = newNode;
        mSlotMap[newEndIdx] = newNode;

        newNode->freeListIterator = freeListIter;

        // Update targetNode's map entry for its new size
        // targetNode now ends at newSlotOffset - 1
        size_t targetEndIdx = newStartIdx - 1;
        mSlotMap[targetEndIdx] = targetNode;
    }

    AllocationId allocationId = calculateIdByOffset(targetNode->slot.offset);
    return { allocationId, targetNode->slot.offset };
}

BufferAllocator::InternalSlotNode* BufferAllocator::getNodeById(
        AllocationId id) const noexcept {
    if (!isValid(id)) {
        return nullptr;
    }

    // AllocationId is 1-based
    size_t index = id - 1;
    if (index >= mSlotMap.size()) {
        return nullptr;
    }
    return mSlotMap[index];
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
    // Check previous
    const size_t currentStartIdx = node->slot.offset / mSlotSize;
    if (currentStartIdx > 0) {
        InternalSlotNode* prev = mSlotMap[currentStartIdx - 1];
        if (prev && prev->slot.isFree()) {
            prev->slot.slotSize += node->slot.slotSize;
            assert_invariant(prev->slot.slotSize % mSlotSize == 0);

            mFreeList.erase(prev->freeListIterator);

            size_t nodeIndex = std::distance(&mNodePool[0], node);
            mFreeNodeIndices.push_back(nodeIndex);

            node = prev;
        }
    }

    // Check next
    const size_t currentEndIdx = (node->slot.offset + node->slot.slotSize) / mSlotSize;
    if (currentEndIdx < mSlotMap.size()) {
        InternalSlotNode* next = mSlotMap[currentEndIdx];
        if (next && next->slot.isFree()) {
            node->slot.slotSize += next->slot.slotSize;
            assert_invariant(node->slot.slotSize % mSlotSize == 0);

            mFreeList.erase(next->freeListIterator);

            size_t nextIndex = std::distance(&mNodePool[0], next);
            mFreeNodeIndices.push_back(nextIndex);
        }
    }

    size_t newStartIdx = node->slot.offset / mSlotSize;
    size_t newEndIdx = (node->slot.offset + node->slot.slotSize) / mSlotSize - 1;
    mSlotMap[newStartIdx] = node;
    mSlotMap[newEndIdx] = node;

    node->freeListIterator = mFreeList.emplace(node->slot.slotSize, node);
}

BufferAllocator::allocation_size_t BufferAllocator::getTotalSize() const noexcept {
    return mTotalSize;
}

BufferAllocator::allocation_size_t
    BufferAllocator::getAllocationOffset(AllocationId id) const {
    assert_invariant(isValid(id));

    return (id - 1) * mSlotSize;
}

bool BufferAllocator::isLockedByGpu(AllocationId id) const {
    InternalSlotNode* targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);

    return targetNode->slot.gpuUseCount > 0;
}

BufferAllocator::AllocationId BufferAllocator::calculateIdByOffset(
        allocation_size_t offset) const {
    assert_invariant(offset % mSlotSize == 0);

    // The ID is 1-based since we use 0 for UNALLOCATED.
    return (offset / mSlotSize) + 1;
}

BufferAllocator::allocation_size_t BufferAllocator::getAllocationSize(AllocationId id) const {
    InternalSlotNode* targetNode = getNodeById(id);
    assert_invariant(targetNode != nullptr);

    return targetNode->slot.slotSize;
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
