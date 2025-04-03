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

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using testing::_;
using testing::EmptySizedString;
using testing::IsNull;
using testing::MockCppCallback;
using testing::StrictMock;
using testing::WithArg;

using MockRequestAdapterCallback = MockCppCallback<wgpu::RequestAdapterCallback<void>*>;

class AdapterCreationTest : public ::testing::TestWithParam<wgpu::CallbackMode> {
  protected:
    void SetUp() override {
        // TODO(345685638): these tests are timed out on TSAN bots.
        DAWN_TEST_UNSUPPORTED_IF(DawnTest::IsTsan());

        dawnProcSetProcs(&native::GetProcs());

        {
            auto nativeInstance = std::make_unique<native::Instance>();
            // TODO(347047627): Use a webgpu.h version of enumerateAdapters
            for (native::Adapter& nativeAdapter : nativeInstance->EnumerateAdapters()) {
                anyAdapterAvailable = true;

                wgpu::AdapterInfo info;
                wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
                adapter.GetInfo(&info);
                if (!adapter.HasFeature(wgpu::FeatureName::CoreFeaturesAndLimits)) {
                    continue;
                }
                swiftShaderAvailable |= gpu_info::IsGoogleSwiftshader(info.vendorID, info.deviceID);
                discreteGPUAvailable |= info.adapterType == wgpu::AdapterType::DiscreteGPU;
                integratedGPUAvailable |= info.adapterType == wgpu::AdapterType::IntegratedGPU;
            }
        }

        instance = wgpu::CreateInstance();
    }

    void TearDown() override {
        instance = nullptr;
        dawnProcSetProcs(nullptr);
    }

    void RequestAdapter(const wgpu::Instance& inst, const wgpu::RequestAdapterOptions* options) {
        wgpu::CallbackMode callbackMode = GetParam();
        wgpu::Future future = inst.RequestAdapter(options, callbackMode, mMockCb.Callback());
        switch (callbackMode) {
            case wgpu::CallbackMode::WaitAnyOnly: {
                // Callback should complete as soon as poll once.
                EXPECT_EQ(inst.WaitAny(future, 0), wgpu::WaitStatus::Success);
                break;
            }
            case wgpu::CallbackMode::AllowSpontaneous:
                // Callback should already be called.
                break;
            case wgpu::CallbackMode::AllowProcessEvents:
                inst.ProcessEvents();
                break;
        }
    }

    wgpu::Instance instance;
    StrictMock<MockRequestAdapterCallback> mMockCb;
    bool anyAdapterAvailable = false;
    bool swiftShaderAvailable = false;
    bool discreteGPUAvailable = false;
    bool integratedGPUAvailable = false;
};

INSTANTIATE_TEST_SUITE_P(,
                         AdapterCreationTest,
                         ::testing::ValuesIn(std::initializer_list<wgpu::CallbackMode>{
                             wgpu::CallbackMode::WaitAnyOnly,
                             wgpu::CallbackMode::AllowProcessEvents,
                             wgpu::CallbackMode::AllowSpontaneous}));

// Test that requesting the default adapter works and is non-compat.
TEST_P(AdapterCreationTest, DefaultNonCompatAdapter) {
    wgpu::RequestAdapterOptions options = {};

    wgpu::Adapter adapter;
    EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
        .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));

    RequestAdapter(instance, &options);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    wgpu::AdapterInfo info;
    adapter.GetInfo(&info);
    EXPECT_TRUE(adapter.HasFeature(wgpu::FeatureName::CoreFeaturesAndLimits));
}

// Test that passing nullptr for the options gets the default adapter
TEST_P(AdapterCreationTest, NullGivesDefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    wgpu::Adapter adapter;
    EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
        .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));

    RequestAdapter(instance, nullptr);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
}

// Test that requesting the fallback adapter returns SwiftShader.
TEST_P(AdapterCreationTest, FallbackAdapter) {
    wgpu::RequestAdapterOptions options = {};
    options.forceFallbackAdapter = true;

    wgpu::Adapter adapter;
    if (swiftShaderAvailable) {
        EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
            .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    } else {
        EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Unavailable, IsNull(), _))
            .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    }
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, swiftShaderAvailable);
    if (adapter != nullptr) {
        wgpu::AdapterInfo info;
        adapter.GetInfo(&info);

        EXPECT_EQ(info.adapterType, wgpu::AdapterType::CPU);
        EXPECT_TRUE(gpu_info::IsGoogleSwiftshader(info.vendorID, info.deviceID));
    }
}

// Test that requesting a high performance GPU works
TEST_P(AdapterCreationTest, PreferHighPerformance) {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::HighPerformance;

    wgpu::Adapter adapter;
    if (anyAdapterAvailable) {
        EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
            .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    } else {
        EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Unavailable, IsNull(), _))
            .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    }
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (discreteGPUAvailable) {
        wgpu::AdapterInfo info;
        wgpu::DawnAdapterPropertiesPowerPreference powerPreferenceProperties;
        info.nextInChain = &powerPreferenceProperties;
        adapter.GetInfo(&info);
        EXPECT_EQ(info.adapterType, wgpu::AdapterType::DiscreteGPU);
        EXPECT_EQ(powerPreferenceProperties.powerPreference, options.powerPreference);
    }
}

// Test that requesting a low power GPU works
TEST_P(AdapterCreationTest, PreferLowPower) {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::LowPower;

    wgpu::Adapter adapter;
    if (anyAdapterAvailable) {
        EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
            .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    } else {
        EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Unavailable, IsNull(), _))
            .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    }
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (integratedGPUAvailable) {
        wgpu::AdapterInfo info;
        wgpu::DawnAdapterPropertiesPowerPreference powerPreferenceProperties;
        info.nextInChain = &powerPreferenceProperties;
        adapter.GetInfo(&info);
        EXPECT_EQ(info.adapterType, wgpu::AdapterType::IntegratedGPU);
        EXPECT_EQ(powerPreferenceProperties.powerPreference, options.powerPreference);
    }
}

// Test that requesting a Compatibility adapter is supported.
TEST_P(AdapterCreationTest, Compatibility) {
    wgpu::RequestAdapterOptions options = {};
    options.featureLevel = wgpu::FeatureLevel::Compatibility;

    wgpu::Adapter adapter;
    EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
        .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
}

// Test that GetInstance() returns the correct Instance.
TEST_P(AdapterCreationTest, GetInstance) {
    wgpu::RequestAdapterOptions options = {};

    wgpu::Adapter adapter;
    EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
        .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    EXPECT_EQ(adapter.GetInstance().Get(), instance.Get());
}

// Test that calling AdapterGetInfo returns separate allocations for strings.
// However, the string contents are equivalent.
TEST_P(AdapterCreationTest, InfoUnique) {
    wgpu::RequestAdapterOptions options = {};

    wgpu::Adapter adapter;
    EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
        .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (!adapter) {
        return;
    }

    wgpu::AdapterInfo info1;
    wgpu::AdapterInfo info2;
    adapter.GetInfo(&info1);
    adapter.GetInfo(&info2);

    EXPECT_NE(info1.vendor.data, info2.vendor.data);
    EXPECT_EQ(info1.vendor, info2.vendor);
    EXPECT_NE(info1.architecture.data, info2.architecture.data);
    EXPECT_EQ(info1.architecture, info2.architecture);
    EXPECT_NE(info1.device.data, info2.device.data);
    EXPECT_EQ(info1.device, info2.device);
    EXPECT_NE(info1.description.data, info2.description.data);
    EXPECT_EQ(info1.description, info2.description);
}

// Test move assignment of the adapter info.
TEST_P(AdapterCreationTest, InfoMoveAssign) {
    wgpu::RequestAdapterOptions options = {};

    wgpu::Adapter adapter;
    EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
        .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (!adapter) {
        return;
    }

    wgpu::AdapterInfo info1;
    wgpu::AdapterInfo info2;
    adapter.GetInfo(&info1);
    adapter.GetInfo(&info2);

    wgpu::StringView vendor = info1.vendor;
    wgpu::StringView architecture = info1.architecture;
    wgpu::StringView device = info1.device;
    wgpu::StringView description = info1.description;
    wgpu::BackendType backendType = info1.backendType;
    wgpu::AdapterType adapterType = info1.adapterType;
    uint32_t vendorID = info1.vendorID;
    uint32_t deviceID = info1.deviceID;

    info2 = std::move(info1);

    // Expect info2 to have info1's old contents.
    EXPECT_EQ(info2.vendor, vendor);
    EXPECT_EQ(info2.architecture, architecture);
    EXPECT_EQ(info2.device, device);
    EXPECT_EQ(info2.description, description);
    EXPECT_EQ(info2.backendType, backendType);
    EXPECT_EQ(info2.adapterType, adapterType);
    EXPECT_EQ(info2.vendorID, vendorID);
    EXPECT_EQ(info2.deviceID, deviceID);

    // Expect info1 to be empty.
    EXPECT_EQ(info1.vendor.data, nullptr);
    EXPECT_EQ(info1.vendor.length, wgpu::kStrlen);
    EXPECT_EQ(info1.architecture.data, nullptr);
    EXPECT_EQ(info1.architecture.length, wgpu::kStrlen);
    EXPECT_EQ(info1.device.data, nullptr);
    EXPECT_EQ(info1.device.length, wgpu::kStrlen);
    EXPECT_EQ(info1.description.data, nullptr);
    EXPECT_EQ(info1.description.length, wgpu::kStrlen);
    EXPECT_EQ(info1.backendType, static_cast<wgpu::BackendType>(0));
    EXPECT_EQ(info1.adapterType, static_cast<wgpu::AdapterType>(0));
    EXPECT_EQ(info1.vendorID, 0u);
    EXPECT_EQ(info1.deviceID, 0u);
}

// Test move construction of the adapter info.
TEST_P(AdapterCreationTest, InfoMoveConstruct) {
    wgpu::RequestAdapterOptions options = {};

    wgpu::Adapter adapter;
    EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
        .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (!adapter) {
        return;
    }

    wgpu::AdapterInfo info1;
    adapter.GetInfo(&info1);

    wgpu::StringView vendor = info1.vendor;
    wgpu::StringView architecture = info1.architecture;
    wgpu::StringView device = info1.device;
    wgpu::StringView description = info1.description;
    wgpu::BackendType backendType = info1.backendType;
    wgpu::AdapterType adapterType = info1.adapterType;
    uint32_t vendorID = info1.vendorID;
    uint32_t deviceID = info1.deviceID;

    wgpu::AdapterInfo info2(std::move(info1));

    // Expect info2 to have info1's old contents.
    EXPECT_EQ(info2.vendor, vendor);
    EXPECT_EQ(info2.architecture, architecture);
    EXPECT_EQ(info2.device, device);
    EXPECT_EQ(info2.description, description);
    EXPECT_EQ(info2.backendType, backendType);
    EXPECT_EQ(info2.adapterType, adapterType);
    EXPECT_EQ(info2.vendorID, vendorID);
    EXPECT_EQ(info2.deviceID, deviceID);

    // Expect info1 to be empty.
    EXPECT_EQ(info1.vendor.data, nullptr);
    EXPECT_EQ(info1.vendor.length, wgpu::kStrlen);
    EXPECT_EQ(info1.architecture.data, nullptr);
    EXPECT_EQ(info1.architecture.length, wgpu::kStrlen);
    EXPECT_EQ(info1.device.data, nullptr);
    EXPECT_EQ(info1.device.length, wgpu::kStrlen);
    EXPECT_EQ(info1.description.data, nullptr);
    EXPECT_EQ(info1.description.length, wgpu::kStrlen);
    EXPECT_EQ(info1.backendType, static_cast<wgpu::BackendType>(0));
    EXPECT_EQ(info1.adapterType, static_cast<wgpu::AdapterType>(0));
    EXPECT_EQ(info1.vendorID, 0u);
    EXPECT_EQ(info1.deviceID, 0u);
}

// Test that the adapter info can outlive the adapter.
TEST_P(AdapterCreationTest, InfoOutliveAdapter) {
    wgpu::RequestAdapterOptions options = {};

    wgpu::Adapter adapter;
    EXPECT_CALL(mMockCb, Call(wgpu::RequestAdapterStatus::Success, _, EmptySizedString()))
        .WillOnce(WithArg<1>([&](wgpu::Adapter result) { adapter = std::move(result); }));
    RequestAdapter(instance, &options);

    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (!adapter) {
        return;
    }

    wgpu::AdapterInfo info;
    adapter.GetInfo(&info);

    // Make a copy of the info.
    std::string vendor{std::string_view(info.vendor)};
    std::string architecture{std::string_view(info.architecture)};
    std::string device{std::string_view(info.device)};
    std::string description{std::string_view(info.description)};

    // Release the adapter.
    adapter = nullptr;

    // Ensure we still read the info (pointers are still valid).
    // Check the values are equal to make sure they haven't been overwritten,
    // and to make sure the compiler can't elide no-op pointer reads.
    EXPECT_EQ(std::string_view(info.vendor), vendor);
    EXPECT_EQ(std::string_view(info.architecture), architecture);
    EXPECT_EQ(std::string_view(info.device), device);
    EXPECT_EQ(std::string_view(info.description), description);
}

}  // anonymous namespace
}  // namespace dawn
