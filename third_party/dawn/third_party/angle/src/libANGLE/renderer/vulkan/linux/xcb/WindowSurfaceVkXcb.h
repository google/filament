//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkXcb.h:
//    Defines the class interface for WindowSurfaceVkXcb, implementing WindowSurfaceVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKXCB_H_
#define LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKXCB_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

struct xcb_connection_t;

namespace rx
{

class WindowSurfaceVkXcb : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkXcb(const egl::SurfaceState &surfaceState,
                       EGLNativeWindowType window,
                       xcb_connection_t *conn);

  private:
    angle::Result createSurfaceVk(vk::ErrorContext *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::ErrorContext *context, gl::Extents *extentsOut) override;

    xcb_connection_t *mXcbConnection;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_XCB_WINDOWSURFACEVKXCB_H_
