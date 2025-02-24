//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"

#include "angle_gl.h"
#include "common/angleutils.h"
#include "common/utilities.h"
#include "compiler/translator/VariablePacker.h"

namespace
{

static sh::GLenum types[] = {
    GL_FLOAT_MAT4,                     // 0
    GL_FLOAT_MAT2,                     // 1
    GL_FLOAT_VEC4,                     // 2
    GL_INT_VEC4,                       // 3
    GL_BOOL_VEC4,                      // 4
    GL_FLOAT_MAT3,                     // 5
    GL_FLOAT_VEC3,                     // 6
    GL_INT_VEC3,                       // 7
    GL_BOOL_VEC3,                      // 8
    GL_FLOAT_VEC2,                     // 9
    GL_INT_VEC2,                       // 10
    GL_BOOL_VEC2,                      // 11
    GL_FLOAT,                          // 12
    GL_INT,                            // 13
    GL_BOOL,                           // 14
    GL_SAMPLER_2D,                     // 15
    GL_SAMPLER_CUBE,                   // 16
    GL_SAMPLER_EXTERNAL_OES,           // 17
    GL_SAMPLER_2D_RECT_ANGLE,          // 18
    GL_UNSIGNED_INT,                   // 19
    GL_UNSIGNED_INT_VEC2,              // 20
    GL_UNSIGNED_INT_VEC3,              // 21
    GL_UNSIGNED_INT_VEC4,              // 22
    GL_FLOAT_MAT2x3,                   // 23
    GL_FLOAT_MAT2x4,                   // 24
    GL_FLOAT_MAT3x2,                   // 25
    GL_FLOAT_MAT3x4,                   // 26
    GL_FLOAT_MAT4x2,                   // 27
    GL_FLOAT_MAT4x3,                   // 28
    GL_SAMPLER_3D,                     // 29
    GL_SAMPLER_2D_ARRAY,               // 30
    GL_SAMPLER_2D_SHADOW,              // 31
    GL_SAMPLER_CUBE_SHADOW,            // 32
    GL_SAMPLER_2D_ARRAY_SHADOW,        // 33
    GL_INT_SAMPLER_2D,                 // 34
    GL_INT_SAMPLER_CUBE,               // 35
    GL_INT_SAMPLER_3D,                 // 36
    GL_INT_SAMPLER_2D_ARRAY,           // 37
    GL_UNSIGNED_INT_SAMPLER_2D,        // 38
    GL_UNSIGNED_INT_SAMPLER_CUBE,      // 39
    GL_UNSIGNED_INT_SAMPLER_3D,        // 40
    GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,  // 41
};

static sh::GLenum nonSqMatTypes[] = {GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x4, GL_FLOAT_MAT3x2,
                                     GL_FLOAT_MAT3x4, GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x3};

// Creates either a single variable or an array variable depending on numVars.
sh::ShaderVariable CreateShaderVariable(sh::GLenum type, int numVars)
{
    ASSERT(numVars != 0);
    if (numVars == 1)
    {
        return sh::ShaderVariable(type);
    }
    return sh::ShaderVariable(type, numVars);
}

}  // anonymous namespace

TEST(VariablePacking, Pack)
{
    std::vector<sh::ShaderVariable> vars;
    const int kMaxRows = 16;
    // test no vars.
    EXPECT_TRUE(CheckVariablesInPackingLimits(kMaxRows, vars));

    for (size_t tt = 0; tt < ArraySize(types); ++tt)
    {
        sh::GLenum type            = types[tt];
        int num_rows               = sh::GetTypePackingRows(type);
        int num_components_per_row = sh::GetTypePackingComponentsPerRow(type);
        // Check 1 of the type.
        vars.clear();
        vars.push_back(sh::ShaderVariable(type));
        EXPECT_TRUE(CheckVariablesInPackingLimits(kMaxRows, vars));

        // Check exactly the right amount of 1 type as an array.
        int num_vars = kMaxRows / num_rows;
        vars.clear();
        vars.push_back(CreateShaderVariable(type, num_vars));
        EXPECT_TRUE(CheckVariablesInPackingLimits(kMaxRows, vars));

        // test too many
        vars.clear();
        vars.push_back(CreateShaderVariable(type, num_vars + 1));
        EXPECT_FALSE(CheckVariablesInPackingLimits(kMaxRows, vars));

        // Check exactly the right amount of 1 type as individual vars.
        num_vars =
            kMaxRows / num_rows * ((num_components_per_row > 2) ? 1 : (4 / num_components_per_row));
        vars.clear();
        for (int ii = 0; ii < num_vars; ++ii)
        {
            vars.push_back(sh::ShaderVariable(type));
        }
        EXPECT_TRUE(CheckVariablesInPackingLimits(kMaxRows, vars));

        // Check 1 too many.
        vars.push_back(sh::ShaderVariable(type));
        EXPECT_FALSE(CheckVariablesInPackingLimits(kMaxRows, vars));
    }

    // Test example from GLSL ES 3.0 spec chapter 11.
    vars.clear();
    vars.push_back(sh::ShaderVariable(GL_FLOAT_VEC4));
    vars.push_back(sh::ShaderVariable(GL_FLOAT_MAT3));
    vars.push_back(sh::ShaderVariable(GL_FLOAT_MAT3));
    vars.push_back(sh::ShaderVariable(GL_FLOAT_VEC2, 6));
    vars.push_back(sh::ShaderVariable(GL_FLOAT_VEC2, 4));
    vars.push_back(sh::ShaderVariable(GL_FLOAT_VEC2));
    vars.push_back(sh::ShaderVariable(GL_FLOAT, 3));
    vars.push_back(sh::ShaderVariable(GL_FLOAT, 2));
    vars.push_back(sh::ShaderVariable(GL_FLOAT));
    EXPECT_TRUE(CheckVariablesInPackingLimits(kMaxRows, vars));
}

TEST(VariablePacking, PackSizes)
{
    for (size_t tt = 0; tt < ArraySize(types); ++tt)
    {
        sh::GLenum type = types[tt];

        int expectedComponents = gl::VariableComponentCount(type);
        int expectedRows       = gl::VariableRowCount(type);

        if (type == GL_FLOAT_MAT2)
        {
            expectedComponents = 4;
        }
        else if (gl::IsMatrixType(type))
        {
            int squareSize = std::max(gl::VariableRowCount(type), gl::VariableColumnCount(type));
            expectedComponents = squareSize;
            expectedRows       = squareSize;
        }

        EXPECT_EQ(expectedComponents, sh::GetTypePackingComponentsPerRow(type));
        EXPECT_EQ(expectedRows, sh::GetTypePackingRows(type));
    }
}

// Check special assumptions about packing non-square mats
TEST(VariablePacking, NonSquareMats)
{

    for (size_t mt = 0; mt < ArraySize(nonSqMatTypes); ++mt)
    {

        sh::GLenum type = nonSqMatTypes[mt];

        int rows       = gl::VariableRowCount(type);
        int cols       = gl::VariableColumnCount(type);
        int squareSize = std::max(rows, cols);

        std::vector<sh::ShaderVariable> vars;
        vars.push_back(sh::ShaderVariable(type));

        // Fill columns
        for (int row = 0; row < squareSize; row++)
        {
            for (int col = squareSize; col < 4; ++col)
            {
                vars.push_back(sh::ShaderVariable(GL_FLOAT));
            }
        }

        EXPECT_TRUE(CheckVariablesInPackingLimits(squareSize, vars));

        // and one scalar and packing should fail
        vars.push_back(sh::ShaderVariable(GL_FLOAT));
        EXPECT_FALSE(CheckVariablesInPackingLimits(squareSize, vars));
    }
}

// Scalar type variables can be packed sharing rows with other variables.
TEST(VariablePacking, ReuseRows)
{
    std::vector<sh::ShaderVariable> vars;
    const int kMaxRows = 512;

    // uniform bool u0[129];
    // uniform bool u1[129];
    // uniform bool u2[129];
    // uniform bool u3[129];
    {
        int num_arrays             = 4;
        int num_elements_per_array = kMaxRows / num_arrays + 1;
        for (int ii = 0; ii < num_arrays; ++ii)
        {
            vars.push_back(sh::ShaderVariable(GL_BOOL, num_elements_per_array));
        }
        EXPECT_TRUE(CheckVariablesInPackingLimits(kMaxRows, vars));
    }

    vars.clear();
    // uniform vec2 u0[257];
    // uniform float u1[257];
    // uniform int u1[257];
    {
        int num_elements_per_array = kMaxRows / 2 + 1;
        vars.push_back(sh::ShaderVariable(GL_FLOAT_VEC2, num_elements_per_array));
        vars.push_back(sh::ShaderVariable(GL_FLOAT, num_elements_per_array));
        vars.push_back(sh::ShaderVariable(GL_INT, num_elements_per_array));
        EXPECT_TRUE(CheckVariablesInPackingLimits(kMaxRows, vars));
    }
}

// Check the packer supports and flattens structures.
TEST(VariablePacking, Struct)
{
    std::vector<sh::ShaderVariable> fields;
    const int kMaxRows = 16;

    // Test example from GLSL ES 3.0 spec chapter 11, but with structs
    std::vector<sh::ShaderVariable> vars;
    vars.push_back(sh::ShaderVariable(GL_NONE));

    sh::ShaderVariable &parentStruct = vars[0];
    parentStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT_VEC4));
    parentStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT_MAT3));

    parentStruct.fields.push_back(sh::ShaderVariable(GL_NONE));
    sh::ShaderVariable &innerStruct = parentStruct.fields.back();
    innerStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT_MAT3));
    innerStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT_VEC2, 6));
    innerStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT_VEC2, 4));

    parentStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT_VEC2));
    parentStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT, 3));
    parentStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT, 2));
    parentStruct.fields.push_back(sh::ShaderVariable(GL_FLOAT));

    EXPECT_TRUE(CheckVariablesInPackingLimits(kMaxRows, vars));
}
