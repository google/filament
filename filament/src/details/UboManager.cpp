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
#include "details/FenceManager.h"

#include <backend/DriverEnums.h>
#include <private/utils/Tracing.h>

#include <vector>

namespace filament {

namespace {
using namespace utils;
using namespace backend;

using AllocationId = BufferAllocator::AllocationId;
using allocation_size_t = BufferAllocator::allocation_size_t;
} // anonymous namespace

UboManager::UboManager(DriverApi& driver, allocation_size_t defaultSlotSizeInBytes,
        allocation_size_t defaultTotalSizeInBytes)
        : mAllocator(defaultTotalSizeInBytes, defaultSlotSizeInBytes) {
    reallocate(driver, defaultTotalSizeInBytes);
}

void UboManager::beginFrame(DriverApi& driver,
        const std::unordered_map<const FMaterial*, ResourceList<FMaterialInstance>>& materialInstances) {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    // Check finished frames and decrement GPU count accordingly.
    mFenceManager.reclaimCompletedResources(driver,
            [this](AllocationId id) { mAllocator.releaseGpu(id); });

    // Actually merge the slots.
    mAllocator.releaseFreeSlots();

    // Traverse all MIs and see which of them need slot allocation.
    if (allocateOnDemand(materialInstances) == SUCCESS) {
        // No need to grow the buffer, so we can just map the buffer for writing and return.
        mMemoryMappedBufferHandle = driver.mapBuffer(mUbHandle, 0, mUboSize, MapBufferAccessFlags::WRITE_BIT,
                "UboManager");

        return;
    }

    // Calculate the required size and grow the Ubo.
    const allocation_size_t requiredSize = calculateRequiredSize(materialInstances);
    reallocate(driver, requiredSize);

    // Allocate slots for each MI on the new Ubo.
    allocateAllInstances(materialInstances);

    // Map the buffer so that we can write to it
    mMemoryMappedBufferHandle =
            driver.mapBuffer(mUbHandle, 0, mUboSize, MapBufferAccessFlags::WRITE_BIT, "UboManager");

    // Invalidate the migrated MIs, so that next commit() call must be triggered.
    for (const auto& materialInstance : materialInstances) {
        materialInstance.second.forEach([](const FMaterialInstance* mi) {
            if (!mi->isUsingUboBatching()) {
                return;
            }

            mi->getUniformBuffer().invalidate();
        });
    }
}

void UboManager::finishBeginFrame(DriverApi& driver) {
    if (mMemoryMappedBufferHandle) {
        driver.unmapBuffer(mMemoryMappedBufferHandle);
        mMemoryMappedBufferHandle.clear();
    }
}

void UboManager::endFrame(DriverApi& driver,
        const std::unordered_map<const FMaterial*, ResourceList<FMaterialInstance>>& materialInstances) {
    BufferAllocator& allocator = mAllocator;
    std::unordered_set<AllocationId> allocationIds;
    for (const auto& materialInstance : materialInstances) {
        materialInstance.second.forEach([&allocator, &allocationIds](const FMaterialInstance* mi) {
            if (!mi->isUsingUboBatching()) {
                return;
            }

            const AllocationId id = mi->getAllocationId();
            if (!BufferAllocator::isValid(id)) {
                return;
            }

            allocator.acquireGpu(id);
            allocationIds.insert(id);
        });
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

void UboManager::retireSlot(BufferAllocator::AllocationId id) {
    if (!BufferAllocator::isValid(id))
        return;
    mAllocator.retire(id);
}

UboManager::AllocationResult UboManager::allocateOnDemand(
        const std::unordered_map<const FMaterial*, ResourceList<FMaterialInstance>>&
                materialInstances) {
    // Collect all MIs that need allocation into two groups.
    std::vector<FMaterialInstance*> newInstances;
    std::vector<FMaterialInstance*> existingInstances;
    for (const auto& [_, miList] : materialInstances) {
        miList.forEach([&](FMaterialInstance* mi) {
            if (!mi->isUsingUboBatching()) {
                return;
            }
            if (BufferAllocator::isValid(mi->getAllocationId())) {
                existingInstances.push_back(mi);
            } else {
                newInstances.push_back(mi);
            }
        });
    }

    bool reallocationNeeded = false;

    // Pass 1: Allocate slots for new material instances (that don't have a slot yet).
    for (FMaterialInstance* mi : newInstances) {
        auto [newId, newOffset] = mAllocator.allocate(mi->getUniformBuffer().getSize());
        mi->assignUboAllocation(mUbHandle, newId, newOffset);
        if (!BufferAllocator::isValid(newId)) {
            reallocationNeeded = true;
        }
    }

    // Pass 2: Allocate slots for existing material instances that need to be orphaned.
    for (FMaterialInstance* mi : existingInstances) {
        if (mi->getUniformBuffer().isDirty() && mAllocator.isLockedByGpu(mi->getAllocationId())) {
            mAllocator.retire(mi->getAllocationId());
            auto [newId, newOffset] = mAllocator.allocate(mi->getUniformBuffer().getSize());
            mi->assignUboAllocation(mUbHandle, newId, newOffset);
            if (!BufferAllocator::isValid(newId)) {
                reallocationNeeded = true;
            }
        }
    }

    return reallocationNeeded ? REALLOCATION_REQUIRED : SUCCESS;
}

void UboManager::allocateAllInstances(
        const std::unordered_map<const FMaterial*, ResourceList<FMaterialInstance>>&
                materialInstances) {
    for (const auto& [_, miList] : materialInstances) {
        miList.forEach([this](FMaterialInstance* mi) {
            if (!mi->isUsingUboBatching()) {
                return;
            }
            auto [newId, newOffset] = mAllocator.allocate(mi->getUniformBuffer().getSize());
            assert_invariant(BufferAllocator::isValid(newId));
            mi->assignUboAllocation(mUbHandle, newId, newOffset);
        });
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

allocation_size_t UboManager::calculateRequiredSize(
        const std::unordered_map<const FMaterial*, ResourceList<FMaterialInstance>>&
                materialInstances) {
    BufferAllocator& allocator = mAllocator;
    allocation_size_t newBufferSize = 0;
    for (const auto& materialInstance: materialInstances) {
        materialInstance.second.forEach([&newBufferSize, &allocator](const FMaterialInstance* mi) {
            if (!mi->isUsingUboBatching()) {
                return;
            }

            const AllocationId allocationId = mi->getAllocationId();
            if (allocationId == BufferAllocator::REALLOCATION_REQUIRED) {
                // For MIs whose parameters have been updated, aside from the slot it is being
                // occupied by the GPU, we need to preserve an additional slot for it.
                newBufferSize += 2 * allocator.alignUp(mi->getUniformBuffer().getSize());
            } else {
                newBufferSize += allocator.alignUp(mi->getUniformBuffer().getSize());
            }
        });
    }
    return allocator.alignUp(newBufferSize * BUFFER_SIZE_GROWTH_MULTIPLIER);
}

} // namespace filament
