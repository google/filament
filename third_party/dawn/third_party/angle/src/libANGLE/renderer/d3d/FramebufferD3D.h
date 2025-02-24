//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferD3D.h: Defines the DefaultAttachmentD3D and FramebufferD3D classes.

#ifndef LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_
#define LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_

#include <cstdint>
#include <vector>

#include "common/Color.h"
#include "common/Optional.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/FramebufferImpl.h"

namespace gl
{
class FramebufferAttachment;
struct PixelPackState;

typedef std::vector<const FramebufferAttachment *> AttachmentList;
}  // namespace gl

namespace rx
{
class RendererD3D;
class RenderTargetD3D;

struct ClearParameters
{
    ClearParameters();
    ClearParameters(const ClearParameters &other);

    gl::DrawBufferMask clearColor;
    gl::ColorF colorF;
    gl::ColorI colorI;
    gl::ColorUI colorUI;
    GLenum colorType;
    gl::BlendStateExt::ColorMaskStorage::Type colorMask;

    bool clearDepth;
    float depthValue;

    bool clearStencil;
    GLint stencilValue;
    GLuint stencilWriteMask;

    bool scissorEnabled;
    gl::Rectangle scissor;
};

class FramebufferD3D : public FramebufferImpl
{
  public:
    FramebufferD3D(const gl::FramebufferState &data, RendererD3D *renderer);
    ~FramebufferD3D() override;

    angle::Result clear(const gl::Context *context, GLbitfield mask) override;
    angle::Result clearBufferfv(const gl::Context *context,
                                GLenum buffer,
                                GLint drawbuffer,
                                const GLfloat *values) override;
    angle::Result clearBufferuiv(const gl::Context *context,
                                 GLenum buffer,
                                 GLint drawbuffer,
                                 const GLuint *values) override;
    angle::Result clearBufferiv(const gl::Context *context,
                                GLenum buffer,
                                GLint drawbuffer,
                                const GLint *values) override;
    angle::Result clearBufferfi(const gl::Context *context,
                                GLenum buffer,
                                GLint drawbuffer,
                                GLfloat depth,
                                GLint stencil) override;

    angle::Result readPixels(const gl::Context *context,
                             const gl::Rectangle &area,
                             GLenum format,
                             GLenum type,
                             const gl::PixelPackState &pack,
                             gl::Buffer *packBuffer,
                             void *pixels) override;

    angle::Result blit(const gl::Context *context,
                       const gl::Rectangle &sourceArea,
                       const gl::Rectangle &destArea,
                       GLbitfield mask,
                       GLenum filter) override;

    gl::FramebufferStatus checkStatus(const gl::Context *context) const override;

    angle::Result syncState(const gl::Context *context,
                            GLenum binding,
                            const gl::Framebuffer::DirtyBits &dirtyBits,
                            gl::Command command) override;

    const gl::AttachmentList &getColorAttachmentsForRender(const gl::Context *context);

    const gl::DrawBufferMask getLastColorAttachmentsForRenderMask() const
    {
        return mColorAttachmentsForRenderMask;
    }

    void destroy(const gl::Context *context) override;

  private:
    virtual angle::Result clearImpl(const gl::Context *context,
                                    const ClearParameters &clearParams) = 0;

    virtual angle::Result readPixelsImpl(const gl::Context *context,
                                         const gl::Rectangle &area,
                                         GLenum format,
                                         GLenum type,
                                         size_t outputPitch,
                                         const gl::PixelPackState &pack,
                                         gl::Buffer *packBuffer,
                                         uint8_t *pixels) = 0;

    virtual angle::Result blitImpl(const gl::Context *context,
                                   const gl::Rectangle &sourceArea,
                                   const gl::Rectangle &destArea,
                                   const gl::Rectangle *scissor,
                                   bool blitRenderTarget,
                                   bool blitDepth,
                                   bool blitStencil,
                                   GLenum filter,
                                   const gl::Framebuffer *sourceFramebuffer) = 0;

    RendererD3D *mRenderer;
    Optional<gl::AttachmentList> mColorAttachmentsForRender;
    gl::DrawBufferMask mCurrentActiveProgramOutputs;
    gl::DrawBufferMask mColorAttachmentsForRenderMask;

    gl::FramebufferAttachment mMockAttachment;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_
