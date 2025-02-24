//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IOSurfaceSurfaceVkMac.h:
//    Subclasses SurfaceVk for the Mac platform to implement PBuffers using an IOSurface
//

#ifndef LIBANGLE_RENDERER_VULKAN_MAC_IOSURFACESURFACEVKMAC_H_
#define LIBANGLE_RENDERER_VULKAN_MAC_IOSURFACESURFACEVKMAC_H_

#include "libANGLE/renderer/vulkan/SurfaceVk.h"

struct __IOSurface;
typedef __IOSurface *IOSurfaceRef;

namespace egl
{
class AttributeMap;
}  // namespace egl

namespace rx
{

class IOSurfaceSurfaceVkMac : public OffscreenSurfaceVk
{
  public:
    IOSurfaceSurfaceVkMac(const egl::SurfaceState &state,
                          EGLClientBuffer buffer,
                          const egl::AttributeMap &attribs,
                          vk::Renderer *renderer);
    ~IOSurfaceSurfaceVkMac() override;

    egl::Error initialize(const egl::Display *display) override;

    egl::Error unMakeCurrent(const gl::Context *context) override;

    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;

    static bool ValidateAttributes(const DisplayVk *displayVk,
                                   EGLClientBuffer buffer,
                                   const egl::AttributeMap &attribs);

  protected:
    angle::Result initializeImpl(DisplayVk *displayVk) override;

  private:
    int computeAlignment() const;

    IOSurfaceRef mIOSurface;

    int mPlane;
    int mFormatIndex;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_MAC_IOSURFACESURFACEVKMAC_H_
