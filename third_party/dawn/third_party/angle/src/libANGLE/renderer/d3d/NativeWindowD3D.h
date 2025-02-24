//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// NativeWindowD3D.h: Defines NativeWindowD3D, a class for managing and performing operations on an
// EGLNativeWindowType for the D3D renderers.

#ifndef LIBANGLE_RENDERER_D3D_NATIVEWINDOWD3D_H_
#define LIBANGLE_RENDERER_D3D_NATIVEWINDOWD3D_H_

#include "common/debug.h"
#include "common/platform.h"

#include <EGL/eglplatform.h>
#include "libANGLE/Config.h"

namespace rx
{
class NativeWindowD3D : angle::NonCopyable
{
  public:
    NativeWindowD3D(EGLNativeWindowType window);
    virtual ~NativeWindowD3D();

    virtual bool initialize()                     = 0;
    virtual bool getClientRect(LPRECT rect) const = 0;
    virtual bool isIconic() const                 = 0;

    inline EGLNativeWindowType getNativeWindow() const { return mWindow; }

  private:
    EGLNativeWindowType mWindow;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_NATIVEWINDOWD3D_H_
