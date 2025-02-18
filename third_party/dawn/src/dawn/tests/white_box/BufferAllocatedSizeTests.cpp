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

#include <algorithm>

#include "dawn/tests/DawnTest.h"

#include "dawn/common/Math.h"
#include "dawn/native/DawnNative.h"

namespace dawn {
namespace {

class BufferAllocatedSizeTests : public DawnTest {
  protected:
    wgpu::Buffer CreateBuffer(wgpu::BufferUsage usage, uint64_t size) {
        wgpu::BufferDescriptor desc = {};
        desc.usage = usage;
        desc.size = size;
        return device.CreateBuffer(&desc);
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }
};

// Test expected allocated size for buffers with uniform usage
TEST_P(BufferAllocatedSizeTests, UniformUsage) {
    // Some backends have a minimum buffer size, so make sure
    // we allocate above that.
    constexpr uint32_t kMinBufferSize = 4u;

    uint32_t requiredBufferAlignment = 1u;
    if (IsD3D12()) {
        requiredBufferAlignment = 256u;
    } else if (IsMetal()) {
        requiredBufferAlignment = 16u;
    } else if (IsVulkan()) {
        requiredBufferAlignment = 4u;
    }

    // Test uniform usage
    {
        const uint32_t bufferSize = kMinBufferSize;
        wgpu::Buffer buffer = CreateBuffer(wgpu::BufferUsage::Uniform, bufferSize);
        EXPECT_EQ(native::GetAllocatedSizeForTesting(buffer.Get()),
                  Align(bufferSize, requiredBufferAlignment));
    }

    // Test uniform usage and with size just above requiredBufferAlignment allocates to the next
    // multiple of |requiredBufferAlignment|
    {
        const uint32_t bufferSize = std::max(1u + requiredBufferAlignment, kMinBufferSize);
        wgpu::Buffer buffer =
            CreateBuffer(wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage, bufferSize);
        EXPECT_EQ(native::GetAllocatedSizeForTesting(buffer.Get()),
                  Align(bufferSize, requiredBufferAlignment));
    }

    // Test uniform usage and another usage
    {
        const uint32_t bufferSize = kMinBufferSize;
        wgpu::Buffer buffer =
            CreateBuffer(wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage, bufferSize);
        EXPECT_EQ(native::GetAllocatedSizeForTesting(buffer.Get()),
                  Align(bufferSize, requiredBufferAlignment));
    }
}

DAWN_INSTANTIATE_TEST(BufferAllocatedSizeTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
