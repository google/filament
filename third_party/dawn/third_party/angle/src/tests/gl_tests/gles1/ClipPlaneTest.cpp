//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ClipPlaneTest.cpp: Tests basic usage of user clip planes.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class ClipPlaneTest : public ANGLETest<>
{
  protected:
    ClipPlaneTest()
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
TEST_P(ClipPlaneTest, InitialState)
{
    GLint planeCount = 0;
    glGetIntegerv(GL_MAX_CLIP_PLANES, &planeCount);
    EXPECT_GL_NO_ERROR();

    EXPECT_GE(planeCount, 1);  // spec minimum

    GLfloat clipPlane[4] = {};

    for (int i = 0; i < planeCount; i++)
    {
        GLenum plane = GL_CLIP_PLANE0 + i;

        EXPECT_GL_FALSE(glIsEnabled(plane));
        EXPECT_GL_NO_ERROR();

        glGetClipPlanef(plane, clipPlane);
        EXPECT_GL_NO_ERROR();

        EXPECT_EQ(0.0f, clipPlane[0]);
        EXPECT_EQ(0.0f, clipPlane[1]);
        EXPECT_EQ(0.0f, clipPlane[2]);
        EXPECT_EQ(0.0f, clipPlane[3]);
    }
}

// Negative test for invalid clip planes.
TEST_P(ClipPlaneTest, Negative)
{
    GLint planeCount = 0;
    glGetIntegerv(GL_MAX_CLIP_PLANES, &planeCount);
    EXPECT_GL_NO_ERROR();

    glClipPlanef(GL_CLIP_PLANE0 + planeCount, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glClipPlanef(GL_CLIP_PLANE0 - 1, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glGetClipPlanef(GL_CLIP_PLANE0 + planeCount, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glGetClipPlanef(GL_CLIP_PLANE0 - 1, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Test for setting clip plane values.
TEST_P(ClipPlaneTest, Set)
{
    GLint planeCount = 0;
    glGetIntegerv(GL_MAX_CLIP_PLANES, &planeCount);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < planeCount; i++)
    {
        GLenum plane = GL_CLIP_PLANE0 + i;

        EXPECT_GL_FALSE(glIsEnabled(plane));
        EXPECT_GL_NO_ERROR();

        GLfloat clipPlane[4] = {
            i + 0.0f,
            i + 1.0f,
            i + 2.0f,
            i + 3.0f,
        };

        glClipPlanef(plane, clipPlane);
        EXPECT_GL_NO_ERROR();

        GLfloat actualClipPlane[4] = {};

        glGetClipPlanef(plane, actualClipPlane);
        EXPECT_EQ(clipPlane[0], actualClipPlane[0]);
        EXPECT_EQ(clipPlane[1], actualClipPlane[1]);
        EXPECT_EQ(clipPlane[2], actualClipPlane[2]);
        EXPECT_EQ(clipPlane[3], actualClipPlane[3]);

        glEnable(plane);
        EXPECT_GL_NO_ERROR();
        EXPECT_GL_TRUE(glIsEnabled(plane));
    }
}

ANGLE_INSTANTIATE_TEST_ES1(ClipPlaneTest);
