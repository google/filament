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

#include <utils/Slice.h>
#include <utils/FixedCapacityVector.h>

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
