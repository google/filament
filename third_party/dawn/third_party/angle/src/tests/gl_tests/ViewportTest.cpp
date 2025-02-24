//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class ViewportTest : public ANGLETest<>
{
  protected:
    ViewportTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mProgram = 0;
    }

    void runNonScissoredTest()
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        runTest();
    }

    void runScissoredTest()
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_SCISSOR_TEST);
        glScissor(0, getWindowHeight() / 2, getWindowWidth(), getWindowHeight() / 2);

        runTest();
    }

    void runTest()
    {
        // Firstly ensure that no errors have been hit.
        EXPECT_GL_NO_ERROR();

        GLint viewportSize[4];
        glGetIntegerv(GL_VIEWPORT, viewportSize);

        // Clear to green. Might be a scissored clear, if scissorSize != window size
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a red quad centered in the middle of the viewport, with dimensions 25% of the size
        // of the viewport.
        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f, 0.25f);

        GLint centerViewportX = viewportSize[0] + (viewportSize[2] / 2);
        GLint centerViewportY = viewportSize[1] + (viewportSize[3] / 2);

        GLint redQuadLeftSideX   = viewportSize[0] + viewportSize[2] * 3 / 8;
        GLint redQuadRightSideX  = viewportSize[0] + viewportSize[2] * 5 / 8;
        GLint redQuadTopSideY    = viewportSize[1] + viewportSize[3] * 3 / 8;
        GLint redQuadBottomSideY = viewportSize[1] + viewportSize[3] * 5 / 8;

        // The midpoint of the viewport should be red.
        checkPixel(centerViewportX, centerViewportY, true);

        // Pixels just inside the red quad should be red.
        checkPixel(redQuadLeftSideX, redQuadTopSideY, true);
        checkPixel(redQuadLeftSideX, redQuadBottomSideY - 1, true);
        checkPixel(redQuadRightSideX - 1, redQuadTopSideY, true);
        checkPixel(redQuadRightSideX - 1, redQuadBottomSideY - 1, true);

        // Pixels just outside the red quad shouldn't be red.
        checkPixel(redQuadLeftSideX - 1, redQuadTopSideY - 1, false);
        checkPixel(redQuadLeftSideX - 1, redQuadBottomSideY, false);
        checkPixel(redQuadRightSideX, redQuadTopSideY - 1, false);
        checkPixel(redQuadRightSideX, redQuadBottomSideY, false);

        // Pixels just within the viewport shouldn't be red.
        checkPixel(viewportSize[0], viewportSize[1], false);
        checkPixel(viewportSize[0], viewportSize[1] + viewportSize[3] - 1, false);
        checkPixel(viewportSize[0] + viewportSize[2] - 1, viewportSize[1], false);
        checkPixel(viewportSize[0] + viewportSize[2] - 1, viewportSize[1] + viewportSize[3] - 1,
                   false);
    }

    void checkPixel(GLint x, GLint y, GLboolean renderedRed)
    {
        // By default, expect the pixel to be black.
        GLint expectedRedChannel   = 0;
        GLint expectedGreenChannel = 0;

        GLint scissorSize[4];
        glGetIntegerv(GL_SCISSOR_BOX, scissorSize);

        EXPECT_GL_NO_ERROR();

        if (scissorSize[0] <= x && x < scissorSize[0] + scissorSize[2] && scissorSize[1] <= y &&
            y < scissorSize[1] + scissorSize[3])
        {
            // If the pixel lies within the scissor rect, then it should have been cleared to green.
            // If we rendered a red square on top of it, then the pixel should be red (the green
            // channel will have been reset to 0).
            expectedRedChannel   = renderedRed ? 255 : 0;
            expectedGreenChannel = renderedRed ? 0 : 255;
        }

        // If the pixel is within the bounds of the window, then we check it. Otherwise we skip it.
        if (0 <= x && x < getWindowWidth() && 0 <= y && y < getWindowHeight())
        {
            EXPECT_PIXEL_EQ(x, y, expectedRedChannel, expectedGreenChannel, 0, 255);
        }
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glUseProgram(mProgram);

        glClearColor(0, 0, 0, 1);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Call glViewport and glScissor with default parameters.
        glScissor(0, 0, getWindowWidth(), getWindowHeight());
        glViewport(0, 0, getWindowWidth(), getWindowHeight());

        glDisable(GL_DEPTH_TEST);
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram;
};

TEST_P(ViewportTest, QuarterWindow)
{
    glViewport(0, 0, getWindowWidth() / 4, getWindowHeight() / 4);

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, QuarterWindowCentered)
{
    glViewport(getWindowWidth() * 3 / 8, getWindowHeight() * 3 / 8, getWindowWidth() / 4,
               getWindowHeight() / 4);

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, FullWindow)
{
    glViewport(0, 0, getWindowWidth(), getWindowHeight());

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, FullWindowOffCenter)
{
    glViewport(-getWindowWidth() / 2, getWindowHeight() / 2, getWindowWidth(), getWindowHeight());

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, DoubleWindow)
{
    glViewport(0, 0, getWindowWidth() * 2, getWindowHeight() * 2);

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, DoubleWindowCentered)
{
    glViewport(-getWindowWidth() / 2, -getWindowHeight() / 2, getWindowWidth() * 2,
               getWindowHeight() * 2);

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, DoubleWindowOffCenter)
{
    glViewport(-getWindowWidth() * 3 / 4, getWindowHeight() * 3 / 4, getWindowWidth(),
               getWindowHeight());

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, TripleWindow)
{
    glViewport(0, 0, getWindowWidth() * 3, getWindowHeight() * 3);

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, TripleWindowCentered)
{
    glViewport(-getWindowWidth(), -getWindowHeight(), getWindowWidth() * 3, getWindowHeight() * 3);

    runNonScissoredTest();

    runScissoredTest();
}

TEST_P(ViewportTest, TripleWindowOffCenter)
{
    glViewport(-getWindowWidth() * 3 / 2, -getWindowHeight() * 3 / 2, getWindowWidth() * 3,
               getWindowHeight() * 3);

    runNonScissoredTest();

    runScissoredTest();
}

// Test line rendering with a non-standard viewport.
TEST_P(ViewportTest, DrawLineWithViewport)
{
    // We assume in the test the width and height are equal and we are tracing
    // the line from bottom left to top right. Verify that all pixels along that line
    // have been traced with green.
    ASSERT_EQ(getWindowWidth(), getWindowHeight());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(program);

    std::vector<Vector3> vertices = {{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};

    const GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Set the viewport.
    GLint quarterWidth  = getWindowWidth() / 4;
    GLint quarterHeight = getWindowHeight() / 4;
    glViewport(quarterWidth, quarterHeight, quarterWidth, quarterHeight);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));

    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    for (GLint x = quarterWidth; x < getWindowWidth() / 2; x++)
    {
        EXPECT_PIXEL_COLOR_EQ(x, x, GLColor::green);
    }
}

// Test line rendering with an overly large viewport.
TEST_P(ViewportTest, DrawLineWithLargeViewport)
{
    // We assume in the test the width and height are equal and we are tracing
    // the line from bottom left to top right. Verify that all pixels along that line
    // have been traced with green.
    ASSERT_EQ(getWindowWidth(), getWindowHeight());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(program);

    std::vector<Vector3> vertices = {{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};

    const GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Set the viewport.
    glViewport(0, 0, getWindowWidth() * 2, getWindowHeight() * 2);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));

    glDisableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    for (GLint x = 0; x < getWindowWidth(); x++)
    {
        EXPECT_PIXEL_COLOR_EQ(x, x, GLColor::green);
    }
}

// Test very large viewport sizes so sanitizers can verify there is no undefined behaviour
TEST_P(ViewportTest, Overflow)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(program);

    std::vector<Vector3> vertices = {{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};

    const GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    constexpr int kMaxSize            = std::numeric_limits<int>::max();
    const int kTestViewportSizes[][4] = {
        {
            kMaxSize,
            kMaxSize,
            1,
            1,
        },
        {
            0,
            0,
            kMaxSize,
            kMaxSize,
        },
        {
            1,
            1,
            kMaxSize,
            kMaxSize,
        },
        {
            kMaxSize,
            kMaxSize,
            kMaxSize,
            kMaxSize,
        },
    };

    for (const int *viewportSize : kTestViewportSizes)
    {
        // Set the viewport.
        glViewport(viewportSize[0], viewportSize[1], viewportSize[2], viewportSize[3]);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));

        glDisableVertexAttribArray(positionLocation);

        ASSERT_GL_NO_ERROR();
    }
}

// Test that viewport dimensions are clamped to implementation-defined maximums
TEST_P(ViewportTest, ClampOnStore)
{
    GLint maxDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxDims);
    ASSERT_GT(maxDims[0], 0);
    ASSERT_GT(maxDims[1], 0);

    GLint viewport[4];
    glViewport(0, 0, 1, 1);
    ASSERT_GL_NO_ERROR();
    glGetIntegerv(GL_VIEWPORT, viewport);
    ASSERT_EQ(viewport[0], 0);
    ASSERT_EQ(viewport[1], 0);
    ASSERT_EQ(viewport[2], 1);
    ASSERT_EQ(viewport[3], 1);

    glViewport(0, 0, -2, 2);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
    glGetIntegerv(GL_VIEWPORT, viewport);
    ASSERT_EQ(viewport[0], 0);
    ASSERT_EQ(viewport[1], 0);
    ASSERT_EQ(viewport[2], 1);
    ASSERT_EQ(viewport[3], 1);

    glViewport(0, 0, 3, -3);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
    glGetIntegerv(GL_VIEWPORT, viewport);
    ASSERT_EQ(viewport[0], 0);
    ASSERT_EQ(viewport[1], 0);
    ASSERT_EQ(viewport[2], 1);
    ASSERT_EQ(viewport[3], 1);

    glViewport(0, 0, maxDims[0], maxDims[1]);
    ASSERT_GL_NO_ERROR();
    glGetIntegerv(GL_VIEWPORT, viewport);
    ASSERT_EQ(viewport[0], 0);
    ASSERT_EQ(viewport[1], 0);
    ASSERT_EQ(viewport[2], maxDims[0]);
    ASSERT_EQ(viewport[3], maxDims[1]);

    glViewport(0, 0, maxDims[0] + 1, maxDims[1] + 1);
    ASSERT_GL_NO_ERROR();
    glGetIntegerv(GL_VIEWPORT, viewport);
    ASSERT_EQ(viewport[0], 0);
    ASSERT_EQ(viewport[1], 0);
    ASSERT_EQ(viewport[2], maxDims[0]);
    ASSERT_EQ(viewport[3], maxDims[1]);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against. D3D11 Feature Level 9 and D3D9 emulate large and negative viewports
// in the vertex shader. We should test both of these as well as D3D11 Feature Level 10_0+.
ANGLE_INSTANTIATE_TEST(ViewportTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES2_D3D11_PRESENT_PATH_FAST(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES2_METAL(),
                       ES2_OPENGL(),
                       ES2_VULKAN());

// This test suite is not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ViewportTest);

}  // namespace
