//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QualificationOrder_test.cpp:
//   OpenGL ES 3.1 removes the strict order of qualifiers imposed by the grammar.
//   This file contains tests for invalid order and usage of qualifiers.

#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class QualificationOrderFragmentShaderTest : public ShaderCompileTreeTest
{
  public:
    QualificationOrderFragmentShaderTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->MaxDrawBuffers = (getShaderSpec() == SH_GLES2_SPEC) ? 1 : 8;
    }

    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }

    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

class QualificationOrderVertexShaderTest : public QualificationOrderFragmentShaderTest
{
  public:
    QualificationOrderVertexShaderTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
};

// Repeating centroid qualifier is invalid.
TEST_F(QualificationOrderFragmentShaderTest, RepeatingCentroid)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "flat centroid centroid in float myValue;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Repeating uniform storage qualifiers is invalid.
TEST_F(QualificationOrderFragmentShaderTest, RepeatingUniforms)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform uniform float myValue;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Repeating varying storage qualifiers is invalid.
TEST_F(QualificationOrderFragmentShaderTest, RepeatingVaryings)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "varying varying vec4 myColor;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Layout qualifier should be before the storage qualifiers.
TEST_F(QualificationOrderFragmentShaderTest, WrongOrderQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out layout(location=1) vec4 myColor;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Centroid out is the correct order. Out centroid is incorrect.
TEST_F(QualificationOrderVertexShaderTest, WrongOrderCentroidOut)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 uv;\n"
        "out centroid vec4 position;\n"
        "void main() {\n"
        "position = uv;\n"
        "gl_Position = uv;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Centroid in is the correct order. In centroid is incorrect.
TEST_F(QualificationOrderFragmentShaderTest, WrongOrderCentroidIn)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in centroid vec4 colorIN;\n"
        "out vec4 colorOUT;\n"
        "void main() {\n"
        "colorOUT = colorIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Type cannot be before the storage qualifier.
TEST_F(QualificationOrderFragmentShaderTest, WrongOrderTypeStorage)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "centroid in vec4 colorIN;\n"
        "vec4 out colorOUT;\n"
        "void main() {\n"
        "colorOUT = colorIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have two conflicting storage qualifiers.
TEST_F(QualificationOrderFragmentShaderTest, RepeatingDifferentStorageQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "centroid in vec4 colorIN;\n"
        "uniform out vec4 colorOUT;\n"
        "void main() {\n"
        "colorOUT = colorIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have two different layout qualifiers.
TEST_F(QualificationOrderFragmentShaderTest, RepeatingLayoutQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 colorIN;\n"
        "layout(location=0) layout(location=0) out vec4 colorOUT;\n"
        "void main() {\n"
        "colorOUT = colorIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have repeating invariant qualifiers.
TEST_F(QualificationOrderFragmentShaderTest, RepeatingInvariantQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 colorIN;\n"
        "invariant invariant out vec4 colorOUT;\n"
        "void main() {\n"
        "colorOUT = colorIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have repeating storage qualifiers.
TEST_F(QualificationOrderVertexShaderTest, RepeatingAttributes)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "attribute attribute vec4 positionIN;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Wrong order for invariant varying. It should be 'invariant varying', not 'varying invariant'.
TEST_F(QualificationOrderVertexShaderTest, VaryingInvariantWrongOrder)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "attribute vec4 positionIN;\n"
        "varying invariant vec4 dataOUT;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "dataOUT = 0.5 * dataOUT + vec4(0.5);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have repeating storage qualifiers.
TEST_F(QualificationOrderVertexShaderTest, AttributeVaryingMix)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "attribute varying vec4 positionIN;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have repeating storage qualifiers.
TEST_F(QualificationOrderVertexShaderTest, VaryingAttributeMix)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "varying attribute vec4 positionIN;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have repeating interpolation qualifiers.
TEST_F(QualificationOrderVertexShaderTest, RepeatingInterpolationQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 positionIN;\n"
        "flat flat out vec4 dataOUT;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "dataOUT = 0.5 * dataOUT + vec4(0.5);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Wrong order for the interpolation and storage qualifier. The correct order is interpolation
// qualifier and then storage qualifier.
TEST_F(QualificationOrderVertexShaderTest, WrongOrderInterpolationStorageQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 positionIN;\n"
        "out flat vec4 dataOUT;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "dataOUT = 0.5 * dataOUT + vec4(0.5);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// The correct order is invariant, interpolation, storage.
TEST_F(QualificationOrderVertexShaderTest, WrongOrderInvariantInterpolationStorageQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 positionIN;\n"
        "flat invariant out vec4 dataOUT;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "dataOUT = 0.5 * dataOUT + vec4(0.5);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// The invariant qualifer has to be before the storage qualifiers.
TEST_F(QualificationOrderVertexShaderTest, WrongOrderInvariantNotFirst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 positionIN;\n"
        "centroid out invariant vec4 dataOUT;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "dataOUT = 0.5 * dataOUT + vec4(0.5);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// The precision qualifier is after the storage qualifiers.
TEST_F(QualificationOrderVertexShaderTest, WrongOrderPrecision)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 positionIN;\n"
        "highp centroid out vec4 dataOUT;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "dataOUT = 0.5 * dataOUT + vec4(0.5);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have multiple declarations of the 'in' storage qualifier.
TEST_F(QualificationOrderVertexShaderTest, RepeatingInQualifier)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in in vec4 positionIN;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// A variable cannot have multiple declarations of the 'attribute' storage qualifier.
TEST_F(QualificationOrderVertexShaderTest, RepeatingAttributeQualifier)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "attribute attribute vec4 positionIN;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Vertex input cannot be qualified with invariant.
TEST_F(QualificationOrderVertexShaderTest, InvariantVertexInput)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "invariant attribute vec4 positionIN;\n"
        "void main() {\n"
        "gl_Position = positionIN;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Cannot have a function parameter with the invariant qualifier.
TEST_F(QualificationOrderFragmentShaderTest, InvalidFunctionParametersInvariant)
{
    const std::string &shaderString =
        "precision lowp float;\n"
        "varying float value;\n"
        "float foo0 (invariant in float x) {\n"
        "   return 2.0*x;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "   gl_FragColor = vec4(foo0(value));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure" << mInfoLog;
    }
}

// Cannot have a function parameter with the attribute qualifier.
TEST_F(QualificationOrderFragmentShaderTest, InvalidFunctionParametersAttribute)
{
    const std::string &shaderString =
        "precision lowp float;\n"
        "varying float value;\n"
        "float foo0 (attribute float x) {\n"
        "   return 2.0*x;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "   gl_FragColor = vec4(foo0(value));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure" << mInfoLog;
    }
}

// Cannot have a function parameter with the varying qualifier.
TEST_F(QualificationOrderFragmentShaderTest, InvalidFunctionParametersVarying)
{
    const std::string &shaderString =
        "precision lowp float;\n"
        "varying float value;\n"
        "float foo0 (varying float x) {\n"
        "   return 2.0*x;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "   gl_FragColor = vec4(foo0(value));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure" << mInfoLog;
    }
}

// Cannot have a function parameter with the layout qualifier
TEST_F(QualificationOrderFragmentShaderTest, InvalidFunctionParametersLayout)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision lowp float;\n"
        "in float value;\n"
        "float foo0 (layout(location = 3) in float x) {\n"
        "   return 2.0*x;\n"
        "}\n"
        "out vec4 colorOUT;\n"
        "void main()\n"
        "{\n"
        "   colorOUT = vec4(foo0(value));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure" << mInfoLog;
    }
}

// Cannot have a function parameter with the centroid qualifier
TEST_F(QualificationOrderFragmentShaderTest, InvalidFunctionParametersCentroidIn)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision lowp float;\n"
        "in float value;\n"
        "float foo0 (centroid in float x) {\n"
        "   return 2.0*x;\n"
        "}\n"
        "out vec4 colorOUT;\n"
        "void main()\n"
        "{\n"
        "   colorOUT = vec4(foo0(value));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure" << mInfoLog;
    }
}

// Cannot have a function parameter with the flat qualifier
TEST_F(QualificationOrderFragmentShaderTest, InvalidFunctionParametersFlatIn)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision lowp float;\n"
        "in float value;\n"
        "float foo0 (flat in float x) {\n"
        "   return 2.0*x;\n"
        "}\n"
        "out vec4 colorOUT;\n"
        "void main()\n"
        "{\n"
        "   colorOUT = vec4(foo0(value));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure" << mInfoLog;
    }
}

// Output layout location qualifier can't appear more than once within a declaration.
// GLSL ES 3.00.6 section 4.3.8.2 Output Layout Qualifiers.
TEST_F(QualificationOrderFragmentShaderTest, TwoOutputLocations)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location=1, location=2) out vec4 myColor;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}
