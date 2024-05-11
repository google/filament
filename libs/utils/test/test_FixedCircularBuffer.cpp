/*
 * Copyright (C) 2023 The Android Open Source Project
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
 *
 */

#include <gtest/gtest.h>

#include <utils/FixedCircularBuffer.h>

using namespace utils;

TEST(FixedCircularBufferTest, Simple) {
    FixedCircularBuffer<int> circularBuffer(4);
    EXPECT_EQ(circularBuffer.size(), 0);

    EXPECT_FALSE(circularBuffer.push(1).has_value());
    EXPECT_FALSE(circularBuffer.push(2).has_value());
    EXPECT_FALSE(circularBuffer.push(3).has_value());
    EXPECT_EQ(circularBuffer.size(), 3);
    EXPECT_EQ(circularBuffer.pop(), 1);
    EXPECT_EQ(circularBuffer.pop(), 2);
    EXPECT_EQ(circularBuffer.pop(), 3);
    EXPECT_EQ(circularBuffer.size(), 0);

    EXPECT_FALSE(circularBuffer.push(4).has_value());
    EXPECT_FALSE(circularBuffer.push(5).has_value());
    EXPECT_FALSE(circularBuffer.push(6).has_value());
    EXPECT_FALSE(circularBuffer.push(7).has_value());
    EXPECT_EQ(circularBuffer.size(), 4);
    EXPECT_TRUE(circularBuffer.full());
    EXPECT_EQ(circularBuffer.pop(), 4);
    EXPECT_EQ(circularBuffer.pop(), 5);
    EXPECT_EQ(circularBuffer.pop(), 6);
    EXPECT_EQ(circularBuffer.pop(), 7);
}

TEST(FixedCircularBufferTest, Displace) {
    FixedCircularBuffer<int> circularBuffer(4);
    EXPECT_EQ(circularBuffer.size(), 0);
    EXPECT_FALSE(circularBuffer.push(1).has_value());
    EXPECT_FALSE(circularBuffer.push(2).has_value());
    EXPECT_FALSE(circularBuffer.push(3).has_value());
    EXPECT_FALSE(circularBuffer.push(4).has_value());
    EXPECT_TRUE(circularBuffer.full());

    {
        auto v = circularBuffer.push(5);
        EXPECT_EQ(v.value(), 1);
    }
    {
        auto v = circularBuffer.push(6);
        EXPECT_EQ(v.value(), 2);
    }
    EXPECT_TRUE(circularBuffer.full());

    EXPECT_EQ(circularBuffer.pop(), 3);
    EXPECT_EQ(circularBuffer.size(), 3);
}

TEST(FixedCircularBufferTest, ZeroCapacity) {
    FixedCircularBuffer<int> circularBuffer(0);
    EXPECT_EQ(circularBuffer.size(), 0);
    EXPECT_EQ(circularBuffer.full(), false);

    auto v = circularBuffer.push(1);
    EXPECT_EQ(v.value(), 1);
    EXPECT_EQ(circularBuffer.size(), 0);
    EXPECT_EQ(circularBuffer.full(), false);
}

TEST(FixedCircularBufferTest, Exceptions) {
#if !defined(NDEBUG) && defined(GTEST_HAS_DEATH_TEST)
    FixedCircularBuffer<int> circularBuffer(4);

    EXPECT_DEATH({
        circularBuffer.pop();  // should assert
    }, "failed assertion");

    circularBuffer.push(1);
    circularBuffer.push(2);
    circularBuffer.push(3);
    circularBuffer.push(4);
    circularBuffer.push(5); // should not assert
#endif
}
