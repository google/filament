//
// Copyright 2025 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unit tests for ANGLE's MemoryBuffer class.
//

#include "common/MemoryBuffer.h"

#include <gtest/gtest.h>

using namespace angle;

namespace
{

// Test usage of MemoryBuffer with multiple resizes
TEST(MemoryBufferTest, MultipleResizes)
{
    MemoryBuffer buffer;

    ASSERT_TRUE(buffer.resize(100));
    ASSERT_EQ(buffer.size(), 100u);
    buffer.assertTotalAllocatedBytes(100u);
    buffer.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(buffer.resize(300));
    ASSERT_EQ(buffer.size(), 300u);
    buffer.assertTotalAllocatedBytes(400u);
    buffer.assertTotalCopiedBytes(100u);

    ASSERT_TRUE(buffer.resize(100));
    ASSERT_EQ(buffer.size(), 100u);
    buffer.assertTotalAllocatedBytes(400u);
    buffer.assertTotalCopiedBytes(100u);

    ASSERT_TRUE(buffer.resize(400));
    ASSERT_EQ(buffer.size(), 400u);
    buffer.assertTotalAllocatedBytes(800u);
    buffer.assertTotalCopiedBytes(200u);
}

// Test usage of MemoryBuffer with reserve and then multiple resizes
TEST(MemoryBufferTest, ReserveThenResize)
{
    MemoryBuffer buffer;

    ASSERT_TRUE(buffer.reserve(300));
    ASSERT_EQ(buffer.size(), 0u);

    ASSERT_TRUE(buffer.resize(100));
    ASSERT_EQ(buffer.size(), 100u);
    buffer.assertTotalAllocatedBytes(300u);
    buffer.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(buffer.resize(300));
    ASSERT_EQ(buffer.size(), 300u);
    buffer.assertTotalAllocatedBytes(300u);
    buffer.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(buffer.resize(100));
    ASSERT_EQ(buffer.size(), 100u);
    buffer.assertTotalAllocatedBytes(300u);
    buffer.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(buffer.resize(400));
    ASSERT_EQ(buffer.size(), 400u);
    buffer.assertTotalAllocatedBytes(700u);
    buffer.assertTotalCopiedBytes(100u);
}

// Test usage of MemoryBuffer with clearAndReserve and then multiple resizes
TEST(MemoryBufferTest, ClearAndReserveThenResize)
{
    MemoryBuffer buffer;

    ASSERT_TRUE(buffer.clearAndReserve(300));
    ASSERT_EQ(buffer.size(), 0u);

    ASSERT_TRUE(buffer.resize(100));
    ASSERT_EQ(buffer.size(), 100u);
    buffer.assertTotalAllocatedBytes(300u);
    buffer.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(buffer.resize(300));
    ASSERT_EQ(buffer.size(), 300u);
    buffer.assertTotalAllocatedBytes(300u);
    buffer.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(buffer.resize(100));
    ASSERT_EQ(buffer.size(), 100u);
    buffer.assertTotalAllocatedBytes(300u);
    buffer.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(buffer.clearAndReserve(400));
    ASSERT_EQ(buffer.size(), 0u);

    ASSERT_TRUE(buffer.resize(400));
    ASSERT_EQ(buffer.size(), 400u);
    buffer.assertTotalAllocatedBytes(700u);
    buffer.assertTotalCopiedBytes(0u);
}

// Test appending and destroying MemoryBuffer
TEST(MemoryBufferTest, AppendAndDestroy)
{
    MemoryBuffer bufferSrc;
    MemoryBuffer bufferDst;

    ASSERT_TRUE(bufferSrc.clearAndReserve(100));
    ASSERT_EQ(bufferSrc.size(), 0u);

    ASSERT_TRUE(bufferSrc.resize(100));
    ASSERT_EQ(bufferSrc.size(), 100u);
    bufferSrc.assertTotalAllocatedBytes(100u);
    bufferSrc.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(bufferDst.clearAndReserve(200));
    ASSERT_EQ(bufferDst.size(), 0u);

    ASSERT_TRUE(bufferDst.resize(100));
    ASSERT_EQ(bufferDst.size(), 100u);
    ASSERT_TRUE(bufferDst.append(bufferSrc));
    ASSERT_EQ(bufferDst.size(), 200u);
    bufferDst.assertTotalAllocatedBytes(200u);
    bufferDst.assertTotalCopiedBytes(0u);

    ASSERT_TRUE(bufferDst.append(bufferSrc));
    ASSERT_EQ(bufferDst.size(), 300u);
    bufferDst.assertTotalAllocatedBytes(500u);
    bufferDst.assertTotalCopiedBytes(200u);

    ASSERT_TRUE(bufferDst.append(bufferDst));
    ASSERT_EQ(bufferDst.size(), 600u);
    bufferDst.assertTotalAllocatedBytes(1100u);
    bufferDst.assertTotalCopiedBytes(500u);

    bufferDst.destroy();
    ASSERT_EQ(bufferDst.size(), 0u);
    bufferDst.assertTotalAllocatedBytes(0u);
    bufferDst.assertTotalCopiedBytes(0u);
}

}  // namespace
