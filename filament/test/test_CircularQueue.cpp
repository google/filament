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
 */

#include <gtest/gtest.h>

#include "../src/FrameInfo.h"

#include <utility>
#include <vector>

using namespace filament;

TEST(CircularQueue, Empty) {
    CircularQueue<int, 4> const q{};
    EXPECT_EQ(q.capacity(), 4);
    EXPECT_EQ(q.size(), 0);
    EXPECT_TRUE(q.empty());
}

TEST(CircularQueue, PushFront) {
    CircularQueue<int, 4> q;
    q.push_front(1);
    EXPECT_EQ(q.size(), 1);
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.front(), 1);

    q.push_front(2);
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.front(), 2);
    EXPECT_EQ(q[1], 1);

    q.push_front(3);
    q.push_front(4);
    EXPECT_EQ(q.size(), 4);
    EXPECT_EQ(q.front(), 4);
    EXPECT_EQ(q[1], 3);
    EXPECT_EQ(q[2], 2);
    EXPECT_EQ(q[3], 1);
}

TEST(CircularQueue, EmplaceFront) {
    CircularQueue<std::pair<int, int>, 2> q;
    q.emplace_front(1, 2);
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.front(), std::make_pair(1, 2));

    q.emplace_front(3, 4);
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.front(), std::make_pair(3, 4));
    EXPECT_EQ(q[1], std::make_pair(1, 2));
}


TEST(CircularQueue, PopBack) {
    CircularQueue<int, 4> q;
    q.push_front(1);
    q.push_front(2);
    q.push_front(3);
    EXPECT_EQ(q.size(), 3);

    q.pop_back();
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.front(), 3);
    EXPECT_EQ(q[1], 2);

    q.pop_back();
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.front(), 3);

    q.pop_back();
    EXPECT_EQ(q.size(), 0);
    EXPECT_TRUE(q.empty());
}

TEST(CircularQueue, PushPop) {
    CircularQueue<int, 3> q;
    q.push_front(1);
    q.push_front(2);
    q.push_front(3);
    EXPECT_EQ(q.size(), 3);

    q.pop_back();
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.front(), 3);
    EXPECT_EQ(q[1], 2);

    q.push_front(4);
    EXPECT_EQ(q.size(), 3);
    EXPECT_EQ(q.front(), 4);
    EXPECT_EQ(q[1], 3);
    EXPECT_EQ(q[2], 2);

    q.pop_back();
    q.pop_back();
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.front(), 4);
}

struct TestObject {
    static int construction_count;
    static int destruction_count;
    TestObject() { construction_count++; }
    TestObject(const TestObject&) { construction_count++; }
    TestObject(TestObject&&) noexcept { construction_count++; }
    ~TestObject() { destruction_count++; }
};

int TestObject::construction_count = 0;
int TestObject::destruction_count = 0;


TEST(CircularQueue, ObjectLifecycle) {
    TestObject::construction_count = 0;
    TestObject::destruction_count = 0;
    {
        CircularQueue<TestObject, 3> q;
        EXPECT_EQ(TestObject::construction_count, 0);
        EXPECT_EQ(TestObject::destruction_count, 0);

        q.emplace_front();
        EXPECT_EQ(TestObject::construction_count, 1);
        EXPECT_EQ(TestObject::destruction_count, 0);

        TestObject const t;
        EXPECT_EQ(TestObject::construction_count, 2);
        q.push_front(t);
        EXPECT_EQ(TestObject::construction_count, 3);
        EXPECT_EQ(TestObject::destruction_count, 0);

        q.push_front(TestObject{});
        EXPECT_EQ(TestObject::construction_count, 5); // 4 for {}, 5 for move
        EXPECT_EQ(TestObject::destruction_count, 1); // destruction of temporary

        EXPECT_EQ(q.size(), 3);

        q.pop_back();
        EXPECT_EQ(TestObject::destruction_count, 2);

        q.pop_back();
        EXPECT_EQ(TestObject::destruction_count, 3);
    }
    EXPECT_EQ(TestObject::construction_count, 5);
    EXPECT_EQ(TestObject::destruction_count, 5);
}

struct PopBackTestObject {
    int id;
    static std::vector<int> destroyed_ids;

    PopBackTestObject(int const id) : id(id) {}
    ~PopBackTestObject() {
        destroyed_ids.push_back(id);
    }
};
std::vector<int> PopBackTestObject::destroyed_ids;

TEST(CircularQueue, PopBackDestroysCorrectObject) {
    PopBackTestObject::destroyed_ids.clear();
    {
        CircularQueue<PopBackTestObject, 4> q;
        q.emplace_front(1);
        q.emplace_front(2);
        q.emplace_front(3);

        // The queue contains [3, 2, 1], where 3 is at the front and 1 is at the back.
        q.pop_back(); // Should destroy 1.
        ASSERT_EQ(PopBackTestObject::destroyed_ids.size(), 1);
        EXPECT_EQ(PopBackTestObject::destroyed_ids[0], 1);

        q.pop_back(); // Should destroy 2.
        ASSERT_EQ(PopBackTestObject::destroyed_ids.size(), 2);
        EXPECT_EQ(PopBackTestObject::destroyed_ids[1], 2);

        q.emplace_front(4);
        // The queue contains [4, 3]. 3 is at the back.
        q.pop_back(); // Should destroy 3.
        ASSERT_EQ(PopBackTestObject::destroyed_ids.size(), 3);
        EXPECT_EQ(PopBackTestObject::destroyed_ids[2], 3);
    }
    // Queue is destroyed, remaining object with id 4 is destroyed.
    ASSERT_EQ(PopBackTestObject::destroyed_ids.size(), 4);
    EXPECT_EQ(PopBackTestObject::destroyed_ids[3], 4);
}

TEST(CircularQueue, PointerStability) {
    CircularQueue<int, 5> q;
    q.push_front(1);
    q.push_front(2);

    // q is [2, 1]
    const int* p1 = &q[1];
    EXPECT_EQ(*p1, 1);

    q.push_front(3);
    q.push_front(4);
    // q is [4, 3, 2, 1]
    EXPECT_EQ(*p1, 1);

    const int* p3 = &q[1];
    EXPECT_EQ(*p3, 3);

    q.pop_back(); // removes 1
    // q is [4, 3, 2]
    // p1 is now invalid, but p3 should be fine.
    EXPECT_EQ(*p3, 3);

    q.push_front(5);
    q.push_front(6);
    // q is [6, 5, 4, 3, 2]
    // p3 is still fine.
    EXPECT_EQ(*p3, 3);

    q.pop_back(); // removes 2
    q.pop_back(); // removes 3
    // q is [6, 5, 4]
    // p3 is now invalid.
}
