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

#include <iostream>

namespace filament::backend {

WebGPUStagePool::WebGPUStagePool(wgpu::Device const& device) : mDevice(device) {}

WebGPUStagePool::~WebGPUStagePool() = default;

wgpu::Buffer WebGPUStagePool::acquireBuffer(size_t requiredSize) {
    std::cout << "Run Yu: required size in acquireBuffer: " << requiredSize << std::endl;
    std::cout << "Run Yu: the pool size is " << mBuffers.size() << std::endl;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        auto iter = mBuffers.lower_bound(requiredSize);
        if (iter != mBuffers.end()) {
            wgpu::Buffer bufferFromPool = iter->second;
            std::cout << "Run Yu: found buffer in the pool with size " << bufferFromPool.GetSize()
                      << std::endl;
            mBuffers.erase(iter);
            if (bufferFromPool.GetMapState() != wgpu::BufferMapState::Mapped) {
                std::cout << "Run Yu: buffer from pool is not mapped!!" << std::endl;
            }
            return bufferFromPool;
        }
    }
    return createNewBuffer(requiredSize);
}

void WebGPUStagePool::addBufferToPool(wgpu::Buffer buffer) {
    std::lock_guard<std::mutex> lock(mMutex);
    std::cout << "Run Yu: adding buffer to the pool with size " << buffer.GetSize() << std::endl;
    mBuffers.insert({buffer.GetSize(), buffer});
    std::cout << "Run Yu: added buffer to the pool with size " << buffer.GetSize() << std::endl;

    bool allMapped = true;
    for (const auto& pair : mBuffers) {
        auto state = pair.second.GetMapState();
        if (state != wgpu::BufferMapState::Mapped) {
            allMapped = false;
            std::cout << "Run Yu: the buffer with size " << pair.second.GetSize()
                      << " is not mapped but somehow was added to the pool, its state is "
                      << static_cast<int>(state) << std::endl;
        }
    }
    if (!allMapped) {
        std::cout << "Run Yu: found buffers that are not mapped\n";
    } else {
        std::cout << "Run Yu: all buffers are mapped\n";
    }
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
