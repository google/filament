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

#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::Mock;
using testing::Return;

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
    wgpu::CompilationMessage mMessage = {nullptr,
                                         ToOutputStringView("Test Message"),
                                         wgpu::CompilationMessageType::Info,
                                         2,
                                         4,
                                         6,
                                         8,
                                         4,
                                         6,
                                         8};
    wgpu::CompilationInfo mCompilationInfo = {nullptr, 1, &mMessage};
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireShaderModuleTests);

// Check getting CompilationInfo for a successfully created shader module
TEST_P(WireShaderModuleTests, GetCompilationInfo) {
    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallShaderModuleGetCompilationInfoCallback(
                apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
                reinterpret_cast<const WGPUCompilationInfo*>(&mCompilationInfo));
        }));
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
                                     return infoMessage->message == mMessage.message &&
                                            infoMessage->nextInChain == mMessage.nextInChain &&
                                            infoMessage->type == mMessage.type &&
                                            infoMessage->lineNum == mMessage.lineNum &&
                                            infoMessage->linePos == mMessage.linePos &&
                                            infoMessage->offset == mMessage.offset &&
                                            infoMessage->length == mMessage.length;
                                 })))
            .Times(1);

        FlushCallbacks();
    });
}

// Test that calling GetCompilationInfo then disconnecting the wire calls the callback with
// instance dropped.
TEST_P(WireShaderModuleTests, GetCompilationInfoBeforeDisconnect) {
    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallShaderModuleGetCompilationInfoCallback(
                apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
                reinterpret_cast<const WGPUCompilationInfo*>(&mCompilationInfo));
        }));
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::InstanceDropped, nullptr))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

// Test that calling GetCompilationInfo after disconnecting the wire calls the callback with
// instance dropped.
TEST_P(WireShaderModuleTests, GetCompilationInfoAfterDisconnect) {
    GetWireClient()->Disconnect();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::InstanceDropped, nullptr))
            .Times(1);

        GetCompilationInfo();
    });
}

// Test that requests inside user callbacks before disconnect are called.
TEST_P(WireShaderModuleTests, GetCompilationInfoInsideCallbackBeforeDisconnect) {
    static constexpr size_t kNumRequests = 10;

    GetCompilationInfo();

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallShaderModuleGetCompilationInfoCallback(
                apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
                reinterpret_cast<const WGPUCompilationInfo*>(&mCompilationInfo));
        }));
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CompilationInfoRequestStatus::InstanceDropped, nullptr))
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

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallShaderModuleGetCompilationInfoCallback(
                apiShaderModule, WGPUCompilationInfoRequestStatus_Success,
                reinterpret_cast<const WGPUCompilationInfo*>(&mCompilationInfo));
        }));
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

}  // anonymous namespace
}  // namespace dawn::wire
