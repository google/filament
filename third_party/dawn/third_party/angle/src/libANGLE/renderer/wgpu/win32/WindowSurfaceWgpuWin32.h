//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWin32.h:
//    Defines the class interface for WindowSurfaceWgpuWin32, implementing WindowSurfaceWgpu.
//

#ifndef LIBANGLE_RENDERER_WGPU_WIN32_WINDOWSURFACEWGPUWIN32_H_
#define LIBANGLE_RENDERER_WGPU_WIN32_WINDOWSURFACEWGPUWIN32_H_

#include "libANGLE/renderer/wgpu/SurfaceWgpu.h"

namespace rx
{
class WindowSurfaceWgpuWin32 : public WindowSurfaceWgpu
{
  public:
    WindowSurfaceWgpuWin32(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);

  private:
    angle::Result createWgpuSurface(const egl::Display *display,
                                    wgpu::Surface *outSurface) override;
    angle::Result getCurrentWindowSize(const egl::Display *display, gl::Extents *outSize) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_WIN32_WINDOWSURFACEWGPUWIN32_H_
