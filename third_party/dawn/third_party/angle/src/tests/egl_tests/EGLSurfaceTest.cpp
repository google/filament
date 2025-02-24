//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLSurfaceTest:
//   Tests pertaining to egl::Surface.
//

#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "common/Color.h"
#include "common/platform.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"
#include "util/Timer.h"

#if defined(ANGLE_ENABLE_D3D11)
#    define INITGUID
#    include <guiddef.h>

#    include <d3d11.h>
#    include <dcomp.h>
#endif

using namespace angle;

namespace
{

class EGLSurfaceTest : public ANGLETest<>
{
  protected:
    EGLSurfaceTest()
        : mDisplay(EGL_NO_DISPLAY),
          mWindowSurface(EGL_NO_SURFACE),
          mPbufferSurface(EGL_NO_SURFACE),
          mContext(EGL_NO_CONTEXT),
          mSecondContext(EGL_NO_CONTEXT),
          mOSWindow(nullptr)
    {}

    void testSetUp() override
    {
        mOSWindow = OSWindow::New();
        mOSWindow->initialize("EGLSurfaceTest", 64, 64);
    }

    void tearDownContextAndSurface()
    {
        if (mDisplay == EGL_NO_DISPLAY)
        {
            return;
        }

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (mWindowSurface != EGL_NO_SURFACE)
        {
            eglDestroySurface(mDisplay, mWindowSurface);
            mWindowSurface = EGL_NO_SURFACE;
        }

        if (mPbufferSurface != EGL_NO_SURFACE)
        {
            eglDestroySurface(mDisplay, mPbufferSurface);
            mPbufferSurface = EGL_NO_SURFACE;
        }

        if (mContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(mDisplay, mContext);
            mContext = EGL_NO_CONTEXT;
        }

        if (mSecondContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(mDisplay, mSecondContext);
            mSecondContext = EGL_NO_CONTEXT;
        }
    }

    // Release any resources created in the test body
    void testTearDown() override
    {
        tearDownContextAndSurface();

        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
        }

        mOSWindow->destroy();
        OSWindow::Delete(&mOSWindow);

        for (OSWindow *win : mOtherWindows)
        {
            if (win != nullptr)
            {
                win->destroy();
                OSWindow::Delete(&win);
            }
        }
        mOtherWindows.clear();

        ASSERT_TRUE(mWindowSurface == EGL_NO_SURFACE && mContext == EGL_NO_CONTEXT);
    }

    void initializeDisplay()
    {
        GLenum platformType = GetParam().getRenderer();
        GLenum deviceType   = GetParam().getDeviceType();

        std::vector<EGLint> displayAttributes;
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        displayAttributes.push_back(platformType);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
        displayAttributes.push_back(EGL_DONT_CARE);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
        displayAttributes.push_back(EGL_DONT_CARE);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
        displayAttributes.push_back(deviceType);
        displayAttributes.push_back(EGL_NONE);

        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                            reinterpret_cast<void *>(mOSWindow->getNativeDisplay()),
                                            displayAttributes.data());
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        EGLint majorVersion, minorVersion;
        ASSERT_TRUE(eglInitialize(mDisplay, &majorVersion, &minorVersion) == EGL_TRUE);

        eglBindAPI(EGL_OPENGL_ES_API);
        ASSERT_EGL_SUCCESS();
    }

    void initializeSingleContext(EGLContext *context, EGLint virtualizationGroup = EGL_DONT_CARE)
    {
        ASSERT_TRUE(*context == EGL_NO_CONTEXT);

        EGLint contextAttibutes[] = {EGL_CONTEXT_CLIENT_VERSION, GetParam().majorVersion,
                                     EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE, virtualizationGroup,
                                     EGL_NONE};

        if (!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_context_virtualization"))
        {
            contextAttibutes[2] = EGL_NONE;
        }

        *context = eglCreateContext(mDisplay, mConfig, nullptr, contextAttibutes);
        ASSERT_EGL_SUCCESS();
        ASSERT_TRUE(*context != EGL_NO_CONTEXT);
    }

    void initializeMainContext() { initializeSingleContext(&mContext); }

    void initializeAllContexts()
    {
        initializeSingleContext(&mContext);
        initializeSingleContext(&mSecondContext);
    }

    void initializeWindowSurfaceWithAttribs(EGLConfig config,
                                            const std::vector<EGLint> &additionalAttributes,
                                            EGLenum expectedResult)
    {
        ASSERT_TRUE(mWindowSurface == EGL_NO_SURFACE);

        EGLint surfaceType = EGL_NONE;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_SURFACE_TYPE, &surfaceType);

        if (surfaceType & EGL_WINDOW_BIT)
        {
            std::vector<EGLint> windowAttributes = additionalAttributes;
            windowAttributes.push_back(EGL_NONE);

            // Create first window surface
            mWindowSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(),
                                                    windowAttributes.data());
        }

        ASSERT_EGLENUM_EQ(eglGetError(), expectedResult);
    }

    void initializeSurfaceWithAttribs(EGLConfig config,
                                      const std::vector<EGLint> &additionalAttributes)
    {
        mConfig = config;

        EGLint surfaceType = EGL_NONE;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_SURFACE_TYPE, &surfaceType);

        if (surfaceType & EGL_WINDOW_BIT)
        {
            initializeWindowSurfaceWithAttribs(config, additionalAttributes, EGL_SUCCESS);
        }

        if (surfaceType & EGL_PBUFFER_BIT)
        {
            ASSERT_TRUE(mPbufferSurface == EGL_NO_SURFACE);

            std::vector<EGLint> pbufferAttributes = additionalAttributes;

            // Give pbuffer non-zero dimensions.
            pbufferAttributes.push_back(EGL_WIDTH);
            pbufferAttributes.push_back(64);
            pbufferAttributes.push_back(EGL_HEIGHT);
            pbufferAttributes.push_back(64);
            pbufferAttributes.push_back(EGL_NONE);

            mPbufferSurface = eglCreatePbufferSurface(mDisplay, mConfig, pbufferAttributes.data());
            ASSERT_EGL_SUCCESS();
        }
    }

    void initializeSurface(EGLConfig config)
    {
        std::vector<EGLint> additionalAttributes;
        initializeSurfaceWithAttribs(config, additionalAttributes);
    }

    EGLConfig chooseDefaultConfig(bool requireWindowSurface) const
    {
        const EGLint configAttributes[] = {EGL_RED_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_GREEN_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_BLUE_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_ALPHA_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_DEPTH_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_STENCIL_SIZE,
                                           EGL_DONT_CARE,
                                           EGL_SAMPLE_BUFFERS,
                                           0,
                                           EGL_SURFACE_TYPE,
                                           requireWindowSurface ? EGL_WINDOW_BIT : EGL_DONT_CARE,
                                           EGL_NONE};

        EGLint configCount;
        EGLConfig config;
        if (eglChooseConfig(mDisplay, configAttributes, &config, 1, &configCount) != EGL_TRUE)
            return nullptr;
        if (configCount != 1)
            return nullptr;
        return config;
    }

    void initializeSurfaceWithDefaultConfig(bool requireWindowSurface)
    {
        EGLConfig defaultConfig = chooseDefaultConfig(requireWindowSurface);
        ASSERT_NE(defaultConfig, nullptr);
        initializeSurface(defaultConfig);
    }

    GLuint createProgram(const char *fs = angle::essl1_shaders::fs::Red())
    {
        return CompileProgram(angle::essl1_shaders::vs::Simple(), fs);
    }

    void drawWithProgram(GLuint program)
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        GLint positionLocation =
            glGetAttribLocation(program, angle::essl1_shaders::PositionAttrib());

        glUseProgram(program);

        const GLfloat vertices[] = {
            -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,

            -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
        };

        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(positionLocation);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        EXPECT_PIXEL_EQ(mOSWindow->getWidth() / 2, mOSWindow->getHeight() / 2, 255, 0, 0, 255);
    }

    void runMessageLoopTest(EGLSurface secondSurface, EGLContext secondContext)
    {
        eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
        ASSERT_EGL_SUCCESS();

        // Make a second context current
        eglMakeCurrent(mDisplay, secondSurface, secondSurface, secondContext);
        eglDestroySurface(mDisplay, mWindowSurface);

        // Create second window surface
        std::vector<EGLint> surfaceAttributes;
        surfaceAttributes.push_back(EGL_NONE);
        surfaceAttributes.push_back(EGL_NONE);

        mWindowSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(),
                                                &surfaceAttributes[0]);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
        ASSERT_EGL_SUCCESS();

        mOSWindow->signalTestEvent();
        mOSWindow->messageLoop();
        ASSERT_TRUE(mOSWindow->didTestEventFire());

        // Simple operation to test the FBO is set appropriately
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void runWaitSemaphoreTest(bool useSecondContext);
    void runDestroyNotCurrentSurfaceTest(bool testWindowsSurface);

    void drawQuadThenTearDown();

    EGLDisplay mDisplay;
    EGLSurface mWindowSurface;
    EGLSurface mPbufferSurface;
    EGLContext mContext;
    EGLContext mSecondContext;
    EGLConfig mConfig;
    OSWindow *mOSWindow;
    std::vector<OSWindow *> mOtherWindows;
};

class EGLFloatSurfaceTest : public EGLSurfaceTest
{
  protected:
    EGLFloatSurfaceTest() : EGLSurfaceTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
    }

    void testSetUp() override
    {
        mOSWindow = OSWindow::New();
        mOSWindow->initialize("EGLFloatSurfaceTest", 64, 64);
    }

    void testTearDown() override
    {
        EGLSurfaceTest::testTearDown();
        glDeleteProgram(mProgram);
    }

    GLuint createProgram()
    {
        constexpr char kFS[] =
            "precision highp float;\n"
            "void main()\n"
            "{\n"
            "   gl_FragColor = vec4(1.0, 2.0, 3.0, 4.0);\n"
            "}\n";
        return CompileProgram(angle::essl1_shaders::vs::Simple(), kFS);
    }

    bool initializeSurfaceWithFloatConfig()
    {
        const EGLint configAttributes[] = {EGL_SURFACE_TYPE,
                                           EGL_WINDOW_BIT,
                                           EGL_RED_SIZE,
                                           16,
                                           EGL_GREEN_SIZE,
                                           16,
                                           EGL_BLUE_SIZE,
                                           16,
                                           EGL_ALPHA_SIZE,
                                           16,
                                           EGL_COLOR_COMPONENT_TYPE_EXT,
                                           EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT,
                                           EGL_NONE,
                                           EGL_NONE};

        initializeDisplay();
        EGLConfig config;
        if (EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config) == EGL_FALSE)
        {
            std::cout << "EGLConfig for a float surface is not supported, skipping test"
                      << std::endl;
            return false;
        }

        initializeSurface(config);
        initializeMainContext();

        eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
        mProgram = createProgram();
        return true;
    }

    GLuint mProgram;
};

class EGLSingleBufferTest : public ANGLETest<>
{
  protected:
    EGLSingleBufferTest() {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);
        ASSERT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
        mMajorVersion = GetParam().majorVersion;
    }

    void testTearDown() override
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
    }

    bool chooseConfig(EGLConfig *config, bool mutableRenderBuffer) const
    {
        bool result          = false;
        EGLint count         = 0;
        EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
        EGLint attribs[]     = {
            EGL_RED_SIZE,
            8,
            EGL_GREEN_SIZE,
            8,
            EGL_BLUE_SIZE,
            8,
            EGL_ALPHA_SIZE,
            0,
            EGL_RENDERABLE_TYPE,
            clientVersion,
            EGL_SURFACE_TYPE,
            EGL_WINDOW_BIT | (mutableRenderBuffer ? EGL_MUTABLE_RENDER_BUFFER_BIT_KHR : 0),
            EGL_NONE};

        result = eglChooseConfig(mDisplay, attribs, config, 1, &count);
        return result && (count > 0);
    }

    bool createContext(EGLConfig config, EGLContext *context)
    {
        EXPECT_TRUE(*context == EGL_NO_CONTEXT);

        bool result      = false;
        EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};

        *context = eglCreateContext(mDisplay, config, nullptr, attribs);
        result   = (*context != EGL_NO_CONTEXT);
        EXPECT_TRUE(result);
        return result;
    }

    bool createWindowSurface(EGLConfig config,
                             EGLNativeWindowType win,
                             EGLSurface *surface,
                             EGLint renderBuffer) const
    {
        EXPECT_TRUE(*surface == EGL_NO_SURFACE);

        bool result      = false;
        EGLint attribs[] = {EGL_RENDER_BUFFER, renderBuffer, EGL_NONE};

        *surface = eglCreateWindowSurface(mDisplay, config, win, attribs);
        result   = (*surface != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    uint32_t drawAndSwap(EGLSurface &surface, EGLDisplay &display, uint32_t color, bool flush);

    EGLDisplay mDisplay  = EGL_NO_DISPLAY;
    EGLint mMajorVersion = 0;
    const EGLint kWidth  = 32;
    const EGLint kHeight = 32;
};

class EGLAndroidAutoRefreshTest : public EGLSingleBufferTest
{};

// Test clearing and checking the color is correct
TEST_P(EGLFloatSurfaceTest, Clearing)
{
    ANGLE_SKIP_TEST_IF(!initializeSurfaceWithFloatConfig());

    ASSERT_NE(0u, mProgram) << "shader compilation failed.";
    ASSERT_GL_NO_ERROR();

    GLColor32F clearColor(0.0f, 1.0f, 2.0f, 3.0f);
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR32F_EQ(0, 0, clearColor);
}

// Test drawing and checking the color is correct
TEST_P(EGLFloatSurfaceTest, Drawing)
{
    ANGLE_SKIP_TEST_IF(!initializeSurfaceWithFloatConfig());

    ASSERT_NE(0u, mProgram) << "shader compilation failed.";
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_32F_EQ(0, 0, 1.0f, 2.0f, 3.0f, 4.0f);
}

class EGLSurfaceTest3 : public EGLSurfaceTest
{};

// Test a surface bug where we could have two Window surfaces active
// at one time, blocking message loops. See http://crbug.com/475085
TEST_P(EGLSurfaceTest, MessageLoopBug)
{
    // http://anglebug.com/42261801
    ANGLE_SKIP_TEST_IF(IsAndroid());

    // http://anglebug.com/42261815
    ANGLE_SKIP_TEST_IF(IsOzone());

    // http://anglebug.com/42264022
    ANGLE_SKIP_TEST_IF(IsIOS());

    initializeDisplay();
    initializeSurfaceWithDefaultConfig(true);
    initializeMainContext();

    runMessageLoopTest(EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

// Tests the message loop bug, but with setting a second context
// instead of null.
TEST_P(EGLSurfaceTest, MessageLoopBugContext)
{
    // http://anglebug.com/42261801
    ANGLE_SKIP_TEST_IF(IsAndroid());

    // http://anglebug.com/42261815
    ANGLE_SKIP_TEST_IF(IsOzone());

    // http://anglebug.com/42264022
    ANGLE_SKIP_TEST_IF(IsIOS());

    initializeDisplay();
    initializeSurfaceWithDefaultConfig(true);
    initializeAllContexts();

    ANGLE_SKIP_TEST_IF(!mPbufferSurface);
    runMessageLoopTest(mPbufferSurface, mSecondContext);
}

// Test a bug where calling makeCurrent twice would release the surface
TEST_P(EGLSurfaceTest, MakeCurrentTwice)
{
    initializeDisplay();
    initializeSurfaceWithDefaultConfig(false);
    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Simple operation to test the FBO is set appropriately
    glClear(GL_COLOR_BUFFER_BIT);
}

// Test that we dont crash during a clear when specified scissor is outside render area
// due to reducing window size.
TEST_P(EGLSurfaceTest, ShrinkWindowThenScissoredClear)
{
    initializeDisplay();
    initializeSurfaceWithDefaultConfig(false);
    initializeMainContext();

    // Create 64x64 window and make it current
    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Resize window to 32x32
    mOSWindow->resize(32, 32);

    // Perform empty swap
    eglSwapBuffers(mDisplay, mWindowSurface);

    // Enable scissor test
    glEnable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();

    // Set scissor to (50, 50, 10, 10)
    glScissor(50, 50, 10, 10);
    ASSERT_GL_NO_ERROR();

    // Clear to specific color
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable scissor test
    glDisable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();
}

// Test that we dont early return from a clear when specified scissor is outside render area
// before increasing window size.
TEST_P(EGLSurfaceTest, GrowWindowThenScissoredClear)
{
    initializeDisplay();
    initializeSurfaceWithDefaultConfig(false);
    initializeMainContext();

    // Create 64x64 window and make it current
    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Resize window to 128x128
    mOSWindow->resize(128, 128);

    // Perform empty swap
    eglSwapBuffers(mDisplay, mWindowSurface);

    // Enable scissor test
    glEnable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();

    // Set scissor to (64, 64, 10, 10)
    glScissor(64, 64, 10, 10);
    ASSERT_GL_NO_ERROR();

    // Clear to specific color
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable scissor test
    glDisable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(64, 64, 10, 10, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Test that just a ClearBuffer* with an invalid scissor doesn't cause an assert.
TEST_P(EGLSurfaceTest3, ShrinkWindowThenScissoredClearBuffer)
{
    initializeDisplay();
    initializeSurfaceWithDefaultConfig(false);
    initializeMainContext();

    // Create 64x64 window and make it current
    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Resize window to 32x32
    mOSWindow->resize(32, 32);

    // Perform empty swap
    eglSwapBuffers(mDisplay, mWindowSurface);

    // Enable scissor test
    glEnable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();

    // Set scissor to (50, 50, 10, 10)
    glScissor(50, 50, 10, 10);
    ASSERT_GL_NO_ERROR();

    std::vector<GLint> testInt(4);
    glClearBufferiv(GL_COLOR, 0, testInt.data());
    std::vector<GLuint> testUint(4);
    glClearBufferuiv(GL_COLOR, 0, testUint.data());
    std::vector<GLfloat> testFloat(4);
    glClearBufferfv(GL_COLOR, 0, testFloat.data());

    // Disable scissor test
    glDisable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();
}

// This is a regression test to verify that surfaces are not prematurely destroyed.
TEST_P(EGLSurfaceTest, SurfaceUseAfterFreeBug)
{
    initializeDisplay();

    // Initialize an RGBA8 window and pbuffer surface
    constexpr EGLint kSurfaceAttributes[] = {EGL_RED_SIZE,     8,
                                             EGL_GREEN_SIZE,   8,
                                             EGL_BLUE_SIZE,    8,
                                             EGL_ALPHA_SIZE,   8,
                                             EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                             EGL_NONE};

    EGLint configCount      = 0;
    EGLConfig surfaceConfig = nullptr;
    ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, kSurfaceAttributes, &surfaceConfig, 1, &configCount));
    ASSERT_NE(configCount, 0);
    ASSERT_NE(surfaceConfig, nullptr);

    initializeSurface(surfaceConfig);
    initializeAllContexts();
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);
    ASSERT_NE(mPbufferSurface, EGL_NO_SURFACE);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mSecondContext);
    ASSERT_EGL_SUCCESS();
    glClear(GL_COLOR_BUFFER_BIT);

    eglMakeCurrent(mDisplay, mPbufferSurface, mPbufferSurface, mContext);
    ASSERT_EGL_SUCCESS();
    glClear(GL_COLOR_BUFFER_BIT);

    eglDestroySurface(mDisplay, mPbufferSurface);
    ASSERT_EGL_SUCCESS();
    mPbufferSurface = EGL_NO_SURFACE;

    eglDestroyContext(mDisplay, mSecondContext);
    ASSERT_EGL_SUCCESS();
    mSecondContext = EGL_NO_CONTEXT;
}

// Test that the window surface is correctly resized after calling swapBuffers
TEST_P(EGLSurfaceTest, ResizeWindow)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());
    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());
    // http://anglebug.com/42264022
    ANGLE_SKIP_TEST_IF(IsIOS());
    ANGLE_SKIP_TEST_IF(IsLinux() && IsARM());

    // Necessary for a window resizing test if there is no per-frame window size query
    setWindowVisible(mOSWindow, true);

    GLenum platform               = GetParam().getRenderer();
    bool platformSupportsZeroSize = platform == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE ||
                                    platform == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE;
    int minSize = platformSupportsZeroSize ? 0 : 1;

    initializeDisplay();
    initializeSurfaceWithDefaultConfig(true);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    EGLint height;
    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(64, height);  // initial size

    // set window's height to 0 (if possible) or 1
    mOSWindow->resize(64, minSize);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    // TODO(syoussefi): the GLX implementation still reads the window size as 64x64 through
    // XGetGeometry.  http://anglebug.com/42261800
    ANGLE_SKIP_TEST_IF(IsLinux() && IsOpenGL());

    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(minSize, height);

    // restore window's height
    mOSWindow->resize(64, 64);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(64, height);
}

// Test that the backbuffer is correctly resized after calling swapBuffers
TEST_P(EGLSurfaceTest, ResizeWindowWithDraw)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux());
    // http://anglebug.com/42264022
    ANGLE_SKIP_TEST_IF(IsIOS());

    // Necessary for a window resizing test if there is no per-frame window size query
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithDefaultConfig(true);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);

    int size      = 64;
    EGLint height = 0;
    EGLint width  = 0;

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    // Clear to red
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &width);
    ASSERT_EGL_SUCCESS();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(size - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(size - 1, size - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, size - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(-1, -1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(size, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, size, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(size, size, GLColor::transparentBlack);

    // set window's size small
    size = 1;
    mOSWindow->resize(size, size);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    // Clear to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &width);
    ASSERT_EGL_SUCCESS();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(size - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(size - 1, size - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, size - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(-1, -1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(size, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, size, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(size, size, GLColor::transparentBlack);

    // set window's height large
    size = 128;
    mOSWindow->resize(size, size);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    // Clear to blue
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &height);
    eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &width);
    ASSERT_EGL_SUCCESS();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(size - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(size - 1, size - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, size - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(-1, -1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(size, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, size, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(size, size, GLColor::transparentBlack);
}

// Test that the window can be reset repeatedly before surface creation.
TEST_P(EGLSurfaceTest, ResetNativeWindow)
{
    setWindowVisible(mOSWindow, true);

    initializeDisplay();

    for (int i = 0; i < 10; ++i)
    {
        mOSWindow->resetNativeWindow();
    }

    initializeSurfaceWithDefaultConfig(true);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();
}

// Test swap buffer without any draw calls.
TEST_P(EGLSurfaceTest, SwapWithoutAnyDraw)
{
    initializeDisplay();
    initializeSurfaceWithDefaultConfig(true);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    for (int i = 0; i < 10; ++i)
    {
        eglSwapBuffers(mDisplay, mWindowSurface);
        ASSERT_EGL_SUCCESS();
    }
}

// Test creating a surface that supports a EGLConfig with 16bit
// support GL_RGB565
TEST_P(EGLSurfaceTest, CreateWithEGLConfig5650Support)
{
    const EGLint configAttributes[] = {EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT,
                                       EGL_RED_SIZE,
                                       5,
                                       EGL_GREEN_SIZE,
                                       6,
                                       EGL_BLUE_SIZE,
                                       5,
                                       EGL_ALPHA_SIZE,
                                       0,
                                       EGL_DEPTH_SIZE,
                                       0,
                                       EGL_STENCIL_SIZE,
                                       0,
                                       EGL_SAMPLE_BUFFERS,
                                       0,
                                       EGL_NONE};

    initializeDisplay();
    EGLConfig config;
    if (EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config) == EGL_FALSE)
    {
        std::cout << "EGLConfig for a GL_RGB565 surface is not supported, skipping test"
                  << std::endl;
        return;
    }

    initializeSurface(config);
    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    drawWithProgram(program);
    EXPECT_GL_NO_ERROR();
    glDeleteProgram(program);
}

// Test creating a surface that supports a EGLConfig with 16bit
// support GL_RGBA4
TEST_P(EGLSurfaceTest, CreateWithEGLConfig4444Support)
{
    const EGLint configAttributes[] = {EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT,
                                       EGL_RED_SIZE,
                                       4,
                                       EGL_GREEN_SIZE,
                                       4,
                                       EGL_BLUE_SIZE,
                                       4,
                                       EGL_ALPHA_SIZE,
                                       4,
                                       EGL_DEPTH_SIZE,
                                       0,
                                       EGL_STENCIL_SIZE,
                                       0,
                                       EGL_SAMPLE_BUFFERS,
                                       0,
                                       EGL_NONE};

    initializeDisplay();
    EGLConfig config;
    if (EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config) == EGL_FALSE)
    {
        std::cout << "EGLConfig for a GL_RGBA4 surface is not supported, skipping test"
                  << std::endl;
        return;
    }

    initializeSurface(config);
    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    drawWithProgram(program);
    EXPECT_GL_NO_ERROR();
    glDeleteProgram(program);
}

// Test creating a surface that supports a EGLConfig with 16bit
// support GL_RGB5_A1
TEST_P(EGLSurfaceTest, CreateWithEGLConfig5551Support)
{
    const EGLint configAttributes[] = {EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT,
                                       EGL_RED_SIZE,
                                       5,
                                       EGL_GREEN_SIZE,
                                       5,
                                       EGL_BLUE_SIZE,
                                       5,
                                       EGL_ALPHA_SIZE,
                                       1,
                                       EGL_DEPTH_SIZE,
                                       0,
                                       EGL_STENCIL_SIZE,
                                       0,
                                       EGL_SAMPLE_BUFFERS,
                                       0,
                                       EGL_NONE};

    initializeDisplay();
    EGLConfig config;
    if (EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config) == EGL_FALSE)
    {
        std::cout << "EGLConfig for a GL_RGB5_A1 surface is not supported, skipping test"
                  << std::endl;
        return;
    }

    initializeSurface(config);
    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    drawWithProgram(program);
    EXPECT_GL_NO_ERROR();
    glDeleteProgram(program);
}

// Test creating a surface that supports a EGLConfig without alpha support
TEST_P(EGLSurfaceTest, CreateWithEGLConfig8880Support)
{
    const EGLint configAttributes[] = {EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT,
                                       EGL_RED_SIZE,
                                       8,
                                       EGL_GREEN_SIZE,
                                       8,
                                       EGL_BLUE_SIZE,
                                       8,
                                       EGL_ALPHA_SIZE,
                                       0,
                                       EGL_DEPTH_SIZE,
                                       0,
                                       EGL_STENCIL_SIZE,
                                       0,
                                       EGL_SAMPLE_BUFFERS,
                                       0,
                                       EGL_NONE};

    initializeDisplay();
    EGLConfig config;
    if (EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config) == EGL_FALSE)
    {
        std::cout << "EGLConfig for a GL_RGB8_OES surface is not supported, skipping test"
                  << std::endl;
        return;
    }

    initializeSurface(config);
    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    drawWithProgram(program);
    EXPECT_GL_NO_ERROR();
    glDeleteProgram(program);
}

// Test creating a surface that supports GL_RGB10_A2 with BT2020 colorspaces
TEST_P(EGLSurfaceTest, CreateWithEGLConfig1010102Support)
{
    const EGLint configAttributes[] = {EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT,
                                       EGL_RED_SIZE,
                                       10,
                                       EGL_GREEN_SIZE,
                                       10,
                                       EGL_BLUE_SIZE,
                                       10,
                                       EGL_ALPHA_SIZE,
                                       2,
                                       EGL_DEPTH_SIZE,
                                       0,
                                       EGL_STENCIL_SIZE,
                                       0,
                                       EGL_SAMPLE_BUFFERS,
                                       0,
                                       EGL_NONE};

    initializeDisplay();
    ASSERT_NE(mDisplay, EGL_NO_DISPLAY);

    if (EGLWindow::FindEGLConfig(mDisplay, configAttributes, &mConfig) == EGL_FALSE)
    {
        std::cout << "EGLConfig for a GL_RGB10_A2 surface is not supported, skipping test"
                  << std::endl;
        return;
    }

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_gl_colorspace_bt2020_hlg"));
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_gl_colorspace_bt2020_linear"));
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_gl_colorspace_bt2020_pq"));

    initializeMainContext();
    ASSERT_NE(mContext, EGL_NO_CONTEXT);

    constexpr std::array<EGLint, 3u> kBt2020Colorspaces = {EGL_GL_COLORSPACE_BT2020_HLG_EXT,
                                                           EGL_GL_COLORSPACE_BT2020_LINEAR_EXT,
                                                           EGL_GL_COLORSPACE_BT2020_PQ_EXT};
    for (EGLint bt2020Colorspace : kBt2020Colorspaces)
    {
        std::vector<EGLint> winSurfaceAttribs;
        winSurfaceAttribs.push_back(EGL_GL_COLORSPACE_KHR);
        winSurfaceAttribs.push_back(bt2020Colorspace);

        initializeWindowSurfaceWithAttribs(mConfig, winSurfaceAttribs, EGL_SUCCESS);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);

        EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext));
        ASSERT_EGL_SUCCESS();

        GLuint program = createProgram();
        ASSERT_NE(0u, program);
        drawWithProgram(program);
        EXPECT_GL_NO_ERROR();
        glDeleteProgram(program);

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(mDisplay, mWindowSurface);
        mWindowSurface = EGL_NO_SURFACE;
    }
}

TEST_P(EGLSurfaceTest, FixedSizeWindow)
{
    const EGLint configAttributes[] = {EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT,
                                       EGL_RED_SIZE,
                                       8,
                                       EGL_GREEN_SIZE,
                                       8,
                                       EGL_BLUE_SIZE,
                                       8,
                                       EGL_ALPHA_SIZE,
                                       0,
                                       EGL_DEPTH_SIZE,
                                       0,
                                       EGL_STENCIL_SIZE,
                                       0,
                                       EGL_SAMPLE_BUFFERS,
                                       0,
                                       EGL_NONE};

    initializeDisplay();
    ANGLE_SKIP_TEST_IF(EGLWindow::FindEGLConfig(mDisplay, configAttributes, &mConfig) == EGL_FALSE);

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_window_fixed_size"));

    constexpr EGLint kInitialSize = 64;
    constexpr EGLint kUpdateSize  = 32;

    EGLint surfaceAttributes[] = {
        EGL_FIXED_SIZE_ANGLE, EGL_TRUE, EGL_WIDTH, kInitialSize, EGL_HEIGHT, kInitialSize, EGL_NONE,
    };

    // Create first window surface
    mWindowSurface =
        eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), surfaceAttributes);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(EGL_NO_SURFACE, mWindowSurface);

    initializeMainContext();
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext));
    ASSERT_EGL_SUCCESS();

    EGLint queryIsFixedSize = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface(mDisplay, mWindowSurface, EGL_FIXED_SIZE_ANGLE, &queryIsFixedSize));
    ASSERT_EGL_SUCCESS();
    EXPECT_EGL_TRUE(queryIsFixedSize);

    EGLint queryWidth = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &queryWidth));
    ASSERT_EGL_SUCCESS();
    EXPECT_EQ(kInitialSize, queryWidth);

    EGLint queryHeight = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &queryHeight));
    ASSERT_EGL_SUCCESS();
    EXPECT_EQ(kInitialSize, queryHeight);

    // Update the size
    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, mWindowSurface, EGL_WIDTH, kUpdateSize));
    ASSERT_EGL_SUCCESS();

    EXPECT_EGL_TRUE(eglWaitNative(EGL_CORE_NATIVE_ENGINE));
    ASSERT_EGL_SUCCESS();

    EGLint queryUpdatedWidth = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &queryUpdatedWidth));
    ASSERT_EGL_SUCCESS();
    EXPECT_EQ(kUpdateSize, queryUpdatedWidth);
}

TEST_P(EGLSurfaceTest3, MakeCurrentDifferentSurfaces)
{
    const EGLint configAttributes[] = {
        EGL_RED_SIZE,   8, EGL_GREEN_SIZE,   8, EGL_BLUE_SIZE,      8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 0, EGL_SAMPLE_BUFFERS, 0, EGL_NONE};
    EGLSurface firstPbufferSurface;
    EGLSurface secondPbufferSurface;

    initializeDisplay();
    ANGLE_SKIP_TEST_IF(EGLWindow::FindEGLConfig(mDisplay, configAttributes, &mConfig) == EGL_FALSE);

    EGLint surfaceType = 0;
    eglGetConfigAttrib(mDisplay, mConfig, EGL_SURFACE_TYPE, &surfaceType);
    bool supportsPbuffers    = (surfaceType & EGL_PBUFFER_BIT) != 0;
    EGLint bindToTextureRGBA = 0;
    eglGetConfigAttrib(mDisplay, mConfig, EGL_BIND_TO_TEXTURE_RGBA, &bindToTextureRGBA);
    bool supportsBindTexImage = (bindToTextureRGBA == EGL_TRUE);

    const EGLint pBufferAttributes[] = {
        EGL_WIDTH,          64,
        EGL_HEIGHT,         64,
        EGL_TEXTURE_FORMAT, supportsPbuffers ? EGL_TEXTURE_RGBA : EGL_NO_TEXTURE,
        EGL_TEXTURE_TARGET, supportsBindTexImage ? EGL_TEXTURE_2D : EGL_NO_TEXTURE,
        EGL_NONE,           EGL_NONE,
    };

    // Create the surfaces
    firstPbufferSurface = eglCreatePbufferSurface(mDisplay, mConfig, pBufferAttributes);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(EGL_NO_SURFACE, firstPbufferSurface);
    secondPbufferSurface = eglCreatePbufferSurface(mDisplay, mConfig, pBufferAttributes);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(EGL_NO_SURFACE, secondPbufferSurface);

    initializeMainContext();

    // Use the same surface for both draw and read
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, firstPbufferSurface, firstPbufferSurface, mContext));

    // TODO(http://anglebug.com/42264803): Failing with OpenGL ES backend on Android.
    // Must be after the eglMakeCurrent() so the renderer string is initialized.
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && IsAndroid());

    glClearColor(kFloatRed.R, kFloatRed.G, kFloatRed.B, kFloatRed.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Use different surfaces for draw and read, read should stay the same
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, secondPbufferSurface, firstPbufferSurface, mContext));
    glClearColor(kFloatBlue.R, kFloatBlue.G, kFloatBlue.B, kFloatBlue.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    // Verify draw surface was cleared
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, secondPbufferSurface, secondPbufferSurface, mContext));
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, firstPbufferSurface, secondPbufferSurface, mContext));
    ASSERT_EGL_SUCCESS();

    // Blit the source surface to the destination surface
    glBlitFramebuffer(0, 0, 64, 64, 0, 0, 64, 64, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, firstPbufferSurface, firstPbufferSurface, mContext));
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

#if defined(ANGLE_ENABLE_D3D11)
class EGLSurfaceTestD3D11 : public EGLSurfaceTest
{
  protected:
    // offset - draw into the texture at offset (|offset|, |offset|)
    // pix25 - the expected pixel value at (25, 25)
    // pix75 - the expected pixel value at (75, 75)
    void testTextureOffset(int offset, UINT pix25, UINT pix75)
    {
        initializeDisplay();

        const EGLint configAttributes[] = {
            EGL_RED_SIZE,   8, EGL_GREEN_SIZE,   8, EGL_BLUE_SIZE,      8, EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 0, EGL_SAMPLE_BUFFERS, 0, EGL_NONE};

        EGLConfig config;
        ASSERT_EGL_TRUE(EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config));

        mConfig = config;
        initializeMainContext();

        EGLAttrib device       = 0;
        EGLAttrib newEglDevice = 0;
        ASSERT_EGL_TRUE(eglQueryDisplayAttribEXT(mDisplay, EGL_DEVICE_EXT, &newEglDevice));
        ASSERT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(newEglDevice),
                                                EGL_D3D11_DEVICE_ANGLE, &device));
        angle::ComPtr<ID3D11Device> d3d11Device(reinterpret_cast<ID3D11Device *>(device));
        ASSERT_TRUE(!!d3d11Device);

        constexpr UINT kTextureWidth  = 100;
        constexpr UINT kTextureHeight = 100;
        constexpr Color<uint8_t> kOpaqueBlack(0, 0, 0, 255);
        std::vector<Color<uint8_t>> textureData(kTextureWidth * kTextureHeight, kOpaqueBlack);

        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem                = textureData.data();
        initialData.SysMemPitch            = kTextureWidth * sizeof(kOpaqueBlack);

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Format               = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.Width                = kTextureWidth;
        desc.Height               = kTextureHeight;
        desc.ArraySize            = 1;
        desc.MipLevels            = 1;
        desc.SampleDesc.Count     = 1;
        desc.Usage                = D3D11_USAGE_DEFAULT;
        desc.BindFlags            = D3D11_BIND_RENDER_TARGET;
        angle::ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = d3d11Device->CreateTexture2D(&desc, &initialData, &texture);
        ASSERT_TRUE(SUCCEEDED(hr));

        angle::ComPtr<ID3D11DeviceContext> d3d11Context;
        d3d11Device->GetImmediateContext(&d3d11Context);

        // Specify a texture offset of (50, 50) when rendering to the pbuffer surface.
        const EGLint surfaceAttributes[] = {EGL_WIDTH,
                                            kTextureWidth,
                                            EGL_HEIGHT,
                                            kTextureHeight,
                                            EGL_TEXTURE_OFFSET_X_ANGLE,
                                            offset,
                                            EGL_TEXTURE_OFFSET_Y_ANGLE,
                                            offset,
                                            EGL_NONE};
        EGLClientBuffer buffer           = reinterpret_cast<EGLClientBuffer>(texture.Get());
        mPbufferSurface = eglCreatePbufferFromClientBuffer(mDisplay, EGL_D3D_TEXTURE_ANGLE, buffer,
                                                           config, surfaceAttributes);
        ASSERT_EGL_SUCCESS();

        eglMakeCurrent(mDisplay, mPbufferSurface, mPbufferSurface, mContext);
        ASSERT_EGL_SUCCESS();

        // glClear should only clear subrect at offset (50, 50) without explicit scissor.
        glClearColor(0, 0, 1, 1);  // Blue
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_EQ(25, 25, 0, 0, pix25, 255);
        EXPECT_PIXEL_EQ(75, 75, 0, 0, pix75, 255);
        EXPECT_GL_NO_ERROR();

        // Drawing with a shader should also update the same subrect only without explicit viewport.
        GLuint program = createProgram();  // Red
        ASSERT_NE(0u, program);
        GLint positionLocation =
            glGetAttribLocation(program, angle::essl1_shaders::PositionAttrib());
        glUseProgram(program);
        const GLfloat vertices[] = {
            -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,
            -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
        };
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(positionLocation);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        EXPECT_PIXEL_EQ(25, 25, pix25, 0, 0, 255);
        EXPECT_PIXEL_EQ(75, 75, pix75, 0, 0, 255);
        EXPECT_GL_NO_ERROR();

        glDeleteProgram(program);
        EXPECT_GL_NO_ERROR();

        // Blit framebuffer should also blit to the same subrect despite the dstX/Y arguments.
        GLRenderbuffer renderBuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 50, 50);
        EXPECT_GL_NO_ERROR();

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderBuffer);
        EXPECT_GL_NO_ERROR();

        glClearColor(0, 1, 0, 1);  // Green
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_EQ(25, 25, 0, 255, 0, 255);
        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0u);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBlitFramebuffer(0, 0, 50, 50, 0, 0, kTextureWidth, kTextureWidth, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0u);
        EXPECT_PIXEL_EQ(25, 25, 0, pix25, 0, 255);
        EXPECT_PIXEL_EQ(75, 75, 0, pix75, 0, 255);
        EXPECT_GL_NO_ERROR();
    }

    // Draws into a surface at the specified offset using the values of gl_FragCoord in the
    // fragment shader.
    // texturedimension - dimension of the D3D texture and surface.
    // offset - draw into the texture at offset (|offset|, |offset|)
    void setupFragCoordOffset(int textureDimension, int offset)
    {
        ANGLE_SKIP_TEST_IF(!IsEGLClientExtensionEnabled("EGL_ANGLE_platform_angle_d3d"));
        initializeDisplay();

        EGLAttrib device       = 0;
        EGLAttrib newEglDevice = 0;
        ASSERT_EGL_TRUE(eglQueryDisplayAttribEXT(mDisplay, EGL_DEVICE_EXT, &newEglDevice));
        ASSERT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(newEglDevice),
                                                EGL_D3D11_DEVICE_ANGLE, &device));
        angle::ComPtr<ID3D11Device> d3d11Device(reinterpret_cast<ID3D11Device *>(device));
        ASSERT_TRUE(!!d3d11Device);

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Format               = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.Width                = textureDimension;
        desc.Height               = textureDimension;
        desc.ArraySize            = 1;
        desc.MipLevels            = 1;
        desc.SampleDesc.Count     = 1;
        desc.Usage                = D3D11_USAGE_DEFAULT;
        desc.BindFlags            = D3D11_BIND_RENDER_TARGET;
        angle::ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = d3d11Device->CreateTexture2D(&desc, nullptr, &texture);
        ASSERT_TRUE(SUCCEEDED(hr));

        const EGLint surfaceAttributes[] = {EGL_WIDTH,
                                            textureDimension,
                                            EGL_HEIGHT,
                                            textureDimension,
                                            EGL_TEXTURE_OFFSET_X_ANGLE,
                                            offset,
                                            EGL_TEXTURE_OFFSET_Y_ANGLE,
                                            offset,
                                            EGL_NONE};
        EGLClientBuffer buffer           = reinterpret_cast<EGLClientBuffer>(texture.Get());

        const EGLint configAttributes[] = {
            EGL_RED_SIZE,   8, EGL_GREEN_SIZE,   8, EGL_BLUE_SIZE,      8, EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 0, EGL_SAMPLE_BUFFERS, 0, EGL_NONE};

        EGLConfig config;
        ASSERT_EGL_TRUE(EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config));
        mConfig = config;

        mPbufferSurface = eglCreatePbufferFromClientBuffer(mDisplay, EGL_D3D_TEXTURE_ANGLE, buffer,
                                                           config, surfaceAttributes);
        ASSERT_EGL_SUCCESS();

        initializeMainContext();

        eglMakeCurrent(mDisplay, mPbufferSurface, mPbufferSurface, mContext);
        ASSERT_EGL_SUCCESS();

        // Fragment shader that uses the gl_FragCoord values to output the (x, y) position of
        // the current pixel as the color.
        //    - Reverse the offset that was applied to the original coordinates
        //    - 0.5 is subtracted because gl_FragCoord gives the pixel center
        //    - Divided by the size to give a max value of 1
        std::stringstream fs;
        fs << "precision mediump float;" << "void main()" << "{" << "    float dimension = float("
           << textureDimension << ");" << "    float offset = float(" << offset << ");"
           << "    gl_FragColor = vec4((gl_FragCoord.x + offset - 0.5) / dimension,"
           << "                        (gl_FragCoord.y + offset - 0.5) / dimension,"
           << "                         gl_FragCoord.z,"
           << "                         gl_FragCoord.w);" << "}";

        GLuint program = createProgram(fs.str().c_str());
        ASSERT_NE(0u, program);
        glUseProgram(program);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        const GLfloat vertices[] = {
            -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,
            -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
        };

        GLint positionLocation =
            glGetAttribLocation(program, angle::essl1_shaders::PositionAttrib());
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(positionLocation);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(positionLocation);

        glDeleteProgram(program);

        EXPECT_GL_NO_ERROR();
    }
};

// Test that rendering to an IDCompositionSurface using a pbuffer works.
TEST_P(EGLSurfaceTestD3D11, CreateDirectCompositionSurface)
{
    ANGLE_SKIP_TEST_IF(!IsEGLClientExtensionEnabled("EGL_ANGLE_platform_angle_d3d"));
    initializeDisplay();

    EGLAttrib device       = 0;
    EGLAttrib newEglDevice = 0;
    ASSERT_EGL_TRUE(eglQueryDisplayAttribEXT(mDisplay, EGL_DEVICE_EXT, &newEglDevice));
    ASSERT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(newEglDevice),
                                            EGL_D3D11_DEVICE_ANGLE, &device));
    angle::ComPtr<ID3D11Device> d3d11Device(reinterpret_cast<ID3D11Device *>(device));
    ASSERT_TRUE(!!d3d11Device);

    HMODULE dcompLibrary = LoadLibraryA("dcomp.dll");
    if (!dcompLibrary)
    {
        std::cout << "DirectComposition not supported" << std::endl;
        return;
    }
    typedef HRESULT(WINAPI * PFN_DCOMPOSITION_CREATE_DEVICE2)(IUnknown * dxgiDevice, REFIID iid,
                                                              void **dcompositionDevice);
    PFN_DCOMPOSITION_CREATE_DEVICE2 createDComp = reinterpret_cast<PFN_DCOMPOSITION_CREATE_DEVICE2>(
        GetProcAddress(dcompLibrary, "DCompositionCreateDevice2"));
    if (!createDComp)
    {
        std::cout << "DirectComposition2 not supported" << std::endl;
        FreeLibrary(dcompLibrary);
        return;
    }

    angle::ComPtr<IDCompositionDevice> dcompDevice;
    HRESULT hr = createDComp(d3d11Device.Get(), IID_PPV_ARGS(dcompDevice.GetAddressOf()));
    ASSERT_TRUE(SUCCEEDED(hr));

    angle::ComPtr<IDCompositionSurface> dcompSurface;
    hr = dcompDevice->CreateSurface(100, 100, DXGI_FORMAT_B8G8R8A8_UNORM,
                                    DXGI_ALPHA_MODE_PREMULTIPLIED, dcompSurface.GetAddressOf());
    ASSERT_TRUE(SUCCEEDED(hr));

    angle::ComPtr<ID3D11Texture2D> texture;
    POINT updateOffset;
    hr = dcompSurface->BeginDraw(nullptr, IID_PPV_ARGS(texture.GetAddressOf()), &updateOffset);
    ASSERT_TRUE(SUCCEEDED(hr));

    const EGLint configAttributes[] = {
        EGL_RED_SIZE,   8, EGL_GREEN_SIZE,   8, EGL_BLUE_SIZE,      8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 0, EGL_SAMPLE_BUFFERS, 0, EGL_NONE};

    EGLConfig config;
    ASSERT_EGL_TRUE(EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config));

    const EGLint surfaceAttributes[] = {EGL_WIDTH,
                                        100,
                                        EGL_HEIGHT,
                                        100,
                                        EGL_TEXTURE_OFFSET_X_ANGLE,
                                        updateOffset.x,
                                        EGL_TEXTURE_OFFSET_Y_ANGLE,
                                        updateOffset.y,
                                        EGL_NONE};

    EGLClientBuffer buffer = reinterpret_cast<EGLClientBuffer>(texture.Get());
    mPbufferSurface = eglCreatePbufferFromClientBuffer(mDisplay, EGL_D3D_TEXTURE_ANGLE, buffer,
                                                       config, surfaceAttributes);
    ASSERT_EGL_SUCCESS();

    mConfig = config;
    initializeMainContext();

    eglMakeCurrent(mDisplay, mPbufferSurface, mPbufferSurface, mContext);
    ASSERT_EGL_SUCCESS();

    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    drawWithProgram(program);
    EXPECT_GL_NO_ERROR();
    glDeleteProgram(program);
}

// Tests drawing into a surface created with negative offsets.
TEST_P(EGLSurfaceTestD3D11, CreateSurfaceWithTextureNegativeOffset)
{
    ANGLE_SKIP_TEST_IF(!IsEGLClientExtensionEnabled("EGL_ANGLE_platform_angle_d3d"));
    testTextureOffset(-50, 255, 0);
}

// Tests drawing into a surface created with offsets.
TEST_P(EGLSurfaceTestD3D11, CreateSurfaceWithTextureOffset)
{
    ANGLE_SKIP_TEST_IF(!IsEGLClientExtensionEnabled("EGL_ANGLE_platform_angle_d3d"));
    testTextureOffset(50, 0, 255);
}

TEST_P(EGLSurfaceTestD3D11, CreateSurfaceWithMSAA)
{
    ANGLE_SKIP_TEST_IF(!IsEGLClientExtensionEnabled("EGL_ANGLE_platform_angle_d3d"));

    // clang-format off
    const EGLint configAttributes[] =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 0,
        EGL_STENCIL_SIZE, 0,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, 4,
        EGL_NONE
    };
    // clang-format on

    initializeDisplay();
    EGLConfig config;
    if (EGLWindow::FindEGLConfig(mDisplay, configAttributes, &config) == EGL_FALSE)
    {
        std::cout << "EGLConfig for 4xMSAA is not supported, skipping test" << std::endl;
        return;
    }

    initializeSurface(config);
    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    GLuint program = createProgram();
    ASSERT_NE(0u, program);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint positionLocation = glGetAttribLocation(program, angle::essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    glUseProgram(program);

    const GLfloat halfPixelOffset = 0.5f * 2.0f / mOSWindow->getWidth();
    // clang-format off
    const GLfloat vertices[] =
    {
        -1.0f + halfPixelOffset,  1.0f, 0.5f,
        -1.0f + halfPixelOffset, -1.0f, 0.5f,
         1.0f,                   -1.0f, 0.5f
    };
    // clang-format on

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    EXPECT_PIXEL_NEAR(0, 0, 127, 0, 0, 255, 10);
    EXPECT_GL_NO_ERROR();

    glDeleteProgram(program);
}

// Tests that gl_FragCoord.xy is offset with the EGL_TEXTURE_OFFSET_[X|Y]_ANGLE values specified
// at surface creation, using positive offsets
TEST_P(EGLSurfaceTestD3D11, FragCoordOffset)
{
    constexpr int kTextureDimension = 28;
    constexpr int kOffset           = 6;

    setupFragCoordOffset(kTextureDimension, kOffset);

    // With a positive offset, nothing is drawn in any pixels to the left of and above |kOffset|.
    for (int x = 0; x < kOffset; x++)
    {
        for (int y = 0; y < kOffset; y++)
        {
            EXPECT_PIXEL_EQ(x, y, 0, 0, 0, 0);
        }
    }

    // The rest of the texture's color should be the value of the (x, y) coordinates.
    for (int x = kOffset; x < kTextureDimension; x++)
    {
        for (int y = kOffset; y < kTextureDimension; y++)
        {
            EXPECT_PIXEL_NEAR(x, y, x * 255.0 / kTextureDimension, y * 255.0 / kTextureDimension,
                              191, 255, 0.5);
        }
    }
}

// Tests that gl_FragCoord.xy is offset with the EGL_TEXTURE_OFFSET_[X|Y]_ANGLE values specified
// at surface creation, using negative offsets.
TEST_P(EGLSurfaceTestD3D11, FragCoordOffsetNegative)
{
    constexpr int kTextureDimension = 28;
    constexpr int kOffset           = 6;

    setupFragCoordOffset(kTextureDimension, -kOffset);

    // With a negative offset, nothing is drawn in pixels to the right of and below |koffset|.
    for (int x = kTextureDimension - kOffset; x < kTextureDimension; x++)
    {
        for (int y = kTextureDimension - kOffset; y < kTextureDimension; y++)
        {
            EXPECT_PIXEL_EQ(x, y, 0, 0, 0, 0);
        }
    }

    // The rest of the texture's color should be the value of the (x, y) coordinates.
    for (int x = 0; x < kTextureDimension - kOffset; x++)
    {
        for (int y = 0; y < kTextureDimension - kOffset; y++)
        {
            EXPECT_PIXEL_NEAR(x, y, x * 255.0 / kTextureDimension, y * 255.0 / kTextureDimension,
                              191, 255, 0.5);
        }
    }
}

#endif  // ANGLE_ENABLE_D3D11

// Verify bliting between two surfaces works correctly.
TEST_P(EGLSurfaceTest3, BlitBetweenSurfaces)
{
    initializeDisplay();
    ASSERT_NE(mDisplay, EGL_NO_DISPLAY);

    initializeSurfaceWithDefaultConfig(true);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);
    ASSERT_NE(mContext, EGL_NO_CONTEXT);

    EGLSurface surface1;
    EGLSurface surface2;

    const EGLint surfaceAttributes[] = {
        EGL_WIDTH, 64, EGL_HEIGHT, 64, EGL_NONE,
    };

    surface1 = eglCreatePbufferSurface(mDisplay, mConfig, surfaceAttributes);
    ASSERT_EGL_SUCCESS();
    surface2 = eglCreatePbufferSurface(mDisplay, mConfig, surfaceAttributes);
    ASSERT_EGL_SUCCESS();

    // Clear surface1.
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface1, surface1, mContext));

    // TODO(http://anglebug.com/42264803): Failing with OpenGL ES backend on Android and
    // Windows. Must be after the eglMakeCurrent() so the renderer string is initialized.
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && (IsAndroid() || IsWindows()));

    glClearColor(kFloatRed.R, kFloatRed.G, kFloatRed.B, kFloatRed.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Blit from surface1 to surface2.
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface2, surface1, mContext));
    glBlitFramebuffer(0, 0, 64, 64, 0, 0, 64, 64, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Confirm surface1 has the clear color.
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface1, surface1, mContext));
    EXPECT_PIXEL_COLOR_EQ(32, 32, GLColor::red);

    // Confirm surface2 has the blited clear color.
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface2, surface2, mContext));
    EXPECT_PIXEL_COLOR_EQ(32, 32, GLColor::red);

    eglDestroySurface(mDisplay, surface1);
    eglDestroySurface(mDisplay, surface2);
}

// Verify bliting between two surfaces works correctly.
TEST_P(EGLSurfaceTest3, BlitBetweenSurfacesWithDeferredClear)
{
    initializeDisplay();
    ASSERT_NE(mDisplay, EGL_NO_DISPLAY);

    initializeSurfaceWithDefaultConfig(true);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);
    ASSERT_NE(mContext, EGL_NO_CONTEXT);

    EGLSurface surface1;
    EGLSurface surface2;

    const EGLint surfaceAttributes[] = {
        EGL_WIDTH, 64, EGL_HEIGHT, 64, EGL_NONE,
    };

    surface1 = eglCreatePbufferSurface(mDisplay, mConfig, surfaceAttributes);
    ASSERT_EGL_SUCCESS();
    surface2 = eglCreatePbufferSurface(mDisplay, mConfig, surfaceAttributes);
    ASSERT_EGL_SUCCESS();

    // Clear surface1.
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface1, surface1, mContext));

    // TODO(http://anglebug.com/42264803): Failing with OpenGL ES backend on Android and
    // Windows. Must be after the eglMakeCurrent() so the renderer string is initialized.
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && (IsAndroid() || IsWindows()));

    glClearColor(kFloatRed.R, kFloatRed.G, kFloatRed.B, kFloatRed.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    // Force the clear to be flushed
    EXPECT_PIXEL_COLOR_EQ(32, 32, GLColor::red);

    // Clear to green, but don't read it back so the clear is deferred.
    glClearColor(kFloatGreen.R, kFloatGreen.G, kFloatGreen.B, kFloatGreen.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Blit from surface1 to surface2.
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface2, surface1, mContext));
    glBlitFramebuffer(0, 0, 64, 64, 0, 0, 64, 64, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Confirm surface1 has the clear color.
    EXPECT_PIXEL_COLOR_EQ(32, 32, GLColor::green);

    // Confirm surface2 has the blited clear color.
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface2, surface2, mContext));
    EXPECT_PIXEL_COLOR_EQ(32, 32, GLColor::green);

    eglDestroySurface(mDisplay, surface1);
    eglDestroySurface(mDisplay, surface2);
}

// Verify switching between a surface with robust resource init and one without still clears alpha.
TEST_P(EGLSurfaceTest, RobustResourceInitAndEmulatedAlpha)
{
    // http://anglebug.com/42263827
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && isGLRenderer() && IsLinux());

    // http://anglebug.com/40644775
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsNexus5X() && isGLESRenderer());

    initializeDisplay();
    ASSERT_NE(mDisplay, EGL_NO_DISPLAY);

    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_robust_resource_initialization"));

    // Initialize and draw red to a Surface with robust resource init enabled.
    constexpr EGLint kRGBAAttributes[] = {EGL_RED_SIZE,     8,
                                          EGL_GREEN_SIZE,   8,
                                          EGL_BLUE_SIZE,    8,
                                          EGL_ALPHA_SIZE,   8,
                                          EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                          EGL_NONE};

    EGLint configCount   = 0;
    EGLConfig rgbaConfig = nullptr;
    ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, kRGBAAttributes, &rgbaConfig, 1, &configCount));
    ASSERT_EQ(configCount, 1);
    ASSERT_NE(rgbaConfig, nullptr);

    std::vector<EGLint> robustInitAttribs;
    robustInitAttribs.push_back(EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE);
    robustInitAttribs.push_back(EGL_TRUE);

    initializeSurfaceWithAttribs(rgbaConfig, robustInitAttribs);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);

    initializeMainContext();
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mContext, EGL_NO_CONTEXT);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();
    eglSwapBuffers(mDisplay, mWindowSurface);

    // RGBA robust init setup complete. Draw red and verify.
    {
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        glUseProgram(program);

        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        eglSwapBuffers(mDisplay, mWindowSurface);
    }

    tearDownContextAndSurface();

    // Create second RGB surface with robust resource disabled.
    constexpr EGLint kRGBAttributes[] = {EGL_RED_SIZE,     8,
                                         EGL_GREEN_SIZE,   8,
                                         EGL_BLUE_SIZE,    8,
                                         EGL_ALPHA_SIZE,   0,
                                         EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                         EGL_NONE};

    configCount         = 0;
    EGLConfig rgbConfig = nullptr;
    ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, kRGBAttributes, &rgbConfig, 1, &configCount));
    ASSERT_EQ(configCount, 1);
    ASSERT_NE(rgbConfig, nullptr);

    initializeSurface(rgbConfig);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);

    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // RGB non-robust init setup complete. Draw red and verify.
    {
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        glUseProgram(program);

        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        eglSwapBuffers(mDisplay, mWindowSurface);
    }
}

void EGLSurfaceTest::drawQuadThenTearDown()
{
    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    {
        ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
        eglSwapBuffers(mDisplay, mWindowSurface);
        ASSERT_EGL_SUCCESS();
    }

    tearDownContextAndSurface();
}

// Tests the EGL_ANGLE_create_surface_swap_interval extension if available.
TEST_P(EGLSurfaceTest, CreateSurfaceSwapIntervalANGLE)
{
    initializeDisplay();
    ASSERT_NE(mDisplay, EGL_NO_DISPLAY);

    mConfig = chooseDefaultConfig(true);
    ASSERT_NE(mConfig, nullptr);

    if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_create_surface_swap_interval"))
    {
        // Test error conditions.
        EGLint minSwapInterval = 0;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_MIN_SWAP_INTERVAL, &minSwapInterval);
        ASSERT_EGL_SUCCESS();

        if (minSwapInterval > 0)
        {
            std::vector<EGLint> min1SwapAttribs = {EGL_SWAP_INTERVAL_ANGLE, minSwapInterval - 1};
            initializeWindowSurfaceWithAttribs(mConfig, min1SwapAttribs, EGL_BAD_ATTRIBUTE);
        }

        EGLint maxSwapInterval = 0;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_MAX_SWAP_INTERVAL, &maxSwapInterval);
        ASSERT_EGL_SUCCESS();

        if (maxSwapInterval < std::numeric_limits<EGLint>::max())
        {
            std::vector<EGLint> max1SwapAttribs = {EGL_SWAP_INTERVAL_ANGLE, maxSwapInterval + 1};
            initializeWindowSurfaceWithAttribs(mConfig, max1SwapAttribs, EGL_BAD_ATTRIBUTE);
        }

        // Test valid min/max usage.
        {
            std::vector<EGLint> minSwapAttribs = {EGL_SWAP_INTERVAL_ANGLE, minSwapInterval};
            initializeWindowSurfaceWithAttribs(mConfig, minSwapAttribs, EGL_SUCCESS);
            drawQuadThenTearDown();
        }

        if (minSwapInterval != maxSwapInterval)
        {
            std::vector<EGLint> maxSwapAttribs = {EGL_SWAP_INTERVAL_ANGLE, maxSwapInterval};
            initializeWindowSurfaceWithAttribs(mConfig, maxSwapAttribs, EGL_SUCCESS);
            drawQuadThenTearDown();
        }
    }
    else
    {
        // Test extension unavailable error.
        std::vector<EGLint> swapInterval1Attribs = {EGL_SWAP_INTERVAL_ANGLE, 1};
        initializeWindowSurfaceWithAttribs(mConfig, swapInterval1Attribs, EGL_BAD_ATTRIBUTE);
    }
}

// Test that setting a surface's timestamp attribute works when the extension
// EGL_ANGLE_timestamp_surface_attribute is supported.
TEST_P(EGLSurfaceTest, TimestampSurfaceAttribute)
{
    initializeDisplay();
    ASSERT_NE(mDisplay, EGL_NO_DISPLAY);
    mConfig = chooseDefaultConfig(true);
    ASSERT_NE(mConfig, nullptr);
    initializeSurface(mConfig);
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);
    initializeMainContext();

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    const bool extensionSupported =
        IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_get_frame_timestamps") ||
        IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_timestamp_surface_attribute");

    EGLBoolean setSurfaceAttrib =
        eglSurfaceAttrib(mDisplay, mWindowSurface, EGL_TIMESTAMPS_ANDROID, EGL_TRUE);

    if (extensionSupported)
    {
        EXPECT_EGL_TRUE(setSurfaceAttrib);

        // Swap so the swapchain gets created.
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_EGL_TRUE(eglSwapBuffers(mDisplay, mWindowSurface));

        // Query to confirm the attribute persists across swaps.
        EGLint timestampEnabled = 0;
        EXPECT_EGL_TRUE(
            eglQuerySurface(mDisplay, mWindowSurface, EGL_TIMESTAMPS_ANDROID, &timestampEnabled));
        EXPECT_NE(timestampEnabled, 0);

        // Resize window and swap.
        mOSWindow->resize(256, 256);
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_EGL_TRUE(eglSwapBuffers(mDisplay, mWindowSurface));

        // Query to confirm the attribute persists across swapchain recreations.
        timestampEnabled = 0;
        EXPECT_EGL_TRUE(
            eglQuerySurface(mDisplay, mWindowSurface, EGL_TIMESTAMPS_ANDROID, &timestampEnabled));
        EXPECT_NE(timestampEnabled, 0);
    }
    else
    {
        EXPECT_EGL_FALSE(setSurfaceAttrib);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";
}

TEST_P(EGLSingleBufferTest, OnCreateWindowSurface)
{
    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_SINGLE_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EGLint actualRenderbuffer;
    EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_RENDER_BUFFER, &actualRenderbuffer));
    if (actualRenderbuffer == EGL_SINGLE_BUFFER)
    {
        EXPECT_EGL_TRUE(actualRenderbuffer == EGL_SINGLE_BUFFER);

        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
        ASSERT_GL_NO_ERROR();
        // Flush should result in update of screen. Must be visually confirmed.
        // Pixel test for automation.
        EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
    }
    else
    {
        std::cout << "SKIP test, no EGL_SINGLE_BUFFER support." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

TEST_P(EGLSingleBufferTest, OnSetSurfaceAttrib)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));

    // Transition into EGL_SINGLE_BUFFER mode.
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (eglSwapBuffers(mDisplay, surface))
    {
        EGLint actualRenderbuffer;
        EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_RENDER_BUFFER, &actualRenderbuffer));
        EXPECT_EGL_TRUE(actualRenderbuffer == EGL_SINGLE_BUFFER);

        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
        // Flush should result in update of screen. Must be visually confirmed Green window.

        // Check color for automation.
        EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);

        // Switch back to EGL_BACK_BUFFEr and check.
        EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_BACK_BUFFER));
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_EGL_TRUE(eglSwapBuffers(mDisplay, surface));

        EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_RENDER_BUFFER, &actualRenderbuffer));
        EXPECT_EGL_TRUE(actualRenderbuffer == EGL_BACK_BUFFER);

        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    }
    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

uint32_t EGLSingleBufferTest::drawAndSwap(EGLSurface &surface,
                                          EGLDisplay &display,
                                          uint32_t color,
                                          bool flush)
{
    ASSERT(color < 256);

    glClearColor((float)color / 255.f, (float)color / 255.f, (float)color / 255.f,
                 (float)color / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (flush)
    {
        glFlush();
    }
    else
    {
        eglSwapBuffers(display, surface);
    }

    return (color | color << 8 | color << 16 | color << 24);
}

// Replicate dEQP-EGL.functional.mutable_render_buffer#basic
TEST_P(EGLSingleBufferTest, MutableRenderBuffer)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config       = EGL_NO_CONFIG_KHR;
    const EGLint attribs[] = {EGL_RED_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_ALPHA_SIZE,
                              8,
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT | EGL_MUTABLE_RENDER_BUFFER_BIT_KHR,
                              EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES2_BIT,
                              EGL_NONE};
    EGLint count           = 0;
    ANGLE_SKIP_TEST_IF(!eglChooseConfig(mDisplay, attribs, &config, 1, &count));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    int frameNumber = 1;

    // run a few back-buffered frames
    for (; frameNumber < 5; frameNumber++)
    {
        drawAndSwap(surface, mDisplay, frameNumber, false);
    }

    if (eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER))
    {
        drawAndSwap(surface, mDisplay, frameNumber, false);
        frameNumber++;

        // test a few single-buffered frames
        for (; frameNumber < 10; frameNumber++)
        {
            uint32_t backBufferPixel  = 0xFFFFFFFF;
            uint32_t frontBufferPixel = drawAndSwap(surface, mDisplay, frameNumber, true);
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &backBufferPixel);
            EXPECT_EQ(backBufferPixel, frontBufferPixel);
        }
    }

    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    // switch back to back-buffer rendering
    if (eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_BACK_BUFFER))
    {
        for (; frameNumber < 14; frameNumber++)
        {
            drawAndSwap(surface, mDisplay, frameNumber, false);
        }
    }
    else
    {
        std::cout << "EGL_BACK_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Tests bug with incorrect ImageLayout::SharedPresent barrier.
TEST_P(EGLSingleBufferTest, SharedPresentBarrier)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));

    // Transition into EGL_SINGLE_BUFFER mode.
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (eglSwapBuffers(mDisplay, surface))
    {
        EGLint actualRenderbuffer;
        EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_RENDER_BUFFER, &actualRenderbuffer));
        EXPECT_EGL_TRUE(actualRenderbuffer == EGL_SINGLE_BUFFER);

        for (int i = 0; i < 5; ++i)
        {
            GLColor testColor(rand() % 256, rand() % 256, rand() % 256, 255);
            angle::Vector4 clearColor = testColor.toNormalizedVector();
            glClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
            glClear(GL_COLOR_BUFFER_BIT);
            // Skip flush because present operations may add other barriers that will make appear
            // that everything works as expected.

            // Check color without flush - may get invalid result if have incorrect barrier bug.
            EXPECT_PIXEL_COLOR_EQ(1, 1, testColor);
        }
    }
    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Tests scissored clear on single buffer surface
TEST_P(EGLSingleBufferTest, ScissoredClear)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));
    if (eglSwapBuffers(mDisplay, surface))
    {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();

        glEnable(GL_SCISSOR_TEST);
        glScissor(1, 1, 10, 10);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(2, 2, GLColor::green);
    }
    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Tests scissored clear on single buffer surface
TEST_P(EGLSingleBufferTest, ScissoredDraw)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));
    if (eglSwapBuffers(mDisplay, surface))
    {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();

        glEnable(GL_SCISSOR_TEST);
        glScissor(1, 1, 10, 10);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);
        glFlush();
        glDisable(GL_SCISSOR_TEST);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(2, 2, GLColor::green);
    }
    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Tests that "one off" submission is waited before destroying the surface.
TEST_P(EGLSingleBufferTest, WaitOneOffSubmission)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLContext context2 = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context2));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    const EGLint pbufferSurfaceAttrs[] = {
        EGL_WIDTH, 1024, EGL_HEIGHT, 1024, EGL_NONE,
    };
    EGLSurface pbufferSurface = eglCreatePbufferSurface(mDisplay, config, pbufferSurfaceAttrs);
    ASSERT_EGL_SUCCESS() << "eglCreatePbufferSurface failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    // Query age for the first time to avoid submitting debug information a second time.
    EGLint age = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, surface, EGL_BUFFER_AGE_EXT, &age));

    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));
    // Transition into EGL_SINGLE_BUFFER mode.
    if (eglSwapBuffers(mDisplay, surface))
    {
        // Submit heavy work to the GPU before querying the buffer age.
        std::thread([this, context2, pbufferSurface]() {
            EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, pbufferSurface, pbufferSurface, context2));
            ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

            ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
            drawQuadInstanced(greenProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, false,
                              1000);

            EXPECT_EGL_TRUE(
                eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
            ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";
        }).join();

        // Querying the buffer age should perform first acquire of the image and "one off"
        // submission to change image layout to the VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR.
        EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, surface, EGL_BUFFER_AGE_EXT, &age));
    }
    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroySurface(mDisplay, pbufferSurface);
    pbufferSurface = EGL_NO_SURFACE;

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;

    eglDestroyContext(mDisplay, context2);
    context2 = EGL_NO_CONTEXT;
}

// Checks that |WindowSurfaceVk::swamImpl| acquires and process next swapchain image in case of
// shared present mode, when called from flush.
TEST_P(EGLSingleBufferTest, AcquireImageFromSwapImpl)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));
    if (eglSwapBuffers(mDisplay, surface))
    {
        ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

        // Draw into the single buffered surface.
        // Acquire next swapchain image should be deferred (Vulkan back-end).
        drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);
        glFlush();

        // Prepare auxilary framebuffer.
        GLRenderbuffer renderBuffer;
        GLFramebuffer framebuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 50, 50);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderBuffer);
        EXPECT_GL_NO_ERROR();

        // Draw into the auxilary framebuffer just to generate commands into the command buffers.
        // Otherwise below flush will be ignored.
        drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);

        // Switch back to the Windows Surface and perform flush.
        // In Vulkan back-end flush will translate into |swapImpl| call while acquire next swapchain
        // image is still deferred. |swapImpl| must perform the acquire in that case, otherwise
        // ASSERT will trigger in |present|.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glFlush();
    }
    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Test that setting a surface to EGL_SINGLE_BUFFER after enabling
// EGL_FRONT_BUFFER_AUTO_REFRESH_ANDROID does not disable auto refresh
TEST_P(EGLAndroidAutoRefreshTest, Basic)
{
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_front_buffer_auto_refresh"));
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));
    ANGLE_SKIP_TEST_IF(!IsAndroid());

    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_BACK_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EXPECT_EGL_TRUE(
        eglSurfaceAttrib(mDisplay, surface, EGL_FRONT_BUFFER_AUTO_REFRESH_ANDROID, EGL_TRUE));

    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));

    // Transition into EGL_SINGLE_BUFFER mode.
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (eglSwapBuffers(mDisplay, surface))
    {
        EGLint actualRenderbuffer;
        EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_RENDER_BUFFER, &actualRenderbuffer));
        EXPECT_EGL_TRUE(actualRenderbuffer == EGL_SINGLE_BUFFER);

        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
        // Flush should result in update of screen. Must be visually confirmed Green window.

        // Check color for automation.
        EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);

        // Switch back to EGL_BACK_BUFFER and check.
        EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_BACK_BUFFER));
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_EGL_TRUE(eglSwapBuffers(mDisplay, surface));

        EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_RENDER_BUFFER, &actualRenderbuffer));
        EXPECT_EGL_TRUE(actualRenderbuffer == EGL_BACK_BUFFER);

        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    }
    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Tests that CPU throttling unlocked call, added in the implicit swap buffers call, is executed.
TEST_P(EGLAndroidAutoRefreshTest, SwapCPUThrottling)
{
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_front_buffer_auto_refresh"));
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));
    ANGLE_SKIP_TEST_IF(!IsAndroid());

    // Use high resolution to increase GPU load.
    const EGLint kWidth  = 2048;
    const EGLint kHeight = 2048;

    // These settings are expected to trigger CPU throttling in present.
    constexpr size_t kFrameFlushCount   = 8;
    constexpr GLuint kDrawInstanceCount = 256;

    EGLConfig config = EGL_NO_CONFIG_KHR;
    ANGLE_SKIP_TEST_IF(!chooseConfig(&config, true));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_EGL_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;
    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLSingleBufferTest", kWidth, kHeight);
    EXPECT_EGL_TRUE(
        createWindowSurface(config, osWindow->getNativeWindow(), &surface, EGL_SINGLE_BUFFER));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EGLint actualRenderbuffer;
    EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_RENDER_BUFFER, &actualRenderbuffer));
    if (actualRenderbuffer == EGL_SINGLE_BUFFER)
    {
        // Enable auto refresh to prevent present from waiting on GPU.
        EXPECT_EGL_TRUE(
            eglSurfaceAttrib(mDisplay, surface, EGL_FRONT_BUFFER_AUTO_REFRESH_ANDROID, EGL_TRUE));

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        glViewport(0, 0, kWidth, kHeight);

        for (size_t i = 0; i < kFrameFlushCount; ++i)
        {
            // Perform heavy draw call to load GPU.
            drawQuadInstanced(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, false,
                              kDrawInstanceCount);
            // This should cause implicit swap and possible CPU throttling in the tail call.
            glFlush();
        }

        // Tests same as the glFlush above.
        drawQuadInstanced(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, false,
                          kDrawInstanceCount);
        glFinish();
    }
    else
    {
        std::cout << "EGL_SINGLE_BUFFER mode is not supported." << std::endl;
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

void EGLSurfaceTest::runWaitSemaphoreTest(bool useSecondContext)
{
    // Note: This test requires visual inspection for rendering artifacts.
    // However, absence of artifacts does not guarantee that there is no problem.

    initializeDisplay();

    constexpr int kInitialSize   = 64;
    constexpr int kWindowWidth   = 1080;
    constexpr int kWindowWHeight = 1920;

    mOSWindow->resize(kWindowWidth, kWindowWHeight);

    // Initialize an RGBA8 window and pbuffer surface
    constexpr EGLint kSurfaceAttributes[] = {EGL_RED_SIZE,     8,
                                             EGL_GREEN_SIZE,   8,
                                             EGL_BLUE_SIZE,    8,
                                             EGL_ALPHA_SIZE,   8,
                                             EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                             EGL_NONE};

    EGLint configCount      = 0;
    EGLConfig surfaceConfig = nullptr;
    ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, kSurfaceAttributes, &surfaceConfig, 1, &configCount));
    ASSERT_NE(configCount, 0);
    ASSERT_NE(surfaceConfig, nullptr);

    initializeSurface(surfaceConfig);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);
    ASSERT_NE(mPbufferSurface, EGL_NO_SURFACE);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    if (useSecondContext)
    {
        ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
        initializeSingleContext(&mSecondContext, 0);
    }

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    GLint posAttrib = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(posAttrib, -1);
    glEnableVertexAttribArray(posAttrib);
    ASSERT_GL_NO_ERROR();

    GLint colorUniformLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    constexpr int kFrameCount = 60 * 4;  // 4 sec @ 60Hz; 2 sec @ 120Hz;
    constexpr int kGridW      = 5;
    constexpr int kGridH      = 5;
    constexpr int kAnimDiv    = 20;

    for (int frame = 0; frame < kFrameCount; ++frame)
    {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        for (int y = 0; y < kGridH; ++y)
        {
            // This should force "flushToPrimary()" each line in ANGLE
            GLuint query;
            glGenQueries(1, &query);
            ASSERT_GL_NO_ERROR();
            glBeginQuery(GL_TIME_ELAPSED_EXT, query);
            ASSERT_GL_NO_ERROR();

            for (int x = 0; x < kGridW; ++x)
            {
                const int xc        = (x + frame / kAnimDiv) % kGridW;
                const Vector4 color = {(xc + 0.5f) / kGridW, (y + 0.5f) / kGridH, 0.0f, 1.0f};

                const GLfloat x0 = (x + 0.1f) / kGridW * 2.0f - 1.0f;
                const GLfloat x1 = (x + 0.9f) / kGridW * 2.0f - 1.0f;
                const GLfloat y0 = (y + 0.1f) / kGridH * 2.0f - 1.0f;
                const GLfloat y1 = (y + 0.9f) / kGridH * 2.0f - 1.0f;

                std::array<Vector3, 6> vertexData;
                vertexData[0] = {x0, y1, 0.5f};
                vertexData[1] = {x0, y0, 0.5f};
                vertexData[2] = {x1, y1, 0.5f};
                vertexData[3] = {x1, y1, 0.5f};
                vertexData[4] = {x0, y0, 0.5f};
                vertexData[5] = {x1, y0, 0.5f};

                glUniform4f(colorUniformLocation, color.x(), color.y(), color.z(), color.w());
                glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, vertexData.data());
                glDrawArrays(GL_TRIANGLES, 0, 6);
                ASSERT_GL_NO_ERROR();
            }

            glEndQuery(GL_TIME_ELAPSED_EXT);
            glDeleteQueries(1, &query);
            ASSERT_GL_NO_ERROR();
        }

        if (useSecondContext)
        {
            std::thread([this] {
                eglBindAPI(EGL_OPENGL_ES_API);
                ASSERT_EGL_SUCCESS();
                eglMakeCurrent(mDisplay, mPbufferSurface, mPbufferSurface, mSecondContext);
                ASSERT_EGL_SUCCESS();
                glEnable(GL_SCISSOR_TEST);
                glScissor(0, 0, 1, 1);
                glClear(GL_COLOR_BUFFER_BIT);
                ASSERT_GL_NO_ERROR();
                eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                ASSERT_EGL_SUCCESS();
            }).join();
        }
        else
        {
            eglMakeCurrent(mDisplay, mPbufferSurface, mPbufferSurface, mContext);
            ASSERT_EGL_SUCCESS();
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            ASSERT_EGL_SUCCESS();
            eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
            ASSERT_EGL_SUCCESS();
        }

        eglSwapBuffers(mDisplay, mWindowSurface);
        ASSERT_EGL_SUCCESS();
    }

    mOSWindow->resize(kInitialSize, kInitialSize);
}

// Test that there no artifacts because of the bug when wait semaphore could be added after
// rendering commands. This was possible by switching to Pbuffer surface and submit.
TEST_P(EGLSurfaceTest, DISABLED_WaitSemaphoreAddedAfterCommands)
{
    runWaitSemaphoreTest(false);
}

// Test that there no artifacts because of the bug when rendering commands could be submitted
// without adding wait semaphore. This was possible if submit commands from other thread.
TEST_P(EGLSurfaceTest, DISABLED_CommandsSubmittedWithoutWaitSemaphore)
{
    runWaitSemaphoreTest(true);
}

void EGLSurfaceTest::runDestroyNotCurrentSurfaceTest(bool testWindowsSurface)
{
    initializeDisplay();

    // Initialize an RGBA8 window and pbuffer surface
    constexpr EGLint kSurfaceAttributes[] = {EGL_RED_SIZE,     8,
                                             EGL_GREEN_SIZE,   8,
                                             EGL_BLUE_SIZE,    8,
                                             EGL_ALPHA_SIZE,   8,
                                             EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                             EGL_NONE};

    EGLint configCount      = 0;
    EGLConfig surfaceConfig = nullptr;
    ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, kSurfaceAttributes, &surfaceConfig, 1, &configCount));
    ASSERT_NE(configCount, 0);
    ASSERT_NE(surfaceConfig, nullptr);

    initializeSurface(surfaceConfig);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);
    ASSERT_NE(mPbufferSurface, EGL_NO_SURFACE);

    EGLSurface &testSurface  = testWindowsSurface ? mWindowSurface : mPbufferSurface;
    EGLSurface &otherSurface = testWindowsSurface ? mPbufferSurface : mWindowSurface;

    eglMakeCurrent(mDisplay, testSurface, testSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Start RenderPass in the testSurface
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 4, 4);
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();

    // Make other surface current keeping the context.
    // If bug present, the context may have unflushed work, related to the testSurface.
    eglMakeCurrent(mDisplay, otherSurface, otherSurface, mContext);
    ASSERT_EGL_SUCCESS();

    if (testWindowsSurface)
    {
        // This may flush Window Surface RenderPass
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, 4, 4);
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
        ASSERT_GL_NO_ERROR();
    }

    // Destroy the surface
    eglDestroySurface(mDisplay, testSurface);
    testSurface = EGL_NO_SURFACE;

    // This will submit all work (if bug present - include work related to the deleted testSurface).
    eglMakeCurrent(mDisplay, otherSurface, otherSurface, mContext);
    ASSERT_EGL_SUCCESS();
}

// Test that there is no crash because of the bug when not current PBuffer Surface destroyed, while
// there are still unflushed work in the Context.
TEST_P(EGLSurfaceTest, DestroyNotCurrentPbufferSurface)
{
    runDestroyNotCurrentSurfaceTest(false);
}

// Test that there is no crash because of the bug when not current Window Surface destroyed, while
// there are still unflushed work in the Context.
TEST_P(EGLSurfaceTest, DestroyNotCurrentWindowSurface)
{
    runDestroyNotCurrentSurfaceTest(true);
}

// Test that there is no tearing because of incorrect pipeline barriers
TEST_P(EGLSurfaceTest, DISABLED_RandomClearTearing)
{
    // Note: This test requires visual inspection for rendering artifacts.
    // However, absence of artifacts does not guarantee that there is no problem.

    initializeDisplay();

    constexpr int kInitialSize   = 64;
    constexpr int kWindowWidth   = 1080;
    constexpr int kWindowWHeight = 1920;

    mOSWindow->resize(kWindowWidth, kWindowWHeight);

    initializeSurfaceWithDefaultConfig(true);
    initializeMainContext();
    ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    constexpr int kFrameCount = 60 * 4;  // 4 sec @ 60Hz; 2 sec @ 120Hz;

    for (int frame = 0; frame < kFrameCount; ++frame)
    {
        glClearColor(rand() % 256 / 255.0f, rand() % 256 / 255.0f, rand() % 256 / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        eglSwapBuffers(mDisplay, mWindowSurface);
        ASSERT_EGL_SUCCESS();
    }

    mOSWindow->resize(kInitialSize, kInitialSize);
}

// Make sure a surface (from the same window) can be recreated after being destroyed, even if it's
// still current.
// This is to recreate the app behavior in https://issuetracker.google.com/292285899, which is
// not the correct spec behavior. It serves as a purpose to test the workaround feature
// uncurrent_egl_surface_upon_surface_destroy that is enabled only on vulkan backend to help
// the app get over the problem.
TEST_P(EGLSurfaceTest, DestroyAndRecreateWhileCurrent)
{
    setWindowVisible(mOSWindow, true);

    initializeDisplay();

    mConfig = chooseDefaultConfig(true);
    ASSERT_NE(mConfig, nullptr);

    EGLint surfaceType = EGL_NONE;
    eglGetConfigAttrib(mDisplay, mConfig, EGL_SURFACE_TYPE, &surfaceType);
    ASSERT_NE((surfaceType & EGL_WINDOW_BIT), 0);

    initializeWindowSurfaceWithAttribs(mConfig, {}, EGL_SUCCESS);
    initializeMainContext();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Draw with this surface to make sure it's used.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glViewport(0, 0, 64, 64);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Destroy the surface while it's current; it won't actually be destroyed.
    eglDestroySurface(mDisplay, mWindowSurface);
    mWindowSurface = EGL_NO_SURFACE;

    // Create another surface from the same window right away.
    initializeWindowSurfaceWithAttribs(mConfig, {}, EGL_SUCCESS);

    // Make the new surface current; this leads to the actual destruction of the previous surface.
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext));
    ASSERT_EGL_SUCCESS();

    // Verify everything still works
    ANGLE_GL_PROGRAM(program2, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program2, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Regression test for a bug where destroying more than 2 surfaces during termination
// overflowed the unlocked tail call storage.
TEST_P(EGLSurfaceTest, CreateMultiWindowsSurfaceNoDestroy)
{
    initializeDisplay();

    // Initialize and create multi RGBA8 window surfaces
    constexpr EGLint kSurfaceAttributes[] = {EGL_RED_SIZE,     8,
                                             EGL_GREEN_SIZE,   8,
                                             EGL_BLUE_SIZE,    8,
                                             EGL_ALPHA_SIZE,   8,
                                             EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                             EGL_NONE};

    EGLint configCount      = 0;
    EGLConfig surfaceConfig = nullptr;
    ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, kSurfaceAttributes, &surfaceConfig, 1, &configCount));
    ASSERT_NE(configCount, 0);
    ASSERT_NE(surfaceConfig, nullptr);

    initializeSurface(surfaceConfig);

    // Create 3 window surfaces to trigger error
    std::vector<EGLint> windowAttributes;
    windowAttributes.push_back(EGL_NONE);

    for (int i = 0; i < 3; i++)
    {
        OSWindow *w = OSWindow::New();
        w->initialize("EGLSurfaceTest", 64, 64);

        eglCreateWindowSurface(mDisplay, mConfig, w->getNativeWindow(), windowAttributes.data());
        ASSERT_EGL_SUCCESS();
        mOtherWindows.push_back(w);
    }
}

// Test that querying EGL_RENDER_BUFFER of surface and context returns correct value.
// Context's render buffer should only change once eglSwapBuffers is called.
TEST_P(EGLSurfaceTest, QueryRenderBuffer)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));
    ANGLE_SKIP_TEST_IF(!IsAndroid());

    const EGLint configAttributes[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE,
                                       EGL_WINDOW_BIT | EGL_MUTABLE_RENDER_BUFFER_BIT_KHR,
                                       EGL_NONE};

    initializeDisplay();
    ANGLE_SKIP_TEST_IF(EGLWindow::FindEGLConfig(mDisplay, configAttributes, &mConfig) == EGL_FALSE);

    // Create window surface and make current
    mWindowSurface =
        eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), nullptr);
    ASSERT_EGL_SUCCESS();
    ASSERT_NE(EGL_NO_SURFACE, mWindowSurface);

    initializeMainContext();
    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext));
    ASSERT_EGL_SUCCESS();

    // Set to single buffer mode and query the value
    ASSERT_EGL_TRUE(
        eglSurfaceAttrib(mDisplay, mWindowSurface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));

    EGLint queryRenderBuffer;
    ASSERT_EGL_TRUE(
        eglQuerySurface(mDisplay, mWindowSurface, EGL_RENDER_BUFFER, &queryRenderBuffer));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(queryRenderBuffer, EGL_SINGLE_BUFFER);

    ASSERT_EGL_TRUE(eglQueryContext(mDisplay, mContext, EGL_RENDER_BUFFER, &queryRenderBuffer));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(queryRenderBuffer, EGL_BACK_BUFFER);

    // Swap buffers and then query the value
    ASSERT_EGL_TRUE(eglSwapBuffers(mDisplay, mWindowSurface));
    ASSERT_EGL_SUCCESS();

    ASSERT_EGL_TRUE(
        eglQuerySurface(mDisplay, mWindowSurface, EGL_RENDER_BUFFER, &queryRenderBuffer));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(queryRenderBuffer, EGL_SINGLE_BUFFER);

    ASSERT_EGL_TRUE(eglQueryContext(mDisplay, mContext, EGL_RENDER_BUFFER, &queryRenderBuffer));
    ASSERT_EGL_SUCCESS();
    ASSERT_EQ(queryRenderBuffer, EGL_SINGLE_BUFFER);

    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    ASSERT_EGL_TRUE(eglDestroySurface(mDisplay, mWindowSurface));
    mWindowSurface = EGL_NO_SURFACE;
    ASSERT_EGL_TRUE(eglDestroyContext(mDisplay, mContext));
    mContext = EGL_NO_CONTEXT;
    ASSERT_EGL_SUCCESS();
}

// Test that new API eglQuerySupportedCompressionRatesEXT could work, and
// validation for the API should also work. If any rate can be queried, then use
// that rate to create window surface. Query the surface's compression rate
// should get the expected rate, and a simple draw should succeed on the surface.
TEST_P(EGLSurfaceTest, SurfaceFixedRateCompression)
{
    initializeDisplay();
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_surface_compression"));
    // Initialize an RGBA8 window and pbuffer surface
    constexpr EGLint kSurfaceAttributes[] = {EGL_RED_SIZE,
                                             8,
                                             EGL_GREEN_SIZE,
                                             8,
                                             EGL_BLUE_SIZE,
                                             8,
                                             EGL_ALPHA_SIZE,
                                             8,
                                             EGL_SURFACE_TYPE,
                                             EGL_WINDOW_BIT,
                                             EGL_RENDERABLE_TYPE,
                                             EGL_OPENGL_ES2_BIT,
                                             EGL_NONE};
    EGLint configCount                    = 0;
    EGLint numRates                       = 0;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, kSurfaceAttributes, &mConfig, 1, &configCount));
    ASSERT_NE(configCount, 0);
    ASSERT_NE(mConfig, nullptr);
    // Fail, invalid display
    EXPECT_EGL_FALSE(eglQuerySupportedCompressionRatesEXT(EGL_NO_DISPLAY, mConfig, nullptr, nullptr,
                                                          0, &numRates));
    ASSERT_EGL_ERROR(EGL_BAD_DISPLAY);
    // Fail, rate_size < 0
    EXPECT_EGL_FALSE(
        eglQuerySupportedCompressionRatesEXT(mDisplay, mConfig, nullptr, nullptr, -1, &numRates));
    ASSERT_EGL_ERROR(EGL_BAD_PARAMETER);
    // Fail, pointer rates is nullptr
    EXPECT_EGL_FALSE(
        eglQuerySupportedCompressionRatesEXT(mDisplay, mConfig, nullptr, nullptr, 1, &numRates));
    ASSERT_EGL_ERROR(EGL_BAD_PARAMETER);
    // Fail, return num_rates is nullptr
    EXPECT_EGL_FALSE(
        eglQuerySupportedCompressionRatesEXT(mDisplay, mConfig, nullptr, nullptr, 0, nullptr));
    ASSERT_EGL_ERROR(EGL_BAD_PARAMETER);
    EGLint rates[3];
    // Success, actual values of rates are depended on each platform
    EXPECT_EGL_TRUE(
        eglQuerySupportedCompressionRatesEXT(mDisplay, mConfig, NULL, rates, 3, &numRates));
    ASSERT_EGL_SUCCESS();

    if (numRates > 0 && rates[0] != EGL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT)
    {
        // If any rate can be queried, then use that rate to create window surface and test
        std::vector<EGLint> winSurfaceAttribs;
        winSurfaceAttribs.push_back(EGL_SURFACE_COMPRESSION_EXT);
        winSurfaceAttribs.push_back(rates[0]);
        // Create window surface using the selected rate.
        initializeWindowSurfaceWithAttribs(mConfig, winSurfaceAttribs, EGL_SUCCESS);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);
        EGLint selectedRate;
        ASSERT_EGL_TRUE(
            eglQuerySurface(mDisplay, mWindowSurface, EGL_SURFACE_COMPRESSION_EXT, &selectedRate));
        ASSERT_EGL_SUCCESS();
        ASSERT_EQ(selectedRate, rates[0]);
        initializeMainContext();
        EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext));
        ASSERT_EGL_SUCCESS();
        // Make sure the surface works. Draw red and verify.
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        glUseProgram(program);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        ASSERT_EGL_TRUE(eglSwapBuffers(mDisplay, mWindowSurface));

        EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, nullptr, nullptr, mContext));
        EXPECT_EGL_TRUE(eglDestroySurface(mDisplay, mWindowSurface));
        mWindowSurface = EGL_NO_SURFACE;

        // Create another surface using default rate.
        winSurfaceAttribs.back() = EGL_SURFACE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;
        initializeWindowSurfaceWithAttribs(mConfig, winSurfaceAttribs, EGL_SUCCESS);
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(mWindowSurface, EGL_NO_SURFACE);
        selectedRate = EGL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT;
        ASSERT_EGL_TRUE(
            eglQuerySurface(mDisplay, mWindowSurface, EGL_SURFACE_COMPRESSION_EXT, &selectedRate));
        ASSERT_EGL_SUCCESS();
        ASSERT_NE(selectedRate, EGL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT);
        EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext));
        ASSERT_EGL_SUCCESS();
        // Make sure the surface works. Draw red and verify.
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        ASSERT_EGL_TRUE(eglSwapBuffers(mDisplay, mWindowSurface));
    }
}

}  // anonymous namespace

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLSingleBufferTest);
ANGLE_INSTANTIATE_TEST(EGLSingleBufferTest,
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLAndroidAutoRefreshTest);
ANGLE_INSTANTIATE_TEST(EGLAndroidAutoRefreshTest, WithNoFixture(ES3_VULKAN()));

ANGLE_INSTANTIATE_TEST(EGLSurfaceTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES3_METAL()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()),
                       WithNoFixture(ES2_VULKAN_SWIFTSHADER()),
                       WithNoFixture(ES3_VULKAN_SWIFTSHADER()));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLFloatSurfaceTest);
ANGLE_INSTANTIATE_TEST(EGLFloatSurfaceTest,
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLSurfaceTest3);
ANGLE_INSTANTIATE_TEST(EGLSurfaceTest3,
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES3_VULKAN()),
                       WithNoFixture(ES3_VULKAN_SWIFTSHADER()));

#if defined(ANGLE_ENABLE_D3D11)
ANGLE_INSTANTIATE_TEST(EGLSurfaceTestD3D11, WithNoFixture(ES2_D3D11()), WithNoFixture(ES3_D3D11()));
#endif
