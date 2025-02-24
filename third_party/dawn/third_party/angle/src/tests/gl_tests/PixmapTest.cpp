//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/OSPixmap.h"
#include "util/OSWindow.h"

#include <iostream>

using namespace angle;

class PixmapTest : public ANGLETest<>
{
  protected:
    PixmapTest()
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
        mTextureProgram =
            CompileProgram(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "u_tex2D");
        ASSERT_NE(-1, mTextureUniformLocation);

        EGLWindow *window = getEGLWindow();

        EGLint surfaceType = 0;
        eglGetConfigAttrib(window->getDisplay(), window->getConfig(), EGL_SURFACE_TYPE,
                           &surfaceType);
        mSupportsPixmaps = (surfaceType & EGL_PIXMAP_BIT) != 0;

        EGLint bindToTextureRGBA = 0;
        eglGetConfigAttrib(window->getDisplay(), window->getConfig(), EGL_BIND_TO_TEXTURE_RGBA,
                           &bindToTextureRGBA);
        mSupportsBindTexImage =
            IsEGLDisplayExtensionEnabled(window->getDisplay(), "EGL_NOK_texture_from_pixmap") &&
            (bindToTextureRGBA == EGL_TRUE);

        if (mSupportsPixmaps)
        {
            mOSPixmap.reset(CreateOSPixmap());

            OSWindow *osWindow = getOSWindow();

            EGLint nativeVisual = 0;
            ASSERT_TRUE(eglGetConfigAttrib(window->getDisplay(), window->getConfig(),
                                           EGL_NATIVE_VISUAL_ID, &nativeVisual));
            ASSERT_TRUE(mOSPixmap->initialize(osWindow->getNativeDisplay(), mPixmapSize,
                                              mPixmapSize, nativeVisual));

            std::vector<EGLint> attribs;
            if (mSupportsBindTexImage)
            {
                attribs.push_back(EGL_TEXTURE_FORMAT);
                attribs.push_back(EGL_TEXTURE_RGBA);

                attribs.push_back(EGL_TEXTURE_TARGET);
                attribs.push_back(EGL_TEXTURE_2D);
            }

            attribs.push_back(EGL_NONE);

            mPixmap = eglCreatePixmapSurface(window->getDisplay(), window->getConfig(),
                                             mOSPixmap->getNativePixmap(), attribs.data());
            ASSERT_NE(mPixmap, EGL_NO_SURFACE);
            ASSERT_EGL_SUCCESS();
        }

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteProgram(mTextureProgram);

        if (mPixmap)
        {
            EGLWindow *window = getEGLWindow();
            eglDestroySurface(window->getDisplay(), mPixmap);
        }

        mOSPixmap = nullptr;
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;

    std::unique_ptr<OSPixmap> mOSPixmap;
    EGLSurface mPixmap = EGL_NO_SURFACE;

    const size_t mPixmapSize = 32;
    bool mSupportsPixmaps;
    bool mSupportsBindTexImage;
};

// Test clearing a Pixmap and checking the color is correct
TEST_P(PixmapTest, Clearing)
{
    ANGLE_SKIP_TEST_IF(!mSupportsPixmaps);

    EGLWindow *window = getEGLWindow();

    // Clear the window surface to blue and verify
    window->makeCurrent();
    ASSERT_EGL_SUCCESS();

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

    // Apply the Pixmap and clear it to purple and verify
    eglMakeCurrent(window->getDisplay(), mPixmap, mPixmap, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPixmapSize), static_cast<GLsizei>(mPixmapSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(static_cast<GLint>(mPixmapSize) / 2, static_cast<GLint>(mPixmapSize) / 2, 255,
                    0, 255, 255);

    // Rebind the window surface and verify that it is still blue
    window->makeCurrent();
    ASSERT_EGL_SUCCESS();
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);
}

// Bind the Pixmap to a texture and verify it renders correctly
TEST_P(PixmapTest, BindTexImage)
{
    // Test skipped because pixmaps are not supported or pixmaps do not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPixmaps || !mSupportsBindTexImage);

    // This test fails flakily on Linux intel when run with many other tests.
    ANGLE_SKIP_TEST_IF(IsLinux() && IsIntel());
    // http://anglebug.com/42263925
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    EGLWindow *window = getEGLWindow();

    // Apply the Pixmap and clear it to purple
    eglMakeCurrent(window->getDisplay(), mPixmap, mPixmap, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPixmapSize), static_cast<GLsizei>(mPixmapSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(mPixmapSize) / 2, static_cast<GLint>(mPixmapSize) / 2, 255,
                    0, 255, 255);

    // Apply the window surface
    window->makeCurrent();

    // Create a texture and bind the pixmap to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPixmap, EGL_BACK_BUFFER);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Draw a quad and verify that it is purple
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    // Unbind the texture
    eglReleaseTexImage(window->getDisplay(), mPixmap, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    // Verify that purple was drawn
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 255, 255);

    glDeleteTextures(1, &texture);
}

// Bind a Pixmap, redefine the texture, and verify it renders correctly
TEST_P(PixmapTest, BindTexImageAndRedefineTexture)
{
    // Test skipped because pixmaps are not supported or Pixmaps do not support binding to RGBA
    // textures.
    ANGLE_SKIP_TEST_IF(!mSupportsPixmaps || !mSupportsBindTexImage);

    EGLWindow *window = getEGLWindow();

    // Apply the Pixmap and clear it to purple
    eglMakeCurrent(window->getDisplay(), mPixmap, mPixmap, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPixmapSize), static_cast<GLsizei>(mPixmapSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(mPixmapSize) / 2, static_cast<GLint>(mPixmapSize) / 2, 255,
                    0, 255, 255);

    // Apply the window surface
    window->makeCurrent();

    // Create a texture and bind the Pixmap to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPixmap, EGL_BACK_BUFFER);
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

    drawQuad(mTextureProgram, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    // Verify that magenta was drawn
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 255, 255);

    glDeleteTextures(1, &texture);
}

ANGLE_INSTANTIATE_TEST_ES2(PixmapTest);
