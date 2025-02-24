//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SwapChainPanelNativeWindow.h: NativeWindow for managing ISwapChainPanel native window types.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_WINRT_SWAPCHAINPANELNATIVEWINDOW_H_
#define LIBANGLE_RENDERER_D3D_D3D11_WINRT_SWAPCHAINPANELNATIVEWINDOW_H_

#include "libANGLE/renderer/d3d/d3d11/winrt/InspectableNativeWindow.h"

#if defined(ANGLE_ENABLE_WINDOWS_APP_SDK)
using ISwapChainPanel          = ABI::Microsoft::UI::Xaml::Controls::ISwapChainPanel;
using ISizeChangedEventHandler = ABI::Microsoft::UI::Xaml::ISizeChangedEventHandler;
using ISizeChangedEventArgs    = ABI::Microsoft::UI::Xaml::ISizeChangedEventArgs;
using ICoreDispatcher          = ABI::Microsoft::UI::Dispatching::IDispatcherQueue;
using IDispatchedHandler       = ABI::Microsoft::UI::Dispatching::IDispatcherQueueHandler;
#else
using ISwapChainPanel          = ABI::Windows::UI::Xaml::Controls::ISwapChainPanel;
using ISizeChangedEventHandler = ABI::Windows::UI::Xaml::ISizeChangedEventHandler;
using ISizeChangedEventArgs    = ABI::Windows::UI::Xaml::ISizeChangedEventArgs;
using ICoreDispatcher          = ABI::Windows::UI::Core::ICoreDispatcher;
using IDispatchedHandler       = ABI::Windows::UI::Core::IDispatchedHandler;
#endif

#include <memory>

namespace rx
{
class SwapChainPanelNativeWindow : public InspectableNativeWindow,
                                   public std::enable_shared_from_this<SwapChainPanelNativeWindow>
{
  public:
    ~SwapChainPanelNativeWindow();

    bool initialize(EGLNativeWindowType window, IPropertySet *propertySet) override;
    HRESULT createSwapChain(ID3D11Device *device,
                            IDXGIFactory2 *factory,
                            DXGI_FORMAT format,
                            unsigned int width,
                            unsigned int height,
                            bool containsAlpha,
                            IDXGISwapChain1 **swapChain) override;

  protected:
    HRESULT scaleSwapChain(const Size &windowSize, const RECT &clientRect) override;

    bool registerForSizeChangeEvents();
    void unregisterForSizeChangeEvents();

  private:
    ComPtr<ISwapChainPanel> mSwapChainPanel;
    ComPtr<ICoreDispatcher> mSwapChainPanelDispatcher;
    ComPtr<IMap<HSTRING, IInspectable *>> mPropertyMap;
    ComPtr<IDXGISwapChain1> mSwapChain;
};

__declspec(uuid("8ACBD974-8187-4508-AD80-AEC77F93CF36")) class SwapChainPanelSizeChangedHandler
    : public Microsoft::WRL::RuntimeClass<
          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
          ISizeChangedEventHandler>
{
  public:
    SwapChainPanelSizeChangedHandler() {}
    HRESULT RuntimeClassInitialize(std::shared_ptr<InspectableNativeWindow> host)
    {
        if (!host)
        {
            return E_INVALIDARG;
        }

        mHost = host;
        return S_OK;
    }

    // ISizeChangedEventHandler
    IFACEMETHOD(Invoke)
    (IInspectable *sender, ISizeChangedEventArgs *sizeChangedEventArgs)
    {
        std::shared_ptr<InspectableNativeWindow> host = mHost.lock();
        if (host)
        {
            // The size of the ISwapChainPanel control is returned in DIPs.
            // We are keeping these in dips because the swapchain created for composition
            // also uses dip units. This keeps dimensions, viewports, etc in the same unit.
            // XAML Clients of the ISwapChainPanel are required to use dips to define their
            // layout sizes as well.
            ABI::Windows::Foundation::Size newSize;
            HRESULT result = sizeChangedEventArgs->get_NewSize(&newSize);
            if (SUCCEEDED(result))
            {
                host->setNewClientSize(newSize);
            }
        }

        return S_OK;
    }

  private:
    std::weak_ptr<InspectableNativeWindow> mHost;
};

HRESULT GetSwapChainPanelSize(const ComPtr<ISwapChainPanel> &swapChainPanel,
                              const ComPtr<ICoreDispatcher> &dispatcher,
                              Size *windowSize);
}  // namespace rx
#endif  // LIBANGLE_RENDERER_D3D_D3D11_WINRT_SWAPCHAINPANELNATIVEWINDOW_H_
