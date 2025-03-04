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

namespace dawn::wire {
namespace {

using testing::_;
using testing::Invoke;
using testing::NotNull;
using testing::Return;
using testing::Unused;

class WireExtensionTests : public WireTest {
  public:
    WireExtensionTests() {}
    ~WireExtensionTests() override = default;
};

// Serialize/Deserializes a chained struct correctly.
TEST_F(WireExtensionTests, ChainedStruct) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    wgpu::ShaderSourceWGSL clientExt = {};
    shaderModuleDesc.nextInChain = &clientExt;
    clientExt.code = {"/* comment */", WGPU_STRLEN};

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                const auto* ext =
                    reinterpret_cast<const WGPUShaderSourceWGSL*>(serverDesc->nextInChain);
                EXPECT_EQ(ext->chain.sType, WGPUSType_ShaderSourceWGSL);
                EXPECT_NE(ext->code.length, WGPU_STRLEN) << "The wire should decay WGPU_STRLEN";
                EXPECT_EQ(0, memcmp(ext->code.data, clientExt.code.data, ext->code.length));
                EXPECT_EQ(ext->code.length, strlen(clientExt.code.data));
                EXPECT_EQ(ext->chain.next, nullptr);

                return apiShaderModule;
            }));
    FlushClient();
}

// Serialize/Deserializes multiple chained structs correctly.
TEST_F(WireExtensionTests, MultipleChainedStructs) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};

    wgpu::ShaderSourceWGSL clientExt2 = {};
    clientExt2.code = {"/* comment 2 */", WGPU_STRLEN};

    wgpu::ShaderSourceWGSL clientExt1 = {};
    clientExt1.code = {"/* comment 1 */", WGPU_STRLEN};
    clientExt1.nextInChain = &clientExt2;
    shaderModuleDesc.nextInChain = &clientExt1;

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule1 = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                const auto* ext1 =
                    reinterpret_cast<const WGPUShaderSourceWGSL*>(serverDesc->nextInChain);
                EXPECT_EQ(ext1->chain.sType, WGPUSType_ShaderSourceWGSL);
                EXPECT_NE(ext1->code.length, WGPU_STRLEN) << "The wire should decay WGPU_STRLEN";
                EXPECT_EQ(0, memcmp(ext1->code.data, clientExt1.code.data, ext1->code.length));
                EXPECT_EQ(ext1->code.length, strlen(clientExt1.code.data));

                const auto* ext2 = reinterpret_cast<const WGPUShaderSourceWGSL*>(ext1->chain.next);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_ShaderSourceWGSL);
                EXPECT_NE(ext2->code.length, WGPU_STRLEN) << "The wire should decay WGPU_STRLEN";
                EXPECT_EQ(0, memcmp(ext2->code.data, clientExt2.code.data, ext2->code.length));
                EXPECT_EQ(ext2->code.length, strlen(clientExt2.code.data));
                EXPECT_EQ(ext2->chain.next, nullptr);

                return apiShaderModule;
            }));
    FlushClient();

    // Swap the order of the chained structs.
    shaderModuleDesc.nextInChain = &clientExt2;
    clientExt2.nextInChain = &clientExt1;
    clientExt1.nextInChain = nullptr;

    wgpu::ShaderModule shaderModule2 = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                const auto* ext2 =
                    reinterpret_cast<const WGPUShaderSourceWGSL*>(serverDesc->nextInChain);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_ShaderSourceWGSL);
                EXPECT_NE(ext2->code.length, WGPU_STRLEN) << "The wire should decay WGPU_STRLEN";
                EXPECT_EQ(0, memcmp(ext2->code.data, clientExt2.code.data, ext2->code.length));
                EXPECT_EQ(ext2->code.length, strlen(clientExt2.code.data));

                const auto* ext1 = reinterpret_cast<const WGPUShaderSourceWGSL*>(ext2->chain.next);
                EXPECT_EQ(ext1->chain.sType, WGPUSType_ShaderSourceWGSL);
                EXPECT_NE(ext1->code.length, WGPU_STRLEN) << "The wire should decay WGPU_STRLEN";
                EXPECT_EQ(0, memcmp(ext1->code.data, clientExt1.code.data, ext1->code.length));
                EXPECT_EQ(ext1->code.length, strlen(clientExt1.code.data));
                EXPECT_EQ(ext1->chain.next, nullptr);

                return apiShaderModule;
            }));
    FlushClient();
}

// Test that a chained struct with Invalid sType passes through as Invalid.
TEST_F(WireExtensionTests, InvalidSType) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    wgpu::ShaderSourceWGSL clientExt = {};
    shaderModuleDesc.nextInChain = &clientExt;
    clientExt.sType = wgpu::SType(0);

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                EXPECT_EQ(serverDesc->nextInChain->sType, WGPUSType(0));
                EXPECT_EQ(serverDesc->nextInChain->next, nullptr);

                return apiShaderModule;
            }));
    FlushClient();
}

// Test that a chained struct with unknown sType passes through as Invalid.
TEST_F(WireExtensionTests, UnknownSType) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    wgpu::ShaderSourceWGSL clientExt = {};
    shaderModuleDesc.nextInChain = &clientExt;
    clientExt.sType = static_cast<wgpu::SType>(-1);

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                EXPECT_EQ(serverDesc->nextInChain->sType, WGPUSType(0));
                EXPECT_EQ(serverDesc->nextInChain->next, nullptr);

                return apiShaderModule;
            }));
    FlushClient();
}

// Test that if both an invalid and valid stype are passed on the chain, only the invalid
// sType passes through as Invalid.
TEST_F(WireExtensionTests, ValidAndInvalidSTypeInChain) {
    WGPUShaderModuleDescriptor shaderModuleDesc = {};

    WGPUShaderSourceWGSL clientExt2 = {};
    clientExt2.chain.sType = WGPUSType(0);
    clientExt2.chain.next = nullptr;

    WGPUShaderSourceWGSL clientExt1 = {};
    clientExt1.chain.sType = WGPUSType_ShaderSourceWGSL;
    clientExt1.chain.next = &clientExt2.chain;
    clientExt1.code = {"/* comment 1 */", WGPU_STRLEN};
    shaderModuleDesc.nextInChain = &clientExt1.chain;

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule1 = wgpuDeviceCreateShaderModule(cDevice, &shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                const auto* ext =
                    reinterpret_cast<const WGPUShaderSourceWGSL*>(serverDesc->nextInChain);
                EXPECT_EQ(ext->chain.sType, clientExt1.chain.sType);
                EXPECT_NE(ext->code.length, WGPU_STRLEN) << "The wire should decay WGPU_STRLEN";
                EXPECT_EQ(0, memcmp(ext->code.data, clientExt1.code.data, ext->code.length));
                EXPECT_EQ(ext->code.length, strlen(clientExt1.code.data));

                EXPECT_EQ(ext->chain.next->sType, WGPUSType(0));
                EXPECT_EQ(ext->chain.next->next, nullptr);

                return apiShaderModule;
            }));
    FlushClient();

    // Swap the order of the chained structs.
    shaderModuleDesc.nextInChain = &clientExt2.chain;
    clientExt2.chain.next = &clientExt1.chain;
    clientExt1.chain.next = nullptr;

    wgpu::ShaderModule shaderModule2 = wgpuDeviceCreateShaderModule(cDevice, &shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                EXPECT_EQ(serverDesc->nextInChain->sType, WGPUSType(0));

                const auto* ext =
                    reinterpret_cast<const WGPUShaderSourceWGSL*>(serverDesc->nextInChain->next);
                EXPECT_EQ(ext->chain.sType, clientExt1.chain.sType);
                EXPECT_NE(ext->code.length, WGPU_STRLEN) << "The wire should decay WGPU_STRLEN";
                EXPECT_EQ(0, memcmp(ext->code.data, clientExt1.code.data, ext->code.length));
                EXPECT_EQ(ext->code.length, strlen(clientExt1.code.data));
                EXPECT_EQ(ext->chain.next, nullptr);

                return apiShaderModule;
            }));
    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
