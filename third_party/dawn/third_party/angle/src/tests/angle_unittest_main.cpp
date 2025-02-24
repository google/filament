//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "GLSLANG/ShaderLang.h"
#include "gtest/gtest.h"
#include "test_utils/runner/TestSuite.h"

class CompilerTestEnvironment : public testing::Environment
{
  public:
    void SetUp() override
    {
        if (!sh::Initialize())
        {
            FAIL() << "Failed to initialize the compiler.";
        }
    }

    void TearDown() override
    {
        if (!sh::Finalize())
        {
            FAIL() << "Failed to finalize the compiler.";
        }
    }
};

// This variable is also defined in test_utils_unittest_helper.
bool gVerbose = false;

int main(int argc, char **argv)
{
    for (int argIndex = 1; argIndex < argc; ++argIndex)
    {
        if (strcmp(argv[argIndex], "-v") == 0 || strcmp(argv[argIndex], "--verbose") == 0)
        {
            gVerbose = true;
        }
    }

    angle::TestSuite testSuite(&argc, argv);
    testing::AddGlobalTestEnvironment(new CompilerTestEnvironment());
    return testSuite.run();
}
