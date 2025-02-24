//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLBackwardsCompatibleContextTest.cpp.cpp:
//   Coverage of the EGL_ANGLE_create_context_backwards_compatible extension

#include <vector>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/angle_test_instantiate.h"

namespace angle
{

namespace
{
std::pair<EGLint, EGLint> GetCurrentContextVersion()
{
    const char *versionString = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    EXPECT_TRUE(strstr(versionString, "OpenGL ES") != nullptr);
    return {versionString[10] - '0', versionString[12] - '0'};
}
}  // anonymous namespace

class EGLBackwardsCompatibleContextTest : public ANGLETest<>
{
  public:
    EGLBackwardsCompatibleContextTest() : mDisplay(0) {}

    void testSetUp() override
    {
        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
                      EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        ASSERT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));

        int configsCount = 0;
        ASSERT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &configsCount));
        ASSERT_TRUE(configsCount != 0);

        std::vector<EGLConfig> configs(configsCount);
        ASSERT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), configsCount, &configsCount));

        for (auto config : configs)
        {
            EGLint surfaceType;
            eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
            if (surfaceType & EGL_PBUFFER_BIT)
            {
                mConfig = config;
                break;
            }
        }
        if (!mConfig)
        {
            mConfig = configs[0];
        }
        ASSERT_NE(nullptr, mConfig);

        EGLint surfaceType = EGL_NONE;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_SURFACE_TYPE, &surfaceType);
        if (surfaceType & EGL_PBUFFER_BIT)
        {
            const EGLint pbufferAttribs[] = {
                EGL_WIDTH, 500, EGL_HEIGHT, 500, EGL_NONE,
            };
            mPbuffer = eglCreatePbufferSurface(mDisplay, mConfig, pbufferAttribs);
            EXPECT_TRUE(mPbuffer != EGL_NO_SURFACE);
        }
    }

    void testTearDown() override
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (mPbuffer != EGL_NO_SURFACE)
        {
            eglDestroySurface(mDisplay, mPbuffer);
        }

        eglTerminate(mDisplay);
    }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLSurface mPbuffer = EGL_NO_SURFACE;
    EGLConfig mConfig   = 0;
};

// Test extension presence.  All backends should expose this extension
TEST_P(EGLBackwardsCompatibleContextTest, PbufferDifferentConfig)
{
    EXPECT_TRUE(
        IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_create_context_backwards_compatible"));
}

// Test that disabling backwards compatibility will always return the expected context version
TEST_P(EGLBackwardsCompatibleContextTest, BackwardsCompatibleDisbled)
{
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_create_context_backwards_compatible"));
    ANGLE_SKIP_TEST_IF(!mPbuffer);

    std::pair<EGLint, EGLint> testVersions[] = {
        {1, 0}, {1, 1}, {2, 0}, {3, 0}, {3, 1}, {3, 2},
    };

    for (const auto &version : testVersions)
    {
        EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION,
                            version.first,
                            EGL_CONTEXT_MINOR_VERSION,
                            version.second,
                            EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE,
                            EGL_FALSE,
                            EGL_NONE,
                            EGL_NONE};

        EGLContext context = eglCreateContext(mDisplay, mConfig, nullptr, attribs);
        if (context == EGL_NO_CONTEXT)
        {
            // Context version not supported
            continue;
        }

        ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, mPbuffer, mPbuffer, context));

        auto contextVersion = GetCurrentContextVersion();
        EXPECT_EQ(version, contextVersion);

        ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        eglDestroyContext(mDisplay, context);
    }
}

// Test that if it's possible to create an ES3 context, requesting an ES2 context should return an
// ES3 context as well
TEST_P(EGLBackwardsCompatibleContextTest, BackwardsCompatibleEnabledES3)
{
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_create_context_backwards_compatible"));
    ANGLE_SKIP_TEST_IF(!mPbuffer);

    EGLint es3ContextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE, EGL_NONE};

    EGLContext es3Context = eglCreateContext(mDisplay, mConfig, nullptr, es3ContextAttribs);
    ANGLE_SKIP_TEST_IF(es3Context == EGL_NO_CONTEXT);

    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, mPbuffer, mPbuffer, es3Context));
    auto es3ContextVersion = GetCurrentContextVersion();
    eglDestroyContext(mDisplay, es3Context);

    EGLint es2ContextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 2, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE, EGL_NONE};

    EGLContext es2Context = eglCreateContext(mDisplay, mConfig, nullptr, es2ContextAttribs);
    EXPECT_NE(es2Context, EGL_NO_CONTEXT);

    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, mPbuffer, mPbuffer, es2Context));
    auto es2ContextVersion = GetCurrentContextVersion();
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    eglDestroyContext(mDisplay, es2Context);

    EXPECT_EQ(es3ContextVersion, es2ContextVersion);
}

// Test that if ES1.1 is supported and a 1.0 context is requested, an ES 1.1 context is returned
TEST_P(EGLBackwardsCompatibleContextTest, BackwardsCompatibleEnabledES1)
{
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANGLE_create_context_backwards_compatible"));
    ANGLE_SKIP_TEST_IF(!mPbuffer);

    EGLint es11ContextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 1, EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE, EGL_NONE};

    EGLContext es11Context = eglCreateContext(mDisplay, mConfig, nullptr, es11ContextAttribs);
    ANGLE_SKIP_TEST_IF(es11Context == EGL_NO_CONTEXT);

    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, mPbuffer, mPbuffer, es11Context));
    auto es11ContextVersion = GetCurrentContextVersion();
    ASSERT_EQ(std::make_pair(1, 1), es11ContextVersion);
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    eglDestroyContext(mDisplay, es11Context);

    EGLint es10ContextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 1, EGL_CONTEXT_MINOR_VERSION, 0, EGL_NONE, EGL_NONE};

    EGLContext es10Context = eglCreateContext(mDisplay, mConfig, nullptr, es10ContextAttribs);
    EXPECT_NE(es10Context, EGL_NO_CONTEXT);

    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, mPbuffer, mPbuffer, es10Context));
    auto es10ContextVersion = GetCurrentContextVersion();
    ASSERT_EQ(std::make_pair(1, 1), es10ContextVersion);
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    eglDestroyContext(mDisplay, es10Context);
}

ANGLE_INSTANTIATE_TEST(EGLBackwardsCompatibleContextTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES2_VULKAN()));

}  // namespace angle
