// Copyright 2019 The Dawn & Tint Authors
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

#include <array>
#include <vector>

#include "dawn/tests/DawnTest.h"

namespace dawn {
namespace {

class NonzeroBufferCreationTests : public DawnTest {};

// Verify that each byte of the buffer has all been initialized to 1 with the toggle enabled when it
// is created with CopyDst usage.
TEST_P(NonzeroBufferCreationTests, BufferCreationWithCopyDstUsage) {
    constexpr uint32_t kSize = 32u;

    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint8_t> expectedData(kSize, uint8_t(1u));
    EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<uint32_t*>(expectedData.data()), buffer, 0,
                               kSize / sizeof(uint32_t));
}

// Verify that each byte of the buffer has all been initialized to 1 with the toggle enabled when it
// is created with MapWrite without CopyDst usage.
TEST_P(NonzeroBufferCreationTests, BufferCreationWithMapWriteWithoutCopyDstUsage) {
    constexpr uint32_t kSize = 32u;

    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;

    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint8_t> expectedData(kSize, uint8_t(1u));
    EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<uint32_t*>(expectedData.data()), buffer, 0,
                               kSize / sizeof(uint32_t));
}

// Verify that each byte of the buffer has all been initialized to 1 with the toggle enabled when
// it is created with mappedAtCreation == true.
TEST_P(NonzeroBufferCreationTests, BufferCreationWithMappedAtCreation) {
    // When we use Dawn wire, the lazy initialization of the buffers with mappedAtCreation == true
    // are done in the Dawn wire and we don't plan to get it work with the toggle
    // "nonzero_clear_resources_on_creation_for_testing" (we will have more tests on it in the
    // BufferZeroInitTests.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    constexpr uint32_t kSize = 32u;

    wgpu::BufferDescriptor defaultDescriptor;
    defaultDescriptor.size = kSize;
    defaultDescriptor.mappedAtCreation = true;

    const std::vector<uint8_t> expectedData(kSize, uint8_t(1u));
    const uint32_t* expectedDataPtr = reinterpret_cast<const uint32_t*>(expectedData.data());

    // Buffer with MapRead usage
    {
        wgpu::BufferDescriptor descriptor = defaultDescriptor;
        descriptor.usage = wgpu::BufferUsage::MapRead;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        const uint8_t* mappedData = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
        EXPECT_EQ(0, memcmp(mappedData, expectedData.data(), kSize));
        buffer.Unmap();

        MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, kSize);
        mappedData = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
        EXPECT_EQ(0, memcmp(mappedData, expectedData.data(), kSize));
        buffer.Unmap();
    }

    // Buffer with MapWrite usage
    {
        wgpu::BufferDescriptor descriptor = defaultDescriptor;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        const uint8_t* mappedData = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
        EXPECT_EQ(0, memcmp(mappedData, expectedData.data(), kSize));
        buffer.Unmap();

        EXPECT_BUFFER_U32_RANGE_EQ(expectedDataPtr, buffer, 0, kSize / sizeof(uint32_t));
    }

    // Buffer with neither MapRead nor MapWrite usage
    {
        wgpu::BufferDescriptor descriptor = defaultDescriptor;
        descriptor.usage = wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        const uint8_t* mappedData = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
        EXPECT_EQ(0, memcmp(mappedData, expectedData.data(), kSize));
        buffer.Unmap();

        EXPECT_BUFFER_U32_RANGE_EQ(expectedDataPtr, buffer, 0, kSize / sizeof(uint32_t));
    }
}

DAWN_INSTANTIATE_TEST(NonzeroBufferCreationTests,
                      D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                   {"lazy_clear_resource_on_first_use"}),
                      D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                   {"lazy_clear_resource_on_first_use"}),
                      MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                   {"lazy_clear_resource_on_first_use"}),
                      OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                    {"lazy_clear_resource_on_first_use"}),
                      OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                      VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                    {"lazy_clear_resource_on_first_use"}));

}  // anonymous namespace
}  // namespace dawn
