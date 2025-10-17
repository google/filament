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

#ifndef TNT_FILAMENT_DETAILS_UBOMANAGER_H
#define TNT_FILAMENT_DETAILS_UBOMANAGER_H

#include "ResourceList.h"
#include "backend/DriverApiForward.h"

#include "details/BufferAllocator.h"

#include <backend/Handle.h>
#include <private/backend/DriverApi.h>

#include <unordered_set>

namespace filament {

class FMaterialInstance;

// This class is NOT thread-safe and designed to be used on a single core thread.
//
// It internally manages the actual allocator (e.g., mAllocator) without any
// synchronization primitives, and the allocator itself is not thread-safe as well. Concurrent
// access from multiple threads to the same UboManager instance will result in data races and
// undefined behavior.
class UboManager {
public:
    explicit UboManager(backend::DriverApi& driver,
            BufferAllocator::allocation_size_t defaultSlotSizeInBytes,
            BufferAllocator::allocation_size_t defaultTotalSizeInBytes);

    UboManager(UboManager const&) = delete;
    UboManager(UboManager&&) = delete;

    // This method manage most of the UBO allocation lifecycle, which includes:
    // 1. Releasing UBO slots from previous frames that are no longer in use by the GPU.
    // 2. Allocating new slots for MaterialInstances that need them (e.g., new instances or
    //    instances with modified uniforms).
    // 3. Reallocating a larger shared UBO if the current one is insufficient.
    // 4. Mapping the shared UBO into CPU-accessible memory to prepare for uniform data writes.
    void beginFrame(backend::DriverApi& driver,
            const ResourceList<FMaterialInstance>& materialInstances);

    // Unmap the buffer here
    void finishBeginFrame(backend::DriverApi& driver) const;

    // Create a fence and associate it with a set of allocation ids.
    // The gpuUseCount of these allocations will be incremented, and they will be decremented
    // After the corresponding frame has been done.
    void endFrame(backend::DriverApi& driver,
            const ResourceList<FMaterialInstance>& materialInstances);

    void terminate(backend::DriverApi& driver);

    void updateSlot(backend::DriverApi& driver, BufferAllocator::AllocationId id,
            backend::BufferDescriptor bufferDescriptor) const;

    // Returns the size of the actual UBO. Note that when there's allocation failed, it will be
    // reallocated to a bigger size at the next frame.
    [[nodiscard]] BufferAllocator::allocation_size_t getTotalSize() const noexcept;

private:
    constexpr static float BUFFER_SIZE_GROWTH_MULTIPLIER = 1.5f;

    // Query the offset by the allocation id.
    [[nodiscard]] BufferAllocator::allocation_size_t getAllocationOffset(
            BufferAllocator::AllocationId id) const;

    void checkFenceAndUnlockSlots(backend::DriverApi& driver);
    // Returns true if the current buffer needs reallocation.
    // Otherwise, returns false.
    bool updateMaterialInstanceAllocations(
            const ResourceList<FMaterialInstance>& materialInstances, bool forceAllocateAll);
    void reallocate(backend::DriverApi& driver, BufferAllocator::allocation_size_t requiredSize);
    BufferAllocator::allocation_size_t calculateRequiredSize(
            const ResourceList<FMaterialInstance>& materialInstances);

    backend::Handle<backend::HwBufferObject> mUbHandle;
    backend::MemoryMappedBufferHandle mMmbHandle;
    BufferAllocator::allocation_size_t mUboSize{};
    bool mNeedReallocate{};

    // Not ideal, but we need to know which slots to decrement gpuUseCount for each frame.
    using FenceAllocationList = std::vector<std::pair<backend::Handle<backend::HwFence>,
            std::unordered_set<BufferAllocator::AllocationId>>>;
    FenceAllocationList mFenceAllocationList;

    BufferAllocator mAllocator;
};

} // namespace filament

#endif
