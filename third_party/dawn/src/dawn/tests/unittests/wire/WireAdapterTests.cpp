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

#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"
#include "webgpu/webgpu_cpp.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::EmptySizedString;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::IsNull;
using testing::NonEmptySizedString;
using testing::NotNull;
using testing::Return;
using testing::SizedString;
using testing::WithArg;

using WireAdapterTestBase = WireFutureTest<wgpu::RequestDeviceCallback<void>*>;
class WireAdapterTests : public WireAdapterTestBase {
  protected:
    void RequestDevice(const wgpu::DeviceDescriptor* descriptor) {
        this->mFutureIDs.push_back(
            adapter
                .RequestDevice(descriptor, this->GetParam().callbackMode, this->mMockCb.Callback())
                .id);
    }
};
DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireAdapterTests);

// Test that an empty DeviceDescriptor is passed from the client to the server.
TEST_P(WireAdapterTests, RequestDeviceEmptyDescriptor) {
    wgpu::DeviceDescriptor desc = {};
    RequestDevice(&desc);

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>(Invoke([&](const WGPUDeviceDescriptor* apiDesc) {
            EXPECT_EQ(apiDesc->label.data, nullptr);
            EXPECT_EQ(apiDesc->requiredFeatureCount, 0u);
            EXPECT_EQ(apiDesc->requiredLimits, nullptr);

            // Call the callback so the test doesn't wait indefinitely.
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 kEmptyOutputStringView);
        })));
    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call).Times(1);

        FlushCallbacks();
    });
}

// Test that a null DeviceDescriptor is passed from the client to the server as an empty one.
TEST_P(WireAdapterTests, RequestDeviceNullDescriptor) {
    RequestDevice(nullptr);

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>(Invoke([&](const WGPUDeviceDescriptor* apiDesc) {
            EXPECT_EQ(apiDesc->label.data, nullptr);
            EXPECT_EQ(apiDesc->requiredFeatureCount, 0u);
            EXPECT_EQ(apiDesc->requiredLimits, nullptr);

            // Call the callback so the test doesn't wait indefinitely.
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 kEmptyOutputStringView);
        })));
    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call).Times(1);

        FlushCallbacks();
    });
}

// Test that the DeviceDescriptor is not allowed to pass callbacks from the client to
// the server.
TEST_P(WireAdapterTests, RequestDeviceCallbackPointers) {
    wgpu::DeviceDescriptor desc = {};
    desc.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device&, wgpu::DeviceLostReason, wgpu::StringView) {});
    desc.SetUncapturedErrorCallback([](const wgpu::Device&, wgpu::ErrorType, wgpu::StringView) {});
    RequestDevice(&desc);

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>(Invoke([&](const WGPUDeviceDescriptor* apiDesc) {
            EXPECT_STREQ(apiDesc->label.data, desc.label.data);

            // The callback should not be passed through to the server, and it should be overridden.
            WGPUDeviceDescriptor& inputDesc = *reinterpret_cast<WGPUDeviceDescriptor*>(&desc);
            ASSERT_NE(apiDesc->deviceLostCallbackInfo.callback,
                      inputDesc.deviceLostCallbackInfo.callback);
            ASSERT_NE(apiDesc->deviceLostCallbackInfo.callback, nullptr);
            ASSERT_NE(apiDesc->uncapturedErrorCallbackInfo.callback,
                      inputDesc.uncapturedErrorCallbackInfo.callback);
            ASSERT_NE(apiDesc->uncapturedErrorCallbackInfo.callback, nullptr);

            // Call the callback so the test doesn't wait indefinitely.
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 kEmptyOutputStringView);
        })));
    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call).Times(1);

        FlushCallbacks();
    });
}

// Test that RequestDevice forwards the device information to the client.
TEST_P(WireAdapterTests, RequestDeviceSuccess) {
    wgpu::SupportedLimits fakeLimits = {};
    fakeLimits.limits.maxTextureDimension1D = 433;
    fakeLimits.limits.maxVertexAttributes = 1243;

    std::initializer_list<WGPUFeatureName> fakeFeaturesList = {
        WGPUFeatureName_Depth32FloatStencil8,
        WGPUFeatureName_TextureCompressionBC,
    };
    WGPUSupportedFeatures fakeFeatures = {fakeFeaturesList.size(), std::data(fakeFeaturesList)};

    wgpu::DeviceDescriptor desc = {};
    RequestDevice(&desc);

    // Expect the server to receive the message. Then, mock a fake reply.
    WGPUDevice apiDevice = api.GetNewDevice();
    // The backend device should not be known by the wire server.
    EXPECT_FALSE(GetWireServer()->IsDeviceKnown(apiDevice));

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            // Set on device creation to forward callbacks to the client.
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);

            EXPECT_CALL(api, DeviceGetLimits(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *reinterpret_cast<wgpu::SupportedLimits*>(limits) = fakeLimits;
                    return WGPUStatus_Success;
                })));

            EXPECT_CALL(api, DeviceGetFeatures(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(
                    Invoke([&](WGPUSupportedFeatures* features) { *features = fakeFeatures; })));

            // The backend device should still not be known by the wire server since the
            // callback has not been called yet.
            EXPECT_FALSE(GetWireServer()->IsDeviceKnown(apiDevice));
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success,
                                                 apiDevice, kEmptyOutputStringView);
            // After the callback is called, the backend device is now known by the server.
            EXPECT_TRUE(GetWireServer()->IsDeviceKnown(apiDevice));
        }));

    FlushClient();
    FlushFutures();

    wgpu::Device device;
    // Expect the callback in the client and all the device information to match.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
            .WillOnce(WithArg<1>(Invoke([&](wgpu::Device result) {
                device = std::move(result);

                wgpu::AdapterInfo adapterInfo;
                EXPECT_EQ(device.GetAdapterInfo(&adapterInfo), wgpu::Status::Success);
                EXPECT_EQ(adapterInfo.vendor, kEmptyOutputStringView);
                EXPECT_EQ(adapterInfo.architecture, kEmptyOutputStringView);
                EXPECT_EQ(adapterInfo.device, kEmptyOutputStringView);
                EXPECT_EQ(adapterInfo.description, kEmptyOutputStringView);

                wgpu::SupportedLimits limits;
                EXPECT_EQ(device.GetLimits(&limits), wgpu::Status::Success);
                EXPECT_EQ(limits.limits.maxTextureDimension1D,
                          fakeLimits.limits.maxTextureDimension1D);
                EXPECT_EQ(limits.limits.maxVertexAttributes, fakeLimits.limits.maxVertexAttributes);

                WGPUSupportedFeatures features = {};
                device.GetFeatures(reinterpret_cast<wgpu::SupportedFeatures*>(&features));

                std::vector<WGPUFeatureName> featuresList(
                    features.features, features.features + features.featureCount);
                ASSERT_EQ(featuresList.size(), fakeFeaturesList.size());
                std::unordered_set<WGPUFeatureName> featureSet(fakeFeaturesList);
                for (WGPUFeatureName feature : featuresList) {
                    EXPECT_EQ(featureSet.erase(feature), 1u);
                }
            })));
        FlushCallbacks();
    });

    EXPECT_EQ(device.GetAdapter().Get(), adapter.Get());

    device = nullptr;
    // Cleared when the device is destroyed.
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);
    EXPECT_CALL(api, DeviceRelease(apiDevice));

    // Server has not recevied the release yet, so the device should be known.
    EXPECT_TRUE(GetWireServer()->IsDeviceKnown(apiDevice));
    FlushClient();
    // After receiving the release call, the device is no longer known by the server.
    EXPECT_FALSE(GetWireServer()->IsDeviceKnown(apiDevice));
}

// Test that features requested that the implementation supports, but not the
// wire reject the callback.
TEST_P(WireAdapterTests, RequestFeatureUnsupportedByWire) {
    std::initializer_list<WGPUFeatureName> fakeFeaturesList = {
        // Default feature is an undefined feature.
        {},
        WGPUFeatureName_TextureCompressionASTC,
    };
    WGPUSupportedFeatures fakeFeatures = {fakeFeaturesList.size(), std::data(fakeFeaturesList)};

    wgpu::DeviceDescriptor desc = {};
    RequestDevice(&desc);

    // Expect the server to receive the message. Then, mock a fake reply.
    // The reply contains features that the device implementation supports, but the
    // wire does not.
    WGPUDevice apiDevice = api.GetNewDevice();
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            EXPECT_CALL(api, DeviceGetFeatures(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(
                    Invoke([&](WGPUSupportedFeatures* features) { *features = fakeFeatures; })));

            // The device was actually created, but the wire didn't support its features.
            // Expect it to be released.
            EXPECT_CALL(api, DeviceRelease(apiDevice));

            // Fake successful creation. The client still receives a failure due to
            // unsupported features.
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success,
                                                 apiDevice, kEmptyOutputStringView);
        }));
    FlushClient();
    FlushFutures();

    // Expect an error callback since the feature is not supported.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::RequestDeviceStatus::Error, IsNull(), NonEmptySizedString()))
            .Times(1);

        FlushCallbacks();
    });
}

// Test that RequestDevice errors forward to the client.
TEST_P(WireAdapterTests, RequestDeviceError) {
    wgpu::DeviceDescriptor desc = {};
    RequestDevice(&desc);

    // Expect the server to receive the message. Then, mock an error.
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 ToOutputStringView("Request device failed"));
        }));
    FlushClient();
    FlushFutures();

    // Expect the callback in the client.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::RequestDeviceStatus::Error, IsNull(),
                                 SizedString("Request device failed")))
            .Times(1);
        FlushCallbacks();
    });
}

// Test that RequestDevice can complete successfully even if the adapter is deleted
// before the callback happens.
TEST_P(WireAdapterTests, RequestDeviceAdapterDestroyedBeforeCallback) {
    wgpu::DeviceDescriptor desc = {};
    RequestDevice(&desc);
    adapter = nullptr;

    // Mock a reply from the server.
    WGPUDevice apiDevice = api.GetNewDevice();
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            // Set on device creation to forward callbacks to the client.
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);
            EXPECT_CALL(api, DeviceGetLimits(apiDevice, NotNull())).Times(1);
            EXPECT_CALL(api, DeviceGetFeatures(apiDevice, NotNull())).Times(1);

            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success,
                                                 apiDevice, kEmptyOutputStringView);
        }));
    FlushClient();
    FlushFutures();

    wgpu::Device device;
    // Expect the callback in the client.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()))
            .WillOnce(WithArg<1>(Invoke([&](wgpu::Device result) { device = std::move(result); })));
        FlushCallbacks();
    });

    device = nullptr;
    // Cleared when the device is destroyed.
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);
    EXPECT_CALL(api, DeviceRelease(apiDevice));
    FlushClient();
}

// Test that RequestDevice receives unknown status if the wire is disconnected
// before the callback happens.
TEST_P(WireAdapterTests, RequestDeviceWireDisconnectedBeforeCallback) {
    wgpu::DeviceDescriptor desc = {};
    RequestDevice(&desc);

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::RequestDeviceStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

}  // anonymous namespace
}  // namespace dawn::wire
