//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GLSLCompatibilityOutputTest.cpp
//   Test compiler output for glsl compatibility mode
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class GLSLCompatibilityOutputTest : public MatchOutputCodeTest
{
  public:
    GLSLCompatibilityOutputTest()
        : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_GLSL_COMPATIBILITY_OUTPUT)
    {}
};

// Verify gl_Position is written when compiling in compatibility mode
TEST_F(GLSLCompatibilityOutputTest, GLPositionWrittenTest)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main() {\n"
        "}";
    compile(shaderString);
    EXPECT_TRUE(foundInCode("gl_Position"));
}
