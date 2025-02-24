//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkFuchsia.h:
//    Subclasses WindowSurfaceVk for the Fuchsia platform.
//

#ifndef LIBANGLE_RENDERER_VULKAN_FUCHSIA_WINDOWSURFACEVKFUCHSIA_H_
#define LIBANGLE_RENDERER_VULKAN_FUCHSIA_WINDOWSURFACEVKFUCHSIA_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace rx
{

class WindowSurfaceVkFuchsia : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkFuchsia(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);
    ~WindowSurfaceVkFuchsia() override;

    static bool isValidNativeWindow(EGLNativeWindowType window);

  private:
    angle::Result createSurfaceVk(vk::ErrorContext *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::ErrorContext *context, gl::Extents *extentsOut) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_FUCHSIA_WINDOWSURFACEVKFUCHSIA_H_
