//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MatrixMultTest.cpp: Tests basic usage of glMultMatrix(f|x).

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "common/matrix_utils.h"
#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class MatrixMultTest : public ANGLETest<>
{
  protected:
    MatrixMultTest()
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

// Multiplies identity matrix with itself.
TEST_P(MatrixMultTest, Identity)
{
    angle::Mat4 testMatrix;
    angle::Mat4 outputMatrix;

    glMultMatrixf(testMatrix.data());
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(angle::Mat4(), outputMatrix);
}

// Multiplies translation matrix and checks matrix underneath in stack is not affected.
TEST_P(MatrixMultTest, Translation)
{
    glPushMatrix();

    angle::Mat4 testMatrix = angle::Mat4::Translate(angle::Vector3(1.0f, 0.0f, 0.0f));

    angle::Mat4 outputMatrix;

    glMultMatrixf(testMatrix.data());
    EXPECT_GL_NO_ERROR();

    glMultMatrixf(testMatrix.data());
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(angle::Mat4::Translate(angle::Vector3(2.0f, 0.0f, 0.0f)), outputMatrix);

    glPopMatrix();

    glGetFloatv(GL_MODELVIEW_MATRIX, outputMatrix.data());
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(angle::Mat4(), outputMatrix);
}

ANGLE_INSTANTIATE_TEST_ES1(MatrixMultTest);
