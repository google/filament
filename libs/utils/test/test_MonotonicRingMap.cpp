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

#include <utils/MonotonicRingMap.h>

using namespace utils;

TEST(MonotonicRingMap, Empty) {
    MonotonicRingMap<4, uint64_t, uint64_t> map;
    EXPECT_TRUE(map.empty());
    EXPECT_FALSE(map.full());
    EXPECT_EQ(0, map.size());
    EXPECT_EQ(4, map.capacity());
    EXPECT_EQ(nullptr, map.find(100));
}

TEST(MonotonicRingMap, Insert) {
    MonotonicRingMap<4, uint64_t, uint64_t> map;
    map.insert(10, 100);
    map.insert(20, 200);
    EXPECT_EQ(2, map.size());
    EXPECT_EQ(10, map.front().first);
    EXPECT_EQ(100, map.front().second);
    EXPECT_EQ(20, map.back().first);
    EXPECT_EQ(200, map.back().second);
}

TEST(MonotonicRingMap, Find) {
    MonotonicRingMap<4, uint64_t, uint64_t> map;
    map.insert(10, 100);
    map.insert(20, 200);
    map.insert(30, 300);

    const uint64_t* v1 = map.find(10);
    ASSERT_NE(nullptr, v1);
    EXPECT_EQ(100, *v1);

    const uint64_t* v2 = map.find(20);
    ASSERT_NE(nullptr, v2);
    EXPECT_EQ(200, *v2);

    const uint64_t* v3 = map.find(30);
    ASSERT_NE(nullptr, v3);
    EXPECT_EQ(300, *v3);

    EXPECT_EQ(nullptr, map.find(5));    // smaller than min
    EXPECT_EQ(nullptr, map.find(25));   // in between
    EXPECT_EQ(nullptr, map.find(35));   // larger than max
}

TEST(MonotonicRingMap, Full) {
    MonotonicRingMap<4, uint64_t, uint64_t> map;
    map.insert(10, 100);
    map.insert(20, 200);
    map.insert(30, 300);
    map.insert(40, 400);

    EXPECT_TRUE(map.full());
    EXPECT_EQ(4, map.size());

    EXPECT_EQ(10, map.front().first);
    EXPECT_EQ(40, map.back().first);

    // Now insert a new element, which should overwrite the oldest (10)
    map.insert(50, 500);
    EXPECT_TRUE(map.full());
    EXPECT_EQ(4, map.size());

    EXPECT_EQ(20, map.front().first); // 20 is the new oldest
    EXPECT_EQ(50, map.back().first);

    EXPECT_EQ(nullptr, map.find(10));
    ASSERT_NE(nullptr, map.find(20));
    ASSERT_NE(nullptr, map.find(50));
}

TEST(MonotonicRingMap, FindAfterWrapAround) {
    MonotonicRingMap<4, uint64_t, uint64_t> map;
    map.insert(10, 100);
    map.insert(20, 200);
    map.insert(30, 300);
    map.insert(40, 400);
    map.insert(50, 500); // oldest is 20, head is at index 1

    EXPECT_EQ(20, map.front().first);
    EXPECT_EQ(50, map.back().first);

    ASSERT_NE(nullptr, map.find(20));
    ASSERT_NE(nullptr, map.find(30));
    ASSERT_NE(nullptr, map.find(40));
    ASSERT_NE(nullptr, map.find(50));

    map.insert(60, 600); // oldest is 30, head is at index 2

    EXPECT_EQ(30, map.front().first);
    EXPECT_EQ(60, map.back().first);

    EXPECT_EQ(nullptr, map.find(20));
    ASSERT_NE(nullptr, map.find(30));
    ASSERT_NE(nullptr, map.find(60));

    map.insert(70, 700); // oldest is 40, head is at index 3
    map.insert(80, 800); // oldest is 50, head is at index 0

    EXPECT_EQ(50, map.front().first);
    EXPECT_EQ(80, map.back().first);

    EXPECT_EQ(nullptr, map.find(40));
    ASSERT_NE(nullptr, map.find(50));
    ASSERT_NE(nullptr, map.find(60));
    ASSERT_NE(nullptr, map.find(70));
    ASSERT_NE(nullptr, map.find(80));
}

#if !defined(NDEBUG)
TEST(MonotonicRingMap, MonotonicityDeathTest) {
    MonotonicRingMap<4, uint64_t, uint64_t> map;
    map.insert(10, 100);
    map.insert(20, 200);
    EXPECT_DEATH(map.insert(15, 150), "");
}
#endif
