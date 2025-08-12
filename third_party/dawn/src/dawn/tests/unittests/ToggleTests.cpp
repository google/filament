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

#include <memory>
#include <vector>

#include "dawn/dawn_proc.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using testing::Contains;
using testing::MockCallback;
using testing::NotNull;
using testing::SaveArg;
using testing::StrEq;

class ToggleTest : public testing::Test {
  public:
    // Helper function for getting the null adapter with given toggles.
    native::Adapter CreateNullAdapter(native::Instance* instance,
                                      const wgpu::DawnTogglesDescriptor* toggles) {
        wgpu::RequestAdapterOptions options;
        options.backendType = wgpu::BackendType::Null;
        options.nextInChain = toggles;

        // Enumerate null adapter, should have one result.
        auto adapters = instance->EnumerateAdapters(&options);
        EXPECT_EQ(adapters.size(), 1u);
        // Pick the returned null adapter.
        native::Adapter nullAdapter = adapters[0];
        EXPECT_NE(nullAdapter.Get(), nullptr);
        EXPECT_EQ(native::FromAPI(nullAdapter.Get())->GetPhysicalDevice()->GetBackendType(),
                  wgpu::BackendType::Null);

        return nullAdapter;
    }

  protected:
    void SetUp() override { dawnProcSetProcs(&native::GetProcs()); }

    void TearDown() override { dawnProcSetProcs(nullptr); }
};

using InstanceToggleTest = ToggleTest;

// Test that instance toggles are set by requirement or default as expected.
TEST_F(InstanceToggleTest, InstanceTogglesSet) {
    auto validateInstanceToggles = [](const native::Instance* nativeInstance,
                                      std::initializer_list<const char*> enableToggles,
                                      std::initializer_list<const char*> disableToggles) {
        const native::InstanceBase* instance = native::FromAPI(nativeInstance->Get());
        const native::TogglesState& instanceTogglesState = instance->GetTogglesState();
        std::vector<const char*> enabledToggles = instanceTogglesState.GetEnabledToggleNames();
        std::vector<const char*> disabledToggles = instanceTogglesState.GetDisabledToggleNames();
        EXPECT_EQ(disabledToggles.size(), disableToggles.size());
        EXPECT_EQ(enabledToggles.size(), enableToggles.size());
        for (auto* enableToggle : enableToggles) {
            EXPECT_THAT(enabledToggles, Contains(StrEq(enableToggle)));
        }
        for (auto* disableToggle : disableToggles) {
            EXPECT_THAT(disabledToggles, Contains(StrEq(disableToggle)));
        }
    };

    // Create instance with no toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        // Create an instance with default toggles, where AllowUnsafeAPIs is disabled.
        instance = std::make_unique<native::Instance>();
        validateInstanceToggles(instance.get(), {}, {"allow_unsafe_apis"});
    }

    // Create instance with empty toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        // Make an instance descriptor chaining an empty toggles descriptor
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with default toggles, where AllowUnsafeAPIs is disabled and
        // DisallowUnsafeAPIs is enabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {}, {"allow_unsafe_apis"});
    }

    // Create instance with AllowUnsafeAPIs explicitly enabled in toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.enabledToggleCount = 1;
        instanceTogglesDesc.enabledToggles = &allowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with AllowUnsafeAPIs explicitly enabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {allowUnsafeApisToggle}, {});
    }

    // Create instance with AllowUnsafeAPIs explicitly disabled in toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.disabledToggleCount = 1;
        instanceTogglesDesc.disabledToggles = &allowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with AllowUnsafeAPIs explicitly disabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {}, {allowUnsafeApisToggle});
    }
}

// Test that instance toggles are inherited to the adapters and devices it creates.
TEST_F(InstanceToggleTest, InstanceTogglesInheritToAdapterAndDevice) {
    auto validateInstanceTogglesInheritedToAdapter = [&](native::Instance* nativeInstance) {
        native::InstanceBase* instance = native::FromAPI(nativeInstance->Get());
        const native::TogglesState& instanceTogglesState = instance->GetTogglesState();

        // Get the null adapter with default toggles.
        Ref<native::AdapterBase> nullAdapter =
            native::FromAPI(CreateNullAdapter(nativeInstance, nullptr).Get());

        auto& adapterTogglesState = nullAdapter->GetTogglesState();

        // Creater a default device.
        native::DeviceBase* nullDevice = nullAdapter->APICreateDevice();

        // Check instance toggles are inherited by adapter and device.
        native::TogglesInfo togglesInfo;
        static_assert(std::is_same_v<std::underlying_type_t<native::Toggle>, int>);
        for (int i = 0; i < static_cast<int>(native::Toggle::EnumCount); i++) {
            native::Toggle toggle = static_cast<native::Toggle>(i);
            if (togglesInfo.GetToggleInfo(toggle)->stage != native::ToggleStage::Instance) {
                continue;
            }
            EXPECT_EQ(instanceTogglesState.IsSet(toggle), adapterTogglesState.IsSet(toggle));
            EXPECT_EQ(instanceTogglesState.IsEnabled(toggle),
                      adapterTogglesState.IsEnabled(toggle));
            EXPECT_EQ(instanceTogglesState.IsEnabled(toggle), nullDevice->IsToggleEnabled(toggle));
        }

        nullDevice->Release();
    };

    // Create instance with AllowUnsafeAPIs explicitly enabled in toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.enabledToggleCount = 1;
        instanceTogglesDesc.enabledToggles = &allowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with DisallowUnsafeApis explicitly enabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceTogglesInheritedToAdapter(instance.get());
    }

    // Create instance with AllowUnsafeAPIs explicitly disabled in toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.disabledToggleCount = 1;
        instanceTogglesDesc.disabledToggles = &allowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with DisallowUnsafeApis explicitly enabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceTogglesInheritedToAdapter(instance.get());
    }
}

using AdapterToggleTest = ToggleTest;

// Test that adapter toggles are set and/or overridden by requirement or default as expected, and
// get inherited to devices it creates.
TEST_F(AdapterToggleTest, AdapterTogglesSetAndInheritToDevice) {
    // Create an instance with default toggles, where AllowUnsafeAPIs is disabled.
    std::unique_ptr<native::Instance> instance;
    instance = std::make_unique<native::Instance>();
    // AllowUnsafeAPIs should be disabled by default.
    native::InstanceBase* instanceBase = native::FromAPI(instance->Get());
    ASSERT_FALSE(instanceBase->GetTogglesState().IsEnabled(native::Toggle::AllowUnsafeAPIs));

    // Validate an adapter has expected toggles state.
    auto ValidateAdapterToggles = [](const native::AdapterBase* adapterBase,
                                     std::initializer_list<const char*> enableToggles,
                                     std::initializer_list<const char*> disableToggles) {
        const native::TogglesState& adapterTogglesState = adapterBase->GetTogglesState();
        std::vector<const char*> enabledToggles = adapterTogglesState.GetEnabledToggleNames();
        std::vector<const char*> disabledToggles = adapterTogglesState.GetDisabledToggleNames();
        EXPECT_EQ(disabledToggles.size(), disableToggles.size());
        EXPECT_EQ(enabledToggles.size(), enableToggles.size());
        for (auto* enableToggle : enableToggles) {
            EXPECT_THAT(enabledToggles, Contains(StrEq(enableToggle)));
        }
        for (auto* disableToggle : disableToggles) {
            EXPECT_THAT(disabledToggles, Contains(StrEq(disableToggle)));
        }
    };

    // Validate an adapter's toggles get inherited to the device it creates.
    auto ValidateAdapterTogglesInheritedToDevice = [](native::AdapterBase* adapter) {
        auto& adapterTogglesState = adapter->GetTogglesState();

        // Creater a default device from the adapter.
        Ref<native::DeviceBase> device = AcquireRef(adapter->APICreateDevice());

        // Check adapter toggles are inherited by the device.
        native::TogglesInfo togglesInfo;
        static_assert(std::is_same_v<std::underlying_type_t<native::Toggle>, int>);
        for (int i = 0; i < static_cast<int>(native::Toggle::EnumCount); i++) {
            native::Toggle toggle = static_cast<native::Toggle>(i);
            if (togglesInfo.GetToggleInfo(toggle)->stage > native::ToggleStage::Adapter) {
                continue;
            }
            EXPECT_EQ(adapterTogglesState.IsEnabled(toggle), device->IsToggleEnabled(toggle));
        }
    };

    // Do the test by creating an adapter with given toggles descriptor and validate its toggles
    // state is as expected and get inherited to devices.
    auto CreateAdapterAndValidateToggles =
        [this, &instance, ValidateAdapterToggles, ValidateAdapterTogglesInheritedToDevice](
            const wgpu::DawnTogglesDescriptor* requiredAdapterToggles,
            std::initializer_list<const char*> expectedEnabledToggles,
            std::initializer_list<const char*> expectedDisabledToggles) {
            native::Adapter adapter = CreateNullAdapter(instance.get(), requiredAdapterToggles);
            ValidateAdapterToggles(native::FromAPI(adapter.Get()), expectedEnabledToggles,
                                   expectedDisabledToggles);
            ValidateAdapterTogglesInheritedToDevice(native::FromAPI(adapter.Get()));
        };

    const char* allowUnsafeApisToggle = "allow_unsafe_apis";
    const char* useDXCToggle = "use_dxc";

    // Create adapter with no toggles descriptor
    {
        // The created adapter should inherit disabled AllowUnsafeAPIs toggle from instance.
        CreateAdapterAndValidateToggles(nullptr, {}, {allowUnsafeApisToggle});
    }

    // Create adapter with empty toggles descriptor
    {
        wgpu::DawnTogglesDescriptor adapterTogglesDesc = {};

        // The created adapter should inherit disabled AllowUnsafeAPIs toggle from instance.
        CreateAdapterAndValidateToggles(&adapterTogglesDesc, {}, {allowUnsafeApisToggle});
    }

    // Create adapter with UseDXC enabled in toggles descriptor
    {
        wgpu::DawnTogglesDescriptor adapterTogglesDesc = {};
        adapterTogglesDesc.enabledToggleCount = 1;
        adapterTogglesDesc.enabledToggles = &useDXCToggle;

        // The created adapter should enable required UseDXC toggle and inherit disabled
        // AllowUnsafeAPIs toggle from instance.
        CreateAdapterAndValidateToggles(&adapterTogglesDesc, {useDXCToggle},
                                        {allowUnsafeApisToggle});
    }

    // Create adapter with explicitly overriding AllowUnsafeAPIs in toggles descriptor
    {
        wgpu::DawnTogglesDescriptor adapterTogglesDesc = {};
        adapterTogglesDesc.enabledToggleCount = 1;
        adapterTogglesDesc.enabledToggles = &allowUnsafeApisToggle;

        // The created adapter should enable overridden AllowUnsafeAPIs toggle.
        CreateAdapterAndValidateToggles(&adapterTogglesDesc, {allowUnsafeApisToggle}, {});
    }

    // Create adapter with UseDXC enabled and explicitly overriding AllowUnsafeAPIs in toggles
    // descriptor
    {
        std::vector<const char*> enableAdapterToggles = {useDXCToggle, allowUnsafeApisToggle};
        wgpu::DawnTogglesDescriptor adapterTogglesDesc = {};
        adapterTogglesDesc.enabledToggleCount = enableAdapterToggles.size();
        adapterTogglesDesc.enabledToggles = enableAdapterToggles.data();

        // The created adapter should enable required UseDXC and overridden AllowUnsafeAPIs toggle.
        CreateAdapterAndValidateToggles(&adapterTogglesDesc, {useDXCToggle, allowUnsafeApisToggle},
                                        {});
    }
}

class DeviceToggleTest : public ToggleTest {
  public:
    // Create a device with given toggles descriptor from the given adapter, and validate its
    // toggles state.
    void CreateDeviceAndValidateToggles(
        native::AdapterBase* adapter,
        const wgpu::DawnTogglesDescriptor* requiredDeviceToggles,
        std::initializer_list<const char*> expectedEnabledToggles,
        std::initializer_list<const char*> expectedDisabledToggles) {
        native::DeviceDescriptor desc{};
        desc.nextInChain = requiredDeviceToggles;

        // Creater a device with given toggles descriptor from the adapter.
        Ref<native::DeviceBase> device = AcquireRef(adapter->APICreateDevice(&desc));

        // Check device toggles state is as expected.
        native::TogglesInfo togglesInfo;
        auto deviceEnabledToggles = device->GetTogglesUsed();
        for (auto* enableToggle : expectedEnabledToggles) {
            EXPECT_THAT(deviceEnabledToggles, Contains(StrEq(enableToggle)));
            native::Toggle toggle = togglesInfo.ToggleNameToEnum(enableToggle);
            DAWN_ASSERT(toggle != native::Toggle::InvalidEnum);
            EXPECT_TRUE(device->IsToggleEnabled(toggle));
        }
        for (auto* enableToggle : expectedDisabledToggles) {
            EXPECT_THAT(deviceEnabledToggles, Not(Contains(StrEq(enableToggle))));
            native::Toggle toggle = togglesInfo.ToggleNameToEnum(enableToggle);
            DAWN_ASSERT(toggle != native::Toggle::InvalidEnum);
            EXPECT_FALSE(device->IsToggleEnabled(toggle));
        }
    }

  protected:
    static constexpr auto kMultipleDevicesPerAdapter =
        wgpu::InstanceFeatureName::MultipleDevicesPerAdapter;
    static constexpr wgpu::InstanceDescriptor instanceDesc = {
        .requiredFeatureCount = 1,
        .requiredFeatures = &kMultipleDevicesPerAdapter,
    };
};

// Test that device toggles are set by requirement or default as expected.
TEST_F(DeviceToggleTest, DeviceSetToggles) {
    // Create an instance with default toggles.
    std::unique_ptr<native::Instance> instance;
    instance = std::make_unique<native::Instance>(&instanceDesc);

    // Create a null adapter from the instance.
    native::Adapter nullAdapter = CreateNullAdapter(instance.get(), nullptr);
    Ref<native::AdapterBase> adapter = native::FromAPI(nullAdapter.Get());

    // DumpShader is a device toggle.
    const char* dumpShaderToggle = "dump_shaders";

    // Create device with no toggles descriptor.
    {
        // DumpShader toggles is not set and treated as disabled in device.
        CreateDeviceAndValidateToggles(adapter.Get(), nullptr, {}, {dumpShaderToggle});
    }

    // Create device with a device toggle required enabled. This should always work.
    {
        wgpu::DawnTogglesDescriptor deviceTogglesDesc = {};
        deviceTogglesDesc.enabledToggleCount = 1;
        deviceTogglesDesc.enabledToggles = &dumpShaderToggle;

        // The device created from all adapter should have DumpShader device toggle enabled.
        CreateDeviceAndValidateToggles(adapter.Get(), &deviceTogglesDesc, {dumpShaderToggle}, {});
    }

    // Create device with a device toggle required disabled. This should always work.
    {
        wgpu::DawnTogglesDescriptor deviceTogglesDesc = {};
        deviceTogglesDesc.disabledToggleCount = 1;
        deviceTogglesDesc.disabledToggles = &dumpShaderToggle;

        // The device created from all adapter should have DumpShader device toggle enabled.
        CreateDeviceAndValidateToggles(adapter.Get(), &deviceTogglesDesc, {}, {dumpShaderToggle});
    }
}

// Test that device can override non-forced instance toggles by requirement as expected.
TEST_F(DeviceToggleTest, DeviceOverridingInstanceToggle) {
    // Create an instance with default toggles, where AllowUnsafeAPIs is disabled.
    std::unique_ptr<native::Instance> instance;
    instance = std::make_unique<native::Instance>(&instanceDesc);
    // AllowUnsafeAPIs should be disabled by default.
    native::InstanceBase* instanceBase = native::FromAPI(instance->Get());
    ASSERT_FALSE(instanceBase->GetTogglesState().IsEnabled(native::Toggle::AllowUnsafeAPIs));

    // Create a null adapter from the instance. AllowUnsafeAPIs should be inherited disabled.
    native::Adapter nullAdapter = CreateNullAdapter(instance.get(), nullptr);
    Ref<native::AdapterBase> adapter = native::FromAPI(nullAdapter.Get());

    native::PhysicalDeviceBase* nullPhysicalDevice = adapter->GetPhysicalDevice();
    wgpu::FeatureLevel featureLevel = adapter->GetFeatureLevel();

    // Create null adapters with the AllowUnsafeAPIs toggle set/forced to enabled/disabled, using
    // the same physical device and feature level as the known null adapter.
    auto CreateAdapterWithAllowUnsafeAPIsToggle = [&adapter, instanceBase, nullPhysicalDevice,
                                                   featureLevel](bool isAllowUnsafeAPIsEnabled,
                                                                 bool isAllowUnsafeAPIsForced) {
        native::TogglesState adapterTogglesState = adapter->GetTogglesState();
        adapterTogglesState.SetForTesting(native::Toggle::AllowUnsafeAPIs, isAllowUnsafeAPIsEnabled,
                                          isAllowUnsafeAPIsForced);

        Ref<native::AdapterBase> resultAdapter;
        resultAdapter = AcquireRef<native::AdapterBase>(
            new native::AdapterBase(instanceBase, nullPhysicalDevice, featureLevel,
                                    adapterTogglesState, wgpu::PowerPreference::Undefined));

        // AllowUnsafeAPIs should be set as expected.
        EXPECT_TRUE(resultAdapter->GetTogglesState().IsSet(native::Toggle::AllowUnsafeAPIs));
        EXPECT_EQ(resultAdapter->GetTogglesState().IsEnabled(native::Toggle::AllowUnsafeAPIs),
                  isAllowUnsafeAPIsEnabled);

        return resultAdapter;
    };

    Ref<native::AdapterBase> adapterEnableAllowUnsafeAPIs =
        CreateAdapterWithAllowUnsafeAPIsToggle(true, false);
    Ref<native::AdapterBase> adapterDisableAllowUnsafeAPIs =
        CreateAdapterWithAllowUnsafeAPIsToggle(false, false);
    Ref<native::AdapterBase> adapterForcedEnableAllowUnsafeAPIs =
        CreateAdapterWithAllowUnsafeAPIsToggle(true, true);
    Ref<native::AdapterBase> adapterForcedDisableAllowUnsafeAPIs =
        CreateAdapterWithAllowUnsafeAPIsToggle(false, true);

    // AllowUnsafeAPIs is an instance toggle, and can be overridden by device.
    const char* allowUnsafeApisToggle = "allow_unsafe_apis";

    // Create device with no toggles descriptor will inherite the adapter toggle.
    {
        // The device created from default adapter should inherit disabled AllowUnsafeAPIs toggle
        // from adapter.
        CreateDeviceAndValidateToggles(adapter.Get(), nullptr, {}, {allowUnsafeApisToggle});
        // The device created from AllowUnsafeAPIs enabled/disabled adapter should inherit it.
        CreateDeviceAndValidateToggles(adapterEnableAllowUnsafeAPIs.Get(), nullptr,
                                       {allowUnsafeApisToggle}, {});
        CreateDeviceAndValidateToggles(adapterDisableAllowUnsafeAPIs.Get(), nullptr, {},
                                       {allowUnsafeApisToggle});
        // The device created from AllowUnsafeAPIs forced adapter should inherit forced
        // enabled/disabled toggle.
        CreateDeviceAndValidateToggles(adapterForcedEnableAllowUnsafeAPIs.Get(), nullptr,
                                       {allowUnsafeApisToggle}, {});
        CreateDeviceAndValidateToggles(adapterForcedDisableAllowUnsafeAPIs.Get(), nullptr, {},
                                       {allowUnsafeApisToggle});
    }

    // Create device trying to override instance toggle to enable. This should work as long as the
    // toggle is not forced.
    {
        wgpu::DawnTogglesDescriptor deviceTogglesDesc = {};
        deviceTogglesDesc.enabledToggleCount = 1;
        deviceTogglesDesc.enabledToggles = &allowUnsafeApisToggle;

        // The device created from default adapter should have AllowUnsafeAPIs device toggle
        // enabled.
        CreateDeviceAndValidateToggles(adapter.Get(), &deviceTogglesDesc, {allowUnsafeApisToggle},
                                       {});
        // The device created from UseDXC enabled/disabled adapter should have AllowUnsafeAPIs
        // device toggle overridden to enabled.
        CreateDeviceAndValidateToggles(adapterEnableAllowUnsafeAPIs.Get(), &deviceTogglesDesc,
                                       {allowUnsafeApisToggle}, {});
        CreateDeviceAndValidateToggles(adapterDisableAllowUnsafeAPIs.Get(), &deviceTogglesDesc,
                                       {allowUnsafeApisToggle}, {});
        // The device created from UseAllowUnsafeAPIs forced enabled adapter should have
        // AllowUnsafeAPIs device toggle enabled.
        CreateDeviceAndValidateToggles(adapterForcedEnableAllowUnsafeAPIs.Get(), &deviceTogglesDesc,
                                       {allowUnsafeApisToggle}, {});
        // The device created from AllowUnsafeAPIs forced disabled adapter should have
        // AllowUnsafeAPIs device toggle disabled, since it can not override the forced toggle.
        CreateDeviceAndValidateToggles(adapterForcedDisableAllowUnsafeAPIs.Get(),
                                       &deviceTogglesDesc, {}, {allowUnsafeApisToggle});
    }

    // Create device trying to override instance toggle to disable. This should work as long as the
    // toggle is not forced.
    {
        wgpu::DawnTogglesDescriptor deviceTogglesDesc = {};
        deviceTogglesDesc.disabledToggleCount = 1;
        deviceTogglesDesc.disabledToggles = &allowUnsafeApisToggle;

        // The device created from default adapter should have UseDXC device toggle enabled.
        CreateDeviceAndValidateToggles(adapter.Get(), &deviceTogglesDesc, {},
                                       {allowUnsafeApisToggle});
        // The device created from UseDXC enabled/disabled adapter should have UseDXC device toggle
        // overridden to disabled.
        CreateDeviceAndValidateToggles(adapterEnableAllowUnsafeAPIs.Get(), &deviceTogglesDesc, {},
                                       {allowUnsafeApisToggle});
        CreateDeviceAndValidateToggles(adapterDisableAllowUnsafeAPIs.Get(), &deviceTogglesDesc, {},
                                       {allowUnsafeApisToggle});
        // The device created from UseDXC forced enabled adapter should have UseDXC device toggle
        // enabled, since it can not override the forced toggle.
        CreateDeviceAndValidateToggles(adapterForcedEnableAllowUnsafeAPIs.Get(), &deviceTogglesDesc,
                                       {allowUnsafeApisToggle}, {});
        // The device created from UseDXC forced disabled adapter should have UseDXC device toggle
        // disabled.
        CreateDeviceAndValidateToggles(adapterForcedDisableAllowUnsafeAPIs.Get(),
                                       &deviceTogglesDesc, {}, {allowUnsafeApisToggle});
    }
}

// Test that device can override non-forced adapter toggles by requirement as expected.
TEST_F(DeviceToggleTest, DeviceOverridingAdapterToggle) {
    // Create an instance with default toggles, where AllowUnsafeAPIs is disabled.
    std::unique_ptr<native::Instance> instance;
    instance = std::make_unique<native::Instance>(&instanceDesc);
    // AllowUnsafeAPIs should be disabled by default.
    native::InstanceBase* instanceBase = native::FromAPI(instance->Get());
    ASSERT_FALSE(instanceBase->GetTogglesState().IsEnabled(native::Toggle::AllowUnsafeAPIs));

    // Create a null adapter from the instance. AllowUnsafeAPIs should be inherited disabled.
    native::Adapter nullAdapter = CreateNullAdapter(instance.get(), nullptr);
    Ref<native::AdapterBase> adapter = native::FromAPI(nullAdapter.Get());
    // The null adapter will not set or force-set the UseDXC toggle by default.
    ASSERT_FALSE(adapter->GetTogglesState().IsSet(native::Toggle::UseDXC));

    native::PhysicalDeviceBase* nullPhysicalDevice = adapter->GetPhysicalDevice();
    wgpu::FeatureLevel featureLevel = adapter->GetFeatureLevel();

    // Create null adapters with the UseDXC toggle set/forced to enabled/disabled, using the same
    // physical device and feature level as the known null adapter.
    auto CreateAdapterWithDXCToggle = [&adapter, instanceBase, nullPhysicalDevice, featureLevel](
                                          bool isUseDXCEnabled, bool isUseDXCForced) {
        native::TogglesState adapterTogglesState = adapter->GetTogglesState();
        adapterTogglesState.SetForTesting(native::Toggle::UseDXC, isUseDXCEnabled, isUseDXCForced);

        Ref<native::AdapterBase> resultAdapter;
        resultAdapter = AcquireRef<native::AdapterBase>(
            new native::AdapterBase(instanceBase, nullPhysicalDevice, featureLevel,
                                    adapterTogglesState, wgpu::PowerPreference::Undefined));

        // AllowUnsafeAPIs should be inherited disabled by default.
        EXPECT_TRUE(resultAdapter->GetTogglesState().IsSet(native::Toggle::AllowUnsafeAPIs));
        EXPECT_FALSE(resultAdapter->GetTogglesState().IsEnabled(native::Toggle::AllowUnsafeAPIs));

        // The adapter should have UseDXC toggle set to given state.
        EXPECT_TRUE(resultAdapter->GetTogglesState().IsSet(native::Toggle::UseDXC));
        EXPECT_EQ(resultAdapter->GetTogglesState().IsEnabled(native::Toggle::UseDXC),
                  isUseDXCEnabled);

        return resultAdapter;
    };

    Ref<native::AdapterBase> adapterEnableDXC = CreateAdapterWithDXCToggle(true, false);
    Ref<native::AdapterBase> adapterDisableDXC = CreateAdapterWithDXCToggle(false, false);
    Ref<native::AdapterBase> adapterForcedEnableDXC = CreateAdapterWithDXCToggle(true, true);
    Ref<native::AdapterBase> adapterForcedDisableDXC = CreateAdapterWithDXCToggle(false, true);

    // UseDXC is an adapter toggle, and can be overridden by device.
    const char* useDXCToggle = "use_dxc";

    // Create device with no toggles descriptor.
    {
        // The device created from default adapter should inherit enabled/disabled toggles from
        // adapter. All other toggles that are not set are also treated as disabled in device.
        CreateDeviceAndValidateToggles(adapter.Get(), nullptr, {}, {useDXCToggle});
        // The device created from UseDXC enabled/disabled adapter should inherit forced
        // enabled/disabled UseDXC toggle.
        CreateDeviceAndValidateToggles(adapterEnableDXC.Get(), nullptr, {useDXCToggle}, {});
        CreateDeviceAndValidateToggles(adapterDisableDXC.Get(), nullptr, {}, {useDXCToggle});
        // The device created from UseDXC forced adapter should inherit forced enabled/disabled
        // UseDXC toggle.
        CreateDeviceAndValidateToggles(adapterForcedEnableDXC.Get(), nullptr, {useDXCToggle}, {});
        CreateDeviceAndValidateToggles(adapterForcedDisableDXC.Get(), nullptr, {}, {useDXCToggle});
    }

    // Create device trying to override adapter toggle to enable. This should work as long as the
    // toggle is not forced.
    {
        wgpu::DawnTogglesDescriptor deviceTogglesDesc = {};
        deviceTogglesDesc.enabledToggleCount = 1;
        deviceTogglesDesc.enabledToggles = &useDXCToggle;

        // The device created from default adapter should have UseDXC device toggle enabled.
        CreateDeviceAndValidateToggles(adapter.Get(), &deviceTogglesDesc, {useDXCToggle}, {});
        // The device created from UseDXC enabled/disabled adapter should have UseDXC device toggle
        // overridden to enabled.
        CreateDeviceAndValidateToggles(adapterEnableDXC.Get(), &deviceTogglesDesc, {useDXCToggle},
                                       {});
        CreateDeviceAndValidateToggles(adapterDisableDXC.Get(), &deviceTogglesDesc, {useDXCToggle},
                                       {});
        // The device created from UseDXC forced enabled adapter should have UseDXC device toggle
        // enabled.
        CreateDeviceAndValidateToggles(adapterForcedEnableDXC.Get(), &deviceTogglesDesc,
                                       {useDXCToggle}, {});
        // The device created from UseDXC forced disabled adapter should have UseDXC device toggle
        // disabled, since it can not override the forced toggle.
        CreateDeviceAndValidateToggles(adapterForcedDisableDXC.Get(), &deviceTogglesDesc, {},
                                       {useDXCToggle});
    }

    // Create device trying to override adapter toggle to disable. This should work as long as the
    // toggle is not forced.
    {
        wgpu::DawnTogglesDescriptor deviceTogglesDesc = {};
        deviceTogglesDesc.disabledToggleCount = 1;
        deviceTogglesDesc.disabledToggles = &useDXCToggle;

        // The device created from default adapter should have UseDXC device toggle enabled.
        CreateDeviceAndValidateToggles(adapter.Get(), &deviceTogglesDesc, {}, {useDXCToggle});
        // The device created from UseDXC enabled/disabled adapter should have UseDXC device toggle
        // overridden to disabled.
        CreateDeviceAndValidateToggles(adapterEnableDXC.Get(), &deviceTogglesDesc, {},
                                       {useDXCToggle});
        CreateDeviceAndValidateToggles(adapterDisableDXC.Get(), &deviceTogglesDesc, {},
                                       {useDXCToggle});
        // The device created from UseDXC forced enabled adapter should have UseDXC device toggle
        // enabled, since it can not override the forced toggle.
        CreateDeviceAndValidateToggles(adapterForcedEnableDXC.Get(), &deviceTogglesDesc,
                                       {useDXCToggle}, {});
        // The device created from UseDXC forced disabled adapter should have UseDXC device toggle
        // disabled.
        CreateDeviceAndValidateToggles(adapterForcedDisableDXC.Get(), &deviceTogglesDesc, {},
                                       {useDXCToggle});
    }
}

}  // anonymous namespace
}  // namespace dawn
