//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_D3D_D3D11_EXTERNALIMAGESIBLINGIMPL11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_EXTERNALIMAGESIBLINGIMPL11_H_

#include "libANGLE/renderer/ImageImpl.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{

class Renderer11;
class RenderTargetD3D;

class ExternalImageSiblingImpl11 : public ExternalImageSiblingImpl
{
  public:
    ExternalImageSiblingImpl11(Renderer11 *renderer,
                               EGLClientBuffer clientBuffer,
                               const egl::AttributeMap &attribs);
    ~ExternalImageSiblingImpl11() override;

    // ExternalImageSiblingImpl interface
    egl::Error initialize(const egl::Display *display) override;
    gl::Format getFormat() const override;
    bool isRenderable(const gl::Context *context) const override;
    bool isTexturable(const gl::Context *context) const override;
    bool isYUV() const override;
    bool hasProtectedContent() const override;
    gl::Extents getSize() const override;
    size_t getSamples() const override;

    // FramebufferAttachmentObjectImpl interface
    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;
    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

  private:
    angle::Result createRenderTarget(const gl::Context *context);

    Renderer11 *mRenderer;
    EGLClientBuffer mBuffer;
    egl::AttributeMap mAttribs;

    TextureHelper11 mTexture;

    gl::Format mFormat   = gl::Format::Invalid();
    bool mIsRenderable   = false;
    bool mIsTexturable   = false;
    bool mIsTextureArray = false;
    EGLint mWidth        = 0;
    EGLint mHeight       = 0;
    GLsizei mSamples     = 0;
    UINT mArraySlice     = 0;

    std::unique_ptr<RenderTargetD3D> mRenderTarget;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_EXTERNALIMAGESIBLINGIMPL11_H_
