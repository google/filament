//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceMtl.h: Defines the class interface for Metal Surface.

#ifndef LIBANGLE_RENDERER_METAL_SURFACEMTL_H_
#define LIBANGLE_RENDERER_METAL_SURFACEMTL_H_

#import <Metal/Metal.h>
#import <QuartzCore/CALayer.h>
#import <QuartzCore/CAMetalLayer.h>

#include "libANGLE/renderer/FramebufferImpl.h"
#include "libANGLE/renderer/SurfaceImpl.h"
#include "libANGLE/renderer/metal/RenderTargetMtl.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_resources.h"
#include "libANGLE/renderer/metal/mtl_state_cache.h"

namespace rx
{

class DisplayMtl;

#define ANGLE_TO_EGL_TRY(EXPR)                                 \
    do                                                         \
    {                                                          \
        if (ANGLE_UNLIKELY((EXPR) != angle::Result::Continue)) \
        {                                                      \
            return egl::EglBadSurface();                       \
        }                                                      \
    } while (0)

class SurfaceMtl : public SurfaceImpl
{
  public:
    SurfaceMtl(DisplayMtl *display,
               const egl::SurfaceState &state,
               const egl::AttributeMap &attribs);
    ~SurfaceMtl() override;

    void destroy(const egl::Display *display) override;

    egl::Error initialize(const egl::Display *display) override;

    egl::Error makeCurrent(const gl::Context *context) override;
    egl::Error unMakeCurrent(const gl::Context *context) override;
    egl::Error swap(const gl::Context *context) override;
    egl::Error postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height) override;

    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    egl::Error getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc) override;
    egl::Error getMscRate(EGLint *numerator, EGLint *denominator) override;
    void setSwapInterval(const egl::Display *display, EGLint interval) override;
    void setFixedWidth(EGLint width) override;
    void setFixedHeight(EGLint height) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    const mtl::TextureRef &getColorTexture() { return mColorTexture; }
    const mtl::Format &getColorFormat() const { return mColorFormat; }
    int getSamples() const { return mSamples; }
    bool hasDepthStencil() const { return mDepthTexture || mStencilTexture; }

    bool hasRobustResourceInit() const { return mRobustResourceInit; }

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;
    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

  protected:
    // Ensure companion (MS, depth, stencil) textures' size is correct w.r.t color texture.
    angle::Result ensureCompanionTexturesSizeCorrect(const gl::Context *context,
                                                     const gl::Extents &size);
    angle::Result resolveColorTextureIfNeeded(const gl::Context *context);

    // Normal textures
    mtl::TextureRef mColorTexture;
    mtl::TextureRef mDepthTexture;
    mtl::TextureRef mStencilTexture;

    // Implicit multisample texture
    mtl::TextureRef mMSColorTexture;

    bool mUsePackedDepthStencil = false;
    // Auto resolve MS texture at the end of render pass or requires a separate blitting pass?
    bool mAutoResolveMSColorTexture = false;

    bool mRobustResourceInit = false;
    bool mColorTextureInitialized         = true;
    bool mDepthStencilTexturesInitialized = true;

    mtl::Format mColorFormat;
    mtl::Format mDepthFormat;
    mtl::Format mStencilFormat;

    int mSamples = 0;

    RenderTargetMtl mColorRenderTarget;
    RenderTargetMtl mColorManualResolveRenderTarget;
    RenderTargetMtl mDepthRenderTarget;
    RenderTargetMtl mStencilRenderTarget;
};

class WindowSurfaceMtl : public SurfaceMtl
{
  public:
    WindowSurfaceMtl(DisplayMtl *display,
                     const egl::SurfaceState &state,
                     EGLNativeWindowType window,
                     const egl::AttributeMap &attribs);
    ~WindowSurfaceMtl() override;

    void destroy(const egl::Display *display) override;

    egl::Error initialize(const egl::Display *display) override;

    egl::Error swap(const gl::Context *context) override;

    void setSwapInterval(const egl::Display *display, EGLint interval) override;
    EGLint getSwapBehavior() const override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    // width and height can change with client window resizing
    EGLint getWidth() const override;
    EGLint getHeight() const override;
    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;
    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

    angle::Result ensureCurrentDrawableObtained(const gl::Context *context);

    // Ensure the the texture returned from getColorTexture() is ready for glReadPixels(). This
    // implicitly calls ensureCurrentDrawableObtained().
    angle::Result ensureColorTextureReadyForReadPixels(const gl::Context *context);
    bool preserveBuffer() const { return mRetainBuffer; }

  private:
    angle::Result swapImpl(const gl::Context *context);
    angle::Result obtainNextDrawable(const gl::Context *context);
    angle::Result ensureCompanionTexturesSizeCorrect(const gl::Context *context);

    CGSize calcExpectedDrawableSize() const;
    // Check if metal layer has been resized.
    bool checkIfLayerResized(const gl::Context *context);

    mtl::AutoObjCPtr<CAMetalLayer *> mMetalLayer = nil;
    CALayer *mLayer;
    mtl::AutoObjCPtr<id<CAMetalDrawable>> mCurrentDrawable = nil;

    // Cache last known drawable size that is used by GL context. Can be used to detect resize
    // event. We don't use mMetalLayer.drawableSize directly since it might be changed internally by
    // metal runtime.
    CGSize mCurrentKnownDrawableSize;

    bool mRetainBuffer = false;
};

// Offscreen surface, base class of PBuffer.
class OffscreenSurfaceMtl : public SurfaceMtl
{
  public:
    OffscreenSurfaceMtl(DisplayMtl *display,
                        const egl::SurfaceState &state,
                        const egl::AttributeMap &attribs);
    ~OffscreenSurfaceMtl() override;

    void destroy(const egl::Display *display) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    egl::Error swap(const gl::Context *context) override;

    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

  protected:
    angle::Result ensureTexturesSizeCorrect(const gl::Context *context);

    gl::Extents mSize;
};

// PBuffer surface
class PBufferSurfaceMtl : public OffscreenSurfaceMtl
{
  public:
    PBufferSurfaceMtl(DisplayMtl *display,
                      const egl::SurfaceState &state,
                      const egl::AttributeMap &attribs);

    void setFixedWidth(EGLint width) override;
    void setFixedHeight(EGLint height) override;
};

}  // namespace rx
#endif /* LIBANGLE_RENDERER_METAL_SURFACEMTL_H_ */
