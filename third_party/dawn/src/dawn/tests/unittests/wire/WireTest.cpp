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

#include "dawn/tests/unittests/wire/WireTest.h"

#include "dawn/common/StringViewUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

using testing::_;
using testing::AnyNumber;
using testing::AtLeast;
using testing::AtMost;
using testing::Exactly;
using testing::Mock;
using testing::MockCallback;
using testing::NotNull;
using testing::Return;
using testing::SaveArg;
using testing::StrEq;
using testing::WithArg;

namespace dawn {

namespace {
// WireTest sets the wire proc table as the global proc table.
// Tests that use multiple wires may inherit WireTest multiple times (see
// WireConfusionDeathTest). Refcount how many WireTest instances are running
// to make sure we don't unset the proc table until the test is done.
uint32_t sWireProcTableRefCount = 0;
}  // namespace

WireTest::WireTest() {
    // Set up default expectation for Device.Destroy to ensure we can track that every device on the
    // server has Destroy called.
    ON_CALL(api, DeviceDestroy).WillByDefault([this](WGPUDevice device) {
        mDeviceDestroyed[device] = true;
    });
}

WireTest::~WireTest() {
    // Verify that all devices had Destroy called on them.
    for (auto& [_, destroyed] : mDeviceDestroyed) {
        EXPECT_TRUE(destroyed);
    }
}

wire::client::MemoryTransferService* WireTest::GetClientMemoryTransferService() {
    return nullptr;
}

wire::server::MemoryTransferService* WireTest::GetServerMemoryTransferService() {
    return nullptr;
}

void WireTest::SetUp() {
    DawnProcTable mockProcs;
    api.GetProcTable(&mockProcs);

    mS2cBuf = std::make_unique<utils::TerribleCommandBuffer>();
    mC2sBuf = std::make_unique<utils::TerribleCommandBuffer>(mWireServer.get());

    wire::WireServerDescriptor serverDesc = {};
    serverDesc.procs = &mockProcs;
    serverDesc.serializer = mS2cBuf.get();
    serverDesc.memoryTransferService = GetServerMemoryTransferService();

    mWireServer.reset(new wire::WireServer(serverDesc));
    mC2sBuf->SetHandler(mWireServer.get());

    wire::WireClientDescriptor clientDesc = {};
    clientDesc.serializer = mC2sBuf.get();
    clientDesc.memoryTransferService = GetClientMemoryTransferService();

    mWireClient.reset(new wire::WireClient(clientDesc));
    mS2cBuf->SetHandler(mWireClient.get());

    if (sWireProcTableRefCount == 0) {
        dawnProcSetProcs(&wire::client::GetProcs());
    }
    ++sWireProcTableRefCount;

    auto reservedInstance = GetWireClient()->ReserveInstance();
    instance = wgpu::Instance::Acquire(reservedInstance.instance);
    apiInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceAddRef(apiInstance));
    EXPECT_TRUE(GetWireServer()->InjectInstance(apiInstance, reservedInstance.handle));

    // Create the adapter for testing.
    apiAdapter = api.GetNewAdapter();
    MockCallback<void (*)(wgpu::RequestAdapterStatus, wgpu::Adapter, wgpu::StringView, void*)>
        adapterCb;
    instance.RequestAdapter(nullptr, wgpu::CallbackMode::AllowSpontaneous, adapterCb.Callback(),
                            adapterCb.MakeUserdata(this));

    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, _, _)).WillOnce([&]() {
        EXPECT_CALL(api, AdapterHasFeature(apiAdapter, _)).WillRepeatedly(Return(false));

        EXPECT_CALL(api, AdapterGetInfo(apiAdapter, NotNull()))
            .WillOnce(WithArg<1>([&](WGPUAdapterInfo* info) {
                *info = {};
                info->vendor = kEmptyOutputStringView;
                info->architecture = kEmptyOutputStringView;
                info->device = kEmptyOutputStringView;
                info->description = kEmptyOutputStringView;
                return WGPUStatus_Success;
            }));

        EXPECT_CALL(api, AdapterGetLimits(apiAdapter, NotNull()))
            .WillOnce(WithArg<1>([&](WGPULimits* limits) {
                *limits = {};
                return WGPUStatus_Success;
            }));

        EXPECT_CALL(api, AdapterGetFeatures(apiAdapter, NotNull()))
            .WillOnce(WithArg<1>([&](WGPUSupportedFeatures* features) { *features = {}; }));

        api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Success,
                                               apiAdapter, kEmptyOutputStringView);
    });
    FlushClient();
    EXPECT_CALL(adapterCb, Call(wgpu::RequestAdapterStatus::Success, NotNull(), StrEq(""), this))
        .WillOnce(SaveArg<1>(&adapter));
    FlushServer();
    EXPECT_NE(adapter, nullptr);

    // Create the device for testing.
    apiDevice = GetNewDevice();
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
                                     deviceLostCallback.Callback());
    deviceDesc.SetUncapturedErrorCallback(uncapturedErrorCallback.TemplatedCallback(),
                                          uncapturedErrorCallback.TemplatedCallbackUserdata());
    EXPECT_CALL(deviceLostCallback, Call).Times(AtMost(1));

    MockCallback<void (*)(wgpu::RequestDeviceStatus, wgpu::Device, wgpu::StringView, void*)>
        deviceCb;
    adapter.RequestDevice(&deviceDesc, wgpu::CallbackMode::AllowSpontaneous, deviceCb.Callback(),
                          deviceCb.MakeUserdata(this));
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>([&](const WGPUDeviceDescriptor* desc) {
            // Set on device creation to forward callbacks to the client.
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);

            // The mock objects currently require us to manually set the callbacks because we
            // are no longer explicitly calling the setters anymore.
            ProcTableAsClass::Object* object =
                reinterpret_cast<ProcTableAsClass::Object*>(apiDevice);
            object->mDeviceLostCallback = desc->deviceLostCallbackInfo.callback;
            object->mDeviceLostUserdata1 = desc->deviceLostCallbackInfo.userdata1;
            object->mDeviceLostUserdata2 = desc->deviceLostCallbackInfo.userdata2;
            object->mUncapturedErrorCallback = desc->uncapturedErrorCallbackInfo.callback;
            object->mUncapturedErrorUserdata1 = desc->uncapturedErrorCallbackInfo.userdata1;
            object->mUncapturedErrorUserdata2 = desc->uncapturedErrorCallbackInfo.userdata2;

            EXPECT_CALL(api, DeviceGetLimits(apiDevice, NotNull()))
                .WillOnce(WithArg<1>([&](WGPULimits* limits) {
                    *limits = {};
                    return WGPUStatus_Success;
                }));

            EXPECT_CALL(api, DeviceGetFeatures(apiDevice, NotNull()))
                .WillOnce(WithArg<1>([&](WGPUSupportedFeatures* features) { *features = {}; }));

            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success,
                                                 apiDevice, kEmptyOutputStringView);
        }));
    FlushClient();
    EXPECT_CALL(deviceCb, Call(wgpu::RequestDeviceStatus::Success, NotNull(), StrEq(""), this))
        .WillOnce(SaveArg<1>(&device));
    FlushServer();
    EXPECT_NE(device, nullptr);

    // The GetQueue is done on WireClient startup so we expect it now.
    queue = device.GetQueue();
    apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, DeviceGetQueue(apiDevice)).WillOnce(Return(apiQueue));
    FlushClient();
}

void WireTest::TearDown() {
    instance = nullptr;
    adapter = nullptr;
    device = nullptr;
    queue = nullptr;

    --sWireProcTableRefCount;
    if (sWireProcTableRefCount == 0) {
        dawnProcSetProcs(nullptr);
    }

    // Derived classes should call the base TearDown() first. The client must
    // be reset before any mocks are deleted.
    DeleteClient();
    DeleteServer();
}

WGPUDevice WireTest::GetNewDevice() {
    auto device = api.GetNewDevice();
    mDeviceDestroyed[device] = false;
    return device;
}

void WireTest::FlushClient(bool success) {
    ASSERT_EQ(mC2sBuf->Flush(), success);

    Mock::VerifyAndClearExpectations(&api);
}

void WireTest::FlushServer(bool success) {
    ASSERT_EQ(mS2cBuf->Flush(), success);
}

wire::WireServer* WireTest::GetWireServer() {
    return mWireServer.get();
}

wire::WireClient* WireTest::GetWireClient() {
    return mWireClient.get();
}

wire::CommandSerializer* WireTest::GetC2SSerializer() {
    return mC2sBuf.get();
}

wire::CommandSerializer* WireTest::GetS2CSerializer() {
    return mS2cBuf.get();
}

size_t WireTest::GetC2SMaxAllocationSize() {
    return mC2sBuf->GetMaximumAllocationSize();
}

void WireTest::DeleteServer() {
    mC2sBuf->SetHandler(nullptr);
    mWireServer = nullptr;
}

void WireTest::DeleteClient() {
    mS2cBuf->SetHandler(nullptr);
    mWireClient = nullptr;
}

}  // namespace dawn
