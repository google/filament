//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RecordConstantPrecision_test.cpp:
//   Test for recording constant variable precision when it affects consuming expression.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class RecordConstantPrecisionTest : public MatchOutputCodeTest
{
  public:
    RecordConstantPrecisionTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT) {}
};

// The constant's precision must be specified if its precision is higher than the other operands,
// since it increases the precision of the consuming expression.
TEST_F(RecordConstantPrecisionTest, HigherPrecisionConstantAsParameter)
{
    const std::string &shaderString = R"(
uniform mediump float u;
void main()
{
    const highp float a = 4096.5;
    mediump float b = fract(a + u);
    gl_FragColor = vec4(b);
})";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("const highp float s"));
    ASSERT_FALSE(foundInCode("fract(4096.5"));
    ASSERT_FALSE(foundInCode("fract((4096.5"));
}

// The constant's precision does not need to be specified if its precision is equal to the other
// operands, as it does not increase the precision of the consuming expression.  For simplicity
// however, the constant's precision is specified anyway.
TEST_F(RecordConstantPrecisionTest, EqualPrecisionConstantAsParameter)
{
    const std::string &shaderString = R"(
uniform mediump float u;
void main()
{
    const mediump float a = 4096.5;
    mediump float b = fract(a + u);
    gl_FragColor = vec4(b);
})";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("const mediump float s"));
    ASSERT_FALSE(foundInCode("fract((4096.5"));
}

// The constant's precision must be specified if its precision is higher than the other operands,
// since it increases the precision of the consuming expression.  This applies also when the
// constant is part of a constant expression that can be folded.
TEST_F(RecordConstantPrecisionTest, FoldedBinaryConstantPrecisionIsHigher)
{
    const std::string &shaderString = R"(
uniform mediump float u;
void main()
{
    const highp float a = 4095.5;
    mediump float b = fract((a + 1.0) + u);
    gl_FragColor = vec4(b);
})";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("const highp float s"));
    ASSERT_FALSE(foundInCode("fract(4096.5"));
    ASSERT_FALSE(foundInCode("fract((4096.5"));
}

// The constant's precision must be specified if its precision is higher than the other operands,
// since it increases the precision of the consuming expression.  This applies also when the
// constant is part of a constant expression that can be folded.
TEST_F(RecordConstantPrecisionTest, FoldedUnaryConstantPrecisionIsHigher)
{
    const std::string &shaderString = R"(
uniform mediump float u;
void main()
{
    const highp float a = 0.5;
    mediump float b = sin(fract(a) + u);
    gl_FragColor = vec4(b);
})";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("const highp float s"));
    ASSERT_FALSE(foundInCode("sin(0.5"));
    ASSERT_FALSE(foundInCode("sin((0.5"));
}

// The constant's precision must be specified if its precision is higher than the other operands,
// since it increases the precision of the consuming expression.  This applies also when the
// constant is part of a constructor expression.  Note that lowp constants never need their
// precision specified.
TEST_F(RecordConstantPrecisionTest, HigherPrecisionConstantInConstructor)
{
    const std::string &shaderString = R"(
uniform mediump float u;
void main()
{
    const highp float a = 4096.5;
    const lowp float b = 1.0;
    lowp vec4 result = vec4(b, a, b, u);
    gl_FragColor = result;
})";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("const highp float s"));
    ASSERT_FALSE(foundInCode("const lowp float s"));
    ASSERT_TRUE(foundInCode("vec4(1.0, s"));
}

// The constant's precision does not need to be specified if its used to initialize a variable.
TEST_F(RecordConstantPrecisionTest, HigherPrecisionConstantInAssignment)
{
    const std::string &shaderString = R"(
uniform mediump float u;
void main()
{
    const highp float a = 4096.5;
    mediump float b = a;
    mediump float c;
    c = a;
    gl_FragColor = vec4(b, b, c, c);
})";
    compile(shaderString);
    ASSERT_FALSE(foundInCode("const highp float s"));
    ASSERT_TRUE(foundInCode("b = 4096.5"));
    ASSERT_TRUE(foundInCode("c = 4096.5"));
}

// The constant's precision does not need to be specified if its used as an index.
TEST_F(RecordConstantPrecisionTest, HigherPrecisionConstantInIndex)
{
    const std::string &shaderString = R"(
uniform mediump float u;
void main()
{
    const highp int a = 330;
    mediump float b[340];
    gl_FragColor = vec4(b[a]);
})";
    compile(shaderString);
    ASSERT_FALSE(foundInCode("const highp int s"));
    ASSERT_TRUE(foundInCode("b[330]"));
}
