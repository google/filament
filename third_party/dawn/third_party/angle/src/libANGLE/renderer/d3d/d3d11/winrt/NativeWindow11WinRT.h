//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindow11WinRT.h: NativeWindow base class for managing IInspectable native window types.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_WINRT_NATIVEWINDOW11WINRT_H_
#define LIBANGLE_RENDERER_D3D_D3D11_WINRT_NATIVEWINDOW11WINRT_H_

#include "libANGLE/renderer/d3d/d3d11/NativeWindow11.h"

#include <windows.applicationmodel.core.h>
#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <memory>

namespace rx
{
class InspectableNativeWindow;

class NativeWindow11WinRT : public NativeWindow11
{
  public:
    NativeWindow11WinRT(EGLNativeWindowType window, bool hasAlpha);

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
    bool mHasAlpha;
    std::shared_ptr<InspectableNativeWindow> mImpl;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_WINRT_NATIVEWINDOW11WINRT_H_
