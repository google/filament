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

#include <memory>

#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"
#include "gmock/gmock.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::Ne;
using testing::Return;

static constexpr WGPUQueueWorkDoneStatus kError = static_cast<WGPUQueueWorkDoneStatus>(0);

using WireQueueTestBase = WireFutureTest<wgpu::QueueWorkDoneCallback<void>*>;
class WireQueueTests : public WireQueueTestBase {
  protected:
    void OnSubmittedWorkDone() {
        this->mFutureIDs.push_back(
            queue.OnSubmittedWorkDone(this->GetParam().callbackMode, this->mMockCb.Callback()).id);
    }
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireQueueTests);

// Test that a successful OnSubmittedWorkDone call is forwarded to the client.
TEST_P(WireQueueTests, OnSubmittedWorkDoneSuccess) {
    OnSubmittedWorkDone();

    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, WGPUQueueWorkDoneStatus_Success);
    }));
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::QueueWorkDoneStatus::Success)).Times(1);

        FlushCallbacks();
    });
}

// Test that an error OnSubmittedWorkDone call is forwarded as an error to the client.
TEST_P(WireQueueTests, OnSubmittedWorkDoneError) {
    OnSubmittedWorkDone();

    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, kError);
    }));
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(Ne(wgpu::QueueWorkDoneStatus::Success))).Times(1);

        FlushCallbacks();
    });
}

// Test registering an OnSubmittedWorkDone then disconnecting the wire after the server responded to
// the client will call the callback with instance dropped.
TEST_P(WireQueueTests, OnSubmittedWorkDoneBeforeDisconnectAfterReply) {
    // On Async and Spontaneous mode, it is not possible to simulate this because on the server
    // reponse, the callback would also be fired.
    DAWN_SKIP_TEST_IF(IsSpontaneous());

    OnSubmittedWorkDone();

    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, kError);
    }));
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::QueueWorkDoneStatus::CallbackCancelled)).Times(1);

        GetWireClient()->Disconnect();
    });
}

// Test registering an OnSubmittedWorkDone then disconnecting the wire before the server responded
// to the client (i.e. before the event was ever ready) will call the callback with instance
// dropped.
TEST_P(WireQueueTests, OnSubmittedWorkDoneBeforeDisconnectBeforeReply) {
    OnSubmittedWorkDone();

    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, kError);
    }));
    FlushClient();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::QueueWorkDoneStatus::CallbackCancelled)).Times(1);

        GetWireClient()->Disconnect();
    });
}

// Test registering an OnSubmittedWorkDone after disconnecting the wire calls the callback with
// success.
TEST_P(WireQueueTests, OnSubmittedWorkDoneAfterDisconnect) {
    GetWireClient()->Disconnect();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::QueueWorkDoneStatus::CallbackCancelled)).Times(1);

        OnSubmittedWorkDone();
    });
}

// Test that requests inside user callbacks before disconnect are called
TEST_P(WireQueueTests, OnSubmittedWorkDoneInsideCallbackBeforeDisconnect) {
    static constexpr size_t kNumRequests = 10;
    OnSubmittedWorkDone();

    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, kError);
    }));
    FlushClient();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::QueueWorkDoneStatus::CallbackCancelled))
            .Times(kNumRequests + 1)
            .WillOnce([&]() {
                for (size_t i = 0; i < kNumRequests; i++) {
                    OnSubmittedWorkDone();
                }
            })
            .WillRepeatedly(Return());

        GetWireClient()->Disconnect();
    });
}

// Test releasing the default queue, then its device. Both should be released when the device is
// released since the device holds a reference to the queue. Regresssion test for crbug.com/1332926.
TEST_F(WireQueueTests, DefaultQueueThenDeviceReleased) {
    // Note: The test fixture gets the default queue.

    // Release the queue which is the last external client reference.
    // The device still holds a reference.
    queue = nullptr;
    FlushClient();

    // Release the device which holds an internal reference to the queue.
    // Now, the queue and device should be released on the server.
    device = nullptr;

    EXPECT_CALL(api, QueueRelease(apiQueue));
    EXPECT_CALL(api, DeviceRelease(apiDevice));
    // These set X callback methods are called before the device is released.
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);
    FlushClient();

    // Indicate to the fixture that the device was already released.
    DefaultApiDeviceWasReleased();
}

// Test the device, then its default queue. The default queue should be released when its external
// reference is dropped since releasing the device drops the internal reference. Regresssion test
// for crbug.com/1332926.
TEST_F(WireQueueTests, DeviceThenDefaultQueueReleased) {
    // Note: The test fixture gets the default queue.

    // Release the device which holds an internal reference to the queue.
    // Now, the should be released on the server, but not the queue since
    // the default queue still has one external reference.
    device = nullptr;

    EXPECT_CALL(api, DeviceRelease(apiDevice));
    // These set X callback methods are called before the device is released.
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);
    FlushClient();

    // Release the external queue reference. The queue should be released.
    queue = nullptr;
    EXPECT_CALL(api, QueueRelease(apiQueue));
    FlushClient();

    // Indicate to the fixture that the device was already released.
    DefaultApiDeviceWasReleased();
}

// Only one default queue is supported now so we cannot test ~Queue triggering ClearAllCallbacks
// since it is always destructed after the test TearDown, and we cannot create a new queue obj
// with wgpuDeviceGetQueue

}  // anonymous namespace
}  // namespace dawn::wire
