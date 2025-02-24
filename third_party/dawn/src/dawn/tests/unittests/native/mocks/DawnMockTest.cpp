// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/native/mocks/DawnMockTest.h"

#include <utility>

#include "dawn/dawn_proc.h"
#include "dawn/native/ChainUtils.h"

using testing::_;
using testing::AtMost;

namespace dawn::native {

DawnMockTest::DawnMockTest() : mDeviceToggles(ToggleStage::Device) {}

void DawnMockTest::SetUp() {
    dawnProcSetProcs(&dawn::native::GetProcs());
    Ref<InstanceBase> instance = APICreateInstance(nullptr);
    const auto& adapters = instance->EnumerateAdapters();
    DAWN_ASSERT(!adapters.empty());

    wgpu::DeviceDescriptor desc = {};
    desc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
                               mDeviceLostCallback.Callback());
    desc.SetUncapturedErrorCallback(mDeviceErrorCallback.TemplatedCallback(),
                                    mDeviceErrorCallback.TemplatedCallbackUserdata());
    DeviceDescriptor* nativeDesc = reinterpret_cast<DeviceDescriptor*>(&desc);

    auto result = ValidateAndUnpack(nativeDesc);
    DAWN_ASSERT(result.IsSuccess());
    UnpackedPtr<DeviceDescriptor> unpackedDesc = result.AcquireSuccess();

    Ref<DeviceBase::DeviceLostEvent> lostEvent = DeviceBase::DeviceLostEvent::Create(nativeDesc);

    auto deviceMock = AcquireRef(new ::testing::NiceMock<DeviceMock>(
        adapters[0].Get(), unpackedDesc, mDeviceToggles, std::move(lostEvent)));
    mDeviceMock = deviceMock.Get();
    device = wgpu::Device::Acquire(ToAPI(ReturnToAPI<DeviceBase>(std::move(deviceMock))));
}

void DawnMockTest::DropDevice() {
    if (device == nullptr) {
        return;
    }

    EXPECT_CALL(mDeviceLostCallback,
                Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Destroyed, _))
        .Times(AtMost(1));

    // Since the device owns the instance in these tests, we need to explicitly verify that the
    // instance has completed all work. To do this, we take an additional ref to the instance here
    // and use it to process events until completion after dropping the device.
    Ref<InstanceBase> instance = mDeviceMock->GetInstance();

    mDeviceMock = nullptr;
    device = nullptr;

    do {
    } while (instance->ProcessEvents());
    instance = nullptr;
}

DawnMockTest::~DawnMockTest() {
    DropDevice();
    dawnProcSetProcs(nullptr);
}

void DawnMockTest::ProcessEvents() {
    mDeviceMock->GetInstance()->ProcessEvents();
}

}  // namespace dawn::native
