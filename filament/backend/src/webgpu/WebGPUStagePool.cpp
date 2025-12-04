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

namespace filament::backend {

WebGPUStagePool::WebGPUStagePool(wgpu::Device const& device) : mDevice(device) {}

WebGPUStagePool::~WebGPUStagePool() = default;

wgpu::Buffer WebGPUStagePool::acquireBuffer(size_t requiredSize,
        std::shared_ptr<WebGPUSubmissionState> latestSubmissionState) {
    wgpu::Buffer buffer;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        auto iter = mBuffers.lower_bound(requiredSize);
        if (iter != mBuffers.end()) {
            buffer = iter->second;
            mBuffers.erase(iter);
        }
    }
    if (!buffer.Get()) {
        buffer = createNewBuffer(requiredSize);
    }
    mInProgress.push_back({latestSubmissionState, buffer});
    return buffer;
}

void WebGPUStagePool::recycleBuffer(wgpu::Buffer buffer) {
    struct UserData final {
        wgpu::Buffer buffer;
        WebGPUStagePool* webGPUStagePool;
    };
    auto userData =
            std::make_unique<UserData>(UserData{ .buffer = buffer, .webGPUStagePool = this });
    buffer.MapAsync(wgpu::MapMode::Write, 0, buffer.GetSize(), wgpu::CallbackMode::AllowSpontaneous,
            [data = std::move(userData)](wgpu::MapAsyncStatus status, const char* message) {
                if (UTILS_LIKELY(status == wgpu::MapAsyncStatus::Success)) {
                    if (!data->webGPUStagePool) {
                        return;
                    }
                    std::lock_guard<std::mutex> lock(data->webGPUStagePool->mMutex);
                    data->webGPUStagePool->mBuffers.insert(
                            { data->buffer.GetSize(), data->buffer });
                } else {
                    FWGPU_LOGE << "Failed to MapAsync when recycling staging buffer: " << message;
                }
            });
}

void WebGPUStagePool::gc() {
    // We found that MapAsync would sometimes lead to GetMappedRange returning nullptr if the
    // command using that staging buffer has not finished executing, so here we only recycle those
    // buffers that are not still being used by any command
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
    wgpu::BufferDescriptor descriptor{
        .label = "Filament WebGPU Staging Buffer",
        .usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc,
        .size = bufferSize,
        .mappedAtCreation = true };
    return mDevice.CreateBuffer(&descriptor);
}

} // namespace filament::backend
