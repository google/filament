//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLImageD3D.h: Defines the rx::EGLImageD3D class, the D3D implementation of EGL images

#ifndef LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_
#define LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_

#include "libANGLE/renderer/ImageImpl.h"

namespace gl
{
class Context;
}

namespace egl
{
class AttributeMap;
}

namespace rx
{
class FramebufferAttachmentObjectImpl;
class TextureD3D;
class RenderbufferD3D;
class RendererD3D;
class RenderTargetD3D;

class EGLImageD3D final : public ImageImpl
{
  public:
    EGLImageD3D(const egl::ImageState &state,
                EGLenum target,
                const egl::AttributeMap &attribs,
                RendererD3D *renderer);
    ~EGLImageD3D() override;

    egl::Error initialize(const egl::Display *display) override;

    angle::Result orphan(const gl::Context *context, egl::ImageSibling *sibling) override;

    angle::Result getRenderTarget(const gl::Context *context, RenderTargetD3D **outRT) const;

  private:
    angle::Result copyToLocalRendertarget(const gl::Context *context);

    RendererD3D *mRenderer;
    RenderTargetD3D *mRenderTarget;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_EGLIMAGED3D_H_
