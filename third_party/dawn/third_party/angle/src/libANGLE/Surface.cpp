//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Surface.cpp: Implements the egl::Surface class, representing a drawing surface
// such as the client area of a window, including any back buffers.
// Implements EGLSurface and related functionality. [EGL 1.4] section 2.2 page 3.

#include "libANGLE/Surface.h"

#include <EGL/eglext.h>

#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Texture.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/EGLImplFactory.h"
#include "libANGLE/trace.h"

namespace egl
{
namespace
{
angle::SubjectIndex kSurfaceImplSubjectIndex = 0;
}  // namespace

SurfaceState::SurfaceState(SurfaceID idIn,
                           const egl::Config *configIn,
                           const AttributeMap &attributesIn)
    : id(idIn),
      label(nullptr),
      config((configIn != nullptr) ? new egl::Config(*configIn) : nullptr),
      attributes(attributesIn),
      timestampsEnabled(false),
      autoRefreshEnabled(false),
      directComposition(false),
      swapBehavior(EGL_NONE),
      swapInterval(0)
{
    directComposition = attributes.get(EGL_DIRECT_COMPOSITION_ANGLE, EGL_FALSE) == EGL_TRUE;
    swapInterval      = attributes.getAsInt(EGL_SWAP_INTERVAL_ANGLE, 1);
}

SurfaceState::~SurfaceState()
{
    delete config;
}

bool SurfaceState::isRobustResourceInitEnabled() const
{
    return attributes.get(EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE, EGL_FALSE) == EGL_TRUE;
}

bool SurfaceState::hasProtectedContent() const
{
    return attributes.get(EGL_PROTECTED_CONTENT_EXT, EGL_FALSE) == EGL_TRUE;
}

Surface::Surface(EGLint surfaceType,
                 SurfaceID id,
                 const egl::Config *config,
                 const AttributeMap &attributes,
                 bool forceRobustResourceInit,
                 EGLenum buftype)
    : FramebufferAttachmentObject(),
      mState(id, config, attributes),
      mImplementation(nullptr),
      mRefCount(0),
      mDestroyed(false),
      mType(surfaceType),
      mBuftype(buftype),
      mPostSubBufferRequested(false),
      mLargestPbuffer(false),
      mGLColorspace(EGL_GL_COLORSPACE_LINEAR),
      mVGAlphaFormat(EGL_VG_ALPHA_FORMAT_NONPRE),
      mVGColorspace(EGL_VG_COLORSPACE_sRGB),
      mMipmapTexture(false),
      mMipmapLevel(0),
      mHorizontalResolution(EGL_UNKNOWN),
      mVerticalResolution(EGL_UNKNOWN),
      mMultisampleResolve(EGL_MULTISAMPLE_RESOLVE_DEFAULT),
      mFixedSize(false),
      mFixedWidth(0),
      mFixedHeight(0),
      mTextureFormat(TextureFormat::NoTexture),
      mTextureTarget(EGL_NO_TEXTURE),
      // FIXME: Determine actual pixel aspect ratio
      mPixelAspectRatio(static_cast<EGLint>(1.0 * EGL_DISPLAY_SCALING)),
      mRenderBuffer(EGL_BACK_BUFFER),
      mRequestedRenderBuffer(EGL_BACK_BUFFER),
      mRequestedSwapInterval(mState.swapInterval),
      mOrientation(0),
      mTexture(nullptr),
      mColorFormat(config->renderTargetFormat),
      mDSFormat(config->depthStencilFormat),
      mIsCurrentOnAnyContext(false),
      mLockBufferPtr(nullptr),
      mLockBufferPitch(0),
      mBufferAgeQueriedSinceLastSwap(false),
      mIsDamageRegionSet(false),
      mColorInitState(gl::InitState::Initialized),
      mDepthStencilInitState(gl::InitState::Initialized),
      mImplObserverBinding(this, kSurfaceImplSubjectIndex)
{
    mPostSubBufferRequested =
        (attributes.get(EGL_POST_SUB_BUFFER_SUPPORTED_NV, EGL_FALSE) == EGL_TRUE);

    if (mType == EGL_PBUFFER_BIT)
    {
        mLargestPbuffer = (attributes.get(EGL_LARGEST_PBUFFER, EGL_FALSE) == EGL_TRUE);
    }

    if (mType == EGL_PIXMAP_BIT)
    {
        mRenderBuffer          = EGL_SINGLE_BUFFER;
        mRequestedRenderBuffer = EGL_SINGLE_BUFFER;
    }

    if (mType == EGL_WINDOW_BIT)
    {
        mRenderBuffer          = mState.attributes.getAsInt(EGL_RENDER_BUFFER, EGL_BACK_BUFFER);
        mRequestedRenderBuffer = mRenderBuffer;
    }

    mGLColorspace =
        static_cast<EGLenum>(attributes.get(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR));
    mVGAlphaFormat =
        static_cast<EGLenum>(attributes.get(EGL_VG_ALPHA_FORMAT, EGL_VG_ALPHA_FORMAT_NONPRE));
    mVGColorspace = static_cast<EGLenum>(attributes.get(EGL_VG_COLORSPACE, EGL_VG_COLORSPACE_sRGB));
    mMipmapTexture = (attributes.get(EGL_MIPMAP_TEXTURE, EGL_FALSE) == EGL_TRUE);

    mRobustResourceInitialization =
        forceRobustResourceInit ||
        (attributes.get(EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE, EGL_FALSE) == EGL_TRUE);
    if (mRobustResourceInitialization)
    {
        mColorInitState        = gl::InitState::MayNeedInit;
        mDepthStencilInitState = gl::InitState::MayNeedInit;
    }

    mFixedSize = (attributes.get(EGL_FIXED_SIZE_ANGLE, EGL_FALSE) == EGL_TRUE);
    if (mFixedSize)
    {
        mFixedWidth  = static_cast<size_t>(attributes.get(EGL_WIDTH, 0));
        mFixedHeight = static_cast<size_t>(attributes.get(EGL_HEIGHT, 0));
    }

    if (mType != EGL_WINDOW_BIT)
    {
        mTextureFormat = attributes.getAsPackedEnum(EGL_TEXTURE_FORMAT, TextureFormat::NoTexture);
        mTextureTarget = static_cast<EGLenum>(attributes.get(EGL_TEXTURE_TARGET, EGL_NO_TEXTURE));
    }

    mOrientation = static_cast<EGLint>(attributes.get(EGL_SURFACE_ORIENTATION_ANGLE, 0));

    mTextureOffset.x = static_cast<int>(mState.attributes.get(EGL_TEXTURE_OFFSET_X_ANGLE, 0));
    mTextureOffset.y = static_cast<int>(mState.attributes.get(EGL_TEXTURE_OFFSET_Y_ANGLE, 0));
}

Surface::~Surface() {}

rx::FramebufferAttachmentObjectImpl *Surface::getAttachmentImpl() const
{
    return mImplementation;
}

Error Surface::destroyImpl(const Display *display)
{
    if (mImplementation)
    {
        mImplementation->destroy(display);
    }

    ASSERT(!mTexture);

    SafeDelete(mImplementation);

    delete this;
    return NoError();
}

void Surface::postSwap(const gl::Context *context)
{
    if (mRobustResourceInitialization && mState.swapBehavior != EGL_BUFFER_PRESERVED)
    {
        mColorInitState        = gl::InitState::MayNeedInit;
        mDepthStencilInitState = gl::InitState::MayNeedInit;
        onStateChange(angle::SubjectMessage::SubjectChanged);
    }

    mBufferAgeQueriedSinceLastSwap = false;

    mIsDamageRegionSet = false;
}

Error Surface::initialize(const Display *display)
{
    GLenum overrideRenderTargetFormat = mState.config->renderTargetFormat;

    // To account for color space differences, override the renderTargetFormat with the
    // non-linear format. If no suitable non-linear format was found, return
    // EGL_BAD_MATCH error
    if (!gl::ColorspaceFormatOverride(mGLColorspace, &overrideRenderTargetFormat))
    {
        return egl::EglBadMatch();
    }

    // If an override is required update mState.config as well
    if (mState.config->renderTargetFormat != overrideRenderTargetFormat)
    {
        egl::Config *overrideConfig        = new egl::Config(*(mState.config));
        overrideConfig->renderTargetFormat = overrideRenderTargetFormat;
        delete mState.config;
        mState.config = overrideConfig;

        mColorFormat = gl::Format(mState.config->renderTargetFormat);
        mDSFormat    = gl::Format(mState.config->depthStencilFormat);
    }

    ANGLE_TRY(mImplementation->initialize(display));

    // Initialized here since impl is nullptr in the constructor.
    // Must happen after implementation initialize for Android.
    mState.swapBehavior = mImplementation->getSwapBehavior();

    if (mBuftype == EGL_IOSURFACE_ANGLE)
    {
        GLenum internalFormat =
            static_cast<GLenum>(mState.attributes.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE));
        GLenum type = static_cast<GLenum>(mState.attributes.get(EGL_TEXTURE_TYPE_ANGLE));

        // GL_RGBA + GL_HALF_FLOAT is not a valid format/type combination in GLES like it is in
        // desktop GL. Adjust the frontend format to be sized RGBA16F.
        if (internalFormat == GL_RGBA && type == GL_HALF_FLOAT)
        {
            internalFormat = GL_RGBA16F;
        }
        mColorFormat = gl::Format(internalFormat, type);
    }
    if (mBuftype == EGL_D3D_TEXTURE_ANGLE)
    {
        const angle::Format *colorFormat = mImplementation->getD3DTextureColorFormat();
        ASSERT(colorFormat != nullptr);
        GLenum internalFormat = colorFormat->fboImplementationInternalFormat;
        mColorFormat          = gl::Format(internalFormat, colorFormat->componentType);
        mGLColorspace         = EGL_GL_COLORSPACE_LINEAR;
        if (mColorFormat.info->colorEncoding == GL_SRGB)
        {
            mGLColorspace = EGL_GL_COLORSPACE_SRGB;
        }
    }

    if (mType == EGL_WINDOW_BIT && display->getExtensions().getFrameTimestamps)
    {
        mState.supportedCompositorTimings = mImplementation->getSupportedCompositorTimings();
        mState.supportedTimestamps        = mImplementation->getSupportedTimestamps();
    }

    mImplObserverBinding.bind(mImplementation);

    return NoError();
}

Error Surface::makeCurrent(const gl::Context *context)
{
    if (isLocked())
    {
        return EglBadAccess();
    }
    ANGLE_TRY(mImplementation->makeCurrent(context));
    mIsCurrentOnAnyContext = true;
    addRef();
    return NoError();
}

Error Surface::unMakeCurrent(const gl::Context *context)
{
    ANGLE_TRY(mImplementation->unMakeCurrent(context));
    mIsCurrentOnAnyContext = false;
    return releaseRef(context->getDisplay());
}

Error Surface::releaseRef(const Display *display)
{
    ASSERT(mRefCount > 0);
    mRefCount--;
    if (mRefCount == 0 && mDestroyed)
    {
        ASSERT(display);
        return destroyImpl(display);
    }

    return NoError();
}

Error Surface::onDestroy(const Display *display)
{
    mDestroyed = true;
    if (mRefCount == 0)
    {
        return destroyImpl(display);
    }
    return NoError();
}

void Surface::setLabel(EGLLabelKHR label)
{
    mState.label = label;
}

EGLLabelKHR Surface::getLabel() const
{
    return mState.label;
}

EGLint Surface::getType() const
{
    return mType;
}

Error Surface::prepareSwap(const gl::Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "egl::Surface::prepareSwap");
    return mImplementation->prepareSwap(context);
}

Error Surface::swap(gl::Context *context)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "egl::Surface::swap");
    context->onPreSwap();

    context->getState().getOverlay()->onSwap();

    ANGLE_TRY(updatePropertiesOnSwap(context));
    ANGLE_TRY(mImplementation->swap(context));
    postSwap(context);
    return NoError();
}

Error Surface::swapWithDamage(gl::Context *context, const EGLint *rects, EGLint n_rects)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "egl::Surface::swapWithDamage");
    context->onPreSwap();

    context->getState().getOverlay()->onSwap();

    ANGLE_TRY(updatePropertiesOnSwap(context));
    ANGLE_TRY(mImplementation->swapWithDamage(context, rects, n_rects));
    postSwap(context);
    return NoError();
}

Error Surface::swapWithFrameToken(gl::Context *context, EGLFrameTokenANGLE frameToken)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "egl::Surface::swapWithFrameToken");
    context->onPreSwap();

    context->getState().getOverlay()->onSwap();

    ANGLE_TRY(updatePropertiesOnSwap(context));
    ANGLE_TRY(mImplementation->swapWithFrameToken(context, frameToken));
    postSwap(context);
    return NoError();
}

Error Surface::postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height)
{
    if (width == 0 || height == 0)
    {
        return egl::NoError();
    }

    context->getState().getOverlay()->onSwap();

    ANGLE_TRY(updatePropertiesOnSwap(context));
    ANGLE_TRY(mImplementation->postSubBuffer(context, x, y, width, height));
    postSwap(context);
    return NoError();
}

Error Surface::setPresentationTime(EGLnsecsANDROID time)
{
    return mImplementation->setPresentationTime(time);
}

Error Surface::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    return mImplementation->querySurfacePointerANGLE(attribute, value);
}

EGLint Surface::isPostSubBufferSupported() const
{
    return mPostSubBufferRequested && mImplementation->isPostSubBufferSupported();
}

void Surface::setRequestedSwapInterval(EGLint interval)
{
    mRequestedSwapInterval = interval;
}

void Surface::setSwapInterval(const Display *display, EGLint interval)
{
    mImplementation->setSwapInterval(display, interval);
    mState.swapInterval = interval;
}

void Surface::setMipmapLevel(EGLint level)
{
    // Level is set but ignored
    UNIMPLEMENTED();
    mMipmapLevel = level;
}

void Surface::setMultisampleResolve(EGLenum resolve)
{
    // Behaviour is set but ignored
    UNIMPLEMENTED();
    mMultisampleResolve = resolve;
}

void Surface::setSwapBehavior(EGLenum behavior)
{
    // Behaviour is set but ignored
    UNIMPLEMENTED();
    mState.swapBehavior = behavior;
}

void Surface::setFixedWidth(EGLint width)
{
    mFixedWidth = width;
    mImplementation->setFixedWidth(width);
}

void Surface::setFixedHeight(EGLint height)
{
    mFixedHeight = height;
    mImplementation->setFixedHeight(height);
}

const Config *Surface::getConfig() const
{
    return mState.config;
}

EGLint Surface::getPixelAspectRatio() const
{
    return mPixelAspectRatio;
}

EGLenum Surface::getRenderBuffer() const
{
    return mRenderBuffer;
}

EGLenum Surface::getRequestedRenderBuffer() const
{
    return mRequestedRenderBuffer;
}

EGLenum Surface::getSwapBehavior() const
{
    return mState.swapBehavior;
}

TextureFormat Surface::getTextureFormat() const
{
    return mTextureFormat;
}

EGLenum Surface::getTextureTarget() const
{
    return mTextureTarget;
}

bool Surface::getLargestPbuffer() const
{
    return mLargestPbuffer;
}

EGLenum Surface::getGLColorspace() const
{
    return mGLColorspace;
}

EGLenum Surface::getVGAlphaFormat() const
{
    return mVGAlphaFormat;
}

EGLenum Surface::getVGColorspace() const
{
    return mVGColorspace;
}

bool Surface::getMipmapTexture() const
{
    return mMipmapTexture;
}

EGLint Surface::getMipmapLevel() const
{
    return mMipmapLevel;
}

EGLint Surface::getHorizontalResolution() const
{
    return mHorizontalResolution;
}

EGLint Surface::getVerticalResolution() const
{
    return mVerticalResolution;
}

EGLenum Surface::getMultisampleResolve() const
{
    return mMultisampleResolve;
}

EGLint Surface::isFixedSize() const
{
    return mFixedSize;
}

EGLint Surface::getWidth() const
{
    return mFixedSize ? static_cast<EGLint>(mFixedWidth) : mImplementation->getWidth();
}

EGLint Surface::getHeight() const
{
    return mFixedSize ? static_cast<EGLint>(mFixedHeight) : mImplementation->getHeight();
}

egl::Error Surface::getUserWidth(const egl::Display *display, EGLint *value) const
{
    if (mFixedSize)
    {
        *value = static_cast<EGLint>(mFixedWidth);
        return NoError();
    }
    else
    {
        return mImplementation->getUserWidth(display, value);
    }
}

egl::Error Surface::getUserHeight(const egl::Display *display, EGLint *value) const
{
    if (mFixedSize)
    {
        *value = static_cast<EGLint>(mFixedHeight);
        return NoError();
    }
    else
    {
        return mImplementation->getUserHeight(display, value);
    }
}

Error Surface::bindTexImage(gl::Context *context, gl::Texture *texture, EGLint buffer)
{
    ASSERT(!mTexture);
    ANGLE_TRY(mImplementation->bindTexImage(context, texture, buffer));
    Surface *previousSurface = texture->getBoundSurface();
    if (previousSurface != nullptr)
    {
        ANGLE_TRY(previousSurface->releaseTexImage(context, buffer));
    }
    if (texture->bindTexImageFromSurface(context, this) == angle::Result::Stop)
    {
        return Error(EGL_BAD_SURFACE);
    }
    mTexture = texture;
    addRef();

    return NoError();
}

Error Surface::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    ASSERT(context);

    ANGLE_TRY(mImplementation->releaseTexImage(context, buffer));

    ASSERT(mTexture);
    ANGLE_TRY(ResultToEGL(mTexture->releaseTexImageFromSurface(context)));

    return releaseTexImageFromTexture(context);
}

Error Surface::getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc)
{
    return mImplementation->getSyncValues(ust, msc, sbc);
}

Error Surface::getMscRate(EGLint *numerator, EGLint *denominator)
{
    return mImplementation->getMscRate(numerator, denominator);
}

Error Surface::releaseTexImageFromTexture(const gl::Context *context)
{
    ASSERT(mTexture);
    mTexture = nullptr;
    return releaseRef(context->getDisplay());
}

gl::Extents Surface::getAttachmentSize(const gl::ImageIndex & /*target*/) const
{
    return gl::Extents(getWidth(), getHeight(), 1);
}

gl::Format Surface::getAttachmentFormat(GLenum binding, const gl::ImageIndex &target) const
{
    return (binding == GL_BACK ? mColorFormat : mDSFormat);
}

GLsizei Surface::getAttachmentSamples(const gl::ImageIndex &target) const
{
    return getConfig()->samples;
}

bool Surface::isRenderable(const gl::Context *context,
                           GLenum binding,
                           const gl::ImageIndex &imageIndex) const
{
    return true;
}

bool Surface::isYUV() const
{
    // EGL_EXT_yuv_surface is not implemented.
    return false;
}

bool Surface::isExternalImageWithoutIndividualSync() const
{
    return false;
}

bool Surface::hasFrontBufferUsage() const
{
    return false;
}

GLuint Surface::getId() const
{
    return mState.id.value;
}

Error Surface::getBufferAgeImpl(const gl::Context *context, EGLint *age) const
{
    // When EGL_BUFFER_PRESERVED, the previous frame contents are copied to
    // current frame, so the buffer age is always 1.
    if (mState.swapBehavior == EGL_BUFFER_PRESERVED)
    {
        if (age != nullptr)
        {
            *age = 1;
        }
        return egl::NoError();
    }
    return mImplementation->getBufferAge(context, age);
}

Error Surface::getBufferAge(const gl::Context *context, EGLint *age)
{
    Error err = getBufferAgeImpl(context, age);
    if (!err.isError())
    {
        mBufferAgeQueriedSinceLastSwap = true;
    }
    return err;
}

gl::InitState Surface::initState(GLenum binding, const gl::ImageIndex & /*imageIndex*/) const
{
    switch (binding)
    {
        case GL_BACK:
            return mColorInitState;
        case GL_DEPTH:
        case GL_STENCIL:
            return mDepthStencilInitState;
        default:
            UNREACHABLE();
            return gl::InitState::Initialized;
    }
}

void Surface::setInitState(GLenum binding,
                           const gl::ImageIndex & /*imageIndex*/,
                           gl::InitState initState)
{
    switch (binding)
    {
        case GL_BACK:
            mColorInitState = initState;
            break;
        case GL_DEPTH:
        case GL_STENCIL:
            mDepthStencilInitState = initState;
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void Surface::setTimestampsEnabled(bool enabled)
{
    mImplementation->setTimestampsEnabled(enabled);
    mState.timestampsEnabled = enabled;
}

bool Surface::isTimestampsEnabled() const
{
    return mState.timestampsEnabled;
}

Error Surface::setAutoRefreshEnabled(bool enabled)
{
    ANGLE_TRY(mImplementation->setAutoRefreshEnabled(enabled));
    mState.autoRefreshEnabled = enabled;
    return NoError();
}

bool Surface::hasProtectedContent() const
{
    return mState.hasProtectedContent();
}

const SupportedCompositorTiming &Surface::getSupportedCompositorTimings() const
{
    return mState.supportedCompositorTimings;
}

Error Surface::getCompositorTiming(EGLint numTimestamps,
                                   const EGLint *names,
                                   EGLnsecsANDROID *values) const
{
    return mImplementation->getCompositorTiming(numTimestamps, names, values);
}

Error Surface::getNextFrameId(EGLuint64KHR *frameId) const
{
    return mImplementation->getNextFrameId(frameId);
}

const SupportedTimestamps &Surface::getSupportedTimestamps() const
{
    return mState.supportedTimestamps;
}

Error Surface::getFrameTimestamps(EGLuint64KHR frameId,
                                  EGLint numTimestamps,
                                  const EGLint *timestamps,
                                  EGLnsecsANDROID *values) const
{
    return mImplementation->getFrameTimestamps(frameId, numTimestamps, timestamps, values);
}

void Surface::onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message)
{
    ASSERT(index == kSurfaceImplSubjectIndex);
    switch (message)
    {
        case angle::SubjectMessage::SubjectChanged:
            onStateChange(angle::SubjectMessage::ContentsChanged);
            break;
        case angle::SubjectMessage::SurfaceChanged:
            onStateChange(angle::SubjectMessage::SurfaceChanged);
            break;
        case angle::SubjectMessage::SwapchainImageChanged:
            onStateChange(angle::SubjectMessage::SwapchainImageChanged);
            break;
        default:
            UNREACHABLE();
            break;
    }
}

Error Surface::setRenderBuffer(EGLint renderBuffer)
{
    ANGLE_TRY(mImplementation->setRenderBuffer(renderBuffer));
    mRenderBuffer = renderBuffer;
    return NoError();
}

void Surface::setRequestedRenderBuffer(EGLint requestedRenderBuffer)
{
    mRequestedRenderBuffer = requestedRenderBuffer;
}

Error Surface::updatePropertiesOnSwap(const gl::Context *context)
{
    if ((mRenderBuffer != mRequestedRenderBuffer) &&
        context->getDisplay()->getExtensions().mutableRenderBufferKHR &&
        (getConfig()->surfaceType & EGL_MUTABLE_RENDER_BUFFER_BIT_KHR))
    {
        ANGLE_TRY(setRenderBuffer(mRequestedRenderBuffer));
    }
    if (mState.swapInterval != mRequestedSwapInterval)
    {
        setSwapInterval(context->getDisplay(), mRequestedSwapInterval);
    }
    return NoError();
}

bool Surface::isLocked() const
{
    return (mLockBufferPtr != nullptr);
}

EGLint Surface::getBitmapPitch() const
{
    return mLockBufferPitch;
}

EGLint Surface::getBitmapOrigin() const
{
    return mImplementation->origin();
}

EGLint Surface::getRedOffset() const
{
    const gl::InternalFormat &format = *mColorFormat.info;
    if (gl::IsBGRAFormat(format.internalFormat))
    {
        return format.blueBits + format.greenBits;
    }
    else
    {
        return 0;
    }
}

EGLint Surface::getGreenOffset() const
{
    const gl::InternalFormat &format = *mColorFormat.info;
    if (gl::IsBGRAFormat(format.internalFormat))
    {
        return format.blueBits;
    }
    else
    {
        return format.redBits;
    }
}

EGLint Surface::getBlueOffset() const
{
    const gl::InternalFormat &format = *mColorFormat.info;
    if (gl::IsBGRAFormat(format.internalFormat))
    {
        return 0;
    }
    else
    {
        return format.redBits + format.greenBits;
    }
}

EGLint Surface::getAlphaOffset() const
{
    const gl::InternalFormat &format = *mColorFormat.info;
    if (format.isLUMA())
    {
        return format.luminanceBits;  // Luma always first, alpha optional
    }
    // For RGBA/BGRA alpha is last
    return format.blueBits + format.greenBits + format.redBits;
}

EGLint Surface::getLuminanceOffset() const
{
    return 0;
}

EGLint Surface::getBitmapPixelSize() const
{
    constexpr EGLint kBitsPerByte    = 8;
    const gl::InternalFormat &format = *mColorFormat.info;
    return (format.pixelBytes * kBitsPerByte);
}

EGLAttribKHR Surface::getBitmapPointer() const
{
    return static_cast<EGLAttribKHR>((intptr_t)mLockBufferPtr);
}

EGLint Surface::getCompressionRate(const egl::Display *display) const
{
    return mImplementation->getCompressionRate(display);
}

egl::Error Surface::lockSurfaceKHR(const egl::Display *display, const AttributeMap &attributes)
{
    EGLint lockBufferUsageHint = attributes.getAsInt(
        EGL_LOCK_USAGE_HINT_KHR, (EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR));

    bool preservePixels = ((attributes.getAsInt(EGL_MAP_PRESERVE_PIXELS_KHR, false) == EGL_TRUE) ||
                           (mState.swapBehavior == EGL_BUFFER_PRESERVED));

    return mImplementation->lockSurface(display, lockBufferUsageHint, preservePixels,
                                        &mLockBufferPtr, &mLockBufferPitch);
}

egl::Error Surface::unlockSurfaceKHR(const egl::Display *display)
{
    mLockBufferPtr   = nullptr;
    mLockBufferPitch = 0;
    return mImplementation->unlockSurface(display, true);
}

WindowSurface::WindowSurface(rx::EGLImplFactory *implFactory,
                             SurfaceID id,
                             const egl::Config *config,
                             EGLNativeWindowType window,
                             const AttributeMap &attribs,
                             bool robustResourceInit)
    : Surface(EGL_WINDOW_BIT, id, config, attribs, robustResourceInit)
{
    mImplementation = implFactory->createWindowSurface(mState, window, attribs);
}

void Surface::setDamageRegion(const EGLint *rects, EGLint n_rects)
{
    mIsDamageRegionSet = true;
}

WindowSurface::~WindowSurface() {}

PbufferSurface::PbufferSurface(rx::EGLImplFactory *implFactory,
                               SurfaceID id,
                               const Config *config,
                               const AttributeMap &attribs,
                               bool robustResourceInit)
    : Surface(EGL_PBUFFER_BIT, id, config, attribs, robustResourceInit)
{
    mImplementation = implFactory->createPbufferSurface(mState, attribs);
}

PbufferSurface::PbufferSurface(rx::EGLImplFactory *implFactory,
                               SurfaceID id,
                               const Config *config,
                               EGLenum buftype,
                               EGLClientBuffer clientBuffer,
                               const AttributeMap &attribs,
                               bool robustResourceInit)
    : Surface(EGL_PBUFFER_BIT, id, config, attribs, robustResourceInit, buftype)
{
    mImplementation =
        implFactory->createPbufferFromClientBuffer(mState, buftype, clientBuffer, attribs);
}

PbufferSurface::~PbufferSurface() {}

PixmapSurface::PixmapSurface(rx::EGLImplFactory *implFactory,
                             SurfaceID id,
                             const Config *config,
                             NativePixmapType nativePixmap,
                             const AttributeMap &attribs,
                             bool robustResourceInit)
    : Surface(EGL_PIXMAP_BIT, id, config, attribs, robustResourceInit)
{
    mImplementation = implFactory->createPixmapSurface(mState, nativePixmap, attribs);
}

PixmapSurface::~PixmapSurface() {}

// SurfaceDeleter implementation.

SurfaceDeleter::SurfaceDeleter(const Display *display) : mDisplay(display) {}

SurfaceDeleter::~SurfaceDeleter() {}

void SurfaceDeleter::operator()(Surface *surface)
{
    ANGLE_SWALLOW_ERR(surface->onDestroy(mDisplay));
}

}  // namespace egl
