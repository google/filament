//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindow11Win32.h: Implementation of NativeWindow11 using win32 window APIs.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_WIN32_NATIVEWINDOW11WIN32_H_
#define LIBANGLE_RENDERER_D3D_D3D11_WIN32_NATIVEWINDOW11WIN32_H_

#include "libANGLE/renderer/d3d/d3d11/NativeWindow11.h"

typedef interface IDCompositionDevice IDCompositionDevice;
typedef interface IDCompositionTarget IDCompositionTarget;
typedef interface IDCompositionVisual IDCompositionVisual;

namespace rx
{

class NativeWindow11Win32 : public NativeWindow11
{
  public:
    NativeWindow11Win32(EGLNativeWindowType window, bool hasAlpha, bool directComposition);
    ~NativeWindow11Win32() override;

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

  private:
    bool mDirectComposition;
    bool mHasAlpha;
    IDCompositionDevice *mDevice;
    IDCompositionTarget *mCompositionTarget;
    IDCompositionVisual *mVisual;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_WIN32_NATIVEWINDOW11WIN32_H_
