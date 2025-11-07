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

#include "details/FenceManager.h"

#include <utils/Logger.h>

namespace filament {

using namespace backend;

void FenceManager::track(DriverApi& driver, std::unordered_set<AllocationId>&& allocationIds) {
    if (allocationIds.empty()) {
        return;
    }
    mFenceAllocationList.emplace_back(driver.createFence(), std::move(allocationIds));
}

void FenceManager::reclaimCompletedResources(DriverApi& driver,
        std::function<void(AllocationId)> const& onReclaimed) {
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

void FenceManager::reset(DriverApi& driver) {
    for (auto& [fence, _] : mFenceAllocationList) {
        if (fence) {
            driver.destroyFence(std::move(fence));
        }
    }
    mFenceAllocationList.clear();
}

} // namespace filament
