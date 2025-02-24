//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unit tests for ANGLE's Optional helper class.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/Optional.h"

namespace
{

TEST(OptionalTest, BasicInvalid)
{
    Optional<int> testInvalid;
    ASSERT_FALSE(testInvalid.valid());
    ASSERT_EQ(Optional<int>::Invalid(), testInvalid);
}

TEST(OptionalTest, BasicValid)
{
    Optional<int> testValid(3);
    ASSERT_TRUE(testValid.valid());
    ASSERT_EQ(3, testValid.value());
    ASSERT_NE(Optional<int>::Invalid(), testValid);
}

TEST(OptionalTest, Copies)
{
    Optional<int> testValid(3);
    Optional<int> testInvalid;

    Optional<int> testCopy = testInvalid;
    ASSERT_FALSE(testCopy.valid());
    ASSERT_EQ(testInvalid, testCopy);

    testCopy = testValid;
    ASSERT_TRUE(testCopy.valid());
    ASSERT_EQ(3, testCopy.value());
    ASSERT_EQ(testValid, testCopy);
}

}  // namespace
