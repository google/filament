//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderBufferMtl.h:
//    Defines the class interface for RenderBufferMtl, implementing RenderBufferImpl.
//

#ifndef LIBANGLE_RENDERER_METAL_RENDERBUFFERMTL_H_
#define LIBANGLE_RENDERER_METAL_RENDERBUFFERMTL_H_

#include "libANGLE/renderer/RenderbufferImpl.h"
#include "libANGLE/renderer/metal/RenderTargetMtl.h"
#include "libANGLE/renderer/metal/mtl_resources.h"

namespace rx
{

class RenderbufferMtl : public RenderbufferImpl
{
  public:
    RenderbufferMtl(const gl::RenderbufferState &state);
    ~RenderbufferMtl() override;

    void onDestroy(const gl::Context *context) override;

    angle::Result setStorage(const gl::Context *context,
                             GLenum internalformat,
                             GLsizei width,
                             GLsizei height) override;
    angle::Result setStorageMultisample(const gl::Context *context,
                                        GLsizei samples,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height,
                                        gl::MultisamplingMode mode) override;
    angle::Result setStorageEGLImageTarget(const gl::Context *context, egl::Image *image) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

  private:
    angle::Result setStorageImpl(const gl::Context *context,
                                 GLsizei samples,
                                 GLenum internalformat,
                                 GLsizei width,
                                 GLsizei height,
                                 gl::MultisamplingMode mode);

    void releaseTexture();

    mtl::Format mFormat;
    mtl::TextureRef mTexture;
    mtl::TextureRef mImplicitMSTexture;
    RenderTargetMtl mRenderTarget;
};

}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_RENDERBUFFERMTL_H_ */
