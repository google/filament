//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FogTest.cpp: Tests basic usage of glFog.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class FogTest : public ANGLETest<>
{
  protected:
    FogTest()
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

// Initial state check.
TEST_P(FogTest, InitialState)
{
    EXPECT_GL_FALSE(glIsEnabled(GL_FOG));
    EXPECT_GL_NO_ERROR();

    GLint fogMode;
    glGetIntegerv(GL_FOG_MODE, &fogMode);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_EXP, fogMode);

    GLfloat fogModeAsFloat;
    glGetFloatv(GL_FOG_MODE, &fogModeAsFloat);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_EXP, fogModeAsFloat);

    GLfloat fogStart;
    GLfloat fogEnd;
    GLfloat fogDensity;

    glGetFloatv(GL_FOG_START, &fogStart);
    EXPECT_GL_NO_ERROR();
    glGetFloatv(GL_FOG_END, &fogEnd);
    EXPECT_GL_NO_ERROR();
    glGetFloatv(GL_FOG_DENSITY, &fogDensity);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(0.0f, fogStart);
    EXPECT_EQ(1.0f, fogEnd);
    EXPECT_EQ(1.0f, fogDensity);

    const GLColor32F kInitialFogColor(0.0f, 0.0f, 0.0f, 0.0f);
    GLColor32F initialFogColor;
    glGetFloatv(GL_FOG_COLOR, &initialFogColor.R);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(kInitialFogColor, initialFogColor);
}

// Negative test for parameter names.
TEST_P(FogTest, NegativeParameter)
{
    glFogfv(0, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Negative test for parameter values.
TEST_P(FogTest, NegativeValues)
{
    glFogf(GL_FOG_MODE, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glFogf(GL_FOG_DENSITY, -1.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Checks that fog state can be set.
TEST_P(FogTest, Set)
{
    GLfloat fogValue[4] = {};

    glFogf(GL_FOG_MODE, GL_EXP2);
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_FOG_MODE, fogValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_EXP2, fogValue[0]);

    glFogf(GL_FOG_DENSITY, 2.0f);
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_FOG_DENSITY, fogValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(2.0f, fogValue[0]);

    glFogf(GL_FOG_START, 2.0f);
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_FOG_START, fogValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(2.0f, fogValue[0]);

    glFogf(GL_FOG_END, 2.0f);
    EXPECT_GL_NO_ERROR();

    glGetFloatv(GL_FOG_END, fogValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(2.0f, fogValue[0]);

    const GLColor32F testColor(0.1f, 0.2f, 0.3f, 0.4f);
    glFogfv(GL_FOG_COLOR, &testColor.R);
    EXPECT_GL_NO_ERROR();
    glGetFloatv(GL_FOG_COLOR, fogValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0.1f, fogValue[0]);
    EXPECT_EQ(0.2f, fogValue[1]);
    EXPECT_EQ(0.3f, fogValue[2]);
    EXPECT_EQ(0.4f, fogValue[3]);
}

class FogBlendTest : public ANGLETest<>
{
  protected:
    FogBlendTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

// Draws a circle with a shadow effect using fog and blending. As seen in Street Fighter IV.
TEST_P(FogBlendTest, ShadowEffect)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int32_t w = getWindowWidth();
    int32_t h = getWindowHeight();

    int32_t radius = h / 4;

    std::vector<GLColor> colors;
    for (int32_t y = 0; y < h; y++)
    {
        for (int32_t x = 0; x < w; x++)
        {
            int32_t centerDistanceX = x - w / 2;
            int32_t centerDistanceY = y - h / 2;

            float centerDistance =
                sqrt(centerDistanceX * centerDistanceX + centerDistanceY * centerDistanceY);

            if (centerDistance > static_cast<float>(radius))
            {
                colors.push_back(GLColor::transparentBlack);
            }
            else
            {
                colors.push_back(GLColor::green);
            }
        }
    }

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());

    glClearColor(GLColor::red.R, GLColor::red.G, GLColor::red.B, GLColor::red.A);
    glClearDepthf(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0, w, h, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glShadeModel(GL_FLAT);

    // TODO(http://anglebug.com/42266063): This currently renders incorrectly on ANGLE
    // Draw shadow effect
    glEnable(GL_FOG);
    const GLfloat fogColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);

    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, -1);
    glFogf(GL_FOG_END, -0.1);

    GLfloat shadowOffset = 32.0f;
    GLfloat wf           = static_cast<GLfloat>(w);
    GLfloat hf           = static_cast<GLfloat>(h);

    const GLfloat shadowPositions[8] = {shadowOffset,      0, shadowOffset,      hf,
                                        wf + shadowOffset, 0, wf + shadowOffset, hf};
    glVertexPointer(2, GL_FLOAT, 0, shadowPositions);

    const GLfloat texcoords[8] = {0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0};
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

    glColor4f(1, 1, 1, 0.6);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Draws color circle
    glDisable(GL_FOG);

    const GLfloat positions[8] = {
        0, 0, 0, hf, wf, 0, wf, hf,
    };
    glVertexPointer(2, GL_FLOAT, 0, positions);

    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

    glColor4f(1, 1, 1, 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // The circle should be green
    EXPECT_PIXEL_COLOR_NEAR(w / 2, h / 2, GLColor::green, 0);

    // The background should be red
    EXPECT_PIXEL_COLOR_NEAR(w / 8, h / 8, GLColor::red, 0);
    EXPECT_PIXEL_COLOR_NEAR(w - w / 8, h / 8, GLColor::red, 0);
    EXPECT_PIXEL_COLOR_NEAR(w / 8, h - h / 8, GLColor::red, 0);
    EXPECT_PIXEL_COLOR_NEAR(w - w / 8, h - h / 8, GLColor::red, 0);

    // The shadow should be darkened red
    float shadowCenterX = w / 2 + radius + shadowOffset / 2.0f;
    EXPECT_PIXEL_COLOR_NEAR(shadowCenterX, h / 2, GLColor(102u, 0, 0, 194u), 0);
}

ANGLE_INSTANTIATE_TEST_ES1(FogTest);
ANGLE_INSTANTIATE_TEST_ES1(FogBlendTest);
