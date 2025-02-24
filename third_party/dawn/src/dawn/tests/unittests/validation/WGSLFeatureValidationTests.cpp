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

#include <algorithm>
#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"
#include "dawn/utils/WireHelper.h"

namespace dawn {
namespace {

class WGSLFeatureValidationTest : public ValidationTest {
  protected:
    struct InstanceSpec {
        bool useTestingFeatures = true;
        bool allowUnsafeAPIs = false;
        bool exposeExperimental = false;
        std::vector<const char*> blocklist = {};
    };

    // Override the default SetUp to do nothing since the tests will call SetUp(InstanceSpec)
    // explicitly each time.
    void SetUp() override {}
    void SetUp(InstanceSpec spec) {
        // The blocklist that will be shared between both the native and wire descriptors.
        wgpu::DawnWGSLBlocklist blocklist;
        blocklist.blocklistedFeatureCount = spec.blocklist.size();
        blocklist.blocklistedFeatures = spec.blocklist.data();

        // Build the native instance descriptor.
        std::vector<const char*> enabledToggles;
        if (spec.useTestingFeatures) {
            enabledToggles.push_back("expose_wgsl_testing_features");
        }
        if (spec.allowUnsafeAPIs) {
            enabledToggles.push_back("allow_unsafe_apis");
        }
        if (spec.exposeExperimental) {
            enabledToggles.push_back("expose_wgsl_experimental_features");
        }

        wgpu::DawnTogglesDescriptor togglesDesc;
        togglesDesc.nextInChain = &blocklist;
        togglesDesc.enabledToggleCount = enabledToggles.size();
        togglesDesc.enabledToggles = enabledToggles.data();

        wgpu::InstanceDescriptor nativeDesc;
        nativeDesc.nextInChain = &togglesDesc;

        // Build the wire instance descriptor.
        wgpu::DawnWireWGSLControl wgslControl;
        wgslControl.nextInChain = &blocklist;
        wgslControl.enableExperimental = spec.allowUnsafeAPIs || spec.exposeExperimental;
        wgslControl.enableUnsafe = spec.allowUnsafeAPIs;
        wgslControl.enableTesting = spec.useTestingFeatures;

        wgpu::InstanceDescriptor wireDesc;
        wireDesc.nextInChain = &wgslControl;

        ValidationTest::SetUp(&nativeDesc, &wireDesc);
    }
};

wgpu::WGSLLanguageFeatureName kNonExistentFeature = {};

// Check HasFeature for an Instance that doesn't have unsafe APIs.
TEST_F(WGSLFeatureValidationTest, HasFeatureDefaultInstance) {
    SetUp({});

    // Shipped features are present.
    ASSERT_TRUE(
        instance.HasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName::ChromiumTestingShipped));
    ASSERT_TRUE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingShippedWithKillswitch));

    // Experimental and unimplemented features are not present.
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingExperimental));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnsafeExperimental));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnimplemented));

    // Non-existent features are not present.
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(kNonExistentFeature));
}

// Check HasFeature for an Instance that has unsafe APIs.
TEST_F(WGSLFeatureValidationTest, HasFeatureExposeExperimental) {
    SetUp({.exposeExperimental = true});

    // Shipped and experimental features are present.
    ASSERT_TRUE(
        instance.HasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName::ChromiumTestingShipped));
    ASSERT_TRUE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingShippedWithKillswitch));
    ASSERT_TRUE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingExperimental));

    // Unsafe and unimplemented features are not present.
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnsafeExperimental));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnimplemented));

    // Non-existent features are not present.
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(kNonExistentFeature));
}

// Check HasFeature for an Instance that has unsafe APIs.
TEST_F(WGSLFeatureValidationTest, HasFeatureAllowUnsafeInstance) {
    SetUp({.allowUnsafeAPIs = true});

    // Shipped and experimental features are present.
    ASSERT_TRUE(
        instance.HasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName::ChromiumTestingShipped));
    ASSERT_TRUE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingShippedWithKillswitch));
    ASSERT_TRUE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingExperimental));
    ASSERT_TRUE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnsafeExperimental));

    // Unimplemented features are not present.
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnimplemented));

    // Non-existent features are not present.
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(kNonExistentFeature));
}

// Check HasFeature for an Instance that doesn't have the expose_wgsl_testing_features toggle.
TEST_F(WGSLFeatureValidationTest, HasFeatureWithoutExposeWGSLTestingFeatures) {
    SetUp({.useTestingFeatures = false});

    // None of the testing features are present.
    ASSERT_FALSE(
        instance.HasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName::ChromiumTestingShipped));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingShippedWithKillswitch));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingExperimental));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnsafeExperimental));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnimplemented));
}

// Tests for the behavior of getting WGSL language features.
TEST_F(WGSLFeatureValidationTest, GetFeatures) {
    SetUp({});

    wgpu::SupportedWGSLLanguageFeatures supportedFeatures = {};
    ASSERT_EQ(wgpu::Status::Success, instance.GetWGSLLanguageFeatures(&supportedFeatures));
    ASSERT_NE(0u, supportedFeatures.featureCount);
    const wgpu::WGSLLanguageFeatureName* features = supportedFeatures.features;

    // Exactly featureCount features should be written, and all return true in HasWGSLFeature.
    for (size_t i = 0; i < supportedFeatures.featureCount; i++) {
        ASSERT_TRUE(instance.HasWGSLLanguageFeature(features[i]));
    }

    // Test the presence / absence of some known testing features.
    const wgpu::WGSLLanguageFeatureName* begin = features;
    const wgpu::WGSLLanguageFeatureName* end = features + supportedFeatures.featureCount;
    ASSERT_NE(std::find(begin, end, wgpu::WGSLLanguageFeatureName::ChromiumTestingShipped), end);
    ASSERT_NE(
        std::find(begin, end, wgpu::WGSLLanguageFeatureName::ChromiumTestingShippedWithKillswitch),
        end);

    ASSERT_EQ(std::find(begin, end, wgpu::WGSLLanguageFeatureName::ChromiumTestingUnimplemented),
              end);
    ASSERT_EQ(
        std::find(begin, end, wgpu::WGSLLanguageFeatureName::ChromiumTestingUnsafeExperimental),
        end);
    ASSERT_EQ(std::find(begin, end, wgpu::WGSLLanguageFeatureName::ChromiumTestingExperimental),
              end);
}

// Check that the enabled / disabled features are used to validate the WGSL shaders.
TEST_F(WGSLFeatureValidationTest, UsingFeatureInShaderModule) {
    SetUp({});

    utils::CreateShaderModule(device, R"(
        requires chromium_testing_shipped;
    )");
    utils::CreateShaderModule(device, R"(
        requires chromium_testing_shipped_with_killswitch;
    )");

    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        requires chromium_testing_unimplemented;
    )"));
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        requires chromium_testing_unsafe_experimental;
    )"));
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        requires chromium_testing_experimental;
    )"));
}

// Test using DawnWGSLBlocklist to block features with a killswitch by name.
TEST_F(WGSLFeatureValidationTest, BlockListOfKillswitchedFeatures) {
    SetUp({.allowUnsafeAPIs = true, .blocklist = {"chromium_testing_shipped_with_killswitch"}});

    // The blocklisted feature is not present.
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingShippedWithKillswitch));

    // The others are.
    ASSERT_TRUE(
        instance.HasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName::ChromiumTestingShipped));
    ASSERT_TRUE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingExperimental));
    ASSERT_TRUE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnsafeExperimental));

    // Using the blocklisted extension fails.
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        requires chromium_testing_shipped_with_killswitch;
    )"));
}

// Test that DawnWGSLBlocklist can block any feature name (even without a killswitch).
TEST_F(WGSLFeatureValidationTest, BlockListOfAnyFeature) {
    SetUp({.allowUnsafeAPIs = true,
           .blocklist = {"chromium_testing_shipped", "chromium_testing_experimental",
                         "chromium_testing_unsafe_experimental"}});

    // All blocklisted features aren't present.
    ASSERT_FALSE(
        instance.HasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName::ChromiumTestingShipped));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingExperimental));
    ASSERT_FALSE(instance.HasWGSLLanguageFeature(
        wgpu::WGSLLanguageFeatureName::ChromiumTestingUnsafeExperimental));
}

// Test that DawnWGSLBlocklist can contain garbage names without causing problems.
TEST_F(WGSLFeatureValidationTest, BlockListGarbageName) {
    SetUp({.blocklist = {"LE_GARBAGE"}});
    ASSERT_NE(instance, nullptr);
}

}  // anonymous namespace
}  // namespace dawn
