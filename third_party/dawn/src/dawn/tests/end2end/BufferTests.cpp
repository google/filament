// Copyright 2017 The Dawn & Tint Authors
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

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {
namespace {

using testing::_;
using testing::MockCppCallback;
using testing::StrictMock;

using MockMapAsyncCallback = MockCppCallback<wgpu::BufferMapCallback<void>*>;
using FutureCallbackMode = wgpu::CallbackMode;

DAWN_TEST_PARAM_STRUCT(BufferMappingTestParams, FutureCallbackMode);

class BufferMappingTests : public DawnTestWithParams<BufferMappingTestParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<BufferMappingTestParams>::SetUp();
        // Wire only supports polling / spontaneous futures.
        DAWN_TEST_UNSUPPORTED_IF(UsesWire() &&
                                 GetParam().mFutureCallbackMode == wgpu::CallbackMode::WaitAnyOnly);
    }

    void MapAsyncAndWait(const wgpu::Buffer& buffer,
                         wgpu::MapMode mode,
                         size_t offset,
                         size_t size,
                         wgpu::BufferMapCallback<> cb = nullptr) {
        wgpu::Future future;

        if (cb) {
            future = buffer.MapAsync(mode, offset, size, GetParam().mFutureCallbackMode, cb);
        } else {
            EXPECT_CALL(mMockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
            future = buffer.MapAsync(mode, offset, size, GetParam().mFutureCallbackMode,
                                     mMockCb.Callback());
        }

        switch (GetParam().mFutureCallbackMode) {
            case wgpu::CallbackMode::WaitAnyOnly: {
                ASSERT_EQ(GetInstance().WaitAny(future, UINT64_MAX), wgpu::WaitStatus::Success);
                break;
            }
            case wgpu::CallbackMode::AllowProcessEvents:
            case wgpu::CallbackMode::AllowSpontaneous:
                while (GetInstance().WaitAny(future, 0) == wgpu::WaitStatus::TimedOut) {
                    // Flush wire and call instance process events.
                    WaitABit();
                }
                break;
        }
    }

    wgpu::Buffer CreateMapReadBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Buffer CreateMapWriteBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Buffer CreateUniformBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        return device.CreateBuffer(&descriptor);
    }

    StrictMock<MockMapAsyncCallback> mMockCb;
};

void CheckMapping(const void* actual, const void* expected, size_t size) {
    EXPECT_NE(actual, nullptr);
    if (actual != nullptr) {
        EXPECT_EQ(0, memcmp(actual, expected, size));
    }
}

// Test that the simplest map read works
TEST_P(BufferMappingTests, MapRead_Basic) {
    wgpu::Buffer buffer = CreateMapReadBuffer(4);

    const uint32_t myData = 0x01020304;
    constexpr size_t kSize = sizeof(myData);
    queue.WriteBuffer(buffer, 0, &myData, kSize);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, 4);
    CheckMapping(buffer.GetConstMappedRange(), &myData, kSize);
    CheckMapping(buffer.GetConstMappedRange(0, kSize), &myData, kSize);
    buffer.Unmap();
}

// Test map-reading a zero-sized buffer.
TEST_P(BufferMappingTests, MapRead_ZeroSized) {
    wgpu::Buffer buffer = CreateMapReadBuffer(0);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, wgpu::kWholeMapSize);
    ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
    buffer.Unmap();
}

// Test map-reading with a non-zero offset
TEST_P(BufferMappingTests, MapRead_NonZeroOffset) {
    uint32_t myData[3] = {0x01020304, 0x05060708, 0x090A0B0C};

    wgpu::Buffer buffer = CreateMapReadBuffer(sizeof(myData));
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 8, 4);
    ASSERT_EQ(myData[2], *static_cast<const uint32_t*>(buffer.GetConstMappedRange(8)));
    {
        uint32_t readback = 0;
        ASSERT_EQ(wgpu::Status::Success, buffer.ReadMappedRange(8, &readback, sizeof(readback)));
        EXPECT_EQ(readback, myData[2]);
    }
    buffer.Unmap();
}

// Map read and unmap twice. Test that both of these two iterations work.
TEST_P(BufferMappingTests, MapRead_Twice) {
    wgpu::Buffer buffer = CreateMapReadBuffer(4);

    uint32_t myData = 0x01020304;
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, 4);
    ASSERT_EQ(myData, *static_cast<const uint32_t*>(buffer.GetConstMappedRange()));
    buffer.Unmap();

    myData = 0x05060708;
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, 4);
    ASSERT_EQ(myData, *static_cast<const uint32_t*>(buffer.GetConstMappedRange()));
    buffer.Unmap();
}

// Map read and test multiple get mapped range data
TEST_P(BufferMappingTests, MapRead_MultipleMappedRange) {
    wgpu::Buffer buffer = CreateMapReadBuffer(12);

    uint32_t myData[] = {0x00010203, 0x04050607, 0x08090a0b};
    queue.WriteBuffer(buffer, 0, &myData, 12);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, 12);
    ASSERT_EQ(myData[0], *static_cast<const uint32_t*>(buffer.GetConstMappedRange(0)));
    ASSERT_EQ(myData[1], *(static_cast<const uint32_t*>(buffer.GetConstMappedRange(0)) + 1));
    ASSERT_EQ(myData[2], *(static_cast<const uint32_t*>(buffer.GetConstMappedRange(0)) + 2));
    ASSERT_EQ(myData[2], *static_cast<const uint32_t*>(buffer.GetConstMappedRange(8)));
    buffer.Unmap();
}

// Test map-reading a large buffer.
TEST_P(BufferMappingTests, MapRead_Large) {
    constexpr uint32_t kDataSize = 1000 * 1000;
    constexpr size_t kByteSize = kDataSize * sizeof(uint32_t);
    wgpu::Buffer buffer = CreateMapReadBuffer(kByteSize);

    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }
    queue.WriteBuffer(buffer, 0, myData.data(), kByteSize);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, kByteSize);
    EXPECT_EQ(nullptr, buffer.GetConstMappedRange(0, kByteSize + 4));
    EXPECT_EQ(0, memcmp(buffer.GetConstMappedRange(), myData.data(), kByteSize));
    EXPECT_EQ(0, memcmp(buffer.GetConstMappedRange(8), myData.data() + 2, kByteSize - 8));
    EXPECT_EQ(
        0, memcmp(buffer.GetConstMappedRange(8, kByteSize - 8), myData.data() + 2, kByteSize - 8));
    buffer.Unmap();

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 16, kByteSize - 16);
    // Size is too big.
    EXPECT_EQ(nullptr, buffer.GetConstMappedRange(16, kByteSize - 12));
    // Offset defaults to 0 which is less than 16
    EXPECT_EQ(nullptr, buffer.GetConstMappedRange());
    // Offset less than 8 is less than 16
    EXPECT_EQ(nullptr, buffer.GetConstMappedRange(8));

    // Test a couple values.
    EXPECT_EQ(0, memcmp(buffer.GetConstMappedRange(16), myData.data() + 4, kByteSize - 16));
    EXPECT_EQ(0, memcmp(buffer.GetConstMappedRange(24), myData.data() + 6, kByteSize - 24));

    buffer.Unmap();
}

// Test that GetConstMappedRange works inside map-read callback
TEST_P(BufferMappingTests, MapRead_InCallback) {
    constexpr size_t kBufferSize = 12;
    wgpu::Buffer buffer = CreateMapReadBuffer(kBufferSize);

    uint32_t myData[3] = {0x01020304, 0x05060708, 0x090A0B0C};
    static constexpr size_t kSize = sizeof(myData);
    queue.WriteBuffer(buffer, 0, &myData, kSize);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, kBufferSize,
                    [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                        ASSERT_EQ(status, wgpu::MapAsyncStatus::Success);

                        CheckMapping(buffer.GetConstMappedRange(), &myData, kSize);
                        CheckMapping(buffer.GetConstMappedRange(0, kSize), &myData, kSize);
                        CheckMapping(buffer.GetConstMappedRange(8, 4), &myData[2],
                                     sizeof(uint32_t));

                        buffer.Unmap();
                    });
}

// Test that the simplest map write works.
TEST_P(BufferMappingTests, MapWrite_Basic) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(4);

    const uint32_t myData1 = 2934875;
    {
        // GetMappedRange and GetConstMappedRange.
        MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
        ASSERT_NE(nullptr, buffer.GetMappedRange());
        ASSERT_NE(nullptr, buffer.GetConstMappedRange());
        memcpy(buffer.GetMappedRange(), &myData1, sizeof(myData1));
        buffer.Unmap();
        EXPECT_BUFFER_U32_EQ(myData1, buffer, 0);
    }
    {
        // ReadMappedRange and WriteMappedRange.
        const uint32_t myData2 = 0x0102'0304;
        MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
        uint32_t readback = 0;
        ASSERT_EQ(wgpu::Status::Success,
                  static_cast<wgpu::Status>(buffer.ReadMappedRange(0, &readback, sizeof(myData2))));
        EXPECT_EQ(readback, myData1);
        ASSERT_EQ(wgpu::Status::Success,
                  static_cast<wgpu::Status>(buffer.WriteMappedRange(0, &myData2, sizeof(myData2))));
        buffer.Unmap();
        EXPECT_BUFFER_U32_EQ(myData2, buffer, 0);
    }
}

// Test that the simplest map write works with a range.
TEST_P(BufferMappingTests, MapWrite_BasicRange) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(4);

    uint32_t myData = 2934875;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
    ASSERT_NE(nullptr, buffer.GetMappedRange(0, 4));
    ASSERT_NE(nullptr, buffer.GetConstMappedRange(0, 4));
    memcpy(buffer.GetMappedRange(), &myData, sizeof(myData));
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Test map-writing a zero-sized buffer.
TEST_P(BufferMappingTests, MapWrite_ZeroSized) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(0);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, wgpu::kWholeMapSize);
    ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
    ASSERT_NE(buffer.GetMappedRange(), nullptr);
    buffer.Unmap();
}

// Test map-writing with a non-zero offset.
TEST_P(BufferMappingTests, MapWrite_NonZeroOffset) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(20);

    uint32_t myData = 2934875;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 8, 4);
    memcpy(buffer.GetMappedRange(8, 4), &myData, sizeof(myData));
    buffer.Unmap();
    EXPECT_BUFFER_U32_EQ(myData, buffer, 8);

    {
        // WriteMappedRange with offset.
        uint32_t myData2 = 12345;
        MapAsyncAndWait(buffer, wgpu::MapMode::Write, 16, 4);
        ASSERT_EQ(wgpu::Status::Success, static_cast<wgpu::Status>(buffer.WriteMappedRange(
                                             16, &myData2, sizeof(myData2))));
        buffer.Unmap();
        EXPECT_BUFFER_U32_EQ(myData, buffer, 8);
        EXPECT_BUFFER_U32_EQ(myData2, buffer, 16);
    }
}

// Map, write and unmap twice. Test that both of these two iterations work.
TEST_P(BufferMappingTests, MapWrite_Twice) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(4);

    uint32_t myData = 2934875;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
    memcpy(buffer.GetMappedRange(), &myData, sizeof(myData));
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);

    myData = 9999999;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
    memcpy(buffer.GetMappedRange(), &myData, sizeof(myData));
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Map write and unmap twice with different ranges and make sure the first write is preserved
TEST_P(BufferMappingTests, MapWrite_TwicePreserve) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(12);

    uint32_t data1 = 0x08090a0b;
    size_t offset1 = 8;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, offset1, wgpu::kWholeMapSize);
    memcpy(buffer.GetMappedRange(offset1), &data1, sizeof(data1));
    buffer.Unmap();

    uint32_t data2 = 0x00010203;
    size_t offset2 = 0;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, offset2, wgpu::kWholeMapSize);
    memcpy(buffer.GetMappedRange(offset2), &data2, sizeof(data2));
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(data1, buffer, offset1);
    EXPECT_BUFFER_U32_EQ(data2, buffer, offset2);
}

// Map write and unmap twice with overlapping ranges and make sure data is updated correctly
TEST_P(BufferMappingTests, MapWrite_TwiceRangeOverlap) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(16);

    uint32_t data1[] = {0x01234567, 0x89abcdef};
    size_t offset1 = 8;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, offset1, 8);
    memcpy(buffer.GetMappedRange(offset1, 8), data1, 8);
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(0x00000000, buffer, 0);
    EXPECT_BUFFER_U32_EQ(0x00000000, buffer, 4);
    EXPECT_BUFFER_U32_EQ(0x01234567, buffer, 8);
    EXPECT_BUFFER_U32_EQ(0x89abcdef, buffer, 12);

    uint32_t data2[] = {0x01234567, 0x89abcdef, 0x55555555};
    size_t offset2 = 0;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, offset2, 12);
    memcpy(buffer.GetMappedRange(offset2, 12), data2, 12);
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(0x01234567, buffer, 0);
    EXPECT_BUFFER_U32_EQ(0x89abcdef, buffer, 4);
    EXPECT_BUFFER_U32_EQ(0x55555555, buffer, 8);
    EXPECT_BUFFER_U32_EQ(0x89abcdef, buffer, 12);
}

// Map write and test multiple mapped range data get updated correctly
TEST_P(BufferMappingTests, MapWrite_MultipleMappedRange) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(12);

    uint32_t data1 = 0x08090a0b;
    size_t offset1 = 8;
    uint32_t data2 = 0x00010203;
    size_t offset2 = 0;
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 12);
    memcpy(buffer.GetMappedRange(offset1), &data1, sizeof(data1));
    memcpy(buffer.GetMappedRange(offset2), &data2, sizeof(data2));
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(data1, buffer, offset1);
    EXPECT_BUFFER_U32_EQ(data2, buffer, offset2);
}

// Test mapping a large buffer.
TEST_P(BufferMappingTests, MapWrite_Large) {
    constexpr uint32_t kDataSize = 1000 * 1000;
    constexpr size_t kByteSize = kDataSize * sizeof(uint32_t);
    wgpu::Buffer buffer = CreateMapWriteBuffer(kDataSize * sizeof(uint32_t));

    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 16, kByteSize - 20);
    EXPECT_EQ(nullptr, buffer.GetMappedRange());
    EXPECT_EQ(nullptr, buffer.GetMappedRange(0));
    EXPECT_EQ(nullptr, buffer.GetMappedRange(8));
    EXPECT_EQ(nullptr, buffer.GetMappedRange(16, kByteSize - 8));
    memcpy(buffer.GetMappedRange(16, kByteSize - 20), myData.data(), kByteSize - 20);
    buffer.Unmap();
    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffer, 16, kDataSize - 5);
}

// Stress test mapping many buffers.
TEST_P(BufferMappingTests, MapWrite_ManySimultaneous) {
    constexpr uint32_t kDataSize = 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    constexpr uint32_t kBuffers = 100;
    std::array<wgpu::Buffer, kBuffers> buffers;
    uint32_t mapCompletedCount = 0;

    // Create buffers.
    wgpu::BufferDescriptor descriptor;
    descriptor.size = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    for (uint32_t i = 0; i < kBuffers; ++i) {
        buffers[i] = device.CreateBuffer(&descriptor);
    }

    std::array<wgpu::Future, kBuffers> futures;
    for (uint32_t i = 0; i < kBuffers; ++i) {
        futures[i] = buffers[i].MapAsync(
            wgpu::MapMode::Write, 0, descriptor.size, GetParam().mFutureCallbackMode,
            [&mapCompletedCount](wgpu::MapAsyncStatus status, wgpu::StringView) {
                ASSERT_EQ(wgpu::MapAsyncStatus::Success, status);
                mapCompletedCount++;
            });
    }

    switch (GetParam().mFutureCallbackMode) {
        case wgpu::CallbackMode::WaitAnyOnly: {
            std::array<wgpu::FutureWaitInfo, kBuffers> waitInfos;
            for (uint32_t i = 0; i < kBuffers; ++i) {
                waitInfos[i] = {futures[i]};
            }
            size_t count = waitInfos.size();
            wgpu::InstanceCapabilities instanceCapabilities;
            wgpu::GetInstanceCapabilities(&instanceCapabilities);
            do {
                size_t waitCount = std::min(count, instanceCapabilities.timedWaitAnyMaxCount);
                auto waitInfoStart = waitInfos.begin() + (count - waitCount);
                GetInstance().WaitAny(waitCount, &*waitInfoStart, UINT64_MAX);
                auto it = std::partition(waitInfoStart, waitInfoStart + waitCount,
                                         [](const auto& info) { return !info.completed; });
                count = std::distance(waitInfos.begin(), it);
            } while (count > 0);
            break;
        }
        case wgpu::CallbackMode::AllowProcessEvents:
        case wgpu::CallbackMode::AllowSpontaneous:
            // Wait for all mappings to complete
            while (mapCompletedCount != kBuffers) {
                WaitABit();
            }
            break;
    }

    // All buffers are mapped, write into them and unmap them all.
    for (uint32_t i = 0; i < kBuffers; ++i) {
        memcpy(buffers[i].GetMappedRange(0, descriptor.size), myData.data(), descriptor.size);
        buffers[i].Unmap();
    }

    // Check the content of the buffers.
    for (uint32_t i = 0; i < kBuffers; ++i) {
        EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffers[i], 0, kDataSize);
    }
}

// Test that the map offset isn't updated when the call is an error.
TEST_P(BufferMappingTests, OffsetNotUpdatedOnError) {
    uint32_t data[3] = {0xCA7, 0xB0A7, 0xBA7};
    wgpu::Buffer buffer = CreateMapReadBuffer(sizeof(data));
    queue.WriteBuffer(buffer, 0, data, sizeof(data));

    EXPECT_CALL(mMockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
    EXPECT_CALL(mMockCb, Call(wgpu::MapAsyncStatus::Error, _)).Times(1);

    // Map the buffer but do not wait on the result yet.
    wgpu::FutureWaitInfo f1 = {buffer.MapAsync(wgpu::MapMode::Read, 8, 4,
                                               GetParam().mFutureCallbackMode, mMockCb.Callback())};

    // Call MapAsync another time, the callback will be rejected with error status
    // and mMapOffset is not updated because the buffer is already being mapped and it doesn't
    // allow multiple MapAsync requests.
    wgpu::FutureWaitInfo f2;
    // TODO(crbug.com/42241221): Inject a validation error from the wire client, so that behavior
    // is consistent between native and wire.
    if (!UsesWire()) {
        ASSERT_DEVICE_ERROR(
            f2 = {buffer.MapAsync(wgpu::MapMode::Read, 0, 4, GetParam().mFutureCallbackMode,
                                  mMockCb.Callback())});
    } else {
        f2 = {buffer.MapAsync(wgpu::MapMode::Read, 0, 4, GetParam().mFutureCallbackMode,
                              mMockCb.Callback())};
    }

    switch (GetParam().mFutureCallbackMode) {
        case wgpu::CallbackMode::WaitAnyOnly: {
            ASSERT_EQ(GetInstance().WaitAny(1, &f1, UINT64_MAX), wgpu::WaitStatus::Success);
            ASSERT_EQ(GetInstance().WaitAny(1, &f2, UINT64_MAX), wgpu::WaitStatus::Success);
            EXPECT_TRUE(f1.completed);
            EXPECT_TRUE(f2.completed);
            break;
        }
        case wgpu::CallbackMode::AllowProcessEvents:
        case wgpu::CallbackMode::AllowSpontaneous:
            WaitForAllOperations();
            break;
    }

    // mMapOffset has not been updated so it should still be 4, which is data[1]
    ASSERT_EQ(0, memcmp(buffer.GetConstMappedRange(8), &data[2], sizeof(uint32_t)));
}

// Test that Get(Const)MappedRange work inside map-write callback.
TEST_P(BufferMappingTests, MapWrite_InCallbackDefault) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(4);

    static constexpr uint32_t myData = 2934875;
    static constexpr size_t kSize = sizeof(myData);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, kSize,
                    [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                        EXPECT_NE(nullptr, buffer.GetConstMappedRange());
                        void* ptr = buffer.GetMappedRange();
                        EXPECT_NE(nullptr, ptr);
                        if (ptr != nullptr) {
                            uint32_t data = myData;
                            memcpy(ptr, &data, kSize);
                        }
                        buffer.Unmap();
                    });

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Test that Get(Const)MappedRange with range work inside map-write callback.
TEST_P(BufferMappingTests, MapWrite_InCallbackRange) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(4);

    static constexpr uint32_t myData = 2934875;
    static constexpr size_t kSize = sizeof(myData);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, kSize,
                    [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                        EXPECT_NE(nullptr, buffer.GetConstMappedRange(0, kSize));
                        void* ptr = buffer.GetMappedRange(0, kSize);
                        EXPECT_NE(nullptr, ptr);
                        if (ptr != nullptr) {
                            uint32_t data = myData;
                            memcpy(ptr, &data, kSize);
                        }
                        buffer.Unmap();
                    });

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Regression test for crbug.com/dawn/969 where this test
// produced invalid barriers.
TEST_P(BufferMappingTests, MapWrite_ZeroSizedTwice) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(0);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, wgpu::kWholeMapSize);
    buffer.Unmap();

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, wgpu::kWholeMapSize);
}

// Regression test for crbug.com/1421170 where dropping a buffer which needs
// padding bytes initialization resulted in a use-after-free.
TEST_P(BufferMappingTests, RegressChromium1421170) {
    // Create a mappable buffer of size 7. It will be internally
    // aligned such that the padding bytes need to be zero initialized.
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 7;
    descriptor.usage = wgpu::BufferUsage::MapWrite;
    descriptor.mappedAtCreation = false;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    // Drop the buffer. The pending commands to zero initialize the
    // padding bytes should stay valid.
    buffer = nullptr;
    // Flush pending commands.
    device.Tick();
}

DAWN_INSTANTIATE_TEST_P(BufferMappingTests,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), VulkanBackend(),
                         OpenGLBackend(), OpenGLESBackend()},
                        std::initializer_list<wgpu::CallbackMode>{
                            wgpu::CallbackMode::WaitAnyOnly, wgpu::CallbackMode::AllowProcessEvents,
                            wgpu::CallbackMode::AllowSpontaneous});

class BufferMappingCallbackTests : public BufferMappingTests {
  protected:
    void SubmitCommandBuffer(wgpu::Buffer buffer) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        {
            // Record enough commands to make sure the submission cannot be completed by GPU too
            // quick.
            constexpr int kRepeatCount = 50;
            constexpr int kBufferSize = 1024 * 1024 * 10;
            wgpu::Buffer tempWriteBuffer = CreateMapWriteBuffer(kBufferSize);
            wgpu::Buffer tempReadBuffer = CreateUniformBuffer(kBufferSize);
            for (int i = 0; i < kRepeatCount; ++i) {
                encoder.CopyBufferToBuffer(tempWriteBuffer, 0, tempReadBuffer, 0,
                                           kBufferSize - 1024);
            }
        }

        if (buffer) {
            if (buffer.GetUsage() & wgpu::BufferUsage::CopyDst) {
                encoder.ClearBuffer(buffer);
            } else {
                wgpu::Buffer tempBuffer = CreateMapReadBuffer(buffer.GetSize());
                encoder.CopyBufferToBuffer(buffer, 0, tempBuffer, 0, buffer.GetSize());
            }
        }
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void WaitAll(std::vector<bool>& done, std::vector<wgpu::Future> futures) {
        if (GetParam().mFutureCallbackMode == wgpu::CallbackMode::WaitAnyOnly) {
            std::vector<wgpu::FutureWaitInfo> waitInfos;
            waitInfos.reserve(futures.size());
            for (wgpu::Future f : futures) {
                waitInfos.push_back({f});
            }
            size_t count = waitInfos.size();
            do {
                GetInstance().WaitAny(count, waitInfos.data(), UINT64_MAX);
                auto it = std::partition(waitInfos.begin(), waitInfos.end(),
                                         [](const auto& info) { return !info.completed; });
                count = std::distance(waitInfos.begin(), it);
            } while (count > 0);
        } else {
            do {
                WaitABit();
            } while (std::any_of(done.begin(), done.end(), [](bool done) { return !done; }));
        }
    }
};

TEST_P(BufferMappingCallbackTests, EmptySubmissionAndThenMap) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(4);
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, wgpu::kWholeMapSize);
    buffer.Unmap();

    std::vector<bool> done = {false, false};

    // 1. submission without using buffer.
    SubmitCommandBuffer({});
    wgpu::Future f1 = queue.OnSubmittedWorkDone(
        GetParam().mFutureCallbackMode, [&](wgpu::QueueWorkDoneStatus status) {
            ASSERT_EQ(status, wgpu::QueueWorkDoneStatus::Success);
            done[0] = true;

            // Step 2 callback should be called first, this is the second.
            const std::vector<bool> kExpected = {true, true};
            EXPECT_EQ(done, kExpected);
        });

    // 2.
    wgpu::Future f2 = buffer.MapAsync(wgpu::MapMode::Write, 0, wgpu::kWholeMapSize,
                                      GetParam().mFutureCallbackMode,
                                      [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                          ASSERT_EQ(status, wgpu::MapAsyncStatus::Success);
                                          done[1] = true;

                                          // The buffer is not used by step 1, so this callback is
                                          // called first.
                                          const std::vector<bool> kExpected = {false, true};
                                          EXPECT_EQ(done, kExpected);
                                      });

    WaitAll(done, {f1, f2});
}

// Test the spec's promise ordering guarantee that a buffer mapping promise created before a
// onSubmittedWorkDone promise must resolve in that order.
TEST_P(BufferMappingCallbackTests, MapThenWaitWorkDone) {
    wgpu::Buffer buffer = CreateMapWriteBuffer(4);
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, wgpu::kWholeMapSize);
    buffer.Unmap();

    std::vector<bool> done = {false, false};

    // 0. Submit a command buffer which uses the buffer
    SubmitCommandBuffer(buffer);

    // 1. Map the buffer.
    wgpu::Future f1 = buffer.MapAsync(wgpu::MapMode::Write, 0, wgpu::kWholeMapSize,
                                      GetParam().mFutureCallbackMode,
                                      [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                          ASSERT_EQ(status, wgpu::MapAsyncStatus::Success);
                                          done[0] = true;

                                          // This callback must be called first.
                                          const std::vector<bool> kExpected = {true, false};
                                          EXPECT_EQ(done, kExpected);
                                      });

    // 2. Wait for command completion.
    wgpu::Future f2 = queue.OnSubmittedWorkDone(
        GetParam().mFutureCallbackMode, [&](wgpu::QueueWorkDoneStatus status) {
            ASSERT_EQ(status, wgpu::QueueWorkDoneStatus::Success);
            done[1] = true;

            // The buffer mapping callback must have been called before this one.
            const std::vector<bool> kExpected = {true, true};
            EXPECT_EQ(done, kExpected);
        });

    WaitAll(done, {f1, f2});

    buffer.Unmap();
}

TEST_P(BufferMappingCallbackTests, EmptySubmissionWriteAndThenMap) {
    wgpu::Buffer buffer = CreateMapReadBuffer(4);
    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, wgpu::kWholeMapSize);
    buffer.Unmap();

    std::vector<bool> done = {false, false};

    // 1. submission without using buffer.
    SubmitCommandBuffer({});
    wgpu::Future f1 = queue.OnSubmittedWorkDone(
        GetParam().mFutureCallbackMode, [&](wgpu::QueueWorkDoneStatus status) {
            ASSERT_EQ(status, wgpu::QueueWorkDoneStatus::Success);
            done[0] = true;

            // Step 2 callback should be called first, this is the second.
            const std::vector<bool> kExpected = {true, false};
            EXPECT_EQ(done, kExpected);
        });

    int32_t data = 0x12345678;
    queue.WriteBuffer(buffer, 0, &data, sizeof(data));

    // 2.
    wgpu::Future f2 =
        buffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, GetParam().mFutureCallbackMode,
                        [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            ASSERT_EQ(status, wgpu::MapAsyncStatus::Success);
                            done[1] = true;

                            // The buffer is not used by step 1, so this callback is called first.
                            const std::vector<bool> kExpected = {true, true};
                            EXPECT_EQ(done, kExpected);
                        });

    WaitAll(done, {f1, f2});

    buffer.Unmap();
}

DAWN_INSTANTIATE_TEST_P(BufferMappingCallbackTests,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), VulkanBackend()},
                        std::initializer_list<wgpu::CallbackMode>{
                            wgpu::CallbackMode::WaitAnyOnly, wgpu::CallbackMode::AllowProcessEvents,
                            wgpu::CallbackMode::AllowSpontaneous});

class BufferMappedAtCreationTests : public DawnTest {
  protected:
    const void* MapAsyncAndWait(const wgpu::Buffer& buffer, wgpu::MapMode mode, size_t size) {
        bool done = false;
        buffer.MapAsync(mode, 0, size, wgpu::CallbackMode::AllowProcessEvents,
                        [&done](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            EXPECT_EQ(wgpu::MapAsyncStatus::Success, status);
                            done = true;
                        });

        while (!done) {
            WaitABit();
        }

        return buffer.GetConstMappedRange(0, size);
    }

    void UnmapBuffer(const wgpu::Buffer& buffer) { buffer.Unmap(); }

    wgpu::Buffer BufferMappedAtCreation(wgpu::BufferUsage usage, uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;
        descriptor.mappedAtCreation = true;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Buffer BufferMappedAtCreationWithData(wgpu::BufferUsage usage,
                                                const std::vector<uint32_t>& data) {
        size_t byteLength = data.size() * sizeof(uint32_t);
        wgpu::Buffer buffer = BufferMappedAtCreation(usage, byteLength);
        memcpy(buffer.GetMappedRange(), data.data(), byteLength);
        return buffer;
    }
};

// Test that the simplest mappedAtCreation works for MapWrite buffers.
TEST_P(BufferMappedAtCreationTests, MapWriteUsageSmall) {
    uint32_t myData = 230502;
    wgpu::Buffer buffer = BufferMappedAtCreationWithData(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(buffer);
    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Test that the simplest mappedAtCreation works for MapRead buffers.
TEST_P(BufferMappedAtCreationTests, MapReadUsageSmall) {
    uint32_t myData = 230502;
    wgpu::Buffer buffer = BufferMappedAtCreationWithData(wgpu::BufferUsage::MapRead, {myData});
    UnmapBuffer(buffer);

    const void* mappedData = MapAsyncAndWait(buffer, wgpu::MapMode::Read, 4);
    ASSERT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));
    UnmapBuffer(buffer);
}

// Test that the simplest mappedAtCreation works for non-mappable buffers.
TEST_P(BufferMappedAtCreationTests, NonMappableUsageSmall) {
    uint32_t myData = 4239;
    wgpu::Buffer buffer = BufferMappedAtCreationWithData(wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(buffer);

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Test mappedAtCreation for a large MapWrite buffer
TEST_P(BufferMappedAtCreationTests, MapWriteUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    wgpu::Buffer buffer = BufferMappedAtCreationWithData(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(buffer);

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffer, 0, kDataSize);
}

// Test mappedAtCreation for a large MapRead buffer
TEST_P(BufferMappedAtCreationTests, MapReadUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    wgpu::Buffer buffer = BufferMappedAtCreationWithData(wgpu::BufferUsage::MapRead, myData);
    UnmapBuffer(buffer);

    const void* mappedData =
        MapAsyncAndWait(buffer, wgpu::MapMode::Read, kDataSize * sizeof(uint32_t));
    ASSERT_EQ(0, memcmp(mappedData, myData.data(), kDataSize * sizeof(uint32_t)));
    UnmapBuffer(buffer);
}

// Test mappedAtCreation for a large non-mappable buffer
TEST_P(BufferMappedAtCreationTests, NonMappableUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    wgpu::Buffer buffer = BufferMappedAtCreationWithData(wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(buffer);

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffer, 0, kDataSize);
}

// Test destroying a non-mappable buffer mapped at creation.
// This is a regression test for an issue where the D3D12 backend thought the buffer was actually
// mapped and tried to unlock the heap residency (when actually the buffer was using a staging
// buffer)
TEST_P(BufferMappedAtCreationTests, DestroyNonMappableWhileMappedForCreation) {
    wgpu::Buffer buffer = BufferMappedAtCreation(wgpu::BufferUsage::CopySrc, 4);
    buffer.Destroy();
}

// Test destroying a mappable buffer mapped at creation.
TEST_P(BufferMappedAtCreationTests, DestroyMappableWhileMappedForCreation) {
    wgpu::Buffer buffer = BufferMappedAtCreation(wgpu::BufferUsage::MapRead, 4);
    buffer.Destroy();
}

// Test that mapping a buffer is valid after mappedAtCreation and Unmap
TEST_P(BufferMappedAtCreationTests, CreateThenMapSuccess) {
    static uint32_t myData = 230502;
    wgpu::Buffer buffer = BufferMappedAtCreationWithData(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(buffer);

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);

    bool done = false;
    buffer.MapAsync(wgpu::MapMode::Write, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                    [&done](wgpu::MapAsyncStatus status, wgpu::StringView) {
                        EXPECT_EQ(wgpu::MapAsyncStatus::Success, status);
                        done = true;
                    });

    while (!done) {
        WaitABit();
    }

    UnmapBuffer(buffer);
}

// Test that is is invalid to map a buffer twice when using mappedAtCreation
TEST_P(BufferMappedAtCreationTests, CreateThenMapBeforeUnmapFailure) {
    uint32_t myData = 230502;
    wgpu::Buffer buffer = BufferMappedAtCreationWithData(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, {myData});

    ASSERT_DEVICE_ERROR([&] {
        bool done = false;
        buffer.MapAsync(wgpu::MapMode::Write, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                        [&done](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            EXPECT_EQ(wgpu::MapAsyncStatus::Error, status);
                            done = true;
                        });

        while (!done) {
            WaitABit();
        }
    }());

    // mappedAtCreation is unaffected by the MapWrite error.
    UnmapBuffer(buffer);
}

// Test that creating a zero-sized buffer mapped is allowed.
TEST_P(BufferMappedAtCreationTests, ZeroSized) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::Vertex;
    descriptor.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    ASSERT_NE(nullptr, buffer.GetMappedRange());

    // Check that unmapping the buffer works too.
    UnmapBuffer(buffer);
}

// Test that creating a zero-sized mapppable buffer mapped. (it is a different code path)
TEST_P(BufferMappedAtCreationTests, ZeroSizedMappableBuffer) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::MapWrite;
    descriptor.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    ASSERT_NE(nullptr, buffer.GetMappedRange());

    // Check that unmapping the buffer works too.
    UnmapBuffer(buffer);
}

// Test that creating a zero-sized error buffer mapped. (it is a different code path)
TEST_P(BufferMappedAtCreationTests, ZeroSizedErrorBuffer) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Storage;
    descriptor.mappedAtCreation = true;
    wgpu::Buffer buffer;
    ASSERT_DEVICE_ERROR(buffer = device.CreateBuffer(&descriptor));

    ASSERT_NE(nullptr, buffer.GetMappedRange());
}

// Test the result of GetMappedRange when mapped at creation.
TEST_P(BufferMappedAtCreationTests, GetMappedRange) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    descriptor.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    ASSERT_EQ(buffer.GetMappedRange(), buffer.GetConstMappedRange());
    ASSERT_NE(buffer.GetMappedRange(), nullptr);
    buffer.Unmap();
}

// Test the result of GetMappedRange when mapped at creation for a zero-sized buffer.
TEST_P(BufferMappedAtCreationTests, GetMappedRangeZeroSized) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    descriptor.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    ASSERT_EQ(buffer.GetMappedRange(), buffer.GetConstMappedRange());
    ASSERT_NE(buffer.GetMappedRange(), nullptr);
    buffer.Unmap();
}

DAWN_INSTANTIATE_TEST(BufferMappedAtCreationTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_resource_heap_tier2"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class BufferTests : public DawnTest {};

// Test that creating a zero-buffer is allowed.
TEST_P(BufferTests, ZeroSizedBuffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 0;
    desc.usage = wgpu::BufferUsage::CopyDst;
    device.CreateBuffer(&desc);
}

// Test that creating a very large buffers fails gracefully.
TEST_P(BufferTests, CreateBufferOOM) {
    // TODO(crbug.com/346377856): fails on ANGLE/D3D11, but is likely a Dawn/GL bug that only
    // repros on Windows for some reason
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES() && IsANGLED3D11());

    // TODO(http://crbug.com/dawn/749): Missing support.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsAsan());
    DAWN_TEST_UNSUPPORTED_IF(IsTsan());

    wgpu::BufferDescriptor descriptor;
    descriptor.usage = wgpu::BufferUsage::CopyDst;

    descriptor.size = std::numeric_limits<uint64_t>::max();
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));

    // UINT64_MAX may be special cased. Test a smaller, but really large buffer also fails
    descriptor.size = 1ull << 50;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
}

// Test that in the creation of buffers validation errors should always be prior to OOM.
TEST_P(BufferTests, CreateBufferOOMWithValidationError) {
    // Fails on TSAN due to allowing too much memory. TSAN max is `0x10000000000` and the test
    // allocates `0x4000000000000000`
    DAWN_SUPPRESS_TEST_IF(IsTsan());

    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::BufferDescriptor descriptor;
    descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::Uniform;

    descriptor.size = std::numeric_limits<uint64_t>::max();
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));

    // UINT64_MAX may be special cased. Test a smaller, but really large buffer also fails
    descriptor.size = 1ull << 50;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
}

// Test that a very large buffer mappedAtCreation fails gracefully.
TEST_P(BufferTests, BufferMappedAtCreationOOM) {
    // TODO(crbug.com/346377856): fails on ANGLE/D3D11, but is likely a Dawn/GL bug that only
    // repros on Windows for some reason
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES() && IsANGLED3D11());

    // TODO(http://crbug.com/dawn/749): Missing support.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsAsan());
    DAWN_TEST_UNSUPPORTED_IF(IsTsan());

    auto Check = [&](bool nonNull, const wgpu::BufferDescriptor* desc) {
        wgpu::Buffer buffer = device.CreateBuffer(desc);
        if (nonNull) {
            ASSERT_NE(nullptr, buffer.Get());
            ASSERT_NE(nullptr, buffer.GetMappedRange());
        } else {
            ASSERT_EQ(nullptr, buffer.Get());
        }
    };

    for (wgpu::BufferUsage usage : {
             wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::None,
             wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite,
             wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
         }) {
        wgpu::BufferDescriptor descriptor;
        descriptor.usage = usage;
        descriptor.mappedAtCreation = true;

        // Control: test a small buffer works.
        descriptor.size = 4;
        Check(true, &descriptor);

        // Test an enormous buffer fails
        descriptor.size = std::numeric_limits<uint64_t>::max();
        Check(false, &descriptor);

        // UINT64_MAX may be special cased. Test a smaller, but really large buffer also fails
        descriptor.size = 1ull << 50;
        Check(false, &descriptor);
    }
}

// Test OOM cases that can't be reliably triggered, but can be simulated.
TEST_P(BufferTests, BufferMappedAtCreationOOM_Simulated) {
    auto Check = [&](bool mapError, bool deviceError, const wgpu::BufferDescriptor* desc) {
        wgpu::Buffer buffer;
        if (deviceError) {
            ASSERT_DEVICE_ERROR(buffer = device.CreateBuffer(desc));
        } else {
            buffer = device.CreateBuffer(desc);
        }
        if (mapError) {
            ASSERT_EQ(nullptr, buffer.Get());
        } else {
            ASSERT_NE(nullptr, buffer.Get());
            ASSERT_NE(nullptr, buffer.GetMappedRange());
        }
    };

    for (wgpu::BufferUsage usage : {
             wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::None,
             wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite,
             wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
         }) {
        wgpu::DawnFakeBufferOOMForTesting oomForTesting;
        wgpu::BufferDescriptor descriptor;
        descriptor.nextInChain = &oomForTesting;
        descriptor.size = 4;
        descriptor.usage = usage;
        descriptor.mappedAtCreation = true;

        // Control: test a small buffer works.
        Check(false, false, &descriptor);

        // Test OOM in the wire client before ever sending a command.
        if (UsesWire()) {
            oomForTesting = {};
            oomForTesting.fakeOOMAtWireClientMap = true;
            Check(true, false, &descriptor);
        }

        // Test OOM in Dawn Native. If using the wire, this could cause a
        // non-null client buffer to point to a null server buffer.
        oomForTesting = {};
        oomForTesting.fakeOOMAtNativeMap = true;
        Check(!UsesWire(), false, &descriptor);

        // Test OOM in Dawn Native AND in the device allocation. Similar to previous case.
        oomForTesting = {};
        oomForTesting.fakeOOMAtNativeMap = true;
        oomForTesting.fakeOOMAtDevice = true;
        Check(!UsesWire(), false, &descriptor);

        // Test OOM only in device allocation. There should be no nulls returned at either layer.
        oomForTesting = {};
        oomForTesting.fakeOOMAtDevice = true;
        Check(false, true, &descriptor);
    }
}

TEST_P(BufferTests, CreateErrorBuffer) {
    wgpu::BufferDescriptor desc{.usage = wgpu::BufferUsage::CopySrc, .size = 8};
    wgpu::Buffer buffer;

    desc.mappedAtCreation = false;
    buffer = device.CreateErrorBuffer(&desc);
    ASSERT_NE(buffer, nullptr);

    desc.mappedAtCreation = true;
    buffer = device.CreateErrorBuffer(&desc);
    ASSERT_EQ(buffer, nullptr);

    wgpu::DawnBufferDescriptorErrorInfoFromWireClient ext;
    ext.outOfMemory = true;
    desc.nextInChain = &ext;

    desc.mappedAtCreation = false;
    ASSERT_DEVICE_ERROR(buffer = device.CreateErrorBuffer(&desc));
    ASSERT_NE(buffer, nullptr);

    desc.mappedAtCreation = true;
    buffer = device.CreateErrorBuffer(&desc);
    ASSERT_EQ(buffer, nullptr);
}

// Test that mapping an OOM buffer fails gracefully
TEST_P(BufferTests, CreateBufferOOMMapAsync) {
    // TODO(crbug.com/346377856): fails on ANGLE/D3D11, but is likely a Dawn/GL bug that only
    // repros on Windows for some reason
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES() && IsANGLED3D11());

    // TODO(http://crbug.com/dawn/749): Missing support.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsAsan());
    DAWN_TEST_UNSUPPORTED_IF(IsTsan());

    auto RunTest = [this](const wgpu::BufferDescriptor& descriptor) {
        wgpu::Buffer buffer;
        ASSERT_DEVICE_ERROR(buffer = device.CreateBuffer(&descriptor));

        bool done = false;
        ASSERT_DEVICE_ERROR(buffer.MapAsync((descriptor.usage & wgpu::BufferUsage::MapRead)
                                                ? wgpu::MapMode::Read
                                                : wgpu::MapMode::Write,
                                            0, 4, wgpu::CallbackMode::AllowProcessEvents,
                                            [&done](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                                EXPECT_EQ(wgpu::MapAsyncStatus::Error, status);
                                                done = true;
                                            }));

        while (!done) {
            WaitABit();
        }
    };

    for (wgpu::BufferUsage usage : {
             wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite,
             wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
         }) {
        wgpu::BufferDescriptor descriptor;
        descriptor.usage = usage;

        // Test an enormous buffer
        descriptor.size = std::numeric_limits<uint64_t>::max();
        RunTest(descriptor);

        // UINT64_MAX may be special cased. Test a smaller, but really large buffer also fails
        descriptor.size = 1ull << 50;
        RunTest(descriptor);
    }
}

DAWN_INSTANTIATE_TEST(BufferTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class BufferNoSuballocationTests : public DawnTest {};

// Regression test for crbug.com/1313172
// This tests a buffer. It then performs writeBuffer and immediately destroys
// it. Though writeBuffer references a destroyed buffer, it should not crash.
TEST_P(BufferNoSuballocationTests, WriteBufferThenDestroy) {
    uint32_t myData = 0x01020304;

    wgpu::BufferDescriptor desc;
    desc.size = 1024;
    desc.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    // Enqueue a pending write into the buffer.
    constexpr size_t kSize = sizeof(myData);
    queue.WriteBuffer(buffer, 0, &myData, kSize);

    // Destroy the buffer.
    buffer.Destroy();

    // Flush and wait for all commands.
    queue.Submit(0, nullptr);
    WaitForAllOperations();
}

DAWN_INSTANTIATE_TEST(BufferNoSuballocationTests,
                      D3D11Backend({"disable_resource_suballocation"}),
                      D3D12Backend({"disable_resource_suballocation"}),
                      MetalBackend({"disable_resource_suballocation"}),
                      OpenGLBackend({"disable_resource_suballocation"}),
                      OpenGLESBackend({"disable_resource_suballocation"}),
                      VulkanBackend({"disable_resource_suballocation"}));

class BufferMapExtendedUsagesTests : public DawnTest {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        wgpu::Limits required = {};
        required.maxStorageBuffersInVertexStage = supported.maxStorageBuffersInVertexStage;
        required.maxStorageBuffersPerShaderStage = supported.maxStorageBuffersPerShaderStage;
        return required;
    }

  protected:
    void SetUp() override {
        DawnTest::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        // Skip all tests if the required feature is not supported.
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::BufferMapExtendedUsages}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (!UsesWire() && SupportsFeatures({wgpu::FeatureName::BufferMapExtendedUsages})) {
            requiredFeatures.push_back(wgpu::FeatureName::BufferMapExtendedUsages);
        }
        return requiredFeatures;
    }

    void MapAsyncAndWait(const wgpu::Buffer& buffer,
                         wgpu::MapMode mode,
                         size_t offset,
                         size_t size) {
        wgpu::Future future = buffer.MapAsync(mode, offset, size, wgpu::CallbackMode::WaitAnyOnly,
                                              [](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                                  ASSERT_EQ(wgpu::MapAsyncStatus::Success, status);
                                              });
        wgpu::FutureWaitInfo waitInfo = {future};
        GetInstance().WaitAny(1, &waitInfo, UINT64_MAX);
        ASSERT_TRUE(waitInfo.completed);
    }

    wgpu::Buffer CreateBufferFromData(const void* data, uint64_t size, wgpu::BufferUsage usage) {
        if (!(usage & wgpu::BufferUsage::MapWrite)) {
            return utils::CreateBufferFromData(device, data, size, usage);
        }

        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;

        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, size);
        memcpy(buffer.GetMappedRange(), data, size);
        buffer.Unmap();

        return buffer;
    }

    enum class ColorSrc {
        UniformBuffer,
        VertexBuffer,
        StorageBuffer,
    };

    wgpu::RenderPipeline CreateRenderPipelineForTest(
        ColorSrc colorSource,
        wgpu::VertexStepMode vertexBufferStepMode = wgpu::VertexStepMode::Vertex,
        wgpu::VertexFormat vertexBufferFormat = wgpu::VertexFormat::Unorm8x4) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;

        std::ostringstream vs;
        vs << R"(
            struct VertexOut {
                @location(0) color : vec4f,
                @builtin(position) position : vec4f,
            }

            const vertexPos = array(
                vec2f(-1.0, -1.0),
                vec2f( 3.0, -1.0),
                vec2f(-1.0,  3.0));
        )";

        switch (colorSource) {
            case ColorSrc::UniformBuffer:
                // Color is from uniform buffer.
                vs << R"(
                struct Uniforms {
                    color : vec4f,
                }
                @binding(0) @group(0) var<uniform> uniforms : Uniforms;

                @vertex
                fn main(@builtin(vertex_index) vertexIndex : u32) -> VertexOut {
                    var output : VertexOut;
                    output.position = vec4f(vertexPos[vertexIndex % 3], 0.0, 1.0);
                    output.color = uniforms.color;
                    return output;
                })";
                break;
            case ColorSrc::VertexBuffer:
                // Color is from vertex buffer.
                vs << R"(
                @vertex
                fn main(@location(0) vertexColor : vec4f,
                        @builtin(vertex_index) vertexIndex : u32) -> VertexOut {
                    var output : VertexOut;
                    output.position = vec4f(vertexPos[vertexIndex % 3], 0.0, 1.0);
                    output.color = vertexColor;
                    return output;
                })";

                pipelineDescriptor.vertex.bufferCount = 1;
                pipelineDescriptor.cBuffers[0].arrayStride =
                    utils::VertexFormatSize(vertexBufferFormat);
                pipelineDescriptor.cBuffers[0].attributeCount = 1;
                pipelineDescriptor.cBuffers[0].stepMode = vertexBufferStepMode;
                pipelineDescriptor.cAttributes[0].format = vertexBufferFormat;
                break;
            case ColorSrc::StorageBuffer:
                vs << R"(
                struct Uniforms {
                    color : vec4f,
                }
                @binding(0) @group(0) var<storage, read> ssbo : Uniforms;

                @vertex
                fn main(@builtin(vertex_index) vertexIndex : u32) -> VertexOut {
                    var output : VertexOut;
                    output.position = vec4f(vertexPos[vertexIndex % 3], 0.0, 1.0);
                    output.color = ssbo.color;
                    return output;
                })";
                break;
        }
        constexpr char fs[] = R"(
            @fragment
            fn main(@location(0) color : vec4f) -> @location(0) vec4f {
                return color;
            })";

        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, vs.str().c_str());
        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, fs);

        pipelineDescriptor.cFragment.targetCount = 1;
        pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
        return pipeline;
    }

    void EncodeAndSubmitRenderPassForTest(const wgpu::RenderPassDescriptor& renderPass,
                                          wgpu::RenderPipeline pipeline,
                                          wgpu::Buffer vertexBuffer,
                                          wgpu::Buffer indexBuffer,
                                          wgpu::BindGroup uniformsBindGroup,
                                          wgpu::CommandEncoder existingEncoder = nullptr) {
        wgpu::CommandEncoder commandEncoder =
            existingEncoder ? std::move(existingEncoder) : device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(pipeline);
        if (uniformsBindGroup) {
            renderPassEncoder.SetBindGroup(0, uniformsBindGroup);
        }
        if (vertexBuffer) {
            renderPassEncoder.SetVertexBuffer(0, vertexBuffer);
        }

        if (indexBuffer) {
            renderPassEncoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
            renderPassEncoder.DrawIndexed(3);
        } else {
            renderPassEncoder.Draw(3);
        }
        renderPassEncoder.End();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }

    void MixMapWriteAndGPUWriteBufferThenDraw(ColorSrc colorSrc);

    static constexpr wgpu::BufferUsage kMapExtendedUsages[] = {
        wgpu::BufferUsage::CopySrc,  wgpu::BufferUsage::CopyDst,      wgpu::BufferUsage::Index,
        wgpu::BufferUsage::Vertex,   wgpu::BufferUsage::Uniform,      wgpu::BufferUsage::Storage,
        wgpu::BufferUsage::Indirect, wgpu::BufferUsage::QueryResolve,
    };
};

// Test that the map read for any kind of buffer works
TEST_P(BufferMapExtendedUsagesTests, MapReadWithAnyUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;

    for (const auto otherUsage : kMapExtendedUsages) {
        descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst | otherUsage;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        uint32_t myData = 0x01020304;
        constexpr size_t kSize = sizeof(myData);
        queue.WriteBuffer(buffer, 0, &myData, kSize);

        MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, 4);
        CheckMapping(buffer.GetConstMappedRange(), &myData, kSize);
        CheckMapping(buffer.GetConstMappedRange(0, kSize), &myData, kSize);
        buffer.Unmap();
    }
}

// Test that the map write for any kind of buffer works
TEST_P(BufferMapExtendedUsagesTests, MapWriteWithAnyUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;

    for (const auto otherUsage : kMapExtendedUsages) {
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc | otherUsage;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        uint32_t myData = 2934875;
        MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
        ASSERT_NE(nullptr, buffer.GetMappedRange());
        ASSERT_NE(nullptr, buffer.GetConstMappedRange());
        memcpy(buffer.GetMappedRange(), &myData, sizeof(myData));
        buffer.Unmap();

        EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
    }
}

// Test that mapping a vertex buffer, modifying the data then draw with the buffer works.
TEST_P(BufferMapExtendedUsagesTests, MapWriteVertexBufferAndDraw) {
    const utils::RGBA8 kReds[] = {utils::RGBA8::kRed, utils::RGBA8::kRed, utils::RGBA8::kRed};
    const utils::RGBA8 kGreens[] = {utils::RGBA8::kGreen, utils::RGBA8::kGreen,
                                    utils::RGBA8::kGreen};

    // Create buffer with initial red color data.
    wgpu::Buffer vertexBuffer = CreateBufferFromData(
        kReds, sizeof(kReds), wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Vertex);

    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTest(ColorSrc::VertexBuffer);

    auto redRenderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto greenRenderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // First render pass: draw with red color vertex buffer.
    EncodeAndSubmitRenderPassForTest(redRenderPass.renderPassInfo, renderPipeline, vertexBuffer,
                                     nullptr, nullptr);

    // Second render pass: draw with green color vertex buffer.
    MapAsyncAndWait(vertexBuffer, wgpu::MapMode::Write, 0, sizeof(kGreens));
    ASSERT_NE(nullptr, vertexBuffer.GetMappedRange());
    memcpy(vertexBuffer.GetMappedRange(), kGreens, sizeof(kGreens));
    vertexBuffer.Unmap();

    EncodeAndSubmitRenderPassForTest(greenRenderPass.renderPassInfo, renderPipeline, vertexBuffer,
                                     nullptr, nullptr);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, redRenderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, greenRenderPass.color, 0, 0);
}

// Test that mapping a index buffer, modifying the data then draw with the buffer works.
TEST_P(BufferMapExtendedUsagesTests, MapWriteIndexBufferAndDraw) {
    const utils::RGBA8 kVertexColors[] = {
        utils::RGBA8::kRed,   utils::RGBA8::kRed,   utils::RGBA8::kRed,
        utils::RGBA8::kGreen, utils::RGBA8::kGreen, utils::RGBA8::kGreen,
    };
    // Last index is unused. It is only to make sure the index buffer's size is multiple of 4.
    const uint16_t kRedIndices[] = {0, 1, 2, 0};
    const uint16_t kGreenIndices[] = {3, 4, 5, 3};

    wgpu::Buffer vertexBuffer =
        CreateBufferFromData(kVertexColors, sizeof(kVertexColors), wgpu::BufferUsage::Vertex);
    wgpu::Buffer indexBuffer = CreateBufferFromData(
        kRedIndices, sizeof(kRedIndices), wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Index);

    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTest(ColorSrc::VertexBuffer);

    auto redRenderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto greenRenderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // First render pass: draw with red color index buffer.
    EncodeAndSubmitRenderPassForTest(redRenderPass.renderPassInfo, renderPipeline, vertexBuffer,
                                     indexBuffer, nullptr);

    // Second render pass: draw with green color index buffer.
    MapAsyncAndWait(indexBuffer, wgpu::MapMode::Write, 0, sizeof(kGreenIndices));
    ASSERT_NE(nullptr, indexBuffer.GetMappedRange());
    memcpy(indexBuffer.GetMappedRange(), kGreenIndices, sizeof(kGreenIndices));
    indexBuffer.Unmap();

    EncodeAndSubmitRenderPassForTest(greenRenderPass.renderPassInfo, renderPipeline, vertexBuffer,
                                     indexBuffer, nullptr);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, redRenderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, greenRenderPass.color, 0, 0);
}

// Test that mapping an occlusion QueryResolve buffer then draw with the buffer then mapping again
// works.
TEST_P(BufferMapExtendedUsagesTests, MapWriteQueryBufferThenDrawThenMapWrite) {
    constexpr uint64_t kExpectedVal2 = 1;
    constexpr uint64_t kExpectedVal3 = 2;
    constexpr size_t kQueryResolveBufferSize = 3 * sizeof(uint64_t);
    const utils::RGBA8 kReds[] = {utils::RGBA8::kRed, utils::RGBA8::kRed, utils::RGBA8::kRed};

    // Create buffer with initial red color data.
    wgpu::Buffer vertexBuffer =
        CreateBufferFromData(kReds, sizeof(kReds), wgpu::BufferUsage::Vertex);

    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTest(ColorSrc::VertexBuffer);

    // Create Occlusion Query Set
    wgpu::QuerySet querySet;
    {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = 1;
        descriptor.type = wgpu::QueryType::Occlusion;
        querySet = device.CreateQuerySet(&descriptor);
    }

    // Create QueryResolve buffer with 2nd expected value written to 2nd uint64_t element.
    wgpu::Buffer queryBuffer;
    {
        constexpr uint64_t kInitialData[] = {0, kExpectedVal2, 0};
        queryBuffer =
            CreateBufferFromData(kInitialData, sizeof(kInitialData),
                                 wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::MapWrite |
                                     wgpu::BufferUsage::CopySrc);
    }

    // Draw with occlusion query resolved to 1st uint64_t element.
    {
        auto renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        renderPass.renderPassInfo.occlusionQuerySet = querySet;
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder =
            commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
        renderPassEncoder.SetPipeline(renderPipeline);
        renderPassEncoder.SetVertexBuffer(0, vertexBuffer);
        renderPassEncoder.BeginOcclusionQuery(0);
        renderPassEncoder.Draw(3);
        renderPassEncoder.EndOcclusionQuery();
        renderPassEncoder.End();

        commandEncoder.ResolveQuerySet(querySet, 0, 1, queryBuffer, 0);

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }

    // Map write to 3rd uint64_t element
    {
        constexpr size_t k3rdElemOffset = 2 * sizeof(uint64_t);
        MapAsyncAndWait(queryBuffer, wgpu::MapMode::Write, 0, kQueryResolveBufferSize);
        ASSERT_NE(nullptr, queryBuffer.GetMappedRange());
        memcpy(queryBuffer.GetMappedRange(k3rdElemOffset), &kExpectedVal3, sizeof(kExpectedVal3));
        queryBuffer.Unmap();
    }

    class NonZeroExpectation : public detail::Expectation {
      public:
        testing::AssertionResult Check(const void* data, size_t size) override {
            DAWN_ASSERT(size % sizeof(uint64_t) == 0);
            const uint64_t* actual = static_cast<const uint64_t*>(data);

            if (actual[0] == 0) {
                return testing::AssertionFailure() << "Expected data[0] to be non-zero.\n";
            }

            return testing::AssertionSuccess();
        }
    };

    EXPECT_BUFFER(queryBuffer, 0, sizeof(uint64_t), new NonZeroExpectation());
    EXPECT_BUFFER_U64_EQ(kExpectedVal2, queryBuffer, sizeof(uint64_t));
    EXPECT_BUFFER_U64_EQ(kExpectedVal3, queryBuffer, 2 * sizeof(uint64_t));
}

// Test that mapping a uniform buffer, modifying the data then draw with the buffer works.
TEST_P(BufferMapExtendedUsagesTests, MapWriteUniformBufferAndDraw) {
    const float kRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
    const float kGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};

    // Create buffer with initial red color data.
    wgpu::Buffer uniformBuffer = CreateBufferFromData(
        &kRed, sizeof(kRed), wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Uniform);

    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTest(ColorSrc::UniformBuffer);
    wgpu::BindGroup uniformsBindGroup = utils::MakeBindGroup(
        device, renderPipeline.GetBindGroupLayout(0), {{0, uniformBuffer, 0, sizeof(kRed)}});

    auto redRenderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto greenRenderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // First render pass: draw with red color uniform buffer.
    EncodeAndSubmitRenderPassForTest(redRenderPass.renderPassInfo, renderPipeline, nullptr, nullptr,
                                     uniformsBindGroup);

    // Second render pass: draw with green color uniform buffer.
    MapAsyncAndWait(uniformBuffer, wgpu::MapMode::Write, 0, sizeof(kGreen));
    ASSERT_NE(nullptr, uniformBuffer.GetMappedRange());
    memcpy(uniformBuffer.GetMappedRange(), &kGreen, sizeof(kGreen));
    uniformBuffer.Unmap();

    EncodeAndSubmitRenderPassForTest(greenRenderPass.renderPassInfo, renderPipeline, nullptr,
                                     nullptr, uniformsBindGroup);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, redRenderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, greenRenderPass.color, 0, 0);
}

// Test that mapping a storage buffer, modifying the data then draw with the buffer works.
TEST_P(BufferMapExtendedUsagesTests, MapWriteStorageBufferAndDraw) {
    const float kRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
    const float kGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};

    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 1);

    // Create buffer with initial red color data.
    wgpu::Buffer storageBuffer = CreateBufferFromData(
        &kRed, sizeof(kRed), wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Storage);

    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTest(ColorSrc::StorageBuffer);
    wgpu::BindGroup uniformsBindGroup = utils::MakeBindGroup(
        device, renderPipeline.GetBindGroupLayout(0), {{0, storageBuffer, 0, sizeof(kRed)}});

    auto redRenderPass = utils::CreateBasicRenderPass(device, 1, 1);
    auto greenRenderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // First render pass: draw with red color uniform buffer.
    EncodeAndSubmitRenderPassForTest(redRenderPass.renderPassInfo, renderPipeline, nullptr, nullptr,
                                     uniformsBindGroup);

    // Second render pass: draw with green color uniform buffer.
    MapAsyncAndWait(storageBuffer, wgpu::MapMode::Write, 0, sizeof(kGreen));
    ASSERT_NE(nullptr, storageBuffer.GetMappedRange());
    memcpy(storageBuffer.GetMappedRange(), &kGreen, sizeof(kGreen));
    storageBuffer.Unmap();

    EncodeAndSubmitRenderPassForTest(greenRenderPass.renderPassInfo, renderPipeline, nullptr,
                                     nullptr, uniformsBindGroup);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, redRenderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, greenRenderPass.color, 0, 0);
}

// Test that map write a storage buffer, modifying it on GPU, then map read it on CPU works.
TEST_P(BufferMapExtendedUsagesTests, MapWriteThenGPUWriteStorageBufferThenMapRead) {
    const uint32_t kInitialValue = 1;
    const uint32_t kExpectedValue = 2;
    constexpr size_t kSize = sizeof(kExpectedValue);

    wgpu::ComputePipeline pipeline;
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            struct SSBO {
                value : u32
            }
            @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

            @compute @workgroup_size(1) fn main() {
                ssbo.value += 1u;
            })");

        pipeline = device.CreateComputePipeline(&csDesc);
    }

    // Create buffer and write initial value.
    wgpu::Buffer ssbo;
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = kSize;

        descriptor.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
        ssbo = device.CreateBuffer(&descriptor);

        MapAsyncAndWait(ssbo, wgpu::MapMode::Write, 0, 4);
        ASSERT_NE(nullptr, ssbo.GetMappedRange());
        memcpy(ssbo.GetMappedRange(), &kInitialValue, sizeof(kInitialValue));
        ssbo.Unmap();
    }

    // Modify the buffer's value in compute shader.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        ASSERT_NE(nullptr, pipeline.Get());
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {
                                                             {0, ssbo, 0, kSize},
                                                         });
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Read the modified value.
    MapAsyncAndWait(ssbo, wgpu::MapMode::Read, 0, 4);
    CheckMapping(ssbo.GetConstMappedRange(0, kSize), &kExpectedValue, kSize);
    ssbo.Unmap();
}

// Test the follow scenario:
// - map write a buffer
// - modifying it on GPU.
// - map write it again.
// - draw using the buffer.
void BufferMapExtendedUsagesTests::MixMapWriteAndGPUWriteBufferThenDraw(ColorSrc colorSrc) {
    const float kRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
    const float kFinalColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr size_t kSize = sizeof(kFinalColor);

    // Create buffer with initial red color data.
    wgpu::Buffer ssbo;
    {
        wgpu::BufferUsage usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;

        switch (colorSrc) {
            case ColorSrc::UniformBuffer:
                usage |= wgpu::BufferUsage::Uniform;
                break;
            case ColorSrc::VertexBuffer:
                usage |= wgpu::BufferUsage::Vertex;
                break;
            case ColorSrc::StorageBuffer:
                // already include
                break;
        }
        ssbo = CreateBufferFromData(&kRed, kSize, usage);
    }

    // Compute pipeline
    wgpu::ComputePipeline pipeline;
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var<storage, read_write> ssbo : vec4f;
            @compute @workgroup_size(1) fn main() {
                ssbo.g = 1.0;
            })");

        pipeline = device.CreateComputePipeline(&csDesc);
    }

    // Modify the buffer's green channel in compute shader.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        ASSERT_NE(nullptr, pipeline.Get());
        wgpu::BindGroup ssboWritebindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, ssbo, 0, kSize},
                                 });
        pass.SetBindGroup(0, ssboWritebindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // MapWrite and modify the buffer's blue channel
    {
        const float kOne = 1.0f;
        constexpr size_t kBlueChannelOffset = 2 * sizeof(float);
        MapAsyncAndWait(ssbo, wgpu::MapMode::Write, 0, kSize);
        ASSERT_NE(nullptr, ssbo.GetMappedRange());
        memcpy(ssbo.GetMappedRange(kBlueChannelOffset), &kOne, sizeof(kOne));
        ssbo.Unmap();
    }

    // Draw using the color from the buffer.
    {
        // Render pipeline
        wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTest(
            colorSrc, wgpu::VertexStepMode::Instance, wgpu::VertexFormat::Float32x4);

        wgpu::Buffer vertexBuffer;
        wgpu::BindGroup ssboReadBindGroup;
        switch (colorSrc) {
            case ColorSrc::VertexBuffer:
                vertexBuffer = ssbo;
                break;
            case ColorSrc::UniformBuffer:
            case ColorSrc::StorageBuffer:
                ssboReadBindGroup = utils::MakeBindGroup(
                    device, renderPipeline.GetBindGroupLayout(0), {{0, ssbo, 0, kSize}});
                break;
        }

        auto finalRenderPass = utils::CreateBasicRenderPass(device, 1, 1);
        EncodeAndSubmitRenderPassForTest(finalRenderPass.renderPassInfo, renderPipeline,
                                         vertexBuffer, nullptr, ssboReadBindGroup);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kWhite, finalRenderPass.color, 0, 0);
    }

    // Read the final value.
    MapAsyncAndWait(ssbo, wgpu::MapMode::Read, 0, kSize);
    CheckMapping(ssbo.GetConstMappedRange(0, kSize), &kFinalColor, kSize);
    ssbo.Unmap();
}

TEST_P(BufferMapExtendedUsagesTests, MixMapWriteAndGPUWriteVertexBufferThenDraw) {
    MixMapWriteAndGPUWriteBufferThenDraw(ColorSrc::VertexBuffer);
}

TEST_P(BufferMapExtendedUsagesTests, MixMapWriteAndGPUWriteUniformBufferThenDraw) {
    MixMapWriteAndGPUWriteBufferThenDraw(ColorSrc::UniformBuffer);
}

TEST_P(BufferMapExtendedUsagesTests, MixMapWriteAndGPUWriteStorageBufferThenDraw) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 1);
    MixMapWriteAndGPUWriteBufferThenDraw(ColorSrc::StorageBuffer);
}

// Test the follow scenario:
// - map write a storage buffer
// - modifying it on GPU.
// - copy another buffer to the storage buffer.
// - draw using the storage buffer.
TEST_P(BufferMapExtendedUsagesTests,
       MapWriteThenGPUWriteStorageBufferThenCopyFromAnotherBufferThenDraw) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageBuffersInVertexStage < 1);

    const float kRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
    const float kBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
    const float kFinalColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr size_t kSize = sizeof(kFinalColor);

    // Create buffer with initial red color data.
    wgpu::Buffer ssbo;
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = kSize;

        descriptor.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopyDst;
        ssbo = device.CreateBuffer(&descriptor);

        MapAsyncAndWait(ssbo, wgpu::MapMode::Write, 0, kSize);
        ASSERT_NE(nullptr, ssbo.GetMappedRange());
        memcpy(ssbo.GetMappedRange(), &kRed, kSize);
        ssbo.Unmap();
    }
    // Create buffer with blue color data.
    wgpu::Buffer blueBuffer =
        utils::CreateBufferFromData(device, &kBlue, sizeof(kBlue), wgpu::BufferUsage::CopySrc);

    // Compute pipeline
    wgpu::ComputePipeline pipeline;
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var<storage, read_write> ssbo : vec4f;
            @compute @workgroup_size(1) fn main() {
                ssbo.g = 1.0;
            })");

        pipeline = device.CreateComputePipeline(&csDesc);
    }

    // Modify the buffer's green channel in compute shader.

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        ASSERT_NE(nullptr, pipeline.Get());
        wgpu::BindGroup ssboWritebindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, ssbo, 0, kSize},
                                 });
        pass.SetBindGroup(0, ssboWritebindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();
    }

    // Modify the buffer's blue channel with CopyB2B
    {
        constexpr size_t kBlueChannelOffset = 2 * sizeof(float);
        encoder.CopyBufferToBuffer(blueBuffer, kBlueChannelOffset, ssbo, kBlueChannelOffset,
                                   sizeof(float));
    }

    // Draw using the color from the buffer.
    {
        // Render pipeline
        wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTest(ColorSrc::StorageBuffer);
        wgpu::BindGroup ssboReadBindGroup = utils::MakeBindGroup(
            device, renderPipeline.GetBindGroupLayout(0), {{0, ssbo, 0, kSize}});

        auto finalRenderPass = utils::CreateBasicRenderPass(device, 1, 1);
        EncodeAndSubmitRenderPassForTest(finalRenderPass.renderPassInfo, renderPipeline, nullptr,
                                         nullptr, ssboReadBindGroup, std::move(encoder));

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kWhite, finalRenderPass.color, 0, 0);
    }
}

DAWN_INSTANTIATE_TEST(BufferMapExtendedUsagesTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
