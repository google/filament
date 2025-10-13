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

#include <cstdint>
#include <list>
#include <map>
#include <unordered_map>

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
        const allocation_size_t offset;       // 4 bytes
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
    // We don't release the slot immediately in this function even if it is not being used,
    // the release is centralized in releaseFreeSlots().
    void retire(AllocationId id);

    // Increments the GPU read-lock.
    void acquireGpu(AllocationId id);

    // Decrements the GPU read-lock.
    // We don't release the slot immediately in this function even if it is not being used,
    // the release is centralized in releaseFreeSlots().
    void releaseGpu(AllocationId id);

    // Traverse all slots and free all slots that are not being used by both CPU and GPU.
    // Perform the merge at the same time.
    void releaseFreeSlots();

    // Resets the allocator to its initial state with a new total size.
    // All existing allocations are cleared.
    void reset(allocation_size_t newTotalSize);

    // Size of the UBO in bytes.
    [[nodiscard]] allocation_size_t getTotalSize() const noexcept;

    // Query the allocation offset by AllocationId.
    allocation_size_t getAllocationOffset(AllocationId id) const;

private:
    [[nodiscard]] AllocationId calculateIdByOffset(allocation_size_t offset) const;
    [[nodiscard]] allocation_size_t alignUp(allocation_size_t size) const noexcept;

    // Having an internal node type holding the base slot node and additional information.
    struct InternalSlotNode {
        Slot mSlot;
        std::list<InternalSlotNode>::iterator mSlotPoolIterator;
        std::multimap<allocation_size_t, InternalSlotNode*>::iterator mFreeListIterator;
        std::unordered_map<allocation_size_t, InternalSlotNode*>::iterator mOffsetMapIterator;
    };

    [[nodiscard]] InternalSlotNode* getNodeById(AllocationId id) const noexcept;

    allocation_size_t mTotalSize;
    allocation_size_t mSlotSize; // Size of a single slot in bytes.
    std::list<InternalSlotNode> mSlotPool; // All slots, including both allocated and freed
    std::multimap</*slot size*/allocation_size_t, InternalSlotNode*> mFreeList;
    std::unordered_map</*slot offset*/allocation_size_t, InternalSlotNode*> mOffsetMap;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_BUFFERALLOCATOR_H
