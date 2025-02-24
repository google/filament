//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DrawTextureTest.cpp: Tests basic usage of glDrawTex*.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <memory>
#include <vector>

using namespace angle;

class DrawTextureTest : public ANGLETest<>
{
  protected:
    DrawTextureTest()
    {
        setWindowWidth(kWindowWidth);
        setWindowHeight(kWindowHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        mTexture.reset(new GLTexture());
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mTexture->get());
    }

    void testTearDown() override { mTexture.reset(); }

    std::unique_ptr<GLTexture> mTexture;

    static constexpr int kWindowWidth  = 32;
    static constexpr int kWindowHeight = 32;
};

// Negative test for invalid width/height values.
TEST_P(DrawTextureTest, NegativeValue)
{
    glDrawTexiOES(0, 0, 0, 0, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glDrawTexiOES(0, 0, 0, -1, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glDrawTexiOES(0, 0, 0, 0, -1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glDrawTexiOES(0, 0, 0, -1, -1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Basic draw.
TEST_P(DrawTextureTest, Basic)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    glDrawTexiOES(0, 0, 0, 1, 1);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that odd viewport dimensions are handled correctly.
// If the viewport dimension is even, then the incorrect way
// of getting the center screen coordinate by dividing by 2 and
// converting to integer will work in that case, but not if
// the viewport dimension is odd.
TEST_P(DrawTextureTest, CorrectNdcForOddViewportDimensions)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // clang-format off
    std::array<GLColor, 2> textureData = {
        GLColor::green, GLColor::green
    };
    // clang-format on

    glViewport(0, 0, 3, 3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());

    GLint cropRect[] = {0, 0, 2, 1};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    GLint x = 1;
    GLint y = 1;
    glDrawTexiOES(x, y, 0, 2, 1);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(x, y, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(x + 1, y, GLColor::green);

    EXPECT_PIXEL_COLOR_EQ(x, y + 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(x + 1, y + 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(x + 2, y, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(x + 3, y, GLColor::black);
}

// Tests that vertex attributes enabled with fewer than 6 verts do not cause a crash.
TEST_P(DrawTextureTest, VertexAttributesNoCrash)
{
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, 0, &GLColor::white);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    EXPECT_GL_NO_ERROR();

    glDrawTexiOES(0, 0, 0, 1, 1);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that the color array, if enabled, is not used as the vertex color.
TEST_P(DrawTextureTest, ColorArrayNotUsed)
{
    glEnableClientState(GL_COLOR_ARRAY);

    // This color is set to black on purpose to ensure that the color in the upcoming vertex array
    // is not used in the texture draw. If it is used, then the texture we want to read will be
    // modulated with the color in the vertex array instead of GL_CURRENT_COLOR (which at the moment
    // is white (1.0, 1.0, 1.0, 1.0).
    glColorPointer(4, GL_FLOAT, 0, &GLColor::black);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    EXPECT_GL_NO_ERROR();

    glDrawTexiOES(0, 0, 0, 1, 1);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that values of differenty types are properly normalized with glColorPointer
TEST_P(DrawTextureTest, ColorArrayDifferentTypes)
{
    constexpr GLubyte kTextureColorData[] = {0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
                                             0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF};
    constexpr GLfloat kVertexPtrData[]    = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
    constexpr GLfloat kTexCoordPtrData[]  = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    constexpr GLubyte kGLubyteData[]      = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    constexpr GLfloat kGLfloatData[]      = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                             1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    constexpr GLfixed kGLfixedData[]      = {0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000,
                                             0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000,
                                             0x10000, 0x10000, 0x10000, 0x10000};

    // We check a pixel coordinate at the border of where linear interpolation starts as
    // we fail to get correct interpolated values when we do not normalize the GLbyte values.
    constexpr GLint kCheckedPixelX         = 16;
    constexpr GLint kCheckedPixelY         = 8;
    constexpr unsigned int kPixelTolerance = 10u;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kTextureColorData);
    glVertexPointer(2, GL_FLOAT, 0, kVertexPtrData);
    glTexCoordPointer(2, GL_FLOAT, 0, kTexCoordPtrData);

    // Ensure the results do not change unexpectedly regardless of the color data format

    // Test GLubyte
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, kGLubyteData);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(kCheckedPixelX, kCheckedPixelY, GLColor::red, kPixelTolerance);

    // Test GLfloat
    glColorPointer(4, GL_FLOAT, 0, kGLfloatData);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(kCheckedPixelX, kCheckedPixelY, GLColor::red, kPixelTolerance);

    // Test GLfixed
    glColorPointer(4, GL_FIXED, 0, kGLfixedData);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(kCheckedPixelX, kCheckedPixelY, GLColor::red, kPixelTolerance);
}

// Tests that drawing a primitive works with enabled tex coord pointer, but with texture disabled.
TEST_P(DrawTextureTest, DrawWithTexCoordPtrDataAndDisabledTexture2D)
{
    std::vector<GLfloat> vertexPtrData   = {-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f};
    std::vector<GLfloat> texCoordPtrData = {0.0f};

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw a triangle fan that covers the entire window. GL_TEXTURE_2D is disabled even though
    // texture coord pointer is set.
    glVertexPointer(2, GL_FLOAT, 0, vertexPtrData.data());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordPtrData.data());
    glDisable(GL_TEXTURE_2D);
    glColor4ub(0, 0xFF, 0xFF, 0xFF);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kWindowWidth, kWindowHeight, GLColor::cyan);
}

// Tests that drawing a primitive works with enabled tex coord pointer and texture environment set
// so the used texture replaces the current color.
TEST_P(DrawTextureTest, DrawWithTexCoordPtrDataAndEnvModeReplace)
{
    std::vector<GLfloat> vertexPtrData   = {-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f};
    std::vector<GLfloat> texCoordPtrData = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f};

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up the texture. By default, the texture is multiplied by the current color set through
    // glColor calls. Here the environment is set so it would replace the color instead.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Draw a triangle fan that covers the entire window. The texture should replace the color
    // regardless of the value of the current color.
    glVertexPointer(2, GL_FLOAT, 0, vertexPtrData.data());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordPtrData.data());
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kWindowWidth, kWindowHeight, GLColor::red);
}

// Tests that tex coord pointer is only used when texture is enabled, and that it is possible to
// disable the texture and draw another primitive by setting a color without using the texture data.
TEST_P(DrawTextureTest, DrawWithTexCoordPtrThenDisableTexture2DAndDrawAnother)
{
    // There will be two vertex arrays for triangle fan draws. Both cover the entire screen, but the
    // one with no texture uses more vertices.
    std::vector<GLfloat> vertexPtrDataWithTexture = {-1.0f, -1.0f, -1.0f, 1.0f,
                                                     1.0f,  1.0f,  1.0f,  -1.0f};
    std::vector<GLfloat> vertexPtrDataNoTexture   = {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
                                                     1.0f,  1.0f,  1.0f,  0.0f, 1.0f, -1.0f};
    std::vector<GLfloat> texCoordPtrData = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f};

    // Enable vertex and texture coordinate pointers. Also set up texture data with four colors; one
    // color per corner.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    constexpr GLColor kColorCornerBL = GLColor(0x11, 0x22, 0x33, 0xFF);
    constexpr GLColor kColorCornerBR = GLColor(0x44, 0x55, 0x66, 0xFF);
    constexpr GLColor kColorCornerTL = GLColor(0x77, 0x88, 0x99, 0xFF);
    constexpr GLColor kColorCornerTR = GLColor(0xAA, 0xBB, 0xCC, 0xFF);

    std::vector<GLColor> textureData;
    textureData.push_back(kColorCornerBL);
    textureData.push_back(kColorCornerBR);
    textureData.push_back(kColorCornerTL);
    textureData.push_back(kColorCornerTR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());

    // Draw the first triangle fan using texture coord pointer.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    glVertexPointer(2, GL_FLOAT, 0, vertexPtrDataWithTexture.data());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordPtrData.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kWindowWidth / 2, kWindowHeight / 2, kColorCornerBL);
    EXPECT_PIXEL_RECT_EQ(kWindowWidth / 2, 0, kWindowWidth / 2, kWindowHeight / 2, kColorCornerBR);
    EXPECT_PIXEL_RECT_EQ(0, kWindowHeight / 2, kWindowWidth / 2, kWindowHeight / 2, kColorCornerTL);
    EXPECT_PIXEL_RECT_EQ(kWindowWidth / 2, kWindowHeight / 2, kWindowWidth / 2, kWindowHeight / 2,
                         kColorCornerTR);

    // Disable texture and draw the second triangle fan. This time, the draw uses more vertex
    // coordinates and a preset color. Note that the texture coord pointer must no longer be used.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);

    glVertexPointer(2, GL_FLOAT, 0, vertexPtrDataNoTexture.data());
    glColor4ub(0, 0, 0xFF, 0xFF);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kWindowWidth, kWindowHeight, GLColor::blue);

    // Re-enable texture and draw using the same vertex pointer. This is to make sure that enabling
    // GL_TEXTURE_2D is enough to use the texture data.
    // There is no need to set the texture coord pointer again. However, since GL_TEXTURE_ENV_MODE
    // is set to the default GL_MODULATE, the effect of glColor4ub() from before should be reversed
    // by resetting the color to the default (1, 1, 1, 1).
    // In addition, since the tex coord pointer is not defined for the current vertex pointer, the
    // texture colors will shift position to be mapped to the new primitive, which will only cover
    // the top-left half of the window.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, kWindowHeight / 4, kColorCornerBL);
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth * 3 / 4, kWindowHeight - 1, kColorCornerBR);
    EXPECT_PIXEL_COLOR_EQ(0, kWindowHeight * 3 / 4, kColorCornerTL);
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth / 2, kWindowHeight - 1, kColorCornerTR);

    // Update the vertex pointer to the original and draw a final triangle fan.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glVertexPointer(2, GL_FLOAT, 0, vertexPtrDataWithTexture.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kWindowWidth / 2, kWindowHeight / 2, kColorCornerBL);
    EXPECT_PIXEL_RECT_EQ(kWindowWidth / 2, 0, kWindowWidth / 2, kWindowHeight / 2, kColorCornerBR);
    EXPECT_PIXEL_RECT_EQ(0, kWindowHeight / 2, kWindowWidth / 2, kWindowHeight / 2, kColorCornerTL);
    EXPECT_PIXEL_RECT_EQ(kWindowWidth / 2, kWindowHeight / 2, kWindowWidth / 2, kWindowHeight / 2,
                         kColorCornerTR);
}

ANGLE_INSTANTIATE_TEST_ES1(DrawTextureTest);
