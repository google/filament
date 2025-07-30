/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "private/backend/CircularBuffer.h"

#include <cstdint>
#include <cstring>
#include <numeric>
#include <vector>

namespace filament::backend {

// Test fixture for CircularBuffer
class CircularBufferTest : public ::testing::Test {
protected:
    // Common setup can go here if needed
};

TEST_F(CircularBufferTest, Construction) {
    constexpr size_t bufferSize = 1024;
    CircularBuffer cb(bufferSize);

    EXPECT_EQ(cb.size(), bufferSize);
    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.getUsed(), 0);
}

TEST_F(CircularBufferTest, SimpleAlloc) {
    constexpr size_t bufferSize = 1024;
    CircularBuffer cb(bufferSize);

    constexpr size_t allocSize = 100;
    void* p = cb.allocate(allocSize);
    ASSERT_NE(p, nullptr);

    EXPECT_FALSE(cb.empty());
    EXPECT_EQ(cb.getUsed(), allocSize);

    // Write some predictable data to the allocated memory
    std::vector<uint8_t> data(allocSize);
    std::iota(data.begin(), data.end(), 0); // Fill with 0, 1, 2, ...
    memcpy(p, data.data(), allocSize);

    // Get the buffer
    CircularBuffer::Range range = cb.getBuffer();
    ASSERT_EQ(range.tail, p);
    EXPECT_EQ(static_cast<char*>(range.head) - static_cast<char*>(range.tail), allocSize);

    // Verify the data
    EXPECT_EQ(memcmp(range.tail, data.data(), allocSize), 0);

    // Check state after getBuffer()
    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.getUsed(), 0);
}

TEST_F(CircularBufferTest, ZeroAlloc) {
    constexpr size_t bufferSize = 1024;
    CircularBuffer cb(bufferSize);

    // A zero-sized allocation should not change the state.
    void* p1 = cb.allocate(0);
    ASSERT_NE(p1, nullptr);
    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.getUsed(), 0);

    // Allocate some data, then a zero-sized block.
    void* p2 = cb.allocate(50);
    void* p3 = cb.allocate(0);
    ASSERT_EQ(p3, static_cast<char*>(p2) + 50);
    EXPECT_EQ(cb.getUsed(), 50);

    CircularBuffer::Range range = cb.getBuffer();
    EXPECT_EQ(static_cast<char*>(range.head) - static_cast<char*>(range.tail), 50);
    EXPECT_TRUE(cb.empty());
}


TEST_F(CircularBufferTest, MultipleAllocs) {
    constexpr size_t bufferSize = 1024;
    CircularBuffer cb(bufferSize);

    void* first_p = nullptr;
    size_t totalAllocSize = 0;
    const size_t allocSizes[] = { 10, 20, 30, 40 };
    const int numAllocs = sizeof(allocSizes) / sizeof(allocSizes[0]);

    for (int i = 0; i < numAllocs; ++i) {
        const size_t allocSize = allocSizes[i];
        void* p = cb.allocate(allocSize);
        ASSERT_NE(p, nullptr);
        if (i == 0) {
            first_p = p;
        }

        // Write some unique data for each block
        std::vector<uint8_t> data(allocSize);
        std::iota(data.begin(), data.end(), (uint8_t)totalAllocSize);
        memcpy(p, data.data(), allocSize);
        totalAllocSize += allocSize;
    }

    EXPECT_EQ(cb.getUsed(), totalAllocSize);

    CircularBuffer::Range range = cb.getBuffer();
    ASSERT_EQ(range.tail, first_p);
    EXPECT_EQ(static_cast<char*>(range.head) - static_cast<char*>(range.tail), totalAllocSize);

    // Verify all the data is contiguous and correct
    size_t currentOffset = 0;
    for (int i = 0; i < numAllocs; ++i) {
        const size_t allocSize = allocSizes[i];
        std::vector<uint8_t> expected_data(allocSize);
        std::iota(expected_data.begin(), expected_data.end(), (uint8_t)currentOffset);

        EXPECT_EQ(memcmp(static_cast<char*>(range.tail) + currentOffset, expected_data.data(), allocSize), 0);
        currentOffset += allocSize;
    }

    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.getUsed(), 0);
}

TEST_F(CircularBufferTest, FullBuffer) {
    constexpr size_t bufferSize = 4096;
    CircularBuffer cb(bufferSize);

    // Allocate the whole buffer
    void* p = cb.allocate(bufferSize);
    ASSERT_NE(p, nullptr);

    EXPECT_EQ(cb.getUsed(), bufferSize);
    EXPECT_FALSE(cb.empty());

    std::vector<uint8_t> data(bufferSize, 0xAB);
    memcpy(p, data.data(), bufferSize);

    CircularBuffer::Range range = cb.getBuffer();
    EXPECT_EQ(range.tail, p);
    EXPECT_EQ(static_cast<char*>(range.head) - static_cast<char*>(range.tail), bufferSize);
    EXPECT_EQ(memcmp(range.tail, data.data(), bufferSize), 0);

    EXPECT_TRUE(cb.empty());
}

TEST_F(CircularBufferTest, WrapAround) {
    const size_t pageSize = CircularBuffer::getBlockSize();
    // Use a size that may not be page-aligned to test robustness.
    const size_t bufferSize = pageSize * 2 - 128;
    CircularBuffer cb(bufferSize);

    // 1. Allocate 3/4 of the buffer to advance the pointers.
    const size_t allocSize1 = bufferSize * 3 / 4;
    void* p1 = cb.allocate(allocSize1);
    ASSERT_NE(p1, nullptr);
    memcpy(p1, std::vector<uint8_t>(allocSize1, 1).data(), allocSize1);

    // 2. Consume the first chunk. `mHead` and `mTail` are now advanced.
    (void)cb.getBuffer();
    EXPECT_TRUE(cb.empty());

    // 3. Allocate another 3/4 of the buffer. This will cause the internal `mHead`
    // to point beyond `mData + mSize`. The allocation should still succeed and
    // return a pointer that is virtually contiguous.
    const size_t allocSize2 = bufferSize * 3 / 4;
    void* p2 = cb.allocate(allocSize2);
    ASSERT_NE(p2, nullptr);
    std::vector<uint8_t> data2(allocSize2);
    std::iota(data2.begin(), data2.end(), 100);
    memcpy(p2, data2.data(), allocSize2);

    EXPECT_EQ(cb.getUsed(), allocSize2);

    // 4. Consume the second chunk. `getBuffer()` will handle the wrap-around logic internally.
    CircularBuffer::Range r2 = cb.getBuffer();
    EXPECT_EQ(r2.tail, p2);
    EXPECT_EQ(static_cast<char*>(r2.head) - static_cast<char*>(r2.tail), allocSize2);
    EXPECT_EQ(memcmp(r2.tail, data2.data(), allocSize2), 0);
    EXPECT_TRUE(cb.empty());

    // 5. After wrapping, we should be able to make another large allocation.
    // This verifies the internal pointers were reset correctly and the buffer is healthy.
    const size_t allocSize3 = bufferSize - 1; // Almost full buffer
    void* p3 = cb.allocate(allocSize3);
    ASSERT_NE(p3, nullptr);
    EXPECT_EQ(cb.getUsed(), allocSize3);

    // And verify its data.
    std::vector<uint8_t> data3(allocSize3, 3);
    memcpy(p3, data3.data(), allocSize3);

    CircularBuffer::Range r3 = cb.getBuffer();
    EXPECT_EQ(r3.tail, p3);
    EXPECT_EQ(memcmp(r3.tail, data3.data(), allocSize3), 0);
    EXPECT_TRUE(cb.empty());
}

} // namespace filament::backend