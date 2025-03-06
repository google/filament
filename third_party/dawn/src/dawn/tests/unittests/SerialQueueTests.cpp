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

#include <utility>
#include <vector>

#include "dawn/common/SerialQueue.h"
#include "dawn/common/TypedInteger.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using TestSerialQueue = SerialQueue<uint64_t, int>;

// A number of basic tests for SerialQueue that are difficult to split from one another
TEST(SerialQueue, BasicTest) {
    TestSerialQueue queue;

    // Queue starts empty
    ASSERT_TRUE(queue.Empty());

    // Iterating on empty queue 1) works 2) doesn't produce any values
    for ([[maybe_unused]] int value : queue.IterateAll()) {
        ASSERT_TRUE(false);
    }

    // Enqueuing values as const ref or rvalue ref
    queue.Enqueue(1, 0);
    queue.Enqueue(2, 0);
    queue.Enqueue(std::move(3), 1);

    // Iterating over a non-empty queue produces the expected result
    std::vector<int> expectedValues = {1, 2, 3};
    for (int value : queue.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());

    // Clear works and makes the queue empty and iteration does nothing.
    queue.Clear();
    ASSERT_TRUE(queue.Empty());

    for ([[maybe_unused]] int value : queue.IterateAll()) {
        ASSERT_TRUE(false);
    }
}

// Test enqueuing vectors works
TEST(SerialQueue, EnqueueVectors) {
    TestSerialQueue queue;

    std::vector<int> vector1 = {1, 2, 3, 4};
    std::vector<int> vector2 = {5, 6, 7, 8};
    std::vector<int> vector3 = {9, 0};

    queue.Enqueue(vector1, 0);
    queue.Enqueue(std::move(vector2), 0);
    queue.Enqueue(vector3, 1);

    std::vector<int> expectedValues = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    for (int value : queue.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
}

// Test IterateUpTo
TEST(SerialQueue, IterateUpTo) {
    TestSerialQueue queue;

    std::vector<int> vector1 = {1, 2, 3, 4};
    std::vector<int> vector2 = {5, 6, 7, 8};
    std::vector<int> vector3 = {9, 0};

    queue.Enqueue(vector1, 0);
    queue.Enqueue(std::move(vector2), 1);
    queue.Enqueue(vector3, 2);

    std::vector<int> expectedValues = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int value : queue.IterateUpTo(1)) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
    EXPECT_EQ(queue.LastSerial(), 2u);
}

// Test ClearUpTo
TEST(SerialQueue, ClearUpTo) {
    TestSerialQueue queue;

    std::vector<int> vector1 = {1, 2, 3, 4};
    std::vector<int> vector2 = {5, 6, 7, 8};
    std::vector<int> vector3 = {9, 0};

    queue.Enqueue(vector1, 0);
    queue.Enqueue(std::move(vector2), 0);
    queue.Enqueue(vector3, 1);

    queue.ClearUpTo(0);
    EXPECT_EQ(queue.LastSerial(), 1u);

    std::vector<int> expectedValues = {9, 0};
    for (int value : queue.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
}

// Test FirstSerial
TEST(SerialQueue, FirstSerial) {
    TestSerialQueue queue;

    std::vector<int> vector1 = {1, 2, 3, 4};
    std::vector<int> vector2 = {5, 6, 7, 8};
    std::vector<int> vector3 = {9, 0};

    queue.Enqueue(vector1, 0);
    queue.Enqueue(std::move(vector2), 1);
    queue.Enqueue(vector3, 2);

    EXPECT_EQ(queue.FirstSerial(), 0u);

    queue.ClearUpTo(1);
    EXPECT_EQ(queue.FirstSerial(), 2u);

    queue.Clear();
    queue.Enqueue(vector1, 6);
    EXPECT_EQ(queue.FirstSerial(), 6u);
}

// Test LastSerial
TEST(SerialQueue, LastSerial) {
    TestSerialQueue queue;

    queue.Enqueue({1}, 0);
    EXPECT_EQ(queue.LastSerial(), 0u);

    queue.Enqueue({2}, 1);
    EXPECT_EQ(queue.LastSerial(), 1u);
}

// Test basic functionality with type integers
TEST(SerialQueue, TypedInteger) {
    using MySerial = TypedInteger<struct MySerialT, uint64_t>;
    using MySerialQueue = SerialQueue<MySerial, int>;

    MySerialQueue queue;
    queue.Enqueue(1, MySerial(0));
    queue.Enqueue(2, MySerial(0));

    std::vector<int> expectedValues = {1, 2};
    for (int value : queue.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
}

}  // anonymous namespace
}  // namespace dawn
