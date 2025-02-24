//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLQueryContextTest : public ANGLETest<>
{
  public:
    void testSetUp() override
    {
        int clientVersion = GetParam().majorVersion;

        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_TRUE(eglInitialize(mDisplay, nullptr, nullptr) != EGL_FALSE);

        EGLint ncfg;
        EGLint cfgattrs[] = {EGL_RED_SIZE,
                             8,
                             EGL_GREEN_SIZE,
                             8,
                             EGL_BLUE_SIZE,
                             8,
                             EGL_RENDERABLE_TYPE,
                             clientVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
                             EGL_NONE};
        EXPECT_TRUE(eglChooseConfig(mDisplay, cfgattrs, &mConfig, 1, &ncfg) != EGL_FALSE);
        EXPECT_TRUE(ncfg == 1);

        EGLint ctxattrs[] = {EGL_CONTEXT_CLIENT_VERSION, clientVersion, EGL_NONE};
        mContext          = eglCreateContext(mDisplay, mConfig, nullptr, ctxattrs);
        EXPECT_TRUE(mContext != EGL_NO_CONTEXT);

        EGLint surfaceType = EGL_NONE;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_SURFACE_TYPE, &surfaceType);
        if (surfaceType & EGL_PBUFFER_BIT)
        {
            EGLint surfattrs[] = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE};
            mSurface           = eglCreatePbufferSurface(mDisplay, mConfig, surfattrs);
            EXPECT_TRUE(mSurface != EGL_NO_SURFACE);
        }
    }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroyContext(mDisplay, mContext);
            if (mSurface)
            {
                eglDestroySurface(mDisplay, mSurface);
            }
            eglTerminate(mDisplay);
        }
        ASSERT_EGL_SUCCESS() << "Error during test TearDown";
    }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLConfig mConfig   = EGL_NO_CONFIG_KHR;
    EGLContext mContext = EGL_NO_CONTEXT;
    EGLSurface mSurface = EGL_NO_SURFACE;
};

TEST_P(EGLQueryContextTest, GetConfigID)
{
    EGLint configId, contextConfigId;
    EXPECT_TRUE(eglGetConfigAttrib(mDisplay, mConfig, EGL_CONFIG_ID, &configId) != EGL_FALSE);
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONFIG_ID, &contextConfigId) != EGL_FALSE);
    EXPECT_TRUE(configId == contextConfigId);
}

TEST_P(EGLQueryContextTest, GetClientType)
{
    EGLint clientType;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONTEXT_CLIENT_TYPE, &clientType) !=
                EGL_FALSE);
    EXPECT_TRUE(clientType == EGL_OPENGL_ES_API);
}

TEST_P(EGLQueryContextTest, GetClientVersion)
{
    EGLint clientVersion;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONTEXT_CLIENT_VERSION, &clientVersion) !=
                EGL_FALSE);
    EXPECT_GE(clientVersion, GetParam().majorVersion);
}

// Tests querying the client major version from the context.
TEST_P(EGLQueryContextTest, GetClientMajorVersion)
{
    EGLint majorVersion;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONTEXT_MAJOR_VERSION, &majorVersion) !=
                EGL_FALSE);
    EXPECT_GE(majorVersion, GetParam().majorVersion);
}

// Tests querying the client minor version from the context.
TEST_P(EGLQueryContextTest, GetClientMinorVersion)
{
    EGLint minorVersion;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONTEXT_MINOR_VERSION, &minorVersion) !=
                EGL_FALSE);
    EXPECT_GE(minorVersion, GetParam().minorVersion);
}

TEST_P(EGLQueryContextTest, GetRenderBufferNoSurface)
{
    EGLint renderBuffer;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_RENDER_BUFFER, &renderBuffer) != EGL_FALSE);
    EXPECT_TRUE(renderBuffer == EGL_NONE);
}

TEST_P(EGLQueryContextTest, GetRenderBufferBoundSurface)
{
    ANGLE_SKIP_TEST_IF(!mSurface);

    EGLint renderBuffer, contextRenderBuffer;
    EXPECT_TRUE(eglQuerySurface(mDisplay, mSurface, EGL_RENDER_BUFFER, &renderBuffer) != EGL_FALSE);
    EXPECT_TRUE(eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) != EGL_FALSE);
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_RENDER_BUFFER, &contextRenderBuffer) !=
                EGL_FALSE);
    EXPECT_TRUE(renderBuffer == contextRenderBuffer);
    ASSERT_EGL_SUCCESS();
}

TEST_P(EGLQueryContextTest, BadDisplay)
{
    EGLint val;
    EXPECT_TRUE(eglQueryContext(EGL_NO_DISPLAY, mContext, EGL_CONTEXT_CLIENT_TYPE, &val) ==
                EGL_FALSE);
    EXPECT_TRUE(eglGetError() == EGL_BAD_DISPLAY);
}

TEST_P(EGLQueryContextTest, NotInitialized)
{
    EGLint val;
    testTearDown();
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONTEXT_CLIENT_TYPE, &val) == EGL_FALSE);
    EXPECT_TRUE(eglGetError() == EGL_NOT_INITIALIZED);

    mDisplay = EGL_NO_DISPLAY;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;
}

TEST_P(EGLQueryContextTest, BadContext)
{
    EGLint val;
    EXPECT_TRUE(eglQueryContext(mDisplay, EGL_NO_CONTEXT, EGL_CONTEXT_CLIENT_TYPE, &val) ==
                EGL_FALSE);
    EXPECT_TRUE(eglGetError() == EGL_BAD_CONTEXT);
}

TEST_P(EGLQueryContextTest, BadAttribute)
{
    EGLint val;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_HEIGHT, &val) == EGL_FALSE);
    EXPECT_TRUE(eglGetError() == EGL_BAD_ATTRIBUTE);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLQueryContextTest);
ANGLE_INSTANTIATE_TEST(EGLQueryContextTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_OPENGL()));
