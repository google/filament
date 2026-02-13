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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUFENCE_H
#define TNT_FILAMENT_BACKEND_WEBGPUFENCE_H

#include "WebGPUQueueManager.h"

#include "DriverBase.h"

#include <backend/DriverEnums.h>

#include <memory>

namespace filament::backend {

class WebGPUFence : public HwFence {
public:
    WebGPUFence();
    ~WebGPUFence();

    // Initialize the fence with a submission state from the queue manager.
    void setSubmissionState(std::shared_ptr<WebGPUSubmissionState> state);

    // Note that getStatus cannot be const since it's implemented with wait();
    FenceStatus getStatus();
    FenceStatus wait(uint64_t timeout);

private:
    std::mutex mLock;
    std::condition_variable mCond;
    std::shared_ptr<WebGPUSubmissionState> mState;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUFENCE_H
