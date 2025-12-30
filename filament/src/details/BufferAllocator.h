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

#ifndef TNT_FILAMENT_DETAILS_BUFFERALLOCATOR_H
#define TNT_FILAMENT_DETAILS_BUFFERALLOCATOR_H

#include <utils/FixedCapacityVector.h>

#include <cstdint>
#include <map>


namespace filament {

// This class is NOT thread-safe.
//
// It internally manages shared state (e.g., mSlotPool, mFreeList, mOffsetMap) without any
// synchronization primitives. Concurrent access from multiple threads to the same
// BufferAllocator instance will result in data races and undefined behavior.
//
// If an instance of this class is to be shared between threads, all calls to its member
// functions MUST be protected by external synchronization (e.g., a std::mutex).
class BufferAllocator {
public:
    using allocation_size_t = uint32_t;
    using AllocationId = uint32_t;

    static constexpr AllocationId UNALLOCATED = 0;
    static constexpr AllocationId REALLOCATION_REQUIRED = ~0u;

    struct Slot {
        allocation_size_t offset;             // 4 bytes
        allocation_size_t slotSize;           // 4 bytes
        bool isAllocated;                     // 1 byte
        char padding[3];                      // 3 bytes
        uint32_t gpuUseCount;                 // 4 bytes

        [[nodiscard]] bool isFree() const noexcept {
            return !isAllocated && gpuUseCount == 0;
        }
    };

    // `slotSize` is derived from the GPU's uniform buffer offset alignment requirement,
    // which can be up to 256 bytes.
    explicit BufferAllocator(allocation_size_t totalSize,
            allocation_size_t slotSize);

    BufferAllocator(BufferAllocator const&) = delete;
    BufferAllocator(BufferAllocator&&) = delete;

    // Allocate a new slot and return its id and slot offset in the UBO.
    // If the returned id is not valid, that means there's no large enough slot for allocation.
    [[nodiscard]] std::pair<AllocationId, allocation_size_t> allocate(
            allocation_size_t size) noexcept;

    // Call it when MaterialInstance gives up the ownership of the allocation.
    // The slot is released and potential merging is performed immediately.
    void retire(AllocationId id);

    // Increments the GPU read-lock.
    void acquireGpu(AllocationId id);

    // Decrements the GPU read-lock.
    // If the count reaches zero and the slot is not allocated, it is released and merged
    // immediately.
    void releaseGpu(AllocationId id);

    // Resets the allocator to its initial state with a new total size.
    // All existing allocations are cleared.
    void reset(allocation_size_t newTotalSize);

    // Size of the UBO in bytes.
    [[nodiscard]] allocation_size_t getTotalSize() const noexcept;

    // Query the allocation offset by AllocationId.
    [[nodiscard]] allocation_size_t getAllocationOffset(AllocationId id) const;

    [[nodiscard]] bool isLockedByGpu(AllocationId id) const;

    [[nodiscard]] allocation_size_t alignUp(allocation_size_t size) const noexcept;

    [[nodiscard]] allocation_size_t getAllocationSize(AllocationId id) const;

    [[nodiscard]] static bool isValid(AllocationId id);

private:
    [[nodiscard]] AllocationId calculateIdByOffset(allocation_size_t offset) const;

    // Having an internal node type holding the base slot node and additional information.
    struct InternalSlotNode {
        Slot slot;
        std::multimap<allocation_size_t, InternalSlotNode*>::iterator freeListIterator;
    };

    [[nodiscard]] InternalSlotNode* getNodeById(AllocationId id);
    [[nodiscard]] const InternalSlotNode* getNodeById(AllocationId id) const;

    void freeSlot(InternalSlotNode* node);

    allocation_size_t mTotalSize;
    const allocation_size_t mSlotSize; // Size of a single slot in bytes
    utils::FixedCapacityVector<InternalSlotNode> mNodes;
    std::multimap</*slot size*/ allocation_size_t, InternalSlotNode*> mFreeList;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_BUFFERALLOCATOR_H
