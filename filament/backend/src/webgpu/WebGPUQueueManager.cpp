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

#include "WebGPUQueueManager.h"

#include <utils/Panic.h>

#include <chrono>
#include <cstdint>
#include <thread>

namespace filament::backend {

FenceStatus WebGPUSubmissionState::waitForCompletion(uint64_t timeout) {
    using namespace std::chrono;
    auto now = steady_clock::now();
    steady_clock::time_point until = steady_clock::time_point::max();
    if (now <= steady_clock::time_point::max() - nanoseconds(timeout)) {
        until = now + nanoseconds(timeout);
    }

    std::unique_lock<std::mutex> lock(mLock);
    mCond.wait_until(lock, until, [this] {
        return mStatus == FenceStatus::CONDITION_SATISFIED || mStatus == FenceStatus::ERROR;
    });
    // Note that mStatus is default to FenceStatus::TIMEOUT_EXPIRED, so if SetStatus() wasn't
    // called, then FenceStatus::TIMEOUT_EXPIRED will be returned, which is aligned with the
    // condition if the wait_until above times out.
    return mStatus;
}


void WebGPUSubmissionState::setStatus(FenceStatus status) {
    std::lock_guard<std::mutex> const lock(mLock);
    mStatus = status;
    mCond.notify_all();
}

WebGPUQueueManager::WebGPUQueueManager(wgpu::Device const& device)
        : mDevice(device),
          mQueue(device.GetQueue()),
          mLatestSubmissionState(
                  std::make_shared<WebGPUSubmissionState>(FenceStatus::CONDITION_SATISFIED)) {}

WebGPUQueueManager::~WebGPUQueueManager() = default;

wgpu::CommandEncoder WebGPUQueueManager::getCommandEncoder() {
    if (!mCommandEncoder) {
        wgpu::CommandEncoderDescriptor commandEncoderDescriptor = {
            .label = "Filament Command Encoder",
        };
        mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
        ASSERT_POSTCONDITION(mCommandEncoder, "Failed to create command encoder.");
    }
    return mCommandEncoder;
}

void WebGPUQueueManager::flush() {
    submit();
}

void WebGPUQueueManager::finish() {
    submit();

    // This is a strange side-effect of the dawn callback system, where we would need to keep
    // calling ProcessEvents() to be able to advance an internal sequence ID and reach our callback.
    // This is similar to draining a work queue. We currently have no other way to force the "last"
    // callback to be called.
    while (mLatestSubmissionState->getStatus() == FenceStatus::TIMEOUT_EXPIRED) {
        mDevice.GetAdapter().GetInstance().ProcessEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::shared_ptr<WebGPUSubmissionState> WebGPUQueueManager::getLatestSubmissionState() {
    return mLatestSubmissionState;
}

void WebGPUQueueManager::submit() {
    if (!mCommandEncoder) {
        return;
    }

    mLatestSubmissionState = std::make_shared<WebGPUSubmissionState>();

    wgpu::CommandBufferDescriptor commandBufferDescriptor{
        .label = "Filament Command Buffer",
    };

    auto cbuf = mCommandEncoder.Finish(&commandBufferDescriptor);
    mQueue.Submit(1, &cbuf);

    mQueue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowSpontaneous,
            [state = mLatestSubmissionState](wgpu::QueueWorkDoneStatus status,
                    wgpu::StringView sv) {
                switch (status) {
                    case wgpu::QueueWorkDoneStatus::Success:
                        state->setStatus(FenceStatus::CONDITION_SATISFIED);
                        break;
                    case wgpu::QueueWorkDoneStatus::Error:
                    case wgpu::QueueWorkDoneStatus::CallbackCancelled:
                        state->setStatus(FenceStatus::ERROR);
                        break;
                }
            });

    mCommandEncoder = nullptr;
}

} // namespace filament::backend
