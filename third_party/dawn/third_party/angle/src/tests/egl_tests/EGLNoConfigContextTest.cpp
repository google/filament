//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLNoConfigContectTest.cpp:
//   EGL extension EGL_KHR_no_config_context allows a context to be created
//   without a config specified. This means all surfaces are compatible.
//   As a result compatibility checks are circumvented.
//   This test suite creates and verifies creating a configless context
//   and then verifies simple rendering to ensure compatibility.
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLNoConfigContextTest : public ANGLETest<>
{
  public:
    EGLNoConfigContextTest() : mDisplay(EGL_NO_DISPLAY), mContext(EGL_NO_CONTEXT) {}

    void testSetUp() override
    {
        int clientVersion = GetParam().majorVersion;

        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
                      EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));

        mExtensionSupported = IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_no_config_context");
        if (!mExtensionSupported)
        {
            return;  // Not supported, don't create context
        }

        EGLint ctxattrs[] = {EGL_CONTEXT_CLIENT_VERSION, clientVersion, EGL_NONE};
        mContext          = eglCreateContext(mDisplay, EGL_NO_CONFIG_KHR, nullptr, ctxattrs);
        EXPECT_TRUE(mContext != EGL_NO_CONTEXT);
    }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            if (mContext != EGL_NO_CONTEXT)
            {
                eglDestroyContext(mDisplay, mContext);
                mContext = EGL_NO_CONTEXT;
            }
            eglTerminate(mDisplay);
            eglReleaseThread();
        }
        ASSERT_EGL_SUCCESS() << "Error during test TearDown";
    }

    EGLDisplay mDisplay      = EGL_NO_DISPLAY;
    EGLContext mContext      = EGL_NO_CONTEXT;
    bool mExtensionSupported = false;
};

// Check that context has no config.
TEST_P(EGLNoConfigContextTest, QueryConfigID)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);
    EXPECT_TRUE(mDisplay);
    EXPECT_TRUE(mContext);

    EGLint configId = -1;
    EXPECT_EGL_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONFIG_ID, &configId));
    EXPECT_TRUE(configId == 0);
    ASSERT_EGL_SUCCESS();
}

// Any surface should be eglMakeCurrent compatible with no-config context.
// Do a glClear and glReadPixel to verify rendering.
TEST_P(EGLNoConfigContextTest, RenderCheck)
{
    ANGLE_SKIP_TEST_IF(!mExtensionSupported);

    // Get all the configs
    EGLint count;
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &count));
    EXPECT_TRUE(count > 0);
    std::vector<EGLConfig> configs(count);
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), count, &count));

    // For each config, create PbufferSurface and do a render check
    EGLSurface surface = EGL_NO_SURFACE;
    for (auto config : configs)
    {
        const uint32_t kWidth  = 1;
        const uint32_t kHeight = 1;

        EGLint configId;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_CONFIG_ID, &configId));

        EGLint surfaceType;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType));
        EGLint bufferSize;
        EXPECT_EGL_TRUE(eglGetConfigAttrib(mDisplay, config, EGL_BUFFER_SIZE, &bufferSize));
        constexpr int kRGB8BitSize = 24;  // RGB8 is 24 bits
        if (isVulkanRenderer() && bufferSize == kRGB8BitSize &&
            (surfaceType & EGL_PBUFFER_BIT) != EGL_PBUFFER_BIT)
        {
            // Skip this config, since the Vulkan backend doesn't support RGB8 pbuffer surfaces.
            continue;
        }

        EGLint surfattrs[] = {EGL_WIDTH, kWidth, EGL_HEIGHT, kHeight, EGL_NONE};
        surface            = eglCreatePbufferSurface(mDisplay, config, surfattrs);
        EXPECT_TRUE(surface != EGL_NO_SURFACE);

        EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, surface, surface, mContext));
        ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed with Config: " << configId << '\n';

        // ClearColor RED
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR() << "glClear failed";

        if (bufferSize > 32)
        {  // GL_FLOAT configs
            EXPECT_PIXEL_COLOR32F_EQ(0, 0, kFloatRed);
        }
        else
        {  // GL_UNSIGNED_BYTE configs
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        }

        eglDestroySurface(mDisplay, surface);
        surface = EGL_NO_SURFACE;
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLNoConfigContextTest);
ANGLE_INSTANTIATE_TEST(EGLNoConfigContextTest,
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES3_VULKAN()));
