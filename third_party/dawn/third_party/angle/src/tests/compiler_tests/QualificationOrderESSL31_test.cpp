//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QualificationOrderESSL31_test.cpp:
//   OpenGL ES 3.1 removes the strict order of qualifiers imposed by the grammar.
//   This file contains tests for invalid order and usage of qualifiers in GLSL ES 3.10.

#include "gtest/gtest.h"

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class QualificationVertexShaderTestESSL31 : public ShaderCompileTreeTest
{
  public:
    QualificationVertexShaderTestESSL31() {}

  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }

    const TIntermSymbol *findSymbolInAST(const ImmutableString &symbolName)
    {
        return FindSymbolNode(mASTRoot, symbolName);
    }
};

// GLSL ES 3.10 has relaxed checks on qualifier order. Any order is correct.
TEST_F(QualificationVertexShaderTestESSL31, CentroidOut)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision lowp float;\n"
        "out centroid float something;\n"
        "void main(){\n"
        "   something = 1.0;\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success" << mInfoLog;
    }
    else
    {
        const TIntermSymbol *node = findSymbolInAST(ImmutableString("something"));
        ASSERT_NE(nullptr, node);

        const TType &type = node->getType();
        EXPECT_EQ(EvqCentroidOut, type.getQualifier());
    }
}

// GLSL ES 3.10 has relaxed checks on qualifier order. Any order is correct.
TEST_F(QualificationVertexShaderTestESSL31, AllQualifiersMixed)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision lowp float;\n"
        "highp out invariant centroid flat vec4 something;\n"
        "void main(){\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success" << mInfoLog;
    }
    else
    {
        const TIntermSymbol *node = findSymbolInAST(ImmutableString("something"));
        ASSERT_NE(nullptr, node);

        const TType &type = node->getType();
        EXPECT_TRUE(type.isInvariant());
        EXPECT_EQ(EvqFlatOut, type.getQualifier());
        EXPECT_EQ(EbpHigh, type.getPrecision());
    }
}

// GLSL ES 3.10 allows multiple layout qualifiers to be specified.
TEST_F(QualificationVertexShaderTestESSL31, MultipleLayouts)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision lowp float;\n"
        "in layout(location=1) layout(location=2) vec4 something;\n"
        "void main(){\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success" << mInfoLog;
    }
    else
    {
        const TIntermSymbol *node = findSymbolInAST(ImmutableString("something"));
        ASSERT_NE(nullptr, node);

        const TType &type = node->getType();
        EXPECT_EQ(EvqVertexIn, type.getQualifier());
        EXPECT_EQ(2, type.getLayoutQualifier().location);
    }
}

// The test checks layout qualifier overriding when multiple layouts are specified.
TEST_F(QualificationVertexShaderTestESSL31, MultipleLayoutsInterfaceBlock)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision lowp float;\n"
        "out float someValue;\n"
        "layout(shared) layout(std140) layout(column_major) uniform MyInterface\n"
        "{ vec4 something; } MyInterfaceName;\n"
        "void main(){\n"
        "   someValue = MyInterfaceName.something.r;\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success" << mInfoLog;
    }
    else
    {
        const TIntermSymbol *node = findSymbolInAST(ImmutableString("MyInterfaceName"));
        ASSERT_NE(nullptr, node);

        const TType &type                = node->getType();
        TLayoutQualifier layoutQualifier = type.getLayoutQualifier();
        EXPECT_EQ(EbsStd140, layoutQualifier.blockStorage);
        EXPECT_EQ(EmpColumnMajor, layoutQualifier.matrixPacking);
    }
}

// The test checks layout qualifier overriding when multiple layouts are specified.
TEST_F(QualificationVertexShaderTestESSL31, MultipleLayoutsInterfaceBlock2)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision lowp float;\n"
        "out float someValue;\n"
        "layout(row_major) layout(std140) layout(shared) uniform MyInterface\n"
        "{ vec4 something; } MyInterfaceName;\n"
        "void main(){\n"
        "   someValue = MyInterfaceName.something.r;\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success" << mInfoLog;
    }
    else
    {
        const TIntermSymbol *node = findSymbolInAST(ImmutableString("MyInterfaceName"));
        ASSERT_NE(nullptr, node);

        const TType &type                = node->getType();
        TLayoutQualifier layoutQualifier = type.getLayoutQualifier();
        EXPECT_EQ(EbsShared, layoutQualifier.blockStorage);
        EXPECT_EQ(EmpRowMajor, layoutQualifier.matrixPacking);
    }
}
