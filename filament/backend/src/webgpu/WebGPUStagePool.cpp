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

#include <iostream>

namespace filament::backend {

WebGPUStagePool::WebGPUStagePool(wgpu::Device const& device) : mDevice(device) {}

WebGPUStagePool::~WebGPUStagePool() = default;

wgpu::Buffer WebGPUStagePool::acquireBuffer(size_t requiredSize,
        std::shared_ptr<WebGPUSubmissionState> latestSubmissionState) {
    std::cout << "Run Yu: required size in acquireBuffer: " << requiredSize << std::endl;
    std::cout << "Run Yu: the pool size is " << mBuffers.size() << std::endl;
    wgpu::Buffer buffer;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        auto iter = mBuffers.lower_bound(requiredSize);
        if (iter != mBuffers.end()) {
            buffer = iter->second;
            std::cout << "Run Yu: found buffer in the pool with size " << buffer.GetSize()
                      << std::endl;
            mBuffers.erase(iter);
            if (buffer.GetMapState() != wgpu::BufferMapState::Mapped) {
                std::cout << "Run Yu: buffer from pool is not mapped!!" << std::endl;
            }
        }
    }
    if (!buffer.Get()) {
        buffer = createNewBuffer(requiredSize);
    }
    mInProgress.push_back({latestSubmissionState, buffer});
    return buffer;
}

void WebGPUStagePool::recycleBuffer(wgpu::Buffer buffer) {
    using UserData = std::pair<wgpu::Buffer, WebGPUStagePool*>;
    auto userData = std::make_unique<UserData>(buffer, this);
    buffer.MapAsync(
            wgpu::MapMode::Write, 0, buffer.GetSize(), wgpu::CallbackMode::AllowSpontaneous,
            [](wgpu::MapAsyncStatus status, const char* message, UserData* userData) {
                if (UTILS_LIKELY(status == wgpu::MapAsyncStatus::Success)) {
                    std::unique_ptr<UserData> data(static_cast<UserData*>(userData));
                    auto [buf, pool] = *data;
                    if (pool) {
                        std::cout << "Run Yu: MapAsync successful with size " << buf.GetSize() << std::endl;
                        std::lock_guard<std::mutex> lock(pool->mMutex);
                        pool->mBuffers.insert({ buf.GetSize(), buf });
                    }
                } else {
                    FWGPU_LOGE << "Failed to MapAsync when recycling staging buffer: " << message;
                }
            },
            userData.release());
}

void WebGPUStagePool::gc() {
    // We found that MapAsync would lead to nullptr with GetMappedRange if the command using that
    // staging buffer has not finished executing, so here we only recycle those buffers that are not
    // still being used by any command
    std::vector<std::pair<std::shared_ptr<WebGPUSubmissionState>, wgpu::Buffer>> stillInProgress;
    for (auto& [st, buffer]: mInProgress) {
        if (st->getStatus() == FenceStatus::CONDITION_SATISFIED) {
            recycleBuffer(buffer);
        } else {
            stillInProgress.push_back({st, buffer});
        }
    }
    std::swap(mInProgress, stillInProgress);
}

wgpu::Buffer WebGPUStagePool::createNewBuffer(size_t bufferSize) {
    std::cout << "Run Yu: creating new buffer with size " << bufferSize << std::endl;
    wgpu::BufferDescriptor descriptor{
        .label = "Filament WebGPU Staging Buffer",
        .usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc,
        .size = bufferSize,
        .mappedAtCreation = true };
    return mDevice.CreateBuffer(&descriptor);
}

} // namespace filament::backend
