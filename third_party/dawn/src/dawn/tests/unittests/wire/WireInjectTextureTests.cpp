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

#include <utility>

#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::Mock;
using testing::Return;

class WireInjectTextureTests : public WireTest {
  public:
    WireInjectTextureTests() {}
    ~WireInjectTextureTests() override = default;

    std::pair<ReservedTexture, wgpu::Texture> ReserveTexture(
        const wgpu::TextureDescriptor* desc = &kPlaceholderDesc) {
        auto reservation = GetWireClient()->ReserveTexture(
            device.Get(), reinterpret_cast<const WGPUTextureDescriptor*>(desc));
        return {reservation, wgpu::Texture::Acquire(reservation.texture)};
    }

    // A placeholder texture format for ReserveTexture. The data in it doesn't matter as long as
    // we don't call texture reflection methods.
    static constexpr wgpu::TextureDescriptor kPlaceholderDesc = {};
};

// Test that reserving and injecting a texture makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectTextureTests, CallAfterReserveInject) {
    auto [reservation, texture] = ReserveTexture();

    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, TextureAddRef(apiTexture));
    ASSERT_TRUE(
        GetWireServer()->InjectTexture(apiTexture, reservation.handle, reservation.deviceHandle));

    wgpu::TextureView view = texture.CreateView();
    WGPUTextureView apiPlaceholderView = api.GetNewTextureView();
    EXPECT_CALL(api, TextureCreateView(apiTexture, nullptr)).WillOnce(Return(apiPlaceholderView));
    FlushClient();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectTextureTests, ReserveDifferentIDs) {
    auto [reservation1, texture1] = ReserveTexture();
    auto [reservation2, texture2] = ReserveTexture();

    ASSERT_NE(reservation1.handle.id, reservation2.handle.id);
    ASSERT_NE(texture1.Get(), texture2.Get());
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectTextureTests, InjectExistingID) {
    auto [reservation, texture] = ReserveTexture();

    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, TextureAddRef(apiTexture));
    ASSERT_TRUE(
        GetWireServer()->InjectTexture(apiTexture, reservation.handle, reservation.deviceHandle));

    // ID already in use, call fails.
    ASSERT_FALSE(
        GetWireServer()->InjectTexture(apiTexture, reservation.handle, reservation.deviceHandle));
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectTextureTests, ReuseIDAndGeneration) {
    // Do this loop multiple times since the first time, we can't test `generation - 1` since
    // generation == 0.
    ReservedTexture reservation;
    wgpu::Texture texture;
    WGPUTexture apiTexture = nullptr;
    for (int i = 0; i < 2; ++i) {
        std::tie(reservation, texture) = ReserveTexture();

        apiTexture = api.GetNewTexture();
        EXPECT_CALL(api, TextureAddRef(apiTexture));
        ASSERT_TRUE(GetWireServer()->InjectTexture(apiTexture, reservation.handle,
                                                   reservation.deviceHandle));

        // Release the texture. It should be possible to reuse the ID now, but not the generation
        texture = nullptr;
        EXPECT_CALL(api, TextureRelease(apiTexture));
        FlushClient();

        // Invalid to inject with the same ID and generation.
        ASSERT_FALSE(GetWireServer()->InjectTexture(apiTexture, reservation.handle,
                                                    reservation.deviceHandle));
        if (i > 0) {
            EXPECT_GE(reservation.handle.generation, 1u);

            // Invalid to inject with the same ID and lesser generation.
            reservation.handle.generation -= 1;
            ASSERT_FALSE(GetWireServer()->InjectTexture(apiTexture, reservation.handle,
                                                        reservation.deviceHandle));
        }
    }

    // Valid to inject with the same ID and greater generation.
    EXPECT_CALL(api, TextureAddRef(apiTexture));
    reservation.handle.generation += 2;
    ASSERT_TRUE(
        GetWireServer()->InjectTexture(apiTexture, reservation.handle, reservation.deviceHandle));
}

// Test that the server only borrows the texture and does a single addref-release
TEST_F(WireInjectTextureTests, InjectedTextureLifetime) {
    auto [reservation, texture] = ReserveTexture();

    // Injecting the texture adds a reference
    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, TextureAddRef(apiTexture));
    ASSERT_TRUE(
        GetWireServer()->InjectTexture(apiTexture, reservation.handle, reservation.deviceHandle));

    // Releasing the texture removes a single reference.
    texture = nullptr;
    EXPECT_CALL(api, TextureRelease(apiTexture));
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that a texture reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectTextureTests, ReclaimTextureReservation) {
    // Test that doing a reservation and full release is an error.
    {
        auto [reservation, texture] = ReserveTexture();
        texture = nullptr;
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        auto [reservation1, texture1] = ReserveTexture();
        GetWireClient()->ReclaimTextureReservation(reservation1);

        auto [reservation2, texture2] = ReserveTexture();

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.handle.id, reservation2.handle.id);
        ASSERT_NE(reservation1.handle.generation, reservation2.handle.generation);

        // No errors should occur.
        FlushClient();
    }
}

// Test the reflection of texture creation parameters for reserved textures.
TEST_F(WireInjectTextureTests, ReservedTextureReflection) {
    wgpu::TextureDescriptor desc = {};
    desc.size = {10, 11, 12};
    desc.format = wgpu::TextureFormat::R32Float;
    desc.dimension = wgpu::TextureDimension::e3D;
    desc.mipLevelCount = 1000;
    desc.sampleCount = 3;
    desc.usage = wgpu::TextureUsage::RenderAttachment;

    auto [reservation, texture] = ReserveTexture(&desc);

    ASSERT_EQ(desc.size.width, texture.GetWidth());
    ASSERT_EQ(desc.size.height, texture.GetHeight());
    ASSERT_EQ(desc.size.depthOrArrayLayers, texture.GetDepthOrArrayLayers());
    ASSERT_EQ(desc.format, texture.GetFormat());
    ASSERT_EQ(desc.dimension, texture.GetDimension());
    ASSERT_EQ(desc.mipLevelCount, texture.GetMipLevelCount());
    ASSERT_EQ(desc.sampleCount, texture.GetSampleCount());
}

}  // anonymous namespace
}  // namespace dawn::wire
