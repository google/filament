//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// aligned_memory_unittests: Tests for the aligned memory allocator.
//   Tests copied from Chrome: src/base/memory/aligned_memory_unittests.cc.
//

#include "common/aligned_memory.h"

#include <memory>

#include "gtest/gtest.h"

#define EXPECT_ALIGNED(ptr, align) EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(ptr) & (align - 1))

namespace angle
{

// Test that dynamic allocation works as expected.
TEST(AlignedMemoryTest, DynamicAllocation)
{
    void *p = AlignedAlloc(8, 8);
    EXPECT_TRUE(p);
    EXPECT_ALIGNED(p, 8);
    AlignedFree(p);

    p = AlignedAlloc(8, 16);
    EXPECT_TRUE(p);
    EXPECT_ALIGNED(p, 16);
    AlignedFree(p);

    p = AlignedAlloc(8, 256);
    EXPECT_TRUE(p);
    EXPECT_ALIGNED(p, 256);
    AlignedFree(p);

    p = AlignedAlloc(8, 4096);
    EXPECT_TRUE(p);
    EXPECT_ALIGNED(p, 4096);
    AlignedFree(p);
}

}  // namespace angle
