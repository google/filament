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

#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <atomic>

namespace filament::backend {

FenceStatus WebGPUFence::getStatus() { return mStatus.load(); }

void WebGPUFence::addMarkerToQueueState(wgpu::Queue const& queue) {
    queue.OnSubmittedWorkDone(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::QueueWorkDoneStatus status,
                    std::atomic<FenceStatus>* const fenceStatus) {
                if (!fenceStatus) {
                    return;
                }
                switch (status) {
                    case wgpu::QueueWorkDoneStatus::Success:
                        fenceStatus->store(FenceStatus::CONDITION_SATISFIED);
                        break;
                    case wgpu::QueueWorkDoneStatus::CallbackCancelled:
                    case wgpu::QueueWorkDoneStatus::Error:
                        fenceStatus->store(FenceStatus::ERROR);
                        break;
                }
            },
            &mStatus);
}

} // namespace filament::backend
