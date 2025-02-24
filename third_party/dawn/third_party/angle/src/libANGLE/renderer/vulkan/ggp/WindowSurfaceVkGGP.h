//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkGGP.h:
//    Google Game Platform implementation of WindowSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_GGP_WINDOWSURFACEVKGGP_H_
#define LIBANGLE_RENDERER_VULKAN_GGP_WINDOWSURFACEVKGGP_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace rx
{

class WindowSurfaceVkGGP : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkGGP(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);

  private:
    egl::Error swapWithFrameToken(const gl::Context *context,
                                  EGLFrameTokenANGLE frameToken) override;
    angle::Result createSurfaceVk(vk::ErrorContext *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::ErrorContext *context, gl::Extents *extentsOut) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_GGP_WINDOWSURFACEVKGGP_H_
