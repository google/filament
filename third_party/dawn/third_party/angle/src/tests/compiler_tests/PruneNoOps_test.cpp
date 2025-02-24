//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PruneNoOps_test.cpp:
//   Tests for pruning no-op statements.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class PruneNoOpsTest : public MatchOutputCodeTest
{
  public:
    PruneNoOpsTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_GLSL_COMPATIBILITY_OUTPUT) {}
};

// Test that a switch statement with a constant expression without a matching case is pruned.
TEST_F(PruneNoOpsTest, SwitchStatementWithConstantExpressionNoMatchingCase)
{
    const std::string shaderString = R"(#version 300 es
precision mediump float;
out vec4 color;

void main(void)
{
    switch (10)
    {
        case 0:
            color = vec4(0);
            break;
        case 1:
            color = vec4(1);
            break;
    }
})";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("switch"));
    ASSERT_TRUE(notFoundInCode("case"));
}

// Test that a switch statement with a constant expression with a default is not pruned.
TEST_F(PruneNoOpsTest, SwitchStatementWithConstantExpressionWithDefault)
{
    const std::string shaderString = R"(#version 300 es
precision mediump float;
out vec4 color;

void main(void)
{
    switch (10)
    {
        case 0:
            color = vec4(0);
            break;
        case 1:
            color = vec4(1);
            break;
        default:
            color = vec4(0.5);
            break;
    }
})";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("switch"));
    ASSERT_TRUE(foundInCode("case"));
}

// Test that a switch statement with a constant expression with a matching case is not pruned.
TEST_F(PruneNoOpsTest, SwitchStatementWithConstantExpressionWithMatchingCase)
{
    const std::string shaderString = R"(#version 300 es
precision mediump float;
out vec4 color;

void main(void)
{
    switch (10)
    {
        case 0:
            color = vec4(0);
            break;
        case 10:
            color = vec4(1);
            break;
    }
})";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("switch"));
    ASSERT_TRUE(foundInCode("case"));
}

}  // namespace
