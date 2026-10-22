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

#include "src/dawn/tests/unittests/wire/WireTest.h"
#include "src/utils/compiler.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Unused;

class WireExtensionTests : public WireTest {
  public:
    WireExtensionTests() {}
    ~WireExtensionTests() override = default;
};

// Serialize/Deserializes a chained struct correctly.
TEST_F(WireExtensionTests, ChainedStruct) {
    wgpu::BindGroupLayoutEntry bglEntry = {};
    wgpu::ExternalTextureBindingLayout clientExt = {};
    bglEntry.nextInChain = &clientExt;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;

    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(
            [&](Unused, const WGPUBindGroupLayoutDescriptor* serverDesc) -> WGPUBindGroupLayout {
                EXPECT_EQ(serverDesc->entryCount, 1u);
                const auto* ext = reinterpret_cast<const WGPUExternalTextureBindingLayout*>(
                    serverDesc->entries[0].nextInChain);
                EXPECT_EQ(ext->chain.sType, WGPUSType_ExternalTextureBindingLayout);
                EXPECT_EQ(ext->chain.next, nullptr);

                return apiBgl;
            });
    FlushClient();
}

// Serialize/Deserializes multiple chained structs correctly.
TEST_F(WireExtensionTests, MultipleChainedStructs) {
    wgpu::BindGroupLayoutEntry bglEntry = {};

    wgpu::TexelBufferBindingLayout clientExt2 = {};
    clientExt2.access = wgpu::TexelBufferAccess::ReadOnly;
    clientExt2.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::ExternalTextureBindingLayout clientExt1 = {};
    clientExt1.nextInChain = &clientExt2;
    bglEntry.nextInChain = &clientExt1;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;

    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    wgpu::BindGroupLayout bgl1 = device.CreateBindGroupLayout(&bglDesc);
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(
            [&](Unused, const WGPUBindGroupLayoutDescriptor* serverDesc) -> WGPUBindGroupLayout {
                EXPECT_EQ(serverDesc->entryCount, 1u);
                const auto* ext1 = reinterpret_cast<const WGPUExternalTextureBindingLayout*>(
                    serverDesc->entries[0].nextInChain);
                EXPECT_EQ(ext1->chain.sType, WGPUSType_ExternalTextureBindingLayout);

                const auto* ext2 =
                    reinterpret_cast<const WGPUTexelBufferBindingLayout*>(ext1->chain.next);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_TexelBufferBindingLayout);
                EXPECT_EQ(ext2->access, WGPUTexelBufferAccess_ReadOnly);
                EXPECT_EQ(ext2->format, WGPUTextureFormat_RGBA8Unorm);
                EXPECT_EQ(ext2->chain.next, nullptr);

                return apiBgl;
            });
    FlushClient();

    // Swap the order of the chained structs.
    bglEntry.nextInChain = &clientExt2;
    clientExt2.nextInChain = &clientExt1;
    clientExt1.nextInChain = nullptr;

    wgpu::BindGroupLayout bgl2 = device.CreateBindGroupLayout(&bglDesc);
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(
            [&](Unused, const WGPUBindGroupLayoutDescriptor* serverDesc) -> WGPUBindGroupLayout {
                EXPECT_EQ(serverDesc->entryCount, 1u);
                const auto* ext2 = reinterpret_cast<const WGPUTexelBufferBindingLayout*>(
                    serverDesc->entries[0].nextInChain);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_TexelBufferBindingLayout);
                EXPECT_EQ(ext2->access, WGPUTexelBufferAccess_ReadOnly);
                EXPECT_EQ(ext2->format, WGPUTextureFormat_RGBA8Unorm);

                const auto* ext1 =
                    reinterpret_cast<const WGPUExternalTextureBindingLayout*>(ext2->chain.next);
                EXPECT_EQ(ext1->chain.sType, WGPUSType_ExternalTextureBindingLayout);
                EXPECT_EQ(ext1->chain.next, nullptr);

                return apiBgl;
            });
    FlushClient();
}

// Test that a chained struct with Invalid sType passes through as Invalid.
TEST_F(WireExtensionTests, InvalidSType) {
    wgpu::BindGroupLayoutEntry bglEntry = {};

    wgpu::DawnWireWGSLControl clientExt = {};
    bglEntry.nextInChain = &clientExt;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;

    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(
            [&](Unused, const WGPUBindGroupLayoutDescriptor* serverDesc) -> WGPUBindGroupLayout {
                EXPECT_EQ(serverDesc->entryCount, 1u);
                const auto* ext = reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(
                    serverDesc->entries[0].nextInChain);
                EXPECT_EQ(ext->chain.sType, WGPUSType_DawnInjectedInvalidSType);
                EXPECT_EQ(ext->chain.next, nullptr);
                EXPECT_EQ(ext->invalidSType, WGPUSType_DawnWireWGSLControl);

                return apiBgl;
            });
    FlushClient();
}

// Test that a chained struct with unknown sType passes through as Invalid.
TEST_F(WireExtensionTests, UnknownSType) {
    wgpu::BindGroupLayoutEntry bglEntry = {};
    wgpu::ChainedStruct clientExt = {};
    bglEntry.nextInChain = &clientExt;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;

    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(
            [&](Unused, const WGPUBindGroupLayoutDescriptor* serverDesc) -> WGPUBindGroupLayout {
                EXPECT_EQ(serverDesc->entryCount, 1u);
                const auto* ext = reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(
                    serverDesc->entries[0].nextInChain);
                EXPECT_EQ(ext->chain.sType, WGPUSType_DawnInjectedInvalidSType);
                EXPECT_EQ(ext->chain.next, nullptr);
                EXPECT_EQ(ext->invalidSType, WGPUSType(0));

                return apiBgl;
            });
    FlushClient();
}

// Test that if both an invalid and valid stype are passed on the chain, only the invalid
// sType passes through as Invalid.
TEST_F(WireExtensionTests, ValidAndInvalidSTypeInChain) {
    wgpu::BindGroupLayoutEntry bglEntry = {};

    wgpu::DawnWireWGSLControl clientExt2 = {};
    wgpu::ExternalTextureBindingLayout clientExt1 = {};
    clientExt1.nextInChain = &clientExt2;
    bglEntry.nextInChain = &clientExt1;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;

    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    wgpu::BindGroupLayout bgl1 = device.CreateBindGroupLayout(&bglDesc);
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(
            [&](Unused, const WGPUBindGroupLayoutDescriptor* serverDesc) -> WGPUBindGroupLayout {
                EXPECT_EQ(serverDesc->entryCount, 1u);
                const auto* ext1 = reinterpret_cast<const WGPUExternalTextureBindingLayout*>(
                    serverDesc->entries[0].nextInChain);
                EXPECT_EQ(ext1->chain.sType, WGPUSType_ExternalTextureBindingLayout);

                const auto* ext2 =
                    reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(ext1->chain.next);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_DawnInjectedInvalidSType);
                EXPECT_EQ(ext2->chain.next, nullptr);
                EXPECT_EQ(ext2->invalidSType, WGPUSType_DawnWireWGSLControl);

                return apiBgl;
            });
    FlushClient();

    // Swap the order of the chained structs.
    bglEntry.nextInChain = &clientExt2;
    clientExt2.nextInChain = &clientExt1;
    clientExt1.nextInChain = nullptr;

    wgpu::BindGroupLayout bgl2 = device.CreateBindGroupLayout(&bglDesc);
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(
            [&](Unused, const WGPUBindGroupLayoutDescriptor* serverDesc) -> WGPUBindGroupLayout {
                EXPECT_EQ(serverDesc->entryCount, 1u);
                const auto* ext2 = reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(
                    serverDesc->entries[0].nextInChain);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_DawnInjectedInvalidSType);
                EXPECT_EQ(ext2->invalidSType, WGPUSType_DawnWireWGSLControl);

                const auto* ext1 =
                    reinterpret_cast<const WGPUExternalTextureBindingLayout*>(ext2->chain.next);
                EXPECT_EQ(ext1->chain.sType, WGPUSType_ExternalTextureBindingLayout);
                EXPECT_EQ(ext1->chain.next, nullptr);

                return apiBgl;
            });
    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
