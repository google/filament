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

class AdapterPropertiesD3DTest : public DawnTest {};

// TODO(dawn:2257) test that is is invalid to request AdapterPropertiesD3D if the
// feature is not available.

// Test that it is possible to query the d3d properties, and it is populated with a valid data.
TEST_P(AdapterPropertiesD3DTest, GetD3DProperties) {
    DAWN_TEST_UNSUPPORTED_IF(!adapter.HasFeature(wgpu::FeatureName::AdapterPropertiesD3D));
    {
        wgpu::AdapterInfo info;
        wgpu::AdapterPropertiesD3D d3dProperties;
        info.nextInChain = &d3dProperties;

        adapter.GetInfo(&info);

        // This is the minimum D3D shader model Dawn supports.
        EXPECT_GE(d3dProperties.shaderModel, 50u);
    }
    {
        wgpu::AdapterInfo adapterInfo;
        wgpu::AdapterPropertiesD3D d3dProperties;
        adapterInfo.nextInChain = &d3dProperties;

        device.GetAdapterInfo(&adapterInfo);

        // This is the minimum D3D shader model Dawn supports.
        EXPECT_GE(d3dProperties.shaderModel, 50u);
    }
}

DAWN_INSTANTIATE_TEST(AdapterPropertiesD3DTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
