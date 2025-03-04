// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/RingBufferAllocator.h"
#include "gtest/gtest.h"

namespace dawn::native {

constexpr uint64_t RingBufferAllocator::kInvalidOffset;

// Number of basic tests for Ringbuffer
TEST(RingBufferAllocatorTests, BasicTest) {
    constexpr uint64_t sizeInBytes = 64000;
    RingBufferAllocator allocator(sizeInBytes);

    // Ensure no requests exist on empty buffer.
    EXPECT_TRUE(allocator.Empty());

    ASSERT_EQ(allocator.GetSize(), sizeInBytes);

    // Ensure failure upon sub-allocating an oversized request.
    ASSERT_EQ(allocator.Allocate(sizeInBytes + 1, ExecutionSerial(0)),
              RingBufferAllocator::kInvalidOffset);

    // Fill the entire buffer with two requests of equal size.
    ASSERT_EQ(allocator.Allocate(sizeInBytes / 2, ExecutionSerial(1)), 0u);
    ASSERT_EQ(allocator.Allocate(sizeInBytes / 2, ExecutionSerial(2)), 32000u);

    // Ensure the buffer is full.
    ASSERT_EQ(allocator.Allocate(1, ExecutionSerial(3)), RingBufferAllocator::kInvalidOffset);
}

// Tests that several ringbuffer allocations do not fail.
TEST(RingBufferAllocatorTests, RingBufferManyAlloc) {
    constexpr uint64_t maxNumOfFrames = 64000;
    constexpr uint64_t frameSizeInBytes = 4;

    RingBufferAllocator allocator(maxNumOfFrames * frameSizeInBytes);

    size_t offset = 0;
    for (ExecutionSerial i(0); i < ExecutionSerial(maxNumOfFrames); ++i) {
        offset = allocator.Allocate(frameSizeInBytes, i);
        ASSERT_EQ(offset, uint64_t(i) * frameSizeInBytes);
    }
}

// Tests ringbuffer sub-allocations of the same serial are correctly tracked.
TEST(RingBufferAllocatorTests, AllocInSameFrame) {
    constexpr uint64_t maxNumOfFrames = 3;
    constexpr uint64_t frameSizeInBytes = 4;

    RingBufferAllocator allocator(maxNumOfFrames * frameSizeInBytes);

    //    F1
    //  [xxxx|--------]
    size_t offset = allocator.Allocate(frameSizeInBytes, ExecutionSerial(1));

    //    F1   F2
    //  [xxxx|xxxx|----]

    offset = allocator.Allocate(frameSizeInBytes, ExecutionSerial(2));

    //    F1     F2
    //  [xxxx|xxxxxxxx]

    offset = allocator.Allocate(frameSizeInBytes, ExecutionSerial(2));

    ASSERT_EQ(offset, 8u);
    ASSERT_EQ(allocator.GetUsedSize(), frameSizeInBytes * 3);

    allocator.Deallocate(ExecutionSerial(2));

    ASSERT_EQ(allocator.GetUsedSize(), 0u);
    EXPECT_TRUE(allocator.Empty());
}

// Tests ringbuffer sub-allocation at various offsets.
TEST(RingBufferAllocatorTests, RingBufferSubAlloc) {
    constexpr uint64_t maxNumOfFrames = 10;
    constexpr uint64_t frameSizeInBytes = 4;

    RingBufferAllocator allocator(maxNumOfFrames * frameSizeInBytes);

    // Sub-alloc the first eight frames.
    ExecutionSerial serial(0);
    while (serial < ExecutionSerial(8)) {
        allocator.Allocate(frameSizeInBytes, serial);
        serial++;
    }

    // Each frame corrresponds to the serial number (for simplicity).
    //
    //    F1   F2   F3   F4   F5   F6   F7   F8
    //  [xxxx|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx|xxxx|--------]
    //

    // Ensure an oversized allocation fails (only 8 bytes left)
    ASSERT_EQ(allocator.Allocate(frameSizeInBytes * 3, serial),
              RingBufferAllocator::kInvalidOffset);
    ASSERT_EQ(allocator.GetUsedSize(), frameSizeInBytes * 8);

    // Reclaim the first 3 frames.
    allocator.Deallocate(ExecutionSerial(2));

    //                 F4   F5   F6   F7   F8
    //  [------------|xxxx|xxxx|xxxx|xxxx|xxxx|--------]
    //
    ASSERT_EQ(allocator.GetUsedSize(), frameSizeInBytes * 5);

    // Re-try the over-sized allocation.
    size_t offset = allocator.Allocate(frameSizeInBytes * 3, ExecutionSerial(serial));

    //        F9       F4   F5   F6   F7   F8
    //  [xxxxxxxxxxxx|xxxx|xxxx|xxxx|xxxx|xxxx|xxxxxxxx]
    //                                         ^^^^^^^^ wasted

    // In this example, Deallocate(8) could not reclaim the wasted bytes. The wasted bytes
    // were added to F9's sub-allocation.
    // TODO(bryan.bernhart@intel.com): Decide if Deallocate(8) should free these wasted bytes.

    ASSERT_EQ(offset, 0u);
    ASSERT_EQ(allocator.GetUsedSize(), frameSizeInBytes * maxNumOfFrames);

    // Ensure we are full.
    ASSERT_EQ(allocator.Allocate(frameSizeInBytes, serial), RingBufferAllocator::kInvalidOffset);

    // Reclaim the next two frames.
    allocator.Deallocate(ExecutionSerial(4));

    //        F9       F4   F5   F6   F7   F8
    //  [xxxxxxxxxxxx|----|----|xxxx|xxxx|xxxx|xxxxxxxx]
    //
    ASSERT_EQ(allocator.GetUsedSize(), frameSizeInBytes * 8);

    // Sub-alloc the chunk in the middle.
    serial++;
    offset = allocator.Allocate(frameSizeInBytes * 2, serial);

    ASSERT_EQ(offset, frameSizeInBytes * 3);
    ASSERT_EQ(allocator.GetUsedSize(), frameSizeInBytes * maxNumOfFrames);

    //        F9         F10      F6   F7   F8
    //  [xxxxxxxxxxxx|xxxxxxxxx|xxxx|xxxx|xxxx|xxxxxxxx]
    //

    // Ensure we are full.
    ASSERT_EQ(allocator.Allocate(frameSizeInBytes, serial), RingBufferAllocator::kInvalidOffset);

    // Reclaim all.
    allocator.Deallocate(kMaxExecutionSerial);

    EXPECT_TRUE(allocator.Empty());
}

// Checks if ringbuffer sub-allocation does not overflow.
TEST(RingBufferAllocatorTests, RingBufferOverflow) {
    RingBufferAllocator allocator(std::numeric_limits<uint64_t>::max());

    ASSERT_EQ(allocator.Allocate(1, ExecutionSerial(1)), 0u);
    ASSERT_EQ(allocator.Allocate(std::numeric_limits<uint64_t>::max(), ExecutionSerial(1)),
              RingBufferAllocator::kInvalidOffset);
}

}  // namespace dawn::native
