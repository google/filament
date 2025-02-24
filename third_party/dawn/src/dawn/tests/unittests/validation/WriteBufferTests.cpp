// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

class WriteBufferTest : public ValidationTest {
  public:
    wgpu::Buffer CreateWritableBuffer(uint64_t size) {
        wgpu::BufferDescriptor desc;
        desc.usage = wgpu::BufferUsage::CopyDst;
        desc.size = size;
        return device.CreateBuffer(&desc);
    }

    wgpu::CommandBuffer EncodeWriteBuffer(wgpu::Buffer buffer,
                                          uint64_t bufferOffset,
                                          uint64_t size) {
        std::vector<uint8_t> data(size);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteBuffer(buffer, bufferOffset, data.data(), size);
        return encoder.Finish();
    }
};

// Tests that the buffer offset is validated to be a multiple of 4 bytes.
TEST_F(WriteBufferTest, OffsetAlignment) {
    wgpu::Buffer buffer = CreateWritableBuffer(64);
    EncodeWriteBuffer(buffer, 0, 4);
    EncodeWriteBuffer(buffer, 4, 4);
    EncodeWriteBuffer(buffer, 60, 4);
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 1, 4));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 2, 4));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 3, 4));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 5, 4));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 11, 4));
}

// Tests that the buffer size is validated to be a multiple of 4 bytes.
TEST_F(WriteBufferTest, SizeAlignment) {
    wgpu::Buffer buffer = CreateWritableBuffer(64);
    EncodeWriteBuffer(buffer, 0, 64);
    EncodeWriteBuffer(buffer, 4, 60);
    EncodeWriteBuffer(buffer, 40, 24);
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 0, 63));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 4, 1));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 4, 2));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 40, 23));
}

// Tests that the buffer size and offset are validated to fit within the bounds of the buffer.
TEST_F(WriteBufferTest, BufferBounds) {
    wgpu::Buffer buffer = CreateWritableBuffer(64);
    EncodeWriteBuffer(buffer, 0, 64);
    EncodeWriteBuffer(buffer, 4, 60);
    EncodeWriteBuffer(buffer, 40, 24);
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 0, 68));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 4, 64));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 60, 8));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 64, 4));
}

// Tests that the destination buffer's usage is validated to contain CopyDst.
TEST_F(WriteBufferTest, RequireCopyDstUsage) {
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::CopySrc;
    desc.size = 64;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 0, 64));
}

// Tests that the destination buffer's state is validated at submission.
TEST_F(WriteBufferTest, ValidBufferState) {
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::CopyDst;
    desc.size = 64;
    desc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::CommandBuffer commands = EncodeWriteBuffer(buffer, 0, 64);
    ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));

    commands = EncodeWriteBuffer(buffer, 0, 64);
    buffer.Unmap();
    device.GetQueue().Submit(1, &commands);
}

}  // namespace
