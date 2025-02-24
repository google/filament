//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class FramebufferRenderMipmapTest : public ANGLETest<>
{
  protected:
    FramebufferRenderMipmapTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mColorLocation = glGetUniformLocation(mProgram, essl1_shaders::ColorUniform());

        glUseProgram(mProgram);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram;
    GLint mColorLocation;
};

// Validate that if we are in ES3 or GL_OES_fbo_render_mipmap exists, there are no validation errors
// when using a non-zero level in glFramebufferTexture2D.
TEST_P(FramebufferRenderMipmapTest, Validation)
{
    bool renderToMipmapSupported =
        IsGLExtensionEnabled("GL_OES_fbo_render_mipmap") || getClientMajorVersion() > 2;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    const GLint levels = 5;
    for (GLint i = 0; i < levels; i++)
    {
        GLsizei size = 1 << ((levels - 1) - i);
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    EXPECT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    for (GLint i = 0; i < levels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, i);

        if (i > 0 && !renderToMipmapSupported)
        {
            EXPECT_GL_ERROR(GL_INVALID_VALUE);
        }
        else
        {
            EXPECT_GL_NO_ERROR();
            EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GLenum(GL_FRAMEBUFFER_COMPLETE));
        }
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
}

// Render to various levels of a texture and check that they have the correct color data via
// ReadPixels
TEST_P(FramebufferRenderMipmapTest, RenderToMipmap)
{
    bool renderToMipmapSupported =
        IsGLExtensionEnabled("GL_OES_fbo_render_mipmap") || getClientMajorVersion() > 2;
    ANGLE_SKIP_TEST_IF(!renderToMipmapSupported);

    const GLfloat levelColors[] = {
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    };
    const GLint testLevels = static_cast<GLint>(ArraySize(levelColors) / 4);

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    for (GLint i = 0; i < testLevels; i++)
    {
        GLsizei size = 1 << ((testLevels - 1) - i);
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    EXPECT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    // Render to the levels of the texture with different colors
    for (GLint i = 0; i < testLevels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, i);
        EXPECT_GL_NO_ERROR();

        glUseProgram(mProgram);
        glUniform4fv(mColorLocation, 1, levelColors + (i * 4));

        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();
    }

    // Test that the levels of the texture are correct
    for (GLint i = 0; i < testLevels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, i);
        EXPECT_GL_NO_ERROR();

        const GLfloat *color = levelColors + (i * 4);
        EXPECT_PIXEL_EQ(0, 0, color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255);
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(FramebufferRenderMipmapTest);
