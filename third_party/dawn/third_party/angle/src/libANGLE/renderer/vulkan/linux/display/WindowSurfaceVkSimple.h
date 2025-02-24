//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkSimple.h:
//    Defines the class interface for WindowSurfaceVkSimple, implementing WindowSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DISPLAY_WINDOWSURFACEVKSIMPLE_H_
#define LIBANGLE_RENDERER_VULKAN_DISPLAY_WINDOWSURFACEVKSIMPLE_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace rx
{

class WindowSurfaceVkSimple final : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkSimple(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);
    ~WindowSurfaceVkSimple() final;

  private:
    angle::Result createSurfaceVk(vk::ErrorContext *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::ErrorContext *context, gl::Extents *extentsOut) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPLAY_WINDOWSURFACEVKSIMPLE_H_
