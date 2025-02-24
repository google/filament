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

namespace dawn {
namespace {

class MemoryAllocationStressTests : public DawnTest {};

// Test memory allocation is freed correctly when creating and destroying large buffers.
// It will consume a total of 100G of memory, 1G each time. Expect not to trigger out of memory on
// devices with gpu memory less than 100G.
TEST_P(MemoryAllocationStressTests, LargeBuffer) {
    // TODO(crbug.com/dawn/957): Memory leak on D3D12, the memory of destroyed buffer cannot be
    // released.
    DAWN_TEST_UNSUPPORTED_IF(IsD3D12());

    // TODO(crbug.com/dawn/957): Check whether it can be reproduced on each backend.
    DAWN_TEST_UNSUPPORTED_IF(IsD3D11() || IsMetal() || IsOpenGL() || IsOpenGLES() || IsVulkan());

    uint32_t count = 100;
    for (uint32_t i = 0; i < count; i++) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 1024 * 1024 * 1024;  // 1G
        descriptor.usage = wgpu::BufferUsage::Storage;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        buffer.Destroy();
    }
}

DAWN_INSTANTIATE_TEST(MemoryAllocationStressTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
