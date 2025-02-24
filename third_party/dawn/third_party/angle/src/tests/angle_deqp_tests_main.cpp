//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_deqp_gtest_main:
//   Entry point for standalone dEQP tests.

#include <gtest/gtest.h>

#include "test_utils/runner/TestSuite.h"

// Defined in angle_deqp_gtest.cpp. Declared here so we don't need to make a header that we import
// in Chromium.
namespace angle
{
int RunGLCTSTests(int *argc, char **argv);
}  // namespace angle

int main(int argc, char **argv)
{
#if defined(ANGLE_PLATFORM_MACOS)
    // By default, we should hook file API functions on macOS to avoid slow Metal shader caching
    // file access.
    angle::InitMetalFileAPIHooking(argc, argv);
#endif

    return angle::RunGLCTSTests(&argc, argv);
}
