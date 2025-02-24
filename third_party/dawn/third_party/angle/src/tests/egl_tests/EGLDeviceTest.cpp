//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_ENABLE_D3D9
#    define ANGLE_ENABLE_D3D9
#endif

#ifndef ANGLE_ENABLE_D3D11
#    define ANGLE_ENABLE_D3D11
#endif

#include <d3d11.h>

#include "test_utils/ANGLETest.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"
#include "util/com_utils.h"
#include "util/gles_loader_autogen.h"

using namespace angle;

class EGLDeviceCreationTest : public ANGLETest<>
{
  protected:
    EGLDeviceCreationTest()
        : mD3D11Module(nullptr),
          mD3D11CreateDevice(nullptr),
          mDevice(nullptr),
          mDeviceContext(nullptr),
          mDeviceCreationD3D11ExtAvailable(false),
          mOSWindow(nullptr),
          mDisplay(EGL_NO_DISPLAY),
          mSurface(EGL_NO_SURFACE),
          mContext(EGL_NO_CONTEXT),
          mConfig(0)
    {}

    void testSetUp() override
    {
        ASSERT_TRUE(isD3D11Renderer());

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

    void testTearDown() override
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
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
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
        ASSERT_EQ(nullptr, mDevice);  // The device shouldn't be created twice

        HRESULT hr =
            mD3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0, nullptr, 0,
                               D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mDeviceContext);

        ASSERT_TRUE(SUCCEEDED(hr));
        ASSERT_GE(mFeatureLevel, D3D_FEATURE_LEVEL_9_3);
    }

    void CreateWindowSurface()
    {
        EGLint majorVersion, minorVersion;
        ASSERT_EGL_TRUE(eglInitialize(mDisplay, &majorVersion, &minorVersion));

        eglBindAPI(EGL_OPENGL_ES_API);
        ASSERT_EGL_SUCCESS();

        // Choose a config
        const EGLint configAttributes[] = {EGL_NONE};
        EGLint configCount              = 0;
        ASSERT_EGL_TRUE(eglChooseConfig(mDisplay, configAttributes, &mConfig, 1, &configCount));

        // Create an OS Window
        mOSWindow = OSWindow::New();
        mOSWindow->initialize("EGLSurfaceTest", 64, 64);

        // Create window surface
        mSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), nullptr);
        ASSERT_EGL_SUCCESS();

        // Create EGL context
        EGLint contextAttibutes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        mContext                  = eglCreateContext(mDisplay, mConfig, nullptr, contextAttibutes);
        ASSERT_EGL_SUCCESS();

        // Make the surface current
        eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
        ASSERT_EGL_SUCCESS();
    }

    // This triggers a D3D device lost on current Windows systems
    // This behavior could potentially change in the future
    void trigger9_3DeviceLost()
    {
        ID3D11Buffer *gsBuffer       = nullptr;
        D3D11_BUFFER_DESC bufferDesc = {0};
        bufferDesc.ByteWidth         = 64;
        bufferDesc.Usage             = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;

        HRESULT result = mDevice->CreateBuffer(&bufferDesc, nullptr, &gsBuffer);
        ASSERT_TRUE(SUCCEEDED(result));

        mDeviceContext->GSSetConstantBuffers(0, 1, &gsBuffer);
        SafeRelease(gsBuffer);
        gsBuffer = nullptr;

        result = mDevice->GetDeviceRemovedReason();
        ASSERT_TRUE(FAILED(result));
    }

    HMODULE mD3D11Module;
    PFN_D3D11_CREATE_DEVICE mD3D11CreateDevice;

    ID3D11Device *mDevice;
    ID3D11DeviceContext *mDeviceContext;
    D3D_FEATURE_LEVEL mFeatureLevel;

    bool mDeviceCreationD3D11ExtAvailable;

    OSWindow *mOSWindow;

    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    EGLConfig mConfig;
};

// Test that creating a EGLDeviceEXT from D3D11 device works, and it can be queried to retrieve
// D3D11 device
TEST_P(EGLDeviceCreationTest, BasicD3D11Device)
{
    ANGLE_SKIP_TEST_IF(!mDeviceCreationD3D11ExtAvailable);

    CreateD3D11Device();

    EGLDeviceEXT eglDevice =
        eglCreateDeviceANGLE(EGL_D3D11_DEVICE_ANGLE, reinterpret_cast<void *>(mDevice), nullptr);
    ASSERT_NE(EGL_NO_DEVICE_EXT, eglDevice);
    ASSERT_EGL_SUCCESS();

    EGLAttrib deviceAttrib;
    eglQueryDeviceAttribEXT(eglDevice, EGL_D3D11_DEVICE_ANGLE, &deviceAttrib);
    ASSERT_EGL_SUCCESS();

    ID3D11Device *queriedDevice = reinterpret_cast<ID3D11Device *>(deviceAttrib);
    ASSERT_EQ(mFeatureLevel, queriedDevice->GetFeatureLevel());

    eglReleaseDeviceANGLE(eglDevice);
}

// Test that creating a EGLDeviceEXT from D3D11 device works, and it can be queried to retrieve
// D3D11 device
TEST_P(EGLDeviceCreationTest, BasicD3D11DeviceViaFuncPointer)
{
    ANGLE_SKIP_TEST_IF(!mDeviceCreationD3D11ExtAvailable);

    CreateD3D11Device();

    EGLDeviceEXT eglDevice =
        eglCreateDeviceANGLE(EGL_D3D11_DEVICE_ANGLE, reinterpret_cast<void *>(mDevice), nullptr);
    ASSERT_NE(EGL_NO_DEVICE_EXT, eglDevice);
    ASSERT_EGL_SUCCESS();

    EGLAttrib deviceAttrib;
    eglQueryDeviceAttribEXT(eglDevice, EGL_D3D11_DEVICE_ANGLE, &deviceAttrib);
    ASSERT_EGL_SUCCESS();

    ID3D11Device *queriedDevice = reinterpret_cast<ID3D11Device *>(deviceAttrib);
    ASSERT_EQ(mFeatureLevel, queriedDevice->GetFeatureLevel());

    eglReleaseDeviceANGLE(eglDevice);
}

// Test that creating a EGLDeviceEXT from D3D11 device works, and can be used for rendering
TEST_P(EGLDeviceCreationTest, RenderingUsingD3D11Device)
{
    CreateD3D11Device();

    EGLDeviceEXT eglDevice =
        eglCreateDeviceANGLE(EGL_D3D11_DEVICE_ANGLE, reinterpret_cast<void *>(mDevice), nullptr);
    ASSERT_EGL_SUCCESS();

    // Create an EGLDisplay using the EGLDevice
    mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevice, nullptr);
    ASSERT_NE(EGL_NO_DISPLAY, mDisplay);

    // Create a surface using the display
    CreateWindowSurface();

    // Perform some very basic rendering
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(32, 32, 255, 0, 255, 255);

    // Note that we must call TearDown() before we release the EGL device, since the display
    // depends on the device
    ASSERT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    testTearDown();

    eglReleaseDeviceANGLE(eglDevice);
}

// Test that calling eglGetPlatformDisplayEXT with the same device returns the same display
TEST_P(EGLDeviceCreationTest, GetPlatformDisplayTwice)
{
    CreateD3D11Device();

    EGLDeviceEXT eglDevice =
        eglCreateDeviceANGLE(EGL_D3D11_DEVICE_ANGLE, reinterpret_cast<void *>(mDevice), nullptr);
    ASSERT_EGL_SUCCESS();

    // Create an EGLDisplay using the EGLDevice
    EGLDisplay display1 = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevice, nullptr);
    ASSERT_NE(EGL_NO_DISPLAY, display1);

    EGLDisplay display2 = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevice, nullptr);
    ASSERT_NE(EGL_NO_DISPLAY, display2);

    ASSERT_EQ(display1, display2);

    eglTerminate(display1);
    eglReleaseDeviceANGLE(eglDevice);
}

// Test that creating a EGLDeviceEXT from an invalid D3D11 device fails
TEST_P(EGLDeviceCreationTest, InvalidD3D11Device)
{
    ANGLE_SKIP_TEST_IF(!mDeviceCreationD3D11ExtAvailable);

    CreateD3D11Device();

    // Use mDeviceContext instead of mDevice
    EGLDeviceEXT eglDevice = eglCreateDeviceANGLE(
        EGL_D3D11_DEVICE_ANGLE, reinterpret_cast<void *>(mDeviceContext), nullptr);
    EXPECT_EQ(EGL_NO_DEVICE_EXT, eglDevice);
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
}

// Test that EGLDeviceEXT holds a ref to the D3D11 device
TEST_P(EGLDeviceCreationTest, D3D11DeviceReferenceCounting)
{
    ANGLE_SKIP_TEST_IF(!mDeviceCreationD3D11ExtAvailable);

    CreateD3D11Device();

    EGLDeviceEXT eglDevice =
        eglCreateDeviceANGLE(EGL_D3D11_DEVICE_ANGLE, reinterpret_cast<void *>(mDevice), nullptr);
    ASSERT_NE(EGL_NO_DEVICE_EXT, eglDevice);
    ASSERT_EGL_SUCCESS();

    // Now release our D3D11 device/context
    SafeRelease(mDevice);
    SafeRelease(mDeviceContext);

    EGLAttrib deviceAttrib;
    eglQueryDeviceAttribEXT(eglDevice, EGL_D3D11_DEVICE_ANGLE, &deviceAttrib);
    ASSERT_EGL_SUCCESS();

    ID3D11Device *queriedDevice = reinterpret_cast<ID3D11Device *>(deviceAttrib);
    ASSERT_EQ(mFeatureLevel, queriedDevice->GetFeatureLevel());

    eglReleaseDeviceANGLE(eglDevice);
}

// Test that creating a EGLDeviceEXT from a D3D9 device fails
TEST_P(EGLDeviceCreationTest, AnyD3D9Device)
{
    ANGLE_SKIP_TEST_IF(!mDeviceCreationD3D11ExtAvailable);

    std::string fakeD3DDevice = "This is a string, not a D3D device";

    EGLDeviceEXT eglDevice = eglCreateDeviceANGLE(
        EGL_D3D9_DEVICE_ANGLE, reinterpret_cast<void *>(&fakeD3DDevice), nullptr);
    EXPECT_EQ(EGL_NO_DEVICE_EXT, eglDevice);
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
}

class EGLDeviceQueryTest : public ANGLETest<>
{
  protected:
    EGLDeviceQueryTest() {}

    void testSetUp() override
    {
        if (!eglQueryDeviceStringEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDeviceStringEXT was not "
                      "found";
        }

        if (!eglQueryDisplayAttribEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDisplayAttribEXT was "
                      "not found";
        }

        if (!eglQueryDeviceAttribEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDeviceAttribEXT was not "
                      "found";
        }

        EGLAttrib angleDevice = 0;
        EXPECT_EGL_TRUE(
            eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));
        if (!IsEGLDeviceExtensionEnabled(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                         "EGL_ANGLE_device_d3d9") &&
            !IsEGLDeviceExtensionEnabled(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                         "EGL_ANGLE_device_d3d11"))
        {
            FAIL() << "ANGLE extensions EGL_ANGLE_device_d3d9 or EGL_ANGLE_device_d3d11 were not "
                      "found";
        }
    }
};

// This test attempts to obtain a D3D11 device and a D3D9 device using the eglQueryDeviceAttribEXT
// function.
// If the test is configured to use D3D11 then it should succeed to obtain a D3D11 device.
// If the test is confitured to use D3D9, then it should succeed to obtain a D3D9 device.
TEST_P(EGLDeviceQueryTest, QueryDevice)
{
    EGLAttrib angleDevice = 0;
    EXPECT_EGL_TRUE(
        eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));

    if (IsEGLDeviceExtensionEnabled(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                    "EGL_ANGLE_device_d3d11"))
    {
        EGLAttrib device11 = 0;
        EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                EGL_D3D11_DEVICE_ANGLE, &device11));
        ID3D11Device *d3d11Device = reinterpret_cast<ID3D11Device *>(device11);
        IDXGIDevice *dxgiDevice   = DynamicCastComObject<IDXGIDevice>(d3d11Device);
        EXPECT_TRUE(dxgiDevice != nullptr);
        SafeRelease(dxgiDevice);
    }

    if (IsEGLDeviceExtensionEnabled(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                    "EGL_ANGLE_device_d3d9"))
    {
        EGLAttrib device9 = 0;
        EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                EGL_D3D9_DEVICE_ANGLE, &device9));
        IDirect3DDevice9 *d3d9Device = reinterpret_cast<IDirect3DDevice9 *>(device9);
        IDirect3D9 *d3d9             = nullptr;
        EXPECT_EQ(S_OK, d3d9Device->GetDirect3D(&d3d9));
        EXPECT_TRUE(d3d9 != nullptr);
        SafeRelease(d3d9);
    }
}

// This test attempts to obtain a D3D11 device from a D3D9 configured system and a D3D9 device from
// a D3D11 configured system using the eglQueryDeviceAttribEXT function.
// If the test is configured to use D3D11 then it should fail to obtain a D3D11 device.
// If the test is confitured to use D3D9, then it should fail to obtain a D3D9 device.
TEST_P(EGLDeviceQueryTest, QueryDeviceBadAttribute)
{
    EGLAttrib angleDevice = 0;
    EXPECT_EGL_TRUE(
        eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));

    if (!IsEGLDeviceExtensionEnabled(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                     "EGL_ANGLE_device_d3d11"))
    {
        EGLAttrib device11 = 0;
        EXPECT_EGL_FALSE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                 EGL_D3D11_DEVICE_ANGLE, &device11));
    }

    if (!IsEGLDeviceExtensionEnabled(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                     "EGL_ANGLE_device_d3d9"))
    {
        EGLAttrib device9 = 0;
        EXPECT_EGL_FALSE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                                 EGL_D3D9_DEVICE_ANGLE, &device9));
    }
}

// Ensure that:
//    - calling getPlatformDisplayEXT using ANGLE_Platform with some parameters
//    - extracting the EGLDeviceEXT from the EGLDisplay
//    - calling getPlatformDisplayEXT with this EGLDeviceEXT
// results in the same EGLDisplay being returned from getPlatformDisplayEXT both times
TEST_P(EGLDeviceQueryTest, GetPlatformDisplayDeviceReuse)
{
    EGLAttrib eglDevice = 0;
    EXPECT_EGL_TRUE(
        eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &eglDevice));

    EGLDisplay display2 = eglGetPlatformDisplayEXT(
        EGL_PLATFORM_DEVICE_EXT, reinterpret_cast<EGLDeviceEXT>(eglDevice), nullptr);
    EXPECT_EQ(getEGLWindow()->getDisplay(), display2);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(EGLDeviceCreationTest, WithNoFixture(ES2_D3D11()));
ANGLE_INSTANTIATE_TEST(EGLDeviceQueryTest, ES2_D3D9(), ES2_D3D11());
