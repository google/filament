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

#include <limits>
#include <memory>
#include <vector>

#include "dawn/common/Platform.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "gmock/gmock.h"

using testing::_;
using testing::HasSubstr;
using testing::Invoke;
using testing::MockCppCallback;
using testing::TestParamInfo;
using testing::Values;
using testing::WithParamInterface;

using MockMapAsyncCallback = MockCppCallback<void (*)(wgpu::MapAsyncStatus, wgpu::StringView)>;

class BufferValidationTest : public ValidationTest {
  protected:
    wgpu::Buffer CreateMapReadBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::MapRead;

        return device.CreateBuffer(&descriptor);
    }

    wgpu::Buffer CreateMapWriteBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::MapWrite;

        return device.CreateBuffer(&descriptor);
    }

    wgpu::Buffer BufferMappedAtCreation(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;
        descriptor.mappedAtCreation = true;

        return device.CreateBuffer(&descriptor);
    }

    wgpu::Queue queue;

    void SetUp() override {
        ValidationTest::SetUp();
        queue = device.GetQueue();
    }
};

// Test case where creation should succeed
TEST_F(BufferValidationTest, CreationSuccess) {
    // Success
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::Uniform;

        device.CreateBuffer(&descriptor);
    }
}

// Test case where creation should succeed
TEST_F(BufferValidationTest, CreationMaxBufferSize) {
    // Success when at limit
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = GetSupportedLimits().limits.maxBufferSize;
        descriptor.usage = wgpu::BufferUsage::Uniform;

        device.CreateBuffer(&descriptor);
    }
    // Error once it is pass the (default) limit on the device. (Note that MaxLimitTests tests at
    // max possible limit given the adapters.)
    {
        wgpu::BufferDescriptor descriptor;
        ASSERT_TRUE(GetSupportedLimits().limits.maxBufferSize <
                    std::numeric_limits<uint32_t>::max());
        descriptor.size = GetSupportedLimits().limits.maxBufferSize + 1;
        descriptor.usage = wgpu::BufferUsage::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }
}

// Test restriction on usages must not be None (0)
TEST_F(BufferValidationTest, CreationMapUsageNotZero) {
    // Zero (None) usage is an error
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::None;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }
}

// Test restriction on usages allowed with MapRead and MapWrite
TEST_F(BufferValidationTest, CreationMapUsageRestrictions) {
    // MapRead with CopyDst is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

        device.CreateBuffer(&descriptor);
    }

    // MapRead with something else is an error
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }

    // MapWrite with CopySrc is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;

        device.CreateBuffer(&descriptor);
    }

    // MapWrite with something else is an error
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }
}

class BufferMappingValidationTest : public BufferValidationTest,
                                    public WithParamInterface<wgpu::MapMode> {
  protected:
    wgpu::Buffer CreateBuffer(uint64_t size) {
        switch (GetParam()) {
            case wgpu::MapMode::Read:
                return CreateMapReadBuffer(size);
            case wgpu::MapMode::Write:
                return CreateMapWriteBuffer(size);
            default:
                DAWN_UNREACHABLE();
        }
        DAWN_UNREACHABLE();
    }

    wgpu::Buffer CreateMappedAtCreationBuffer(uint64_t size) {
        switch (GetParam()) {
            case wgpu::MapMode::Read:
                return BufferMappedAtCreation(size, wgpu::BufferUsage::MapRead);
            case wgpu::MapMode::Write:
                return BufferMappedAtCreation(size, wgpu::BufferUsage::MapWrite);
            default:
                DAWN_UNREACHABLE();
        }
        DAWN_UNREACHABLE();
    }

    void AssertMapAsyncError(wgpu::Buffer buffer,
                             wgpu::MapMode mode,
                             size_t offset,
                             size_t size,
                             bool deviceError = true) {
        // We use a new mock callback here so that the validation on the call happens as soon as the
        // scope of this call ends. This is possible since we are using Spontaneous mode.
        MockMapAsyncCallback mockCb;

        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, _)).Times(1);
        if (deviceError) {
            ASSERT_DEVICE_ERROR(buffer.MapAsync(
                mode, offset, size, wgpu::CallbackMode::AllowSpontaneous, mockCb.Callback()));
        } else {
            buffer.MapAsync(mode, offset, size, wgpu::CallbackMode::AllowSpontaneous,
                            mockCb.Callback());
        }
    }
};

INSTANTIATE_TEST_SUITE_P(,
                         BufferMappingValidationTest,
                         testing::Values(wgpu::MapMode::Read, wgpu::MapMode::Write),
                         [](const TestParamInfo<BufferMappingValidationTest::ParamType>& info) {
                             switch (info.param) {
                                 case wgpu::MapMode::Read:
                                     return "Read";
                                 case wgpu::MapMode::Write:
                                     return "Write";
                                 default:
                                     DAWN_UNREACHABLE();
                             }
                             DAWN_UNREACHABLE();
                         });

// Test the success case for mapping buffer.
TEST_P(BufferMappingValidationTest, MapAsync_Success) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    WaitForAllOperations();
    buffer.Unmap();
}

// Test map async with a buffer that's an error
TEST_P(BufferMappingValidationTest, MapAsync_ErrorBuffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer;
    ASSERT_DEVICE_ERROR(buffer = device.CreateBuffer(&desc));

    AssertMapAsyncError(buffer, GetParam(), 0, 4);
}

// Test map async with an invalid offset and size alignment.
TEST_P(BufferMappingValidationTest, MapAsync_OffsetSizeAlignment) {
    // Control case, offset aligned to 8 and size to 4 is valid
    {
        wgpu::Buffer buffer = CreateBuffer(12);

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 8, 4, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
    }

    // Error case, offset aligned to 4 is an error.
    {
        wgpu::Buffer buffer = CreateBuffer(12);
        AssertMapAsyncError(buffer, GetParam(), 4, 4);
    }

    // Error case, size aligned to 2 is an error.
    {
        wgpu::Buffer buffer = CreateBuffer(8);
        AssertMapAsyncError(buffer, GetParam(), 0, 6);
    }
}

// Test map async with an invalid offset and size OOB checks
TEST_P(BufferMappingValidationTest, MapAsync_OffsetSizeOOB) {
    // Valid case: full buffer is ok.
    {
        wgpu::Buffer buffer = CreateBuffer(8);

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 0, 8, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
    }
    // Valid case: range in the middle of the buffer is ok.
    {
        wgpu::Buffer buffer = CreateBuffer(16);

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 8, 4, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
    }
    // Valid case: empty range at the end of the buffer is ok.
    {
        wgpu::Buffer buffer = CreateBuffer(8);

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 8, 0, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
    }

    // Error case, offset is larger than the buffer size (even if size is 0).
    {
        wgpu::Buffer buffer = CreateBuffer(12);
        AssertMapAsyncError(buffer, GetParam(), 16, 0);
    }
    // Error case, offset + size is larger than the buffer
    {
        wgpu::Buffer buffer = CreateBuffer(12);
        AssertMapAsyncError(buffer, GetParam(), 8, 8);
    }
    // Error case, offset + size is larger than the buffer, overflow case.
    {
        wgpu::Buffer buffer = CreateBuffer(12);
        AssertMapAsyncError(buffer, GetParam(), 8, std::numeric_limits<size_t>::max() & ~size_t(7));
    }
}

// Test map async with a buffer that has the wrong usage
TEST_P(BufferMappingValidationTest, MapAsync_WrongUsage) {
    {
        wgpu::BufferDescriptor desc;
        desc.usage = wgpu::BufferUsage::Vertex;
        desc.size = 4;
        wgpu::Buffer buffer = device.CreateBuffer(&desc);

        AssertMapAsyncError(buffer, GetParam(), 0, 4);
    }
    {
        wgpu::Buffer buffer =
            GetParam() == wgpu::MapMode::Read ? CreateMapWriteBuffer(4) : CreateMapReadBuffer(4);
        AssertMapAsyncError(buffer, GetParam(), 0, 4);
    }
}

// Test map async with a wrong mode
TEST_P(BufferMappingValidationTest, MapAsync_WrongMode) {
    {
        wgpu::Buffer buffer = CreateBuffer(4);
        AssertMapAsyncError(buffer, wgpu::MapMode::None, 0, 4);
    }
    {
        wgpu::Buffer buffer = CreateBuffer(4);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read | wgpu::MapMode::Write, 0, 4);
    }
}

// Test map async with a buffer that's already mapped
TEST_P(BufferMappingValidationTest, MapAsync_AlreadyMapped) {
    {
        wgpu::Buffer buffer = CreateBuffer(4);

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();

        AssertMapAsyncError(buffer, GetParam(), 0, 4);
    }
    {
        wgpu::Buffer buffer = CreateMappedAtCreationBuffer(4);
        AssertMapAsyncError(buffer, GetParam(), 0, 4);
    }
}

// Test MapAsync() immediately causes a pending map error
TEST_P(BufferMappingValidationTest, MapAsync_PendingMap) {
    // Note that in the wire, we currently don't generate a validation error while in native we do.
    // If eventually we add a way to inject errors on the wire, we may be able to make this behavior
    // more aligned.
    bool validationError = !UsesWire();

    // Overlapping range
    {
        wgpu::Buffer buffer = CreateBuffer(4);

        // The first map async call should succeed while the second one should fail
        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());

        AssertMapAsyncError(buffer, GetParam(), 0, 4, validationError);
        WaitForAllOperations();
    }

    // Non-overlapping range
    {
        wgpu::Buffer buffer = CreateBuffer(16);

        // The first map async call should succeed while the second one should fail
        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 0, 8, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());

        AssertMapAsyncError(buffer, GetParam(), 8, 8, validationError);
        WaitForAllOperations();
    }
}

// Test map async with a buffer that's destroyed
TEST_P(BufferMappingValidationTest, MapAsync_Destroy) {
    wgpu::Buffer buffer = CreateBuffer(4);
    buffer.Destroy();
    AssertMapAsyncError(buffer, GetParam(), 0, 4);
}

// Test map async but unmapping before the result is ready.
TEST_P(BufferMappingValidationTest, MapAsync_UnmapBeforeResult) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Aborted, HasSubstr("unmapped"))).Times(1);

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    buffer.Unmap();
    WaitForAllOperations();
}

// When a MapAsync is cancelled with Unmap it might still be in flight, test doing a new request
// works as expected and we don't get the cancelled request's data.
TEST_P(BufferMappingValidationTest, MapAsync_UnmapBeforeResultAndMapAgain) {
    wgpu::Buffer buffer = CreateBuffer(16);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Aborted, HasSubstr("unmapped"))).Times(1);
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);

    buffer.MapAsync(GetParam(), 0, 8, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    buffer.Unmap();
    buffer.MapAsync(GetParam(), 8, 8, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());

    WaitForAllOperations();

    // Check that only the second MapAsync had an effect
    ASSERT_EQ(nullptr, buffer.GetConstMappedRange(0));
    ASSERT_NE(nullptr, buffer.GetConstMappedRange(8));
}

// Test map async but destroying before the result is ready.
TEST_P(BufferMappingValidationTest, MapAsync_DestroyBeforeResult) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Aborted, HasSubstr("destroyed"))).Times(1);

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    buffer.Destroy();
    WaitForAllOperations();
}

// Test map async but dropping the last reference before the result is ready.
TEST_P(BufferMappingValidationTest, MapAsync_DroppedBeforeResult) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Aborted, HasSubstr("destroyed"))).Times(1);

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    buffer = nullptr;
    WaitForAllOperations();
}

// Test that the MapCallback isn't fired twice when unmap() is called inside the callback
TEST_P(BufferMappingValidationTest, MapAsync_UnmapCalledInCallback) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).WillOnce(Invoke([&] {
        buffer.Unmap();
    }));

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    WaitForAllOperations();
}

// Test that the MapCallback isn't fired twice when destroy() is called inside the callback
TEST_P(BufferMappingValidationTest, MapAsync_DestroyCalledInCallback) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).WillOnce(Invoke([&] {
        buffer.Destroy();
    }));

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    WaitForAllOperations();
}

// Test MapAsync call in MapAsync success callback
TEST_P(BufferMappingValidationTest, MapAsync_RetryInSuccessCallback) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).WillOnce(Invoke([&] {
        // MapAsync call on destroyed buffer should be invalid
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, HasSubstr("already mapped")))
            .Times(1);
        ASSERT_DEVICE_ERROR(buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowSpontaneous,
                                            mockCb.Callback()));
    }));

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    WaitForAllOperations();
}

// Test MapAsync call in MapAsync validation error callback
TEST_P(BufferMappingValidationTest, MapAsync_RetryInErrorCallback) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, _)).WillOnce(Invoke([&] {
        // Retry with valid parameter and it should succeed
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
    }));

    // Wrong map mode on buffer is invalid and it should reject with validation error
    ASSERT_DEVICE_ERROR(buffer.MapAsync(
        GetParam() == wgpu::MapMode::Read ? wgpu::MapMode::Write : wgpu::MapMode::Read, 0, 4,
        wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback()));
    WaitForAllOperations();
}

// Test MapAsync call in MapAsync unmapped callback
TEST_P(BufferMappingValidationTest, MapAsync_RetryInUnmappedCallback) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Aborted, _)).WillOnce(Invoke([&] {
        // MapAsync call on unmapped buffer should be valid
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
    }));

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    buffer.Unmap();
    WaitForAllOperations();
}

// Test MapAsync call in MapAsync destroyed callback
TEST_P(BufferMappingValidationTest, MapAsync_RetryInDestroyedCallback) {
    wgpu::Buffer buffer = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Aborted, _)).WillOnce(Invoke([&] {
        // MapAsync call on destroyed buffer should be invalid
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, HasSubstr("destroyed"))).Times(1);
        ASSERT_DEVICE_ERROR(buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowSpontaneous,
                                            mockCb.Callback()));
    }));

    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    buffer.Destroy();
    WaitForAllOperations();
}

// Test the success case for mappedAtCreation
TEST_F(BufferValidationTest, MappedAtCreationSuccess) {
    BufferMappedAtCreation(4, wgpu::BufferUsage::MapWrite);
}

// Test the success case for mappedAtCreation for a non-mappable usage
TEST_F(BufferValidationTest, NonMappableMappedAtCreationSuccess) {
    BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
}

// Test there is an error when mappedAtCreation is set but the size isn't aligned to 4.
TEST_F(BufferValidationTest, MappedAtCreationSizeAlignment) {
    ASSERT_DEVICE_ERROR(BufferMappedAtCreation(2, wgpu::BufferUsage::MapWrite));
}

// Test that it is valid to destroy an error buffer
TEST_F(BufferValidationTest, DestroyErrorBuffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buf;
    ASSERT_DEVICE_ERROR(buf = device.CreateBuffer(&desc));

    buf.Destroy();
}

// Test that it is valid to Destroy an unmapped buffer
TEST_P(BufferMappingValidationTest, DestroyUnmappedBuffer) {
    wgpu::Buffer buffer = CreateBuffer(4);
    buffer.Destroy();
}

// Test that it is valid to Destroy a destroyed buffer
TEST_P(BufferMappingValidationTest, DestroyDestroyedBuffer) {
    wgpu::Buffer buffer = CreateBuffer(4);
    buffer.Destroy();
    buffer.Destroy();
}

// Test that it is valid to Unmap an error buffer
TEST_F(BufferValidationTest, UnmapErrorBuffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buf;
    ASSERT_DEVICE_ERROR(buf = device.CreateBuffer(&desc));

    buf.Unmap();
}

// Test that it is valid to Unmap a destroyed buffer
TEST_P(BufferMappingValidationTest, UnmapDestroyedBuffer) {
    wgpu::Buffer buffer = CreateBuffer(4);
    buffer.Destroy();
    buffer.Unmap();
}

// Test that unmap then mapping a destroyed buffer is an error.
// Regression test for crbug.com/1388920.
TEST_P(BufferMappingValidationTest, MapDestroyedBufferAfterUnmap) {
    wgpu::Buffer buffer = CreateMapReadBuffer(4);
    buffer.Destroy();
    buffer.Unmap();

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Error, HasSubstr("destroyed"))).Times(1);
    ASSERT_DEVICE_ERROR(buffer.MapAsync(GetParam(), 0, wgpu::kWholeMapSize,
                                        wgpu::CallbackMode::AllowSpontaneous, mockCb.Callback()));

    WaitForAllOperations();
}

// Test that it is valid to submit a buffer in a queue with a map usage if it is unmapped
TEST_F(BufferValidationTest, SubmitBufferWithMapUsage) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;

    wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
    wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test that it is invalid to submit a mapped buffer in a queue
TEST_F(BufferValidationTest, SubmitMappedBuffer) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    {
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        bufA.MapAsync(wgpu::MapMode::Write, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                      mockCb.Callback());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        WaitForAllOperations();
    }
    {
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        bufB.MapAsync(wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                      mockCb.Callback());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        WaitForAllOperations();
    }
    {
        wgpu::BufferDescriptor mappedBufferDesc = descriptorA;
        mappedBufferDesc.mappedAtCreation = true;
        wgpu::Buffer bufA = device.CreateBuffer(&mappedBufferDesc);
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        WaitForAllOperations();
    }
    {
        wgpu::BufferDescriptor mappedBufferDesc = descriptorB;
        mappedBufferDesc.mappedAtCreation = true;
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBuffer(&mappedBufferDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        WaitForAllOperations();
    }
}

// Test that it is invalid to submit a destroyed buffer in a queue
TEST_F(BufferValidationTest, SubmitDestroyedBuffer) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst;

    wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
    wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

    bufA.Destroy();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Test that a map usage is not required to call Unmap
TEST_F(BufferValidationTest, UnmapWithoutMapUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buf = device.CreateBuffer(&descriptor);

    buf.Unmap();
}

// Test that it is valid to call Unmap on a buffer that is not mapped
TEST_P(BufferMappingValidationTest, UnmapUnmappedBuffer) {
    wgpu::Buffer buffer = CreateBuffer(4);

    // Buffer starts unmapped. Unmap shouldn't fail.
    buffer.Unmap();

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call).Times(1);
    buffer.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowSpontaneous, mockCb.Callback());
    buffer.Unmap();

    // Unmapping a second time shouldn't fail.
    buffer.Unmap();
}

// Test that it is invalid to call GetMappedRange on an unmapped buffer.
TEST_F(BufferValidationTest, GetMappedRange_OnUnmappedBuffer) {
    {
        // Unmapped at creation case.
        wgpu::BufferDescriptor desc;
        desc.size = 4;
        desc.usage = wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buf = device.CreateBuffer(&desc);

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }

    // Unmapped after mappedAtCreation case.
    {
        wgpu::Buffer buf = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        buf.Unmap();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }
}

// Test that it is invalid to call GetMappedRange on an unmapped buffer.
TEST_P(BufferMappingValidationTest, GetMappedRange_OnUnmappedBuffer) {
    // Unmapped after valid mapping.
    wgpu::Buffer buf = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
    buf.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    WaitForAllOperations();
    buf.Unmap();

    ASSERT_EQ(nullptr, buf.GetMappedRange());
    ASSERT_EQ(nullptr, buf.GetConstMappedRange());
}

// Test that it is invalid to call GetMappedRange on a destroyed buffer.
TEST_F(BufferValidationTest, GetMappedRange_OnDestroyedBuffer) {
    // Destroyed after creation case.
    {
        wgpu::BufferDescriptor desc;
        desc.size = 4;
        desc.usage = wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buf = device.CreateBuffer(&desc);
        buf.Destroy();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }

    // Destroyed after mappedAtCreation case.
    {
        wgpu::Buffer buf = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        buf.Destroy();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }
}

// Test that it is invalid to call GetMappedRange on a destroyed buffer.
TEST_P(BufferMappingValidationTest, GetMappedRange_OnDestroyedBuffer) {
    // Destroyed after MapAsync case.
    wgpu::Buffer buf = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
    buf.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    WaitForAllOperations();
    buf.Destroy();

    ASSERT_EQ(nullptr, buf.GetMappedRange());
    ASSERT_EQ(nullptr, buf.GetConstMappedRange());
}

// Test that it is invalid to call GetMappedRange on a buffer after MapAsync for reading
TEST_F(BufferValidationTest, GetMappedRange_NonConstOnMappedForReading) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
    buf.MapAsync(wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                 mockCb.Callback());
    WaitForAllOperations();

    ASSERT_EQ(nullptr, buf.GetMappedRange());
}

// Test valid cases to call GetMappedRange on a buffer.
TEST_F(BufferValidationTest, GetMappedRange_ValidBufferStateCases) {
    // GetMappedRange after mappedAtCreation case.
    {
        wgpu::Buffer buffer = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }
}

// Test valid cases to call GetMappedRange on a buffer.
TEST_P(BufferMappingValidationTest, GetMappedRange_ValidBufferStateCases) {
    // GetMappedRange after MapAsync case.
    wgpu::Buffer buf = CreateBuffer(4);

    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
    buf.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
    WaitForAllOperations();

    ASSERT_NE(buf.GetConstMappedRange(), nullptr);
    if (GetParam() == wgpu::MapMode::Write) {
        ASSERT_EQ(buf.GetConstMappedRange(), buf.GetMappedRange());
    }
}

// Test valid cases to call GetMappedRange on an error buffer.
TEST_F(BufferValidationTest, GetMappedRange_OnErrorBuffer) {
    // GetMappedRange after mappedAtCreation a zero-sized buffer returns a non-nullptr.
    // This is to check we don't do a malloc(0).
    {
        wgpu::Buffer buffer;
        ASSERT_DEVICE_ERROR(buffer = BufferMappedAtCreation(
                                0, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead));

        ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }

    // GetMappedRange after mappedAtCreation non-OOM returns a non-nullptr.
    {
        wgpu::Buffer buffer;
        ASSERT_DEVICE_ERROR(buffer = BufferMappedAtCreation(
                                4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead));

        ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }
}

// Test valid cases to call GetMappedRange on an error buffer that's also OOM.
TEST_F(BufferValidationTest, GetMappedRange_OnErrorBuffer_OOM) {
    // TODO(crbug.com/dawn/1506): new (std::nothrow) crashes on OOM on Mac ARM64 because libunwind
    // doesn't see the previous catchall try-catch.
    DAWN_SKIP_TEST_IF(DAWN_PLATFORM_IS(MACOS) && DAWN_PLATFORM_IS(ARM64));

    uint64_t kStupidLarge = uint64_t(1) << uint64_t(63);

    if (UsesWire()) {
        wgpu::Buffer buffer = BufferMappedAtCreation(
            kStupidLarge, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead);
        ASSERT_EQ(nullptr, buffer.Get());
    } else {
        wgpu::Buffer buffer;
        ASSERT_DEVICE_ERROR(
            buffer = BufferMappedAtCreation(
                kStupidLarge, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead));

        // GetMappedRange after mappedAtCreation OOM case returns nullptr.
        ASSERT_EQ(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }
}

// Test validation of the GetMappedRange parameters
TEST_P(BufferMappingValidationTest, GetMappedRange_OffsetSizeOOB) {
    MockMapAsyncCallback mockCb;
    EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(11);

    // Valid case: full range is ok
    {
        wgpu::Buffer buffer = CreateBuffer(8);
        buffer.MapAsync(GetParam(), 0, 8, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_NE(buffer.GetConstMappedRange(0, 8), nullptr);
    }

    // Valid case: full range is ok with defaulted MapAsync size
    {
        wgpu::Buffer buffer = CreateBuffer(8);
        buffer.MapAsync(GetParam(), 0, wgpu::kWholeMapSize, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_NE(buffer.GetConstMappedRange(0, 8), nullptr);
    }

    // Valid case: full range is ok with defaulted MapAsync size and defaulted GetMappedRangeSize
    {
        wgpu::Buffer buffer = CreateBuffer(8);
        buffer.MapAsync(GetParam(), 0, wgpu::kWholeMapSize, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_NE(buffer.GetConstMappedRange(0, wgpu::kWholeMapSize), nullptr);
    }

    // Valid case: empty range at the end is ok
    {
        wgpu::Buffer buffer = CreateBuffer(8);
        buffer.MapAsync(GetParam(), 0, 8, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_NE(buffer.GetConstMappedRange(8, 0), nullptr);
    }

    // Valid case: range in the middle is ok.
    {
        wgpu::Buffer buffer = CreateBuffer(16);
        buffer.MapAsync(GetParam(), 0, 16, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_NE(buffer.GetConstMappedRange(8, 4), nullptr);
    }

    // Error case: offset is larger than the mapped range (even with size = 0)
    {
        wgpu::Buffer buffer = CreateBuffer(8);
        buffer.MapAsync(GetParam(), 0, 8, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_EQ(buffer.GetConstMappedRange(9, 0), nullptr);
        EXPECT_EQ(buffer.GetConstMappedRange(16, 0), nullptr);
        EXPECT_EQ(buffer.GetConstMappedRange(std::numeric_limits<size_t>::max(), 0), nullptr);
    }

    // Error case: offset is larger than the buffer size (even with size = 0)
    {
        wgpu::Buffer buffer = CreateBuffer(16);
        buffer.MapAsync(GetParam(), 8, 8, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_EQ(buffer.GetConstMappedRange(16, 4), nullptr);
        EXPECT_EQ(buffer.GetConstMappedRange(24, 0), nullptr);
        EXPECT_EQ(buffer.GetConstMappedRange(std::numeric_limits<size_t>::max(), 0), nullptr);
    }

    // Error case: offset + size is larger than the mapped range
    {
        wgpu::Buffer buffer = CreateBuffer(12);
        buffer.MapAsync(GetParam(), 0, 12, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_EQ(buffer.GetConstMappedRange(8, 5), nullptr);
        EXPECT_EQ(buffer.GetConstMappedRange(8, 8), nullptr);
    }

    // Error case: offset + size is larger than the mapped range, overflow case
    {
        wgpu::Buffer buffer = CreateBuffer(12);
        buffer.MapAsync(GetParam(), 0, 12, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        // set size to (max - 1) to avoid being equal to kWholeMapSize
        EXPECT_EQ(buffer.GetConstMappedRange(8, std::numeric_limits<size_t>::max() - 1), nullptr);
    }

    // Error case: size is larger than the mapped range when using default kWholeMapSize
    {
        wgpu::Buffer buffer = CreateBuffer(12);
        buffer.MapAsync(GetParam(), 0, 8, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_EQ(buffer.GetConstMappedRange(0), nullptr);
    }

    // Error case: offset is before the start of the range (even with size = 0)
    {
        wgpu::Buffer buffer = CreateBuffer(12);
        buffer.MapAsync(GetParam(), 8, 4, wgpu::CallbackMode::AllowProcessEvents,
                        mockCb.Callback());
        WaitForAllOperations();
        EXPECT_EQ(buffer.GetConstMappedRange(7, 4), nullptr);
        EXPECT_EQ(buffer.GetConstMappedRange(0, 4), nullptr);
        EXPECT_EQ(buffer.GetConstMappedRange(0, 12), nullptr);
        EXPECT_EQ(buffer.GetConstMappedRange(0, 0), nullptr);
    }
}

// Test that the buffer creation parameters are correctly reflected for succesfully created buffers.
TEST_F(BufferValidationTest, CreationParameterReflectionForValidBuffer) {
    // Test reflection on two succesfully created but different buffers. The reflected data should
    // be different!
    {
        wgpu::BufferDescriptor desc;
        desc.size = 16;
        desc.usage = wgpu::BufferUsage::Uniform;
        wgpu::Buffer buf = device.CreateBuffer(&desc);

        EXPECT_EQ(wgpu::BufferUsage::Uniform, buf.GetUsage());
        EXPECT_EQ(16u, buf.GetSize());
    }
    {
        wgpu::BufferDescriptor desc;
        desc.size = 32;
        desc.usage = wgpu::BufferUsage::Storage;
        wgpu::Buffer buf = device.CreateBuffer(&desc);

        EXPECT_EQ(wgpu::BufferUsage::Storage, buf.GetUsage());
        EXPECT_EQ(32u, buf.GetSize());
    }
}

// Test that the buffer creation parameters are correctly reflected for buffers invalid because of
// validation errors.
TEST_F(BufferValidationTest, CreationParameterReflectionForErrorBuffer) {
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::Uniform;
    desc.size = 19;
    desc.mappedAtCreation = true;

    // Error! MappedAtCreation requires size % 4 == 0.
    wgpu::Buffer buf;
    ASSERT_DEVICE_ERROR(buf = device.CreateBuffer(&desc));

    // Reflection data is still exactly what was in the descriptor.
    EXPECT_EQ(wgpu::BufferUsage::Uniform, buf.GetUsage());
    EXPECT_EQ(19u, buf.GetSize());
}

// Test that the buffer creation parameters are correctly reflected for buffers invalid because of
// OOM.
TEST_F(BufferValidationTest, CreationParameterReflectionForOOMBuffer) {
    constexpr uint64_t kAmazinglyLargeSize = 0x1234'5678'90AB'CDEF;
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::Storage;
    desc.size = kAmazinglyLargeSize;

    // OOM!
    wgpu::Buffer buf;
    ASSERT_DEVICE_ERROR(buf = device.CreateBuffer(&desc));

    // Reflection data is still exactly what was in the descriptor.
    EXPECT_EQ(wgpu::BufferUsage::Storage, buf.GetUsage());
    EXPECT_EQ(kAmazinglyLargeSize, buf.GetSize());
}

// Test that buffer reflection doesn't show internal usages
TEST_F(BufferValidationTest, CreationParameterReflectionNoInternalUsage) {
    wgpu::BufferDescriptor desc;
    desc.size = 16;
    // QueryResolve also adds kInternalStorageBuffer for processing of queries.
    desc.usage = wgpu::BufferUsage::QueryResolve;
    wgpu::Buffer buf = device.CreateBuffer(&desc);

    // The reflection shouldn't show kInternalStorageBuffer
    EXPECT_EQ(wgpu::BufferUsage::QueryResolve, buf.GetUsage());
    EXPECT_EQ(16u, buf.GetSize());
}

// Test that GetMapState() shows expected buffer map state
TEST_F(BufferValidationTest, GetMapState) {
    // MappedAtCreation + Unmap
    {
        wgpu::Buffer buf = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());
        buf.Unmap();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MappedAtCreation + Destroy
    {
        wgpu::Buffer buf = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());
        buf.Destroy();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }
}

// Test that GetMapState() shows expected buffer map state
TEST_P(BufferMappingValidationTest, GetMapState) {
    // MapAsync + Unmap
    {
        wgpu::Buffer buf = CreateBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buf.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());

        WaitForAllOperations();
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());

        buf.Unmap();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapAsync + Unmap before the callback
    {
        wgpu::Buffer buf = CreateBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Aborted, _)).Times(1);
        buf.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());

        buf.Unmap();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());

        WaitForAllOperations();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapAsync + Destroy
    {
        wgpu::Buffer buf = CreateBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Success, _)).Times(1);
        buf.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());

        WaitForAllOperations();
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());

        buf.Destroy();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapAsync + Destroy before the callback
    {
        wgpu::Buffer buf = CreateBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());

        MockMapAsyncCallback mockCb;
        EXPECT_CALL(mockCb, Call(wgpu::MapAsyncStatus::Aborted, _)).Times(1);
        buf.MapAsync(GetParam(), 0, 4, wgpu::CallbackMode::AllowProcessEvents, mockCb.Callback());
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());

        buf.Destroy();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());

        WaitForAllOperations();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }
}

class BufferMapExtendedUsagesValidationTest : public BufferValidationTest {
  protected:
    void SetUp() override {
        DAWN_SKIP_TEST_IF(UsesWire());
        BufferValidationTest::SetUp();
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::BufferMapExtendedUsages};
    }
};

// Test that MapRead or MapWrite can be combined with any other usage when creating
// a buffer.
TEST_F(BufferMapExtendedUsagesValidationTest, CreationMapUsageReadOrWriteNoRestrictions) {
    constexpr wgpu::BufferUsage kNonMapUsages[] = {
        wgpu::BufferUsage::CopySrc,  wgpu::BufferUsage::CopyDst,      wgpu::BufferUsage::Index,
        wgpu::BufferUsage::Vertex,   wgpu::BufferUsage::Uniform,      wgpu::BufferUsage::Storage,
        wgpu::BufferUsage::Indirect, wgpu::BufferUsage::QueryResolve,
    };

    // MapRead with anything is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;

        for (const auto otherUsage : kNonMapUsages) {
            descriptor.usage = wgpu::BufferUsage::MapRead | otherUsage;

            device.CreateBuffer(&descriptor);
        }
    }

    // MapWrite with anything is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;

        for (const auto otherUsage : kNonMapUsages) {
            descriptor.usage = wgpu::BufferUsage::MapWrite | otherUsage;

            device.CreateBuffer(&descriptor);
        }
    }

    // MapRead | MapWrite with anything is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;

        for (const auto otherUsage : kNonMapUsages) {
            descriptor.usage =
                wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite | otherUsage;

            device.CreateBuffer(&descriptor);
        }
    }
}
