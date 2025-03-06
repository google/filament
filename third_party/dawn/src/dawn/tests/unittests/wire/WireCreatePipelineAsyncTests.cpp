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

#include <memory>
#include <utility>

#include "dawn/common/StringViewUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::EmptySizedString;
using testing::InvokeWithoutArgs;
using testing::IsNull;
using testing::NonEmptySizedString;
using testing::NotNull;
using testing::Return;
using testing::SizedString;

using WireCreateComputePipelineAsyncTestBase =
    WireFutureTest<wgpu::CreateComputePipelineAsyncCallback<void>*>;
class WireCreateComputePipelineAsyncTest : public WireCreateComputePipelineAsyncTestBase {
  protected:
    void CreateComputePipelineAsync(wgpu::ComputePipelineDescriptor const* desc) {
        this->mFutureIDs.push_back(device
                                       .CreateComputePipelineAsync(desc,
                                                                   this->GetParam().callbackMode,
                                                                   this->mMockCb.Callback())
                                       .id);
    }

    // Sets up default descriptors to use in the tests.
    void SetUp() override {
        WireCreateComputePipelineAsyncTestBase::SetUp();

        apiPipeline = api.GetNewComputePipeline();

        wgpu::ShaderModuleDescriptor shaderDesc = {};
        mShader = device.CreateShaderModule(&shaderDesc);
        mApiShader = api.GetNewShaderModule();
        EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(mApiShader));
        FlushClient();

        mDescriptor.compute.module = mShader;
    }

    void TearDown() override {
        // We must lose all references to objects before calling parent TearDown to avoid
        // referencing the proc table after it gets cleared.
        mDescriptor = {};
        mShader = nullptr;

        WireCreateComputePipelineAsyncTestBase::TearDown();
    }

    wgpu::ShaderModule mShader;
    WGPUShaderModule mApiShader;
    wgpu::ComputePipelineDescriptor mDescriptor = {};

    // A successfully created pipeline.
    WGPUComputePipeline apiPipeline;
};

using WireCreateRenderPipelineAsyncTestBase =
    WireFutureTest<wgpu::CreateRenderPipelineAsyncCallback<void>*>;
class WireCreateRenderPipelineAsyncTest : public WireCreateRenderPipelineAsyncTestBase {
  protected:
    void CreateRenderPipelineAsync(wgpu::RenderPipelineDescriptor const* desc) {
        this->mFutureIDs.push_back(device
                                       .CreateRenderPipelineAsync(desc,
                                                                  this->GetParam().callbackMode,
                                                                  this->mMockCb.Callback())
                                       .id);
    }

    // Sets up default descriptors to use in the tests.
    void SetUp() override {
        WireCreateRenderPipelineAsyncTestBase::SetUp();

        apiPipeline = api.GetNewRenderPipeline();

        wgpu::ShaderModuleDescriptor shaderDesc = {};
        mShader = device.CreateShaderModule(&shaderDesc);
        mApiShader = api.GetNewShaderModule();
        EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(mApiShader));
        FlushClient();

        mDescriptor.vertex.module = mShader;
        mFragment.module = mShader;
        mDescriptor.fragment = &mFragment;
    }

    void TearDown() override {
        // We must lose all references to objects before calling parent TearDown to avoid
        // referencing the proc table after it gets cleared.
        mDescriptor = {};
        mFragment = {};
        mShader = nullptr;

        WireCreateRenderPipelineAsyncTestBase::TearDown();
    }

    wgpu::ShaderModule mShader;
    WGPUShaderModule mApiShader;
    wgpu::FragmentState mFragment = {};
    wgpu::RenderPipelineDescriptor mDescriptor = {};

    // A successfully created pipeline.
    WGPURenderPipeline apiPipeline;
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireCreateComputePipelineAsyncTest);
DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireCreateRenderPipelineAsyncTest);

// Test when creating a compute pipeline with CreateComputePipelineAsync() successfully.
TEST_P(WireCreateComputePipelineAsyncTest, CreateSuccess) {
    CreateComputePipelineAsync(&mDescriptor);

    EXPECT_CALL(api, OnDeviceCreateComputePipelineAsync(apiDevice, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateComputePipelineAsyncCallback(apiDevice,
                                                             WGPUCreatePipelineAsyncStatus_Success,
                                                             apiPipeline, kEmptyOutputStringView);
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb,
                    Call(wgpu::CreatePipelineAsyncStatus::Success, NotNull(), SizedString("")))
            .Times(1);

        FlushCallbacks();
    });
}

// Test when creating a render pipeline with CreateRenderPipelineAsync() successfully.
TEST_P(WireCreateRenderPipelineAsyncTest, CreateSuccess) {
    CreateRenderPipelineAsync(&mDescriptor);

    EXPECT_CALL(api, OnDeviceCreateRenderPipelineAsync(apiDevice, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateRenderPipelineAsyncCallback(apiDevice,
                                                            WGPUCreatePipelineAsyncStatus_Success,
                                                            apiPipeline, kEmptyOutputStringView);
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb,
                    Call(wgpu::CreatePipelineAsyncStatus::Success, NotNull(), EmptySizedString()))
            .Times(1);

        FlushCallbacks();
    });
}

// Test when creating a compute pipeline with CreateComputePipelineAsync() results in an error.
TEST_P(WireCreateComputePipelineAsyncTest, CreateError) {
    CreateComputePipelineAsync(&mDescriptor);

    EXPECT_CALL(api, OnDeviceCreateComputePipelineAsync(apiDevice, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateComputePipelineAsyncCallback(
                apiDevice, WGPUCreatePipelineAsyncStatus_ValidationError, nullptr,
                ToOutputStringView("Some error message"));
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::ValidationError, IsNull(),
                                 SizedString("Some error message")))
            .Times(1);

        FlushCallbacks();
    });
}

// Test when creating a render pipeline with CreateRenderPipelineAsync() results in an error.
TEST_P(WireCreateRenderPipelineAsyncTest, CreateError) {
    CreateRenderPipelineAsync(&mDescriptor);

    EXPECT_CALL(api, OnDeviceCreateRenderPipelineAsync(apiDevice, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateRenderPipelineAsyncCallback(
                apiDevice, WGPUCreatePipelineAsyncStatus_ValidationError, nullptr,
                ToOutputStringView("Some error message"));
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::ValidationError, IsNull(),
                                 SizedString("Some error message")))
            .Times(1);

        FlushCallbacks();
    });
}

// Test that registering a callback then wire disconnect calls the callback with
// InstanceDropped.
TEST_P(WireCreateComputePipelineAsyncTest, CreateThenDisconnect) {
    CreateComputePipelineAsync(&mDescriptor);

    EXPECT_CALL(api, OnDeviceCreateComputePipelineAsync(apiDevice, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateComputePipelineAsyncCallback(apiDevice,
                                                             WGPUCreatePipelineAsyncStatus_Success,
                                                             apiPipeline, kEmptyOutputStringView);
        }));

    FlushClient();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

// Test that registering a callback then wire disconnect calls the callback with
// InstanceDropped.
TEST_P(WireCreateRenderPipelineAsyncTest, CreateThenDisconnect) {
    CreateRenderPipelineAsync(&mDescriptor);

    EXPECT_CALL(api, OnDeviceCreateRenderPipelineAsync(apiDevice, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateRenderPipelineAsyncCallback(apiDevice,
                                                            WGPUCreatePipelineAsyncStatus_Success,
                                                            apiPipeline, kEmptyOutputStringView);
        }));

    FlushClient();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

// Test that registering a callback after wire disconnect calls the callback with
// InstanceDropped.
TEST_P(WireCreateComputePipelineAsyncTest, CreateAfterDisconnect) {
    GetWireClient()->Disconnect();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        CreateComputePipelineAsync(&mDescriptor);
    });
}

// Test that registering a callback after wire disconnect calls the callback with
// InstanceDropped.
TEST_P(WireCreateRenderPipelineAsyncTest, CreateAfterDisconnect) {
    GetWireClient()->Disconnect();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        CreateRenderPipelineAsync(&mDescriptor);
    });
}

TEST_P(WireCreateComputePipelineAsyncTest, CreateAndDropInstance) {
    // For spontaneous, dropping the instance does not immediately call the callback because it is
    // allowed to resolve later.
    DAWN_SKIP_TEST_IF(IsSpontaneous());

    CreateComputePipelineAsync(&mDescriptor);

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        instance = nullptr;
    });
}

TEST_P(WireCreateRenderPipelineAsyncTest, CreateAndDropInstance) {
    // For spontaneous, dropping the instance does not immediately call the callback because it is
    // allowed to resolve later.
    DAWN_SKIP_TEST_IF(IsSpontaneous());

    CreateRenderPipelineAsync(&mDescriptor);

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        instance = nullptr;
    });
}

TEST_P(WireCreateComputePipelineAsyncTest, CreateAfterDroppingInstance) {
    // For spontaneous, dropping the instance does not immediately call the callback because it is
    // allowed to resolve later.
    DAWN_SKIP_TEST_IF(IsSpontaneous());
    instance = nullptr;

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        CreateComputePipelineAsync(&mDescriptor);
    });
}

TEST_P(WireCreateRenderPipelineAsyncTest, CreateAfterDroppingInstance) {
    // For spontaneous, dropping the instance does not immediately call the callback because it is
    // allowed to resolve later.
    DAWN_SKIP_TEST_IF(IsSpontaneous());
    instance = nullptr;

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::CreatePipelineAsyncStatus::InstanceDropped, IsNull(),
                                 NonEmptySizedString()))
            .Times(1);

        CreateRenderPipelineAsync(&mDescriptor);
    });
}

// Test that if the server is deleted before the callback, it forces the
// callback to complete.
TEST(WireCreatePipelineAsyncTestNullBackend, ServerDeletedBeforeCallback) {
    // This test sets up its own wire facilities, because unlike the other
    // tests which use mocks, this test needs the null backend and the
    // threadpool which automatically pushes async pipeline compilation
    // to completion. With mocks, we need to explicitly trigger callbacks,
    // but this test depends on triggering the async compilation from
    // *within* the wire server destructor.
    auto c2sBuf = std::make_unique<dawn::utils::TerribleCommandBuffer>();
    auto s2cBuf = std::make_unique<dawn::utils::TerribleCommandBuffer>();

    dawn::wire::WireServerDescriptor serverDesc = {};
    serverDesc.procs = &dawn::native::GetProcs();
    serverDesc.serializer = s2cBuf.get();

    auto wireServer = std::make_unique<dawn::wire::WireServer>(serverDesc);
    c2sBuf->SetHandler(wireServer.get());

    dawn::wire::WireClientDescriptor clientDesc = {};
    clientDesc.serializer = c2sBuf.get();

    auto wireClient = std::make_unique<dawn::wire::WireClient>(clientDesc);
    s2cBuf->SetHandler(wireClient.get());

    dawnProcSetProcs(&dawn::wire::client::GetProcs());

    auto reserved = wireClient->ReserveInstance();
    wgpu::Instance instance = wgpu::Instance::Acquire(reserved.instance);
    wireServer->InjectInstance(dawn::native::GetProcs().createInstance(nullptr), reserved.handle);

    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.backendType = wgpu::BackendType::Null;

    wgpu::Adapter adapter;
    instance.RequestAdapter(&adapterOptions, wgpu::CallbackMode::AllowSpontaneous,
                            [&adapter](wgpu::RequestAdapterStatus, wgpu::Adapter result,
                                       wgpu::StringView) { adapter = std::move(result); });
    ASSERT_TRUE(c2sBuf->Flush());
    ASSERT_TRUE(s2cBuf->Flush());

    wgpu::DeviceDescriptor deviceDesc = {};
    wgpu::Device device;
    adapter.RequestDevice(&deviceDesc, wgpu::CallbackMode::AllowSpontaneous,
                          [&device](wgpu::RequestDeviceStatus, wgpu::Device result,
                                    wgpu::StringView) { device = std::move(result); });
    ASSERT_TRUE(c2sBuf->Flush());
    ASSERT_TRUE(s2cBuf->Flush());

    wgpu::ShaderSourceWGSL wgslDesc = {};
    wgslDesc.code.data = "@compute @workgroup_size(64) fn main() {}";

    wgpu::ShaderModuleDescriptor smDesc = {};
    smDesc.nextInChain = &wgslDesc;

    wgpu::ShaderModule sm = device.CreateShaderModule(&smDesc);

    wgpu::ComputePipelineDescriptor computeDesc = {};
    computeDesc.compute.module = sm;

    wgpu::ComputePipeline pipeline;
    device.CreateComputePipelineAsync(
        &computeDesc, wgpu::CallbackMode::AllowSpontaneous,
        [&pipeline](wgpu::CreatePipelineAsyncStatus, wgpu::ComputePipeline result,
                    wgpu::StringView) { pipeline = std::move(result); });
    ASSERT_TRUE(c2sBuf->Flush());

    // Delete the server. It should force async work to complete.
    c2sBuf->SetHandler(nullptr);
    wireServer.reset();

    ASSERT_TRUE(s2cBuf->Flush());
    ASSERT_NE(pipeline, nullptr);

    pipeline = nullptr;
    sm = nullptr;
    device = nullptr;
    adapter = nullptr;
    instance = nullptr;

    s2cBuf->SetHandler(nullptr);
}

}  // anonymous namespace
}  // namespace dawn::wire
