//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CurrentColorTest.cpp: Tests basic usage of glColor4(f|ub|x).

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class CurrentColorTest : public ANGLETest<>
{
  protected:
    CurrentColorTest()
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

// State query: Checks the initial state is correct.
TEST_P(CurrentColorTest, InitialState)
{
    const GLColor32F kFloatWhite(1.0f, 1.0f, 1.0f, 1.0f);
    GLColor32F actualColor;
    glGetFloatv(GL_CURRENT_COLOR, &actualColor.R);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(kFloatWhite, actualColor);
}

// Set test: Checks that the current color is properly set and retrieved.
TEST_P(CurrentColorTest, Set)
{
    float epsilon = 0.00001f;

    glColor4f(0.1f, 0.2f, 0.3f, 0.4f);
    EXPECT_GL_NO_ERROR();

    GLColor32F floatColor;
    glGetFloatv(GL_CURRENT_COLOR, &floatColor.R);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor32F(0.1f, 0.2f, 0.3f, 0.4f), floatColor);

    glColor4ub(0xff, 0x0, 0x55, 0x33);

    glGetFloatv(GL_CURRENT_COLOR, &floatColor.R);
    EXPECT_GL_NO_ERROR();

    EXPECT_NEAR(1.0f, floatColor.R, epsilon);
    EXPECT_NEAR(0.0f, floatColor.G, epsilon);
    EXPECT_NEAR(1.0f / 3.0f, floatColor.B, epsilon);
    EXPECT_NEAR(0.2f, floatColor.A, epsilon);

    glColor4x(0x10000, 0x0, 0x3333, 0x5555);

    glGetFloatv(GL_CURRENT_COLOR, &floatColor.R);
    EXPECT_GL_NO_ERROR();

    EXPECT_NEAR(1.0f, floatColor.R, epsilon);
    EXPECT_NEAR(0.0f, floatColor.G, epsilon);
    EXPECT_NEAR(0.2f, floatColor.B, epsilon);
    EXPECT_NEAR(1.0f / 3.0f, floatColor.A, epsilon);
}

ANGLE_INSTANTIATE_TEST_ES1(CurrentColorTest);
