//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceWgpuX11.h:
//    Defines the class interface for WindowSurfaceWgpuX11, implementing WindowSurfaceWgpu.
//

#ifndef LIBANGLE_RENDERER_WGPU_LINUX_X11_WINDOWSURFACEWGPUX11_H_
#define LIBANGLE_RENDERER_WGPU_LINUX_X11_WINDOWSURFACEWGPUX11_H_

#include "libANGLE/renderer/wgpu/SurfaceWgpu.h"

namespace rx
{
class WindowSurfaceWgpuX11 : public WindowSurfaceWgpu
{
  public:
    WindowSurfaceWgpuX11(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);

  private:
    angle::Result createWgpuSurface(const egl::Display *display,
                                    wgpu::Surface *outSurface) override;
    angle::Result getCurrentWindowSize(const egl::Display *display, gl::Extents *outSize) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_LINUX_X11_WINDOWSURFACEWGPUX11_H_
