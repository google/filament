//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

using namespace angle;

class PbufferTest : public ANGLETest<>
{
  protected:
    PbufferTest()
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
        constexpr char kVS[] =
            R"(precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
                texcoord.y = 1.0 - texcoord.y;
            })";

        constexpr char kFS[] =
            R"(precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            })";

        mTextureProgram = CompileProgram(kVS, kFS);
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        EGLWindow *window = getEGLWindow();

        EGLint surfaceType = 0;
        eglGetConfigAttrib(window->getDisplay(), window->getConfig(), EGL_SURFACE_TYPE,
                           &surfaceType);
        mSupportsPbuffers = (surfaceType & EGL_PBUFFER_BIT) != 0;

        mPbuffer = createTestPbufferSurface();
        if (mSupportsPbuffers)
        {
            ASSERT_NE(mPbuffer, EGL_NO_SURFACE);
            ASSERT_EGL_SUCCESS();
        }
        else
        {
            ASSERT_EQ(mPbuffer, EGL_NO_SURFACE);
            ASSERT_EGL_ERROR(EGL_BAD_MATCH);
        }
        ASSERT_GL_NO_ERROR();
    }

    EGLSurface createTestPbufferSurface()
    {
        EGLWindow *window        = getEGLWindow();
        EGLint bindToTextureRGBA = 0;
        eglGetConfigAttrib(window->getDisplay(), window->getConfig(), EGL_BIND_TO_TEXTURE_RGBA,
                           &bindToTextureRGBA);
        mSupportsBindTexImage = (bindToTextureRGBA == EGL_TRUE);

        const EGLint pBufferAttributes[] = {
            EGL_WIDTH,          static_cast<EGLint>(mPbufferSize),
            EGL_HEIGHT,         static_cast<EGLint>(mPbufferSize),
            EGL_TEXTURE_FORMAT, mSupportsBindTexImage ? EGL_TEXTURE_RGBA : EGL_NO_TEXTURE,
            EGL_TEXTURE_TARGET, mSupportsBindTexImage ? EGL_TEXTURE_2D : EGL_NO_TEXTURE,
            EGL_NONE,           EGL_NONE,
        };

        return eglCreatePbufferSurface(window->getDisplay(), window->getConfig(),
                                       pBufferAttributes);
    }

    void testTearDown() override
    {
        glDeleteProgram(mTextureProgram);

        destroyPbuffer();
    }

    void destroyPbuffer()
    {
        if (mPbuffer)
        {
            destroyTestPbufferSurface(mPbuffer);
        }
    }

    void destroyTestPbufferSurface(EGLSurface pbuffer)
    {
        EGLWindow *window = getEGLWindow();
        eglDestroySurface(window->getDisplay(), pbuffer);
    }

    void recreatePbufferInSrgbColorspace()
    {
        EGLWindow *window = getEGLWindow();

        destroyPbuffer();

        const EGLint pBufferSrgbAttributes[] = {
            EGL_WIDTH,
            static_cast<EGLint>(mPbufferSize),
            EGL_HEIGHT,
            static_cast<EGLint>(mPbufferSize),
            EGL_TEXTURE_FORMAT,
            mSupportsBindTexImage ? EGL_TEXTURE_RGBA : EGL_NO_TEXTURE,
            EGL_TEXTURE_TARGET,
            mSupportsBindTexImage ? EGL_TEXTURE_2D : EGL_NO_TEXTURE,
            EGL_GL_COLORSPACE_KHR,
            EGL_GL_COLORSPACE_SRGB_KHR,
            EGL_NONE,
            EGL_NONE,
        };

        mPbuffer = eglCreatePbufferSurface(window->getDisplay(), window->getConfig(),
                                           pBufferSrgbAttributes);
    }

    void drawColorQuad(GLColor color)
    {
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);
        GLint colorUniformLocation =
            glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
        ASSERT_NE(colorUniformLocation, -1);
        glUniform4fv(colorUniformLocation, 1, color.toNormalizedVector().data());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0);
        glUseProgram(0);
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;

    const size_t mPbufferSize = 32;
    EGLSurface mPbuffer       = EGL_NO_SURFACE;
    bool mSupportsPbuffers;
    bool mSupportsBindTexImage;
};

class PbufferColorspaceTest : public PbufferTest
{};

// Test clearing a Pbuffer and checking the color is correct
TEST_P(PbufferTest, Clearing)
{
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers);

    EGLWindow *window = getEGLWindow();

    // Clear the window surface to blue and verify
    window->makeCurrent();
    ASSERT_EGL_SUCCESS();

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);

    // Apply the Pbuffer and clear it to purple and verify
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(static_cast<GLint>(mPbufferSize) / 2, static_cast<GLint>(mPbufferSize) / 2, 255,
                    0, 255, 255);

    // Rebind the window surface and verify that it is still blue
    window->makeCurrent();
    ASSERT_EGL_SUCCESS();
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);
}

// Bind the Pbuffer to a texture and verify it renders correctly
TEST_P(PbufferTest, BindTexImage)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);

    EGLWindow *window = getEGLWindow();

    // Apply the Pbuffer and clear it to purple
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(mPbufferSize) / 2,
                          static_cast<GLint>(mPbufferSize) / 2, GLColor::magenta);

    // Apply the window surface
    window->makeCurrent();

    // Create a texture and bind the Pbuffer to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Draw a quad and verify that it is purple
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Unbind the texture
    eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    // Verify that purple was drawn
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 255, 255);

    glDeleteTextures(1, &texture);
}

// Test various EGL level cases for eglBindTexImage.
TEST_P(PbufferTest, BindTexImageAlreadyBound)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);
    EGLWindow *window = getEGLWindow();
    window->makeCurrent();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();

    // This is being tested
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();
    // If buffer is already bound to a texture then an EGL_BAD_ACCESS error is returned.
    EXPECT_FALSE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_ERROR(EGL_BAD_ACCESS);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ANGLE_SKIP_TEST_IF(status == GL_FRAMEBUFFER_UNSUPPORTED);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    drawColorQuad(GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(mPbufferSize) / 2,
                          static_cast<GLint>(mPbufferSize) / 2, GLColor::magenta);

    destroyPbuffer();
    ASSERT_EGL_SUCCESS();
    ASSERT_GL_NO_ERROR();
}

// Test that eglBindTexImage overwriting previous bind works.
TEST_P(PbufferTest, BindTexImageOverwrite)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);

    EGLWindow *window = getEGLWindow();
    window->makeCurrent();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();

    // This is being tested: setup a binding that will be overwritten.
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ANGLE_SKIP_TEST_IF(status == GL_FRAMEBUFFER_UNSUPPORTED);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    drawColorQuad(GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(mPbufferSize) / 2,
                          static_cast<GLint>(mPbufferSize) / 2, GLColor::magenta);

    EGLSurface otherPbuffer = createTestPbufferSurface();
    ASSERT_NE(otherPbuffer, EGL_NO_SURFACE);
    // This is being tested: replace the previous binding.
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), otherPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();

    drawColorQuad(GLColor::yellow);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(mPbufferSize) / 2,
                          static_cast<GLint>(mPbufferSize) / 2, GLColor::yellow);

    destroyTestPbufferSurface(otherPbuffer);
    destroyPbuffer();
    ASSERT_EGL_SUCCESS();
    ASSERT_GL_NO_ERROR();
}

// Test that eglBindTexImage overwriting previous bind works and does not crash on releaseTexImage.
TEST_P(PbufferTest, BindTexImageOverwriteNoCrashOnReleaseTexImage)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);
    EGLWindow *window = getEGLWindow();
    window->makeCurrent();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    EGLSurface otherPbuffer = createTestPbufferSurface();
    ASSERT_NE(otherPbuffer, EGL_NO_SURFACE);

    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), otherPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();
    EXPECT_TRUE(eglReleaseTexImage(window->getDisplay(), otherPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();
    EXPECT_TRUE(eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));  // No-op.
    ASSERT_EGL_SUCCESS();
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();
    EXPECT_TRUE(eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();

    destroyTestPbufferSurface(otherPbuffer);
    destroyPbuffer();
    ASSERT_EGL_SUCCESS();
    ASSERT_GL_NO_ERROR();
}

// Test that eglBindTexImage pbuffer is unbound when the texture is destroyed.
TEST_P(PbufferTest, BindTexImageReleaseViaTextureDestroy)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);
    EGLWindow *window = getEGLWindow();
    window->makeCurrent();

    // Bind to a texture that will be destroyed.
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        EXPECT_GL_NO_ERROR();

        // This is being tested: setup a binding that will be overwritten.
        EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
        ASSERT_EGL_SUCCESS();

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        ANGLE_SKIP_TEST_IF(status == GL_FRAMEBUFFER_UNSUPPORTED);
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

        drawColorQuad(GLColor::magenta);
        ASSERT_GL_NO_ERROR();
    }

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();

    // This is being tested.
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();

    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(mPbufferSize) / 2,
                          static_cast<GLint>(mPbufferSize) / 2, GLColor::magenta);

    destroyPbuffer();
    ASSERT_EGL_SUCCESS();
    ASSERT_GL_NO_ERROR();
}

// Test that eglBindTexImage pbuffer is unbound when eglReleaseTexImage is called.
TEST_P(PbufferTest, BindTexImagePbufferReleaseWhileBoundToFBOColorBuffer)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);
    EGLWindow *window = getEGLWindow();
    window->makeCurrent();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();

    // This is being tested: setup a binding to a pbuffer that will be unbound.
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ANGLE_SKIP_TEST_IF(status == GL_FRAMEBUFFER_UNSUPPORTED);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    // This is being tested: unbind the pbuffer, detect it via framebuffer status.
    EXPECT_TRUE(eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, status);

    destroyPbuffer();
    ASSERT_EGL_SUCCESS();
    ASSERT_GL_NO_ERROR();
}

// Test that eglBindTexImage pbuffer is bound when the pbuffer is destroyed.
TEST_P(PbufferTest, BindTexImagePbufferDestroyWhileBound)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);
    EGLWindow *window = getEGLWindow();
    window->makeCurrent();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();

    // This is being tested: setup a binding to a pbuffer that will be destroyed.
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ANGLE_SKIP_TEST_IF(status == GL_FRAMEBUFFER_UNSUPPORTED);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    drawColorQuad(GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // This is being tested: destroy the pbuffer, but the underlying binding still works.
    destroyPbuffer();
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(mPbufferSize) / 2,
                          static_cast<GLint>(mPbufferSize) / 2, GLColor::magenta);

    ASSERT_EGL_SUCCESS();
    ASSERT_GL_NO_ERROR();
}

// Test that eglBindTexImage overwrite releases the previous pbuffer if the previous is orphaned.
TEST_P(PbufferTest, BindTexImageOverwriteReleasesOrphanedPbuffer)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);

    EGLWindow *window = getEGLWindow();
    window->makeCurrent();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();

    // This is being tested: setup a binding to a pbuffer that will be destroyed.
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS();

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ANGLE_SKIP_TEST_IF(status == GL_FRAMEBUFFER_UNSUPPORTED);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    // Write magenta. This shouldn't be read below.
    drawColorQuad(GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // This is being tested: destroy the pbuffer, but the underlying binding still works.
    destroyPbuffer();
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    EGLSurface otherPbuffer = createTestPbufferSurface();
    // This is being tested: bind a new pbuffer. The one orphaned above will now be really
    // deallocated and we hope some sort of assert fires if something goes wrong.
    EXPECT_TRUE(eglBindTexImage(window->getDisplay(), otherPbuffer, EGL_BACK_BUFFER));

    // Write yellow.
    drawColorQuad(GLColor::yellow);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(mPbufferSize) / 2,
                          static_cast<GLint>(mPbufferSize) / 2, GLColor::yellow);

    destroyTestPbufferSurface(otherPbuffer);
    ASSERT_EGL_SUCCESS();
    ASSERT_GL_NO_ERROR();
}

// Verify that binding a pbuffer works after using a texture normally.
TEST_P(PbufferTest, BindTexImageAfterTexImage)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);

    EGLWindow *window = getEGLWindow();

    // Apply the Pbuffer and clear it to magenta
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(static_cast<GLint>(mPbufferSize) / 2,
                          static_cast<GLint>(mPbufferSize) / 2, GLColor::magenta);

    // Apply the window surface
    window->makeCurrent();
    glViewport(0, 0, getWindowWidth(), getWindowHeight());

    // Create a simple blue texture.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);
    EXPECT_GL_NO_ERROR();

    // Draw a quad and verify blue
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Bind the Pbuffer to the texture
    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    // Draw a quad and verify magenta
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);

    // Unbind the texture
    eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();
}

// Test clearing a Pbuffer in sRGB colorspace and checking the color is correct.
// Then bind the Pbuffer to a texture and verify it renders correctly
TEST_P(PbufferTest, ClearAndBindTexImageSrgb)
{
    EGLWindow *window = getEGLWindow();

    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_colorspace"));
    // Possible GLES driver bug on Pixel2 devices: http://anglebug.com/42263865
    ANGLE_SKIP_TEST_IF(IsPixel2() && IsOpenGLES());

    GLubyte kLinearColor[] = {132, 55, 219, 255};
    GLubyte kSrgbColor[]   = {190, 128, 238, 255};

    // Switch to sRGB
    recreatePbufferInSrgbColorspace();
    EGLint colorspace = 0;
    eglQuerySurface(window->getDisplay(), mPbuffer, EGL_GL_COLORSPACE, &colorspace);
    EXPECT_EQ(colorspace, EGL_GL_COLORSPACE_SRGB_KHR);

    // Clear the Pbuffer surface with `kLinearColor`
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(kLinearColor[0] / 255.0f, kLinearColor[1] / 255.0f, kLinearColor[2] / 255.0f,
                 kLinearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Expect glReadPixels to be `kSrgbColor` with a tolerance of 1
    EXPECT_PIXEL_NEAR(static_cast<GLint>(mPbufferSize) / 2, static_cast<GLint>(mPbufferSize) / 2,
                      kSrgbColor[0], kSrgbColor[1], kSrgbColor[2], kSrgbColor[3], 1);

    window->makeCurrent();

    // Create a texture and bind the Pbuffer to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Sample from a texture with `kSrgbColor` data and render into a surface in linear colorspace.
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Unbind the texture
    eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    // Expect glReadPixels to be `kLinearColor` with a tolerance of 1
    EXPECT_PIXEL_NEAR(getWindowWidth() / 2, getWindowHeight() / 2, kLinearColor[0], kLinearColor[1],
                      kLinearColor[2], kLinearColor[3], 1);

    glDeleteTextures(1, &texture);
}

// Test clearing a Pbuffer in sRGB colorspace and checking the color is correct.
// Then bind the Pbuffer to a texture and verify it renders correctly.
// Then change texture state to skip decode and verify it renders correctly.
TEST_P(PbufferTest, ClearAndBindTexImageSrgbSkipDecode)
{
    EGLWindow *window = getEGLWindow();

    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(window->getDisplay(), "EGL_KHR_gl_colorspace"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode"));
    // Possible GLES driver bug on Pixel devices: http://anglebug.com/42263865
    ANGLE_SKIP_TEST_IF((IsPixel2() || IsPixel4()) && IsOpenGLES());

    GLubyte kLinearColor[] = {132, 55, 219, 255};
    GLubyte kSrgbColor[]   = {190, 128, 238, 255};

    // Switch to sRGB
    recreatePbufferInSrgbColorspace();
    EGLint colorspace = 0;
    eglQuerySurface(window->getDisplay(), mPbuffer, EGL_GL_COLORSPACE, &colorspace);
    EXPECT_EQ(colorspace, EGL_GL_COLORSPACE_SRGB_KHR);

    // Clear the Pbuffer surface with `kLinearColor`
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(kLinearColor[0] / 255.0f, kLinearColor[1] / 255.0f, kLinearColor[2] / 255.0f,
                 kLinearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Expect glReadPixels to be `kSrgbColor` with a tolerance of 1
    EXPECT_PIXEL_NEAR(static_cast<GLint>(mPbufferSize) / 2, static_cast<GLint>(mPbufferSize) / 2,
                      kSrgbColor[0], kSrgbColor[1], kSrgbColor[2], kSrgbColor[3], 1);

    window->makeCurrent();

    // Create a texture and bind the Pbuffer to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Sample from a texture with `kSrgbColor` data and render into a surface in linear colorspace.
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Expect glReadPixels to be `kLinearColor` with a tolerance of 1
    EXPECT_PIXEL_NEAR(getWindowWidth() / 2, getWindowHeight() / 2, kLinearColor[0], kLinearColor[1],
                      kLinearColor[2], kLinearColor[3], 1);

    // Set skip decode for the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
    drawQuad(mTextureProgram, "position", 0.5f);

    // Texture is in skip decode mode, expect glReadPixels to be `kSrgbColor` with tolerance of 1
    EXPECT_PIXEL_NEAR(getWindowWidth() / 2, getWindowHeight() / 2, kSrgbColor[0], kSrgbColor[1],
                      kSrgbColor[2], kSrgbColor[3], 1);

    // Unbind the texture
    eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    glDeleteTextures(1, &texture);
}

// Verify that when eglBind/ReleaseTexImage are called, the texture images are freed and their
// size information is correctly updated.
TEST_P(PbufferTest, TextureSizeReset)
{
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers);
    ANGLE_SKIP_TEST_IF(!mSupportsBindTexImage);
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    // Fill the texture with white pixels
    std::vector<GLColor> whitePixels(mPbufferSize * mPbufferSize, GLColor::white);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(mPbufferSize),
                 static_cast<GLsizei>(mPbufferSize), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 whitePixels.data());
    EXPECT_GL_NO_ERROR();

    // Draw the white texture and verify that the pixels are correct
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    // Bind the EGL surface and draw with it, results are undefined since nothing has
    // been written to it
    EGLWindow *window = getEGLWindow();
    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Clear the back buffer to a unique color (green)
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Unbind the EGL surface and try to draw with the texture again, the texture's size should
    // now be zero and incomplete so the back buffer should be black
    eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
}

// Bind a Pbuffer, redefine the texture, and verify it renders correctly
TEST_P(PbufferTest, BindTexImageAndRedefineTexture)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);

    EGLWindow *window = getEGLWindow();

    // Apply the Pbuffer and clear it to purple
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(mPbufferSize) / 2, static_cast<GLint>(mPbufferSize) / 2, 255,
                    0, 255, 255);

    // Apply the window surface
    window->makeCurrent();

    // Create a texture and bind the Pbuffer to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Redefine the texture
    unsigned int pixelValue = 0xFFFF00FF;
    std::vector<unsigned int> pixelData(getWindowWidth() * getWindowHeight(), pixelValue);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, &pixelData[0]);

    // Draw a quad and verify that it is magenta
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Verify that magenta was drawn
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 255, 255);

    glDeleteTextures(1, &texture);
}

// Bind the Pbuffer to a texture, use that texture as Framebuffer color attachment and then
// destroy framebuffer, texture and Pbuffer.
TEST_P(PbufferTest, UseAsFramebufferColorThenDestroy)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);

    EGLWindow *window = getEGLWindow();

    // Apply the window surface
    window->makeCurrent();

    // Create a texture and bind the Pbuffer to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    // Create Framebuffer and use texture as color attachment
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ANGLE_SKIP_TEST_IF(status == GL_FRAMEBUFFER_UNSUPPORTED);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    ASSERT_GL_NO_ERROR();

    // Draw a quad in order to open a RenderPass
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(redProgram);
    ASSERT_GL_NO_ERROR();

    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Unbind resources
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_GL_NO_ERROR();

    // Delete resources
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
    ASSERT_GL_NO_ERROR();

    // Destroy Pbuffer
    destroyPbuffer();

    // Finish work
    glFinish();
    ASSERT_GL_NO_ERROR();
}

// Bind the Pbuffer to a texture, use that texture as Framebuffer color attachment and then
// destroy framebuffer, texture and Pbuffer. A bound but released TexImages are destroyed
// only when the binding is overwritten.
TEST_P(PbufferTest, UseAsFramebufferColorThenDeferredDestroy)
{
    // Test skipped because Pbuffers are not supported or Pbuffer does not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers || !mSupportsBindTexImage);
    EGLWindow *window = getEGLWindow();
    window->makeCurrent();

    // Create a texture and bind the Pbuffer to it
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    EGLSurface otherPbuffer = createTestPbufferSurface();
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(otherPbuffer, EGL_NO_SURFACE);

    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();
    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_ERROR(EGL_BAD_ACCESS);
    eglBindTexImage(window->getDisplay(), otherPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();
    eglReleaseTexImage(window->getDisplay(), otherPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();
    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();
    eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    destroyPbuffer();
    destroyTestPbufferSurface(otherPbuffer);
    ASSERT_EGL_SUCCESS();

    // Finish work
    glFinish();
    ASSERT_GL_NO_ERROR();
}

// Test the validation errors for bad parameters for eglCreatePbufferSurface
TEST_P(PbufferTest, NegativeValidationBadAttributes)
{
    EGLWindow *window = getEGLWindow();
    EGLSurface pbufferSurface;

    const EGLint invalidPBufferAttributeList[][3] = {
        {EGL_MIPMAP_TEXTURE, EGL_MIPMAP_TEXTURE, EGL_NONE},
        {EGL_LARGEST_PBUFFER, EGL_LARGEST_PBUFFER, EGL_NONE},
    };

    for (size_t i = 0; i < 2; i++)
    {
        pbufferSurface = eglCreatePbufferSurface(window->getDisplay(), window->getConfig(),
                                                 &invalidPBufferAttributeList[i][0]);
        ASSERT_EQ(pbufferSurface, EGL_NO_SURFACE);
        ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }
}

// Test that passing colorspace attributes do not generate EGL validation errors
// when EGL_ANGLE_colorspace_attribute_passthrough extension is supported.
TEST_P(PbufferColorspaceTest, CreateSurfaceWithColorspace)
{
    EGLDisplay dpy = getEGLWindow()->getDisplay();
    const bool extensionSupported =
        IsEGLDisplayExtensionEnabled(dpy, "EGL_EXT_gl_colorspace_display_p3_passthrough");
    const bool passthroughExtensionSupported =
        IsEGLDisplayExtensionEnabled(dpy, "EGL_ANGLE_colorspace_attribute_passthrough");

    EGLSurface pbufferSurface        = EGL_NO_SURFACE;
    const EGLint pBufferAttributes[] = {
        EGL_WIDTH,         static_cast<EGLint>(mPbufferSize),
        EGL_HEIGHT,        static_cast<EGLint>(mPbufferSize),
        EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_DISPLAY_P3_PASSTHROUGH_EXT,
        EGL_NONE,          EGL_NONE,
    };

    pbufferSurface = eglCreatePbufferSurface(dpy, getEGLWindow()->getConfig(), pBufferAttributes);
    if (extensionSupported)
    {
        // If EGL_EXT_gl_colorspace_display_p3_passthrough is supported
        // "pbufferSurface" should be a valid pbuffer surface.
        ASSERT_NE(pbufferSurface, EGL_NO_SURFACE);
        ASSERT_EGL_SUCCESS();
    }
    else if (!extensionSupported && passthroughExtensionSupported)
    {
        // If EGL_ANGLE_colorspace_attribute_passthrough was the only extension supported
        // we should not expect a validation error.
        ASSERT_NE(eglGetError(), EGL_BAD_ATTRIBUTE);
    }
    else
    {
        // Otherwise we should expect an EGL_BAD_ATTRIBUTE validation error.
        ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // Cleanup
    if (pbufferSurface != EGL_NO_SURFACE)
    {
        eglDestroySurface(dpy, pbufferSurface);
    }
}

ANGLE_INSTANTIATE_TEST_ES2(PbufferTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(PbufferColorspaceTest);
ANGLE_INSTANTIATE_TEST_ES3_AND(PbufferColorspaceTest,
                               ES3_VULKAN().enable(Feature::EglColorspaceAttributePassthrough));
