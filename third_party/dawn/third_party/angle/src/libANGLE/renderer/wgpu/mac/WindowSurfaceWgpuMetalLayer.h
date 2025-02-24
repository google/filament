//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceWgpuMetalLayer.h:
//    Defines the class interface for WindowSurfaceWgpuMetalLayer, implementing WindowSurfaceWgpu.
//

#ifndef LIBANGLE_RENDERER_WGPU_MAC_WINDOWSURFACEWGPUMETALLAYER_H_
#define LIBANGLE_RENDERER_WGPU_MAC_WINDOWSURFACEWGPUMETALLAYER_H_

#include "libANGLE/renderer/wgpu/SurfaceWgpu.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace rx
{
class WindowSurfaceWgpuMetalLayer : public WindowSurfaceWgpu
{
  public:
    WindowSurfaceWgpuMetalLayer(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);

    egl::Error initialize(const egl::Display *display) override;
    void destroy(const egl::Display *display) override;

  private:
    angle::Result createWgpuSurface(const egl::Display *display,
                                    wgpu::Surface *outSurface) override;
    angle::Result getCurrentWindowSize(const egl::Display *display, gl::Extents *outSize) override;

    id<MTLDevice> mMetalDevice;
    CAMetalLayer *mMetalLayer;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_MAC_WINDOWSURFACEWGPUMETALLAYER_H_
