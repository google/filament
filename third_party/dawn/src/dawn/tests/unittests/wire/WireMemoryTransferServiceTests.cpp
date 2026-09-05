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

#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/tests/MockCallback.h"
#include "src/dawn/tests/ParamGenerator.h"
#include "src/dawn/tests/StringViewMatchers.h"
#include "src/dawn/tests/unittests/wire/WireTest.h"
#include "src/dawn/wire/client/ClientMemoryTransferService_mock.h"
#include "src/dawn/wire/server/ServerMemoryTransferService_mock.h"
#include "src/utils/compiler.h"

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
        case wgpu::MapMode::None:
            os << "None";
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

using MockClientMemoryHandle = client::MockMemoryTransferService::MockMemoryHandle;
using MockServerMemoryHandle = server::MockMemoryTransferService::MockMemoryHandle;

// WireMemoryTransferServiceTests test the MemoryTransferService with buffer mapping.
// They test the basic success and error cases for buffer mapping, and they test
// mocked failures of each fallible MemoryTransferService method that an embedder
// could implement.
// The test harness defines multiple helpers for expecting operations on MemoryHandles
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
            case wgpu::MapMode::None:
                return wgpu::BufferUsage::Vertex;
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

    std::span<std::byte> GetSpanToClientBufferContent() {
        return {ByteSpanFromRef(mClientBufferContent)};
    }

    std::tuple<WGPUBuffer, wgpu::Buffer, MockClientMemoryHandle*, MockServerMemoryHandle*>
    CreateValidBuffer() {
        WGPUBuffer apiBuffer;
        wgpu::Buffer buffer;

        // The client should create and serialize the appropriate handles on buffer creation.
        auto clientHandle = ExpectHandleCreation(true);
        ExpectHandleSerialization(clientHandle);
        std::tie(apiBuffer, buffer) = CreateBuffer();

        // When the commands are flushed, the server should appropriately deserialize the handles.
        auto serverHandle = ExpectHandleDeserialization(true);
        FlushClient();

        return std::make_tuple(apiBuffer, buffer, clientHandle, serverHandle);
    }

    MockClientMemoryHandle* ExpectHandleCreation(bool success) {
        if (!success) {
            EXPECT_CALL(mClientMTS, CreateMemoryHandle(kBufferSize)).WillOnce(Return(nullptr));
            return nullptr;
        }

        auto* memoryHandle = new StrictMock<MockClientMemoryHandle>();

        EXPECT_CALL(mClientMTS, CreateMemoryHandle(kBufferSize))
            .WillOnce(Return(std::unique_ptr<MockClientMemoryHandle>(memoryHandle)));

        if (GetParam().mMappedAtCreation || GetParam().mMapMode == wgpu::MapMode::Write) {
            EXPECT_CALL(*memoryHandle, GetData)
                .WillOnce(Return(GetSpanToClientBufferContent()))
                .RetiresOnSaturation();
        }

        if (GetParam().mMappedAtCreation) {
            EXPECT_CALL(*memoryHandle, GetData)
                .WillOnce(Return(GetSpanToClientBufferContent()))
                .RetiresOnSaturation();
        }

        return memoryHandle;
    }

    void ExpectHandleSerialization(MockClientMemoryHandle* clientHandle) {
        DAWN_ASSERT(clientHandle != nullptr);

        EXPECT_CALL(*clientHandle, GetSerializeCreateSize).WillOnce([&] {
            return sizeof(mSerializeCreateInfo);
        });
        EXPECT_CALL(*clientHandle, SerializeCreate)
            .WillOnce(WithArg<0>([&](std::span<std::byte> serializeBuffer) {
                // TODO(https://crbug.com/524406299) use copy_from.
                DAWN_UNSAFE_TODO(memcpy(serializeBuffer.data(), &mSerializeCreateInfo, kDataSize));
            }));
    }

    MockServerMemoryHandle* ExpectHandleDeserialization(bool success) {
        if (!success) {
            EXPECT_CALL(mServerMTS, DeserializeMemoryHandle).WillOnce(Return(nullptr));
            return nullptr;
        }

        auto* memoryHandle = new StrictMock<MockServerMemoryHandle>();
        EXPECT_CALL(mServerMTS, DeserializeMemoryHandle)
            .WillOnce(Return(std::unique_ptr<MockServerMemoryHandle>(memoryHandle)));
        return memoryHandle;
    }

    void ExpectClientSerializeData(MockClientMemoryHandle* clientHandle) {
        DAWN_ASSERT(clientHandle != nullptr);

        EXPECT_CALL(*clientHandle, GetSerializeDataUpdateSize).WillOnce(Return(kBufferSize));
        EXPECT_CALL(*clientHandle, SerializeDataUpdate)
            .WillOnce(WithArg<0>([&](std::span<std::byte> serializeSpan) {
                // TODO(https://crbug.com/524406299) use copy_from.
                DAWN_UNSAFE_TODO(memcpy(serializeSpan.data(), &mClientBufferContent, kBufferSize));
            }));
    }
    void ExpectServerSerializeData(MockServerMemoryHandle* serverHandle) {
        DAWN_ASSERT(serverHandle != nullptr);

        EXPECT_CALL(*serverHandle, GetSerializeDataUpdateSize).WillOnce(Return(kBufferSize));
        EXPECT_CALL(*serverHandle, SerializeDataUpdate)
            .WillOnce(WithArg<0>([&](std::span<std::byte> serializeSpan) {
                // TODO(https://crbug.com/524406299) use copy_from.
                DAWN_UNSAFE_TODO(memcpy(serializeSpan.data(), &mServerBufferContent, kBufferSize));
            }));
    }

    void ExpectClientDeserializeData(bool success, MockClientMemoryHandle* clientHandle) {
        DAWN_ASSERT(clientHandle != nullptr);

        EXPECT_CALL(*clientHandle,
                    DeserializeDataUpdate(testing::Matcher<std::span<const std::byte>>(
                                              testing::Truly([](std::span<const std::byte> arg) {
                                                  return arg.size() == kBufferSize;
                                              })),
                                          static_cast<size_t>(0u), kBufferSize))
            .WillOnce(WithArg<0>([&, success](std::span<const std::byte> deserializeSpan) {
                if (success) {
                    // TODO(https://crbug.com/524406299) use copy_from.
                    DAWN_UNSAFE_TODO(
                        memcpy(&mClientBufferContent, deserializeSpan.data(), kBufferSize));
                }
                return success;
            }));
    }
    void ExpectServerDeserializeData(bool success, MockServerMemoryHandle* serverHandle) {
        DAWN_ASSERT(serverHandle != nullptr);

        std::span<std::byte> target{ByteSpanFromRef(mServerBufferContent)};
        EXPECT_CALL(
            *serverHandle,
            DeserializeDataUpdate(
                testing::Matcher<std::span<const std::byte>>(testing::Truly(
                    [](std::span<const std::byte> arg) { return arg.size() == kBufferSize; })),
                static_cast<size_t>(0u), kBufferSize,
                testing::Matcher<std::span<std::byte>>(
                    testing::Truly([target](std::span<std::byte> arg) {
                        return arg.data() == target.data() && arg.size() == target.size();
                    }))))
            .WillOnce(WithArg<0>([&, success](std::span<const std::byte> deserializePointer) {
                if (success) {
                    // TODO(https://crbug.com/524406299) use copy_from.
                    DAWN_UNSAFE_TODO(memcpy(&mServerBufferContent, deserializePointer.data(),
                                            deserializePointer.size()));
                }
                return success;
            }));
    }

    // Sets expectations for a successful map async call and verifies that the results match for the
    // MapMode.
    void ExpectSuccessfulMapAsync(WGPUBuffer apiBuffer,
                                  wgpu::Buffer buffer,
                                  MockClientMemoryHandle* clientHandle,
                                  MockServerMemoryHandle* serverHandle) {
        DAWN_ASSERT(clientHandle != nullptr);
        DAWN_ASSERT(serverHandle != nullptr);

        wgpu::MapMode mode = GetParam().mMapMode;

        // Mode independent expectations.
        EXPECT_CALL(api,
                    OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mode), 0, kBufferSize, _))
            .WillOnce([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                               kEmptyOutputStringView);
            });
        EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

        switch (mode) {
            case wgpu::MapMode::Read: {
                EXPECT_CALL(*clientHandle, GetData)
                    .WillOnce(Return(GetSpanToClientBufferContent()));
                EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
                    .WillOnce(Return(&mServerBufferContent));

                buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                                mMapAsyncCb.Callback());

                // The server should serialize its buffer when the client flushes.
                ExpectServerSerializeData(serverHandle);
                FlushClient();

                // The client should deserialize into its buffer when the server flushes.
                ExpectClientDeserializeData(true, clientHandle);
                FlushServer();

                // The data between the server and the client should be the same now.
                EXPECT_EQ(mServerBufferContent, mClientBufferContent);
                break;
            }
            case wgpu::MapMode::Write: {
                EXPECT_CALL(*clientHandle, GetData)
                    .WillOnce(Return(GetSpanToClientBufferContent()));

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
                               MockClientMemoryHandle* clientHandle,
                               MockServerMemoryHandle* serverHandle) {
        DAWN_ASSERT(clientHandle != nullptr);
        DAWN_ASSERT(serverHandle != nullptr);

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
                ExpectClientSerializeData(clientHandle);
                buffer.Unmap();

                // The server should deserialize into its buffer when the client flushes.
                EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
                EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
                    .WillOnce(Return(&mServerBufferContent));
                ExpectServerDeserializeData(true, serverHandle);
                FlushClient();

                // The data between the server and the client should be the same now.
                EXPECT_EQ(mServerBufferContent, mClientBufferContent);
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }

    // Not that this function sets the client/serverHandle to nullptr in the caller if the unmap
    // ends up destroying the handles. Other Expect* assume ASSERT handles are not nullptr so it is
    // the test's responsibility to not call them when handles are nullptr.
    void ExpectSuccessfulUnmapAtCreation(WGPUBuffer apiBuffer,
                                         wgpu::Buffer buffer,
                                         MockClientMemoryHandle*& clientHandle,
                                         MockServerMemoryHandle*& serverHandle) {
        DAWN_ASSERT(clientHandle != nullptr);
        DAWN_ASSERT(serverHandle != nullptr);
        ASSERT_TRUE(GetParam().mMappedAtCreation);

        // Ensure that the contents on the client and server side are different now as a part of the
        // test.
        mClientBufferContent = kDataGenerator++;
        mServerBufferContent = kDataGenerator++;
        ASSERT_THAT(mClientBufferContent, Ne(mServerBufferContent));

        // If the buffer was only mappable for mappedAtCreation, we expect destruction of the
        // MemoryHandle.
        bool isMappedAtCreationOnly =
            !(GetParam().mMapMode & (wgpu::MapMode::Read | wgpu::MapMode::Write));

        if (isMappedAtCreationOnly) {
            EXPECT_CALL(*clientHandle, GetData).WillOnce(Return(GetSpanToClientBufferContent()));
            EXPECT_CALL(*clientHandle, Destroy).Times(1);
            EXPECT_CALL(*serverHandle, Destroy).Times(1);
        }

        ExpectSuccessfulUnmap(wgpu::MapMode::Write, apiBuffer, buffer, clientHandle, serverHandle);

        if (isMappedAtCreationOnly) {
            clientHandle = nullptr;
            serverHandle = nullptr;
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
    MockClientMemoryHandle* clientHandle;
    MockServerMemoryHandle* serverHandle;
    std::tie(apiBuffer, buffer, clientHandle, serverHandle) = CreateValidBuffer();

    // The client handles are destroyed on buffer.Destroy().
    EXPECT_CALL(*clientHandle, Destroy).Times(1);
    buffer.Destroy();

    // The server handles are destroyed when the destroy command is flushed.
    EXPECT_CALL(*serverHandle, Destroy).Times(1);
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
    auto* clientHandle = ExpectHandleCreation(true);
    ExpectHandleSerialization(clientHandle);
    std::tie(apiBuffer, buffer) = CreateBuffer();

    // When the commands are flushed, mock that the server fails to deserialize the handle.
    ExpectHandleDeserialization(false);
    FlushClient(false);

    // The client handles are destroyed when the buffer is released.
    EXPECT_CALL(*clientHandle, Destroy).Times(1);
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
    MockClientMemoryHandle* clientHandle;
    MockServerMemoryHandle* serverHandle;
    std::tie(apiBuffer, buffer, clientHandle, serverHandle) = CreateValidBuffer();

    // If we were mappedAtCreation, successfully handle that initial unmapping now.
    if (GetParam().mMappedAtCreation) {
        ExpectSuccessfulUnmapAtCreation(apiBuffer, buffer, clientHandle, serverHandle);
    }

    // Ensure that the contents on the client and server side are different now as a part of the
    // test.
    mClientBufferContent = kDataGenerator++;
    mServerBufferContent = kDataGenerator++;
    ASSERT_THAT(mClientBufferContent, Ne(mServerBufferContent));

    ExpectSuccessfulMapAsync(apiBuffer, buffer, clientHandle, serverHandle);
    ExpectSuccessfulUnmap(GetParam().mMapMode, apiBuffer, buffer, clientHandle, serverHandle);

    // The client handles are destroyed when the buffer is released.
    EXPECT_CALL(*clientHandle, Destroy).Times(1);
    buffer = nullptr;

    // The server handles are destroyed when the release command is flushed.
    EXPECT_CALL(*serverHandle, Destroy).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test unsuccessful mapping with error.
TEST_P(WireMemoryTransferServiceBufferMapAsyncTests, Error) {
    wgpu::MapMode mode = GetParam().mMapMode;

    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientMemoryHandle* clientHandle;
    MockServerMemoryHandle* serverHandle;
    std::tie(apiBuffer, buffer, clientHandle, serverHandle) = CreateValidBuffer();

    // If we were mappedAtCreation, successfully handle that initial unmapping now.
    if (GetParam().mMappedAtCreation) {
        ExpectSuccessfulUnmapAtCreation(apiBuffer, buffer, clientHandle, serverHandle);
    }

    buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                    mMapAsyncCb.Callback());

    // Make the server respond to the callback with an error.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mode), 0, kBufferSize, _))
        .WillOnce([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Error,
                                           ToOutputStringView("Validation error"));
        });
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
    EXPECT_CALL(*clientHandle, Destroy).Times(1);
    buffer = nullptr;

    // The server handles are destroyed when the release command is flushed.
    EXPECT_CALL(*serverHandle, Destroy).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test DeserializeDataUpdate (actual data) failure w.r.t MapAsync.
TEST_P(WireMemoryTransferServiceBufferMapAsyncTests, DeserializeDataUpdateFailure) {
    wgpu::MapMode mode = GetParam().mMapMode;

    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientMemoryHandle* clientHandle;
    MockServerMemoryHandle* serverHandle;
    std::tie(apiBuffer, buffer, clientHandle, serverHandle) = CreateValidBuffer();

    // If we were mappedAtCreation, successfully handle that initial unmapping now.
    if (GetParam().mMappedAtCreation) {
        ExpectSuccessfulUnmapAtCreation(apiBuffer, buffer, clientHandle, serverHandle);
    }

    // Set mode independent expectations for the map async call now.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mode), 0, kBufferSize, _))
        .WillOnce([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        });

    switch (mode) {
        case wgpu::MapMode::Read: {
            EXPECT_CALL(mMapAsyncCb, Call(Ne(wgpu::MapAsyncStatus::Success), _)).Times(1);
            EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
                .WillOnce(Return(&mServerBufferContent));

            buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                            mMapAsyncCb.Callback());

            // The server should serialize its buffer when the client flushes.
            ExpectServerSerializeData(serverHandle);
            FlushClient();

            // Mock that the client fails to deserialize into its buffer when the server flushes.
            ExpectClientDeserializeData(false, clientHandle);
            FlushServer(false);
            break;
        }
        case wgpu::MapMode::Write: {
            EXPECT_CALL(mMapAsyncCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

            ASSERT_THAT(clientHandle, NotNull());
            EXPECT_CALL(*clientHandle, GetData).WillOnce(Return(GetSpanToClientBufferContent()));

            buffer.MapAsync(mode, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                            mMapAsyncCb.Callback());
            FlushClient();
            FlushServer();

            // The client should serialize its buffer when Unmap is called.
            ExpectClientSerializeData(clientHandle);
            buffer.Unmap();

            // Mock that the server fails to deserialize into its buffer when the client flushes.
            EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
                .WillOnce(Return(&mServerBufferContent));
            ExpectServerDeserializeData(false, serverHandle);
            FlushClient(false);
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }

    // The client handles are destroyed when the buffer is released.
    EXPECT_CALL(*clientHandle, Destroy).Times(1);
    buffer = nullptr;

    // The server handles are destroyed when the release command is flushed.
    EXPECT_CALL(*serverHandle, Destroy).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test mapping and then destroying the buffer before unmapping on the client side.
TEST_P(WireMemoryTransferServiceBufferMapAsyncTests, DestroyBeforeUnmap) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientMemoryHandle* clientHandle;
    MockServerMemoryHandle* serverHandle;
    std::tie(apiBuffer, buffer, clientHandle, serverHandle) = CreateValidBuffer();

    // If we were mappedAtCreation, successfully handle that initial unmapping now.
    if (GetParam().mMappedAtCreation) {
        ExpectSuccessfulUnmapAtCreation(apiBuffer, buffer, clientHandle, serverHandle);
    }

    ExpectSuccessfulMapAsync(apiBuffer, buffer, clientHandle, serverHandle);

    // THIS IS THE TEST: destroy the buffer before unmapping and check it destroyed the mapping
    // immediately, both in the client and server side.
    {
        // Destroying the buffer should immediately destroy the client handles.
        EXPECT_CALL(*clientHandle, Destroy).Times(1);
        buffer.Destroy();

        // Flushing the client should destroy the server handles.
        EXPECT_CALL(*serverHandle, Destroy).Times(1);
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
                       MappedAtCreation>({wgpu::MapMode::Read, wgpu::MapMode::Write,
                                          wgpu::MapMode::None},
                                         {true})),
    &TestParamToString<WireMemoryTransferServiceBufferMappedAtCreationTests::ParamType>);

// Test DeserializeDataUpdate (actual data) failure w.r.t for the initial Unmap.
TEST_P(WireMemoryTransferServiceBufferMappedAtCreationTests, DeserializeDataUpdateFailure) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientMemoryHandle* clientHandle;
    MockServerMemoryHandle* serverHandle;
    std::tie(apiBuffer, buffer, clientHandle, serverHandle) = CreateValidBuffer();

    // The client should serialize its buffer when Unmap is called. Additionally, if we are in just
    // mappedAtCreation, the MemoryHandle should be destroyed now.
    ExpectClientSerializeData(clientHandle);
    if (GetParam().mMapMode == wgpu::MapMode::None) {
        EXPECT_CALL(*clientHandle, Destroy).Times(1);
        clientHandle = nullptr;
    }
    buffer.Unmap();

    // Mock that the server fails to deserialize into its buffer when the client flushes.
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&mServerBufferContent));
    ExpectServerDeserializeData(false, serverHandle);
    FlushClient(false);

    // The client handles are destroyed when the buffer is released.
    if (clientHandle != nullptr) {
        EXPECT_CALL(*clientHandle, Destroy).Times(1);
    }
    buffer = nullptr;

    // The server handles are destroyed when the release command is flushed.
    EXPECT_CALL(*serverHandle, Destroy).Times(1);
    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    FlushClient();
}

// Test mapping and then destroying the buffer before unmapping on the client side.
TEST_P(WireMemoryTransferServiceBufferMappedAtCreationTests, DestroyBeforeUnmap) {
    WGPUBuffer apiBuffer;
    wgpu::Buffer buffer;
    MockClientMemoryHandle* clientHandle;
    MockServerMemoryHandle* serverHandle;
    std::tie(apiBuffer, buffer, clientHandle, serverHandle) = CreateValidBuffer();

    // THIS IS THE TEST: destroy the buffer before unmapping and check it destroyed the mapping
    // immediately, both in the client and server side.
    {
        // Destroying the buffer should immediately destroy the client handles.
        EXPECT_CALL(*clientHandle, Destroy).Times(1);
        buffer.Destroy();

        // Flushing the client should destroy the server handles.
        EXPECT_CALL(*serverHandle, Destroy).Times(1);
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
