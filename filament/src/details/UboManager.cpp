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

#include <utils/Logger.h>

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
    // Check finished frames and decrement GPU count accordingly.
    checkFenceAndUnlockSlots(driver);

    // Actually merge the slots.
    mAllocator.releaseFreeSlots();

    // Traverse all MIs and see which of them need slot allocation.
    AllocationResult allocationResult =
            updateMaterialInstanceAllocations(materialInstances, ON_DEMAND);

    if (allocationResult == SUCCESS) {
        // No need to grow the buffer, so we can just map the buffer for writing and return.
        mMemoryMappedBufferHandle = driver.mapBuffer(mUbHandle, 0, mUboSize, MapBufferAccessFlags::WRITE_BIT,
                "UboManager");

        return;
    }

    // Calculate the required size and grow the Ubo.
    const allocation_size_t requiredSize = calculateRequiredSize(materialInstances);
    reallocate(driver, requiredSize);

    // Allocate slots for each MI on the new Ubo.
    UTILS_UNUSED_IN_RELEASE const AllocationResult needReallocationAgain =
            updateMaterialInstanceAllocations(materialInstances, ALWAYS);
    assert_invariant(needReallocationAgain != REALLOCATION_REQUIRED);

    // Map the buffer so that we can write to it
    mMemoryMappedBufferHandle =
            driver.mapBuffer(mUbHandle, 0, mUboSize, MapBufferAccessFlags::WRITE_BIT, "UboManager");

    // Migrate all MI data to the new allocated slots.
    for (const auto& materialInstance : materialInstances) {
        materialInstance.second.forEach([this, &driver](const FMaterialInstance* mi) {
            if (!mi->isUsingUboBatching()) {
                return;
            }

            const AllocationId allocationId = mi->getAllocationId();
            assert_invariant(BufferAllocator::isValid(allocationId));
            updateSlot(driver, allocationId, mi->getUniformBuffer().toBufferDescriptor(driver));
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

    mFenceAllocationList.emplace_back( driver.createFence(), std::move(allocationIds) );
}

void UboManager::terminate(DriverApi& driver) {
    for (auto& [fence, _]: mFenceAllocationList) {
        if (fence) {
            driver.destroyFence(std::move(fence));
        }
    }
    mFenceAllocationList.clear();

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

allocation_size_t UboManager::getTotalSize() const noexcept {
    return mUboSize;
}

allocation_size_t UboManager::getAllocationOffset(AllocationId id) const {
    return mAllocator.getAllocationOffset(id);
}

void UboManager::checkFenceAndUnlockSlots(DriverApi& driver) {
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
                LOG(WARNING) << "A fence is either an error or hasn't signaled, but the new fence has "
                          "been "
                          "signaled. Will release the resource anyways."
                       << io::endl;
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

    // Release GPU locks for all completed fences.
    for (auto it = mFenceAllocationList.begin(); it != firstToKeep; ++it) {
        for (const AllocationId& id: it->second) {
            mAllocator.releaseGpu(id);
        }
        // Destroy the fence handle as it's no longer needed.
        driver.destroyFence(std::move(it->first));
    }

    mFenceAllocationList.erase(mFenceAllocationList.begin(), firstToKeep);
}

UboManager::AllocationResult UboManager::updateMaterialInstanceAllocations(
        const std::unordered_map<const FMaterial*, ResourceList<FMaterialInstance>>&
                materialInstances,
        AllocationMode allocationMode) {
    AllocationResult result = SUCCESS;
    for (const auto& materialInstance: materialInstances) {
        materialInstance.second.forEach([this, &result, allocationMode](FMaterialInstance* mi) {
            if (tryAllocateMaterialInstanceSlot(mi, mAllocator, mUbHandle, allocationMode) ==
                    REALLOCATION_REQUIRED) {
                result = REALLOCATION_REQUIRED;
            }
        });
    }

    return result;
}

UboManager::AllocationResult UboManager::tryAllocateMaterialInstanceSlot(FMaterialInstance* mi,
        BufferAllocator& allocator, const Handle<HwBufferObject>& ubHandle,
        AllocationMode allocationMode) {
    if (!mi->isUsingUboBatching()) {
        return SUCCESS;
    }

    const AllocationId id = mi->getAllocationId();
    auto allocateAndAssign = [&](AllocationId originalId) -> AllocationResult {
        auto [newId, newOffset] = allocator.allocate(mi->getUniformBuffer().getSize());
        // Special handling for instances that were previously UNALLOCATED:
        // If the new allocation also fails, keep it UNALLOCATED to signal initial failure.
        if (originalId == BufferAllocator::UNALLOCATED &&
                newId == BufferAllocator::REALLOCATION_REQUIRED) {
            newId = BufferAllocator::UNALLOCATED;
        }

        // Even if the allocation is not valid, we need to set it to let the following
        // process knows.
        mi->assignUboAllocation(ubHandle, newId, newOffset);

        if (!BufferAllocator::isValid(newId)) {
            return REALLOCATION_REQUIRED;
        }
        return SUCCESS;
    };

    if (!BufferAllocator::isValid(id) || allocationMode == ALWAYS) {
        // The material instance is first time being allocated (if !isValid)
        // or is being forcibly re-allocated (if ALWAYS).
        return allocateAndAssign(id);
    }

    if (mi->getUniformBuffer().isDirty() && allocator.isLockedByGpu(id)) {
        // If the uniform buffer is updated and the slot is still being locked by GPU,
        // we will need to allocate a new slot to write the updated content.
        // This is known as "orphaning".
        allocator.retire(id);
        return allocateAndAssign(id);
    }
    // Else, the slot is valid and no need to allocate a new slot.

    return SUCCESS;
}

void UboManager::reallocate(DriverApi& driver, allocation_size_t requiredSize) {
    if (mUbHandle) {
        driver.destroyBufferObject(mUbHandle);
    }

    for (auto& [fence, allocations]: mFenceAllocationList) {
        driver.destroyFence(std::move(fence));
    }
    mFenceAllocationList.clear();

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
