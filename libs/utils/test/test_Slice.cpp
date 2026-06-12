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

#include <utils/FixedCapacityVector.h>
#include <utils/Slice.h>

#include <gtest/gtest.h>

#include <array>
#include <vector>

using namespace utils;

TEST(SliceTest, HashIsEqual) {
    FixedCapacityVector<int> vec1 = {1, 3, 3, 7};
    FixedCapacityVector<int> vec2 = {1, 3, 3, 7};
    Slice<const int> slice1 = vec1.as_slice();
    Slice<const int> slice2 = vec2.as_slice();

    EXPECT_TRUE(slice1.hash() == vec1.hash());
    EXPECT_TRUE(slice1.hash() == slice2.hash());
}

TEST(SliceTest, HashIsNotEqual) {
    FixedCapacityVector<int> vec1 = {1, 3, 3, 7};
    FixedCapacityVector<int> vec2 = {4, 2, 0};
    Slice<const int> slice1 = vec1.as_slice();
    Slice<const int> slice2 = vec2.as_slice();

    EXPECT_FALSE(slice1.hash() == slice2.hash());
}

TEST(SliceTest, ValueEqual) {
    FixedCapacityVector<int> vec1 = {1, 3, 3, 7};
    FixedCapacityVector<int> vec2 = {1, 3, 3, 7};
    Slice<const int> slice1 = vec1.as_slice();
    Slice<const int> slice2 = vec2.as_slice();

    EXPECT_TRUE(slice1 == slice2);
}

TEST(SliceTest, ValueNotEqual) {
    FixedCapacityVector<int> vec1 = {1, 3, 3, 7};
    FixedCapacityVector<int> vec2 = {4, 2, 0};
    Slice<const int> slice1 = vec1.as_slice();
    Slice<const int> slice2 = vec2.as_slice();

    EXPECT_FALSE(slice1 == slice2);
}

TEST(SliceTest, CanCoerceMutableToConst) {
    FixedCapacityVector<int> vec = {1, 3, 3, 7};
    Slice<int> sliceMutable = vec.as_slice();
    Slice<const int> sliceConst = sliceMutable;
}

TEST(SliceTest, CanCompareMutableWithConst) {
    FixedCapacityVector<int> vec1 = {1, 3, 3, 7};
    FixedCapacityVector<int> vec2 = {1, 3, 3, 7};
    Slice<int> slice1 = vec1.as_slice();
    Slice<const int> slice2 = vec2.as_slice();

    EXPECT_TRUE(slice1.hash() == slice2.hash());
}

TEST(SliceTest, CanCompareConstWithMutable) {
    FixedCapacityVector<int> vec1 = {1, 3, 3, 7};
    FixedCapacityVector<int> vec2 = {1, 3, 3, 7};
    Slice<const int> slice1 = vec1.as_slice();
    Slice<int> slice2 = vec2.as_slice();

    EXPECT_TRUE(slice1.hash() == slice2.hash());
}

TEST(SliceTest, ConstructFromCArray) {
    int arr[] = {10, 20, 30};
    Slice<int> s = arr;
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(s[0], 10);
    EXPECT_EQ(s[1], 20);
    EXPECT_EQ(s[2], 30);
}

TEST(SliceTest, ConstructFromStdArray) {
    std::array<int, 3> arr = {10, 20, 30};
    Slice<int> s = arr;
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(s[0], 10);
    EXPECT_EQ(s[1], 20);
    EXPECT_EQ(s[2], 30);

    Slice<const int> sConst = arr;
    EXPECT_EQ(sConst.size(), 3);
}

TEST(SliceTest, ConstructFromContainer) {
    std::vector<int> vec = {1, 2, 3, 4};
    Slice<int> s = vec;
    EXPECT_EQ(s.size(), 4);
    EXPECT_EQ(s[3], 4);
}

TEST(SliceTest, Subviews) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    Slice<int> s = vec;

    Slice<int> sFirst = s.first(2);
    EXPECT_EQ(sFirst.size(), 2);
    EXPECT_EQ(sFirst[0], 1);
    EXPECT_EQ(sFirst[1], 2);

    Slice<int> sLast = s.last(2);
    EXPECT_EQ(sLast.size(), 2);
    EXPECT_EQ(sLast[0], 4);
    EXPECT_EQ(sLast[1], 5);

    Slice<int> sSub = s.subspan(1, 3);
    EXPECT_EQ(sSub.size(), 3);
    EXPECT_EQ(sSub[0], 2);
    EXPECT_EQ(sSub[1], 3);
    EXPECT_EQ(sSub[2], 4);
}

TEST(SliceTest, ReverseIterators) {
    std::vector<int> vec = {1, 2, 3};
    Slice<int> s = vec;

    std::vector<int> reversed;
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
        reversed.push_back(*it);
    }
    EXPECT_EQ(reversed.size(), 3);
    EXPECT_EQ(reversed[0], 3);
    EXPECT_EQ(reversed[1], 2);
    EXPECT_EQ(reversed[2], 1);
}

TEST(SliceTest, AsBytes) {
    int val = 0x12345678;
    Slice<int> s(&val, 1);

    Slice<const uint8_t> bytes = as_bytes(s);
    EXPECT_EQ(bytes.size(), sizeof(int));
    EXPECT_EQ(bytes.size_bytes(), sizeof(int));

    Slice<uint8_t> writableBytes = as_writable_bytes(s);
    EXPECT_EQ(writableBytes.size(), sizeof(int));
    writableBytes[0] = 0xAA;
}

#if __cplusplus >= 201703L
TEST(SliceTest, DeductionGuides) {
    std::vector<int> vec = {5, 6, 7};
    Slice s = vec;
    EXPECT_EQ(s.size(), 3);
}
#endif
