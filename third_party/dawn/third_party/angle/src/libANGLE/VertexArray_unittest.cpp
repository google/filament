//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unit tests for VertexArray and related classes.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/bitset_utils.h"
#include "common/utilities.h"
#include "libANGLE/VertexArray.h"

using namespace gl;

// Tests that function GetIndexFromDirtyBit computes the index properly.
TEST(VertexArrayTest, VerifyGetIndexFromDirtyBit)
{
    VertexArray::DirtyBits dirtyBits;
    constexpr size_t bits[] = {2, 4, 9, 16, 25, 35};
    constexpr GLint count   = sizeof(bits) / sizeof(size_t);
    for (GLint i = 0; i < count; i++)
    {
        dirtyBits.set(bits[i]);
    }

    for (size_t dirtyBit : dirtyBits)
    {
        const size_t index = VertexArray::GetVertexIndexFromDirtyBit(dirtyBit);
        if (dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_0)
        {
            continue;
        }
        else if (dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX)
        {
            EXPECT_EQ(dirtyBit - VertexArray::DIRTY_BIT_ATTRIB_0, index);
        }
        else if (dirtyBit < VertexArray::DIRTY_BIT_BINDING_MAX)
        {
            EXPECT_EQ(dirtyBit - VertexArray::DIRTY_BIT_BINDING_0, index);
        }
        else if (dirtyBit < VertexArray::DIRTY_BIT_BUFFER_DATA_MAX)
        {
            EXPECT_EQ(dirtyBit - VertexArray::DIRTY_BIT_BUFFER_DATA_0, index);
        }
        else
            ASSERT_TRUE(false);
    }
}
