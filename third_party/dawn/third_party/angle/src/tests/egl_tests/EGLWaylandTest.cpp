//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLWaylandTest.cpp: tests for EGL_EXT_platform_wayland

#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-client.h>
#include <wayland-egl-backend.h>

#include "test_utils/ANGLETest.h"
#include "util/linux/wayland/WaylandWindow.h"

using namespace angle;

namespace
{
const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
}

class EGLWaylandTest : public ANGLETest<>
{
  public:
    std::vector<EGLint> getDisplayAttributes() const
    {
        std::vector<EGLint> attribs;

        attribs.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        attribs.push_back(GetParam().getRenderer());
        attribs.push_back(EGL_NONE);

        return attribs;
    }

    void testSetUp() override
    {
        mOsWindow = WaylandWindow::New();
        ASSERT_TRUE(mOsWindow->initialize("EGLWaylandTest", 500, 500));
        setWindowVisible(mOsWindow, true);

        EGLNativeDisplayType waylandDisplay = mOsWindow->getNativeDisplay();
        std::vector<EGLint> attribs         = getDisplayAttributes();
        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, (void *)waylandDisplay,
                                            attribs.data());
        ASSERT_NE(EGL_NO_DISPLAY, mDisplay);

        ASSERT_TRUE(EGL_TRUE == eglInitialize(mDisplay, nullptr, nullptr));

        int nConfigs = 0;
        ASSERT_TRUE(EGL_TRUE == eglGetConfigs(mDisplay, nullptr, 0, &nConfigs));
        ASSERT_GE(nConfigs, 1);

        int nReturnedConfigs = 0;
        mConfigs.resize(nConfigs);
        ASSERT_TRUE(EGL_TRUE ==
                    eglGetConfigs(mDisplay, mConfigs.data(), nConfigs, &nReturnedConfigs));
        ASSERT_EQ(nConfigs, nReturnedConfigs);
    }

    void testTearDown() override
    {
        mConfigs.clear();
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
        OSWindow::Delete(&mOsWindow);
    }

    OSWindow *mOsWindow;
    EGLDisplay mDisplay;
    std::vector<EGLConfig> mConfigs;
};

// Test that a Wayland window can be created and used for rendering
TEST_P(EGLWaylandTest, WaylandWindowRendering)
{
    for (EGLConfig config : mConfigs)
    {
        // Finally, try to do a clear on the window.
        EGLContext context = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_NE(EGL_NO_CONTEXT, context);

        EGLSurface window =
            eglCreateWindowSurface(mDisplay, config, mOsWindow->getNativeWindow(), nullptr);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(mDisplay, window, window, context);
        ASSERT_EGL_SUCCESS();

        glViewport(0, 0, 500, 500);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(250, 250, 0, 0, 255, 255);

        // Teardown
        eglDestroySurface(mDisplay, window);
        ASSERT_EGL_SUCCESS();

        eglDestroyContext(mDisplay, context);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        ASSERT_EGL_SUCCESS();
    }
}

// Test that a Wayland window can swap buffers multiple times with no issues
TEST_P(EGLWaylandTest, SwapBuffers)
{
    for (EGLConfig config : mConfigs)
    {
        EGLContext context = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_NE(EGL_NO_CONTEXT, context);

        EGLSurface surface =
            eglCreateWindowSurface(mDisplay, config, mOsWindow->getNativeWindow(), nullptr);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(mDisplay, surface, surface, context);
        ASSERT_EGL_SUCCESS();

        const uint32_t loopcount = 16;
        for (uint32_t i = 0; i < loopcount; i++)
        {
            mOsWindow->messageLoop();

            glViewport(0, 0, 500, 500);
            glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ASSERT_GL_NO_ERROR() << "glClear failed";
            EXPECT_PIXEL_EQ(250, 250, 0, 0, 255, 255);

            eglSwapBuffers(mDisplay, surface);
            ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
        }

        // Teardown
        eglDestroySurface(mDisplay, surface);
        ASSERT_EGL_SUCCESS();

        eglDestroyContext(mDisplay, context);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        ASSERT_EGL_SUCCESS();
    }
}

ANGLE_INSTANTIATE_TEST(EGLWaylandTest, WithNoFixture(ES2_VULKAN()));
