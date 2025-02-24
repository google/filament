//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MatrixStackTest.cpp: Tests basic usage of gl(Push|Pop)Matrix.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <vector>

using namespace angle;

class MatrixStackTest : public ANGLETest<>
{
  protected:
    MatrixStackTest()
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

// State query: Checks the initial state is correct; that there is only one matrix on the stack.
TEST_P(MatrixStackTest, InitialState)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    EXPECT_GL_ERROR(GL_STACK_UNDERFLOW);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    EXPECT_GL_ERROR(GL_STACK_UNDERFLOW);

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    EXPECT_GL_ERROR(GL_STACK_UNDERFLOW);
}

// Tests that caps are greater than or equal to spec minimums and that we can actually push them
// that much.
TEST_P(MatrixStackTest, Limits)
{
    GLint modelviewMatrixMax;
    GLint projectionMatrixMax;
    GLint textureMatrixMax;
    GLint textureUnits;  // For iterating over texture matrices

    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &modelviewMatrixMax);
    glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &projectionMatrixMax);
    glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &textureMatrixMax);
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &textureUnits);

    EXPECT_GE(modelviewMatrixMax, 16);
    EXPECT_GE(projectionMatrixMax, 2);
    EXPECT_GE(textureMatrixMax, 2);

    glMatrixMode(GL_MODELVIEW);

    for (int i = 0; i < modelviewMatrixMax; i++)
    {
        glPushMatrix();
        if (i == modelviewMatrixMax - 1)
        {
            EXPECT_GL_ERROR(GL_STACK_OVERFLOW);
        }
        else
        {
            EXPECT_GL_NO_ERROR();
        }
    }

    glMatrixMode(GL_PROJECTION);

    for (int i = 0; i < projectionMatrixMax; i++)
    {
        glPushMatrix();
        if (i == projectionMatrixMax - 1)
        {
            EXPECT_GL_ERROR(GL_STACK_OVERFLOW);
        }
        else
        {
            EXPECT_GL_NO_ERROR();
        }
    }

    glMatrixMode(GL_TEXTURE);

    for (int i = 0; i < textureUnits; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        for (int j = 0; j < textureMatrixMax; j++)
        {
            glPushMatrix();
            if (j == textureMatrixMax - 1)
            {
                EXPECT_GL_ERROR(GL_STACK_OVERFLOW);
            }
            else
            {
                EXPECT_GL_NO_ERROR();
            }
        }
    }
}

ANGLE_INSTANTIATE_TEST_ES1(MatrixStackTest);
