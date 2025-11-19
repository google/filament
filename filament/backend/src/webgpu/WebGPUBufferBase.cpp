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

#include "WebGPUBufferBase.h"

#include "WebGPUConstants.h"
#include "WebGPUQueueManager.h"

#include "DriverBase.h"
#include <backend/BufferDescriptor.h>

#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <cstring>

namespace filament::backend {

namespace {

// Creates a wgpu::Buffer, ensuring its size is a multiple of FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS.
// WebGPU's WriteBuffer requires the write size to be a multiple of 4. By ensuring the buffer
// size is also a multiple of 4, we simplify the update logic.
[[nodiscard]] wgpu::Buffer createBuffer(wgpu::Device const& device, const wgpu::BufferUsage usage,
        uint32_t size, const char* const label) {
    // Write size must be divisible by WEBGPU_BUFFER_SIZE_MODULUS (e.g. 4).
    // If the whole buffer is written to as is common, so must the buffer size.
    size += (FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS - (size % FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS)) %
            FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS;
    wgpu::BufferDescriptor descriptor{
        .label = label,
        .usage = usage,
        .size = size,
        .mappedAtCreation = false };
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    FILAMENT_CHECK_POSTCONDITION(buffer) << "Failed to create buffer for " << label;
    return buffer;
}

} // namespace

WebGPUBufferBase::WebGPUBufferBase(wgpu::Device const& device, const wgpu::BufferUsage usage,
        const uint32_t size, char const* const label)
    : mBuffer{ createBuffer(device, usage, size, label) } {}

// Updates the GPU buffer with data from a BufferDescriptor.
// WebGPU requires that the size of the data copied from the staging buffer to the GPU buffer is a
// multiple of 4. This function handles cases where the buffer descriptor's size is not a multiple
// of 4 by padding with zeros.
void WebGPUBufferBase::updateGPUBuffer(BufferDescriptor&& bufferDescriptor,
        const uint32_t byteOffset, wgpu::Device const& device,
        WebGPUQueueManager* const webGPUQueueManager) {
    FILAMENT_CHECK_PRECONDITION(bufferDescriptor.buffer)
            << "updateGPUBuffer called with a null buffer";
    FILAMENT_CHECK_PRECONDITION(bufferDescriptor.size + byteOffset <= mBuffer.GetSize())
            << "Attempting to copy " << bufferDescriptor.size << " bytes into a buffer of size "
            << mBuffer.GetSize() << " at offset " << byteOffset;
    FILAMENT_CHECK_PRECONDITION(byteOffset % FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS == 0)
            << "Byte offset must be a multiple of " << FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS
            << " but is " << byteOffset;

    // TODO: All buffer objects are created with CopyDst usage.
    // This may have some performance implications. That should be investigated later.
    assert_invariant(mBuffer.GetUsage() & wgpu::BufferUsage::CopyDst);

    // Calculate some alignment related sizes
    const size_t remainder = bufferDescriptor.size % FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS;
    const size_t mainBulk = bufferDescriptor.size - remainder;
    const size_t stagingBufferSize =
            remainder == 0 ? bufferDescriptor.size : mainBulk + FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS;

    // create a staging buffer
    wgpu::BufferDescriptor descriptor{
        .label = "Filament WebGPU Staging Buffer",
        .usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc,
        .size = stagingBufferSize};
    wgpu::Buffer stagingBuffer = device.CreateBuffer(&descriptor);

    struct UserData final {
        uint32_t byteOffset;
        size_t stagingBufferSize;
        size_t remainder;
        BufferDescriptor srcBufferDescriptor;
        wgpu::Buffer stagingBuffer;
        WebGPUQueueManager* const webGPUQueueManager;
        wgpu::Buffer dstBuffer;
    };
    auto userData = std::make_unique<UserData>(UserData{
        .byteOffset = byteOffset,
        .stagingBufferSize = stagingBufferSize,
        .remainder = remainder,
        .srcBufferDescriptor = std::move(bufferDescriptor),
        .stagingBuffer = stagingBuffer,
        .webGPUQueueManager = webGPUQueueManager,
        .dstBuffer = mBuffer});
    stagingBuffer.MapAsync(
            wgpu::MapMode::Write, 0, stagingBufferSize, wgpu::CallbackMode::AllowProcessEvents,
            [](wgpu::MapAsyncStatus status, const char* message, UserData* userdata) {
                std::unique_ptr<UserData> data(static_cast<UserData*>(userdata));
                if (UTILS_LIKELY(status == wgpu::MapAsyncStatus::Success)) {
                    void* mappedRange = data->stagingBuffer.GetMappedRange();
                    memcpy(mappedRange, data->srcBufferDescriptor.buffer,
                            data->srcBufferDescriptor.size);
                    if (data->remainder != 0) {
                        uint8_t* paddingStart =
                                static_cast<uint8_t*>(mappedRange) + data->srcBufferDescriptor.size;
                        memset(paddingStart, 0,
                                FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS - data->remainder);
                    }
                    data->stagingBuffer.Unmap();
                    data->webGPUQueueManager->getCommandEncoder().CopyBufferToBuffer(
                            data->stagingBuffer, 0, data->dstBuffer, data->byteOffset,
                            data->stagingBufferSize);
                } else {
                    FWGPU_LOGE << "Failed to map staging buffer for readPixels: " << message;
                }
            },
            userData.release());
}

} // namespace filament::backend
