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

#include "details/BufferAllocator.h"

#include <private/backend/DriverApi.h>

#include <backend/DriverApiForward.h>
#include <backend/Handle.h>

#include <cstdint>
#include <vector>

class UboManagerTest;

namespace filament {

class FMaterial;
class FMaterialInstance;

// This class is NOT thread-safe and designed to be used on a single core thread.
//
// It internally manages the actual allocator (e.g., mAllocator) without any
// synchronization primitives, and the allocator itself is not thread-safe as well. Concurrent
// access from multiple threads to the same UboManager instance will result in data races and
// undefined behavior.
class UboManager {
public:
    // Tracks completion of submitted frames without storing per-frame allocation lists.
    //
    // This class is designed for single-threaded access.
    class FenceManager {
    public:
        using Serial = uint64_t;

        FenceManager() = default;
        ~FenceManager() = default;

        FenceManager(FenceManager const&) = delete;
        FenceManager(FenceManager&&) = delete;

        // Creates a fence after the current frame's UBO reads.
        void track(backend::DriverApi& driver);

        // Advance the completed serial and destroy fences covered by that completion.
        void reclaimCompletedResources(backend::DriverApi& driver);

        [[nodiscard]] Serial getSubmittedSerial() const noexcept { return mSubmittedSerial; }
        [[nodiscard]] Serial getCompletedSerial() const noexcept { return mCompletedSerial; }

        // Destroys all tracked fences and clears the tracking list.
        // This is used for cleanup during termination or major reallocations.
        void reset(backend::DriverApi& driver);

    private:
        struct FrameFence {
            backend::Handle<backend::HwFence> fence;
            Serial serial;
        };
        std::vector<FrameFence> mFences;
        Serial mSubmittedSerial = 0;
        Serial mCompletedSerial = 0;
    };

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
    // Note that it must happen before committing all MIs.
    void beginFrame(backend::DriverApi& driver);

    // Unmap the buffer here
    void finishBeginFrame(backend::DriverApi& driver);

    // Fence the current frame without visiting its material instances.
    void endFrame(backend::DriverApi& driver);

    void terminate(backend::DriverApi& driver);

    void updateSlot(backend::DriverApi& driver, BufferAllocator::AllocationId id,
            backend::BufferDescriptor bufferDescriptor) const;

    // Call this to register a new material instance to UboManager.
    void manageMaterialInstance(FMaterialInstance* instance);

    // Call this when a material instance is destroyed.
    void unmanageMaterialInstance(FMaterialInstance* materialInstance);

    // Returns the size of the actual UBO. Note that when there's allocation failed, it will be
    // reallocated to a bigger size at the next frame.
    [[nodiscard]] BufferAllocator::allocation_size_t getTotalSize() const noexcept;

    // For testing
    [[nodiscard]] backend::MemoryMappedBufferHandle getMemoryMappedBufferHandle() const noexcept {
        return mMemoryMappedBufferHandle;
    }

private:
    friend class ::UboManagerTest;

    constexpr static float BUFFER_SIZE_GROWTH_MULTIPLIER = 1.5f;

    enum AllocationResult {
        SUCCESS,
        REALLOCATION_REQUIRED
    };

    // Query the offset by the allocation id.
    [[nodiscard]] BufferAllocator::allocation_size_t getAllocationOffset(
            BufferAllocator::AllocationId id) const;

    AllocationResult allocateOnDemand();

    void allocateAllInstances();

    // Keep the allocator's ownership until the last submitted frame has completed.
    void deferRetirement(BufferAllocator::AllocationId id);

    void reallocate(backend::DriverApi& driver, BufferAllocator::allocation_size_t requiredSize);

    BufferAllocator::allocation_size_t calculateRequiredSize();

    backend::Handle<backend::HwBufferObject> mUbHandle;
    backend::MemoryMappedBufferHandle mMemoryMappedBufferHandle;
    BufferAllocator::allocation_size_t mUboSize{};
    std::vector<FMaterialInstance*> mPendingInstances;
    std::vector<FMaterialInstance*> mManagedInstances;

    FenceManager mFenceManager;
    BufferAllocator mAllocator;
    struct RetiredAllocation {
        BufferAllocator::AllocationId id;
        FenceManager::Serial serial;
    };
    // Appended in submission order; only retired allocations need GPU lifetime tracking.
    std::vector<RetiredAllocation> mRetiredAllocations;
    // Instances destroyed during a frame must wait for that frame's endFrame fence.
    std::vector<BufferAllocator::AllocationId> mFreedAllocations;
};

} // namespace filament

#endif
