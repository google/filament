//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CoreWindowNativeWindow.cpp: NativeWindow for managing ICoreWindow native window types.

#include "libANGLE/renderer/d3d/d3d11/winrt/CoreWindowNativeWindow.h"

#include <windows.graphics.display.h>

using namespace ABI::Windows::Foundation::Collections;

namespace rx
{
CoreWindowNativeWindow::~CoreWindowNativeWindow()
{
    unregisterForSizeChangeEvents();
}

bool CoreWindowNativeWindow::initialize(EGLNativeWindowType window, IPropertySet *propertySet)
{
    ComPtr<IPropertySet> props = propertySet;
    ComPtr<IInspectable> win   = reinterpret_cast<IInspectable *>(window);
    SIZE swapChainSize         = {};
    HRESULT result             = S_OK;

    // IPropertySet is an optional parameter and can be null.
    // If one is specified, cache as an IMap and read the properties
    // used for initial host initialization.
    if (propertySet)
    {
        result = props.As(&mPropertyMap);
        if (FAILED(result))
        {
            return false;
        }

        // The EGLRenderSurfaceSizeProperty is optional and may be missing. The IPropertySet
        // was prevalidated to contain the EGLNativeWindowType before being passed to
        // this host.
        result = GetOptionalSizePropertyValue(mPropertyMap, EGLRenderSurfaceSizeProperty,
                                              &swapChainSize, &mSwapChainSizeSpecified);
        if (FAILED(result))
        {
            return false;
        }

        // The EGLRenderResolutionScaleProperty is optional and may be missing. The IPropertySet
        // was prevalidated to contain the EGLNativeWindowType before being passed to
        // this host.
        result = GetOptionalSinglePropertyValue(mPropertyMap, EGLRenderResolutionScaleProperty,
                                                &mSwapChainScale, &mSwapChainScaleSpecified);
        if (FAILED(result))
        {
            return false;
        }

        if (!mSwapChainScaleSpecified)
        {
            // Default value for the scale is 1.0f
            mSwapChainScale = 1.0f;
        }

        // A EGLRenderSurfaceSizeProperty and a EGLRenderResolutionScaleProperty can't both be
        // specified
        if (mSwapChainScaleSpecified && mSwapChainSizeSpecified)
        {
            ERR() << "It is invalid to specify both an EGLRenderSurfaceSizeProperty and a "
                     "EGLRenderResolutionScaleProperty.";
            return false;
        }
    }

    if (SUCCEEDED(result))
    {
        result = win.As(&mCoreWindow);
    }

    if (SUCCEEDED(result))
    {
        // If a swapchain size is specfied, then the automatic resize
        // behaviors implemented by the host should be disabled.  The swapchain
        // will be still be scaled when being rendered to fit the bounds
        // of the host.
        // Scaling of the swapchain output occurs automatically because if
        // the scaling mode setting DXGI_SCALING_STRETCH on the swapchain.
        if (mSwapChainSizeSpecified)
        {
            mClientRect = {0, 0, swapChainSize.cx, swapChainSize.cy};
        }
        else
        {
            Size coreWindowSize;
            result = GetCoreWindowSizeInPixels(mCoreWindow, &coreWindowSize);

            if (SUCCEEDED(result))
            {
                mClientRect = clientRect(coreWindowSize);
            }
        }
    }

    if (SUCCEEDED(result))
    {
        mNewClientRect     = mClientRect;
        mClientRectChanged = false;
        return registerForSizeChangeEvents();
    }

    return false;
}

bool CoreWindowNativeWindow::registerForSizeChangeEvents()
{
    ComPtr<IWindowSizeChangedEventHandler> sizeChangedHandler;
    HRESULT result = Microsoft::WRL::MakeAndInitialize<CoreWindowSizeChangedHandler>(
        sizeChangedHandler.ReleaseAndGetAddressOf(), this->shared_from_this());
    if (SUCCEEDED(result))
    {
        result = mCoreWindow->add_SizeChanged(sizeChangedHandler.Get(), &mSizeChangedEventToken);
    }

    if (SUCCEEDED(result))
    {
        return true;
    }

    return false;
}

void CoreWindowNativeWindow::unregisterForSizeChangeEvents()
{
    if (mCoreWindow)
    {
        (void)mCoreWindow->remove_SizeChanged(mSizeChangedEventToken);
    }
    mSizeChangedEventToken.value = 0;
}

HRESULT CoreWindowNativeWindow::createSwapChain(ID3D11Device *device,
                                                IDXGIFactory2 *factory,
                                                DXGI_FORMAT format,
                                                unsigned int width,
                                                unsigned int height,
                                                bool containsAlpha,
                                                IDXGISwapChain1 **swapChain)
{
    if (device == nullptr || factory == nullptr || swapChain == nullptr || width == 0 ||
        height == 0)
    {
        return E_INVALIDARG;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
    swapChainDesc.Width                 = width;
    swapChainDesc.Height                = height;
    swapChainDesc.Format                = format;
    swapChainDesc.Stereo                = FALSE;
    swapChainDesc.SampleDesc.Count      = 1;
    swapChainDesc.SampleDesc.Quality    = 0;
    swapChainDesc.BufferUsage =
        DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.Scaling     = DXGI_SCALING_STRETCH;
    swapChainDesc.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED;

    *swapChain = nullptr;

    ComPtr<IDXGISwapChain1> newSwapChain;
    HRESULT result = factory->CreateSwapChainForCoreWindow(
        device, mCoreWindow.Get(), &swapChainDesc, nullptr, newSwapChain.ReleaseAndGetAddressOf());
    if (SUCCEEDED(result))
    {
        result = newSwapChain.CopyTo(swapChain);
    }

    if (SUCCEEDED(result))
    {
        // If automatic swapchain resize behaviors have been disabled, then
        // unregister for the resize change events.
        if (mSupportsSwapChainResize == false)
        {
            unregisterForSizeChangeEvents();
        }
    }

    return result;
}

inline HRESULT CoreWindowNativeWindow::scaleSwapChain(const Size &windowSize,
                                                      const RECT &clientRect)
{
    // We don't need to do any additional work to scale CoreWindow swapchains.
    // Using DXGI_SCALING_STRETCH to create the swapchain above does all the necessary work.
    return S_OK;
}

HRESULT GetCoreWindowSizeInPixels(const ComPtr<ABI::Windows::UI::Core::ICoreWindow> &coreWindow,
                                  Size *windowSize)
{
    ABI::Windows::Foundation::Rect bounds;
    HRESULT result = coreWindow->get_Bounds(&bounds);
    if (SUCCEEDED(result))
    {
        *windowSize = {ConvertDipsToPixels(bounds.Width), ConvertDipsToPixels(bounds.Height)};
    }

    return result;
}

static float GetLogicalDpi()
{
    ComPtr<ABI::Windows::Graphics::Display::IDisplayInformationStatics> displayInformationStatics;
    ComPtr<ABI::Windows::Graphics::Display::IDisplayInformation> displayInformation;

    if (SUCCEEDED(GetActivationFactory(
            HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayInformation).Get(),
            displayInformationStatics.GetAddressOf())))
    {
        if (SUCCEEDED(displayInformationStatics->GetForCurrentView(&displayInformation)))
        {
            float dpi = 96.0f;
            if (SUCCEEDED(displayInformation->get_LogicalDpi(&dpi)))
            {
                return dpi;
            }
        }
    }

    // Return 96 dpi as a default if display properties cannot be obtained.
    return 96.0f;
}

float ConvertDipsToPixels(float dips)
{
    static const float dipsPerInch = 96.0f;
    return dips * GetLogicalDpi() / dipsPerInch;
}
}  // namespace rx
