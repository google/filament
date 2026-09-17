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
// functions MUST be protected by external synchronization (e.g., a utils::Mutex).
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

        [[nodiscard]] bool isFree() const noexcept {
            return !isAllocated;
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

    // Call it when a slot is no longer in use by the MaterialInstance or GPU.
    // The slot is released and potential merging is performed immediately.
    void retire(AllocationId id);

    // Resets the allocator to its initial state with a new total size.
    // All existing allocations are cleared.
    void reset(allocation_size_t newTotalSize);

    // Size of the UBO in bytes.
    [[nodiscard]] allocation_size_t getTotalSize() const noexcept;

    // Query the allocation offset by AllocationId.
    [[nodiscard]] allocation_size_t getAllocationOffset(AllocationId id) const;

    [[nodiscard]] allocation_size_t alignUp(allocation_size_t size) const noexcept;

    // Rounds `size` down to a multiple of the slot size.
    [[nodiscard]] allocation_size_t alignDown(allocation_size_t size) const noexcept;

    [[nodiscard]] allocation_size_t getAllocationSize(AllocationId id) const;

    [[nodiscard]] static bool isValid(AllocationId id);

    // Number of allocations that have not been retired yet.
    [[nodiscard]] uint32_t getAllocationCount() const noexcept { return mAllocationCount; }

private:
    [[nodiscard]] allocation_size_t slotIndexFromOffset(allocation_size_t offset) const noexcept;
    [[nodiscard]] AllocationId calculateIdByOffset(allocation_size_t offset) const;

    // Having an internal node type holding the base slot node and additional information.
    struct InternalSlotNode {
        Slot slot;
        std::multimap<allocation_size_t, InternalSlotNode*>::iterator freeListIterator;
    };

    [[nodiscard]] InternalSlotNode* getNodeById(AllocationId id);
    [[nodiscard]] const InternalSlotNode* getNodeById(AllocationId id) const;

    void copySlotToTail(allocation_size_t tailIndex, const InternalSlotNode* head) noexcept;
    void freeSlot(InternalSlotNode* node);

    allocation_size_t mTotalSize;
    const allocation_size_t mSlotSize; // Size of a single slot in bytes
    const uint8_t mSlotSizeShift;
    utils::FixedCapacityVector<InternalSlotNode> mNodes;
    std::multimap</*slot size*/ allocation_size_t, InternalSlotNode*> mFreeList;
    uint32_t mAllocationCount = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_BUFFERALLOCATOR_H
