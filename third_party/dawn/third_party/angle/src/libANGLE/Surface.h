//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Surface.h: Defines the egl::Surface class, representing a drawing surface
// such as the client area of a window, including any back buffers.
// Implements EGLSurface and related functionality. [EGL 1.4] section 2.2 page 3.

#ifndef LIBANGLE_SURFACE_H_
#define LIBANGLE_SURFACE_H_

#include <memory>

#include <EGL/egl.h>

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/SurfaceImpl.h"

namespace gl
{
class Context;
class Framebuffer;
class Texture;
}  // namespace gl

namespace rx
{
class EGLImplFactory;
}

namespace egl
{
class Display;
struct Config;

using SupportedCompositorTiming = angle::PackedEnumBitSet<CompositorTiming>;
using SupportedTimestamps       = angle::PackedEnumBitSet<Timestamp>;

struct SurfaceState final : private angle::NonCopyable
{
    SurfaceState(SurfaceID idIn, const egl::Config *configIn, const AttributeMap &attributesIn);
    ~SurfaceState();

    bool isRobustResourceInitEnabled() const;
    bool hasProtectedContent() const;

    SurfaceID id;

    EGLLabelKHR label;
    const egl::Config *config;
    AttributeMap attributes;

    bool timestampsEnabled;
    bool autoRefreshEnabled;
    SupportedCompositorTiming supportedCompositorTimings;
    SupportedTimestamps supportedTimestamps;
    bool directComposition;
    EGLenum swapBehavior;
    EGLint swapInterval;
};

class Surface : public LabeledObject, public gl::FramebufferAttachmentObject
{
  public:
    rx::SurfaceImpl *getImplementation() const { return mImplementation; }

    void setLabel(EGLLabelKHR label) override;
    EGLLabelKHR getLabel() const override;

    EGLint getType() const;

    Error initialize(const Display *display);
    Error makeCurrent(const gl::Context *context);
    Error unMakeCurrent(const gl::Context *context);
    Error prepareSwap(const gl::Context *context);
    Error swap(gl::Context *context);
    Error swapWithDamage(gl::Context *context, const EGLint *rects, EGLint n_rects);
    Error swapWithFrameToken(gl::Context *context, EGLFrameTokenANGLE frameToken);
    Error postSubBuffer(const gl::Context *context,
                        EGLint x,
                        EGLint y,
                        EGLint width,
                        EGLint height);
    Error setPresentationTime(EGLnsecsANDROID time);
    Error querySurfacePointerANGLE(EGLint attribute, void **value);
    Error bindTexImage(gl::Context *context, gl::Texture *texture, EGLint buffer);
    Error releaseTexImage(const gl::Context *context, EGLint buffer);

    Error getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc);
    Error getMscRate(EGLint *numerator, EGLint *denominator);

    EGLint isPostSubBufferSupported() const;

    void setRequestedSwapInterval(EGLint interval);
    void setSwapInterval(const Display *display, EGLint interval);
    Error onDestroy(const Display *display);

    void setMipmapLevel(EGLint level);
    void setMultisampleResolve(EGLenum resolve);
    void setSwapBehavior(EGLenum behavior);

    void setFixedWidth(EGLint width);
    void setFixedHeight(EGLint height);

    const Config *getConfig() const;

    // width and height can change with client window resizing
    EGLint getWidth() const;
    EGLint getHeight() const;
    // Note: windows cannot be resized on Android.  The approach requires
    // calling vkGetPhysicalDeviceSurfaceCapabilitiesKHR.  However, that is
    // expensive; and there are troublesome timing issues for other parts of
    // ANGLE (which cause test failures and crashes).  Therefore, a
    // special-Android-only path is created just for the querying of EGL_WIDTH
    // and EGL_HEIGHT.
    // https://issuetracker.google.com/issues/153329980
    egl::Error getUserWidth(const egl::Display *display, EGLint *value) const;
    egl::Error getUserHeight(const egl::Display *display, EGLint *value) const;
    EGLint getPixelAspectRatio() const;
    EGLenum getRenderBuffer() const;
    EGLenum getRequestedRenderBuffer() const;
    EGLenum getSwapBehavior() const;
    TextureFormat getTextureFormat() const;
    EGLenum getTextureTarget() const;
    bool getLargestPbuffer() const;
    EGLenum getGLColorspace() const;
    EGLenum getVGAlphaFormat() const;
    EGLenum getVGColorspace() const;
    bool getMipmapTexture() const;
    EGLint getMipmapLevel() const;
    EGLint getHorizontalResolution() const;
    EGLint getVerticalResolution() const;
    EGLenum getMultisampleResolve() const;
    bool hasProtectedContent() const override;
    bool hasFoveatedRendering() const override { return false; }
    const gl::FoveationState *getFoveationState() const override { return nullptr; }

    // For lock surface buffer
    EGLint getBitmapPitch() const;
    EGLint getBitmapOrigin() const;
    EGLint getRedOffset() const;
    EGLint getGreenOffset() const;
    EGLint getBlueOffset() const;
    EGLint getAlphaOffset() const;
    EGLint getLuminanceOffset() const;
    EGLint getBitmapPixelSize() const;
    EGLAttribKHR getBitmapPointer() const;
    EGLint getCompressionRate(const egl::Display *display) const;
    egl::Error lockSurfaceKHR(const egl::Display *display, const AttributeMap &attributes);
    egl::Error unlockSurfaceKHR(const egl::Display *display);

    bool isLocked() const;
    bool isCurrentOnAnyContext() const { return mIsCurrentOnAnyContext; }

    gl::Texture *getBoundTexture() const { return mTexture; }

    EGLint isFixedSize() const;

    // FramebufferAttachmentObject implementation
    gl::Extents getAttachmentSize(const gl::ImageIndex &imageIndex) const override;
    gl::Format getAttachmentFormat(GLenum binding, const gl::ImageIndex &imageIndex) const override;
    GLsizei getAttachmentSamples(const gl::ImageIndex &imageIndex) const override;
    bool isRenderable(const gl::Context *context,
                      GLenum binding,
                      const gl::ImageIndex &imageIndex) const override;
    bool isYUV() const override;
    bool isExternalImageWithoutIndividualSync() const override;
    bool hasFrontBufferUsage() const override;

    void onAttach(const gl::Context *context, rx::UniqueSerial framebufferSerial) override {}
    void onDetach(const gl::Context *context, rx::UniqueSerial framebufferSerial) override {}
    SurfaceID id() const { return mState.id; }
    GLuint getId() const override;

    EGLint getOrientation() const { return mOrientation; }

    bool directComposition() const { return mState.directComposition; }

    gl::InitState initState(GLenum binding, const gl::ImageIndex &imageIndex) const override;
    void setInitState(GLenum binding,
                      const gl::ImageIndex &imageIndex,
                      gl::InitState initState) override;

    bool isRobustResourceInitEnabled() const { return mRobustResourceInitialization; }

    const gl::Format &getBindTexImageFormat() const { return mColorFormat; }

    // EGL_ANDROID_get_frame_timestamps entry points
    void setTimestampsEnabled(bool enabled);
    bool isTimestampsEnabled() const;

    // EGL_ANDROID_front_buffer_auto_refresh entry points
    Error setAutoRefreshEnabled(bool enabled);

    const SupportedCompositorTiming &getSupportedCompositorTimings() const;
    Error getCompositorTiming(EGLint numTimestamps,
                              const EGLint *names,
                              EGLnsecsANDROID *values) const;

    Error getNextFrameId(EGLuint64KHR *frameId) const;
    const SupportedTimestamps &getSupportedTimestamps() const;
    Error getFrameTimestamps(EGLuint64KHR frameId,
                             EGLint numTimestamps,
                             const EGLint *timestamps,
                             EGLnsecsANDROID *values) const;

    // Returns the offset into the texture backing the surface if specified via texture offset
    // attributes (see EGL_ANGLE_d3d_texture_client_buffer extension). Returns zero offset
    // otherwise.
    const gl::Offset &getTextureOffset() const { return mTextureOffset; }

    Error getBufferAge(const gl::Context *context, EGLint *age);

    Error setRenderBuffer(EGLint renderBuffer);
    void setRequestedRenderBuffer(EGLint requestedRenderBuffer);

    bool bufferAgeQueriedSinceLastSwap() const { return mBufferAgeQueriedSinceLastSwap; }
    void setDamageRegion(const EGLint *rects, EGLint n_rects);
    bool isDamageRegionSet() const { return mIsDamageRegionSet; }

    void addRef() { mRefCount++; }
    void release()
    {
        ASSERT(mRefCount > 0);
        mRefCount--;
    }
    bool isReferenced() const { return mRefCount > 0; }

  protected:
    Surface(EGLint surfaceType,
            SurfaceID id,
            const egl::Config *config,
            const AttributeMap &attributes,
            bool forceRobustResourceInit,
            EGLenum buftype = EGL_NONE);
    ~Surface() override;
    rx::FramebufferAttachmentObjectImpl *getAttachmentImpl() const override;

    // ANGLE-only method, used internally
    friend class gl::Texture;
    Error releaseTexImageFromTexture(const gl::Context *context);

    SurfaceState mState;
    rx::SurfaceImpl *mImplementation;
    int mRefCount;
    bool mDestroyed;

    EGLint mType;
    EGLenum mBuftype;

    bool mPostSubBufferRequested;

    bool mLargestPbuffer;
    EGLenum mGLColorspace;
    EGLenum mVGAlphaFormat;
    EGLenum mVGColorspace;
    bool mMipmapTexture;
    EGLint mMipmapLevel;
    EGLint mHorizontalResolution;
    EGLint mVerticalResolution;
    EGLenum mMultisampleResolve;

    bool mFixedSize;
    size_t mFixedWidth;
    size_t mFixedHeight;

    bool mRobustResourceInitialization;

    TextureFormat mTextureFormat;
    EGLenum mTextureTarget;

    EGLint mPixelAspectRatio;        // Display aspect ratio
    EGLenum mRenderBuffer;           // Render buffer
    EGLenum mRequestedRenderBuffer;  // Requested render buffer

    EGLint mRequestedSwapInterval;

    EGLint mOrientation;

    // We don't use a binding pointer here. We don't ever want to own an orphaned texture. If a
    // Texture is deleted the Surface is unbound in onDestroy.
    gl::Texture *mTexture;

    gl::Format mColorFormat;
    gl::Format mDSFormat;

    gl::Offset mTextureOffset;

    bool mIsCurrentOnAnyContext;  // The surface is current to a context/client API
    uint8_t *mLockBufferPtr;      // Memory owned by backend.
    EGLint mLockBufferPitch;

    bool mBufferAgeQueriedSinceLastSwap;
    bool mIsDamageRegionSet;

  private:
    Error getBufferAgeImpl(const gl::Context *context, EGLint *age) const;

    Error destroyImpl(const Display *display);

    void postSwap(const gl::Context *context);
    Error releaseRef(const Display *display);

    // ObserverInterface implementation.
    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    Error updatePropertiesOnSwap(const gl::Context *context);

    gl::InitState mColorInitState;
    gl::InitState mDepthStencilInitState;
    angle::ObserverBinding mImplObserverBinding;
};

class WindowSurface final : public Surface
{
  public:
    WindowSurface(rx::EGLImplFactory *implFactory,
                  SurfaceID id,
                  const Config *config,
                  EGLNativeWindowType window,
                  const AttributeMap &attribs,
                  bool robustResourceInit);
    ~WindowSurface() override;
};

class PbufferSurface final : public Surface
{
  public:
    PbufferSurface(rx::EGLImplFactory *implFactory,
                   SurfaceID id,
                   const Config *config,
                   const AttributeMap &attribs,
                   bool robustResourceInit);
    PbufferSurface(rx::EGLImplFactory *implFactory,
                   SurfaceID id,
                   const Config *config,
                   EGLenum buftype,
                   EGLClientBuffer clientBuffer,
                   const AttributeMap &attribs,
                   bool robustResourceInit);

  protected:
    ~PbufferSurface() override;
};

class PixmapSurface final : public Surface
{
  public:
    PixmapSurface(rx::EGLImplFactory *implFactory,
                  SurfaceID id,
                  const Config *config,
                  NativePixmapType nativePixmap,
                  const AttributeMap &attribs,
                  bool robustResourceInit);

  protected:
    ~PixmapSurface() override;
};

class [[nodiscard]] ScopedSurfaceRef
{
  public:
    ScopedSurfaceRef(Surface *surface) : mSurface(surface)
    {
        if (mSurface)
        {
            mSurface->addRef();
        }
    }
    ~ScopedSurfaceRef()
    {
        if (mSurface)
        {
            mSurface->release();
        }
    }

  private:
    Surface *const mSurface;
};

class SurfaceDeleter final
{
  public:
    SurfaceDeleter(const Display *display);
    ~SurfaceDeleter();
    void operator()(Surface *surface);

  private:
    const Display *mDisplay;
};

using SurfacePointer = std::unique_ptr<Surface, SurfaceDeleter>;

}  // namespace egl

#endif  // LIBANGLE_SURFACE_H_
