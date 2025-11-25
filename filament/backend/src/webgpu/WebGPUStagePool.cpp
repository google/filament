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

#include "WebGPUStagePool.h"

#include "WebGPUConstants.h"
#include "WebGPUQueueManager.h"

#include <memory>

namespace filament::backend {

WebGPUStagePool::WebGPUStagePool(wgpu::Device const& device) : mDevice(device) {}

WebGPUStagePool::~WebGPUStagePool() = default;

wgpu::Buffer WebGPUStagePool::acquireBuffer(std::shared_ptr<WebGPUSubmissionState> submission,
        size_t requiredSize) {
    wgpu::Buffer buf;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (auto iter = mBuffers.lower_bound(requiredSize); iter != mBuffers.end()) {
            buf = iter->second;
            mBuffers.erase(iter);
        }
    }
    if (!buf.Get()) {
        buf = createNewBuffer(requiredSize);
    }
    mSubmitted.push_back({ submission, buf });
    return buf;
}

void WebGPUStagePool::addBufferToPool(wgpu::Buffer buffer) {
    using UserData = std::pair<wgpu::Buffer, WebGPUStagePool*>;
    auto userData = new UserData{buffer, this};
    buffer.MapAsync(
            wgpu::MapMode::Write, 0, buffer.GetSize(), wgpu::CallbackMode::AllowSpontaneous,
            [](wgpu::MapAsyncStatus status, const char* message, UserData* userData) {
                if (UTILS_LIKELY(status == wgpu::MapAsyncStatus::Success)) {
                    std::unique_ptr<UserData> data(static_cast<UserData*>(userData));
                    auto [buffer, t] = *data;
                    {
                        std::lock_guard<std::mutex> lock(t->mMutex);
                        t->mBuffers.insert({ buffer.GetSize(), buffer });
                    }
                }
            },
            userData);
}

wgpu::Buffer WebGPUStagePool::createNewBuffer(size_t bufferSize) {
    wgpu::BufferDescriptor descriptor{
        .label = "Filament WebGPU Staging Buffer",
        .usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc,
        .size = bufferSize,
        .mappedAtCreation = true,
    };
    return mDevice.CreateBuffer(&descriptor);
}

void WebGPUStagePool::gc() {
    std::vector<std::pair<std::shared_ptr<WebGPUSubmissionState>, wgpu::Buffer>> newSubmitted;
    for (auto& [st, buffer]: mSubmitted) {
        if (st->getStatus() == FenceStatus::CONDITION_SATISFIED) {
            addBufferToPool(buffer);
        } else {
            newSubmitted.push_back({st, buffer});
        }
    }
    std::swap(mSubmitted, newSubmitted);
}

} // namespace filament::backend
