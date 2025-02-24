//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MatrixLoadTest.cpp: Tests basic usage of glColor4(f|ub|x).

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "common/matrix_utils.h"
#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class MatrixLoadTest : public ANGLETest<>
{
  protected:
    MatrixLoadTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

// Checks that a matrix can be loaded.
TEST_P(MatrixLoadTest, Basic)
{
    angle::Mat4 testMatrix(0.0f, 4.0f, 8.0f, 12.0f, 1.0f, 5.0f, 9.0f, 10.0f, 2.0f, 6.0f, 10.0f,
                           14.0f, 3.0f, 7.0f, 11.0f, 15.0f);

    angle::Mat4 outputMatrix;

    glLoadMatrixf(testMatrix.data());
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(testMatrix, outputMatrix);
}

// Checks that loading a matrix doesn't affect the matrix below in the stack.
TEST_P(MatrixLoadTest, PushPop)
{
    glPushMatrix();

    angle::Mat4 testMatrix(0.0f, 4.0f, 8.0f, 12.0f, 1.0f, 5.0f, 9.0f, 10.0f, 2.0f, 6.0f, 10.0f,
                           14.0f, 3.0f, 7.0f, 11.0f, 15.0f);

    angle::Mat4 outputMatrix;

    glLoadMatrixf(testMatrix.data());
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(testMatrix, outputMatrix);

    glPopMatrix();

    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(angle::Mat4(), outputMatrix);
}

// Checks that matrices can be loaded for each type of matrix.
TEST_P(MatrixLoadTest, Modes)
{
    angle::Mat4 testMatrix(0.0f, 4.0f, 8.0f, 12.0f, 1.0f, 5.0f, 9.0f, 10.0f, 2.0f, 6.0f, 10.0f,
                           14.0f, 3.0f, 7.0f, 11.0f, 15.0f);
    angle::Mat4 outputMatrix;

    std::vector<std::pair<GLenum, GLenum>> modeTypes = {{GL_PROJECTION, GL_PROJECTION_MATRIX},
                                                        {GL_MODELVIEW, GL_MODELVIEW_MATRIX},
                                                        {GL_TEXTURE, GL_TEXTURE_MATRIX}};

    for (auto modeType : modeTypes)
    {
        auto mode       = modeType.first;
        auto matrixType = modeType.second;

        glMatrixMode(mode);
        EXPECT_GL_NO_ERROR();
        glGetFloatv(matrixType, outputMatrix.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(angle::Mat4(), outputMatrix);

        glLoadMatrixf(testMatrix.data());
        glGetFloatv(matrixType, outputMatrix.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(testMatrix, outputMatrix);
    }
}

ANGLE_INSTANTIATE_TEST_ES1(MatrixLoadTest);
