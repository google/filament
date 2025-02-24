//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferVk.h:
//    Defines the class interface for FramebufferVk, implementing FramebufferImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_FRAMEBUFFERVK_H_
#define LIBANGLE_RENDERER_VULKAN_FRAMEBUFFERVK_H_

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/FramebufferImpl.h"
#include "libANGLE/renderer/RenderTargetCache.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/SurfaceVk.h"
#include "libANGLE/renderer/vulkan/UtilsVk.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_resource.h"

namespace rx
{
class RenderTargetVk;
class WindowSurfaceVk;

class FramebufferVk : public FramebufferImpl
{
  public:
    FramebufferVk(vk::Renderer *renderer, const gl::FramebufferState &state);
    ~FramebufferVk() override;
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
    RenderTargetVk *getDepthStencilRenderTarget() const
    {
        return mRenderTargetCache.getDepthStencil();
    }

    // Internal helper function for readPixels operations.
    angle::Result readPixelsImpl(ContextVk *contextVk,
                                 const gl::Rectangle &area,
                                 const PackPixelsParams &packPixelsParams,
                                 VkImageAspectFlagBits copyAspectFlags,
                                 RenderTargetVk *renderTarget,
                                 void *pixels);

    gl::Extents getReadImageExtents() const;
    // Return the framebuffer's non-rotated render area.  This is a gl::Rectangle that is based on
    // the dimensions of the framebuffer, IS NOT rotated, and IS NOT y-flipped
    gl::Rectangle getNonRotatedCompleteRenderArea() const
    {
        const gl::Box &dimensions = mState.getDimensions();
        return gl::Rectangle(0, 0, dimensions.width, dimensions.height);
    }
    gl::Rectangle getRotatedCompleteRenderArea(ContextVk *contextVk) const;
    gl::Rectangle getRotatedScissoredRenderArea(ContextVk *contextVk) const;
    // Returns render area with deferred clears in consideration. When deferred clear is used
    // in the render pass, the render area must cover the whole framebuffer.
    gl::Rectangle getRenderArea(ContextVk *contextVk) const;
    uint32_t getLayerCount() const { return mCurrentFramebufferDesc.getLayerCount(); }

    const gl::DrawBufferMask &getEmulatedAlphaAttachmentMask() const;
    RenderTargetVk *getColorDrawRenderTarget(size_t colorIndexGL) const;
    RenderTargetVk *getColorReadRenderTarget() const;

    angle::Result startNewRenderPass(ContextVk *contextVk,
                                     const gl::Rectangle &renderArea,
                                     vk::RenderPassCommandBuffer **commandBufferOut,
                                     bool *renderPassDescChangedOut);

    GLint getSamples() const;

    const vk::RenderPassDesc &getRenderPassDesc() const { return mRenderPassDesc; }

    angle::Result getFramebuffer(ContextVk *contextVk, vk::RenderPassFramebuffer *framebufferOut);

    bool hasDeferredClears() const { return !mDeferredClears.empty(); }
    bool hasDeferredDepthClear() const { return mDeferredClears.testDepth(); }
    bool hasDeferredStencilClear() const { return mDeferredClears.testStencil(); }
    angle::Result flushDepthStencilDeferredClear(ContextVk *contextVk,
                                                 VkImageAspectFlagBits aspect);

    void switchToColorFramebufferFetchMode(ContextVk *contextVk, bool hasColorFramebufferFetch);

    bool updateLegacyDither(ContextVk *contextVk);

    void setBackbuffer(WindowSurfaceVk *backbuffer) { mBackbuffer = backbuffer; }
    WindowSurfaceVk *getBackbuffer() const { return mBackbuffer; }

    void releaseCurrentFramebuffer(ContextVk *contextVk);

    const QueueSerial &getLastRenderPassQueueSerial() const { return mLastRenderPassQueueSerial; }

    bool hasAnyExternalAttachments() const { return mIsExternalColorAttachments.any(); }

    bool hasFrontBufferUsage() const
    {
        return (mAttachmentHasFrontBufferUsage & mState.getColorAttachmentsMask()).any();
    }

    bool isFoveationEnabled() { return mFoveationState.isFoveated(); }

  private:
    enum class ClearWithCommand
    {
        Always,
        OptimizeWithLoadOp,
    };

    enum class RenderTargetImage
    {
        Attachment,
        Resolve,
        FragmentShadingRate,
    };

    struct RenderTargetInfo
    {
        RenderTargetInfo() : renderTarget(nullptr), renderTargetImage(RenderTargetImage::Attachment)
        {}
        RenderTargetInfo(RenderTargetVk *renderTarget, RenderTargetImage renderTargetImage)
            : renderTarget(renderTarget), renderTargetImage(renderTargetImage)
        {}
        RenderTargetVk *renderTarget;
        RenderTargetImage renderTargetImage;
    };

    // Returns the attachments to be used to create a framebuffer.  The views returned in
    // |unpackedAttachments| are not necessarily packed, but the render targets in
    // |packedRenderTargetsInfoOut| are.  In particular, the resolve attachment views need to stay
    // sparse to be placed in |RenderPassFramebuffer|, but the calling function will have to pack
    // them to match the render buffers before creating a framebuffer.
    angle::Result getAttachmentsAndRenderTargets(
        vk::ErrorContext *context,
        vk::FramebufferAttachmentsVector<VkImageView> *unpackedAttachments,
        vk::FramebufferAttachmentsVector<RenderTargetInfo> *packedRenderTargetsInfoOut);

    angle::Result createNewFramebuffer(
        ContextVk *contextVk,
        uint32_t framebufferWidth,
        const uint32_t framebufferHeight,
        const uint32_t framebufferLayers,
        const vk::FramebufferAttachmentsVector<VkImageView> &unpackedAttachments,
        const vk::FramebufferAttachmentsVector<RenderTargetInfo> &renderTargetsInfo);

    // The 'in' rectangles must be clipped to the scissor and FBO. The clipping is done in 'blit'.
    angle::Result blitWithCommand(ContextVk *contextVk,
                                  const gl::Rectangle &sourceArea,
                                  const gl::Rectangle &destArea,
                                  RenderTargetVk *readRenderTarget,
                                  RenderTargetVk *drawRenderTarget,
                                  GLenum filter,
                                  bool colorBlit,
                                  bool depthBlit,
                                  bool stencilBlit,
                                  bool flipX,
                                  bool flipY);

    // Resolve color with subpass attachment
    angle::Result resolveColorWithSubpass(ContextVk *contextVk,
                                          const UtilsVk::BlitResolveParameters &params);

    // Resolve depth/stencil with subpass attachment
    angle::Result resolveDepthStencilWithSubpass(ContextVk *contextVk,
                                                 const UtilsVk::BlitResolveParameters &params,
                                                 VkImageAspectFlags aspects);

    // Resolve color with vkCmdResolveImage
    angle::Result resolveColorWithCommand(ContextVk *contextVk,
                                          const UtilsVk::BlitResolveParameters &params,
                                          vk::ImageHelper *srcImage);

    angle::Result clearImpl(const gl::Context *context,
                            gl::DrawBufferMask clearColorBuffers,
                            bool clearDepth,
                            bool clearStencil,
                            const VkClearColorValue &clearColorValue,
                            const VkClearDepthStencilValue &clearDepthStencilValue);

    void mergeClearsWithDeferredClears(
        gl::DrawBufferMask clearColorBuffers,
        bool clearDepth,
        bool clearStencil,
        const gl::DrawBuffersArray<VkClearColorValue> &clearColorValues,
        const VkClearDepthStencilValue &clearDepthStencilValue);
    angle::Result clearWithDraw(ContextVk *contextVk,
                                const gl::Rectangle &clearArea,
                                gl::DrawBufferMask clearColorBuffers,
                                bool clearDepth,
                                bool clearStencil,
                                gl::BlendStateExt::ColorMaskStorage::Type colorMasks,
                                uint8_t stencilMask,
                                const gl::DrawBuffersArray<VkClearColorValue> &clearColorValues,
                                const VkClearDepthStencilValue &clearDepthStencilValue);
    void restageDeferredClears(ContextVk *contextVk);
    void restageDeferredClearsForReadFramebuffer(ContextVk *contextVk);
    void restageDeferredClearsImpl(ContextVk *contextVk);
    angle::Result flushDeferredClears(ContextVk *contextVk);
    void clearWithCommand(ContextVk *contextVk,
                          const gl::Rectangle &scissoredRenderArea,
                          ClearWithCommand behavior,
                          vk::ClearValuesArray *clears);
    void clearWithLoadOp(ContextVk *contextVk);
    void updateActiveColorMasks(size_t colorIndex, bool r, bool g, bool b, bool a);
    void updateRenderPassDesc(ContextVk *contextVk);
    angle::Result updateColorAttachment(const gl::Context *context, uint32_t colorIndex);
    void updateColorAttachmentColorspace(gl::SrgbWriteControlMode srgbWriteControlMode);
    angle::Result updateDepthStencilAttachment(const gl::Context *context);
    void updateDepthStencilAttachmentSerial(ContextVk *contextVk);
    angle::Result flushColorAttachmentUpdates(const gl::Context *context,
                                              bool deferClears,
                                              uint32_t colorIndex);
    angle::Result flushDepthStencilAttachmentUpdates(const gl::Context *context, bool deferClears);
    angle::Result invalidateImpl(ContextVk *contextVk,
                                 size_t count,
                                 const GLenum *attachments,
                                 bool isSubInvalidate,
                                 const gl::Rectangle &invalidateArea);

    RenderTargetVk *getReadPixelsRenderTarget(GLenum format) const;
    VkImageAspectFlagBits getReadPixelsAspectFlags(GLenum format) const;

    VkClearValue getCorrectedColorClearValue(size_t colorIndexGL,
                                             const VkClearColorValue &clearColor) const;

    void updateLayerCount();

    angle::Result ensureFragmentShadingRateImageAndViewInitialized(
        ContextVk *contextVk,
        const uint32_t fragmentShadingRateAttachmentWidth,
        const uint32_t fragmentShadingRateAttachmentHeight);
    angle::Result generateFragmentShadingRateWithCPU(
        ContextVk *contextVk,
        const uint32_t fragmentShadingRateWidth,
        const uint32_t fragmentShadingRateHeight,
        const uint32_t fragmentShadingRateBlockWidth,
        const uint32_t fragmentShadingRateBlockHeight,
        const uint32_t foveatedAttachmentWidth,
        const uint32_t foveatedAttachmentHeight,
        const std::vector<gl::FocalPoint> &activeFocalPoints);
    angle::Result generateFragmentShadingRateWithCompute(
        ContextVk *contextVk,
        const uint32_t fragmentShadingRateWidth,
        const uint32_t fragmentShadingRateHeight,
        const uint32_t fragmentShadingRateBlockWidth,
        const uint32_t fragmentShadingRateBlockHeight,
        const uint32_t foveatedAttachmentWidth,
        const uint32_t foveatedAttachmentHeight,
        const std::vector<gl::FocalPoint> &activeFocalPoints);
    angle::Result updateFragmentShadingRateAttachment(ContextVk *contextVk,
                                                      const gl::FoveationState &foveationState,
                                                      const gl::Extents &foveatedAttachmentSize);
    angle::Result updateFoveationState(ContextVk *contextVk,
                                       const gl::FoveationState &newFoveationState,
                                       const gl::Extents &foveatedAttachmentSize);

    void insertCache(ContextVk *contextVk,
                     const vk::FramebufferDesc &desc,
                     vk::FramebufferHelper &&newFramebuffer);

    WindowSurfaceVk *mBackbuffer;

    vk::RenderPassDesc mRenderPassDesc;
    RenderTargetCache<RenderTargetVk> mRenderTargetCache;

    // This variable is used to quickly compute if we need to do a masked clear. If a color
    // channel is masked out, we check against the Framebuffer Attachments (RenderTargets) to see
    // if the masked out channel is present in any of the attachments.
    gl::BlendStateExt::ColorMaskStorage::Type mActiveColorComponentMasksForClear;

    // When we draw to the framebuffer, and the real format has an alpha channel but the format of
    // the framebuffer does not, we need to mask out the alpha channel. This DrawBufferMask will
    // contain the mask to apply to the alpha channel when drawing.
    gl::DrawBufferMask mEmulatedAlphaAttachmentMask;

    // mCurrentFramebufferDesc is used to detect framebuffer changes using its serials. Therefore,
    // it must be maintained even when using the imageless framebuffer extension.
    vk::FramebufferDesc mCurrentFramebufferDesc;

    // The framebuffer cache actually owns the Framebuffer object and manages its lifetime. We just
    // store the current VkFramebuffer handle here that associated with mCurrentFramebufferDesc.
    vk::Framebuffer mCurrentFramebuffer;

    vk::ClearValuesArray mDeferredClears;

    // Whether any of the color attachments are an external image such as dmabuf, AHB etc.  In such
    // cases, some optimizations are disabled such as deferred clears because the results need to be
    // made externally available.
    gl::DrawBufferMask mIsExternalColorAttachments;
    gl::DrawBufferMask mAttachmentHasFrontBufferUsage;

    bool mIsCurrentFramebufferCached;
    bool mIsYUVResolve;

    gl::FoveationState mFoveationState;
    vk::ImageHelper mFragmentShadingRateImage;
    vk::ImageViewHelper mFragmentShadingRateImageView;

    // Serial of the render pass this framebuffer has opened, if any.
    QueueSerial mLastRenderPassQueueSerial;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_FRAMEBUFFERVK_H_
