//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FrameBufferMtl.h:
//    Defines the class interface for FrameBufferMtl, implementing FrameBufferImpl.
//

#ifndef LIBANGLE_RENDERER_METAL_FRAMEBUFFERMTL_H_
#define LIBANGLE_RENDERER_METAL_FRAMEBUFFERMTL_H_

#import <Metal/Metal.h>

#include "common/FixedVector.h"
#include "libANGLE/renderer/FramebufferImpl.h"
#include "libANGLE/renderer/metal/RenderTargetMtl.h"
#include "libANGLE/renderer/metal/mtl_render_utils.h"

namespace rx
{
namespace mtl
{
class RenderCommandEncoder;
}  // namespace mtl
class ContextMtl;
class SurfaceMtl;

class FramebufferMtl : public FramebufferImpl
{
  public:
    explicit FramebufferMtl(const gl::FramebufferState &state, ContextMtl *context, bool flipY);
    ~FramebufferMtl() override;
    void destroy(const gl::Context *context) override;

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

    const gl::InternalFormat &getImplementationColorReadFormat(
        const gl::Context *context) const override;
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

    angle::Result getSamplePosition(const gl::Context *context,
                                    size_t index,
                                    GLfloat *xy) const override;

    RenderTargetMtl *getColorReadRenderTarget(const gl::Context *context) const;
    RenderTargetMtl *getDepthRenderTarget() const { return mDepthRenderTarget; }
    RenderTargetMtl *getStencilRenderTarget() const { return mStencilRenderTarget; }

    void setFlipY(bool flipY) { mFlipY = flipY; }
    bool flipY() const { return mFlipY; }

    gl::Rectangle getCompleteRenderArea() const;
    int getSamples() const;
    WindowSurfaceMtl *getAttachedBackbuffer() const { return mBackbuffer; }

    bool renderPassHasStarted(ContextMtl *contextMtl) const;
    bool renderPassHasDefaultWidthOrHeight() const
    {
        return mRenderPassDesc.defaultWidth > 0 || mRenderPassDesc.defaultHeight > 0;
    }
    angle::Result ensureRenderPassStarted(const gl::Context *context,
                                          mtl::RenderCommandEncoder **encoderOut);

    // Call this to notify FramebufferMtl whenever its render pass has started.
    void onStartedDrawingToFrameBuffer(const gl::Context *context);
    void onFrameEnd(const gl::Context *context);

    // The actual area will be adjusted based on framebuffer flipping property.
    gl::Rectangle getCorrectFlippedReadArea(const gl::Context *context,
                                            const gl::Rectangle &glArea) const;

    // NOTE: this method doesn't do the flipping of area. Caller must do it if needed before
    // callling this. See getReadPixelsArea().
    angle::Result readPixelsImpl(const gl::Context *context,
                                 const gl::Rectangle &area,
                                 const PackPixelsParams &packPixelsParams,
                                 const RenderTargetMtl *renderTarget,
                                 uint8_t *pixels) const;
    void setBackbuffer(WindowSurfaceMtl *backbuffer) { mBackbuffer = backbuffer; }
    WindowSurfaceMtl *getBackbuffer() const { return mBackbuffer; }

  private:
    void reset();
    angle::Result invalidateImpl(const gl::Context *context,
                                 size_t count,
                                 const GLenum *attachments);
    angle::Result blitWithDraw(const gl::Context *context,
                               FramebufferMtl *srcFrameBuffer,
                               bool blitColorBuffer,
                               bool blitDepthBuffer,
                               bool blitStencilBuffer,
                               GLenum filter,
                               const mtl::BlitParams &baseParams);
    angle::Result clearImpl(const gl::Context *context,
                            gl::DrawBufferMask clearColorBuffers,
                            mtl::ClearRectParams *clearOpts);

    angle::Result clearWithLoadOp(const gl::Context *context,
                                  gl::DrawBufferMask clearColorBuffers,
                                  const mtl::ClearRectParams &clearOpts);

    angle::Result clearWithLoadOpRenderPassNotStarted(const gl::Context *context,
                                                      gl::DrawBufferMask clearColorBuffers,
                                                      const mtl::ClearRectParams &clearOpts);

    angle::Result clearWithLoadOpRenderPassStarted(const gl::Context *context,
                                                   gl::DrawBufferMask clearColorBuffers,
                                                   const mtl::ClearRectParams &clearOpts,
                                                   mtl::RenderCommandEncoder *encoder);

    angle::Result clearWithDraw(const gl::Context *context,
                                gl::DrawBufferMask clearColorBuffers,
                                const mtl::ClearRectParams &clearOpts);

    // Initialize load store options for a render pass's first start (i.e. not render pass resuming
    // from interruptions such as those caused by a conversion compute pass)
    void setLoadStoreActionOnRenderPassFirstStart(mtl::RenderPassAttachmentDesc *attachmentOut,
                                                  const bool forceDepthStencilMultisampleLoad);

    // Fill RenderPassDesc with relevant attachment's info from GL front end.
    angle::Result prepareRenderPass(const gl::Context *context,
                                    mtl::RenderPassDesc *descOut,
                                    gl::Command command);

    // Check if a render pass specified by the given RenderPassDesc has started or not, if not this
    // method will start the render pass and return its render encoder.
    angle::Result ensureRenderPassStarted(const gl::Context *context,
                                          const mtl::RenderPassDesc &desc,
                                          mtl::RenderCommandEncoder **encoderOut);

    angle::Result updateColorRenderTarget(const gl::Context *context, size_t colorIndexGL);
    angle::Result updateDepthRenderTarget(const gl::Context *context);
    angle::Result updateStencilRenderTarget(const gl::Context *context);
    angle::Result updateCachedRenderTarget(const gl::Context *context,
                                           const gl::FramebufferAttachment *attachment,
                                           RenderTargetMtl **cachedRenderTarget);

    angle::Result readPixelsToPBO(const gl::Context *context,
                                  const gl::Rectangle &area,
                                  const PackPixelsParams &packPixelsParams,
                                  const RenderTargetMtl *renderTarget) const;

    angle::Result readPixelsToBuffer(const gl::Context *context,
                                     const gl::Rectangle &area,
                                     const RenderTargetMtl *renderTarget,
                                     bool reverseRowOrder,
                                     const angle::Format &dstAngleFormat,
                                     uint32_t dstBufferOffset,
                                     uint32_t dstBufferRowPitch,
                                     const mtl::BufferRef *dstBuffer) const;

    bool totalBitsUsedIsLessThanOrEqualToMaxBitsSupported(const gl::Context *context) const;

    RenderTargetMtl *getColorReadRenderTargetNoCache(const gl::Context *context) const;
    angle::Result prepareForUse(const gl::Context *context) const;

    // Perform unresolve step for loading into memoryless MS attachments.
    angle::Result unresolveIfNeeded(const gl::Context *context, mtl::RenderCommandEncoder *encoder);

    // NOTE: we cannot use RenderTargetCache here because it doesn't support separate
    // depth & stencil attachments as of now. Separate depth & stencil could be useful to
    // save spaces on iOS devices. See doc/PackedDepthStencilSupport.md.
    angle::FixedVector<RenderTargetMtl *, mtl::kMaxRenderTargets> mColorRenderTargets;
    RenderTargetMtl *mDepthRenderTarget   = nullptr;
    RenderTargetMtl *mStencilRenderTarget = nullptr;
    mtl::RenderPassDesc mRenderPassDesc;

    const mtl::Format *mRenderPassFirstColorAttachmentFormat = nullptr;
    bool mRenderPassAttachmentsSameColorType                 = false;

    // Flag indicating the render pass start is a clean start or a resume from interruption such
    // as by a compute pass.
    bool mRenderPassCleanStart = false;

    WindowSurfaceMtl *mBackbuffer = nullptr;
    bool mFlipY                   = false;

    mtl::BufferRef mReadPixelBuffer;

    uint64_t mStartedRenderEncoderSerial = 0;
};
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_FRAMEBUFFERMTL_H */
