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
#include <memory>

#include "dawn/wire/WireClient.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/tests/unittests/wire/WireFutureTest.h"
#include "src/dawn/tests/unittests/wire/WireTest.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Return;
using testing::Unused;

using WireShaderModuleCreationTests = WireTest;

using WireShaderModuleTestBase = WireFutureTest<wgpu::CompilationInfoCallback<void>*>;
class WireShaderModuleTests : public WireShaderModuleTestBase {
  protected:
    void GetCompilationInfo() {
        this->mFutureIDs.push_back(
            shaderModule.GetCompilationInfo(this->GetParam().callbackMode, this->mMockCb.Callback())
                .id);
    }

    void SetUp() override {
        WireShaderModuleTestBase::SetUp();

        wgpu::ShaderModuleDescriptor descriptor = {};
        apiShaderModule = api.GetNewShaderModule();
        shaderModule = device.CreateShaderModule(&descriptor);
        EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiShaderModule));
        FlushClient();
    }

    void TearDown() override {
        // We must lose all references to objects before calling parent TearDown to avoid
        // referencing the proc table after it gets cleared.
        shaderModule = nullptr;

        WireShaderModuleTestBase::TearDown();
    }

    wgpu::ShaderModule shaderModule;
    WGPUShaderModule apiShaderModule;

    // Default responses.
    wgpu::DawnCompilationMessageUtf16 mUtf18 = {{nullptr, 4, 6, 8}};
    wgpu::CompilationMessage mMessage = {
        &mUtf18, ToOutputStringView("Test Message"), wgpu::CompilationMessageType::Info, 2, 4, 6,
        8};
    wgpu::CompilationInfo mCompilationInfo = {nullptr, 1, &mMessage};
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireShaderModuleTests);

// Check getting CompilationInfo for a successfully created shader module
TEST_P(WireShaderModuleTests, GetCompilationInfo) {
    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _)).WillOnce([&] {
        api.CallShaderModuleGetCompilationInfoCallback(
            apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
            reinterpret_cast<const WGPUCompilationInfo*>(&mCompilationInfo));
    });
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::Success,
                                 MatchesLambda([&](const wgpu::CompilationInfo* info) -> bool {
                                     if (info->messageCount != mCompilationInfo.messageCount) {
                                         return false;
                                     }
                                     const wgpu::CompilationMessage* infoMessage =
                                         &info->messages[0];
                                     EXPECT_NE(infoMessage->message.length, WGPU_STRLEN);
                                     EXPECT_NE(infoMessage->nextInChain, nullptr);
                                     EXPECT_EQ(infoMessage->nextInChain->sType,
                                               wgpu::SType::DawnCompilationMessageUtf16);
                                     const auto* utf16 =
                                         reinterpret_cast<const wgpu::DawnCompilationMessageUtf16*>(
                                             infoMessage->nextInChain);

                                     // The client should always be copying the data returned from
                                     // the server so the memory addresses should never be equal.
                                     EXPECT_NE(info, &mCompilationInfo);
                                     EXPECT_NE(infoMessage, &mMessage);
                                     EXPECT_NE(utf16, &mUtf18);

                                     return infoMessage->message == mMessage.message &&
                                            infoMessage->type == mMessage.type &&
                                            infoMessage->lineNum == mMessage.lineNum &&
                                            infoMessage->linePos == mMessage.linePos &&
                                            infoMessage->offset == mMessage.offset &&
                                            infoMessage->length == mMessage.length &&
                                            utf16->linePos == mUtf18.linePos &&
                                            utf16->offset == mUtf18.offset &&
                                            utf16->length == mUtf18.length;
                                 })))
            .Times(1);

        FlushCallbacks();
    });
}

TEST_P(WireShaderModuleTests, GetCompilationInfoMixedUseOfDawnCompilationMessages) {
    // Verify bookkeeping for the DawnCompilationMessageUtf16 instances.
    // An earlier version of the feature code was incorrectly indexing the
    // utf16 vector. It assumed the utf16 chained message for the i'th message
    // would appear in the i'th slot on the utf16 vector. Construct a case where
    // that is not true.
    wgpu::DawnCompilationMessageUtf16 utf16 = {{nullptr, 30, 32, 34}};
    wgpu::CompilationMessage message0 = {
        .nextInChain = nullptr, .lineNum = 0, .linePos = 0, .offset = 0, .length = 0};
    wgpu::CompilationMessage message1 = {
        .nextInChain = &utf16, .lineNum = 0, .linePos = 0, .offset = 0, .length = 0};
    std::array<wgpu::CompilationMessage, 2> messages = {message0, message1};
    wgpu::CompilationInfo compilationInfo = {nullptr, 2, messages.data()};

    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _)).WillOnce([&] {
        api.CallShaderModuleGetCompilationInfoCallback(
            apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
            reinterpret_cast<const WGPUCompilationInfo*>(&compilationInfo));
    });
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(
            mockCb,
            Call(wgpu::CompilationInfoRequestStatus::Success,
                 MatchesLambda([&](const wgpu::CompilationInfo* info) -> bool {
                     if (info->messageCount != compilationInfo.messageCount) {
                         return false;
                     }
                     const wgpu::CompilationMessage* msg0 = &info->messages[0];
                     EXPECT_EQ(msg0->nextInChain, nullptr);

                     // SAFETY: index into std::array 'messages' with 2 elements.
                     const wgpu::CompilationMessage* msg1 = DAWN_UNSAFE_BUFFERS(&info->messages[1]);
                     EXPECT_NE(msg1->nextInChain, nullptr);
                     EXPECT_EQ(msg1->nextInChain->sType, wgpu::SType::DawnCompilationMessageUtf16);
                     const auto* utf16_1 =
                         reinterpret_cast<const wgpu::DawnCompilationMessageUtf16*>(
                             msg1->nextInChain);

                     // The client should always be copying the data returned from
                     // the server so the memory addresses should never be equal.
                     EXPECT_NE(utf16_1, &utf16);

                     return utf16_1->linePos == utf16.linePos && utf16_1->offset == utf16.offset &&
                            utf16_1->length == utf16.length;
                 })))
            .Times(1);

        FlushCallbacks();
    });
}

TEST_P(WireShaderModuleTests, GetCompilationInfoDuplicateDawnMessageStructIsDropped) {
    // Setup a message that has two DawnCompilationMessageUtf16 structs chained to it
    // This is invalid. The implementation drops the second one.
    // crbug.com/523731236
    wgpu::DawnCompilationMessageUtf16 secondUtf16 = {{nullptr, 30, 32, 34}};
    wgpu::DawnCompilationMessageUtf16 firstUtf16 = {{&secondUtf16, 20, 22, 24}};
    wgpu::CompilationMessage message = {
        .nextInChain = &firstUtf16, .lineNum = 0, .linePos = 0, .offset = 0, .length = 0};
    wgpu::CompilationInfo compilationInfo = {nullptr, 1, &message};

    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _)).WillOnce([&] {
        api.CallShaderModuleGetCompilationInfoCallback(
            apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
            reinterpret_cast<const WGPUCompilationInfo*>(&compilationInfo));
    });
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::Success,
                                 MatchesLambda([&](const wgpu::CompilationInfo* info) -> bool {
                                     if (info->messageCount != compilationInfo.messageCount) {
                                         return false;
                                     }
                                     const wgpu::CompilationMessage* infoMessage =
                                         &info->messages[0];
                                     EXPECT_NE(infoMessage->message.length, WGPU_STRLEN);
                                     EXPECT_NE(infoMessage->nextInChain, nullptr);
                                     EXPECT_EQ(infoMessage->nextInChain->sType,
                                               wgpu::SType::DawnCompilationMessageUtf16);
                                     const auto* utf16 =
                                         reinterpret_cast<const wgpu::DawnCompilationMessageUtf16*>(
                                             infoMessage->nextInChain);

                                     // The client should always be copying the data returned from
                                     // the server so the memory addresses should never be equal.
                                     EXPECT_NE(utf16, &firstUtf16);
                                     EXPECT_NE(utf16, &secondUtf16);
                                     // The chain ends after the first struct.
                                     EXPECT_EQ(utf16->nextInChain, nullptr)
                                         << " " << &firstUtf16 << " " << &secondUtf16;

                                     return utf16->linePos == firstUtf16.linePos &&
                                            utf16->offset == firstUtf16.offset &&
                                            utf16->length == firstUtf16.length;
                                 })))
            .Times(1);

        FlushCallbacks();
    });
}

// Test that calling GetCompilationInfo then disconnecting the wire calls the callback with
// instance dropped.
TEST_P(WireShaderModuleTests, GetCompilationInfoBeforeDisconnect) {
    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _)).WillOnce([&] {
        api.CallShaderModuleGetCompilationInfoCallback(
            apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
            reinterpret_cast<const WGPUCompilationInfo*>(&mCompilationInfo));
    });
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::CallbackCancelled, nullptr))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

// Test that calling GetCompilationInfo after disconnecting the wire calls the callback with
// instance dropped.
TEST_P(WireShaderModuleTests, GetCompilationInfoAfterDisconnect) {
    GetWireClient()->Disconnect();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::CallbackCancelled, nullptr))
            .Times(1);

        GetCompilationInfo();
    });
}

// Test that requests inside user callbacks before disconnect are called.
TEST_P(WireShaderModuleTests, GetCompilationInfoInsideCallbackBeforeDisconnect) {
    static constexpr size_t kNumRequests = 10;

    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _)).WillOnce([&] {
        api.CallShaderModuleGetCompilationInfoCallback(
            apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
            reinterpret_cast<const WGPUCompilationInfo*>(&mCompilationInfo));
    });
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::CallbackCancelled, nullptr))
            .Times(kNumRequests + 1)
            .WillOnce([&]() {
                for (size_t i = 0; i < kNumRequests; i++) {
                    GetCompilationInfo();
                }
            })
            .WillRepeatedly(Return());

        GetWireClient()->Disconnect();
    });
}

// Test that requests inside user callbacks before object destruction are called
TEST_P(WireShaderModuleTests, GetCompilationInfoInsideCallbackBeforeDestruction) {
    static constexpr size_t kNumRequests = 10;

    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _)).WillOnce([&] {
        api.CallShaderModuleGetCompilationInfoCallback(
            apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
            reinterpret_cast<const WGPUCompilationInfo*>(&mCompilationInfo));
    });
    FlushClient();
    FlushFutures();

    if (IsSpontaneous()) {
        // In spontaneous mode, the callbacks can be fired immediately so they all happen when we
        // flush the first callback.
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::Success, _))
                .Times(kNumRequests + 1)
                .WillOnce([&]() {
                    for (size_t i = 0; i < kNumRequests; i++) {
                        GetCompilationInfo();
                    }
                    shaderModule = nullptr;
                })
                .WillRepeatedly(Return());

            FlushCallbacks();
        });
    } else {
        // In non-spontaneous mode, we need to flush the client and callbacks again before the
        // second round of callbacks are fired.
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::Success, _))
                .WillOnce([&]() {
                    for (size_t i = 0; i < kNumRequests; i++) {
                        GetCompilationInfo();
                    }
                    shaderModule = nullptr;
                });

            FlushCallbacks();
        });

        FlushClient();
        FlushFutures();
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::Success, _))
                .Times(kNumRequests)
                .WillRepeatedly(Return());

            FlushCallbacks();
        });
    }
}

// Test that a chained struct with Invalid sType causes CreateErrorShaderModule to be called.
TEST_F(WireShaderModuleCreationTests, InvalidSType) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};

    wgpu::DawnWireWGSLControl clientExt = {};
    shaderModuleDesc.nextInChain = &clientExt;

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateErrorShaderModule(apiDevice, _, _))
        .WillOnce([&](Unused, const WGPUShaderModuleDescriptor* serverDesc,
                      WGPUStringView errorMessage) -> WGPUShaderModule {
            EXPECT_EQ(serverDesc->nextInChain, nullptr);
            EXPECT_THAT(ToString(errorMessage),
                        testing::HasSubstr("Unsupported or invalid chained struct with SType"));

            return apiShaderModule;
        });
    FlushClient();
}

// Test that a chained struct with unknown sType causes CreateErrorShaderModule to be called.
TEST_F(WireShaderModuleCreationTests, UnknownSType) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    wgpu::ChainedStruct clientExt = {};
    shaderModuleDesc.nextInChain = &clientExt;

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateErrorShaderModule(apiDevice, _, _))
        .WillOnce([&](Unused, const WGPUShaderModuleDescriptor* serverDesc,
                      WGPUStringView errorMessage) -> WGPUShaderModule {
            EXPECT_EQ(serverDesc->nextInChain, nullptr);
            EXPECT_THAT(ToString(errorMessage),
                        testing::HasSubstr("Unsupported or invalid chained struct with SType (0)"));

            return apiShaderModule;
        });
    FlushClient();
}

// Test that if both an invalid and valid stype are passed on the chain, CreateErrorShaderModule is
// called.
TEST_F(WireShaderModuleCreationTests, ValidAndInvalidSTypeInChain) {
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};

    wgpu::DawnWireWGSLControl clientExt2 = {};
    wgpu::ShaderSourceWGSL clientExt1 = {};
    clientExt1.code = {"/* comment 1 */", WGPU_STRLEN};
    clientExt1.nextInChain = &clientExt2;
    shaderModuleDesc.nextInChain = &clientExt1;

    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    wgpu::ShaderModule shaderModule1 = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateErrorShaderModule(apiDevice, _, _))
        .WillOnce([&](Unused, const WGPUShaderModuleDescriptor* serverDesc,
                      WGPUStringView errorMessage) -> WGPUShaderModule {
            EXPECT_EQ(serverDesc->nextInChain, nullptr);
            EXPECT_THAT(ToString(errorMessage),
                        testing::HasSubstr("Unsupported or invalid chained struct with SType"));

            return apiShaderModule;
        });
    FlushClient();

    // Swap the order of the chained structs.
    shaderModuleDesc.nextInChain = &clientExt2;
    clientExt2.nextInChain = &clientExt1;
    clientExt1.nextInChain = nullptr;

    wgpu::ShaderModule shaderModule2 = device.CreateShaderModule(&shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateErrorShaderModule(apiDevice, _, _))
        .WillOnce([&](Unused, const WGPUShaderModuleDescriptor* serverDesc,
                      WGPUStringView errorMessage) -> WGPUShaderModule {
            EXPECT_EQ(serverDesc->nextInChain, nullptr);
            EXPECT_THAT(ToString(errorMessage),
                        testing::HasSubstr("Unsupported or invalid chained struct with SType"));

            return apiShaderModule;
        });
    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
