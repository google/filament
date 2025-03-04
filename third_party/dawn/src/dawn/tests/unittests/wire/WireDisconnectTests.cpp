// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/wire/WireTest.h"

#include "dawn/common/Assert.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Exactly;
using testing::InvokeWithoutArgs;
using testing::MockCallback;
using testing::Return;
using testing::Sequence;
using testing::SizedString;

class WireDisconnectTests : public WireTest {};

// Test that commands are not received if the client disconnects.
TEST_F(WireDisconnectTests, CommandsAfterDisconnect) {
    // Check that commands work at all.
    wgpu::CommandEncoder encoder1 = device.CreateCommandEncoder();

    WGPUCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCmdBufEncoder));
    FlushClient();

    // Disconnect.
    GetWireClient()->Disconnect();

    // Command is not received because client disconnected.
    wgpu::CommandEncoder encoder2 = device.CreateCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(_, _)).Times(Exactly(0));
    FlushClient();
}

// Test that commands that are serialized before a disconnect but flushed
// after are received.
TEST_F(WireDisconnectTests, FlushAfterDisconnect) {
    // Check that commands work at all.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Disconnect.
    GetWireClient()->Disconnect();

    // Already-serialized commmands are still received.
    WGPUCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCmdBufEncoder));
    FlushClient();
}

// Check that disconnecting the wire client calls the device lost callback exacty once.
TEST_F(WireDisconnectTests, CallsDeviceLostCallback) {
    // Disconnect the wire client. We should receive device lost only once.
    EXPECT_CALL(deviceLostCallback, Call(_, wgpu::DeviceLostReason::InstanceDropped, _))
        .Times(Exactly(1));
    GetWireClient()->Disconnect();
    GetWireClient()->Disconnect();
}

// Check that disconnecting the wire client after a device loss does not trigger the callback
// again.
TEST_F(WireDisconnectTests, ServerLostThenDisconnect) {
    api.CallDeviceLostCallback(apiDevice, WGPUDeviceLostReason_Unknown,
                               ToOutputStringView("some reason"));

    // Flush the device lost return command.
    EXPECT_CALL(deviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                         SizedString("some reason")))
        .Times(Exactly(1));
    FlushServer();

    // Disconnect the client. We shouldn't see the lost callback again.
    GetWireClient()->Disconnect();
}

// Check that disconnecting the wire client inside the device loss callback does not trigger the
// callback again.
TEST_F(WireDisconnectTests, ServerLostThenDisconnectInCallback) {
    api.CallDeviceLostCallback(apiDevice, WGPUDeviceLostReason_Unknown,
                               ToOutputStringView("lost reason"));

    // Disconnect the client inside the lost callback. We should see the callback
    // only once.
    EXPECT_CALL(deviceLostCallback,
                Call(_, wgpu::DeviceLostReason::Unknown, SizedString("lost reason")))
        .Times(Exactly(1))
        .WillOnce(InvokeWithoutArgs([&] { GetWireClient()->Disconnect(); }));
    FlushServer();
}

// Check that a device loss after a disconnect does not trigger the callback again.
TEST_F(WireDisconnectTests, DisconnectThenServerLost) {
    // Disconnect the client. We should see the callback once.
    EXPECT_CALL(deviceLostCallback, Call(_, wgpu::DeviceLostReason::InstanceDropped, _))
        .Times(Exactly(1));
    GetWireClient()->Disconnect();

    // Lose the device on the server. The client callback shouldn't be
    // called again.
    api.CallDeviceLostCallback(apiDevice, WGPUDeviceLostReason_Unknown,
                               ToOutputStringView("lost reason"));
    FlushServer();
}

// Test that client objects are all destroyed if the WireClient is destroyed.
TEST_F(WireDisconnectTests, DeleteClientDestroysObjects) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::Sampler sampler = device.CreateSampler();

    WGPUCommandEncoder apiCommandEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCommandEncoder));

    WGPUSampler apiSampler = api.GetNewSampler();
    EXPECT_CALL(api, DeviceCreateSampler(apiDevice, _)).WillOnce(Return(apiSampler));

    FlushClient();

    DeleteClient();

    // Expect release on all objects created by the client.
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);
    EXPECT_CALL(api, DeviceRelease(apiDevice)).Times(1);
    EXPECT_CALL(api, QueueRelease(apiQueue)).Times(1);
    EXPECT_CALL(api, CommandEncoderRelease(apiCommandEncoder)).Times(1);
    EXPECT_CALL(api, SamplerRelease(apiSampler)).Times(1);
    EXPECT_CALL(api, AdapterRelease(apiAdapter)).Times(1);
    EXPECT_CALL(api, InstanceRelease(apiInstance)).Times(1);
    FlushClient();

    // Signal that we already released and cleared callbacks for |apiDevice|
    DefaultApiDeviceWasReleased();
    DefaultApiAdapterWasReleased();
}

}  // anonymous namespace
}  // namespace dawn::wire
