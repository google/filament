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

#include <utility>
#include <vector>

#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"

namespace dawn {
namespace {

using testing::_;
using testing::EmptySizedString;
using testing::InSequence;
using testing::IsNull;
using testing::MockCppCallback;
using testing::NonEmptySizedString;
using testing::NotNull;
using testing::WithArgs;

class RequestDeviceValidationTest : public ValidationTest {
  protected:
    using MockDeviceLostCallback = MockCppCallback<wgpu::DeviceLostCallback<void>*>;

    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }

    MockCppCallback<wgpu::RequestDeviceCallback<void>*> mRequestDeviceCallback;
};

// Test that requesting a device without specifying limits is valid.
TEST_F(RequestDeviceValidationTest, NoRequiredLimits) {
    wgpu::DeviceDescriptor descriptor;

    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([](wgpu::Device device) {
            // Check one of the default limits.
            wgpu::Limits limits;
            device.GetLimits(&limits);
            EXPECT_EQ(limits.maxBindGroups, 4u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

// Test that requesting a device with the default limits is valid.
TEST_F(RequestDeviceValidationTest, DefaultLimits) {
    wgpu::Limits limits = {};
    wgpu::DeviceDescriptor descriptor;
    descriptor.requiredLimits = &limits;

    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([](wgpu::Device device) {
            // Check one of the default limits.
            wgpu::Limits limits;
            device.GetLimits(&limits);
            EXPECT_EQ(limits.maxTextureArrayLayers, 256u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

// Test that requesting a device where a required limit is above the maximum value.
TEST_F(RequestDeviceValidationTest, HigherIsBetter) {
    wgpu::Limits limits = {};
    wgpu::DeviceDescriptor descriptor;
    descriptor.requiredLimits = &limits;

    wgpu::Limits supportedLimits;
    EXPECT_EQ(adapter.GetLimits(&supportedLimits), wgpu::Status::Success);

    // If we can support better than the default, test below the max.
    if (supportedLimits.maxBindGroups > 4u) {
        limits.maxBindGroups = supportedLimits.maxBindGroups - 1;
        EXPECT_CALL(mRequestDeviceCallback,
                    Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
            .WillOnce(WithArgs<1>([&](wgpu::Device device) {
                wgpu::Limits limits;
                device.GetLimits(&limits);

                // Check we got exactly the request.
                EXPECT_EQ(limits.maxBindGroups, supportedLimits.maxBindGroups - 1);
                // Check another default limit.
                EXPECT_EQ(limits.maxTextureArrayLayers, 256u);
            }));
        adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                              mRequestDeviceCallback.Callback());
    }

    // Test the max.
    limits.maxBindGroups = supportedLimits.maxBindGroups;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([&](wgpu::Device device) {
            wgpu::Limits limits;
            device.GetLimits(&limits);

            // Check we got exactly the request.
            EXPECT_EQ(limits.maxBindGroups, supportedLimits.maxBindGroups);
            // Check another default limit.
            EXPECT_EQ(limits.maxTextureArrayLayers, 256u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());

    // Test above the max.
    limits.maxBindGroups = supportedLimits.maxBindGroups + 1;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Error, IsNull(), NonEmptySizedString()))
        .Times(1);
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());

    // Test worse than the default
    limits.maxBindGroups = 3u;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([&](wgpu::Device device) {
            wgpu::Limits limits;
            device.GetLimits(&limits);

            // Check we got the default.
            EXPECT_EQ(limits.maxBindGroups, 4u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

// Test that requesting a device where a required limit is below the minimum value.
TEST_F(RequestDeviceValidationTest, LowerIsBetter) {
    wgpu::Limits limits = {};
    wgpu::DeviceDescriptor descriptor;
    descriptor.requiredLimits = &limits;

    wgpu::Limits supportedLimits;
    EXPECT_EQ(adapter.GetLimits(&supportedLimits), wgpu::Status::Success);

    // Test below the min.
    limits.minUniformBufferOffsetAlignment = supportedLimits.minUniformBufferOffsetAlignment / 2;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Error, IsNull(), NonEmptySizedString()))
        .Times(1);
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());

    // Test the min.
    limits.minUniformBufferOffsetAlignment = supportedLimits.minUniformBufferOffsetAlignment;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([&](wgpu::Device device) {
            wgpu::Limits limits;
            device.GetLimits(&limits);

            // Check we got exactly the request.
            EXPECT_EQ(limits.minUniformBufferOffsetAlignment,
                      supportedLimits.minUniformBufferOffsetAlignment);
            // Check another default limit.
            EXPECT_EQ(limits.maxTextureArrayLayers, 256u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());

    // IF we can support better than the default, test above the min.
    if (supportedLimits.minUniformBufferOffsetAlignment > 256u) {
        limits.minUniformBufferOffsetAlignment =
            supportedLimits.minUniformBufferOffsetAlignment * 2;
        EXPECT_CALL(mRequestDeviceCallback,
                    Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
            .WillOnce(WithArgs<1>([&](wgpu::Device device) {
                wgpu::Limits limits;
                device.GetLimits(&limits);

                // Check we got exactly the request.
                EXPECT_EQ(limits.minUniformBufferOffsetAlignment,
                          supportedLimits.minUniformBufferOffsetAlignment * 2);
                // Check another default limit.
                EXPECT_EQ(limits.maxTextureArrayLayers, 256u);
            }));
        adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                              mRequestDeviceCallback.Callback());
    }

    // Test worse than the default
    limits.minUniformBufferOffsetAlignment = 2u * 256u;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([&](wgpu::Device device) {
            wgpu::Limits limits;
            device.GetLimits(&limits);

            // Check we got the default.
            EXPECT_EQ(limits.minUniformBufferOffsetAlignment, 256u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

// Test that if an error occurs when requesting a device, the device lost callback is called
// appropriately.
TEST_F(RequestDeviceValidationTest, ErrorTriggersDeviceLost) {
    // Invalid descriptor chains:
    //   - ChainedStruct: This should cause an early validation error.
    //   - DawnFakeDeviceInitializeErrorForTesting: This should cause an internal device error.
    wgpu::ChainedStruct chain1;
    wgpu::DawnFakeDeviceInitializeErrorForTesting chain2;
    std::array<wgpu::ChainedStruct*, 2> chains = {&chain1, &chain2};

    for (const auto* chain : chains) {
        SCOPED_TRACE(absl::StrFormat("Chain SType: %s", chain->sType));
        {
            wgpu::DeviceDescriptor descriptor;
            descriptor.nextInChain = chain;

            // Device lost callback mode: AllowSpontaneous.
            MockDeviceLostCallback lostCb;
            descriptor.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
                                             lostCb.Callback());

            // When in spontaneous mode, the request device callback should fire immediately before
            // the device lost callback.
            InSequence s;
            EXPECT_CALL(mRequestDeviceCallback,
                        Call(wgpu::RequestDeviceStatus::Error, IsNull(), NonEmptySizedString()))
                .Times(1);
            EXPECT_CALL(lostCb, Call(_, wgpu::DeviceLostReason::FailedCreation, _)).Times(1);
            adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                                  mRequestDeviceCallback.Callback());
        }
        {
            wgpu::DeviceDescriptor descriptor;
            descriptor.nextInChain = chain;

            // Device lost callback mode: AllowProcessEvents.
            MockDeviceLostCallback lostCb;
            descriptor.SetDeviceLostCallback(wgpu::CallbackMode::AllowProcessEvents,
                                             lostCb.Callback());

            EXPECT_CALL(mRequestDeviceCallback,
                        Call(wgpu::RequestDeviceStatus::Error, IsNull(), NonEmptySizedString()))
                .Times(1);
            adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                                  mRequestDeviceCallback.Callback());

            // When in a non-spontaneous mode for the device lost, the request device callback
            // should fire, but the device lost callback should only fire when the Instance level
            // API is called.
            EXPECT_CALL(lostCb, Call(_, wgpu::DeviceLostReason::FailedCreation, _)).Times(1);
            instance.ProcessEvents();
        }
    }
}

// Test that RG11B10UfloatRenderable is implicitly enabled when TextureFormatsTier1 is active.
TEST_F(RequestDeviceValidationTest, TextureFormatsTier1ImpliesRG11B10UfloatRenderable) {
    wgpu::DeviceDescriptor descriptor = {};
    std::vector<wgpu::FeatureName> features = {wgpu::FeatureName::TextureFormatsTier1};
    descriptor.requiredFeatures = features.data();
    descriptor.requiredFeatureCount = features.size();

    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([](wgpu::Device device) {
            EXPECT_TRUE(device.HasFeature(wgpu::FeatureName::RG11B10UfloatRenderable));
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

// Test that TextureFormatsTier1 is implicitly enabled when TextureFormatsTier2 is active.
TEST_F(RequestDeviceValidationTest, TextureFormatsTier2ImpliesTextureFormatsTier1) {
    wgpu::DeviceDescriptor descriptor = {};
    std::vector<wgpu::FeatureName> features = {wgpu::FeatureName::TextureFormatsTier2};
    descriptor.requiredFeatures = features.data();
    descriptor.requiredFeatureCount = features.size();

    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([](wgpu::Device device) {
            EXPECT_TRUE(device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

class DeviceTickValidationTest : public ValidationTest {};

// Device destroy before API-level Tick should always result in no-op and false.
TEST_F(DeviceTickValidationTest, DestroyDeviceBeforeAPITick) {
    ExpectDeviceDestruction();
    device.Destroy();
    device.Tick();
}

class DeviceGetAHardwareBufferPropertiesValidationTest : public ValidationTest {
    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }
};

// Test that calling GetAHardwareBufferProperties will generate an error
// if the required feature is not present.
TEST_F(DeviceGetAHardwareBufferPropertiesValidationTest,
       GetAHardwareBufferPropertiesRequiresAHBFeature) {
    // The parameter values shouldn't matter, as the call should fail validation
    // before calling into the implementation (verified by checking the error
    // message).
    void* handle = nullptr;
    wgpu::AHardwareBufferProperties* properties = nullptr;

    ASSERT_DEVICE_ERROR(
        device.GetAHardwareBufferProperties(handle, properties),
        testing::HasSubstr(
            "without the FeatureName::SharedTextureMemoryAHardwareBuffer feature being set"));
}

class RequestDeviceCoreValidationTest : public RequestDeviceValidationTest {
    // Create a core-defaulting adapter
    bool UseCompatibilityMode() const override { return false; }
};

// Test that requiring wgpu::FeatureName::CoreFeaturesAndLimits explicitly when calling
// RequestingDevice on a core-defaulting adapter gives a device with core limits.
TEST_F(RequestDeviceCoreValidationTest, Explicit) {
    wgpu::DeviceDescriptor descriptor = {};
    std::vector<wgpu::FeatureName> features = {wgpu::FeatureName::CoreFeaturesAndLimits};
    descriptor.requiredFeatures = features.data();
    descriptor.requiredFeatureCount = features.size();

    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([](wgpu::Device device) {
            EXPECT_TRUE(device.HasFeature(wgpu::FeatureName::CoreFeaturesAndLimits));
            // Check one of limits to be greater than compat tier.
            dawn::utils::ComboLimits limits;
            device.GetLimits(limits.GetLinked());
            EXPECT_GT(limits.maxStorageBuffersInVertexStage, 0u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

// Test that calling RequestingDevice on a core-defaulting adapter gives a device with core
// limits on default.
TEST_F(RequestDeviceCoreValidationTest, Implicit) {
    wgpu::DeviceDescriptor descriptor = {};

    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([](wgpu::Device device) {
            EXPECT_TRUE(device.HasFeature(wgpu::FeatureName::CoreFeaturesAndLimits));
            // Check one of limits to be greater than compat tier.
            dawn::utils::ComboLimits limits;
            device.GetLimits(limits.GetLinked());
            EXPECT_GT(limits.maxStorageBuffersInVertexStage, 0u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

class RequestDeviceCompatValidationTest : public RequestDeviceValidationTest {
    // Create a compat-defaulting adapter
    bool UseCompatibilityMode() const override { return true; }
};

// Test that requiring wgpu::FeatureName::CoreFeaturesAndLimits when calling RequestingDevice on a
// compat-defaulting adapter gives a device with core limits.
TEST_F(RequestDeviceCompatValidationTest, CreateCore) {
    wgpu::DeviceDescriptor descriptor = {};
    std::vector<wgpu::FeatureName> features = {wgpu::FeatureName::CoreFeaturesAndLimits};
    descriptor.requiredFeatures = features.data();
    descriptor.requiredFeatureCount = features.size();

    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([](wgpu::Device device) {
            EXPECT_TRUE(device.HasFeature(wgpu::FeatureName::CoreFeaturesAndLimits));
            // Check one of limits to be greater than compat tier.
            dawn::utils::ComboLimits limits;
            device.GetLimits(limits.GetLinked());
            EXPECT_GT(limits.maxStorageBuffersInVertexStage, 0u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

// Test that calling RequestingDevice on a compat-defaulting adapter gives a device with compat
// limits on default.
TEST_F(RequestDeviceCompatValidationTest, CreateCompat) {
    wgpu::DeviceDescriptor descriptor = {};

    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([](wgpu::Device device) {
            EXPECT_FALSE(device.HasFeature(wgpu::FeatureName::CoreFeaturesAndLimits));
            // Check one of limits to be compat tier.
            dawn::utils::ComboLimits limits;
            device.GetLimits(limits.GetLinked());
            EXPECT_EQ(limits.maxStorageBuffersInVertexStage, 0u);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

class RequestDeviceWithImmediateDataValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }

    MockCppCallback<void (*)(wgpu::RequestDeviceStatus, wgpu::Device, wgpu::StringView)>
        mRequestDeviceCallback;
};

// Test that requesting a device where a required immediate data range byte size limit is above the
// maximum value.
TEST_F(RequestDeviceWithImmediateDataValidationTest, HigherIsBetter) {
    wgpu::Limits limits = {};

    wgpu::DeviceDescriptor descriptor;
    descriptor.requiredLimits = &limits;

    wgpu::Limits supportedLimits;
    EXPECT_EQ(adapter.GetLimits(&supportedLimits), wgpu::Status::Success);

    uint32_t supportedImmediateDataLimit = supportedLimits.maxImmediateSize;

    // DeviceNull has a maxImmediateSize of 64, larger than the default of 16.
    EXPECT_GT(supportedImmediateDataLimit, kDefaultMaxImmediateDataBytes);

    // Test below the max.
    limits.maxImmediateSize = kDefaultMaxImmediateDataBytes;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([&](wgpu::Device device) {
            wgpu::Limits deviceLimits;
            device.GetLimits(&deviceLimits);
            // Check we got exactly the request.
            EXPECT_EQ(deviceLimits.maxImmediateSize, kDefaultMaxImmediateDataBytes);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());

    // Test the max.
    limits.maxImmediateSize = supportedImmediateDataLimit;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([&](wgpu::Device device) {
            wgpu::Limits deviceLimits;
            device.GetLimits(&deviceLimits);

            // Check we got exactly the request.
            EXPECT_EQ(deviceLimits.maxImmediateSize, supportedImmediateDataLimit);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());

    // Test above the max.
    limits.maxImmediateSize = supportedImmediateDataLimit + 4;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Error, IsNull(), NonEmptySizedString()))
        .Times(1);
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());

    // Test worse than the default
    limits.maxImmediateSize = kDefaultMaxImmediateDataBytes / 2;
    EXPECT_CALL(mRequestDeviceCallback,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
        .WillOnce(WithArgs<1>([&](wgpu::Device device) {
            wgpu::Limits deviceLimits;
            device.GetLimits(&deviceLimits);
            // Check we got exactly the request because it's between tier0 and tier1.
            EXPECT_EQ(deviceLimits.maxImmediateSize, kDefaultMaxImmediateDataBytes / 2);
        }));
    adapter.RequestDevice(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                          mRequestDeviceCallback.Callback());
}

}  // anonymous namespace
}  // namespace dawn
