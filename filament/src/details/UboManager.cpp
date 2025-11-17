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

#include "details/UboManager.h"

#include "MaterialInstance.h"
#include "details/BufferAllocator.h"

#include <backend/DriverEnums.h>
#include <private/utils/Tracing.h>
#include <utils/Logger.h>

#include <vector>

namespace filament {

namespace {
using namespace utils;
using namespace backend;

using AllocationId = BufferAllocator::AllocationId;
using allocation_size_t = BufferAllocator::allocation_size_t;
} // anonymous namespace

// ------------------------------------------------------------------------------------------------
// FenceManager
// ------------------------------------------------------------------------------------------------

void UboManager::FenceManager::track(DriverApi& driver, std::unordered_set<AllocationId>&& allocationIds) {
    if (allocationIds.empty()) {
        return;
    }
    mFenceAllocationList.emplace_back(driver.createFence(), std::move(allocationIds));
}

void UboManager::FenceManager::reclaimCompletedResources(DriverApi& driver,
        std::function<void(AllocationId)> const& onReclaimed) {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    uint32_t signaledCount = 0;
    bool seenSignaledFence = false;

    // Iterate from the newest fence to the oldest.
    for (auto it = mFenceAllocationList.rbegin(); it != mFenceAllocationList.rend(); ++it) {
        const Handle<HwFence>& fence = it->first;
        const FenceStatus status = driver.getFenceStatus(fence);

        // If we have already seen a signaled fence, we can assume all older fences
        // are also complete, regardless of their reported status (e.g., TIMEOUT_EXPIRED).
        // This is guaranteed by the in-order execution of GPU command queues.
        if (seenSignaledFence) {
            signaledCount++;
#ifndef NDEBUG
            if (UTILS_UNLIKELY(status != FenceStatus::CONDITION_SATISFIED)) {
                LOG(WARNING) << "A fence is either in an error state or hasn't signaled, but a newer "
                                 "fence has. Will release the resource anyway.";
            }
#endif
            continue;
        }

        if (status == FenceStatus::CONDITION_SATISFIED) {
            seenSignaledFence = true;
            signaledCount++;
        }
    }

    if (signaledCount == 0) {
        // No fences have completed, nothing to do.
        return;
    }

    auto firstToKeep = mFenceAllocationList.begin() + signaledCount;

    // Invoke the callback for all resources protected by completed fences.
    for (auto it = mFenceAllocationList.begin(); it != firstToKeep; ++it) {
        for (const AllocationId& id : it->second) {
            onReclaimed(id);
        }
        // Destroy the fence handle as it's no longer needed.
        driver.destroyFence(std::move(it->first));
    }

    mFenceAllocationList.erase(mFenceAllocationList.begin(), firstToKeep);
}

void UboManager::FenceManager::reset(DriverApi& driver) {
    for (auto& [fence, _] : mFenceAllocationList) {
        if (fence) {
            driver.destroyFence(std::move(fence));
        }
    }
    mFenceAllocationList.clear();
}


// ------------------------------------------------------------------------------------------------
// UboManager
// ------------------------------------------------------------------------------------------------

UboManager::UboManager(DriverApi& driver, allocation_size_t defaultSlotSizeInBytes,
        allocation_size_t defaultTotalSizeInBytes)
        : mAllocator(defaultTotalSizeInBytes, defaultSlotSizeInBytes) {
    reallocate(driver, defaultTotalSizeInBytes);
}

void UboManager::beginFrame(DriverApi& driver) {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    // Check finished frames and decrement GPU count accordingly.
    mFenceManager.reclaimCompletedResources(driver,
            [this](AllocationId id) { mAllocator.releaseGpu(id); });

    // Actually merge the slots.
    mAllocator.releaseFreeSlots();

    // Traverse all MIs and see which of them need slot allocation.
    if (allocateOnDemand() == SUCCESS) {
        // No need to grow the buffer, so we can just map the buffer for writing and return.
        mMemoryMappedBufferHandle = driver.mapBuffer(mUbHandle, 0, mUboSize, MapBufferAccessFlags::WRITE_BIT,
                "UboManager");

        return;
    }

    // Calculate the required size and grow the Ubo.
    const allocation_size_t requiredSize = calculateRequiredSize();
    reallocate(driver, requiredSize);

    // Allocate slots for each MI on the new Ubo.
    allocateAllInstances();

    // Map the buffer so that we can write to it
    mMemoryMappedBufferHandle =
            driver.mapBuffer(mUbHandle, 0, mUboSize, MapBufferAccessFlags::WRITE_BIT, "UboManager");

    // Invalidate the migrated MIs, so that next commit() call must be triggered.
    for (const auto& mi : mManagedInstances) {
        mi->getUniformBuffer().invalidate();
    }
}

void UboManager::finishBeginFrame(DriverApi& driver) {
    if (mMemoryMappedBufferHandle) {
        driver.unmapBuffer(mMemoryMappedBufferHandle);
        mMemoryMappedBufferHandle.clear();
    }
}

void UboManager::endFrame(DriverApi& driver) {
    std::unordered_set<AllocationId> allocationIds;
    for (const auto& mi : mManagedInstances) {
        const AllocationId id = mi->getAllocationId();
        if (!BufferAllocator::isValid(id)) {
            return;
        }

        mAllocator.acquireGpu(id);
        allocationIds.insert(id);
    }

    mFenceManager.track(driver, std::move(allocationIds));
}

void UboManager::terminate(DriverApi& driver) {
    mFenceManager.reset(driver);
    driver.destroyBufferObject(mUbHandle);
}

void UboManager::updateSlot(DriverApi& driver, AllocationId id,
        BufferDescriptor bufferDescriptor) const {
    if (!mMemoryMappedBufferHandle)
        return;

    const allocation_size_t offset = mAllocator.getAllocationOffset(id);
    driver.copyToMemoryMappedBuffer(mMemoryMappedBufferHandle, offset, std::move(bufferDescriptor));
}

void UboManager::manageMaterialInstance(FMaterialInstance* instance) {
    mPendingInstances.insert(instance);
}

void UboManager::unmanageMaterialInstance(const FMaterialInstance* materialInstance) {
    AllocationId id = materialInstance->getAllocationId();
    if (!BufferAllocator::isValid(id))
        return;

    // const_cast should be safe here since this cast is just to match the container type.
    auto mi = const_cast<FMaterialInstance*>(materialInstance);
    if (UTILS_UNLIKELY(mPendingInstances.contains(mi))) {
        mPendingInstances.erase(mi);
    }

    if (UTILS_LIKELY(mManagedInstances.contains(mi))) {
        mManagedInstances.erase(mi);
    }
    mAllocator.retire(id);
}

UboManager::AllocationResult UboManager::allocateOnDemand() {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    bool reallocationNeeded = false;

    // Pass 1: Allocate slots for new material instances (that don't have a slot yet).
    for (FMaterialInstance* mi : mPendingInstances) {
        mManagedInstances.insert(mi);
        auto [newId, newOffset] = mAllocator.allocate(mi->getUniformBuffer().getSize());
        mi->assignUboAllocation(mUbHandle, newId, newOffset);
        if (!BufferAllocator::isValid(newId)) {
            reallocationNeeded = true;
        }
    }
    mPendingInstances.clear();

    // Pass 2: Allocate slots for existing material instances that need to be orphaned.
    for (auto& mi: mManagedInstances) {
        if (!BufferAllocator::isValid(mi->getAllocationId())) {
            continue;
        }

        // This instance doesn't need orphaning.
        if (!mi->getUniformBuffer().isDirty() || !mAllocator.isLockedByGpu(mi->getAllocationId())) {
            continue;
        }

        mAllocator.retire(mi->getAllocationId());

        // If the space is already not sufficient, we don't need to give another try on allocation.
        if (reallocationNeeded) {
            mi->assignUboAllocation(mUbHandle, REALLOCATION_REQUIRED, 0);
            continue;
        }

        auto [newId, newOffset] = mAllocator.allocate(mi->getUniformBuffer().getSize());
        mi->assignUboAllocation(mUbHandle, newId, newOffset);
        if (!BufferAllocator::isValid(newId)) {
            reallocationNeeded = true;
        }
    }

    return reallocationNeeded ? REALLOCATION_REQUIRED : SUCCESS;
}

void UboManager::allocateAllInstances() {
    for (const auto& mi: mManagedInstances) {
        auto [newId, newOffset] = mAllocator.allocate(mi->getUniformBuffer().getSize());
        assert_invariant(BufferAllocator::isValid(newId));
        mi->assignUboAllocation(mUbHandle, newId, newOffset);
    }
}

allocation_size_t UboManager::getTotalSize() const noexcept {
    return mUboSize;
}

allocation_size_t UboManager::getAllocationOffset(AllocationId id) const {
    return mAllocator.getAllocationOffset(id);
}

void UboManager::reallocate(DriverApi& driver, allocation_size_t requiredSize) {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    if (mUbHandle) {
        driver.destroyBufferObject(mUbHandle);
    }

    mFenceManager.reset(driver);
    mAllocator.reset(requiredSize);
    mUboSize = requiredSize;
    mUbHandle = driver.createBufferObject(requiredSize, BufferObjectBinding::UNIFORM,
            BufferUsage::DYNAMIC | BufferUsage::SHARED_WRITE_BIT);
}

allocation_size_t UboManager::calculateRequiredSize() {
    allocation_size_t newBufferSize = 0;
    for (const auto& mi: mManagedInstances) {
        const AllocationId allocationId = mi->getAllocationId();
        if (allocationId == BufferAllocator::REALLOCATION_REQUIRED) {
            // For MIs whose parameters have been updated, aside from the slot it is being
            // occupied by the GPU, we need to preserve an additional slot for it.
            newBufferSize += 2 * mAllocator.alignUp(mi->getUniformBuffer().getSize());
        } else {
            newBufferSize += mAllocator.alignUp(mi->getUniformBuffer().getSize());
        }
    }
    return mAllocator.alignUp(newBufferSize * BUFFER_SIZE_GROWTH_MULTIPLIER);
}

} // namespace filament
