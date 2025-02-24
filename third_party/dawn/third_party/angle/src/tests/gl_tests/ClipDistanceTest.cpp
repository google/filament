//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ClipDistanceTest.cpp: Test cases for
// GL_APPLE_clip_distance / GL_EXT_clip_cull_distance / GL_ANGLE_clip_cull_distance extensions.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

using namespace angle;

class ClipDistanceAPPLETest : public ANGLETest<>
{
  protected:
    ClipDistanceAPPLETest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setExtensionsEnabled(false);
    }
};

// Query max clip distances and enable, disable states of clip distances
TEST_P(ClipDistanceAPPLETest, StateQuery)
{
    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_APPLE, &maxClipDistances);
    EXPECT_EQ(maxClipDistances, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    auto assertState = [](GLenum pname, bool valid, bool expectedState) {
        EXPECT_EQ(glIsEnabled(pname), valid ? expectedState : false);
        EXPECT_GL_ERROR(valid ? GL_NO_ERROR : GL_INVALID_ENUM);

        GLboolean result = false;
        glGetBooleanv(pname, &result);
        EXPECT_EQ(result, valid ? expectedState : false);
        EXPECT_GL_ERROR(valid ? GL_NO_ERROR : GL_INVALID_ENUM);
    };

    for (size_t i = 0; i < 8; i++)
    {
        assertState(GL_CLIP_DISTANCE0_APPLE + i, false, false);

        glEnable(GL_CLIP_DISTANCE0_APPLE + i);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        assertState(GL_CLIP_DISTANCE0_APPLE + i, false, false);

        glDisable(GL_CLIP_DISTANCE0_APPLE + i);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        assertState(GL_CLIP_DISTANCE0_APPLE + i, false, false);
    }

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    ASSERT_GL_NO_ERROR();

    glGetIntegerv(GL_MAX_CLIP_DISTANCES_APPLE, &maxClipDistances);
    EXPECT_EQ(maxClipDistances, 8);
    EXPECT_GL_NO_ERROR();

    for (size_t i = 0; i < 8; i++)
    {
        assertState(GL_CLIP_DISTANCE0_APPLE + i, true, false);

        glEnable(GL_CLIP_DISTANCE0_APPLE + i);
        EXPECT_GL_NO_ERROR();

        assertState(GL_CLIP_DISTANCE0_APPLE + i, true, true);

        glDisable(GL_CLIP_DISTANCE0_APPLE + i);
        EXPECT_GL_NO_ERROR();

        assertState(GL_CLIP_DISTANCE0_APPLE + i, true, false);
    }
}

// Check that gl_ClipDistance is not defined for fragment shaders
TEST_P(ClipDistanceAPPLETest, FragmentShader)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    constexpr char kVS[] = R"(
#extension GL_APPLE_clip_distance : require

void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);

    gl_ClipDistance[0] = gl_Position.w;
})";

    constexpr char kFS[] = R"(
#extension GL_APPLE_clip_distance : require

void main()
{
    gl_FragColor = vec4(gl_ClipDistance[0], 1.0, 1.0, 1.0);
})";

    GLProgram prg;
    prg.makeRaster(kVS, kFS);
    EXPECT_FALSE(prg.valid());
}

// Check that gl_ClipDistance cannot be redeclared as a global
TEST_P(ClipDistanceAPPLETest, NotVarying)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    constexpr char kVS[] = R"(
#extension GL_APPLE_clip_distance : require

highp float gl_ClipDistance[1];

void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);

    gl_ClipDistance[0] = gl_Position.w;
})";

    GLProgram prg;
    prg.makeRaster(kVS, essl1_shaders::fs::Red());
    EXPECT_FALSE(prg.valid());
}

// Check that gl_ClipDistance size cannot be undefined
TEST_P(ClipDistanceAPPLETest, UndefinedArraySize)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    constexpr char kVS[] = R"(
#extension GL_APPLE_clip_distance : require

void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < 8; i++)
    {
        gl_ClipDistance[i] = gl_Position.w;
    }
})";

    GLProgram prg;
    prg.makeRaster(kVS, essl1_shaders::fs::Red());
    EXPECT_FALSE(prg.valid());
}

// Check that gl_ClipDistance size cannot be more than maximum
TEST_P(ClipDistanceAPPLETest, OutOfRangeArraySize)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_APPLE, &maxClipDistances);

    std::stringstream vsImplicit;
    vsImplicit << R"(#extension GL_APPLE_clip_distance : require
void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    gl_ClipDistance[)"
               << maxClipDistances << R"(] = gl_Position.w;
})";

    std::stringstream vsRedeclared;
    vsRedeclared << R"(#extension GL_APPLE_clip_distance : require
varying highp float gl_ClipDistance[)"
                 << (maxClipDistances + 1) << R"(];
void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    gl_ClipDistance[)"
                 << (maxClipDistances - 1) << R"(] = gl_Position.w;
})";

    std::stringstream vsRedeclaredInvalidIndex;
    vsRedeclaredInvalidIndex << R"(#extension GL_APPLE_clip_distance : require
varying highp float gl_ClipDistance[)"
                             << (maxClipDistances - 2) << R"(];
void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    gl_ClipDistance[)" << (maxClipDistances - 1)
                             << R"(] = gl_Position.w;
})";

    for (auto stream : {&vsImplicit, &vsRedeclared, &vsRedeclaredInvalidIndex})
    {
        GLProgram prg;
        prg.makeRaster(stream->str().c_str(), essl1_shaders::fs::Red());
        EXPECT_FALSE(prg.valid());
    }
}

// Write to one gl_ClipDistance element
TEST_P(ClipDistanceAPPLETest, OneClipDistance)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    constexpr char kVS[] = R"(
#extension GL_APPLE_clip_distance : require

uniform vec4 u_plane;

attribute vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_ClipDistance[0] = dot(gl_Position, u_plane);
})";

    ANGLE_GL_PROGRAM(programRed, kVS, essl1_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_CLIP_DISTANCE0_APPLE);

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    glUniform4f(glGetUniformLocation(programRed, "u_plane"), 1, 0, 0, 0.5);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    // All pixels on the left of the plane x = -0.5 must be blue
    GLuint x      = 0;
    GLuint y      = 0;
    GLuint width  = getWindowWidth() / 4 - 1;
    GLuint height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::blue);

    // All pixels on the right of the plane x = -0.5 must be red
    x      = getWindowWidth() / 4 + 2;
    y      = 0;
    width  = getWindowWidth() - x;
    height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);

    // Clear to green
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    glUniform4f(glGetUniformLocation(programRed, "u_plane"), -1, 0, 0, -0.5);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    // All pixels on the left of the plane x = -0.5 must be red
    x      = 0;
    y      = 0;
    width  = getWindowWidth() / 4 - 1;
    height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);

    // All pixels on the right of the plane x = -0.5 must be green
    x      = getWindowWidth() / 4 + 2;
    y      = 0;
    width  = getWindowWidth() - x;
    height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::green);

    // Disable GL_CLIP_DISTANCE
    glDisable(GL_CLIP_DISTANCE0_APPLE);
    drawQuad(programRed, "a_position", 0);

    // All pixels must be red
    x      = 0;
    y      = 0;
    width  = getWindowWidth();
    height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
}

// Write to each gl_ClipDistance element
TEST_P(ClipDistanceAPPLETest, EachClipDistance)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    for (size_t i = 0; i < 8; i++)
    {
        std::stringstream vertexShaderStr;
        vertexShaderStr << "#extension GL_APPLE_clip_distance : require\n"
                        << "uniform vec4 u_plane;\n"
                        << "attribute vec2 a_position;\n"
                        << "void main()\n"
                        << "{\n"
                        << "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
                        << "    gl_ClipDistance[" << i << "] = dot(gl_Position, u_plane);\n"
                        << "}";

        ANGLE_GL_PROGRAM(programRed, vertexShaderStr.str().c_str(), essl1_shaders::fs::Red());
        glUseProgram(programRed);
        ASSERT_GL_NO_ERROR();

        // Enable the current clip distance, disable all others.
        for (size_t j = 0; j < 8; j++)
        {
            if (j == i)
                glEnable(GL_CLIP_DISTANCE0_APPLE + j);
            else
                glDisable(GL_CLIP_DISTANCE0_APPLE + j);
        }

        // Clear to blue
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw full screen quad with color red
        glUniform4f(glGetUniformLocation(programRed, "u_plane"), 1, 0, 0, 0.5);
        EXPECT_GL_NO_ERROR();
        drawQuad(programRed, "a_position", 0);
        EXPECT_GL_NO_ERROR();

        // All pixels on the left of the plane x = -0.5 must be blue
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth() / 4 - 1;
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::blue);

        // All pixels on the right of the plane x = -0.5 must be red
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);

        // Clear to green
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw full screen quad with color red
        glUniform4f(glGetUniformLocation(programRed, "u_plane"), -1, 0, 0, -0.5);
        EXPECT_GL_NO_ERROR();
        drawQuad(programRed, "a_position", 0);
        EXPECT_GL_NO_ERROR();

        // All pixels on the left of the plane x = -0.5 must be red
        x      = 0;
        y      = 0;
        width  = getWindowWidth() / 4 - 1;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);

        // All pixels on the right of the plane x = -0.5 must be green
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::green);

        // Disable GL_CLIP_DISTANCE
        glDisable(GL_CLIP_DISTANCE0_APPLE + i);
        drawQuad(programRed, "a_position", 0);

        // All pixels must be red
        x      = 0;
        y      = 0;
        width  = getWindowWidth();
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }
}

// Use 8 clip distances to draw an octagon
TEST_P(ClipDistanceAPPLETest, Octagon)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    constexpr char kVS[] = R"(
#extension GL_APPLE_clip_distance : require

attribute vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_ClipDistance[0] = dot(gl_Position, vec4( 1,  0, 0, 0.5));
    gl_ClipDistance[1] = dot(gl_Position, vec4(-1,  0, 0, 0.5));
    gl_ClipDistance[2] = dot(gl_Position, vec4( 0,  1, 0, 0.5));
    gl_ClipDistance[3] = dot(gl_Position, vec4( 0, -1, 0, 0.5));
    gl_ClipDistance[4] = dot(gl_Position, vec4( 1,  1, 0, 0.70710678));
    gl_ClipDistance[5] = dot(gl_Position, vec4( 1, -1, 0, 0.70710678));
    gl_ClipDistance[6] = dot(gl_Position, vec4(-1,  1, 0, 0.70710678));
    gl_ClipDistance[7] = dot(gl_Position, vec4(-1, -1, 0, 0.70710678));
})";

    ANGLE_GL_PROGRAM(programRed, kVS, essl1_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_CLIP_DISTANCE0_APPLE);
    glEnable(GL_CLIP_DISTANCE1_APPLE);
    glEnable(GL_CLIP_DISTANCE2_APPLE);
    glEnable(GL_CLIP_DISTANCE3_APPLE);
    glEnable(GL_CLIP_DISTANCE4_APPLE);
    glEnable(GL_CLIP_DISTANCE5_APPLE);
    glEnable(GL_CLIP_DISTANCE6_APPLE);
    glEnable(GL_CLIP_DISTANCE7_APPLE);

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    // Top edge
    EXPECT_PIXEL_COLOR_EQ(32, 56, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(32, 40, GLColor::red);

    // Top-right edge
    EXPECT_PIXEL_COLOR_EQ(48, 48, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(40, 40, GLColor::red);

    // Right edge
    EXPECT_PIXEL_COLOR_EQ(56, 32, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(40, 32, GLColor::red);

    // Bottom-right edge
    EXPECT_PIXEL_COLOR_EQ(48, 16, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(40, 24, GLColor::red);

    // Bottom edge
    EXPECT_PIXEL_COLOR_EQ(32, 8, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(32, 24, GLColor::red);

    // Bottom-left edge
    EXPECT_PIXEL_COLOR_EQ(16, 16, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(24, 24, GLColor::red);

    // Left edge
    EXPECT_PIXEL_COLOR_EQ(8, 32, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(24, 32, GLColor::red);

    // Top-left edge
    EXPECT_PIXEL_COLOR_EQ(16, 48, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(24, 40, GLColor::red);
}

// Write to 3 clip distances
TEST_P(ClipDistanceAPPLETest, ThreeClipDistances)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    constexpr char kVS[] = R"(
#extension GL_APPLE_clip_distance : require

uniform vec4 u_plane[3];

attribute vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_ClipDistance[0] = dot(gl_Position, u_plane[0]);
    gl_ClipDistance[3] = dot(gl_Position, u_plane[1]);
    gl_ClipDistance[7] = dot(gl_Position, u_plane[2]);
})";

    ANGLE_GL_PROGRAM(programRed, kVS, essl1_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Enable 3 clip distances
    glEnable(GL_CLIP_DISTANCE0_APPLE);
    glEnable(GL_CLIP_DISTANCE3_APPLE);
    glEnable(GL_CLIP_DISTANCE7_APPLE);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    // x = -0.5
    glUniform4f(glGetUniformLocation(programRed, "u_plane[0]"), 1, 0, 0, 0.5);
    // x = 0.5
    glUniform4f(glGetUniformLocation(programRed, "u_plane[1]"), -1, 0, 0, 0.5);
    // x + y = 1
    glUniform4f(glGetUniformLocation(programRed, "u_plane[2]"), -1, -1, 0, 1);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    {
        // All pixels on the left of the plane x = -0.5 must be blue
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth() / 4 - 1;
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::blue);

        // All pixels from the plane x = -0.5 to the plane x = 0 must be red
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() / 2 - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }

    {
        // Check pixels to the right of the plane x = 0
        std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
        glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                     actualColors.data());
        for (int y = 0; y < getWindowHeight(); ++y)
        {
            for (int x = getWindowWidth() / 2; x < getWindowWidth(); ++x)
            {
                const int currentPosition = y * getWindowHeight() + x;

                if (x < getWindowWidth() * 3 / 2 - y - 1 && x < getWindowWidth() * 3 / 4 - 1)
                {
                    // Bottom of the plane x + y = 1 clipped by x = 0.5 plane
                    EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
                }
                else if (x > getWindowWidth() * 3 / 2 - y + 1 || x > getWindowWidth() * 3 / 4 + 1)
                {
                    // Top of the plane x + y = 1 plus right of x = 0.5 plane
                    EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
                }
            }
        }
    }

    // Clear to green
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable gl_ClipDistance[3]
    glDisable(GL_CLIP_DISTANCE3_APPLE);

    // Draw full screen quad with color red
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    {
        // All pixels on the left of the plane x = -0.5 must be green
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth() / 4 - 1;
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::green);

        // All pixels from the plane x = -0.5 to the plane x = 0 must be red
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() / 2 - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }

    // Check pixels to the right of the plane x = 0
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int y = 0; y < getWindowHeight(); ++y)
    {
        for (int x = getWindowWidth() / 2; x < getWindowWidth(); ++x)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (x < getWindowWidth() * 3 / 2 - y - 1)
            {
                // Bottom of the plane x + y = 1
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else if (x > getWindowWidth() * 3 / 2 - y + 1)
            {
                // Top of the plane x + y = 1
                EXPECT_EQ(GLColor::green, actualColors[currentPosition]);
            }
        }
    }
}

// Redeclare gl_ClipDistance in shader with explicit size, also use it in a global function
// outside main()
TEST_P(ClipDistanceAPPLETest, ThreeClipDistancesRedeclared)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_APPLE_clip_distance"));

    constexpr char kVS[] = R"(
#extension GL_APPLE_clip_distance : require

varying highp float gl_ClipDistance[3];

void computeClipDistances(in vec4 position, in vec4 plane[3])
{
    gl_ClipDistance[0] = dot(position, plane[0]);
    gl_ClipDistance[1] = dot(position, plane[1]);
    gl_ClipDistance[2] = dot(position, plane[2]);
}

uniform vec4 u_plane[3];

attribute vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    computeClipDistances(gl_Position, u_plane);
})";

    ANGLE_GL_PROGRAM(programRed, kVS, essl1_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Enable 3 clip distances
    glEnable(GL_CLIP_DISTANCE0_APPLE);
    glEnable(GL_CLIP_DISTANCE1_APPLE);
    glEnable(GL_CLIP_DISTANCE2_APPLE);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    // x = -0.5
    glUniform4f(glGetUniformLocation(programRed, "u_plane[0]"), 1, 0, 0, 0.5);
    // x = 0.5
    glUniform4f(glGetUniformLocation(programRed, "u_plane[1]"), -1, 0, 0, 0.5);
    // x + y = 1
    glUniform4f(glGetUniformLocation(programRed, "u_plane[2]"), -1, -1, 0, 1);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    {
        // All pixels on the left of the plane x = -0.5 must be blue
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth() / 4 - 1;
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::blue);

        // All pixels from the plane x = -0.5 to the plane x = 0 must be red
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() / 2 - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }

    // Check pixels to the right of the plane x = 0
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int y = 0; y < getWindowHeight(); ++y)
    {
        for (int x = getWindowWidth() / 2; x < getWindowWidth(); ++x)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (x < getWindowWidth() * 3 / 2 - y - 1 && x < getWindowWidth() * 3 / 4 - 1)
            {
                // Bottom of the plane x + y = 1 clipped by x = 0.5 plane
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else if (x > getWindowWidth() * 3 / 2 - y + 1 || x > getWindowWidth() * 3 / 4 + 1)
            {
                // Top of the plane x + y = 1 plus right of x = 0.5 plane
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

using ClipCullDistanceTestParams = std::tuple<angle::PlatformParameters, bool>;

std::string PrintToStringParamName(const ::testing::TestParamInfo<ClipCullDistanceTestParams> &info)
{
    std::stringstream ss;
    ss << std::get<0>(info.param);
    if (std::get<1>(info.param))
    {
        ss << "__EXT";
    }
    else
    {
        ss << "__ANGLE";
    }
    return ss.str();
}

class ClipCullDistanceTest : public ANGLETest<ClipCullDistanceTestParams>
{
  protected:
    const bool mCullDistanceSupportRequired;
    const std::string kExtensionName;

    ClipCullDistanceTest()
        : mCullDistanceSupportRequired(::testing::get<1>(GetParam())),
          kExtensionName(::testing::get<1>(GetParam()) ? "GL_EXT_clip_cull_distance"
                                                       : "GL_ANGLE_clip_cull_distance")
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setExtensionsEnabled(false);
    }
};

// Query max clip distances and enable, disable states of clip distances
TEST_P(ClipCullDistanceTest, StateQuery)
{
    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    EXPECT_EQ(maxClipDistances, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint maxCullDistances = 0;
    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    EXPECT_EQ(maxCullDistances, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint maxCombinedClipAndCullDistances = 0;
    glGetIntegerv(GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES_EXT, &maxCombinedClipAndCullDistances);
    EXPECT_EQ(maxCombinedClipAndCullDistances, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    auto assertState = [](GLenum pname, bool valid, bool expectedState) {
        EXPECT_EQ(glIsEnabled(pname), valid ? expectedState : false);
        EXPECT_GL_ERROR(valid ? GL_NO_ERROR : GL_INVALID_ENUM);

        GLboolean result = false;
        glGetBooleanv(pname, &result);
        EXPECT_EQ(result, valid ? expectedState : false);
        EXPECT_GL_ERROR(valid ? GL_NO_ERROR : GL_INVALID_ENUM);
    };

    for (size_t i = 0; i < 8; i++)
    {
        assertState(GL_CLIP_DISTANCE0_EXT + i, false, false);

        glEnable(GL_CLIP_DISTANCE0_EXT + i);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        assertState(GL_CLIP_DISTANCE0_EXT + i, false, false);

        glDisable(GL_CLIP_DISTANCE0_EXT + i);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        assertState(GL_CLIP_DISTANCE0_EXT + i, false, false);
    }

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    ASSERT_GL_NO_ERROR();

    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    EXPECT_GE(maxClipDistances, 8);
    EXPECT_GL_NO_ERROR();

    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    if (mCullDistanceSupportRequired)
    {
        EXPECT_GE(maxCullDistances, 8);
    }
    else
    {
        EXPECT_TRUE(maxCullDistances == 0 || maxCullDistances >= 8);
    }
    EXPECT_GL_NO_ERROR();

    glGetIntegerv(GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES_EXT, &maxCombinedClipAndCullDistances);
    if (mCullDistanceSupportRequired)
    {
        EXPECT_GE(maxCombinedClipAndCullDistances, 8);
    }
    else
    {
        EXPECT_TRUE(maxCombinedClipAndCullDistances == 0 || maxCombinedClipAndCullDistances >= 8);
    }
    EXPECT_GL_NO_ERROR();

    for (size_t i = 0; i < 8; i++)
    {
        assertState(GL_CLIP_DISTANCE0_EXT + i, true, false);

        glEnable(GL_CLIP_DISTANCE0_EXT + i);
        EXPECT_GL_NO_ERROR();

        assertState(GL_CLIP_DISTANCE0_EXT + i, true, true);

        glDisable(GL_CLIP_DISTANCE0_EXT + i);
        EXPECT_GL_NO_ERROR();

        assertState(GL_CLIP_DISTANCE0_EXT + i, true, false);
    }
}

// Check that gl_ClipDistance and gl_CullDistance sizes cannot be undefined
TEST_P(ClipCullDistanceTest, UndefinedArraySize)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVSClip = R"(#version 300 es
#extension )" + kExtensionName +
                          R"( : require

void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < gl_MaxClipDistances; i++)
    {
        gl_ClipDistance[i] = gl_Position.w;
    }
})";

    std::string kVSCull = R"(#version 300 es
#extension )" + kExtensionName +
                          R"( : require

void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < gl_MaxCullDistances; i++)
    {
        gl_CullDistance[i] = gl_Position.w;
    }
})";

    for (auto vs : {kVSClip, kVSCull})
    {
        GLProgram prg;
        prg.makeRaster(vs.c_str(), essl1_shaders::fs::Red());
        EXPECT_FALSE(prg.valid());
    }
}

// Check that shaders with invalid or missing storage qualifiers are rejected
TEST_P(ClipCullDistanceTest, StorageQualifiers)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::stringstream vertexSource;
    auto vs = [this, &vertexSource](std::string name, std::string qualifier) {
        vertexSource.str(std::string());
        vertexSource.clear();
        vertexSource << "#version 300 es\n"
                     << "#extension " << kExtensionName << " : require\n"
                     << qualifier << " highp float " << name << "[1];\n"
                     << "void main()\n"
                     << "{\n"
                     << "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
                     << "    " << name << "[0] = 1.0;\n"
                     << "}";
    };

    std::stringstream fragmentSource;
    auto fs = [this, &fragmentSource](std::string name, std::string qualifier) {
        fragmentSource.str(std::string());
        fragmentSource.clear();
        fragmentSource << "#version 300 es\n"
                       << "#extension " << kExtensionName << " : require\n"
                       << qualifier << " highp float " << name << "[1];\n"
                       << "out highp vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    my_FragColor = vec4(" << name << "[0], 0.0, 0.0, 1.0);\n"
                       << "}";
    };

    auto checkProgram = [=, &vertexSource, &fragmentSource](std::string name,
                                                            std::string qualifierVertex,
                                                            std::string qualifierFragment) {
        GLProgram program;
        vs(name, qualifierVertex);
        fs(name, qualifierFragment);
        program.makeRaster(vertexSource.str().c_str(), fragmentSource.str().c_str());
        return program.valid();
    };

    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    ASSERT_GT(maxClipDistances, 0);

    GLint maxCullDistances = 0;
    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    if (mCullDistanceSupportRequired)
    {
        ASSERT_GT(maxCullDistances, 0);
    }
    else
    {
        ASSERT_GE(maxCullDistances, 0);
    }

    std::pair<std::string, int> entries[2] = {{"gl_ClipDistance", maxClipDistances},
                                              {"gl_CullDistance", maxCullDistances}};
    for (auto entry : entries)
    {
        if (entry.second == 0)
            continue;

        EXPECT_TRUE(checkProgram(entry.first, "out", "in"));

        EXPECT_FALSE(checkProgram(entry.first, "", ""));
        EXPECT_FALSE(checkProgram(entry.first, "", "in"));
        EXPECT_FALSE(checkProgram(entry.first, "", "out"));
        EXPECT_FALSE(checkProgram(entry.first, "in", ""));
        EXPECT_FALSE(checkProgram(entry.first, "in", "in"));
        EXPECT_FALSE(checkProgram(entry.first, "in", "out"));
        EXPECT_FALSE(checkProgram(entry.first, "out", ""));
        EXPECT_FALSE(checkProgram(entry.first, "out", "out"));
    }
}

// Check that array sizes cannot be more than maximum
TEST_P(ClipCullDistanceTest, OutOfRangeArraySize)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    auto test = [this](std::string name, int maxSize) {
        std::stringstream vsImplicit;
        vsImplicit << R"(#version 300 es
        #extension )"
                   << kExtensionName << R"( : require
void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    )" << name << "["
                   << maxSize << R"(] = gl_Position.w;
})";

        std::stringstream vsRedeclared;
        vsRedeclared << R"(#version 300 es
#extension )" << kExtensionName
                     << R"( : require
out highp float )" << name
                     << "[" << (maxSize + 1) << R"(];
void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    )" << name << "[" << (maxSize ? maxSize - 1 : 0)
                     << R"(] = gl_Position.w;
})";

        std::stringstream vsRedeclaredInvalidIndex;
        vsRedeclaredInvalidIndex << R"(#version 300 es
#extension )" << kExtensionName << R"( : require
out highp float )" << name << "[" << (maxSize ? maxSize - 2 : 0)
                                 << R"(];
void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    )" << name << "[" << (maxSize ? maxSize - 1 : 0)
                                 << R"(] = gl_Position.w;
})";

        for (auto stream : {&vsImplicit, &vsRedeclared, &vsRedeclaredInvalidIndex})
        {
            GLProgram prg;
            prg.makeRaster(stream->str().c_str(), essl1_shaders::fs::Red());
            EXPECT_FALSE(prg.valid());
        }
    };

    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    ASSERT_GT(maxClipDistances, 0);

    GLint maxCullDistances = 0;
    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    if (mCullDistanceSupportRequired)
    {
        ASSERT_GT(maxCullDistances, 0);
    }
    else
    {
        ASSERT_GE(maxCullDistances, 0);
    }

    test("gl_ClipDistance", maxClipDistances);
    test("gl_CullDistance", maxCullDistances);
}

// Check that shader validation enforces matching array sizes between shader stages
TEST_P(ClipCullDistanceTest, SizeCheck)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::stringstream vertexSource;
    auto vs = [this, &vertexSource](std::string name, bool declare, int size) {
        vertexSource.str(std::string());
        vertexSource.clear();
        vertexSource << "#version 300 es\n";
        vertexSource << "#extension " << kExtensionName << " : require\n";
        if (declare)
        {
            ASSERT(size);
            vertexSource << "out highp float " << name << "[" << size << "];\n";
        }
        vertexSource << "void main()\n";
        vertexSource << "{\n";
        vertexSource << "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n";
        if (size)
        {
            vertexSource << "    " << name << "[" << (size - 1) << "] = 1.0;\n";
        }
        vertexSource << "}";
    };

    std::stringstream fragmentSource;
    auto fs = [this, &fragmentSource](std::string name, bool declare, int size) {
        fragmentSource.str(std::string());
        fragmentSource.clear();
        fragmentSource << "#version 300 es\n";
        fragmentSource << "#extension " << kExtensionName << " : require\n";
        if (declare)
        {
            ASSERT(size);
            fragmentSource << "in highp float " << name << "[" << size << "];\n";
        }
        fragmentSource << "out highp vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    my_FragColor = vec4(";
        if (size)
        {
            fragmentSource << name << "[" << (size - 1) << "]";
        }
        else
        {
            fragmentSource << "1.0";
        }
        fragmentSource << ", 0.0, 0.0, 1.0);\n";
        fragmentSource << "}\n";
    };

    auto checkProgram = [=, &vertexSource, &fragmentSource](std::string name, bool declareVertex,
                                                            int sizeVertex, bool declareFragment,
                                                            int sizeFragment) {
        GLProgram program;
        vs(name, declareVertex, sizeVertex);
        fs(name, declareFragment, sizeFragment);
        program.makeRaster(vertexSource.str().c_str(), fragmentSource.str().c_str());
        return program.valid();
    };

    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    ASSERT_GT(maxClipDistances, 0);

    GLint maxCullDistances = 0;
    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    if (mCullDistanceSupportRequired)
    {
        ASSERT_GT(maxCullDistances, 0);
    }
    else
    {
        ASSERT_GE(maxCullDistances, 0);
    }

    std::pair<std::string, int> entries[2] = {{"gl_ClipDistance", maxClipDistances},
                                              {"gl_CullDistance", maxCullDistances}};
    for (auto entry : entries)
    {
        const std::string name = entry.first;
        const int maxSize      = entry.second;

        // Any VS array size is valid when the value is not accessed in the fragment shader
        for (int i = 1; i <= maxSize; i++)
        {
            EXPECT_TRUE(checkProgram(name, false, i, false, 0));
            EXPECT_TRUE(checkProgram(name, true, i, false, 0));
        }

        // Any FS array size is invalid when the value is not written in the vertex shader
        for (int i = 1; i <= maxSize; i++)
        {
            EXPECT_FALSE(checkProgram(name, false, 0, false, i));
            EXPECT_FALSE(checkProgram(name, false, 0, true, i));
        }

        // Matching sizes are valid both for redeclared and implicitly sized arrays
        for (int i = 1; i <= maxSize; i++)
        {
            EXPECT_TRUE(checkProgram(name, false, i, false, i));
            EXPECT_TRUE(checkProgram(name, false, i, true, i));
            EXPECT_TRUE(checkProgram(name, true, i, false, i));
            EXPECT_TRUE(checkProgram(name, true, i, true, i));
        }

        // Non-matching sizes are invalid both for redeclared and implicitly sized arrays
        for (int i = 2; i <= maxSize; i++)
        {
            EXPECT_FALSE(checkProgram(name, false, i - 1, false, i));
            EXPECT_FALSE(checkProgram(name, false, i - 1, true, i));
            EXPECT_FALSE(checkProgram(name, true, i - 1, false, i));
            EXPECT_FALSE(checkProgram(name, true, i - 1, true, i));

            EXPECT_FALSE(checkProgram(name, false, i, false, i - 1));
            EXPECT_FALSE(checkProgram(name, false, i, true, i - 1));
            EXPECT_FALSE(checkProgram(name, true, i, false, i - 1));
            EXPECT_FALSE(checkProgram(name, true, i, true, i - 1));
        }

        // Out-of-range sizes are invalid
        {
            EXPECT_FALSE(checkProgram(name, false, 0, false, maxSize + 1));
            EXPECT_FALSE(checkProgram(name, false, maxSize + 1, false, 0));
            EXPECT_FALSE(checkProgram(name, false, maxSize + 1, false, maxSize + 1));
            EXPECT_FALSE(checkProgram(name, false, 0, true, maxSize + 1));
            EXPECT_FALSE(checkProgram(name, false, maxSize + 1, true, maxSize + 1));
            EXPECT_FALSE(checkProgram(name, true, maxSize + 1, false, 0));
            EXPECT_FALSE(checkProgram(name, true, maxSize + 1, false, maxSize + 1));
            EXPECT_FALSE(checkProgram(name, true, maxSize + 1, true, maxSize + 1));
        }
    }
}

// Check that the sum of clip and cull distance array sizes is valid
TEST_P(ClipCullDistanceTest, SizeCheckCombined)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::stringstream vertexSource;
    auto vs = [this, &vertexSource](bool declareClip, int sizeClip, bool declareCull,
                                    int sizeCull) {
        vertexSource.str(std::string());
        vertexSource.clear();
        vertexSource << "#version 300 es\n";
        vertexSource << "#extension " << kExtensionName << " : require\n";
        if (declareClip)
        {
            ASSERT(sizeClip);
            vertexSource << "out highp float gl_ClipDistance[" << sizeClip << "];\n";
        }
        if (declareCull)
        {
            ASSERT(sizeCull);
            vertexSource << "out highp float gl_CullDistance[" << sizeCull << "];\n";
        }
        vertexSource << "void main()\n"
                     << "{\n"
                     << "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
                     << "    gl_ClipDistance[" << (sizeClip - 1) << "] = 1.0;\n"
                     << "    gl_CullDistance[" << (sizeCull - 1) << "] = 1.0;\n"
                     << "}";
    };

    std::stringstream fragmentSource;
    auto fs = [this, &fragmentSource](bool declareClip, int sizeClip, bool declareCull,
                                      int sizeCull) {
        fragmentSource.str(std::string());
        fragmentSource.clear();
        fragmentSource << "#version 300 es\n";
        fragmentSource << "#extension " << kExtensionName << " : require\n";
        if (declareClip)
        {
            ASSERT(sizeClip);
            fragmentSource << "in highp float gl_ClipDistance[" << sizeClip << "];\n";
        }
        if (declareCull)
        {
            ASSERT(sizeClip);
            fragmentSource << "in highp float gl_CullDistance[" << sizeCull << "];\n";
        }
        fragmentSource << "out highp vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    my_FragColor = vec4(\n"
                       << "        gl_ClipDistance[" << (sizeClip - 1) << "],\n"
                       << "        gl_CullDistance[" << (sizeCull - 1) << "],\n"
                       << "        0.0, 1.0);\n"
                       << "}\n";
    };

    auto checkProgram = [=, &vertexSource, &fragmentSource](
                            bool declareVertexClip, bool declareFragmentClip, int sizeClip,
                            bool declareVertexCull, bool declareFragmentCull, int sizeCull) {
        GLProgram program;
        vs(declareVertexClip, sizeClip, declareVertexCull, sizeCull);
        fs(declareVertexClip, sizeClip, declareVertexCull, sizeCull);
        program.makeRaster(vertexSource.str().c_str(), fragmentSource.str().c_str());
        return program.valid();
    };

    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    ASSERT_GT(maxClipDistances, 0);

    GLint maxCullDistances = 0;
    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    if (mCullDistanceSupportRequired)
    {
        ASSERT_GT(maxCullDistances, 0);
    }
    else
    {
        ASSERT_GE(maxCullDistances, 0);
    }

    GLint maxCombinedClipAndCullDistances = 0;
    glGetIntegerv(GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES_EXT, &maxCombinedClipAndCullDistances);
    if (mCullDistanceSupportRequired)
    {
        ASSERT_GT(maxCombinedClipAndCullDistances, 0);
    }
    else
    {
        ASSERT_GE(maxCombinedClipAndCullDistances, 0);
    }

    for (int sizeClip = 1; sizeClip <= maxClipDistances; sizeClip++)
    {
        for (int sizeCull = 1; sizeCull <= maxCullDistances; sizeCull++)
        {
            // clang-format off
            const bool valid = sizeClip + sizeCull <= maxCombinedClipAndCullDistances;
            EXPECT_EQ(checkProgram(false, false, sizeClip, false, false, sizeCull), valid);
            EXPECT_EQ(checkProgram(false, false, sizeClip, false, true,  sizeCull), valid);
            EXPECT_EQ(checkProgram(false, false, sizeClip, true,  false, sizeCull), valid);
            EXPECT_EQ(checkProgram(false, false, sizeClip, true,  true,  sizeCull), valid);
            EXPECT_EQ(checkProgram(false, true,  sizeClip, false, false, sizeCull), valid);
            EXPECT_EQ(checkProgram(false, true,  sizeClip, false, true,  sizeCull), valid);
            EXPECT_EQ(checkProgram(false, true,  sizeClip, true,  false, sizeCull), valid);
            EXPECT_EQ(checkProgram(false, true,  sizeClip, true,  true,  sizeCull), valid);
            EXPECT_EQ(checkProgram(true,  false, sizeClip, false, false, sizeCull), valid);
            EXPECT_EQ(checkProgram(true,  false, sizeClip, false, true,  sizeCull), valid);
            EXPECT_EQ(checkProgram(true,  false, sizeClip, true,  false, sizeCull), valid);
            EXPECT_EQ(checkProgram(true,  false, sizeClip, true,  true,  sizeCull), valid);
            EXPECT_EQ(checkProgram(true,  true,  sizeClip, false, false, sizeCull), valid);
            EXPECT_EQ(checkProgram(true,  true,  sizeClip, false, true,  sizeCull), valid);
            EXPECT_EQ(checkProgram(true,  true,  sizeClip, true,  false, sizeCull), valid);
            EXPECT_EQ(checkProgram(true,  true,  sizeClip, true,  true,  sizeCull), valid);
            // clang-format on
        }
    }
}

// Test that declared but unused built-ins do not cause frontend failures. The internal uniform,
// which is used for passing GL state on some platforms, could be removed when built-ins are not
// accessed.
TEST_P(ClipCullDistanceTest, Unused)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::stringstream vertexSource;
    auto vs = [this, &vertexSource](std::string name) {
        vertexSource.str(std::string());
        vertexSource.clear();
        vertexSource << "#version 300 es\n"
                     << "#extension " << kExtensionName << " : require\n"
                     << "out highp float " << name << "[8];\n"
                     << "void main() { gl_Position = vec4(0.0, 0.0, 0.0, 1.0); }";
    };

    std::stringstream fragmentSource;
    auto fs = [this, &fragmentSource](std::string name, bool declare) {
        fragmentSource.str(std::string());
        fragmentSource.clear();
        fragmentSource << "#version 300 es\n";
        fragmentSource << "#extension " << kExtensionName << " : require\n";
        if (declare)
        {
            fragmentSource << "in highp float " << name << "[8];\n";
        }
        fragmentSource << "out highp vec4 my_FragColor;\n"
                       << "void main() { my_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }\n";
    };

    auto checkProgram = [=, &vertexSource, &fragmentSource](std::string name,
                                                            bool declareFragment) {
        GLProgram program;
        vs(name);
        fs(name, declareFragment);
        program.makeRaster(vertexSource.str().c_str(), fragmentSource.str().c_str());
        return program.valid();
    };

    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    ASSERT_GT(maxClipDistances, 0);

    GLint maxCullDistances = 0;
    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    if (mCullDistanceSupportRequired)
    {
        ASSERT_GT(maxCullDistances, 0);
    }
    else
    {
        ASSERT_GE(maxCullDistances, 0);
    }

    std::pair<std::string, int> entries[2] = {{"gl_ClipDistance", maxClipDistances},
                                              {"gl_CullDistance", maxCullDistances}};
    for (auto entry : entries)
    {
        if (entry.second == 0)
            continue;

        EXPECT_TRUE(checkProgram(entry.first, false));
        EXPECT_TRUE(checkProgram(entry.first, true));
    }
}

// Test that unused gl_ClipDistance does not cause a translator crash
TEST_P(ClipCullDistanceTest, UnusedVertexVaryingNoCrash)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
precision highp float;
void main()
{
  float r = gl_ClipDistance[1] + 0.5;
})";

    GLProgram prg;
    prg.makeRaster(kVS.c_str(), essl3_shaders::fs::Red());

    EXPECT_TRUE(prg.valid());
}

// Test that unused gl_ClipDistance does not cause a translator crash
TEST_P(ClipCullDistanceTest, UnusedFragmentVaryingNoCrash)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kFS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
precision highp float;
out vec4 my_FragColor;
void main()
{
  float r = gl_ClipDistance[1] + 0.5;
})";

    GLProgram prg;
    prg.makeRaster(essl3_shaders::vs::Simple(), kFS.c_str());

    EXPECT_FALSE(prg.valid());
}

// Test that length() does not compile for unsized arrays
TEST_P(ClipCullDistanceTest, UnsizedArrayLength)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    for (std::string name : {"gl_ClipDistance", "gl_CullDistance"})
    {
        std::stringstream vertexSource;
        vertexSource << "#version 300 es\n"
                     << "#extension " << kExtensionName << " : require\n"
                     << "void main() { " << name << ".length(); }";

        GLProgram program;
        program.makeRaster(vertexSource.str().c_str(), essl3_shaders::fs::Red());
        EXPECT_FALSE(program.valid()) << name;
    }
}

// Test that length() returns correct values for sized arrays
TEST_P(ClipCullDistanceTest, SizedArrayLength)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::stringstream vertexSource;
    auto vs = [this, &vertexSource](std::string name, bool declare, int size) {
        vertexSource.str(std::string());
        vertexSource.clear();
        vertexSource << "#version 300 es\n";
        vertexSource << "#extension " << kExtensionName << " : require\n";
        if (declare)
        {
            vertexSource << "out highp float " << name << "[" << size << "];\n";
        }
        vertexSource << "in vec4 a_position;\n"
                     << "out float v_length;\n"
                     << "void main()\n"
                     << "{\n"
                     << "    gl_Position = a_position;\n"
                     << "    v_length = float(" << name << ".length()) / 16.0;\n";
        // Assign all elements to avoid undefined behavior
        for (int i = 0; i < size; ++i)
        {
            vertexSource << "    " << name << "[" << i << "] = 1.0;\n";
        }
        vertexSource << "}";
    };

    std::string kFS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
in mediump float v_length;
out mediump vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(v_length, 0.0, 0.0, 1.0);
})";

    auto checkLength = [this, vs, kFS, &vertexSource](std::string name, bool declare, int size) {
        GLProgram program;
        vs(name, declare, size);
        program.makeRaster(vertexSource.str().c_str(), kFS.c_str());
        ASSERT_TRUE(program.valid()) << name;

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(program, "a_position", 0);
        EXPECT_PIXEL_NEAR(0, 0, size * 16, 0, 0, 255, 1);
    };

    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    ASSERT_GT(maxClipDistances, 0);

    GLint maxCullDistances = 0;
    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    if (mCullDistanceSupportRequired)
    {
        ASSERT_GT(maxCullDistances, 0);
    }
    else
    {
        ASSERT_GE(maxCullDistances, 0);
    }

    std::pair<std::string, int> entries[2] = {{"gl_ClipDistance", maxClipDistances},
                                              {"gl_CullDistance", maxCullDistances}};
    for (auto entry : entries)
    {
        const std::string name = entry.first;
        const int maxSize      = entry.second;
        for (int i = 1; i <= maxSize; i++)
        {
            checkLength(name, false, i);
            checkLength(name, true, i);
        }
    }
}

// Test that pruning clip/cull distance variables does not cause a translator crash
TEST_P(ClipCullDistanceTest, Pruned)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::stringstream vertexSource;
    auto vs = [this, &vertexSource](std::string name, bool doReturn) {
        vertexSource.str(std::string());
        vertexSource.clear();
        vertexSource << "#version 300 es\n";
        vertexSource << "#extension " << kExtensionName << " : require\n";
        vertexSource << "void main()\n"
                     << "{\n"
                     << "    " << (doReturn ? "return;\n" : "") << "    " << name << "[1];\n";
        vertexSource << "}";
    };

    std::stringstream fragmentSource;
    auto fs = [this, &fragmentSource](std::string name) {
        fragmentSource.str(std::string());
        fragmentSource.clear();
        fragmentSource << "#version 300 es\n";
        fragmentSource << "#extension " << kExtensionName << " : require\n";
        fragmentSource << "out mediump vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    my_FragColor = vec4(" << name << "[1]);\n";
        fragmentSource << "}";
    };

    auto checkPruning = [vs, fs, &vertexSource, &fragmentSource](std::string name, bool doReturn) {
        GLProgram program;
        vs(name, doReturn);
        fs(name);
        program.makeRaster(vertexSource.str().c_str(), fragmentSource.str().c_str());
        ASSERT_TRUE(program.valid()) << name << (doReturn ? " after return" : "");
    };

    GLint maxClipDistances = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES_EXT, &maxClipDistances);
    ASSERT_GT(maxClipDistances, 0);
    checkPruning("gl_ClipDistance", false);
    checkPruning("gl_ClipDistance", true);

    GLint maxCullDistances = 0;
    glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
    if (mCullDistanceSupportRequired)
    {
        ASSERT_GT(maxCullDistances, 0);
        checkPruning("gl_CullDistance", false);
        checkPruning("gl_CullDistance", true);
    }
}

// Write to one gl_ClipDistance element
TEST_P(ClipCullDistanceTest, OneClipDistance)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require

uniform vec4 u_plane;

in vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_ClipDistance[0] = dot(gl_Position, u_plane);
})";

    ANGLE_GL_PROGRAM(programRed, kVS.c_str(), essl3_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_CLIP_DISTANCE0_EXT);

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    glUniform4f(glGetUniformLocation(programRed, "u_plane"), 1, 0, 0, 0.5);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    // All pixels on the left of the plane x = -0.5 must be blue
    GLuint x      = 0;
    GLuint y      = 0;
    GLuint width  = getWindowWidth() / 4 - 1;
    GLuint height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::blue);

    // All pixels on the right of the plane x = -0.5 must be red
    x      = getWindowWidth() / 4 + 2;
    y      = 0;
    width  = getWindowWidth() - x;
    height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);

    // Clear to green
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    glUniform4f(glGetUniformLocation(programRed, "u_plane"), -1, 0, 0, -0.5);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    // All pixels on the left of the plane x = -0.5 must be red
    x      = 0;
    y      = 0;
    width  = getWindowWidth() / 4 - 1;
    height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);

    // All pixels on the right of the plane x = -0.5 must be green
    x      = getWindowWidth() / 4 + 2;
    y      = 0;
    width  = getWindowWidth() - x;
    height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::green);

    // Disable GL_CLIP_DISTANCE
    glDisable(GL_CLIP_DISTANCE0_EXT);
    drawQuad(programRed, "a_position", 0);

    // All pixels must be red
    x      = 0;
    y      = 0;
    width  = getWindowWidth();
    height = getWindowHeight();
    EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
}

// Write to each gl_ClipDistance element
TEST_P(ClipCullDistanceTest, EachClipDistance)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    for (size_t i = 0; i < 8; i++)
    {
        std::stringstream vertexShaderStr;
        vertexShaderStr << "#version 300 es\n"
                        << "#extension " << kExtensionName << " : require\n"
                        << "uniform vec4 u_plane;\n"
                        << "in vec2 a_position;\n"
                        << "void main()\n"
                        << "{\n"
                        << "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
                        << "    gl_ClipDistance[" << i << "] = dot(gl_Position, u_plane);\n"
                        << "}";

        ANGLE_GL_PROGRAM(programRed, vertexShaderStr.str().c_str(), essl3_shaders::fs::Red());
        glUseProgram(programRed);
        ASSERT_GL_NO_ERROR();

        // Enable the current clip distance, disable all others.
        for (size_t j = 0; j < 8; j++)
        {
            if (j == i)
                glEnable(GL_CLIP_DISTANCE0_EXT + j);
            else
                glDisable(GL_CLIP_DISTANCE0_EXT + j);
        }

        // Clear to blue
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw full screen quad with color red
        glUniform4f(glGetUniformLocation(programRed, "u_plane"), 1, 0, 0, 0.5);
        EXPECT_GL_NO_ERROR();
        drawQuad(programRed, "a_position", 0);
        EXPECT_GL_NO_ERROR();

        // All pixels on the left of the plane x = -0.5 must be blue
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth() / 4 - 1;
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::blue);

        // All pixels on the right of the plane x = -0.5 must be red
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);

        // Clear to green
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw full screen quad with color red
        glUniform4f(glGetUniformLocation(programRed, "u_plane"), -1, 0, 0, -0.5);
        EXPECT_GL_NO_ERROR();
        drawQuad(programRed, "a_position", 0);
        EXPECT_GL_NO_ERROR();

        // All pixels on the left of the plane x = -0.5 must be red
        x      = 0;
        y      = 0;
        width  = getWindowWidth() / 4 - 1;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);

        // All pixels on the right of the plane x = -0.5 must be green
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::green);

        // Disable GL_CLIP_DISTANCE
        glDisable(GL_CLIP_DISTANCE0_EXT + i);
        drawQuad(programRed, "a_position", 0);

        // All pixels must be red
        x      = 0;
        y      = 0;
        width  = getWindowWidth();
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }
}

// Use 8 clip distances to draw an octagon
TEST_P(ClipCullDistanceTest, Octagon)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require

in vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_ClipDistance[0] = dot(gl_Position, vec4( 1,  0, 0, 0.5));
    gl_ClipDistance[1] = dot(gl_Position, vec4(-1,  0, 0, 0.5));
    gl_ClipDistance[2] = dot(gl_Position, vec4( 0,  1, 0, 0.5));
    gl_ClipDistance[3] = dot(gl_Position, vec4( 0, -1, 0, 0.5));
    gl_ClipDistance[4] = dot(gl_Position, vec4( 1,  1, 0, 0.70710678));
    gl_ClipDistance[5] = dot(gl_Position, vec4( 1, -1, 0, 0.70710678));
    gl_ClipDistance[6] = dot(gl_Position, vec4(-1,  1, 0, 0.70710678));
    gl_ClipDistance[7] = dot(gl_Position, vec4(-1, -1, 0, 0.70710678));
})";

    ANGLE_GL_PROGRAM(programRed, kVS.c_str(), essl3_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_CLIP_DISTANCE0_EXT);
    glEnable(GL_CLIP_DISTANCE1_EXT);
    glEnable(GL_CLIP_DISTANCE2_EXT);
    glEnable(GL_CLIP_DISTANCE3_EXT);
    glEnable(GL_CLIP_DISTANCE4_EXT);
    glEnable(GL_CLIP_DISTANCE5_EXT);
    glEnable(GL_CLIP_DISTANCE6_EXT);
    glEnable(GL_CLIP_DISTANCE7_EXT);

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    // Top edge
    EXPECT_PIXEL_COLOR_EQ(32, 56, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(32, 40, GLColor::red);

    // Top-right edge
    EXPECT_PIXEL_COLOR_EQ(48, 48, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(40, 40, GLColor::red);

    // Right edge
    EXPECT_PIXEL_COLOR_EQ(56, 32, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(40, 32, GLColor::red);

    // Bottom-right edge
    EXPECT_PIXEL_COLOR_EQ(48, 16, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(40, 24, GLColor::red);

    // Bottom edge
    EXPECT_PIXEL_COLOR_EQ(32, 8, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(32, 24, GLColor::red);

    // Bottom-left edge
    EXPECT_PIXEL_COLOR_EQ(16, 16, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(24, 24, GLColor::red);

    // Left edge
    EXPECT_PIXEL_COLOR_EQ(8, 32, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(24, 32, GLColor::red);

    // Top-left edge
    EXPECT_PIXEL_COLOR_EQ(16, 48, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(24, 40, GLColor::red);
}

// Write to 3 clip distances
TEST_P(ClipCullDistanceTest, ThreeClipDistances)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require

uniform vec4 u_plane[3];

in vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_ClipDistance[0] = dot(gl_Position, u_plane[0]);
    gl_ClipDistance[3] = dot(gl_Position, u_plane[1]);
    gl_ClipDistance[7] = dot(gl_Position, u_plane[2]);
})";

    ANGLE_GL_PROGRAM(programRed, kVS.c_str(), essl3_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Enable 3 clip distances
    glEnable(GL_CLIP_DISTANCE0_EXT);
    glEnable(GL_CLIP_DISTANCE3_EXT);
    glEnable(GL_CLIP_DISTANCE7_EXT);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    // x = -0.5
    glUniform4f(glGetUniformLocation(programRed, "u_plane[0]"), 1, 0, 0, 0.5);
    // x = 0.5
    glUniform4f(glGetUniformLocation(programRed, "u_plane[1]"), -1, 0, 0, 0.5);
    // x + y = 1
    glUniform4f(glGetUniformLocation(programRed, "u_plane[2]"), -1, -1, 0, 1);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    {
        // All pixels on the left of the plane x = -0.5 must be blue
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth() / 4 - 1;
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::blue);

        // All pixels from the plane x = -0.5 to the plane x = 0 must be red
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() / 2 - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }

    {
        // Check pixels to the right of the plane x = 0
        std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
        glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                     actualColors.data());
        for (int y = 0; y < getWindowHeight(); ++y)
        {
            for (int x = getWindowWidth() / 2; x < getWindowWidth(); ++x)
            {
                const int currentPosition = y * getWindowHeight() + x;

                if (x < getWindowWidth() * 3 / 2 - y - 1 && x < getWindowWidth() * 3 / 4 - 1)
                {
                    // Bottom of the plane x + y = 1 clipped by x = 0.5 plane
                    EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
                }
                else if (x > getWindowWidth() * 3 / 2 - y + 1 || x > getWindowWidth() * 3 / 4 + 1)
                {
                    // Top of the plane x + y = 1 plus right of x = 0.5 plane
                    EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
                }
            }
        }
    }

    // Clear to green
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable gl_ClipDistance[3]
    glDisable(GL_CLIP_DISTANCE3_EXT);

    // Draw full screen quad with color red
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    {
        // All pixels on the left of the plane x = -0.5 must be green
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth() / 4 - 1;
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::green);

        // All pixels from the plane x = -0.5 to the plane x = 0 must be red
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() / 2 - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }

    // Check pixels to the right of the plane x = 0
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int y = 0; y < getWindowHeight(); ++y)
    {
        for (int x = getWindowWidth() / 2; x < getWindowWidth(); ++x)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (x < getWindowWidth() * 3 / 2 - y - 1)
            {
                // Bottom of the plane x + y = 1
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else if (x > getWindowWidth() * 3 / 2 - y + 1)
            {
                // Top of the plane x + y = 1
                EXPECT_EQ(GLColor::green, actualColors[currentPosition]);
            }
        }
    }
}

// Redeclare gl_ClipDistance in shader with explicit size, also use it in a global function
// outside main()
TEST_P(ClipCullDistanceTest, ThreeClipDistancesRedeclared)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require

out highp float gl_ClipDistance[3];

void computeClipDistances(in vec4 position, in vec4 plane[3])
{
    gl_ClipDistance[0] = dot(position, plane[0]);
    gl_ClipDistance[1] = dot(position, plane[1]);
    gl_ClipDistance[2] = dot(position, plane[2]);
}

uniform vec4 u_plane[3];

in vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    computeClipDistances(gl_Position, u_plane);
})";

    ANGLE_GL_PROGRAM(programRed, kVS.c_str(), essl3_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Enable 3 clip distances
    glEnable(GL_CLIP_DISTANCE0_EXT);
    glEnable(GL_CLIP_DISTANCE1_EXT);
    glEnable(GL_CLIP_DISTANCE2_EXT);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    // x = -0.5
    glUniform4f(glGetUniformLocation(programRed, "u_plane[0]"), 1, 0, 0, 0.5);
    // x = 0.5
    glUniform4f(glGetUniformLocation(programRed, "u_plane[1]"), -1, 0, 0, 0.5);
    // x + y = 1
    glUniform4f(glGetUniformLocation(programRed, "u_plane[2]"), -1, -1, 0, 1);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    {
        // All pixels on the left of the plane x = -0.5 must be blue
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth() / 4 - 1;
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::blue);

        // All pixels from the plane x = -0.5 to the plane x = 0 must be red
        x      = getWindowWidth() / 4 + 2;
        y      = 0;
        width  = getWindowWidth() / 2 - x;
        height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }

    // Check pixels to the right of the plane x = 0
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int y = 0; y < getWindowHeight(); ++y)
    {
        for (int x = getWindowWidth() / 2; x < getWindowWidth(); ++x)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (x < getWindowWidth() * 3 / 2 - y - 1 && x < getWindowWidth() * 3 / 4 - 1)
            {
                // Bottom of the plane x + y = 1 clipped by x = 0.5 plane
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else if (x > getWindowWidth() * 3 / 2 - y + 1 || x > getWindowWidth() * 3 / 4 + 1)
            {
                // Top of the plane x + y = 1 plus right of x = 0.5 plane
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

// Read clip distance varyings in fragment shaders
TEST_P(ClipCullDistanceTest, ClipInterpolation)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
in vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    gl_ClipDistance[0] = dot(gl_Position, vec4( 1,  0, 0, 0.5));
    gl_ClipDistance[1] = dot(gl_Position, vec4(-1,  0, 0, 0.5));
    gl_ClipDistance[2] = dot(gl_Position, vec4( 0,  1, 0, 0.5));
    gl_ClipDistance[3] = dot(gl_Position, vec4( 0, -1, 0, 0.5));
    gl_ClipDistance[4] = gl_ClipDistance[0];
    gl_ClipDistance[5] = gl_ClipDistance[1];
    gl_ClipDistance[6] = gl_ClipDistance[2];
    gl_ClipDistance[7] = gl_ClipDistance[3];
})";

    std::string kFS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
precision highp float;
out vec4 my_FragColor;
void main()
{
    float r = gl_ClipDistance[0] + gl_ClipDistance[1];
    float g = gl_ClipDistance[2] + gl_ClipDistance[3];
    float b = gl_ClipDistance[4] + gl_ClipDistance[5];
    float a = gl_ClipDistance[6] + gl_ClipDistance[7];
    my_FragColor = vec4(r, g, b, a) * 0.5;
})";

    ANGLE_GL_PROGRAM(programRed, kVS.c_str(), kFS.c_str());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_CLIP_DISTANCE0_EXT);
    glEnable(GL_CLIP_DISTANCE1_EXT);
    glEnable(GL_CLIP_DISTANCE2_EXT);
    glEnable(GL_CLIP_DISTANCE3_EXT);
    glEnable(GL_CLIP_DISTANCE4_EXT);
    glEnable(GL_CLIP_DISTANCE5_EXT);
    glEnable(GL_CLIP_DISTANCE6_EXT);
    glEnable(GL_CLIP_DISTANCE7_EXT);

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int x = 0; x < getWindowWidth(); x++)
    {
        for (int y = 0; y < getWindowHeight(); y++)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (x >= getWindowWidth() / 4 && x < getWindowWidth() * 3 / 4 &&
                y >= getWindowHeight() / 4 && y < getWindowHeight() * 3 / 4)
            {
                EXPECT_COLOR_NEAR(GLColor(127, 127, 127, 127), actualColors[currentPosition], 1);
            }
            else
            {
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

// Write to one gl_CullDistance element
TEST_P(ClipCullDistanceTest, OneCullDistance)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require

uniform vec4 u_plane;

in vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_CullDistance[0] = dot(gl_Position, u_plane);
})";

    GLProgram programRed;
    programRed.makeRaster(kVS.c_str(), essl3_shaders::fs::Red());
    if (!mCullDistanceSupportRequired)
    {
        GLint maxCullDistances;
        glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
        if (maxCullDistances == 0)
        {
            ASSERT_FALSE(programRed.valid());
            return;
        }
    }
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    glUniform4f(glGetUniformLocation(programRed, "u_plane"), 1, 0, 0, 0.5);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    {
        // All pixels must be red
        GLuint x      = 0;
        GLuint y      = 0;
        GLuint width  = getWindowWidth();
        GLuint height = getWindowHeight();
        EXPECT_PIXEL_RECT_EQ(x, y, width, height, GLColor::red);
    }

    // Clear to green
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    glUniform4f(glGetUniformLocation(programRed, "u_plane"), 1, 1, 0, 0);
    EXPECT_GL_NO_ERROR();
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    // All pixels on the plane y >= -x must be red
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int x = 0; x < getWindowWidth(); ++x)
    {
        for (int y = 0; y < getWindowHeight(); ++y)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if ((x + y) >= 0)
            {
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else
            {
                EXPECT_EQ(GLColor::green, actualColors[currentPosition]);
            }
        }
    }
}

// Read cull distance varyings in fragment shaders
TEST_P(ClipCullDistanceTest, CullInterpolation)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
in vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    gl_CullDistance[0] = dot(gl_Position, vec4( 1,  0, 0, 1));
    gl_CullDistance[1] = dot(gl_Position, vec4(-1,  0, 0, 1));
    gl_CullDistance[2] = dot(gl_Position, vec4( 0,  1, 0, 1));
    gl_CullDistance[3] = dot(gl_Position, vec4( 0, -1, 0, 1));
    gl_CullDistance[4] = gl_CullDistance[0];
    gl_CullDistance[5] = gl_CullDistance[1];
    gl_CullDistance[6] = gl_CullDistance[2];
    gl_CullDistance[7] = gl_CullDistance[3];
})";

    std::string kFS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
precision highp float;
out vec4 my_FragColor;
void main()
{
    float r = gl_CullDistance[0] + gl_CullDistance[1];
    float g = gl_CullDistance[2] + gl_CullDistance[3];
    float b = gl_CullDistance[4] + gl_CullDistance[5];
    float a = gl_CullDistance[6] + gl_CullDistance[7];
    my_FragColor = vec4(r, g, b, a) * 0.25;
})";

    GLProgram programRed;
    programRed.makeRaster(kVS.c_str(), kFS.c_str());
    if (!mCullDistanceSupportRequired)
    {
        GLint maxCullDistances;
        glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
        if (maxCullDistances == 0)
        {
            ASSERT_FALSE(programRed.valid());
            return;
        }
    }
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    drawQuad(programRed, "a_position", 0, 0.5);
    EXPECT_GL_NO_ERROR();

    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int x = 0; x < getWindowWidth(); x++)
    {
        for (int y = 0; y < getWindowHeight(); y++)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (x >= getWindowWidth() / 4 && x < getWindowWidth() * 3 / 4 &&
                y >= getWindowHeight() / 4 && y < getWindowHeight() * 3 / 4)
            {
                EXPECT_COLOR_NEAR(GLColor(127, 127, 127, 127), actualColors[currentPosition], 1);
            }
            else
            {
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

// Read both clip and cull distance varyings in fragment shaders
TEST_P(ClipCullDistanceTest, ClipCullInterpolation)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
in vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    gl_ClipDistance[0] = dot(gl_Position, vec4( 1,  0, 0, 0.5));
    gl_ClipDistance[1] = dot(gl_Position, vec4(-1,  0, 0, 0.5));
    gl_ClipDistance[2] = dot(gl_Position, vec4( 0,  1, 0, 0.5));
    gl_ClipDistance[3] = dot(gl_Position, vec4( 0, -1, 0, 0.5));
    gl_CullDistance[0] = dot(gl_Position, vec4( 1,  0, 0, 1));
    gl_CullDistance[1] = dot(gl_Position, vec4(-1,  0, 0, 1));
    gl_CullDistance[2] = dot(gl_Position, vec4( 0,  1, 0, 1));
    gl_CullDistance[3] = dot(gl_Position, vec4( 0, -1, 0, 1));
})";

    std::string kFS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require
precision highp float;
out vec4 my_FragColor;
void main()
{
    my_FragColor =
        vec4(gl_ClipDistance[0] + gl_ClipDistance[1],
             gl_ClipDistance[2] + gl_ClipDistance[3],
             gl_CullDistance[0] + gl_CullDistance[1],
             gl_CullDistance[2] + gl_CullDistance[3]) *
        vec4(0.5, 0.5, 0.25, 0.25);
})";

    GLProgram programRed;
    programRed.makeRaster(kVS.c_str(), kFS.c_str());
    if (!mCullDistanceSupportRequired)
    {
        GLint maxCullDistances;
        glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
        if (maxCullDistances == 0)
        {
            ASSERT_FALSE(programRed.valid());
            return;
        }
    }
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_CLIP_DISTANCE0_EXT);
    glEnable(GL_CLIP_DISTANCE1_EXT);
    glEnable(GL_CLIP_DISTANCE2_EXT);
    glEnable(GL_CLIP_DISTANCE3_EXT);

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw full screen quad with color red
    drawQuad(programRed, "a_position", 0);
    EXPECT_GL_NO_ERROR();

    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int x = 0; x < getWindowWidth(); x++)
    {
        for (int y = 0; y < getWindowHeight(); y++)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (x >= getWindowWidth() / 4 && x < getWindowWidth() * 3 / 4 &&
                y >= getWindowHeight() / 4 && y < getWindowHeight() * 3 / 4)
            {
                EXPECT_COLOR_NEAR(GLColor(127, 127, 127, 127), actualColors[currentPosition], 1);
            }
            else
            {
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

// Write to 4 clip distances
TEST_P(ClipCullDistanceTest, FourClipDistances)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require

in vec2 a_position;
uniform vec4 u_plane[4];

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_ClipDistance[0] = dot(gl_Position, u_plane[0]);
    gl_ClipDistance[1] = dot(gl_Position, u_plane[1]);
    gl_ClipDistance[2] = dot(gl_Position, u_plane[2]);
    gl_ClipDistance[3] = dot(gl_Position, u_plane[3]);
})";

    ANGLE_GL_PROGRAM(programRed, kVS.c_str(), essl3_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Enable 3 clip distances
    glEnable(GL_CLIP_DISTANCE0_EXT);
    glEnable(GL_CLIP_DISTANCE2_EXT);
    glEnable(GL_CLIP_DISTANCE3_EXT);
    ASSERT_GL_NO_ERROR();

    // Disable 1 clip distances
    glDisable(GL_CLIP_DISTANCE1_EXT);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    constexpr unsigned int kNumVertices                  = 12;
    const std::array<Vector3, kNumVertices> quadVertices = {
        {Vector3(-1.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f),
         Vector3(1.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, -1.0f, 0.0f),
         Vector3(1.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-1.0f, -1.0f, 0.0f),
         Vector3(-1.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-1.0f, 1.0f, 0.0f)}};

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * kNumVertices, quadVertices.data(),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    GLint positionLocation = glGetAttribLocation(programRed, "a_position");
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    ASSERT_GL_NO_ERROR();

    glEnableVertexAttribArray(positionLocation);
    ASSERT_GL_NO_ERROR();

    // Draw full screen quad and small size triangle with color red
    // y <= 1.0f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[0]"), 0, -1, 0, 1);
    // y >= 0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[1]"), 0, 1, 0, -0.5);
    // y >= 3x-0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[2]"), -3, 1, 0, 0.5);
    // y >= -3x-0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[3]"), 3, 1, 0, 0.5);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, kNumVertices);
    EXPECT_GL_NO_ERROR();

    const int windowWidth   = getWindowWidth();
    const int windowHeight  = getWindowHeight();
    auto checkLeftPlaneFunc = [windowWidth, windowHeight](int x, int y) -> float {
        return (3 * (x - (windowWidth / 2 - 1)) - (windowHeight / 4 + 1) + y);
    };
    auto checkRightPlaneFunc = [windowWidth, windowHeight](int x, int y) -> float {
        return (-3 * (x - (windowWidth / 2)) - (windowHeight / 4 + 1) + y);
    };

    // Only pixels in the triangle must be red
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int x = 0; x < getWindowWidth(); ++x)
    {
        for (int y = 0; y < getWindowHeight(); ++y)
        {
            // The drawing method of Swiftshader and Native graphic card is different. So the
            // compare function doesn't check the value on the line.
            const int currentPosition = y * getWindowHeight() + x;

            if (checkLeftPlaneFunc(x, y) > 0 && checkRightPlaneFunc(x, y) > 0)
            {
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else if (checkLeftPlaneFunc(x, y) < 0 || checkRightPlaneFunc(x, y) < 0)
            {
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

// Write to 4 cull distances
TEST_P(ClipCullDistanceTest, FourCullDistances)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName));

    // SwiftShader bug: http://anglebug.com/42263990
    ANGLE_SKIP_TEST_IF(isSwiftshader());

    std::string kVS = R"(#version 300 es
#extension )" + kExtensionName +
                      R"( : require

uniform vec4 u_plane[4];

in vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_CullDistance[0] = dot(gl_Position, u_plane[0]);
    gl_CullDistance[1] = dot(gl_Position, u_plane[1]);
    gl_CullDistance[2] = dot(gl_Position, u_plane[2]);
    gl_CullDistance[3] = dot(gl_Position, u_plane[3]);
})";

    GLProgram programRed;
    programRed.makeRaster(kVS.c_str(), essl3_shaders::fs::Red());
    if (!mCullDistanceSupportRequired)
    {
        GLint maxCullDistances;
        glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
        if (maxCullDistances == 0)
        {
            ASSERT_FALSE(programRed.valid());
            return;
        }
    }
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    constexpr unsigned int kNumVertices                  = 12;
    const std::array<Vector2, kNumVertices> quadVertices = {
        {Vector2(-1.0f, 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f),
         Vector2(0.0f, 0.0f), Vector2(1.0f, -1.0f), Vector2(1.0f, -1.0f), Vector2(0.0f, 0.0f),
         Vector2(-1.0f, -1.0f), Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f), Vector2(-1.0f, 1.0f)}};

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * kNumVertices, quadVertices.data(),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    GLint positionLocation = glGetAttribLocation(programRed, "a_position");
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    ASSERT_GL_NO_ERROR();

    glEnableVertexAttribArray(positionLocation);
    ASSERT_GL_NO_ERROR();

    // Draw full screen quad and small size triangle with color red
    // y <= 1.0f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[0]"), 0, -1, 0, 1);
    // y >= 0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[1]"), 0, 1, 0, -0.5);
    // y >= 3x-0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[2]"), -3, 1, 0, 0.5);
    // y >= -3x-0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[3]"), 3, 1, 0, 0.5);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, kNumVertices);
    EXPECT_GL_NO_ERROR();

    // Only the bottom triangle must be culled
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int x = 0; x < getWindowWidth(); ++x)
    {
        for (int y = 0; y < getWindowHeight(); ++y)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (y > x || y >= -x + getWindowHeight() - 1)
            {
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else
            {
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

// Verify that EXT_clip_cull_distance works with EXT_geometry_shader
TEST_P(ClipCullDistanceTest, ClipDistanceInteractWithGeometryShader)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName) ||
                       !EnsureGLExtensionEnabled("GL_EXT_geometry_shader"));

    std::string kVS = R"(#version 310 es
#extension )" + kExtensionName +
                      R"( : require
#extension GL_EXT_geometry_shader : require

in vec2 a_position;
uniform vec4 u_plane[4];

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_ClipDistance[0] = dot(gl_Position, u_plane[0]);
    gl_ClipDistance[1] = dot(gl_Position, u_plane[1]);
    gl_ClipDistance[2] = dot(gl_Position, u_plane[2]);
    gl_ClipDistance[3] = dot(gl_Position, u_plane[3]);
})";

    std::string kGS = R"(#version 310 es
#extension )" + kExtensionName +
                      R"( : require
#extension GL_EXT_geometry_shader : require

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in gl_PerVertex {
    highp vec4 gl_Position;
    highp float gl_ClipDistance[];
} gl_in[];

out gl_PerVertex {
    highp vec4 gl_Position;
    highp float gl_ClipDistance[];
};

uniform vec4 u_plane[4];

void GetNewPosition(int i)
{
    gl_Position = 2.0f * gl_in[i].gl_Position;

    for (int index = 0 ; index < 4 ; index++)
    {
        if (gl_in[i].gl_ClipDistance[index] < 0.0f)
        {
            gl_ClipDistance[index] = dot(gl_Position, u_plane[index]);
        }
    }
    EmitVertex();
}

void main()
{
    for (int i = 0 ; i < 3 ; i++)
    {
        GetNewPosition(i);
    }
    EndPrimitive();
})";

    ANGLE_GL_PROGRAM_WITH_GS(programRed, kVS.c_str(), kGS.c_str(), essl31_shaders::fs::Red());
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Enable 3 clip distances
    glEnable(GL_CLIP_DISTANCE0_EXT);
    glEnable(GL_CLIP_DISTANCE2_EXT);
    glEnable(GL_CLIP_DISTANCE3_EXT);
    ASSERT_GL_NO_ERROR();

    // Disable 1 clip distances
    glDisable(GL_CLIP_DISTANCE1_EXT);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    constexpr unsigned int kNumVertices                  = 12;
    const std::array<Vector3, kNumVertices> quadVertices = {
        {Vector3(-0.5f, 0.5f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, 0.5f, 0.0f),
         Vector3(0.5f, 0.5f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, -0.5f, 0.0f),
         Vector3(0.5f, -0.5f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-0.5f, -0.5f, 0.0f),
         Vector3(-0.5f, -0.5f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-0.5f, 0.5f, 0.0f)}};

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * kNumVertices, quadVertices.data(),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    GLint positionLocation = glGetAttribLocation(programRed, "a_position");
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    ASSERT_GL_NO_ERROR();

    glEnableVertexAttribArray(positionLocation);
    ASSERT_GL_NO_ERROR();

    // Draw full screen quad and small size triangle with color red
    // y <= 1.0f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[0]"), 0, -1, 0, 1);
    // y >= 0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[1]"), 0, 1, 0, -0.5);
    // y >= 3x-0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[2]"), -3, 1, 0, 0.5);
    // y >= -3x-0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[3]"), 3, 1, 0, 0.5);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, kNumVertices);
    EXPECT_GL_NO_ERROR();

    const int windowWidth   = getWindowWidth();
    const int windowHeight  = getWindowHeight();
    auto checkLeftPlaneFunc = [windowWidth, windowHeight](int x, int y) -> float {
        return (3 * (x - (windowWidth / 2 - 1)) - (windowHeight / 4 + 1) + y);
    };
    auto checkRightPlaneFunc = [windowWidth, windowHeight](int x, int y) -> float {
        return (-3 * (x - (windowWidth / 2)) - (windowHeight / 4 + 1) + y);
    };

    // Only pixels in the triangle must be red
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int x = 0; x < getWindowWidth(); ++x)
    {
        for (int y = 0; y < getWindowHeight(); ++y)
        {
            // The drawing method of Swiftshader and Native graphic card is different. So the
            // compare function doesn't check the value on the line.
            const int currentPosition = y * getWindowHeight() + x;

            if (checkLeftPlaneFunc(x, y) > 0 && checkRightPlaneFunc(x, y) > 0)
            {
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else if (checkLeftPlaneFunc(x, y) < 0 || checkRightPlaneFunc(x, y) < 0)
            {
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

// Verify that EXT_clip_cull_distance works with EXT_geometry_shader
TEST_P(ClipCullDistanceTest, CullDistanceInteractWithGeometryShader)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(kExtensionName) ||
                       !EnsureGLExtensionEnabled("GL_EXT_geometry_shader"));

    std::string kVS = R"(#version 310 es
#extension )" + kExtensionName +
                      R"( : require
#extension GL_EXT_geometry_shader : require

in vec2 a_position;
uniform vec4 u_plane[4];

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);

    gl_CullDistance[0] = dot(gl_Position, u_plane[0]);
    gl_CullDistance[1] = dot(gl_Position, u_plane[1]);
    gl_CullDistance[2] = dot(gl_Position, u_plane[2]);
    gl_CullDistance[3] = dot(gl_Position, u_plane[3]);
})";

    std::string kGS = R"(#version 310 es
#extension )" + kExtensionName +
                      R"( : require
#extension GL_EXT_geometry_shader : require

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in gl_PerVertex {
    highp vec4 gl_Position;
    highp float gl_CullDistance[];
} gl_in[];

out gl_PerVertex {
    highp vec4 gl_Position;
    highp float gl_CullDistance[];
};

uniform vec4 u_plane[4];

void GetNewPosition(int i)
{
    gl_Position = 2.0f * gl_in[i].gl_Position;

    for (int index = 0 ; index < 4 ; index++)
    {
        if (gl_in[i].gl_CullDistance[index] < 0.0f)
        {
            gl_CullDistance[index] = dot(gl_Position, u_plane[index]);
        }
    }
    EmitVertex();
}

void main()
{
    for (int i = 0 ; i < 3 ; i++)
    {
        GetNewPosition(i);
    }
    EndPrimitive();
})";

    GLProgram programRed;
    programRed.makeRaster(kVS.c_str(), kGS.c_str(), essl3_shaders::fs::Red());
    if (!mCullDistanceSupportRequired)
    {
        GLint maxCullDistances;
        glGetIntegerv(GL_MAX_CULL_DISTANCES_EXT, &maxCullDistances);
        if (maxCullDistances == 0)
        {
            ASSERT_FALSE(programRed.valid());
            return;
        }
    }
    glUseProgram(programRed);
    ASSERT_GL_NO_ERROR();

    // Clear to blue
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    constexpr unsigned int kNumVertices                  = 12;
    const std::array<Vector3, kNumVertices> quadVertices = {
        {Vector3(-0.5f, 0.5f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, 0.5f, 0.0f),
         Vector3(0.5f, 0.5f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, -0.5f, 0.0f),
         Vector3(0.5f, -0.5f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-0.5f, -0.5f, 0.0f),
         Vector3(-0.5f, -0.5f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-0.5f, 0.5f, 0.0f)}};

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * kNumVertices, quadVertices.data(),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    GLint positionLocation = glGetAttribLocation(programRed, "a_position");
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    ASSERT_GL_NO_ERROR();

    glEnableVertexAttribArray(positionLocation);
    ASSERT_GL_NO_ERROR();

    // Draw full screen quad and small size triangle with color red
    // y <= 1.0f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[0]"), 0, -1, 0, 1);
    // y >= 0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[1]"), 0, 1, 0, -0.5);
    // y >= 3x-0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[2]"), -3, 1, 0, 0.5);
    // y >= -3x-0.5f
    glUniform4f(glGetUniformLocation(programRed, "u_plane[3]"), 3, 1, 0, 0.5);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, kNumVertices);
    EXPECT_GL_NO_ERROR();

    // Only pixels in the triangle must be red
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    for (int x = 0; x < getWindowWidth(); ++x)
    {
        for (int y = 0; y < getWindowHeight(); ++y)
        {
            const int currentPosition = y * getWindowHeight() + x;

            if (y > x || y >= -x + getWindowHeight() - 1)
            {
                EXPECT_EQ(GLColor::red, actualColors[currentPosition]);
            }
            else
            {
                EXPECT_EQ(GLColor::blue, actualColors[currentPosition]);
            }
        }
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ClipDistanceAPPLETest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ClipCullDistanceTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(ClipCullDistanceTest,
                                 PrintToStringParamName,
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3,
                                 ANGLE_ALL_TEST_PLATFORMS_ES31);
