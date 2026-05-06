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

#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/ChunkedCommandSerializer.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireServer.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::Return;

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
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 4, 4, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));

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

}  // anonymous namespace
}  // namespace dawn::wire
