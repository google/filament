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

#include <private/backend/CircularBuffer.h>

#include <gtest/gtest.h>

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

    // 1. Allocate 3/4 of Bank 0 to advance the pointers.
    const size_t allocSize1 = bufferSize * 3 / 4;
    void* p1 = cb.allocate(allocSize1);
    ASSERT_NE(p1, nullptr);
    memcpy(p1, std::vector<uint8_t>(allocSize1, 1).data(), allocSize1);

    // 2. Consume the first chunk and switch to Bank 1.
    (void)cb.getBuffer();
    EXPECT_TRUE(cb.empty());
    cb.setBank(1);

    // 3. Allocate 3/4 of the buffer in Bank 1.
    const size_t allocSize2 = bufferSize * 3 / 4;
    void* p2 = cb.allocate(allocSize2);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(static_cast<char*>(p1) + cb.getBankStride(), static_cast<char*>(p2));
    std::vector<uint8_t> data2(allocSize2);
    std::iota(data2.begin(), data2.end(), 100);
    memcpy(p2, data2.data(), allocSize2);

    EXPECT_EQ(cb.getUsed(), allocSize2);

    // 4. Consume the second chunk and switch back to Bank 0.
    CircularBuffer::Range r2 = cb.getBuffer();
    EXPECT_EQ(r2.tail, p2);
    EXPECT_EQ(static_cast<char*>(r2.head) - static_cast<char*>(r2.tail), allocSize2);
    EXPECT_EQ(memcmp(r2.tail, data2.data(), allocSize2), 0);
    EXPECT_TRUE(cb.empty());
    cb.setBank(0);

    // 5. After switching back to Bank 0, we can make a nearly full allocation starting at p1.
    const size_t allocSize3 = bufferSize - 1; // Almost full buffer
    void* p3 = cb.allocate(allocSize3);
    ASSERT_NE(p3, nullptr);
    EXPECT_EQ(p1, p3);
    EXPECT_EQ(cb.getUsed(), allocSize3);

    // And verify its data.
    std::vector<uint8_t> data3(allocSize3, 3);
    memcpy(p3, data3.data(), allocSize3);

    CircularBuffer::Range r3 = cb.getBuffer();
    EXPECT_EQ(r3.tail, p3);
    EXPECT_EQ(memcmp(r3.tail, data3.data(), allocSize3), 0);
    EXPECT_TRUE(cb.empty());
}

TEST_F(CircularBufferTest, BankSwitching) {
    const size_t pageSize = CircularBuffer::getBlockSize();
    const size_t bufferSize = pageSize * 8;
    CircularBuffer cb(bufferSize);
    EXPECT_EQ(bufferSize + pageSize, cb.getBankStride());

    // Initial allocation starts at the base of Bank 0.
    char* const bank0Base = static_cast<char*>(cb.allocate(128));
    ASSERT_NE(bank0Base, nullptr);
    EXPECT_EQ(0u, cb.getBank(bank0Base));
    CircularBuffer::Range r0 = cb.getBuffer();
    EXPECT_EQ(bank0Base, static_cast<char*>(r0.tail));

    // Switching to Bank 1 places the next allocation at bank0Base + getBankStride().
    cb.setBank(1);
    char* const bank1Base = static_cast<char*>(cb.allocate(256));
    EXPECT_EQ(bank0Base + cb.getBankStride(), bank1Base);
    EXPECT_EQ(1u, cb.getBank(bank1Base));
    CircularBuffer::Range r1 = cb.getBuffer();
    EXPECT_EQ(bank1Base, static_cast<char*>(r1.tail));

    // Staying in Bank 1 without calling setBank() continues contiguously in Bank 1.
    char* const bank1Cont = static_cast<char*>(cb.allocate(256));
    EXPECT_EQ(bank1Base + 256, bank1Cont);
    EXPECT_EQ(1u, cb.getBank(bank1Cont));
    (void)cb.getBuffer();

    // Switching back to Bank 0 resets to bank0Base (offset 0 of Bank 0).
    cb.setBank(0);
    char* const bank0Reset = static_cast<char*>(cb.allocate(64));
    EXPECT_EQ(bank0Base, bank0Reset);
    EXPECT_EQ(0u, cb.getBank(bank0Reset));
    (void)cb.getBuffer();

#if !defined(__EMSCRIPTEN__)
    // Writing into the guard page at the end of either Bank 0 or Bank 1 must fault.
    EXPECT_DEATH({
        *reinterpret_cast<char volatile*>(bank0Base + bufferSize) = 0x42;
    }, "");
    EXPECT_DEATH({
        *reinterpret_cast<char volatile*>(bank1Base + bufferSize) = 0x42;
    }, "");
#endif
}

} // namespace filament::backend
