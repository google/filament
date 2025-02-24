//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BGRATextureTest.cpp: Tests GLES1-specific usage of BGRA textures

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class BGRATextureTest : public ANGLETest<>
{
  protected:
    BGRATextureTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    GLColor rgbswap(GLColor x)
    {
        GLColor y;
        y.B = x.R;
        y.G = x.G;
        y.R = x.B;
        y.A = x.A;
        return y;
    }

    void clearGLColor(GLColor color)
    {
        Vector4 colorF = color.toNormalizedVector();
        glClearColor(colorF.x(), colorF.y(), colorF.z(), colorF.w());
    }
};

// Test that BGRA sampling samples color components and alpha correctly.
TEST_P(BGRATextureTest, BGRATextureSampling)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"));

    struct Vertex
    {
        GLfloat position[3];
        GLfloat uv[2];
    };

    glEnable(GL_TEXTURE_2D);

    std::array<GLColor, 4> testImage = {
        GLColor::cyan,
        GLColor::yellow,
        GLColor::magenta,
        GLColor(1, 2, 3, 0),
    };

    GLTexture texRGBA;
    {
        glBindTexture(GL_TEXTURE_2D, texRGBA);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     testImage.data());
        EXPECT_GL_NO_ERROR();
    }

    GLTexture texBGRA;
    {
        std::array<GLColor, 4> testImage2 = testImage;
        for (auto &color : testImage2)
        {
            color = rgbswap(color);
        }

        glBindTexture(GL_TEXTURE_2D, texBGRA);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 2, 2, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE,
                     testImage2.data());
        EXPECT_GL_NO_ERROR();
    }

    GLColor clearColor = GLColor(102, 102, 102, 255);

    clearGLColor(clearColor);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 0.9f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumf(-1, 1, -1, 1, 5.0, 60.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -8.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    };
    glVertexPointer(3, GL_FLOAT, sizeof vertices[0], &vertices[0].position);
    glTexCoordPointer(2, GL_FLOAT, sizeof vertices[0], &vertices[0].uv);

    std::array<GLuint, 8> draws = {
        texRGBA.get(), texBGRA.get(), texRGBA.get(), texRGBA.get(),
        texBGRA.get(), texBGRA.get(), texRGBA.get(), texBGRA.get(),
    };

    for (auto tex : draws)
    {
        glBindTexture(GL_TEXTURE_2D, tex);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());
        EXPECT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_NEAR(8, 8, testImage[0], 0);
        EXPECT_PIXEL_COLOR_NEAR(24, 8, testImage[1], 0);
        EXPECT_PIXEL_COLOR_NEAR(8, 24, testImage[2], 0);
        EXPECT_PIXEL_COLOR_NEAR(24, 24, clearColor, 0);
    }
}

ANGLE_INSTANTIATE_TEST_ES1(BGRATextureTest);
