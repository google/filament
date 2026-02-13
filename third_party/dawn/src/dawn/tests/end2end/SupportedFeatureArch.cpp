// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/common/GPUInfo.h"
#include "dawn/tests/DawnTest.h"

// The purpose of these tests is to prevent regressions of features and limits on architectures that
// are known to have these supported capabilities.
//
// Why? Because of the nature of feature/limit support in WebGPU we can easily accidentally drop
// (regress) optional capabilities for specific devices and dawn/CTS testing machinery and even
// external webgpu clients will respond gracefully. We (dawn) never want to silently regress
// features on critical platforms. If we are regressing it should be a deliberate decision which may
// involve changes to the tests below.
//
// How? Currently it is difficult to know our features/limit support for all hardware (could even be
// driver version dependent). So here we conservatively assert that specific architectures have
// specific features/limits.
//
// When? This file may need to be updated with new conservative tests if the CQ changes hardware or
// if we have deliberately chosen to regress specific features/limits on some devices.
//
// Not an official dawn/webgpu resource but this page can provide insights into features/limits:
// https://web3dsurvey.com/webgpu

namespace dawn {
namespace {

using FeatureArchInfoTestBase = DawnTestWithParams<>;

class FeatureArchInfoTest_MaxLimits : public FeatureArchInfoTestBase {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }
};

TEST_P(FeatureArchInfoTest_MaxLimits, SubgroupsSupported) {
    const bool subgroupSupportExpected =
        // All Apple Silicon
        gpu_info::IsApple(GetParam().adapterProperties.vendorID) ||
        // All Intel >Gen9 on Metal/Vulkan, and D3D12+DXC
        ((IsMetal() || IsVulkan() || IsDXC()) &&
         gpu_info::IsIntel(GetParam().adapterProperties.vendorID) &&
         !gpu_info::IsIntelGen9(GetParam().adapterProperties.vendorID,
                                GetParam().adapterProperties.deviceID));

    DAWN_TEST_UNSUPPORTED_IF(!subgroupSupportExpected);
    EXPECT_TRUE(this->SupportsFeatures({wgpu::FeatureName::Subgroups}));
}

TEST_P(FeatureArchInfoTest_MaxLimits, ShaderF16Supported) {
    // All apple silicon devices support f16
    const bool shaderF16SupportExpected = gpu_info::IsApple(GetParam().adapterProperties.vendorID);

    DAWN_TEST_UNSUPPORTED_IF(!shaderF16SupportExpected);
    EXPECT_TRUE(this->SupportsFeatures({wgpu::FeatureName::ShaderF16}));
}

TEST_P(FeatureArchInfoTest_MaxLimits, WorkgroupSizeMin1024Expected) {
    // All apple silicon devices are known to have 1k workgroup size limit.
    // This code will need to be changed if a new apple silicon device is released with a different
    // limit. Nvidia also support >= 1k workgroups.
    const bool isWorkgroupSizeGT1k = gpu_info::IsApple(GetParam().adapterProperties.vendorID) ||
                                     gpu_info::IsNvidia(GetParam().adapterProperties.vendorID);
    DAWN_TEST_UNSUPPORTED_IF(!isWorkgroupSizeGT1k);

    EXPECT_GE(GetAdapterLimits().maxComputeInvocationsPerWorkgroup, 1024u);
    // Check that the device was created with the requested limit.
    EXPECT_EQ(GetSupportedLimits().maxComputeInvocationsPerWorkgroup,
              GetAdapterLimits().maxComputeInvocationsPerWorkgroup);
}

class FeatureArchInfoTest_TieredMaxLimits : public FeatureArchInfoTestBase {
  protected:
    bool GetRequireUseTieredLimits() override { return true; }
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }
};

TEST_P(FeatureArchInfoTest_TieredMaxLimits, D3DHighMaxVertexAttributes) {
    const bool isWindowsHighEnd =
        gpu_info::IsNvidia(GetParam().adapterProperties.vendorID) && (IsD3D11() || IsD3D12());
    DAWN_TEST_UNSUPPORTED_IF(!isWindowsHighEnd);

    // High-end windows desktop GPU should report at least 30 even when tiered is enabled
    // See crbug.com/430371785
    EXPECT_GE(GetAdapterLimits().maxVertexAttributes, 30u);
}

TEST_P(FeatureArchInfoTest_TieredMaxLimits, AppleMaxTextureDimension2D) {
    const bool isAppleSilicon = gpu_info::IsApple(GetParam().adapterProperties.vendorID);
    DAWN_TEST_UNSUPPORTED_IF(!isAppleSilicon);

    // Apple silicon should report > 16k. Most modern devices should support > 16k. Update this when
    // appropriate.
    EXPECT_GE(GetAdapterLimits().maxTextureDimension2D, 16384u);
}

TEST_P(FeatureArchInfoTest_TieredMaxLimits, MaxUniformBufferBindingSize) {
    // Swiftshader will return a lower limit than any modern device on CQ.
    DAWN_TEST_UNSUPPORTED_IF(!IsSwiftshader());
    EXPECT_GE(GetAdapterLimits().maxUniformBufferBindingSize, 65536u);
    // Check that the device was created with the requested limit.
    EXPECT_EQ(GetSupportedLimits().maxUniformBufferBindingSize,
              GetAdapterLimits().maxUniformBufferBindingSize);
}

TEST_P(FeatureArchInfoTest_MaxLimits, MinUniformBufferOffsetAlignment) {
    // All devices on CQ report 256. A value of 32 is commonplace on iOS (note the less than).
    EXPECT_LE(GetAdapterLimits().minUniformBufferOffsetAlignment, 256u);
    // Check that the device was created with the requested limit.
    EXPECT_EQ(GetSupportedLimits().minUniformBufferOffsetAlignment,
              GetAdapterLimits().minUniformBufferOffsetAlignment);
}

DAWN_INSTANTIATE_TEST(FeatureArchInfoTest_MaxLimits,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(FeatureArchInfoTest_TieredMaxLimits,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
