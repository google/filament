//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceMtl.mm:
//    Implements the class methods for SurfaceMtl.
//

#include "libANGLE/renderer/metal/SurfaceMtl.h"

#include "common/platform.h"
#include "libANGLE/Display.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/FrameBufferMtl.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_utils.h"
#include "mtl_command_buffer.h"

// Compiler can turn on programmatical frame capture in release build by defining
// ANGLE_METAL_FRAME_CAPTURE flag.
#if defined(NDEBUG) && !defined(ANGLE_METAL_FRAME_CAPTURE)
#    define ANGLE_METAL_FRAME_CAPTURE_ENABLED 0
#else
#    define ANGLE_METAL_FRAME_CAPTURE_ENABLED 1
#endif

namespace rx
{

namespace
{

constexpr angle::FormatID kDefaultFrameBufferDepthFormatId   = angle::FormatID::D32_FLOAT;
constexpr angle::FormatID kDefaultFrameBufferStencilFormatId = angle::FormatID::S8_UINT;
constexpr angle::FormatID kDefaultFrameBufferDepthStencilFormatId =
    angle::FormatID::D24_UNORM_S8_UINT;

angle::Result CreateOrResizeTexture(const gl::Context *context,
                                    const mtl::Format &format,
                                    uint32_t width,
                                    uint32_t height,
                                    uint32_t samples,
                                    bool renderTargetOnly,
                                    mtl::TextureRef *textureOut)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    bool allowFormatView   = format.hasDepthAndStencilBits();
    if (*textureOut)
    {
        ANGLE_TRY((*textureOut)->resize(contextMtl, width, height));
        size_t resourceSize = EstimateTextureSizeInBytes(format, width, height, 1, samples, 1);
        if (*textureOut)
        {
            (*textureOut)->setEstimatedByteSize(resourceSize);
        }
    }
    else if (samples > 1)
    {
        ANGLE_TRY(mtl::Texture::Make2DMSTexture(contextMtl, format, width, height, samples,
                                                /** renderTargetOnly */ renderTargetOnly,
                                                /** allowFormatView */ allowFormatView,
                                                textureOut));
    }
    else
    {
        ANGLE_TRY(mtl::Texture::Make2DTexture(contextMtl, format, width, height, 1,
                                              /** renderTargetOnly */ renderTargetOnly,
                                              /** allowFormatView */ allowFormatView, textureOut));
    }
    return angle::Result::Continue;
}

}  // anonymous namespace

// SurfaceMtl implementation
SurfaceMtl::SurfaceMtl(DisplayMtl *display,
                       const egl::SurfaceState &state,
                       const egl::AttributeMap &attribs)
    : SurfaceImpl(state)
{
    mRobustResourceInit =
        attribs.get(EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE, EGL_FALSE) == EGL_TRUE;
    mColorFormat = display->getPixelFormat(angle::FormatID::B8G8R8A8_UNORM);

    mSamples = state.config->samples;

    int depthBits   = 0;
    int stencilBits = 0;
    if (state.config)
    {
        depthBits   = state.config->depthSize;
        stencilBits = state.config->stencilSize;
    }

    if (depthBits && stencilBits)
    {
        if (display->getFeatures().allowSeparateDepthStencilBuffers.enabled)
        {
            mDepthFormat   = display->getPixelFormat(kDefaultFrameBufferDepthFormatId);
            mStencilFormat = display->getPixelFormat(kDefaultFrameBufferStencilFormatId);
        }
        else
        {
            // We must use packed depth stencil
            mUsePackedDepthStencil = true;
            mDepthFormat   = display->getPixelFormat(kDefaultFrameBufferDepthStencilFormatId);
            mStencilFormat = mDepthFormat;
        }
    }
    else if (depthBits)
    {
        mDepthFormat = display->getPixelFormat(kDefaultFrameBufferDepthFormatId);
    }
    else if (stencilBits)
    {
        mStencilFormat = display->getPixelFormat(kDefaultFrameBufferStencilFormatId);
    }
}

SurfaceMtl::~SurfaceMtl() {}

void SurfaceMtl::destroy(const egl::Display *display)
{
    mColorTexture   = nullptr;
    mDepthTexture   = nullptr;
    mStencilTexture = nullptr;

    mMSColorTexture = nullptr;

    mColorRenderTarget.reset();
    mColorManualResolveRenderTarget.reset();
    mDepthRenderTarget.reset();
    mStencilRenderTarget.reset();
}

egl::Error SurfaceMtl::initialize(const egl::Display *display)
{
    return egl::NoError();
}

egl::Error SurfaceMtl::makeCurrent(const gl::Context *context)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    StartFrameCapture(contextMtl);

    return egl::NoError();
}

egl::Error SurfaceMtl::unMakeCurrent(const gl::Context *context)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    contextMtl->flushCommandBuffer(mtl::WaitUntilScheduled);

    StopFrameCapture();
    return egl::NoError();
}

egl::Error SurfaceMtl::swap(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error SurfaceMtl::postSubBuffer(const gl::Context *context,
                                     EGLint x,
                                     EGLint y,
                                     EGLint width,
                                     EGLint height)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

egl::Error SurfaceMtl::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

egl::Error SurfaceMtl::bindTexImage(const gl::Context *context, gl::Texture *texture, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

egl::Error SurfaceMtl::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

egl::Error SurfaceMtl::getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

egl::Error SurfaceMtl::getMscRate(EGLint *numerator, EGLint *denominator)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

void SurfaceMtl::setSwapInterval(const egl::Display *display, EGLint interval) {}

void SurfaceMtl::setFixedWidth(EGLint width)
{
    UNIMPLEMENTED();
}

void SurfaceMtl::setFixedHeight(EGLint height)
{
    UNIMPLEMENTED();
}

EGLint SurfaceMtl::getWidth() const
{
    if (mColorTexture)
    {
        return static_cast<EGLint>(mColorTexture->widthAt0());
    }
    return 0;
}

EGLint SurfaceMtl::getHeight() const
{
    if (mColorTexture)
    {
        return static_cast<EGLint>(mColorTexture->heightAt0());
    }
    return 0;
}

EGLint SurfaceMtl::isPostSubBufferSupported() const
{
    return EGL_FALSE;
}

EGLint SurfaceMtl::getSwapBehavior() const
{
    // dEQP-EGL.functional.query_surface.* requires that for a surface with swap
    // behavior=EGL_BUFFER_PRESERVED, config.surfaceType must contain
    // EGL_SWAP_BEHAVIOR_PRESERVED_BIT.
    // Since we don't support EGL_SWAP_BEHAVIOR_PRESERVED_BIT in egl::Config for now, let's just use
    // EGL_BUFFER_DESTROYED as default swap behavior.
    return EGL_BUFFER_DESTROYED;
}

angle::Result SurfaceMtl::initializeContents(const gl::Context *context,
                                             GLenum binding,
                                             const gl::ImageIndex &imageIndex)
{
    ASSERT(mColorTexture);

    ContextMtl *contextMtl = mtl::GetImpl(context);

    // Use loadAction=clear
    mtl::RenderPassDesc rpDesc;
    rpDesc.rasterSampleCount = mColorTexture->samples();

    switch (binding)
    {
        case GL_BACK:
        {
            if (mColorTextureInitialized)
            {
                return angle::Result::Continue;
            }
            rpDesc.numColorAttachments = 1;
            mColorRenderTarget.toRenderPassAttachmentDesc(&rpDesc.colorAttachments[0]);
            rpDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
            MTLClearColor black                   = {};
            rpDesc.colorAttachments[0].clearColor =
                mtl::EmulatedAlphaClearColor(black, mColorTexture->getColorWritableMask());
            mColorTextureInitialized = true;
            break;
        }
        case GL_DEPTH:
        case GL_STENCIL:
        {
            if (mDepthStencilTexturesInitialized)
            {
                return angle::Result::Continue;
            }
            if (mDepthTexture)
            {
                mDepthRenderTarget.toRenderPassAttachmentDesc(&rpDesc.depthAttachment);
                rpDesc.depthAttachment.loadAction = MTLLoadActionClear;
            }
            if (mStencilTexture)
            {
                mStencilRenderTarget.toRenderPassAttachmentDesc(&rpDesc.stencilAttachment);
                rpDesc.stencilAttachment.loadAction = MTLLoadActionClear;
            }
            mDepthStencilTexturesInitialized = true;
            break;
        }
        default:
            UNREACHABLE();
            break;
    }
    mtl::RenderCommandEncoder *encoder = contextMtl->getRenderPassCommandEncoder(rpDesc);
    encoder->setStoreAction(MTLStoreActionStore);

    return angle::Result::Continue;
}

angle::Result SurfaceMtl::getAttachmentRenderTarget(const gl::Context *context,
                                                    GLenum binding,
                                                    const gl::ImageIndex &imageIndex,
                                                    GLsizei samples,
                                                    FramebufferAttachmentRenderTarget **rtOut)
{
    ASSERT(mColorTexture);

    switch (binding)
    {
        case GL_BACK:
            *rtOut = &mColorRenderTarget;
            break;
        case GL_DEPTH:
            *rtOut = mDepthFormat.valid() ? &mDepthRenderTarget : nullptr;
            break;
        case GL_STENCIL:
            *rtOut = mStencilFormat.valid() ? &mStencilRenderTarget : nullptr;
            break;
        case GL_DEPTH_STENCIL:
            // NOTE(hqle): ES 3.0 feature
            UNREACHABLE();
            break;
    }

    return angle::Result::Continue;
}

egl::Error SurfaceMtl::attachToFramebuffer(const gl::Context *context, gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

egl::Error SurfaceMtl::detachFromFramebuffer(const gl::Context *context,
                                             gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

angle::Result SurfaceMtl::ensureCompanionTexturesSizeCorrect(const gl::Context *context,
                                                             const gl::Extents &size)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);

    ASSERT(mColorTexture);

    if (mSamples > 1 && (!mMSColorTexture || mMSColorTexture->sizeAt0() != size))
    {
        mAutoResolveMSColorTexture =
            contextMtl->getDisplay()->getFeatures().allowMultisampleStoreAndResolve.enabled;
        ANGLE_TRY(CreateOrResizeTexture(context, mColorFormat, size.width, size.height, mSamples,
                                        /** renderTargetOnly */ mAutoResolveMSColorTexture,
                                        &mMSColorTexture));

        if (mAutoResolveMSColorTexture)
        {
            // Use auto MSAA resolve at the end of render pass.
            mColorRenderTarget.setImplicitMSTexture(mMSColorTexture);
        }
        else
        {
            mColorRenderTarget.setTexture(mMSColorTexture);
        }
    }

    if (mDepthFormat.valid() && (!mDepthTexture || mDepthTexture->sizeAt0() != size))
    {
        ANGLE_TRY(CreateOrResizeTexture(context, mDepthFormat, size.width, size.height, mSamples,
                                        /** renderTargetOnly */ false, &mDepthTexture));

        mDepthRenderTarget.set(mDepthTexture, mtl::kZeroNativeMipLevel, 0, mDepthFormat);
        // Robust resource init: should initialize depth to 1.0.
        mDepthStencilTexturesInitialized = false;
    }

    if (mStencilFormat.valid() && (!mStencilTexture || mStencilTexture->sizeAt0() != size))
    {
        if (mUsePackedDepthStencil)
        {
            mStencilTexture = mDepthTexture;
        }
        else
        {
            ANGLE_TRY(CreateOrResizeTexture(context, mStencilFormat, size.width, size.height,
                                            mSamples,
                                            /** renderTargetOnly */ false, &mStencilTexture));
        }

        mStencilRenderTarget.set(mStencilTexture, mtl::kZeroNativeMipLevel, 0, mStencilFormat);
        // Robust resource init: should initialize stencil to zero.
        mDepthStencilTexturesInitialized = false;
    }

    return angle::Result::Continue;
}

angle::Result SurfaceMtl::resolveColorTextureIfNeeded(const gl::Context *context)
{
    ASSERT(mMSColorTexture);
    if (!mAutoResolveMSColorTexture)
    {
        // Manually resolve texture
        ContextMtl *contextMtl = mtl::GetImpl(context);

        mColorManualResolveRenderTarget.set(mColorTexture, mtl::kZeroNativeMipLevel, 0,
                                            mColorFormat);
        mtl::RenderCommandEncoder *encoder =
            contextMtl->getRenderTargetCommandEncoder(mColorManualResolveRenderTarget);
        ANGLE_TRY(contextMtl->getDisplay()->getUtils().blitColorWithDraw(
            context, encoder, mColorFormat.actualAngleFormat(), mMSColorTexture));
        contextMtl->endEncoding(true);
        mColorManualResolveRenderTarget.reset();
    }
    return angle::Result::Continue;
}

// WindowSurfaceMtl implementation.
WindowSurfaceMtl::WindowSurfaceMtl(DisplayMtl *display,
                                   const egl::SurfaceState &state,
                                   EGLNativeWindowType window,
                                   const egl::AttributeMap &attribs)
    : SurfaceMtl(display, state, attribs), mLayer((__bridge CALayer *)(window))
{
    // NOTE(hqle): Width and height attributes is ignored for now.
    mCurrentKnownDrawableSize = CGSizeMake(0, 0);
}

WindowSurfaceMtl::~WindowSurfaceMtl() {}

void WindowSurfaceMtl::destroy(const egl::Display *display)
{
    SurfaceMtl::destroy(display);

    mCurrentDrawable = nil;
    if (mMetalLayer && mMetalLayer.get() != mLayer)
    {
        // If we created metal layer in WindowSurfaceMtl::initialize(),
        // we need to detach it from super layer now.
        [mMetalLayer.get() removeFromSuperlayer];
    }
    mMetalLayer = nil;
}

egl::Error WindowSurfaceMtl::initialize(const egl::Display *display)
{
    egl::Error re = SurfaceMtl::initialize(display);
    if (re.isError())
    {
        return re;
    }

    DisplayMtl *displayMtl    = mtl::GetImpl(display);
    id<MTLDevice> metalDevice = displayMtl->getMetalDevice();

    StartFrameCapture(metalDevice, displayMtl->cmdQueue().get());

    ANGLE_MTL_OBJC_SCOPE
    {
        if ([mLayer isKindOfClass:CAMetalLayer.class])
        {
            mMetalLayer = static_cast<CAMetalLayer *>(mLayer);
        }
        else
        {
            mMetalLayer             = mtl::adoptObjCPtr([[CAMetalLayer alloc] init]);
            mMetalLayer.get().frame = mLayer.frame;
        }

        mMetalLayer.get().device          = metalDevice;
        mMetalLayer.get().pixelFormat     = mColorFormat.metalFormat;
        mMetalLayer.get().framebufferOnly = NO;  // Support blitting and glReadPixels

#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        // Autoresize with parent layer.
        mMetalLayer.get().autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
#endif
        if (mMetalLayer.get() != mLayer)
        {
            mMetalLayer.get().contentsScale = mLayer.contentsScale;

            [mLayer addSublayer:mMetalLayer.get()];
        }

        // ensure drawableSize is set to correct value:
        mMetalLayer.get().drawableSize = mCurrentKnownDrawableSize = calcExpectedDrawableSize();
    }

    return egl::NoError();
}

egl::Error WindowSurfaceMtl::swap(const gl::Context *context)
{
    ANGLE_TO_EGL_TRY(swapImpl(context));

    return egl::NoError();
}

void WindowSurfaceMtl::setSwapInterval(const egl::Display *display, EGLint interval)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    mMetalLayer.get().displaySyncEnabled = interval != 0;
#endif
}

// width and height can change with client window resizing
EGLint WindowSurfaceMtl::getWidth() const
{
    return static_cast<EGLint>(mCurrentKnownDrawableSize.width);
}

EGLint WindowSurfaceMtl::getHeight() const
{
    return static_cast<EGLint>(mCurrentKnownDrawableSize.height);
}

EGLint WindowSurfaceMtl::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

angle::Result WindowSurfaceMtl::initializeContents(const gl::Context *context,
                                                   GLenum binding,
                                                   const gl::ImageIndex &imageIndex)
{
    ANGLE_TRY(ensureCurrentDrawableObtained(context));
    return SurfaceMtl::initializeContents(context, binding, imageIndex);
}

angle::Result WindowSurfaceMtl::getAttachmentRenderTarget(const gl::Context *context,
                                                          GLenum binding,
                                                          const gl::ImageIndex &imageIndex,
                                                          GLsizei samples,
                                                          FramebufferAttachmentRenderTarget **rtOut)
{
    ANGLE_TRY(ensureCurrentDrawableObtained(context));
    ANGLE_TRY(ensureCompanionTexturesSizeCorrect(context));

    return SurfaceMtl::getAttachmentRenderTarget(context, binding, imageIndex, samples, rtOut);
}

egl::Error WindowSurfaceMtl::attachToFramebuffer(const gl::Context *context,
                                                 gl::Framebuffer *framebuffer)
{
    FramebufferMtl *framebufferMtl = GetImplAs<FramebufferMtl>(framebuffer);
    ASSERT(!framebufferMtl->getBackbuffer());
    framebufferMtl->setBackbuffer(this);
    framebufferMtl->setFlipY(true);
    return egl::NoError();
}

egl::Error WindowSurfaceMtl::detachFromFramebuffer(const gl::Context *context,
                                                   gl::Framebuffer *framebuffer)
{
    FramebufferMtl *framebufferMtl = GetImplAs<FramebufferMtl>(framebuffer);
    ASSERT(framebufferMtl->getBackbuffer() == this);
    framebufferMtl->setBackbuffer(nullptr);
    framebufferMtl->setFlipY(false);
    return egl::NoError();
}

angle::Result WindowSurfaceMtl::ensureCurrentDrawableObtained(const gl::Context *context)
{
    if (!mCurrentDrawable)
    {
        ANGLE_TRY(obtainNextDrawable(context));
    }

    return angle::Result::Continue;
}

angle::Result WindowSurfaceMtl::ensureCompanionTexturesSizeCorrect(const gl::Context *context)
{
    ASSERT(mMetalLayer);

    gl::Extents size(static_cast<int>(mMetalLayer.get().drawableSize.width),
                     static_cast<int>(mMetalLayer.get().drawableSize.height), 1);

    ANGLE_TRY(SurfaceMtl::ensureCompanionTexturesSizeCorrect(context, size));

    return angle::Result::Continue;
}

angle::Result WindowSurfaceMtl::ensureColorTextureReadyForReadPixels(const gl::Context *context)
{
    ANGLE_TRY(ensureCurrentDrawableObtained(context));

    if (mMSColorTexture)
    {
        if (mMSColorTexture->isCPUReadMemNeedSync())
        {
            ANGLE_TRY(resolveColorTextureIfNeeded(context));
            mMSColorTexture->resetCPUReadMemNeedSync();
        }
    }

    return angle::Result::Continue;
}

CGSize WindowSurfaceMtl::calcExpectedDrawableSize() const
{
    CGSize currentLayerSize           = mMetalLayer.get().bounds.size;
    CGFloat currentLayerContentsScale = mMetalLayer.get().contentsScale;
    CGSize expectedDrawableSize = CGSizeMake(currentLayerSize.width * currentLayerContentsScale,
                                             currentLayerSize.height * currentLayerContentsScale);

    return expectedDrawableSize;
}

bool WindowSurfaceMtl::checkIfLayerResized(const gl::Context *context)
{
    if (mMetalLayer.get() != mLayer)
    {
        if (mMetalLayer.get().contentsScale != mLayer.contentsScale)
        {
            // Parent layer's content scale has changed, update Metal layer's scale factor.
            mMetalLayer.get().contentsScale = mLayer.contentsScale;
        }
#if !TARGET_OS_OSX && !TARGET_OS_MACCATALYST
        // Only macOS supports autoresizing mask. Thus, the metal layer has to be manually
        // updated.
        if (!CGRectEqualToRect(mMetalLayer.get().bounds, mLayer.bounds))
        {
            // Parent layer's bounds has changed, update the Metal layer's bounds as well.
            mMetalLayer.get().bounds = mLayer.bounds;
        }
#endif
    }

    CGSize currentLayerDrawableSize = mMetalLayer.get().drawableSize;
    CGSize expectedDrawableSize     = calcExpectedDrawableSize();

    // NOTE(hqle): We need to compare the size against mCurrentKnownDrawableSize also.
    // That is because metal framework might internally change the drawableSize property of
    // metal layer, and it might become equal to expectedDrawableSize. If that happens, we cannot
    // know whether the layer has been resized or not.
    if (currentLayerDrawableSize.width != expectedDrawableSize.width ||
        currentLayerDrawableSize.height != expectedDrawableSize.height ||
        mCurrentKnownDrawableSize.width != expectedDrawableSize.width ||
        mCurrentKnownDrawableSize.height != expectedDrawableSize.height)
    {
        // Resize the internal drawable texture.
        mMetalLayer.get().drawableSize = mCurrentKnownDrawableSize = expectedDrawableSize;

        return true;
    }

    return false;
}

angle::Result WindowSurfaceMtl::obtainNextDrawable(const gl::Context *context)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        ContextMtl *contextMtl = mtl::GetImpl(context);
        ANGLE_CHECK(contextMtl, mMetalLayer, gl::err::kInternalError, GL_INVALID_OPERATION);

        // Check if layer was resized
        if (checkIfLayerResized(context))
        {
            contextMtl->onBackbufferResized(context, this);
        }

        mCurrentDrawable = [mMetalLayer nextDrawable];
        if (!mCurrentDrawable)
        {
            // The GPU might be taking too long finishing its rendering to the previous frame.
            // Try again, indefinitely wait until the previous frame render finishes.
            // TODO: this may wait forever here
            mMetalLayer.get().allowsNextDrawableTimeout = NO;
            mCurrentDrawable                            = [mMetalLayer nextDrawable];
            mMetalLayer.get().allowsNextDrawableTimeout = YES;
        }

        if (!mColorTexture)
        {
            mColorTexture = mtl::Texture::MakeFromMetal(mCurrentDrawable.get().texture);
            ASSERT(!mColorRenderTarget.getTexture());
            mColorRenderTarget.setWithImplicitMSTexture(mColorTexture, mMSColorTexture,
                                                        mtl::kZeroNativeMipLevel, 0, mColorFormat);
        }
        else
        {
            mColorTexture->set(mCurrentDrawable.get().texture);
        }
        mColorTextureInitialized = false;

        ANGLE_MTL_LOG("Current metal drawable size=%d,%d", mColorTexture->width(),
                      mColorTexture->height());

        // Now we have to resize depth stencil buffers if required.
        ANGLE_TRY(ensureCompanionTexturesSizeCorrect(context));

        return angle::Result::Continue;
    }
}

angle::Result WindowSurfaceMtl::swapImpl(const gl::Context *context)
{
    if (mCurrentDrawable)
    {
        ASSERT(mColorTexture);

        ContextMtl *contextMtl = mtl::GetImpl(context);

        if (mMSColorTexture)
        {
            ANGLE_TRY(resolveColorTextureIfNeeded(context));
        }

        contextMtl->present(context, mCurrentDrawable);

        StopFrameCapture();
        StartFrameCapture(contextMtl);

        // Invalidate current drawable
        mColorTexture->set(nil);
        mCurrentDrawable = nil;
    }
    // Robust resource init: should initialize stencil zero and depth to 1.0 after swap.
    if (mDepthTexture || mStencilTexture)
    {
        mDepthStencilTexturesInitialized = false;
    }
    return angle::Result::Continue;
}

// OffscreenSurfaceMtl implementation
OffscreenSurfaceMtl::OffscreenSurfaceMtl(DisplayMtl *display,
                                         const egl::SurfaceState &state,
                                         const egl::AttributeMap &attribs)
    : SurfaceMtl(display, state, attribs)
{
    mSize = gl::Extents(attribs.getAsInt(EGL_WIDTH, 1), attribs.getAsInt(EGL_HEIGHT, 1), 1);
}

OffscreenSurfaceMtl::~OffscreenSurfaceMtl() {}

void OffscreenSurfaceMtl::destroy(const egl::Display *display)
{
    SurfaceMtl::destroy(display);
}

EGLint OffscreenSurfaceMtl::getWidth() const
{
    return mSize.width;
}

EGLint OffscreenSurfaceMtl::getHeight() const
{
    return mSize.height;
}

egl::Error OffscreenSurfaceMtl::swap(const gl::Context *context)
{
    // Check for surface resize.
    ANGLE_TO_EGL_TRY(ensureTexturesSizeCorrect(context));

    return egl::NoError();
}

egl::Error OffscreenSurfaceMtl::bindTexImage(const gl::Context *context,
                                             gl::Texture *texture,
                                             EGLint buffer)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    contextMtl->flushCommandBuffer(mtl::NoWait);

    // Initialize offscreen textures if needed:
    ANGLE_TO_EGL_TRY(ensureTexturesSizeCorrect(context));

    return egl::NoError();
}

egl::Error OffscreenSurfaceMtl::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);

    if (mMSColorTexture)
    {
        ANGLE_TO_EGL_TRY(resolveColorTextureIfNeeded(context));
    }

    // NOTE(hqle): Should we finishCommandBuffer or flush is enough?
    contextMtl->flushCommandBuffer(mtl::NoWait);
    return egl::NoError();
}

angle::Result OffscreenSurfaceMtl::getAttachmentRenderTarget(
    const gl::Context *context,
    GLenum binding,
    const gl::ImageIndex &imageIndex,
    GLsizei samples,
    FramebufferAttachmentRenderTarget **rtOut)
{
    // Initialize offscreen textures if needed:
    ANGLE_TRY(ensureTexturesSizeCorrect(context));

    return SurfaceMtl::getAttachmentRenderTarget(context, binding, imageIndex, samples, rtOut);
}

angle::Result OffscreenSurfaceMtl::ensureTexturesSizeCorrect(const gl::Context *context)
{
    if (!mColorTexture || mColorTexture->sizeAt0() != mSize)
    {
        ANGLE_TRY(CreateOrResizeTexture(context, mColorFormat, mSize.width, mSize.height, 1,
                                        /** renderTargetOnly */ false, &mColorTexture));

        mColorRenderTarget.set(mColorTexture, mtl::kZeroNativeMipLevel, 0, mColorFormat);
    }

    return ensureCompanionTexturesSizeCorrect(context, mSize);
}

// PBufferSurfaceMtl implementation
PBufferSurfaceMtl::PBufferSurfaceMtl(DisplayMtl *display,
                                     const egl::SurfaceState &state,
                                     const egl::AttributeMap &attribs)
    : OffscreenSurfaceMtl(display, state, attribs)
{}

void PBufferSurfaceMtl::setFixedWidth(EGLint width)
{
    mSize.width = width;
}

void PBufferSurfaceMtl::setFixedHeight(EGLint height)
{
    mSize.height = height;
}

}  // namespace rx
