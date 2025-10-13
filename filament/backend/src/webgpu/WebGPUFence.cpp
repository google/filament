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

#include "WebGPUFence.h"

#include <chrono>

namespace filament::backend {

WebGPUFence::WebGPUFence() = default;
WebGPUFence::~WebGPUFence() = default;

void WebGPUFence::setSubmissionState(std::shared_ptr<WebGPUSubmissionState> state) {
    std::lock_guard<std::mutex> const lock(mLock);
    mState = state;
    mCond.notify_one();
}

FenceStatus WebGPUFence::getStatus() {
    return wait(0);
}

FenceStatus WebGPUFence::wait(uint64_t timeout) {
    // we have to take into account that the STL's wait_for() actually works with
    // time_points relative to steady_clock::now() internally.
    using namespace std::chrono;
    auto now = steady_clock::now();
    steady_clock::time_point until = steady_clock::time_point::max();
    if (now <= steady_clock::time_point::max() - nanoseconds(timeout)) {
        until = now + nanoseconds(timeout);
    }
    {
        std::unique_lock<std::mutex> lock(mLock);
        bool const success = mCond.wait_until(lock, until, [this] {
            return bool(mState);
        });
        if (!success) {
            return FenceStatus::TIMEOUT_EXPIRED;
        }
    }
    auto duration_ns = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(steady_clock::now() - now)
                    .count());
    // In the unlikely event that duration_ns is too close to timeout, we just assume the leftover
    // timeout is 0.
    if (timeout == 0 || duration_ns > timeout) {
        duration_ns = timeout;
    }
    return mState->waitForCompletion(timeout - duration_ns);
}

} // namespace filament::backend
