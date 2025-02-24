// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/common/FutureUtils.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class BasicTests : public DawnTest {};

// Test adapter filter by vendor id.
TEST_P(BasicTests, VendorIdFilter) {
    DAWN_TEST_UNSUPPORTED_IF(!HasVendorIdFilter());

    ASSERT_EQ(GetAdapterProperties().vendorID, GetVendorIdFilter());
}

// Test adapter filter by backend type.
TEST_P(BasicTests, BackendType) {
    DAWN_TEST_UNSUPPORTED_IF(!HasBackendTypeFilter());

    ASSERT_EQ(GetAdapterProperties().backendType, GetBackendTypeFilter());
}

// Test Queue::WriteBuffer changes the content of the buffer, but really this is the most
// basic test possible, and tests the test harness
TEST_P(BasicTests, QueueWriteBuffer) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t value = 0x01020304;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);
}

// Test a validation error for Queue::WriteBuffer but really this is the most basic test possible
// for ASSERT_DEVICE_ERROR
TEST_P(BasicTests, QueueWriteBufferError) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint8_t value = 187;
    ASSERT_DEVICE_ERROR(queue.WriteBuffer(buffer, 1000, &value, sizeof(value)));
}

TEST_P(BasicTests, GetInstanceCapabilities) {
    wgpu::InstanceCapabilities instanceCapabilities{};
    auto status = wgpu::GetInstanceCapabilities(&instanceCapabilities);
    EXPECT_EQ(status, wgpu::Status::Success);
    EXPECT_EQ(instanceCapabilities.timedWaitAnyEnable, !UsesWire());
    EXPECT_EQ(instanceCapabilities.timedWaitAnyMaxCount, kTimedWaitAnyMaxCountDefault);
    EXPECT_EQ(instanceCapabilities.nextInChain, nullptr);

    wgpu::ChainedStruct chained{};
    instanceCapabilities.nextInChain = &chained;
    status = wgpu::GetInstanceCapabilities(&instanceCapabilities);
    EXPECT_EQ(status, wgpu::Status::Error);
}

DAWN_INSTANTIATE_TEST(BasicTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
