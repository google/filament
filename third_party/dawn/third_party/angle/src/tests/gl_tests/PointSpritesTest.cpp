//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Some of the pointsprite tests below were ported from Khronos WebGL
// conformance test suite.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <cmath>

using namespace angle;

constexpr char kVertexShaderSource[] =
    R"(attribute vec4 vPosition;
        uniform float uPointSize;
        void main()
        {
            gl_PointSize = uPointSize;
            gl_Position  = vPosition;
        })";

constexpr GLfloat kMinMaxPointSize = 1.0f;

class PointSpritesTest : public ANGLETest<>
{
  protected:
    const int windowWidth  = 256;
    const int windowHeight = 256;
    PointSpritesTest()
    {
        setWindowWidth(windowWidth);
        setWindowHeight(windowHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    float s2p(float s) { return (s + 1.0f) * 0.5f * (GLfloat)windowWidth; }

    void testPointCoordAndPointSizeCompliance(GLProgram &program)
    {
        glUseProgram(program);

        GLfloat pointSizeRange[2] = {};
        glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);

        GLfloat maxPointSize = pointSizeRange[1];

        ASSERT_TRUE(maxPointSize >= 1);
        maxPointSize = floorf(maxPointSize);
        ASSERT_TRUE((int)maxPointSize % 1 == 0);

        maxPointSize       = std::min(maxPointSize, 64.0f);
        GLfloat pointWidth = maxPointSize / windowWidth;
        GLint step         = static_cast<GLint>(floorf(maxPointSize / 4));
        GLint pointStep    = std::max<GLint>(1, step);

        GLint pointSizeLoc = glGetUniformLocation(program, "uPointSize");
        ASSERT_GL_NO_ERROR();

        glUniform1f(pointSizeLoc, maxPointSize);
        ASSERT_GL_NO_ERROR();

        GLfloat pixelOffset = ((int)maxPointSize % 2) ? (1.0f / (GLfloat)windowWidth) : 0;
        GLBuffer vertexObject;

        glBindBuffer(GL_ARRAY_BUFFER, vertexObject);
        ASSERT_GL_NO_ERROR();

        GLfloat thePoints[] = {-0.5f + pixelOffset, -0.5f + pixelOffset, 0.5f + pixelOffset,
                               -0.5f + pixelOffset, -0.5f + pixelOffset, 0.5f + pixelOffset,
                               0.5f + pixelOffset,  0.5f + pixelOffset};

        glBufferData(GL_ARRAY_BUFFER, sizeof(thePoints), thePoints, GL_STATIC_DRAW);
        ASSERT_GL_NO_ERROR();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_POINTS, 0, 4);
        ASSERT_GL_NO_ERROR();

        for (float py = 0; py < 2; ++py)
        {
            for (float px = 0; px < 2; ++px)
            {
                float pointX = -0.5f + px + pixelOffset;
                float pointY = -0.5f + py + pixelOffset;
                for (int yy = 0; yy < maxPointSize; yy += pointStep)
                {
                    for (int xx = 0; xx < maxPointSize; xx += pointStep)
                    {
                        // formula for s and t from OpenGL ES 2.0 spec section 3.3
                        float xw         = s2p(pointX);
                        float yw         = s2p(pointY);
                        float u          = xx / maxPointSize * 2 - 1;
                        float v          = yy / maxPointSize * 2 - 1;
                        int xf           = static_cast<int>(floorf(s2p(pointX + u * pointWidth)));
                        int yf           = static_cast<int>(floorf(s2p(pointY + v * pointWidth)));
                        float s          = 0.5f + (xf + 0.5f - xw) / maxPointSize;
                        float t          = 0.5f + (yf + 0.5f - yw) / maxPointSize;
                        GLubyte color[4] = {static_cast<GLubyte>(floorf(s * 255)),
                                            static_cast<GLubyte>(floorf((1 - t) * 255)), 0, 255};
                        EXPECT_PIXEL_NEAR(xf, yf, color[0], color[1], color[2], color[3], 4);
                    }
                }
            }
        }
    }
};

// Checks gl_PointCoord and gl_PointSize
// https://www.khronos.org/registry/webgl/sdk/tests/conformance/glsl/variables/gl-pointcoord.html
TEST_P(PointSpritesTest, PointCoordAndPointSizeCompliance)
{
    constexpr char fs[] =
        R"(precision mediump float;
        void main()
        {
            gl_FragColor = vec4(gl_PointCoord.x, gl_PointCoord.y, 0, 1);
        })";

    ANGLE_GL_PROGRAM(program, kVertexShaderSource, fs);

    testPointCoordAndPointSizeCompliance(program);
}

// Checks gl_PointCoord and gl_PointSize, but use the gl_PointCoord inside a function.
// In Vulkan, we need to inject some code into the shader to flip the Y coordinate, and we
// need to make sure this code injection works even if someone uses gl_PointCoord outside the
// main function.
TEST_P(PointSpritesTest, UsingPointCoordInsideFunction)
{
    constexpr char fs[] =
        R"(precision mediump float;
        void foo() 
        {
            gl_FragColor = vec4(gl_PointCoord.x, gl_PointCoord.y, 0, 1);
        }

        void main()
        {
            foo();
        })";

    ANGLE_GL_PROGRAM(program, kVertexShaderSource, fs);

    testPointCoordAndPointSizeCompliance(program);
}

// Verify that drawing a point without enabling any attributes succeeds
// https://www.khronos.org/registry/webgl/sdk/tests/conformance/rendering/point-no-attributes.html
TEST_P(PointSpritesTest, PointWithoutAttributesCompliance)
{
    GLfloat pointSizeRange[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);
    GLfloat maxPointSize = pointSizeRange[1];
    ANGLE_SKIP_TEST_IF(maxPointSize < kMinMaxPointSize);

    constexpr char kVS[] = R"(void main()
{
    gl_PointSize = 2.0;
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Blue());
    ASSERT_GL_NO_ERROR();

    glUseProgram(program);

    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();

    // expect the center pixel to be blue
    EXPECT_PIXEL_COLOR_EQ((windowWidth - 1) / 2, (windowHeight - 1) / 2, GLColor::blue);
}

// This is a regression test for a graphics driver bug affecting end caps on roads in MapsGL
// https://www.khronos.org/registry/webgl/sdk/tests/conformance/rendering/point-with-gl-pointcoord-in-fragment-shader.html
TEST_P(PointSpritesTest, PointCoordRegressionTest)
{
    GLfloat pointSizeRange[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);
    GLfloat maxPointSize = pointSizeRange[1];
    ANGLE_SKIP_TEST_IF(maxPointSize < kMinMaxPointSize);

    constexpr char kFS[] = R"(precision mediump float;
varying vec4 v_color;
void main()
{
    // It seems as long as this mathematical expression references
    // gl_PointCoord, the fragment's color is incorrect.
    vec2 diff = gl_PointCoord - vec2(.5, .5);
    if (length(diff) > 0.5)
        discard;

    // The point should be a solid color.
    gl_FragColor = v_color;
})";

    constexpr char kVS[] = R"(varying vec4 v_color;
// The X and Y coordinates of the center of the point.
attribute vec2 a_vertex;
uniform float u_pointSize;
void main()
{
    gl_PointSize = u_pointSize;
    gl_Position  = vec4(a_vertex, 0.0, 1.0);
    // The color of the point.
    v_color = vec4(0.0, 1.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    ASSERT_GL_NO_ERROR();

    glUseProgram(program);

    glClearColor(0, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint pointSizeLoc = glGetUniformLocation(program, "u_pointSize");
    ASSERT_GL_NO_ERROR();

    GLfloat pointSize = std::min<GLfloat>(20.0f, maxPointSize);
    glUniform1f(pointSizeLoc, pointSize);
    ASSERT_GL_NO_ERROR();

    GLBuffer vertexObject;
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, vertexObject);
    ASSERT_GL_NO_ERROR();

    GLfloat thePoints[] = {0.0f, 0.0f};

    glBufferData(GL_ARRAY_BUFFER, sizeof(thePoints), thePoints, GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();

    // expect the center pixel to be green
    EXPECT_PIXEL_EQ((windowWidth - 1) / 2, (windowHeight - 1) / 2, 0, 255, 0, 255);
}

// Verify GL_VERTEX_PROGRAM_POINT_SIZE is enabled
// https://www.khronos.org/registry/webgl/sdk/tests/conformance/rendering/point-size.html
TEST_P(PointSpritesTest, PointSizeEnabledCompliance)
{
    constexpr char kFS[] = R"(precision mediump float;
varying vec4 color;

void main()
{
    gl_FragColor = color;
})";

    constexpr char kVS[] = R"(attribute vec3 pos;
attribute vec4 colorIn;
uniform float pointSize;
varying vec4 color;

void main()
{
    gl_PointSize = pointSize;
    color        = colorIn;
    gl_Position  = vec4(pos, 1.0);
})";

    // The WebGL test is drawn on a 2x2 canvas. Emulate this by setting a 2x2 viewport.
    glViewport(0, 0, 2, 2);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    ASSERT_GL_NO_ERROR();

    glUseProgram(program);

    glDisable(GL_BLEND);

    // The choice of (0.4, 0.4) ensures that the centers of the surrounding
    // pixels are not contained within the point when it is of size 1, but
    // that they definitely are when it is of size 2.
    GLfloat vertices[] = {0.4f, 0.4f, 0.0f};
    GLubyte colors[]   = {255, 0, 0, 255};

    GLBuffer vertexObject;
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, vertexObject);
    ASSERT_GL_NO_ERROR();

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), nullptr, GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    ASSERT_GL_NO_ERROR();

    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0,
                          reinterpret_cast<void *>(sizeof(vertices)));
    glEnableVertexAttribArray(1);

    GLint pointSizeLoc = glGetUniformLocation(program, "pointSize");
    ASSERT_GL_NO_ERROR();

    glUniform1f(pointSizeLoc, 1.0f);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(ArraySize(vertices)) / 3);
    ASSERT_GL_NO_ERROR();

    // Test the pixels around the target Red pixel to ensure
    // they are the expected color values
    for (GLint y = 0; y < 2; ++y)
    {
        for (GLint x = 0; x < 2; ++x)
        {
            // 1x1 is expected to be a red pixel
            // All others are black
            GLubyte expectedColor[4] = {0, 0, 0, 0};
            if (x == 1 && y == 1)
            {
                expectedColor[0] = 255;
                expectedColor[3] = 255;
            }
            EXPECT_PIXEL_EQ(x, y, expectedColor[0], expectedColor[1], expectedColor[2],
                            expectedColor[3]);
        }
    }

    GLfloat pointSizeRange[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);

    if (pointSizeRange[1] >= 2.0)
    {
        // Draw a point of size 2 and verify it fills the appropriate region.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1f(pointSizeLoc, 2.0f);
        ASSERT_GL_NO_ERROR();

        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(ArraySize(vertices)) / 3);
        ASSERT_GL_NO_ERROR();

        // Test the pixels to ensure the target is ALL Red pixels
        for (GLint y = 0; y < 2; ++y)
        {
            for (GLint x = 0; x < 2; ++x)
            {
                EXPECT_PIXEL_EQ(x, y, 255, 0, 0, 255);
            }
        }
    }
}

// Verify that rendering works correctly when gl_PointSize is declared in a shader but isn't used
TEST_P(PointSpritesTest, PointSizeDeclaredButUnused)
{
    constexpr char kVS[] = R"(attribute highp vec4 position;
void main(void)
{
    gl_PointSize = 1.0;
    gl_Position  = position;
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    ASSERT_GL_NO_ERROR();

    glUseProgram(program);
    drawQuad(program, "position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    // expect the center pixel to be red
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
}

// Test to cover a bug where the D3D11 rasterizer state would not be update when switching between
// draw types.  This causes the cull face to potentially be incorrect when drawing emulated point
// spites.
TEST_P(PointSpritesTest, PointSpriteAlternatingDrawTypes)
{
    // TODO(anglebug.com/42262976): Investigate possible ARM driver bug.
    ANGLE_SKIP_TEST_IF(IsFuchsia() && IsARM() && IsVulkan());

    GLfloat pointSizeRange[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);
    GLfloat maxPointSize = pointSizeRange[1];
    ANGLE_SKIP_TEST_IF(maxPointSize < kMinMaxPointSize);

    constexpr char kVS[] = R"(uniform float u_pointSize;
void main()
{
    gl_PointSize = u_pointSize;
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(pointProgram, kVS, essl1_shaders::fs::Blue());

    ANGLE_GL_PROGRAM(quadProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ASSERT_GL_NO_ERROR();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    const GLfloat quadVertices[] = {
        -1.0f, 1.0f, 0.5f, 1.0f, -1.0f, 0.5f, -1.0f, -1.0f, 0.5f,

        -1.0f, 1.0f, 0.5f, 1.0f, 1.0f,  0.5f, 1.0f,  -1.0f, 0.5f,
    };

    glUseProgram(quadProgram);
    GLint positionLocation = glGetAttribLocation(quadProgram, essl1_shaders::PositionAttrib());
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, quadVertices);
    glEnableVertexAttribArray(positionLocation);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(pointProgram);
    GLint pointSizeLoc = glGetUniformLocation(pointProgram, "u_pointSize");
    ASSERT_GL_NO_ERROR();
    GLfloat pointSize = std::min<GLfloat>(16.0f, maxPointSize);
    glUniform1f(pointSizeLoc, pointSize);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();

    // expect the center pixel to be blue
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);
}

// This checks for an NVIDIA driver bug where points larger than the maximum reported point size can
// be drawn. Point size should be clamped to the point size range as specified in GLES 3.0.5 section
// 3.4.
TEST_P(PointSpritesTest, PointSizeAboveMaxIsClamped)
{
    // Failed on NVIDIA GeForce GTX 1080 - no pixels from the point were detected in the
    // framebuffer. http://anglebug.com/42260857
    ANGLE_SKIP_TEST_IF(IsD3D9());

    // Failed on AMD OSX and Windows trybots - no pixels from the point were detected in the
    // framebuffer. http://anglebug.com/42260859
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    // TODO(anglebug.com/40096805)
    ANGLE_SKIP_TEST_IF(IsMetal() && IsAMD());

    GLfloat pointSizeRange[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);
    GLfloat maxPointSize = pointSizeRange[1];

    if (maxPointSize < 4)
    {
        // This test is only able to test larger points.
        return;
    }

    // Create a renderbuffer that has height and width equal to the max point size.
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, static_cast<GLsizei>(maxPointSize),
                          static_cast<GLsizei>(maxPointSize));
    // Switch the render target from the default window surface to the newly created renderbuffer so
    // that the test can handle implementations with a very large max point size.
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    glViewport(0, 0, maxPointSize, maxPointSize);
    ASSERT_GL_NO_ERROR();

    constexpr char kVS[] =
        "attribute vec2 vPosition;\n"
        "uniform float uPointSize;\n"
        "void main()\n"
        "{\n"
        "    gl_PointSize = uPointSize;\n"
        "    gl_Position  = vec4(vPosition, 0, 1);\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    GLfloat testPointSize = floorf(maxPointSize * 2.0f);

    GLint pointSizeLoc = glGetUniformLocation(program, "uPointSize");
    glUniform1f(pointSizeLoc, testPointSize);
    ASSERT_GL_NO_ERROR();

    // The point will be a square centered at gl_Position. As the framebuffer is the same size as
    // the square, setting the center of the point on the right edge of the viewport will result in
    // the left edge of the point square to be at the center of the viewport.
    GLfloat pointXPosition = 1;

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    GLfloat thePoints[] = {pointXPosition, 0.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(thePoints), thePoints, GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Clear the framebuffer to green
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the red point
    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();

    // Pixels to the right of the framebuffer center should be covered by the point.
    EXPECT_PIXEL_NEAR(maxPointSize / 2 + 2, maxPointSize / 2, 255, 0, 0, 255, 1);

    // Pixels to the left of the framebuffer center should not be covered by the point.
    EXPECT_PIXEL_NEAR(maxPointSize / 2 - 2, maxPointSize / 2, 0, 255, 0, 255, 1);
}

// Use this to select which configurations (e.g. which renderer, which GLES
// major version) these tests should be run against.
//
// We test on D3D11 9_3 because the existing D3D11 PointSprite implementation
// uses Geometry Shaders which are not supported for 9_3.
// D3D9 and D3D11 are also tested to ensure no regressions.
ANGLE_INSTANTIATE_TEST_ES2_AND(PointSpritesTest,
                               ES2_VULKAN().enable(Feature::EmulatedPrerotation90),
                               ES2_VULKAN().enable(Feature::EmulatedPrerotation180),
                               ES2_VULKAN().enable(Feature::EmulatedPrerotation270));
