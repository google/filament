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
#include "WebGPUConstants.h"

#include <backend/DriverEnums.h>

#include <memory>
#include <webgpu/webgpu_cpp.h>

#include <condition_variable>
#include <chrono>
#include <cstdint>
#include <mutex>

namespace filament::backend {

WebGPUFence::~WebGPUFence() noexcept {
    // if the fence was being waited on, make sure to return with an error
    std::lock_guard const lock(state->lock);
    state->status = FenceStatus::ERROR;
    state->cond.notify_all();
}

FenceStatus WebGPUFence::getStatus() const {
    std::lock_guard const lock(state->lock);
    return state->status;
}

FenceStatus WebGPUFence::wait(uint64_t const timeout) {
    // we have to take into account that the STL's wait_for() actually works with
    // time_points relative to steady_clock::now() internally.
    using namespace std::chrono;
    auto const now = steady_clock::now();
    steady_clock::time_point until = steady_clock::time_point::max();
    if (now <= steady_clock::time_point::max() - nanoseconds(timeout)) {
        until = now + nanoseconds(timeout);
    }

    std::unique_lock lock(state->lock);
    state->cond.wait_until(lock, until,
            [state = state] { return state->status != FenceStatus::TIMEOUT_EXPIRED; });
    return state->status;
}

void WebGPUFence::addMarkerToQueueState(wgpu::Queue const& queue) {
    // The lambda function is called when the work is done. It updates the fence status based on the
    // result of the work.
    // Because the callback holds a weak_ptr to the state of the HwFence object, it can be
    // destroyed before the fence signals.
    std::weak_ptr const weak = state;
    queue.OnSubmittedWorkDone(
            wgpu::CallbackMode::AllowSpontaneous,
            [weak](const wgpu::QueueWorkDoneStatus status, wgpu::StringView message) {
                if (auto const state = weak.lock()) {
                    std::unique_lock const lock(state->lock);
                    if (state->status != FenceStatus::ERROR) {
                        switch (status) {
                            case wgpu::QueueWorkDoneStatus::Success:
                                state->status = FenceStatus::CONDITION_SATISFIED;
                                state->cond.notify_all();
                                break;
                            case wgpu::QueueWorkDoneStatus::CallbackCancelled:
                            case wgpu::QueueWorkDoneStatus::Error:
                                state->status = FenceStatus::ERROR;
                                state->cond.notify_all();
                                FWGPU_LOGW << "WebGPUFence: wgpu::QueueWorkDoneStatus::Error. " <<
                                        message;
                                break;
                        }
                    }
                }
            });
}

} // namespace filament::backend
