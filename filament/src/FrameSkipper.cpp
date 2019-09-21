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

#include "details/FrameSkipper.h"
#include "details/Engine.h"

namespace filament {
namespace details {

FrameSkipper::FrameSkipper(FEngine& engine, size_t latency) noexcept
        : mEngine(engine), mLast((uint32_t)std::max(latency, size_t(1)) - 1) {
    assert(latency <= MAX_FRAME_LATENCY);
}

FrameSkipper::~FrameSkipper() noexcept {
    for (FFence* fence : mDelayedFences) {
        if (fence) {
            mEngine.destroy(fence);
        }
    }
}

bool FrameSkipper::beginFrame() noexcept {
    auto& fences = mDelayedFences;
    FFence* const fence = fences.front();
    if (fence) {
        auto status = fence->wait(Fence::Mode::DONT_FLUSH, 0);
        if (status == Fence::FenceStatus::TIMEOUT_EXPIRED) {
            // fence not ready, skip frame
            return false;
        }
        mEngine.destroy(fence);
    }
    // shift all fences down by 1
    std::move(fences.begin() + 1, fences.end(), fences.begin());
    fences.back() = nullptr;
    return true;
}

void FrameSkipper::endFrame() noexcept {
    mDelayedFences[mLast] = mEngine.createFence(FFence::Type::HARD);
}


} // namespace details
} // namespace filament
