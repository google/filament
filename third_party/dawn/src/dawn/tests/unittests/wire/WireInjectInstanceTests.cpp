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

#include <array>

#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Mock;
using testing::MockCallback;
using testing::NotNull;
using testing::Return;

class WireInjectInstanceTests : public WireTest {
  public:
    WireInjectInstanceTests() {}
    ~WireInjectInstanceTests() override = default;
};

// Test that reserving and injecting an instance makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectInstanceTests, CallAfterReserveInject) {
    auto reserved = GetWireClient()->ReserveInstance();

    WGPUInstance serverInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceAddRef(serverInstance));
    ASSERT_TRUE(GetWireServer()->InjectInstance(serverInstance, reserved.handle));

    MockCallback<void (*)(wgpu::RequestAdapterStatus, wgpu::Adapter, wgpu::StringView, void*)>
        adapterCb;
    instance.RequestAdapter(nullptr, wgpu::CallbackMode::AllowSpontaneous, adapterCb.Callback(),
                            adapterCb.MakeUserdata(this));

    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, _, _)).WillOnce([&]() {
        api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Error, nullptr,
                                               ToOutputStringView("Some error message."));
    });
    FlushClient();

    EXPECT_CALL(adapterCb, Call(wgpu::RequestAdapterStatus::Error, _, _, this));
    FlushServer();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectInstanceTests, ReserveDifferentIDs) {
    auto reserved1 = GetWireClient()->ReserveInstance();
    auto reserved2 = GetWireClient()->ReserveInstance();

    ASSERT_NE(reserved1.handle.id, reserved2.handle.id);
    ASSERT_NE(reserved1.instance, reserved2.instance);
}

// Test that injecting the same id fails.
TEST_F(WireInjectInstanceTests, InjectExistingID) {
    auto reserved = GetWireClient()->ReserveInstance();

    WGPUInstance serverInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceAddRef(serverInstance));
    ASSERT_TRUE(GetWireServer()->InjectInstance(serverInstance, reserved.handle));

    // ID already in use, call fails.
    ASSERT_FALSE(GetWireServer()->InjectInstance(serverInstance, reserved.handle));
}

// Test that the server only borrows the instance and does a single addref-release
TEST_F(WireInjectInstanceTests, InjectedInstanceLifetime) {
    auto reserved = GetWireClient()->ReserveInstance();

    // Injecting the instance adds a reference
    WGPUInstance serverInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceAddRef(serverInstance));
    ASSERT_TRUE(GetWireServer()->InjectInstance(serverInstance, reserved.handle));

    // Releasing the instance removes a single reference.
    wgpuInstanceRelease(reserved.instance);
    EXPECT_CALL(api, InstanceRelease(serverInstance));
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that a device reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectInstanceTests, ReclaimInstanceReservation) {
    // Test that doing a reservation and full release is an error.
    {
        auto reserved = GetWireClient()->ReserveInstance();
        wgpuInstanceRelease(reserved.instance);
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        auto reserved1 = GetWireClient()->ReserveInstance();
        GetWireClient()->ReclaimInstanceReservation(reserved1);

        auto reserved2 = GetWireClient()->ReserveInstance();

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reserved1.handle.id, reserved2.handle.id);
        ASSERT_NE(reserved1.handle.generation, reserved2.handle.generation);

        // No errors should occur.
        FlushClient();
    }
}

}  // anonymous namespace
}  // namespace dawn::wire
