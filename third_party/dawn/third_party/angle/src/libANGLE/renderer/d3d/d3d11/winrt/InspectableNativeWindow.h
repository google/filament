//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// InspectableNativeWindow.h: Host specific implementation interface for
// managing IInspectable native window types.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_WINRT_INSPECTABLENATIVEWINDOW_H_
#define LIBANGLE_RENDERER_D3D_D3D11_WINRT_INSPECTABLENATIVEWINDOW_H_

#include "common/debug.h"
#include "common/platform.h"

#include "angle_windowsstore.h"

#include <EGL/eglplatform.h>

#include <windows.applicationmodel.core.h>
#undef GetCurrentTime
#if defined(ANGLE_ENABLE_WINDOWS_APP_SDK)
#    include <microsoft.ui.dispatching.h>
#    include <microsoft.ui.xaml.h>
#    include <microsoft.ui.xaml.media.dxinterop.h>
#else
#    include <windows.ui.xaml.h>
#    include <windows.ui.xaml.media.dxinterop.h>
#endif
#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;

#if defined(ANGLE_ENABLE_WINDOWS_APP_SDK)
using ISwapChainPanel = ABI::Microsoft::UI::Xaml::Controls::ISwapChainPanel;
#else
using ISwapChainPanel = ABI::Windows::UI::Xaml::Controls::ISwapChainPanel;
#endif

namespace rx
{
class InspectableNativeWindow
{
  public:
    InspectableNativeWindow()
        : mSupportsSwapChainResize(true),
          mSwapChainSizeSpecified(false),
          mSwapChainScaleSpecified(false),
          mSwapChainScale(1.0f),
          mClientRectChanged(false),
          mClientRect({0, 0, 0, 0}),
          mNewClientRect({0, 0, 0, 0})
    {
        mSizeChangedEventToken.value = 0;
    }
    virtual ~InspectableNativeWindow() {}

    virtual bool initialize(EGLNativeWindowType window, IPropertySet *propertySet) = 0;
    virtual HRESULT createSwapChain(ID3D11Device *device,
                                    IDXGIFactory2 *factory,
                                    DXGI_FORMAT format,
                                    unsigned int width,
                                    unsigned int height,
                                    bool containsAlpha,
                                    IDXGISwapChain1 **swapChain)                   = 0;

    bool getClientRect(RECT *rect)
    {
        if (mClientRectChanged)
        {
            mClientRect = mNewClientRect;
        }

        *rect = mClientRect;

        return true;
    }

    // setNewClientSize is used by the WinRT size change handler. It isn't used by the rest of
    // ANGLE.
    void setNewClientSize(const Size &newWindowSize)
    {
        // If the client doesn't support swapchain resizing then we should have already unregistered
        // from size change handler
        ASSERT(mSupportsSwapChainResize);

        if (mSupportsSwapChainResize)
        {
            // If the swapchain size was specified then we should ignore this call too
            if (!mSwapChainSizeSpecified)
            {
                mNewClientRect     = clientRect(newWindowSize);
                mClientRectChanged = true;

                // If a scale was specified, then now is the time to apply the scale matrix for the
                // new swapchain size and window size
                if (mSwapChainScaleSpecified)
                {
                    scaleSwapChain(newWindowSize, mNewClientRect);
                }
            }

            // Even if the swapchain size was fixed, the window might have changed size.
            // In this case, we should recalculate the scale matrix to account for the new window
            // size
            if (mSwapChainSizeSpecified)
            {
                scaleSwapChain(newWindowSize, mClientRect);
            }
        }
    }

  protected:
    virtual HRESULT scaleSwapChain(const Size &windowSize, const RECT &clientRect) = 0;
    RECT clientRect(const Size &size);

    bool mSupportsSwapChainResize;  // Support for IDXGISwapChain::ResizeBuffers method
    bool mSwapChainSizeSpecified;   // If an EGLRenderSurfaceSizeProperty was specified
    bool mSwapChainScaleSpecified;  // If an EGLRenderResolutionScaleProperty was specified
    float mSwapChainScale;  // The scale value specified by the EGLRenderResolutionScaleProperty
                            // property
    RECT mClientRect;
    RECT mNewClientRect;
    bool mClientRectChanged;

    EventRegistrationToken mSizeChangedEventToken;
};

bool IsCoreWindow(EGLNativeWindowType window,
                  ComPtr<ABI::Windows::UI::Core::ICoreWindow> *coreWindow = nullptr);
bool IsSwapChainPanel(EGLNativeWindowType window,
                      ComPtr<ISwapChainPanel> *swapChainPanel = nullptr);
bool IsEGLConfiguredPropertySet(
    EGLNativeWindowType window,
    ABI::Windows::Foundation::Collections::IPropertySet **propertySet = nullptr,
    IInspectable **inspectable                                        = nullptr);

HRESULT GetOptionalPropertyValue(
    const ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> &propertyMap,
    const wchar_t *propertyName,
    boolean *hasKey,
    ComPtr<ABI::Windows::Foundation::IPropertyValue> &propertyValue);

HRESULT GetOptionalSizePropertyValue(
    const ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> &propertyMap,
    const wchar_t *propertyName,
    SIZE *value,
    bool *valueExists);

HRESULT GetOptionalSinglePropertyValue(
    const ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> &propertyMap,
    const wchar_t *propertyName,
    float *value,
    bool *valueExists);
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_WINRT_INSPECTABLENATIVEWINDOW_H_
