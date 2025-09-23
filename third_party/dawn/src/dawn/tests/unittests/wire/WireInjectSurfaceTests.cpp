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

#include <array>
#include <utility>

#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Mock;
using testing::Return;
using testing::WithArgs;

class WireInjectSurfaceTests : public WireTest {
  public:
    void SetUp() override {
        WireTest::SetUp();

        mCapabilities.usages = mSupportedUsages;
        mCapabilities.formatCount = mSupportedFormats.size();
        mCapabilities.formats = mSupportedFormats.data();
        mCapabilities.presentModeCount = mSupportedPresentModes.size();
        mCapabilities.presentModes = mSupportedPresentModes.data();
        mCapabilities.alphaModeCount = mSupportedAlphaModes.size();
        mCapabilities.alphaModes = mSupportedAlphaModes.data();

        mConfiguration.format = wgpu::TextureFormat::RGBA8Unorm;
        mConfiguration.alphaMode = wgpu::CompositeAlphaMode::Opaque;
        mConfiguration.presentMode = wgpu::PresentMode::Fifo;
        mConfiguration.width = 100;
        mConfiguration.height = 100;
        mConfiguration.usage = wgpu::TextureUsage::RenderAttachment;
        mConfiguration.device = device;
        mConfiguration.viewFormatCount = 0;
    }

    void TearDown() override {
        mConfiguration.device = nullptr;

        WireTest::TearDown();
    }

    std::pair<ReservedSurface, wgpu::Surface> ReserveSurface(const WGPUSurfaceCapabilities* caps) {
        auto reservation = GetWireClient()->ReserveSurface(instance.Get(), &mCapabilities);
        return {reservation, wgpu::Surface::Acquire(reservation.surface)};
    }

    WGPUSurfaceCapabilities mCapabilities;
    WGPUTextureUsage mSupportedUsages =
        WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    std::array<WGPUTextureFormat, 2> mSupportedFormats = {WGPUTextureFormat_RGBA8Unorm,
                                                          WGPUTextureFormat_RGBA8UnormSrgb};
    std::array<WGPUPresentMode, 2> mSupportedPresentModes = {WGPUPresentMode_Fifo,
                                                             WGPUPresentMode_Mailbox};
    std::array<WGPUCompositeAlphaMode, 2> mSupportedAlphaModes = {WGPUCompositeAlphaMode_Opaque,
                                                                  WGPUCompositeAlphaMode_Inherit};

    wgpu::SurfaceConfiguration mConfiguration;
};

// Test that reserving and injecting a surface makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectSurfaceTests, CallAfterReserveInject) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    // The client assumes a reserved surface is unconfigured, so it can't present.
    EXPECT_EQ(wgpu::Status::Error, surface.Present());
    FlushClient();

    surface.Configure(&mConfiguration);
    EXPECT_CALL(api, SurfaceConfigure(apiSurface, _));
    EXPECT_EQ(wgpu::Status::Success, surface.Present());
    EXPECT_CALL(api, SurfacePresent(apiSurface)).WillOnce(Return(WGPUStatus_Success));
    FlushClient();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectSurfaceTests, ReserveDifferentIDs) {
    auto [reservation1, surface1] = ReserveSurface(&mCapabilities);
    auto [reservation2, surface2] = ReserveSurface(&mCapabilities);

    ASSERT_NE(reservation1.handle.id, reservation2.handle.id);
    ASSERT_NE(surface1.Get(), surface2.Get());
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectSurfaceTests, InjectExistingID) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    // ID already in use, call fails.
    ASSERT_FALSE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));
}

// Test that the server only borrows the surface and does a single addref-release
TEST_F(WireInjectSurfaceTests, InjectedSurfaceLifetime) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    // Injecting the surface adds a reference
    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    // Releasing the surface removes a single reference.
    surface = nullptr;
    EXPECT_CALL(api, SurfaceRelease(apiSurface));
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that a surface reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectSurfaceTests, ReclaimSurfaceReservation) {
    // Test that doing a reservation and full release is an error.
    {
        auto [reservation, surface] = ReserveSurface(&mCapabilities);
        surface = nullptr;
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        auto [reservation1, surface1] = ReserveSurface(&mCapabilities);
        GetWireClient()->ReclaimSurfaceReservation(reservation1);

        auto [reservation2, surface2] = ReserveSurface(&mCapabilities);

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.handle.id, reservation2.handle.id);
        ASSERT_NE(reservation1.handle.generation, reservation2.handle.generation);

        // No errors should occur.
        FlushClient();
    }
}

// Test that the surface returns capabilities that were provided during the injection.
TEST_F(WireInjectSurfaceTests, Capabilities) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    wgpu::SurfaceCapabilities caps;
    wgpu::Status status = surface.GetCapabilities(adapter, &caps);
    EXPECT_EQ(status, wgpu::Status::Success);

    EXPECT_EQ(caps.formatCount, mCapabilities.formatCount);
    for (uint32_t i = 0; i < caps.formatCount; i++) {
        EXPECT_EQ(static_cast<WGPUTextureFormat>(caps.formats[i]), mCapabilities.formats[i]);
    }

    EXPECT_EQ(caps.presentModeCount, mCapabilities.presentModeCount);
    for (uint32_t i = 0; i < caps.presentModeCount; i++) {
        EXPECT_EQ(static_cast<WGPUPresentMode>(caps.presentModes[i]),
                  mCapabilities.presentModes[i]);
    }

    EXPECT_EQ(caps.alphaModeCount, mCapabilities.alphaModeCount);
    for (uint32_t i = 0; i < caps.alphaModeCount; i++) {
        EXPECT_EQ(static_cast<WGPUCompositeAlphaMode>(caps.alphaModes[i]),
                  mCapabilities.alphaModes[i]);
    }

    EXPECT_EQ(static_cast<WGPUTextureUsage>(caps.usages), mCapabilities.usages);
}

// Test that the texture's reflection is correct for injected surface in the wire.
TEST_F(WireInjectSurfaceTests, TextureReflection) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    surface.Configure(&mConfiguration);

    wgpu::SurfaceTexture texture;
    surface.GetCurrentTexture(&texture);
    EXPECT_EQ(texture.status, wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal);

    EXPECT_EQ(mConfiguration.width, texture.texture.GetWidth());
    EXPECT_EQ(mConfiguration.height, texture.texture.GetHeight());
    EXPECT_EQ(mConfiguration.usage, texture.texture.GetUsage());
    EXPECT_EQ(mConfiguration.format, texture.texture.GetFormat());
    EXPECT_EQ(1u, texture.texture.GetDepthOrArrayLayers());
    EXPECT_EQ(1u, texture.texture.GetMipLevelCount());
    EXPECT_EQ(1u, texture.texture.GetSampleCount());
    EXPECT_EQ(wgpu::TextureDimension::e2D, texture.texture.GetDimension());
}

// Test a successful call to GetCurrentTexture.
TEST_F(WireInjectSurfaceTests, GetCurrentTextureSuccess) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    surface.Configure(&mConfiguration);
    EXPECT_CALL(api, SurfaceConfigure(apiSurface, _));

    wgpu::SurfaceTexture texture;
    surface.GetCurrentTexture(&texture);
    EXPECT_EQ(texture.status, wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal);

    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, SurfaceGetCurrentTexture(apiSurface, _))
        .WillOnce(WithArgs<1>([&](WGPUSurfaceTexture* out) {
            out->texture = apiTexture;
            out->status = WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal;
        }));

    // Check that methods on the returned texture are forwarded to the server correctly.
    texture.texture.Destroy();
    EXPECT_CALL(api, TextureDestroy(apiTexture));

    FlushClient();
}

// Test a valid call to GetCurrentTexture that ends up being an error on the server side.
TEST_F(WireInjectSurfaceTests, GetCurrentTextureErrorServerSide) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    surface.Configure(&mConfiguration);
    EXPECT_CALL(api, SurfaceConfigure(apiSurface, _));

    wgpu::SurfaceTexture texture;
    surface.GetCurrentTexture(&texture);
    EXPECT_EQ(texture.status, wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal);

    // Oh no, the GetCurrentTexture server-side fails.
    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, SurfaceGetCurrentTexture(apiSurface, _))
        .WillOnce(WithArgs<1>([&](WGPUSurfaceTexture* out) {
            out->texture = nullptr;
            out->status = WGPUSurfaceGetCurrentTextureStatus_Error;

            EXPECT_CALL(api, DeviceCreateErrorTexture(apiDevice, _)).WillOnce(Return(apiTexture));
        }));

    // Check that methods on the returned texture are forwarded to the server correctly.
    texture.texture.Destroy();
    EXPECT_CALL(api, TextureDestroy(apiTexture));

    FlushClient();
}

// Test calling GetCurrentTexture while unconfigured.
TEST_F(WireInjectSurfaceTests, GetCurrentTextureUnconfigured) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    // Test while never configured.
    wgpu::SurfaceTexture texture;
    surface.GetCurrentTexture(&texture);
    EXPECT_EQ(texture.status, wgpu::SurfaceGetCurrentTextureStatus::Error);
    EXPECT_EQ(texture.texture, nullptr);

    // Test after configure/unconfigure.
    surface.Configure(&mConfiguration);
    EXPECT_CALL(api, SurfaceConfigure(apiSurface, _));

    surface.Unconfigure();
    EXPECT_CALL(api, SurfaceUnconfigure(apiSurface));

    surface.GetCurrentTexture(&texture);
    EXPECT_EQ(texture.status, wgpu::SurfaceGetCurrentTextureStatus::Error);
    EXPECT_EQ(texture.texture, nullptr);

    // The server sees no calls to GetCurrentTexture.
    FlushClient();
}

// Test calling GetCurrentTexture after the device is destroyed.
// Note that this tests the same code path as device loss.
TEST_F(WireInjectSurfaceTests, GetCurrentTextureDeviceDestroyed) {
    auto [reservation, surface] = ReserveSurface(&mCapabilities);

    WGPUSurface apiSurface = api.GetNewSurface();
    EXPECT_CALL(api, SurfaceAddRef(apiSurface));
    ASSERT_TRUE(
        GetWireServer()->InjectSurface(apiSurface, reservation.handle, reservation.instanceHandle));

    surface.Configure(&mConfiguration);
    EXPECT_CALL(api, SurfaceConfigure(apiSurface, _));

    device.Destroy();
    EXPECT_CALL(api, DeviceDestroy(apiDevice));

    wgpu::SurfaceTexture texture;
    surface.GetCurrentTexture(&texture);

    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, DeviceCreateErrorTexture(apiDevice, _)).WillOnce(Return(apiTexture));

    EXPECT_EQ(texture.status, wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal);
    EXPECT_NE(texture.texture.Get(), nullptr);

    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
