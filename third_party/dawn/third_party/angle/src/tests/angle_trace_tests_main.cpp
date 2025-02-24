//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_perftests_main.cpp
//   Entry point for the gtest-based performance tests.
//

#include <gtest/gtest.h>

#include "test_utils/runner/TestSuite.h"

void ANGLEProcessTraceTestArgs(int *argc, char **argv);
void RegisterTraceTests();

int main(int argc, char **argv)
{
    ANGLEProcessTraceTestArgs(&argc, argv);
    RegisterTraceTests();
    angle::TestSuite testSuite(&argc, argv);
    return testSuite.run();
}
