//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SurfaceWgpu.cpp:
//    Implements the class methods for SurfaceWgpu.
//

#include "libANGLE/renderer/wgpu/SurfaceWgpu.h"

#include "common/debug.h"

#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"
#include "libANGLE/renderer/wgpu/FramebufferWgpu.h"

namespace rx
{

constexpr wgpu::TextureUsage kSurfaceTextureUsage =
    wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
    wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;

SurfaceWgpu::SurfaceWgpu(const egl::SurfaceState &surfaceState) : SurfaceImpl(surfaceState) {}

SurfaceWgpu::~SurfaceWgpu() {}

angle::Result SurfaceWgpu::createDepthStencilAttachment(uint32_t width,
                                                        uint32_t height,
                                                        const webgpu::Format &webgpuFormat,
                                                        wgpu::Device &device,
                                                        AttachmentImage *outDepthStencilAttachment)
{
    wgpu::TextureDescriptor desc = outDepthStencilAttachment->texture.createTextureDescriptor(
        kSurfaceTextureUsage, wgpu::TextureDimension::e2D, {width, height, 1},
        webgpuFormat.getActualWgpuTextureFormat(), 1, 1);

    constexpr uint32_t level = 0;
    constexpr uint32_t layer = 0;

    ANGLE_TRY(outDepthStencilAttachment->texture.initImage(webgpuFormat.getIntendedFormatID(),
                                                           webgpuFormat.getActualImageFormatID(),
                                                           device, gl::LevelIndex(level), desc));

    wgpu::TextureView view;
    ANGLE_TRY(
        outDepthStencilAttachment->texture.createTextureView(gl::LevelIndex(level), layer, view));
    outDepthStencilAttachment->renderTarget.set(
        &outDepthStencilAttachment->texture, view, webgpu::LevelIndex(level), layer,
        outDepthStencilAttachment->texture.toWgpuTextureFormat());
    return angle::Result::Continue;
}

OffscreenSurfaceWgpu::OffscreenSurfaceWgpu(const egl::SurfaceState &surfaceState)
    : SurfaceWgpu(surfaceState),
      mWidth(surfaceState.attributes.getAsInt(EGL_WIDTH, 0)),
      mHeight(surfaceState.attributes.getAsInt(EGL_HEIGHT, 0))
{}

OffscreenSurfaceWgpu::~OffscreenSurfaceWgpu() {}

egl::Error OffscreenSurfaceWgpu::initialize(const egl::Display *display)
{
    return angle::ResultToEGL(initializeImpl(display));
}

egl::Error OffscreenSurfaceWgpu::swap(const gl::Context *context)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error OffscreenSurfaceWgpu::bindTexImage(const gl::Context *context,
                                              gl::Texture *texture,
                                              EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error OffscreenSurfaceWgpu::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

void OffscreenSurfaceWgpu::setSwapInterval(const egl::Display *display, EGLint interval) {}

EGLint OffscreenSurfaceWgpu::getWidth() const
{
    return mWidth;
}

EGLint OffscreenSurfaceWgpu::getHeight() const
{
    return mHeight;
}

EGLint OffscreenSurfaceWgpu::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

angle::Result OffscreenSurfaceWgpu::initializeContents(const gl::Context *context,
                                                       GLenum binding,
                                                       const gl::ImageIndex &imageIndex)
{
    UNIMPLEMENTED();
    return angle::Result::Continue;
}

egl::Error OffscreenSurfaceWgpu::attachToFramebuffer(const gl::Context *context,
                                                     gl::Framebuffer *framebuffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error OffscreenSurfaceWgpu::detachFromFramebuffer(const gl::Context *context,
                                                       gl::Framebuffer *framebuffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

angle::Result OffscreenSurfaceWgpu::getAttachmentRenderTarget(
    const gl::Context *context,
    GLenum binding,
    const gl::ImageIndex &imageIndex,
    GLsizei samples,
    FramebufferAttachmentRenderTarget **rtOut)
{
    if (binding == GL_BACK)
    {
        *rtOut = &mColorAttachment.renderTarget;
    }
    else
    {
        ASSERT(binding == GL_DEPTH || binding == GL_STENCIL || binding == GL_DEPTH_STENCIL);
        *rtOut = &mDepthStencilAttachment.renderTarget;
    }

    return angle::Result::Continue;
}

angle::Result OffscreenSurfaceWgpu::initializeImpl(const egl::Display *display)
{
    DisplayWgpu *displayWgpu = webgpu::GetImpl(display);
    wgpu::Device &device     = displayWgpu->getDevice();

    const egl::Config *config = mState.config;

    if (config->renderTargetFormat != GL_NONE)
    {
        const webgpu::Format &webgpuFormat = displayWgpu->getFormat(config->renderTargetFormat);
        wgpu::TextureDescriptor desc       = mColorAttachment.texture.createTextureDescriptor(
            kSurfaceTextureUsage, wgpu::TextureDimension::e2D,
            {static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight), 1},
            webgpuFormat.getActualWgpuTextureFormat(), 1, 1);

        constexpr uint32_t level = 0;
        constexpr uint32_t layer = 0;

        ANGLE_TRY(mColorAttachment.texture.initImage(webgpuFormat.getIntendedFormatID(),
                                                     webgpuFormat.getActualImageFormatID(), device,
                                                     gl::LevelIndex(level), desc));

        wgpu::TextureView view;
        ANGLE_TRY(mColorAttachment.texture.createTextureView(gl::LevelIndex(level), layer, view));
        mColorAttachment.renderTarget.set(&mColorAttachment.texture, view,
                                          webgpu::LevelIndex(level), layer,
                                          mColorAttachment.texture.toWgpuTextureFormat());
    }

    if (config->depthStencilFormat != GL_NONE)
    {
        const webgpu::Format &webgpuFormat = displayWgpu->getFormat(config->depthStencilFormat);
        ANGLE_TRY(createDepthStencilAttachment(static_cast<uint32_t>(mWidth),
                                               static_cast<uint32_t>(mHeight), webgpuFormat, device,
                                               &mDepthStencilAttachment));
    }

    return angle::Result::Continue;
}

WindowSurfaceWgpu::WindowSurfaceWgpu(const egl::SurfaceState &surfaceState,
                                     EGLNativeWindowType window)
    : SurfaceWgpu(surfaceState), mNativeWindow(window)
{}

WindowSurfaceWgpu::~WindowSurfaceWgpu() {}

egl::Error WindowSurfaceWgpu::initialize(const egl::Display *display)
{
    return angle::ResultToEGL(initializeImpl(display));
}

void WindowSurfaceWgpu::destroy(const egl::Display *display)
{
    mSurface   = nullptr;
    mColorAttachment.renderTarget.reset();
    mColorAttachment.texture.resetImage();
    mDepthStencilAttachment.renderTarget.reset();
    mDepthStencilAttachment.texture.resetImage();
}

egl::Error WindowSurfaceWgpu::swap(const gl::Context *context)
{
    return angle::ResultToEGL(swapImpl(context));
}

egl::Error WindowSurfaceWgpu::bindTexImage(const gl::Context *context,
                                           gl::Texture *texture,
                                           EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error WindowSurfaceWgpu::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

void WindowSurfaceWgpu::setSwapInterval(const egl::Display *display, EGLint interval)
{
    UNIMPLEMENTED();
}

EGLint WindowSurfaceWgpu::getWidth() const
{
    return mCurrentSurfaceSize.width;
}

EGLint WindowSurfaceWgpu::getHeight() const
{
    return mCurrentSurfaceSize.height;
}

EGLint WindowSurfaceWgpu::getSwapBehavior() const
{
    UNIMPLEMENTED();
    return 0;
}

angle::Result WindowSurfaceWgpu::initializeContents(const gl::Context *context,
                                                    GLenum binding,
                                                    const gl::ImageIndex &imageIndex)
{
    UNIMPLEMENTED();
    return angle::Result::Continue;
}

egl::Error WindowSurfaceWgpu::attachToFramebuffer(const gl::Context *context,
                                                  gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

egl::Error WindowSurfaceWgpu::detachFromFramebuffer(const gl::Context *context,
                                                    gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

angle::Result WindowSurfaceWgpu::getAttachmentRenderTarget(
    const gl::Context *context,
    GLenum binding,
    const gl::ImageIndex &imageIndex,
    GLsizei samples,
    FramebufferAttachmentRenderTarget **rtOut)
{
    if (binding == GL_BACK)
    {
        *rtOut = &mColorAttachment.renderTarget;
    }
    else
    {
        ASSERT(binding == GL_DEPTH || binding == GL_STENCIL || binding == GL_DEPTH_STENCIL);
        *rtOut = &mDepthStencilAttachment.renderTarget;
    }

    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpu::initializeImpl(const egl::Display *display)
{
    DisplayWgpu *displayWgpu = webgpu::GetImpl(display);
    wgpu::Adapter &adapter   = displayWgpu->getAdapter();

    ANGLE_TRY(createWgpuSurface(display, &mSurface));

    gl::Extents size;
    ANGLE_TRY(getCurrentWindowSize(display, &size));

    wgpu::SurfaceCapabilities surfaceCapabilities;
    wgpu::Status getCapabilitiesStatus = mSurface.GetCapabilities(adapter, &surfaceCapabilities);
    if (getCapabilitiesStatus != wgpu::Status::Success)
    {
        ERR() << "wgpu::Surface::GetCapabilities failed: "
              << gl::FmtHex(static_cast<uint32_t>(getCapabilitiesStatus));
        return angle::Result::Stop;
    }

    const egl::Config *config = mState.config;
    ASSERT(config->renderTargetFormat != GL_NONE);
    mSurfaceTextureFormat = &displayWgpu->getFormat(config->renderTargetFormat);
    ASSERT(std::find(surfaceCapabilities.formats,
                     surfaceCapabilities.formats + surfaceCapabilities.formatCount,
                     mSurfaceTextureFormat->getActualWgpuTextureFormat()) !=
           (surfaceCapabilities.formats + surfaceCapabilities.formatCount));

    mSurfaceTextureUsage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                           wgpu::TextureUsage::CopyDst;
    ASSERT((surfaceCapabilities.usages & mSurfaceTextureUsage) == mSurfaceTextureUsage);

    // Default to the always supported Fifo present mode. Use Mailbox if it's available.
    mPresentMode = wgpu::PresentMode::Fifo;
    for (size_t i = 0; i < surfaceCapabilities.presentModeCount; i++)
    {
        if (surfaceCapabilities.presentModes[i] == wgpu::PresentMode::Mailbox)
        {
            mPresentMode = wgpu::PresentMode::Mailbox;
        }
    }

    if (config->depthStencilFormat != GL_NONE)
    {
        mDepthStencilFormat = &displayWgpu->getFormat(config->depthStencilFormat);
    }
    else
    {
        mDepthStencilFormat = nullptr;
    }

    ANGLE_TRY(configureSurface(display, size));
    ANGLE_TRY(updateCurrentTexture(display));

    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpu::swapImpl(const gl::Context *context)
{
    const egl::Display *display = context->getDisplay();
    ContextWgpu *contextWgpu    = webgpu::GetImpl(context);

    ANGLE_TRY(contextWgpu->flush(webgpu::RenderPassClosureReason::EGLSwapBuffers));

    mSurface.Present();

    gl::Extents size;
    ANGLE_TRY(getCurrentWindowSize(display, &size));
    if (size != mCurrentSurfaceSize)
    {
        ANGLE_TRY(configureSurface(display, size));
    }

    ANGLE_TRY(updateCurrentTexture(display));

    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpu::configureSurface(const egl::Display *display,
                                                  const gl::Extents &size)
{
    DisplayWgpu *displayWgpu = webgpu::GetImpl(display);
    wgpu::Device &device     = displayWgpu->getDevice();

    ASSERT(mSurfaceTextureFormat != nullptr);

    wgpu::SurfaceConfiguration surfaceConfig = {};
    surfaceConfig.device                     = device;
    surfaceConfig.format                     = mSurfaceTextureFormat->getActualWgpuTextureFormat();
    surfaceConfig.usage                      = mSurfaceTextureUsage;
    surfaceConfig.width                      = size.width;
    surfaceConfig.height                     = size.height;
    surfaceConfig.presentMode                = mPresentMode;

    mSurface.Configure(&surfaceConfig);

    if (mDepthStencilFormat)
    {
        ANGLE_TRY(createDepthStencilAttachment(
            static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height),
            *mDepthStencilFormat, device, &mDepthStencilAttachment));
    }

    mCurrentSurfaceSize = size;
    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpu::updateCurrentTexture(const egl::Display *display)
{
    wgpu::SurfaceTexture texture;
    mSurface.GetCurrentTexture(&texture);
    if (texture.status != wgpu::SurfaceGetCurrentTextureStatus::Success)
    {
        ERR() << "wgpu::Surface::GetCurrentTexture failed: "
              << gl::FmtHex(static_cast<uint32_t>(texture.status));
        return angle::Result::Stop;
    }

    wgpu::TextureFormat wgpuFormat = texture.texture.GetFormat();
    angle::FormatID angleFormat    = webgpu::GetFormatIDFromWgpuTextureFormat(wgpuFormat);

    ANGLE_TRY(mColorAttachment.texture.initExternal(angleFormat, angleFormat, texture.texture));

    wgpu::TextureView view;
    ANGLE_TRY(mColorAttachment.texture.createTextureView(gl::LevelIndex(0), 0, view));

    mColorAttachment.renderTarget.set(&mColorAttachment.texture, view, webgpu::LevelIndex(0), 0,
                                      wgpuFormat);

    return angle::Result::Continue;
}

}  // namespace rx
