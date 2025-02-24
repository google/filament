//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PointParameterTest.cpp: Tests basic usage of GLES1 point parameters.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class PointParameterTest : public ANGLETest<>
{
  protected:
    PointParameterTest()
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

// Checks that disable / enable works as expected.
TEST_P(PointParameterTest, InitialState)
{
    GLfloat params[3] = {};
    glGetFloatv(GL_POINT_SIZE, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1.0f, params[0]);

    EXPECT_GL_FALSE(glIsEnabled(GL_POINT_SMOOTH));
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_POINT_SIZE_MIN, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0.0f, params[0]);

    // The user-specified point size, GL_POINT_SIZE_MAX,
    // is initially equal to the implementation maximum.
    GLfloat range[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, range);
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_POINT_SIZE_MAX, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(range[1], params[0]);

    glGetFloatv(GL_POINT_FADE_THRESHOLD_SIZE, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1.0f, params[0]);

    glGetFloatv(GL_POINT_DISTANCE_ATTENUATION, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1.0f, params[0]);
    EXPECT_EQ(0.0f, params[1]);
    EXPECT_EQ(0.0f, params[2]);

    EXPECT_GL_FALSE(glIsEnabled(GL_POINT_SPRITE_OES));
    EXPECT_GL_NO_ERROR();
}

// Negative test for parameter names
TEST_P(PointParameterTest, NegativeEnum)
{
    glPointParameterf(0, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glPointParameterfv(0, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glPointParameterf(GL_POINT_DISTANCE_ATTENUATION, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Negative test for parameter values
TEST_P(PointParameterTest, NegativeValue)
{
    glPointParameterf(GL_POINT_SIZE_MIN, -1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glPointParameterf(GL_POINT_SIZE_MAX, -1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, -1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    GLfloat badPointDistanceAttenuation[3] = {1.0f, -1.0f, 1.0f};
    glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, badPointDistanceAttenuation);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glPointSize(-1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Tests that point parameters can be set.
TEST_P(PointParameterTest, Set)
{
    GLfloat params[3] = {};

    GLfloat range[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, range);
    EXPECT_GL_NO_ERROR();

    if (range[0] != range[1])
    {
        glPointSize(range[0]);
        EXPECT_GL_NO_ERROR();
        glGetFloatv(GL_POINT_SIZE, params);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(range[0], params[0]);

        glPointSize(range[1]);
        EXPECT_GL_NO_ERROR();
        glGetFloatv(GL_POINT_SIZE, params);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(range[1], params[0]);
    }

    glPointParameterf(GL_POINT_SIZE_MIN, 1.0f);
    EXPECT_GL_NO_ERROR();
    glPointParameterf(GL_POINT_SIZE_MAX, 10.0f);
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_POINT_SIZE_MIN, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1.0f, params[0]);

    glGetFloatv(GL_POINT_SIZE_MAX, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(10.0f, params[0]);

    glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 5.0f);
    EXPECT_GL_NO_ERROR();
    glGetFloatv(GL_POINT_FADE_THRESHOLD_SIZE, params);
    EXPECT_EQ(5.0f, params[0]);

    GLfloat distanceAttenuation[3] = {1.0f, 2.0f, 3.0f};
    glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation);
    EXPECT_GL_NO_ERROR();
    glGetFloatv(GL_POINT_DISTANCE_ATTENUATION, params);
    EXPECT_EQ(1.0f, params[0]);
    EXPECT_EQ(2.0f, params[1]);
    EXPECT_EQ(3.0f, params[2]);
}

ANGLE_INSTANTIATE_TEST_ES1(PointParameterTest);
