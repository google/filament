//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceWgpu.h:
//    Defines the class interface for SurfaceWgpu, implementing SurfaceImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_SURFACEWGPU_H_
#define LIBANGLE_RENDERER_WGPU_SURFACEWGPU_H_

#include "libANGLE/renderer/SurfaceImpl.h"

#include "libANGLE/renderer/wgpu/RenderTargetWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_helpers.h"

#include <dawn/webgpu_cpp.h>

namespace rx
{

class SurfaceWgpu : public SurfaceImpl
{
  public:
    SurfaceWgpu(const egl::SurfaceState &surfaceState);
    ~SurfaceWgpu() override;

  protected:
    struct AttachmentImage
    {
        webgpu::ImageHelper texture;
        RenderTargetWgpu renderTarget;
    };
    angle::Result createDepthStencilAttachment(uint32_t width,
                                               uint32_t height,
                                               const webgpu::Format &webgpuFormat,
                                               wgpu::Device &device,
                                               AttachmentImage *outDepthStencilAttachment);
};

class OffscreenSurfaceWgpu : public SurfaceWgpu
{
  public:
    OffscreenSurfaceWgpu(const egl::SurfaceState &surfaceState);
    ~OffscreenSurfaceWgpu() override;

    egl::Error initialize(const egl::Display *display) override;
    egl::Error swap(const gl::Context *context) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    void setSwapInterval(const egl::Display *display, EGLint interval) override;

    // width and height can change with client window resizing
    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint getSwapBehavior() const override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

  private:
    angle::Result initializeImpl(const egl::Display *display);

    EGLint mWidth;
    EGLint mHeight;

    AttachmentImage mColorAttachment;
    AttachmentImage mDepthStencilAttachment;
};

class WindowSurfaceWgpu : public SurfaceWgpu
{
  public:
    WindowSurfaceWgpu(const egl::SurfaceState &surfaceState, EGLNativeWindowType window);
    ~WindowSurfaceWgpu() override;

    egl::Error initialize(const egl::Display *display) override;
    void destroy(const egl::Display *display) override;
    egl::Error swap(const gl::Context *context) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    void setSwapInterval(const egl::Display *display, EGLint interval) override;

    // width and height can change with client window resizing
    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint getSwapBehavior() const override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    egl::Error attachToFramebuffer(const gl::Context *context,
                                   gl::Framebuffer *framebuffer) override;
    egl::Error detachFromFramebuffer(const gl::Context *context,
                                     gl::Framebuffer *framebuffer) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

  protected:
    EGLNativeWindowType getNativeWindow() const { return mNativeWindow; }

  private:
    angle::Result initializeImpl(const egl::Display *display);

    angle::Result swapImpl(const gl::Context *context);

    angle::Result configureSurface(const egl::Display *display, const gl::Extents &size);
    angle::Result updateCurrentTexture(const egl::Display *display);

    virtual angle::Result createWgpuSurface(const egl::Display *display,
                                            wgpu::Surface *outSurface) = 0;
    virtual angle::Result getCurrentWindowSize(const egl::Display *display,
                                               gl::Extents *outSize)   = 0;

    EGLNativeWindowType mNativeWindow;
    wgpu::Surface mSurface;

    const webgpu::Format *mSurfaceTextureFormat = nullptr;
    wgpu::TextureUsage mSurfaceTextureUsage;
    wgpu::PresentMode mPresentMode;

    const webgpu::Format *mDepthStencilFormat = nullptr;

    gl::Extents mCurrentSurfaceSize;

    AttachmentImage mColorAttachment;
    AttachmentImage mDepthStencilAttachment;
};

WindowSurfaceWgpu *CreateWgpuWindowSurface(const egl::SurfaceState &surfaceState,
                                           EGLNativeWindowType window);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_SURFACEWGPU_H_
