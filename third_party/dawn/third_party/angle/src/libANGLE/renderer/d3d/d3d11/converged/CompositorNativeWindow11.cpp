//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CompositorNativeWindow11.cpp: Implementation of NativeWindow11 using Windows.UI.Composition APIs
// which work in both Win32 and WinRT contexts.

#include "libANGLE/renderer/d3d/d3d11/converged/CompositorNativeWindow11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

#include "common/debug.h"

using namespace Microsoft::WRL;

namespace rx
{

CompositorNativeWindow11::CompositorNativeWindow11(EGLNativeWindowType window, bool hasAlpha)
    : NativeWindow11(window), mHasAlpha(hasAlpha)
{
    ABI::Windows::UI::Composition::ISpriteVisual *inspPtr =
        reinterpret_cast<ABI::Windows::UI::Composition::ISpriteVisual *>(window);
    mHostVisual = Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ISpriteVisual>{inspPtr};
}

CompositorNativeWindow11::~CompositorNativeWindow11() = default;

bool CompositorNativeWindow11::initialize()
{
    return true;
}

bool CompositorNativeWindow11::getClientRect(LPRECT rect) const
{
    ComPtr<ABI::Windows::UI::Composition::IVisual> visual;
    mHostVisual.As(&visual);

    ABI::Windows::Foundation::Numerics::Vector2 size;
    HRESULT hr = visual->get_Size(&size);
    if (FAILED(hr))
    {
        return false;
    }

    ABI::Windows::Foundation::Numerics::Vector3 offset;
    hr = visual->get_Offset(&offset);
    if (FAILED(hr))
    {
        return false;
    }

    rect->top    = static_cast<LONG>(offset.Y);
    rect->left   = static_cast<LONG>(offset.X);
    rect->right  = static_cast<LONG>(offset.X) + static_cast<LONG>(size.X);
    rect->bottom = static_cast<LONG>(offset.Y) + static_cast<LONG>(size.Y);

    return true;
}

bool CompositorNativeWindow11::isIconic() const
{
    return false;
}

HRESULT CompositorNativeWindow11::createSwapChain(ID3D11Device *device,
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

    HRESULT hr{E_FAIL};

    ComPtr<ABI::Windows::UI::Composition::ICompositionObject> hostVisual;
    hr = mHostVisual.As(&hostVisual);
    if (FAILED(hr))
    {
        return hr;
    }

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Composition::ICompositor> compositor;
    hr = hostVisual->get_Compositor(&compositor);
    if (FAILED(hr))
    {
        return hr;
    }

    ComPtr<ABI::Windows::UI::Composition::ICompositorInterop> interop;

    hr = compositor.As(&interop);
    if (FAILED(hr))
    {
        return hr;
    }

    ComPtr<IDXGIFactory2> factory2;
    factory2.Attach(d3d11::DynamicCastComObject<IDXGIFactory2>(factory));

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
    swapChainDesc.AlphaMode   = mHasAlpha ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE;
#ifndef ANGLE_ENABLE_WINDOWS_UWP
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
#endif
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    hr = factory2->CreateSwapChainForComposition(device, &swapChainDesc, nullptr, &swapChain1);
    if (SUCCEEDED(hr))
    {
        swapChain1.CopyTo(swapChain);
    }

    hr = interop->CreateCompositionSurfaceForSwapChain(swapChain1.Get(), &mSurface);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = compositor->CreateSurfaceBrushWithSurface(mSurface.Get(), &mSurfaceBrush);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = mSurfaceBrush.As(&mCompositionBrush);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = mHostVisual->put_Brush(mCompositionBrush.Get());
    if (FAILED(hr))
    {
        return hr;
    }

    return hr;
}

void CompositorNativeWindow11::commitChange()
{
    // Windows::UI::Composition uses an implicit commit model hence no action needed here
}

// static
bool CompositorNativeWindow11::IsValidNativeWindow(EGLNativeWindowType window)
{
    return IsSupportedWinRelease() && IsSpriteVisual(window);
}

// static
bool CompositorNativeWindow11::IsSupportedWinRelease()
{
    RoHelper helper;
    if (!helper.WinRtAvailable())
    {
        return false;
    }

    return helper.SupportedWindowsRelease();
}

bool CompositorNativeWindow11::IsSpriteVisual(EGLNativeWindowType window)
{
    RoHelper helper;

    ABI::Windows::UI::Composition::ISpriteVisual *inspp =
        reinterpret_cast<ABI::Windows::UI::Composition::ISpriteVisual *>(window);
    HSTRING className, spriteClassName;
    HSTRING_HEADER spriteClassNameHeader;

    auto hr = helper.GetStringReference(RuntimeClass_Windows_UI_Composition_SpriteVisual,
                                        &spriteClassName, &spriteClassNameHeader);
    if (FAILED(hr))
    {
        return false;
    }

    hr = inspp->GetRuntimeClassName(&className);
    if (FAILED(hr))
    {
        return false;
    }

    INT32 result = -1;
    hr           = helper.WindowsCompareStringOrdinal(className, spriteClassName, &result);

    helper.WindowsDeleteString(className);

    if (FAILED(hr))
    {
        return false;
    }

    if (result == 0)
    {
        return true;
    }

    return false;
}

// RoHelperImpl

template <typename T>
bool AssignProcAddress(HMODULE comBaseModule, const char *name, T *&outProc)
{
    outProc = reinterpret_cast<T *>(GetProcAddress(comBaseModule, name));
    return *outProc != nullptr;
}

RoHelper::RoHelper()
    : mFpWindowsCreateStringReference(nullptr),
      mFpGetActivationFactory(nullptr),
      mFpWindowsCompareStringOrdinal(nullptr),
      mFpCreateDispatcherQueueController(nullptr),
      mFpWindowsDeleteString(nullptr),
      mFpRoInitialize(nullptr),
      mFpRoUninitialize(nullptr),
      mWinRtAvailable(false),
      mWinRtInitialized(false),
      mComBaseModule(nullptr),
      mCoreMessagingModule(nullptr)
{

#ifdef ANGLE_ENABLE_WINDOWS_UWP
    mFpWindowsCreateStringReference    = &::WindowsCreateStringReference;
    mFpRoInitialize                    = &::RoInitialize;
    mFpRoUninitialize                  = &::RoUninitialize;
    mFpWindowsDeleteString             = &::WindowsDeleteString;
    mFpGetActivationFactory            = &::RoGetActivationFactory;
    mFpWindowsCompareStringOrdinal     = &::WindowsCompareStringOrdinal;
    mFpCreateDispatcherQueueController = &::CreateDispatcherQueueController;
    mWinRtAvailable                    = true;
#else

    mComBaseModule = LoadLibraryA("ComBase.dll");

    if (mComBaseModule == nullptr)
    {
        return;
    }

    if (!AssignProcAddress(mComBaseModule, "WindowsCreateStringReference",
                           mFpWindowsCreateStringReference))
    {
        return;
    }

    if (!AssignProcAddress(mComBaseModule, "RoGetActivationFactory", mFpGetActivationFactory))
    {
        return;
    }

    if (!AssignProcAddress(mComBaseModule, "WindowsCompareStringOrdinal",
                           mFpWindowsCompareStringOrdinal))
    {
        return;
    }

    if (!AssignProcAddress(mComBaseModule, "WindowsDeleteString", mFpWindowsDeleteString))
    {
        return;
    }

    if (!AssignProcAddress(mComBaseModule, "RoInitialize", mFpRoInitialize))
    {
        return;
    }

    if (!AssignProcAddress(mComBaseModule, "RoUninitialize", mFpRoUninitialize))
    {
        return;
    }

    mCoreMessagingModule = LoadLibraryA("coremessaging.dll");

    if (mCoreMessagingModule == nullptr)
    {
        return;
    }

    if (!AssignProcAddress(mCoreMessagingModule, "CreateDispatcherQueueController",
                           mFpCreateDispatcherQueueController))
    {
        return;
    }

    auto result = RoInitialize(RO_INIT_MULTITHREADED);

    if (SUCCEEDED(result) || result == RPC_E_CHANGED_MODE)
    {
        mWinRtAvailable = true;

        if (SUCCEEDED(result))
        {
            mWinRtInitialized = true;
        }
    }
#endif
}

RoHelper::~RoHelper()
{
#ifndef ANGLE_ENABLE_WINDOWS_UWP
    if (mWinRtInitialized)
    {
        RoUninitialize();
    }

    if (mCoreMessagingModule != nullptr)
    {
        FreeLibrary(mCoreMessagingModule);
        mCoreMessagingModule = nullptr;
    }

    if (mComBaseModule != nullptr)
    {
        FreeLibrary(mComBaseModule);
        mComBaseModule = nullptr;
    }
#endif
}

bool RoHelper::WinRtAvailable() const
{
    return mWinRtAvailable;
}

bool RoHelper::SupportedWindowsRelease()
{
    if (!mWinRtAvailable)
    {
        return false;
    }

    HSTRING className, contractName;
    HSTRING_HEADER classNameHeader, contractNameHeader;
    boolean isSupported = false;

    HRESULT hr = GetStringReference(RuntimeClass_Windows_Foundation_Metadata_ApiInformation,
                                    &className, &classNameHeader);

    if (FAILED(hr))
    {
        return !!isSupported;
    }

    Microsoft::WRL::ComPtr<ABI::Windows::Foundation::Metadata::IApiInformationStatics> api;

    hr = GetActivationFactory(
        className, __uuidof(ABI::Windows::Foundation::Metadata::IApiInformationStatics), &api);

    if (FAILED(hr))
    {
        return !!isSupported;
    }

    hr = GetStringReference(L"Windows.Foundation.UniversalApiContract", &contractName,
                            &contractNameHeader);
    if (FAILED(hr))
    {
        return !!isSupported;
    }

    api->IsApiContractPresentByMajor(contractName, 6, &isSupported);

    return !!isSupported;
}

HRESULT RoHelper::GetStringReference(PCWSTR source, HSTRING *act, HSTRING_HEADER *header)
{
    if (!mWinRtAvailable)
    {
        return E_FAIL;
    }

    const wchar_t *str = static_cast<const wchar_t *>(source);

    unsigned int length;
    HRESULT hr = SizeTToUInt32(::wcslen(str), &length);
    if (FAILED(hr))
    {
        return hr;
    }

    return mFpWindowsCreateStringReference(source, length, header, act);
}

HRESULT RoHelper::GetActivationFactory(const HSTRING act, const IID &interfaceId, void **fac)
{
    if (!mWinRtAvailable)
    {
        return E_FAIL;
    }
    auto hr = mFpGetActivationFactory(act, interfaceId, fac);
    return hr;
}

HRESULT RoHelper::WindowsCompareStringOrdinal(HSTRING one, HSTRING two, int *result)
{
    if (!mWinRtAvailable)
    {
        return E_FAIL;
    }
    return mFpWindowsCompareStringOrdinal(one, two, result);
}

HRESULT RoHelper::CreateDispatcherQueueController(
    DispatcherQueueOptions options,
    ABI::Windows::System::IDispatcherQueueController **dispatcherQueueController)
{
    if (!mWinRtAvailable)
    {
        return E_FAIL;
    }
    return mFpCreateDispatcherQueueController(options, dispatcherQueueController);
}

HRESULT RoHelper::WindowsDeleteString(HSTRING one)
{
    if (!mWinRtAvailable)
    {
        return E_FAIL;
    }
    return mFpWindowsDeleteString(one);
}

HRESULT RoHelper::RoInitialize(RO_INIT_TYPE type)
{
    return mFpRoInitialize(type);
}

void RoHelper::RoUninitialize()
{
    mFpRoUninitialize();
}

}  // namespace rx
