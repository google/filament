//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkHeadless.h:
//    Defines the class interface for WindowSurfaceVkHeadless, implementing WindowSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DISPLAY_WINDOWSURFACEVKHEADLESS_H_
#define LIBANGLE_RENDERER_VULKAN_DISPLAY_WINDOWSURFACEVKHEADLESS_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace rx
{

class WindowSurfaceVkHeadless final : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkHeadless(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);
    ~WindowSurfaceVkHeadless() final;

  private:
    angle::Result createSurfaceVk(vk::ErrorContext *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::ErrorContext *context, gl::Extents *extentsOut) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPLAY_WINDOWSURFACEVKHEADLESS_H_
