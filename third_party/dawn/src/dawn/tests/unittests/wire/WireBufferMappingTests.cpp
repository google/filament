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

#include <limits>
#include <memory>

#include "dawn/common/Assert.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

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
using testing::Return;
using testing::SizedString;

// For the buffer tests, we make passing a map mode optional to reuse the same test fixture for
// tests that test multiple modes and tests that are mode specific. By making it an optional, it
// allows us to determine whether the map mode is necessary when generating the test names.
using MapMode = std::optional<wgpu::MapMode>;
DAWN_WIRE_FUTURE_TEST_PARAM_STRUCT(WireBufferParam, MapMode);

using WireBufferMappingTestBase =
    WireFutureTestWithParams<wgpu::BufferMapCallback<void>*, WireBufferParam>;

// General mapping tests that either do not care about the specific mapping mode, or apply to both.
class WireBufferMappingTests : public WireBufferMappingTestBase {
  protected:
    void SetUp() override {
        WireBufferMappingTestBase::SetUp();
        apiBuffer = api.GetNewBuffer();

        if (GetParam().mMapMode) {
            SetupBuffer(GetMapMode());
        }
    }

    void TearDown() override {
        // We must lose all references to objects before calling parent TearDown to avoid
        // referencing the proc table after it gets cleared.
        buffer = nullptr;

        WireBufferMappingTestBase::TearDown();
    }

    void MapAsync(wgpu::MapMode mode, size_t offset, size_t size) {
        this->mFutureIDs.push_back(buffer
                                       .MapAsync(mode, offset, size, this->GetParam().callbackMode,
                                                 this->mMockCb.Callback())
                                       .id);
    }

    wgpu::MapMode GetMapMode() {
        DAWN_ASSERT(GetParam().mMapMode);
        return *GetParam().mMapMode;
    }

    void SetupBuffer(wgpu::MapMode mapMode) {
        wgpu::BufferUsage usage = wgpu::BufferUsage::MapRead;
        if (mapMode == wgpu::MapMode::Read) {
            usage = wgpu::BufferUsage::MapRead;
        } else if (mapMode == wgpu::MapMode::Write) {
            usage = wgpu::BufferUsage::MapWrite;
        }

        wgpu::BufferDescriptor descriptor = {};
        descriptor.size = kBufferSize;
        descriptor.usage = usage;
        buffer = device.CreateBuffer(&descriptor);

        EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
            .WillOnce(Return(apiBuffer))
            .RetiresOnSaturation();
        FlushClient();
    }

    // Sets up the correct mapped range call expectations given the map mode.
    void ExpectMappedRangeCall(uint64_t bufferSize, void* bufferContent) {
        wgpu::MapMode mapMode = GetMapMode();
        if (mapMode == wgpu::MapMode::Read) {
            EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, bufferSize))
                .WillOnce(Return(bufferContent));
        } else if (mapMode == wgpu::MapMode::Write) {
            EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, bufferSize))
                .WillOnce(Return(bufferContent));
        }
    }

    // Test to exercise client functions that should override server response for callbacks.
    template <typename CancelFn, typename ExpFn>
    void TestEarlyMapCancelled(CancelFn cancelMapping,
                               ExpFn addExpectations,
                               wgpu::MapAsyncStatus expected,
                               const char* expectedMessage,
                               bool calledInCancelFn) {
        wgpu::MapMode mapMode = GetMapMode();
        MapAsync(mapMode, 0, kBufferSize);

        uint32_t bufferContent = 31337;
        EXPECT_CALL(
            api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                               kEmptyOutputStringView);
            }));
        ExpectMappedRangeCall(kBufferSize, &bufferContent);
        addExpectations();

        // The callback should get called with the expected status, regardless if the server has
        // responded.
        if (calledInCancelFn) {
            // In spontaneous mode, the callback gets called as a part of the cancel function.
            ExpectWireCallbacksWhen([&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(expected, SizedString(expectedMessage))).Times(1);

                cancelMapping();
            });
            FlushClient();
            FlushCallbacks();
        } else {
            // Otherwise, the callback will fire when we flush them.
            cancelMapping();
            FlushClient();
            ExpectWireCallbacksWhen([&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(expected, SizedString(expectedMessage))).Times(1);

                FlushCallbacks();
            });
        }
    }

    // Test to exercise client functions that should override server error response for callbacks.
    template <typename CancelFn, typename ExpFn>
    void TestEarlyMapErrorCancelled(CancelFn cancelMapping,
                                    ExpFn addExpectations,
                                    wgpu::MapAsyncStatus expected,
                                    const char* expectedMessage,
                                    bool calledInCancelFn) {
        wgpu::MapMode mapMode = GetMapMode();
        MapAsync(mapMode, 0, kBufferSize);

        EXPECT_CALL(
            api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Error,
                                               ToOutputStringView("Validation error"));
            }));

        // Ensure that the server had a chance to respond if relevant.
        FlushClient();
        FlushFutures();

        addExpectations();

        // The callback should get called with the expected status status, not server-side error,
        // even if the request fails on the server side.
        if (calledInCancelFn) {
            // In spontaneous mode, the callback gets called as a part of the cancel function.
            ExpectWireCallbacksWhen([&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(expected, SizedString(expectedMessage))).Times(1);

                cancelMapping();
            });
            FlushClient();
            FlushCallbacks();
        } else {
            // Otherwise, the callback will fire when we flush them.
            cancelMapping();
            FlushClient();
            ExpectWireCallbacksWhen([&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(expected, SizedString(expectedMessage))).Times(1);

                FlushCallbacks();
            });
        }
    }

    // Test to exercise client functions that would cancel callbacks don't cause the callback to be
    // fired twice.
    template <typename CancelFn, typename ExpFn>
    void TestCancelInCallback(CancelFn cancelMapping, ExpFn addExpectations) {
        wgpu::MapMode mapMode = GetMapMode();
        MapAsync(mapMode, 0, kBufferSize);

        uint32_t bufferContent = 31337;
        EXPECT_CALL(
            api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                               kEmptyOutputStringView);
            }));
        ExpectMappedRangeCall(kBufferSize, &bufferContent);

        // Ensure that the server had a chance to respond if relevant.
        FlushClient();
        FlushFutures();

        addExpectations();

        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).WillOnce([&]() {
                cancelMapping();
            });

            FlushCallbacks();
        });

        // Make sure that the cancel function is called and flush more callbacks to ensure that
        // nothing else happens.
        FlushClient();
        FlushFutures();
        FlushCallbacks();
    }

    static constexpr uint64_t kBufferSize = sizeof(uint32_t);
    // A successfully created buffer
    wgpu::Buffer buffer;
    WGPUBuffer apiBuffer;
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireBufferMappingTests,
                                    {wgpu::MapMode::Read, wgpu::MapMode::Write});

// Check that things work correctly when a validation error happens when mapping the buffer.
TEST_P(WireBufferMappingTests, ErrorWhileMapping) {
    wgpu::MapMode mapMode = GetMapMode();
    MapAsync(mapMode, 0, kBufferSize);

    EXPECT_CALL(api,
                OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Error,
                                           ToOutputStringView("Validation error"));
        }));

    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, SizedString("Validation error")))
            .Times(1);

        FlushCallbacks();
    });

    EXPECT_EQ(nullptr, buffer.GetConstMappedRange(0, kBufferSize));
}

// Check the map callback when the map request would have worked, but Unmap() was called.
TEST_P(WireBufferMappingTests, UnmapCalledTooEarly) {
    TestEarlyMapCancelled([&]() { buffer.Unmap(); },
                          [&]() { EXPECT_CALL(api, BufferUnmap(apiBuffer)); },
                          wgpu::MapAsyncStatus::Aborted,
                          "Buffer was unmapped before mapping was resolved.", IsSpontaneous());
}

// Check that if Unmap() was called early client-side, we disregard server-side validation errors.
TEST_P(WireBufferMappingTests, UnmapCalledTooEarlyServerSideError) {
    TestEarlyMapErrorCancelled([&]() { buffer.Unmap(); },
                               [&]() { EXPECT_CALL(api, BufferUnmap(apiBuffer)); },
                               wgpu::MapAsyncStatus::Aborted,
                               "Buffer was unmapped before mapping was resolved.", IsSpontaneous());
}

// Check the map callback when the map request would have worked, but Destroy() was called.
TEST_P(WireBufferMappingTests, DestroyCalledTooEarly) {
    TestEarlyMapCancelled([&]() { buffer.Destroy(); },
                          [&]() { EXPECT_CALL(api, BufferDestroy(apiBuffer)); },
                          wgpu::MapAsyncStatus::Aborted,
                          "Buffer was destroyed before mapping was resolved.", IsSpontaneous());
}

// Check that if Destroy() was called early client-side, we disregard server-side validation errors.
TEST_P(WireBufferMappingTests, DestroyCalledTooEarlyServerSideError) {
    TestEarlyMapErrorCancelled(
        [&]() { buffer.Destroy(); }, [&]() { EXPECT_CALL(api, BufferDestroy(apiBuffer)); },
        wgpu::MapAsyncStatus::Aborted, "Buffer was destroyed before mapping was resolved.",
        IsSpontaneous());
}

// Check the map callback when the map request would have worked, but the device was released.
TEST_P(WireBufferMappingTests, DeviceReleasedTooEarly) {
    TestEarlyMapCancelled([&]() { device = nullptr; },
                          [&]() {
                              EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);
                              EXPECT_CALL(api, DeviceRelease(apiDevice));
                          },
                          wgpu::MapAsyncStatus::Aborted,
                          "The Device was lost before mapping was resolved.", false);
    DefaultApiDeviceWasReleased();
}

// Check that if device is released early client-side, we disregard server-side validation errors.
TEST_P(WireBufferMappingTests, DeviceReleasedTooEarlyServerSideError) {
    TestEarlyMapErrorCancelled(
        [&]() { device = nullptr; },
        [&]() {
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, _)).Times(1);
            EXPECT_CALL(api, DeviceRelease(apiDevice));
        },
        wgpu::MapAsyncStatus::Aborted, "The Device was lost before mapping was resolved.", false);
    DefaultApiDeviceWasReleased();
}

// Check the map callback when the map request would have worked, but the device was destroyed.
TEST_P(WireBufferMappingTests, DeviceDestroyedTooEarly) {
    TestEarlyMapCancelled(
        [&]() { device.Destroy(); }, [&]() { EXPECT_CALL(api, DeviceDestroy(apiDevice)); },
        wgpu::MapAsyncStatus::Aborted, "The Device was lost before mapping was resolved.", false);
}

// Check that if device is destroyed early client-side, we disregard server-side validation errors.
TEST_P(WireBufferMappingTests, DeviceDestroyedTooEarlyServerSideError) {
    TestEarlyMapErrorCancelled(
        [&]() { device.Destroy(); }, [&]() { EXPECT_CALL(api, DeviceDestroy(apiDevice)); },
        wgpu::MapAsyncStatus::Aborted, "The Device was lost before mapping was resolved.", false);
}

// Test that the callback isn't fired twice when Unmap() is called inside the callback.
TEST_P(WireBufferMappingTests, UnmapInsideMapCallback) {
    TestCancelInCallback([&]() { buffer.Unmap(); },
                         [&]() { EXPECT_CALL(api, BufferUnmap(apiBuffer)); });
}

// Test that the callback isn't fired twice when Destroy() is called inside the callback.
TEST_P(WireBufferMappingTests, DestroyInsideMapCallback) {
    TestCancelInCallback([&]() { buffer.Destroy(); },
                         [&]() { EXPECT_CALL(api, BufferDestroy(apiBuffer)); });
}

// Test that the callback isn't fired twice when Release() is called inside the callback with the
// last ref.
TEST_P(WireBufferMappingTests, ReleaseInsideMapCallback) {
    // TODO(dawn:1621): Suppressed because the mapping handling still touches the buffer after it is
    // destroyed triggering an ASAN error when in MapWrite mode.
    DAWN_SKIP_TEST_IF(GetMapMode() == wgpu::MapMode::Write);

    TestCancelInCallback([&]() { buffer = nullptr; },
                         [&]() { EXPECT_CALL(api, BufferRelease(apiBuffer)); });
}

// Tests specific to mapping for reading.
class WireBufferMappingReadTests : public WireBufferMappingTests {
  protected:
    void SetUp() override {
        WireBufferMappingTests::SetUp();
        SetupBuffer(wgpu::MapMode::Read);
    }
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireBufferMappingReadTests);

// Check mapping for reading a succesfully created buffer.
TEST_P(WireBufferMappingReadTests, MappingSuccess) {
    MapAsync(wgpu::MapMode::Read, 0, kBufferSize);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

        FlushCallbacks();
    });

    EXPECT_EQ(bufferContent,
              *static_cast<const uint32_t*>(buffer.GetConstMappedRange(0, kBufferSize)));
    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    buffer.Unmap();
    FlushClient();
}

// Check that an error map read while a buffer is already mapped won't changed the result of get
// mapped range.
TEST_P(WireBufferMappingReadTests, MappingErrorWhileAlreadyMapped) {
    // Successful map
    MapAsync(wgpu::MapMode::Read, 0, kBufferSize);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    EXPECT_CALL(api, BufferGetConstMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

        FlushCallbacks();
    });

    // Map failure while the buffer is already mapped
    MapAsync(wgpu::MapMode::Read, 0, kBufferSize);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Read, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Error,
                                           ToOutputStringView("Already mapped"));
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, SizedString("Already mapped")))
            .Times(1);

        FlushCallbacks();
    });

    EXPECT_EQ(bufferContent,
              *static_cast<const uint32_t*>(buffer.GetConstMappedRange(0, kBufferSize)));
}

// Tests specific to mapping for writing.
class WireBufferMappingWriteTests : public WireBufferMappingTests {
  protected:
    void SetUp() override {
        WireBufferMappingTests::SetUp();
        SetupBuffer(wgpu::MapMode::Write);
    }
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireBufferMappingWriteTests);

// Check mapping for writing a succesfully created buffer.
TEST_P(WireBufferMappingWriteTests, MappingSuccess) {
    MapAsync(wgpu::MapMode::Write, 0, kBufferSize);

    uint32_t serverBufferContent = 31337;
    uint32_t updatedContent = 4242;

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&serverBufferContent));

    // The map write callback always gets a buffer full of zeroes.
    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

        FlushCallbacks();
    });

    uint32_t* lastMapWritePointer = static_cast<uint32_t*>(buffer.GetMappedRange(0, kBufferSize));
    ASSERT_EQ(0u, *lastMapWritePointer);

    // Write something to the mapped pointer
    *lastMapWritePointer = updatedContent;

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    buffer.Unmap();

    FlushClient();

    // After the buffer is unmapped, the content of the buffer is updated on the server
    ASSERT_EQ(serverBufferContent, updatedContent);
}

// Check that an error map write while a buffer is already mapped.
TEST_P(WireBufferMappingWriteTests, MappingErrorWhileAlreadyMapped) {
    // Successful map
    MapAsync(wgpu::MapMode::Write, 0, kBufferSize);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&bufferContent));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

        FlushCallbacks();
    });

    // Map failure while the buffer is already mapped
    MapAsync(wgpu::MapMode::Write, 0, kBufferSize);
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Error,
                                           ToOutputStringView("Already mapped"));
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, SizedString("Already mapped")))
            .Times(1);

        FlushCallbacks();
    });

    EXPECT_NE(nullptr, static_cast<const uint32_t*>(buffer.GetConstMappedRange(0, kBufferSize)));
}

// Tests specific to mapped at creation.
class WireBufferMappedAtCreationTests : public WireBufferMappingTests {
  protected:
    void SetUp() override {
        WireBufferMappingTestBase::SetUp();
        apiBuffer = api.GetNewBuffer();
    }
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireBufferMappedAtCreationTests);

// Test successful buffer creation with mappedAtCreation=true
TEST_F(WireBufferMappedAtCreationTests, Success) {
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = kBufferSize;
    descriptor.mappedAtCreation = true;

    uint32_t apiBufferData = 1234;
    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    buffer = device.CreateBuffer(&descriptor);
    FlushClient();

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    buffer.Unmap();
    FlushClient();
}

// Test that releasing a buffer mapped at creation does not call Unmap
TEST_F(WireBufferMappedAtCreationTests, ReleaseBeforeUnmap) {
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = kBufferSize;
    descriptor.mappedAtCreation = true;

    uint32_t apiBufferData = 1234;
    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    buffer = device.CreateBuffer(&descriptor);
    FlushClient();

    EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
    buffer = nullptr;
    FlushClient();
}

// Test that it is valid to map a buffer after it is mapped at creation and unmapped.
TEST_P(WireBufferMappedAtCreationTests, MapSuccess) {
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = kBufferSize;
    descriptor.usage = wgpu::BufferUsage::MapWrite;
    descriptor.mappedAtCreation = true;

    uint32_t apiBufferData = 1234;
    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    buffer = device.CreateBuffer(&descriptor);
    FlushClient();

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    buffer.Unmap();
    FlushClient();

    MapAsync(wgpu::MapMode::Write, 0, kBufferSize);

    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, kBufferSize))
        .WillOnce(Return(&apiBufferData));
    FlushClient();
    FlushFutures();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

        FlushCallbacks();
    });
}

// Test that it is invalid to map a buffer after mappedAtCreation but before Unmap
TEST_P(WireBufferMappedAtCreationTests, MapFailure) {
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = kBufferSize;
    descriptor.mappedAtCreation = true;

    uint32_t apiBufferData = 1234;
    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
    EXPECT_CALL(api, BufferGetMappedRange(apiBuffer, 0, 4)).WillOnce(Return(&apiBufferData));

    buffer = device.CreateBuffer(&descriptor);
    FlushClient();

    MapAsync(wgpu::MapMode::Write, 0, kBufferSize);

    // Note that the validation logic is entirely on the native side so we inject the validation
    // error here and flush the server response to mock the expected behavior.
    EXPECT_CALL(api, OnBufferMapAsync(apiBuffer, WGPUMapMode_Write, 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Error,
                                           ToOutputStringView("Already mapped"));
        }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, SizedString("Already mapped")))
            .Times(1);

        FlushCallbacks();
    });

    EXPECT_NE(nullptr, static_cast<const uint32_t*>(buffer.GetConstMappedRange(0, kBufferSize)));

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    buffer.Unmap();

    FlushClient();
}

// Check that trying to create a buffer of size MAX_SIZE_T won't get OOM error at the client side.
TEST_F(WireBufferMappingTests, MaxSizeMappableBufferOOMDirectly) {
    size_t kOOMSize = std::numeric_limits<size_t>::max();

    // Check for mappedAtCreation.
    {
        wgpu::BufferDescriptor descriptor = {};
        descriptor.usage = wgpu::BufferUsage::CopySrc;
        descriptor.size = kOOMSize;
        descriptor.mappedAtCreation = true;

        device.CreateBuffer(&descriptor);
        FlushClient();
    }

    // Check for MapRead usage.
    {
        wgpu::BufferDescriptor descriptor = {};
        descriptor.usage = wgpu::BufferUsage::MapRead;
        descriptor.size = kOOMSize;

        device.CreateBuffer(&descriptor);
        EXPECT_CALL(api, DeviceCreateErrorBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
        EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
        FlushClient();
    }

    // Check for MapWrite usage.
    {
        wgpu::BufferDescriptor descriptor = {};
        descriptor.usage = wgpu::BufferUsage::MapWrite;
        descriptor.size = kOOMSize;

        device.CreateBuffer(&descriptor);
        EXPECT_CALL(api, DeviceCreateErrorBuffer(apiDevice, _)).WillOnce(Return(apiBuffer));
        EXPECT_CALL(api, BufferRelease(apiBuffer)).Times(1);
        FlushClient();
    }
}

// Test that registering a callback then wire disconnect calls the callback with
// InstanceDropped.
TEST_P(WireBufferMappingTests, MapThenDisconnect) {
    wgpu::MapMode mapMode = GetMapMode();
    MapAsync(mapMode, 0, kBufferSize);

    uint32_t bufferContent = 0;
    EXPECT_CALL(api,
                OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    ExpectMappedRangeCall(kBufferSize, &bufferContent);

    FlushClient();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::InstanceDropped, _)).Times(1);

        GetWireClient()->Disconnect();
    });
}

// Test that registering a callback after wire disconnect calls the callback with
// InstanceDropped.
TEST_P(WireBufferMappingTests, MapAfterDisconnect) {
    wgpu::MapMode mapMode = GetMapMode();
    GetWireClient()->Disconnect();

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::InstanceDropped, _)).Times(1);

        MapAsync(mapMode, 0, kBufferSize);
    });
}

// Test that mapping again while pending map cause an error on the callback.
TEST_P(WireBufferMappingTests, PendingMapImmediateError) {
    wgpu::MapMode mapMode = GetMapMode();
    MapAsync(mapMode, 0, kBufferSize);

    // Calls for the first successful map.
    uint32_t bufferContent = 0;
    EXPECT_CALL(api,
                OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    ExpectMappedRangeCall(kBufferSize, &bufferContent);

    if (IsSpontaneous()) {
        // In spontaneous mode, the second map on the pending immediately calls the callback.
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error,
                                     SizedString("Buffer already has an outstanding map pending.")))
                .Times(1);

            MapAsync(mapMode, 0, kBufferSize);
        });

        FlushClient();
        FlushFutures();
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

            FlushCallbacks();
        });
    } else {
        // Otherwise, the callback will fire alongside the success one when we flush the callbacks.
        MapAsync(mapMode, 0, kBufferSize);

        FlushClient();
        FlushFutures();
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error,
                                     SizedString("Buffer already has an outstanding map pending.")))
                .Times(1);
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

            FlushCallbacks();
        });
    }
}

// Test that GetMapState() returns map state as expected
TEST_P(WireBufferMappingTests, GetMapState) {
    wgpu::MapMode mapMode = GetMapMode();
    uint32_t bufferContent = 31337;

    // Server-side success case
    {
        // Map state should initially be unmapped.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Unmapped);

        EXPECT_CALL(
            api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                               kEmptyOutputStringView);
            }));
        ExpectMappedRangeCall(kBufferSize, &bufferContent);
        MapAsync(mapMode, 0, kBufferSize);

        // Map state should become pending immediately after map async call.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Pending);
        FlushClient();

        // Map state should be pending until receiving a response from server.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Pending);
        FlushFutures();

        // Map state should still be pending until the callback has been called.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Pending);
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).WillOnce([&]() {
                ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Mapped);
            });

            FlushCallbacks();
        });

        // Mapping succeeded.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Mapped);
    }

    EXPECT_CALL(api, BufferUnmap(apiBuffer)).Times(1);
    buffer.Unmap();
    FlushClient();
    ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Unmapped);

    // Server-side error case
    {
        EXPECT_CALL(
            api, OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
            .WillOnce(InvokeWithoutArgs([&] {
                api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Error,
                                               ToOutputStringView("Error"));
            }));

        // Map state should initially be unmapped.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Unmapped);
        MapAsync(mapMode, 0, kBufferSize);

        // Map state should become pending immediately after map async call.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Pending);
        FlushClient();

        // Map state should be pending until receiving a response from server.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Pending);
        FlushFutures();

        // Map state should still be pending until the callback has been called.
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Pending);
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, SizedString("Error")))
                .WillOnce(
                    [&]() { ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Unmapped); });

            FlushCallbacks();
        });

        // Mapping failed
        ASSERT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Unmapped);
    }
}

// Test that requests inside user callbacks before disconnect are called.
TEST_P(WireBufferMappingTests, MapInsideCallbackBeforeDisconnect) {
    wgpu::MapMode mapMode = GetMapMode();
    MapAsync(mapMode, 0, kBufferSize);

    uint32_t bufferContent = 0;
    EXPECT_CALL(api,
                OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    ExpectMappedRangeCall(kBufferSize, &bufferContent);

    FlushClient();

    static constexpr size_t kNumRequests = 10;
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::InstanceDropped, _))
            .Times(kNumRequests + 1)
            .WillOnce([&]() {
                for (size_t i = 0; i < kNumRequests; i++) {
                    MapAsync(mapMode, 0, kBufferSize);
                }
            })
            .WillRepeatedly(Return());

        GetWireClient()->Disconnect();
    });
}

// Test that requests inside user callbacks before buffer destroy are called.
TEST_P(WireBufferMappingTests, MapInsideCallbackBeforeDestroy) {
    wgpu::MapMode mapMode = GetMapMode();
    MapAsync(mapMode, 0, kBufferSize);

    uint32_t bufferContent = 0;
    EXPECT_CALL(api,
                OnBufferMapAsync(apiBuffer, static_cast<WGPUMapMode>(mapMode), 0, kBufferSize, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallBufferMapAsyncCallback(apiBuffer, WGPUMapAsyncStatus_Success,
                                           kEmptyOutputStringView);
        }));
    ExpectMappedRangeCall(kBufferSize, &bufferContent);

    FlushClient();
    FlushFutures();

    static constexpr size_t kNumRequests = 10;
    if (IsSpontaneous()) {
        // In spontaneous mode, when the success callback fires, the first MapAsync request
        // generated by the callback is queued, then all subsequent requests' callbacks are
        // immediately called with Error because there's already a pending map. Finally, when we
        // call Destroy, the queued request's callback is then called with Aborted.
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).WillOnce([&]() {
                for (size_t i = 0; i < kNumRequests; i++) {
                    MapAsync(mapMode, 0, kBufferSize);
                }
            });
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error,
                                     SizedString("Buffer already has an outstanding map pending.")))
                .Times(kNumRequests - 1);

            FlushCallbacks();
        });
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb,
                        Call(wgpu::MapAsyncStatus::Aborted,
                             SizedString("Buffer was destroyed before mapping was resolved.")))
                .Times(1);

            buffer.Destroy();
        });
        FlushCallbacks();
    } else {
        // In non-spontaneous modes, the first callback doesn't trigger any other immediate
        // callbacks, but internally, all but the first MapAsync call's callback is set to be ready
        // with Error because there's already a pending map. When we call Destroy, the first
        // pending request is then marked ready with Aborted. The callbacks all run when we flush
        // them.
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).WillOnce([&]() {
                for (size_t i = 0; i < kNumRequests; i++) {
                    MapAsync(mapMode, 0, kBufferSize);
                }
            });

            FlushCallbacks();
        });
        buffer.Destroy();
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error,
                                     SizedString("Buffer already has an outstanding map pending.")))
                .Times(kNumRequests - 1);
            EXPECT_CALL(mockCb,
                        Call(wgpu::MapAsyncStatus::Aborted,
                             SizedString("Buffer was destroyed before mapping was resolved.")))
                .Times(1);

            FlushCallbacks();
        });
    }
}

}  // anonymous namespace
}  // namespace dawn::wire
