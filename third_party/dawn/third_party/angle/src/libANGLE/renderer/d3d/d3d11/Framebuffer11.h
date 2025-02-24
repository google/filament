//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer11.h: Defines the Framebuffer11 class.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_

#include "libANGLE/Observer.h"
#include "libANGLE/renderer/RenderTargetCache.h"
#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{
class Renderer11;

class Framebuffer11 : public FramebufferD3D
{
  public:
    Framebuffer11(const gl::FramebufferState &data, Renderer11 *renderer);
    ~Framebuffer11() override;

    angle::Result discard(const gl::Context *context,
                          size_t count,
                          const GLenum *attachments) override;
    angle::Result invalidate(const gl::Context *context,
                             size_t count,
                             const GLenum *attachments) override;
    angle::Result invalidateSub(const gl::Context *context,
                                size_t count,
                                const GLenum *attachments,
                                const gl::Rectangle &area) override;

    // Invalidate the cached swizzles of all bound texture attachments.
    angle::Result markAttachmentsDirty(const gl::Context *context) const;

    angle::Result syncState(const gl::Context *context,
                            GLenum binding,
                            const gl::Framebuffer::DirtyBits &dirtyBits,
                            gl::Command command) override;

    const gl::AttachmentArray<RenderTarget11 *> &getCachedColorRenderTargets() const
    {
        return mRenderTargetCache.getColors();
    }
    const RenderTarget11 *getCachedDepthStencilRenderTarget() const
    {
        return mRenderTargetCache.getDepthStencil();
    }

    RenderTarget11 *getFirstRenderTarget() const;

    angle::Result getSamplePosition(const gl::Context *context,
                                    size_t index,
                                    GLfloat *xy) const override;

    const gl::InternalFormat &getImplementationColorReadFormat(
        const gl::Context *context) const override;

  private:
    angle::Result clearImpl(const gl::Context *context,
                            const ClearParameters &clearParams) override;

    angle::Result readPixelsImpl(const gl::Context *context,
                                 const gl::Rectangle &area,
                                 GLenum format,
                                 GLenum type,
                                 size_t outputPitch,
                                 const gl::PixelPackState &pack,
                                 gl::Buffer *packBuffer,
                                 uint8_t *pixels) override;

    angle::Result blitImpl(const gl::Context *context,
                           const gl::Rectangle &sourceArea,
                           const gl::Rectangle &destArea,
                           const gl::Rectangle *scissor,
                           bool blitRenderTarget,
                           bool blitDepth,
                           bool blitStencil,
                           GLenum filter,
                           const gl::Framebuffer *sourceFramebuffer) override;

    angle::Result invalidateBase(const gl::Context *context,
                                 size_t count,
                                 const GLenum *attachments,
                                 bool useEXTBehavior) const;
    angle::Result invalidateAttachment(const gl::Context *context,
                                       const gl::FramebufferAttachment *attachment) const;

    Renderer11 *const mRenderer;
    RenderTargetCache<RenderTarget11> mRenderTargetCache;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_
