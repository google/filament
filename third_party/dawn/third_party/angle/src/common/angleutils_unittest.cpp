//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angleutils_unittest.cpp: Unit tests for ANGLE's common utilities.

#include "gtest/gtest.h"

#include "common/angleutils.h"

namespace
{

// Test that multiple array indices are written out in the right order.
TEST(ArrayIndexString, MultipleArrayIndices)
{
    std::vector<unsigned int> indices;
    indices.push_back(12);
    indices.push_back(34);
    indices.push_back(56);
    EXPECT_EQ("[56][34][12]", ArrayIndexString(indices));
}

}  // anonymous namespace
