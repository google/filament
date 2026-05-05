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

// Tests for DawnTestBase infrastructure: SupportsFeatures and HandleDeviceCreationFailure.

#include <vector>

#include "dawn/tests/DawnTest.h"

namespace dawn {
namespace {

// Tests for SupportsFeatures: verifies that SupportsFeatures correctly reports feature
// availability.
class DawnTestInfraTest : public DawnTest {};

// Verify SupportsFeatures returns false for a fabricated feature that no adapter supports.
TEST_P(DawnTestInfraTest, UnsupportedFeatureReturnsFalse) {
    auto fakeFeature = static_cast<wgpu::FeatureName>(0x7FFFFFFF);
    EXPECT_FALSE(SupportsFeatures({fakeFeature}));
}

// Verify SupportsFeatures returns true with CoreFeaturesAndLimits. It should works in
// both wire and non-wire modes.
TEST_P(DawnTestInfraTest, SupportedFeatureReturnsTrue) {
    EXPECT_TRUE(SupportsFeatures({wgpu::FeatureName::CoreFeaturesAndLimits}));
}

// Verify SupportsFeatures returns false in wire mode with DawnNative. All native adapters
// should support it but the wire explicitly filters out (see dawn/wire/SupportedFeatures.cpp).
TEST_P(DawnTestInfraTest, WireUnsupportedFeatureReturnsFalse) {
    DAWN_TEST_UNSUPPORTED_IF(!UsesWire());
    EXPECT_FALSE(SupportsFeatures({wgpu::FeatureName::DawnNative}));
}

DAWN_INSTANTIATE_TEST(DawnTestInfraTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend());

// Tests for HandleDeviceCreationFailure: verifies that requesting an unsupported feature causes the
// test to be gracefully skipped (not hard-failed).
class UnsupportedFeatureSkipsTest : public DawnTest {
  protected:
    // Request a fabricated feature that no adapter supports. This causes device creation to fail,
    // and HandleDeviceCreationFailure should skip the test gracefully.
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {static_cast<wgpu::FeatureName>(0x7FFFFFFF)};
    }
};

// If HandleDeviceCreationFailure correctly skips, this test body is never reached, and the test
// shows as SKIPPED. If it incorrectly hard-fails, the test shows as FAILED.
TEST_P(UnsupportedFeatureSkipsTest, SkipsGracefully) {
    // This should never execute - device creation should have failed and the test should be
    // skipped in SetUp via HandleDeviceCreationFailure.
    GTEST_FAIL() << "Test body should not be reached; device creation with unsupported feature "
                    "should have caused the test to be skipped.";
}

DAWN_INSTANTIATE_TEST(UnsupportedFeatureSkipsTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
