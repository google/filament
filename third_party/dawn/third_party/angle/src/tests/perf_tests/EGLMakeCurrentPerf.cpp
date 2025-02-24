//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLMakeCurrentPerfTest:
//   Performance test for eglMakeCurrent.
//

#include "ANGLEPerfTest.h"
#include "common/platform.h"
#include "common/system_utils.h"
#include "platform/PlatformMethods.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/angle_test_instantiate.h"

#define ITERATIONS 20

using namespace testing;

namespace
{
class EGLMakeCurrentPerfTest : public ANGLEPerfTest,
                               public WithParamInterface<angle::PlatformParameters>
{
  public:
    EGLMakeCurrentPerfTest();

    void step() override;
    void SetUp() override;
    void TearDown() override;

  private:
    OSWindow *mOSWindow;
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLConfig mConfig;
    std::array<EGLContext, 2> mContexts;
    std::unique_ptr<angle::Library> mEGLLibrary;
};

EGLMakeCurrentPerfTest::EGLMakeCurrentPerfTest()
    : ANGLEPerfTest("EGLMakeCurrent", "", "_run", ITERATIONS),
      mOSWindow(nullptr),
      mDisplay(EGL_NO_DISPLAY),
      mSurface(EGL_NO_SURFACE),
      mConfig(nullptr),
      mContexts({})
{
    auto platform = GetParam().eglParameters;

    std::vector<EGLint> displayAttributes;
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
    displayAttributes.push_back(platform.renderer);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
    displayAttributes.push_back(platform.majorVersion);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
    displayAttributes.push_back(platform.minorVersion);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
    displayAttributes.push_back(platform.deviceType);
    displayAttributes.push_back(EGL_NONE);

    mOSWindow = OSWindow::New();
    mOSWindow->initialize("EGLMakeCurrent Test", 64, 64);

    mEGLLibrary.reset(
        angle::OpenSharedLibrary(ANGLE_EGL_LIBRARY_NAME, angle::SearchType::ModuleDir));

    LoadProc getProc = reinterpret_cast<LoadProc>(mEGLLibrary->getSymbol("eglGetProcAddress"));

    if (!getProc)
    {
        abortTest();
    }
    else
    {
        LoadUtilEGL(getProc);
        // Test harness warmup calls glFinish so we need GLES too.
        LoadUtilGLES(getProc);

        if (!eglGetPlatformDisplayEXT)
        {
            abortTest();
        }
        else
        {
            mDisplay = eglGetPlatformDisplayEXT(
                EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(mOSWindow->getNativeDisplay()),
                &displayAttributes[0]);
        }
    }
}

void EGLMakeCurrentPerfTest::SetUp()
{
    ANGLEPerfTest::SetUp();

    ASSERT_NE(EGL_NO_DISPLAY, mDisplay);
    EGLint majorVersion, minorVersion;
    ASSERT_TRUE(eglInitialize(mDisplay, &majorVersion, &minorVersion));

    EGLint numConfigs;
    EGLint configAttrs[] = {EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            GetParam().majorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
                            EGL_SURFACE_TYPE,
                            EGL_WINDOW_BIT,
                            EGL_NONE};

    ASSERT_TRUE(eglChooseConfig(mDisplay, configAttrs, &mConfig, 1, &numConfigs));

    mContexts[0] = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, nullptr);
    ASSERT_NE(EGL_NO_CONTEXT, mContexts[0]);
    mContexts[1] = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, nullptr);
    ASSERT_NE(EGL_NO_CONTEXT, mContexts[1]);

    mSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), nullptr);
    ASSERT_NE(EGL_NO_SURFACE, mSurface);
    ASSERT_TRUE(eglMakeCurrent(mDisplay, mSurface, mSurface, mContexts[0]));
}

void EGLMakeCurrentPerfTest::TearDown()
{
    ANGLEPerfTest::TearDown();
    eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(mDisplay, mSurface);
    eglDestroyContext(mDisplay, mContexts[0]);
    eglDestroyContext(mDisplay, mContexts[1]);
}

void EGLMakeCurrentPerfTest::step()
{
    int mCurrContext = 0;
    for (int x = 0; x < ITERATIONS; x++)
    {
        mCurrContext = (mCurrContext + 1) % mContexts.size();
        eglMakeCurrent(mDisplay, mSurface, mSurface, mContexts[mCurrContext]);
    }
}

TEST_P(EGLMakeCurrentPerfTest, Run)
{
    run();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLMakeCurrentPerfTest);
// We want to run this test on GL(ES) and Vulkan everywhere except Android
#if !defined(ANGLE_PLATFORM_ANDROID)
ANGLE_INSTANTIATE_TEST(EGLMakeCurrentPerfTest,
                       angle::ES2_D3D11(),
                       angle::ES2_METAL(),
                       angle::ES2_OPENGL(),
                       angle::ES2_OPENGLES(),
                       angle::ES2_VULKAN());
#endif

}  // namespace
