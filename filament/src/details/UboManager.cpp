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

#include <algorithm>
#include <cstddef>
#include <utility>
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

void UboManager::FenceManager::track(DriverApi& driver) {
    mFences.push_back({ driver.createFence(), ++mSubmittedSerial });
}

void UboManager::FenceManager::reclaimCompletedResources(DriverApi& driver) {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    // A completed fence covers every earlier submission on the same command stream.
    for (auto it = mFences.rbegin(); it != mFences.rend(); ++it) {
        // Fences are only created by track() and erased as soon as they're destroyed.
        assert_invariant(it->fence);
        if (driver.getFenceStatus(it->fence) != FenceStatus::CONDITION_SATISFIED) {
            continue;
        }

        mCompletedSerial = it->serial;
        auto firstToKeep = it.base();
        for (auto completed = mFences.begin(); completed != firstToKeep; ++completed) {
            driver.destroyFence(std::move(completed->fence));
        }
        mFences.erase(mFences.begin(), firstToKeep);
        break;
    }
}

void UboManager::FenceManager::reset(DriverApi& driver) {
    for (FrameFence& frame : mFences) {
        assert_invariant(frame.fence);
        driver.destroyFence(std::move(frame.fence));
    }
    mFences.clear();
    mSubmittedSerial = 0;
    mCompletedSerial = 0;
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
    mFenceManager.reclaimCompletedResources(driver);
    // Relies on mRetiredAllocations being sorted by serial (see UboManager.h).
    auto firstToKeep = mRetiredAllocations.begin();
    while (firstToKeep != mRetiredAllocations.end() &&
            mFenceManager.hasCompleted(firstToKeep->serial)) {
        mAllocator.retire(firstToKeep->id);
        ++firstToKeep;
    }
    mRetiredAllocations.erase(mRetiredAllocations.begin(), firstToKeep);

    // endFrame has now fenced any reads issued before these instances were destroyed.
    for (AllocationId id: mFreedAllocations) {
        deferRetirement(id);
    }
    mFreedAllocations.clear();

    // Traverse all MIs and see which of them need slot allocation.
    if (allocateOnDemand() == SUCCESS) {
        // No need to grow the buffer, so we can just map the buffer for writing and return.
        mMemoryMappedBufferHandle = driver.mapBuffer(mUbHandle, 0, mUboSize, MapBufferAccessFlags::WRITE_BIT,
                "UboManager");

        return;
    }

    // Calculate the required size and grow the Ubo.
    const allocation_size_t requiredSize = calculateRequiredSize(driver.getMaxUniformBufferSize());
    reallocate(driver, requiredSize);

    // Allocate slots for each MI on the new Ubo.
    allocateAllInstances();

    // Map the buffer so that we can write to it
    mMemoryMappedBufferHandle =
            driver.mapBuffer(mUbHandle, 0, mUboSize, MapBufferAccessFlags::WRITE_BIT, "UboManager");

    // Invalidate the migrated MIs, so that next commit() call must be triggered.
    for (const auto* mi : mManagedInstances) {
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
    // Every allocated slot is owned by exactly one of:
    //  - a managed instance (may be read by this frame),
    //  - mFreedAllocations (released this frame, may have been read by this frame),
    //  - mRetiredAllocations (last read by a frame whose fence is already tracked).
    // Pending instances own no slot. So if the first two are empty, nothing issued since the
    // last fence reads the UBO and there is nothing new to fence.
    assert_invariant(isAllocationAccountingConsistent());
    if (!mManagedInstances.empty() || !mFreedAllocations.empty()) {
        mFenceManager.track(driver);
    }
}

// Must only be called for allocations whose last GPU use was already covered by a fence
// (i.e. submitted before the last endFrame()). Otherwise the stamped serial is too early and
// the slot may be reused while still in flight. Defer via mFreedAllocations in that case.
void UboManager::deferRetirement(AllocationId id) {
    if (!mFenceManager.isPending()) {
        mAllocator.retire(id);
    } else {
        const FenceManager::Serial serial = mFenceManager.getSubmittedSerial();
        // beginFrame() retires a prefix of mRetiredAllocations, so it must stay sorted.
        assert_invariant(mRetiredAllocations.empty() ||
                mRetiredAllocations.back().serial <= serial);
        mRetiredAllocations.push_back({ id, serial });
    }
}

#ifndef NDEBUG
bool UboManager::isAllocationAccountingConsistent() const noexcept {
    const size_t managedCount = std::count_if(mManagedInstances.begin(), mManagedInstances.end(),
            [](const FMaterialInstance* mi) {
                return BufferAllocator::isValid(mi->getAllocationId());
            });
    return mAllocator.getAllocationCount() ==
            managedCount + mFreedAllocations.size() + mRetiredAllocations.size();
}
#endif

void UboManager::terminate(DriverApi& driver) {
    mFenceManager.reset(driver);
    driver.destroyBufferObject(mUbHandle);
}

void UboManager::updateSlot(DriverApi& driver, AllocationId id,
        BufferDescriptor bufferDescriptor) const {
    if (!mMemoryMappedBufferHandle) {
        return;
    }

    const allocation_size_t offset = mAllocator.getAllocationOffset(id);
    driver.copyToMemoryMappedBuffer(mMemoryMappedBufferHandle, offset, std::move(bufferDescriptor));
}

void UboManager::manageMaterialInstance(FMaterialInstance* instance) {
    assert_invariant(std::find(mPendingInstances.begin(), mPendingInstances.end(), instance) ==
            mPendingInstances.end());
    assert_invariant(std::find(mManagedInstances.begin(), mManagedInstances.end(), instance) ==
            mManagedInstances.end());
    mPendingInstances.push_back(instance);
}

void UboManager::unmanageMaterialInstance(FMaterialInstance* materialInstance) {
    AllocationId id = materialInstance->getAllocationId();

    auto itPending =
            std::find(mPendingInstances.begin(), mPendingInstances.end(), materialInstance);
    if (UTILS_UNLIKELY(itPending != mPendingInstances.end())) {
        std::swap(*itPending, mPendingInstances.back());
        mPendingInstances.pop_back();

        // This MI is not even allocated yet. We just return here.
        return;
    }

    auto itManaged =
            std::find(mManagedInstances.begin(), mManagedInstances.end(), materialInstance);
    if (UTILS_UNLIKELY(itManaged == mManagedInstances.end())) {
        // This MI was never managed, so it owns no slot.
        return;
    }
    std::swap(*itManaged, mManagedInstances.back());
    mManagedInstances.pop_back();

    // Outside beginFrame(), every managed instance owns a valid slot (see allocateOnDemand()).
    assert_invariant(BufferAllocator::isValid(id));

    // We push the allocation id back to the list, and defer the actual retirement to beginFrame,
    // so that we centralized all the retirements at the same place.
    mFreedAllocations.push_back(id);
    materialInstance->assignUboAllocation(mUbHandle, BufferAllocator::UNALLOCATED, 0);
}

UboManager::AllocationResult UboManager::allocateOnDemand() {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    bool reallocationNeeded = false;
    const size_t previouslyManagedCount = mManagedInstances.size();

    // Pass 1: Allocate slots for new material instances (that don't have a slot yet).
    for (auto* mi : mPendingInstances) {
        mManagedInstances.push_back(mi);
        auto [newId, newOffset] = mAllocator.allocate(mi->getUniformBuffer().getSize());

        // Even if the newId is not valid, we assign it to the MI so that the following process knows
        // this material instance was not allocated successfully. Then we can calculate the new
        // required UBO size properly.
        mi->assignUboAllocation(mUbHandle, newId, newOffset);

        if (!BufferAllocator::isValid(newId)) {
            reallocationNeeded = true;
        }
    }
    mPendingInstances.clear();

    // Pass 2: Allocate slots for existing material instances that need to be orphaned.
    // Newly allocated instances have never been submitted and must not be orphaned.
    for (size_t i = 0; i < previouslyManagedCount; ++i) {
        FMaterialInstance* mi = mManagedInstances[i];
        // These instances were all allocated by a previous successful beginFrame().
        assert_invariant(BufferAllocator::isValid(mi->getAllocationId()));

        // Every existing allocation is conservatively considered used by the last submission.
        // No per-allocation GPU count is needed, since owned slots cannot be reused anyway.
        // So a dirty instance needs orphaning only while that submission is still pending.
        if (!mi->getUniformBuffer().isDirty() || !mFenceManager.isPending()) {
            continue;
        }

        deferRetirement(mi->getAllocationId());

        // If the space is already not sufficient, we don't need to give another try on allocation.
        if (reallocationNeeded) {
            mi->assignUboAllocation(mUbHandle, REALLOCATION_REQUIRED, 0);
            continue;
        }

        auto [newId, newOffset] = mAllocator.allocate(mi->getUniformBuffer().getSize());

        // Even if the newId is not valid, we assign it to the MI so that the following process knows
        // this material instance was not allocated successfully. Then we can calculate the new
        // required UBO size properly.
        mi->assignUboAllocation(mUbHandle, newId, newOffset);

        if (!BufferAllocator::isValid(newId)) {
            reallocationNeeded = true;
        }
    }

    return reallocationNeeded ? REALLOCATION_REQUIRED : SUCCESS;
}

void UboManager::allocateAllInstances() {
    for (auto* mi: mManagedInstances) {
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
    // Old serials and allocation IDs belong to the discarded UBO generation.
    mRetiredAllocations.clear();
    // beginFrame() drains mFreedAllocations before it can reach reallocation.
    assert_invariant(mFreedAllocations.empty());
    mAllocator.reset(requiredSize);
    mUboSize = requiredSize;
    mUbHandle = driver.createBufferObject(requiredSize, BufferObjectBinding::UNIFORM,
            BufferUsage::DYNAMIC | BufferUsage::SHARED_WRITE_BIT);
}

allocation_size_t UboManager::calculateRequiredSize(size_t maxUniformBufferSize) {
    allocation_size_t newBufferSize = 0;
    for (const auto* mi: mManagedInstances) {
        const AllocationId allocationId = mi->getAllocationId();
        if (allocationId == BufferAllocator::REALLOCATION_REQUIRED) {
            // For MIs whose parameters have been updated, aside from the slot it is being
            // occupied by the GPU, we need to preserve an additional slot for it.
            newBufferSize += 2 * mAllocator.alignUp(mi->getUniformBuffer().getSize());
        } else {
            newBufferSize += mAllocator.alignUp(mi->getUniformBuffer().getSize());
        }
    }

    // TODO: Add more buffers if one is not enough?
    // The device limit may not be a multiple of the slot size, so round it down.
    return std::min(mAllocator.alignDown((allocation_size_t)maxUniformBufferSize),
            mAllocator.alignUp(newBufferSize * BUFFER_SIZE_GROWTH_MULTIPLIER));
}

} // namespace filament
