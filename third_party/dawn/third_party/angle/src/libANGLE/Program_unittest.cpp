//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unit tests for Program and related classes.
//

#include <gtest/gtest.h>

#include "libANGLE/Program.h"

using namespace gl;

namespace
{

// Tests that the log length properly counts the terminating \0.
TEST(InfoLogTest, LogLengthCountsTerminator)
{
    InfoLog infoLog;
    EXPECT_EQ(0u, infoLog.getLength());
    infoLog << " ";

    // " \n\0" = 3 characters
    EXPECT_EQ(3u, infoLog.getLength());
}

// Tests that the log doesn't append newlines to an empty string
TEST(InfoLogTest, InfoLogEmptyString)
{
    InfoLog infoLog;
    EXPECT_EQ(0u, infoLog.getLength());
    infoLog << "";

    // "" = 3 characters
    EXPECT_EQ(0u, infoLog.getLength());
}

// Tests that newlines get appended to the info log properly.
TEST(InfoLogTest, AppendingNewline)
{
    InfoLog infoLog;

    infoLog << "First" << 1 << 'x';
    infoLog << "Second" << 2 << 'y';

    std::string expected = "First1x\nSecond2y\n";

    EXPECT_EQ(expected, infoLog.str());
}

}  // namespace
