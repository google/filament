//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferMtl.mm:
//    Implements the class methods for FramebufferMtl.
//

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/metal/ContextMtl.h"

#include <TargetConditionals.h>

#include "common/MemoryBuffer.h"
#include "common/angleutils.h"
#include "common/debug.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/renderer/metal/BufferMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/FrameBufferMtl.h"
#include "libANGLE/renderer/metal/SurfaceMtl.h"
#include "libANGLE/renderer/metal/mtl_utils.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace rx
{
namespace
{
// Override clear color based on texture's write mask
void OverrideMTLClearColor(const mtl::TextureRef &texture,
                           const mtl::ClearColorValue &clearColor,
                           MTLClearColor *colorOut)
{
    if (texture)
    {
        *colorOut = mtl::EmulatedAlphaClearColor(clearColor.toMTLClearColor(),
                                                 texture->getColorWritableMask());
    }
    else
    {
        *colorOut = clearColor.toMTLClearColor();
    }
}

const gl::InternalFormat &GetReadAttachmentInfo(const gl::Context *context,
                                                RenderTargetMtl *renderTarget)
{
    GLenum implFormat;

    if (renderTarget)
    {
        implFormat = renderTarget->getFormat().actualAngleFormat().fboImplementationInternalFormat;
    }
    else
    {
        implFormat = GL_NONE;
    }

    return gl::GetSizedInternalFormatInfo(implFormat);
}

angle::Result CopyTextureSliceLevelToTempBuffer(const gl::Context *context,
                                                const mtl::TextureRef &srcTexture,
                                                const mtl::MipmapNativeLevel &mipNativeLevel,
                                                uint32_t layerIndex,
                                                mtl::BufferRef *outBuffer)
{
    ASSERT(outBuffer);

    ContextMtl *contextMtl           = mtl::GetImpl(context);
    auto formatId                    = mtl::Format::MetalToAngleFormatID(srcTexture->pixelFormat());
    const mtl::Format &metalFormat   = contextMtl->getPixelFormat(formatId);
    const angle::Format &angleFormat = metalFormat.actualAngleFormat();

    uint32_t width       = srcTexture->width(mipNativeLevel);
    uint32_t height      = srcTexture->height(mipNativeLevel);
    uint32_t sizeInBytes = width * height * angleFormat.pixelBytes;

    mtl::BufferRef tempBuffer;
    ANGLE_TRY(mtl::Buffer::MakeBufferWithStorageMode(
        contextMtl, mtl::Buffer::getStorageModeForSharedBuffer(contextMtl), sizeInBytes, nullptr,
        &tempBuffer));

    gl::Rectangle region(0, 0, width, height);
    uint32_t bytesPerRow = angleFormat.pixelBytes * width;
    uint32_t destOffset  = 0;
    ANGLE_TRY(mtl::ReadTexturePerSliceBytesToBuffer(context, srcTexture, bytesPerRow, region,
                                                    mipNativeLevel, layerIndex, destOffset,
                                                    tempBuffer));

    *outBuffer = tempBuffer;
    return angle::Result::Continue;
}

angle::Result Copy2DTextureSlice0Level0ToTempTexture(const gl::Context *context,
                                                     const mtl::TextureRef &srcTexture,
                                                     mtl::TextureRef *outTexture)
{
    ASSERT(outTexture);

    ContextMtl *contextMtl = mtl::GetImpl(context);
    auto formatId          = mtl::Format::MetalToAngleFormatID(srcTexture->pixelFormat());
    const auto &format     = contextMtl->getPixelFormat(formatId);

    mtl::TextureRef tempTexture;
    ANGLE_TRY(mtl::Texture::Make2DTexture(contextMtl, format, srcTexture->widthAt0(),
                                          srcTexture->heightAt0(), srcTexture->mipmapLevels(),
                                          false, true, &tempTexture));

    auto *blitEncoder = contextMtl->getBlitCommandEncoder();
    blitEncoder->copyTexture(srcTexture,
                             0,                          // srcStartSlice
                             mtl::MipmapNativeLevel(0),  // MipmapNativeLevel
                             tempTexture,                // dst
                             0,                          // dstStartSlice
                             mtl::MipmapNativeLevel(0),  // dstStartLevel
                             1,                          // sliceCount,
                             1);                         // levelCount

    *outTexture = tempTexture;
    return angle::Result::Continue;
}

}  // namespace

// FramebufferMtl implementation
FramebufferMtl::FramebufferMtl(const gl::FramebufferState &state, ContextMtl *context, bool flipY)
    : FramebufferImpl(state),
      mColorRenderTargets(context->getNativeCaps().maxColorAttachments, nullptr),
      mBackbuffer(nullptr),
      mFlipY(flipY)
{
    reset();
}

FramebufferMtl::~FramebufferMtl() {}

void FramebufferMtl::reset()
{
    for (auto &rt : mColorRenderTargets)
    {
        rt = nullptr;
    }
    mDepthRenderTarget = mStencilRenderTarget = nullptr;

    mRenderPassFirstColorAttachmentFormat = nullptr;

    mReadPixelBuffer = nullptr;
}

void FramebufferMtl::destroy(const gl::Context *context)
{
    reset();
}

angle::Result FramebufferMtl::discard(const gl::Context *context,
                                      size_t count,
                                      const GLenum *attachments)
{
    return invalidate(context, count, attachments);
}

angle::Result FramebufferMtl::invalidate(const gl::Context *context,
                                         size_t count,
                                         const GLenum *attachments)
{
    return invalidateImpl(context, count, attachments);
}

angle::Result FramebufferMtl::invalidateSub(const gl::Context *context,
                                            size_t count,
                                            const GLenum *attachments,
                                            const gl::Rectangle &area)
{
    if (area.encloses(getCompleteRenderArea()))
    {
        return invalidateImpl(context, count, attachments);
    }
    return angle::Result::Continue;
}

angle::Result FramebufferMtl::clear(const gl::Context *context, GLbitfield mask)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);

    if (ANGLE_UNLIKELY(contextMtl->getForceResyncDrawFramebuffer()))
    {
        ANGLE_TRY(syncState(context, GL_DRAW_FRAMEBUFFER, gl::Framebuffer::DirtyBits(),
                            gl::Command::Clear));
    }

    mtl::ClearRectParams clearOpts;

    bool clearColor   = IsMaskFlagSet(mask, static_cast<GLbitfield>(GL_COLOR_BUFFER_BIT));
    bool clearDepth   = IsMaskFlagSet(mask, static_cast<GLbitfield>(GL_DEPTH_BUFFER_BIT));
    bool clearStencil = IsMaskFlagSet(mask, static_cast<GLbitfield>(GL_STENCIL_BUFFER_BIT));

    gl::DrawBufferMask clearColorBuffers;
    if (clearColor)
    {
        clearColorBuffers    = mState.getEnabledDrawBuffers();
        clearOpts.clearColor = contextMtl->getClearColorValue();
    }
    if (clearDepth)
    {
        clearOpts.clearDepth = contextMtl->getClearDepthValue();
    }
    if (clearStencil)
    {
        clearOpts.clearStencil = contextMtl->getClearStencilValue();
    }

    return clearImpl(context, clearColorBuffers, &clearOpts);
}

angle::Result FramebufferMtl::clearBufferfv(const gl::Context *context,
                                            GLenum buffer,
                                            GLint drawbuffer,
                                            const GLfloat *values)
{
    if (ANGLE_UNLIKELY(mtl::GetImpl(context)->getForceResyncDrawFramebuffer()))
    {
        ANGLE_TRY(syncState(context, GL_DRAW_FRAMEBUFFER, gl::Framebuffer::DirtyBits(),
                            gl::Command::Clear));
    }

    mtl::ClearRectParams clearOpts;

    gl::DrawBufferMask clearColorBuffers;
    if (buffer == GL_DEPTH)
    {
        clearOpts.clearDepth = values[0];
    }
    else
    {
        clearColorBuffers.set(drawbuffer);
        clearOpts.clearColor = mtl::ClearColorValue(values[0], values[1], values[2], values[3]);
    }

    return clearImpl(context, clearColorBuffers, &clearOpts);
}
angle::Result FramebufferMtl::clearBufferuiv(const gl::Context *context,
                                             GLenum buffer,
                                             GLint drawbuffer,
                                             const GLuint *values)
{
    if (ANGLE_UNLIKELY(mtl::GetImpl(context)->getForceResyncDrawFramebuffer()))
    {
        ANGLE_TRY(syncState(context, GL_DRAW_FRAMEBUFFER, gl::Framebuffer::DirtyBits(),
                            gl::Command::Clear));
    }

    gl::DrawBufferMask clearColorBuffers;
    clearColorBuffers.set(drawbuffer);

    mtl::ClearRectParams clearOpts;
    clearOpts.clearColor = mtl::ClearColorValue(values[0], values[1], values[2], values[3]);

    return clearImpl(context, clearColorBuffers, &clearOpts);
}
angle::Result FramebufferMtl::clearBufferiv(const gl::Context *context,
                                            GLenum buffer,
                                            GLint drawbuffer,
                                            const GLint *values)
{
    if (ANGLE_UNLIKELY(mtl::GetImpl(context)->getForceResyncDrawFramebuffer()))
    {
        ANGLE_TRY(syncState(context, GL_DRAW_FRAMEBUFFER, gl::Framebuffer::DirtyBits(),
                            gl::Command::Clear));
    }

    mtl::ClearRectParams clearOpts;

    gl::DrawBufferMask clearColorBuffers;
    if (buffer == GL_STENCIL)
    {
        clearOpts.clearStencil = values[0] & mtl::kStencilMaskAll;
    }
    else
    {
        clearColorBuffers.set(drawbuffer);
        clearOpts.clearColor = mtl::ClearColorValue(values[0], values[1], values[2], values[3]);
    }

    return clearImpl(context, clearColorBuffers, &clearOpts);
}
angle::Result FramebufferMtl::clearBufferfi(const gl::Context *context,
                                            GLenum buffer,
                                            GLint drawbuffer,
                                            GLfloat depth,
                                            GLint stencil)
{
    mtl::ClearRectParams clearOpts;
    clearOpts.clearDepth   = depth;
    clearOpts.clearStencil = stencil & mtl::kStencilMaskAll;

    return clearImpl(context, gl::DrawBufferMask(), &clearOpts);
}

const gl::InternalFormat &FramebufferMtl::getImplementationColorReadFormat(
    const gl::Context *context) const
{
    return GetReadAttachmentInfo(context, getColorReadRenderTargetNoCache(context));
}

angle::Result FramebufferMtl::readPixels(const gl::Context *context,
                                         const gl::Rectangle &area,
                                         GLenum format,
                                         GLenum type,
                                         const gl::PixelPackState &pack,
                                         gl::Buffer *packBuffer,
                                         void *pixels)
{
    // Clip read area to framebuffer.
    const gl::Extents &fbSize = getState().getReadAttachment()->getSize();
    const gl::Rectangle fbRect(0, 0, fbSize.width, fbSize.height);

    gl::Rectangle clippedArea;
    if (!ClipRectangle(area, fbRect, &clippedArea))
    {
        // nothing to read
        return angle::Result::Continue;
    }
    gl::Rectangle flippedArea = getCorrectFlippedReadArea(context, clippedArea);

    ContextMtl *contextMtl = mtl::GetImpl(context);

    const gl::InternalFormat &sizedFormatInfo = gl::GetInternalFormatInfo(format, type);

    GLuint outputPitch = 0;
    ANGLE_CHECK_GL_MATH(contextMtl,
                        sizedFormatInfo.computeRowPitch(type, area.width, pack.alignment,
                                                        pack.rowLength, &outputPitch));
    GLuint outputSkipBytes = 0;
    ANGLE_CHECK_GL_MATH(contextMtl, sizedFormatInfo.computeSkipBytes(type, outputPitch, 0, pack,
                                                                     false, &outputSkipBytes));

    outputSkipBytes += (clippedArea.x - area.x) * sizedFormatInfo.pixelBytes +
                       (clippedArea.y - area.y) * outputPitch;

    const angle::Format &angleFormat = GetFormatFromFormatType(format, type);

    PackPixelsParams params(flippedArea, angleFormat, outputPitch, pack.reverseRowOrder, packBuffer,
                            0);

    if (params.packBuffer)
    {
        // If PBO is active, pixels is treated as offset.
        params.offset = reinterpret_cast<ptrdiff_t>(pixels) + outputSkipBytes;
    }

    if (mFlipY)
    {
        params.reverseRowOrder = !params.reverseRowOrder;
    }

    ANGLE_TRY(readPixelsImpl(context, flippedArea, params, getColorReadRenderTarget(context),
                             static_cast<uint8_t *>(pixels) + outputSkipBytes));

    return angle::Result::Continue;
}

namespace
{

using FloatRectangle = gl::RectangleImpl<float>;

float clamp0Max(float v, float max)
{
    return std::max(0.0f, std::min(max, v));
}

void ClampToBoundsAndAdjustCorrespondingValue(float a,
                                              float originalASize,
                                              float maxSize,
                                              float b,
                                              float originalBSize,
                                              float *newA,
                                              float *newB)
{
    float clippedA = clamp0Max(a, maxSize);
    float delta    = clippedA - a;
    *newA          = clippedA;
    *newB          = b + delta * originalBSize / originalASize;
}

void ClipRectToBoundsAndAdjustCorrespondingRect(const FloatRectangle &a,
                                                const gl::Rectangle &originalA,
                                                const gl::Rectangle &clipDimensions,
                                                const FloatRectangle &b,
                                                const gl::Rectangle &originalB,
                                                FloatRectangle *newA,
                                                FloatRectangle *newB)
{
    float newAValues[4];
    float newBValues[4];
    ClampToBoundsAndAdjustCorrespondingValue(a.x0(), originalA.width, clipDimensions.width, b.x0(),
                                             originalB.width, &newAValues[0], &newBValues[0]);
    ClampToBoundsAndAdjustCorrespondingValue(a.y0(), originalA.height, clipDimensions.height,
                                             b.y0(), originalB.height, &newAValues[1],
                                             &newBValues[1]);
    ClampToBoundsAndAdjustCorrespondingValue(a.x1(), originalA.width, clipDimensions.width, b.x1(),
                                             originalB.width, &newAValues[2], &newBValues[2]);
    ClampToBoundsAndAdjustCorrespondingValue(a.y1(), originalA.height, clipDimensions.height,
                                             b.y1(), originalB.height, &newAValues[3],
                                             &newBValues[3]);

    *newA = FloatRectangle(newAValues);
    *newB = FloatRectangle(newBValues);
}

void ClipRectsToBoundsAndAdjustCorrespondingRect(const FloatRectangle &a,
                                                 const gl::Rectangle &originalA,
                                                 const gl::Rectangle &aClipDimensions,
                                                 const FloatRectangle &b,
                                                 const gl::Rectangle &originalB,
                                                 const gl::Rectangle &bClipDimensions,
                                                 FloatRectangle *newA,
                                                 FloatRectangle *newB)
{
    FloatRectangle tempA;
    FloatRectangle tempB;
    ClipRectToBoundsAndAdjustCorrespondingRect(a, originalA, aClipDimensions, b, originalB, &tempA,
                                               &tempB);
    ClipRectToBoundsAndAdjustCorrespondingRect(tempB, originalB, bClipDimensions, tempA, originalA,
                                               newB, newA);
}

void RoundValueAndAdjustCorrespondingValue(float a,
                                           float originalASize,
                                           float b,
                                           float originalBSize,
                                           int *newA,
                                           float *newB)
{
    float roundedA = std::round(a);
    float delta    = roundedA - a;
    *newA          = static_cast<int>(roundedA);
    *newB          = b + delta * originalBSize / originalASize;
}

gl::Rectangle RoundRectToPixelsAndAdjustCorrespondingRectToMatch(const FloatRectangle &a,
                                                                 const gl::Rectangle &originalA,
                                                                 const FloatRectangle &b,
                                                                 const gl::Rectangle &originalB,
                                                                 FloatRectangle *newB)
{
    int newAValues[4];
    float newBValues[4];
    RoundValueAndAdjustCorrespondingValue(a.x0(), originalA.width, b.x0(), originalB.width,
                                          &newAValues[0], &newBValues[0]);
    RoundValueAndAdjustCorrespondingValue(a.y0(), originalA.height, b.y0(), originalB.height,
                                          &newAValues[1], &newBValues[1]);
    RoundValueAndAdjustCorrespondingValue(a.x1(), originalA.width, b.x1(), originalB.width,
                                          &newAValues[2], &newBValues[2]);
    RoundValueAndAdjustCorrespondingValue(a.y1(), originalA.height, b.y1(), originalB.height,
                                          &newAValues[3], &newBValues[3]);

    *newB = FloatRectangle(newBValues);
    return gl::Rectangle(newAValues[0], newAValues[1], newAValues[2] - newAValues[0],
                         newAValues[3] - newAValues[1]);
}

}  // namespace

angle::Result FramebufferMtl::blit(const gl::Context *context,
                                   const gl::Rectangle &sourceAreaIn,
                                   const gl::Rectangle &destAreaIn,
                                   GLbitfield mask,
                                   GLenum filter)
{
    bool blitColorBuffer   = (mask & GL_COLOR_BUFFER_BIT) != 0;
    bool blitDepthBuffer   = (mask & GL_DEPTH_BUFFER_BIT) != 0;
    bool blitStencilBuffer = (mask & GL_STENCIL_BUFFER_BIT) != 0;

    const gl::State &glState                = context->getState();
    const gl::Framebuffer *glSrcFramebuffer = glState.getReadFramebuffer();

    FramebufferMtl *srcFrameBuffer = mtl::GetImpl(glSrcFramebuffer);

    blitColorBuffer =
        blitColorBuffer && srcFrameBuffer->getColorReadRenderTarget(context) != nullptr;
    blitDepthBuffer   = blitDepthBuffer && srcFrameBuffer->getDepthRenderTarget() != nullptr;
    blitStencilBuffer = blitStencilBuffer && srcFrameBuffer->getStencilRenderTarget() != nullptr;

    if (!blitColorBuffer && !blitDepthBuffer && !blitStencilBuffer)
    {
        // No-op
        return angle::Result::Continue;
    }

    if (ANGLE_UNLIKELY(mtl::GetImpl(context)->getForceResyncDrawFramebuffer()))
    {
        ANGLE_TRY(syncState(context, GL_DRAW_FRAMEBUFFER, gl::Framebuffer::DirtyBits(),
                            gl::Command::Blit));
    }

    const gl::Rectangle srcFramebufferDimensions = srcFrameBuffer->getCompleteRenderArea();
    const gl::Rectangle dstFramebufferDimensions = this->getCompleteRenderArea();

    FloatRectangle srcRect(sourceAreaIn);
    FloatRectangle dstRect(destAreaIn);

    FloatRectangle clippedSrcRect;
    FloatRectangle clippedDstRect;
    ClipRectsToBoundsAndAdjustCorrespondingRect(srcRect, sourceAreaIn, srcFramebufferDimensions,
                                                dstRect, destAreaIn, dstFramebufferDimensions,
                                                &clippedSrcRect, &clippedDstRect);

    FloatRectangle adjustedSrcRect;
    gl::Rectangle srcClippedDestArea = RoundRectToPixelsAndAdjustCorrespondingRectToMatch(
        clippedDstRect, destAreaIn, clippedSrcRect, sourceAreaIn, &adjustedSrcRect);

    if (srcFrameBuffer->flipY())
    {
        adjustedSrcRect.y =
            srcFramebufferDimensions.height - adjustedSrcRect.y - adjustedSrcRect.height;
        adjustedSrcRect = adjustedSrcRect.flip(false, true);
    }

    // If the destination is flipped in either direction, we will flip the source instead so that
    // the destination area is always unflipped.
    adjustedSrcRect =
        adjustedSrcRect.flip(srcClippedDestArea.isReversedX(), srcClippedDestArea.isReversedY());
    srcClippedDestArea = srcClippedDestArea.removeReversal();

    // Clip the destination area to the framebuffer size and scissor.
    gl::Rectangle scissoredDestArea;
    if (!gl::ClipRectangle(ClipRectToScissor(glState, dstFramebufferDimensions, false),
                           srcClippedDestArea, &scissoredDestArea))
    {
        return angle::Result::Continue;
    }

    // Use blit with draw
    mtl::BlitParams baseParams;
    baseParams.dstTextureSize =
        gl::Extents(dstFramebufferDimensions.width, dstFramebufferDimensions.height, 1);
    baseParams.dstRect        = srcClippedDestArea;
    baseParams.dstScissorRect = scissoredDestArea;
    baseParams.dstFlipY       = this->flipY();

    baseParams.srcNormalizedCoords =
        mtl::NormalizedCoords(adjustedSrcRect.x, adjustedSrcRect.y, adjustedSrcRect.width,
                              adjustedSrcRect.height, srcFramebufferDimensions);
    // This flag is for auto flipping the rect inside RenderUtils. Since we already flip it using
    // getCorrectFlippedReadArea(). This flag is not needed.
    baseParams.srcYFlipped = false;
    baseParams.unpackFlipX = false;
    baseParams.unpackFlipY = false;

    return blitWithDraw(context, srcFrameBuffer, blitColorBuffer, blitDepthBuffer,
                        blitStencilBuffer, filter, baseParams);
}

angle::Result FramebufferMtl::blitWithDraw(const gl::Context *context,
                                           FramebufferMtl *srcFrameBuffer,
                                           bool blitColorBuffer,
                                           bool blitDepthBuffer,
                                           bool blitStencilBuffer,
                                           GLenum filter,
                                           const mtl::BlitParams &baseParams)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    // Use blit with draw
    mtl::RenderCommandEncoder *renderEncoder = nullptr;

    // Blit Depth & stencil
    if (blitDepthBuffer || blitStencilBuffer)
    {
        mtl::DepthStencilBlitParams dsBlitParams;
        memcpy(&dsBlitParams, &baseParams, sizeof(baseParams));
        RenderTargetMtl *srcDepthRt   = srcFrameBuffer->getDepthRenderTarget();
        RenderTargetMtl *srcStencilRt = srcFrameBuffer->getStencilRenderTarget();

        if (blitDepthBuffer)
        {
            dsBlitParams.src      = srcDepthRt->getTexture();
            dsBlitParams.srcLevel = srcDepthRt->getLevelIndex();
            dsBlitParams.srcLayer = srcDepthRt->getLayerIndex();
        }

        if (blitStencilBuffer && srcStencilRt->getTexture())
        {
            dsBlitParams.srcStencil = srcStencilRt->getTexture()->getStencilView();
            dsBlitParams.srcLevel   = srcStencilRt->getLevelIndex();
            dsBlitParams.srcLayer   = srcStencilRt->getLayerIndex();

            if (!contextMtl->getDisplay()->getFeatures().hasShaderStencilOutput.enabled &&
                mStencilRenderTarget)
            {
                // Directly writing to stencil in shader is not supported, use temporary copy buffer
                // work around. This is a compute pass.
                mtl::StencilBlitViaBufferParams stencilOnlyBlitParams = dsBlitParams;
                stencilOnlyBlitParams.dstStencil      = mStencilRenderTarget->getTexture();
                stencilOnlyBlitParams.dstStencilLayer = mStencilRenderTarget->getLayerIndex();
                stencilOnlyBlitParams.dstStencilLevel = mStencilRenderTarget->getLevelIndex();
                stencilOnlyBlitParams.dstPackedDepthStencilFormat =
                    mStencilRenderTarget->getFormat().hasDepthAndStencilBits();

                ANGLE_TRY(contextMtl->getDisplay()->getUtils().blitStencilViaCopyBuffer(
                    context, stencilOnlyBlitParams));

                // Prevent the stencil to be blitted with draw again
                dsBlitParams.srcStencil = nullptr;
            }
        }

        // The actual blitting of depth and/or stencil
        ANGLE_TRY(ensureRenderPassStarted(context, &renderEncoder));
        ANGLE_TRY(contextMtl->getDisplay()->getUtils().blitDepthStencilWithDraw(
            context, renderEncoder, dsBlitParams));
    }  // if (blitDepthBuffer || blitStencilBuffer)
    else
    {
        ANGLE_TRY(ensureRenderPassStarted(context, &renderEncoder));
    }

    // Blit color
    if (blitColorBuffer)
    {
        mtl::ColorBlitParams colorBlitParams;
        memcpy(&colorBlitParams, &baseParams, sizeof(baseParams));

        RenderTargetMtl *srcColorRt = srcFrameBuffer->getColorReadRenderTarget(context);
        ASSERT(srcColorRt);

        colorBlitParams.src      = srcColorRt->getTexture();
        colorBlitParams.srcLevel = srcColorRt->getLevelIndex();
        colorBlitParams.srcLayer = srcColorRt->getLayerIndex();

        colorBlitParams.enabledBuffers = getState().getEnabledDrawBuffers();
        colorBlitParams.filter         = filter;
        colorBlitParams.dstLuminance   = srcColorRt->getFormat().actualAngleFormat().isLUMA();

        ANGLE_TRY(contextMtl->getDisplay()->getUtils().blitColorWithDraw(
            context, renderEncoder, srcColorRt->getFormat().actualAngleFormat(), colorBlitParams));
    }

    return angle::Result::Continue;
}

bool FramebufferMtl::totalBitsUsedIsLessThanOrEqualToMaxBitsSupported(
    const gl::Context *context) const
{
    ContextMtl *contextMtl = mtl::GetImpl(context);

    uint32_t bitsUsed = 0;
    for (const gl::FramebufferAttachment &attachment : mState.getColorAttachments())
    {
        if (attachment.isAttached())
        {
            bitsUsed += attachment.getRedSize() + attachment.getGreenSize() +
                        attachment.getBlueSize() + attachment.getAlphaSize();
        }
    }

    return bitsUsed <= contextMtl->getDisplay()->getMaxColorTargetBits();
}

gl::FramebufferStatus FramebufferMtl::checkStatus(const gl::Context *context) const
{
    if (mState.hasSeparateDepthAndStencilAttachments())
    {
        ContextMtl *contextMtl = mtl::GetImpl(context);
        if (!contextMtl->getDisplay()->getFeatures().allowSeparateDepthStencilBuffers.enabled)
        {
            return gl::FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_UNSUPPORTED,
                gl::err::kFramebufferIncompleteUnsupportedSeparateDepthStencilBuffers);
        }

        ASSERT(mState.getDepthAttachment()->getFormat().info->depthBits > 0);
        ASSERT(mState.getStencilAttachment()->getFormat().info->stencilBits > 0);
        if (mState.getDepthAttachment()->getFormat().info->stencilBits != 0 ||
            mState.getStencilAttachment()->getFormat().info->depthBits != 0)
        {
            return gl::FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_UNSUPPORTED,
                gl::err::
                    kFramebufferIncompleteUnsupportedSeparateDepthStencilBuffersCombinedFormat);
        }
    }

    if (!totalBitsUsedIsLessThanOrEqualToMaxBitsSupported(context))
    {
        return gl::FramebufferStatus::Incomplete(
            GL_FRAMEBUFFER_UNSUPPORTED,
            gl::err::kFramebufferIncompleteColorBitsUsedExceedsMaxColorBitsSupported);
    }

    return gl::FramebufferStatus::Complete();
}

angle::Result FramebufferMtl::syncState(const gl::Context *context,
                                        GLenum binding,
                                        const gl::Framebuffer::DirtyBits &dirtyBits,
                                        gl::Command command)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    bool mustNotifyContext = false;
    // Cache old mRenderPassDesc before update*RenderTarget() invalidate it.
    mtl::RenderPassDesc oldRenderPassDesc = mRenderPassDesc;

    for (size_t dirtyBit : dirtyBits)
    {
        switch (dirtyBit)
        {
            case gl::Framebuffer::DIRTY_BIT_DEPTH_ATTACHMENT:
                ANGLE_TRY(updateDepthRenderTarget(context));
                break;
            case gl::Framebuffer::DIRTY_BIT_STENCIL_ATTACHMENT:
                ANGLE_TRY(updateStencilRenderTarget(context));
                break;
            case gl::Framebuffer::DIRTY_BIT_DEPTH_BUFFER_CONTENTS:
                // Restore depth attachment load action as its content may have been updated
                // after framebuffer invalidation.
                mRenderPassDesc.depthAttachment.loadAction = MTLLoadActionLoad;
                break;
            case gl::Framebuffer::DIRTY_BIT_STENCIL_BUFFER_CONTENTS:
                // Restore stencil attachment load action as its content may have been updated
                // after framebuffer invalidation.
                mRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
                break;
            case gl::Framebuffer::DIRTY_BIT_DRAW_BUFFERS:
                mustNotifyContext = true;
                break;
            case gl::Framebuffer::DIRTY_BIT_READ_BUFFER:
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_WIDTH:
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_HEIGHT:
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_SAMPLES:
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_FIXED_SAMPLE_LOCATIONS:
                break;
            default:
            {
                static_assert(gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_0 == 0, "FB dirty bits");
                if (dirtyBit < gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_MAX)
                {
                    size_t colorIndexGL = static_cast<size_t>(
                        dirtyBit - gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_0);
                    ANGLE_TRY(updateColorRenderTarget(context, colorIndexGL));
                }
                else
                {
                    ASSERT(dirtyBit >= gl::Framebuffer::DIRTY_BIT_COLOR_BUFFER_CONTENTS_0 &&
                           dirtyBit < gl::Framebuffer::DIRTY_BIT_COLOR_BUFFER_CONTENTS_MAX);
                    // NOTE: might need to notify context.

                    // Restore color attachment load action as its content may have been updated
                    // after framebuffer invalidation.
                    size_t colorIndexGL = static_cast<size_t>(
                        dirtyBit - gl::Framebuffer::DIRTY_BIT_COLOR_BUFFER_CONTENTS_0);
                    mRenderPassDesc.colorAttachments[colorIndexGL].loadAction = MTLLoadActionLoad;
                }
                break;
            }
        }
    }

    // If attachments have been changed and this is the current draw framebuffer,
    // update the Metal context's incompatible attachments cache before preparing a render pass.
    static_assert(gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_0 == 0, "FB dirty bits");
    constexpr gl::Framebuffer::DirtyBits kAttachmentsMask =
        gl::Framebuffer::DirtyBits::Mask(gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_MAX);
    if (mustNotifyContext || (dirtyBits & kAttachmentsMask).any())
    {
        const gl::State &glState = context->getState();
        if (mtl::GetImpl(glState.getDrawFramebuffer()) == this)
        {
            contextMtl->updateIncompatibleAttachments(glState);
        }
    }

    ANGLE_TRY(prepareRenderPass(context, &mRenderPassDesc, command));
    bool renderPassChanged = !oldRenderPassDesc.equalIgnoreLoadStoreOptions(mRenderPassDesc);

    if (mustNotifyContext || renderPassChanged)
    {
        FramebufferMtl *currentDrawFramebuffer =
            mtl::GetImpl(context->getState().getDrawFramebuffer());
        if (currentDrawFramebuffer == this)
        {
            contextMtl->onDrawFrameBufferChangedState(context, this, renderPassChanged);
        }

        // Recreate pixel reading buffer if needed in future.
        mReadPixelBuffer = nullptr;
    }

    return angle::Result::Continue;
}

angle::Result FramebufferMtl::getSamplePosition(const gl::Context *context,
                                                size_t index,
                                                GLfloat *xy) const
{
    rx::GetSamplePosition(getSamples(), index, xy);
    return angle::Result::Continue;
}

angle::Result FramebufferMtl::prepareForUse(const gl::Context *context) const
{
    if (mBackbuffer)
    {
        // Backbuffer might obtain new drawable, which means it might change the
        // the native texture used as the target of the render pass.
        // We need to call this before creating render encoder.
        ANGLE_TRY(mBackbuffer->ensureCurrentDrawableObtained(context));

        if (mBackbuffer->hasRobustResourceInit())
        {
            ANGLE_TRY(mBackbuffer->initializeContents(context, GL_BACK, gl::ImageIndex::Make2D(0)));
            if (mBackbuffer->hasDepthStencil())
            {
                ANGLE_TRY(
                    mBackbuffer->initializeContents(context, GL_DEPTH, gl::ImageIndex::Make2D(0)));
            }
        }
    }
    return angle::Result::Continue;
}

RenderTargetMtl *FramebufferMtl::getColorReadRenderTarget(const gl::Context *context) const
{
    if (mState.getReadIndex() >= mColorRenderTargets.size())
    {
        return nullptr;
    }

    if (IsError(prepareForUse(context)))
    {
        return nullptr;
    }

    return mColorRenderTargets[mState.getReadIndex()];
}

RenderTargetMtl *FramebufferMtl::getColorReadRenderTargetNoCache(const gl::Context *context) const
{
    if (mState.getReadIndex() >= mColorRenderTargets.size())
    {
        return nullptr;
    }

    if (mBackbuffer)
    {
        // If we have a backbuffer/window surface, we can take the old path here and return
        // the cached color render target.
        return getColorReadRenderTarget(context);
    }
    // If we have no backbuffer, get the attachment from state color attachments, as it may have
    // changed before syncing.
    const gl::FramebufferAttachment *attachment = mState.getColorAttachment(mState.getReadIndex());
    RenderTargetMtl *currentTarget              = nullptr;
    if (attachment->getRenderTarget(context, attachment->getRenderToTextureSamples(),
                                    &currentTarget) == angle::Result::Stop)
    {
        return nullptr;
    }
    return currentTarget;
}

int FramebufferMtl::getSamples() const
{
    return mRenderPassDesc.rasterSampleCount;
}

gl::Rectangle FramebufferMtl::getCompleteRenderArea() const
{
    return gl::Rectangle(0, 0, mState.getDimensions().width, mState.getDimensions().height);
}

bool FramebufferMtl::renderPassHasStarted(ContextMtl *contextMtl) const
{
    return contextMtl->hasStartedRenderPass(mRenderPassDesc);
}

angle::Result FramebufferMtl::ensureRenderPassStarted(const gl::Context *context,
                                                      mtl::RenderCommandEncoder **encoderOut)
{
    return ensureRenderPassStarted(context, mRenderPassDesc, encoderOut);
}

angle::Result FramebufferMtl::ensureRenderPassStarted(const gl::Context *context,
                                                      const mtl::RenderPassDesc &desc,
                                                      mtl::RenderCommandEncoder **encoderOut)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);

    mtl::RenderCommandEncoder *encoder = contextMtl->getRenderCommandEncoder();
    if (encoder && encoder->getSerial() == mStartedRenderEncoderSerial)
    {
        // Already started.
        *encoderOut = encoder;
        return angle::Result::Continue;
    }

    ANGLE_TRY(prepareForUse(context));

    // Only support ensureRenderPassStarted() with different load & store options only. The
    // texture, level, slice must be the same.
    ASSERT(desc.equalIgnoreLoadStoreOptions(mRenderPassDesc));

    encoder                     = contextMtl->getRenderPassCommandEncoder(desc);
    mStartedRenderEncoderSerial = encoder->getSerial();

    ANGLE_TRY(unresolveIfNeeded(context, encoder));

    if (mRenderPassCleanStart)
    {
        // After a clean start we should reset the loadOp to MTLLoadActionLoad in case this render
        // pass could be interrupted by a conversion compute shader pass then being resumed later.
        mRenderPassCleanStart = false;
        for (mtl::RenderPassColorAttachmentDesc &colorAttachment : mRenderPassDesc.colorAttachments)
        {
            colorAttachment.loadAction = MTLLoadActionLoad;
        }
        mRenderPassDesc.depthAttachment.loadAction   = MTLLoadActionLoad;
        mRenderPassDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
    }

    *encoderOut = encoder;

    return angle::Result::Continue;
}

void FramebufferMtl::setLoadStoreActionOnRenderPassFirstStart(
    mtl::RenderPassAttachmentDesc *attachmentOut,
    const bool forceDepthStencilMultisampleLoad)
{
    ASSERT(mRenderPassCleanStart);

    mtl::RenderPassAttachmentDesc &attachment = *attachmentOut;

    if (!forceDepthStencilMultisampleLoad && attachment.storeAction == MTLStoreActionDontCare)
    {
        // If we previously discarded attachment's content, then don't need to load it.
        attachment.loadAction = MTLLoadActionDontCare;
    }
    else
    {
        attachment.loadAction = MTLLoadActionLoad;
    }

    if (attachment.hasImplicitMSTexture())
    {
        attachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
    }
    else
    {
        attachment.storeAction = MTLStoreActionStore;  // Default action is store
    }
}

void FramebufferMtl::onStartedDrawingToFrameBuffer(const gl::Context *context)
{
    mRenderPassCleanStart = true;

    // If any of the render targets need to load their multisample textures, we should do the same
    // for depth/stencil.
    bool forceDepthStencilMultisampleLoad = false;

    // Compute loadOp based on previous storeOp and reset storeOp flags:
    for (mtl::RenderPassColorAttachmentDesc &colorAttachment : mRenderPassDesc.colorAttachments)
    {
        forceDepthStencilMultisampleLoad |=
            colorAttachment.storeAction == MTLStoreActionStoreAndMultisampleResolve;
        setLoadStoreActionOnRenderPassFirstStart(&colorAttachment, false);
    }
    // Depth load/store
    setLoadStoreActionOnRenderPassFirstStart(&mRenderPassDesc.depthAttachment,
                                             forceDepthStencilMultisampleLoad);

    // Stencil load/store
    setLoadStoreActionOnRenderPassFirstStart(&mRenderPassDesc.stencilAttachment,
                                             forceDepthStencilMultisampleLoad);
}

void FramebufferMtl::onFrameEnd(const gl::Context *context)
{
    if (!mBackbuffer || mBackbuffer->preserveBuffer())
    {
        return;
    }

    ContextMtl *contextMtl = mtl::GetImpl(context);
    // Always discard default FBO's depth stencil & multisample buffers at the end of the frame:
    if (this->renderPassHasStarted(contextMtl))
    {
        mtl::RenderCommandEncoder *encoder = contextMtl->getRenderCommandEncoder();

        constexpr GLenum dsAttachments[] = {GL_DEPTH, GL_STENCIL};
        (void)invalidateImpl(context, 2, dsAttachments);
        if (mBackbuffer->getSamples() > 1)
        {
            encoder->setColorStoreAction(MTLStoreActionMultisampleResolve, 0);
        }

        contextMtl->endEncoding(false);

        // Reset discard flag.
        onStartedDrawingToFrameBuffer(context);
    }
}

angle::Result FramebufferMtl::updateColorRenderTarget(const gl::Context *context,
                                                      size_t colorIndexGL)
{
    ASSERT(colorIndexGL < mColorRenderTargets.size());
    // Reset load store action
    mRenderPassDesc.colorAttachments[colorIndexGL].reset();
    return updateCachedRenderTarget(context, mState.getColorAttachment(colorIndexGL),
                                    &mColorRenderTargets[colorIndexGL]);
}

angle::Result FramebufferMtl::updateDepthRenderTarget(const gl::Context *context)
{
    // Reset load store action
    mRenderPassDesc.depthAttachment.reset();
    return updateCachedRenderTarget(context, mState.getDepthAttachment(), &mDepthRenderTarget);
}

angle::Result FramebufferMtl::updateStencilRenderTarget(const gl::Context *context)
{
    // Reset load store action
    mRenderPassDesc.stencilAttachment.reset();
    return updateCachedRenderTarget(context, mState.getStencilAttachment(), &mStencilRenderTarget);
}

angle::Result FramebufferMtl::updateCachedRenderTarget(const gl::Context *context,
                                                       const gl::FramebufferAttachment *attachment,
                                                       RenderTargetMtl **cachedRenderTarget)
{
    RenderTargetMtl *newRenderTarget = nullptr;
    if (attachment)
    {
        ASSERT(attachment->isAttached());
        ANGLE_TRY(attachment->getRenderTarget(context, attachment->getRenderToTextureSamples(),
                                              &newRenderTarget));
    }
    *cachedRenderTarget = newRenderTarget;
    return angle::Result::Continue;
}

angle::Result FramebufferMtl::prepareRenderPass(const gl::Context *context,
                                                mtl::RenderPassDesc *pDescOut,
                                                gl::Command command)
{
    // Skip incompatible attachments for draw ops to avoid triggering Metal runtime failures.
    const gl::DrawBufferMask incompatibleAttachments =
        (command == gl::Command::Draw) ? mtl::GetImpl(context)->getIncompatibleAttachments()
                                       : gl::DrawBufferMask();
    const gl::DrawBufferMask enabledDrawBuffers =
        getState().getEnabledDrawBuffers() & ~incompatibleAttachments;

    mtl::RenderPassDesc &desc = *pDescOut;

    mRenderPassFirstColorAttachmentFormat = nullptr;
    mRenderPassAttachmentsSameColorType   = true;
    uint32_t maxColorAttachments = static_cast<uint32_t>(mState.getColorAttachments().size());
    desc.numColorAttachments     = 0;
    desc.rasterSampleCount       = 1;
    for (uint32_t colorIndexGL = 0; colorIndexGL < maxColorAttachments; ++colorIndexGL)
    {
        ASSERT(colorIndexGL < mColorRenderTargets.size());

        mtl::RenderPassColorAttachmentDesc &colorAttachment = desc.colorAttachments[colorIndexGL];
        const RenderTargetMtl *colorRenderTarget            = mColorRenderTargets[colorIndexGL];

        // GL allows data types of fragment shader color outputs to be incompatible with disabled
        // color attachments. To prevent various Metal validation issues, assign textures only to
        // enabled attachments.
        if (colorRenderTarget && enabledDrawBuffers.test(colorIndexGL))
        {
            colorRenderTarget->toRenderPassAttachmentDesc(&colorAttachment);

            desc.numColorAttachments = std::max(desc.numColorAttachments, colorIndexGL + 1);
            desc.rasterSampleCount =
                std::max(desc.rasterSampleCount, colorRenderTarget->getRenderSamples());

            if (!mRenderPassFirstColorAttachmentFormat)
            {
                mRenderPassFirstColorAttachmentFormat = &colorRenderTarget->getFormat();
            }
            else
            {
                if (mRenderPassFirstColorAttachmentFormat->actualAngleFormat().isSint() !=
                        colorRenderTarget->getFormat().actualAngleFormat().isSint() ||
                    mRenderPassFirstColorAttachmentFormat->actualAngleFormat().isUint() !=
                        colorRenderTarget->getFormat().actualAngleFormat().isUint())
                {
                    mRenderPassAttachmentsSameColorType = false;
                }
            }
        }
        else
        {
            colorAttachment.reset();
        }
    }

    if (mDepthRenderTarget)
    {
        mDepthRenderTarget->toRenderPassAttachmentDesc(&desc.depthAttachment);
        desc.rasterSampleCount =
            std::max(desc.rasterSampleCount, mDepthRenderTarget->getRenderSamples());
    }
    else
    {
        desc.depthAttachment.reset();
    }

    if (mStencilRenderTarget)
    {
        mStencilRenderTarget->toRenderPassAttachmentDesc(&desc.stencilAttachment);
        desc.rasterSampleCount =
            std::max(desc.rasterSampleCount, mStencilRenderTarget->getRenderSamples());
    }
    else
    {
        desc.stencilAttachment.reset();
    }

    if (desc.numColorAttachments == 0 && mDepthRenderTarget == nullptr &&
        mStencilRenderTarget == nullptr)
    {
        desc.defaultWidth  = mState.getDefaultWidth();
        desc.defaultHeight = mState.getDefaultHeight();
    }

    return angle::Result::Continue;
}

angle::Result FramebufferMtl::clearWithLoadOp(const gl::Context *context,
                                              gl::DrawBufferMask clearColorBuffers,
                                              const mtl::ClearRectParams &clearOpts)
{
    ContextMtl *contextMtl             = mtl::GetImpl(context);
    bool startedRenderPass             = contextMtl->hasStartedRenderPass(mRenderPassDesc);
    mtl::RenderCommandEncoder *encoder = nullptr;

    if (startedRenderPass)
    {
        ANGLE_TRY(ensureRenderPassStarted(context, &encoder));
        if (encoder->hasDrawCalls())
        {
            // Render pass already has draw calls recorded, it is better to use clear with draw
            // operation.
            return clearWithDraw(context, clearColorBuffers, clearOpts);
        }
        else
        {
            // If render pass has started but there is no draw call yet. It is OK to change the
            // loadOp.
            return clearWithLoadOpRenderPassStarted(context, clearColorBuffers, clearOpts, encoder);
        }
    }
    else
    {
        return clearWithLoadOpRenderPassNotStarted(context, clearColorBuffers, clearOpts);
    }
}

angle::Result FramebufferMtl::clearWithLoadOpRenderPassNotStarted(
    const gl::Context *context,
    gl::DrawBufferMask clearColorBuffers,
    const mtl::ClearRectParams &clearOpts)
{
    mtl::RenderPassDesc tempDesc = mRenderPassDesc;

    for (uint32_t colorIndexGL = 0; colorIndexGL < tempDesc.numColorAttachments; ++colorIndexGL)
    {
        ASSERT(colorIndexGL < tempDesc.colorAttachments.size());

        mtl::RenderPassColorAttachmentDesc &colorAttachment =
            tempDesc.colorAttachments[colorIndexGL];
        const mtl::TextureRef &texture = colorAttachment.texture;

        if (clearColorBuffers.test(colorIndexGL))
        {
            colorAttachment.loadAction = MTLLoadActionClear;
            OverrideMTLClearColor(texture, clearOpts.clearColor.value(),
                                  &colorAttachment.clearColor);
        }
        else
        {
            colorAttachment.loadAction = MTLLoadActionLoad;
        }

        if (colorAttachment.hasImplicitMSTexture())
        {
            colorAttachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
        }
        else
        {
            colorAttachment.storeAction = MTLStoreActionStore;
        }
    }

    if (clearOpts.clearDepth.valid())
    {
        tempDesc.depthAttachment.loadAction = MTLLoadActionClear;
        tempDesc.depthAttachment.clearDepth = clearOpts.clearDepth.value();
    }
    else
    {
        tempDesc.depthAttachment.loadAction = MTLLoadActionLoad;
    }

    if (tempDesc.depthAttachment.hasImplicitMSTexture())
    {
        tempDesc.depthAttachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
    }
    else
    {
        tempDesc.depthAttachment.storeAction = MTLStoreActionStore;
    }

    if (clearOpts.clearStencil.valid())
    {
        tempDesc.stencilAttachment.loadAction   = MTLLoadActionClear;
        tempDesc.stencilAttachment.clearStencil = clearOpts.clearStencil.value();
    }
    else
    {
        tempDesc.stencilAttachment.loadAction = MTLLoadActionLoad;
    }

    if (tempDesc.stencilAttachment.hasImplicitMSTexture())
    {
        tempDesc.stencilAttachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
    }
    else
    {
        tempDesc.stencilAttachment.storeAction = MTLStoreActionStore;
    }

    // Start new render encoder with loadOp=Clear
    mtl::RenderCommandEncoder *encoder;
    return ensureRenderPassStarted(context, tempDesc, &encoder);
}

angle::Result FramebufferMtl::clearWithLoadOpRenderPassStarted(
    const gl::Context *context,
    gl::DrawBufferMask clearColorBuffers,
    const mtl::ClearRectParams &clearOpts,
    mtl::RenderCommandEncoder *encoder)
{
    ASSERT(!encoder->hasDrawCalls());

    for (uint32_t colorIndexGL = 0; colorIndexGL < mRenderPassDesc.numColorAttachments;
         ++colorIndexGL)
    {
        ASSERT(colorIndexGL < mRenderPassDesc.colorAttachments.size());

        mtl::RenderPassColorAttachmentDesc &colorAttachment =
            mRenderPassDesc.colorAttachments[colorIndexGL];
        const mtl::TextureRef &texture = colorAttachment.texture;

        if (clearColorBuffers.test(colorIndexGL))
        {
            MTLClearColor clearVal;
            OverrideMTLClearColor(texture, clearOpts.clearColor.value(), &clearVal);

            encoder->setColorLoadAction(MTLLoadActionClear, clearVal, colorIndexGL);
        }
    }

    if (clearOpts.clearDepth.valid())
    {
        encoder->setDepthLoadAction(MTLLoadActionClear, clearOpts.clearDepth.value());
    }

    if (clearOpts.clearStencil.valid())
    {
        encoder->setStencilLoadAction(MTLLoadActionClear, clearOpts.clearStencil.value());
    }

    return angle::Result::Continue;
}

angle::Result FramebufferMtl::clearWithDraw(const gl::Context *context,
                                            gl::DrawBufferMask clearColorBuffers,
                                            const mtl::ClearRectParams &clearOpts)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    DisplayMtl *display    = contextMtl->getDisplay();

    if (mRenderPassAttachmentsSameColorType)
    {
        // Start new render encoder if not already.
        mtl::RenderCommandEncoder *encoder;
        ANGLE_TRY(ensureRenderPassStarted(context, mRenderPassDesc, &encoder));

        return display->getUtils().clearWithDraw(context, encoder, clearOpts);
    }

    // Not all attachments have the same color type.
    mtl::ClearRectParams overrideClearOps = clearOpts;
    overrideClearOps.enabledBuffers.reset();

    // First clear depth/stencil without color attachment
    if (clearOpts.clearDepth.valid() || clearOpts.clearStencil.valid())
    {
        mtl::RenderPassDesc dsOnlyDesc     = mRenderPassDesc;
        dsOnlyDesc.numColorAttachments     = 0;
        mtl::RenderCommandEncoder *encoder = contextMtl->getRenderPassCommandEncoder(dsOnlyDesc);

        ANGLE_TRY(display->getUtils().clearWithDraw(context, encoder, overrideClearOps));
    }

    // Clear the color attachment one by one.
    overrideClearOps.enabledBuffers.set(0);
    for (size_t drawbuffer : clearColorBuffers)
    {
        if (drawbuffer >= mRenderPassDesc.numColorAttachments)
        {
            // Iteration over drawbuffer indices always goes in ascending order
            break;
        }
        RenderTargetMtl *renderTarget = mColorRenderTargets[drawbuffer];
        if (!renderTarget || !renderTarget->getTexture())
        {
            continue;
        }
        const mtl::Format &format     = renderTarget->getFormat();
        mtl::PixelType clearColorType = overrideClearOps.clearColor.value().getType();
        if ((clearColorType == mtl::PixelType::Int && !format.actualAngleFormat().isSint()) ||
            (clearColorType == mtl::PixelType::UInt && !format.actualAngleFormat().isUint()) ||
            (clearColorType == mtl::PixelType::Float && format.actualAngleFormat().isInt()))
        {
            continue;
        }

        overrideClearOps.clearWriteMaskArray[0] = overrideClearOps.clearWriteMaskArray[drawbuffer];

        mtl::RenderCommandEncoder *encoder =
            contextMtl->getRenderTargetCommandEncoder(*renderTarget);
        ANGLE_TRY(display->getUtils().clearWithDraw(context, encoder, overrideClearOps));
    }

    return angle::Result::Continue;
}

angle::Result FramebufferMtl::clearImpl(const gl::Context *context,
                                        gl::DrawBufferMask clearColorBuffers,
                                        mtl::ClearRectParams *pClearOpts)
{
    auto &clearOpts = *pClearOpts;

    if (!clearOpts.clearColor.valid() && !clearOpts.clearDepth.valid() &&
        !clearOpts.clearStencil.valid())
    {
        // No Op.
        return angle::Result::Continue;
    }

    ContextMtl *contextMtl = mtl::GetImpl(context);
    const gl::Rectangle renderArea(0, 0, mState.getDimensions().width,
                                   mState.getDimensions().height);

    clearOpts.colorFormat    = mRenderPassFirstColorAttachmentFormat;
    clearOpts.dstTextureSize = mState.getExtents();
    clearOpts.clearArea      = ClipRectToScissor(contextMtl->getState(), renderArea, false);
    clearOpts.flipY          = mFlipY;

    // Discard clear altogether if scissor has 0 width or height.
    if (clearOpts.clearArea.width == 0 || clearOpts.clearArea.height == 0)
    {
        return angle::Result::Continue;
    }

    clearOpts.clearWriteMaskArray = contextMtl->getWriteMaskArray();
    uint32_t stencilMask          = contextMtl->getStencilMask();
    if (!contextMtl->getDepthMask())
    {
        // Disable depth clearing, since depth write is disable
        clearOpts.clearDepth.reset();
    }

    // Only clear enabled buffers
    clearOpts.enabledBuffers = clearColorBuffers;

    bool allBuffersUnmasked = true;
    for (size_t enabledBuffer : clearColorBuffers)
    {
        if (clearOpts.clearWriteMaskArray[enabledBuffer] != MTLColorWriteMaskAll)
        {
            allBuffersUnmasked = false;
            break;
        }
    }

    if (clearOpts.clearArea == renderArea &&
        (!clearOpts.clearColor.valid() || allBuffersUnmasked) &&
        (!clearOpts.clearStencil.valid() ||
         (stencilMask & mtl::kStencilMaskAll) == mtl::kStencilMaskAll))
    {
        return clearWithLoadOp(context, clearColorBuffers, clearOpts);
    }

    return clearWithDraw(context, clearColorBuffers, clearOpts);
}

angle::Result FramebufferMtl::invalidateImpl(const gl::Context *context,
                                             size_t count,
                                             const GLenum *attachments)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    gl::DrawBufferMask invalidateColorBuffers;
    bool invalidateDepthBuffer   = false;
    bool invalidateStencilBuffer = false;

    for (size_t i = 0; i < count; ++i)
    {
        const GLenum attachment = attachments[i];

        switch (attachment)
        {
            case GL_DEPTH:
            case GL_DEPTH_ATTACHMENT:
                invalidateDepthBuffer = true;
                break;
            case GL_STENCIL:
            case GL_STENCIL_ATTACHMENT:
                invalidateStencilBuffer = true;
                break;
            case GL_DEPTH_STENCIL_ATTACHMENT:
                invalidateDepthBuffer   = true;
                invalidateStencilBuffer = true;
                break;
            default:
                ASSERT(
                    (attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT15) ||
                    (attachment == GL_COLOR));

                invalidateColorBuffers.set(
                    attachment == GL_COLOR ? 0u : (attachment - GL_COLOR_ATTACHMENT0));
        }
    }

    // Set the appropriate storeOp for attachments.
    // If we already start the render pass, then need to set the store action now.
    bool renderPassStarted = contextMtl->hasStartedRenderPass(mRenderPassDesc);
    mtl::RenderCommandEncoder *encoder =
        renderPassStarted ? contextMtl->getRenderCommandEncoder() : nullptr;

    for (uint32_t i = 0; i < mRenderPassDesc.numColorAttachments; ++i)
    {
        if (invalidateColorBuffers.test(i))
        {
            // Some opaque formats, like RGB8, are emulated as RGBA with alpha channel initialized
            // to 1.0. Invalidating such attachments may lead to random values in their alpha
            // channel, so skip invalidation in this case.
            RenderTargetMtl *renderTarget = mColorRenderTargets[i];
            if (renderTarget && renderTarget->getTexture())
            {
                const mtl::Format &mtlFormat        = renderTarget->getFormat();
                const angle::Format &intendedFormat = mtlFormat.intendedAngleFormat();
                const angle::Format &actualFormat   = mtlFormat.actualAngleFormat();
                if (intendedFormat.alphaBits == 0 && actualFormat.alphaBits)
                {
                    continue;
                }
            }

            mtl::RenderPassColorAttachmentDesc &colorAttachment =
                mRenderPassDesc.colorAttachments[i];
            colorAttachment.storeAction = MTLStoreActionDontCare;
            if (renderPassStarted)
            {
                encoder->setColorStoreAction(MTLStoreActionDontCare, i);
            }
        }
    }

    if (invalidateDepthBuffer && mDepthRenderTarget)
    {
        mRenderPassDesc.depthAttachment.storeAction = MTLStoreActionDontCare;
        if (renderPassStarted)
        {
            encoder->setDepthStoreAction(MTLStoreActionDontCare);
        }
    }

    if (invalidateStencilBuffer && mStencilRenderTarget)
    {
        mRenderPassDesc.stencilAttachment.storeAction = MTLStoreActionDontCare;
        if (renderPassStarted)
        {
            encoder->setStencilStoreAction(MTLStoreActionDontCare);
        }
    }

    // Do not encode any further commands in this render pass which can affect the
    // framebuffer, or their effects will be lost.
    contextMtl->endEncoding(false);
    // Reset discard flag.
    onStartedDrawingToFrameBuffer(context);

    return angle::Result::Continue;
}

gl::Rectangle FramebufferMtl::getCorrectFlippedReadArea(const gl::Context *context,
                                                        const gl::Rectangle &glArea) const
{
    RenderTargetMtl *readRT = getColorReadRenderTarget(context);
    if (!readRT)
    {
        readRT = mDepthRenderTarget;
    }
    if (!readRT)
    {
        readRT = mStencilRenderTarget;
    }
    ASSERT(readRT);
    gl::Rectangle flippedArea = glArea;
    if (mFlipY)
    {
        flippedArea.y = readRT->getTexture()->height(readRT->getLevelIndex()) - flippedArea.y -
                        flippedArea.height;
    }

    return flippedArea;
}

namespace
{

angle::Result readPixelsCopyImpl(
    const gl::Context *context,
    const gl::Rectangle &area,
    const PackPixelsParams &packPixelsParams,
    const RenderTargetMtl *renderTarget,
    const std::function<angle::Result(const gl::Rectangle &region, const uint8_t *&src)> &getDataFn,
    uint8_t *pixels)
{
    const mtl::Format &readFormat        = renderTarget->getFormat();
    const angle::Format &readAngleFormat = readFormat.actualAngleFormat();

    auto packPixelsRowParams = packPixelsParams;
    gl::Rectangle srcRowRegion(area.x, area.y, area.width, 1);
    int bufferRowPitch = area.width * readAngleFormat.pixelBytes;

    int rowOffset = packPixelsParams.reverseRowOrder ? -1 : 1;
    int startRow  = packPixelsParams.reverseRowOrder ? (area.y1() - 1) : area.y;

    // Copy pixels row by row
    packPixelsRowParams.area.height     = 1;
    packPixelsRowParams.reverseRowOrder = false;
    for (int r = startRow, i = 0; i < area.height;
         ++i, r += rowOffset, pixels += packPixelsRowParams.outputPitch)
    {
        srcRowRegion.y             = r;
        packPixelsRowParams.area.y = packPixelsParams.area.y + i;

        const uint8_t *src;
        ANGLE_TRY(getDataFn(srcRowRegion, src));
        PackPixels(packPixelsRowParams, readAngleFormat, bufferRowPitch, src, pixels);
    }

    return angle::Result::Continue;
}

}  // namespace

angle::Result FramebufferMtl::readPixelsImpl(const gl::Context *context,
                                             const gl::Rectangle &area,
                                             const PackPixelsParams &packPixelsParams,
                                             const RenderTargetMtl *renderTarget,
                                             uint8_t *pixels) const
{
    ContextMtl *contextMtl             = mtl::GetImpl(context);
    const angle::FeaturesMtl &features = contextMtl->getDisplay()->getFeatures();

    if (!renderTarget)
    {
        return angle::Result::Continue;
    }

    if (packPixelsParams.packBuffer)
    {
        return readPixelsToPBO(context, area, packPixelsParams, renderTarget);
    }

    mtl::TextureRef texture;
    if (mBackbuffer)
    {
        // Backbuffer might have MSAA texture as render target, needs to obtain the
        // resolved texture to be able to read pixels.
        ANGLE_TRY(mBackbuffer->ensureColorTextureReadyForReadPixels(context));
        texture = mBackbuffer->getColorTexture();
    }
    else
    {
        texture = renderTarget->getTexture();
        // For non-default framebuffer, MSAA read pixels is disallowed.
        if (!texture)
        {
            return angle::Result::Stop;
        }
        ANGLE_CHECK(contextMtl, texture->samples() == 1, gl::err::kInternalError,
                    GL_INVALID_OPERATION);
    }

    const mtl::Format &readFormat        = renderTarget->getFormat();
    const angle::Format &readAngleFormat = readFormat.actualAngleFormat();

    if (features.copyIOSurfaceToNonIOSurfaceForReadOptimization.enabled &&
        texture->hasIOSurface() && texture->mipmapLevels() == 1 &&
        texture->textureType() == MTLTextureType2D)
    {
        // Reading a texture may be slow if it's an IOSurface because metal has to lock/unlock the
        // surface, whereas copying the texture to non IOSurface texture and then reading from that
        // may be fast depending on the GPU/driver.
        ANGLE_TRY(Copy2DTextureSlice0Level0ToTempTexture(context, texture, &texture));
    }

    if (features.copyTextureToBufferForReadOptimization.enabled)
    {
        mtl::BufferRef buffer;
        ANGLE_TRY(CopyTextureSliceLevelToTempBuffer(context, texture, renderTarget->getLevelIndex(),
                                                    renderTarget->getLayerIndex(), &buffer));

        int bufferRowPitch =
            texture->width(renderTarget->getLevelIndex()) * readAngleFormat.pixelBytes;

        buffer->syncContent(contextMtl, contextMtl->getBlitCommandEncoder());
        const uint8_t *bufferData = buffer->mapReadOnly(contextMtl);

        angle::Result result = readPixelsCopyImpl(
            context, area, packPixelsParams, renderTarget,
            [&](const gl::Rectangle &region, const uint8_t *&src) {
                src =
                    bufferData + region.y * bufferRowPitch + region.x * readAngleFormat.pixelBytes;
                return angle::Result::Continue;
            },
            pixels);

        buffer->unmap(contextMtl);

        return result;
    }

    if (texture->isBeingUsedByGPU(contextMtl))
    {
        contextMtl->flushCommandBuffer(mtl::WaitUntilFinished);
    }

    angle::MemoryBuffer readPixelRowBuffer;
    int bufferRowPitch = area.width * readAngleFormat.pixelBytes;
    ANGLE_CHECK_GL_ALLOC(contextMtl, readPixelRowBuffer.resize(bufferRowPitch));
    return readPixelsCopyImpl(
        context, area, packPixelsParams, renderTarget,
        [&](const gl::Rectangle &region, const uint8_t *&src) {
            // Read the pixels data to the row buffer
            ANGLE_TRY(mtl::ReadTexturePerSliceBytes(
                context, texture, bufferRowPitch, region, renderTarget->getLevelIndex(),
                renderTarget->getLayerIndex(), readPixelRowBuffer.data()));
            src = readPixelRowBuffer.data();
            return angle::Result::Continue;
        },
        pixels);
}

angle::Result FramebufferMtl::readPixelsToPBO(const gl::Context *context,
                                              const gl::Rectangle &area,
                                              const PackPixelsParams &packPixelsParams,
                                              const RenderTargetMtl *renderTarget) const
{
    ASSERT(packPixelsParams.packBuffer);
    ASSERT(renderTarget);

    ContextMtl *contextMtl = mtl::GetImpl(context);

    ANGLE_CHECK_GL_MATH(contextMtl,
                        packPixelsParams.offset <= std::numeric_limits<uint32_t>::max());
    uint32_t offset = static_cast<uint32_t>(packPixelsParams.offset);

    BufferMtl *packBufferMtl = mtl::GetImpl(packPixelsParams.packBuffer);
    mtl::BufferRef dstBuffer = packBufferMtl->getCurrentBuffer();

    return readPixelsToBuffer(context, area, renderTarget, packPixelsParams.reverseRowOrder,
                              *packPixelsParams.destFormat, offset, packPixelsParams.outputPitch,
                              &dstBuffer);
}

angle::Result FramebufferMtl::readPixelsToBuffer(const gl::Context *context,
                                                 const gl::Rectangle &area,
                                                 const RenderTargetMtl *renderTarget,
                                                 bool reverseRowOrder,
                                                 const angle::Format &dstAngleFormat,
                                                 uint32_t dstBufferOffset,
                                                 uint32_t dstBufferRowPitch,
                                                 const mtl::BufferRef *pDstBuffer) const
{
    ASSERT(renderTarget);

    ContextMtl *contextMtl = mtl::GetImpl(context);

    const mtl::Format &readFormat        = renderTarget->getFormat();
    const angle::Format &readAngleFormat = readFormat.actualAngleFormat();

    mtl::TextureRef texture = renderTarget->getTexture();

    const mtl::BufferRef &dstBuffer = *pDstBuffer;

    if (dstAngleFormat.id != readAngleFormat.id || texture->samples() > 1 ||
        (dstBufferOffset % dstAngleFormat.pixelBytes) ||
        (dstBufferOffset % mtl::kTextureToBufferBlittingAlignment) ||
        (dstBufferRowPitch < area.width * dstAngleFormat.pixelBytes))
    {
        const angle::Format *actualDstAngleFormat;

        // SRGB is special case: We need to write sRGB values to buffer, not linear values.
        switch (readAngleFormat.id)
        {
            case angle::FormatID::B8G8R8A8_UNORM_SRGB:
            case angle::FormatID::R8G8B8_UNORM_SRGB:
            case angle::FormatID::R8G8B8A8_UNORM_SRGB:
                if (dstAngleFormat.id != readAngleFormat.id)
                {
                    switch (dstAngleFormat.id)
                    {
                        case angle::FormatID::B8G8R8A8_UNORM:
                            actualDstAngleFormat =
                                &angle::Format::Get(angle::FormatID::B8G8R8A8_UNORM_SRGB);
                            break;
                        case angle::FormatID::R8G8B8A8_UNORM:
                            actualDstAngleFormat =
                                &angle::Format::Get(angle::FormatID::R8G8B8A8_UNORM_SRGB);
                            break;
                        default:
                            // Unsupported format.
                            ANGLE_GL_UNREACHABLE(contextMtl);
                    }
                    break;
                }
                OS_FALLTHROUGH;
            default:
                actualDstAngleFormat = &dstAngleFormat;
        }

        // Use compute shader
        mtl::CopyPixelsToBufferParams params;
        params.buffer            = dstBuffer;
        params.bufferStartOffset = dstBufferOffset;
        params.bufferRowPitch    = dstBufferRowPitch;

        params.texture                = texture;
        params.textureArea            = area;
        params.textureLevel           = renderTarget->getLevelIndex();
        params.textureSliceOrDeph     = renderTarget->getLayerIndex();
        params.reverseTextureRowOrder = reverseRowOrder;

        ANGLE_TRY(contextMtl->getDisplay()->getUtils().packPixelsFromTextureToBuffer(
            contextMtl, *actualDstAngleFormat, params));
    }
    else
    {
        // Use blit command encoder
        if (!reverseRowOrder)
        {
            ANGLE_TRY(mtl::ReadTexturePerSliceBytesToBuffer(
                context, texture, dstBufferRowPitch, area, renderTarget->getLevelIndex(),
                renderTarget->getLayerIndex(), dstBufferOffset, dstBuffer));
        }
        else
        {
            gl::Rectangle srcRowRegion(area.x, area.y, area.width, 1);

            int startRow = area.y1() - 1;

            uint32_t bufferRowOffset = dstBufferOffset;
            // Copy pixels row by row
            for (int r = startRow, copiedRows = 0; copiedRows < area.height;
                 ++copiedRows, --r, bufferRowOffset += dstBufferRowPitch)
            {
                srcRowRegion.y = r;

                // Read the pixels data to the buffer's row
                ANGLE_TRY(mtl::ReadTexturePerSliceBytesToBuffer(
                    context, texture, dstBufferRowPitch, srcRowRegion,
                    renderTarget->getLevelIndex(), renderTarget->getLayerIndex(), bufferRowOffset,
                    dstBuffer));
            }
        }
    }

    return angle::Result::Continue;
}

angle::Result FramebufferMtl::unresolveIfNeeded(const gl::Context *context,
                                                mtl::RenderCommandEncoder *encoder)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    DisplayMtl *display    = contextMtl->getDisplay();

    const mtl::RenderPassDesc &renderPassDesc = encoder->renderPassDesc();
    const gl::Rectangle renderArea            = this->getCompleteRenderArea();

    mtl::BlitParams baseParams;
    baseParams.dstTextureSize = gl::Extents(renderArea.width, renderArea.height, 1);
    baseParams.dstRect        = renderArea;
    baseParams.dstScissorRect = renderArea;
    baseParams.dstFlipY       = false;

    baseParams.srcNormalizedCoords =
        mtl::NormalizedCoords(0, 0, renderArea.width, renderArea.height, renderArea);

    baseParams.srcYFlipped = false;
    baseParams.unpackFlipX = false;
    baseParams.unpackFlipY = false;

    // Unresolve any color attachment if the intended loadAction = MTLLoadActionLoad and the
    // respective MS texture is memoryless.
    mtl::ColorBlitParams colorBlitParams;
    colorBlitParams.BlitParams::operator=(baseParams);
    for (uint32_t colorIndexGL = 0; colorIndexGL < renderPassDesc.numColorAttachments;
         ++colorIndexGL)
    {
        const mtl::RenderPassColorAttachmentDesc &colorAttachment =
            renderPassDesc.colorAttachments[colorIndexGL];

        if (colorAttachment.loadAction != MTLLoadActionLoad ||
            !colorAttachment.hasImplicitMSTexture() ||
            !colorAttachment.implicitMSTexture->shouldNotLoadStore())
        {
            continue;
        }
        const RenderTargetMtl *colorRenderTarget = mColorRenderTargets[colorIndexGL];
        const angle::Format &angleFormat = colorRenderTarget->getFormat().actualAngleFormat();

        // Blit the resolve texture to the MS texture.
        colorBlitParams.src      = colorAttachment.texture;
        colorBlitParams.srcLevel = colorAttachment.level;
        colorBlitParams.srcLayer = colorAttachment.sliceOrDepth;

        colorBlitParams.enabledBuffers.reset();
        colorBlitParams.enabledBuffers.set(colorIndexGL);
        colorBlitParams.filter       = GL_NEAREST;
        colorBlitParams.dstLuminance = angleFormat.isLUMA();

        ANGLE_TRY(
            display->getUtils().blitColorWithDraw(context, encoder, angleFormat, colorBlitParams));
    }

    // Similarly, unresolve depth/stencil attachments.
    mtl::DepthStencilBlitParams dsBlitParams;
    dsBlitParams.BlitParams::operator=(baseParams);
    const mtl::RenderPassDepthAttachmentDesc &depthAttachment = renderPassDesc.depthAttachment;
    if (depthAttachment.loadAction == MTLLoadActionLoad && depthAttachment.hasImplicitMSTexture() &&
        depthAttachment.implicitMSTexture->shouldNotLoadStore())
    {
        dsBlitParams.src      = depthAttachment.texture;
        dsBlitParams.srcLevel = depthAttachment.level;
        dsBlitParams.srcLayer = depthAttachment.sliceOrDepth;
    }

    const mtl::RenderPassStencilAttachmentDesc &stencilAttachment =
        renderPassDesc.stencilAttachment;
    if (stencilAttachment.loadAction == MTLLoadActionLoad &&
        stencilAttachment.hasImplicitMSTexture() &&
        stencilAttachment.implicitMSTexture->shouldNotLoadStore())
    {
        if (mState.hasSeparateDepthAndStencilAttachments())
        {
            // Blit depth/stencil separately.
            ANGLE_TRY(contextMtl->getDisplay()->getUtils().blitDepthStencilWithDraw(
                context, encoder, dsBlitParams));
            dsBlitParams.src = nullptr;
        }

        dsBlitParams.srcStencil = stencilAttachment.texture->getStencilView();
        dsBlitParams.srcLevel   = stencilAttachment.level;
        dsBlitParams.srcLayer   = stencilAttachment.sliceOrDepth;
    }

    if (dsBlitParams.src || dsBlitParams.srcStencil)
    {
        ANGLE_TRY(contextMtl->getDisplay()->getUtils().blitDepthStencilWithDraw(context, encoder,
                                                                                dsBlitParams));
    }

    return angle::Result::Continue;
}

}  // namespace rx
