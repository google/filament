// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/tests/DawnTest.h"

namespace dawn {
namespace {

class MemoryHeapPropertiesTest : public DawnTest {
  protected:
    void CheckMemoryHeapProperties(const wgpu::AdapterPropertiesMemoryHeaps& memoryHeapProperties) {
        EXPECT_GT(memoryHeapProperties.heapCount, 0u);
        for (size_t i = 0; i < memoryHeapProperties.heapCount; ++i) {
            // Check the heap is non-zero in size.
            EXPECT_GT(memoryHeapProperties.heapInfo[i].size, 0ull);

            constexpr wgpu::HeapProperty kValidProps =
                wgpu::HeapProperty::DeviceLocal | wgpu::HeapProperty::HostVisible |
                wgpu::HeapProperty::HostCoherent | wgpu::HeapProperty::HostUncached |
                wgpu::HeapProperty::HostCached;

            // Check the heap properties only contain the set of valid enums.
            EXPECT_EQ(memoryHeapProperties.heapInfo[i].properties & ~kValidProps, 0u);

            // Check the heap properties have at least one bit.
            EXPECT_NE(uint32_t(memoryHeapProperties.heapInfo[i].properties), 0u);
        }
    }
};

// TODO(dawn:2257) test that is is invalid to request AdapterPropertiesMemoryHeaps if the
// feature is not available.

// Test that it is possible to query the memory, and it is populated with valid enums.
TEST_P(MemoryHeapPropertiesTest, GetMemoryHeapProperties) {
    DAWN_TEST_UNSUPPORTED_IF(!adapter.HasFeature(wgpu::FeatureName::AdapterPropertiesMemoryHeaps));
    {
        wgpu::AdapterInfo info;
        wgpu::AdapterPropertiesMemoryHeaps memoryHeapProperties;
        info.nextInChain = &memoryHeapProperties;

        adapter.GetInfo(&info);
        CheckMemoryHeapProperties(memoryHeapProperties);
    }
    {
        wgpu::AdapterInfo adapterInfo;
        wgpu::AdapterPropertiesMemoryHeaps memoryHeapProperties;
        adapterInfo.nextInChain = &memoryHeapProperties;

        device.GetAdapterInfo(&adapterInfo);
        CheckMemoryHeapProperties(memoryHeapProperties);
    }
}

DAWN_INSTANTIATE_TEST(MemoryHeapPropertiesTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
