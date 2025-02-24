//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLBufferAgeTest.cpp:
//   EGL extension EGL_EXT_buffer_age
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"

using namespace angle;

class EGLBufferAgeTest : public ANGLETest<>
{
  public:
    EGLBufferAgeTest() : mDisplay(EGL_NO_DISPLAY) {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
        mMajorVersion       = GetParam().majorVersion;
        mExtensionSupported = IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_buffer_age");
    }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(mDisplay);
            eglReleaseThread();
            mDisplay = EGL_NO_DISPLAY;
        }
        ASSERT_EGL_SUCCESS() << "Error during test TearDown";
    }

    virtual bool chooseConfig(EGLConfig *config) const
    {
        bool result          = false;
        EGLint count         = 0;
        EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
        EGLint attribs[]     = {EGL_RED_SIZE,
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
                                EGL_WINDOW_BIT,
                                EGL_NONE};

        result = eglChooseConfig(mDisplay, attribs, config, 1, &count);
        EXPECT_EGL_TRUE(result && (count > 0));
        return result;
    }

    bool createContext(EGLConfig config, EGLContext *context)
    {
        bool result      = false;
        EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};

        *context = eglCreateContext(mDisplay, config, nullptr, attribs);
        result   = (*context != EGL_NO_CONTEXT);
        EXPECT_TRUE(result);
        return result;
    }

    bool createWindowSurface(EGLConfig config, EGLNativeWindowType win, EGLSurface *surface) const
    {
        bool result      = false;
        EGLint attribs[] = {EGL_NONE};

        *surface = eglCreateWindowSurface(mDisplay, config, win, attribs);
        result   = (*surface != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    EGLint queryAge(EGLSurface surface) const
    {
        EGLint age  = 0;
        bool result = eglQuerySurface(mDisplay, surface, EGL_BUFFER_AGE_EXT, &age);
        EXPECT_TRUE(result);
        return age;
    }

    EGLint queryAgeAttribKHR(EGLSurface surface) const
    {
        EGLAttribKHR age = 0;
        bool result      = eglQuerySurface64KHR(mDisplay, surface, EGL_BUFFER_AGE_EXT, &age);
        EXPECT_TRUE(result);
        return age;
    }

    EGLDisplay mDisplay      = EGL_NO_DISPLAY;
    EGLint mMajorVersion     = 0;
    const EGLint kWidth      = 64;
    const EGLint kHeight     = 64;
    bool mExtensionSupported = false;
};

class EGLBufferAgeTest_MSAA : public EGLBufferAgeTest
{
  public:
    EGLBufferAgeTest_MSAA() : EGLBufferAgeTest() {}

    bool chooseConfig(EGLConfig *config) const override
    {
        bool result          = false;
        EGLint count         = 0;
        EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
        EGLint attribs[]     = {EGL_RED_SIZE,
                                8,
                                EGL_GREEN_SIZE,
                                8,
                                EGL_BLUE_SIZE,
                                8,
                                EGL_ALPHA_SIZE,
                                8,
                                EGL_RENDERABLE_TYPE,
                                clientVersion,
                                EGL_SAMPLE_BUFFERS,
                                1,
                                EGL_SAMPLES,
                                4,
                                EGL_SURFACE_TYPE,
                                EGL_WINDOW_BIT,
                                EGL_NONE};

        result = eglChooseConfig(mDisplay, attribs, config, 1, &count);
        EXPECT_EGL_TRUE(result && (count > 0));
        return result;
    }
};

class EGLBufferAgeTest_MSAA_DS : public EGLBufferAgeTest
{
  public:
    EGLBufferAgeTest_MSAA_DS() : EGLBufferAgeTest() {}

    bool chooseConfig(EGLConfig *config) const override
    {
        bool result          = false;
        EGLint count         = 0;
        EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
        EGLint attribs[]     = {EGL_RED_SIZE,
                                8,
                                EGL_GREEN_SIZE,
                                8,
                                EGL_BLUE_SIZE,
                                8,
                                EGL_ALPHA_SIZE,
                                8,
                                EGL_DEPTH_SIZE,
                                8,
                                EGL_STENCIL_SIZE,
                                8,
                                EGL_RENDERABLE_TYPE,
                                clientVersion,
                                EGL_SAMPLE_BUFFERS,
                                1,
                                EGL_SAMPLES,
                                4,
                                EGL_SURFACE_TYPE,
                                EGL_WINDOW_BIT,
                                EGL_NONE};

        result = eglChooseConfig(mDisplay, attribs, config, 1, &count);
        EXPECT_EGL_TRUE(result && (count > 0));
        return result;
    }
};

// Query for buffer age
TEST_P(EGLBufferAgeTest, QueryBufferAge)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);

    const bool lockSurface3ExtSupported =
        IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_lock_surface3");

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLBufferAgeTest", kWidth, kHeight);
    EXPECT_TRUE(createWindowSurface(config, osWindow->getNativeWindow(), &surface));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glClearColor(1.0, 0.0, 0.0, 1.0);

    const uint32_t loopcount = 15;
    EGLint expectedAge       = 0;
    for (uint32_t i = 0; i < loopcount; i++)
    {
        // Alternate between eglQuerySurface and eglQuerySurface64KHR
        EGLint age = -1;
        if (i % 2 == 0 || !lockSurface3ExtSupported)
        {
            age = queryAge(surface);
        }
        else
        {
            age = static_cast<EGLint>(queryAgeAttribKHR(surface));
        }

        // Should start with zero age and then flip to buffer count - which we don't know.
        if ((expectedAge == 0) && (age > 0))
        {
            expectedAge = age;
        }
        EXPECT_EQ(age, expectedAge);

        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR() << "glClear failed";
        eglSwapBuffers(mDisplay, surface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    }

    EXPECT_GT(expectedAge, 0);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Query for buffer age after several loops of swapping buffers
TEST_P(EGLBufferAgeTest, QueryBufferAgeAfterLoop)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLBufferAgeTest", kWidth, kHeight);
    EXPECT_TRUE(createWindowSurface(config, osWindow->getNativeWindow(), &surface));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glClearColor(1.0, 0.0, 0.0, 1.0);

    const uint32_t loopcount = 5;
    for (uint32_t i = 0; i < loopcount; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR() << "glClear failed";
        eglSwapBuffers(mDisplay, surface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    }

    // This query age should not reset age
    EXPECT_GT(queryAge(surface), 0);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Verify contents of buffer are as expected
TEST_P(EGLBufferAgeTest, VerifyContents)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLBufferAgeTest", kWidth, kHeight);
    EXPECT_TRUE(createWindowSurface(config, osWindow->getNativeWindow(), &surface));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    const angle::GLColor kLightGray(191, 191, 191, 255);  // 0.75
    const angle::GLColor kDarkGray(64, 64, 64, 255);      // 0.25
    const angle::GLColor kColorSet[] = {
        GLColor::blue,  GLColor::cyan,   kDarkGray,      GLColor::green,   GLColor::red,
        GLColor::white, GLColor::yellow, GLColor::black, GLColor::magenta, kLightGray,
        GLColor::black,  // Extra loops until color cycled through
        GLColor::black, GLColor::black,  GLColor::black, GLColor::black};

    EGLint age                   = 0;
    angle::GLColor expectedColor = GLColor::black;
    int loopCount                = (sizeof(kColorSet) / sizeof(kColorSet[0]));
    for (int i = 0; i < loopCount; i++)
    {
        age = queryAge(surface);
        if (age > 0)
        {
            // Check that color/content is what we expect
            expectedColor = kColorSet[i - age];
            EXPECT_PIXEL_COLOR_EQ(1, 1, expectedColor);
        }

        float red   = kColorSet[i].R / 255.0;
        float green = kColorSet[i].G / 255.0;
        float blue  = kColorSet[i].B / 255.0;
        float alpha = kColorSet[i].A / 255.0;

        glClearColor(red, green, blue, alpha);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR() << "glClear failed";
        eglSwapBuffers(mDisplay, surface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    }

    EXPECT_GT(age, 0);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Verify contents of buffer are as expected for a multisample image
TEST_P(EGLBufferAgeTest_MSAA, VerifyContentsForMultisampled)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLBufferAgeTest_MSAA", kWidth, kHeight);
    EXPECT_TRUE(createWindowSurface(config, osWindow->getNativeWindow(), &surface));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    std::vector<angle::GLColor> kColorSet;
    for (uint32_t i = 0; i < 16; i++)
    {
        kColorSet.emplace_back(i * 10, 0, 0, 255);
    }

    // Set up
    glClearColor(0, 0, 0, 0);
    glClearDepthf(0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EGLint age     = 0;
    GLuint program = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

    for (size_t i = 0; i < kColorSet.size(); i++)
    {
        age = queryAge(surface);
        if (age > 0)
        {
            // Check that color/content is what we expect
            angle::GLColor expectedColor = kColorSet[i - age];
            EXPECT_PIXEL_COLOR_EQ(1, 1, expectedColor);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(program);
        glUniform4fv(colorLocation, 1, kColorSet[i].toNormalizedVector().data());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

        eglSwapBuffers(mDisplay, surface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    }

    EXPECT_GE(age, 0);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Verify contents of buffer are as expected for a multisample image with depth/stencil enabled
TEST_P(EGLBufferAgeTest_MSAA_DS, VerifyContentsForMultisampledWithDepthStencil)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLBufferAgeTest_MSAA", kWidth, kHeight);
    EXPECT_TRUE(createWindowSurface(config, osWindow->getNativeWindow(), &surface));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    std::vector<angle::GLColor> kColorSet;
    for (uint32_t i = 0; i < 16; i++)
    {
        kColorSet.emplace_back(i * 10, 0, 0, 255);
    }

    // Set up
    glClearColor(0, 0, 0, 0);
    glClearDepthf(0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EGLint age     = 0;
    GLuint program = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

    for (size_t i = 0; i < kColorSet.size(); i++)
    {
        age = queryAge(surface);
        if (age > 0)
        {
            // Check that color/content is what we expect
            angle::GLColor expectedColor = kColorSet[i - age];
            EXPECT_PIXEL_COLOR_EQ(1, 1, expectedColor);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(program);
        glUniform4fv(colorLocation, 1, kColorSet[i].toNormalizedVector().data());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

        eglSwapBuffers(mDisplay, surface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    }

    EXPECT_GE(age, 0);

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Verify EGL_BAD_SURFACE when not current
TEST_P(EGLBufferAgeTest, UncurrentContextBadSurface)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(&config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLBufferAgeTest", kWidth, kHeight);
    EXPECT_TRUE(createWindowSurface(config, osWindow->getNativeWindow(), &surface));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    // No current context

    EGLint value = 0;
    EXPECT_EGL_FALSE(eglQuerySurface(mDisplay, surface, EGL_BUFFER_AGE_EXT, &value));
    EXPECT_EGL_ERROR(EGL_BAD_SURFACE);

    EGLContext otherContext = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &otherContext));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    // Surface current to another context
    EXPECT_TRUE(eglMakeCurrent(mDisplay, surface, surface, otherContext));
    // Make context the active context
    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

    value = 0;
    EXPECT_EGL_FALSE(eglQuerySurface(mDisplay, surface, EGL_BUFFER_AGE_EXT, &value));
    EXPECT_EGL_ERROR(EGL_BAD_SURFACE);

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, otherContext);
    otherContext = EGL_NO_CONTEXT;

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Expect age always == 1 when EGL_BUFFER_PRESERVED is chosen
TEST_P(EGLBufferAgeTest, BufferPreserved)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);

    EGLConfig config     = EGL_NO_CONFIG_KHR;
    EGLint count         = 0;
    EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
    EGLint attribs[]     = {EGL_RED_SIZE,
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
                            EGL_WINDOW_BIT | EGL_SWAP_BEHAVIOR_PRESERVED_BIT,
                            EGL_NONE};

    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, &config, 1, &count));
    // Skip if no configs, this indicates EGL_BUFFER_PRESERVED is not supported.
    ANGLE_SKIP_TEST_IF(count == 0);

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLBufferAgeTest", kWidth, kHeight);
    EXPECT_TRUE(createWindowSurface(config, osWindow->getNativeWindow(), &surface));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glClearColor(1.0, 0.0, 0.0, 1.0);

    const uint32_t loopcount = 10;
    EGLint expectedAge       = 1;
    for (uint32_t i = 0; i < loopcount; i++)
    {
        EGLint age = queryAge(surface);
        EXPECT_EQ(age, expectedAge);

        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR() << "glClear failed";
        eglSwapBuffers(mDisplay, surface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    }

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

// Expect age always == 0 when EGL_SINGLE_BUFFER is chosen
TEST_P(EGLBufferAgeTest, SingleBuffer)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_mutable_render_buffer"));

    EGLConfig config     = EGL_NO_CONFIG_KHR;
    EGLint count         = 0;
    EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
    EGLint attribs[]     = {EGL_RED_SIZE,
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
                            EGL_WINDOW_BIT | EGL_MUTABLE_RENDER_BUFFER_BIT_KHR,
                            EGL_NONE};

    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, &config, 1, &count));
    // Skip if no configs, this indicates EGL_SINGLE_BUFFER is not supported.
    ANGLE_SKIP_TEST_IF(count == 0);

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface surface = EGL_NO_SURFACE;

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("EGLBufferAgeTest", kWidth, kHeight);
    EXPECT_TRUE(createWindowSurface(config, osWindow->getNativeWindow(), &surface));
    ASSERT_EGL_SUCCESS() << "eglCreateWindowSurface failed.";

    EXPECT_TRUE(eglMakeCurrent(mDisplay, surface, surface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    // Set render buffer to EGL_SINGLE_BUFFER
    EXPECT_EGL_TRUE(eglSurfaceAttrib(mDisplay, surface, EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));

    glClearColor(1.0, 0.0, 0.0, 1.0);

    // Expect age = 0 before first eglSwapBuffers() call
    EGLint age = queryAge(surface);
    EXPECT_EQ(age, 0);
    eglSwapBuffers(mDisplay, surface);
    ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";

    const uint32_t loopcount = 10;
    EGLint expectedAge       = 0;
    for (uint32_t i = 0; i < loopcount; i++)
    {
        age = queryAge(surface);
        EXPECT_EQ(age, expectedAge);

        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR() << "glClear failed";
        eglSwapBuffers(mDisplay, surface);
        ASSERT_EGL_SUCCESS() << "eglSwapBuffers failed.";
    }

    EXPECT_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent - uncurrent failed.";

    eglDestroySurface(mDisplay, surface);
    surface = EGL_NO_SURFACE;
    osWindow->destroy();
    OSWindow::Delete(&osWindow);

    eglDestroyContext(mDisplay, context);
    context = EGL_NO_CONTEXT;
}

ANGLE_INSTANTIATE_TEST(EGLBufferAgeTest,
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES3_METAL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));
ANGLE_INSTANTIATE_TEST(EGLBufferAgeTest_MSAA,
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES3_METAL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));
ANGLE_INSTANTIATE_TEST(EGLBufferAgeTest_MSAA_DS,
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES3_METAL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));
