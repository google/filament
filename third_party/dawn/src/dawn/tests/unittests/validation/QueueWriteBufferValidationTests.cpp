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

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/utils/WGPUHelpers.h"

class QueueWriteBufferValidationTest : public ValidationTest {
  private:
    void SetUp() override {
        ValidationTest::SetUp();
        queue = device.GetQueue();
    }

  protected:
    wgpu::Buffer CreateBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Queue queue;
};

// Test the success case for WriteBuffer
TEST_F(QueueWriteBufferValidationTest, Success) {
    wgpu::Buffer buf = CreateBuffer(4);

    uint32_t foo = 0x01020304;
    queue.WriteBuffer(buf, 0, &foo, sizeof(foo));
}

// Test error case for WriteBuffer out of bounds
TEST_F(QueueWriteBufferValidationTest, OutOfBounds) {
    wgpu::Buffer buf = CreateBuffer(4);

    uint32_t foo[2] = {0, 0};
    ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, foo, 8));
}

// Test error case for WriteBuffer out of bounds with an overflow
TEST_F(QueueWriteBufferValidationTest, OutOfBoundsOverflow) {
    wgpu::Buffer buf = CreateBuffer(1024);

    uint32_t foo[2] = {0, 0};

    // An offset that when added to "4" would overflow to be zero and pass validation without
    // overflow checks.
    uint64_t offset = uint64_t(int64_t(0) - int64_t(4));

    ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, offset, foo, 4));
}

// Test error case for WriteBuffer with the wrong usage
TEST_F(QueueWriteBufferValidationTest, WrongUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Vertex;
    wgpu::Buffer buf = device.CreateBuffer(&descriptor);

    uint32_t foo = 0;
    ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, &foo, sizeof(foo)));
}

// Test WriteBuffer with unaligned size
TEST_F(QueueWriteBufferValidationTest, UnalignedSize) {
    wgpu::Buffer buf = CreateBuffer(4);

    uint16_t value = 123;
    ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, &value, sizeof(value)));
}

// Test WriteBuffer with unaligned offset
TEST_F(QueueWriteBufferValidationTest, UnalignedOffset) {
    wgpu::Buffer buf = CreateBuffer(8);

    uint32_t value = 0x01020304;
    ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 2, &value, sizeof(value)));
}

// Test WriteBuffer with destroyed buffer
TEST_F(QueueWriteBufferValidationTest, DestroyedBuffer) {
    wgpu::Buffer buf = CreateBuffer(4);
    buf.Destroy();

    uint32_t value = 0;
    ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, &value, sizeof(value)));
}

// Test WriteBuffer with mapped buffer
TEST_F(QueueWriteBufferValidationTest, MappedBuffer) {
    // mappedAtCreation
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        descriptor.mappedAtCreation = true;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        uint32_t value = 0;
        ASSERT_DEVICE_ERROR(queue.WriteBuffer(buffer, 0, &value, sizeof(value)));
    }

    // MapAsync
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
        wgpu::Buffer buf = device.CreateBuffer(&descriptor);

        buf.MapAsync(wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                     [](wgpu::MapAsyncStatus, wgpu::StringView) {});
        uint32_t value = 0;
        ASSERT_DEVICE_ERROR(queue.WriteBuffer(buf, 0, &value, sizeof(value)));
    }
}
