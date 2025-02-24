//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLDirectCompositionTest.cpp:
//   Tests pertaining to DirectComposition and WindowsUIComposition.

#ifdef ANGLE_ENABLE_D3D11_COMPOSITOR_NATIVE_WINDOW

#    include <d3d11.h>
#    include "test_utils/ANGLETest.h"

#    include <DispatcherQueue.h>
#    include <VersionHelpers.h>
#    include <Windows.Foundation.h>
#    include <windows.ui.composition.Desktop.h>
#    include <windows.ui.composition.h>
#    include <windows.ui.composition.interop.h>
#    include <wrl.h>
#    include <memory>

#    include "libANGLE/renderer/d3d/d3d11/converged/CompositorNativeWindow11.h"
#    include "util/OSWindow.h"
#    include "util/com_utils.h"
#    include "util/test_utils.h"

using namespace angle;
using namespace ABI::Windows::System;
using namespace ABI::Windows::UI::Composition;
using namespace ABI::Windows::UI::Composition::Desktop;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

const int WINDOWWIDTH = 200, WINDOWHEIGHT = 200;

class EGLDirectCompositionTest : public ANGLETest<>
{
  protected:
    EGLDirectCompositionTest() : mOSWindow(nullptr) {}

    void testSetUp() override
    {
        if (!mRoHelper.SupportedWindowsRelease())
        {
            return;
        }

        // Create an OS Window
        mOSWindow = OSWindow::New();

        mOSWindow->initialize("EGLDirectCompositionTest", WINDOWWIDTH, WINDOWHEIGHT);
        auto nativeWindow = mOSWindow->getNativeWindow();
        setWindowVisible(mOSWindow, true);

        // Create DispatcherQueue for window to process compositor callbacks
        CreateDispatcherQueue(mDispatcherController);

        HSTRING act;
        HSTRING_HEADER header;

        auto hr = mRoHelper.GetStringReference(RuntimeClass_Windows_UI_Composition_Compositor, &act,
                                               &header);

        ASSERT_TRUE(SUCCEEDED(hr));

        void *fac = nullptr;
        hr        = mRoHelper.GetActivationFactory(act, __uuidof(IActivationFactory), &fac);
        ASSERT_TRUE(SUCCEEDED(hr));

        ComPtr<IActivationFactory> compositorFactory;

        compositorFactory.Attach((IActivationFactory *)fac);

        hr = compositorFactory->ActivateInstance(&mCompositor);
        ASSERT_TRUE(SUCCEEDED(hr));

        // Create a DesktopWindowTarget against native window (HWND)
        CreateDesktopWindowTarget(mCompositor, static_cast<HWND>(nativeWindow), mDesktopTarget);

        ASSERT_TRUE(SUCCEEDED(mCompositor->CreateSpriteVisual(mAngleHost.GetAddressOf())));

        ComPtr<IVisual> angleVis;
        ASSERT_TRUE(SUCCEEDED(mAngleHost.As(&angleVis)));

        ASSERT_TRUE(SUCCEEDED(angleVis->put_Size(
            {static_cast<FLOAT>(WINDOWWIDTH), static_cast<FLOAT>(WINDOWHEIGHT)})));

        ASSERT_TRUE(SUCCEEDED(angleVis->put_Offset({0, 0, 0})));

        ComPtr<ICompositionTarget> compTarget;
        ASSERT_TRUE(SUCCEEDED(mDesktopTarget.As(&compTarget)));
        ASSERT_TRUE(SUCCEEDED(compTarget->put_Root(angleVis.Get())));

        Init();
    }

    void CreateDispatcherQueue(ComPtr<IDispatcherQueueController> &controller)
    {
        DispatcherQueueOptions options{sizeof(DispatcherQueueOptions), DQTYPE_THREAD_CURRENT,
                                       DQTAT_COM_STA};

        auto hr = mRoHelper.CreateDispatcherQueueController(options, controller.GetAddressOf());

        ASSERT_TRUE(SUCCEEDED(hr));
    }

    void CreateDesktopWindowTarget(ComPtr<ICompositor> const &compositor,
                                   const HWND window,
                                   ComPtr<IDesktopWindowTarget> &target)
    {
        namespace abi = ABI::Windows::UI::Composition::Desktop;

        ComPtr<ICompositorDesktopInterop> interop;
        ASSERT_TRUE(SUCCEEDED(compositor.As(&interop)));

        ASSERT_TRUE(SUCCEEDED(interop->CreateDesktopWindowTarget(
            window, true, reinterpret_cast<abi::IDesktopWindowTarget **>(target.GetAddressOf()))));
    }

    void Init()
    {
        if (!mRoHelper.SupportedWindowsRelease())
        {
            return;
        }

        DPI_AWARENESS_CONTEXT
        WINAPI
        SetThreadDpiAwarenessContext(_In_ DPI_AWARENESS_CONTEXT dpiContext);

        auto userModule = LoadLibraryA("user32.dll");

        if (userModule == nullptr)
        {
            return;
        }

        auto temp = GetProcAddress(userModule, "SetThreadDpiAwarenessContext");

        mFpSetThreadDpiAwarenessContext = reinterpret_cast<_SetThreadDpiAwarenessContext *>(temp);

        const EGLint configAttributes[] = {
            EGL_RED_SIZE,   8, EGL_GREEN_SIZE,   8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 8, EGL_STENCIL_SIZE, 8, EGL_NONE};

        const EGLint defaultDisplayAttributes[] = {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE,
            EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_NONE,
        };

        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
            reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
                eglGetProcAddress("eglGetPlatformDisplayEXT"));
        ASSERT_TRUE(eglGetPlatformDisplayEXT != nullptr);

        mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                               reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY),
                                               defaultDisplayAttributes);
        ASSERT_TRUE(mEglDisplay != EGL_NO_DISPLAY);

        ASSERT_EGL_TRUE(eglInitialize(mEglDisplay, nullptr, nullptr));

        EGLint nConfigs = 0;

        ASSERT_EGL_TRUE(eglGetConfigs(mEglDisplay, nullptr, 0, &nConfigs));
        ASSERT_TRUE(nConfigs != 0);

        ASSERT_EGL_TRUE(eglChooseConfig(mEglDisplay, configAttributes, &mEglConfig, 1, &nConfigs));
    }

    void CreateSurface(ComPtr<ABI::Windows::UI::Composition::ISpriteVisual> visual,
                       EGLSurface &surface)
    {
        auto displayExtensions = eglQueryString(mEglDisplay, EGL_EXTENSIONS);

        // Check that the EGL_ANGLE_windows_ui_composition display extension is available
        ASSERT_TRUE(strstr(displayExtensions, "EGL_ANGLE_windows_ui_composition") != nullptr);

        const EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

        // Use a spritevisual as the nativewindowtype
        surface =
            eglCreateWindowSurface(mEglDisplay, mEglConfig,
                                   static_cast<EGLNativeWindowType>((void *)visual.Get()), nullptr);
        ASSERT_TRUE(surface != EGL_NO_SURFACE);

        mEglContext = eglCreateContext(mEglDisplay, mEglConfig, EGL_NO_CONTEXT, contextAttributes);
        ASSERT_TRUE(mEglContext != EGL_NO_CONTEXT);

        ASSERT_TRUE(eglMakeCurrent(mEglDisplay, surface, surface, mEglContext) != EGL_FALSE);
    }

    void testTearDown() override
    {
        if (!mRoHelper.SupportedWindowsRelease())
        {
            return;
        }
        if (mEglDisplay != EGL_NO_DISPLAY)
        {
            ASSERT_EGL_TRUE(eglTerminate(mEglDisplay));
            mEglDisplay = EGL_NO_DISPLAY;
        }

        OSWindow::Delete(&mOSWindow);
    }

    OSWindow *mOSWindow;
    ComPtr<ICompositor> mCompositor;
    ComPtr<IDispatcherQueueController> mDispatcherController;
    ComPtr<ICompositionColorBrush> mColorBrush;
    ComPtr<IDesktopWindowTarget> mDesktopTarget;
    ComPtr<ISpriteVisual> mAngleHost;

    EGLDisplay mEglDisplay;
    EGLContext mEglContext;
    EGLConfig mEglConfig;
    rx::RoHelper mRoHelper;

    using _SetThreadDpiAwarenessContext =
        DPI_AWARENESS_CONTEXT WINAPI(DPI_AWARENESS_CONTEXT dpiContext);

    _SetThreadDpiAwarenessContext *mFpSetThreadDpiAwarenessContext;
};

// This tests that a surface created using a SpriteVisual as container has the expected dimensions
// which should match the dimensions of the SpriteVisual passed in
TEST_P(EGLDirectCompositionTest, SurfaceSizeFromSpriteSize)
{
    // Only attempt this test when on Windows 10 1803+
    ANGLE_SKIP_TEST_IF(!mRoHelper.SupportedWindowsRelease());

    EGLSurface s{nullptr};
    CreateSurface(mAngleHost, s);

    EGLint surfacewidth = 0, surfaceheight = 0;
    eglQuerySurface(mEglDisplay, s, EGL_WIDTH, &surfacewidth);
    eglQuerySurface(mEglDisplay, s, EGL_HEIGHT, &surfaceheight);

    ComPtr<IVisual> angleVis;
    ASSERT_TRUE(SUCCEEDED(mAngleHost.As(&angleVis)));

    ABI::Windows::Foundation::Numerics::Vector2 visualsize{0, 0};

    ASSERT_TRUE(SUCCEEDED(angleVis->get_Size(&visualsize)));

    ASSERT_TRUE(surfacewidth == static_cast<int>(visualsize.X));
    ASSERT_TRUE(surfaceheight == static_cast<int>(visualsize.Y));

    ASSERT_TRUE(eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) !=
                EGL_FALSE);
    ASSERT_EGL_TRUE(eglDestroySurface(mEglDisplay, s));
    ASSERT_EGL_TRUE(eglDestroyContext(mEglDisplay, mEglContext));
    mEglContext = EGL_NO_CONTEXT;
}

// This tests that a WindowSurface can be created using a SpriteVisual as the containing window
// and that pixels can be successfully rendered into the resulting WindowSurface
TEST_P(EGLDirectCompositionTest, RenderSolidColor)
{
    // Only attempt this test when on Windows 10 1803+
    ANGLE_SKIP_TEST_IF(!mRoHelper.SupportedWindowsRelease());

    EGLSurface s{nullptr};
    CreateSurface(mAngleHost, s);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    glViewport(0, 0, WINDOWWIDTH, WINDOWHEIGHT);
    glClear(GL_COLOR_BUFFER_BIT);

    ASSERT_EGL_TRUE(eglSwapBuffers(mEglDisplay, s));

    // ensure user/DWM have a chance to paint the window and kick it to the top of the desktop
    // zorder before we attempt to sample
    angle::Sleep(200);
    mOSWindow->messageLoop();

    uint8_t *pixelBuffer = static_cast<uint8_t *>(malloc(WINDOWWIDTH * WINDOWHEIGHT * 4));
    ZeroMemory(pixelBuffer, WINDOWWIDTH * WINDOWHEIGHT * 4);

    // In order to accurately capture a bitmap, we need to temporarily shift into per-monitor DPI
    // mode in order to get the window offset from desktop correct
    auto previous = mFpSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    bool success  = mOSWindow->takeScreenshot(pixelBuffer);
    mFpSetThreadDpiAwarenessContext(previous);
    ASSERT_EGL_TRUE(success);

    ASSERT_EGL_TRUE(pixelBuffer[(50 * 50 * 4)] == 255);
    ASSERT_EGL_TRUE(pixelBuffer[(50 * 50 * 4) + 1] == 0);
    ASSERT_EGL_TRUE(pixelBuffer[(50 * 50 * 4) + 2] == 0);
    ASSERT_EGL_TRUE(pixelBuffer[(50 * 50 * 4) + 3] == 255);

    ASSERT_TRUE(eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) !=
                EGL_FALSE);
    ASSERT_EGL_TRUE(eglDestroySurface(mEglDisplay, s));
    ASSERT_EGL_TRUE(eglDestroyContext(mEglDisplay, mEglContext));
    mEglContext = EGL_NO_CONTEXT;
}

ANGLE_INSTANTIATE_TEST(EGLDirectCompositionTest, WithNoFixture(ES2_D3D11()));

#endif  // ANGLE_ENABLE_D3D11_COMPOSITOR_NATIVE_WINDOW
