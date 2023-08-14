/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "FrameSkipper.h"

#include <utils/Log.h>
#include <utils/debug.h>

namespace filament {

using namespace utils;
using namespace backend;

FrameSkipper::FrameSkipper(size_t latency) noexcept
        : mLast(latency - 1) {
    assert_invariant(latency <= MAX_FRAME_LATENCY);
}

FrameSkipper::~FrameSkipper() noexcept = default;

void FrameSkipper::terminate(DriverApi& driver) noexcept {
    for (auto fence : mDelayedFences) {
        if (fence) {
            driver.destroyFence(fence);
        }
    }
}

bool FrameSkipper::beginFrame(DriverApi& driver) noexcept {
    auto& fences = mDelayedFences;
    auto fence = fences.front();
    if (fence) {
        auto status = driver.getFenceStatus(fence);
        if (status == FenceStatus::TIMEOUT_EXPIRED) {
            // Sync not ready, skip frame
            return false;
        }
        assert_invariant(status == FenceStatus::CONDITION_SATISFIED);
        driver.destroyFence(fence);
    }
    // shift all fences down by 1
    std::move(fences.begin() + 1, fences.end(), fences.begin());
    fences.back() = {};
    return true;
}

void FrameSkipper::endFrame(DriverApi& driver) noexcept {
    // If the user produced a new frame despite the fact that the previous one wasn't finished
    // (i.e. FrameSkipper::beginFrame() returned false), we need to make sure to replace
    // a fence that might be here already)
    auto& fence = mDelayedFences[mLast];
    if (fence) {
        driver.destroyFence(fence);
    }
    fence = driver.createFence();
}

} // namespace filament
