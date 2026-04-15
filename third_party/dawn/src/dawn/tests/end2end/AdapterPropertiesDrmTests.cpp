// Copyright 2026 The Dawn & Tint Authors
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

class AdapterPropertiesDrmTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!IsLinux());
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::AdapterPropertiesDrm));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::AdapterPropertiesDrm})) {
            return {wgpu::FeatureName::AdapterPropertiesDrm};
        }
        return {};
    }
};

TEST_P(AdapterPropertiesDrmTest, GetDrmProperties) {
    {
        wgpu::AdapterInfo info;
        wgpu::AdapterPropertiesDrm drmProperties;
        info.nextInChain = &drmProperties;

        EXPECT_EQ(adapter.GetInfo(&info), wgpu::Status::Success);

        if (drmProperties.hasPrimary) {
            EXPECT_NE(drmProperties.primaryMajor, 0u);
        }
        if (drmProperties.hasRender) {
            EXPECT_NE(drmProperties.renderMajor, 0u);
        }

        if (!drmProperties.hasPrimary) {
            EXPECT_EQ(drmProperties.primaryMajor, 0u);
            EXPECT_EQ(drmProperties.primaryMinor, 0u);
        }
        if (!drmProperties.hasRender) {
            EXPECT_EQ(drmProperties.renderMajor, 0u);
            EXPECT_EQ(drmProperties.renderMinor, 0u);
        }
    }
    {
        wgpu::AdapterInfo adapterInfo;
        wgpu::AdapterPropertiesDrm drmProperties;
        adapterInfo.nextInChain = &drmProperties;

        EXPECT_EQ(adapter.GetInfo(&adapterInfo), wgpu::Status::Success);

        if (drmProperties.hasPrimary) {
            EXPECT_NE(drmProperties.primaryMajor, 0u);
        }
        if (drmProperties.hasRender) {
            EXPECT_NE(drmProperties.renderMajor, 0u);
        }

        if (!drmProperties.hasPrimary) {
            EXPECT_EQ(drmProperties.primaryMajor, 0u);
            EXPECT_EQ(drmProperties.primaryMinor, 0u);
        }
        if (!drmProperties.hasRender) {
            EXPECT_EQ(drmProperties.renderMajor, 0u);
            EXPECT_EQ(drmProperties.renderMinor, 0u);
        }
    }
}

DAWN_INSTANTIATE_TEST(AdapterPropertiesDrmTest, VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
