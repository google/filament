//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkMac.h:
//    Subclasses WindowSurfaceVk for the Mac platform.
//

#ifndef LIBANGLE_RENDERER_VULKAN_MAC_WINDOWSURFACEVKMAC_H_
#define LIBANGLE_RENDERER_VULKAN_MAC_WINDOWSURFACEVKMAC_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

#include <Cocoa/Cocoa.h>

namespace rx
{

class WindowSurfaceVkMac : public WindowSurfaceVk
{
  public:
    WindowSurfaceVkMac(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);
    ~WindowSurfaceVkMac() override;

  private:
    angle::Result createSurfaceVk(vk::ErrorContext *context, gl::Extents *extentsOut) override;
    angle::Result getCurrentWindowSize(vk::ErrorContext *context, gl::Extents *extentsOut) override;

    CAMetalLayer *mMetalLayer;
    id<MTLDevice> mMetalDevice;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_MAC_WINDOWSURFACEVKMAC_H_
