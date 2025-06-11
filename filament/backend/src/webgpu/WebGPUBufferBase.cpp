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

namespace filament::backend {

namespace {

[[nodiscard]] wgpu::Buffer createBuffer(wgpu::Device const& device, const wgpu::BufferUsage usage,
        uint32_t size, const char* const label) {
    // Write size must be divisible by WEBGPU_BUFFER_SIZE_MODULUS (e.g. 4).
    // If the whole buffer is written to as is common, so must the buffer size.
    size += (WEBGPU_BUFFER_SIZE_MODULUS - (size % WEBGPU_BUFFER_SIZE_MODULUS)) %
            WEBGPU_BUFFER_SIZE_MODULUS;
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

void WebGPUBufferBase::updateGPUBuffer(BufferDescriptor const& bufferDescriptor,
        const uint32_t byteOffset, wgpu::Queue const& queue) {
    FILAMENT_CHECK_PRECONDITION(bufferDescriptor.buffer)
            << "updateGPUBuffer called with a null buffer";
    FILAMENT_CHECK_PRECONDITION(bufferDescriptor.size + byteOffset <= mBuffer.GetSize())
            << "Attempting to copy " << bufferDescriptor.size << " bytes into a buffer of size "
            << mBuffer.GetSize() << " at offset " << byteOffset;
    FILAMENT_CHECK_PRECONDITION(byteOffset % WEBGPU_BUFFER_SIZE_MODULUS == 0)
            << "Byte offset must be a multiple of " << WEBGPU_BUFFER_SIZE_MODULUS << " but is "
            << byteOffset;

    // TODO: All buffer objects are created with CopyDst usage.
    // This may have some performance implications. That should be investigated later.
    assert_invariant(mBuffer.GetUsage() & wgpu::BufferUsage::CopyDst);

    const size_t remainder = bufferDescriptor.size % WEBGPU_BUFFER_SIZE_MODULUS;

    // WriteBuffer is an async call. But cpu buffer data is already written to the staging
    // buffer on return from the WriteBuffer.
    const size_t legalSize = bufferDescriptor.size - remainder;
    queue.WriteBuffer(mBuffer, byteOffset, bufferDescriptor.buffer, legalSize);
    if (remainder != 0) {
        const uint8_t* remainderStart =
                static_cast<const uint8_t*>(bufferDescriptor.buffer) + legalSize;
        memcpy(mRemainderChunk.data(), remainderStart, remainder);
        // Pad the remainder with zeros to ensure deterministic behavior, though GPU shouldn't
        // access this
        std::memset(mRemainderChunk.data() + remainder, 0, WEBGPU_BUFFER_SIZE_MODULUS - remainder);

        queue.WriteBuffer(mBuffer, byteOffset + legalSize, &mRemainderChunk,
                WEBGPU_BUFFER_SIZE_MODULUS);
    }
}

}// namespace filament::backend
