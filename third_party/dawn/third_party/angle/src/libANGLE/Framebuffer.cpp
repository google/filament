//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer.cpp: Implements the gl::Framebuffer class. Implements GL framebuffer
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4 page 105.

#include "libANGLE/Framebuffer.h"

#include "common/Optional.h"
#include "common/bitset_utils.h"
#include "common/utilities.h"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/PixelLocalStorage.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/ShareGroup.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/FramebufferImpl.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/RenderbufferImpl.h"
#include "libANGLE/renderer/SurfaceImpl.h"

using namespace angle;

namespace gl
{

namespace
{

// Check the |checkAttachment| in reference to |firstAttachment| for the sake of multiview
// framebuffer completeness.
FramebufferStatus CheckMultiviewStateMatchesForCompleteness(
    const FramebufferAttachment *firstAttachment,
    const FramebufferAttachment *checkAttachment)
{
    ASSERT(firstAttachment && checkAttachment);
    ASSERT(firstAttachment->isAttached() && checkAttachment->isAttached());

    if (firstAttachment->isMultiview() != checkAttachment->isMultiview())
    {
        return FramebufferStatus::Incomplete(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR,
                                             err::kFramebufferIncompleteMultiviewMismatch);
    }
    if (firstAttachment->getNumViews() != checkAttachment->getNumViews())
    {
        return FramebufferStatus::Incomplete(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR,
                                             err::kFramebufferIncompleteMultiviewViewsMismatch);
    }
    if (checkAttachment->getBaseViewIndex() + checkAttachment->getNumViews() >
        checkAttachment->getSize().depth)
    {
        return FramebufferStatus::Incomplete(GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR,
                                             err::kFramebufferIncompleteMultiviewBaseViewMismatch);
    }

    return FramebufferStatus::Complete();
}

FramebufferStatus CheckAttachmentCompleteness(const Context *context,
                                              const FramebufferAttachment &attachment)
{
    ASSERT(attachment.isAttached());

    const Extents &size = attachment.getSize();
    if (size.width == 0 || size.height == 0)
    {
        return FramebufferStatus::Incomplete(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                                             err::kFramebufferIncompleteAttachmentZeroSize);
    }

    if (!attachment.isRenderable(context))
    {
        return FramebufferStatus::Incomplete(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                                             err::kFramebufferIncompleteAttachmentNotRenderable);
    }

    if (attachment.type() == GL_TEXTURE)
    {
        // [EXT_geometry_shader] Section 9.4.1, "Framebuffer Completeness"
        // If <image> is a three-dimensional texture or a two-dimensional array texture and the
        // attachment is not layered, the selected layer is less than the depth or layer count,
        // respectively, of the texture.
        if (!attachment.isLayered())
        {
            if (attachment.layer() >= size.depth)
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                    err::kFramebufferIncompleteAttachmentLayerGreaterThanDepth);
            }
        }
        // If <image> is a three-dimensional texture or a two-dimensional array texture and the
        // attachment is layered, the depth or layer count, respectively, of the texture is less
        // than or equal to the value of MAX_FRAMEBUFFER_LAYERS_EXT.
        else
        {
            if (size.depth >= context->getCaps().maxFramebufferLayers)
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                    err::kFramebufferIncompleteAttachmentDepthGreaterThanMaxLayers);
            }
        }

        // ES3 specifies that cube map texture attachments must be cube complete.
        // This language is missing from the ES2 spec, but we enforce it here because some
        // desktop OpenGL drivers also enforce this validation.
        // TODO(jmadill): Check if OpenGL ES2 drivers enforce cube completeness.
        const Texture *texture = attachment.getTexture();
        ASSERT(texture);
        if (texture->getType() == TextureType::CubeMap &&
            !texture->getTextureState().isCubeComplete())
        {
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                err::kFramebufferIncompleteAttachmentNotCubeComplete);
        }

        if (!texture->getImmutableFormat())
        {
            GLuint attachmentMipLevel = static_cast<GLuint>(attachment.mipLevel());

            // From the ES 3.0 spec, pg 213:
            // If the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is TEXTURE and the value of
            // FRAMEBUFFER_ATTACHMENT_OBJECT_NAME does not name an immutable-format texture,
            // then the value of FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL must be in the
            // range[levelbase, q], where levelbase is the value of TEXTURE_BASE_LEVEL and q is
            // the effective maximum texture level defined in the Mipmapping discussion of
            // section 3.8.10.4.
            if (attachmentMipLevel < texture->getBaseLevel() ||
                attachmentMipLevel > texture->getMipmapMaxLevel())
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                    err::kFramebufferIncompleteAttachmentLevelOutOfBaseMaxLevelRange);
            }

            // Form the ES 3.0 spec, pg 213/214:
            // If the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is TEXTURE and the value of
            // FRAMEBUFFER_ATTACHMENT_OBJECT_NAME does not name an immutable-format texture and
            // the value of FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL is not levelbase, then the
            // texture must be mipmap complete, and if FRAMEBUFFER_ATTACHMENT_OBJECT_NAME names
            // a cubemap texture, the texture must also be cube complete.
            if (attachmentMipLevel != texture->getBaseLevel() && !texture->isMipmapComplete())
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                    err::kFramebufferIncompleteAttachmentLevelNotBaseLevelForIncompleteMipTexture);
            }
        }
    }

    return FramebufferStatus::Complete();
}

FramebufferStatus CheckAttachmentSampleCounts(const Context *context,
                                              GLsizei currAttachmentSamples,
                                              GLsizei samples,
                                              bool colorAttachment)
{
    if (currAttachmentSamples != samples)
    {
        if (colorAttachment)
        {
            // APPLE_framebuffer_multisample, which EXT_draw_buffers refers to, requires that
            // all color attachments have the same number of samples for the FBO to be complete.
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                err::kFramebufferIncompleteMultisampleInconsistentSampleCounts);
        }
        else
        {
            // CHROMIUM_framebuffer_mixed_samples allows a framebuffer to be considered complete
            // when its depth or stencil samples are a multiple of the number of color samples.
            if (!context->getExtensions().framebufferMixedSamplesCHROMIUM)
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                    err::kFramebufferIncompleteMultisampleInconsistentSampleCounts);
            }

            if ((currAttachmentSamples % std::max(samples, 1)) != 0)
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                    err::
                        kFramebufferIncompleteMultisampleDepthStencilSampleCountDivisibleByColorSampleCount);
            }
        }
    }

    return FramebufferStatus::Complete();
}

FramebufferStatus CheckAttachmentSampleCompleteness(const Context *context,
                                                    const FramebufferAttachment &attachment,
                                                    bool colorAttachment,
                                                    Optional<int> *samples,
                                                    Optional<bool> *fixedSampleLocations,
                                                    Optional<int> *renderToTextureSamples)
{
    ASSERT(attachment.isAttached());

    if (attachment.type() == GL_TEXTURE)
    {
        const Texture *texture = attachment.getTexture();
        ASSERT(texture);
        GLenum sizedInternalFormat    = attachment.getFormat().info->sizedInternalFormat;
        const TextureCaps &formatCaps = context->getTextureCaps().get(sizedInternalFormat);
        if (static_cast<GLuint>(attachment.getSamples()) > formatCaps.getMaxSamples())
        {
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                err::kFramebufferIncompleteAttachmentSamplesGreaterThanMaxSupportedSamples);
        }

        const ImageIndex &attachmentImageIndex = attachment.getTextureImageIndex();
        bool fixedSampleloc = texture->getAttachmentFixedSampleLocations(attachmentImageIndex);
        if (fixedSampleLocations->valid() && fixedSampleloc != fixedSampleLocations->value())
        {
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                err::kFramebufferIncompleteMultisampleInconsistentFixedSampleLocations);
        }
        else
        {
            *fixedSampleLocations = fixedSampleloc;
        }
    }

    if (renderToTextureSamples->valid())
    {
        // Only check against RenderToTextureSamples if they actually exist.
        if (renderToTextureSamples->value() !=
            FramebufferAttachment::kDefaultRenderToTextureSamples)
        {
            FramebufferStatus sampleCountStatus =
                CheckAttachmentSampleCounts(context, attachment.getRenderToTextureSamples(),
                                            renderToTextureSamples->value(), colorAttachment);
            if (!sampleCountStatus.isComplete())
            {
                return sampleCountStatus;
            }
        }
    }
    else
    {
        *renderToTextureSamples = attachment.getRenderToTextureSamples();
    }

    if (samples->valid())
    {
        // RenderToTextureSamples takes precedence if they exist.
        if (renderToTextureSamples->value() ==
            FramebufferAttachment::kDefaultRenderToTextureSamples)
        {

            FramebufferStatus sampleCountStatus = CheckAttachmentSampleCounts(
                context, attachment.getSamples(), samples->value(), colorAttachment);
            if (!sampleCountStatus.isComplete())
            {
                return sampleCountStatus;
            }
        }
    }
    else
    {
        *samples = attachment.getSamples();
    }

    return FramebufferStatus::Complete();
}

// Needed to index into the attachment arrays/bitsets.
static_assert(static_cast<size_t>(IMPLEMENTATION_MAX_DRAW_BUFFERS) ==
                  Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_MAX,
              "Framebuffer Dirty bit mismatch");
static_assert(static_cast<size_t>(IMPLEMENTATION_MAX_DRAW_BUFFERS) ==
                  Framebuffer::DIRTY_BIT_DEPTH_ATTACHMENT,
              "Framebuffer Dirty bit mismatch");
static_assert(static_cast<size_t>(IMPLEMENTATION_MAX_DRAW_BUFFERS + 1) ==
                  Framebuffer::DIRTY_BIT_STENCIL_ATTACHMENT,
              "Framebuffer Dirty bit mismatch");

angle::Result InitAttachment(const Context *context, FramebufferAttachment *attachment)
{
    ASSERT(attachment->isAttached());
    if (attachment->initState() == InitState::MayNeedInit)
    {
        ANGLE_TRY(attachment->initializeContents(context));
    }
    return angle::Result::Continue;
}

bool AttachmentOverlapsWithTexture(const FramebufferAttachment &attachment,
                                   const Texture *texture,
                                   const Sampler *sampler)
{
    if (!attachment.isTextureWithId(texture->id()))
    {
        return false;
    }

    const gl::ImageIndex &index      = attachment.getTextureImageIndex();
    GLuint attachmentLevel           = static_cast<GLuint>(index.getLevelIndex());
    GLuint textureEffectiveBaseLevel = texture->getTextureState().getEffectiveBaseLevel();
    GLuint textureMaxLevel           = textureEffectiveBaseLevel;
    if ((sampler && IsMipmapFiltered(sampler->getSamplerState().getMinFilter())) ||
        IsMipmapFiltered(texture->getSamplerState().getMinFilter()))
    {
        textureMaxLevel = texture->getMipmapMaxLevel();
    }

    return attachmentLevel >= textureEffectiveBaseLevel && attachmentLevel <= textureMaxLevel;
}

constexpr ComponentType GetAttachmentComponentType(GLenum componentType)
{
    switch (componentType)
    {
        case GL_INT:
            return ComponentType::Int;
        case GL_UNSIGNED_INT:
            return ComponentType::UnsignedInt;
        default:
            return ComponentType::Float;
    }
}

bool HasSupportedStencilBitCount(const Framebuffer *framebuffer)
{
    const FramebufferAttachment *stencilAttachment =
        framebuffer ? framebuffer->getStencilOrDepthStencilAttachment() : nullptr;
    return !stencilAttachment || stencilAttachment->getStencilSize() == 8;
}

}  // anonymous namespace

FramebufferStatus FramebufferStatus::Complete()
{
    FramebufferStatus result;
    result.status = GL_FRAMEBUFFER_COMPLETE;
    result.reason = nullptr;
    return result;
}

FramebufferStatus FramebufferStatus::Incomplete(GLenum status, const char *reason)
{
    ASSERT(status != GL_FRAMEBUFFER_COMPLETE);

    FramebufferStatus result;
    result.status = status;
    result.reason = reason;
    return result;
}

// This constructor is only used for default framebuffers.
FramebufferState::FramebufferState(rx::UniqueSerial serial)
    : mId(Framebuffer::kDefaultDrawFramebufferHandle),
      mFramebufferSerial(serial),
      mLabel(),
      mColorAttachments(1),
      mColorAttachmentsMask(0),
      mDrawBufferStates(1, GL_BACK),
      mReadBufferState(GL_BACK),
      mDrawBufferTypeMask(),
      mDefaultWidth(0),
      mDefaultHeight(0),
      mDefaultSamples(0),
      mDefaultFixedSampleLocations(GL_FALSE),
      mDefaultLayers(0),
      mFlipY(GL_FALSE),
      mWebGLDepthStencilConsistent(true),
      mDefaultFramebufferReadAttachmentInitialized(false),
      mSrgbWriteControlMode(SrgbWriteControlMode::Default)
{
    ASSERT(mDrawBufferStates.size() > 0);
    mEnabledDrawBuffers.set(0);
}

FramebufferState::FramebufferState(const Caps &caps, FramebufferID id, rx::UniqueSerial serial)
    : mId(id),
      mFramebufferSerial(serial),
      mLabel(),
      mColorAttachments(caps.maxColorAttachments),
      mColorAttachmentsMask(0),
      mDrawBufferStates(caps.maxDrawBuffers, GL_NONE),
      mReadBufferState(GL_COLOR_ATTACHMENT0_EXT),
      mDrawBufferTypeMask(),
      mDefaultWidth(0),
      mDefaultHeight(0),
      mDefaultSamples(0),
      mDefaultFixedSampleLocations(GL_FALSE),
      mDefaultLayers(0),
      mFlipY(GL_FALSE),
      mWebGLDepthStencilConsistent(true),
      mDefaultFramebufferReadAttachmentInitialized(false),
      mSrgbWriteControlMode(SrgbWriteControlMode::Default)
{
    ASSERT(mId != Framebuffer::kDefaultDrawFramebufferHandle);
    ASSERT(mDrawBufferStates.size() > 0);
    mDrawBufferStates[0] = GL_COLOR_ATTACHMENT0_EXT;
}

FramebufferState::~FramebufferState() {}

const std::string &FramebufferState::getLabel() const
{
    return mLabel;
}

const FramebufferAttachment *FramebufferState::getAttachment(const Context *context,
                                                             GLenum attachment) const
{
    if (attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT15)
    {
        return getColorAttachment(attachment - GL_COLOR_ATTACHMENT0);
    }

    // WebGL1 allows a developer to query for attachment parameters even when "inconsistant" (i.e.
    // multiple conflicting attachment points) and requires us to return the framebuffer attachment
    // associated with WebGL.
    switch (attachment)
    {
        case GL_COLOR:
        case GL_BACK:
            return getColorAttachment(0);
        case GL_DEPTH:
        case GL_DEPTH_ATTACHMENT:
            if (context->isWebGL1())
            {
                return getWebGLDepthAttachment();
            }
            else
            {
                return getDepthAttachment();
            }
        case GL_STENCIL:
        case GL_STENCIL_ATTACHMENT:
            if (context->isWebGL1())
            {
                return getWebGLStencilAttachment();
            }
            else
            {
                return getStencilAttachment();
            }
        case GL_DEPTH_STENCIL:
        case GL_DEPTH_STENCIL_ATTACHMENT:
            if (context->isWebGL1())
            {
                return getWebGLDepthStencilAttachment();
            }
            else
            {
                return getDepthStencilAttachment();
            }
        default:
            UNREACHABLE();
            return nullptr;
    }
}

uint32_t FramebufferState::getReadIndex() const
{
    ASSERT(mReadBufferState == GL_BACK ||
           (mReadBufferState >= GL_COLOR_ATTACHMENT0 && mReadBufferState <= GL_COLOR_ATTACHMENT15));
    uint32_t readIndex = mReadBufferState == GL_BACK ? 0 : mReadBufferState - GL_COLOR_ATTACHMENT0;
    ASSERT(readIndex < mColorAttachments.size());
    return readIndex;
}

const FramebufferAttachment *FramebufferState::getReadAttachment() const
{
    if (mReadBufferState == GL_NONE)
    {
        return nullptr;
    }

    uint32_t readIndex = getReadIndex();
    const gl::FramebufferAttachment &framebufferAttachment =
        isDefault() ? mDefaultFramebufferReadAttachment : mColorAttachments[readIndex];

    return framebufferAttachment.isAttached() ? &framebufferAttachment : nullptr;
}

const FramebufferAttachment *FramebufferState::getReadPixelsAttachment(GLenum readFormat) const
{
    switch (readFormat)
    {
        case GL_DEPTH_COMPONENT:
            return getDepthAttachment();
        case GL_STENCIL_INDEX_OES:
            return getStencilOrDepthStencilAttachment();
        case GL_DEPTH_STENCIL_OES:
            return getDepthStencilAttachment();
        default:
            return getReadAttachment();
    }
}

const FramebufferAttachment *FramebufferState::getFirstNonNullAttachment() const
{
    auto *colorAttachment = getFirstColorAttachment();
    if (colorAttachment)
    {
        return colorAttachment;
    }
    return getDepthOrStencilAttachment();
}

const FramebufferAttachment *FramebufferState::getFirstColorAttachment() const
{
    for (const FramebufferAttachment &colorAttachment : mColorAttachments)
    {
        if (colorAttachment.isAttached())
        {
            return &colorAttachment;
        }
    }

    return nullptr;
}

const FramebufferAttachment *FramebufferState::getDepthOrStencilAttachment() const
{
    if (mDepthAttachment.isAttached())
    {
        return &mDepthAttachment;
    }
    if (mStencilAttachment.isAttached())
    {
        return &mStencilAttachment;
    }
    return nullptr;
}

const FramebufferAttachment *FramebufferState::getStencilOrDepthStencilAttachment() const
{
    if (mStencilAttachment.isAttached())
    {
        return &mStencilAttachment;
    }
    return getDepthStencilAttachment();
}

const FramebufferAttachment *FramebufferState::getColorAttachment(size_t colorAttachment) const
{
    ASSERT(colorAttachment < mColorAttachments.size());
    return mColorAttachments[colorAttachment].isAttached() ? &mColorAttachments[colorAttachment]
                                                           : nullptr;
}

const FramebufferAttachment *FramebufferState::getDepthAttachment() const
{
    return mDepthAttachment.isAttached() ? &mDepthAttachment : nullptr;
}

const FramebufferAttachment *FramebufferState::getWebGLDepthAttachment() const
{
    return mWebGLDepthAttachment.isAttached() ? &mWebGLDepthAttachment : nullptr;
}

const FramebufferAttachment *FramebufferState::getWebGLDepthStencilAttachment() const
{
    return mWebGLDepthStencilAttachment.isAttached() ? &mWebGLDepthStencilAttachment : nullptr;
}

const FramebufferAttachment *FramebufferState::getStencilAttachment() const
{
    return mStencilAttachment.isAttached() ? &mStencilAttachment : nullptr;
}

const FramebufferAttachment *FramebufferState::getWebGLStencilAttachment() const
{
    return mWebGLStencilAttachment.isAttached() ? &mWebGLStencilAttachment : nullptr;
}

const FramebufferAttachment *FramebufferState::getDepthStencilAttachment() const
{
    // A valid depth-stencil attachment has the same resource bound to both the
    // depth and stencil attachment points.
    if (mDepthAttachment.isAttached() && mStencilAttachment.isAttached() &&
        mDepthAttachment == mStencilAttachment)
    {
        return &mDepthAttachment;
    }

    return nullptr;
}

const Extents FramebufferState::getAttachmentExtentsIntersection() const
{
    int32_t width  = std::numeric_limits<int32_t>::max();
    int32_t height = std::numeric_limits<int32_t>::max();
    for (const FramebufferAttachment &attachment : mColorAttachments)
    {
        if (attachment.isAttached())
        {
            width  = std::min(width, attachment.getSize().width);
            height = std::min(height, attachment.getSize().height);
        }
    }

    if (mDepthAttachment.isAttached())
    {
        width  = std::min(width, mDepthAttachment.getSize().width);
        height = std::min(height, mDepthAttachment.getSize().height);
    }

    if (mStencilAttachment.isAttached())
    {
        width  = std::min(width, mStencilAttachment.getSize().width);
        height = std::min(height, mStencilAttachment.getSize().height);
    }

    return Extents(width, height, 0);
}

bool FramebufferState::attachmentsHaveSameDimensions() const
{
    Optional<Extents> attachmentSize;

    auto hasMismatchedSize = [&attachmentSize](const FramebufferAttachment &attachment) {
        if (!attachment.isAttached())
        {
            return false;
        }

        if (!attachmentSize.valid())
        {
            attachmentSize = attachment.getSize();
            return false;
        }

        const auto &prevSize = attachmentSize.value();
        const auto &curSize  = attachment.getSize();
        return (curSize.width != prevSize.width || curSize.height != prevSize.height);
    };

    for (const auto &attachment : mColorAttachments)
    {
        if (hasMismatchedSize(attachment))
        {
            return false;
        }
    }

    if (hasMismatchedSize(mDepthAttachment))
    {
        return false;
    }

    return !hasMismatchedSize(mStencilAttachment);
}

bool FramebufferState::hasSeparateDepthAndStencilAttachments() const
{
    // if we have both a depth and stencil buffer, they must refer to the same object
    // since we only support packed_depth_stencil and not separate depth and stencil
    return (getDepthAttachment() != nullptr && getStencilAttachment() != nullptr &&
            getDepthStencilAttachment() == nullptr);
}

const FramebufferAttachment *FramebufferState::getDrawBuffer(size_t drawBufferIdx) const
{
    ASSERT(drawBufferIdx < mDrawBufferStates.size());
    if (mDrawBufferStates[drawBufferIdx] != GL_NONE)
    {
        // ES3 spec: "If the GL is bound to a draw framebuffer object, the ith buffer listed in bufs
        // must be COLOR_ATTACHMENTi or NONE"
        ASSERT(mDrawBufferStates[drawBufferIdx] == GL_COLOR_ATTACHMENT0 + drawBufferIdx ||
               (drawBufferIdx == 0 && mDrawBufferStates[drawBufferIdx] == GL_BACK));

        if (mDrawBufferStates[drawBufferIdx] == GL_BACK)
        {
            return getColorAttachment(0);
        }
        else
        {
            return getColorAttachment(mDrawBufferStates[drawBufferIdx] - GL_COLOR_ATTACHMENT0);
        }
    }
    else
    {
        return nullptr;
    }
}

size_t FramebufferState::getDrawBufferCount() const
{
    return mDrawBufferStates.size();
}

bool FramebufferState::colorAttachmentsAreUniqueImages() const
{
    for (size_t firstAttachmentIdx = 0; firstAttachmentIdx < mColorAttachments.size();
         firstAttachmentIdx++)
    {
        const FramebufferAttachment &firstAttachment = mColorAttachments[firstAttachmentIdx];
        if (!firstAttachment.isAttached())
        {
            continue;
        }

        for (size_t secondAttachmentIdx = firstAttachmentIdx + 1;
             secondAttachmentIdx < mColorAttachments.size(); secondAttachmentIdx++)
        {
            const FramebufferAttachment &secondAttachment = mColorAttachments[secondAttachmentIdx];
            if (!secondAttachment.isAttached())
            {
                continue;
            }

            if (firstAttachment == secondAttachment)
            {
                return false;
            }
        }
    }

    return true;
}

bool FramebufferState::hasDepth() const
{
    return mDepthAttachment.isAttached() && mDepthAttachment.getDepthSize() > 0;
}

bool FramebufferState::hasStencil() const
{
    return mStencilAttachment.isAttached() && mStencilAttachment.getStencilSize() > 0;
}

GLuint FramebufferState::getStencilBitCount() const
{
    return mStencilAttachment.isAttached() ? mStencilAttachment.getStencilSize() : 0;
}

bool FramebufferState::hasExternalTextureAttachment() const
{
    // External textures can only be bound to color attachment 0
    return mColorAttachments[0].isAttached() && mColorAttachments[0].isExternalTexture();
}

bool FramebufferState::hasYUVAttachment() const
{
    // The only attachments that can be YUV are external textures and surfaces, both are attached at
    // color attachment 0.
    return mColorAttachments[0].isAttached() && mColorAttachments[0].isYUV();
}

bool FramebufferState::isMultiview() const
{
    const FramebufferAttachment *attachment = getFirstNonNullAttachment();
    if (attachment == nullptr)
    {
        return false;
    }
    return attachment->isMultiview();
}

int FramebufferState::getBaseViewIndex() const
{
    const FramebufferAttachment *attachment = getFirstNonNullAttachment();
    if (attachment == nullptr)
    {
        return GL_NONE;
    }
    return attachment->getBaseViewIndex();
}

Box FramebufferState::getDimensions() const
{
    Extents extents = getExtents();
    return Box(0, 0, 0, extents.width, extents.height, extents.depth);
}

Extents FramebufferState::getExtents() const
{
    // OpenGLES3.0 (https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf
    // section 4.4.4.2) allows attachments have unequal size.
    const FramebufferAttachment *first = getFirstNonNullAttachment();
    if (first)
    {
        return getAttachmentExtentsIntersection();
    }
    return Extents(getDefaultWidth(), getDefaultHeight(), 0);
}

bool FramebufferState::isBoundAsDrawFramebuffer(const Context *context) const
{
    return context->getState().getDrawFramebuffer()->id() == mId;
}

const FramebufferID Framebuffer::kDefaultDrawFramebufferHandle = {0};

Framebuffer::Framebuffer(const Context *context, rx::GLImplFactory *factory)
    : mState(context->getShareGroup()->generateFramebufferSerial()),
      mImpl(factory->createFramebuffer(mState)),
      mCachedStatus(FramebufferStatus::Incomplete(GL_FRAMEBUFFER_UNDEFINED_OES,
                                                  err::kFramebufferIncompleteSurfaceless)),
      mDirtyDepthAttachmentBinding(this, DIRTY_BIT_DEPTH_ATTACHMENT),
      mDirtyStencilAttachmentBinding(this, DIRTY_BIT_STENCIL_ATTACHMENT),
      mAttachmentChangedAfterEnablingFoveation(false)
{
    mDirtyColorAttachmentBindings.emplace_back(this, DIRTY_BIT_COLOR_ATTACHMENT_0);
    SetComponentTypeMask(getDrawbufferWriteType(0), 0, &mState.mDrawBufferTypeMask);
}

Framebuffer::Framebuffer(const Context *context, rx::GLImplFactory *factory, FramebufferID id)
    : mState(context->getCaps(), id, context->getShareGroup()->generateFramebufferSerial()),
      mImpl(factory->createFramebuffer(mState)),
      mCachedStatus(),
      mDirtyDepthAttachmentBinding(this, DIRTY_BIT_DEPTH_ATTACHMENT),
      mDirtyStencilAttachmentBinding(this, DIRTY_BIT_STENCIL_ATTACHMENT),
      mAttachmentChangedAfterEnablingFoveation(false)
{
    ASSERT(mImpl != nullptr);
    ASSERT(mState.mColorAttachments.size() ==
           static_cast<size_t>(context->getCaps().maxColorAttachments));

    for (uint32_t colorIndex = 0;
         colorIndex < static_cast<uint32_t>(mState.mColorAttachments.size()); ++colorIndex)
    {
        mDirtyColorAttachmentBindings.emplace_back(this, DIRTY_BIT_COLOR_ATTACHMENT_0 + colorIndex);
    }
    if (context->getClientVersion() >= ES_3_0)
    {
        mDirtyBits.set(DIRTY_BIT_READ_BUFFER);
    }
}

Framebuffer::~Framebuffer()
{
    SafeDelete(mImpl);
}

void Framebuffer::onDestroy(const Context *context)
{
    if (isDefault())
    {
        std::ignore = unsetSurfaces(context);
    }

    for (auto &attachment : mState.mColorAttachments)
    {
        attachment.detach(context, mState.mFramebufferSerial);
    }
    mState.mDepthAttachment.detach(context, mState.mFramebufferSerial);
    mState.mStencilAttachment.detach(context, mState.mFramebufferSerial);
    mState.mWebGLDepthAttachment.detach(context, mState.mFramebufferSerial);
    mState.mWebGLStencilAttachment.detach(context, mState.mFramebufferSerial);
    mState.mWebGLDepthStencilAttachment.detach(context, mState.mFramebufferSerial);

    if (mPixelLocalStorage)
    {
        mPixelLocalStorage->onFramebufferDestroyed(context);
    }

    mImpl->destroy(context);
}

egl::Error Framebuffer::setSurfaces(const Context *context,
                                    egl::Surface *surface,
                                    egl::Surface *readSurface)
{
    // This has to be a default framebuffer.
    ASSERT(isDefault());
    ASSERT(mDirtyColorAttachmentBindings.size() == 1);
    ASSERT(mDirtyColorAttachmentBindings[0].getSubjectIndex() == DIRTY_BIT_COLOR_ATTACHMENT_0);

    ASSERT(!mState.mColorAttachments[0].isAttached());
    ASSERT(!mState.mDepthAttachment.isAttached());
    ASSERT(!mState.mStencilAttachment.isAttached());

    if (surface)
    {
        setAttachmentImpl(context, GL_FRAMEBUFFER_DEFAULT, GL_BACK, ImageIndex(), surface,
                          FramebufferAttachment::kDefaultNumViews,
                          FramebufferAttachment::kDefaultBaseViewIndex, false,
                          FramebufferAttachment::kDefaultRenderToTextureSamples);
        mDirtyBits.set(DIRTY_BIT_COLOR_ATTACHMENT_0);

        if (surface->getConfig()->depthSize > 0)
        {
            setAttachmentImpl(context, GL_FRAMEBUFFER_DEFAULT, GL_DEPTH, ImageIndex(), surface,
                              FramebufferAttachment::kDefaultNumViews,
                              FramebufferAttachment::kDefaultBaseViewIndex, false,
                              FramebufferAttachment::kDefaultRenderToTextureSamples);
            mDirtyBits.set(DIRTY_BIT_DEPTH_ATTACHMENT);
        }

        if (surface->getConfig()->stencilSize > 0)
        {
            setAttachmentImpl(context, GL_FRAMEBUFFER_DEFAULT, GL_STENCIL, ImageIndex(), surface,
                              FramebufferAttachment::kDefaultNumViews,
                              FramebufferAttachment::kDefaultBaseViewIndex, false,
                              FramebufferAttachment::kDefaultRenderToTextureSamples);
            mDirtyBits.set(DIRTY_BIT_STENCIL_ATTACHMENT);
        }

        mState.mSurfaceTextureOffset = surface->getTextureOffset();

        // Ensure the backend has a chance to synchronize its content for a new backbuffer.
        mDirtyBits.set(DIRTY_BIT_COLOR_BUFFER_CONTENTS_0);
    }

    setReadSurface(context, readSurface);

    SetComponentTypeMask(getDrawbufferWriteType(0), 0, &mState.mDrawBufferTypeMask);

    ASSERT(mCachedStatus.value().status == GL_FRAMEBUFFER_UNDEFINED_OES);
    ASSERT(mCachedStatus.value().reason == err::kFramebufferIncompleteSurfaceless);
    if (surface)
    {
        mCachedStatus = FramebufferStatus::Complete();
        ANGLE_TRY(surface->getImplementation()->attachToFramebuffer(context, this));
    }

    return egl::NoError();
}

void Framebuffer::setReadSurface(const Context *context, egl::Surface *readSurface)
{
    // This has to be a default framebuffer.
    ASSERT(isDefault());
    ASSERT(mDirtyColorAttachmentBindings.size() == 1);
    ASSERT(mDirtyColorAttachmentBindings[0].getSubjectIndex() == DIRTY_BIT_COLOR_ATTACHMENT_0);

    // Read surface is not attached.
    ASSERT(!mState.mDefaultFramebufferReadAttachment.isAttached());

    // updateAttachment() without mState.mResourceNeedsInit.set()
    mState.mDefaultFramebufferReadAttachment.attach(
        context, GL_FRAMEBUFFER_DEFAULT, GL_BACK, ImageIndex(), readSurface,
        FramebufferAttachment::kDefaultNumViews, FramebufferAttachment::kDefaultBaseViewIndex,
        false, FramebufferAttachment::kDefaultRenderToTextureSamples, mState.mFramebufferSerial);

    if (context->getClientVersion() >= ES_3_0)
    {
        mDirtyBits.set(DIRTY_BIT_READ_BUFFER);
    }
}

egl::Error Framebuffer::unsetSurfaces(const Context *context)
{
    // This has to be a default framebuffer.
    ASSERT(isDefault());
    ASSERT(mDirtyColorAttachmentBindings.size() == 1);
    ASSERT(mDirtyColorAttachmentBindings[0].getSubjectIndex() == DIRTY_BIT_COLOR_ATTACHMENT_0);

    if (mState.mColorAttachments[0].isAttached())
    {
        const egl::Surface *surface = mState.mColorAttachments[0].getSurface();
        mState.mColorAttachments[0].detach(context, mState.mFramebufferSerial);
        mDirtyBits.set(DIRTY_BIT_COLOR_ATTACHMENT_0);

        if (mState.mDepthAttachment.isAttached())
        {
            mState.mDepthAttachment.detach(context, mState.mFramebufferSerial);
            mDirtyBits.set(DIRTY_BIT_DEPTH_ATTACHMENT);
        }

        if (mState.mStencilAttachment.isAttached())
        {
            mState.mStencilAttachment.detach(context, mState.mFramebufferSerial);
            mDirtyBits.set(DIRTY_BIT_STENCIL_ATTACHMENT);
        }

        ANGLE_TRY(surface->getImplementation()->detachFromFramebuffer(context, this));

        ASSERT(mCachedStatus.value().status == GL_FRAMEBUFFER_COMPLETE);
        mCachedStatus = FramebufferStatus::Incomplete(GL_FRAMEBUFFER_UNDEFINED_OES,
                                                      err::kFramebufferIncompleteSurfaceless);
    }
    else
    {
        ASSERT(!mState.mDepthAttachment.isAttached());
        ASSERT(!mState.mStencilAttachment.isAttached());
        ASSERT(mCachedStatus.value().status == GL_FRAMEBUFFER_UNDEFINED_OES);
        ASSERT(mCachedStatus.value().reason == err::kFramebufferIncompleteSurfaceless);
    }

    mState.mDefaultFramebufferReadAttachment.detach(context, mState.mFramebufferSerial);
    mState.mDefaultFramebufferReadAttachmentInitialized = false;
    return egl::NoError();
}

angle::Result Framebuffer::setLabel(const Context *context, const std::string &label)
{
    mState.mLabel = label;

    if (mImpl)
    {
        return mImpl->onLabelUpdate(context);
    }
    return angle::Result::Continue;
}

const std::string &Framebuffer::getLabel() const
{
    return mState.mLabel;
}

bool Framebuffer::detachTexture(Context *context, TextureID textureId)
{
    return detachResourceById(context, GL_TEXTURE, textureId.value);
}

bool Framebuffer::detachRenderbuffer(Context *context, RenderbufferID renderbufferId)
{
    return detachResourceById(context, GL_RENDERBUFFER, renderbufferId.value);
}

bool Framebuffer::detachResourceById(Context *context, GLenum resourceType, GLuint resourceId)
{
    bool found = false;

    for (size_t colorIndex = 0; colorIndex < mState.mColorAttachments.size(); ++colorIndex)
    {
        if (detachMatchingAttachment(context, &mState.mColorAttachments[colorIndex], resourceType,
                                     resourceId))
        {
            found = true;
        }
    }

    if (context->isWebGL1())
    {
        const std::array<FramebufferAttachment *, 3> attachments = {
            {&mState.mWebGLDepthStencilAttachment, &mState.mWebGLDepthAttachment,
             &mState.mWebGLStencilAttachment}};
        for (FramebufferAttachment *attachment : attachments)
        {
            if (detachMatchingAttachment(context, attachment, resourceType, resourceId))
            {
                found = true;
            }
        }
    }
    else
    {
        if (detachMatchingAttachment(context, &mState.mDepthAttachment, resourceType, resourceId))
        {
            found = true;
        }
        if (detachMatchingAttachment(context, &mState.mStencilAttachment, resourceType, resourceId))
        {
            found = true;
        }
    }

    return found;
}

bool Framebuffer::detachMatchingAttachment(Context *context,
                                           FramebufferAttachment *attachment,
                                           GLenum matchType,
                                           GLuint matchId)
{
    if (attachment->isAttached() && attachment->type() == matchType && attachment->id() == matchId)
    {
        const State &contextState = context->getState();
        if (contextState.getPixelLocalStorageActivePlanes() != 0 &&
            this == contextState.getDrawFramebuffer())
        {
            // If a (renderbuffer, texture) object is deleted while its image is attached to the
            // currently bound draw framebuffer object, and pixel local storage is active, then it
            // is as if EndPixelLocalStorageANGLE() had been called with
            // <n>=PIXEL_LOCAL_STORAGE_ACTIVE_PLANES_ANGLE and <storeops> of STORE_OP_STORE_ANGLE.
            context->endPixelLocalStorageImplicit();
        }
        // We go through resetAttachment to make sure that all the required bookkeeping will be done
        // such as updating enabled draw buffer state.
        resetAttachment(context, attachment->getBinding());
        return true;
    }

    return false;
}

const FramebufferAttachment *Framebuffer::getColorAttachment(size_t colorAttachment) const
{
    return mState.getColorAttachment(colorAttachment);
}

const FramebufferAttachment *Framebuffer::getDepthAttachment() const
{
    return mState.getDepthAttachment();
}

const FramebufferAttachment *Framebuffer::getStencilAttachment() const
{
    return mState.getStencilAttachment();
}

const FramebufferAttachment *Framebuffer::getDepthStencilAttachment() const
{
    return mState.getDepthStencilAttachment();
}

const FramebufferAttachment *Framebuffer::getDepthOrStencilAttachment() const
{
    return mState.getDepthOrStencilAttachment();
}

const FramebufferAttachment *Framebuffer::getStencilOrDepthStencilAttachment() const
{
    return mState.getStencilOrDepthStencilAttachment();
}

const FramebufferAttachment *Framebuffer::getReadColorAttachment() const
{
    return mState.getReadAttachment();
}

GLenum Framebuffer::getReadColorAttachmentType() const
{
    const FramebufferAttachment *readAttachment = mState.getReadAttachment();
    return (readAttachment != nullptr ? readAttachment->type() : GL_NONE);
}

const FramebufferAttachment *Framebuffer::getFirstColorAttachment() const
{
    return mState.getFirstColorAttachment();
}

const FramebufferAttachment *Framebuffer::getFirstNonNullAttachment() const
{
    return mState.getFirstNonNullAttachment();
}

const FramebufferAttachment *Framebuffer::getAttachment(const Context *context,
                                                        GLenum attachment) const
{
    return mState.getAttachment(context, attachment);
}

size_t Framebuffer::getDrawbufferStateCount() const
{
    return mState.mDrawBufferStates.size();
}

GLenum Framebuffer::getDrawBufferState(size_t drawBuffer) const
{
    ASSERT(drawBuffer < mState.mDrawBufferStates.size());
    return mState.mDrawBufferStates[drawBuffer];
}

const DrawBuffersVector<GLenum> &Framebuffer::getDrawBufferStates() const
{
    return mState.getDrawBufferStates();
}

void Framebuffer::setDrawBuffers(size_t count, const GLenum *buffers)
{
    auto &drawStates = mState.mDrawBufferStates;

    ASSERT(count <= drawStates.size());
    std::copy(buffers, buffers + count, drawStates.begin());
    std::fill(drawStates.begin() + count, drawStates.end(), GL_NONE);
    mDirtyBits.set(DIRTY_BIT_DRAW_BUFFERS);

    mState.mEnabledDrawBuffers.reset();
    mState.mDrawBufferTypeMask.reset();

    for (size_t index = 0; index < count; ++index)
    {
        SetComponentTypeMask(getDrawbufferWriteType(index), index, &mState.mDrawBufferTypeMask);

        if (drawStates[index] != GL_NONE && mState.mColorAttachments[index].isAttached())
        {
            mState.mEnabledDrawBuffers.set(index);
        }
    }
}

const FramebufferAttachment *Framebuffer::getDrawBuffer(size_t drawBuffer) const
{
    return mState.getDrawBuffer(drawBuffer);
}

ComponentType Framebuffer::getDrawbufferWriteType(size_t drawBuffer) const
{
    const FramebufferAttachment *attachment = mState.getDrawBuffer(drawBuffer);
    if (attachment == nullptr)
    {
        return ComponentType::NoType;
    }

    return GetAttachmentComponentType(attachment->getFormat().info->componentType);
}

ComponentTypeMask Framebuffer::getDrawBufferTypeMask() const
{
    return mState.mDrawBufferTypeMask;
}

bool Framebuffer::hasEnabledDrawBuffer() const
{
    for (size_t drawbufferIdx = 0; drawbufferIdx < mState.mDrawBufferStates.size(); ++drawbufferIdx)
    {
        if (getDrawBuffer(drawbufferIdx) != nullptr)
        {
            return true;
        }
    }

    return false;
}

GLenum Framebuffer::getReadBufferState() const
{
    return mState.mReadBufferState;
}

void Framebuffer::setReadBuffer(GLenum buffer)
{
    ASSERT(buffer == GL_BACK || buffer == GL_NONE ||
           (buffer >= GL_COLOR_ATTACHMENT0 &&
            (buffer - GL_COLOR_ATTACHMENT0) < mState.mColorAttachments.size()));
    if (mState.mReadBufferState != buffer)
    {
        mState.mReadBufferState = buffer;
        mDirtyBits.set(DIRTY_BIT_READ_BUFFER);
    }
}

void Framebuffer::invalidateCompletenessCache()
{
    if (!isDefault())
    {
        mCachedStatus.reset();
    }
    onStateChange(angle::SubjectMessage::DirtyBitsFlagged);
}

const FramebufferStatus &Framebuffer::checkStatusImpl(const Context *context) const
{
    ASSERT(!isDefault());
    ASSERT(hasAnyDirtyBit() || !mCachedStatus.valid());

    mCachedStatus = checkStatusWithGLFrontEnd(context);

    if (mCachedStatus.value().isComplete())
    {
        // We can skip syncState on several back-ends.
        if (mImpl->shouldSyncStateBeforeCheckStatus())
        {
            {
                angle::Result err = syncAllDrawAttachmentState(context, Command::Other);
                if (err != angle::Result::Continue)
                {
                    mCachedStatus =
                        FramebufferStatus::Incomplete(0, err::kFramebufferIncompleteInternalError);
                    return mCachedStatus.value();
                }
            }

            {
                // This binding is not totally correct. It is ok because the parameter isn't used in
                // the GL back-end and the GL back-end is the only user of
                // syncStateBeforeCheckStatus.
                angle::Result err = syncState(context, GL_FRAMEBUFFER, Command::Other);
                if (err != angle::Result::Continue)
                {
                    mCachedStatus =
                        FramebufferStatus::Incomplete(0, err::kFramebufferIncompleteInternalError);
                    return mCachedStatus.value();
                }
            }
        }

        mCachedStatus = mImpl->checkStatus(context);
    }

    return mCachedStatus.value();
}

FramebufferStatus Framebuffer::checkStatusWithGLFrontEnd(const Context *context) const
{
    const State &state = context->getState();

    ASSERT(!isDefault());

    bool hasAttachments = false;
    Optional<unsigned int> colorbufferSize;
    Optional<int> samples;
    Optional<bool> fixedSampleLocations;
    bool hasRenderbuffer = false;
    Optional<int> renderToTextureSamples;
    uint32_t foveatedRenderingAttachmentCount = 0;

    const FramebufferAttachment *firstAttachment = getFirstNonNullAttachment();

    Optional<bool> isLayered;
    Optional<TextureType> colorAttachmentsTextureType;

    for (const FramebufferAttachment &colorAttachment : mState.mColorAttachments)
    {
        if (colorAttachment.isAttached())
        {
            FramebufferStatus attachmentCompleteness =
                CheckAttachmentCompleteness(context, colorAttachment);
            if (!attachmentCompleteness.isComplete())
            {
                return attachmentCompleteness;
            }

            const InternalFormat &format = *colorAttachment.getFormat().info;
            if (format.depthBits > 0 || format.stencilBits > 0)
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                    err::kFramebufferIncompleteDepthStencilInColorBuffer);
            }

            FramebufferStatus attachmentSampleCompleteness =
                CheckAttachmentSampleCompleteness(context, colorAttachment, true, &samples,
                                                  &fixedSampleLocations, &renderToTextureSamples);
            if (!attachmentSampleCompleteness.isComplete())
            {
                return attachmentSampleCompleteness;
            }

            // in GLES 2.0, all color attachments attachments must have the same number of bitplanes
            // in GLES 3.0, there is no such restriction
            if (state.getClientMajorVersion() < 3)
            {
                if (colorbufferSize.valid())
                {
                    if (format.pixelBytes != colorbufferSize.value())
                    {
                        return FramebufferStatus::Incomplete(
                            GL_FRAMEBUFFER_UNSUPPORTED,
                            err::kFramebufferIncompleteAttachmentInconsistantBitPlanes);
                    }
                }
                else
                {
                    colorbufferSize = format.pixelBytes;
                }
            }

            FramebufferStatus attachmentMultiviewCompleteness =
                CheckMultiviewStateMatchesForCompleteness(firstAttachment, &colorAttachment);
            if (!attachmentMultiviewCompleteness.isComplete())
            {
                return attachmentMultiviewCompleteness;
            }

            hasRenderbuffer = hasRenderbuffer || (colorAttachment.type() == GL_RENDERBUFFER);

            if (!hasAttachments)
            {
                isLayered = colorAttachment.isLayered();
                if (isLayered.value())
                {
                    colorAttachmentsTextureType = colorAttachment.getTextureImageIndex().getType();
                }
                hasAttachments = true;
            }
            else
            {
                // [EXT_geometry_shader] section 9.4.1, "Framebuffer Completeness"
                // If any framebuffer attachment is layered, all populated attachments
                // must be layered. Additionally, all populated color attachments must
                // be from textures of the same target. {FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT }
                ASSERT(isLayered.valid());
                if (isLayered.value() != colorAttachment.isLayered())
                {
                    return FramebufferStatus::Incomplete(
                        GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT,
                        err::kFramebufferIncompleteMismatchedLayeredAttachments);
                }
                else if (isLayered.value())
                {
                    ASSERT(colorAttachmentsTextureType.valid());
                    if (colorAttachmentsTextureType.value() !=
                        colorAttachment.getTextureImageIndex().getType())
                    {
                        return FramebufferStatus::Incomplete(
                            GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT,
                            err::kFramebufferIncompleteMismatchedLayeredTexturetypes);
                    }
                }
            }
            if (colorAttachment.hasFoveatedRendering())
            {
                foveatedRenderingAttachmentCount++;
            }
        }
    }

    const FramebufferAttachment &depthAttachment = mState.mDepthAttachment;
    if (depthAttachment.isAttached())
    {
        FramebufferStatus attachmentCompleteness =
            CheckAttachmentCompleteness(context, depthAttachment);
        if (!attachmentCompleteness.isComplete())
        {
            return attachmentCompleteness;
        }

        const InternalFormat &format = *depthAttachment.getFormat().info;
        if (format.depthBits == 0)
        {
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                err::kFramebufferIncompleteAttachmentNoDepthBitsInDepthBuffer);
        }

        FramebufferStatus attachmentSampleCompleteness =
            CheckAttachmentSampleCompleteness(context, depthAttachment, false, &samples,
                                              &fixedSampleLocations, &renderToTextureSamples);
        if (!attachmentSampleCompleteness.isComplete())
        {
            return attachmentSampleCompleteness;
        }

        FramebufferStatus attachmentMultiviewCompleteness =
            CheckMultiviewStateMatchesForCompleteness(firstAttachment, &depthAttachment);
        if (!attachmentMultiviewCompleteness.isComplete())
        {
            return attachmentMultiviewCompleteness;
        }

        hasRenderbuffer = hasRenderbuffer || (depthAttachment.type() == GL_RENDERBUFFER);

        if (!hasAttachments)
        {
            isLayered      = depthAttachment.isLayered();
            hasAttachments = true;
        }
        else
        {
            // [EXT_geometry_shader] section 9.4.1, "Framebuffer Completeness"
            // If any framebuffer attachment is layered, all populated attachments
            // must be layered. {FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT }
            ASSERT(isLayered.valid());
            if (isLayered.value() != depthAttachment.isLayered())
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT,
                    err::kFramebufferIncompleteMismatchedLayeredAttachments);
            }
        }
    }

    const FramebufferAttachment &stencilAttachment = mState.mStencilAttachment;
    if (stencilAttachment.isAttached())
    {
        FramebufferStatus attachmentCompleteness =
            CheckAttachmentCompleteness(context, stencilAttachment);
        if (!attachmentCompleteness.isComplete())
        {
            return attachmentCompleteness;
        }

        const InternalFormat &format = *stencilAttachment.getFormat().info;
        if (format.stencilBits == 0)
        {
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                err::kFramebufferIncompleteAttachmentNoStencilBitsInStencilBuffer);
        }

        FramebufferStatus attachmentSampleCompleteness =
            CheckAttachmentSampleCompleteness(context, stencilAttachment, false, &samples,
                                              &fixedSampleLocations, &renderToTextureSamples);
        if (!attachmentSampleCompleteness.isComplete())
        {
            return attachmentSampleCompleteness;
        }

        FramebufferStatus attachmentMultiviewCompleteness =
            CheckMultiviewStateMatchesForCompleteness(firstAttachment, &stencilAttachment);
        if (!attachmentMultiviewCompleteness.isComplete())
        {
            return attachmentMultiviewCompleteness;
        }

        hasRenderbuffer = hasRenderbuffer || (stencilAttachment.type() == GL_RENDERBUFFER);

        if (!hasAttachments)
        {
            hasAttachments = true;
        }
        else
        {
            // [EXT_geometry_shader] section 9.4.1, "Framebuffer Completeness"
            // If any framebuffer attachment is layered, all populated attachments
            // must be layered.
            // {FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT }
            ASSERT(isLayered.valid());
            if (isLayered.value() != stencilAttachment.isLayered())
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT,
                    err::kFramebufferIncompleteMismatchedLayeredAttachments);
            }
        }
    }

    // Starting from ES 3.0 stencil and depth, if present, should be the same image
    if (state.getClientMajorVersion() >= 3 && depthAttachment.isAttached() &&
        stencilAttachment.isAttached() && stencilAttachment != depthAttachment)
    {
        return FramebufferStatus::Incomplete(
            GL_FRAMEBUFFER_UNSUPPORTED,
            err::kFramebufferIncompleteDepthAndStencilBuffersNotTheSame);
    }

    // [QCOM_texture_foveated] - Additions to Chapter 9.4 (Framebuffer Completeness) -
    // - More than one color attachment is foveated.
    //   { FRAMEBUFFER_INCOMPLETE_FOVEATION_QCOM }
    // - Depth or stencil attachments are foveated textures.
    //   { FRAMEBUFFER_INCOMPLETE_FOVEATION_QCOM }
    // - The framebuffer has been configured for foveation via QCOM_framebuffer_foveated
    //   and any color attachment is a foveated texture.
    //   { FRAMEBUFFER_INCOMPLETE_FOVEATION_QCOM }
    const bool multipleAttachmentsAreFoveated = foveatedRenderingAttachmentCount > 1;
    const bool depthAttachmentIsFoveated =
        depthAttachment.isAttached() && depthAttachment.hasFoveatedRendering();
    const bool stencilAttachmentIsFoveated =
        stencilAttachment.isAttached() && stencilAttachment.hasFoveatedRendering();
    const bool framebufferAndAttachmentsAreFoveated =
        isFoveationEnabled() && foveatedRenderingAttachmentCount > 0;
    if (multipleAttachmentsAreFoveated || depthAttachmentIsFoveated ||
        stencilAttachmentIsFoveated || framebufferAndAttachmentsAreFoveated)
    {
        return FramebufferStatus::Incomplete(GL_FRAMEBUFFER_INCOMPLETE_FOVEATION_QCOM,
                                             err::kFramebufferIncompleteFoveatedRendering);
    }

    // Special additional validation for WebGL 1 DEPTH/STENCIL/DEPTH_STENCIL.
    if (state.isWebGL1())
    {
        if (!mState.mWebGLDepthStencilConsistent)
        {
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_UNSUPPORTED,
                err::kFramebufferIncompleteWebGLDepthStencilInconsistant);
        }

        if (mState.mWebGLDepthStencilAttachment.isAttached())
        {
            if (mState.mWebGLDepthStencilAttachment.getDepthSize() == 0 ||
                mState.mWebGLDepthStencilAttachment.getStencilSize() == 0)
            {
                return FramebufferStatus::Incomplete(
                    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                    err::kFramebufferIncompleteAttachmentWebGLDepthStencilNoDepthOrStencilBits);
            }

            FramebufferStatus attachmentMultiviewCompleteness =
                CheckMultiviewStateMatchesForCompleteness(firstAttachment,
                                                          &mState.mWebGLDepthStencilAttachment);
            if (!attachmentMultiviewCompleteness.isComplete())
            {
                return attachmentMultiviewCompleteness;
            }
        }
        else if (mState.mStencilAttachment.isAttached() &&
                 mState.mStencilAttachment.getDepthSize() > 0)
        {
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                err::kFramebufferIncompleteAttachmentWebGLStencilBufferHasDepthBits);
        }
        else if (mState.mDepthAttachment.isAttached() &&
                 mState.mDepthAttachment.getStencilSize() > 0)
        {
            return FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                err::kFramebufferIncompleteAttachmentWebGLDepthBufferHasStencilBits);
        }
    }

    // ES3.1(section 9.4) requires that if no image is attached to the framebuffer, and either the
    // value of the framebuffer's FRAMEBUFFER_DEFAULT_WIDTH or FRAMEBUFFER_DEFAULT_HEIGHT parameters
    // is zero, the framebuffer is considered incomplete.
    GLint defaultWidth  = mState.getDefaultWidth();
    GLint defaultHeight = mState.getDefaultHeight();
    if (!hasAttachments && (defaultWidth == 0 || defaultHeight == 0))
    {
        return FramebufferStatus::Incomplete(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
                                             err::kFramebufferIncompleteDefaultZeroSize);
    }

    // In ES 2.0 and WebGL, all color attachments must have the same width and height.
    // In ES 3.0, there is no such restriction.
    if ((state.getClientMajorVersion() < 3 || state.getExtensions().webglCompatibilityANGLE) &&
        !mState.attachmentsHaveSameDimensions())
    {
        return FramebufferStatus::Incomplete(
            GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
            err::kFramebufferIncompleteInconsistantAttachmentSizes);
    }

    // ES3.1(section 9.4) requires that if the attached images are a mix of renderbuffers and
    // textures, the value of TEXTURE_FIXED_SAMPLE_LOCATIONS must be TRUE for all attached textures.
    if (fixedSampleLocations.valid() && hasRenderbuffer && !fixedSampleLocations.value())
    {
        return FramebufferStatus::Incomplete(
            GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
            err::kFramebufferIncompleteMultisampleNonFixedSamplesWithRenderbuffers);
    }

    // The WebGL conformance tests implicitly define that all framebuffer
    // attachments must be unique. For example, the same level of a texture can
    // not be attached to two different color attachments.
    if (state.getExtensions().webglCompatibilityANGLE)
    {
        if (!mState.colorAttachmentsAreUniqueImages())
        {
            return FramebufferStatus::Incomplete(GL_FRAMEBUFFER_UNSUPPORTED,
                                                 err::kFramebufferIncompleteAttachmentsNotUnique);
        }
    }

    return FramebufferStatus::Complete();
}

angle::Result Framebuffer::discard(const Context *context, size_t count, const GLenum *attachments)
{
    // Back-ends might make the contents of the FBO undefined. In WebGL 2.0, invalidate operations
    // can be no-ops, so we should probably do that to ensure consistency.
    // TODO(jmadill): WebGL behaviour, and robust resource init behaviour without WebGL.

    return mImpl->discard(context, count, attachments);
}

angle::Result Framebuffer::invalidate(const Context *context,
                                      size_t count,
                                      const GLenum *attachments)
{
    // Back-ends might make the contents of the FBO undefined. In WebGL 2.0, invalidate operations
    // can be no-ops, so we should probably do that to ensure consistency.
    // TODO(jmadill): WebGL behaviour, and robust resource init behaviour without WebGL.

    return mImpl->invalidate(context, count, attachments);
}

bool Framebuffer::partialClearNeedsInit(const Context *context,
                                        bool color,
                                        bool depth,
                                        bool stencil)
{
    const auto &glState = context->getState();

    if (!glState.isRobustResourceInitEnabled())
    {
        return false;
    }

    if (depth && context->getFrontendFeatures().forceDepthAttachmentInitOnClear.enabled)
    {
        return true;
    }

    // Scissors can affect clearing.
    // TODO(jmadill): Check for complete scissor overlap.
    if (glState.isScissorTestEnabled())
    {
        return true;
    }

    // If colors masked, we must clear before we clear. Do a simple check.
    // TODO(jmadill): Filter out unused color channels from the test.
    if (color && glState.anyActiveDrawBufferChannelMasked())
    {
        return true;
    }

    if (stencil)
    {
        ASSERT(HasSupportedStencilBitCount(glState.getDrawFramebuffer()));

        // The least significant |stencilBits| of stencil mask state specify a
        // mask. Compare the masks for differences only in those bits, ignoring any
        // difference in the high bits.
        const auto &depthStencil       = glState.getDepthStencilState();
        const GLuint differentFwdMasks = depthStencil.stencilMask ^ depthStencil.stencilWritemask;
        const GLuint differentBackMasks =
            depthStencil.stencilBackMask ^ depthStencil.stencilBackWritemask;

        if (((differentFwdMasks | differentBackMasks) & 0xFF) != 0)
        {
            return true;
        }
    }

    return false;
}

angle::Result Framebuffer::invalidateSub(const Context *context,
                                         size_t count,
                                         const GLenum *attachments,
                                         const Rectangle &area)
{
    // Back-ends might make the contents of the FBO undefined. In WebGL 2.0, invalidate operations
    // can be no-ops, so we should probably do that to ensure consistency.
    // TODO(jmadill): Make a invalidate no-op in WebGL 2.0.

    return mImpl->invalidateSub(context, count, attachments, area);
}

angle::Result Framebuffer::clear(const Context *context, GLbitfield mask)
{
    ASSERT(mask && !context->getState().isRasterizerDiscardEnabled());

    return mImpl->clear(context, mask);
}

angle::Result Framebuffer::clearBufferfv(const Context *context,
                                         GLenum buffer,
                                         GLint drawbuffer,
                                         const GLfloat *values)
{
    return mImpl->clearBufferfv(context, buffer, drawbuffer, values);
}

angle::Result Framebuffer::clearBufferuiv(const Context *context,
                                          GLenum buffer,
                                          GLint drawbuffer,
                                          const GLuint *values)
{
    return mImpl->clearBufferuiv(context, buffer, drawbuffer, values);
}

angle::Result Framebuffer::clearBufferiv(const Context *context,
                                         GLenum buffer,
                                         GLint drawbuffer,
                                         const GLint *values)
{
    return mImpl->clearBufferiv(context, buffer, drawbuffer, values);
}

angle::Result Framebuffer::clearBufferfi(const Context *context,
                                         GLenum buffer,
                                         GLint drawbuffer,
                                         GLfloat depth,
                                         GLint stencil)
{
    const bool clearDepth =
        getDepthAttachment() != nullptr && context->getState().getDepthStencilState().depthMask;
    const bool clearStencil = getStencilAttachment() != nullptr &&
                              context->getState().getDepthStencilState().stencilWritemask != 0;

    if (clearDepth && clearStencil)
    {
        ASSERT(buffer == GL_DEPTH_STENCIL);
        ANGLE_TRY(mImpl->clearBufferfi(context, GL_DEPTH_STENCIL, drawbuffer, depth, stencil));
    }
    else if (clearDepth && !clearStencil)
    {
        ANGLE_TRY(mImpl->clearBufferfv(context, GL_DEPTH, drawbuffer, &depth));
    }
    else if (!clearDepth && clearStencil)
    {
        ANGLE_TRY(mImpl->clearBufferiv(context, GL_STENCIL, drawbuffer, &stencil));
    }

    return angle::Result::Continue;
}

GLenum Framebuffer::getImplementationColorReadFormat(const Context *context)
{
    const gl::InternalFormat &format = mImpl->getImplementationColorReadFormat(context);
    return format.getReadPixelsFormat(context->getExtensions());
}

GLenum Framebuffer::getImplementationColorReadType(const Context *context)
{
    const gl::InternalFormat &format = mImpl->getImplementationColorReadFormat(context);
    return format.getReadPixelsType(context->getClientVersion());
}

angle::Result Framebuffer::readPixels(const Context *context,
                                      const Rectangle &area,
                                      GLenum format,
                                      GLenum type,
                                      const PixelPackState &pack,
                                      Buffer *packBuffer,
                                      void *pixels)
{
    ANGLE_TRY(mImpl->readPixels(context, area, format, type, pack, packBuffer, pixels));

    if (packBuffer)
    {
        packBuffer->onDataChanged();
    }

    return angle::Result::Continue;
}

angle::Result Framebuffer::blit(const Context *context,
                                const Rectangle &sourceArea,
                                const Rectangle &destArea,
                                GLbitfield mask,
                                GLenum filter)
{
    ASSERT(mask != 0);

    ANGLE_TRY(mImpl->blit(context, sourceArea, destArea, mask, filter));

    // Mark the contents of the attachments dirty
    if ((mask & GL_COLOR_BUFFER_BIT) != 0)
    {
        for (size_t colorIndex : mState.mEnabledDrawBuffers)
        {
            mDirtyBits.set(DIRTY_BIT_COLOR_BUFFER_CONTENTS_0 + colorIndex);
        }
    }
    if ((mask & GL_DEPTH_BUFFER_BIT) != 0)
    {
        mDirtyBits.set(DIRTY_BIT_DEPTH_BUFFER_CONTENTS);
    }
    if ((mask & GL_STENCIL_BUFFER_BIT) != 0)
    {
        mDirtyBits.set(DIRTY_BIT_STENCIL_BUFFER_CONTENTS);
    }
    onStateChange(angle::SubjectMessage::DirtyBitsFlagged);

    return angle::Result::Continue;
}

int Framebuffer::getSamples(const Context *context) const
{
    if (!isComplete(context))
    {
        return 0;
    }

    ASSERT(mCachedStatus.valid() && mCachedStatus.value().isComplete());

    // For a complete framebuffer, all attachments must have the same sample count.
    // In this case return the first nonzero sample size.
    const FramebufferAttachment *firstNonNullAttachment = mState.getFirstNonNullAttachment();
    ASSERT(firstNonNullAttachment == nullptr || firstNonNullAttachment->isAttached());

    return firstNonNullAttachment ? firstNonNullAttachment->getSamples() : 0;
}

int Framebuffer::getReadBufferResourceSamples(const Context *context) const
{
    if (!isComplete(context))
    {
        return 0;
    }

    ASSERT(mCachedStatus.valid() && mCachedStatus.value().isComplete());

    const FramebufferAttachment *readAttachment = mState.getReadAttachment();
    ASSERT(readAttachment == nullptr || readAttachment->isAttached());

    return readAttachment ? readAttachment->getResourceSamples() : 0;
}

angle::Result Framebuffer::getSamplePosition(const Context *context,
                                             size_t index,
                                             GLfloat *xy) const
{
    ANGLE_TRY(mImpl->getSamplePosition(context, index, xy));
    return angle::Result::Continue;
}

bool Framebuffer::hasValidDepthStencil() const
{
    return mState.getDepthStencilAttachment() != nullptr;
}

const gl::Offset &Framebuffer::getSurfaceTextureOffset() const
{
    return mState.getSurfaceTextureOffset();
}

void Framebuffer::setAttachment(const Context *context,
                                GLenum type,
                                GLenum binding,
                                const ImageIndex &textureIndex,
                                FramebufferAttachmentObject *resource)
{
    setAttachment(context, type, binding, textureIndex, resource,
                  FramebufferAttachment::kDefaultNumViews,
                  FramebufferAttachment::kDefaultBaseViewIndex, false,
                  FramebufferAttachment::kDefaultRenderToTextureSamples);
}

void Framebuffer::setAttachmentMultisample(const Context *context,
                                           GLenum type,
                                           GLenum binding,
                                           const ImageIndex &textureIndex,
                                           FramebufferAttachmentObject *resource,
                                           GLsizei samples)
{
    setAttachment(context, type, binding, textureIndex, resource,
                  FramebufferAttachment::kDefaultNumViews,
                  FramebufferAttachment::kDefaultBaseViewIndex, false, samples);
}

void Framebuffer::setAttachment(const Context *context,
                                GLenum type,
                                GLenum binding,
                                const ImageIndex &textureIndex,
                                FramebufferAttachmentObject *resource,
                                GLsizei numViews,
                                GLuint baseViewIndex,
                                bool isMultiview,
                                GLsizei samplesIn)
{
    GLsizei samples = samplesIn;
    // Match the sample count to the attachment's sample count.
    if (resource)
    {
        const InternalFormat *info = resource->getAttachmentFormat(binding, textureIndex).info;
        ASSERT(info);
        GLenum sizedInternalFormat    = info->sizedInternalFormat;
        const TextureCaps &formatCaps = context->getTextureCaps().get(sizedInternalFormat);
        samples                       = formatCaps.getNearestSamples(samples);
    }

    // Context may be null in unit tests.
    if (!context || !context->isWebGL1())
    {
        setAttachmentImpl(context, type, binding, textureIndex, resource, numViews, baseViewIndex,
                          isMultiview, samples);
        return;
    }

    switch (binding)
    {
        case GL_DEPTH_STENCIL:
        case GL_DEPTH_STENCIL_ATTACHMENT:
            mState.mWebGLDepthStencilAttachment.attach(
                context, type, binding, textureIndex, resource, numViews, baseViewIndex,
                isMultiview, samples, mState.mFramebufferSerial);
            break;
        case GL_DEPTH:
        case GL_DEPTH_ATTACHMENT:
            mState.mWebGLDepthAttachment.attach(context, type, binding, textureIndex, resource,
                                                numViews, baseViewIndex, isMultiview, samples,
                                                mState.mFramebufferSerial);
            break;
        case GL_STENCIL:
        case GL_STENCIL_ATTACHMENT:
            mState.mWebGLStencilAttachment.attach(context, type, binding, textureIndex, resource,
                                                  numViews, baseViewIndex, isMultiview, samples,
                                                  mState.mFramebufferSerial);
            break;
        default:
            setAttachmentImpl(context, type, binding, textureIndex, resource, numViews,
                              baseViewIndex, isMultiview, samples);
            return;
    }

    commitWebGL1DepthStencilIfConsistent(context, numViews, baseViewIndex, isMultiview, samples);
}

void Framebuffer::setAttachmentMultiview(const Context *context,
                                         GLenum type,
                                         GLenum binding,
                                         const ImageIndex &textureIndex,
                                         FramebufferAttachmentObject *resource,
                                         GLsizei numViews,
                                         GLint baseViewIndex)
{
    setAttachment(context, type, binding, textureIndex, resource, numViews, baseViewIndex, true,
                  FramebufferAttachment::kDefaultRenderToTextureSamples);
}

void Framebuffer::commitWebGL1DepthStencilIfConsistent(const Context *context,
                                                       GLsizei numViews,
                                                       GLuint baseViewIndex,
                                                       bool isMultiview,
                                                       GLsizei samples)
{
    int count = 0;

    std::array<FramebufferAttachment *, 3> attachments = {{&mState.mWebGLDepthStencilAttachment,
                                                           &mState.mWebGLDepthAttachment,
                                                           &mState.mWebGLStencilAttachment}};
    for (FramebufferAttachment *attachment : attachments)
    {
        if (attachment->isAttached())
        {
            count++;
        }
    }

    mState.mWebGLDepthStencilConsistent = (count <= 1);
    if (!mState.mWebGLDepthStencilConsistent)
    {
        // Inconsistent.
        return;
    }

    auto getImageIndexIfTextureAttachment = [](const FramebufferAttachment &attachment) {
        if (attachment.type() == GL_TEXTURE)
        {
            return attachment.getTextureImageIndex();
        }
        else
        {
            return ImageIndex();
        }
    };

    if (mState.mWebGLDepthAttachment.isAttached())
    {
        const auto &depth = mState.mWebGLDepthAttachment;
        setAttachmentImpl(context, depth.type(), GL_DEPTH_ATTACHMENT,
                          getImageIndexIfTextureAttachment(depth), depth.getResource(), numViews,
                          baseViewIndex, isMultiview, samples);
        setAttachmentImpl(context, GL_NONE, GL_STENCIL_ATTACHMENT, ImageIndex(), nullptr, numViews,
                          baseViewIndex, isMultiview, samples);
    }
    else if (mState.mWebGLStencilAttachment.isAttached())
    {
        const auto &stencil = mState.mWebGLStencilAttachment;
        setAttachmentImpl(context, GL_NONE, GL_DEPTH_ATTACHMENT, ImageIndex(), nullptr, numViews,
                          baseViewIndex, isMultiview, samples);
        setAttachmentImpl(context, stencil.type(), GL_STENCIL_ATTACHMENT,
                          getImageIndexIfTextureAttachment(stencil), stencil.getResource(),
                          numViews, baseViewIndex, isMultiview, samples);
    }
    else if (mState.mWebGLDepthStencilAttachment.isAttached())
    {
        const auto &depthStencil = mState.mWebGLDepthStencilAttachment;
        setAttachmentImpl(context, depthStencil.type(), GL_DEPTH_ATTACHMENT,
                          getImageIndexIfTextureAttachment(depthStencil),
                          depthStencil.getResource(), numViews, baseViewIndex, isMultiview,
                          samples);
        setAttachmentImpl(context, depthStencil.type(), GL_STENCIL_ATTACHMENT,
                          getImageIndexIfTextureAttachment(depthStencil),
                          depthStencil.getResource(), numViews, baseViewIndex, isMultiview,
                          samples);
    }
    else
    {
        setAttachmentImpl(context, GL_NONE, GL_DEPTH_ATTACHMENT, ImageIndex(), nullptr, numViews,
                          baseViewIndex, isMultiview, samples);
        setAttachmentImpl(context, GL_NONE, GL_STENCIL_ATTACHMENT, ImageIndex(), nullptr, numViews,
                          baseViewIndex, isMultiview, samples);
    }
}

void Framebuffer::setAttachmentImpl(const Context *context,
                                    GLenum type,
                                    GLenum binding,
                                    const ImageIndex &textureIndex,
                                    FramebufferAttachmentObject *resource,
                                    GLsizei numViews,
                                    GLuint baseViewIndex,
                                    bool isMultiview,
                                    GLsizei samples)
{
    switch (binding)
    {
        case GL_DEPTH_STENCIL:
        case GL_DEPTH_STENCIL_ATTACHMENT:
            updateAttachment(context, &mState.mDepthAttachment, DIRTY_BIT_DEPTH_ATTACHMENT,
                             &mDirtyDepthAttachmentBinding, type, binding, textureIndex, resource,
                             numViews, baseViewIndex, isMultiview, samples);
            updateAttachment(context, &mState.mStencilAttachment, DIRTY_BIT_STENCIL_ATTACHMENT,
                             &mDirtyStencilAttachmentBinding, type, binding, textureIndex, resource,
                             numViews, baseViewIndex, isMultiview, samples);
            break;

        case GL_DEPTH:
        case GL_DEPTH_ATTACHMENT:
            updateAttachment(context, &mState.mDepthAttachment, DIRTY_BIT_DEPTH_ATTACHMENT,
                             &mDirtyDepthAttachmentBinding, type, binding, textureIndex, resource,
                             numViews, baseViewIndex, isMultiview, samples);
            break;

        case GL_STENCIL:
        case GL_STENCIL_ATTACHMENT:
            updateAttachment(context, &mState.mStencilAttachment, DIRTY_BIT_STENCIL_ATTACHMENT,
                             &mDirtyStencilAttachmentBinding, type, binding, textureIndex, resource,
                             numViews, baseViewIndex, isMultiview, samples);
            break;

        case GL_BACK:
            updateAttachment(context, &mState.mColorAttachments[0], DIRTY_BIT_COLOR_ATTACHMENT_0,
                             &mDirtyColorAttachmentBindings[0], type, binding, textureIndex,
                             resource, numViews, baseViewIndex, isMultiview, samples);
            mState.mColorAttachmentsMask.set(0);

            break;

        default:
        {
            const size_t colorIndex = binding - GL_COLOR_ATTACHMENT0;
            ASSERT(colorIndex < mState.mColorAttachments.size());

            // Caches must be updated before notifying the observers.
            ComponentType componentType = ComponentType::NoType;
            if (!resource)
            {
                mFloat32ColorAttachmentBits.reset(colorIndex);
                mSharedExponentColorAttachmentBits.reset(colorIndex);
                mState.mColorAttachmentsMask.reset(colorIndex);
            }
            else
            {
                const InternalFormat *formatInfo =
                    resource->getAttachmentFormat(binding, textureIndex).info;
                componentType = GetAttachmentComponentType(formatInfo->componentType);
                updateFloat32AndSharedExponentColorAttachmentBits(colorIndex, formatInfo);
                mState.mColorAttachmentsMask.set(colorIndex);
            }
            const bool enabled = (type != GL_NONE && getDrawBufferState(colorIndex) != GL_NONE);
            mState.mEnabledDrawBuffers.set(colorIndex, enabled);
            SetComponentTypeMask(componentType, colorIndex, &mState.mDrawBufferTypeMask);

            const size_t dirtyBit = DIRTY_BIT_COLOR_ATTACHMENT_0 + colorIndex;
            updateAttachment(context, &mState.mColorAttachments[colorIndex], dirtyBit,
                             &mDirtyColorAttachmentBindings[colorIndex], type, binding,
                             textureIndex, resource, numViews, baseViewIndex, isMultiview, samples);
        }
        break;
    }
}

void Framebuffer::updateAttachment(const Context *context,
                                   FramebufferAttachment *attachment,
                                   size_t dirtyBit,
                                   angle::ObserverBinding *onDirtyBinding,
                                   GLenum type,
                                   GLenum binding,
                                   const ImageIndex &textureIndex,
                                   FramebufferAttachmentObject *resource,
                                   GLsizei numViews,
                                   GLuint baseViewIndex,
                                   bool isMultiview,
                                   GLsizei samples)
{
    attachment->attach(context, type, binding, textureIndex, resource, numViews, baseViewIndex,
                       isMultiview, samples, mState.mFramebufferSerial);
    mDirtyBits.set(dirtyBit);
    mState.mResourceNeedsInit.set(dirtyBit, attachment->initState() == InitState::MayNeedInit);
    onDirtyBinding->bind(resource);
    mAttachmentChangedAfterEnablingFoveation = isFoveationEnabled();

    invalidateCompletenessCache();
}

void Framebuffer::resetAttachment(const Context *context, GLenum binding)
{
    setAttachment(context, GL_NONE, binding, ImageIndex(), nullptr);
}

void Framebuffer::setWriteControlMode(SrgbWriteControlMode srgbWriteControlMode)
{
    if (srgbWriteControlMode != mState.getWriteControlMode())
    {
        mState.mSrgbWriteControlMode = srgbWriteControlMode;
        mDirtyBits.set(DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE);
    }
}

angle::Result Framebuffer::syncState(const Context *context,
                                     GLenum framebufferBinding,
                                     Command command) const
{
    if (mDirtyBits.any())
    {
        mDirtyBitsGuard = mDirtyBits;
        ANGLE_TRY(mImpl->syncState(context, framebufferBinding, mDirtyBits, command));
        mDirtyBits.reset();
        mDirtyBitsGuard.reset();
    }
    return angle::Result::Continue;
}

void Framebuffer::onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message)
{
    if (message != angle::SubjectMessage::SubjectChanged)
    {
        // This can be triggered by SubImage calls for Textures.
        if (message == angle::SubjectMessage::ContentsChanged)
        {
            mDirtyBits.set(DIRTY_BIT_COLOR_BUFFER_CONTENTS_0 + index);
            onStateChange(angle::SubjectMessage::DirtyBitsFlagged);
            return;
        }

        // Swapchain changes should only result in color buffer changes.
        if (message == angle::SubjectMessage::SwapchainImageChanged)
        {
            if (index < DIRTY_BIT_COLOR_ATTACHMENT_MAX)
            {
                mDirtyBits.set(DIRTY_BIT_COLOR_BUFFER_CONTENTS_0 + index);
                onStateChange(angle::SubjectMessage::DirtyBitsFlagged);
            }
            return;
        }

        ASSERT(message != angle::SubjectMessage::BindingChanged);

        // This can be triggered by external changes to the default framebuffer.
        if (message == angle::SubjectMessage::SurfaceChanged)
        {
            onStateChange(angle::SubjectMessage::SurfaceChanged);
            return;
        }

        // This can be triggered by freeing TextureStorage in D3D back-end.
        if (message == angle::SubjectMessage::StorageReleased)
        {
            mDirtyBits.set(index);
            invalidateCompletenessCache();
            return;
        }

        // This can be triggered when a subject's foveated rendering state is changed
        if (message == angle::SubjectMessage::FoveatedRenderingStateChanged)
        {
            // Only a color attachment can be foveated.
            ASSERT(index >= DIRTY_BIT_COLOR_ATTACHMENT_0 && index < DIRTY_BIT_COLOR_ATTACHMENT_MAX);
            // Mark the attachment as dirty so we can grab its updated foveation state.
            mDirtyBits.set(index);
            onStateChange(angle::SubjectMessage::DirtyBitsFlagged);
            return;
        }

        // This can be triggered by the GL back-end TextureGL class.
        ASSERT(message == angle::SubjectMessage::DirtyBitsFlagged ||
               message == angle::SubjectMessage::TextureIDDeleted);
        return;
    }

    ASSERT(!mDirtyBitsGuard.valid() || mDirtyBitsGuard.value().test(index));
    mDirtyBits.set(index);

    invalidateCompletenessCache();

    FramebufferAttachment *attachment = getAttachmentFromSubjectIndex(index);

    // Mark the appropriate init flag.
    mState.mResourceNeedsInit.set(index, attachment->initState() == InitState::MayNeedInit);

    static_assert(DIRTY_BIT_COLOR_ATTACHMENT_MAX <= DIRTY_BIT_DEPTH_ATTACHMENT);
    static_assert(DIRTY_BIT_COLOR_ATTACHMENT_MAX <= DIRTY_BIT_STENCIL_ATTACHMENT);

    // Update component type mask, mFloat32ColorAttachmentBits,
    // and mSharedExponentColorAttachmentBits cache
    if (index < DIRTY_BIT_COLOR_ATTACHMENT_MAX)
    {
        const size_t colorIndex = index - DIRTY_BIT_COLOR_ATTACHMENT_0;
        ASSERT(colorIndex < mState.mColorAttachments.size());
        SetComponentTypeMask(
            GetAttachmentComponentType(attachment->getFormat().info->componentType), colorIndex,
            &mState.mDrawBufferTypeMask);
        updateFloat32AndSharedExponentColorAttachmentBits(colorIndex, attachment->getFormat().info);
    }
}

FramebufferAttachment *Framebuffer::getAttachmentFromSubjectIndex(angle::SubjectIndex index)
{
    switch (index)
    {
        case DIRTY_BIT_DEPTH_ATTACHMENT:
            return &mState.mDepthAttachment;
        case DIRTY_BIT_STENCIL_ATTACHMENT:
            return &mState.mStencilAttachment;
        default:
            size_t colorIndex = (index - DIRTY_BIT_COLOR_ATTACHMENT_0);
            ASSERT(colorIndex < mState.mColorAttachments.size());
            return &mState.mColorAttachments[colorIndex];
    }
}

bool Framebuffer::formsRenderingFeedbackLoopWith(const Context *context) const
{
    const State &glState                = context->getState();
    const ProgramExecutable *executable = glState.getLinkedProgramExecutable(context);

    // In some error cases there may be no bound program or executable.
    if (!executable)
        return false;

    const ActiveTextureMask &activeTextures    = executable->getActiveSamplersMask();
    const ActiveTextureTypeArray &textureTypes = executable->getActiveSamplerTypes();

    for (size_t textureIndex : activeTextures)
    {
        unsigned int uintIndex = static_cast<unsigned int>(textureIndex);
        Texture *texture       = glState.getSamplerTexture(uintIndex, textureTypes[textureIndex]);
        const Sampler *sampler = glState.getSampler(uintIndex);
        if (texture && texture->isSamplerComplete(context, sampler) &&
            texture->isBoundToFramebuffer(mState.mFramebufferSerial))
        {
            // Check for level overlap.
            for (const FramebufferAttachment &attachment : mState.mColorAttachments)
            {
                if (AttachmentOverlapsWithTexture(attachment, texture, sampler))
                {
                    return true;
                }
            }

            if (AttachmentOverlapsWithTexture(mState.mDepthAttachment, texture, sampler))
            {
                return true;
            }

            if (AttachmentOverlapsWithTexture(mState.mStencilAttachment, texture, sampler))
            {
                return true;
            }
        }
    }

    return false;
}

bool Framebuffer::formsCopyingFeedbackLoopWith(TextureID copyTextureID,
                                               GLint copyTextureLevel,
                                               GLint copyTextureLayer) const
{
    if (mState.isDefault())
    {
        // It seems impossible to form a texture copying feedback loop with the default FBO.
        return false;
    }

    const FramebufferAttachment *readAttachment = getReadColorAttachment();
    ASSERT(readAttachment);

    if (readAttachment->isTextureWithId(copyTextureID))
    {
        const auto &imageIndex = readAttachment->getTextureImageIndex();
        if (imageIndex.getLevelIndex() == copyTextureLevel)
        {
            // Check 3D/Array texture layers.
            return !imageIndex.hasLayer() || copyTextureLayer == ImageIndex::kEntireLevel ||
                   imageIndex.getLayerIndex() == copyTextureLayer;
        }
    }
    return false;
}

GLint Framebuffer::getDefaultWidth() const
{
    return mState.getDefaultWidth();
}

GLint Framebuffer::getDefaultHeight() const
{
    return mState.getDefaultHeight();
}

GLint Framebuffer::getDefaultSamples() const
{
    return mState.getDefaultSamples();
}

bool Framebuffer::getDefaultFixedSampleLocations() const
{
    return mState.getDefaultFixedSampleLocations();
}

GLint Framebuffer::getDefaultLayers() const
{
    return mState.getDefaultLayers();
}

bool Framebuffer::getFlipY() const
{
    return mState.getFlipY();
}

void Framebuffer::setDefaultWidth(const Context *context, GLint defaultWidth)
{
    mState.mDefaultWidth = defaultWidth;
    mDirtyBits.set(DIRTY_BIT_DEFAULT_WIDTH);
    invalidateCompletenessCache();
}

void Framebuffer::setDefaultHeight(const Context *context, GLint defaultHeight)
{
    mState.mDefaultHeight = defaultHeight;
    mDirtyBits.set(DIRTY_BIT_DEFAULT_HEIGHT);
    invalidateCompletenessCache();
}

void Framebuffer::setDefaultSamples(const Context *context, GLint defaultSamples)
{
    mState.mDefaultSamples = defaultSamples;
    mDirtyBits.set(DIRTY_BIT_DEFAULT_SAMPLES);
    invalidateCompletenessCache();
}

void Framebuffer::setDefaultFixedSampleLocations(const Context *context,
                                                 bool defaultFixedSampleLocations)
{
    mState.mDefaultFixedSampleLocations = defaultFixedSampleLocations;
    mDirtyBits.set(DIRTY_BIT_DEFAULT_FIXED_SAMPLE_LOCATIONS);
    invalidateCompletenessCache();
}

void Framebuffer::setDefaultLayers(GLint defaultLayers)
{
    mState.mDefaultLayers = defaultLayers;
    mDirtyBits.set(DIRTY_BIT_DEFAULT_LAYERS);
}

void Framebuffer::setFlipY(bool flipY)
{
    mState.mFlipY = flipY;
    mDirtyBits.set(DIRTY_BIT_FLIP_Y);
    invalidateCompletenessCache();
}

GLsizei Framebuffer::getNumViews() const
{
    return mState.getNumViews();
}

GLint Framebuffer::getBaseViewIndex() const
{
    return mState.getBaseViewIndex();
}

bool Framebuffer::isMultiview() const
{
    return mState.isMultiview();
}

bool Framebuffer::readDisallowedByMultiview() const
{
    return (mState.isMultiview() && mState.getNumViews() > 1);
}

angle::Result Framebuffer::ensureClearAttachmentsInitialized(const Context *context,
                                                             GLbitfield mask)
{
    const auto &glState = context->getState();
    if (!context->isRobustResourceInitEnabled() || glState.isRasterizerDiscardEnabled())
    {
        return angle::Result::Continue;
    }

    const DepthStencilState &depthStencil = glState.getDepthStencilState();

    bool color = (mask & GL_COLOR_BUFFER_BIT) != 0 && !glState.allActiveDrawBufferChannelsMasked();
    bool depth = (mask & GL_DEPTH_BUFFER_BIT) != 0 && !depthStencil.isDepthMaskedOut();
    bool stencil = (mask & GL_STENCIL_BUFFER_BIT) != 0 &&
                   !depthStencil.isStencilMaskedOut(getStencilBitCount());

    if (!color && !depth && !stencil)
    {
        return angle::Result::Continue;
    }

    if (partialClearNeedsInit(context, color, depth, stencil))
    {
        ANGLE_TRY(ensureDrawAttachmentsInitialized(context));
    }

    // If the impl encounters an error during a a full (non-partial) clear, the attachments will
    // still be marked initialized. This simplifies design, allowing this method to be called before
    // the clear.
    DrawBufferMask clearedColorAttachments =
        color ? mState.getEnabledDrawBuffers() : DrawBufferMask();
    markAttachmentsInitialized(clearedColorAttachments, depth, stencil);

    return angle::Result::Continue;
}

angle::Result Framebuffer::ensureClearBufferAttachmentsInitialized(const Context *context,
                                                                   GLenum buffer,
                                                                   GLint drawbuffer)
{
    if (!context->isRobustResourceInitEnabled() ||
        context->getState().isRasterizerDiscardEnabled() ||
        context->isClearBufferMaskedOut(buffer, drawbuffer, getStencilBitCount()) ||
        mState.mResourceNeedsInit.none())
    {
        return angle::Result::Continue;
    }

    DrawBufferMask clearColorAttachments;
    bool clearDepth   = false;
    bool clearStencil = false;

    switch (buffer)
    {
        case GL_COLOR:
        {
            ASSERT(drawbuffer < static_cast<GLint>(mState.mColorAttachments.size()));
            if (mState.mResourceNeedsInit[drawbuffer])
            {
                clearColorAttachments.set(drawbuffer);
            }
            break;
        }
        case GL_DEPTH:
        {
            if (mState.mResourceNeedsInit[DIRTY_BIT_DEPTH_ATTACHMENT])
            {
                clearDepth = true;
            }
            break;
        }
        case GL_STENCIL:
        {
            if (mState.mResourceNeedsInit[DIRTY_BIT_STENCIL_ATTACHMENT])
            {
                clearStencil = true;
            }
            break;
        }
        case GL_DEPTH_STENCIL:
        {
            if (mState.mResourceNeedsInit[DIRTY_BIT_DEPTH_ATTACHMENT])
            {
                clearDepth = true;
            }
            if (mState.mResourceNeedsInit[DIRTY_BIT_STENCIL_ATTACHMENT])
            {
                clearStencil = true;
            }
            break;
        }
        default:
            UNREACHABLE();
            break;
    }

    if (partialBufferClearNeedsInit(context, buffer) &&
        (clearColorAttachments.any() || clearDepth || clearStencil))
    {
        ANGLE_TRY(mImpl->ensureAttachmentsInitialized(context, clearColorAttachments, clearDepth,
                                                      clearStencil));
    }

    markAttachmentsInitialized(clearColorAttachments, clearDepth, clearStencil);

    return angle::Result::Continue;
}

angle::Result Framebuffer::ensureDrawAttachmentsInitialized(const Context *context)
{
    if (!context->isRobustResourceInitEnabled())
    {
        return angle::Result::Continue;
    }

    DrawBufferMask clearColorAttachments;
    bool clearDepth   = false;
    bool clearStencil = false;

    // Note: we don't actually filter by the draw attachment enum. Just init everything.
    for (size_t bit : mState.mResourceNeedsInit)
    {
        switch (bit)
        {
            case DIRTY_BIT_DEPTH_ATTACHMENT:
                clearDepth = true;
                break;
            case DIRTY_BIT_STENCIL_ATTACHMENT:
                clearStencil = true;
                break;
            default:
                clearColorAttachments[bit] = true;
                break;
        }
    }

    if (clearColorAttachments.any() || clearDepth || clearStencil)
    {
        ANGLE_TRY(mImpl->ensureAttachmentsInitialized(context, clearColorAttachments, clearDepth,
                                                      clearStencil));
        markAttachmentsInitialized(clearColorAttachments, clearDepth, clearStencil);
    }

    return angle::Result::Continue;
}

angle::Result Framebuffer::ensureReadAttachmentsInitialized(const Context *context)
{
    ASSERT(context->isRobustResourceInitEnabled());

    if (mState.mResourceNeedsInit.none())
    {
        return angle::Result::Continue;
    }

    DrawBufferMask clearColorAttachments;
    bool clearDepth   = false;
    bool clearStencil = false;

    if (mState.mReadBufferState != GL_NONE)
    {
        if (isDefault())
        {
            if (!mState.mDefaultFramebufferReadAttachmentInitialized)
            {
                ANGLE_TRY(InitAttachment(context, &mState.mDefaultFramebufferReadAttachment));
                mState.mDefaultFramebufferReadAttachmentInitialized = true;
            }
        }
        else
        {
            size_t readIndex = mState.getReadIndex();
            if (mState.mResourceNeedsInit[readIndex])
            {
                clearColorAttachments[readIndex] = true;
            }
        }
    }

    // Conservatively init depth since it can be read by BlitFramebuffer.
    if (hasDepth() && mState.mResourceNeedsInit[DIRTY_BIT_DEPTH_ATTACHMENT])
    {
        clearDepth = true;
    }

    // Conservatively init stencil since it can be read by BlitFramebuffer.
    if (hasStencil() && mState.mResourceNeedsInit[DIRTY_BIT_STENCIL_ATTACHMENT])
    {
        clearStencil = true;
    }

    if (clearColorAttachments.any() || clearDepth || clearStencil)
    {
        ANGLE_TRY(mImpl->ensureAttachmentsInitialized(context, clearColorAttachments, clearDepth,
                                                      clearStencil));
        markAttachmentsInitialized(clearColorAttachments, clearDepth, clearStencil);
    }

    return angle::Result::Continue;
}

void Framebuffer::markAttachmentsInitialized(const DrawBufferMask &color, bool depth, bool stencil)
{
    // Mark attachments as initialized.
    for (auto colorIndex : color)
    {
        auto &colorAttachment = mState.mColorAttachments[colorIndex];
        ASSERT(colorAttachment.isAttached());
        colorAttachment.setInitState(InitState::Initialized);
        mState.mResourceNeedsInit.reset(colorIndex);
    }

    if (depth && mState.mDepthAttachment.isAttached())
    {
        mState.mDepthAttachment.setInitState(InitState::Initialized);
        mState.mResourceNeedsInit.reset(DIRTY_BIT_DEPTH_ATTACHMENT);
    }

    if (stencil && mState.mStencilAttachment.isAttached())
    {
        mState.mStencilAttachment.setInitState(InitState::Initialized);
        mState.mResourceNeedsInit.reset(DIRTY_BIT_STENCIL_ATTACHMENT);
    }
}

Box Framebuffer::getDimensions() const
{
    return mState.getDimensions();
}

Extents Framebuffer::getExtents() const
{
    return mState.getExtents();
}

GLuint Framebuffer::getFoveatedFeatureBits() const
{
    return mState.mFoveationState.getFoveatedFeatureBits();
}

void Framebuffer::setFoveatedFeatureBits(const GLuint features)
{
    mState.mFoveationState.setFoveatedFeatureBits(features);
}

bool Framebuffer::isFoveationConfigured() const
{
    return mState.mFoveationState.isConfigured();
}

void Framebuffer::configureFoveation()
{
    mState.mFoveationState.configure();
}

void Framebuffer::setFocalPoint(uint32_t layer,
                                uint32_t focalPointIndex,
                                float focalX,
                                float focalY,
                                float gainX,
                                float gainY,
                                float foveaArea)
{
    gl::FocalPoint newFocalPoint(focalX, focalY, gainX, gainY, foveaArea);
    if (mState.mFoveationState.getFocalPoint(layer, focalPointIndex) == newFocalPoint)
    {
        // Nothing to do, early out.
        return;
    }

    mState.mFoveationState.setFocalPoint(layer, focalPointIndex, newFocalPoint);
    mState.mFoveationState.setFoveatedFeatureBits(GL_FOVEATION_ENABLE_BIT_QCOM);
    mDirtyBits.set(DIRTY_BIT_FOVEATION);
    onStateChange(angle::SubjectMessage::DirtyBitsFlagged);
}

const FocalPoint &Framebuffer::getFocalPoint(uint32_t layer, uint32_t focalPoint) const
{
    return mState.mFoveationState.getFocalPoint(layer, focalPoint);
}

GLuint Framebuffer::getSupportedFoveationFeatures() const
{
    return mState.mFoveationState.getSupportedFoveationFeatures();
}

bool Framebuffer::partialBufferClearNeedsInit(const Context *context, GLenum bufferType)
{
    if (!context->isRobustResourceInitEnabled() || mState.mResourceNeedsInit.none())
    {
        return false;
    }

    switch (bufferType)
    {
        case GL_COLOR:
            return partialClearNeedsInit(context, true, false, false);
        case GL_DEPTH:
            return partialClearNeedsInit(context, false, true, false);
        case GL_STENCIL:
            return partialClearNeedsInit(context, false, false, true);
        case GL_DEPTH_STENCIL:
            return partialClearNeedsInit(context, false, true, true);
        default:
            UNREACHABLE();
            return false;
    }
}

PixelLocalStorage &Framebuffer::getPixelLocalStorage(const Context *context)
{
    ASSERT(id().value != 0);
    if (!mPixelLocalStorage)
    {
        mPixelLocalStorage = PixelLocalStorage::Make(context);
    }
    return *mPixelLocalStorage.get();
}

std::unique_ptr<PixelLocalStorage> Framebuffer::detachPixelLocalStorage()
{
    return std::move(mPixelLocalStorage);
}

angle::Result Framebuffer::syncAllDrawAttachmentState(const Context *context, Command command) const
{
    for (size_t drawbufferIdx = 0; drawbufferIdx < mState.getDrawBufferCount(); ++drawbufferIdx)
    {
        ANGLE_TRY(syncAttachmentState(context, command, mState.getDrawBuffer(drawbufferIdx)));
    }

    ANGLE_TRY(syncAttachmentState(context, command, mState.getDepthAttachment()));
    ANGLE_TRY(syncAttachmentState(context, command, mState.getStencilAttachment()));

    return angle::Result::Continue;
}

angle::Result Framebuffer::syncAttachmentState(const Context *context,
                                               Command command,
                                               const FramebufferAttachment *attachment) const
{
    if (!attachment)
    {
        return angle::Result::Continue;
    }

    // Only texture attachments can sync state. Renderbuffer and Surface attachments are always
    // synchronized.
    if (attachment->type() == GL_TEXTURE)
    {
        Texture *texture = attachment->getTexture();
        if (texture->hasAnyDirtyBitExcludingBoundAsAttachmentBit())
        {
            ANGLE_TRY(texture->syncState(context, command));
        }
    }

    return angle::Result::Continue;
}
}  // namespace gl
