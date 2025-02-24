// Copyright 2024 The Dawn & Tint Authors
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

class AdapterPropertiesVkTest : public DawnTest {};

// TODO(dawn:2257) test that is is invalid to request AdapterPropertiesVk if the
// feature is not available.

// Test that it is possible to query the Vulkan properties, and it is populated with a valid data.
TEST_P(AdapterPropertiesVkTest, GetVkProperties) {
    DAWN_TEST_UNSUPPORTED_IF(!adapter.HasFeature(wgpu::FeatureName::AdapterPropertiesVk));
    {
        wgpu::AdapterInfo info;
        wgpu::AdapterPropertiesVk vkProperties;
        info.nextInChain = &vkProperties;

        adapter.GetInfo(&info);

        // The driver version should be set to something but it depends on the hardware.
        EXPECT_NE(vkProperties.driverVersion, 0u);
    }
    {
        wgpu::AdapterInfo adapterInfo;
        wgpu::AdapterPropertiesVk vkProperties;
        adapterInfo.nextInChain = &vkProperties;

        device.GetAdapterInfo(&adapterInfo);

        // The driver version should be set to something but it depends on the hardware.
        EXPECT_NE(vkProperties.driverVersion, 0u);
    }
}

DAWN_INSTANTIATE_TEST(AdapterPropertiesVkTest, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
