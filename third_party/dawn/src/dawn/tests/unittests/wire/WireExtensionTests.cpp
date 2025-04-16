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

    wgpu::ShaderModuleCompilationOptions clientExt2 = {};
    clientExt2.strictMath = true;

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

                const auto* ext2 =
                    reinterpret_cast<const WGPUShaderModuleCompilationOptions*>(ext1->chain.next);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_ShaderModuleCompilationOptions);
                EXPECT_NE(ext2->strictMath, 0u);
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
                const auto* ext2 = reinterpret_cast<const WGPUShaderModuleCompilationOptions*>(
                    serverDesc->nextInChain);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_ShaderModuleCompilationOptions);
                EXPECT_NE(ext2->strictMath, 0u);

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

    wgpu::DawnWireWGSLControl clientExt = {};
    shaderModuleDesc.nextInChain = &clientExt;

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                const auto* ext =
                    reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(serverDesc->nextInChain);
                EXPECT_EQ(ext->chain.sType, WGPUSType_DawnInjectedInvalidSType);
                EXPECT_EQ(ext->chain.next, nullptr);
                EXPECT_EQ(ext->invalidSType, WGPUSType_DawnWireWGSLControl);

                return apiShaderModule;
            }));
    FlushClient();
}

// Test that a chained struct with unknown sType passes through as Invalid.
TEST_F(WireExtensionTests, UnknownSType) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    wgpu::ChainedStruct clientExt = {};
    shaderModuleDesc.nextInChain = &clientExt;

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(
            Invoke([&](Unused, const WGPUShaderModuleDescriptor* serverDesc) -> WGPUShaderModule {
                const auto* ext =
                    reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(serverDesc->nextInChain);
                EXPECT_EQ(ext->chain.sType, WGPUSType_DawnInjectedInvalidSType);
                EXPECT_EQ(ext->chain.next, nullptr);
                EXPECT_EQ(ext->invalidSType, WGPUSType(0));

                return apiShaderModule;
            }));
    FlushClient();
}

// Test that if both an invalid and valid stype are passed on the chain, only the invalid
// sType passes through as Invalid.
TEST_F(WireExtensionTests, ValidAndInvalidSTypeInChain) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};

    wgpu::DawnWireWGSLControl clientExt2 = {};
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

                const auto* ext2 =
                    reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(ext1->chain.next);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_DawnInjectedInvalidSType);
                EXPECT_EQ(ext2->chain.next, nullptr);
                EXPECT_EQ(ext2->invalidSType, WGPUSType_DawnWireWGSLControl);

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
                    reinterpret_cast<const WGPUDawnInjectedInvalidSType*>(serverDesc->nextInChain);
                EXPECT_EQ(ext2->chain.sType, WGPUSType_DawnInjectedInvalidSType);
                EXPECT_EQ(ext2->invalidSType, WGPUSType_DawnWireWGSLControl);

                const auto* ext1 = reinterpret_cast<const WGPUShaderSourceWGSL*>(ext2->chain.next);
                EXPECT_EQ(ext1->chain.sType, WGPUSType_ShaderSourceWGSL);
                EXPECT_EQ(ext1->chain.next, nullptr);
                EXPECT_NE(ext1->code.length, WGPU_STRLEN) << "The wire should decay WGPU_STRLEN";
                EXPECT_EQ(0, memcmp(ext1->code.data, clientExt1.code.data, ext1->code.length));
                EXPECT_EQ(ext1->code.length, strlen(clientExt1.code.data));

                return apiShaderModule;
            }));
    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
