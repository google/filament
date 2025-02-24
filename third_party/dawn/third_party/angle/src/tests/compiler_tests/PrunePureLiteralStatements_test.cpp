//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PrunePureLiteralStatements_test.cpp:
//   Tests for pruning literal statements.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class PrunePureLiteralStatementsTest : public MatchOutputCodeTest
{
  public:
    // The PrunePureLiteralStatements pass is used when outputting ESSL
    PrunePureLiteralStatementsTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT) {}
};

// Most basic test for the pruning
TEST_F(PrunePureLiteralStatementsTest, FloatLiteralStatement)
{
    const std::string shaderString =
        R"(precision mediump float;
        void main()
        {
           float f = 41.0;
           42.0;
           gl_FragColor = vec4(f);
        })";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("41"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test the pruning works for constructed types too
TEST_F(PrunePureLiteralStatementsTest, ConstructorLiteralStatement)
{
    const std::string shaderString =
        R"(precision mediump float;
        void main()
        {
            vec2 f = vec2(41.0, 41.0);
            vec2(42.0, 42.0);
            gl_FragColor = vec4(f, 0.0, 0.0);
        })";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("41"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test the pruning works when the literal is a (non-trivial) expression
TEST_F(PrunePureLiteralStatementsTest, ExpressionLiteralStatement)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "   vec2(21.0, 21.0) + vec2(21.0, 21.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("21"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test that the pruning happens in the for-loop expression too
TEST_F(PrunePureLiteralStatementsTest, ForLoopLiteralExpression)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    for (;; vec2(42.0, 42.0)) {}\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test that the pruning correctly handles the pruning inside switch statements - for a switch with
// one empty case.
TEST_F(PrunePureLiteralStatementsTest, SwitchLiteralExpressionEmptyCase)
{
    const std::string shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    switch(1)\n"
        "    {\n"
        "      default:\n"
        "        42;\n"
        "    }\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("default"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test that the pruning correctly handles the pruning inside switch statements - for a switch with
// multiple cases.
TEST_F(PrunePureLiteralStatementsTest, SwitchLiteralExpressionEmptyCases)
{
    const std::string shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    switch(1)\n"
        "    {\n"
        "      case 1:\n"
        "      case 2:\n"
        "      default:\n"
        "        42;\n"
        "    }\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("default"));
    ASSERT_TRUE(notFoundInCode("case"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test that the pruning correctly handles the pruning inside switch statements - only cases at the
// end are deleted
TEST_F(PrunePureLiteralStatementsTest, SwitchLiteralExpressionOnlyLastCase)
{
    const std::string shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    switch(1)\n"
        "    {\n"
        "      case 1:\n"
        "      default:\n"
        "        42;\n"
        "        break;\n"
        "    }\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("default"));
    ASSERT_TRUE(foundInCode("case"));
    ASSERT_TRUE(notFoundInCode("42"));
}

// Test that the pruning correctly handles the pruning inside switch statements - pruning isn't
// stopped by literal statements
TEST_F(PrunePureLiteralStatementsTest, SwitchLiteralExpressionLiteralDoesntStop)
{
    const std::string shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    switch(1)\n"
        "    {\n"
        "      case 1:\n"
        "        42;\n"
        "      case 2:\n"
        "        43;\n"
        "    }\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("case"));
    ASSERT_TRUE(notFoundInCode("42"));
    ASSERT_TRUE(notFoundInCode("43"));
}

}  // namespace
