//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// UnlockedTailCall_unittest.cpp: Unit tests of the UnlockedTailCall class.

#include <gtest/gtest.h>

#include "libANGLE/angletypes.h"

namespace angle
{
void SetUpTailCall(UnlockedTailCall *unlockedTailCall, int *result)
{
    unlockedTailCall->add([result](void *resultOut) {
        (void)resultOut;
        ++*result;
    });
}

// Test basic functionality
TEST(UnlockedTailCall, Basic)
{
    int a = 10;
    int b = 500;

    UnlockedTailCall unlockedTailCall;
    ASSERT_FALSE(unlockedTailCall.any());

    SetUpTailCall(&unlockedTailCall, &a);
    ASSERT_TRUE(unlockedTailCall.any());

    SetUpTailCall(&unlockedTailCall, &b);
    ASSERT_TRUE(unlockedTailCall.any());

    unlockedTailCall.run(nullptr);
    ASSERT_EQ(a, 11);
    ASSERT_EQ(b, 501);
}
}  // namespace angle
