//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// serial_utils_unittest:
//   Unit tests for the Serial utils.
//

#include <gtest/gtest.h>

#include "libANGLE/renderer/serial_utils.h"

namespace rx
{
TEST(SerialTest, Monotonic)
{
    AtomicSerialFactory factory;

    Serial a = factory.generate();
    Serial b = factory.generate();

    EXPECT_GT(b, a);

    Serial zero;

    EXPECT_GT(a, zero);
    EXPECT_GT(b, zero);
}
}  // namespace rx
