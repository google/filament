//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CoreWindowNativeWindow.h: NativeWindow for managing ICoreWindow native window types.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_WINRT_COREWINDOWNATIVEWINDOW_H_
#define LIBANGLE_RENDERER_D3D_D3D11_WINRT_COREWINDOWNATIVEWINDOW_H_

#include "libANGLE/renderer/d3d/d3d11/winrt/InspectableNativeWindow.h"

#include <memory>

#include <EGL/eglplatform.h>

typedef ABI::Windows::Foundation::
    __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CWindowSizeChangedEventArgs_t
        IWindowSizeChangedEventHandler;

namespace rx
{
float ConvertDipsToPixels(float dips);

class CoreWindowNativeWindow : public InspectableNativeWindow,
                               public std::enable_shared_from_this<CoreWindowNativeWindow>
{
  public:
    ~CoreWindowNativeWindow();

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
    ComPtr<ABI::Windows::UI::Core::ICoreWindow> mCoreWindow;
    ComPtr<IMap<HSTRING, IInspectable *>> mPropertyMap;
};

__declspec(uuid("7F924F66-EBAE-40E5-A10B-B8F35E245190")) class CoreWindowSizeChangedHandler
    : public Microsoft::WRL::RuntimeClass<
          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
          IWindowSizeChangedEventHandler>
{
  public:
    CoreWindowSizeChangedHandler() {}
    HRESULT RuntimeClassInitialize(std::shared_ptr<InspectableNativeWindow> host)
    {
        if (!host)
        {
            return E_INVALIDARG;
        }

        mHost = host;
        return S_OK;
    }

    // IWindowSizeChangedEventHandler
    IFACEMETHOD(Invoke)
    (ABI::Windows::UI::Core::ICoreWindow *sender,
     ABI::Windows::UI::Core::IWindowSizeChangedEventArgs *sizeChangedEventArgs)
    {
        std::shared_ptr<InspectableNativeWindow> host = mHost.lock();
        if (host)
        {
            ABI::Windows::Foundation::Size windowSize;
            if (SUCCEEDED(sizeChangedEventArgs->get_Size(&windowSize)))
            {
                Size windowSizeInPixels = {ConvertDipsToPixels(windowSize.Width),
                                           ConvertDipsToPixels(windowSize.Height)};
                host->setNewClientSize(windowSizeInPixels);
            }
        }

        return S_OK;
    }

  private:
    std::weak_ptr<InspectableNativeWindow> mHost;
};

HRESULT GetCoreWindowSizeInPixels(const ComPtr<ABI::Windows::UI::Core::ICoreWindow> &coreWindow,
                                  Size *windowSize);
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_WINRT_COREWINDOWNATIVEWINDOW_H_
