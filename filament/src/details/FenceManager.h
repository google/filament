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

#ifndef TNT_FILAMENT_DETAILS_FENCEMANAGER_H
#define TNT_FILAMENT_DETAILS_FENCEMANAGER_H

#include "details/BufferAllocator.h"

#include <private/backend/DriverApi.h>
#include <backend/Handle.h>

#include <functional>
#include <unordered_set>
#include <vector>

namespace filament {

// This utility tracks resources that are in-use by the GPU across multiple frames.
// It uses backend fences to determine when the GPU has finished with a set of resources,
// allowing them to be safely reclaimed or reused.
//
// The typical usage is to `track()` a set of resources at the end of a frame and
// call `reclaimCompletedResources()` at the beginning of a future frame to free up
// resources from completed GPU work.
//
// This class is designed for single-threaded access.
class FenceManager {
public:
    using AllocationId = BufferAllocator::AllocationId;

    FenceManager() = default;
    ~FenceManager() = default;

    FenceManager(FenceManager const&) = delete;
    FenceManager(FenceManager&&) = delete;


    // Creates a new fence to track a set of allocation IDs for the current frame.
    // This marks the beginning of GPU's usage of these resources.
    void track(backend::DriverApi& driver, std::unordered_set<AllocationId>&& allocationIds);


    // Checks all tracked fences and invokes a callback for resources associated with
    // completed fences. This should be called once per frame.
    void reclaimCompletedResources(backend::DriverApi& driver,
            std::function<void(AllocationId)> const& onReclaimed);

    // Destroys all tracked fences and clears the tracking list.
    // This is used for cleanup during termination or major reallocations.
    void reset(backend::DriverApi& driver);

private:
    // Not ideal, but we need to know which slots to decrement gpuUseCount for each frame.
    using FenceAndAllocations =
            std::pair<backend::Handle<backend::HwFence>, std::unordered_set<AllocationId>>;
    std::vector<FenceAndAllocations> mFenceAllocationList;
};

} // namespace filament

#endif //TNT_FILAMENT_DETAILS_FENCEMANAGER_H
