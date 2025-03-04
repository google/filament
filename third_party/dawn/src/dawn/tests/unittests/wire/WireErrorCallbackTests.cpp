// Copyright 2019 The Dawn & Tint Authors
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
#include <utility>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::DoAll;
using testing::EmptySizedString;
using testing::InvokeWithoutArgs;
using testing::Mock;
using testing::Return;
using testing::SaveArg;
using testing::SizedString;
using testing::StrictMock;

class WireErrorCallbackTests : public WireTest {};

// Test the return wire for device user warning callbacks
TEST_F(WireErrorCallbackTests, DeviceLoggingCallback) {
    testing::MockCppCallback<wgpu::LoggingCallback<void>*> mockCallback;
    device.SetLoggingCallback(mockCallback.Callback());

    // Setting the injected warning callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceSetLoggingCallbackCallback(apiDevice, WGPULoggingType_Info,
                                             ToOutputStringView("Some message"));

    EXPECT_CALL(mockCallback, Call(wgpu::LoggingType::Info, SizedString("Some message"))).Times(1);

    FlushServer();
}

// Test the return wire for device error callbacks.
TEST_F(WireErrorCallbackTests, DeviceErrorCallbacks) {
    static constexpr std::array<wgpu::ErrorType, 3> kErrorTypes = {
        wgpu::ErrorType::Validation, wgpu::ErrorType::OutOfMemory, wgpu::ErrorType::Internal};

    for (auto type : kErrorTypes) {
        // Calling the callback on the server side will result in the callback being called on the
        // client side when the server is flushed.
        api.CallDeviceUncapturedErrorCallback(apiDevice, static_cast<WGPUErrorType>(type),
                                              ToOutputStringView("Some error message"));
        EXPECT_CALL(uncapturedErrorCallback,
                    Call(CHandleIs(device.Get()), type, SizedString("Some error message")))
            .Times(1);
    }

    FlushServer();
}

// Test the return wire for device lost callback.
TEST_F(WireErrorCallbackTests, DeviceLostCallback) {
    // Calling the callback on the server side will result in the callback being called on the
    // client side when the server is flushed.
    api.CallDeviceLostCallback(apiDevice, WGPUDeviceLostReason_Unknown,
                               ToOutputStringView("Some error message"));

    EXPECT_CALL(deviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                         SizedString("Some error message")))
        .Times(1);

    FlushServer();
}

using WirePopErrorScopeCallbackTestBase = WireFutureTest<wgpu::PopErrorScopeCallback<void>*>;
class WirePopErrorScopeCallbackTests : public WirePopErrorScopeCallbackTestBase {
  protected:
    void PopErrorScope() {
        this->mFutureIDs.push_back(
            device.PopErrorScope(this->GetParam().callbackMode, this->mMockCb.Callback()).id);
    }

    void PushErrorScope(wgpu::ErrorFilter filter) {
        EXPECT_CALL(api, DevicePushErrorScope(apiDevice, static_cast<WGPUErrorFilter>(filter)))
            .Times(1);
        device.PushErrorScope(filter);
        FlushClient();
    }
};
DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WirePopErrorScopeCallbackTests);

// Test the return wire for validation error scopes.
TEST_P(WirePopErrorScopeCallbackTests, TypeAndFilters) {
    static constexpr std::array<std::pair<wgpu::ErrorType, wgpu::ErrorFilter>, 3>
        kErrorTypeAndFilters = {{{wgpu::ErrorType::Validation, wgpu::ErrorFilter::Validation},
                                 {wgpu::ErrorType::OutOfMemory, wgpu::ErrorFilter::OutOfMemory},
                                 {wgpu::ErrorType::Internal, wgpu::ErrorFilter::Internal}}};

    for (const auto& [type, filter] : kErrorTypeAndFilters) {
        PushErrorScope(filter);
        PopErrorScope();
        EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _)).WillOnce([&] {
            api.CallDevicePopErrorScopeCallback(apiDevice, WGPUPopErrorScopeStatus_Success,
                                                static_cast<WGPUErrorType>(type),
                                                ToOutputStringView("Some error message"));
        });

        FlushClient();
        FlushFutures();
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::PopErrorScopeStatus::Success, type,
                                     SizedString("Some error message")))
                .Times(1);

            FlushCallbacks();
        });
    }
}

// Wire disconnect before server response calls the callback with Unknown error type.
TEST_P(WirePopErrorScopeCallbackTests, DisconnectBeforeServerReply) {
    PushErrorScope(wgpu::ErrorFilter::Validation);

    PopErrorScope();
    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _)).Times(1);

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::PopErrorScopeStatus::InstanceDropped,
                                 wgpu::ErrorType::NoError, EmptySizedString()))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

// Wire disconnect after server response calls the callback with returned error type.
TEST_P(WirePopErrorScopeCallbackTests, DisconnectAfterServerReply) {
    // On Async and Spontaneous mode, it is not possible to simulate this because on the server
    // reponse, the callback would also be fired.
    DAWN_SKIP_TEST_IF(IsSpontaneous());

    PushErrorScope(wgpu::ErrorFilter::Validation);
    PopErrorScope();

    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallDevicePopErrorScopeCallback(apiDevice, WGPUPopErrorScopeStatus_Success,
                                            WGPUErrorType_Validation,
                                            ToOutputStringView("Some error message"));
    }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::PopErrorScopeStatus::InstanceDropped,
                                 wgpu::ErrorType::Validation, EmptySizedString()))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

// Empty stack (We are emulating the errors that would be callback-ed from native).
TEST_P(WirePopErrorScopeCallbackTests, EmptyStack) {
    PopErrorScope();

    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallDevicePopErrorScopeCallback(apiDevice, WGPUPopErrorScopeStatus_Success,
                                            WGPUErrorType_NoError,
                                            ToOutputStringView("No error scopes to pop"));
    }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::PopErrorScopeStatus::Success, wgpu::ErrorType::NoError,
                                 SizedString("No error scopes to pop")))
            .Times(1);

        FlushCallbacks();
    });
}

}  // anonymous namespace
}  // namespace dawn::wire
