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

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/debug.h>

#include <algorithm>

#include <stddef.h>

namespace filament {

using namespace utils;
using namespace backend;

FrameSkipper::FrameSkipper(size_t latency) noexcept
        : mLast(std::clamp(latency, size_t(1), MAX_FRAME_LATENCY) - 1) {
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
    if (fences.front()) {
        // Do we have a latency old fence?
        auto status = driver.getFenceStatus(fences.front());
        if (UTILS_UNLIKELY(status == FenceStatus::TIMEOUT_EXPIRED)) {
            // The fence hasn't signaled yet, skip this frame
            return false;
        }
        assert_invariant(status == FenceStatus::CONDITION_SATISFIED);
    }
    return true;
}

void FrameSkipper::endFrame(DriverApi& driver) noexcept {
    auto& fences = mDelayedFences;
    size_t const last = mLast;

    // pop the oldest fence and advance the other ones
    if (fences.front()) {
        driver.destroyFence(fences.front());
    }
    std::move(fences.begin() + 1, fences.end(), fences.begin());

    // add a new fence to the end
    assert_invariant(!fences[last]);

    fences[last] = driver.createFence();
}

} // namespace filament
