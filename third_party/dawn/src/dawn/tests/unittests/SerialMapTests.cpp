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

#include <utility>
#include <vector>

#include "dawn/common/SerialMap.h"
#include "dawn/common/TypedInteger.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using TestSerialMap = SerialMap<uint64_t, int>;

// A number of basic tests for SerialMap that are difficult to split from one another
TEST(SerialMap, BasicTest) {
    TestSerialMap map;

    // Map starts empty
    ASSERT_TRUE(map.Empty());

    // Iterating on empty map 1) works 2) doesn't produce any values
    for ([[maybe_unused]] int value : map.IterateAll()) {
        ASSERT_TRUE(false);
    }

    // Enqueuing values as const ref or rvalue ref
    map.Enqueue(1, 0);
    map.Enqueue(2, 0);
    map.Enqueue(std::move(3), 1);

    // Iterating over a non-empty map produces the expected result
    std::vector<int> expectedValues = {1, 2, 3};
    for (int value : map.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());

    // Clear works and makes the map empty and iteration does nothing.
    map.Clear();
    ASSERT_TRUE(map.Empty());

    for ([[maybe_unused]] int value : map.IterateAll()) {
        ASSERT_TRUE(false);
    }
}

// Test that items can be enqueued in an arbitrary order
TEST(SerialMap, EnqueueOrder) {
    TestSerialMap map;

    // Enqueue values in an arbitrary order
    map.Enqueue(3, 1);
    map.Enqueue(1, 0);
    map.Enqueue(4, 2);
    map.Enqueue(5, 2);
    map.Enqueue(2, 0);

    // Iterating over a non-empty map produces the expected result
    std::vector<int> expectedValues = {1, 2, 3, 4, 5};
    for (int value : map.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
}

// Test enqueuing vectors works
TEST(SerialMap, EnqueueVectors) {
    TestSerialMap map;

    std::vector<int> vector1 = {1, 2, 3, 4};
    std::vector<int> vector2 = {5, 6, 7, 8};
    std::vector<int> vector3 = {9, 0};

    map.Enqueue(vector1, 0);
    map.Enqueue(std::move(vector2), 0);
    map.Enqueue(vector3, 1);

    std::vector<int> expectedValues = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    for (int value : map.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
}

// Test IterateUpTo
TEST(SerialMap, IterateUpTo) {
    TestSerialMap map;

    std::vector<int> vector1 = {1, 2, 3, 4};
    std::vector<int> vector2 = {5, 6, 7, 8};
    std::vector<int> vector3 = {9, 0};

    map.Enqueue(vector1, 0);
    map.Enqueue(std::move(vector2), 1);
    map.Enqueue(vector3, 2);

    std::vector<int> expectedValues = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int value : map.IterateUpTo(1)) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
}

// Test ClearUpTo
TEST(SerialMap, ClearUpTo) {
    TestSerialMap map;

    std::vector<int> vector1 = {1, 2, 3, 4};
    std::vector<int> vector2 = {5, 6, 7, 8};
    std::vector<int> vector3 = {9, 0};

    map.Enqueue(vector1, 0);
    map.Enqueue(std::move(vector2), 0);
    map.Enqueue(vector3, 1);

    map.ClearUpTo(0);

    std::vector<int> expectedValues = {9, 0};
    for (int value : map.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
}

// Test FirstSerial
TEST(SerialMap, FirstSerial) {
    TestSerialMap map;

    std::vector<int> vector1 = {1, 2, 3, 4};
    std::vector<int> vector2 = {5, 6, 7, 8};
    std::vector<int> vector3 = {9, 0};

    map.Enqueue(vector1, 0);
    map.Enqueue(std::move(vector2), 1);
    map.Enqueue(vector3, 2);

    EXPECT_EQ(map.FirstSerial(), 0u);

    map.ClearUpTo(1);
    EXPECT_EQ(map.FirstSerial(), 2u);

    map.Clear();
    map.Enqueue(vector1, 6);
    EXPECT_EQ(map.FirstSerial(), 6u);
}

// Test basic functionality with type integers
TEST(SerialMap, TypedInteger) {
    using MySerial = TypedInteger<struct MySerialT, uint64_t>;
    using MySerialMap = SerialMap<MySerial, int>;

    MySerialMap map;
    map.Enqueue(1, MySerial(0));
    map.Enqueue(2, MySerial(0));

    std::vector<int> expectedValues = {1, 2};
    for (int value : map.IterateAll()) {
        EXPECT_EQ(expectedValues.front(), value);
        ASSERT_FALSE(expectedValues.empty());
        expectedValues.erase(expectedValues.begin());
    }
    ASSERT_TRUE(expectedValues.empty());
}

}  // anonymous namespace
}  // namespace dawn
