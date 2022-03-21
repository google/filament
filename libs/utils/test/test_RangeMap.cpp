/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <utils/RangeMap.h>

using namespace utils;

TEST(RangeMapTest, Simple) {
    RangeMap<int, char> map;

    map.add(45, 104, 'c');
    EXPECT_FALSE(map.has(104));
    EXPECT_EQ(map.get(45), 'c');
    EXPECT_EQ(map.get(103), 'c');
    EXPECT_EQ(map.rangeCount(), 1);

    // Perform a no-op.
    map.set(103, 'c');
    EXPECT_EQ(map.rangeCount(), 1);

    // Extend a neighboring interval.
    map.set(104, 'c');
    EXPECT_TRUE(map.has(104));
    EXPECT_EQ(map.get(104), 'c');
    EXPECT_EQ(map.rangeCount(), 1);

    // Add a second interval.
    map.set(105, 'd');
    EXPECT_EQ(map.get(105), 'd');
    EXPECT_EQ(map.rangeCount(), 2);

    // Extend an existing interval and wipe out the interval we just added.
    map.set(105, 'c');
    EXPECT_EQ(map.get(105), 'c');
    EXPECT_EQ(map.rangeCount(), 1);

    // Clip the endpoint of an existing interval.
    map.set(105, 'd');
    EXPECT_EQ(map.get(104), 'c');
    EXPECT_EQ(map.get(105), 'd');
    EXPECT_EQ(map.rangeCount(), 2);

    // Extend the second interval backwards while clipping the first one.
    map.set(104, 'd');
    EXPECT_EQ(map.get(104), 'd');
    EXPECT_EQ(map.get(103), 'c');
    EXPECT_EQ(map.rangeCount(), 2);

    // Insert a range into the middle of the first range.
    map.add(46, 50, 'e');
    EXPECT_EQ(map.rangeCount(), 4);
    EXPECT_EQ(map.get(45), 'c');
    EXPECT_EQ(map.get(46), 'e');
    EXPECT_EQ(map.get(49), 'e');
    EXPECT_EQ(map.get(50), 'c');

    // Extend the range that we just added backwards, wiping out its leftward neighbor.
    map.add(44, 50, 'e');
    EXPECT_EQ(map.rangeCount(), 3);

    map.reset(45);
    EXPECT_FALSE(map.has(45));
    EXPECT_TRUE(map.has(46));

    map.clear(44, 47);
    EXPECT_EQ(map.rangeCount(), 3);
    EXPECT_FALSE(map.has(46));
    EXPECT_TRUE(map.has(47));
    map.clear(47, 50);
    EXPECT_EQ(map.rangeCount(), 2);

    // Clearing the middle of an existing interval should split it.
    map.reset(55);
    EXPECT_EQ(map.rangeCount(), 3);
    EXPECT_TRUE(map.has(54));
    EXPECT_FALSE(map.has(55));
    EXPECT_TRUE(map.has(56));

    map.clear(-100, 200);
    map.add(0, 10, 'a');
    map.add(12, 15, 'a');
    map.add(20, 22, 'b');
    map.add(23, 26, 'c');
    EXPECT_EQ(map.rangeCount(), 4);

    // Test a bunch of different ways that the first and second intervals can be merged.
    map.add(10, 12, 'a');
    EXPECT_EQ(map.rangeCount(), 3);
    map.clear(10, 12);
    EXPECT_EQ(map.rangeCount(), 4);
    map.add(9, 12, 'a');
    EXPECT_EQ(map.rangeCount(), 3);
    map.clear(10, 12);
    EXPECT_EQ(map.rangeCount(), 4);
    map.add(10, 13, 'a');
    EXPECT_EQ(map.rangeCount(), 3);
    map.add(10, 12, 'b');

    EXPECT_EQ(map.rangeCount(), 5);
    map.add(0, 26, 'b');
    EXPECT_EQ(map.rangeCount(), 1);
    map.add(10, 12, 'c');
    EXPECT_EQ(map.rangeCount(), 3);
    map.add(9, 13, 'c');
    EXPECT_EQ(map.rangeCount(), 3);
}

enum MockImageLayout {
    VK_IMAGE_LAYOUT_UNDEFINED = 0,
    VK_IMAGE_LAYOUT_GENERAL = 1,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
    VK_IMAGE_LAYOUT_MAX_ENUM = 0x7FFFFFFF
};

TEST(RangeMapTest, BugRepro1) {
    utils::RangeMap<int32_t, MockImageLayout> tracker;
    tracker.set(1, VK_IMAGE_LAYOUT_GENERAL);
    tracker.set(2, VK_IMAGE_LAYOUT_GENERAL);
    tracker.set(3, VK_IMAGE_LAYOUT_GENERAL);
    tracker.set(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    tracker.add(0, 2, VK_IMAGE_LAYOUT_GENERAL);
    EXPECT_EQ(tracker.rangeCount(), 1);

    tracker.set(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    EXPECT_EQ(tracker.rangeCount(), 2);

    tracker.reset(3);
    tracker.reset(2);
    tracker.clear(-1, 2);
    EXPECT_EQ(tracker.rangeCount(), 0);
}
