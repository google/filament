//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConstantFolding_test.cpp:
//   Tests for constant folding
//

#include "tests/test_utils/ConstantFoldingTest.h"

using namespace sh;

// Test that zero, true or false are not found in AST when they are not expected. This is to make
// sure that the subsequent tests run correctly.
TEST_F(ConstantFoldingExpressionTest, FoldFloatTestCheck)
{
    const std::string &floatString = "1.0";
    evaluateFloat(floatString);
    ASSERT_FALSE(constantFoundInAST(0.0f));
    ASSERT_FALSE(constantFoundInAST(true));
    ASSERT_FALSE(constantFoundInAST(false));
}

TEST_F(ConstantFoldingTest, FoldIntegerAdd)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 + 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    ASSERT_TRUE(constantFoundInAST(1129));
}

TEST_F(ConstantFoldingTest, FoldIntegerSub)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 - 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    ASSERT_TRUE(constantFoundInAST(1119));
}

TEST_F(ConstantFoldingTest, FoldIntegerMul)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 * 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    ASSERT_TRUE(constantFoundInAST(5620));
}

TEST_F(ConstantFoldingTest, FoldIntegerDiv)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 / 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    // Rounding mode of division is undefined in the spec but ANGLE can be expected to round down.
    ASSERT_TRUE(constantFoundInAST(224));
}

TEST_F(ConstantFoldingTest, FoldIntegerModulus)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out int my_Int;\n"
        "void main() {\n"
        "   const int i = 1124 % 5;\n"
        "   my_Int = i;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_FALSE(constantFoundInAST(1124));
    ASSERT_FALSE(constantFoundInAST(5));
    ASSERT_TRUE(constantFoundInAST(4));
}

TEST_F(ConstantFoldingTest, FoldVectorCrossProduct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec3 my_Vec3;"
        "void main() {\n"
        "   const vec3 v3 = cross(vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, -1.0f, 1.0f));\n"
        "   my_Vec3 = v3;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    std::vector<float> input1(3, 1.0f);
    ASSERT_FALSE(constantVectorFoundInAST(input1));
    std::vector<float> input2;
    input2.push_back(1.0f);
    input2.push_back(-1.0f);
    input2.push_back(1.0f);
    ASSERT_FALSE(constantVectorFoundInAST(input2));
    std::vector<float> result;
    result.push_back(2.0f);
    result.push_back(0.0f);
    result.push_back(-2.0f);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}

// FoldMxNMatrixInverse tests check if the matrix 'inverse' operation
// on MxN matrix is constant folded when argument is constant expression and also
// checks the correctness of the result returned by the constant folding operation.
// All the matrices including matrices in the shader code are in column-major order.
TEST_F(ConstantFoldingTest, Fold2x2MatrixInverse)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in float i;\n"
        "out vec2 my_Vec;\n"
        "void main() {\n"
        "   const mat2 m2 = inverse(mat2(2.0f, 3.0f,\n"
        "                                5.0f, 7.0f));\n"
        "   mat2 m = m2 * mat2(i);\n"
        "   my_Vec = m[0];\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float inputElements[] = {2.0f, 3.0f, 5.0f, 7.0f};
    std::vector<float> input(inputElements, inputElements + 4);
    ASSERT_FALSE(constantColumnMajorMatrixFoundInAST(input));
    float outputElements[] = {-7.0f, 3.0f, 5.0f, -2.0f};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Check if the matrix 'inverse' operation on 3x3 matrix is constant folded.
TEST_F(ConstantFoldingTest, Fold3x3MatrixInverse)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in float i;\n"
        "out vec3 my_Vec;\n"
        "void main() {\n"
        "   const mat3 m3 = inverse(mat3(11.0f, 13.0f, 19.0f,\n"
        "                                23.0f, 29.0f, 31.0f,\n"
        "                                37.0f, 41.0f, 43.0f));\n"
        "   mat3 m = m3 * mat3(i);\n"
        "   my_Vec = m[0];\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float inputElements[] = {11.0f, 13.0f, 19.0f, 23.0f, 29.0f, 31.0f, 37.0f, 41.0f, 43.0f};
    std::vector<float> input(inputElements, inputElements + 9);
    ASSERT_FALSE(constantVectorFoundInAST(input));
    float outputElements[] = {3.0f / 85.0f,    -11.0f / 34.0f, 37.0f / 170.0f,
                              -79.0f / 340.0f, 23.0f / 68.0f,  -12.0f / 85.0f,
                              13.0f / 68.0f,   -3.0f / 68.0f,  -1.0f / 34.0f};
    std::vector<float> result(outputElements, outputElements + 9);
    const float floatFaultTolerance = 0.000001f;
    ASSERT_TRUE(constantVectorNearFoundInAST(result, floatFaultTolerance));
}

// Check if the matrix 'inverse' operation on 4x4 matrix is constant folded.
TEST_F(ConstantFoldingTest, Fold4x4MatrixInverse)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in float i;\n"
        "out vec4 my_Vec;\n"
        "void main() {\n"
        "   const mat4 m4 = inverse(mat4(29.0f, 31.0f, 37.0f, 41.0f,\n"
        "                                43.0f, 47.0f, 53.0f, 59.0f,\n"
        "                                61.0f, 67.0f, 71.0f, 73.0f,\n"
        "                                79.0f, 83.0f, 89.0f, 97.0f));\n"
        "   mat4 m = m4 * mat4(i);\n"
        "   my_Vec = m[0];\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float inputElements[] = {29.0f, 31.0f, 37.0f, 41.0f, 43.0f, 47.0f, 53.0f, 59.0f,
                             61.0f, 67.0f, 71.0f, 73.0f, 79.0f, 83.0f, 89.0f, 97.0f};
    std::vector<float> input(inputElements, inputElements + 16);
    ASSERT_FALSE(constantVectorFoundInAST(input));
    float outputElements[] = {43.0f / 126.0f, -11.0f / 21.0f, -2.0f / 21.0f,  31.0f / 126.0f,
                              -5.0f / 7.0f,   9.0f / 14.0f,   1.0f / 14.0f,   -1.0f / 7.0f,
                              85.0f / 126.0f, -11.0f / 21.0f, 43.0f / 210.0f, -38.0f / 315.0f,
                              -2.0f / 7.0f,   5.0f / 14.0f,   -6.0f / 35.0f,  3.0f / 70.0f};
    std::vector<float> result(outputElements, outputElements + 16);
    const float floatFaultTolerance = 0.00001f;
    ASSERT_TRUE(constantVectorNearFoundInAST(result, floatFaultTolerance));
}

// FoldMxNMatrixDeterminant tests check if the matrix 'determinant' operation
// on MxN matrix is constant folded when argument is constant expression and also
// checks the correctness of the result returned by the constant folding operation.
// All the matrices including matrices in the shader code are in column-major order.
TEST_F(ConstantFoldingTest, Fold2x2MatrixDeterminant)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float my_Float;"
        "void main() {\n"
        "   const float f = determinant(mat2(2.0f, 3.0f,\n"
        "                                    5.0f, 7.0f));\n"
        "   my_Float = f;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float inputElements[] = {2.0f, 3.0f, 5.0f, 7.0f};
    std::vector<float> input(inputElements, inputElements + 4);
    ASSERT_FALSE(constantColumnMajorMatrixFoundInAST(input));
    ASSERT_TRUE(constantFoundInAST(-1.0f));
}

// Check if the matrix 'determinant' operation on 3x3 matrix is constant folded.
TEST_F(ConstantFoldingTest, Fold3x3MatrixDeterminant)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float my_Float;"
        "void main() {\n"
        "   const float f = determinant(mat3(11.0f, 13.0f, 19.0f,\n"
        "                               23.0f, 29.0f, 31.0f,\n"
        "                                    37.0f, 41.0f, 43.0f));\n"
        "   my_Float = f;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float inputElements[] = {11.0f, 13.0f, 19.0f, 23.0f, 29.0f, 31.0f, 37.0f, 41.0f, 43.0f};
    std::vector<float> input(inputElements, inputElements + 9);
    ASSERT_FALSE(constantColumnMajorMatrixFoundInAST(input));
    ASSERT_TRUE(constantFoundInAST(-680.0f));
}

// Check if the matrix 'determinant' operation on 4x4 matrix is constant folded.
TEST_F(ConstantFoldingTest, Fold4x4MatrixDeterminant)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float my_Float;"
        "void main() {\n"
        "   const float f = determinant(mat4(29.0f, 31.0f, 37.0f, 41.0f,\n"
        "                                    43.0f, 47.0f, 53.0f, 59.0f,\n"
        "                                    61.0f, 67.0f, 71.0f, 73.0f,\n"
        "                                    79.0f, 83.0f, 89.0f, 97.0f));\n"
        "   my_Float = f;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float inputElements[] = {29.0f, 31.0f, 37.0f, 41.0f, 43.0f, 47.0f, 53.0f, 59.0f,
                             61.0f, 67.0f, 71.0f, 73.0f, 79.0f, 83.0f, 89.0f, 97.0f};
    std::vector<float> input(inputElements, inputElements + 16);
    ASSERT_FALSE(constantColumnMajorMatrixFoundInAST(input));
    ASSERT_TRUE(constantFoundInAST(-2520.0f));
}

// Check if the matrix 'transpose' operation on 3x3 matrix is constant folded.
// All the matrices including matrices in the shader code are in column-major order.
TEST_F(ConstantFoldingTest, Fold3x3MatrixTranspose)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in float i;\n"
        "out vec3 my_Vec;\n"
        "void main() {\n"
        "   const mat3 m3 = transpose(mat3(11.0f, 13.0f, 19.0f,\n"
        "                                  23.0f, 29.0f, 31.0f,\n"
        "                                  37.0f, 41.0f, 43.0f));\n"
        "   mat3 m = m3 * mat3(i);\n"
        "   my_Vec = m[0];\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float inputElements[] = {11.0f, 13.0f, 19.0f, 23.0f, 29.0f, 31.0f, 37.0f, 41.0f, 43.0f};
    std::vector<float> input(inputElements, inputElements + 9);
    ASSERT_FALSE(constantColumnMajorMatrixFoundInAST(input));
    float outputElements[] = {11.0f, 23.0f, 37.0f, 13.0f, 29.0f, 41.0f, 19.0f, 31.0f, 43.0f};
    std::vector<float> result(outputElements, outputElements + 9);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Test that 0xFFFFFFFF wraps to -1 when parsed as integer.
// This is featured in the examples of ESSL3 section 4.1.3. ESSL3 section 12.42
// means that any 32-bit unsigned integer value is a valid literal.
TEST_F(ConstantFoldingTest, ParseWrappedHexIntLiteral)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision highp int;\n"
        "uniform int inInt;\n"
        "out vec4 my_Vec;\n"
        "void main() {\n"
        "   const int i = 0xFFFFFFFF;\n"
        "   my_Vec = vec4(i * inInt);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(-1));
}

// Test that 3000000000 wraps to -1294967296 when parsed as integer.
// This is featured in the examples of GLSL 4.5, and ESSL behavior should match
// desktop GLSL when it comes to integer parsing.
TEST_F(ConstantFoldingTest, ParseWrappedDecimalIntLiteral)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision highp int;\n"
        "uniform int inInt;\n"
        "out vec4 my_Vec;\n"
        "void main() {\n"
        "   const int i = 3000000000;\n"
        "   my_Vec = vec4(i * inInt);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(-1294967296));
}

// Test that 0xFFFFFFFFu is parsed correctly as an unsigned integer literal.
// This is featured in the examples of ESSL3 section 4.1.3. ESSL3 section 12.42
// means that any 32-bit unsigned integer value is a valid literal.
TEST_F(ConstantFoldingTest, ParseMaxUintLiteral)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision highp int;\n"
        "uniform uint inInt;\n"
        "out vec4 my_Vec;\n"
        "void main() {\n"
        "   const uint i = 0xFFFFFFFFu;\n"
        "   my_Vec = vec4(i * inInt);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0xFFFFFFFFu));
}

// Test that unary minus applied to unsigned int is constant folded correctly.
// This is featured in the examples of ESSL3 section 4.1.3. ESSL3 section 12.42
// means that any 32-bit unsigned integer value is a valid literal.
TEST_F(ConstantFoldingTest, FoldUnaryMinusOnUintLiteral)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision highp int;\n"
        "uniform uint inInt;\n"
        "out vec4 my_Vec;\n"
        "void main() {\n"
        "   const uint i = -1u;\n"
        "   my_Vec = vec4(i * inInt);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0xFFFFFFFFu));
}

// Test that constant mat2 initialization with a mat2 parameter works correctly.
TEST_F(ConstantFoldingTest, FoldMat2ConstructorTakingMat2)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float mult;\n"
        "void main() {\n"
        "   const mat2 cm = mat2(mat2(0.0, 1.0, 2.0, 3.0));\n"
        "   mat2 m = cm * mult;\n"
        "   gl_FragColor = vec4(m[0], m[1]);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float outputElements[] = {0.0f, 1.0f, 2.0f, 3.0f};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Test that constant mat2 initialization with an int parameter works correctly.
TEST_F(ConstantFoldingTest, FoldMat2ConstructorTakingScalar)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float mult;\n"
        "void main() {\n"
        "   const mat2 cm = mat2(3);\n"
        "   mat2 m = cm * mult;\n"
        "   gl_FragColor = vec4(m[0], m[1]);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float outputElements[] = {3.0f, 0.0f, 0.0f, 3.0f};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Test that constant mat2 initialization with a mix of parameters works correctly.
TEST_F(ConstantFoldingTest, FoldMat2ConstructorTakingMix)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float mult;\n"
        "void main() {\n"
        "   const mat2 cm = mat2(-1, vec2(0.0, 1.0), vec4(2.0));\n"
        "   mat2 m = cm * mult;\n"
        "   gl_FragColor = vec4(m[0], m[1]);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float outputElements[] = {-1.0, 0.0f, 1.0f, 2.0f};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Test that constant mat2 initialization with a mat3 parameter works correctly.
TEST_F(ConstantFoldingTest, FoldMat2ConstructorTakingMat3)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float mult;\n"
        "void main() {\n"
        "   const mat2 cm = mat2(mat3(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));\n"
        "   mat2 m = cm * mult;\n"
        "   gl_FragColor = vec4(m[0], m[1]);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float outputElements[] = {0.0f, 1.0f, 3.0f, 4.0f};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Test that constant mat4x3 initialization with a mat3x2 parameter works correctly.
TEST_F(ConstantFoldingTest, FoldMat4x3ConstructorTakingMat3x2)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform float mult;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   const mat4x3 cm = mat4x3(mat3x2(1.0, 2.0,\n"
        "                                   3.0, 4.0,\n"
        "                                   5.0, 6.0));\n"
        "   mat4x3 m = cm * mult;\n"
        "   my_FragColor = vec4(m[0], m[1][0]);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float outputElements[] = {1.0f, 2.0f, 0.0f, 3.0f, 4.0f, 0.0f,
                              5.0f, 6.0f, 1.0f, 0.0f, 0.0f, 0.0f};
    std::vector<float> result(outputElements, outputElements + 12);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Test that constant mat2 initialization with a vec4 parameter works correctly.
TEST_F(ConstantFoldingTest, FoldMat2ConstructorTakingVec4)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float mult;\n"
        "void main() {\n"
        "   const mat2 cm = mat2(vec4(0.0, 1.0, 2.0, 3.0));\n"
        "   mat2 m = cm * mult;\n"
        "   gl_FragColor = vec4(m[0], m[1]);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float outputElements[] = {0.0f, 1.0f, 2.0f, 3.0f};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Test that equality comparison of two different structs with a nested struct inside returns false.
TEST_F(ConstantFoldingTest, FoldNestedDifferentStructEqualityComparison)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct nested {\n"
        "    float f\n;"
        "};\n"
        "struct S {\n"
        "    nested a;\n"
        "    float f;\n"
        "};\n"
        "uniform vec4 mult;\n"
        "void main()\n"
        "{\n"
        "    const S s1 = S(nested(0.0), 2.0);\n"
        "    const S s2 = S(nested(0.0), 3.0);\n"
        "    gl_FragColor = (s1 == s2 ? 1.0 : 0.5) * mult;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0.5f));
}

// Test that equality comparison of two identical structs with a nested struct inside returns true.
TEST_F(ConstantFoldingTest, FoldNestedIdenticalStructEqualityComparison)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct nested {\n"
        "    float f\n;"
        "};\n"
        "struct S {\n"
        "    nested a;\n"
        "    float f;\n"
        "    int i;\n"
        "};\n"
        "uniform vec4 mult;\n"
        "void main()\n"
        "{\n"
        "    const S s1 = S(nested(0.0), 2.0, 3);\n"
        "    const S s2 = S(nested(0.0), 2.0, 3);\n"
        "    gl_FragColor = (s1 == s2 ? 1.0 : 0.5) * mult;\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(1.0f));
}

// Test that right elements are chosen from non-square matrix
TEST_F(ConstantFoldingTest, FoldNonSquareMatrixIndexing)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = mat3x4(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)[1];\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    float outputElements[] = {4.0f, 5.0f, 6.0f, 7.0f};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}

// Test that folding outer product of vectors with non-matching lengths works.
TEST_F(ConstantFoldingTest, FoldNonSquareOuterProduct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    mat3x2 prod = outerProduct(vec2(2.0, 3.0), vec3(5.0, 7.0, 11.0));\n"
        "    my_FragColor = vec4(prod[0].x);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    // clang-format off
    float outputElements[] =
    {
        10.0f, 15.0f,
        14.0f, 21.0f,
        22.0f, 33.0f
    };
    // clang-format on
    std::vector<float> result(outputElements, outputElements + 6);
    ASSERT_TRUE(constantColumnMajorMatrixFoundInAST(result));
}

// Test that folding bit shift left with non-matching signedness works.
TEST_F(ConstantFoldingTest, FoldBitShiftLeftDifferentSignedness)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    uint u = 0xffffffffu << 31;\n"
        "    my_FragColor = vec4(u);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0x80000000u));
}

// Test that folding bit shift right with non-matching signedness works.
TEST_F(ConstantFoldingTest, FoldBitShiftRightDifferentSignedness)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    uint u = 0xffffffffu >> 30;\n"
        "    my_FragColor = vec4(u);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0x3u));
}

// Test that folding signed bit shift right extends the sign bit.
// ESSL 3.00.6 section 5.9 Expressions.
TEST_F(ConstantFoldingTest, FoldBitShiftRightExtendSignBit)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const int i = 0x8fffe000 >> 6;\n"
        "    uint u = uint(i);"
        "    my_FragColor = vec4(u);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    // The bits of the operand are 0x8fffe000 = 1000 1111 1111 1111 1110 0000 0000 0000
    // After shifting, they become              1111 1110 0011 1111 1111 1111 1000 0000 = 0xfe3fff80
    ASSERT_TRUE(constantFoundInAST(0xfe3fff80u));
}

// Signed bit shift left should interpret its operand as a bit pattern. As a consequence a number
// may turn from positive to negative when shifted left.
// ESSL 3.00.6 section 5.9 Expressions.
TEST_F(ConstantFoldingTest, FoldBitShiftLeftInterpretedAsBitPattern)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const int i = 0x1fffffff << 3;\n"
        "    uint u = uint(i);"
        "    my_FragColor = vec4(u);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0xfffffff8u));
}

// Test that dividing the minimum signed integer by -1 works.
// ESSL 3.00.6 section 4.1.3 Integers:
// "However, for the case where the minimum representable value is divided by -1, it is allowed to
// return either the minimum representable value or the maximum representable value."
TEST_F(ConstantFoldingTest, FoldDivideMinimumIntegerByMinusOne)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i = 0x80000000 / (-1);\n"
        "    my_FragColor = vec4(i);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0x7fffffff) || constantFoundInAST(-0x7fffffff - 1));
}

// Test that folding an unsigned integer addition that overflows works.
// ESSL 3.00.6 section 4.1.3 Integers:
// "For all precisions, operations resulting in overflow or underflow will not cause any exception,
// nor will they saturate, rather they will 'wrap' to yield the low-order n bits of the result where
// n is the size in bits of the integer."
TEST_F(ConstantFoldingTest, FoldUnsignedIntegerAddOverflow)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    uint u = 0xffffffffu + 43u;\n"
        "    my_FragColor = vec4(u);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(42u));
}

// Test that folding a signed integer addition that overflows works.
// ESSL 3.00.6 section 4.1.3 Integers:
// "For all precisions, operations resulting in overflow or underflow will not cause any exception,
// nor will they saturate, rather they will 'wrap' to yield the low-order n bits of the result where
// n is the size in bits of the integer."
TEST_F(ConstantFoldingTest, FoldSignedIntegerAddOverflow)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i = 0x7fffffff + 4;\n"
        "    my_FragColor = vec4(i);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(-0x7ffffffd));
}

// Test that folding an unsigned integer subtraction that overflows works.
// ESSL 3.00.6 section 4.1.3 Integers:
// "For all precisions, operations resulting in overflow or underflow will not cause any exception,
// nor will they saturate, rather they will 'wrap' to yield the low-order n bits of the result where
// n is the size in bits of the integer."
TEST_F(ConstantFoldingTest, FoldUnsignedIntegerDiffOverflow)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    uint u = 0u - 5u;\n"
        "    my_FragColor = vec4(u);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0xfffffffbu));
}

// Test that folding a signed integer subtraction that overflows works.
// ESSL 3.00.6 section 4.1.3 Integers:
// "For all precisions, operations resulting in overflow or underflow will not cause any exception,
// nor will they saturate, rather they will 'wrap' to yield the low-order n bits of the result where
// n is the size in bits of the integer."
TEST_F(ConstantFoldingTest, FoldSignedIntegerDiffOverflow)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i = -0x7fffffff - 7;\n"
        "    my_FragColor = vec4(i);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0x7ffffffa));
}

// Test that folding an unsigned integer multiplication that overflows works.
// ESSL 3.00.6 section 4.1.3 Integers:
// "For all precisions, operations resulting in overflow or underflow will not cause any exception,
// nor will they saturate, rather they will 'wrap' to yield the low-order n bits of the result where
// n is the size in bits of the integer."
TEST_F(ConstantFoldingTest, FoldUnsignedIntegerMultiplyOverflow)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    uint u = 0xffffffffu * 10u;\n"
        "    my_FragColor = vec4(u);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(0xfffffff6u));
}

// Test that folding a signed integer multiplication that overflows works.
// ESSL 3.00.6 section 4.1.3 Integers:
// "For all precisions, operations resulting in overflow or underflow will not cause any exception,
// nor will they saturate, rather they will 'wrap' to yield the low-order n bits of the result where
// n is the size in bits of the integer."
TEST_F(ConstantFoldingTest, FoldSignedIntegerMultiplyOverflow)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i = 0x7fffffff * 42;\n"
        "    my_FragColor = vec4(i);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(-42));
}

// Test that folding of negating the minimum representable integer works. Note that in the test
// "0x80000000" is a negative literal, and the minus sign before it is the negation operator.
// ESSL 3.00.6 section 4.1.3 Integers:
// "For all precisions, operations resulting in overflow or underflow will not cause any exception,
// nor will they saturate, rather they will 'wrap' to yield the low-order n bits of the result where
// n is the size in bits of the integer."
TEST_F(ConstantFoldingTest, FoldMinimumSignedIntegerNegation)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i = -0x80000000;\n"
        "    my_FragColor = vec4(i);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    // Negating the minimum signed integer overflows the positive range, so it wraps back to itself.
    ASSERT_TRUE(constantFoundInAST(-0x7fffffff - 1));
}

// Test that folding of shifting the minimum representable integer works.
TEST_F(ConstantFoldingTest, FoldMinimumSignedIntegerRightShift)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i = (0x80000000 >> 1);\n"
        "    int j = (0x80000000 >> 7);\n"
        "    my_FragColor = vec4(i, j, i, j);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(-0x40000000));
    ASSERT_TRUE(constantFoundInAST(-0x01000000));
}

// Test that folding of shifting by 0 works.
TEST_F(ConstantFoldingTest, FoldShiftByZero)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i = (3 >> 0);\n"
        "    int j = (73 << 0);\n"
        "    my_FragColor = vec4(i, j, i, j);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(3));
    ASSERT_TRUE(constantFoundInAST(73));
}

// Test that folding IsInf results in true when the parameter is an out-of-range float literal.
// ESSL 3.00.6 section 4.1.4 Floats:
// "If the value of the floating point number is too large (small) to be stored as a single
// precision value, it is converted to positive (negative) infinity."
// ESSL 3.00.6 section 12.4:
// "Mandate support for signed infinities."
TEST_F(ConstantFoldingTest, FoldIsInfOutOfRangeFloatLiteral)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    bool b = isinf(1.0e2048);\n"
        "    my_FragColor = vec4(b);\n"
        "}\n";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(true));
}

// Regression test case of unary + constant folding of a void struct member.
TEST_F(ConstantFoldingTest, VoidStructMember)
{
    constexpr const char *kShaderString = "struct U{void t;}n(){+U().t";
    ASSERT_FALSE(compile(kShaderString));
}

// Test that floats that are too small to be represented get flushed to zero.
// ESSL 3.00.6 section 4.1.4 Floats:
// "A value with a magnitude too small to be represented as a mantissa and exponent is converted to
// zero."
TEST_F(ConstantFoldingExpressionTest, FoldTooSmallFloat)
{
    const std::string &floatString = "1.0e-2048";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(0.0f));
}

// IEEE 754 dictates that behavior of infinity is derived from limiting cases of real arithmetic.
// lim radians(x) x -> inf = inf
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldRadiansInfinity)
{
    const std::string &floatString = "radians(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that behavior of infinity is derived from limiting cases of real arithmetic.
// lim degrees(x) x -> inf = inf
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldDegreesInfinity)
{
    const std::string &floatString = "degrees(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that sinh(inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldSinhInfinity)
{
    const std::string &floatString = "sinh(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that sinh(-inf) = -inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldSinhNegativeInfinity)
{
    const std::string &floatString = "sinh(-1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that cosh(inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldCoshInfinity)
{
    const std::string &floatString = "cosh(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that cosh(-inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldCoshNegativeInfinity)
{
    const std::string &floatString = "cosh(-1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that asinh(inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldAsinhInfinity)
{
    const std::string &floatString = "asinh(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that asinh(-inf) = -inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldAsinhNegativeInfinity)
{
    const std::string &floatString = "asinh(-1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that acosh(inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldAcoshInfinity)
{
    const std::string &floatString = "acosh(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that pow or powr(0, inf) = 0.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldPowInfinity)
{
    const std::string &floatString = "pow(0.0, 1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(0.0f));
}

// IEEE 754 dictates that exp(inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldExpInfinity)
{
    const std::string &floatString = "exp(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that exp(-inf) = 0.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldExpNegativeInfinity)
{
    const std::string &floatString = "exp(-1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(0.0f));
}

// IEEE 754 dictates that log(inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldLogInfinity)
{
    const std::string &floatString = "log(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that exp2(inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldExp2Infinity)
{
    const std::string &floatString = "exp2(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that exp2(-inf) = 0.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldExp2NegativeInfinity)
{
    const std::string &floatString = "exp2(-1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(0.0f));
}

// IEEE 754 dictates that log2(inf) = inf.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldLog2Infinity)
{
    const std::string &floatString = "log2(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that behavior of infinity is derived from limiting cases of real arithmetic.
// lim sqrt(x) x -> inf = inf
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldSqrtInfinity)
{
    const std::string &floatString = "sqrt(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that rSqrt(inf) = 0
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldInversesqrtInfinity)
{
    const std::string &floatString = "inversesqrt(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(0.0f));
}

// IEEE 754 dictates that behavior of infinity is derived from limiting cases of real arithmetic.
// lim length(x) x -> inf = inf
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldLengthInfinity)
{
    const std::string &floatString = "length(1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that behavior of infinity is derived from limiting cases of real arithmetic.
// lim dot(x, y) x -> inf, y > 0 = inf
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldDotInfinity)
{
    const std::string &floatString = "dot(1.0e2048, 1.0)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// IEEE 754 dictates that behavior of infinity is derived from limiting cases of real arithmetic.
// lim dot(vec2(x, y), vec2(z)) x -> inf, finite y, z > 0 = inf
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldDotInfinity2)
{
    const std::string &floatString = "dot(vec2(1.0e2048, -1.0), vec2(1.0))";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// Faceforward behavior with infinity as a parameter can be derived from dot().
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldFaceForwardInfinity)
{
    const std::string &floatString = "faceforward(4.0, 1.0e2048, 1.0)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-4.0f));
}

// Faceforward behavior with infinity as a parameter can be derived from dot().
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldFaceForwardInfinity2)
{
    const std::string &floatString = "faceforward(vec2(4.0), vec2(1.0e2048, -1.0), vec2(1.0)).x";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-4.0f));
}

// Test that infinity - finite value evaluates to infinity.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldInfinityMinusFinite)
{
    const std::string &floatString = "1.0e2048 - 1.0e20";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// Test that -infinity + finite value evaluates to -infinity.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldMinusInfinityPlusFinite)
{
    const std::string &floatString = "(-1.0e2048) + 1.0e20";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-std::numeric_limits<float>::infinity()));
}

// Test that infinity * finite value evaluates to infinity.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldInfinityMultipliedByFinite)
{
    const std::string &floatString = "1.0e2048 * 1.0e-20";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// Test that infinity * infinity evaluates to infinity.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldInfinityMultipliedByInfinity)
{
    const std::string &floatString = "1.0e2048 * 1.0e2048";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
}

// Test that infinity * negative infinity evaluates to negative infinity.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldInfinityMultipliedByNegativeInfinity)
{
    const std::string &floatString = "1.0e2048 * (-1.0e2048)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-std::numeric_limits<float>::infinity()));
}

// Test that dividing by minus zero results in the appropriately signed infinity.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
// "If both positive and negative zeros are implemented, the correctly signed Inf will be
// generated".
TEST_F(ConstantFoldingExpressionTest, FoldDivideByNegativeZero)
{
    const std::string &floatString = "1.0 / (-0.0)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-std::numeric_limits<float>::infinity()));
    ASSERT_TRUE(hasWarning());
}

// Test that infinity divided by zero evaluates to infinity.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldInfinityDividedByZero)
{
    const std::string &floatString = "1.0e2048 / 0.0";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<float>::infinity()));
    ASSERT_TRUE(hasWarning());
}

// Test that negative infinity divided by zero evaluates to negative infinity.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldMinusInfinityDividedByZero)
{
    const std::string &floatString = "(-1.0e2048) / 0.0";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-std::numeric_limits<float>::infinity()));
    ASSERT_TRUE(hasWarning());
}

// Test that dividing a finite number by infinity results in zero.
// ESSL 3.00.6 section 4.5.1: "Infinities and zeroes are generated as dictated by IEEE".
TEST_F(ConstantFoldingExpressionTest, FoldDivideByInfinity)
{
    const std::string &floatString = "1.0e30 / 1.0e2048";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(0.0f));
}

// Test that unsigned bitfieldExtract is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldUnsignedBitfieldExtract)
{
    const std::string &uintString = "bitfieldExtract(0x00110000u, 16, 5)";
    evaluateUint(uintString);
    ASSERT_TRUE(constantFoundInAST(0x11u));
}

// Test that unsigned bitfieldExtract to extract 32 bits is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldUnsignedBitfieldExtract32Bits)
{
    const std::string &uintString = "bitfieldExtract(0xff0000ffu, 0, 32)";
    evaluateUint(uintString);
    ASSERT_TRUE(constantFoundInAST(0xff0000ffu));
}

// Test that signed bitfieldExtract is folded correctly. The higher bits should be set to 1 if the
// most significant bit of the extracted value is 1.
TEST_F(ConstantFoldingExpressionTest, FoldSignedBitfieldExtract)
{
    const std::string &intString = "bitfieldExtract(0x00110000, 16, 5)";
    evaluateInt(intString);
    // 0xfffffff1 == -15
    ASSERT_TRUE(constantFoundInAST(-15));
}

// Test that bitfieldInsert is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldBitfieldInsert)
{
    const std::string &uintString = "bitfieldInsert(0x04501701u, 0x11u, 8, 5)";
    evaluateUint(uintString);
    ASSERT_TRUE(constantFoundInAST(0x04501101u));
}

// Test that bitfieldInsert to insert 32 bits is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldBitfieldInsert32Bits)
{
    const std::string &uintString = "bitfieldInsert(0xff0000ffu, 0x11u, 0, 32)";
    evaluateUint(uintString);
    ASSERT_TRUE(constantFoundInAST(0x11u));
}

// Test that bitfieldReverse is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldBitfieldReverse)
{
    const std::string &uintString = "bitfieldReverse((1u << 4u) | (1u << 7u))";
    evaluateUint(uintString);
    uint32_t flag1 = 1u << (31u - 4u);
    uint32_t flag2 = 1u << (31u - 7u);
    ASSERT_TRUE(constantFoundInAST(flag1 | flag2));
}

// Test that bitCount is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldBitCount)
{
    const std::string &intString = "bitCount(0x17103121u)";
    evaluateInt(intString);
    ASSERT_TRUE(constantFoundInAST(10));
}

// Test that findLSB is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldFindLSB)
{
    const std::string &intString = "findLSB(0x80010000u)";
    evaluateInt(intString);
    ASSERT_TRUE(constantFoundInAST(16));
}

// Test that findLSB is folded correctly when the operand is zero.
TEST_F(ConstantFoldingExpressionTest, FoldFindLSBZero)
{
    const std::string &intString = "findLSB(0u)";
    evaluateInt(intString);
    ASSERT_TRUE(constantFoundInAST(-1));
}

// Test that findMSB is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldFindMSB)
{
    const std::string &intString = "findMSB(0x01000008u)";
    evaluateInt(intString);
    ASSERT_TRUE(constantFoundInAST(24));
}

// Test that findMSB is folded correctly when the operand is zero.
TEST_F(ConstantFoldingExpressionTest, FoldFindMSBZero)
{
    const std::string &intString = "findMSB(0u)";
    evaluateInt(intString);
    ASSERT_TRUE(constantFoundInAST(-1));
}

// Test that findMSB is folded correctly for a negative integer.
// It is supposed to return the index of the most significant bit set to 0.
TEST_F(ConstantFoldingExpressionTest, FoldFindMSBNegativeInt)
{
    const std::string &intString = "findMSB(-8)";
    evaluateInt(intString);
    ASSERT_TRUE(constantFoundInAST(2));
}

// Test that findMSB is folded correctly for -1.
TEST_F(ConstantFoldingExpressionTest, FoldFindMSBMinusOne)
{
    const std::string &intString = "findMSB(-1)";
    evaluateInt(intString);
    ASSERT_TRUE(constantFoundInAST(-1));
}

// Test that packUnorm4x8 is folded correctly for a vector of zeroes.
TEST_F(ConstantFoldingExpressionTest, FoldPackUnorm4x8Zero)
{
    const std::string &intString = "packUnorm4x8(vec4(0.0))";
    evaluateUint(intString);
    ASSERT_TRUE(constantFoundInAST(0u));
}

// Test that packUnorm4x8 is folded correctly for a vector of ones.
TEST_F(ConstantFoldingExpressionTest, FoldPackUnorm4x8One)
{
    const std::string &intString = "packUnorm4x8(vec4(1.0))";
    evaluateUint(intString);
    ASSERT_TRUE(constantFoundInAST(0xffffffffu));
}

// Test that packSnorm4x8 is folded correctly for a vector of zeroes.
TEST_F(ConstantFoldingExpressionTest, FoldPackSnorm4x8Zero)
{
    const std::string &intString = "packSnorm4x8(vec4(0.0))";
    evaluateUint(intString);
    ASSERT_TRUE(constantFoundInAST(0u));
}

// Test that packSnorm4x8 is folded correctly for a vector of ones.
TEST_F(ConstantFoldingExpressionTest, FoldPackSnorm4x8One)
{
    const std::string &intString = "packSnorm4x8(vec4(1.0))";
    evaluateUint(intString);
    ASSERT_TRUE(constantFoundInAST(0x7f7f7f7fu));
}

// Test that packSnorm4x8 is folded correctly for a vector of minus ones.
TEST_F(ConstantFoldingExpressionTest, FoldPackSnorm4x8MinusOne)
{
    const std::string &intString = "packSnorm4x8(vec4(-1.0))";
    evaluateUint(intString);
    ASSERT_TRUE(constantFoundInAST(0x81818181u));
}

// Test that unpackSnorm4x8 is folded correctly when it needs to clamp the result.
TEST_F(ConstantFoldingExpressionTest, FoldUnpackSnorm4x8Clamp)
{
    const std::string &floatString = "unpackSnorm4x8(0x00000080u).x";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(-1.0f));
}

// Test that unpackUnorm4x8 is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldUnpackUnorm4x8)
{
    const std::string &floatString = "unpackUnorm4x8(0x007bbeefu).z";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(123.0f / 255.0f));
}

// Test that ldexp is folded correctly.
TEST_F(ConstantFoldingExpressionTest, FoldLdexp)
{
    const std::string &floatString = "ldexp(0.625, 1)";
    evaluateFloat(floatString);
    ASSERT_TRUE(constantFoundInAST(1.25f));
}

// Fold a ternary operator.
TEST_F(ConstantFoldingTest, FoldTernary)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp int;
        uniform int u;
        out int my_FragColor;
        void main()
        {
            my_FragColor = (true ? 1 : u);
        })";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(1));
    ASSERT_FALSE(symbolFoundInMain("u"));
}

// Fold a ternary operator inside a consuming expression.
TEST_F(ConstantFoldingTest, FoldTernaryInsideExpression)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp int;
        uniform int u;
        out int my_FragColor;
        void main()
        {
            my_FragColor = ivec2((true ? 1 : u) + 2, 4).x;
        })";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(3));
    ASSERT_FALSE(symbolFoundInMain("u"));
}

// Fold indexing into an array constructor.
TEST_F(ConstantFoldingExpressionTest, FoldArrayConstructorIndexing)
{
    const std::string &floatString = "(float[3](-1.0, 1.0, 2.0))[2]";
    evaluateFloat(floatString);
    ASSERT_FALSE(constantFoundInAST(-1.0f));
    ASSERT_FALSE(constantFoundInAST(1.0f));
    ASSERT_TRUE(constantFoundInAST(2.0f));
}

// Fold indexing into an array of arrays constructor.
TEST_F(ConstantFoldingExpressionTest, FoldArrayOfArraysConstructorIndexing)
{
    const std::string &floatString = "(float[2][2](float[2](-1.0, 1.0), float[2](2.0, 3.0)))[1][0]";
    evaluateFloat(floatString);
    ASSERT_FALSE(constantFoundInAST(-1.0f));
    ASSERT_FALSE(constantFoundInAST(1.0f));
    ASSERT_FALSE(constantFoundInAST(3.0f));
    ASSERT_TRUE(constantFoundInAST(2.0f));
}

// Fold indexing into a named constant array.
TEST_F(ConstantFoldingTest, FoldNamedArrayIndexing)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        const float[3] arr = float[3](-1.0, 1.0, 2.0);
        out float my_FragColor;
        void main()
        {
            my_FragColor = arr[1];
        })";
    compileAssumeSuccess(shaderString);
    ASSERT_FALSE(constantFoundInAST(-1.0f));
    ASSERT_FALSE(constantFoundInAST(2.0f));
    ASSERT_TRUE(constantFoundInAST(1.0f));
    // The variable should be pruned out since after folding the indexing, there are no more
    // references to it.
    ASSERT_FALSE(symbolFoundInAST("arr"));
}

// Fold indexing into a named constant array of arrays.
TEST_F(ConstantFoldingTest, FoldNamedArrayOfArraysIndexing)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision highp float;
        const float[2][2] arr = float[2][2](float[2](-1.0, 1.0), float[2](2.0, 3.0));
        out float my_FragColor;
        void main()
        {
            my_FragColor = arr[0][1];
        })";
    compileAssumeSuccess(shaderString);
    ASSERT_FALSE(constantFoundInAST(-1.0f));
    ASSERT_FALSE(constantFoundInAST(2.0f));
    ASSERT_FALSE(constantFoundInAST(3.0f));
    ASSERT_TRUE(constantFoundInAST(1.0f));
    // The variable should be pruned out since after folding the indexing, there are no more
    // references to it.
    ASSERT_FALSE(symbolFoundInAST("arr"));
}

// Fold indexing into an array constructor where some of the arguments are constant and others are
// non-constant but without side effects.
TEST_F(ConstantFoldingTest, FoldArrayConstructorIndexingWithMixedArguments)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        uniform float u;
        out float my_FragColor;
        void main()
        {
            my_FragColor = float[2](u, 1.0)[1];
        })";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(1.0f));
    ASSERT_FALSE(constantFoundInAST(1));
    ASSERT_FALSE(symbolFoundInMain("u"));
}

// Indexing into an array constructor where some of the arguments have side effects can't be folded.
TEST_F(ConstantFoldingTest, CantFoldArrayConstructorIndexingWithSideEffects)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        out float my_FragColor;
        void main()
        {
            float sideEffectTarget = 0.0;
            float f = float[3](sideEffectTarget = 1.0, 1.0, 2.0)[1];
            my_FragColor = f + sideEffectTarget;
        })";
    compileAssumeSuccess(shaderString);
    // All of the array constructor arguments should be present in the final AST.
    ASSERT_TRUE(constantFoundInAST(1.0f));
    ASSERT_TRUE(constantFoundInAST(2.0f));
}

// Fold comparing two array constructors.
TEST_F(ConstantFoldingTest, FoldArrayConstructorEquality)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        out float my_FragColor;
        void main()
        {
            const bool b = (float[3](2.0, 1.0, -1.0) == float[3](2.0, 1.0, -1.0));
            my_FragColor = b ? 3.0 : 4.0;
        })";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(3.0f));
    ASSERT_FALSE(constantFoundInAST(4.0f));
}

// Fold comparing two named constant arrays.
TEST_F(ConstantFoldingExpressionTest, FoldNamedArrayEquality)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        const float[3] arrA = float[3](-1.0, 1.0, 2.0);
        const float[3] arrB = float[3](-1.0, 1.0, 2.0);
        out float my_FragColor;
        void main()
        {
            const bool b = (arrA == arrB);
            my_FragColor = b ? 3.0 : 4.0;
        })";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(3.0f));
    ASSERT_FALSE(constantFoundInAST(4.0f));
}

// Fold comparing two array of arrays constructors.
TEST_F(ConstantFoldingTest, FoldArrayOfArraysConstructorEquality)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision highp float;
        out float my_FragColor;
        void main()
        {
            const bool b = (float[2][2](float[2](-1.0, 1.0), float[2](2.0, 3.0)) ==
                            float[2][2](float[2](-1.0, 1.0), float[2](2.0, 1000.0)));
            my_FragColor = b ? 4.0 : 5.0;
        })";
    compileAssumeSuccess(shaderString);
    ASSERT_TRUE(constantFoundInAST(5.0f));
    ASSERT_FALSE(constantFoundInAST(4.0f));
}

// Test that casting a negative float to uint results in a warning. ESSL 3.00.6 section 5.4.1
// specifies this as an undefined conversion.
TEST_F(ConstantFoldingExpressionTest, FoldNegativeFloatToUint)
{
    const std::string &uintString = "uint(-1.0)";
    evaluateUint(uintString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<unsigned int>::max()));
    ASSERT_TRUE(hasWarning());
}

// Test that casting a negative float to uint inside a uvec constructor results in a warning. ESSL
// 3.00.6 section 5.4.1 specifies this as an undefined conversion.
TEST_F(ConstantFoldingExpressionTest, FoldNegativeFloatToUvec)
{
    const std::string &uintString = "uvec4(2.0, 1.0, vec2(0.0, -1.0)).w";
    evaluateUint(uintString);
    ASSERT_TRUE(constantFoundInAST(std::numeric_limits<unsigned int>::max()));
    ASSERT_TRUE(hasWarning());
}

// Test that a negative float doesn't result in a warning when it is inside a constructor but isn't
// actually converted.
TEST_F(ConstantFoldingExpressionTest, NegativeFloatInsideUvecConstructorButOutOfRange)
{
    const std::string &uintString = "uvec2(1.0, vec2(0.0, -1.0)).x";
    evaluateUint(uintString);
    ASSERT_FALSE(hasWarning());
}

// Test that a large float (above max int32_t) is converted to unsigned integer correctly.
TEST_F(ConstantFoldingExpressionTest, LargeFloatToUint)
{
    const std::string &uintString = "uint(3221225472.0)";
    evaluateUint(uintString);
    ASSERT_TRUE(constantFoundInAST(3221225472u));
    ASSERT_FALSE(hasWarning());
}

// Test that folding % with a negative dividend generates a warning.
TEST_F(ConstantFoldingExpressionTest, IntegerModulusNegativeDividend)
{
    const std::string &intString = "(-5) % 3";
    evaluateInt(intString);
    ASSERT_TRUE(hasWarning());
}

// Test that folding % with a negative divisor generates a warning.
TEST_F(ConstantFoldingExpressionTest, IntegerModulusNegativeDivisor)
{
    const std::string &intString = "5 % (-3)";
    evaluateInt(intString);
    ASSERT_TRUE(hasWarning());
}

TEST_F(ConstantFoldingExpressionTest, IsnanDifferentComponents)
{
    const std::string &ivec4String =
        "ivec4(mix(ivec2(2), ivec2(3), isnan(vec2(1.0, 0.0 / 0.0))), 4, 5)";
    evaluateIvec4(ivec4String);
    int outputElements[] = {2, 3, 4, 5};
    std::vector<int> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}

TEST_F(ConstantFoldingExpressionTest, IsinfDifferentComponents)
{
    const std::string &ivec4String =
        "ivec4(mix(ivec2(2), ivec2(3), isinf(vec2(0.0, 1.0e2048))), 4, 5)";
    evaluateIvec4(ivec4String);
    int outputElements[] = {2, 3, 4, 5};
    std::vector<int> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}

TEST_F(ConstantFoldingExpressionTest, FloatBitsToIntDifferentComponents)
{
    const std::string &ivec4String = "ivec4(floatBitsToInt(vec2(0.0, 1.0)), 4, 5)";
    evaluateIvec4(ivec4String);
    int outputElements[] = {0, 0x3f800000, 4, 5};
    std::vector<int> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}

TEST_F(ConstantFoldingExpressionTest, FloatBitsToUintDifferentComponents)
{
    const std::string &ivec4String = "ivec4(floatBitsToUint(vec2(0.0, 1.0)), 4, 5)";
    evaluateIvec4(ivec4String);
    int outputElements[] = {0, 0x3f800000, 4, 5};
    std::vector<int> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}

TEST_F(ConstantFoldingExpressionTest, IntBitsToFloatDifferentComponents)
{
    const std::string &vec4String = "vec4(intBitsToFloat(ivec2(0, 0x3f800000)), 0.25, 0.5)";
    evaluateVec4(vec4String);
    float outputElements[] = {0.0, 1.0, 0.25, 0.5};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}

TEST_F(ConstantFoldingExpressionTest, UintBitsToFloatDifferentComponents)
{
    const std::string &vec4String = "vec4(uintBitsToFloat(uvec2(0U, 0x3f800000U)), 0.25, 0.5)";
    evaluateVec4(vec4String);
    float outputElements[] = {0.0, 1.0, 0.25, 0.5};
    std::vector<float> result(outputElements, outputElements + 4);
    ASSERT_TRUE(constantVectorFoundInAST(result));
}
