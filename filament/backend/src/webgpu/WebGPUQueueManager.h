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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUQUEUEMANAGER_H
#define TNT_FILAMENT_BACKEND_WEBGPUQUEUEMANAGER_H

#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <mutex>

namespace filament::backend {

// This struct represents the state of a single submission to the GPU queue.
// A shared pointer to this object is held by WebGPUFence objects.
struct WebGPUSubmissionState {
    WebGPUSubmissionState()
            : mStatus(FenceStatus::TIMEOUT_EXPIRED) {}

    WebGPUSubmissionState(FenceStatus status)
            : mStatus(status) {}

    FenceStatus getStatus() {
        std::lock_guard<std::mutex> const lock(mLock);
        return mStatus;
    }

    FenceStatus waitForCompletion(uint64_t timeout = std::numeric_limits<uint64_t>::max());

    void setStatus(FenceStatus status);

private:
    std::mutex mLock;
    std::condition_variable mCond;
    FenceStatus mStatus;
};

class WebGPUQueueManager {
public:
    WebGPUQueueManager(wgpu::Device const& device);
    ~WebGPUQueueManager();

    // Returns the command encoder for the current workload. Creates one if it doesn't exist.
    wgpu::CommandEncoder getCommandEncoder();

    // Submits the current command buffer and creates a new submission state.
    void flush();

    // Submits the current command buffer and blocks until the work is done.
    void finish();

    // Returns a shared pointer to the latest submission state.
    std::shared_ptr<WebGPUSubmissionState> getLatestSubmissionState();

private:
    void submit();

    wgpu::Device mDevice;
    wgpu::Queue mQueue;
    wgpu::CommandEncoder mCommandEncoder;
    std::shared_ptr<WebGPUSubmissionState> mLatestSubmissionState;
    std::mutex mLock;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUQUEUEMANAGER_H
