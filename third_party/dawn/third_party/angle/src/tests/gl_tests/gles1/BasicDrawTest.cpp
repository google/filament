//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BasicDrawTest.cpp: Tests basic fullscreen quad draw with and without
// GL_TEXTURE_2D enabled.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <vector>

using namespace angle;

class BasicDrawTest : public ANGLETest<>
{
  protected:
    BasicDrawTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    std::vector<float> mPositions = {
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
    };

    void drawRedQuad()
    {
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        EXPECT_GL_NO_ERROR();
        glEnableClientState(GL_VERTEX_ARRAY);
        EXPECT_GL_NO_ERROR();
        glVertexPointer(2, GL_FLOAT, 0, mPositions.data());
        EXPECT_GL_NO_ERROR();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    }
};

// Draws a fullscreen quad with a certain color.
TEST_P(BasicDrawTest, DrawColor)
{
    drawRedQuad();
}

// Checks that textures can be enabled or disabled.
TEST_P(BasicDrawTest, EnableDisableTexture)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Green
    GLubyte texture[] = {
        0x00,
        0xff,
        0x00,
    };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

    // Texturing is disabled; still red;
    drawRedQuad();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Texturing enabled; is green (provided modulate w/ white)
    glEnable(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Check that glClearColorx, glClearDepthx, glLineWidthx, glPolygonOffsetx can work.
TEST_P(BasicDrawTest, DepthTest)
{
    glClearColorx(0x4000, 0x8000, 0x8000, 0x8000);
    EXPECT_GL_NO_ERROR();
    glClearDepthx(0x8000);
    EXPECT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 128, 128, 1.0);

    // Fail Depth Test and can't draw the red triangle
    std::vector<float> Positions = {-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    glEnable(GL_DEPTH_TEST);

    glLineWidthx(0x10000);
    EXPECT_GL_NO_ERROR();
    glPolygonOffsetx(0, 0);
    EXPECT_GL_NO_ERROR();

    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    EXPECT_GL_NO_ERROR();
    glVertexPointer(3, GL_FLOAT, 0, Positions.data());
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 128, 128, 1.0);

    glDisable(GL_DEPTH_TEST);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Check that depth range can be set by glDepthRangex.
TEST_P(BasicDrawTest, SetDepthRangex)
{
    glDepthRangex(0x8000, 0x10000);
    EXPECT_GL_NO_ERROR();

    GLfixed depth_range[2];
    glGetFixedv(GL_DEPTH_RANGE, depth_range);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0x8000, depth_range[0]);
    EXPECT_EQ(0x10000, depth_range[1]);
}

// Check that sample coverage parameters can be set by glSampleCoveragex.
TEST_P(BasicDrawTest, SetSampleCoveragex)
{
    GLfixed isSampleCoverage;
    GLfixed samplecoveragevalue;
    GLfixed samplecoverageinvert;

    glEnable(GL_SAMPLE_COVERAGE);
    EXPECT_GL_NO_ERROR();

    glGetFixedv(GL_SAMPLE_COVERAGE, &isSampleCoverage);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0x10000, isSampleCoverage);

    glSampleCoveragex(0x8000, true);
    EXPECT_GL_NO_ERROR();

    glGetFixedv(GL_SAMPLE_COVERAGE_VALUE, &samplecoveragevalue);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0x8000, samplecoveragevalue);

    glGetFixedv(GL_SAMPLE_COVERAGE_INVERT, &samplecoverageinvert);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0x10000, samplecoverageinvert);
}

ANGLE_INSTANTIATE_TEST_ES1(BasicDrawTest);
