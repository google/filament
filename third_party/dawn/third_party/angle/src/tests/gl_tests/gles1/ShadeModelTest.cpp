//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShadeModelTest.cpp: Tests the shade model API.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class ShadeModelTest : public ANGLETest<>
{
  protected:
    ShadeModelTest()
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

// Checks that the initial state is correct.
TEST_P(ShadeModelTest, InitialState)
{
    GLint shadeModel = 0;
    glGetIntegerv(GL_SHADE_MODEL, &shadeModel);
    EXPECT_GL_NO_ERROR();

    EXPECT_GLENUM_EQ(GL_SMOOTH, shadeModel);
}

// Negative test for shade model.
TEST_P(ShadeModelTest, Negative)
{
    glShadeModel(GL_TEXTURE_2D);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Checks that the state can be set.
TEST_P(ShadeModelTest, Set)
{
    glShadeModel(GL_FLAT);
    EXPECT_GL_NO_ERROR();

    GLint shadeModel;
    glGetIntegerv(GL_SHADE_MODEL, &shadeModel);
    EXPECT_GL_NO_ERROR();

    EXPECT_GLENUM_EQ(GL_FLAT, shadeModel);

    glShadeModel(GL_SMOOTH);
    EXPECT_GL_NO_ERROR();

    glGetIntegerv(GL_SHADE_MODEL, &shadeModel);
    EXPECT_GL_NO_ERROR();

    EXPECT_GLENUM_EQ(GL_SMOOTH, shadeModel);
}

ANGLE_INSTANTIATE_TEST_ES1(ShadeModelTest);
