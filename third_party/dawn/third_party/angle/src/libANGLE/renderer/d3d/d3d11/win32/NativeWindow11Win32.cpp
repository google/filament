//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindow11Win32.cpp: Implementation of NativeWindow11 using win32 window APIs.

#include "libANGLE/renderer/d3d/d3d11/win32/NativeWindow11Win32.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

#include "common/debug.h"

// This header must be included before dcomp.h.
#include <initguid.h>

#include <dcomp.h>

namespace rx
{

NativeWindow11Win32::NativeWindow11Win32(EGLNativeWindowType window,
                                         bool hasAlpha,
                                         bool directComposition)
    : NativeWindow11(window),
      mDirectComposition(directComposition),
      mHasAlpha(hasAlpha),
      mDevice(nullptr),
      mCompositionTarget(nullptr),
      mVisual(nullptr)
{}

NativeWindow11Win32::~NativeWindow11Win32()
{
    SafeRelease(mCompositionTarget);
    SafeRelease(mDevice);
    SafeRelease(mVisual);
}

bool NativeWindow11Win32::initialize()
{
    return true;
}

bool NativeWindow11Win32::getClientRect(LPRECT rect) const
{
    return GetClientRect(getNativeWindow(), rect) == TRUE;
}

bool NativeWindow11Win32::isIconic() const
{
    return IsIconic(getNativeWindow()) == TRUE;
}

HRESULT NativeWindow11Win32::createSwapChain(ID3D11Device *device,
                                             IDXGIFactory *factory,
                                             DXGI_FORMAT format,
                                             UINT width,
                                             UINT height,
                                             UINT samples,
                                             IDXGISwapChain **swapChain)
{
    if (device == nullptr || factory == nullptr || swapChain == nullptr || width == 0 ||
        height == 0)
    {
        return E_INVALIDARG;
    }

    if (mDirectComposition)
    {
        HMODULE dcomp = ::GetModuleHandle(TEXT("dcomp.dll"));
        if (!dcomp)
        {
            return E_INVALIDARG;
        }

        typedef HRESULT(WINAPI * PFN_DCOMPOSITION_CREATE_DEVICE)(
            IDXGIDevice * dxgiDevice, REFIID iid, void **dcompositionDevice);
        PFN_DCOMPOSITION_CREATE_DEVICE createDComp =
            reinterpret_cast<PFN_DCOMPOSITION_CREATE_DEVICE>(
                GetProcAddress(dcomp, "DCompositionCreateDevice"));
        if (!createDComp)
        {
            return E_INVALIDARG;
        }

        if (!mDevice)
        {
            IDXGIDevice *dxgiDevice = d3d11::DynamicCastComObject<IDXGIDevice>(device);
            HRESULT result          = createDComp(dxgiDevice, __uuidof(IDCompositionDevice),
                                         reinterpret_cast<void **>(&mDevice));
            SafeRelease(dxgiDevice);

            if (FAILED(result))
            {
                return result;
            }
        }

        if (!mCompositionTarget)
        {
            HRESULT result =
                mDevice->CreateTargetForHwnd(getNativeWindow(), TRUE, &mCompositionTarget);
            if (FAILED(result))
            {
                return result;
            }
        }

        if (!mVisual)
        {
            HRESULT result = mDevice->CreateVisual(&mVisual);
            if (FAILED(result))
            {
                return result;
            }
        }

        IDXGIFactory2 *factory2             = d3d11::DynamicCastComObject<IDXGIFactory2>(factory);
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width                 = width;
        swapChainDesc.Height                = height;
        swapChainDesc.Format                = format;
        swapChainDesc.Stereo                = FALSE;
        swapChainDesc.SampleDesc.Count      = 1;
        swapChainDesc.SampleDesc.Quality    = 0;
        swapChainDesc.BufferUsage =
            DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_SHADER_INPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling     = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.AlphaMode =
            mHasAlpha ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Flags         = 0;
        IDXGISwapChain1 *swapChain1 = nullptr;
        HRESULT result =
            factory2->CreateSwapChainForComposition(device, &swapChainDesc, nullptr, &swapChain1);
        if (SUCCEEDED(result))
        {
            *swapChain = static_cast<IDXGISwapChain *>(swapChain1);
        }
        mVisual->SetContent(swapChain1);
        mCompositionTarget->SetRoot(mVisual);
        SafeRelease(factory2);
        return result;
    }

    // Use IDXGIFactory2::CreateSwapChainForHwnd if DXGI 1.2 is available to create a
    // DXGI_SWAP_EFFECT_SEQUENTIAL swap chain.
    IDXGIFactory2 *factory2 = d3d11::DynamicCastComObject<IDXGIFactory2>(factory);
    if (factory2 != nullptr)
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width                 = width;
        swapChainDesc.Height                = height;
        swapChainDesc.Format                = format;
        swapChainDesc.Stereo                = FALSE;
        swapChainDesc.SampleDesc.Count      = samples;
        swapChainDesc.SampleDesc.Quality    = 0;
        swapChainDesc.BufferUsage =
            DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_BACK_BUFFER;
        swapChainDesc.BufferCount   = 1;
        swapChainDesc.Scaling       = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect    = DXGI_SWAP_EFFECT_SEQUENTIAL;
        swapChainDesc.AlphaMode     = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags         = 0;
        IDXGISwapChain1 *swapChain1 = nullptr;
        HRESULT result = factory2->CreateSwapChainForHwnd(device, getNativeWindow(), &swapChainDesc,
                                                          nullptr, nullptr, &swapChain1);
        if (SUCCEEDED(result))
        {
            factory2->MakeWindowAssociation(getNativeWindow(), DXGI_MWA_NO_ALT_ENTER);
            *swapChain = static_cast<IDXGISwapChain *>(swapChain1);
        }
        SafeRelease(factory2);
        return result;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
    swapChainDesc.BufferCount                        = 1;
    swapChainDesc.BufferDesc.Format                  = format;
    swapChainDesc.BufferDesc.Width                   = width;
    swapChainDesc.BufferDesc.Height                  = height;
    swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage =
        DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.Flags              = 0;
    swapChainDesc.OutputWindow       = getNativeWindow();
    swapChainDesc.SampleDesc.Count   = samples;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed           = TRUE;
    swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;

    HRESULT result = factory->CreateSwapChain(device, &swapChainDesc, swapChain);
    if (SUCCEEDED(result))
    {
        factory->MakeWindowAssociation(getNativeWindow(), DXGI_MWA_NO_ALT_ENTER);
    }
    return result;
}

void NativeWindow11Win32::commitChange()
{
    if (mDevice)
    {
        mDevice->Commit();
    }
}

// static
bool NativeWindow11Win32::IsValidNativeWindow(EGLNativeWindowType window)
{
    return IsWindow(window) == TRUE;
}
}  // namespace rx
