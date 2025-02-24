//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLCreateContectAttribsTest.cpp:
//   This suite of test cases test invalid attributes passed to eglCreateContext
//   Section 3.7.1 of EGL 1.5 specification provides error cases
//

#include <gtest/gtest.h>
#include <vector>

#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLCreateContextAttribsTest : public ANGLETest<>
{
  public:
    EGLCreateContextAttribsTest() : mDisplay(EGL_NO_DISPLAY) {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
                      EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr) != EGL_FALSE);
    }

    EGLDisplay mDisplay;
};

// Specify invalid client version in the attributes to eglCreateContext
// and verify EGL_BAD_ATTRIBUTE
TEST_P(EGLCreateContextAttribsTest, InvalidClientVersion)
{
    EGLContext context = EGL_NO_CONTEXT;

    // Pick config
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EGLint count     = 0;

    // Get a 1.0 compatible config
    EGLint cfgAttribList1[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT, EGL_NONE};
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, cfgAttribList1, &config, 1, &count));
    ANGLE_SKIP_TEST_IF(count == 0);

    // GLES 0.0 is invalid verify invalid attribute request
    EGLint contextAttribs1[] = {EGL_CONTEXT_MAJOR_VERSION, 0, EGL_CONTEXT_MINOR_VERSION, 0,
                                EGL_NONE};
    context                  = eglCreateContext(mDisplay, config, nullptr, contextAttribs1);
    EXPECT_EQ(context, EGL_NO_CONTEXT);
    ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // Get a 2.0/3.x compatible config
    EGLint cfgAttribList2[] = {EGL_RENDERABLE_TYPE, (EGL_OPENGL_ES2_BIT), EGL_NONE};
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, cfgAttribList2, &config, 1, &count));
    ASSERT_TRUE(count > 0);

    // GLES 2.1 is invalid verify invalid attribute request
    EGLint contextAttribs2[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_CONTEXT_MINOR_VERSION, 1,
                                EGL_NONE};
    context                  = eglCreateContext(mDisplay, config, nullptr, contextAttribs2);
    EXPECT_EQ(context, EGL_NO_CONTEXT);
    ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // GLES 3.3 is invalid verify invalid attribute request
    EGLint contextAttribs3[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 3,
                                EGL_NONE};
    context                  = eglCreateContext(mDisplay, config, nullptr, contextAttribs3);
    EXPECT_EQ(context, EGL_NO_CONTEXT);
    ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // GLES 4.0 is invalid verify invalid attribute request
    EGLint contextAttribs4[] = {EGL_CONTEXT_MAJOR_VERSION, 4, EGL_CONTEXT_MINOR_VERSION, 0,
                                EGL_NONE};
    context                  = eglCreateContext(mDisplay, config, nullptr, contextAttribs4);
    EXPECT_EQ(context, EGL_NO_CONTEXT);
    ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // Cleanup contexts
    eglTerminate(mDisplay);
}

// Choose config that doesn't support requested client version, and verify that eglCreateContext
// sets EGL_BAD_MATCH
TEST_P(EGLCreateContextAttribsTest, IncompatibleConfig)
{
    // Get all the configs
    EGLint count;
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &count) != EGL_FALSE);
    EXPECT_TRUE(count > 0);
    std::vector<EGLConfig> configs(count);
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), count, &count) != EGL_FALSE);

    EGLConfig notGLES1Config = EGL_NO_CONFIG_KHR;
    EGLConfig notGLES2Config = EGL_NO_CONFIG_KHR;
    EGLConfig notGLES3Config = EGL_NO_CONFIG_KHR;

    // Find non API matching configs
    for (auto config : configs)
    {
        EGLint value = 0;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_RENDERABLE_TYPE, &value));

        if (((value & EGL_OPENGL_ES_BIT) == 0) && (notGLES1Config == EGL_NO_CONFIG_KHR))
        {
            notGLES1Config = config;
            continue;
        }
        if (((value & EGL_OPENGL_ES2_BIT) == 0) && (notGLES2Config == EGL_NO_CONFIG_KHR))
        {
            notGLES2Config = config;
            continue;
        }
        if (((value & EGL_OPENGL_ES3_BIT) == 0) && (notGLES3Config == EGL_NO_CONFIG_KHR))
        {
            notGLES3Config = config;
            continue;
        }
    }

    // These selected configs should not be a match with the requested client version.
    EGLContext context = EGL_NO_CONTEXT;
    // Check GLES1
    if (notGLES1Config != EGL_NO_CONFIG_KHR)
    {
        EGLint contextAttribs1[] = {EGL_CONTEXT_MAJOR_VERSION, 1, EGL_CONTEXT_MINOR_VERSION, 0,
                                    EGL_NONE};
        context = eglCreateContext(mDisplay, notGLES1Config, nullptr, contextAttribs1);
        EXPECT_EQ(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_BAD_MATCH);
    }

    // Check GLES2
    if (notGLES2Config != EGL_NO_CONFIG_KHR)
    {
        EGLint contextAttribs2[] = {EGL_CONTEXT_MAJOR_VERSION, 2, EGL_CONTEXT_MINOR_VERSION, 0,
                                    EGL_NONE};
        context = eglCreateContext(mDisplay, notGLES2Config, nullptr, contextAttribs2);
        EXPECT_EQ(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_BAD_MATCH);
    }

    // Check GLES3
    if (notGLES3Config != EGL_NO_CONFIG_KHR)
    {
        EGLint contextAttribs3[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0,
                                    EGL_NONE};
        context = eglCreateContext(mDisplay, notGLES3Config, nullptr, contextAttribs3);
        EXPECT_EQ(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_BAD_MATCH);
    }

    // Cleanup contexts
    eglTerminate(mDisplay);
}

// EGL_IMG_context_priority - set and get attribute
TEST_P(EGLCreateContextAttribsTest, IMGContextPriorityExtension)
{
    const EGLint configAttributes[] = {EGL_RED_SIZE,  8, EGL_GREEN_SIZE,   8,
                                       EGL_BLUE_SIZE, 8, EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                       EGL_NONE};

    // Get all the configs
    EGLint count;
    EGLConfig config;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, configAttributes, &config, 1, &count));
    EXPECT_TRUE(count == 1);

    EGLContext context      = EGL_NO_CONTEXT;
    EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION,
                               2,
                               EGL_CONTEXT_MINOR_VERSION,
                               0,
                               EGL_CONTEXT_PRIORITY_LEVEL_IMG,
                               EGL_CONTEXT_PRIORITY_HIGH_IMG,
                               EGL_NONE};

    if (IsEGLDisplayExtensionEnabled(mDisplay, "EGL_IMG_context_priority"))
    {
        context = eglCreateContext(mDisplay, config, nullptr, contextAttribs);
        EXPECT_NE(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_SUCCESS);

        EGLint value = 0;
        EXPECT_EGL_TRUE(eglQueryContext(mDisplay, context, EGL_CONTEXT_PRIORITY_LEVEL_IMG, &value));
        ASSERT_EGL_ERROR(EGL_SUCCESS);
    }
    else  // Not supported so should get EGL_BAD_ATTRIBUTE
    {
        context = eglCreateContext(mDisplay, config, nullptr, contextAttribs);
        EXPECT_EQ(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

        EGLint noExtensionContextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 2,
                                              EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE};

        context = eglCreateContext(mDisplay, config, nullptr, noExtensionContextAttribs);
        EXPECT_NE(context, EGL_NO_CONTEXT);
        ASSERT_EGL_ERROR(EGL_SUCCESS);

        EGLint value = 0;
        EXPECT_EGL_FALSE(
            eglQueryContext(mDisplay, context, EGL_CONTEXT_PRIORITY_LEVEL_IMG, &value));
        ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    // Cleanup contexts
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    eglDestroyContext(mDisplay, context);
    eglTerminate(mDisplay);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLCreateContextAttribsTest);
ANGLE_INSTANTIATE_TEST(EGLCreateContextAttribsTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES3_VULKAN()));
