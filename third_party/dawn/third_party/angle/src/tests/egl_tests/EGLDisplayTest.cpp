//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>
#include <vector>

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/OSWindow.h"

using namespace angle;

class EGLDisplayTest : public ANGLETest<>
{
  protected:
    EGLConfig chooseConfig(EGLDisplay display)
    {
        const EGLint attribs[] = {EGL_RED_SIZE,
                                  8,
                                  EGL_GREEN_SIZE,
                                  8,
                                  EGL_BLUE_SIZE,
                                  8,
                                  EGL_ALPHA_SIZE,
                                  8,
                                  EGL_RENDERABLE_TYPE,
                                  EGL_OPENGL_ES2_BIT,
                                  EGL_SURFACE_TYPE,
                                  EGL_PBUFFER_BIT | EGL_WINDOW_BIT,
                                  EGL_NONE};
        EGLConfig config       = EGL_NO_CONFIG_KHR;
        EGLint count           = 0;
        EXPECT_EGL_TRUE(eglChooseConfig(display, attribs, &config, 1, &count));
        EXPECT_EGL_TRUE(count > 0);
        return config;
    }

    EGLContext createContext(EGLDisplay display, EGLConfig config)
    {
        const EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_NONE};
        EGLContext context     = eglCreateContext(display, config, nullptr, attribs);
        EXPECT_NE(context, EGL_NO_CONTEXT);
        return context;
    }

    EGLSurface createSurface(EGLDisplay display, EGLConfig config)
    {
        const EGLint attribs[] = {EGL_WIDTH, 64, EGL_HEIGHT, 64, EGL_NONE};
        EGLSurface surface     = eglCreatePbufferSurface(display, config, attribs);
        EXPECT_NE(surface, EGL_NO_SURFACE);
        return surface;
    }
};

// Tests that an eglInitialize can be re-initialized.  The spec says:
//
// > Initializing an already-initialized display is allowed, but the only effect of such a call is
// to return EGL_TRUE and update the EGL version numbers
TEST_P(EGLDisplayTest, InitializeMultipleTimes)
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint major = 0, minor = 0;
    EXPECT_EGL_TRUE(eglInitialize(display, &major, &minor) != EGL_FALSE);
    for (uint32_t i = 0; i < 10; ++i)
    {
        EGLint retryMajor = 123456, retryMinor = -1;
        EXPECT_EGL_TRUE(eglInitialize(display, &retryMajor, &retryMinor) != EGL_FALSE);
        EXPECT_EQ(major, retryMajor) << i;
        EXPECT_EQ(minor, retryMinor) << i;
    }
}

// Test that call eglInitialize() in parallel in multiple threads works
// > Initializing an already-initialized display is allowed, but the only effect
// of such a call is to return EGL_TRUE and update the EGL version numbers
TEST_P(EGLDisplayTest, InitializeMultipleTimesInDifferentThreads)
{
    std::array<std::thread, 10> threads;
    for (std::thread &thread : threads)
    {
        thread = std::thread([&]() {
            EGLDisplay display                 = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            const int INVALID_GL_MAJOR_VERSION = -1;
            const int INVALID_GL_MINOR_VERSION = -1;
            EGLint threadMajor                 = INVALID_GL_MAJOR_VERSION;
            EGLint threadMinor                 = INVALID_GL_MINOR_VERSION;
            EXPECT_EGL_TRUE(eglInitialize(display, &threadMajor, &threadMinor) != EGL_FALSE);
            EXPECT_NE(threadMajor, INVALID_GL_MAJOR_VERSION);
            EXPECT_NE(threadMinor, INVALID_GL_MINOR_VERSION);
        });
    }

    for (std::thread &thread : threads)
    {
        thread.join();
    }
}

// Tests that an EGLDisplay can be re-initialized.
TEST_P(EGLDisplayTest, InitializeTerminateInitialize)
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EXPECT_EGL_TRUE(eglInitialize(display, nullptr, nullptr) != EGL_FALSE);
    EXPECT_EGL_TRUE(eglTerminate(display) != EGL_FALSE);
    EXPECT_EGL_TRUE(eglInitialize(display, nullptr, nullptr) != EGL_FALSE);
}

// Tests that an EGLDisplay can be re-initialized after it was used to draw into a window surface.
TEST_P(EGLDisplayTest, InitializeDrawSwapTerminateLoop)
{
    constexpr int kLoopCount = 2;
    constexpr EGLint kWidth  = 64;
    constexpr EGLint kHeight = 64;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("LockSurfaceTest", kWidth, kHeight);

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    for (int i = 0; i < kLoopCount; ++i)
    {
        EXPECT_EGL_TRUE(eglInitialize(display, nullptr, nullptr) != EGL_FALSE);

        EGLConfig config   = chooseConfig(display);
        EGLContext context = createContext(display, config);
        EGLSurface surface =
            eglCreateWindowSurface(display, config, osWindow->getNativeWindow(), nullptr);
        EXPECT_NE(surface, EGL_NO_SURFACE);

        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, context));

        ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);

        EXPECT_EGL_TRUE(eglSwapBuffers(display, surface));

        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_TRUE(eglTerminate(display) != EGL_FALSE);
    }

    osWindow->destroy();
    OSWindow::Delete(&osWindow);
}

// Tests current Context leaking when call eglTerminate() while it is current.
TEST_P(EGLDisplayTest, ContextLeakAfterTerminate)
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EXPECT_EGL_TRUE(eglInitialize(display, nullptr, nullptr));

    EGLConfig config   = chooseConfig(display);
    EGLContext context = createContext(display, config);
    EGLSurface surface = createSurface(display, config);

    // Make "context" current.
    EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, context));

    // Terminate display while "context" is current.
    EXPECT_EGL_TRUE(eglTerminate(display));

    // Unmake "context" from current and allow Display to actually terminate.
    (void)eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    // Get EGLDisplay again.
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    // Check if Display was actually terminated.
    EGLint val;
    EXPECT_EGL_FALSE(eglQueryContext(display, context, EGL_CONTEXT_CLIENT_TYPE, &val));
    EXPECT_EQ(eglGetError(), EGL_NOT_INITIALIZED);
}

ANGLE_INSTANTIATE_TEST(EGLDisplayTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_METAL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES3_VULKAN()));
