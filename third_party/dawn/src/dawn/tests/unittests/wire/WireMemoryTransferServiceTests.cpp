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

#include <memory>
#include <tuple>
#include <utility>

#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/ParamGenerator.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/client/ClientMemoryTransferService_mock.h"
#include "dawn/wire/server/ServerMemoryTransferService_mock.h"

namespace wgpu {
// Define a stream operator for wgpu::MapMode so that it can be found on resolution for test name
// generation.
// TODO(dawn:2205) Remove this in favor of custom serializer.
static std::ostream& operator<<(std::ostream& os, const MapMode& param) {
    switch (param) {
        case wgpu::MapMode::Read:
            os << "Read";
            break;
        case wgpu::MapMode::Write:
            os << "Write";
            break;
        default:
            DAWN_UNREACHABLE();
    }
    return os;
}
}  // namespace wgpu

namespace dawn::wire {
namespace {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::MockCppCallback;
using testing::Ne;
using testing::NotNull;
using testing::Return;
using testing::SizedString;
using testing::StrictMock;
using testing::WithArg;

using MapMode = wgpu::MapMode;
using MappedAtCreation = bool;
DAWN_TEST_PARAM_STRUCT_TYPES(MapModeParam, MapMode, MappedAtCreation);

MATCHER_P(AsUint32Eq, value, "") {
    return *reinterpret_cast<const uint32_t*>(arg) == value;
}

using MockClientReadHandle = client::MockMemoryTransferService::MockReadHandle;
using MockClientWriteHandle = client::MockMemoryTransferService::MockWriteHandle;
using MockServerReadHandle = server::MockMemoryTransferService::MockReadHandle;
using MockServerWriteHandle = server::MockMemoryTransferService::MockWriteHandle;
using MockClientHandles = std::tuple<MockClientReadHandle*, MockClientWriteHandle*>;
using MockServerHandles = std::tuple<MockServerReadHandle*, MockServerWriteHandle*>;

// WireMemoryTransferServiceTests test the MemoryTransferService with buffer mapping.
// They test the basic success and error cases for buffer mapping, and they test
// mocked failures of each fallible MemoryTransferService method that an embedder
// could implement.
// The test harness defines multiple helpers for expecting operations on Read/Write handles
// and for mocking failures.
// There are tests which check for Success for every mapping operation which mock an entire
// mapping operation from map to unmap, and add all MemoryTransferService expectations. Tests
// which check for errors perform the same mapping operations but insert mocked failures for
// various mapping or MemoryTransferService operations.
class WireMemoryTransferServiceTestBase : public WireTest,
                                          public testing::WithParamInterface<MapModeParam> {
  protected:
    client::MemoryTransferService* GetClientMemoryTransferService() override { return &mClientMTS; }
    server::MemoryTransferService* GetServerMemoryTransferService() override { return &mServerMTS; }

    wgpu::BufferUsage GetUsage() {
        switch (GetParam().mMapMode) {
            case wgpu::MapMode::Read:
                return wgpu::BufferUsage::MapRead;
            case wgpu::MapMode::Write:
                return wgpu::BufferUsage::MapWrite;
            default:
                DAWN_UNREACHABLE();
        }
    }

    std::pair<WGPUBuffer, wgpu::Buffer> CreateBuffer() {
        wgpu::BufferUsage usage = GetUsage();
        bool mappedAtCreation = GetParam().mMappedAtCreation;

        wgpu::BufferDescriptor descriptor = {};
        descriptor.size = kBufferSize;
        descriptor.mappedAtCreation = mappedAtCreation;
        descriptor.usage = usage;

        WGPUBuffer apiBuffer = api.GetNewBuffer();
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));

        return std::make_pair(apiBuffer, buffer);
    }

    std::tuple<WGPUBuffer, wgpu::Buffer, MockClientHandles, MockServerHandles> CreateValidBuffer() {
        WGPUBuffer apiBuffer;
        wgpu::Buffer buffer;

        // The client should create and serialize the appropriate handles on buffer creation.
        auto clientHandles = ExpectHandleCreation(true);
        ExpectHandleSerialization(clientHandles);
        std::tie(apiBuffer, buffer) = CreateBuffer();

        // When the commands are flushed, the server should appropriately deserialize the handles.
        auto serverHandles = ExpectHandleDeserialization(true);
        if (GetParam().mMappedAtCreation) {
            EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
                .WillOnce(Return(&mServerBufferContent));
        }
        FlushClient();

        return std::make_tuple(apiBuffer, buffer, clientHandles, serverHandles);
    }

    MockClientHandles ExpectHandleCreation(bool success) {
        wgpu::MapMode mode = GetParam().mMapMode;
        bool mappedAtCreation = GetParam().mMappedAtCreation;
        switch (mode) {
            case wgpu::MapMode::Read: {
                auto* readHandle = success ? new StrictMock<MockClientReadHandle>() : nullptr;
                EXPECT_CALL(mClientMTS, CreateReadHandle(kBufferSize)).WillOnce(Return(readHandle));
                if (!success) {
                    return std::make_tuple(nullptr, nullptr);
                }
                if (mappedAtCreation) {
                    auto* writeHandle = new StrictMock<MockClientWriteHandle>();
                    EXPECT_CALL(mClientMTS, CreateWriteHandle(kBufferSize))
                        .WillOnce(Return(writeHandle));
                    EXPECT_CALL(*writeHandle, GetData).WillOnce(Return(&mClientBufferContent));
                    return std::make_tuple(readHandle, writeHandle);
                }
                return std::make_tuple(readHandle, nullptr);
            }
            case wgpu::MapMode::Write: {
                auto* writeHandle = success ? new StrictMock<MockClientWriteHandle>() : nullptr;
                EXPECT_CALL(mClientMTS, CreateWriteHandle(kBufferSize))
                    .WillOnce(Return(writeHandle));
                if (!success) {
                    return std::make_tuple(nullptr, nullptr);
                }
                if (mappedAtCreation) {
                    EXPECT_CALL(*writeHandle, GetData).WillOnce(Return(&mClientBufferContent));
                }
                return std::make_tuple(nullptr, writeHandle);
            }
            default:
                DAWN_UNREACHABLE();
        }
    }

    void ExpectHandleSerialization(MockClientHandles& clientHandles) {
        auto* readHandle = std::get<MockClientReadHandle*>(clientHandles);
        if (readHandle) {
            EXPECT_CALL(*readHandle, SerializeCreateSize).WillOnce(InvokeWithoutArgs([&] {
                return sizeof(mSerializeCreateInfo);
            }));
            EXPECT_CALL(*readHandle, SerializeCreate(_))
                .WillOnce(WithArg<0>([&](void* serializePointer) {
                    memcpy(serializePointer, &mSerializeCreateInfo, kDataSize);
                    return kDataSize;
                }));
        }
        auto* writeHandle = std::get<MockClientWriteHandle*>(clientHandles);
        if (writeHandle) {
            EXPECT_CALL(*writeHandle, SerializeCreateSize).WillOnce(InvokeWithoutArgs([&] {
                return sizeof(mSerializeCreateInfo);
            }));
            EXPECT_CALL(*writeHandle, SerializeCreate(_))
                .WillOnce(WithArg<0>([&](void* serializePointer) {
                    memcpy(serializePointer, &mSerializeCreateInfo, kDataSize);
                    return kDataSize;
                }));
        }
    }

    MockServerHandles ExpectHandleDeserialization(bool success) {
        wgpu::MapMode mode = GetParam().mMapMode;
        bool mappedAtCreation = GetParam().mMappedAtCreation;
        switch (mode) {
            case wgpu::MapMode::Read: {
                MockServerWriteHandle* writeHandle = nullptr;
                if (mappedAtCreation) {
                    writeHandle = success ? new StrictMock<MockServerWriteHandle>() : nullptr;
                    EXPECT_CALL(mServerMTS, DeserializeWriteHandle(AsUint32Eq(mSerializeCreateInfo),
                                                                   kDataSize, _))
                        .WillOnce(
                            WithArg<2>([=](server::MemoryTransferService::WriteHandle** handle) {
                                *handle = writeHandle;
                                return success;
                            }));
                    if (!success) {
                        return std::make_tuple(nullptr, nullptr);
                    }
                }

                auto* readHandle = success ? new StrictMock<MockServerReadHandle>() : nullptr;
                EXPECT_CALL(mServerMTS,
                            DeserializeReadHandle(AsUint32Eq(mSerializeCreateInfo), kDataSize, _))
                    .WillOnce(WithArg<2>([=](server::MemoryTransferService::ReadHandle** handle) {
                        *handle = readHandle;
                        return success;
                    }));
                return std::make_tuple(readHandle, writeHandle);
            }
            case wgpu::MapMode::Write: {
                auto* writeHandle = success ? new StrictMock<MockServerWriteHandle>() : nullptr;
                EXPECT_CALL(mServerMTS,
                            DeserializeWriteHandle(AsUint32Eq(mSerializeCreateInfo), kDataSize, _))
                    .WillOnce(WithArg<2>([=](server::MemoryTransferService::WriteHandle** handle) {
                        *handle = writeHandle;
                        return success;
                    }));
                return std::make_tuple(nullptr, writeHandle);
            }
            default:
                DAWN_UNREACHABLE();
        }
    }

    void ExpectClientSerializeData(MockClientHandles& clientHandles) {
        auto* clientHandle = std::get<MockClientWriteHandle*>(clientHandles);
        if (!clientHandle) {
            return;
        }

        EXPECT_CALL(*clientHandle, SizeOfSerializeDataUpdate(_, _)).WillOnce(Return(kDataSize));
        EXPECT_CALL(*clientHandle, SerializeDataUpdate)
            .WillOnce(WithArg<0>([&](void* serializePointer) {
                memcpy(serializePointer, &mClientBufferContent, kBufferSize);
            }));
    }
    void ExpectServerSerializeData(MockServerHandles& serverHandles) {
        auto* serverHandle = std::get<MockServerReadHandle*>(serverHandles);
        if (!serverHandle) {
            return;
        }

        EXPECT_CALL(*serverHandle, SizeOfSerializeDataUpdate(_, _)).WillOnce(Return(kDataSize));
        EXPECT_CALL(*serverHandle, SerializeDataUpdate)
            .WillOnce(WithArg<3>([&](void* serializePointer) {
                memcpy(serializePointer, &mServerBufferContent, kBufferSize);
                return kBufferSize;
            }));
    }

    void ExpectClientDeserializeData(bool success, MockClientHandles& clientHandles) {
        auto* clientHandle = std::get<MockClientReadHandle*>(clientHandles);
        if (!clientHandle) {
            return;
        }

        EXPECT_CALL(*clientHandle, DeserializeDataUpdate(_, kBufferSize, 0, kBufferSize))
            .WillOnce(WithArg<0>([&, success](const void* deserializePointer) {
                if (success) {
                    // Copy the data manually here.
                    memcpy(&mClientBufferContent, deserializePointer, kBufferSize);
                }
                return success;
            }));
    }
    void ExpectServerDeserializeData(bool success, MockServerHandles& serverHandles) {
        auto* serverHandle = std::get<MockServerWriteHandle*>(serverHandles);
        if (!serverHandle) {
            return;
        }

        EXPECT_CALL(*serverHandle, DeserializeDataUpdate(_, kBufferSize, 0, kBufferSize))
            .WillOnce(WithArg<0>([&, success](const void* deserializePointer) {
                if (success) {
                    // Copy the data manually here.
                    memcpy(&mServerBufferContent, deserializePointer, kBufferSize);
                }
                return success;
            }));
    }

    void ExpectClientHandleDestruction(MockClientHandles& clientHandles) {
        auto* readHandle = std::get<MockClientReadHandle*>(clientHandles);
        if (readHandle) {
            EXPECT_CALL(*readHandle, Destroy).Times(1);
        }
        auto* writeHandle = std::get<MockClientWriteHandle*>(clientHandles);
        if (writeHandle) {
            EXPECT_CALL(*writeHandle, Destroy).Times(1);
        }
    }
    void ExpectServerHandleDestruction(MockServerHandles& serverHandles) {
        auto* readHandle = std::get<MockServerReadHandle*>(serverHandles);
        if (readHandle) {
            EXPECT_CALL(*readHandle, Destroy).Times(1);
        }
        auto* writeHandle = std::get<MockServerWriteHandle*>(serverHandles);
        if (writeHandle) {
            EXPECT_CALL(*writeHandle, Destroy).Times(1);
        }
    }

    // Sets expectations for a successful map async call and verifies that the results match for the
    // MapMode.
    void ExpectSuccessfulMapAsync(WGPUBuffer apiBuffer,
                                  wgpu::Buffer buffer,
                                  MockClientHandles& clientHandles,
                                  MockServerHandles& serverHandles) {
        wgpu::MapMode mode = GetParam().mMapMode;

        // Mode independent expectations.
        EXPECT_CALL(api,
                    OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mode), 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                               kEmptyOutputStringView);
            }));
        EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

        switch (mode) {
            case wgpu::MapMode::Read: {
                auto* clientHandle = std::get<MockClientReadHandle*>(clientHandles);
                ASSERT_THAT(clientHandle, NotNull());
                EXPECT_CALL(*clientHandle, GetData).WillOnce(Return(&mClientBufferContent));
                EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
                    .WillOnce(Return(&mServerBufferContent));

                buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                                mMapAsyncCb.Callback());

                // The server should serialize its buffer when the client flushes.
                ExpectServerSerializeData(serverHandles);
                FlushClient();

                // The client should deserialize into its buffer when the server flushes.
                ExpectClientDeserializeData(true, clientHandles);
                FlushServer();

                // The data between the server and the client should be the same now.
                EXPECT_EQ(mServerBufferContent, mClientBufferContent);
                break;
            }
            case wgpu::MapMode::Write: {
                auto* clientHandle = std::get<MockClientWriteHandle*>(clientHandles);
                ASSERT_THAT(clientHandle, NotNull());
                EXPECT_CALL(*clientHandle, GetData).WillOnce(Return(&mClientBufferContent));
                EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
                    .WillOnce(Return(&mClientBufferContent));

                buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                                mMapAsyncCb.Callback());
                FlushClient();
                FlushServer();
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }

    // Note that we pass in the mode explicitly here to allow reuse for mappedAtCreation.
    void ExpectSuccessfulUnmap(wgpu::MapMode mode,
                               WGPUBuffer apiBuffer,
                               wgpu::Buffer buffer,
                               MockClientHandles& clientHandles,
                               MockServerHandles& serverHandles) {
        switch (mode) {
            case wgpu::MapMode::Read: {
                // Unmap the buffer.
                buffer.Unmap();
                EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
                FlushClient();
                break;
            }
            case wgpu::MapMode::Write: {
                // The client should serialize its buffer when Unmap is called.
                ExpectClientSerializeData(clientHandles);
                buffer.Unmap();

                // The server should deserialize into its buffer when the client flushes.
                EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
                ExpectServerDeserializeData(true, serverHandles);
                FlushClient();

                // The data between the server and the client should be the same now.
                EXPECT_EQ(mServerBufferContent, mClientBufferContent);
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }

    void ExpectSuccessfulUnmapAtCreation(WGPUBuffer apiBuffer,
                                         wgpu::Buffer buffer,
                                         MockClientHandles& clientHandles,
                                         MockServerHandles& serverHandles) {
        ASSERT_TRUE(GetParam().mMappedAtCreation);

        // Ensure that the contents on the client and server side are different now as a part of the
        // test.
        mClientBufferContent = kDataGenerator++;
        mServerBufferContent = kDataGenerator++;
        ASSERT_THAT(mClientBufferContent, Ne(mServerBufferContent));

        // If the map mode of the buffer was actually MapRead, we should expect destruction of the
        // WriteHandles, and setting the ReadHandles instead.
        bool isRead = GetParam().mMapMode == wgpu::MapMode::Read;
        if (isRead) {
            auto* clientWriteHandle = std::get<MockClientWriteHandle*>(clientHandles);
            ASSERT_THAT(clientWriteHandle, NotNull());
            EXPECT_CALL(*clientWriteHandle, Destroy).Times(1);
            auto* clientReadHandle = std::get<MockClientReadHandle*>(clientHandles);
            ASSERT_THAT(clientReadHandle, NotNull());
            EXPECT_CALL(*clientReadHandle, GetData).WillOnce(Return(&mClientBufferContent));

            auto* serverHandle = std::get<MockServerWriteHandle*>(serverHandles);
            ASSERT_THAT(serverHandle, NotNull());
            EXPECT_CALL(*serverHandle, Destroy).Times(1);
        }

        ExpectSuccessfulUnmap(wgpu::MapMode::Write, apiBuffer, buffer, clientHandles,
                              serverHandles);

        // Update the handles accordingly if we are MapRead.
        if (isRead) {
            clientHandles =
                std::make_tuple(std::get<MockClientReadHandle*>(clientHandles), nullptr);
            serverHandles =
                std::make_tuple(std::get<MockServerReadHandle*>(serverHandles), nullptr);
        }
    }

    // Static atomic that's used to generate different values across tests.
    static std::atomic<uint32_t> kDataGenerator;

    uint32_t mClientBufferContent = 0;
    uint32_t mServerBufferContent = 0;
    static constexpr size_t kBufferSize = sizeof(uint32_t);

    uint32_t mSerializeCreateInfo = kDataGenerator++;
    static constexpr size_t kDataSize = sizeof(uint32_t);

    StrictMock<MockCppCallback<wgpu::BufferMapCallback<void>*>> mMapAsyncCb;

    StrictMock<wire::server::MockMemoryTransferService> mServerMTS;
    StrictMock<wire::client::MockMemoryTransferService> mClientMTS;
};
std::atomic<uint32_t> WireMemoryTransferServiceTestBase::kDataGenerator = 1337;

class WireMemoryTransferServiceBufferHandleTests : public WireMemoryTransferServiceTestBase {};
INSTANTIATE_TEST_SUITE_P(
    ,
    WireMemoryTransferServiceBufferHandleTests,
    testing::ValuesIn(ParamGenerator<WireMemoryTransferServiceBufferHandleTests::ParamType,
                                     MapMode,
                                     MappedAtCreation>({wgpu::MapMode::Read, wgpu::MapMode::Write},
                                                       {true, false})),
    &TestParamToString<WireMemoryTransferServiceBufferHandleTests::ParamType>);

// Test handle(s) destroy behavior.
TEST_P(WireMemoryTransferServiceBufferHandleTests, Destroy) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientHandles clientHandles;
    MockServerHandles serverHandles;
    std::tie(apiBuffer, buffer, clientHandles, serverHandles) = CreateValidBuffer();

    // The client handles are destroyed on buffer.Destroy().
    ExpectClientHandleDestruction(clientHandles);
    buffer.Destroy();

    // The server handles are destroyed when the destroy command is flushed.
    ExpectServerHandleDestruction(serverHandles);
    EXPECT_CALL(api, BufferDestroy(apiBuffer)).Times(1);
    FlushClient();

    // Releasing the buffer should be reflected on the server when flushed.
    buffer = nullptr;
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test handle(s) creation failure.
TEST_P(WireMemoryTransferServiceBufferHandleTests, CreationFailure) {
    ExpectHandleCreation(false);

    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = kBufferSize;
    descriptor.usage = GetUsage();

    WGPUBuffer apiErrorBuffer = api.GetNewBuffer();
    wgpu::Buffer errorBuffer = device.CreateBuffer(&descriptor);
    EXPECT_CALL(api, DeviceCreateErrorBuffer(apiDevice, _)).WillOnce(Return(apiErrorBuffer));
    FlushClient();

    // Releasing the buffer should be reflected on the server when flushed.
    errorBuffer = nullptr;
    EXPECT_CALL(api, BufferRelease(apiErrorBuffer)).Times(1);
    FlushClient();
}

// Test handle(s) deserialization (only the handles across the wire, not the data) failure.
TEST_P(WireMemoryTransferServiceBufferHandleTests, DeserializationFailure) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;

    // The client should create and serialize the appropriate handles on buffer creation.
    auto clientHandles = ExpectHandleCreation(true);
    ExpectHandleSerialization(clientHandles);
    std::tie(apiBuffer, buffer) = CreateBuffer();

    // When the commands are flushed, mock that the server fails to deserialize the handle.
    ExpectHandleDeserialization(false);
    FlushClient(false);

    // The client handles are destroyed when the buffer is released.
    ExpectClientHandleDestruction(clientHandles);
    buffer = nullptr;

    // Releasing the buffer should be reflected on the server when flushed.
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

class WireMemoryTransferServiceBufferMapAsyncTests : public WireMemoryTransferServiceTestBase {};
INSTANTIATE_TEST_SUITE_P(
    ,
    WireMemoryTransferServiceBufferMapAsyncTests,
    testing::ValuesIn(ParamGenerator<WireMemoryTransferServiceBufferMapAsyncTests::ParamType,
                                     MapMode,
                                     MappedAtCreation>({wgpu::MapMode::Read, wgpu::MapMode::Write},
                                                       {true, false})),
    &TestParamToString<WireMemoryTransferServiceBufferMapAsyncTests::ParamType>);

// Test successful mapping.
TEST_P(WireMemoryTransferServiceBufferMapAsyncTests, Success) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientHandles clientHandles;
    MockServerHandles serverHandles;
    std::tie(apiBuffer, buffer, clientHandles, serverHandles) = CreateValidBuffer();

    // If we were mappedAtCreation, successfully handle that initial unmapping now.
    if (GetParam().mMappedAtCreation) {
        ExpectSuccessfulUnmapAtCreation(apiBuffer, buffer, clientHandles, serverHandles);
    }

    // Ensure that the contents on the client and server side are different now as a part of the
    // test.
    mClientBufferContent = kDataGenerator++;
    mServerBufferContent = kDataGenerator++;
    ASSERT_THAT(mClientBufferContent, Ne(mServerBufferContent));

    ExpectSuccessfulMapAsync(apiBuffer, buffer, clientHandles, serverHandles);
    ExpectSuccessfulUnmap(GetParam().mMapMode, apiBuffer, buffer, clientHandles, serverHandles);

    // The client handles are destroyed when the buffer is released.
    ExpectClientHandleDestruction(clientHandles);
    buffer = nullptr;

    // The server handles are destroyed when the release command is flushed.
    ExpectServerHandleDestruction(serverHandles);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test unsuccessful mapping with error.
TEST_P(WireMemoryTransferServiceBufferMapAsyncTests, Error) {
    wgpu::MapMode mode = GetParam().mMapMode;

    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientHandles clientHandles;
    MockServerHandles serverHandles;
    std::tie(apiBuffer, buffer, clientHandles, serverHandles) = CreateValidBuffer();

    // If we were mappedAtCreation, successfully handle that initial unmapping now.
    if (GetParam().mMappedAtCreation) {
        ExpectSuccessfulUnmapAtCreation(apiBuffer, buffer, clientHandles, serverHandles);
    }

    buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                    mMapAsyncCb.Callback());

    // Make the server respond to the callback with an error.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mode), 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Error,
                                           ToOutputStringView("Validation error"));
        }));
    FlushClient();

    // The callback should happen when the server flushes the response.
    EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Error, SizedString("Validation error")))
        .Times(1);
    FlushServer();

    // Unmap the buffer.
    buffer.Unmap();
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    FlushClient();

    // The client handles are destroyed when the buffer is released.
    ExpectClientHandleDestruction(clientHandles);
    buffer = nullptr;

    // The server handles are destroyed when the release command is flushed.
    ExpectServerHandleDestruction(serverHandles);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test DeserializeDataUpdate (actual data) failure w.r.t MapAsync.
TEST_P(WireMemoryTransferServiceBufferMapAsyncTests, DeserializeDataUpdateFailure) {
    wgpu::MapMode mode = GetParam().mMapMode;

    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientHandles clientHandles;
    MockServerHandles serverHandles;
    std::tie(apiBuffer, buffer, clientHandles, serverHandles) = CreateValidBuffer();

    // If we were mappedAtCreation, successfully handle that initial unmapping now.
    if (GetParam().mMappedAtCreation) {
        ExpectSuccessfulUnmapAtCreation(apiBuffer, buffer, clientHandles, serverHandles);
    }

    // Set mode independent expectations for the map async call now.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mode), 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));

    switch (mode) {
        case wgpu::MapMode::Read: {
            EXPECT_CALL(mMapAsyncCb, Call(Ne(wgpu::MapAsyncStatus::Success), _)).Times(1);
            EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
                .WillOnce(Return(&mServerBufferContent));

            buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                            mMapAsyncCb.Callback());

            // The server should serialize its buffer when the client flushes.
            ExpectServerSerializeData(serverHandles);
            FlushClient();

            // Mock that the client fails to deserialize into its buffer when the server flushes.
            ExpectClientDeserializeData(false, clientHandles);
            FlushServer(false);
            break;
        }
        case wgpu::MapMode::Write: {
            EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
            EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
                .WillOnce(Return(&mClientBufferContent));
            auto* clientHandle = std::get<MockClientWriteHandle*>(clientHandles);
            ASSERT_THAT(clientHandle, NotNull());
            EXPECT_CALL(*clientHandle, GetData).WillOnce(Return(&mClientBufferContent));

            buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                            mMapAsyncCb.Callback());
            FlushClient();
            FlushServer();

            // The client should serialize its buffer when Unmap is called.
            ExpectClientSerializeData(clientHandles);
            buffer.Unmap();

            // Mock that the server fails to deserialize into its buffer when the client flushes.
            ExpectServerDeserializeData(false, serverHandles);
            FlushClient(false);
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }

    // The client handles are destroyed when the buffer is released.
    ExpectClientHandleDestruction(clientHandles);
    buffer = nullptr;

    // The server handles are destroyed when the release command is flushed.
    ExpectServerHandleDestruction(serverHandles);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test mapping and then destroying the buffer before unmapping on the client side.
TEST_P(WireMemoryTransferServiceBufferMapAsyncTests, DestroyBeforeUnmap) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientHandles clientHandles;
    MockServerHandles serverHandles;
    std::tie(apiBuffer, buffer, clientHandles, serverHandles) = CreateValidBuffer();

    // If we were mappedAtCreation, successfully handle that initial unmapping now.
    if (GetParam().mMappedAtCreation) {
        ExpectSuccessfulUnmapAtCreation(apiBuffer, buffer, clientHandles, serverHandles);
    }

    ExpectSuccessfulMapAsync(apiBuffer, buffer, clientHandles, serverHandles);

    // THIS IS THE TEST: destroy the buffer before unmapping and check it destroyed the mapping
    // immediately, both in the client and server side.
    {
        // Destroying the buffer should immediately destroy the client handles.
        ExpectClientHandleDestruction(clientHandles);
        buffer.Destroy();

        // Flushing the client should destroy the server handles.
        ExpectServerHandleDestruction(serverHandles);
        EXPECT_CALL(api, BufferDestroy(apiBuffer)).Times(1);
        FlushClient();

        // The handle(s) are already destroyed so unmap only results in a server unmap call.
        buffer.Unmap();
        EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
        FlushClient();
    }

    // The handle(s) are already destroyed so release only results in a server release call.
    buffer = nullptr;
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

class WireMemoryTransferServiceBufferMappedAtCreationTests
    : public WireMemoryTransferServiceTestBase {};
INSTANTIATE_TEST_SUITE_P(
    ,
    WireMemoryTransferServiceBufferMappedAtCreationTests,
    testing::ValuesIn(
        ParamGenerator<WireMemoryTransferServiceBufferMappedAtCreationTests::ParamType,
                       MapMode,
                       MappedAtCreation>({wgpu::MapMode::Read, wgpu::MapMode::Write}, {true})),
    &TestParamToString<WireMemoryTransferServiceBufferMappedAtCreationTests::ParamType>);

// Test DeserializeDataUpdate (actual data) failure w.r.t for the initial Unmap.
TEST_P(WireMemoryTransferServiceBufferMappedAtCreationTests, DeserializeDataUpdateFailure) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientHandles clientHandles;
    MockServerHandles serverHandles;
    std::tie(apiBuffer, buffer, clientHandles, serverHandles) = CreateValidBuffer();

    // The client should serialize its buffer when Unmap is called. Additionally, if we are in read
    // mode, the client side WriteHandle used for mappedAtCreation should be destroyed now and it
    // should reset to use the ReadHandle.
    ExpectClientSerializeData(clientHandles);
    if (GetParam().mMapMode == wgpu::MapMode::Read) {
        auto* clientWriteHandle = std::get<MockClientWriteHandle*>(clientHandles);
        ASSERT_THAT(clientWriteHandle, NotNull());
        EXPECT_CALL(*clientWriteHandle, Destroy).Times(1);
        auto* clientReadHandle = std::get<MockClientReadHandle*>(clientHandles);
        ASSERT_THAT(clientReadHandle, NotNull());
        EXPECT_CALL(*clientReadHandle, GetData).WillOnce(Return(&mClientBufferContent));
        clientHandles = std::make_tuple(std::get<MockClientReadHandle*>(clientHandles), nullptr);
    }
    buffer.Unmap();

    // Mock that the server fails to deserialize into its buffer when the client flushes.
    ExpectServerDeserializeData(false, serverHandles);
    FlushClient(false);

    // The client handles are destroyed when the buffer is released.
    ExpectClientHandleDestruction(clientHandles);
    buffer = nullptr;

    // The server handles are destroyed when the release command is flushed.
    ExpectServerHandleDestruction(serverHandles);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test mapping and then destroying the buffer before unmapping on the client side.
TEST_P(WireMemoryTransferServiceBufferMappedAtCreationTests, DestroyBeforeUnmap) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientHandles clientHandles;
    MockServerHandles serverHandles;
    std::tie(apiBuffer, buffer, clientHandles, serverHandles) = CreateValidBuffer();

    // THIS IS THE TEST: destroy the buffer before unmapping and check it destroyed the mapping
    // immediately, both in the client and server side.
    {
        // Destroying the buffer should immediately destroy the client handles.
        ExpectClientHandleDestruction(clientHandles);
        buffer.Destroy();

        // Flushing the client should destroy the server handles.
        ExpectServerHandleDestruction(serverHandles);
        EXPECT_CALL(api, BufferDestroy(apiBuffer)).Times(1);
        FlushClient();

        // The handle(s) are already destroyed so unmap only results in a server unmap call.
        buffer.Unmap();
        EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
        FlushClient();
    }

    // The handle(s) are already destroyed so release only results in a server release call.
    buffer = nullptr;
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
