//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PolygonModeTest.cpp: Test cases for GL_NV_polygon_mode and GL_ANGLE_polygon_mode
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class PolygonModeTest : public ANGLETest<>
{
  protected:
    PolygonModeTest()
    {
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setExtensionsEnabled(false);
        setWindowWidth(16);
        setWindowHeight(16);
    }
};

// New state queries and commands fail without the extension
TEST_P(PolygonModeTest, NoExtension)
{
    {
        GLint mode = 0;
        glGetIntegerv(GL_POLYGON_MODE_NV, &mode);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(mode, 0);

        glPolygonModeNV(GL_FRONT_AND_BACK, GL_FILL_NV);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glPolygonModeANGLE(GL_FRONT_AND_BACK, GL_FILL_NV);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    for (GLenum state : {GL_POLYGON_OFFSET_POINT_NV, GL_POLYGON_OFFSET_LINE_NV})
    {
        EXPECT_FALSE(glIsEnabled(state));
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        GLboolean enabled = true;
        glGetBooleanv(state, &enabled);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_TRUE(enabled);

        glEnable(state);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glDisable(state);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Test NV_polygon_mode entrypoints
TEST_P(PolygonModeTest, ExtensionStateNV)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_NV_polygon_mode"));

    // Default state
    {
        GLint mode = 0;
        glGetIntegerv(GL_POLYGON_MODE_NV, &mode);
        EXPECT_GLENUM_EQ(GL_FILL_NV, mode);
        EXPECT_GL_NO_ERROR();
    }
    for (GLenum state : {GL_POLYGON_OFFSET_POINT_NV, GL_POLYGON_OFFSET_LINE_NV})
    {
        EXPECT_FALSE(glIsEnabled(state));
        EXPECT_GL_NO_ERROR();

        GLboolean enabled = true;
        glGetBooleanv(state, &enabled);
        EXPECT_FALSE(enabled);
        EXPECT_GL_NO_ERROR();
    }

    // Polygon mode state updates
    for (GLenum mode : {GL_POINT_NV, GL_LINE_NV, GL_FILL_NV})
    {
        glPolygonModeNV(GL_FRONT_AND_BACK, mode);
        EXPECT_GL_NO_ERROR();

        GLint result = 0;
        glGetIntegerv(GL_POLYGON_MODE_NV, &result);
        EXPECT_GLENUM_EQ(mode, result);
        EXPECT_GL_NO_ERROR();
    }

    // Polygon offset state updates
    for (GLenum state : {GL_POLYGON_OFFSET_POINT_NV, GL_POLYGON_OFFSET_LINE_NV})
    {
        GLboolean enabled = false;

        glEnable(state);
        EXPECT_GL_NO_ERROR();

        EXPECT_TRUE(glIsEnabled(state));
        EXPECT_GL_NO_ERROR();

        glGetBooleanv(state, &enabled);
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(enabled);

        glDisable(state);
        EXPECT_GL_NO_ERROR();

        EXPECT_FALSE(glIsEnabled(state));
        EXPECT_GL_NO_ERROR();

        glGetBooleanv(state, &enabled);
        EXPECT_GL_NO_ERROR();
        EXPECT_FALSE(enabled);
    }

    // Errors
    {
        glPolygonModeNV(GL_FRONT, GL_FILL_NV);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glPolygonModeNV(GL_BACK, GL_FILL_NV);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glPolygonModeNV(GL_FRONT_AND_BACK, 0);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Test ANGLE_polygon_mode entrypoints
TEST_P(PolygonModeTest, ExtensionStateANGLE)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_polygon_mode"));

    // Default state
    {
        GLint mode = 0;
        glGetIntegerv(GL_POLYGON_MODE_ANGLE, &mode);
        EXPECT_GLENUM_EQ(GL_FILL_ANGLE, mode);
        EXPECT_GL_NO_ERROR();
    }
    for (GLenum state : {GL_POLYGON_OFFSET_LINE_ANGLE})
    {
        EXPECT_FALSE(glIsEnabled(state));
        EXPECT_GL_NO_ERROR();

        GLboolean enabled = true;
        glGetBooleanv(state, &enabled);
        EXPECT_FALSE(enabled);
        EXPECT_GL_NO_ERROR();
    }

    // Polygon mode state updates
    for (GLenum mode : {GL_LINE_ANGLE, GL_FILL_ANGLE})
    {
        glPolygonModeANGLE(GL_FRONT_AND_BACK, mode);
        EXPECT_GL_NO_ERROR();

        GLint result = 0;
        glGetIntegerv(GL_POLYGON_MODE_ANGLE, &result);
        EXPECT_GLENUM_EQ(mode, result);
        EXPECT_GL_NO_ERROR();
    }

    // Polygon offset state updates
    for (GLenum state : {GL_POLYGON_OFFSET_LINE_ANGLE})
    {
        GLboolean enabled = false;

        glEnable(state);
        EXPECT_GL_NO_ERROR();

        EXPECT_TRUE(glIsEnabled(state));
        EXPECT_GL_NO_ERROR();

        glGetBooleanv(state, &enabled);
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(enabled);

        glDisable(state);
        EXPECT_GL_NO_ERROR();

        EXPECT_FALSE(glIsEnabled(state));
        EXPECT_GL_NO_ERROR();

        glGetBooleanv(state, &enabled);
        EXPECT_GL_NO_ERROR();
        EXPECT_FALSE(enabled);
    }

    // Errors
    {
        glPolygonModeANGLE(GL_FRONT, GL_FILL_ANGLE);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glPolygonModeANGLE(GL_BACK, GL_FILL_ANGLE);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glPolygonModeANGLE(GL_FRONT_AND_BACK, 0);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glPolygonModeANGLE(GL_FRONT_AND_BACK, GL_POINT_NV);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glIsEnabled(GL_POLYGON_OFFSET_POINT_NV);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        GLboolean enabled = true;
        glGetBooleanv(GL_POLYGON_OFFSET_POINT_NV, &enabled);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Test line rasterization mode
TEST_P(PolygonModeTest, DrawLines)
{
    const bool extensionNV    = EnsureGLExtensionEnabled("GL_NV_polygon_mode");
    const bool extensionANGLE = EnsureGLExtensionEnabled("GL_ANGLE_polygon_mode");
    ANGLE_SKIP_TEST_IF(!extensionNV && !extensionANGLE);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

    const int w = getWindowWidth();
    const int h = getWindowHeight();
    ASSERT(w == h);

    for (bool useNV : {true, false})
    {
        if (useNV && !extensionNV)
        {
            continue;
        }

        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::red);

        // Draw green quad with lines
        if (useNV)
        {
            glPolygonModeNV(GL_FRONT_AND_BACK, GL_LINE_NV);
        }
        else
        {
            glPolygonModeANGLE(GL_FRONT_AND_BACK, GL_LINE_ANGLE);
        }
        glUniform4f(colorLocation, 0.0, 1.0, 0.0, 1.0);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);

        // Nothing was drawn inside triangles
        EXPECT_PIXEL_RECT_EQ(1, 1, 5, 5, GLColor::red);
        EXPECT_PIXEL_RECT_EQ(9, 9, 5, 5, GLColor::red);

        // Main diagonal was drawn
        std::vector<GLColor> colors(w * h);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
        for (int i = 0; i < w; i++)
        {
            const int x = i;
            const int y = w - 1 - i;
            EXPECT_EQ(GLColor::green, colors[y * w + x]) << "x: " << x << " y: " << y;
        }

        // Draw blue quad with triangles
        if (useNV)
        {
            glPolygonModeNV(GL_FRONT_AND_BACK, GL_FILL_NV);
        }
        else
        {
            glPolygonModeANGLE(GL_FRONT_AND_BACK, GL_FILL_ANGLE);
        }
        glUniform4f(colorLocation, 0.0, 0.0, 1.0, 1.0);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
        EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::blue);
    }
}

// Test line rasterization mode with depth offset
TEST_P(PolygonModeTest, DrawLinesWithDepthOffset)
{
    const bool extensionNV    = EnsureGLExtensionEnabled("GL_NV_polygon_mode");
    const bool extensionANGLE = EnsureGLExtensionEnabled("GL_ANGLE_polygon_mode");
    ANGLE_SKIP_TEST_IF(!extensionNV && !extensionANGLE);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

    const int w = getWindowWidth();
    const int h = getWindowHeight();
    ASSERT(w == h);

    glEnable(GL_DEPTH_TEST);

    for (bool useNV : {true, false})
    {
        if (useNV && !extensionNV)
        {
            continue;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw red quad filled
        if (useNV)
        {
            glPolygonModeNV(GL_FRONT_AND_BACK, GL_FILL_NV);
        }
        else
        {
            glPolygonModeANGLE(GL_FRONT_AND_BACK, GL_FILL_ANGLE);
        }
        glUniform4f(colorLocation, 1.0, 0.0, 0.0, 1.0);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);

        // Draw green quad using lines with offset failing the depth test
        if (useNV)
        {
            glEnable(GL_POLYGON_OFFSET_LINE_NV);
            glPolygonModeNV(GL_FRONT_AND_BACK, GL_LINE_NV);
        }
        else
        {
            glEnable(GL_POLYGON_OFFSET_LINE_ANGLE);
            glPolygonModeANGLE(GL_FRONT_AND_BACK, GL_LINE_ANGLE);
        }
        glPolygonOffset(0.0, 2.0);
        glUniform4f(colorLocation, 0.0, 1.0, 0.0, 1.0);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);

        // Depth test must fail
        EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::red);

        // Draw green quad with triangles
        if (useNV)
        {
            glPolygonModeNV(GL_FRONT_AND_BACK, GL_FILL_NV);
        }
        else
        {
            glPolygonModeANGLE(GL_FRONT_AND_BACK, GL_FILL_ANGLE);
        }
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);

        // Change the offset so that depth test passes
        glPolygonOffset(0.0, -2.0);

        // Draw blue quad with lines
        if (useNV)
        {
            glPolygonModeNV(GL_FRONT_AND_BACK, GL_LINE_NV);
        }
        else
        {
            glPolygonModeANGLE(GL_FRONT_AND_BACK, GL_LINE_ANGLE);
        }
        glUniform4f(colorLocation, 0.0, 0.0, 1.0, 1.0);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);

        // Main diagonal was drawn
        std::vector<GLColor> colors(w * h);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
        for (int i = 0; i < w; i++)
        {
            const int x = i;
            const int y = w - 1 - i;
            EXPECT_EQ(GLColor::blue, colors[y * w + x]) << "x: " << x << " y: " << y;
        }
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(PolygonModeTest);
