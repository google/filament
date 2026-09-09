// Copyright 2026 The Dawn & Tint Authors
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

#include <span>

#include "dawn/wire/Wire.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireServer.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/tests/MockCallback.h"
#include "src/dawn/tests/StringViewMatchers.h"
#include "src/dawn/tests/unittests/wire/WireTest.h"
#include "src/dawn/utils/TerribleCommandBuffer.h"
#include "src/dawn/wire/BufferConsumer.h"
#include "src/dawn/wire/ChunkedCommandSerializer.h"
#include "src/dawn/wire/client/Client.h"
#include "src/dawn/wire/server/Server.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::EmptySizedString;
using testing::IsNull;
using testing::MockCppCallback;
using testing::NonEmptySizedString;
using testing::NotNull;
using testing::Return;
using testing::WithArg;

// Fixture that helps execute specific commands through the wire that may not be possible to trigger
// through usage of the dawn::wire::client. It is even more change detecting than regular dawn::wire
// tests so we should use it only when there are no alternatives.
class WireSpecificCommandTests : public WireTest {
  protected:
    template <typename Cmd>
    void AddSpecificServerCmd(const Cmd& cmd) {
        CommandSerializer* c2s = GetC2SSerializer();
        ChunkedCommandSerializer serializer(c2s);

        serializer.SerializeCommand(cmd, *GetWireClient()->GetImplForTesting());
    }

    // Intercept a command that will be sent from the client to the server. This involves first
    // capturing the command, deserializing it as if we were the server, and then re-injecting it
    // back into the client command buffer. Note that this does not currently handle object
    // serialization correctly, so when modifying commands with descriptors that include other
    // WebGPU objects, users may need to manually update those members. This is currently a
    // limitation that is hard to address because of the way the commands are [de]serialized in the
    // wire.
    template <typename Cmd, typename ToIntercept, typename Modifier>
    void InterceptServerCmd(ToIntercept toIntercept, Modifier modifier) {
        Cmd cmd;
        utils::TerribleCommandBuffer* c2sBuf = GetC2SCommandBuffer();
        size_t startOffset = c2sBuf->GetOffsetForTesting();
        toIntercept();
        size_t endOffset = c2sBuf->GetOffsetForTesting();
        auto subrange = c2sBuf->GetContentSubrange(startOffset, endOffset);
        dawn::wire::DeserializeBuffer deserializeBuffer(subrange);
        EXPECT_EQ(WireResult::Success, cmd.Deserialize(&deserializeBuffer, &mAllocator,
                                                       *GetWireServer()->GetImplForTesting()));

        modifier(&cmd);
        c2sBuf->SetOffsetForTesting(startOffset);
        AddSpecificServerCmd(cmd);
    }

  private:
    WireDeserializeAllocator mAllocator;
};

// Regression test for https://issues.chromium.org/492139412 where a server receiving
// Device::Destroy wouldn't realize that the buffers got unmapped and would try to write into them.
// While it's not exactly possible to replicate the issue with WireTests since there is no
// dawn::native backend that will unmap buffers on destroy, we can check that the ordering of
// commands in the server is such that it will check that the buffer is mapped before writing into
// it.
TEST_F(WireSpecificCommandTests, UpdateMappedDataAfterDeviceDestroy_MappedAtCreation) {
    // Create a mapped buffer.
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc;
    descriptor.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
        .WillOnce(Return(apiBuffer))
        .RetiresOnSaturation();
    FlushClient();

    // Force a device destroy without giving the wire::client a chance to unmap client-side buffers.
    DeviceDestroyCmd cmd;
    cmd.self = device.Get();
    AddSpecificServerCmd(cmd);

    EXPECT_CALL(api, DeviceDestroy(apiDevice)).Times(1);
    FlushClient();

    // A call to unmap will get a nullptr mapped range and should not write to it! (if it were, we'd
    // see a crash here since it would write to nullptr).
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(nullptr));
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    buffer.Unmap();
    FlushClient();
}

// The same test at an offset, to check that it doesn't allow bypassing the null check. It was a
// bug found during review of the fix.
TEST_F(WireSpecificCommandTests, UpdateMappedDataAfterDeviceDestroy_MapWriteOffsetNonZero) {
    // Create a mapped buffer.
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = 8;
    descriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
        .WillOnce(Return(apiBuffer))
        .RetiresOnSaturation();
    FlushClient();

    // Map the buffer
    buffer.MapAsync(wgpu::MapMode::Write, 4, 4, wgpu::CallbackMode::AllowProcessEvents,
                    [](wgpu::MapAsyncStatus status, wgpu::StringView) {});
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 4, 4, _)).WillOnce([&] {
        api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                       kEmptyOutputStringView);
    });

    FlushClient();
    FlushServer();
    instance.ProcessEvents();

    // Force a device destroy without giving the wire::client a chance to unmap client-side buffers.
    DeviceDestroyCmd cmd;
    cmd.self = device.Get();
    AddSpecificServerCmd(cmd);

    EXPECT_CALL(api, DeviceDestroy(apiDevice)).Times(1);
    FlushClient();

    // A call to unmap will get a nullptr mapped range and should not write to it! (if it were, we'd
    // see a crash here since it would write to nullptr).
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 4, 4)).WillOnce(Return(nullptr));
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    buffer.Unmap();
    FlushClient();
}

// This is a regression test for https://issues.chromium.org/508092644 where a compromised client
// could potentially ask the server to Unregister a reserved, but not backed Device.
TEST_F(WireSpecificCommandTests, RequestDeviceIdReuseAfterInjectedUnregister) {
    // Set up all the mock callbacks and objects.
    MockCppCallback<wgpu::DeviceLostCallback<void>*> deviceLostCb;
    MockCppCallback<wgpu::RequestDeviceCallback<void>*> requestDeviceCb;
    WGPUDevice apiDeviceA = api.GetNewDevice();
    WGPUFuture futureA;
    WGPUDevice apiDeviceB = api.GetNewDevice();
    WGPUFuture futureB;
    auto SetDeviceCallbacks = [](WGPUDevice apiDevice, const WGPUDeviceDescriptor* desc) {
        ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>(apiDevice);
        object->mDeviceLostCallback = desc->deviceLostCallbackInfo.callback;
        object->mDeviceLostUserdata1 = desc->deviceLostCallbackInfo.userdata1;
        object->mDeviceLostUserdata2 = desc->deviceLostCallbackInfo.userdata2;
    };

    // Add the first request for a device and capture the C->S command for it.
    wgpu::DeviceDescriptor deviceDescA = {};
    deviceDescA.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
                                      deviceLostCb.Callback());
    AdapterRequestDeviceCmd requestA = {};
    InterceptServerCmd<AdapterRequestDeviceCmd>(
        [&]() {
            adapter.RequestDevice(&deviceDescA, wgpu::CallbackMode::AllowSpontaneous,
                                  requestDeviceCb.Callback());
        },
        [&](AdapterRequestDeviceCmd* cmd) { requestA = *cmd; });

    // Immediately unregister the device that was reserved for the first request. This is not
    // normally possible, so we inject the command directly.
    UnregisterObjectCmd unregisterA = {};
    unregisterA.objectType = ObjectType::Device;
    unregisterA.objectId = requestA.deviceObjectHandle.id;
    AddSpecificServerCmd(unregisterA);

    // Add a second request for a device that attempts to reuse the same id that was originally
    // reserved for the first request.
    wgpu::DeviceDescriptor deviceDescB = {};
    deviceDescB.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
                                      deviceLostCb.Callback());
    AdapterRequestDeviceCmd requestB = {};
    InterceptServerCmd<AdapterRequestDeviceCmd>(
        [&]() {
            adapter.RequestDevice(&deviceDescB, wgpu::CallbackMode::AllowSpontaneous,
                                  requestDeviceCb.Callback());
        },
        [&](AdapterRequestDeviceCmd* cmd) {
            cmd->deviceObjectHandle.id = requestA.deviceObjectHandle.id;
            cmd->deviceObjectHandle.generation = requestA.deviceObjectHandle.generation + 1;
            requestB = *cmd;
        });

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>([&](const WGPUDeviceDescriptor* desc) {
            SetDeviceCallbacks(apiDeviceA, desc);
            futureA = api.GetLastFuture();
        }))
        .WillOnce(WithArg<1>([&](const WGPUDeviceDescriptor* desc) {
            SetDeviceCallbacks(apiDeviceB, desc);
            futureB = api.GetLastFuture();
        }));
    FlushClient();

    // Emulate the backend server completing the first request for a device successfully. Even
    // though the backend successfully created the device, because the client has already asked to
    // unregister that reserved slot, the server should just immediately release the successfully
    // created device.
    EXPECT_CALL(api, DeviceRelease(apiDeviceA)).Times(1);
    api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success, apiDeviceA,
                                         kEmptyOutputStringView, futureA);
    // Additionally verify that the server does not back the device.
    EXPECT_EQ(GetWireServer()->GetDevice(requestA.deviceObjectHandle.id,
                                         requestA.deviceObjectHandle.generation),
              nullptr);

    // When we flush the server, the RequestDevice callback for the first request should be called
    // with callback cancelled, and the DeviceLost callback should also fire once for the first
    // requested device.
    EXPECT_CALL(requestDeviceCb, Call(wgpu::RequestDeviceStatus::CallbackCancelled, IsNull(),
                                      NonEmptySizedString()));
    EXPECT_CALL(deviceLostCb, Call).Times(1);
    FlushServer();

    // Emulate the backend server completing the second request for a device successfully.
    api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success, apiDeviceB,
                                         kEmptyOutputStringView, futureB);
    // Verify that the server backs the device.
    EXPECT_EQ(GetWireServer()->GetDevice(requestB.deviceObjectHandle.id,
                                         requestB.deviceObjectHandle.generation),
              apiDeviceB);

    // When the server completes the second request for a device, the RequestDevice callback for
    // the second request should be called with success, and the DeviceLost callback should also
    // fire once for the second requested device because we don't save the device from the
    // RequestDevice callback.
    EXPECT_CALL(requestDeviceCb,
                Call(wgpu::RequestDeviceStatus::Success, NotNull(), EmptySizedString()));
    EXPECT_CALL(deviceLostCb, Call).Times(1);
    FlushServer();

    // Finally, since we dropped the device on the client side, it should be Unregistered from the
    // client, and correspondingly released on the server side once the commands are flushed. Note
    // that the client flush should still fail though because we injected a different ID for the
    // device than the client frontend actually allocated for the second device. This means that the
    // wire client frontend will try to Unregister a non-existent object as a result of the modified
    // and injected second request which will cause the server to fail.
    EXPECT_CALL(api, DeviceRelease(apiDeviceB)).Times(1);
    FlushClient(false);
}

// This is a regression test for https://issues.chromium.org/508092644 where a compromised client
// could potentially ask the server to Unregister a reserved, but not backed Adapter.
TEST_F(WireSpecificCommandTests, RequestAdapterIdReuseAfterInjectedUnregister) {
    // Set up all the mock callbacks and objects.
    MockCppCallback<wgpu::RequestAdapterCallback<void>*> requestAdapterCb;
    WGPUAdapter apiAdapterA = api.GetNewAdapter();
    WGPUFuture futureA;
    WGPUAdapter apiAdapterB = api.GetNewAdapter();
    WGPUFuture futureB;

    // Add the first request for an adapter.
    InstanceRequestAdapterCmd requestA = {};
    InterceptServerCmd<InstanceRequestAdapterCmd>(
        [&]() {
            instance.RequestAdapter(nullptr, wgpu::CallbackMode::AllowSpontaneous,
                                    requestAdapterCb.Callback());
        },
        [&](InstanceRequestAdapterCmd* cmd) { requestA = *cmd; });

    // Immediately unregister the adapter that was reserved for the first request. This is not
    // normally possible, so we inject the command directly.
    UnregisterObjectCmd unregisterA = {};
    unregisterA.objectType = ObjectType::Adapter;
    unregisterA.objectId = requestA.adapterObjectHandle.id;
    AddSpecificServerCmd(unregisterA);

    // Add a second request for an adapter that attempts to reuse the same id that was originally
    // reserved for the first request.
    InstanceRequestAdapterCmd requestB = {};
    InterceptServerCmd<InstanceRequestAdapterCmd>(
        [&]() {
            instance.RequestAdapter(nullptr, wgpu::CallbackMode::AllowSpontaneous,
                                    requestAdapterCb.Callback());
        },
        [&](InstanceRequestAdapterCmd* cmd) {
            cmd->adapterObjectHandle.id = requestA.adapterObjectHandle.id;
            cmd->adapterObjectHandle.generation = requestA.adapterObjectHandle.generation + 1;
            requestB = *cmd;
        });

    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, IsNull(), _))
        .WillOnce([&]() { futureA = api.GetLastFuture(); })
        .WillOnce([&]() { futureB = api.GetLastFuture(); });
    FlushClient();

    // Emulate the backend server completing the first request for an adapter successfully. Even
    // though the backend successfully created the adapter, because the client has already asked to
    // unregister that reserved slot, the server should just immediately release the successfully
    // created adapter.
    EXPECT_CALL(api, AdapterRelease(apiAdapterA)).Times(1);
    api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Success,
                                           apiAdapterA, kEmptyOutputStringView, futureA);

    // When we flush the server, the RequestAdapter callback for the first request should be called
    // with callback cancelled.
    EXPECT_CALL(requestAdapterCb, Call(wgpu::RequestAdapterStatus::CallbackCancelled, IsNull(),
                                       NonEmptySizedString()));
    FlushServer();

    // Emulate the backend server completing the second request for an adapter successfully.
    api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Success,
                                           apiAdapterB, kEmptyOutputStringView, futureB);

    // When the server completes the second request for an adapter, the RequestAdapter callback for
    // the second request should be called with success.
    EXPECT_CALL(requestAdapterCb,
                Call(wgpu::RequestAdapterStatus::Success, NotNull(), EmptySizedString()));
    FlushServer();

    // Finally, since we dropped the adapter on the client side, it should be Unregistered from the
    // client, and correspondingly released on the server side once the commands are flushed. Note
    // that the client flush should still fail though because we injected a different ID for the
    // adapter than the client frontend actually allocated for the second adapter. This means that
    // the wire client frontend will try to Unregister a non-existent object as a result of the
    // modified and injected second request which will cause the server to fail.
    EXPECT_CALL(api, AdapterRelease(apiAdapterB)).Times(1);
    FlushClient(false);
}

// This is a regression test for https://issues.chromium.org/508092644 where a compromised client
// could potentially ask the server to Unregister a reserved, but not backed ComputePipeline. See
// the RequestAdapterIdReuseAfterInjectedUnregister test above for more equivalent descriptions for
// each step throughout the test.
TEST_F(WireSpecificCommandTests, CreateComputePipelineAsyncIdReuseAfterInjectedUnregister) {
    wgpu::ShaderModuleDescriptor shaderDesc = {};
    wgpu::ShaderModule shader = device.CreateShaderModule(&shaderDesc);
    WGPUShaderModule apiShader = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiShader));
    FlushClient();

    wgpu::ComputePipelineDescriptor descriptor = {};
    descriptor.compute.module = shader;

    MockCppCallback<wgpu::CreateComputePipelineAsyncCallback<void>*> createComputePipelineCb;
    WGPUComputePipeline apiPipelineA = api.GetNewComputePipeline();
    WGPUFuture futureA;
    WGPUComputePipeline apiPipelineB = api.GetNewComputePipeline();
    WGPUFuture futureB;

    DeviceCreateComputePipelineAsyncCmd requestA = {};
    InterceptServerCmd<DeviceCreateComputePipelineAsyncCmd>(
        [&]() {
            device.CreateComputePipelineAsync(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                                              createComputePipelineCb.Callback());
        },
        [&](DeviceCreateComputePipelineAsyncCmd* cmd) {
            requestA = *cmd;
            // Manually fix the shader module since the [de]serialization can't handle objects.
            const_cast<WGPUComputePipelineDescriptor*>(cmd->descriptor)->compute.module =
                shader.Get();
        });

    UnregisterObjectCmd unregisterA = {};
    unregisterA.objectType = ObjectType::ComputePipeline;
    unregisterA.objectId = requestA.pipelineObjectHandle.id;
    AddSpecificServerCmd(unregisterA);

    DeviceCreateComputePipelineAsyncCmd requestB = {};
    InterceptServerCmd<DeviceCreateComputePipelineAsyncCmd>(
        [&]() {
            device.CreateComputePipelineAsync(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                                              createComputePipelineCb.Callback());
        },
        [&](DeviceCreateComputePipelineAsyncCmd* cmd) {
            cmd->pipelineObjectHandle.id = requestA.pipelineObjectHandle.id;
            cmd->pipelineObjectHandle.generation = requestA.pipelineObjectHandle.generation + 1;
            requestB = *cmd;
            // Manually fix the shader module since the [de]serialization can't handle objects.
            const_cast<WGPUComputePipelineDescriptor*>(cmd->descriptor)->compute.module =
                shader.Get();
        });

    EXPECT_CALL(api, OnDeviceCreateComputePipelineAsync(apiDevice, NotNull(), _))
        .WillOnce([&]() { futureA = api.GetLastFuture(); })
        .WillOnce([&]() { futureB = api.GetLastFuture(); });
    FlushClient();

    EXPECT_CALL(api, ComputePipelineRelease(apiPipelineA)).Times(1);
    api.CallDeviceCreateComputePipelineAsyncCallback(apiDevice,
                                                     WGPUCreatePipelineAsyncStatus_Success,
                                                     apiPipelineA, kEmptyOutputStringView, futureA);

    EXPECT_CALL(createComputePipelineCb, Call(wgpu::CreatePipelineAsyncStatus::CallbackCancelled,
                                              IsNull(), NonEmptySizedString()));
    FlushServer();

    api.CallDeviceCreateComputePipelineAsyncCallback(apiDevice,
                                                     WGPUCreatePipelineAsyncStatus_Success,
                                                     apiPipelineB, kEmptyOutputStringView, futureB);

    EXPECT_CALL(createComputePipelineCb,
                Call(wgpu::CreatePipelineAsyncStatus::Success, NotNull(), EmptySizedString()));
    FlushServer();

    EXPECT_CALL(api, ComputePipelineRelease(apiPipelineB)).Times(1);
    FlushClient(false);
}

// This is a regression test for https://issues.chromium.org/508092644 where a compromised client
// could potentially ask the server to Unregister a reserved, but not backed RenderPipeline. See the
// RequestAdapterIdReuseAfterInjectedUnregister test above for more equivalent descriptions for
// each step throughout the test.
TEST_F(WireSpecificCommandTests, CreateRenderPipelineAsyncIdReuseAfterInjectedUnregister) {
    wgpu::ShaderModuleDescriptor shaderDesc = {};
    wgpu::ShaderModule shader = device.CreateShaderModule(&shaderDesc);
    WGPUShaderModule apiShader = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiShader));
    FlushClient();

    wgpu::FragmentState fragment = {};
    fragment.module = shader;
    wgpu::RenderPipelineDescriptor descriptor = {};
    descriptor.vertex.module = shader;
    descriptor.fragment = &fragment;

    MockCppCallback<wgpu::CreateRenderPipelineAsyncCallback<void>*> createRenderPipelineCb;
    WGPURenderPipeline apiPipelineA = api.GetNewRenderPipeline();
    WGPUFuture futureA;
    WGPURenderPipeline apiPipelineB = api.GetNewRenderPipeline();
    WGPUFuture futureB;

    DeviceCreateRenderPipelineAsyncCmd requestA = {};
    InterceptServerCmd<DeviceCreateRenderPipelineAsyncCmd>(
        [&]() {
            device.CreateRenderPipelineAsync(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                                             createRenderPipelineCb.Callback());
        },
        [&](DeviceCreateRenderPipelineAsyncCmd* cmd) {
            requestA = *cmd;
            // Manually fix the shader module since the [de]serialization can't handle objects.
            auto* desc = const_cast<WGPURenderPipelineDescriptor*>(cmd->descriptor);
            desc->vertex.module = shader.Get();
            const_cast<WGPUFragmentState*>(desc->fragment)->module = shader.Get();
        });

    UnregisterObjectCmd unregisterA = {};
    unregisterA.objectType = ObjectType::RenderPipeline;
    unregisterA.objectId = requestA.pipelineObjectHandle.id;
    AddSpecificServerCmd(unregisterA);

    DeviceCreateRenderPipelineAsyncCmd requestB = {};
    InterceptServerCmd<DeviceCreateRenderPipelineAsyncCmd>(
        [&]() {
            device.CreateRenderPipelineAsync(&descriptor, wgpu::CallbackMode::AllowSpontaneous,
                                             createRenderPipelineCb.Callback());
        },
        [&](DeviceCreateRenderPipelineAsyncCmd* cmd) {
            cmd->pipelineObjectHandle.id = requestA.pipelineObjectHandle.id;
            cmd->pipelineObjectHandle.generation = requestA.pipelineObjectHandle.generation + 1;
            requestB = *cmd;
            // Manually fix the shader module since the [de]serialization can't handle objects.
            auto* desc = const_cast<WGPURenderPipelineDescriptor*>(cmd->descriptor);
            desc->vertex.module = shader.Get();
            const_cast<WGPUFragmentState*>(desc->fragment)->module = shader.Get();
        });

    EXPECT_CALL(api, OnDeviceCreateRenderPipelineAsync(apiDevice, NotNull(), _))
        .WillOnce([&]() { futureA = api.GetLastFuture(); })
        .WillOnce([&]() { futureB = api.GetLastFuture(); });
    FlushClient();

    EXPECT_CALL(api, RenderPipelineRelease(apiPipelineA)).Times(1);
    api.CallDeviceCreateRenderPipelineAsyncCallback(apiDevice,
                                                    WGPUCreatePipelineAsyncStatus_Success,
                                                    apiPipelineA, kEmptyOutputStringView, futureA);

    EXPECT_CALL(createRenderPipelineCb, Call(wgpu::CreatePipelineAsyncStatus::CallbackCancelled,
                                             IsNull(), NonEmptySizedString()));
    FlushServer();

    api.CallDeviceCreateRenderPipelineAsyncCallback(apiDevice,
                                                    WGPUCreatePipelineAsyncStatus_Success,
                                                    apiPipelineB, kEmptyOutputStringView, futureB);

    EXPECT_CALL(createRenderPipelineCb,
                Call(wgpu::CreatePipelineAsyncStatus::Success, NotNull(), EmptySizedString()));
    FlushServer();

    EXPECT_CALL(api, RenderPipelineRelease(apiPipelineB)).Times(1);
    FlushClient(false);
}

}  // anonymous namespace
}  // namespace dawn::wire
