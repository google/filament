//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindow9.h: Defines NativeWindow9, a class for managing and
// performing operations on an EGLNativeWindowType for the D3D9 renderer.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_NATIVEWINDOW9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_NATIVEWINDOW9_H_

#include "common/debug.h"
#include "common/platform.h"

#include "libANGLE/renderer/d3d/NativeWindowD3D.h"

namespace rx
{

class NativeWindow9 : public NativeWindowD3D
{
  public:
    explicit NativeWindow9(EGLNativeWindowType window);

    bool initialize() override;
    bool getClientRect(LPRECT rect) const override;
    bool isIconic() const override;

    static bool IsValidNativeWindow(EGLNativeWindowType window);
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_NATIVEWINDOW9_H_
