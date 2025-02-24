//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CompositorNativeWindow11.h: Implementation of NativeWindow11 using Windows.UI.Composition APIs
// which work in both Win32 and WinRT contexts.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_COMPOSITORNATIVEWINDOW11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_COMPOSITORNATIVEWINDOW11_H_

#include "libANGLE/renderer/d3d/d3d11/NativeWindow11.h"

#include <dispatcherqueue.h>
#include <windows.foundation.metadata.h>
#include <windows.ui.composition.h>
#include <windows.ui.composition.interop.h>
#include <wrl.h>

namespace rx
{

class RoHelper
{
  public:
    RoHelper();
    ~RoHelper();
    bool WinRtAvailable() const;
    bool SupportedWindowsRelease();
    HRESULT GetStringReference(PCWSTR source, HSTRING *act, HSTRING_HEADER *header);
    HRESULT GetActivationFactory(const HSTRING act, const IID &interfaceId, void **fac);
    HRESULT WindowsCompareStringOrdinal(HSTRING one, HSTRING two, int *result);
    HRESULT CreateDispatcherQueueController(
        DispatcherQueueOptions options,
        ABI::Windows::System::IDispatcherQueueController **dispatcherQueueController);
    HRESULT WindowsDeleteString(HSTRING one);
    HRESULT RoInitialize(RO_INIT_TYPE type);
    void RoUninitialize();

  private:
    using WindowsCreateStringReference_ = HRESULT __stdcall(PCWSTR,
                                                            UINT32,
                                                            HSTRING_HEADER *,
                                                            HSTRING *);

    using GetActivationFactory_ = HRESULT __stdcall(HSTRING, REFIID, void **);

    using WindowsCompareStringOrginal_ = HRESULT __stdcall(HSTRING, HSTRING, int *);

    using WindowsDeleteString_ = HRESULT __stdcall(HSTRING);

    using CreateDispatcherQueueController_ =
        HRESULT __stdcall(DispatcherQueueOptions,
                          ABI::Windows::System::IDispatcherQueueController **);

    using RoInitialize_   = HRESULT __stdcall(RO_INIT_TYPE);
    using RoUninitialize_ = void __stdcall();

    WindowsCreateStringReference_ *mFpWindowsCreateStringReference;
    GetActivationFactory_ *mFpGetActivationFactory;
    WindowsCompareStringOrginal_ *mFpWindowsCompareStringOrdinal;
    CreateDispatcherQueueController_ *mFpCreateDispatcherQueueController;
    WindowsDeleteString_ *mFpWindowsDeleteString;
    RoInitialize_ *mFpRoInitialize;
    RoUninitialize_ *mFpRoUninitialize;

    bool mWinRtAvailable;
    bool mWinRtInitialized;

    HMODULE mComBaseModule;
    HMODULE mCoreMessagingModule;
};

class CompositorNativeWindow11 : public NativeWindow11
{
  public:
    CompositorNativeWindow11(EGLNativeWindowType window, bool hasAlpha);
    ~CompositorNativeWindow11() override;

    bool initialize() override;
    bool getClientRect(LPRECT rect) const override;
    bool isIconic() const override;

    HRESULT createSwapChain(ID3D11Device *device,
                            IDXGIFactory *factory,
                            DXGI_FORMAT format,
                            UINT width,
                            UINT height,
                            UINT samples,
                            IDXGISwapChain **swapChain) override;

    void commitChange() override;

    static bool IsValidNativeWindow(EGLNativeWindowType window);

    static bool IsSupportedWinRelease();

  private:
    static bool IsSpriteVisual(EGLNativeWindowType window);

    bool mHasAlpha;

    RoHelper mRoHelper;

    // Namespace prefix required here for some reason despite using namespace
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ISpriteVisual> mHostVisual;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ICompositionBrush> mCompositionBrush;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ICompositionSurface> mSurface;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ICompositionSurfaceBrush> mSurfaceBrush;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_CONVERGED_COMPOSITORNATIVEWINDOW11_H_
