//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PruneEmptyCases_test.cpp:
//   Tests for pruning empty cases and switch statements. This ensures that the translator doesn't
//   produce switch statements where the last case statement is not followed by anything.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class PruneEmptyCasesTest : public MatchOutputCodeTest
{
  public:
    PruneEmptyCasesTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_GLSL_COMPATIBILITY_OUTPUT) {}
};

// Test that a switch statement that only contains no-ops is pruned entirely.
TEST_F(PruneEmptyCasesTest, SwitchStatementWithOnlyNoOps)
{
    const std::string shaderString =
        R"(#version 300 es

        uniform int ui;

        void main(void)
        {
            int i = ui;
            switch (i)
            {
                case 0:
                case 1:
                    { {} }
                    int j;
                    1;
            }
        })";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("switch"));
    ASSERT_TRUE(notFoundInCode("case"));
}

// Test that a init statement that has a side effect is preserved even if the switch is pruned.
TEST_F(PruneEmptyCasesTest, SwitchStatementWithOnlyNoOpsAndInitWithSideEffect)
{
    const std::string shaderString =
        R"(#version 300 es

        precision mediump float;
        out vec4 my_FragColor;

        uniform int uni_i;

        void main(void)
        {
            int i = uni_i;
            switch (++i)
            {
                case 0:
                case 1:
                    { {} }
                    int j;
                    1;
            }
            my_FragColor = vec4(i);
        })";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("switch"));
    ASSERT_TRUE(notFoundInCode("case"));
    ASSERT_TRUE(foundInCode("++_ui"));
}

// Test a switch statement where the last case only contains no-ops.
TEST_F(PruneEmptyCasesTest, SwitchStatementLastCaseOnlyNoOps)
{
    const std::string shaderString =
        R"(#version 300 es

        precision mediump float;
        out vec4 my_FragColor;

        uniform int ui;

        void main(void)
        {
            int i = ui;
            switch (i)
            {
                case 0:
                    my_FragColor = vec4(0);
                    break;
                case 1:
                case 2:
                    { {} }
                    int j;
                    1;
            }
        })";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("switch"));
    ASSERT_TRUE(foundInCode("case", 1));
}

}  // namespace
