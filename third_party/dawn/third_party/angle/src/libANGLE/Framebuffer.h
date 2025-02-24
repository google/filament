//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer.h: Defines the gl::Framebuffer class. Implements GL framebuffer
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4 page 105.

#ifndef LIBANGLE_FRAMEBUFFER_H_
#define LIBANGLE_FRAMEBUFFER_H_

#include <vector>

#include "common/FixedVector.h"
#include "common/Optional.h"
#include "common/angleutils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Observer.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/State.h"
#include "libANGLE/angletypes.h"

namespace rx
{
class GLImplFactory;
class FramebufferImpl;
class RenderbufferImpl;
class SurfaceImpl;
}  // namespace rx

namespace egl
{
class Display;
class Surface;
}  // namespace egl

namespace gl
{
struct Caps;
class Context;
struct Extensions;
class Framebuffer;
class ImageIndex;
class PixelLocalStorage;
class Renderbuffer;
class TextureCapsMap;

struct FramebufferStatus
{
    bool isComplete() const { return status == GL_FRAMEBUFFER_COMPLETE; }

    static FramebufferStatus Complete();
    static FramebufferStatus Incomplete(GLenum status, const char *reason);

    GLenum status      = GL_FRAMEBUFFER_COMPLETE;
    const char *reason = nullptr;
};

class FramebufferState final : angle::NonCopyable
{
  public:
    explicit FramebufferState(rx::UniqueSerial serial);
    FramebufferState(const Caps &caps, FramebufferID id, rx::UniqueSerial serial);
    ~FramebufferState();

    const std::string &getLabel() const;
    uint32_t getReadIndex() const;

    const FramebufferAttachment *getAttachment(const Context *context, GLenum attachment) const;
    const FramebufferAttachment *getReadAttachment() const;
    const FramebufferAttachment *getFirstNonNullAttachment() const;
    const FramebufferAttachment *getFirstColorAttachment() const;
    const FramebufferAttachment *getDepthOrStencilAttachment() const;
    const FramebufferAttachment *getStencilOrDepthStencilAttachment() const;
    const FramebufferAttachment *getColorAttachment(size_t colorAttachment) const;
    const FramebufferAttachment *getDepthAttachment() const;
    const FramebufferAttachment *getStencilAttachment() const;
    const FramebufferAttachment *getDepthStencilAttachment() const;
    const FramebufferAttachment *getReadPixelsAttachment(GLenum readFormat) const;

    const DrawBuffersVector<GLenum> &getDrawBufferStates() const { return mDrawBufferStates; }
    DrawBufferMask getEnabledDrawBuffers() const { return mEnabledDrawBuffers; }
    GLenum getReadBufferState() const { return mReadBufferState; }

    const DrawBuffersVector<FramebufferAttachment> &getColorAttachments() const
    {
        return mColorAttachments;
    }
    const DrawBufferMask getColorAttachmentsMask() const { return mColorAttachmentsMask; }

    const Extents getAttachmentExtentsIntersection() const;
    bool attachmentsHaveSameDimensions() const;
    bool hasSeparateDepthAndStencilAttachments() const;
    bool colorAttachmentsAreUniqueImages() const;
    Box getDimensions() const;
    Extents getExtents() const;

    const FramebufferAttachment *getDrawBuffer(size_t drawBufferIdx) const;
    size_t getDrawBufferCount() const;

    GLint getDefaultWidth() const { return mDefaultWidth; }
    GLint getDefaultHeight() const { return mDefaultHeight; }
    GLint getDefaultSamples() const { return mDefaultSamples; }
    bool getDefaultFixedSampleLocations() const { return mDefaultFixedSampleLocations; }
    GLint getDefaultLayers() const { return mDefaultLayers; }
    bool getFlipY() const { return mFlipY; }

    bool hasDepth() const;
    bool hasStencil() const;
    GLuint getStencilBitCount() const;

    bool hasExternalTextureAttachment() const;
    bool hasYUVAttachment() const;

    bool isMultiview() const;

    ANGLE_INLINE GLsizei getNumViews() const
    {
        const FramebufferAttachment *attachment = getFirstNonNullAttachment();
        if (attachment == nullptr)
        {
            return FramebufferAttachment::kDefaultNumViews;
        }
        return attachment->getNumViews();
    }

    GLint getBaseViewIndex() const;

    SrgbWriteControlMode getWriteControlMode() const { return mSrgbWriteControlMode; }

    FramebufferID id() const { return mId; }

    bool isDefault() const;

    const Offset &getSurfaceTextureOffset() const { return mSurfaceTextureOffset; }

    rx::UniqueSerial getFramebufferSerial() const { return mFramebufferSerial; }

    bool isBoundAsDrawFramebuffer(const Context *context) const;

    bool isFoveationEnabled() const { return mFoveationState.isFoveated(); }

    const FoveationState &getFoveationState() const { return mFoveationState; }

  private:
    const FramebufferAttachment *getWebGLDepthStencilAttachment() const;
    const FramebufferAttachment *getWebGLDepthAttachment() const;
    const FramebufferAttachment *getWebGLStencilAttachment() const;

    friend class Framebuffer;

    // The Framebuffer ID is unique to a Context.
    // The Framebuffer UniqueSerial is unique to a Share Group.
    FramebufferID mId;
    rx::UniqueSerial mFramebufferSerial;
    std::string mLabel;

    DrawBuffersVector<FramebufferAttachment> mColorAttachments;
    FramebufferAttachment mDepthAttachment;
    FramebufferAttachment mStencilAttachment;

    // Tracks all the color buffers attached to this FramebufferDesc
    DrawBufferMask mColorAttachmentsMask;

    DrawBuffersVector<GLenum> mDrawBufferStates;
    GLenum mReadBufferState;
    DrawBufferMask mEnabledDrawBuffers;
    ComponentTypeMask mDrawBufferTypeMask;

    GLint mDefaultWidth;
    GLint mDefaultHeight;
    GLint mDefaultSamples;
    bool mDefaultFixedSampleLocations;
    GLint mDefaultLayers;
    bool mFlipY;

    // It's necessary to store all this extra state so we can restore attachments
    // when DEPTH_STENCIL/DEPTH/STENCIL is unbound in WebGL 1.
    FramebufferAttachment mWebGLDepthStencilAttachment;
    FramebufferAttachment mWebGLDepthAttachment;
    FramebufferAttachment mWebGLStencilAttachment;
    bool mWebGLDepthStencilConsistent;

    // Tracks if we need to initialize the resources for each attachment.
    angle::BitSet<IMPLEMENTATION_MAX_FRAMEBUFFER_ATTACHMENTS + 2> mResourceNeedsInit;

    bool mDefaultFramebufferReadAttachmentInitialized;
    FramebufferAttachment mDefaultFramebufferReadAttachment;

    // EXT_sRGB_write_control
    SrgbWriteControlMode mSrgbWriteControlMode;

    Offset mSurfaceTextureOffset;

    // GL_QCOM_framebuffer_foveated
    FoveationState mFoveationState;
};

class Framebuffer final : public angle::ObserverInterface,
                          public LabeledObject,
                          public angle::Subject
{
  public:
    // Constructor to build default framebuffers.
    Framebuffer(const Context *context, rx::GLImplFactory *factory);
    // Constructor to build application-defined framebuffers
    Framebuffer(const Context *context, rx::GLImplFactory *factory, FramebufferID id);

    ~Framebuffer() override;
    void onDestroy(const Context *context);

    egl::Error setSurfaces(const Context *context,
                           egl::Surface *surface,
                           egl::Surface *readSurface);
    void setReadSurface(const Context *context, egl::Surface *readSurface);
    egl::Error unsetSurfaces(const Context *context);
    angle::Result setLabel(const Context *context, const std::string &label) override;
    const std::string &getLabel() const override;

    rx::FramebufferImpl *getImplementation() const { return mImpl; }

    FramebufferID id() const { return mState.mId; }

    void setAttachment(const Context *context,
                       GLenum type,
                       GLenum binding,
                       const ImageIndex &textureIndex,
                       FramebufferAttachmentObject *resource);
    void setAttachmentMultisample(const Context *context,
                                  GLenum type,
                                  GLenum binding,
                                  const ImageIndex &textureIndex,
                                  FramebufferAttachmentObject *resource,
                                  GLsizei samples);
    void setAttachmentMultiview(const Context *context,
                                GLenum type,
                                GLenum binding,
                                const ImageIndex &textureIndex,
                                FramebufferAttachmentObject *resource,
                                GLsizei numViews,
                                GLint baseViewIndex);
    void resetAttachment(const Context *context, GLenum binding);

    bool detachTexture(Context *context, TextureID texture);
    bool detachRenderbuffer(Context *context, RenderbufferID renderbuffer);

    const FramebufferAttachment *getColorAttachment(size_t colorAttachment) const;
    const FramebufferAttachment *getDepthAttachment() const;
    const FramebufferAttachment *getStencilAttachment() const;
    const FramebufferAttachment *getDepthStencilAttachment() const;
    const FramebufferAttachment *getDepthOrStencilAttachment() const;
    const FramebufferAttachment *getStencilOrDepthStencilAttachment() const;
    const FramebufferAttachment *getReadColorAttachment() const;
    GLenum getReadColorAttachmentType() const;
    const FramebufferAttachment *getFirstColorAttachment() const;
    const FramebufferAttachment *getFirstNonNullAttachment() const;

    const DrawBuffersVector<FramebufferAttachment> &getColorAttachments() const
    {
        return mState.mColorAttachments;
    }

    const FramebufferState &getState() const { return mState; }

    const FramebufferAttachment *getAttachment(const Context *context, GLenum attachment) const;
    bool isMultiview() const;
    bool readDisallowedByMultiview() const;
    GLsizei getNumViews() const;
    GLint getBaseViewIndex() const;
    Extents getExtents() const;

    size_t getDrawbufferStateCount() const;
    GLenum getDrawBufferState(size_t drawBuffer) const;
    const DrawBuffersVector<GLenum> &getDrawBufferStates() const;
    void setDrawBuffers(size_t count, const GLenum *buffers);
    const FramebufferAttachment *getDrawBuffer(size_t drawBuffer) const;
    ComponentType getDrawbufferWriteType(size_t drawBuffer) const;
    ComponentTypeMask getDrawBufferTypeMask() const;
    DrawBufferMask getDrawBufferMask() const { return mState.mEnabledDrawBuffers; }
    bool hasEnabledDrawBuffer() const;

    GLenum getReadBufferState() const;
    void setReadBuffer(GLenum buffer);

    size_t getNumColorAttachments() const { return mState.mColorAttachments.size(); }
    bool hasDepth() const { return mState.hasDepth(); }
    bool hasStencil() const { return mState.hasStencil(); }
    GLuint getStencilBitCount() const { return mState.getStencilBitCount(); }

    bool hasExternalTextureAttachment() const { return mState.hasExternalTextureAttachment(); }
    bool hasYUVAttachment() const { return mState.hasYUVAttachment(); }

    // This method calls checkStatus.
    int getSamples(const Context *context) const;
    int getReadBufferResourceSamples(const Context *context) const;

    angle::Result getSamplePosition(const Context *context, size_t index, GLfloat *xy) const;

    GLint getDefaultWidth() const;
    GLint getDefaultHeight() const;
    GLint getDefaultSamples() const;
    bool getDefaultFixedSampleLocations() const;
    GLint getDefaultLayers() const;
    bool getFlipY() const;
    void setDefaultWidth(const Context *context, GLint defaultWidth);
    void setDefaultHeight(const Context *context, GLint defaultHeight);
    void setDefaultSamples(const Context *context, GLint defaultSamples);
    void setDefaultFixedSampleLocations(const Context *context, bool defaultFixedSampleLocations);
    void setDefaultLayers(GLint defaultLayers);
    void setFlipY(bool flipY);

    bool isFoveationEnabled() const
    {
        return (mState.mFoveationState.getFoveatedFeatureBits() & GL_FOVEATION_ENABLE_BIT_QCOM);
    }
    void setFoveatedFeatureBits(const GLuint features);
    GLuint getFoveatedFeatureBits() const;
    bool isFoveationConfigured() const;
    void configureFoveation();
    void setFocalPoint(uint32_t layer,
                       uint32_t focalPointIndex,
                       float focalX,
                       float focalY,
                       float gainX,
                       float gainY,
                       float foveaArea);
    const FocalPoint &getFocalPoint(uint32_t layer, uint32_t focalPoint) const;
    GLuint getSupportedFoveationFeatures() const;
    bool hasAnyAttachmentChanged() const { return mAttachmentChangedAfterEnablingFoveation; }

    void invalidateCompletenessCache();
    ANGLE_INLINE bool cachedStatusValid() { return mCachedStatus.valid(); }

    ANGLE_INLINE const FramebufferStatus &checkStatus(const Context *context) const
    {
        // The default framebuffer is always complete except when it is surfaceless in which
        // case it is always unsupported.
        ASSERT(!isDefault() || mCachedStatus.valid());
        if (isDefault() || (!hasAnyDirtyBit() && mCachedStatus.valid()))
        {
            return mCachedStatus.value();
        }

        return checkStatusImpl(context);
    }

    // Helper for checkStatus == GL_FRAMEBUFFER_COMPLETE.
    ANGLE_INLINE bool isComplete(const Context *context) const
    {
        return checkStatus(context).isComplete();
    }

    bool hasValidDepthStencil() const;

    // Returns the offset into the texture backing the default framebuffer's surface if any. Returns
    // zero offset otherwise.  The renderer will apply the offset to scissor and viewport rects used
    // for draws, clears, and blits.
    const Offset &getSurfaceTextureOffset() const;

    angle::Result discard(const Context *context, size_t count, const GLenum *attachments);
    angle::Result invalidate(const Context *context, size_t count, const GLenum *attachments);
    angle::Result invalidateSub(const Context *context,
                                size_t count,
                                const GLenum *attachments,
                                const Rectangle &area);

    angle::Result clear(const Context *context, GLbitfield mask);
    angle::Result clearBufferfv(const Context *context,
                                GLenum buffer,
                                GLint drawbuffer,
                                const GLfloat *values);
    angle::Result clearBufferuiv(const Context *context,
                                 GLenum buffer,
                                 GLint drawbuffer,
                                 const GLuint *values);
    angle::Result clearBufferiv(const Context *context,
                                GLenum buffer,
                                GLint drawbuffer,
                                const GLint *values);
    angle::Result clearBufferfi(const Context *context,
                                GLenum buffer,
                                GLint drawbuffer,
                                GLfloat depth,
                                GLint stencil);

    GLenum getImplementationColorReadFormat(const Context *context);
    GLenum getImplementationColorReadType(const Context *context);

    angle::Result readPixels(const Context *context,
                             const Rectangle &area,
                             GLenum format,
                             GLenum type,
                             const PixelPackState &pack,
                             Buffer *packBuffer,
                             void *pixels);

    angle::Result blit(const Context *context,
                       const Rectangle &sourceArea,
                       const Rectangle &destArea,
                       GLbitfield mask,
                       GLenum filter);
    bool isDefault() const { return mState.isDefault(); }

    enum DirtyBitType : size_t
    {
        DIRTY_BIT_COLOR_ATTACHMENT_0,
        DIRTY_BIT_COLOR_ATTACHMENT_MAX =
            DIRTY_BIT_COLOR_ATTACHMENT_0 + IMPLEMENTATION_MAX_DRAW_BUFFERS,
        DIRTY_BIT_DEPTH_ATTACHMENT = DIRTY_BIT_COLOR_ATTACHMENT_MAX,
        DIRTY_BIT_STENCIL_ATTACHMENT,
        DIRTY_BIT_COLOR_BUFFER_CONTENTS_0,
        DIRTY_BIT_COLOR_BUFFER_CONTENTS_MAX =
            DIRTY_BIT_COLOR_BUFFER_CONTENTS_0 + IMPLEMENTATION_MAX_DRAW_BUFFERS,
        DIRTY_BIT_DEPTH_BUFFER_CONTENTS = DIRTY_BIT_COLOR_BUFFER_CONTENTS_MAX,
        DIRTY_BIT_STENCIL_BUFFER_CONTENTS,
        DIRTY_BIT_DRAW_BUFFERS,
        DIRTY_BIT_READ_BUFFER,
        DIRTY_BIT_DEFAULT_WIDTH,
        DIRTY_BIT_DEFAULT_HEIGHT,
        DIRTY_BIT_DEFAULT_SAMPLES,
        DIRTY_BIT_DEFAULT_FIXED_SAMPLE_LOCATIONS,
        DIRTY_BIT_DEFAULT_LAYERS,
        DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE,
        DIRTY_BIT_FLIP_Y,
        DIRTY_BIT_FOVEATION,
        DIRTY_BIT_UNKNOWN,
        DIRTY_BIT_MAX = DIRTY_BIT_UNKNOWN
    };

    using DirtyBits = angle::BitSet<DIRTY_BIT_MAX>;
    bool hasAnyDirtyBit() const { return mDirtyBits.any(); }

    DrawBufferMask getActiveFloat32ColorAttachmentDrawBufferMask() const
    {
        return mFloat32ColorAttachmentBits & getDrawBufferMask();
    }

    DrawBufferMask getActiveSharedExponentColorAttachmentDrawBufferMask() const
    {
        return mSharedExponentColorAttachmentBits & getDrawBufferMask();
    }

    bool hasResourceThatNeedsInit() const { return mState.mResourceNeedsInit.any(); }

    angle::Result syncState(const Context *context,
                            GLenum framebufferBinding,
                            Command command) const;

    void setWriteControlMode(SrgbWriteControlMode srgbWriteControlMode);

    // Observer implementation
    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    bool formsRenderingFeedbackLoopWith(const Context *context) const;
    bool formsCopyingFeedbackLoopWith(TextureID copyTextureID,
                                      GLint copyTextureLevel,
                                      GLint copyTextureLayer) const;

    angle::Result ensureClearAttachmentsInitialized(const Context *context, GLbitfield mask);
    angle::Result ensureClearBufferAttachmentsInitialized(const Context *context,
                                                          GLenum buffer,
                                                          GLint drawbuffer);
    angle::Result ensureDrawAttachmentsInitialized(const Context *context);

    // Conservatively initializes both read color and depth. Blit can access the depth buffer.
    angle::Result ensureReadAttachmentsInitialized(const Context *context);
    Box getDimensions() const;

    // ANGLE_shader_pixel_local_storage.
    // Lazily creates a PixelLocalStorage object for this Framebuffer.
    PixelLocalStorage &getPixelLocalStorage(const Context *);
    // Returns nullptr if the pixel local storage object has not been created yet.
    PixelLocalStorage *peekPixelLocalStorage() const { return mPixelLocalStorage.get(); }
    // Detaches the the pixel local storage object so the Context can call deleteContextObjects().
    std::unique_ptr<PixelLocalStorage> detachPixelLocalStorage();

    static const FramebufferID kDefaultDrawFramebufferHandle;

  private:
    bool detachResourceById(Context *context, GLenum resourceType, GLuint resourceId);
    bool detachMatchingAttachment(Context *context,
                                  FramebufferAttachment *attachment,
                                  GLenum matchType,
                                  GLuint matchId);
    FramebufferStatus checkStatusWithGLFrontEnd(const Context *context) const;
    const FramebufferStatus &checkStatusImpl(const Context *context) const;
    void setAttachment(const Context *context,
                       GLenum type,
                       GLenum binding,
                       const ImageIndex &textureIndex,
                       FramebufferAttachmentObject *resource,
                       GLsizei numViews,
                       GLuint baseViewIndex,
                       bool isMultiview,
                       GLsizei samplesIn);
    void commitWebGL1DepthStencilIfConsistent(const Context *context,
                                              GLsizei numViews,
                                              GLuint baseViewIndex,
                                              bool isMultiview,
                                              GLsizei samples);
    void setAttachmentImpl(const Context *context,
                           GLenum type,
                           GLenum binding,
                           const ImageIndex &textureIndex,
                           FramebufferAttachmentObject *resource,
                           GLsizei numViews,
                           GLuint baseViewIndex,
                           bool isMultiview,
                           GLsizei samples);
    void updateAttachment(const Context *context,
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
                          GLsizei samples);

    void markAttachmentsInitialized(const DrawBufferMask &color, bool depth, bool stencil);

    // Checks that we have a partially masked clear:
    // * some color channels are masked out
    // * some stencil values are masked out
    // * scissor test partially overlaps the framebuffer
    bool partialClearNeedsInit(const Context *context, bool color, bool depth, bool stencil);
    bool partialBufferClearNeedsInit(const Context *context, GLenum bufferType);

    FramebufferAttachment *getAttachmentFromSubjectIndex(angle::SubjectIndex index);

    ANGLE_INLINE void updateFloat32AndSharedExponentColorAttachmentBits(
        size_t index,
        const InternalFormat *format)
    {
        mFloat32ColorAttachmentBits.set(index, format->type == GL_FLOAT);
        mSharedExponentColorAttachmentBits.set(index, format->type == GL_UNSIGNED_INT_5_9_9_9_REV);
    }

    angle::Result syncAllDrawAttachmentState(const Context *context, Command command) const;
    angle::Result syncAttachmentState(const Context *context,
                                      Command command,
                                      const FramebufferAttachment *attachment) const;

    FramebufferState mState;
    rx::FramebufferImpl *mImpl;

    mutable Optional<FramebufferStatus> mCachedStatus;
    DrawBuffersVector<angle::ObserverBinding> mDirtyColorAttachmentBindings;
    angle::ObserverBinding mDirtyDepthAttachmentBinding;
    angle::ObserverBinding mDirtyStencilAttachmentBinding;

    mutable DirtyBits mDirtyBits;
    DrawBufferMask mFloat32ColorAttachmentBits;
    DrawBufferMask mSharedExponentColorAttachmentBits;

    // The dirty bits guard is checked when we get a dependent state change message. We verify that
    // we don't set a dirty bit that isn't already set, when inside the dirty bits syncState.
    mutable Optional<DirtyBits> mDirtyBitsGuard;

    // ANGLE_shader_pixel_local_storage
    std::unique_ptr<PixelLocalStorage> mPixelLocalStorage;

    // QCOM_framebuffer_foveated
    bool mAttachmentChangedAfterEnablingFoveation;
};

inline bool FramebufferState::isDefault() const
{
    return mId == Framebuffer::kDefaultDrawFramebufferHandle;
}

using UniqueFramebufferPointer = angle::UniqueObjectPointer<Framebuffer, Context>;

}  // namespace gl

#endif  // LIBANGLE_FRAMEBUFFER_H_
