//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferVk.cpp:
//    Implements the class methods for FramebufferVk.
//

#include "libANGLE/renderer/vulkan/FramebufferVk.h"

#include <array>

#include "common/debug.h"
#include "common/vulkan/vk_headers.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/SurfaceVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_resource.h"

namespace rx
{

namespace
{
// Clear values are only used when loadOp=Clear is set in clearWithRenderPassOp.  When starting a
// new render pass, the clear value is set to an unlikely value (bright pink) to stand out better
// in case of a bug.
constexpr VkClearValue kUninitializedClearValue = {{{0.95, 0.05, 0.95, 0.95}}};

// The value to assign an alpha channel that's emulated.  The type is unsigned int, though it will
// automatically convert to the actual data type.
constexpr unsigned int kEmulatedAlphaValue = 1;

bool HasSrcBlitFeature(vk::Renderer *renderer, RenderTargetVk *srcRenderTarget)
{
    angle::FormatID srcFormatID = srcRenderTarget->getImageActualFormatID();
    return renderer->hasImageFormatFeatureBits(srcFormatID, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
}

bool HasDstBlitFeature(vk::Renderer *renderer, RenderTargetVk *dstRenderTarget)
{
    angle::FormatID dstFormatID = dstRenderTarget->getImageActualFormatID();
    return renderer->hasImageFormatFeatureBits(dstFormatID, VK_FORMAT_FEATURE_BLIT_DST_BIT);
}

// Returns false if destination has any channel the source doesn't.  This means that channel was
// emulated and using the Vulkan blit command would overwrite that emulated channel.
bool AreSrcAndDstColorChannelsBlitCompatible(RenderTargetVk *srcRenderTarget,
                                             RenderTargetVk *dstRenderTarget)
{
    const angle::Format &srcFormat = srcRenderTarget->getImageIntendedFormat();
    const angle::Format &dstFormat = dstRenderTarget->getImageIntendedFormat();

    // Luminance/alpha formats are not renderable, so they can't have ended up in a framebuffer to
    // participate in a blit.
    ASSERT(!dstFormat.isLUMA() && !srcFormat.isLUMA());

    // All color formats have the red channel.
    ASSERT(dstFormat.redBits > 0 && srcFormat.redBits > 0);

    return (dstFormat.greenBits > 0 || srcFormat.greenBits == 0) &&
           (dstFormat.blueBits > 0 || srcFormat.blueBits == 0) &&
           (dstFormat.alphaBits > 0 || srcFormat.alphaBits == 0);
}

// Returns false if formats are not identical.  vkCmdResolveImage and resolve attachments both
// require identical formats between source and destination.  vkCmdBlitImage additionally requires
// the same for depth/stencil formats.
bool AreSrcAndDstFormatsIdentical(RenderTargetVk *srcRenderTarget, RenderTargetVk *dstRenderTarget)
{
    angle::FormatID srcFormatID = srcRenderTarget->getImageActualFormatID();
    angle::FormatID dstFormatID = dstRenderTarget->getImageActualFormatID();

    return srcFormatID == dstFormatID;
}

bool AreSrcAndDstDepthStencilChannelsBlitCompatible(RenderTargetVk *srcRenderTarget,
                                                    RenderTargetVk *dstRenderTarget)
{
    const angle::Format &srcFormat = srcRenderTarget->getImageIntendedFormat();
    const angle::Format &dstFormat = dstRenderTarget->getImageIntendedFormat();

    return (dstFormat.depthBits > 0 || srcFormat.depthBits == 0) &&
           (dstFormat.stencilBits > 0 || srcFormat.stencilBits == 0);
}

void EarlyAdjustFlipYForPreRotation(SurfaceRotation blitAngleIn,
                                    SurfaceRotation *blitAngleOut,
                                    bool *blitFlipYOut)
{
    switch (blitAngleIn)
    {
        case SurfaceRotation::Identity:
            // No adjustments needed
            break;
        case SurfaceRotation::Rotated90Degrees:
            *blitAngleOut = SurfaceRotation::Rotated90Degrees;
            *blitFlipYOut = false;
            break;
        case SurfaceRotation::Rotated180Degrees:
            *blitAngleOut = SurfaceRotation::Rotated180Degrees;
            break;
        case SurfaceRotation::Rotated270Degrees:
            *blitAngleOut = SurfaceRotation::Rotated270Degrees;
            *blitFlipYOut = false;
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void AdjustBlitAreaForPreRotation(SurfaceRotation framebufferAngle,
                                  const gl::Rectangle &blitAreaIn,
                                  const gl::Rectangle &framebufferDimensions,
                                  gl::Rectangle *blitAreaOut)
{
    switch (framebufferAngle)
    {
        case SurfaceRotation::Identity:
            // No adjustments needed
            break;
        case SurfaceRotation::Rotated90Degrees:
            blitAreaOut->x = blitAreaIn.y;
            blitAreaOut->y = blitAreaIn.x;
            std::swap(blitAreaOut->width, blitAreaOut->height);
            break;
        case SurfaceRotation::Rotated180Degrees:
            blitAreaOut->x = framebufferDimensions.width - blitAreaIn.x - blitAreaIn.width;
            blitAreaOut->y = framebufferDimensions.height - blitAreaIn.y - blitAreaIn.height;
            break;
        case SurfaceRotation::Rotated270Degrees:
            blitAreaOut->x = framebufferDimensions.height - blitAreaIn.y - blitAreaIn.height;
            blitAreaOut->y = framebufferDimensions.width - blitAreaIn.x - blitAreaIn.width;
            std::swap(blitAreaOut->width, blitAreaOut->height);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void AdjustDimensionsAndFlipForPreRotation(SurfaceRotation framebufferAngle,
                                           gl::Rectangle *framebufferDimensions,
                                           bool *flipX,
                                           bool *flipY)
{
    switch (framebufferAngle)
    {
        case SurfaceRotation::Identity:
            // No adjustments needed
            break;
        case SurfaceRotation::Rotated90Degrees:
            std::swap(framebufferDimensions->width, framebufferDimensions->height);
            std::swap(*flipX, *flipY);
            break;
        case SurfaceRotation::Rotated180Degrees:
            break;
        case SurfaceRotation::Rotated270Degrees:
            std::swap(framebufferDimensions->width, framebufferDimensions->height);
            std::swap(*flipX, *flipY);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

// When blitting, the source and destination areas are viewed like UVs.  For example, a 64x64
// texture if flipped should have an offset of 64 in either X or Y which corresponds to U or V of 1.
// On the other hand, when resolving, the source and destination areas are used as fragment
// coordinates to fetch from.  In that case, when flipped, the texture in the above example must
// have an offset of 63.
void AdjustBlitResolveParametersForResolve(const gl::Rectangle &sourceArea,
                                           const gl::Rectangle &destArea,
                                           UtilsVk::BlitResolveParameters *params)
{
    params->srcOffset[0] = sourceArea.x;
    params->srcOffset[1] = sourceArea.y;
    params->dstOffset[0] = destArea.x;
    params->dstOffset[1] = destArea.y;

    if (sourceArea.isReversedX())
    {
        ASSERT(sourceArea.x > 0);
        --params->srcOffset[0];
    }
    if (sourceArea.isReversedY())
    {
        ASSERT(sourceArea.y > 0);
        --params->srcOffset[1];
    }
    if (destArea.isReversedX())
    {
        ASSERT(destArea.x > 0);
        --params->dstOffset[0];
    }
    if (destArea.isReversedY())
    {
        ASSERT(destArea.y > 0);
        --params->dstOffset[1];
    }
}

// Potentially make adjustments for pre-rotatation.  Depending on the angle some of the params need
// to be swapped and/or changes made to which axis are flipped.
void AdjustBlitResolveParametersForPreRotation(SurfaceRotation framebufferAngle,
                                               SurfaceRotation srcFramebufferAngle,
                                               UtilsVk::BlitResolveParameters *params)
{
    switch (framebufferAngle)
    {
        case SurfaceRotation::Identity:
            break;
        case SurfaceRotation::Rotated90Degrees:
            std::swap(params->stretch[0], params->stretch[1]);
            std::swap(params->srcOffset[0], params->srcOffset[1]);
            std::swap(params->rotatedOffsetFactor[0], params->rotatedOffsetFactor[1]);
            std::swap(params->flipX, params->flipY);
            if (srcFramebufferAngle == framebufferAngle)
            {
                std::swap(params->dstOffset[0], params->dstOffset[1]);
                std::swap(params->stretch[0], params->stretch[1]);
            }
            break;
        case SurfaceRotation::Rotated180Degrees:
            // Combine flip info with api flip.
            params->flipX = !params->flipX;
            params->flipY = !params->flipY;
            break;
        case SurfaceRotation::Rotated270Degrees:
            std::swap(params->stretch[0], params->stretch[1]);
            std::swap(params->srcOffset[0], params->srcOffset[1]);
            std::swap(params->rotatedOffsetFactor[0], params->rotatedOffsetFactor[1]);
            if (srcFramebufferAngle == framebufferAngle)
            {
                std::swap(params->stretch[0], params->stretch[1]);
            }
            // Combine flip info with api flip.
            params->flipX = !params->flipX;
            params->flipY = !params->flipY;
            std::swap(params->flipX, params->flipY);

            break;
        default:
            UNREACHABLE();
            break;
    }
}

vk::FramebufferNonResolveAttachmentMask MakeUnresolveAttachmentMask(const vk::RenderPassDesc &desc)
{
    vk::FramebufferNonResolveAttachmentMask unresolveMask(
        desc.getColorUnresolveAttachmentMask().bits());
    if (desc.hasDepthUnresolveAttachment() || desc.hasStencilUnresolveAttachment())
    {
        // This mask only needs to know if the depth/stencil attachment needs to be unresolved, and
        // is agnostic of the aspect.
        unresolveMask.set(vk::kUnpackedDepthIndex);
    }
    return unresolveMask;
}

bool IsAnyAttachment3DWithoutAllLayers(const RenderTargetCache<RenderTargetVk> &renderTargetCache,
                                       gl::DrawBufferMask colorAttachmentsMask,
                                       uint32_t framebufferLayerCount)
{
    const auto &colorRenderTargets = renderTargetCache.getColors();
    for (size_t colorIndexGL : colorAttachmentsMask)
    {
        RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
        ASSERT(colorRenderTarget);

        const vk::ImageHelper &image = colorRenderTarget->getImageForRenderPass();

        if (image.getType() == VK_IMAGE_TYPE_3D && image.getExtents().depth > framebufferLayerCount)
        {
            return true;
        }
    }

    // Depth/stencil attachments cannot be 3D.
    ASSERT(renderTargetCache.getDepthStencil() == nullptr ||
           renderTargetCache.getDepthStencil()->getImageForRenderPass().getType() !=
               VK_IMAGE_TYPE_3D);

    return false;
}

// Should be called when the image type is VK_IMAGE_TYPE_3D.  Typically, the subresource, offsets
// and extents are filled in as if images are 2D layers (because depth slices of 3D images are also
// specified through "layers" everywhere, particularly by gl::ImageIndex).  This function adjusts
// the layer base/count and offsets.z/extents.z appropriately after these structs are set up.
void AdjustLayersAndDepthFor3DImages(VkImageSubresourceLayers *subresource,
                                     VkOffset3D *offsetsStart,
                                     VkOffset3D *offsetsEnd)
{
    // The struct must be set up as if the image was 2D array.
    ASSERT(offsetsStart->z == 0);
    ASSERT(offsetsEnd->z == 1);

    offsetsStart->z = subresource->baseArrayLayer;
    offsetsEnd->z   = subresource->baseArrayLayer + subresource->layerCount;

    subresource->baseArrayLayer = 0;
    subresource->layerCount     = 1;
}

bool AllowAddingResolveAttachmentsToSubpass(const vk::RenderPassDesc &desc)
{
    // When in render-to-texture emulation mode, there are already resolve attachments present, and
    // render pass compatibility rules would require packing those first before packing resolve
    // attachments that may be added later (through glBlitFramebuffer).  While supporting that is
    // not onerous, the code is simplified by not supporting this combination.  In practice no
    // application should be mixing MSRTT textures and and truly multisampled textures in the same
    // framebuffer (they could be using MSRTT for both).
    //
    // For the same reason, adding resolve attachments after the fact is disabled with YUV resolve.
    return !desc.isRenderToTexture() && !desc.hasYUVResolveAttachment();
}
}  // anonymous namespace

FramebufferVk::FramebufferVk(vk::Renderer *renderer, const gl::FramebufferState &state)
    : FramebufferImpl(state), mBackbuffer(nullptr), mActiveColorComponentMasksForClear(0)
{
    if (mState.isDefault())
    {
        // These are immutable for system default framebuffer.
        mCurrentFramebufferDesc.updateLayerCount(1);
        mCurrentFramebufferDesc.updateIsMultiview(false);
    }

    mIsCurrentFramebufferCached = !renderer->getFeatures().supportsImagelessFramebuffer.enabled;
    mIsYUVResolve               = false;
}

FramebufferVk::~FramebufferVk() = default;

void FramebufferVk::destroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (mFragmentShadingRateImage.valid())
    {
        vk::Renderer *renderer = contextVk->getRenderer();
        mFragmentShadingRateImageView.release(renderer, mFragmentShadingRateImage.getResourceUse());
        mFragmentShadingRateImage.releaseImage(renderer);
    }

    releaseCurrentFramebuffer(contextVk);
}

void FramebufferVk::insertCache(ContextVk *contextVk,
                                const vk::FramebufferDesc &desc,
                                vk::FramebufferHelper &&newFramebuffer)
{
    // Add it into per share group cache
    contextVk->getShareGroup()->getFramebufferCache().insert(contextVk, desc,
                                                             std::move(newFramebuffer));

    // Create a refcounted cache key object and have each attachment keep a refcount to it so that
    // it can be destroyed promptly if those attachments change.
    const vk::SharedFramebufferCacheKey sharedFramebufferCacheKey =
        vk::CreateSharedFramebufferCacheKey(desc);

    // Ask each attachment to hold a reference to the cache so that when any attachment is
    // released, the cache can be destroyed.
    const auto &colorRenderTargets = mRenderTargetCache.getColors();
    for (size_t colorIndexGL : mState.getColorAttachmentsMask())
    {
        colorRenderTargets[colorIndexGL]->onNewFramebuffer(sharedFramebufferCacheKey);
    }

    if (getDepthStencilRenderTarget())
    {
        getDepthStencilRenderTarget()->onNewFramebuffer(sharedFramebufferCacheKey);
    }
}

angle::Result FramebufferVk::discard(const gl::Context *context,
                                     size_t count,
                                     const GLenum *attachments)
{
    return invalidate(context, count, attachments);
}

angle::Result FramebufferVk::invalidate(const gl::Context *context,
                                        size_t count,
                                        const GLenum *attachments)
{
    ContextVk *contextVk = vk::GetImpl(context);

    ANGLE_TRY(invalidateImpl(contextVk, count, attachments, false,
                             getRotatedCompleteRenderArea(contextVk)));
    return angle::Result::Continue;
}

angle::Result FramebufferVk::invalidateSub(const gl::Context *context,
                                           size_t count,
                                           const GLenum *attachments,
                                           const gl::Rectangle &area)
{
    ContextVk *contextVk = vk::GetImpl(context);

    const gl::Rectangle nonRotatedCompleteRenderArea = getNonRotatedCompleteRenderArea();
    gl::Rectangle rotatedInvalidateArea;
    RotateRectangle(contextVk->getRotationDrawFramebuffer(),
                    contextVk->isViewportFlipEnabledForDrawFBO(),
                    nonRotatedCompleteRenderArea.width, nonRotatedCompleteRenderArea.height, area,
                    &rotatedInvalidateArea);

    // If invalidateSub() covers the whole framebuffer area, make it behave as invalidate().
    // The invalidate area is clipped to the render area for use inside invalidateImpl.
    const gl::Rectangle completeRenderArea = getRotatedCompleteRenderArea(contextVk);
    if (ClipRectangle(rotatedInvalidateArea, completeRenderArea, &rotatedInvalidateArea) &&
        rotatedInvalidateArea == completeRenderArea)
    {
        return invalidate(context, count, attachments);
    }

    // If there are deferred clears, restage them.  syncState may have accumulated deferred clears,
    // but if the framebuffer's attachments are used after this call not through the framebuffer,
    // those clears wouldn't get flushed otherwise (for example as the destination of
    // glCopyTex[Sub]Image, shader storage image, etc).
    restageDeferredClears(contextVk);

    if (contextVk->hasActiveRenderPass() &&
        rotatedInvalidateArea.encloses(contextVk->getStartedRenderPassCommands().getRenderArea()))
    {
        // Because the render pass's render area is within the invalidated area, it is fine for
        // invalidateImpl() to use a storeOp of DONT_CARE (i.e. fine to not store the contents of
        // the invalidated area).
        ANGLE_TRY(invalidateImpl(contextVk, count, attachments, true, rotatedInvalidateArea));
    }
    else
    {
        ANGLE_VK_PERF_WARNING(
            contextVk, GL_DEBUG_SEVERITY_LOW,
            "InvalidateSubFramebuffer ignored due to area not covering the render area");
    }

    return angle::Result::Continue;
}

angle::Result FramebufferVk::clear(const gl::Context *context, GLbitfield mask)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "FramebufferVk::clear");
    ContextVk *contextVk = vk::GetImpl(context);

    bool clearColor   = IsMaskFlagSet(mask, static_cast<GLbitfield>(GL_COLOR_BUFFER_BIT));
    bool clearDepth   = IsMaskFlagSet(mask, static_cast<GLbitfield>(GL_DEPTH_BUFFER_BIT));
    bool clearStencil = IsMaskFlagSet(mask, static_cast<GLbitfield>(GL_STENCIL_BUFFER_BIT));
    gl::DrawBufferMask clearColorBuffers;
    if (clearColor)
    {
        clearColorBuffers = mState.getEnabledDrawBuffers();
    }

    const VkClearColorValue &clearColorValue = contextVk->getClearColorValue().color;
    const VkClearDepthStencilValue &clearDepthStencilValue =
        contextVk->getClearDepthStencilValue().depthStencil;

    return clearImpl(context, clearColorBuffers, clearDepth, clearStencil, clearColorValue,
                     clearDepthStencilValue);
}

VkClearColorValue adjustFloatClearColorPrecision(const VkClearColorValue &color,
                                                 const angle::Format &colorFormat)
{
    // Truncate x to b bits: round(x * (2^b-1)) / (2^b-1)
    // Implemented as floor(x * ((1 << b) - 1) + 0.5) / ((1 << b) - 1)

    float floatClearColorRed = color.float32[0];
    GLuint targetRedBits     = colorFormat.redBits;
    floatClearColorRed       = floor(floatClearColorRed * ((1 << targetRedBits) - 1) + 0.5f);
    floatClearColorRed       = floatClearColorRed / ((1 << targetRedBits) - 1);

    float floatClearColorGreen = color.float32[1];
    GLuint targetGreenBits     = colorFormat.greenBits;
    floatClearColorGreen       = floor(floatClearColorGreen * ((1 << targetGreenBits) - 1) + 0.5f);
    floatClearColorGreen       = floatClearColorGreen / ((1 << targetGreenBits) - 1);

    float floatClearColorBlue = color.float32[2];
    GLuint targetBlueBits     = colorFormat.blueBits;
    floatClearColorBlue       = floor(floatClearColorBlue * ((1 << targetBlueBits) - 1) + 0.5f);
    floatClearColorBlue       = floatClearColorBlue / ((1 << targetBlueBits) - 1);

    float floatClearColorAlpha = color.float32[3];
    GLuint targetAlphaBits     = colorFormat.alphaBits;
    floatClearColorAlpha       = floor(floatClearColorAlpha * ((1 << targetAlphaBits) - 1) + 0.5f);
    floatClearColorAlpha       = floatClearColorAlpha / ((1 << targetAlphaBits) - 1);

    VkClearColorValue adjustedClearColor = color;
    adjustedClearColor.float32[0]        = floatClearColorRed;
    adjustedClearColor.float32[1]        = floatClearColorGreen;
    adjustedClearColor.float32[2]        = floatClearColorBlue;
    adjustedClearColor.float32[3]        = floatClearColorAlpha;

    return adjustedClearColor;
}

angle::Result FramebufferVk::clearImpl(const gl::Context *context,
                                       gl::DrawBufferMask clearColorBuffers,
                                       bool clearDepth,
                                       bool clearStencil,
                                       const VkClearColorValue &clearColorValue,
                                       const VkClearDepthStencilValue &clearDepthStencilValue)
{
    ContextVk *contextVk = vk::GetImpl(context);

    const gl::Rectangle scissoredRenderArea = getRotatedScissoredRenderArea(contextVk);
    if (scissoredRenderArea.width == 0 || scissoredRenderArea.height == 0)
    {
        restageDeferredClears(contextVk);
        return angle::Result::Continue;
    }

    // This function assumes that only enabled attachments are asked to be cleared.
    ASSERT((clearColorBuffers & mState.getEnabledDrawBuffers()) == clearColorBuffers);
    ASSERT(!clearDepth || mState.getDepthAttachment() != nullptr);
    ASSERT(!clearStencil || mState.getStencilAttachment() != nullptr);

    gl::BlendStateExt::ColorMaskStorage::Type colorMasks = contextVk->getClearColorMasks();
    bool clearColor                                      = clearColorBuffers.any();

    // When this function is called, there should always be something to clear.
    ASSERT(clearColor || clearDepth || clearStencil);

    gl::DrawBuffersArray<VkClearColorValue> adjustedClearColorValues;
    const gl::DrawBufferMask colorAttachmentMask = mState.getColorAttachmentsMask();
    const auto &colorRenderTargets               = mRenderTargetCache.getColors();
    bool anyAttachmentWithColorspaceOverride     = false;
    for (size_t colorIndexGL = 0; colorIndexGL < colorAttachmentMask.size(); ++colorIndexGL)
    {
        if (colorAttachmentMask[colorIndexGL])
        {
            adjustedClearColorValues[colorIndexGL] = clearColorValue;

            RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
            ASSERT(colorRenderTarget);

            // If a rendertarget has colorspace overrides, we need to clear with a draw
            // to make sure the colorspace override is honored.
            anyAttachmentWithColorspaceOverride =
                anyAttachmentWithColorspaceOverride ||
                colorRenderTarget->hasColorspaceOverrideForWrite();

            if (colorRenderTarget->isYuvResolve())
            {
                // OpenGLES spec says "clear color should be defined in yuv color space and so
                // floating point r, g, and b value will be mapped to corresponding y, u and v
                // value" https://registry.khronos.org/OpenGL/extensions/EXT/EXT_YUV_target.txt.
                // But vulkan spec says "Values in the G, B, and R channels of the color
                // attachment will be written to the Y, CB, and CR channels of the external
                // format image, respectively." So we have to adjust the component mapping from
                // GL order to vulkan order.
                adjustedClearColorValues[colorIndexGL].float32[0] = clearColorValue.float32[2];
                adjustedClearColorValues[colorIndexGL].float32[1] = clearColorValue.float32[0];
                adjustedClearColorValues[colorIndexGL].float32[2] = clearColorValue.float32[1];
            }
            else if (contextVk->getFeatures().adjustClearColorPrecision.enabled)
            {
                const angle::FormatID colorRenderTargetFormat =
                    colorRenderTarget->getImageForRenderPass().getActualFormatID();
                if (colorRenderTargetFormat == angle::FormatID::R5G5B5A1_UNORM)
                {
                    // Temporary workaround for https://issuetracker.google.com/292282210 to avoid
                    // dithering being automatically applied
                    adjustedClearColorValues[colorIndexGL] = adjustFloatClearColorPrecision(
                        clearColorValue, angle::Format::Get(colorRenderTargetFormat));
                }
            }
        }
    }

    const uint8_t stencilMask =
        static_cast<uint8_t>(contextVk->getState().getDepthStencilState().stencilWritemask);

    // The front-end should ensure we don't attempt to clear color if all channels are masked.
    ASSERT(!clearColor || colorMasks != 0);
    // The front-end should ensure we don't attempt to clear depth if depth write is disabled.
    ASSERT(!clearDepth || contextVk->getState().getDepthStencilState().depthMask);
    // The front-end should ensure we don't attempt to clear stencil if all bits are masked.
    ASSERT(!clearStencil || stencilMask != 0);

    // Make sure to close the render pass now if in read-only depth/stencil feedback loop mode and
    // depth/stencil is being cleared.
    if (clearDepth || clearStencil)
    {
        ANGLE_TRY(contextVk->updateRenderPassDepthFeedbackLoopMode(
            clearDepth ? UpdateDepthFeedbackLoopReason::Clear : UpdateDepthFeedbackLoopReason::None,
            clearStencil ? UpdateDepthFeedbackLoopReason::Clear
                         : UpdateDepthFeedbackLoopReason::None));
    }

    const bool scissoredClear = scissoredRenderArea != getRotatedCompleteRenderArea(contextVk);

    // We use the draw path if scissored clear, or color or stencil are masked.  Note that depth
    // clearing is already disabled if there's a depth mask.
    const bool maskedClearColor = clearColor && (mActiveColorComponentMasksForClear & colorMasks) !=
                                                    mActiveColorComponentMasksForClear;
    const bool maskedClearStencil = clearStencil && stencilMask != 0xFF;

    bool clearColorWithDraw =
        clearColor && (maskedClearColor || scissoredClear || anyAttachmentWithColorspaceOverride);
    bool clearDepthWithDraw   = clearDepth && scissoredClear;
    bool clearStencilWithDraw = clearStencil && (maskedClearStencil || scissoredClear);

    const bool isMidRenderPassClear =
        contextVk->hasStartedRenderPassWithQueueSerial(mLastRenderPassQueueSerial) &&
        !contextVk->getStartedRenderPassCommands().getCommandBuffer().empty();
    if (isMidRenderPassClear)
    {
        // Emit debug-util markers for this mid-render-pass clear
        ANGLE_TRY(
            contextVk->handleGraphicsEventLog(rx::GraphicsEventCmdBuf::InRenderPassCmdBufQueryCmd));
    }
    else
    {
        ASSERT(!contextVk->hasActiveRenderPass() ||
               contextVk->hasStartedRenderPassWithQueueSerial(mLastRenderPassQueueSerial));
        // Emit debug-util markers for this outside-render-pass clear
        ANGLE_TRY(
            contextVk->handleGraphicsEventLog(rx::GraphicsEventCmdBuf::InOutsideCmdBufQueryCmd));
    }

    const bool preferDrawOverClearAttachments =
        contextVk->getFeatures().preferDrawClearOverVkCmdClearAttachments.enabled;

    // Merge current clears with the deferred clears, then proceed with only processing deferred
    // clears.  This simplifies the clear paths such that they don't need to consider both the
    // current and deferred clears.  Additionally, it avoids needing to undo an unresolve
    // operation; say attachment A is deferred cleared and multisampled-render-to-texture
    // attachment B is currently cleared.  Assuming a render pass needs to start (because for
    // example attachment C needs to clear with a draw path), starting one with only deferred
    // clears and then applying the current clears won't work as attachment B is unresolved, and
    // there are no facilities to undo that.
    if (preferDrawOverClearAttachments && isMidRenderPassClear)
    {
        // On buggy hardware, prefer to clear with a draw call instead of vkCmdClearAttachments.
        // Note that it's impossible to have deferred clears in the middle of the render pass.
        ASSERT(!mDeferredClears.any());

        clearColorWithDraw   = clearColor;
        clearDepthWithDraw   = clearDepth;
        clearStencilWithDraw = clearStencil;
    }
    else
    {
        gl::DrawBufferMask clearColorDrawBuffersMask;
        if (clearColor && !clearColorWithDraw)
        {
            clearColorDrawBuffersMask = clearColorBuffers;
        }

        mergeClearsWithDeferredClears(clearColorDrawBuffersMask, clearDepth && !clearDepthWithDraw,
                                      clearStencil && !clearStencilWithDraw,
                                      adjustedClearColorValues, clearDepthStencilValue);
    }

    // If any deferred clears, we can further defer them, clear them with vkCmdClearAttachments or
    // flush them if necessary.
    if (mDeferredClears.any())
    {
        const bool clearAnyWithDraw =
            clearColorWithDraw || clearDepthWithDraw || clearStencilWithDraw;

        bool isAnyAttachment3DWithoutAllLayers =
            IsAnyAttachment3DWithoutAllLayers(mRenderTargetCache, mState.getColorAttachmentsMask(),
                                              mCurrentFramebufferDesc.getLayerCount());

        // If we are in an active renderpass that has recorded commands and the framebuffer hasn't
        // changed, inline the clear.
        if (isMidRenderPassClear)
        {
            ANGLE_VK_PERF_WARNING(
                contextVk, GL_DEBUG_SEVERITY_LOW,
                "Clear effectively discarding previous draw call results. Suggest earlier Clear "
                "followed by masked color or depth/stencil draw calls instead, or "
                "glInvalidateFramebuffer to discard data instead");

            ASSERT(!preferDrawOverClearAttachments);

            // clearWithCommand will operate on deferred clears.
            clearWithCommand(contextVk, scissoredRenderArea, ClearWithCommand::OptimizeWithLoadOp,
                             &mDeferredClears);

            // clearWithCommand will clear only those attachments that have been used in the render
            // pass, and removes them from mDeferredClears.  Any deferred clears that are left can
            // be performed with a renderpass loadOp.
            if (mDeferredClears.any())
            {
                clearWithLoadOp(contextVk);
            }
        }
        else
        {
            if (contextVk->hasActiveRenderPass())
            {
                // Typically, clears are deferred such that it's impossible to have a render pass
                // opened without any additional commands recorded on it.  This is not true for some
                // corner cases, such as with 3D or external attachments.  In those cases, a clear
                // can open a render pass that's otherwise empty, and additional clears can continue
                // to be accumulated in the render pass loadOps.
                ASSERT(isAnyAttachment3DWithoutAllLayers || hasAnyExternalAttachments());
                clearWithLoadOp(contextVk);
            }

            // This path will defer the current clears along with deferred clears.  This won't work
            // if any attachment needs to be subsequently cleared with a draw call.  In that case,
            // flush deferred clears, which will start a render pass with deferred clear values.
            // The subsequent draw call will then operate on the cleared attachments.
            //
            // Additionally, if the framebuffer is layered, any attachment is 3D and it has a larger
            // depth than the framebuffer layers, clears cannot be deferred.  This is because the
            // clear may later need to be flushed with vkCmdClearColorImage, which cannot partially
            // clear the 3D texture.  In that case, the clears are flushed immediately too.
            //
            // For external images such as from AHBs, the clears are not deferred so that they are
            // definitely applied before the application uses them outside of the control of ANGLE.
            if (clearAnyWithDraw || isAnyAttachment3DWithoutAllLayers ||
                hasAnyExternalAttachments())
            {
                ANGLE_TRY(flushDeferredClears(contextVk));
            }
            else
            {
                restageDeferredClears(contextVk);
            }
        }

        // If nothing left to clear, early out.
        if (!clearAnyWithDraw)
        {
            ASSERT(mDeferredClears.empty());
            return angle::Result::Continue;
        }
    }

    if (!clearColorWithDraw)
    {
        clearColorBuffers.reset();
    }

    // If we reach here simply because the clear is scissored (as opposed to masked), use
    // vkCmdClearAttachments to clear the attachments.  The attachments that are masked will
    // continue to use a draw call.  For depth, vkCmdClearAttachments can always be used, and no
    // shader/pipeline support would then be required (though this is pending removal of the
    // preferDrawOverClearAttachments workaround).
    //
    // A potential optimization is to use loadOp=Clear for scissored clears, but care needs to be
    // taken to either break the render pass on growRenderArea(), or to turn the op back to Load and
    // revert to vkCmdClearAttachments.  This is not currently deemed necessary.
    if (((clearColorBuffers.any() && !mEmulatedAlphaAttachmentMask.any() && !maskedClearColor) ||
         clearDepthWithDraw || (clearStencilWithDraw && !maskedClearStencil)) &&
        !preferDrawOverClearAttachments && !anyAttachmentWithColorspaceOverride)
    {
        if (!contextVk->hasActiveRenderPass())
        {
            // Start a new render pass if necessary to record the commands.
            vk::RenderPassCommandBuffer *commandBuffer;
            gl::Rectangle renderArea = getRenderArea(contextVk);
            ANGLE_TRY(contextVk->startRenderPass(renderArea, &commandBuffer, nullptr));
        }

        // Build clear values
        vk::ClearValuesArray clears;
        if (!maskedClearColor && !mEmulatedAlphaAttachmentMask.any())
        {
            VkClearValue colorClearValue = {};
            for (size_t colorIndexGL : clearColorBuffers)
            {
                colorClearValue.color = adjustedClearColorValues[colorIndexGL];
                clears.store(static_cast<uint32_t>(colorIndexGL), VK_IMAGE_ASPECT_COLOR_BIT,
                             colorClearValue);
            }
            clearColorBuffers.reset();
        }
        VkImageAspectFlags dsAspectFlags = 0;
        if (clearDepthWithDraw)
        {
            dsAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
            clearDepthWithDraw = false;
        }
        if (clearStencilWithDraw && !maskedClearStencil)
        {
            dsAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
            clearStencilWithDraw = false;
        }
        if (dsAspectFlags != 0)
        {
            VkClearValue dsClearValue = {};
            dsClearValue.depthStencil = clearDepthStencilValue;
            clears.store(vk::kUnpackedDepthIndex, dsAspectFlags, dsClearValue);
        }

        clearWithCommand(contextVk, scissoredRenderArea, ClearWithCommand::Always, &clears);

        if (!clearColorBuffers.any() && !clearStencilWithDraw)
        {
            ASSERT(!clearDepthWithDraw);
            return angle::Result::Continue;
        }
    }

    // The most costly clear mode is when we need to mask out specific color channels or stencil
    // bits. This can only be done with a draw call.
    return clearWithDraw(contextVk, scissoredRenderArea, clearColorBuffers, clearDepthWithDraw,
                         clearStencilWithDraw, colorMasks, stencilMask, adjustedClearColorValues,
                         clearDepthStencilValue);
}

angle::Result FramebufferVk::clearBufferfv(const gl::Context *context,
                                           GLenum buffer,
                                           GLint drawbuffer,
                                           const GLfloat *values)
{
    VkClearValue clearValue = {};

    bool clearDepth = false;
    gl::DrawBufferMask clearColorBuffers;

    if (buffer == GL_DEPTH)
    {
        clearDepth                    = true;
        clearValue.depthStencil.depth = values[0];
    }
    else
    {
        clearColorBuffers.set(drawbuffer);
        clearValue.color.float32[0] = values[0];
        clearValue.color.float32[1] = values[1];
        clearValue.color.float32[2] = values[2];
        clearValue.color.float32[3] = values[3];
    }

    return clearImpl(context, clearColorBuffers, clearDepth, false, clearValue.color,
                     clearValue.depthStencil);
}

angle::Result FramebufferVk::clearBufferuiv(const gl::Context *context,
                                            GLenum buffer,
                                            GLint drawbuffer,
                                            const GLuint *values)
{
    VkClearValue clearValue = {};

    gl::DrawBufferMask clearColorBuffers;
    clearColorBuffers.set(drawbuffer);

    clearValue.color.uint32[0] = values[0];
    clearValue.color.uint32[1] = values[1];
    clearValue.color.uint32[2] = values[2];
    clearValue.color.uint32[3] = values[3];

    return clearImpl(context, clearColorBuffers, false, false, clearValue.color,
                     clearValue.depthStencil);
}

angle::Result FramebufferVk::clearBufferiv(const gl::Context *context,
                                           GLenum buffer,
                                           GLint drawbuffer,
                                           const GLint *values)
{
    VkClearValue clearValue = {};

    bool clearStencil = false;
    gl::DrawBufferMask clearColorBuffers;

    if (buffer == GL_STENCIL)
    {
        clearStencil                    = true;
        clearValue.depthStencil.stencil = static_cast<uint8_t>(values[0]);
    }
    else
    {
        clearColorBuffers.set(drawbuffer);
        clearValue.color.int32[0] = values[0];
        clearValue.color.int32[1] = values[1];
        clearValue.color.int32[2] = values[2];
        clearValue.color.int32[3] = values[3];
    }

    return clearImpl(context, clearColorBuffers, false, clearStencil, clearValue.color,
                     clearValue.depthStencil);
}

angle::Result FramebufferVk::clearBufferfi(const gl::Context *context,
                                           GLenum buffer,
                                           GLint drawbuffer,
                                           GLfloat depth,
                                           GLint stencil)
{
    VkClearValue clearValue = {};

    clearValue.depthStencil.depth   = depth;
    clearValue.depthStencil.stencil = static_cast<uint8_t>(stencil);

    return clearImpl(context, gl::DrawBufferMask(), true, true, clearValue.color,
                     clearValue.depthStencil);
}

const gl::InternalFormat &FramebufferVk::getImplementationColorReadFormat(
    const gl::Context *context) const
{
    ContextVk *contextVk       = vk::GetImpl(context);
    GLenum sizedFormat         = mState.getReadAttachment()->getFormat().info->sizedInternalFormat;
    const vk::Format &vkFormat = contextVk->getRenderer()->getFormat(sizedFormat);
    GLenum implFormat = vkFormat.getActualRenderableImageFormat().fboImplementationInternalFormat;
    return gl::GetSizedInternalFormatInfo(implFormat);
}

angle::Result FramebufferVk::readPixels(const gl::Context *context,
                                        const gl::Rectangle &area,
                                        GLenum format,
                                        GLenum type,
                                        const gl::PixelPackState &pack,
                                        gl::Buffer *packBuffer,
                                        void *pixels)
{
    // Clip read area to framebuffer.
    const gl::Extents &fbSize = getState().getReadPixelsAttachment(format)->getSize();
    const gl::Rectangle fbRect(0, 0, fbSize.width, fbSize.height);
    ContextVk *contextVk = vk::GetImpl(context);

    gl::Rectangle clippedArea;
    if (!ClipRectangle(area, fbRect, &clippedArea))
    {
        // nothing to read
        return angle::Result::Continue;
    }

    // Flush any deferred clears.
    ANGLE_TRY(flushDeferredClears(contextVk));

    GLuint outputSkipBytes = 0;
    PackPixelsParams params;
    ANGLE_TRY(vk::ImageHelper::GetReadPixelsParams(contextVk, pack, packBuffer, format, type, area,
                                                   clippedArea, &params, &outputSkipBytes));

    bool flipY = contextVk->isViewportFlipEnabledForReadFBO();
    switch (params.rotation = contextVk->getRotationReadFramebuffer())
    {
        case SurfaceRotation::Identity:
            // Do not rotate gl_Position (surface matches the device's orientation):
            if (flipY)
            {
                params.area.y = fbRect.height - clippedArea.y - clippedArea.height;
            }
            break;
        case SurfaceRotation::Rotated90Degrees:
            // Rotate gl_Position 90 degrees:
            params.area.x = clippedArea.y;
            params.area.y =
                flipY ? clippedArea.x : fbRect.width - clippedArea.x - clippedArea.width;
            std::swap(params.area.width, params.area.height);
            break;
        case SurfaceRotation::Rotated180Degrees:
            // Rotate gl_Position 180 degrees:
            params.area.x = fbRect.width - clippedArea.x - clippedArea.width;
            params.area.y =
                flipY ? clippedArea.y : fbRect.height - clippedArea.y - clippedArea.height;
            break;
        case SurfaceRotation::Rotated270Degrees:
            // Rotate gl_Position 270 degrees:
            params.area.x = fbRect.height - clippedArea.y - clippedArea.height;
            params.area.y =
                flipY ? fbRect.width - clippedArea.x - clippedArea.width : clippedArea.x;
            std::swap(params.area.width, params.area.height);
            break;
        default:
            UNREACHABLE();
            break;
    }
    if (flipY)
    {
        params.reverseRowOrder = !params.reverseRowOrder;
    }

    ANGLE_TRY(readPixelsImpl(contextVk, params.area, params, getReadPixelsAspectFlags(format),
                             getReadPixelsRenderTarget(format),
                             static_cast<uint8_t *>(pixels) + outputSkipBytes));
    return angle::Result::Continue;
}

RenderTargetVk *FramebufferVk::getColorDrawRenderTarget(size_t colorIndexGL) const
{
    RenderTargetVk *renderTarget = mRenderTargetCache.getColorDraw(mState, colorIndexGL);
    ASSERT(renderTarget && renderTarget->getImageForRenderPass().valid());
    return renderTarget;
}

RenderTargetVk *FramebufferVk::getColorReadRenderTarget() const
{
    RenderTargetVk *renderTarget = mRenderTargetCache.getColorRead(mState);
    ASSERT(renderTarget && renderTarget->getImageForRenderPass().valid());
    return renderTarget;
}

RenderTargetVk *FramebufferVk::getReadPixelsRenderTarget(GLenum format) const
{
    switch (format)
    {
        case GL_DEPTH_COMPONENT:
        case GL_STENCIL_INDEX_OES:
        case GL_DEPTH_STENCIL_OES:
            return getDepthStencilRenderTarget();
        default:
            return getColorReadRenderTarget();
    }
}

VkImageAspectFlagBits FramebufferVk::getReadPixelsAspectFlags(GLenum format) const
{
    switch (format)
    {
        case GL_DEPTH_COMPONENT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case GL_STENCIL_INDEX_OES:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case GL_DEPTH_STENCIL_OES:
            return vk::IMAGE_ASPECT_DEPTH_STENCIL;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

angle::Result FramebufferVk::blitWithCommand(ContextVk *contextVk,
                                             const gl::Rectangle &sourceArea,
                                             const gl::Rectangle &destArea,
                                             RenderTargetVk *readRenderTarget,
                                             RenderTargetVk *drawRenderTarget,
                                             GLenum filter,
                                             bool colorBlit,
                                             bool depthBlit,
                                             bool stencilBlit,
                                             bool flipX,
                                             bool flipY)
{
    // Since blitRenderbufferRect is called for each render buffer that needs to be blitted,
    // it should never be the case that both color and depth/stencil need to be blitted at
    // at the same time.
    ASSERT(colorBlit != (depthBlit || stencilBlit));

    vk::ImageHelper *srcImage = &readRenderTarget->getImageForCopy();
    vk::ImageHelper *dstImage = &drawRenderTarget->getImageForWrite();

    VkImageAspectFlags imageAspectMask = srcImage->getAspectFlags();
    VkImageAspectFlags blitAspectMask  = imageAspectMask;

    // Remove depth or stencil aspects if they are not requested to be blitted.
    if (!depthBlit)
    {
        blitAspectMask &= ~VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (!stencilBlit)
    {
        blitAspectMask &= ~VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    vk::CommandBufferAccess access;
    access.onImageTransferRead(imageAspectMask, srcImage);
    access.onImageTransferWrite(drawRenderTarget->getLevelIndex(), 1,
                                drawRenderTarget->getLayerIndex(), 1, imageAspectMask, dstImage);
    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    VkImageBlit blit               = {};
    blit.srcSubresource.aspectMask = blitAspectMask;
    blit.srcSubresource.mipLevel   = srcImage->toVkLevel(readRenderTarget->getLevelIndex()).get();
    blit.srcSubresource.baseArrayLayer = readRenderTarget->getLayerIndex();
    blit.srcSubresource.layerCount     = 1;
    blit.srcOffsets[0]                 = {sourceArea.x0(), sourceArea.y0(), 0};
    blit.srcOffsets[1]                 = {sourceArea.x1(), sourceArea.y1(), 1};
    blit.dstSubresource.aspectMask     = blitAspectMask;
    blit.dstSubresource.mipLevel = dstImage->toVkLevel(drawRenderTarget->getLevelIndex()).get();
    blit.dstSubresource.baseArrayLayer = drawRenderTarget->getLayerIndex();
    blit.dstSubresource.layerCount     = 1;
    blit.dstOffsets[0]                 = {destArea.x0(), destArea.y0(), 0};
    blit.dstOffsets[1]                 = {destArea.x1(), destArea.y1(), 1};

    // Note: vkCmdBlitImage doesn't actually work between 3D and 2D array images due to Vulkan valid
    // usage restrictions (https://gitlab.khronos.org/vulkan/vulkan/-/issues/3490), but drivers seem
    // to work as expected anyway.  ANGLE continues to use vkCmdBlitImage in that case.

    const bool isSrc3D = srcImage->getType() == VK_IMAGE_TYPE_3D;
    const bool isDst3D = dstImage->getType() == VK_IMAGE_TYPE_3D;
    if (isSrc3D)
    {
        AdjustLayersAndDepthFor3DImages(&blit.srcSubresource, &blit.srcOffsets[0],
                                        &blit.srcOffsets[1]);
    }
    if (isDst3D)
    {
        AdjustLayersAndDepthFor3DImages(&blit.dstSubresource, &blit.dstOffsets[0],
                                        &blit.dstOffsets[1]);
    }

    commandBuffer->blitImage(srcImage->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             dstImage->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                             gl_vk::GetFilter(filter));

    return angle::Result::Continue;
}

angle::Result FramebufferVk::blit(const gl::Context *context,
                                  const gl::Rectangle &sourceAreaIn,
                                  const gl::Rectangle &destAreaIn,
                                  GLbitfield mask,
                                  GLenum filter)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();
    UtilsVk &utilsVk       = contextVk->getUtils();

    // If any clears were picked up when syncing the read framebuffer (as the blit source), restage
    // them.  They correspond to attachments that are not used in the blit.  This will cause the
    // read framebuffer to become dirty, so the attachments will be synced again on the next command
    // that might be using them.
    const gl::State &glState              = contextVk->getState();
    const gl::Framebuffer *srcFramebuffer = glState.getReadFramebuffer();
    FramebufferVk *srcFramebufferVk       = vk::GetImpl(srcFramebuffer);
    if (srcFramebufferVk->mDeferredClears.any())
    {
        srcFramebufferVk->restageDeferredClearsForReadFramebuffer(contextVk);
    }

    // We can sometimes end up in a blit with some clear commands saved. Ensure all clear commands
    // are issued before we issue the blit command.
    ANGLE_TRY(flushDeferredClears(contextVk));

    const bool blitColorBuffer   = (mask & GL_COLOR_BUFFER_BIT) != 0;
    const bool blitDepthBuffer   = (mask & GL_DEPTH_BUFFER_BIT) != 0;
    const bool blitStencilBuffer = (mask & GL_STENCIL_BUFFER_BIT) != 0;

    // If a framebuffer contains a mixture of multisampled and multisampled-render-to-texture
    // attachments, this function could be simultaneously doing a blit on one attachment and resolve
    // on another.  For the most part, this means resolve semantics apply.  However, as the resolve
    // path cannot be taken for multisampled-render-to-texture attachments, the distinction of
    // whether resolve is done for each attachment or blit is made.
    const bool isColorResolve =
        blitColorBuffer &&
        srcFramebufferVk->getColorReadRenderTarget()->getImageForCopy().getSamples() > 1;
    const bool isDepthStencilResolve =
        (blitDepthBuffer || blitStencilBuffer) &&
        srcFramebufferVk->getDepthStencilRenderTarget()->getImageForCopy().getSamples() > 1;
    const bool isResolve = isColorResolve || isDepthStencilResolve;

    bool srcFramebufferFlippedY = contextVk->isViewportFlipEnabledForReadFBO();
    bool dstFramebufferFlippedY = contextVk->isViewportFlipEnabledForDrawFBO();

    gl::Rectangle sourceArea = sourceAreaIn;
    gl::Rectangle destArea   = destAreaIn;

    // Note: GLES (all 3.x versions) require source and destination area to be identical when
    // resolving.
    ASSERT(!isResolve ||
           (sourceArea.x == destArea.x && sourceArea.y == destArea.y &&
            sourceArea.width == destArea.width && sourceArea.height == destArea.height));

    gl::Rectangle srcFramebufferDimensions = srcFramebufferVk->getNonRotatedCompleteRenderArea();
    gl::Rectangle dstFramebufferDimensions = getNonRotatedCompleteRenderArea();

    // If the destination is flipped in either direction, we will flip the source instead so that
    // the destination area is always unflipped.
    sourceArea = sourceArea.flip(destArea.isReversedX(), destArea.isReversedY());
    destArea   = destArea.removeReversal();

    // Calculate the stretch factor prior to any clipping, as it needs to remain constant.
    const double stretch[2] = {
        std::abs(sourceArea.width / static_cast<double>(destArea.width)),
        std::abs(sourceArea.height / static_cast<double>(destArea.height)),
    };

    // Potentially make adjustments for pre-rotatation.  To handle various cases (e.g. clipping)
    // and to not interrupt the normal flow of the code, different adjustments are made in
    // different parts of the code.  These first adjustments are for whether or not to flip the
    // y-axis, and to note the overall rotation (regardless of whether it is the source or
    // destination that is rotated).
    SurfaceRotation srcFramebufferRotation = contextVk->getRotationReadFramebuffer();
    SurfaceRotation dstFramebufferRotation = contextVk->getRotationDrawFramebuffer();
    SurfaceRotation rotation               = SurfaceRotation::Identity;
    // Both the source and destination cannot be rotated (which would indicate both are the default
    // framebuffer (i.e. swapchain image).
    ASSERT((srcFramebufferRotation == SurfaceRotation::Identity) ||
           (dstFramebufferRotation == SurfaceRotation::Identity));
    EarlyAdjustFlipYForPreRotation(srcFramebufferRotation, &rotation, &srcFramebufferFlippedY);
    EarlyAdjustFlipYForPreRotation(dstFramebufferRotation, &rotation, &dstFramebufferFlippedY);

    // First, clip the source area to framebuffer.  That requires transforming the destination area
    // to match the clipped source.
    gl::Rectangle absSourceArea = sourceArea.removeReversal();
    gl::Rectangle clippedSourceArea;
    if (!gl::ClipRectangle(srcFramebufferDimensions, absSourceArea, &clippedSourceArea))
    {
        return angle::Result::Continue;
    }

    // Resize the destination area based on the new size of source.  Note again that stretch is
    // calculated as SrcDimension/DestDimension.
    gl::Rectangle srcClippedDestArea;
    if (isResolve)
    {
        // Source and destination areas are identical in resolve (except rotate it, if appropriate).
        srcClippedDestArea = clippedSourceArea;
        AdjustBlitAreaForPreRotation(dstFramebufferRotation, clippedSourceArea,
                                     dstFramebufferDimensions, &srcClippedDestArea);
    }
    else if (clippedSourceArea == absSourceArea)
    {
        // If there was no clipping, keep destination area as is (except rotate it, if appropriate).
        srcClippedDestArea = destArea;
        AdjustBlitAreaForPreRotation(dstFramebufferRotation, destArea, dstFramebufferDimensions,
                                     &srcClippedDestArea);
    }
    else
    {
        // Shift destination area's x0,y0,x1,y1 by as much as the source area's got shifted (taking
        // stretching into account).  Note that double is used as float doesn't have enough
        // precision near the end of int range.
        double x0Shift = std::round((clippedSourceArea.x - absSourceArea.x) / stretch[0]);
        double y0Shift = std::round((clippedSourceArea.y - absSourceArea.y) / stretch[1]);
        double x1Shift = std::round((absSourceArea.x1() - clippedSourceArea.x1()) / stretch[0]);
        double y1Shift = std::round((absSourceArea.y1() - clippedSourceArea.y1()) / stretch[1]);

        // If the source area was reversed in any direction, the shift should be applied in the
        // opposite direction as well.
        if (sourceArea.isReversedX())
        {
            std::swap(x0Shift, x1Shift);
        }

        if (sourceArea.isReversedY())
        {
            std::swap(y0Shift, y1Shift);
        }

        srcClippedDestArea.x = destArea.x0() + static_cast<int>(x0Shift);
        srcClippedDestArea.y = destArea.y0() + static_cast<int>(y0Shift);
        int x1               = destArea.x1() - static_cast<int>(x1Shift);
        int y1               = destArea.y1() - static_cast<int>(y1Shift);

        srcClippedDestArea.width  = x1 - srcClippedDestArea.x;
        srcClippedDestArea.height = y1 - srcClippedDestArea.y;

        // Rotate srcClippedDestArea if the destination is rotated
        if (dstFramebufferRotation != SurfaceRotation::Identity)
        {
            gl::Rectangle originalSrcClippedDestArea = srcClippedDestArea;
            AdjustBlitAreaForPreRotation(dstFramebufferRotation, originalSrcClippedDestArea,
                                         dstFramebufferDimensions, &srcClippedDestArea);
        }
    }

    // If framebuffers are flipped in Y, flip the source and destination area (which define the
    // transformation regardless of clipping), as well as the blit area (which is the clipped
    // destination area).
    if (srcFramebufferFlippedY)
    {
        sourceArea.y      = srcFramebufferDimensions.height - sourceArea.y;
        sourceArea.height = -sourceArea.height;
    }
    if (dstFramebufferFlippedY)
    {
        destArea.y      = dstFramebufferDimensions.height - destArea.y;
        destArea.height = -destArea.height;

        srcClippedDestArea.y =
            dstFramebufferDimensions.height - srcClippedDestArea.y - srcClippedDestArea.height;
    }

    bool flipX = sourceArea.isReversedX() != destArea.isReversedX();
    bool flipY = sourceArea.isReversedY() != destArea.isReversedY();

    // GLES doesn't allow flipping the parameters of glBlitFramebuffer if performing a resolve.
    ASSERT(!isResolve ||
           (flipX == false && flipY == (srcFramebufferFlippedY != dstFramebufferFlippedY)));

    // Again, transfer the destination flip to source, so destination is unflipped.  Note that
    // destArea was not reversed until the final possible Y-flip.
    ASSERT(!destArea.isReversedX());
    sourceArea = sourceArea.flip(false, destArea.isReversedY());
    destArea   = destArea.removeReversal();

    // Now that clipping and flipping is done, rotate certain values that will be used for
    // UtilsVk::BlitResolveParameters
    gl::Rectangle sourceAreaOld = sourceArea;
    gl::Rectangle destAreaOld   = destArea;
    if (srcFramebufferRotation == rotation)
    {
        AdjustBlitAreaForPreRotation(srcFramebufferRotation, sourceAreaOld,
                                     srcFramebufferDimensions, &sourceArea);
        AdjustDimensionsAndFlipForPreRotation(srcFramebufferRotation, &srcFramebufferDimensions,
                                              &flipX, &flipY);
    }
    SurfaceRotation rememberDestFramebufferRotation = dstFramebufferRotation;
    if (srcFramebufferRotation == SurfaceRotation::Rotated90Degrees)
    {
        dstFramebufferRotation = rotation;
    }
    AdjustBlitAreaForPreRotation(dstFramebufferRotation, destAreaOld, dstFramebufferDimensions,
                                 &destArea);
    dstFramebufferRotation = rememberDestFramebufferRotation;

    // Clip the destination area to the framebuffer size and scissor.  Note that we don't care
    // about the source area anymore.  The offset translation is done based on the original source
    // and destination rectangles.  The stretch factor is already calculated as well.
    gl::Rectangle blitArea;
    if (!gl::ClipRectangle(getRotatedScissoredRenderArea(contextVk), srcClippedDestArea, &blitArea))
    {
        return angle::Result::Continue;
    }

    bool noClip = blitArea == destArea && stretch[0] == 1.0f && stretch[1] == 1.0f;
    bool noFlip = !flipX && !flipY;
    bool disableFlippingBlitWithCommand =
        renderer->getFeatures().disableFlippingBlitWithCommand.enabled;

    UtilsVk::BlitResolveParameters commonParams;
    commonParams.srcOffset[0]           = sourceArea.x;
    commonParams.srcOffset[1]           = sourceArea.y;
    commonParams.dstOffset[0]           = destArea.x;
    commonParams.dstOffset[1]           = destArea.y;
    commonParams.rotatedOffsetFactor[0] = std::abs(sourceArea.width);
    commonParams.rotatedOffsetFactor[1] = std::abs(sourceArea.height);
    commonParams.stretch[0]             = static_cast<float>(stretch[0]);
    commonParams.stretch[1]             = static_cast<float>(stretch[1]);
    commonParams.srcExtents[0]          = srcFramebufferDimensions.width;
    commonParams.srcExtents[1]          = srcFramebufferDimensions.height;
    commonParams.blitArea               = blitArea;
    commonParams.linear                 = filter == GL_LINEAR && !isResolve;
    commonParams.flipX                  = flipX;
    commonParams.flipY                  = flipY;
    commonParams.rotation               = rotation;

    if (blitColorBuffer)
    {
        RenderTargetVk *readRenderTarget      = srcFramebufferVk->getColorReadRenderTarget();
        UtilsVk::BlitResolveParameters params = commonParams;
        params.srcLayer                       = readRenderTarget->getLayerIndex();

        // Multisampled images are not allowed to have mips.
        ASSERT(!isColorResolve || readRenderTarget->getLevelIndex() == gl::LevelIndex(0));

        // If there was no clipping and the format capabilities allow us, use Vulkan's builtin blit.
        // The reason clipping is prohibited in this path is that due to rounding errors, it would
        // be hard to guarantee the image stretching remains perfect.  That also allows us not to
        // have to transform back the destination clipping to source.
        //
        // Non-identity pre-rotation cases do not use Vulkan's builtin blit.  Additionally, blits
        // between 3D and non-3D-non-layer-0 images are forbidden (possibly due to an oversight:
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3490)
        //
        // For simplicity, we either blit all render targets with a Vulkan command, or none.
        bool canBlitWithCommand =
            !isColorResolve && noClip && (noFlip || !disableFlippingBlitWithCommand) &&
            HasSrcBlitFeature(renderer, readRenderTarget) && rotation == SurfaceRotation::Identity;

        // If we need to reinterpret the colorspace of the read RenderTarget then the blit must be
        // done through a shader
        bool reinterpretsColorspace      = readRenderTarget->hasColorspaceOverrideForRead();
        bool areChannelsBlitCompatible   = true;
        bool areFormatsIdentical         = true;
        bool colorAttachmentAlreadyInUse = false;
        for (size_t colorIndexGL : mState.getEnabledDrawBuffers())
        {
            RenderTargetVk *drawRenderTarget = mRenderTargetCache.getColors()[colorIndexGL];
            canBlitWithCommand =
                canBlitWithCommand && HasDstBlitFeature(renderer, drawRenderTarget);
            areChannelsBlitCompatible =
                areChannelsBlitCompatible &&
                AreSrcAndDstColorChannelsBlitCompatible(readRenderTarget, drawRenderTarget);
            areFormatsIdentical = areFormatsIdentical &&
                                  AreSrcAndDstFormatsIdentical(readRenderTarget, drawRenderTarget);

            // If any color attachment of the draw framebuffer was already in use in the currently
            // started renderpass, don't reuse the renderpass for blit.
            colorAttachmentAlreadyInUse =
                colorAttachmentAlreadyInUse || contextVk->isRenderPassStartedAndUsesImage(
                                                   drawRenderTarget->getImageForRenderPass());

            // If we need to reinterpret the colorspace of the draw RenderTarget then the blit must
            // be done through a shader
            reinterpretsColorspace =
                reinterpretsColorspace || drawRenderTarget->hasColorspaceOverrideForWrite();
        }

        // Now that all flipping is done, adjust the offsets for resolve and prerotation
        if (isColorResolve)
        {
            AdjustBlitResolveParametersForResolve(sourceArea, destArea, &params);
        }
        AdjustBlitResolveParametersForPreRotation(rotation, srcFramebufferRotation, &params);

        if (canBlitWithCommand && areChannelsBlitCompatible && !reinterpretsColorspace)
        {
            for (size_t colorIndexGL : mState.getEnabledDrawBuffers())
            {
                RenderTargetVk *drawRenderTarget = mRenderTargetCache.getColors()[colorIndexGL];
                ANGLE_TRY(blitWithCommand(contextVk, sourceArea, destArea, readRenderTarget,
                                          drawRenderTarget, filter, true, false, false, flipX,
                                          flipY));
            }
        }
        // If we're not flipping or rotating, use Vulkan's builtin resolve.
        else if (isColorResolve && !flipX && !flipY && areChannelsBlitCompatible &&
                 areFormatsIdentical && rotation == SurfaceRotation::Identity &&
                 !reinterpretsColorspace)
        {
            // Resolving with a subpass resolve attachment has a few restrictions:
            // 1.) glBlitFramebuffer() needs to copy the read color attachment to all enabled
            // attachments in the draw framebuffer, but Vulkan requires a 1:1 relationship for
            // multisample attachments to resolve attachments in the render pass subpass.
            // Due to this, we currently only support using resolve attachments when there is a
            // single draw attachment enabled.
            // 2.) Using a subpass resolve attachment relies on using the render pass that performs
            // the draw to still be open, so it can be updated to use the resolve attachment to draw
            // into. If there's no render pass with commands, then the multisampled render pass is
            // already done and whose data is already flushed from the tile (in a tile-based
            // renderer), so there's no chance for the resolve attachment to take advantage of the
            // data already being present in the tile.

            // Additionally, when resolving with a resolve attachment, the src and destination
            // offsets must match, the render area must match the resolve area, and there should be
            // no flipping or rotation.  Fortunately, in GLES the blit source and destination areas
            // are already required to be identical.
            ASSERT(params.srcOffset[0] == params.dstOffset[0] &&
                   params.srcOffset[1] == params.dstOffset[1]);
            bool canResolveWithSubpass = mState.getEnabledDrawBuffers().count() == 1 &&
                                         mCurrentFramebufferDesc.getLayerCount() == 1 &&
                                         contextVk->hasStartedRenderPassWithQueueSerial(
                                             srcFramebufferVk->getLastRenderPassQueueSerial()) &&
                                         !colorAttachmentAlreadyInUse;

            if (canResolveWithSubpass)
            {
                const vk::RenderPassCommandBufferHelper &renderPassCommands =
                    contextVk->getStartedRenderPassCommands();
                const vk::RenderPassDesc &renderPassDesc = renderPassCommands.getRenderPassDesc();

                // Make sure that:
                // - The blit and render areas are identical
                // - There is no resolve attachment for the corresponding index already
                // Additionally, disable the optimization for a few corner cases that are
                // unrealistic and inconvenient.
                const uint32_t readColorIndexGL = srcFramebuffer->getState().getReadIndex();
                canResolveWithSubpass =
                    blitArea == renderPassCommands.getRenderArea() &&
                    !renderPassDesc.hasColorResolveAttachment(readColorIndexGL) &&
                    AllowAddingResolveAttachmentsToSubpass(renderPassDesc);
            }

            if (canResolveWithSubpass)
            {
                ANGLE_TRY(resolveColorWithSubpass(contextVk, params));
            }
            else
            {
                ANGLE_TRY(resolveColorWithCommand(contextVk, params,
                                                  &readRenderTarget->getImageForCopy()));
            }
        }
        else
        {
            // Otherwise use a shader to do blit or resolve.

            // Flush the render pass, which may incur a vkQueueSubmit, before taking any views.
            // Otherwise the view serials would not reflect the render pass they are really used in.
            // http://crbug.com/1272266#c22
            ANGLE_TRY(
                contextVk->flushCommandsAndEndRenderPass(RenderPassClosureReason::PrepareForBlit));

            const vk::ImageView *copyImageView = nullptr;
            ANGLE_TRY(readRenderTarget->getCopyImageView(contextVk, &copyImageView));
            ANGLE_TRY(utilsVk.colorBlitResolve(
                contextVk, this, &readRenderTarget->getImageForCopy(), copyImageView, params));
        }
    }

    if (blitDepthBuffer || blitStencilBuffer)
    {
        RenderTargetVk *readRenderTarget      = srcFramebufferVk->getDepthStencilRenderTarget();
        RenderTargetVk *drawRenderTarget      = mRenderTargetCache.getDepthStencil();
        UtilsVk::BlitResolveParameters params = commonParams;
        params.srcLayer                       = readRenderTarget->getLayerIndex();

        // Multisampled images are not allowed to have mips.
        ASSERT(!isDepthStencilResolve || readRenderTarget->getLevelIndex() == gl::LevelIndex(0));

        // Similarly, only blit if there's been no clipping or rotating.
        bool canBlitWithCommand =
            !isDepthStencilResolve && noClip && (noFlip || !disableFlippingBlitWithCommand) &&
            HasSrcBlitFeature(renderer, readRenderTarget) &&
            HasDstBlitFeature(renderer, drawRenderTarget) && rotation == SurfaceRotation::Identity;
        bool areChannelsBlitCompatible =
            AreSrcAndDstDepthStencilChannelsBlitCompatible(readRenderTarget, drawRenderTarget);

        // glBlitFramebuffer requires that depth/stencil blits have matching formats.
        ASSERT(AreSrcAndDstFormatsIdentical(readRenderTarget, drawRenderTarget));

        if (canBlitWithCommand && areChannelsBlitCompatible)
        {
            ANGLE_TRY(blitWithCommand(contextVk, sourceArea, destArea, readRenderTarget,
                                      drawRenderTarget, filter, false, blitDepthBuffer,
                                      blitStencilBuffer, flipX, flipY));
        }
        else
        {
            vk::ImageHelper *depthStencilImage = &readRenderTarget->getImageForCopy();

            VkImageAspectFlags resolveAspects = 0;
            if (blitDepthBuffer)
            {
                resolveAspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if (blitStencilBuffer)
            {
                resolveAspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            // See comment on canResolveWithSubpass for the color path.
            bool canResolveWithSubpass =
                isDepthStencilResolve &&
                !renderer->getFeatures().disableDepthStencilResolveThroughAttachment.enabled &&
                areChannelsBlitCompatible && mCurrentFramebufferDesc.getLayerCount() == 1 &&
                contextVk->hasStartedRenderPassWithQueueSerial(
                    srcFramebufferVk->getLastRenderPassQueueSerial()) &&
                !contextVk->isRenderPassStartedAndUsesImage(
                    drawRenderTarget->getImageForRenderPass()) &&
                noFlip && rotation == SurfaceRotation::Identity;

            if (canResolveWithSubpass)
            {
                const vk::RenderPassCommandBufferHelper &renderPassCommands =
                    contextVk->getStartedRenderPassCommands();
                const vk::RenderPassDesc &renderPassDesc = renderPassCommands.getRenderPassDesc();

                const VkImageAspectFlags depthStencilImageAspects =
                    depthStencilImage->getAspectFlags();
                const bool resolvesAllAspects =
                    (resolveAspects & depthStencilImageAspects) == depthStencilImageAspects;

                // Make sure that:
                // - The blit and render areas are identical
                // - There is no resolve attachment already
                // Additionally, disable the optimization for a few corner cases that are
                // unrealistic and inconvenient.
                //
                // Note: currently, if two separate `glBlitFramebuffer` calls are made for each
                // aspect, only the first one is optimized as a resolve attachment.  Applications
                // should use one `glBlitFramebuffer` call with both aspects if they want to resolve
                // both.
                canResolveWithSubpass =
                    blitArea == renderPassCommands.getRenderArea() &&
                    (resolvesAllAspects ||
                     renderer->getFeatures().supportsDepthStencilIndependentResolveNone.enabled) &&
                    !renderPassDesc.hasDepthStencilResolveAttachment() &&
                    AllowAddingResolveAttachmentsToSubpass(renderPassDesc);
            }

            if (canResolveWithSubpass)
            {
                ANGLE_TRY(resolveDepthStencilWithSubpass(contextVk, params, resolveAspects));
            }
            else
            {
                // See comment for the draw-based color blit.  The render pass must be flushed
                // before creating the views.
                ANGLE_TRY(contextVk->flushCommandsAndEndRenderPass(
                    RenderPassClosureReason::PrepareForBlit));

                // Now that all flipping is done, adjust the offsets for resolve and prerotation
                if (isDepthStencilResolve)
                {
                    AdjustBlitResolveParametersForResolve(sourceArea, destArea, &params);
                }
                AdjustBlitResolveParametersForPreRotation(rotation, srcFramebufferRotation,
                                                          &params);

                // Get depth- and stencil-only views for reading.
                const vk::ImageView *depthView   = nullptr;
                const vk::ImageView *stencilView = nullptr;

                if (blitDepthBuffer)
                {
                    ANGLE_TRY(readRenderTarget->getDepthOrStencilImageViewForCopy(
                        contextVk, VK_IMAGE_ASPECT_DEPTH_BIT, &depthView));
                }

                if (blitStencilBuffer)
                {
                    ANGLE_TRY(readRenderTarget->getDepthOrStencilImageViewForCopy(
                        contextVk, VK_IMAGE_ASPECT_STENCIL_BIT, &stencilView));
                }

                // If shader stencil export is not possible, defer stencil blit/resolve to another
                // pass.
                const bool hasShaderStencilExport =
                    renderer->getFeatures().supportsShaderStencilExport.enabled;

                // Blit depth. If shader stencil export is present, blit stencil as well.
                if (blitDepthBuffer || (blitStencilBuffer && hasShaderStencilExport))
                {
                    ANGLE_TRY(utilsVk.depthStencilBlitResolve(
                        contextVk, this, depthStencilImage, depthView,
                        hasShaderStencilExport ? stencilView : nullptr, params));
                }

                // If shader stencil export is not present, blit stencil through a different path.
                if (blitStencilBuffer && !hasShaderStencilExport)
                {
                    ANGLE_VK_PERF_WARNING(
                        contextVk, GL_DEBUG_SEVERITY_LOW,
                        "Inefficient BlitFramebuffer operation on the stencil aspect "
                        "due to lack of shader stencil export support");
                    ANGLE_TRY(utilsVk.stencilBlitResolveNoShaderExport(
                        contextVk, this, depthStencilImage, stencilView, params));
                }
            }
        }
    }

    return angle::Result::Continue;
}

void FramebufferVk::releaseCurrentFramebuffer(ContextVk *contextVk)
{
    if (mIsCurrentFramebufferCached)
    {
        mCurrentFramebuffer.release();
    }
    else
    {
        contextVk->addGarbage(&mCurrentFramebuffer);
    }
}

void FramebufferVk::updateLayerCount()
{
    uint32_t layerCount = std::numeric_limits<uint32_t>::max();

    // Color attachments.
    const auto &colorRenderTargets = mRenderTargetCache.getColors();
    for (size_t colorIndexGL : mState.getColorAttachmentsMask())
    {
        RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
        ASSERT(colorRenderTarget);
        layerCount = std::min(layerCount, colorRenderTarget->getLayerCount());
    }

    // Depth/stencil attachment.
    RenderTargetVk *depthStencilRenderTarget = getDepthStencilRenderTarget();
    if (depthStencilRenderTarget)
    {
        layerCount = std::min(layerCount, depthStencilRenderTarget->getLayerCount());
    }

    if (layerCount == std::numeric_limits<uint32_t>::max())
    {
        layerCount = mState.getDefaultLayers();
    }

    // While layer count and view count are mutually exclusive, they result in different render
    // passes (and thus framebuffers).  For multiview, layer count is set to view count and a flag
    // signifies that the framebuffer is multiview (as opposed to layered).
    const bool isMultiview = mState.isMultiview();
    if (isMultiview)
    {
        layerCount = mState.getNumViews();
    }

    mCurrentFramebufferDesc.updateLayerCount(layerCount);
    mCurrentFramebufferDesc.updateIsMultiview(isMultiview);
}

angle::Result FramebufferVk::ensureFragmentShadingRateImageAndViewInitialized(
    ContextVk *contextVk,
    const uint32_t fragmentShadingRateAttachmentWidth,
    const uint32_t fragmentShadingRateAttachmentHeight)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    // Release current valid image iff attachment extents need to change.
    if (mFragmentShadingRateImage.valid() &&
        (mFragmentShadingRateImage.getExtents().width != fragmentShadingRateAttachmentWidth ||
         mFragmentShadingRateImage.getExtents().height != fragmentShadingRateAttachmentHeight))
    {
        mFragmentShadingRateImageView.release(renderer, mFragmentShadingRateImage.getResourceUse());
        mFragmentShadingRateImage.releaseImage(renderer);
    }

    if (!mFragmentShadingRateImage.valid())
    {
        VkImageUsageFlags imageUsageFlags =
            VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        // Add storage usage iff we intend to generate data using compute shader
        if (!contextVk->getFeatures().generateFragmentShadingRateAttchementWithCpu.enabled)
        {
            imageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        ANGLE_TRY(mFragmentShadingRateImage.init(
            contextVk, gl::TextureType::_2D,
            VkExtent3D{fragmentShadingRateAttachmentWidth, fragmentShadingRateAttachmentHeight, 1},
            renderer->getFormat(angle::FormatID::R8_UINT), 1, imageUsageFlags, gl::LevelIndex(0), 1,
            1, false, contextVk->getProtectionType() == vk::ProtectionType::Protected));

        ANGLE_TRY(contextVk->initImageAllocation(
            &mFragmentShadingRateImage, false, renderer->getMemoryProperties(),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vk::MemoryAllocationType::TextureImage));

        mFragmentShadingRateImageView.init(renderer);
        ANGLE_TRY(mFragmentShadingRateImageView.initFragmentShadingRateView(
            contextVk, &mFragmentShadingRateImage));
    }

    return angle::Result::Continue;
}

angle::Result FramebufferVk::generateFragmentShadingRateWithCPU(
    ContextVk *contextVk,
    const uint32_t fragmentShadingRateWidth,
    const uint32_t fragmentShadingRateHeight,
    const uint32_t fragmentShadingRateBlockWidth,
    const uint32_t fragmentShadingRateBlockHeight,
    const uint32_t foveatedAttachmentWidth,
    const uint32_t foveatedAttachmentHeight,
    const std::vector<gl::FocalPoint> &activeFocalPoints)
{
    // Fill in image with fragment shading rate data
    size_t bufferSize                   = fragmentShadingRateWidth * fragmentShadingRateHeight;
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size               = bufferSize;
    bufferCreateInfo.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
    vk::RendererScoped<vk::BufferHelper> stagingBuffer(contextVk->getRenderer());
    vk::BufferHelper *buffer = &stagingBuffer.get();
    ANGLE_TRY(buffer->init(contextVk, bufferCreateInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    uint8_t *mappedBuffer;
    ANGLE_TRY(buffer->map(contextVk, &mappedBuffer));
    uint8_t val = 0;
    memset(mappedBuffer, 0, bufferSize);

    // The spec requires min_pixel_density to be computed thusly -
    //
    // min_pixel_density=0.;
    // for(int i=0;i<focalPointsPerLayer;++i)
    // {
    //     focal_point_density = 1./max((focalX[i]-px)^2*gainX[i]^2+
    //                         (focalY[i]-py)^2*gainY[i]^2-foveaArea[i],1.);
    //
    //     min_pixel_density=max(min_pixel_density,focal_point_density);
    // }
    float minPixelDensity   = 0.0f;
    float focalPointDensity = 0.0f;
    for (uint32_t y = 0; y < fragmentShadingRateHeight; y++)
    {
        for (uint32_t x = 0; x < fragmentShadingRateWidth; x++)
        {
            minPixelDensity = 0.0f;
            float px =
                (static_cast<float>(x) * fragmentShadingRateBlockWidth / foveatedAttachmentWidth -
                 0.5f) *
                2.0f;
            float py =
                (static_cast<float>(y) * fragmentShadingRateBlockHeight / foveatedAttachmentHeight -
                 0.5f) *
                2.0f;
            focalPointDensity = 0.0f;
            for (const gl::FocalPoint &focalPoint : activeFocalPoints)
            {
                float density = 1.0f / std::max(std::pow(focalPoint.focalX - px, 2.0f) *
                                                        std::pow(focalPoint.gainX, 2.0f) +
                                                    std::pow(focalPoint.focalY - py, 2.0f) *
                                                        std::pow(focalPoint.gainY, 2.0f) -
                                                    focalPoint.foveaArea,
                                                1.0f);

                // When focal points are overlapping choose the highest quality of all
                if (density > focalPointDensity)
                {
                    focalPointDensity = density;
                }
            }
            minPixelDensity = std::max(minPixelDensity, focalPointDensity);

            // https://docs.vulkan.org/spec/latest/chapters/primsrast.html#primsrast-fragment-shading-rate-attachment
            //
            // w = 2^((texel/4) & 3)
            // h = 2^(texel & 3)
            // `texel` would then be => log2(w) << 2 | log2(h).
            //
            // 1) The supported shading rates are - 1x1, 1x2, 2x1, 2x2
            // 2) log2(1) == 0, log2(2) == 1
            if (minPixelDensity > 0.75f)
            {
                // Use shading rate 1x1
                val = 0;
            }
            else if (minPixelDensity > 0.5f)
            {
                // Use shading rate 2x1
                val = (1 << 2);
            }
            else
            {
                // Use shading rate 2x2
                val = (1 << 2) | 1;
            }
            mappedBuffer[y * fragmentShadingRateWidth + x] = val;
        }
    }

    ANGLE_TRY(buffer->flush(contextVk->getRenderer(), 0, buffer->getSize()));
    buffer->unmap(contextVk->getRenderer());
    // copy data from staging buffer to image
    vk::CommandBufferAccess access;
    access.onBufferTransferRead(buffer);
    access.onImageTransferWrite(gl::LevelIndex(0), 1, 0, 1, VK_IMAGE_ASPECT_COLOR_BIT,
                                &mFragmentShadingRateImage);
    vk::OutsideRenderPassCommandBuffer *dataUpload;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &dataUpload));
    VkBufferImageCopy copy           = {};
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.layerCount = 1;
    copy.imageExtent.depth           = 1;
    copy.imageExtent.width           = fragmentShadingRateWidth;
    copy.imageExtent.height          = fragmentShadingRateHeight;
    dataUpload->copyBufferToImage(buffer->getBuffer().getHandle(),
                                  mFragmentShadingRateImage.getImage(),
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    return angle::Result::Continue;
}

angle::Result FramebufferVk::generateFragmentShadingRateWithCompute(
    ContextVk *contextVk,
    const uint32_t fragmentShadingRateWidth,
    const uint32_t fragmentShadingRateHeight,
    const uint32_t fragmentShadingRateBlockWidth,
    const uint32_t fragmentShadingRateBlockHeight,
    const uint32_t foveatedAttachmentWidth,
    const uint32_t foveatedAttachmentHeight,
    const std::vector<gl::FocalPoint> &activeFocalPoints)
{
    ASSERT(activeFocalPoints.size() < gl::IMPLEMENTATION_MAX_FOCAL_POINTS);

    UtilsVk::GenerateFragmentShadingRateParameters shadingRateParams;
    shadingRateParams.textureWidth          = foveatedAttachmentWidth;
    shadingRateParams.textureHeight         = foveatedAttachmentHeight;
    shadingRateParams.attachmentBlockWidth  = fragmentShadingRateBlockWidth;
    shadingRateParams.attachmentBlockHeight = fragmentShadingRateBlockHeight;
    shadingRateParams.attachmentWidth       = fragmentShadingRateWidth;
    shadingRateParams.attachmentHeight      = fragmentShadingRateHeight;
    shadingRateParams.numFocalPoints        = 0;

    for (const gl::FocalPoint &focalPoint : activeFocalPoints)
    {
        ASSERT(focalPoint.valid());
        shadingRateParams.focalPoints[shadingRateParams.numFocalPoints] = focalPoint;
        shadingRateParams.numFocalPoints++;
    }

    return contextVk->getUtils().generateFragmentShadingRate(
        contextVk, &mFragmentShadingRateImage, &mFragmentShadingRateImageView, shadingRateParams);
}

angle::Result FramebufferVk::updateFragmentShadingRateAttachment(
    ContextVk *contextVk,
    const gl::FoveationState &foveationState,
    const gl::Extents &foveatedAttachmentSize)
{
    const VkExtent2D fragmentShadingRateExtent =
        contextVk->getRenderer()->getMaxFragmentShadingRateAttachmentTexelSize();
    const uint32_t fragmentShadingRateBlockWidth  = fragmentShadingRateExtent.width;
    const uint32_t fragmentShadingRateBlockHeight = fragmentShadingRateExtent.height;
    const uint32_t foveatedAttachmentWidth        = foveatedAttachmentSize.width;
    const uint32_t foveatedAttachmentHeight       = foveatedAttachmentSize.height;
    const uint32_t fragmentShadingRateWidth =
        UnsignedCeilDivide(foveatedAttachmentWidth, fragmentShadingRateBlockWidth);
    const uint32_t fragmentShadingRateHeight =
        UnsignedCeilDivide(foveatedAttachmentHeight, fragmentShadingRateBlockHeight);

    ANGLE_TRY(ensureFragmentShadingRateImageAndViewInitialized(contextVk, fragmentShadingRateWidth,
                                                               fragmentShadingRateHeight));
    ASSERT(mFragmentShadingRateImage.valid());

    std::vector<gl::FocalPoint> activeFocalPoints;
    for (uint32_t point = 0; point < gl::IMPLEMENTATION_MAX_FOCAL_POINTS; point++)
    {
        const gl::FocalPoint &focalPoint = foveationState.getFocalPoint(0, point);
        if (focalPoint.valid())
        {
            activeFocalPoints.push_back(focalPoint);
        }
    }
    ASSERT(activeFocalPoints.size() > 0);

    if (contextVk->getFeatures().generateFragmentShadingRateAttchementWithCpu.enabled)
    {
        ANGLE_TRY(generateFragmentShadingRateWithCPU(
            contextVk, fragmentShadingRateWidth, fragmentShadingRateHeight,
            fragmentShadingRateBlockWidth, fragmentShadingRateBlockHeight, foveatedAttachmentWidth,
            foveatedAttachmentHeight, activeFocalPoints));
    }
    else
    {
        ANGLE_TRY(generateFragmentShadingRateWithCompute(
            contextVk, fragmentShadingRateWidth, fragmentShadingRateHeight,
            fragmentShadingRateBlockWidth, fragmentShadingRateBlockHeight, foveatedAttachmentWidth,
            foveatedAttachmentHeight, activeFocalPoints));
    }

    return angle::Result::Continue;
}

angle::Result FramebufferVk::updateFoveationState(ContextVk *contextVk,
                                                  const gl::FoveationState &newFoveationState,
                                                  const gl::Extents &foveatedAttachmentSize)
{
    const bool isFoveationEnabled                 = newFoveationState.isFoveated();
    vk::ImageOrBufferViewSubresourceSerial serial = vk::kInvalidImageOrBufferViewSubresourceSerial;
    if (isFoveationEnabled)
    {
        ANGLE_TRY(updateFragmentShadingRateAttachment(contextVk, newFoveationState,
                                                      foveatedAttachmentSize));
        ASSERT(mFragmentShadingRateImage.valid());

        serial = mFragmentShadingRateImageView.getSubresourceSerial(gl::LevelIndex(0), 1, 0,
                                                                    vk::LayerMode::All);
    }

    // Update state after the possible failure point.
    mFoveationState = newFoveationState;
    mCurrentFramebufferDesc.updateFragmentShadingRate(serial);
    // mRenderPassDesc will be updated later in updateRenderPassDesc() in case if
    // mCurrentFramebufferDesc was changed.
    return angle::Result::Continue;
}

angle::Result FramebufferVk::resolveColorWithSubpass(ContextVk *contextVk,
                                                     const UtilsVk::BlitResolveParameters &params)
{
    // Vulkan requires a 1:1 relationship for multisample attachments to resolve attachments in the
    // render pass subpass. Due to this, we currently only support using resolve attachments when
    // there is a single draw attachment enabled.
    ASSERT(mState.getEnabledDrawBuffers().count() == 1);
    uint32_t drawColorIndexGL = static_cast<uint32_t>(*mState.getEnabledDrawBuffers().begin());
    RenderTargetVk *drawRenderTarget      = mRenderTargetCache.getColors()[drawColorIndexGL];
    const vk::ImageView *resolveImageView = nullptr;
    ANGLE_TRY(drawRenderTarget->getImageView(contextVk, &resolveImageView));

    const gl::Framebuffer *srcFramebuffer = contextVk->getState().getReadFramebuffer();
    uint32_t readColorIndexGL             = srcFramebuffer->getState().getReadIndex();

    vk::RenderPassCommandBufferHelper &renderPassCommands =
        contextVk->getStartedRenderPassCommands();
    ASSERT(!renderPassCommands.getRenderPassDesc().hasColorResolveAttachment(readColorIndexGL));

    drawRenderTarget->onColorResolve(contextVk, mCurrentFramebufferDesc.getLayerCount(),
                                     readColorIndexGL, *resolveImageView);

    // The render pass is already closed because of the change in the draw buffer.  Just don't let
    // it reactivate now that it has a resolve attachment.
    contextVk->disableRenderPassReactivation();

    return angle::Result::Continue;
}

angle::Result FramebufferVk::resolveDepthStencilWithSubpass(
    ContextVk *contextVk,
    const UtilsVk::BlitResolveParameters &params,
    VkImageAspectFlags aspects)
{
    RenderTargetVk *drawRenderTarget      = mRenderTargetCache.getDepthStencil();
    const vk::ImageView *resolveImageView = nullptr;
    ANGLE_TRY(drawRenderTarget->getImageView(contextVk, &resolveImageView));

    vk::RenderPassCommandBufferHelper &renderPassCommands =
        contextVk->getStartedRenderPassCommands();
    ASSERT(!renderPassCommands.getRenderPassDesc().hasDepthStencilResolveAttachment());

    drawRenderTarget->onDepthStencilResolve(contextVk, mCurrentFramebufferDesc.getLayerCount(),
                                            aspects, *resolveImageView);

    // The render pass is already closed because of the change in the draw buffer.  Just don't let
    // it reactivate now that it has a resolve attachment.
    contextVk->disableRenderPassReactivation();

    return angle::Result::Continue;
}

angle::Result FramebufferVk::resolveColorWithCommand(ContextVk *contextVk,
                                                     const UtilsVk::BlitResolveParameters &params,
                                                     vk::ImageHelper *srcImage)
{
    vk::CommandBufferAccess access;
    access.onImageTransferRead(VK_IMAGE_ASPECT_COLOR_BIT, srcImage);

    for (size_t colorIndexGL : mState.getEnabledDrawBuffers())
    {
        RenderTargetVk *drawRenderTarget = mRenderTargetCache.getColors()[colorIndexGL];
        vk::ImageHelper &dstImage        = drawRenderTarget->getImageForWrite();

        access.onImageTransferWrite(drawRenderTarget->getLevelIndex(), 1,
                                    drawRenderTarget->getLayerIndex(), 1, VK_IMAGE_ASPECT_COLOR_BIT,
                                    &dstImage);
    }

    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    VkImageResolve resolveRegion                = {};
    resolveRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.srcSubresource.mipLevel       = 0;
    resolveRegion.srcSubresource.baseArrayLayer = params.srcLayer;
    resolveRegion.srcSubresource.layerCount     = 1;
    resolveRegion.srcOffset.x                   = params.blitArea.x;
    resolveRegion.srcOffset.y                   = params.blitArea.y;
    resolveRegion.srcOffset.z                   = 0;
    resolveRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    resolveRegion.dstSubresource.layerCount     = 1;
    resolveRegion.dstOffset.x                   = params.blitArea.x;
    resolveRegion.dstOffset.y                   = params.blitArea.y;
    resolveRegion.dstOffset.z                   = 0;
    resolveRegion.extent.width                  = params.blitArea.width;
    resolveRegion.extent.height                 = params.blitArea.height;
    resolveRegion.extent.depth                  = 1;

    angle::VulkanPerfCounters &perfCounters = contextVk->getPerfCounters();
    for (size_t colorIndexGL : mState.getEnabledDrawBuffers())
    {
        RenderTargetVk *drawRenderTarget = mRenderTargetCache.getColors()[colorIndexGL];
        vk::ImageHelper &dstImage        = drawRenderTarget->getImageForWrite();

        vk::LevelIndex levelVk = dstImage.toVkLevel(drawRenderTarget->getLevelIndex());
        resolveRegion.dstSubresource.mipLevel       = levelVk.get();
        resolveRegion.dstSubresource.baseArrayLayer = drawRenderTarget->getLayerIndex();

        srcImage->resolve(&dstImage, resolveRegion, commandBuffer);

        perfCounters.resolveImageCommands++;
    }

    return angle::Result::Continue;
}

gl::FramebufferStatus FramebufferVk::checkStatus(const gl::Context *context) const
{
    // if we have both a depth and stencil buffer, they must refer to the same object
    // since we only support packed_depth_stencil and not separate depth and stencil
    if (mState.hasSeparateDepthAndStencilAttachments())
    {
        return gl::FramebufferStatus::Incomplete(
            GL_FRAMEBUFFER_UNSUPPORTED,
            gl::err::kFramebufferIncompleteUnsupportedSeparateDepthStencilBuffers);
    }

    return gl::FramebufferStatus::Complete();
}

angle::Result FramebufferVk::invalidateImpl(ContextVk *contextVk,
                                            size_t count,
                                            const GLenum *attachments,
                                            bool isSubInvalidate,
                                            const gl::Rectangle &invalidateArea)
{
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

    // Shouldn't try to issue deferred clears if invalidating sub framebuffer.
    ASSERT(mDeferredClears.empty() || !isSubInvalidate);

    // Remove deferred clears for the invalidated attachments.
    if (invalidateDepthBuffer)
    {
        mDeferredClears.reset(vk::kUnpackedDepthIndex);
    }
    if (invalidateStencilBuffer)
    {
        mDeferredClears.reset(vk::kUnpackedStencilIndex);
    }
    for (size_t colorIndexGL : mState.getEnabledDrawBuffers())
    {
        if (invalidateColorBuffers.test(colorIndexGL))
        {
            mDeferredClears.reset(colorIndexGL);
        }
    }

    // If there are still deferred clears, restage them. See relevant comment in invalidateSub.
    restageDeferredClears(contextVk);

    const auto &colorRenderTargets           = mRenderTargetCache.getColors();
    RenderTargetVk *depthStencilRenderTarget = mRenderTargetCache.getDepthStencil();

    // If not a partial invalidate, mark the contents of the invalidated attachments as undefined,
    // so their loadOp can be set to DONT_CARE in the following render pass.
    if (!isSubInvalidate)
    {
        for (size_t colorIndexGL : mState.getEnabledDrawBuffers())
        {
            if (invalidateColorBuffers.test(colorIndexGL))
            {
                RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
                ASSERT(colorRenderTarget);

                bool preferToKeepContentsDefined = false;
                colorRenderTarget->invalidateEntireContent(contextVk, &preferToKeepContentsDefined);
                if (preferToKeepContentsDefined)
                {
                    invalidateColorBuffers.reset(colorIndexGL);
                }
            }
        }

        // If we have a depth / stencil render target, invalidate its aspects.
        if (depthStencilRenderTarget)
        {
            if (invalidateDepthBuffer)
            {
                bool preferToKeepContentsDefined = false;
                depthStencilRenderTarget->invalidateEntireContent(contextVk,
                                                                  &preferToKeepContentsDefined);
                if (preferToKeepContentsDefined)
                {
                    invalidateDepthBuffer = false;
                }
            }
            if (invalidateStencilBuffer)
            {
                bool preferToKeepContentsDefined = false;
                depthStencilRenderTarget->invalidateEntireStencilContent(
                    contextVk, &preferToKeepContentsDefined);
                if (preferToKeepContentsDefined)
                {
                    invalidateStencilBuffer = false;
                }
            }
        }
    }

    // To ensure we invalidate the right renderpass we require that the current framebuffer be the
    // same as the current renderpass' framebuffer. E.g. prevent sequence like:
    //- Bind FBO 1, draw
    //- Bind FBO 2, draw
    //- Bind FBO 1, invalidate D/S
    // to invalidate the D/S of FBO 2 since it would be the currently active renderpass.
    if (contextVk->hasStartedRenderPassWithQueueSerial(mLastRenderPassQueueSerial))
    {
        bool closeRenderPass = false;

        // Mark the invalidated attachments in the render pass for loadOp and storeOp determination
        // at its end.
        vk::PackedAttachmentIndex colorIndexVk(0);
        for (size_t colorIndexGL : mState.getColorAttachmentsMask())
        {
            if (mState.getEnabledDrawBuffers()[colorIndexGL] &&
                invalidateColorBuffers.test(colorIndexGL))
            {
                contextVk->getStartedRenderPassCommands().invalidateRenderPassColorAttachment(
                    contextVk->getState(), colorIndexGL, colorIndexVk, invalidateArea);

                // If invalidating a color image with emulated channels, a clear is automatically
                // staged so the emulated channels don't contain invalid data later.  This is
                // problematic with deferred clears; the clear marks the framebuffer attachment as
                // dirty, and the next command causes |FramebufferVk::syncState| to pick the clear
                // up as a deferred clear.
                //
                // This is normally correct, except if the following command is another draw call;
                // in that case, the render pass does not close, yet the clear is cached in
                // |mDeferredClears|.  When the render pass later closes, it undoes the invalidate
                // and attempts to remove the clear from the image... but it does not exist there
                // anymore (it's in |mDeferredClears|).  Next usage of the image then clears it,
                // undoing the draws after invalidate.
                //
                // In this case, the simplest approach is to close the render pass right away here.
                // Note that it is not possible to make |FramebufferVk::syncState| avoid picking up
                // the clear in |mDeferredClears|, not apply the clear, _and_ keep the render pass
                // open; because future uses of the image (like with |glReadPixels|) will not
                // trigger |FramebufferVk::syncState| and the clear won't be done.
                if (mEmulatedAlphaAttachmentMask[colorIndexGL])
                {
                    closeRenderPass = true;
                }
            }
            ++colorIndexVk;
        }

        if (depthStencilRenderTarget)
        {
            const gl::DepthStencilState &dsState = contextVk->getState().getDepthStencilState();
            if (invalidateDepthBuffer)
            {
                contextVk->getStartedRenderPassCommands().invalidateRenderPassDepthAttachment(
                    dsState, invalidateArea);
            }

            if (invalidateStencilBuffer)
            {
                contextVk->getStartedRenderPassCommands().invalidateRenderPassStencilAttachment(
                    dsState, mState.getStencilBitCount(), invalidateArea);
            }
        }

        if (closeRenderPass)
        {
            ANGLE_TRY(contextVk->flushCommandsAndEndRenderPass(
                RenderPassClosureReason::ColorBufferWithEmulatedAlphaInvalidate));
        }
    }

    return angle::Result::Continue;
}

angle::Result FramebufferVk::updateColorAttachment(const gl::Context *context,
                                                   uint32_t colorIndexGL)
{
    ANGLE_TRY(mRenderTargetCache.updateColorRenderTarget(context, mState, colorIndexGL));

    // Update cached masks for masked clears.
    RenderTargetVk *renderTarget = mRenderTargetCache.getColors()[colorIndexGL];
    if (renderTarget)
    {
        const angle::Format &actualFormat = renderTarget->getImageActualFormat();
        updateActiveColorMasks(colorIndexGL, actualFormat.redBits > 0, actualFormat.greenBits > 0,
                               actualFormat.blueBits > 0, actualFormat.alphaBits > 0);

        const angle::Format &intendedFormat = renderTarget->getImageIntendedFormat();
        mEmulatedAlphaAttachmentMask.set(
            colorIndexGL, intendedFormat.alphaBits == 0 && actualFormat.alphaBits > 0);
    }
    else
    {
        updateActiveColorMasks(colorIndexGL, false, false, false, false);
    }

    const bool enabledColor =
        renderTarget && mState.getColorAttachments()[colorIndexGL].isAttached();
    const bool enabledResolve = enabledColor && renderTarget->hasResolveAttachment();

    if (enabledColor)
    {
        mCurrentFramebufferDesc.updateColor(colorIndexGL, renderTarget->getDrawSubresourceSerial());
        const bool isExternalImage =
            mState.getColorAttachments()[colorIndexGL].isExternalImageWithoutIndividualSync();
        mIsExternalColorAttachments.set(colorIndexGL, isExternalImage);
        mAttachmentHasFrontBufferUsage.set(
            colorIndexGL, mState.getColorAttachments()[colorIndexGL].hasFrontBufferUsage());
    }
    else
    {
        mCurrentFramebufferDesc.updateColor(colorIndexGL,
                                            vk::kInvalidImageOrBufferViewSubresourceSerial);
    }

    if (enabledResolve)
    {
        mCurrentFramebufferDesc.updateColorResolve(colorIndexGL,
                                                   renderTarget->getResolveSubresourceSerial());
    }
    else
    {
        mCurrentFramebufferDesc.updateColorResolve(colorIndexGL,
                                                   vk::kInvalidImageOrBufferViewSubresourceSerial);
    }

    return angle::Result::Continue;
}

void FramebufferVk::updateColorAttachmentColorspace(gl::SrgbWriteControlMode srgbWriteControlMode)
{
    // Update colorspace of color attachments.
    const auto &colorRenderTargets               = mRenderTargetCache.getColors();
    const gl::DrawBufferMask colorAttachmentMask = mState.getColorAttachmentsMask();
    for (size_t colorIndexGL : colorAttachmentMask)
    {
        RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
        ASSERT(colorRenderTarget);
        colorRenderTarget->updateWriteColorspace(srgbWriteControlMode);
    }
}

angle::Result FramebufferVk::updateDepthStencilAttachment(const gl::Context *context)
{
    ANGLE_TRY(mRenderTargetCache.updateDepthStencilRenderTarget(context, mState));

    ContextVk *contextVk = vk::GetImpl(context);
    updateDepthStencilAttachmentSerial(contextVk);

    return angle::Result::Continue;
}

void FramebufferVk::updateDepthStencilAttachmentSerial(ContextVk *contextVk)
{
    RenderTargetVk *depthStencilRT = getDepthStencilRenderTarget();

    if (depthStencilRT != nullptr)
    {
        mCurrentFramebufferDesc.updateDepthStencil(depthStencilRT->getDrawSubresourceSerial());
    }
    else
    {
        mCurrentFramebufferDesc.updateDepthStencil(vk::kInvalidImageOrBufferViewSubresourceSerial);
    }

    if (depthStencilRT != nullptr && depthStencilRT->hasResolveAttachment())
    {
        mCurrentFramebufferDesc.updateDepthStencilResolve(
            depthStencilRT->getResolveSubresourceSerial());
    }
    else
    {
        mCurrentFramebufferDesc.updateDepthStencilResolve(
            vk::kInvalidImageOrBufferViewSubresourceSerial);
    }
}

angle::Result FramebufferVk::flushColorAttachmentUpdates(const gl::Context *context,
                                                         bool deferClears,
                                                         uint32_t colorIndexGL)
{
    ContextVk *contextVk             = vk::GetImpl(context);
    RenderTargetVk *readRenderTarget = nullptr;
    RenderTargetVk *drawRenderTarget = nullptr;

    // It's possible for the read and draw color attachments to be different if different surfaces
    // are bound, so we need to flush any staged updates to both.

    // Draw
    drawRenderTarget = mRenderTargetCache.getColorDraw(mState, colorIndexGL);
    if (drawRenderTarget)
    {
        if (deferClears)
        {
            ANGLE_TRY(
                drawRenderTarget->flushStagedUpdates(contextVk, &mDeferredClears, colorIndexGL,
                                                     mCurrentFramebufferDesc.getLayerCount()));
        }
        else
        {
            ANGLE_TRY(drawRenderTarget->flushStagedUpdates(
                contextVk, nullptr, 0, mCurrentFramebufferDesc.getLayerCount()));
        }
    }

    // Read
    if (mState.getReadBufferState() != GL_NONE && mState.getReadIndex() == colorIndexGL)
    {
        // Flush staged updates to the read render target as well, but only if it's not the same as
        // the draw render target.  This can happen when the read render target is bound to another
        // surface.
        readRenderTarget = mRenderTargetCache.getColorRead(mState);
        if (readRenderTarget && readRenderTarget != drawRenderTarget)
        {
            ANGLE_TRY(readRenderTarget->flushStagedUpdates(
                contextVk, nullptr, 0, mCurrentFramebufferDesc.getLayerCount()));
        }
    }

    return angle::Result::Continue;
}

angle::Result FramebufferVk::flushDepthStencilAttachmentUpdates(const gl::Context *context,
                                                                bool deferClears)
{
    ContextVk *contextVk = vk::GetImpl(context);

    RenderTargetVk *depthStencilRT = getDepthStencilRenderTarget();
    if (depthStencilRT == nullptr)
    {
        return angle::Result::Continue;
    }

    if (deferClears)
    {
        return depthStencilRT->flushStagedUpdates(contextVk, &mDeferredClears,
                                                  vk::kUnpackedDepthIndex,
                                                  mCurrentFramebufferDesc.getLayerCount());
    }

    return depthStencilRT->flushStagedUpdates(contextVk, nullptr, 0,
                                              mCurrentFramebufferDesc.getLayerCount());
}

angle::Result FramebufferVk::syncState(const gl::Context *context,
                                       GLenum binding,
                                       const gl::Framebuffer::DirtyBits &dirtyBits,
                                       gl::Command command)
{
    ContextVk *contextVk = vk::GetImpl(context);

    vk::FramebufferDesc priorFramebufferDesc = mCurrentFramebufferDesc;

    // Keep track of which attachments have dirty content and need their staged updates flushed.
    // The respective functions depend on |mCurrentFramebufferDesc::mLayerCount| which is updated
    // after all attachment render targets are updated.
    gl::DrawBufferMask dirtyColorAttachments;
    bool dirtyDepthStencilAttachment = false;

    bool shouldUpdateColorMaskAndBlend = false;
    bool shouldUpdateLayerCount        = false;

    // Cache new foveation state, if any
    const gl::FoveationState *newFoveationState = nullptr;
    gl::Extents foveatedAttachmentSize;

    // For any updated attachments we'll update their Serials below
    ASSERT(dirtyBits.any());
    for (size_t dirtyBit : dirtyBits)
    {
        switch (dirtyBit)
        {
            case gl::Framebuffer::DIRTY_BIT_DEPTH_ATTACHMENT:
            case gl::Framebuffer::DIRTY_BIT_DEPTH_BUFFER_CONTENTS:
            case gl::Framebuffer::DIRTY_BIT_STENCIL_ATTACHMENT:
            case gl::Framebuffer::DIRTY_BIT_STENCIL_BUFFER_CONTENTS:
                ANGLE_TRY(updateDepthStencilAttachment(context));
                shouldUpdateLayerCount      = true;
                dirtyDepthStencilAttachment = true;
                break;
            case gl::Framebuffer::DIRTY_BIT_READ_BUFFER:
                ANGLE_TRY(mRenderTargetCache.update(context, mState, dirtyBits));
                break;
            case gl::Framebuffer::DIRTY_BIT_DRAW_BUFFERS:
                shouldUpdateColorMaskAndBlend = true;
                shouldUpdateLayerCount        = true;
                break;
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_WIDTH:
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_HEIGHT:
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_SAMPLES:
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_FIXED_SAMPLE_LOCATIONS:
                // Invalidate the cache. If we have performance critical code hitting this path we
                // can add related data (such as width/height) to the cache
                releaseCurrentFramebuffer(contextVk);
                break;
            case gl::Framebuffer::DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE:
                break;
            case gl::Framebuffer::DIRTY_BIT_DEFAULT_LAYERS:
                shouldUpdateLayerCount = true;
                break;
            case gl::Framebuffer::DIRTY_BIT_FOVEATION:
                // This dirty bit is set iff the framebuffer itself is foveated
                ASSERT(mState.isFoveationEnabled());

                newFoveationState      = &mState.getFoveationState();
                foveatedAttachmentSize = mState.getExtents();
                break;
            default:
            {
                static_assert(gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_0 == 0, "FB dirty bits");
                uint32_t colorIndexGL;
                if (dirtyBit < gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_MAX)
                {
                    colorIndexGL = static_cast<uint32_t>(
                        dirtyBit - gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_0);
                }
                else
                {
                    ASSERT(dirtyBit >= gl::Framebuffer::DIRTY_BIT_COLOR_BUFFER_CONTENTS_0 &&
                           dirtyBit < gl::Framebuffer::DIRTY_BIT_COLOR_BUFFER_CONTENTS_MAX);
                    colorIndexGL = static_cast<uint32_t>(
                        dirtyBit - gl::Framebuffer::DIRTY_BIT_COLOR_BUFFER_CONTENTS_0);
                }

                ANGLE_TRY(updateColorAttachment(context, colorIndexGL));

                // Check if attachment has foveated rendering, if so grab foveation state
                const gl::FramebufferAttachment *attachment =
                    mState.getColorAttachment(colorIndexGL);
                if (attachment && attachment->hasFoveatedRendering())
                {
                    // If attachment is foveated the framebuffer must not be.
                    ASSERT(!mState.isFoveationEnabled());

                    newFoveationState = attachment->getFoveationState();
                    ASSERT(newFoveationState != nullptr);

                    foveatedAttachmentSize = attachment->getSize();
                }

                // Window system framebuffer only have one color attachment and its property should
                // never change unless via DIRTY_BIT_DRAW_BUFFERS bit.
                if (!mState.isDefault())
                {
                    shouldUpdateColorMaskAndBlend = true;
                    shouldUpdateLayerCount        = true;
                }
                dirtyColorAttachments.set(colorIndexGL);

                break;
            }
        }
    }

    // A shared attachment's colospace could have been modified in another context, update
    // colorspace of all attachments to reflect current context's colorspace.
    gl::SrgbWriteControlMode srgbWriteControlMode = mState.getWriteControlMode();
    updateColorAttachmentColorspace(srgbWriteControlMode);
    // Update current framebuffer descriptor to reflect the new state.
    mCurrentFramebufferDesc.setWriteControlMode(srgbWriteControlMode);

    if (shouldUpdateColorMaskAndBlend)
    {
        contextVk->updateColorMasks();
        contextVk->updateBlendFuncsAndEquations();
    }

    if (shouldUpdateLayerCount)
    {
        updateLayerCount();
    }

    if (newFoveationState && mFoveationState != *newFoveationState)
    {
        ANGLE_TRY(updateFoveationState(contextVk, *newFoveationState, foveatedAttachmentSize));
    }

    // Defer clears for draw framebuffer ops.  Note that this will result in a render area that
    // completely covers the framebuffer, even if the operation that follows is scissored.
    //
    // Additionally, defer clears for read framebuffer attachments that are not taking part in a
    // blit operation.
    const bool isBlitCommand = command >= gl::Command::Blit && command <= gl::Command::BlitAll;

    bool deferColorClears        = binding == GL_DRAW_FRAMEBUFFER;
    bool deferDepthStencilClears = binding == GL_DRAW_FRAMEBUFFER;
    if (binding == GL_READ_FRAMEBUFFER && isBlitCommand)
    {
        uint32_t blitMask =
            static_cast<uint32_t>(command) - static_cast<uint32_t>(gl::Command::Blit);
        if ((blitMask & gl::CommandBlitBufferColor) == 0)
        {
            deferColorClears = true;
        }
        if ((blitMask & (gl::CommandBlitBufferDepth | gl::CommandBlitBufferStencil)) == 0)
        {
            deferDepthStencilClears = true;
        }
    }

    // If we are notified that any attachment is dirty, but we have deferred clears for them, a
    // flushDeferredClears() call is missing somewhere.  ASSERT this to catch these bugs.
    vk::ClearValuesArray previousDeferredClears = mDeferredClears;

    for (size_t colorIndexGL : dirtyColorAttachments)
    {
        ASSERT(!previousDeferredClears.test(colorIndexGL));
        ANGLE_TRY(flushColorAttachmentUpdates(context, deferColorClears,
                                              static_cast<uint32_t>(colorIndexGL)));
    }
    if (dirtyDepthStencilAttachment)
    {
        ASSERT(!previousDeferredClears.testDepth());
        ASSERT(!previousDeferredClears.testStencil());
        ANGLE_TRY(flushDepthStencilAttachmentUpdates(context, deferDepthStencilClears));
    }

    // No-op redundant changes to prevent closing the RenderPass.
    if (mCurrentFramebufferDesc == priorFramebufferDesc &&
        mCurrentFramebufferDesc.attachmentCount() > 0)
    {
        return angle::Result::Continue;
    }

    // ContextVk::onFramebufferChange will end up calling onRenderPassFinished if necessary,
    // which will trigger ending of current render pass.  |mLastRenderPassQueueSerial| is reset
    // so that the render pass will not get reactivated, since |mCurrentFramebufferDesc| has
    // changed.
    mLastRenderPassQueueSerial = QueueSerial();

    updateRenderPassDesc(contextVk);

    // Deactivate Framebuffer
    releaseCurrentFramebuffer(contextVk);

    // Notify the ContextVk to update the pipeline desc.
    return contextVk->onFramebufferChange(this, command);
}

void FramebufferVk::updateRenderPassDesc(ContextVk *contextVk)
{
    mRenderPassDesc = {};
    mRenderPassDesc.setSamples(getSamples());
    mRenderPassDesc.setViewCount(
        mState.isMultiview() && mState.getNumViews() > 1 ? mState.getNumViews() : 0);

    // Color attachments.
    const auto &colorRenderTargets               = mRenderTargetCache.getColors();
    const gl::DrawBufferMask colorAttachmentMask = mState.getColorAttachmentsMask();
    for (size_t colorIndexGL = 0; colorIndexGL < colorAttachmentMask.size(); ++colorIndexGL)
    {
        if (colorAttachmentMask[colorIndexGL])
        {
            RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
            ASSERT(colorRenderTarget);

            if (colorRenderTarget->isYuvResolve())
            {
                // If this is YUV resolve target, we use resolveImage's format since image maybe
                // nullptr
                auto const &resolveImage = colorRenderTarget->getResolveImageForRenderPass();
                mRenderPassDesc.packColorAttachment(colorIndexGL, resolveImage.getActualFormatID());
                mRenderPassDesc.packYUVResolveAttachment(colorIndexGL);
            }
            else
            {
                // Account for attachments with colorspace override
                angle::FormatID actualFormat =
                    colorRenderTarget->getImageForRenderPass().getActualFormatID();
                if (colorRenderTarget->hasColorspaceOverrideForWrite())
                {
                    actualFormat =
                        colorRenderTarget->getColorspaceOverrideFormatForWrite(actualFormat);
                }

                mRenderPassDesc.packColorAttachment(colorIndexGL, actualFormat);
                // Add the resolve attachment, if any.
                if (colorRenderTarget->hasResolveAttachment())
                {
                    mRenderPassDesc.packColorResolveAttachment(colorIndexGL);
                }
            }
        }
        else
        {
            mRenderPassDesc.packColorAttachmentGap(colorIndexGL);
        }
    }

    // Depth/stencil attachment.
    RenderTargetVk *depthStencilRenderTarget = getDepthStencilRenderTarget();
    if (depthStencilRenderTarget)
    {
        mRenderPassDesc.packDepthStencilAttachment(
            depthStencilRenderTarget->getImageForRenderPass().getActualFormatID());

        // Add the resolve attachment, if any.
        if (depthStencilRenderTarget->hasResolveAttachment())
        {
            mRenderPassDesc.packDepthResolveAttachment();
            mRenderPassDesc.packStencilResolveAttachment();
        }
    }

    if (!contextVk->getFeatures().preferDynamicRendering.enabled &&
        contextVk->isInColorFramebufferFetchMode())
    {
        mRenderPassDesc.setFramebufferFetchMode(vk::FramebufferFetchMode::Color);
    }

    if (contextVk->getFeatures().enableMultisampledRenderToTexture.enabled)
    {
        // Update descriptions regarding multisampled-render-to-texture use.
        bool isRenderToTexture = false;
        for (size_t colorIndexGL : mState.getEnabledDrawBuffers())
        {
            const gl::FramebufferAttachment *color = mState.getColorAttachment(colorIndexGL);
            ASSERT(color);

            if (color->isRenderToTexture())
            {
                isRenderToTexture = true;
                break;
            }
        }
        const gl::FramebufferAttachment *depthStencil = mState.getDepthStencilAttachment();
        if (depthStencil && depthStencil->isRenderToTexture())
        {
            isRenderToTexture = true;
        }

        mCurrentFramebufferDesc.updateRenderToTexture(isRenderToTexture);
        mRenderPassDesc.updateRenderToTexture(isRenderToTexture);
    }

    mCurrentFramebufferDesc.updateUnresolveMask({});
    mRenderPassDesc.setWriteControlMode(mCurrentFramebufferDesc.getWriteControlMode());
    mRenderPassDesc.setFragmentShadingAttachment(
        mCurrentFramebufferDesc.hasFragmentShadingRateAttachment());

    updateLegacyDither(contextVk);
}

angle::Result FramebufferVk::getAttachmentsAndRenderTargets(
    vk::ErrorContext *context,
    vk::FramebufferAttachmentsVector<VkImageView> *unpackedAttachments,
    vk::FramebufferAttachmentsVector<RenderTargetInfo> *packedRenderTargetsInfoOut)
{
    // Color attachments.
    mIsYUVResolve                  = false;
    const auto &colorRenderTargets = mRenderTargetCache.getColors();
    for (size_t colorIndexGL : mState.getColorAttachmentsMask())
    {
        RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
        ASSERT(colorRenderTarget);

        if (colorRenderTarget->isYuvResolve())
        {
            mIsYUVResolve = true;
            if (context->getRenderer()->nullColorAttachmentWithExternalFormatResolve())
            {
                continue;
            }
        }
        const vk::ImageView *imageView = nullptr;
        ANGLE_TRY(colorRenderTarget->getImageViewWithColorspace(
            context, mCurrentFramebufferDesc.getWriteControlMode(), &imageView));
        unpackedAttachments->push_back(imageView->getHandle());

        packedRenderTargetsInfoOut->emplace_back(
            RenderTargetInfo(colorRenderTarget, RenderTargetImage::Attachment));
    }

    // Depth/stencil attachment.
    RenderTargetVk *depthStencilRenderTarget = getDepthStencilRenderTarget();
    if (depthStencilRenderTarget)
    {
        const vk::ImageView *imageView = nullptr;
        ANGLE_TRY(depthStencilRenderTarget->getImageView(context, &imageView));

        unpackedAttachments->push_back(imageView->getHandle());
        packedRenderTargetsInfoOut->emplace_back(
            RenderTargetInfo(depthStencilRenderTarget, RenderTargetImage::Attachment));
    }

    // Fragment shading rate attachment.
    if (mCurrentFramebufferDesc.hasFragmentShadingRateAttachment())
    {
        const vk::ImageViewHelper *imageViewHelper = &mFragmentShadingRateImageView;
        unpackedAttachments->push_back(
            imageViewHelper->getFragmentShadingRateImageView().getHandle());
        packedRenderTargetsInfoOut->emplace_back(nullptr, RenderTargetImage::FragmentShadingRate);
    }

    // Color resolve attachments.  From here on, the views are placed at sparse indices because of
    // |RenderPassFramebuffer|.  That allows more resolve attachments to be added later.
    unpackedAttachments->resize(vk::kMaxFramebufferAttachments, VK_NULL_HANDLE);
    static_assert(vk::RenderPassFramebuffer::kColorResolveAttachmentBegin <
                  vk::kMaxFramebufferAttachments);
    static_assert(vk::RenderPassFramebuffer::kDepthStencilResolveAttachment <
                  vk::kMaxFramebufferAttachments);

    bool anyResolveAttachments = false;

    for (size_t colorIndexGL : mState.getColorAttachmentsMask())
    {
        RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
        ASSERT(colorRenderTarget);

        if (colorRenderTarget->hasResolveAttachment())
        {
            const vk::ImageView *resolveImageView = nullptr;
            ANGLE_TRY(colorRenderTarget->getResolveImageView(context, &resolveImageView));

            constexpr size_t kBaseIndex = vk::RenderPassFramebuffer::kColorResolveAttachmentBegin;
            (*unpackedAttachments)[kBaseIndex + colorIndexGL] = resolveImageView->getHandle();
            packedRenderTargetsInfoOut->emplace_back(
                RenderTargetInfo(colorRenderTarget, RenderTargetImage::Resolve));

            anyResolveAttachments = true;
        }
    }

    // Depth/stencil resolve attachment.
    if (depthStencilRenderTarget && depthStencilRenderTarget->hasResolveAttachment())
    {
        const vk::ImageView *imageView = nullptr;
        ANGLE_TRY(depthStencilRenderTarget->getResolveImageView(context, &imageView));

        (*unpackedAttachments)[vk::RenderPassFramebuffer::kDepthStencilResolveAttachment] =
            imageView->getHandle();
        packedRenderTargetsInfoOut->emplace_back(
            RenderTargetInfo(depthStencilRenderTarget, RenderTargetImage::Resolve));

        anyResolveAttachments = true;
    }

    // Make sure |AllowAddingResolveAttachmentsToSubpass()| is guarding against all cases where a
    // resolve attachment is pre-present in the render pass.
    if (anyResolveAttachments)
    {
        ASSERT(!AllowAddingResolveAttachmentsToSubpass(mRenderPassDesc));
    }

    return angle::Result::Continue;
}

angle::Result FramebufferVk::createNewFramebuffer(
    ContextVk *contextVk,
    uint32_t framebufferWidth,
    const uint32_t framebufferHeight,
    const uint32_t framebufferLayers,
    const vk::FramebufferAttachmentsVector<VkImageView> &unpackedAttachments,
    const vk::FramebufferAttachmentsVector<RenderTargetInfo> &renderTargetsInfo)
{
    ASSERT(!contextVk->getFeatures().preferDynamicRendering.enabled);

    // The backbuffer framebuffer is cached in WindowSurfaceVk instead.
    ASSERT(mBackbuffer == nullptr);
    // Called only when a new framebuffer is needed.
    ASSERT(!mCurrentFramebuffer.valid());

    // When using imageless framebuffers, the framebuffer cache is not utilized.
    const bool useImagelessFramebuffer =
        contextVk->getFeatures().supportsImagelessFramebuffer.enabled;

    // Try to retrieve a framebuffer from the cache.
    if (!useImagelessFramebuffer && contextVk->getShareGroup()->getFramebufferCache().get(
                                        contextVk, mCurrentFramebufferDesc, mCurrentFramebuffer))
    {
        ASSERT(mCurrentFramebuffer.valid());
        mIsCurrentFramebufferCached = true;
        return angle::Result::Continue;
    }

    const vk::RenderPass *compatibleRenderPass = nullptr;
    ANGLE_TRY(contextVk->getCompatibleRenderPass(mRenderPassDesc, &compatibleRenderPass));

    // Create a new framebuffer.
    vk::FramebufferHelper newFramebuffer;

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.flags                   = 0;
    framebufferInfo.renderPass              = compatibleRenderPass->getHandle();
    framebufferInfo.attachmentCount         = static_cast<uint32_t>(renderTargetsInfo.size());
    framebufferInfo.width                   = framebufferWidth;
    framebufferInfo.height                  = framebufferHeight;
    framebufferInfo.layers                  = framebufferLayers;

    // Check that our description matches our attachments. Can catch implementation bugs.
    ASSERT((mIsYUVResolve &&
            contextVk->getRenderer()->nullColorAttachmentWithExternalFormatResolve()) ||
           static_cast<uint32_t>(renderTargetsInfo.size()) ==
               mCurrentFramebufferDesc.attachmentCount());

    if (!useImagelessFramebuffer)
    {
        vk::FramebufferAttachmentsVector<VkImageView> packedAttachments = unpackedAttachments;
        vk::RenderPassFramebuffer::PackViews(&packedAttachments);

        ASSERT(renderTargetsInfo.size() == packedAttachments.size());
        framebufferInfo.pAttachments = packedAttachments.data();

        // The cache key (|FramebufferDesc|) can't distinguish between two framebuffers with 0
        // attachments but with different sizes.  For simplicity, 0-attachment framebuffers are not
        // cached.
        ANGLE_TRY(newFramebuffer.init(contextVk, framebufferInfo));
        if (packedAttachments.empty())
        {
            mCurrentFramebuffer         = std::move(newFramebuffer.getFramebuffer());
            mIsCurrentFramebufferCached = false;
        }
        else
        {
            insertCache(contextVk, mCurrentFramebufferDesc, std::move(newFramebuffer));

            const bool result = contextVk->getShareGroup()->getFramebufferCache().get(
                contextVk, mCurrentFramebufferDesc, mCurrentFramebuffer);
            ASSERT(result);
            mIsCurrentFramebufferCached = true;
        }

        return angle::Result::Continue;
    }

    // For imageless framebuffers, attachment image and create info objects should be defined
    // when creating the new framebuffer.
    vk::FramebufferAttachmentsVector<VkFramebufferAttachmentImageInfo> attachmentImageInfos(
        renderTargetsInfo.size(), {});

    for (size_t index = 0; index < renderTargetsInfo.size(); ++index)
    {
        const RenderTargetInfo &info                     = renderTargetsInfo[index];
        VkFramebufferAttachmentImageInfo &attachmentInfo = attachmentImageInfos[index];

        attachmentInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;

        // The fragment shading rate attachment does not have a corresponding render target, and is
        // handled specially.
        if (info.renderTargetImage == RenderTargetImage::FragmentShadingRate)
        {
            attachmentInfo.width  = mFragmentShadingRateImage.getExtents().width;
            attachmentInfo.height = mFragmentShadingRateImage.getExtents().height;

            attachmentInfo.layerCount = 1;
            attachmentInfo.flags      = mFragmentShadingRateImage.getCreateFlags();
            attachmentInfo.usage      = mFragmentShadingRateImage.getUsage();
            attachmentInfo.viewFormatCount =
                static_cast<uint32_t>(mFragmentShadingRateImage.getViewFormats().size());
            attachmentInfo.pViewFormats = mFragmentShadingRateImage.getViewFormats().data();
            continue;
        }

        vk::ImageHelper *image = (info.renderTargetImage == RenderTargetImage::Resolve ||
                                  info.renderTarget->isYuvResolve())
                                     ? &info.renderTarget->getResolveImageForRenderPass()
                                     : &info.renderTarget->getImageForRenderPass();

        const gl::LevelIndex level = info.renderTarget->getLevelIndexForImage(*image);
        const uint32_t layerCount  = info.renderTarget->getLayerCount();
        const gl::Extents extents  = image->getLevelExtents2D(image->toVkLevel(level));

        attachmentInfo.width           = std::max(extents.width, 1);
        attachmentInfo.height          = std::max(extents.height, 1);
        attachmentInfo.layerCount      = mCurrentFramebufferDesc.isMultiview()
                                             ? std::max<uint32_t>(mRenderPassDesc.viewCount(), 1u)
                                             : layerCount;
        attachmentInfo.flags           = image->getCreateFlags();
        attachmentInfo.usage           = image->getUsage();
        attachmentInfo.viewFormatCount = static_cast<uint32_t>(image->getViewFormats().size());
        attachmentInfo.pViewFormats    = image->getViewFormats().data();
    }

    VkFramebufferAttachmentsCreateInfo attachmentsCreateInfo = {};
    attachmentsCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
    attachmentsCreateInfo.attachmentImageInfoCount =
        static_cast<uint32_t>(attachmentImageInfos.size());
    attachmentsCreateInfo.pAttachmentImageInfos = attachmentImageInfos.data();

    framebufferInfo.flags |= VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    vk::AddToPNextChain(&framebufferInfo, &attachmentsCreateInfo);

    ANGLE_TRY(newFramebuffer.init(contextVk, framebufferInfo));
    mCurrentFramebuffer = std::move(newFramebuffer.getFramebuffer());

    return angle::Result::Continue;
}

angle::Result FramebufferVk::getFramebuffer(ContextVk *contextVk,
                                            vk::RenderPassFramebuffer *framebufferOut)
{
    ASSERT(!mRenderPassDesc.hasDepthStencilFramebufferFetch());
    ASSERT(mCurrentFramebufferDesc.hasColorFramebufferFetch() ==
           mRenderPassDesc.hasColorFramebufferFetch());

    const gl::Extents attachmentsSize = mState.getExtents();
    ASSERT(attachmentsSize.width != 0 && attachmentsSize.height != 0);

    uint32_t framebufferWidth        = static_cast<uint32_t>(attachmentsSize.width);
    uint32_t framebufferHeight       = static_cast<uint32_t>(attachmentsSize.height);
    const uint32_t framebufferLayers = !mCurrentFramebufferDesc.isMultiview()
                                           ? std::max(mCurrentFramebufferDesc.getLayerCount(), 1u)
                                           : 1;

    vk::FramebufferAttachmentsVector<VkImageView> unpackedAttachments;
    vk::FramebufferAttachmentsVector<RenderTargetInfo> renderTargetsInfo;
    ANGLE_TRY(getAttachmentsAndRenderTargets(contextVk, &unpackedAttachments, &renderTargetsInfo));

    vk::Framebuffer framebufferHandle;
    if (contextVk->getFeatures().preferDynamicRendering.enabled)
    {
        // Nothing to do with dynamic rendering.  The image views and other info are still placed in
        // |framebufferOut| to be passed to |vkCmdBeginRendering| similarly to how they are used
        // with imageless framebuffers with render pass objects.
    }
    else if (mCurrentFramebuffer.valid())
    {
        // If a valid framebuffer is already created, use it.  This is not done when the swapchain
        // is being resolved, because the appropriate framebuffer needs to be queried from the back
        // buffer.
        framebufferHandle.setHandle(mCurrentFramebuffer.getHandle());
    }
    else
    {
        // For the default framebuffer attached to a window surface, WindowSurfaceVk caches a
        // handful of framebuffer objects which are queried here.  For the rest, a framebuffer needs
        // to be created based on the current attachments to the FBO.
        if (mBackbuffer == nullptr)
        {
            // Create a new framebuffer
            ANGLE_TRY(createNewFramebuffer(contextVk, framebufferWidth, framebufferHeight,
                                           framebufferLayers, unpackedAttachments,
                                           renderTargetsInfo));
            ASSERT(mCurrentFramebuffer.valid());
            framebufferHandle.setHandle(mCurrentFramebuffer.getHandle());
        }
        else
        {
            const vk::RenderPass *compatibleRenderPass = nullptr;
            ANGLE_TRY(contextVk->getCompatibleRenderPass(mRenderPassDesc, &compatibleRenderPass));

            // If there is a backbuffer, query the framebuffer from WindowSurfaceVk instead.
            ANGLE_TRY(mBackbuffer->getCurrentFramebuffer(
                contextVk,
                mRenderPassDesc.hasColorFramebufferFetch() ? vk::FramebufferFetchMode::Color
                                                           : vk::FramebufferFetchMode::None,
                *compatibleRenderPass, &framebufferHandle));
        }
    }

    if (mBackbuffer != nullptr)
    {
        // Account for swapchain pre-rotation
        framebufferWidth  = renderTargetsInfo[0].renderTarget->getRotatedExtents().width;
        framebufferHeight = renderTargetsInfo[0].renderTarget->getRotatedExtents().height;
    }

    const vk::ImagelessFramebuffer imagelessFramebuffer =
        contextVk->getFeatures().preferDynamicRendering.enabled ||
                (contextVk->getFeatures().supportsImagelessFramebuffer.enabled &&
                 mBackbuffer == nullptr)
            ? vk::ImagelessFramebuffer::Yes
            : vk::ImagelessFramebuffer::No;
    const vk::RenderPassSource source = mBackbuffer == nullptr
                                            ? vk::RenderPassSource::FramebufferObject
                                            : vk::RenderPassSource::DefaultFramebuffer;

    framebufferOut->setFramebuffer(
        contextVk, std::move(framebufferHandle), std::move(unpackedAttachments), framebufferWidth,
        framebufferHeight, framebufferLayers, imagelessFramebuffer, source);

    return angle::Result::Continue;
}

void FramebufferVk::mergeClearsWithDeferredClears(
    gl::DrawBufferMask clearColorBuffers,
    bool clearDepth,
    bool clearStencil,
    const gl::DrawBuffersArray<VkClearColorValue> &clearColorValues,
    const VkClearDepthStencilValue &clearDepthStencilValue)
{
    // Apply clears to mDeferredClears.  Note that clears override deferred clears.

    // Color clears.
    for (size_t colorIndexGL : clearColorBuffers)
    {
        ASSERT(mState.getEnabledDrawBuffers().test(colorIndexGL));
        VkClearValue clearValue =
            getCorrectedColorClearValue(colorIndexGL, clearColorValues[colorIndexGL]);
        mDeferredClears.store(static_cast<uint32_t>(colorIndexGL), VK_IMAGE_ASPECT_COLOR_BIT,
                              clearValue);
    }

    // Depth and stencil clears.
    VkImageAspectFlags dsAspectFlags = 0;
    VkClearValue dsClearValue        = {};
    dsClearValue.depthStencil        = clearDepthStencilValue;
    if (clearDepth)
    {
        dsAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (clearStencil)
    {
        dsAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (dsAspectFlags != 0)
    {
        mDeferredClears.store(vk::kUnpackedDepthIndex, dsAspectFlags, dsClearValue);
    }
}

angle::Result FramebufferVk::clearWithDraw(
    ContextVk *contextVk,
    const gl::Rectangle &clearArea,
    gl::DrawBufferMask clearColorBuffers,
    bool clearDepth,
    bool clearStencil,
    gl::BlendStateExt::ColorMaskStorage::Type colorMasks,
    uint8_t stencilMask,
    const gl::DrawBuffersArray<VkClearColorValue> &clearColorValues,
    const VkClearDepthStencilValue &clearDepthStencilValue)
{
    // All deferred clears should be handled already.
    ASSERT(mDeferredClears.empty());

    UtilsVk::ClearFramebufferParameters params = {};
    params.clearArea                           = clearArea;
    params.depthStencilClearValue              = clearDepthStencilValue;
    params.stencilMask                         = stencilMask;

    params.clearColor   = true;
    params.clearDepth   = clearDepth;
    params.clearStencil = clearStencil;

    const auto &colorRenderTargets = mRenderTargetCache.getColors();
    for (size_t colorIndexGL : clearColorBuffers)
    {
        const RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
        ASSERT(colorRenderTarget);

        params.colorClearValue = clearColorValues[colorIndexGL];
        params.colorFormat     = &colorRenderTarget->getImageForRenderPass().getActualFormat();
        params.colorAttachmentIndexGL = static_cast<uint32_t>(colorIndexGL);
        params.colorMaskFlags =
            gl::BlendStateExt::ColorMaskStorage::GetValueIndexed(colorIndexGL, colorMasks);
        if (mEmulatedAlphaAttachmentMask[colorIndexGL])
        {
            params.colorMaskFlags &= ~VK_COLOR_COMPONENT_A_BIT;
        }

        // TODO: implement clear of layered framebuffers.  UtilsVk::clearFramebuffer should add a
        // geometry shader that is instanced layerCount times (or loops layerCount times), each time
        // selecting a different layer.
        // http://anglebug.com/42263992
        ASSERT(mCurrentFramebufferDesc.isMultiview() || colorRenderTarget->getLayerCount() == 1);

        ANGLE_TRY(contextVk->getUtils().clearFramebuffer(contextVk, this, params));

        // Clear depth/stencil only once!
        params.clearDepth   = false;
        params.clearStencil = false;
    }

    // If there was no color clear, clear depth/stencil alone.
    if (params.clearDepth || params.clearStencil)
    {
        params.clearColor = false;
        ANGLE_TRY(contextVk->getUtils().clearFramebuffer(contextVk, this, params));
    }

    return angle::Result::Continue;
}

VkClearValue FramebufferVk::getCorrectedColorClearValue(size_t colorIndexGL,
                                                        const VkClearColorValue &clearColor) const
{
    VkClearValue clearValue = {};
    clearValue.color        = clearColor;

    if (!mEmulatedAlphaAttachmentMask[colorIndexGL])
    {
        return clearValue;
    }

    // If the render target doesn't have alpha, but its emulated format has it, clear the alpha
    // to 1.
    RenderTargetVk *renderTarget = getColorDrawRenderTarget(colorIndexGL);
    const angle::Format &format  = renderTarget->getImageActualFormat();

    if (format.isUint())
    {
        clearValue.color.uint32[3] = kEmulatedAlphaValue;
    }
    else if (format.isSint())
    {
        clearValue.color.int32[3] = kEmulatedAlphaValue;
    }
    else
    {
        clearValue.color.float32[3] = kEmulatedAlphaValue;
    }

    return clearValue;
}

void FramebufferVk::restageDeferredClears(ContextVk *contextVk)
{
    // Called when restaging clears of the draw framebuffer.  In that case, there can't be any
    // render passes open, otherwise the clear would have applied to the render pass.  In the
    // exceptional occasion in blit where the read framebuffer accumulates deferred clears, it can
    // be deferred while this assumption doesn't hold (and restageDeferredClearsForReadFramebuffer
    // should be used instead).
    ASSERT(!contextVk->hasActiveRenderPass() || !mDeferredClears.any());
    restageDeferredClearsImpl(contextVk);
}

void FramebufferVk::restageDeferredClearsForReadFramebuffer(ContextVk *contextVk)
{
    restageDeferredClearsImpl(contextVk);
}

void FramebufferVk::restageDeferredClearsImpl(ContextVk *contextVk)
{
    // Set the appropriate aspect and clear values for depth and stencil.
    VkImageAspectFlags dsAspectFlags  = 0;
    VkClearValue dsClearValue         = {};
    dsClearValue.depthStencil.depth   = mDeferredClears.getDepthValue();
    dsClearValue.depthStencil.stencil = mDeferredClears.getStencilValue();

    if (mDeferredClears.testDepth())
    {
        dsAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        mDeferredClears.reset(vk::kUnpackedDepthIndex);
    }

    if (mDeferredClears.testStencil())
    {
        dsAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        mDeferredClears.reset(vk::kUnpackedStencilIndex);
    }

    // Go through deferred clears and stage the clears for future.
    for (size_t colorIndexGL : mDeferredClears.getColorMask())
    {
        RenderTargetVk *renderTarget = getColorDrawRenderTarget(colorIndexGL);
        gl::ImageIndex imageIndex =
            renderTarget->getImageIndexForClear(mCurrentFramebufferDesc.getLayerCount());
        renderTarget->getImageForWrite().stageClear(imageIndex, VK_IMAGE_ASPECT_COLOR_BIT,
                                                    mDeferredClears[colorIndexGL]);
        mDeferredClears.reset(colorIndexGL);
    }

    if (dsAspectFlags)
    {
        RenderTargetVk *renderTarget = getDepthStencilRenderTarget();
        ASSERT(renderTarget);

        gl::ImageIndex imageIndex =
            renderTarget->getImageIndexForClear(mCurrentFramebufferDesc.getLayerCount());
        renderTarget->getImageForWrite().stageClear(imageIndex, dsAspectFlags, dsClearValue);
    }
}

void FramebufferVk::clearWithCommand(ContextVk *contextVk,
                                     const gl::Rectangle &scissoredRenderArea,
                                     ClearWithCommand behavior,
                                     vk::ClearValuesArray *clears)
{
    // Clear is not affected by viewport, so ContextVk::updateScissor may have decided on a smaller
    // render area.  Grow the render area to the full framebuffer size as this clear path is taken
    // when not scissored.
    vk::RenderPassCommandBufferHelper *renderPassCommands =
        &contextVk->getStartedRenderPassCommands();
    renderPassCommands->growRenderArea(contextVk, scissoredRenderArea);

    gl::AttachmentVector<VkClearAttachment> attachments;

    const bool optimizeWithLoadOp = behavior == ClearWithCommand::OptimizeWithLoadOp;

    // Go through deferred clears and add them to the list of attachments to clear.  If any
    // attachment is unused, skip the clear.  clearWithLoadOp will follow and move the remaining
    // clears up to loadOp.
    vk::PackedAttachmentIndex colorIndexVk(0);
    for (size_t colorIndexGL : mState.getColorAttachmentsMask())
    {
        if (clears->getColorMask().test(colorIndexGL))
        {
            if (renderPassCommands->hasAnyColorAccess(colorIndexVk) ||
                renderPassCommands->getRenderPassDesc().hasColorUnresolveAttachment(colorIndexGL) ||
                !optimizeWithLoadOp)
            {
                // With render pass objects, the clears are indexed by the subpass-mapped locations.
                // With dynamic rendering, they are indexed by the actual attachment index.
                const uint32_t clearAttachmentIndex =
                    contextVk->getFeatures().preferDynamicRendering.enabled
                        ? colorIndexVk.get()
                        : static_cast<uint32_t>(colorIndexGL);

                attachments.emplace_back(VkClearAttachment{
                    VK_IMAGE_ASPECT_COLOR_BIT, clearAttachmentIndex, (*clears)[colorIndexGL]});
                clears->reset(colorIndexGL);
                ++contextVk->getPerfCounters().colorClearAttachments;

                renderPassCommands->onColorAccess(colorIndexVk, vk::ResourceAccess::ReadWrite);
            }
            else
            {
                // Skip this attachment, so we can use a renderpass loadOp to clear it instead.
                // Note that if loadOp=Clear was already used for this color attachment, it will be
                // overriden by the new clear, which is valid because the attachment wasn't used in
                // between.
            }
        }
        ++colorIndexVk;
    }

    // Add depth and stencil to list of attachments as needed.
    VkImageAspectFlags dsAspectFlags  = 0;
    VkClearValue dsClearValue         = {};
    dsClearValue.depthStencil.depth   = clears->getDepthValue();
    dsClearValue.depthStencil.stencil = clears->getStencilValue();
    if (clears->testDepth() &&
        (renderPassCommands->hasAnyDepthAccess() ||
         renderPassCommands->getRenderPassDesc().hasDepthUnresolveAttachment() ||
         !optimizeWithLoadOp))
    {
        dsAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        // Explicitly mark a depth write because we are clearing the depth buffer.
        renderPassCommands->onDepthAccess(vk::ResourceAccess::ReadWrite);
        clears->reset(vk::kUnpackedDepthIndex);
        ++contextVk->getPerfCounters().depthClearAttachments;
    }

    if (clears->testStencil() &&
        (renderPassCommands->hasAnyStencilAccess() ||
         renderPassCommands->getRenderPassDesc().hasStencilUnresolveAttachment() ||
         !optimizeWithLoadOp))
    {
        dsAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        // Explicitly mark a stencil write because we are clearing the stencil buffer.
        renderPassCommands->onStencilAccess(vk::ResourceAccess::ReadWrite);
        clears->reset(vk::kUnpackedStencilIndex);
        ++contextVk->getPerfCounters().stencilClearAttachments;
    }

    if (dsAspectFlags != 0)
    {
        attachments.emplace_back(VkClearAttachment{dsAspectFlags, 0, dsClearValue});

        // Because we may have changed the depth/stencil access mode, update read only depth/stencil
        // mode.
        renderPassCommands->updateDepthStencilReadOnlyMode(
            contextVk->getDepthStencilAttachmentFlags(), dsAspectFlags);
    }

    if (attachments.empty())
    {
        // If called with the intent to definitely clear something with vkCmdClearAttachments, there
        // must have been something to clear!
        ASSERT(optimizeWithLoadOp);
        return;
    }

    const uint32_t layerCount = mState.isMultiview() ? 1 : mCurrentFramebufferDesc.getLayerCount();

    VkClearRect rect                                     = {};
    rect.rect.offset.x                                   = scissoredRenderArea.x;
    rect.rect.offset.y                                   = scissoredRenderArea.y;
    rect.rect.extent.width                               = scissoredRenderArea.width;
    rect.rect.extent.height                              = scissoredRenderArea.height;
    rect.layerCount                                      = layerCount;
    vk::RenderPassCommandBuffer *renderPassCommandBuffer = &renderPassCommands->getCommandBuffer();

    renderPassCommandBuffer->clearAttachments(static_cast<uint32_t>(attachments.size()),
                                              attachments.data(), 1, &rect);
    return;
}

void FramebufferVk::clearWithLoadOp(ContextVk *contextVk)
{
    vk::RenderPassCommandBufferHelper *renderPassCommands =
        &contextVk->getStartedRenderPassCommands();

    // Update the render pass loadOps to clear the attachments.
    vk::PackedAttachmentIndex colorIndexVk(0);
    for (size_t colorIndexGL : mState.getColorAttachmentsMask())
    {
        if (!mDeferredClears.test(colorIndexGL))
        {
            ++colorIndexVk;
            continue;
        }

        ASSERT(!renderPassCommands->hasAnyColorAccess(colorIndexVk));

        renderPassCommands->updateRenderPassColorClear(colorIndexVk, mDeferredClears[colorIndexGL]);

        mDeferredClears.reset(colorIndexGL);

        ++colorIndexVk;
    }

    VkClearValue dsClearValue         = {};
    dsClearValue.depthStencil.depth   = mDeferredClears.getDepthValue();
    dsClearValue.depthStencil.stencil = mDeferredClears.getStencilValue();
    VkImageAspectFlags dsAspects      = 0;

    if (mDeferredClears.testDepth())
    {
        ASSERT(!renderPassCommands->hasAnyDepthAccess());
        dsAspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
        mDeferredClears.reset(vk::kUnpackedDepthIndex);
    }

    if (mDeferredClears.testStencil())
    {
        ASSERT(!renderPassCommands->hasAnyStencilAccess());
        dsAspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
        mDeferredClears.reset(vk::kUnpackedStencilIndex);
    }

    if (dsAspects != 0)
    {
        renderPassCommands->updateRenderPassDepthStencilClear(dsAspects, dsClearValue);

        // The render pass can no longer be in read-only depth/stencil mode.
        renderPassCommands->updateDepthStencilReadOnlyMode(
            contextVk->getDepthStencilAttachmentFlags(), dsAspects);
    }
}

angle::Result FramebufferVk::getSamplePosition(const gl::Context *context,
                                               size_t index,
                                               GLfloat *xy) const
{
    int sampleCount = getSamples();
    rx::GetSamplePosition(sampleCount, index, xy);
    return angle::Result::Continue;
}

angle::Result FramebufferVk::startNewRenderPass(ContextVk *contextVk,
                                                const gl::Rectangle &renderArea,
                                                vk::RenderPassCommandBuffer **commandBufferOut,
                                                bool *renderPassDescChangedOut)
{
    ANGLE_TRY(contextVk->flushCommandsAndEndRenderPass(RenderPassClosureReason::NewRenderPass));

    // Initialize RenderPass info.
    vk::AttachmentOpsArray renderPassAttachmentOps;
    vk::PackedClearValuesArray packedClearValues;
    gl::DrawBufferMask previousUnresolveColorMask =
        mRenderPassDesc.getColorUnresolveAttachmentMask();
    const bool hasDeferredClears        = mDeferredClears.any();
    const bool previousUnresolveDepth   = mRenderPassDesc.hasDepthUnresolveAttachment();
    const bool previousUnresolveStencil = mRenderPassDesc.hasStencilUnresolveAttachment();

    // Make sure render pass and framebuffer are in agreement w.r.t unresolve attachments.
    ASSERT(mCurrentFramebufferDesc.getUnresolveAttachmentMask() ==
           MakeUnresolveAttachmentMask(mRenderPassDesc));
    // ... w.r.t sRGB write control.
    ASSERT(mCurrentFramebufferDesc.getWriteControlMode() ==
           mRenderPassDesc.getSRGBWriteControlMode());
    // ... w.r.t foveation.
    ASSERT(mCurrentFramebufferDesc.hasFragmentShadingRateAttachment() ==
           mRenderPassDesc.hasFragmentShadingAttachment());

    // Color attachments.
    const auto &colorRenderTargets = mRenderTargetCache.getColors();
    vk::PackedAttachmentIndex colorIndexVk(0);
    for (size_t colorIndexGL : mState.getColorAttachmentsMask())
    {
        RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
        ASSERT(colorRenderTarget);

        // Color render targets are never entirely transient.  Only depth/stencil
        // multisampled-render-to-texture textures can be so.
        ASSERT(!colorRenderTarget->isEntirelyTransient());
        const vk::RenderPassStoreOp storeOp = colorRenderTarget->isImageTransient()
                                                  ? vk::RenderPassStoreOp::DontCare
                                                  : vk::RenderPassStoreOp::Store;

        if (mDeferredClears.test(colorIndexGL))
        {
            renderPassAttachmentOps.setOps(colorIndexVk, vk::RenderPassLoadOp::Clear, storeOp);
            packedClearValues.storeColor(colorIndexVk, mDeferredClears[colorIndexGL]);
            mDeferredClears.reset(colorIndexGL);
        }
        else
        {
            const vk::RenderPassLoadOp loadOp = colorRenderTarget->hasDefinedContent()
                                                    ? vk::RenderPassLoadOp::Load
                                                    : vk::RenderPassLoadOp::DontCare;

            renderPassAttachmentOps.setOps(colorIndexVk, loadOp, storeOp);
            packedClearValues.storeColor(colorIndexVk, kUninitializedClearValue);
        }
        renderPassAttachmentOps.setStencilOps(colorIndexVk, vk::RenderPassLoadOp::DontCare,
                                              vk::RenderPassStoreOp::DontCare);

        // If there's a resolve attachment, and loadOp needs to be LOAD, the multisampled attachment
        // needs to take its value from the resolve attachment.  In this case, an initial subpass is
        // added for this very purpose which uses the resolve attachment as input attachment.  As a
        // result, loadOp of the multisampled attachment can remain DONT_CARE.
        //
        // Note that this only needs to be done if the multisampled image and the resolve attachment
        // come from the same source.  isImageTransient() indicates whether this should happen.
        if (colorRenderTarget->hasResolveAttachment() && colorRenderTarget->isImageTransient())
        {
            if (renderPassAttachmentOps[colorIndexVk].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
            {
                renderPassAttachmentOps[colorIndexVk].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

                // Update the render pass desc to specify that this attachment should be unresolved.
                mRenderPassDesc.packColorUnresolveAttachment(colorIndexGL);
            }
            else
            {
                mRenderPassDesc.removeColorUnresolveAttachment(colorIndexGL);
            }
        }
        else
        {
            ASSERT(!mRenderPassDesc.getColorUnresolveAttachmentMask().test(colorIndexGL));
        }

        ++colorIndexVk;
    }

    // Depth/stencil attachment.
    vk::PackedAttachmentIndex depthStencilAttachmentIndex = vk::kAttachmentIndexInvalid;
    RenderTargetVk *depthStencilRenderTarget              = getDepthStencilRenderTarget();
    if (depthStencilRenderTarget)
    {
        // depth stencil attachment always immediately follows color attachment
        depthStencilAttachmentIndex = colorIndexVk;

        vk::RenderPassLoadOp depthLoadOp     = vk::RenderPassLoadOp::Load;
        vk::RenderPassLoadOp stencilLoadOp   = vk::RenderPassLoadOp::Load;
        vk::RenderPassStoreOp depthStoreOp   = vk::RenderPassStoreOp::Store;
        vk::RenderPassStoreOp stencilStoreOp = vk::RenderPassStoreOp::Store;

        // If the image data was previously discarded (with no update in between), don't attempt to
        // load the image.  Additionally, if the multisampled image data is transient and there is
        // no resolve attachment, there's no data to load.  The latter is the case with
        // depth/stencil texture attachments per GL_EXT_multisampled_render_to_texture2.
        if (!depthStencilRenderTarget->hasDefinedContent() ||
            depthStencilRenderTarget->isEntirelyTransient())
        {
            depthLoadOp = vk::RenderPassLoadOp::DontCare;
        }
        if (!depthStencilRenderTarget->hasDefinedStencilContent() ||
            depthStencilRenderTarget->isEntirelyTransient())
        {
            stencilLoadOp = vk::RenderPassLoadOp::DontCare;
        }

        // If depth/stencil image is transient, no need to store its data at the end of the render
        // pass.
        if (depthStencilRenderTarget->isImageTransient())
        {
            depthStoreOp   = vk::RenderPassStoreOp::DontCare;
            stencilStoreOp = vk::RenderPassStoreOp::DontCare;
        }

        if (mDeferredClears.testDepth() || mDeferredClears.testStencil())
        {
            VkClearValue clearValue = {};

            if (mDeferredClears.testDepth())
            {
                depthLoadOp                   = vk::RenderPassLoadOp::Clear;
                clearValue.depthStencil.depth = mDeferredClears.getDepthValue();
                mDeferredClears.reset(vk::kUnpackedDepthIndex);
            }

            if (mDeferredClears.testStencil())
            {
                stencilLoadOp                   = vk::RenderPassLoadOp::Clear;
                clearValue.depthStencil.stencil = mDeferredClears.getStencilValue();
                mDeferredClears.reset(vk::kUnpackedStencilIndex);
            }

            packedClearValues.storeDepthStencil(depthStencilAttachmentIndex, clearValue);
        }
        else
        {
            packedClearValues.storeDepthStencil(depthStencilAttachmentIndex,
                                                kUninitializedClearValue);
        }

        const angle::Format &format = depthStencilRenderTarget->getImageIntendedFormat();
        // If the format we picked has stencil but user did not ask for it due to hardware
        // limitations, use DONT_CARE for load/store. The same logic for depth follows.
        if (format.stencilBits == 0)
        {
            stencilLoadOp  = vk::RenderPassLoadOp::DontCare;
            stencilStoreOp = vk::RenderPassStoreOp::DontCare;
        }
        if (format.depthBits == 0)
        {
            depthLoadOp  = vk::RenderPassLoadOp::DontCare;
            depthStoreOp = vk::RenderPassStoreOp::DontCare;
        }

        // Similar to color attachments, if there's a resolve attachment and the multisampled image
        // is transient, depth/stencil data need to be unresolved in an initial subpass.
        if (depthStencilRenderTarget->hasResolveAttachment() &&
            depthStencilRenderTarget->isImageTransient())
        {
            const bool unresolveDepth   = depthLoadOp == vk::RenderPassLoadOp::Load;
            const bool unresolveStencil = stencilLoadOp == vk::RenderPassLoadOp::Load;

            if (unresolveDepth)
            {
                depthLoadOp = vk::RenderPassLoadOp::DontCare;
            }

            if (unresolveStencil)
            {
                stencilLoadOp = vk::RenderPassLoadOp::DontCare;

                // If VK_EXT_shader_stencil_export is not supported, stencil unresolve is done
                // through a method that requires stencil to have been cleared.
                if (!contextVk->getFeatures().supportsShaderStencilExport.enabled)
                {
                    stencilLoadOp = vk::RenderPassLoadOp::Clear;

                    VkClearValue clearValue = packedClearValues[depthStencilAttachmentIndex];
                    clearValue.depthStencil.stencil = 0;
                    packedClearValues.storeDepthStencil(depthStencilAttachmentIndex, clearValue);
                }
            }

            if (unresolveDepth || unresolveStencil)
            {
                if (unresolveDepth)
                {
                    mRenderPassDesc.packDepthUnresolveAttachment();
                }
                if (unresolveStencil)
                {
                    mRenderPassDesc.packStencilUnresolveAttachment();
                }
            }
            else
            {
                mRenderPassDesc.removeDepthStencilUnresolveAttachment();
            }
        }

        renderPassAttachmentOps.setOps(depthStencilAttachmentIndex, depthLoadOp, depthStoreOp);
        renderPassAttachmentOps.setStencilOps(depthStencilAttachmentIndex, stencilLoadOp,
                                              stencilStoreOp);
    }

    // If render pass description is changed, the previous render pass desc is no longer compatible.
    // Tell the context so that the graphics pipelines can be recreated.
    //
    // Note that render passes are compatible only if the differences are in loadOp/storeOp values,
    // or the existence of resolve attachments in single subpass render passes.  The modification
    // here can add/remove a subpass, or modify its input attachments.
    gl::DrawBufferMask unresolveColorMask = mRenderPassDesc.getColorUnresolveAttachmentMask();
    const bool unresolveDepth             = mRenderPassDesc.hasDepthUnresolveAttachment();
    const bool unresolveStencil           = mRenderPassDesc.hasStencilUnresolveAttachment();
    const bool unresolveChanged           = previousUnresolveColorMask != unresolveColorMask ||
                                  previousUnresolveDepth != unresolveDepth ||
                                  previousUnresolveStencil != unresolveStencil;
    if (unresolveChanged)
    {
        // Make sure framebuffer is recreated.
        releaseCurrentFramebuffer(contextVk);

        mCurrentFramebufferDesc.updateUnresolveMask(MakeUnresolveAttachmentMask(mRenderPassDesc));
    }

    vk::RenderPassFramebuffer framebuffer = {};
    ANGLE_TRY(getFramebuffer(contextVk, &framebuffer));

    // If deferred clears were used in the render pass, the render area must cover the whole
    // framebuffer.
    ASSERT(!hasDeferredClears || renderArea == getRotatedCompleteRenderArea(contextVk));

    ANGLE_TRY(contextVk->beginNewRenderPass(
        std::move(framebuffer), renderArea, mRenderPassDesc, renderPassAttachmentOps, colorIndexVk,
        depthStencilAttachmentIndex, packedClearValues, commandBufferOut));
    mLastRenderPassQueueSerial = contextVk->getStartedRenderPassCommands().getQueueSerial();

    // Add the images to the renderpass tracking list (through onColorDraw).
    vk::PackedAttachmentIndex colorAttachmentIndex(0);
    for (size_t colorIndexGL : mState.getColorAttachmentsMask())
    {
        RenderTargetVk *colorRenderTarget = colorRenderTargets[colorIndexGL];
        colorRenderTarget->onColorDraw(contextVk, mCurrentFramebufferDesc.getLayerCount(),
                                       colorAttachmentIndex);
        ++colorAttachmentIndex;
    }

    if (depthStencilRenderTarget)
    {
        // This must be called after hasDefined*Content() since it will set content to valid.  If
        // the attachment ends up not used in the render pass, contents will be marked undefined at
        // endRenderPass.  The actual layout determination is also deferred until the same time.
        depthStencilRenderTarget->onDepthStencilDraw(contextVk,
                                                     mCurrentFramebufferDesc.getLayerCount());
    }

    const bool anyUnresolve = unresolveColorMask.any() || unresolveDepth || unresolveStencil;
    if (anyUnresolve)
    {
        // Unresolve attachments if any.
        UtilsVk::UnresolveParameters params;
        params.unresolveColorMask = unresolveColorMask;
        params.unresolveDepth     = unresolveDepth;
        params.unresolveStencil   = unresolveStencil;

        ANGLE_TRY(contextVk->getUtils().unresolve(contextVk, this, params));

        // The unresolve subpass has only one draw call.
        ANGLE_TRY(contextVk->startNextSubpass());
    }

    if (unresolveChanged || anyUnresolve)
    {
        contextVk->onDrawFramebufferRenderPassDescChange(this, renderPassDescChangedOut);
    }

    // Add fragment shading rate to the tracking list.
    if (mCurrentFramebufferDesc.hasFragmentShadingRateAttachment())
    {
        contextVk->onFragmentShadingRateRead(&mFragmentShadingRateImage);
    }

    return angle::Result::Continue;
}

gl::Rectangle FramebufferVk::getRenderArea(ContextVk *contextVk) const
{
    if (hasDeferredClears())
    {
        return getRotatedCompleteRenderArea(contextVk);
    }
    else
    {
        return getRotatedScissoredRenderArea(contextVk);
    }
}

void FramebufferVk::updateActiveColorMasks(size_t colorIndexGL, bool r, bool g, bool b, bool a)
{
    gl::BlendStateExt::ColorMaskStorage::SetValueIndexed(
        colorIndexGL, gl::BlendStateExt::PackColorMask(r, g, b, a),
        &mActiveColorComponentMasksForClear);
}

const gl::DrawBufferMask &FramebufferVk::getEmulatedAlphaAttachmentMask() const
{
    return mEmulatedAlphaAttachmentMask;
}

angle::Result FramebufferVk::readPixelsImpl(ContextVk *contextVk,
                                            const gl::Rectangle &area,
                                            const PackPixelsParams &packPixelsParams,
                                            VkImageAspectFlagBits copyAspectFlags,
                                            RenderTargetVk *renderTarget,
                                            void *pixels)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "FramebufferVk::readPixelsImpl");
    gl::LevelIndex levelGL = renderTarget->getLevelIndex();
    uint32_t layer         = renderTarget->getLayerIndex();
    return renderTarget->getImageForCopy().readPixels(contextVk, area, packPixelsParams,
                                                      copyAspectFlags, levelGL, layer, pixels);
}

gl::Extents FramebufferVk::getReadImageExtents() const
{
    RenderTargetVk *readRenderTarget = mRenderTargetCache.getColorRead(mState);
    return readRenderTarget->getExtents();
}

// Return the framebuffer's rotated render area.  This is a gl::Rectangle that is based on the
// dimensions of the framebuffer, IS ROTATED for the draw FBO, and IS NOT y-flipped
//
// Note: Since the rectangle is not scissored (i.e. x and y are guaranteed to be zero), only the
// width and height must be swapped if the rotation is 90 or 270 degrees.
gl::Rectangle FramebufferVk::getRotatedCompleteRenderArea(ContextVk *contextVk) const
{
    gl::Rectangle renderArea = getNonRotatedCompleteRenderArea();
    if (contextVk->isRotatedAspectRatioForDrawFBO())
    {
        // The surface is rotated 90/270 degrees.  This changes the aspect ratio of the surface.
        std::swap(renderArea.width, renderArea.height);
    }
    return renderArea;
}

// Return the framebuffer's scissored and rotated render area.  This is a gl::Rectangle that is
// based on the dimensions of the framebuffer, is clipped to the scissor, IS ROTATED and IS
// Y-FLIPPED for the draw FBO.
//
// Note: Since the rectangle is scissored, it must be fully rotated, and not just have the width
// and height swapped.
gl::Rectangle FramebufferVk::getRotatedScissoredRenderArea(ContextVk *contextVk) const
{
    const gl::Rectangle renderArea = getNonRotatedCompleteRenderArea();
    bool invertViewport            = contextVk->isViewportFlipEnabledForDrawFBO();
    gl::Rectangle scissoredArea    = ClipRectToScissor(contextVk->getState(), renderArea, false);
    gl::Rectangle rotatedScissoredArea;
    RotateRectangle(contextVk->getRotationDrawFramebuffer(), invertViewport, renderArea.width,
                    renderArea.height, scissoredArea, &rotatedScissoredArea);
    return rotatedScissoredArea;
}

GLint FramebufferVk::getSamples() const
{
    const gl::FramebufferAttachment *lastAttachment = nullptr;

    for (size_t colorIndexGL : mState.getEnabledDrawBuffers() & mState.getColorAttachmentsMask())
    {
        const gl::FramebufferAttachment *color = mState.getColorAttachment(colorIndexGL);
        ASSERT(color);

        if (color->isRenderToTexture())
        {
            return color->getSamples();
        }

        lastAttachment = color;
    }
    const gl::FramebufferAttachment *depthStencil = mState.getDepthOrStencilAttachment();
    if (depthStencil)
    {
        if (depthStencil->isRenderToTexture())
        {
            return depthStencil->getSamples();
        }
        lastAttachment = depthStencil;
    }

    // If none of the attachments are multisampled-render-to-texture, take the sample count from the
    // last attachment (any would have worked, as they would all have the same sample count).
    return std::max(lastAttachment ? lastAttachment->getSamples() : 1, 1);
}

angle::Result FramebufferVk::flushDepthStencilDeferredClear(ContextVk *contextVk,
                                                            VkImageAspectFlagBits aspect)
{
    const bool isDepth = aspect == VK_IMAGE_ASPECT_DEPTH_BIT;

    // Pick out the deferred clear for the given aspect, and issue it ahead of the render pass.
    // This is used when switching this aspect to read-only mode, in which case the clear operation
    // for the aspect cannot be done as part of the render pass loadOp.
    ASSERT(!isDepth || hasDeferredDepthClear());
    ASSERT(isDepth || hasDeferredStencilClear());
    ASSERT(mState.getDepthOrStencilAttachment() != nullptr);

    RenderTargetVk *renderTarget = getDepthStencilRenderTarget();
    vk::ImageHelper &image       = renderTarget->getImageForCopy();

    // Depth/stencil attachments cannot be 3D.
    ASSERT(!renderTarget->is3DImage());

    vk::CommandBufferAccess access;
    access.onImageTransferWrite(renderTarget->getLevelIndex(), 1, renderTarget->getLayerIndex(), 1,
                                image.getAspectFlags(), &image);
    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    VkImageSubresourceRange range = {};
    range.aspectMask              = aspect;
    range.baseMipLevel            = image.toVkLevel(renderTarget->getLevelIndex()).get();
    range.levelCount              = 1;
    range.baseArrayLayer          = renderTarget->getLayerIndex();
    range.layerCount              = 1;

    VkClearDepthStencilValue clearValue = {};

    if (isDepth)
    {
        clearValue.depth = mDeferredClears.getDepthValue();
        mDeferredClears.reset(vk::kUnpackedDepthIndex);
    }
    else
    {
        clearValue.stencil = mDeferredClears.getStencilValue();
        mDeferredClears.reset(vk::kUnpackedStencilIndex);
    }

    commandBuffer->clearDepthStencilImage(
        image.getImage(), image.getCurrentLayout(contextVk->getRenderer()), clearValue, 1, &range);
    return angle::Result::Continue;
}

angle::Result FramebufferVk::flushDeferredClears(ContextVk *contextVk)
{
    if (mDeferredClears.empty())
    {
        return angle::Result::Continue;
    }

    return contextVk->startRenderPass(getRotatedCompleteRenderArea(contextVk), nullptr, nullptr);
}

void FramebufferVk::switchToColorFramebufferFetchMode(ContextVk *contextVk,
                                                      bool hasColorFramebufferFetch)
{
    // Framebuffer fetch use by the shader does not affect the framebuffer object in any way with
    // dynamic rendering.
    ASSERT(!contextVk->getFeatures().preferDynamicRendering.enabled);

    // The switch happens once, and is permanent.
    if (mCurrentFramebufferDesc.hasColorFramebufferFetch() == hasColorFramebufferFetch)
    {
        return;
    }

    mCurrentFramebufferDesc.setColorFramebufferFetchMode(hasColorFramebufferFetch);

    mRenderPassDesc.setFramebufferFetchMode(hasColorFramebufferFetch
                                                ? vk::FramebufferFetchMode::Color
                                                : vk::FramebufferFetchMode::None);
    contextVk->onDrawFramebufferRenderPassDescChange(this, nullptr);

    // Make sure framebuffer is recreated.
    releaseCurrentFramebuffer(contextVk);

    // Clear the framebuffer cache, as none of the old framebuffers are usable.
    if (contextVk->getFeatures().permanentlySwitchToFramebufferFetchMode.enabled)
    {
        ASSERT(hasColorFramebufferFetch);
        releaseCurrentFramebuffer(contextVk);
    }
}

bool FramebufferVk::updateLegacyDither(ContextVk *contextVk)
{
    if (contextVk->getFeatures().supportsLegacyDithering.enabled &&
        mRenderPassDesc.isLegacyDitherEnabled() != contextVk->isDitherEnabled())
    {
        mRenderPassDesc.setLegacyDither(contextVk->isDitherEnabled());
        return true;
    }

    return false;
}
}  // namespace rx
