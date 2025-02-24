//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_METAL_IOSURFACESURFACEMTL_H_
#define LIBANGLE_RENDERER_METAL_IOSURFACESURFACEMTL_H_

#include <IOSurface/IOSurfaceRef.h>
#include "libANGLE/renderer/SurfaceImpl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/SurfaceMtl.h"

namespace metal
{
class AttributeMap;
}  // namespace metal

namespace rx
{

class DisplayMTL;

// Offscreen created from IOSurface
class IOSurfaceSurfaceMtl : public OffscreenSurfaceMtl
{
  public:
    IOSurfaceSurfaceMtl(DisplayMtl *display,
                        const egl::SurfaceState &state,
                        EGLClientBuffer buffer,
                        const egl::AttributeMap &attribs);
    ~IOSurfaceSurfaceMtl() override;

    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

    static bool ValidateAttributes(EGLClientBuffer buffer, const egl::AttributeMap &attribs);

  private:
    angle::Result ensureColorTextureCreated(const gl::Context *context);

    IOSurfaceRef mIOSurface;
    NSUInteger mIOSurfacePlane;
    int mIOSurfaceFormatIdx;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_METAL_IOSURFACESURFACEMTL_H_
