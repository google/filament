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

#include <utils/Log.h>

namespace filament {

namespace {
using namespace utils;
using namespace backend;

using AllocationId = BufferAllocator::AllocationId;
using allocation_size_t = BufferAllocator::allocation_size_t;

// TODO: Remove this after we can get it from config/backend
constexpr allocation_size_t DEFAULT_SLOT_SIZE_IN_BYTES = 256;
constexpr allocation_size_t DEFAULT_TOTAL_SIZE_IN_BYTES = 256 * 20;
} // anonymous namespace

UboManager::UboManager(DriverApi& driver, Engine::Config const& config)
        : mAllocator(DEFAULT_TOTAL_SIZE_IN_BYTES, DEFAULT_SLOT_SIZE_IN_BYTES) {
    reallocate(driver, DEFAULT_TOTAL_SIZE_IN_BYTES);
}

void UboManager::beginFrame(DriverApi& driver,
        const ResourceList<FMaterialInstance>& materialInstances) {
    // TODO: Implement this
}

void UboManager::finishBeginFrame(DriverApi& driver) const {
    // TODO: Implement this
}

void UboManager::endFrame(DriverApi& driver,
        const ResourceList<FMaterialInstance>& materialInstances) {
    BufferAllocator& allocator = mAllocator;
    std::unordered_set<AllocationId> allocationIds;
    materialInstances.forEach([&allocator, &allocationIds](FMaterialInstance* mi) {
        const AllocationId id = mi->getAllocationId();
        allocator.releaseGpu(id);
        allocationIds.insert(id);
    });

    mFenceAllocationList.push_back({ driver.createFence(), std::move(allocationIds) });
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

std::pair<AllocationId, allocation_size_t> UboManager::allocate(uint32_t required_size) {
    auto [id, offset] = mAllocator.allocate(required_size);
    if (id == BufferAllocator::REALLOCATION_REQUIRED) {
        mNeedReallocate = true;
    }

    return { id, offset };
}

void UboManager::retire(AllocationId id) {
    mAllocator.retire(id);
}

void UboManager::acquireGpu(AllocationId id) {
    mAllocator.acquireGpu(id);
}

void UboManager::releaseGpu(AllocationId id) {
    mAllocator.releaseGpu(id);
}

allocation_size_t UboManager::getTotalSize() const noexcept {
    return mUboSize;
}

allocation_size_t UboManager::getAllocationOffset(AllocationId id) const {
    return mAllocator.getAllocationOffset(id);
}

bool UboManager::isLockedByGpu(BufferAllocator::AllocationId id) const {
    return mAllocator.isLockedByGpu(id);
}

void UboManager::updateSlot(DriverApi& driver, BufferAllocator::AllocationId id,
        BufferDescriptor bufferDescriptor) const {
    const auto offset = mAllocator.getAllocationOffset(id);
    driver.copyToMemoryMappedBuffer(mMmbHandle, offset, std::move(bufferDescriptor));
}

void UboManager::checkFenceAndUnlockSlots(DriverApi& driver) {
    uint32_t signaledCount = 0;
    bool seenSignaledFence = false;

    // Iterate from the newest fence to the oldest.
    for (auto it = mFenceAllocationList.rbegin(); it != mFenceAllocationList.rend(); ++it) {
        const auto& fence = it->first;
        const auto status = driver.getFenceStatus(fence);

        // If we have already seen a signaled fence, we can assume all older fences
        // are also complete, regardless of their reported status (e.g., TIMEOUT_EXPIRED).
        // This is guaranteed by the in-order execution of GPU command queues.
        if (seenSignaledFence) {
            signaledCount++;
#ifndef NDEBUG
            if (UTILS_UNLIKELY(status != FenceStatus::CONDITION_SATISFIED)) {
                slog.w << "A fence is either an error or hasn't signaled, but the new fence has "
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
        for (const auto& id: it->second) {
            mAllocator.releaseGpu(id);
        }
        // Destroy the fence handle as it's no longer needed.
        driver.destroyFence(std::move(it->first));
    }

    mFenceAllocationList.erase(mFenceAllocationList.begin(), firstToKeep);

    // Try to merge the unlocked slots.
    mAllocator.releaseFreeSlots();
}

bool UboManager::updateMaterialInstanceAllocations(
        const ResourceList<FMaterialInstance>& materialInstances) {
    mNeedReallocate = false;

    BufferAllocator& allocator = mAllocator;
    bool& needReallocate = mNeedReallocate;
    Handle<HwBufferObject>& ubHandle = mUbHandle;

    materialInstances.forEach([&allocator, &needReallocate, &ubHandle](FMaterialInstance* mi) {
        const AllocationId id = mi->getAllocationId();
        assert_invariant(id != BufferAllocator::REALLOCATION_REQUIRED);
        if (id == BufferAllocator::UNALLOCATED) {
            // The material instance is first time being allocated.
            auto [newId, newOffset] = allocator.allocate(mi->getUniformBuffer().getSize());
            // Even if the allocation is not valid, we need to set it to let the following process
            // knows.
            mi->assignUboAllocation(ubHandle, newId, newOffset);

            if (newId == BufferAllocator::REALLOCATION_REQUIRED) {
                needReallocate = true;
            }
        } else if (mi->getUniformBuffer().isDirty() && allocator.isLockedByGpu(id)) {
            // If the uniform buffer is updated and the slot is still being locked by GPU,
            // we will need to allocate a new slot to write the updated content.
            allocator.retire(id);
            auto [newId, newOffset] = allocator.allocate(mi->getUniformBuffer().getSize());
            // Even if the allocation is not valid, we need to set it to let the following process
            // knows.
            mi->assignUboAllocation(ubHandle, newId, newOffset);

            if (newId == BufferAllocator::REALLOCATION_REQUIRED) {
                needReallocate = true;
            }
        } // Else, the slot is valid and no need to allocate a new slot.
    });

    return mNeedReallocate;
}

void UboManager::reallocate(DriverApi& driver, allocation_size_t requiredSize) {
    if (mUbHandle) {
        driver.destroyBufferObject(mUbHandle);
    }
    mAllocator.reset(requiredSize);
    mUboSize = requiredSize;
    mUbHandle = driver.createBufferObject(requiredSize, BufferObjectBinding::UNIFORM,
            BufferUsage::DYNAMIC);
}

allocation_size_t UboManager::calculateRequiredSize(
        const ResourceList<FMaterialInstance>& materialInstances) {
    BufferAllocator& allocator = mAllocator;
    allocation_size_t newBufferSize = mUboSize;
    materialInstances.forEach([&newBufferSize, &allocator](FMaterialInstance* mi) {
        auto allocationId = mi->getAllocationId();
        if (allocationId == BufferAllocator::REALLOCATION_REQUIRED) {
            // For MIs whose parameters have been updated, aside from the slot it is being
            // occupied by the GPU, we need to preserve an additional slot for it.
            newBufferSize += 2 * allocator.alignUp(mi->getUniformBuffer().getSize());
        } else {
            newBufferSize += allocator.alignUp(mi->getUniformBuffer().getSize());
        }
    });
    return newBufferSize * BUFFER_SIZE_GROWTH_MULTIPLIER;
}

} // namespace filament
