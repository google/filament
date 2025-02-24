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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class CommandEncoderTests : public DawnTest {};

// Tests WriteBuffer commands on CommandEncoder.
TEST_P(CommandEncoderTests, WriteBuffer) {
    wgpu::Buffer bufferA = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc, {0, 0, 0, 0});
    wgpu::Buffer bufferB = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc, {0, 0, 0, 0});
    wgpu::Buffer bufferC = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc, {0, 0, 0, 0});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    const uint32_t kData1 = 1;
    encoder.WriteBuffer(bufferA, 0, reinterpret_cast<const uint8_t*>(&kData1), sizeof(kData1));
    encoder.CopyBufferToBuffer(bufferA, 0, bufferB, sizeof(uint32_t), 3 * sizeof(uint32_t));

    const uint32_t kData2 = 2;
    encoder.WriteBuffer(bufferB, 0, reinterpret_cast<const uint8_t*>(&kData2), sizeof(kData2));
    encoder.CopyBufferToBuffer(bufferB, 0, bufferC, sizeof(uint32_t), 3 * sizeof(uint32_t));

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(0, bufferC, 0);
    EXPECT_BUFFER_U32_EQ(2, bufferC, sizeof(uint32_t));
    EXPECT_BUFFER_U32_EQ(1, bufferC, 2 * sizeof(uint32_t));
    EXPECT_BUFFER_U32_EQ(0, bufferC, 3 * sizeof(uint32_t));
}

DAWN_INSTANTIATE_TEST(CommandEncoderTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
