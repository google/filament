//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLSyncControlTest.cpp:
//   Tests pertaining to eglGetSyncValuesCHROMIUM.

#include <d3d11.h>

#include "test_utils/ANGLETest.h"
#include "util/OSWindow.h"
#include "util/com_utils.h"

using namespace angle;

class EGLSyncControlTest : public testing::Test
{
  protected:
    EGLSyncControlTest() {}

    void SetUp() override
    {
        mD3D11Module = LoadLibrary(TEXT("d3d11.dll"));
        if (mD3D11Module == nullptr)
        {
            std::cout << "Unable to LoadLibrary D3D11" << std::endl;
            return;
        }

        mD3D11CreateDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(
            GetProcAddress(mD3D11Module, "D3D11CreateDevice"));
        if (mD3D11CreateDevice == nullptr)
        {
            std::cout << "Could not retrieve D3D11CreateDevice from d3d11.dll" << std::endl;
            return;
        }

        mD3D11Available = true;

        const char *extensionString =
            static_cast<const char *>(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS));
        if (strstr(extensionString, "EGL_ANGLE_device_creation"))
        {
            if (strstr(extensionString, "EGL_ANGLE_device_creation_d3d11"))
            {
                mDeviceCreationD3D11ExtAvailable = true;
            }
        }
    }

    void TearDown() override
    {
        SafeRelease(mDevice);
        SafeRelease(mDeviceContext);

        OSWindow::Delete(&mOSWindow);

        if (mSurface != EGL_NO_SURFACE)
        {
            eglDestroySurface(mDisplay, mSurface);
            mSurface = EGL_NO_SURFACE;
        }

        if (mContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(mDisplay, mContext);
            mContext = EGL_NO_CONTEXT;
        }

        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
        }
    }

    void CreateD3D11Device()
    {
        ASSERT_TRUE(mD3D11Available);
        ASSERT_EQ(nullptr, mDevice);  // The device shouldn't be created twice

        HRESULT hr =
            mD3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0, nullptr, 0,
                               D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mDeviceContext);

        ASSERT_TRUE(SUCCEEDED(hr));
    }

    void InitializeDisplay()
    {
        EGLint displayAttribs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                                   EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                                   EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE,
                                   EGL_DONT_CARE,
                                   EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE,
                                   EGL_DONT_CARE,
                                   EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE,
                                   EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
                                   EGL_NONE};

        // Create an OS Window
        mOSWindow = OSWindow::New();
        mOSWindow->initialize("EGLSyncControlTest", 64, 64);
        mOSWindow->setVisible(true);

        // Create an EGLDisplay using the EGLDevice
        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                            reinterpret_cast<void *>(mOSWindow->getNativeDisplay()),
                                            displayAttribs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        EGLint majorVersion, minorVersion;
        ASSERT_TRUE(eglInitialize(mDisplay, &majorVersion, &minorVersion) == EGL_TRUE);
    }

    void CreateWindowSurface()
    {
        eglBindAPI(EGL_OPENGL_ES_API);
        ASSERT_EGL_SUCCESS();

        // Choose a config
        const EGLint configAttributes[] = {EGL_NONE};

        EGLint configCount = 0;
        ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, configAttributes, &mConfig, 1, &configCount));

        const EGLint surfaceAttributes[] = {EGL_DIRECT_COMPOSITION_ANGLE, EGL_TRUE, EGL_NONE};

        // Create window surface
        mSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(),
                                          surfaceAttributes);
        if (mSurface == nullptr)
        {
            std::cout << "Unable to create window surface with Direct Composition" << std::endl;
            return;
        }

        mDirectCompositionSurfaceAvailable = true;

        // Create EGL context
        EGLint contextAttibutes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

        mContext = eglCreateContext(mDisplay, mConfig, nullptr, contextAttibutes);
        ASSERT_EGL_SUCCESS();

        // Make the surface current
        eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
        ASSERT_EGL_SUCCESS();
    }

    bool mD3D11Available                       = false;
    HMODULE mD3D11Module                       = nullptr;
    PFN_D3D11_CREATE_DEVICE mD3D11CreateDevice = nullptr;

    ID3D11Device *mDevice               = nullptr;
    ID3D11DeviceContext *mDeviceContext = nullptr;
    D3D_FEATURE_LEVEL mFeatureLevel;

    bool mDeviceCreationD3D11ExtAvailable = false;

    bool mDirectCompositionSurfaceAvailable = false;

    OSWindow *mOSWindow = nullptr;

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLSurface mSurface = EGL_NO_SURFACE;
    EGLContext mContext = EGL_NO_CONTEXT;
    EGLConfig mConfig   = 0;
};

// Basic test for eglGetSyncValuesCHROMIUM extension. Verifies that eglGetSyncValuesCHROMIUM
// can be called on DX11 with direct composition and that it returns reasonable enough values.
TEST_F(EGLSyncControlTest, DISABLED_SyncValuesTest)
{
    static const DWORD kPollInterval    = 10;
    static const int kNumPollIterations = 100;

    if (!mD3D11Available)
    {
        std::cout << "D3D11 not available, skipping test" << std::endl;
        return;
    }

    CreateD3D11Device();
    InitializeDisplay();
    CreateWindowSurface();

    if (!mDirectCompositionSurfaceAvailable)
    {
        std::cout << "Direct Composition surface not available, skipping test" << std::endl;
        return;
    }

    const char *extensionString =
        static_cast<const char *>(eglQueryString(mDisplay, EGL_EXTENSIONS));
    ASSERT_TRUE(strstr(extensionString, "EGL_CHROMIUM_sync_control"));

    EGLuint64KHR ust = 0, msc = 0, sbc = 0;
    // It appears there is a race condition so the very first call to eglGetSyncValuesCHROMIUM
    // can fail within D3D with DXGI_ERROR_FRAME_STATISTICS_DISJOINT.
    // Should that be handled inside eglGetSyncValuesCHROMIUM?
    eglGetSyncValuesCHROMIUM(mDisplay, mSurface, &ust, &msc, &sbc);

    ASSERT_EGL_TRUE(eglGetSyncValuesCHROMIUM(mDisplay, mSurface, &ust, &msc, &sbc));
    // Initial msc and sbc value should be true. Initial ust value is unspecified.
    ASSERT_EQ(0ull, msc);
    ASSERT_EQ(0ull, sbc);

    // Perform some very basic rendering.
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_TRUE(eglSwapBuffers(mDisplay, mSurface));

    // Poll until sbc value increases. Normally it should change within 16-17 ms.
    for (int i = 0; i < kNumPollIterations; i++)
    {
        ::Sleep(kPollInterval);
        ASSERT_EGL_TRUE(eglGetSyncValuesCHROMIUM(mDisplay, mSurface, &ust, &msc, &sbc));
        if (sbc > 0)
            break;
    }

    // sbc should change to 1. msc and ust to some non-zero values.
    ASSERT_EQ(1ull, sbc);
    ASSERT_GT(ust, 0ull);
    ASSERT_GT(msc, 0ull);

    // Perform more rendering.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_TRUE(eglSwapBuffers(mDisplay, mSurface));

    // Poll until sbc value increases. Normally it should change within 16-17 ms.
    EGLuint64KHR ust2 = 0, msc2 = 0, sbc2 = 0;
    for (int i = 0; i < kNumPollIterations; i++)
    {
        ::Sleep(kPollInterval);
        ASSERT_EGL_TRUE(eglGetSyncValuesCHROMIUM(mDisplay, mSurface, &ust2, &msc2, &sbc2));
        if (sbc2 > sbc)
            break;
    }

    // sbc2 should be 2. msc2 and ust2 should be greater than previous msc and ust values.
    ASSERT_EQ(2ull, sbc2);
    ASSERT_GT(ust2, ust);
    ASSERT_GT(msc2, msc);
}
