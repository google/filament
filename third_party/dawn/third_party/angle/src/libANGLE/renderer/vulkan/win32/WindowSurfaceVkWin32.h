//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWin32.h:
//    Defines the class interface for WindowSurfaceVkWin32, implementing WindowSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_WIN32_WINDOWSURFACEVKWIN32_H_
#define LIBANGLE_RENDERER_VULKAN_WIN32_WINDOWSURFACEVKWIN32_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace rx
{

class WindowSurfaceVkWin32 : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkWin32(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);

  private:
    angle::Result createSurfaceVk(vk::ErrorContext *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::ErrorContext *context, gl::Extents *extentsOut) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_WIN32_WINDOWSURFACEVKWIN32_H_
