//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <cmath>

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class MaxTextureSizeTest : public ANGLETest<>
{
  protected:
    MaxTextureSizeTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(precision highp float;
attribute vec4 position;
varying vec2 texcoord;

void main()
{
    gl_Position = position;
    texcoord = (position.xy * 0.5) + 0.5;
})";

        constexpr char kTextureFS[] = R"(precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";

        constexpr char kBlueFS[] = R"(precision highp float;
void main()
{
    gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
})";

        mTextureProgram = CompileProgram(kVS, kTextureFS);
        mBlueProgram    = CompileProgram(kVS, kBlueFS);
        if (mTextureProgram == 0 || mBlueProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTexture2DSize);
        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &mMaxTextureCubeSize);
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &mMaxRenderbufferSize);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteProgram(mTextureProgram);
        glDeleteProgram(mBlueProgram);
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;

    GLuint mBlueProgram;

    GLint mMaxTexture2DSize;
    GLint mMaxTextureCubeSize;
    GLint mMaxRenderbufferSize;
};

TEST_P(MaxTextureSizeTest, SpecificationTexImage)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLsizei textureWidth  = mMaxTexture2DSize;
    GLsizei textureHeight = 64;

    std::vector<GLubyte> data(textureWidth * textureHeight * 4);
    for (int y = 0; y < textureHeight; y++)
    {
        for (int x = 0; x < textureWidth; x++)
        {
            GLubyte *pixel = &data[0] + ((y * textureWidth + x) * 4);

            // Draw a gradient, red in direction, green in y direction
            pixel[0] = static_cast<GLubyte>((float(x) / textureWidth) * 255);
            pixel[1] = static_cast<GLubyte>((float(y) / textureHeight) * 255);
            pixel[2] = 0;
            pixel[3] = 255;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, &data[0]);
    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    std::vector<GLubyte> pixels(getWindowWidth() * getWindowHeight() * 4);
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

    for (int y = 1; y < getWindowHeight(); y++)
    {
        for (int x = 1; x < getWindowWidth(); x++)
        {
            const GLubyte *prevPixel = &pixels[0] + (((y - 1) * getWindowWidth() + (x - 1)) * 4);
            const GLubyte *curPixel  = &pixels[0] + ((y * getWindowWidth() + x) * 4);

            EXPECT_GE(curPixel[0], prevPixel[0]);
            EXPECT_GE(curPixel[1], prevPixel[1]);
            EXPECT_EQ(curPixel[2], prevPixel[2]);
            EXPECT_EQ(curPixel[3], prevPixel[3]);
        }
    }
}

TEST_P(MaxTextureSizeTest, SpecificationTexStorage)
{
    if (getClientMajorVersion() < 3 && (!IsGLExtensionEnabled("GL_EXT_texture_storage") ||
                                        !IsGLExtensionEnabled("GL_OES_rgb8_rgba8")))
    {
        return;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLsizei textureWidth  = 64;
    GLsizei textureHeight = mMaxTexture2DSize;

    std::vector<GLubyte> data(textureWidth * textureHeight * 4);
    for (int y = 0; y < textureHeight; y++)
    {
        for (int x = 0; x < textureWidth; x++)
        {
            GLubyte *pixel = &data[0] + ((y * textureWidth + x) * 4);

            // Draw a gradient, red in direction, green in y direction
            pixel[0] = static_cast<GLubyte>((float(x) / textureWidth) * 255);
            pixel[1] = static_cast<GLubyte>((float(y) / textureHeight) * 255);
            pixel[2] = 0;
            pixel[3] = 255;
        }
    }

    if (getClientMajorVersion() < 3)
    {
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8_OES, textureWidth, textureHeight);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8_OES, textureWidth, textureHeight);
    }
    EXPECT_GL_NO_ERROR();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                    &data[0]);
    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    std::vector<GLubyte> pixels(getWindowWidth() * getWindowHeight() * 4);
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

    for (int y = 1; y < getWindowHeight(); y++)
    {
        for (int x = 1; x < getWindowWidth(); x++)
        {
            const GLubyte *prevPixel = &pixels[0] + (((y - 1) * getWindowWidth() + (x - 1)) * 4);
            const GLubyte *curPixel  = &pixels[0] + ((y * getWindowWidth() + x) * 4);

            EXPECT_GE(curPixel[0], prevPixel[0]);
            EXPECT_GE(curPixel[1], prevPixel[1]);
            EXPECT_EQ(curPixel[2], prevPixel[2]);
            EXPECT_EQ(curPixel[3], prevPixel[3]);
        }
    }
}

TEST_P(MaxTextureSizeTest, RenderToTexture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       (!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit")));

    GLuint fbo       = 0;
    GLuint textureId = 0;
    // create a 1-level texture at maximum size
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    GLsizei textureWidth  = 64;
    GLsizei textureHeight = mMaxTexture2DSize;

    // texture setup code
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, textureWidth, textureHeight, 0, GL_BGRA_EXT,
                 GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    // create an FBO and attach the texture
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    const int frameCount = 64;
    for (int i = 0; i < frameCount; i++)
    {
        // clear the screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        GLubyte clearRed   = static_cast<GLubyte>((float(i) / frameCount) * 255);
        GLubyte clearGreen = 255 - clearRed;
        GLubyte clearBlue  = 0;

        glClearColor(clearRed / 255.0f, clearGreen / 255.0f, clearBlue / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // render blue into the texture
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        drawQuad(mBlueProgram, "position", 0.5f);

        // copy corner of texture to LL corner of window
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, fbo);
        glBlitFramebufferANGLE(0, 0, textureWidth - 1, getWindowHeight() - 1, 0, 0,
                               textureWidth - 1, getWindowHeight() - 1, GL_COLOR_BUFFER_BIT,
                               GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, 0);
        EXPECT_GL_NO_ERROR();

        EXPECT_PIXEL_EQ(textureWidth / 2, getWindowHeight() / 2, 0, 0, 255, 255);
        EXPECT_PIXEL_EQ(textureWidth + 10, getWindowHeight() / 2, clearRed, clearGreen, clearBlue,
                        255);

        swapBuffers();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureId);
}

TEST_P(MaxTextureSizeTest, Render1xTexture)
{
    // This is not spec compliant but checking for now anyway. We can fix it
    // if we find a driver for which this is not true.
    float power = std::roundf(std::log(static_cast<float>(mMaxTexture2DSize)) / std::log(2.0f));
    EXPECT_EQ(std::pow(2.0f, power), mMaxTexture2DSize);

    // Make a the largest possbile texture but not too big.
    const GLint testSize = std::min(mMaxTexture2DSize, 128 * 1024);

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    constexpr GLColor testColor(0, 255, 128, 255);
    std::vector<GLColor> data(testSize, testColor);

    // Test a wide texture.
    {
        GLTexture texture1Id;
        glBindTexture(GL_TEXTURE_2D, texture1Id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mMaxTexture2DSize, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     data.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(mTextureProgram, "position", 0.5f);

        EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), testColor);
    }

    // Test a tall texture.
    {
        GLTexture texture2Id;
        glBindTexture(GL_TEXTURE_2D, texture2Id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, mMaxTexture2DSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     data.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(mTextureProgram, "position", 0.5f);
    }

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), testColor);

    EXPECT_GL_NO_ERROR();
}

// TODO(geofflang): Fix the dependence on glBlitFramebufferANGLE without checks and assuming the
// default framebuffer is BGRA to enable the GL and GLES backends. (http://anglebug.com/42260299)

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(MaxTextureSizeTest, ES2_D3D9(), ES2_D3D11(), ES2_VULKAN(), ES2_METAL());

// This test suite is not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MaxTextureSizeTest);
