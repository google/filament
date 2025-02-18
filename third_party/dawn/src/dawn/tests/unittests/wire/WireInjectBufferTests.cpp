// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::Mock;
using testing::Return;

class WireInjectBufferTests : public WireTest {
  public:
    WireInjectBufferTests() {}
    ~WireInjectBufferTests() override = default;

    std::pair<ReservedBuffer, wgpu::Buffer> ReserveBuffer(
        const wgpu::BufferDescriptor* desc = &kPlaceholderDesc) {
        auto reservation = GetWireClient()->ReserveBuffer(
            device.Get(), reinterpret_cast<const WGPUBufferDescriptor*>(desc));
        return {reservation, wgpu::Buffer::Acquire(reservation.buffer)};
    }

    // A placeholder buffer format for ReserveBuffer. The data in it doesn't matter as long as
    // we don't call buffer reflection methods.
    static constexpr wgpu::BufferDescriptor kPlaceholderDesc = {};
};

// Test that reserving and injecting a buffer makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectBufferTests, CallAfterReserveInject) {
    auto [reservation, buffer] = ReserveBuffer();

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferAddRef(apiBuffer));
    ASSERT_TRUE(
        GetWireServer()->InjectBuffer(apiBuffer, reservation.handle, reservation.deviceHandle));

    buffer.Destroy();
    EXPECT_CALL(api, BufferDestroy(apiBuffer));
    FlushClient();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectBufferTests, ReserveDifferentIDs) {
    auto [reservation1, buffer1] = ReserveBuffer();
    auto [reservation2, buffer2] = ReserveBuffer();

    ASSERT_NE(reservation1.handle.id, reservation2.handle.id);
    ASSERT_NE(buffer1.Get(), buffer2.Get());
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectBufferTests, InjectExistingID) {
    auto [reservation, buffer] = ReserveBuffer();

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferAddRef(apiBuffer));
    ASSERT_TRUE(
        GetWireServer()->InjectBuffer(apiBuffer, reservation.handle, reservation.deviceHandle));

    // ID already in use, call fails.
    ASSERT_FALSE(
        GetWireServer()->InjectBuffer(apiBuffer, reservation.handle, reservation.deviceHandle));
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectBufferTests, ReuseIDAndGeneration) {
    // Do this loop multiple times since the first time, we can't test `generation - 1` since
    // generation == 0.
    ReservedBuffer reservation;
    wgpu::Buffer buffer;
    WGPUBuffer apiBuffer = nullptr;
    for (int i = 0; i < 2; ++i) {
        std::tie(reservation, buffer) = ReserveBuffer();

        apiBuffer = api.GetNewBuffer();
        EXPECT_CALL(api, BufferAddRef(apiBuffer));
        ASSERT_TRUE(
            GetWireServer()->InjectBuffer(apiBuffer, reservation.handle, reservation.deviceHandle));

        // Release the buffer. It should be possible to reuse the ID now, but not the generation
        buffer = nullptr;
        EXPECT_CALL(api, BufferRelease(apiBuffer));
        FlushClient();

        // Invalid to inject with the same ID and generation.
        ASSERT_FALSE(
            GetWireServer()->InjectBuffer(apiBuffer, reservation.handle, reservation.deviceHandle));
        if (i > 0) {
            EXPECT_GE(reservation.handle.generation, 1u);

            // Invalid to inject with the same ID and lesser generation.
            reservation.handle.generation -= 1;
            ASSERT_FALSE(GetWireServer()->InjectBuffer(apiBuffer, reservation.handle,
                                                       reservation.deviceHandle));
        }
    }

    // Valid to inject with the same ID and greater generation.
    EXPECT_CALL(api, BufferAddRef(apiBuffer));
    reservation.handle.generation += 2;
    ASSERT_TRUE(
        GetWireServer()->InjectBuffer(apiBuffer, reservation.handle, reservation.deviceHandle));
}

// Test that the server only borrows the buffer and does a single reference-release
TEST_F(WireInjectBufferTests, InjectedBufferLifetime) {
    auto [reservation, buffer] = ReserveBuffer();

    // Injecting the buffer adds a reference
    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferAddRef(apiBuffer));
    ASSERT_TRUE(
        GetWireServer()->InjectBuffer(apiBuffer, reservation.handle, reservation.deviceHandle));

    // Releasing the buffer removes a single reference.
    buffer = nullptr;
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that a buffer reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectBufferTests, ReclaimBufferReservation) {
    // Test that doing a reservation and full release is an error.
    {
        auto [reservation, buffer] = ReserveBuffer();
        buffer = nullptr;
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        auto [reservation1, buffer1] = ReserveBuffer();
        GetWireClient()->ReclaimBufferReservation(reservation1);

        auto [reservation2, buffer2] = ReserveBuffer();

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.handle.id, reservation2.handle.id);
        ASSERT_NE(reservation1.handle.generation, reservation2.handle.generation);

        // No errors should occur.
        FlushClient();
    }
}

// Test the reflection of buffer creation parameters for reserved buffer.
TEST_F(WireInjectBufferTests, ReservedBufferReflection) {
    wgpu::BufferDescriptor desc;
    desc.size = 10;
    desc.usage = wgpu::BufferUsage::Storage;

    auto [reservation, buffer] = ReserveBuffer(&desc);
    ASSERT_EQ(desc.size, buffer.GetSize());
    ASSERT_EQ(desc.usage, buffer.GetUsage());
}

}  // anonymous namespace
}  // namespace dawn::wire
