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

#include <atomic>

namespace filament::backend {

FenceStatus WebGPUFence::getStatus() { return mStatus.load(); }

void WebGPUFence::addMarkerToQueueState(wgpu::Queue const& queue) {
    // The lambda function is called when the work is done. It updates the fence status based on the
    // result of the work.
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowSpontaneous,
        [this](const wgpu::QueueWorkDoneStatus status, wgpu::StringView message) {
            switch (status) {
                case wgpu::QueueWorkDoneStatus::Success:
                    mStatus.store(FenceStatus::CONDITION_SATISFIED);
                    break;
                case wgpu::QueueWorkDoneStatus::CallbackCancelled:
                case wgpu::QueueWorkDoneStatus::Error:
                    mStatus.store(FenceStatus::ERROR);
                    FWGPU_LOGW << "WebGPUFence: wgpu::QueueWorkDoneStatus::Error. " << message;
                    break;
            }
        });
}

} // namespace filament::backend
