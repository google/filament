//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FixedVector_unittest:
//   Tests of the FixedVector class
//

#include <gtest/gtest.h>

#include "common/FixedVector.h"

namespace angle
{
// Make sure the various constructors compile and do basic checks
TEST(FixedVector, Constructors)
{
    FixedVector<int, 5> defaultContructor;
    EXPECT_EQ(0u, defaultContructor.size());

    FixedVector<int, 5> count(3);
    EXPECT_EQ(3u, count.size());

    FixedVector<int, 5> countAndValue(3, 2);
    EXPECT_EQ(3u, countAndValue.size());
    EXPECT_EQ(2, countAndValue[1]);

    FixedVector<int, 5> initializerList{1, 2, 3, 4, 5};
    EXPECT_EQ(5u, initializerList.size());
    EXPECT_EQ(3, initializerList[2]);

    FixedVector<int, 5> copy(initializerList);
    EXPECT_EQ(copy, initializerList);

    FixedVector<int, 5> copyRValue(std::move(copy));
    EXPECT_EQ(copyRValue, initializerList);
    EXPECT_EQ(0u, copy.size());

    FixedVector<int, 5> assignCopy;
    assignCopy = copyRValue;
    EXPECT_EQ(assignCopy, initializerList);

    FixedVector<int, 5> assignRValue;
    assignRValue = std::move(assignCopy);
    EXPECT_EQ(assignRValue, initializerList);
    EXPECT_EQ(0u, assignCopy.size());

    FixedVector<int, 5> assignmentInitializerList;
    assignmentInitializerList = {1, 2, 3, 4, 5};
    EXPECT_EQ(5u, assignmentInitializerList.size());
    EXPECT_EQ(3, assignmentInitializerList[2]);
}

// Test indexing operations (at, operator[])
TEST(FixedVector, Indexing)
{
    FixedVector<int, 5> vec = {0, 1, 2, 3, 4};
    EXPECT_EQ(0, vec.at(0));
    EXPECT_EQ(vec[0], vec.at(0));
}

// Test the push_back functions
TEST(FixedVector, PushBack)
{
    FixedVector<int, 5> vec;
    vec.push_back(1);
    EXPECT_EQ(1, vec[0]);
    vec.push_back(1);
    vec.push_back(1);
    vec.push_back(1);
    vec.push_back(1);
    EXPECT_EQ(vec.size(), vec.max_size());
}

// Test the pop_back function
TEST(FixedVector, PopBack)
{
    FixedVector<int, 5> vec;
    vec.push_back(1);
    EXPECT_EQ(1, (int)vec.size());
    vec.pop_back();
    EXPECT_EQ(0, (int)vec.size());
}

// Test the back function
TEST(FixedVector, Back)
{
    FixedVector<int, 5> vec;
    vec.push_back(1);
    vec.push_back(2);
    EXPECT_EQ(2, vec.back());
}

// Test the sizing operations
TEST(FixedVector, Size)
{
    FixedVector<int, 5> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(0u, vec.size());
    EXPECT_EQ(5u, vec.max_size());

    vec.push_back(1);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(1u, vec.size());
}

// Test clearing the vector
TEST(FixedVector, Clear)
{
    FixedVector<int, 5> vec = {0, 1, 2, 3, 4};
    vec.clear();
    EXPECT_TRUE(vec.empty());
}

// Test resizing the vector
TEST(FixedVector, Resize)
{
    FixedVector<int, 5> vec;
    vec.resize(5u, 1);
    EXPECT_EQ(5u, vec.size());
    EXPECT_EQ(1, vec[4]);

    vec.resize(2u);
    EXPECT_EQ(2u, vec.size());
}

// Test iterating over the vector
TEST(FixedVector, Iteration)
{
    FixedVector<int, 5> vec = {0, 1, 2, 3};

    int vistedCount = 0;
    for (int value : vec)
    {
        EXPECT_EQ(vistedCount, value);
        vistedCount++;
    }
    EXPECT_EQ(4, vistedCount);
}

// Test the "full" method.
TEST(FixedVector, Full)
{
    FixedVector<int, 2> vec;

    EXPECT_FALSE(vec.full());
    vec.push_back(0);
    EXPECT_FALSE(vec.full());
    vec.push_back(1);
    EXPECT_TRUE(vec.full());
}
}  // namespace angle
