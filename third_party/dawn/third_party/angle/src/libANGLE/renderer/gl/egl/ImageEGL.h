//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageEGL.h: Defines the rx::ImageEGL class, the EGL implementation of EGL images

#ifndef LIBANGLE_RENDERER_GL_EGL_IMAGEEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_IMAGEEGL_H_

#include "libANGLE/renderer/gl/ImageGL.h"

namespace egl
{
class AttributeMap;
}

namespace rx
{

class FunctionsEGL;

class ImageEGL final : public ImageGL
{
  public:
    ImageEGL(const egl::ImageState &state,
             const gl::Context *context,
             EGLenum target,
             const egl::AttributeMap &attribs,
             const FunctionsEGL *egl);
    ~ImageEGL() override;

    egl::Error initialize(const egl::Display *display) override;

    angle::Result orphan(const gl::Context *context, egl::ImageSibling *sibling) override;

    angle::Result setTexture2D(const gl::Context *context,
                               gl::TextureType type,
                               TextureGL *texture,
                               GLenum *outInternalFormat) override;
    angle::Result setRenderbufferStorage(const gl::Context *context,
                                         RenderbufferGL *renderbuffer,
                                         GLenum *outInternalFormat) override;

  private:
    const FunctionsEGL *mEGL;

    // State needed for initialization
    EGLContext mContext;
    EGLenum mTarget;
    bool mPreserveImage;

    GLenum mNativeInternalFormat;

    EGLImage mImage;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_IMAGEEGL_H_
