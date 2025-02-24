//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLSurfacelessContextTest.cpp:
//   Tests for the EGL_KHR_surfaceless_context and GL_OES_surfaceless_context

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class EGLSurfacelessContextTest : public ANGLETest<>
{
  public:
    EGLSurfacelessContextTest() : mDisplay(0) {}

    void testSetUp() override
    {
        EGLAttrib dispattrs[3] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
                                  EGL_NONE};
        mDisplay               = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                                       reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        ASSERT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));

        int nConfigs = 0;
        ASSERT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &nConfigs));
        ASSERT_TRUE(nConfigs != 0);

        int nReturnedConfigs = 0;
        std::vector<EGLConfig> configs(nConfigs);
        ASSERT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), nConfigs, &nReturnedConfigs));
        ASSERT_TRUE(nConfigs == nReturnedConfigs);

        for (auto config : configs)
        {
            EGLint surfaceType;
            eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
            if (surfaceType & EGL_PBUFFER_BIT)
            {
                mConfig           = config;
                mSupportsPbuffers = true;
                break;
            }
        }

        if (!mConfig)
        {
            mConfig = configs[0];
        }

        ASSERT_NE(nullptr, mConfig);
    }

    void testTearDown() override
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (mContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(mDisplay, mContext);
        }

        if (mPbuffer != EGL_NO_SURFACE)
        {
            eglDestroySurface(mDisplay, mPbuffer);
        }

        eglTerminate(mDisplay);
    }

  protected:
    EGLContext createContext()
    {
        const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

        mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, contextAttribs);
        EXPECT_TRUE(mContext != EGL_NO_CONTEXT);
        return mContext;
    }

    EGLSurface createPbuffer(int width, int height)
    {
        if (!mSupportsPbuffers)
        {
            return EGL_NO_SURFACE;
        }

        const EGLint pbufferAttribs[] = {
            EGL_WIDTH, 500, EGL_HEIGHT, 500, EGL_NONE,
        };
        mPbuffer = eglCreatePbufferSurface(mDisplay, mConfig, pbufferAttribs);
        EXPECT_TRUE(mPbuffer != EGL_NO_SURFACE);
        return mPbuffer;
    }

    void createFramebuffer(GLFramebuffer *fbo, GLTexture *tex) const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo->get());

        glBindTexture(GL_TEXTURE_2D, tex->get());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 500, 500, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->get(), 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    }

    bool checkExtension(bool verbose = true) const
    {
        if (!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_surfaceless_context"))
        {
            if (verbose)
            {
                std::cout << "Test skipped because EGL_KHR_surfaceless_context is not available."
                          << std::endl;
            }
            return false;
        }
        return true;
    }

    EGLContext mContext    = EGL_NO_CONTEXT;
    EGLSurface mPbuffer    = EGL_NO_SURFACE;
    bool mSupportsPbuffers = false;
    EGLConfig mConfig      = 0;
    EGLDisplay mDisplay    = EGL_NO_DISPLAY;
};

// Test surfaceless MakeCurrent returns the correct value.
TEST_P(EGLSurfacelessContextTest, MakeCurrentSurfaceless)
{
    EGLContext context = createContext();

    if (eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context))
    {
        ASSERT_TRUE(checkExtension(false));
    }
    else
    {
        // The extension allows EGL_BAD_MATCH with the extension too, but ANGLE
        // shouldn't do that.
        ASSERT_FALSE(checkExtension(false));
    }
}

// Test that the scissor and viewport are set correctly
TEST_P(EGLSurfacelessContextTest, DefaultScissorAndViewport)
{
    if (!checkExtension())
    {
        return;
    }

    EGLContext context = createContext();
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

    GLint scissor[4] = {1, 2, 3, 4};
    glGetIntegerv(GL_SCISSOR_BOX, scissor);
    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(0, scissor[0]);
    ASSERT_EQ(0, scissor[1]);
    ASSERT_EQ(0, scissor[2]);
    ASSERT_EQ(0, scissor[3]);

    GLint viewport[4] = {1, 2, 3, 4};
    glGetIntegerv(GL_VIEWPORT, viewport);
    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(0, viewport[0]);
    ASSERT_EQ(0, viewport[1]);
    ASSERT_EQ(0, viewport[2]);
    ASSERT_EQ(0, viewport[3]);
}

// Test the CheckFramebufferStatus returns the correct value.
TEST_P(EGLSurfacelessContextTest, CheckFramebufferStatus)
{
    if (!checkExtension())
    {
        return;
    }

    EGLContext context = createContext();
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_UNDEFINED_OES, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLFramebuffer fbo;
    GLTexture tex;
    createFramebuffer(&fbo, &tex);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.get());
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
}

// Test that clearing and readpixels work when done in an FBO.
TEST_P(EGLSurfacelessContextTest, ClearReadPixelsInFBO)
{
    if (!checkExtension())
    {
        return;
    }

    EGLContext context = createContext();
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

    GLFramebuffer fbo;
    GLTexture tex;
    createFramebuffer(&fbo, &tex);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.get());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(250, 250, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Test clear+readpixels in an FBO in surfaceless and in the default FBO in a pbuffer
TEST_P(EGLSurfacelessContextTest, Switcheroo)
{
    ANGLE_SKIP_TEST_IF(!checkExtension());
    ANGLE_SKIP_TEST_IF(!mSupportsPbuffers);

    // Fails on NVIDIA Shield TV (http://anglebug.com/42263498)
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsNVIDIA());

    EGLContext context = createContext();
    EGLSurface pbuffer = createPbuffer(500, 500);

    // We need to make the context current to do the one time setup of the FBO
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));
    GLFramebuffer fbo;
    GLTexture tex;
    createFramebuffer(&fbo, &tex);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.get());

    for (int i = 0; i < 4; ++i)
    {
        // Clear to red in the FBO in surfaceless mode
        ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

        glBindFramebuffer(GL_FRAMEBUFFER, fbo.get());
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(250, 250, GLColor::red);
        ASSERT_GL_NO_ERROR();

        // Clear to green in the pbuffer
        ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, pbuffer, pbuffer, context));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(250, 250, GLColor::green);
        ASSERT_GL_NO_ERROR();
    }
}

}  // anonymous namespace

ANGLE_INSTANTIATE_TEST(EGLSurfacelessContextTest,
                       WithNoFixture(ES2_D3D9()),
                       WithNoFixture(ES2_D3D11()),
                       WithNoFixture(ES2_METAL()),
                       WithNoFixture(ES2_OPENGL()),
                       WithNoFixture(ES2_OPENGLES()),
                       WithNoFixture(ES2_VULKAN()));
