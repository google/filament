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

#include <webgpu/webgpu_cpp.h>

#include <condition_variable>
#include <chrono>
#include <cstdint>
#include <mutex>

namespace filament::backend {

FenceStatus WebGPUFence::getStatus() {
    std::lock_guard const lock(mLock);
    return mStatus;
}

FenceStatus WebGPUFence::wait(uint64_t const timeout) {
    std::unique_lock lock(mLock);
    mCondition.wait_for(lock, std::chrono::nanoseconds(timeout),
            [this] { return mStatus != FenceStatus::TIMEOUT_EXPIRED; });
    return mStatus;
}

void WebGPUFence::addMarkerToQueueState(wgpu::Queue const& queue) {
    // The lambda function is called when the work is done. It updates the fence status based on the
    // result of the work.
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowSpontaneous,
        [this](const wgpu::QueueWorkDoneStatus status, wgpu::StringView message) {
            std::unique_lock const lock(mLock);
            switch (status) {
                case wgpu::QueueWorkDoneStatus::Success:
                    mStatus = FenceStatus::CONDITION_SATISFIED;
                    mCondition.notify_all();
                    break;
                case wgpu::QueueWorkDoneStatus::CallbackCancelled:
                case wgpu::QueueWorkDoneStatus::Error:
                    mStatus = FenceStatus::ERROR;
                    mCondition.notify_all();
                    FWGPU_LOGW << "WebGPUFence: wgpu::QueueWorkDoneStatus::Error. " << message;
                    break;
            }
        });
}

} // namespace filament::backend
