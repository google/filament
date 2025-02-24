//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLSurfaceTestMac:
//   Tests pertaining to egl::Surface.
//

#include <gtest/gtest.h>

#include <vector>

#include <AppKit/AppKit.h>

#include "common/Color.h"
#include "common/platform.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"

using namespace angle;

namespace
{

class EGLSurfaceTestMac : public ANGLETest<>
{
  protected:
    void testSetUp() override
    {
        // Get display.
        EGLAttrib dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay              = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                                      reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        ASSERT_TRUE(eglInitialize(mDisplay, nullptr, nullptr) == EGL_TRUE);

        // Find a default config.
        const EGLint configAttributes[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE,     EGL_DONT_CARE,  EGL_GREEN_SIZE,
            EGL_DONT_CARE,    EGL_BLUE_SIZE,  EGL_DONT_CARE,    EGL_ALPHA_SIZE, EGL_DONT_CARE,
            EGL_DEPTH_SIZE,   EGL_DONT_CARE,  EGL_STENCIL_SIZE, EGL_DONT_CARE,  EGL_NONE};

        EGLint configCount;
        EGLint ret = eglChooseConfig(mDisplay, configAttributes, &mConfig, 1, &configCount);

        if (!ret || configCount == 0)
        {
            return;
        }

        // Create a window, context and surface if multisampling is possible.
        mOSWindow = OSWindow::New();
        mOSWindow->initialize("EGLSurfaceTestMac", kWindowWidth, kWindowHeight);
        setWindowVisible(mOSWindow, true);

        EGLint contextAttributes[] = {
            EGL_CONTEXT_MAJOR_VERSION_KHR,
            GetParam().majorVersion,
            EGL_CONTEXT_MINOR_VERSION_KHR,
            GetParam().minorVersion,
            EGL_NONE,
        };

        mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, contextAttributes);
        ASSERT_TRUE(mContext != EGL_NO_CONTEXT);
    }

    void testTearDown() override
    {
        if (mSurface)
        {
            eglSwapBuffers(mDisplay, mSurface);
        }

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (mSurface)
        {
            eglDestroySurface(mDisplay, mSurface);
            ASSERT_EGL_SUCCESS();
        }

        if (mContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(mDisplay, mContext);
            ASSERT_EGL_SUCCESS();
        }

        if (mOSWindow)
        {
            OSWindow::Delete(&mOSWindow);
        }

        eglTerminate(mDisplay);
    }

    void initializeSurface(int contentsScale)
    {
        EGLNativeWindowType nativeWindow = mOSWindow->getNativeWindow();
        CALayer *layer                   = reinterpret_cast<CALayer *>(nativeWindow);
        layer.contentsScale              = contentsScale;

        mSurface = eglCreateWindowSurface(mDisplay, mConfig, nativeWindow, nullptr);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
        ASSERT_EGL_SUCCESS();
    }

  protected:
    static constexpr int kWindowWidth  = 16;
    static constexpr int kWindowHeight = 8;

    OSWindow *mOSWindow = nullptr;
    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLContext mContext = EGL_NO_CONTEXT;
    EGLSurface mSurface = EGL_NO_SURFACE;
    EGLConfig mConfig;
};

// Test Mac's layer's content scale > 1 and check the initial viewport is set correctly.
TEST_P(EGLSurfaceTestMac, ContentsScale)
{
    constexpr int contentsScale = 2;
    initializeSurface(contentsScale);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    ASSERT_EQ(viewport[0], 0);
    ASSERT_EQ(viewport[1], 0);
    ASSERT_EQ(viewport[2], kWindowWidth * contentsScale);
    ASSERT_EQ(viewport[3], kWindowHeight * contentsScale);

    GLColor clearColor(255, 0, 0, 255);
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, clearColor);
    EXPECT_PIXEL_COLOR_EQ(kWindowWidth, 0, clearColor);
    EXPECT_PIXEL_COLOR_EQ(0, kWindowHeight, clearColor);
    EXPECT_PIXEL_COLOR_EQ(kWindowHeight * contentsScale - 1, kWindowHeight * contentsScale - 1,
                          clearColor);
}
}  // anonymous namespace

ANGLE_INSTANTIATE_TEST(EGLSurfaceTestMac,
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES3_METAL()));
