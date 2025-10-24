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

#include "DriverBase.h"
#include <backend/BufferDescriptor.h>

#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <cstring>
#include <iostream>

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
    : mBuffer{ createBuffer(device, usage, size, label) }, mDevice (device) {}

// Updates the GPU buffer with data from a BufferDescriptor.
// WebGPU requires that the size of the data written to a buffer is a multiple of 4.
// This function handles cases where the buffer descriptor's size is not a multiple of 4
// by writing the bulk of the data first, and then copying the remaining bytes into a
// padded temporary chunk which is then written to the buffer.
void WebGPUBufferBase::updateGPUBuffer(BufferDescriptor const& bufferDescriptor,
        const uint32_t byteOffset, wgpu::Queue const& queue) {
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

    // create the staging buffer
    wgpu::BufferDescriptor descriptor{
        .label = "staging buffer",
        .usage =  wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc,
        .size = mBuffer.GetSize(),
        .mappedAtCreation = true };
    wgpu::Buffer stagingBuffer = mDevice.CreateBuffer(&descriptor);
    std::cout << "Run Yu: creating a new staging buffer, with size of " << descriptor.size << std::endl;

    // Calculate some alignment related sizes
    const size_t copySizeRemainder = bufferDescriptor.size % FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS;
    const size_t legalCopySize = bufferDescriptor.size - copySizeRemainder;
    // WGPU requires the offset of GetMappedRange to be a multiple of 8
    const int stagingOffsetShift = -(byteOffset % FILAMENT_WEBGPU_MAPPED_RANGE_OFFSET_MODULUS);

    void *mappedRange = stagingBuffer.GetMappedRange(byteOffset + stagingOffsetShift);
    // copy the data to the staging buffer
    memcpy(mappedRange, bufferDescriptor.buffer, bufferDescriptor.size);

    stagingBuffer.Unmap();

    // Copy the staging buffer contents to the destination buffer.
    wgpu::CommandEncoderDescriptor commandEncodeDescriptor = {};
    commandEncodeDescriptor.label = "copy buffer to buffer";
    const wgpu::CommandEncoder commandEncoder = mDevice.CreateCommandEncoder(&commandEncodeDescriptor);
    commandEncoder.CopyBufferToBuffer(
        stagingBuffer,
        byteOffset + stagingOffsetShift,
        mBuffer,
        byteOffset,
        copySizeRemainder == 0 ?
            bufferDescriptor.size : legalCopySize + FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS);
    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // This is where it gets confusing, I'm not sure how this CPU-to-GPU synchronization works
    // in WGPU. With Vulkan and DX a Fence is used here.
    bool ready = false;
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowSpontaneous,
        [&ready](wgpu::QueueWorkDoneStatus status,
                wgpu::StringView sv) {
            switch (status) {
                case wgpu::QueueWorkDoneStatus::Success:
                    std::cout << "Run Yu, queued commands finished successfully" << std::endl;
                    ready = true;
                    break;
                case wgpu::QueueWorkDoneStatus::Error:
                case wgpu::QueueWorkDoneStatus::CallbackCancelled:
                    std::cout << "Run Yu: queue submission met error or cancellation" << std::endl;
                    break;
            }
        });

    while (!ready) {
        mDevice.Tick();
    }
}

} // namespace filament::backend
