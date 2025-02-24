//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// system_utils_unittest_helper.cpp: Helper to the SystemUtils.RunApp unittest

#include "test_utils_unittest_helper.h"

#include "../src/tests/test_utils/runner/TestSuite.h"
#include "common/system_utils.h"

#include <string.h>

// This variable is also defined in angle_unittest_main.
bool gVerbose = false;

int main(int argc, char **argv)
{
    bool runTestSuite = false;

    for (int argIndex = 1; argIndex < argc; ++argIndex)
    {
        if (strcmp(argv[argIndex], kRunTestSuite) == 0)
        {
            runTestSuite = true;
        }
    }

    if (runTestSuite)
    {
        angle::TestSuite testSuite(&argc, argv);
        return testSuite.run();
    }

    if (argc != 3 || strcmp(argv[1], kRunAppTestArg1) != 0 || strcmp(argv[2], kRunAppTestArg2) != 0)
    {
        fprintf(stderr, "Expected command line:\n%s %s %s\n", argv[0], kRunAppTestArg1,
                kRunAppTestArg2);
        return EXIT_FAILURE;
    }

    std::string env = angle::GetEnvironmentVar(kRunAppTestEnvVarName);
    if (env == "")
    {
        printf("%s", kRunAppTestStdout);
        fflush(stdout);
        fprintf(stderr, "%s", kRunAppTestStderr);
    }
    else
    {
        fprintf(stderr, "%s", env.c_str());
    }

    return EXIT_SUCCESS;
}
