//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstantFoldingTest.cpp:
//   Utilities for constant folding tests.
//

#include "tests/test_utils/ConstantFoldingTest.h"

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/glsl/TranslatorESSL.h"

using namespace sh;

void ConstantFoldingExpressionTest::evaluate(const std::string &type, const std::string &expression)
{
    // We first assign the expression into a const variable so we can also verify that it gets
    // qualified as a constant expression. We then assign that constant expression into my_FragColor
    // to make sure that the value is not pruned.
    std::stringstream shaderStream;
    shaderStream << "#version 310 es\n"
                    "precision mediump float;\n"
                 << "out " << type << " my_FragColor;\n"
                 << "void main()\n"
                    "{\n"
                 << "    const " << type << " v = " << expression << ";\n"
                 << "    my_FragColor = v;\n"
                    "}\n";
    compileAssumeSuccess(shaderStream.str());
}

void ConstantFoldingExpressionTest::evaluateIvec4(const std::string &ivec4Expression)
{
    evaluate("ivec4", ivec4Expression);
}

void ConstantFoldingExpressionTest::evaluateVec4(const std::string &ivec4Expression)
{
    evaluate("vec4", ivec4Expression);
}

void ConstantFoldingExpressionTest::evaluateFloat(const std::string &floatExpression)
{
    evaluate("float", floatExpression);
}

void ConstantFoldingExpressionTest::evaluateInt(const std::string &intExpression)
{
    evaluate("int", intExpression);
}

void ConstantFoldingExpressionTest::evaluateUint(const std::string &uintExpression)
{
    evaluate("uint", uintExpression);
}
