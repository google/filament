//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PruneUnusedFunctions_test.cpp:
//   Test for the pruning of unused functions
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class PruneUnusedFunctionsTest : public MatchOutputCodeTest
{
  public:
    PruneUnusedFunctionsTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT) {}

  protected:
    void compile(const std::string &shaderString)
    {
        ShCompileOptions compileOptions = {};

        MatchOutputCodeTest::compile(shaderString, compileOptions);
    }
};

// Check that unused function and prototypes are removed iff the options is set
TEST_F(PruneUnusedFunctionsTest, UnusedFunctionAndProto)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float unused(float a);\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1.0);\n"
        "}\n"
        "float unused(float a) {\n"
        "    return a;\n"
        "}\n";
    compile(shaderString);
    EXPECT_TRUE(notFoundInCode("unused("));
    EXPECT_TRUE(foundInCode("main(", 1));
}

// Check that unimplemented prototypes are removed iff the options is set
TEST_F(PruneUnusedFunctionsTest, UnimplementedPrototype)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float unused(float a);\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1.0);\n"
        "}\n";
    compile(shaderString);
    EXPECT_TRUE(notFoundInCode("unused("));
    EXPECT_TRUE(foundInCode("main(", 1));
}

// Check that used functions are not pruned (duh)
TEST_F(PruneUnusedFunctionsTest, UsedFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float used(float a);\n"
        "void main() {\n"
        "    gl_FragColor = vec4(used(1.0));\n"
        "}\n"
        "float used(float a) {\n"
        "    return a;\n"
        "}\n";
    compile(shaderString);
    EXPECT_TRUE(foundInCode("used(", 3));
    EXPECT_TRUE(foundInCode("main(", 1));
}

}  // namespace
