//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AppendixALimitations_test.cpp:
//   Tests for validating ESSL 1.00 Appendix A limitations.
//

#include "gtest/gtest.h"

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class AppendixALimitationsTest : public ShaderCompileTreeTest
{
  public:
    AppendixALimitationsTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_WEBGL_SPEC; }
};

// Test an invalid shader where a for loop index is used as an out parameter.
TEST_F(AppendixALimitationsTest, IndexAsFunctionOutParameter)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void fun(out int a)\n"
        "{\n"
        "   a = 2;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    for (int i = 0; i < 2; ++i)\n"
        "    {\n"
        "        fun(i);\n"
        "    }\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test an invalid shader where a for loop index is used as an inout parameter.
TEST_F(AppendixALimitationsTest, IndexAsFunctionInOutParameter)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void fun(int b, inout int a)\n"
        "{\n"
        "   a += b;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    for (int i = 0; i < 2; ++i)\n"
        "    {\n"
        "        fun(2, i);\n"
        "    }\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test a valid shader where a for loop index is used as an in parameter in a function that also has
// an out parameter.
TEST_F(AppendixALimitationsTest, IndexAsFunctionInParameter)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void fun(int b, inout int a)\n"
        "{\n"
        "   a += b;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    for (int i = 0; i < 2; ++i)\n"
        "    {\n"
        "        int a = 1;"
        "        fun(i, a);\n"
        "    }\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test an invalid shader where a for loop index is used as a target of assignment.
TEST_F(AppendixALimitationsTest, IndexAsTargetOfAssignment)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    for (int i = 0; i < 2; ++i)\n"
        "    {\n"
        "        i = 2;\n"
        "    }\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test an invalid shader where a for loop index is incremented inside the loop.
TEST_F(AppendixALimitationsTest, IndexIncrementedInLoopBody)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    for (int i = 0; i < 2; ++i)\n"
        "    {\n"
        "        ++i;\n"
        "    }\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}
