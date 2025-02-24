//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ColorMaskTest.cpp: Test GLES functionality related to color masks, particularly an AMD D3D9
// driver bug.

#include "test_utils/ANGLETest.h"

namespace angle
{
class ColorMaskTest : public ANGLETest<>
{
  protected:
    ColorMaskTest() : mProgram(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        ASSERT_NE(0u, mProgram) << "shader compilation failed.";

        mColorUniform = glGetUniformLocation(mProgram, essl1_shaders::ColorUniform());
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram     = 0;
    GLint mColorUniform = -1;
};

// Some ATI cards have a bug where a draw with a zero color write mask can cause later draws to have
// incorrect results. Test to make sure this bug is not exposed.
TEST_P(ColorMaskTest, AMDZeroColorMaskBug)
{
    int x = getWindowWidth() / 2;
    int y = getWindowHeight() / 2;

    // Clear to blue
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(x, y, 0, 0, 255, 255);

    // Draw a quad with all colors masked and blending disabled, should remain blue
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_BLEND);
    glUseProgram(mProgram);
    glUniform4f(mColorUniform, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_EQ(x, y, 0, 0, 255, 255);

    // Re-enable the color mask, should be red (with blend disabled, the red should overwrite
    // everything)
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glUseProgram(mProgram);
    glUniform4f(mColorUniform, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_EQ(x, y, 255, 0, 0, 0);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against. D3D11 Feature Level 9_3 uses different D3D formats for vertex
// attribs compared to Feature Levels 10_0+, so we should test them separately.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ColorMaskTest);

}  // namespace angle
