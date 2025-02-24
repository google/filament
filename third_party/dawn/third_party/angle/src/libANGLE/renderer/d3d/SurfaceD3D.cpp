//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceD3D.cpp: D3D implementation of an EGL surface

#include "libANGLE/renderer/d3d/SurfaceD3D.h"

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/d3d/DisplayD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/SwapChainD3D.h"

#include <EGL/eglext.h>
#include <tchar.h>
#include <algorithm>

namespace rx
{

SurfaceD3D::SurfaceD3D(const egl::SurfaceState &state,
                       RendererD3D *renderer,
                       egl::Display *display,
                       EGLNativeWindowType window,
                       EGLenum buftype,
                       EGLClientBuffer clientBuffer,
                       const egl::AttributeMap &attribs)
    : SurfaceImpl(state),
      mRenderer(renderer),
      mDisplay(display),
      mFixedSize(window == nullptr || attribs.get(EGL_FIXED_SIZE_ANGLE, EGL_FALSE) == EGL_TRUE),
      mFixedWidth(0),
      mFixedHeight(0),
      mOrientation(static_cast<EGLint>(attribs.get(EGL_SURFACE_ORIENTATION_ANGLE, 0))),
      mRenderTargetFormat(state.config->renderTargetFormat),
      mDepthStencilFormat(state.config->depthStencilFormat),
      mColorFormat(nullptr),
      mSwapChain(nullptr),
      mSwapIntervalDirty(true),
      mNativeWindow(renderer->createNativeWindow(window, state.config, attribs)),
      mWidth(static_cast<EGLint>(attribs.get(EGL_WIDTH, 0))),
      mHeight(static_cast<EGLint>(attribs.get(EGL_HEIGHT, 0))),
      mSwapInterval(1),
      mShareHandle(0),
      mD3DTexture(nullptr),
      mBuftype(buftype)
{
    if (window != nullptr && !mFixedSize)
    {
        mWidth  = -1;
        mHeight = -1;
    }

    if (mFixedSize)
    {
        mFixedWidth  = mWidth;
        mFixedHeight = mHeight;
    }

    switch (buftype)
    {
        case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
            mShareHandle = static_cast<HANDLE>(clientBuffer);
            break;

        case EGL_D3D_TEXTURE_ANGLE:
            mD3DTexture = static_cast<IUnknown *>(clientBuffer);
            ASSERT(mD3DTexture != nullptr);
            mD3DTexture->AddRef();
            break;

        default:
            break;
    }
}

SurfaceD3D::~SurfaceD3D()
{
    releaseSwapChain();
    SafeDelete(mNativeWindow);
    SafeRelease(mD3DTexture);
}

void SurfaceD3D::releaseSwapChain()
{
    SafeDelete(mSwapChain);
}

egl::Error SurfaceD3D::initialize(const egl::Display *display)
{
    if (mNativeWindow->getNativeWindow())
    {
        if (!mNativeWindow->initialize())
        {
            return egl::EglBadSurface();
        }
    }

    if (mBuftype == EGL_D3D_TEXTURE_ANGLE)
    {
        ANGLE_TRY(mRenderer->getD3DTextureInfo(mState.config, mD3DTexture, mState.attributes,
                                               &mFixedWidth, &mFixedHeight, nullptr, nullptr,
                                               &mColorFormat, nullptr));
        if (mState.attributes.contains(EGL_GL_COLORSPACE))
        {
            if (mColorFormat->id != angle::FormatID::R8G8B8A8_TYPELESS &&
                mColorFormat->id != angle::FormatID::B8G8R8A8_TYPELESS)
            {
                return egl::EglBadMatch()
                       << "EGL_GL_COLORSPACE may only be specified for TYPELESS textures";
            }
        }
        if (mColorFormat->id == angle::FormatID::R8G8B8A8_TYPELESS)
        {
            EGLAttrib colorspace =
                mState.attributes.get(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR);
            if (colorspace == EGL_GL_COLORSPACE_SRGB)
            {
                mColorFormat = &angle::Format::Get(angle::FormatID::R8G8B8A8_TYPELESS_SRGB);
            }
        }
        if (mColorFormat->id == angle::FormatID::B8G8R8A8_TYPELESS)
        {
            EGLAttrib colorspace =
                mState.attributes.get(EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR);
            if (colorspace == EGL_GL_COLORSPACE_SRGB)
            {
                mColorFormat = &angle::Format::Get(angle::FormatID::B8G8R8A8_TYPELESS_SRGB);
            }
        }
        mRenderTargetFormat = mColorFormat->fboImplementationInternalFormat;
    }

    ANGLE_TRY(resetSwapChain(display));
    return egl::NoError();
}

egl::Error SurfaceD3D::bindTexImage(const gl::Context *, gl::Texture *, EGLint)
{
    return egl::NoError();
}

egl::Error SurfaceD3D::releaseTexImage(const gl::Context *, EGLint)
{
    return egl::NoError();
}

egl::Error SurfaceD3D::getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc)
{
    if (!mState.directComposition)
    {
        return egl::EglBadSurface()
               << "getSyncValues: surface requires Direct Composition to be enabled";
    }

    return mSwapChain->getSyncValues(ust, msc, sbc);
}

egl::Error SurfaceD3D::getMscRate(EGLint *numerator, EGLint *denominator)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

egl::Error SurfaceD3D::resetSwapChain(const egl::Display *display)
{
    ASSERT(!mSwapChain);

    int width;
    int height;

    if (!mFixedSize)
    {
        RECT windowRect;
        if (!mNativeWindow->getClientRect(&windowRect))
        {
            ASSERT(false);

            return egl::EglBadSurface() << "Could not retrieve the window dimensions";
        }

        width  = windowRect.right - windowRect.left;
        height = windowRect.bottom - windowRect.top;
    }
    else
    {
        // non-window surface - size is determined at creation
        width  = mFixedWidth;
        height = mFixedHeight;
    }

    mSwapChain =
        mRenderer->createSwapChain(mNativeWindow, mShareHandle, mD3DTexture, mRenderTargetFormat,
                                   mDepthStencilFormat, mOrientation, mState.config->samples);
    if (!mSwapChain)
    {
        return egl::EglBadAlloc();
    }

    // This is a bit risky to pass the proxy context here, but it can happen at almost any time.
    DisplayD3D *displayD3D = GetImplAs<DisplayD3D>(display);
    egl::Error error       = resetSwapChain(displayD3D, width, height);
    if (error.isError())
    {
        SafeDelete(mSwapChain);
        return error;
    }

    return egl::NoError();
}

egl::Error SurfaceD3D::resizeSwapChain(DisplayD3D *displayD3D,
                                       int backbufferWidth,
                                       int backbufferHeight)
{
    ASSERT(backbufferWidth >= 0 && backbufferHeight >= 0);
    ASSERT(mSwapChain);

    EGLint status =
        mSwapChain->resize(displayD3D, std::max(1, backbufferWidth), std::max(1, backbufferHeight));

    if (status == EGL_CONTEXT_LOST)
    {
        mDisplay->notifyDeviceLost();
        return egl::Error(status);
    }
    else if (status != EGL_SUCCESS)
    {
        return egl::Error(status);
    }

    mWidth  = backbufferWidth;
    mHeight = backbufferHeight;

    return egl::NoError();
}

egl::Error SurfaceD3D::resetSwapChain(DisplayD3D *displayD3D,
                                      int backbufferWidth,
                                      int backbufferHeight)
{
    ASSERT(backbufferWidth >= 0 && backbufferHeight >= 0);
    ASSERT(mSwapChain);

    EGLint status = mSwapChain->reset(displayD3D, std::max(1, backbufferWidth),
                                      std::max(1, backbufferHeight), mSwapInterval);

    if (status == EGL_CONTEXT_LOST)
    {
        mRenderer->notifyDeviceLost();
        return egl::Error(status);
    }
    else if (status != EGL_SUCCESS)
    {
        return egl::Error(status);
    }

    mWidth             = backbufferWidth;
    mHeight            = backbufferHeight;
    mSwapIntervalDirty = false;

    return egl::NoError();
}

egl::Error SurfaceD3D::swapRect(DisplayD3D *displayD3D,
                                EGLint x,
                                EGLint y,
                                EGLint width,
                                EGLint height)
{
    if (!mSwapChain)
    {
        return egl::NoError();
    }

    if (x + width > mWidth)
    {
        width = mWidth - x;
    }

    if (y + height > mHeight)
    {
        height = mHeight - y;
    }

    if (width != 0 && height != 0)
    {
        EGLint status = mSwapChain->swapRect(displayD3D, x, y, width, height);

        if (status == EGL_CONTEXT_LOST)
        {
            mRenderer->notifyDeviceLost();
            return egl::Error(status);
        }
        else if (status != EGL_SUCCESS)
        {
            return egl::Error(status);
        }
    }

    ANGLE_TRY(checkForOutOfDateSwapChain(displayD3D));

    return egl::NoError();
}

egl::Error SurfaceD3D::checkForOutOfDateSwapChain(DisplayD3D *displayD3D)
{
    RECT client;
    int clientWidth  = getWidth();
    int clientHeight = getHeight();
    bool sizeDirty   = false;
    if (!mFixedSize && !mNativeWindow->isIconic())
    {
        // The window is automatically resized to 150x22 when it's minimized, but the swapchain
        // shouldn't be resized because that's not a useful size to render to.
        if (!mNativeWindow->getClientRect(&client))
        {
            UNREACHABLE();
            return egl::NoError();
        }

        // Grow the buffer now, if the window has grown. We need to grow now to avoid losing
        // information.
        clientWidth  = client.right - client.left;
        clientHeight = client.bottom - client.top;
        sizeDirty    = clientWidth != getWidth() || clientHeight != getHeight();
    }
    else if (mFixedSize)
    {
        clientWidth  = mFixedWidth;
        clientHeight = mFixedHeight;
        sizeDirty    = mFixedWidth != getWidth() || mFixedHeight != getHeight();
    }

    if (mSwapIntervalDirty)
    {
        ANGLE_TRY(resetSwapChain(displayD3D, clientWidth, clientHeight));
    }
    else if (sizeDirty)
    {
        ANGLE_TRY(resizeSwapChain(displayD3D, clientWidth, clientHeight));
    }

    return egl::NoError();
}

egl::Error SurfaceD3D::swap(const gl::Context *context)
{
    DisplayD3D *displayD3D = GetImplAs<DisplayD3D>(context->getDisplay());
    return swapRect(displayD3D, 0, 0, mWidth, mHeight);
}

egl::Error SurfaceD3D::postSubBuffer(const gl::Context *context,
                                     EGLint x,
                                     EGLint y,
                                     EGLint width,
                                     EGLint height)
{
    DisplayD3D *displayD3D = GetImplAs<DisplayD3D>(context->getDisplay());
    return swapRect(displayD3D, x, y, width, height);
}

rx::SwapChainD3D *SurfaceD3D::getSwapChain() const
{
    return mSwapChain;
}

void SurfaceD3D::setSwapInterval(const egl::Display *display, EGLint interval)
{
    if (mSwapInterval == interval)
    {
        return;
    }

    mSwapInterval      = interval;
    mSwapIntervalDirty = true;
}

void SurfaceD3D::setFixedWidth(EGLint width)
{
    mFixedWidth = width;
}

void SurfaceD3D::setFixedHeight(EGLint height)
{
    mFixedHeight = height;
}

EGLint SurfaceD3D::getWidth() const
{
    return mWidth;
}

EGLint SurfaceD3D::getHeight() const
{
    return mHeight;
}

EGLint SurfaceD3D::isPostSubBufferSupported() const
{
    // post sub buffer is always possible on D3D surfaces
    return EGL_TRUE;
}

EGLint SurfaceD3D::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

egl::Error SurfaceD3D::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    if (attribute == EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE)
    {
        *value = mSwapChain->getShareHandle();
    }
    else if (attribute == EGL_DXGI_KEYED_MUTEX_ANGLE)
    {
        *value = mSwapChain->getKeyedMutex();
    }
    else
        UNREACHABLE();

    return egl::NoError();
}

const angle::Format *SurfaceD3D::getD3DTextureColorFormat() const
{
    return mColorFormat;
}

egl::Error SurfaceD3D::attachToFramebuffer(const gl::Context *context, gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

egl::Error SurfaceD3D::detachFromFramebuffer(const gl::Context *context,
                                             gl::Framebuffer *framebuffer)
{
    return egl::NoError();
}

angle::Result SurfaceD3D::getAttachmentRenderTarget(const gl::Context *context,
                                                    GLenum binding,
                                                    const gl::ImageIndex &imageIndex,
                                                    GLsizei samples,
                                                    FramebufferAttachmentRenderTarget **rtOut)
{
    if (binding == GL_BACK)
    {
        *rtOut = mSwapChain->getColorRenderTarget();
    }
    else
    {
        *rtOut = mSwapChain->getDepthStencilRenderTarget();
    }
    return angle::Result::Continue;
}

angle::Result SurfaceD3D::initializeContents(const gl::Context *context,
                                             GLenum binding,
                                             const gl::ImageIndex &imageIndex)
{
    switch (binding)
    {
        case GL_BACK:
            ASSERT(mState.config->renderTargetFormat != GL_NONE);
            ANGLE_TRY(mRenderer->initRenderTarget(context, mSwapChain->getColorRenderTarget()));
            break;

        case GL_DEPTH:
        case GL_STENCIL:
            ASSERT(mState.config->depthStencilFormat != GL_NONE);
            ANGLE_TRY(
                mRenderer->initRenderTarget(context, mSwapChain->getDepthStencilRenderTarget()));
            break;

        default:
            UNREACHABLE();
            break;
    }
    return angle::Result::Continue;
}

WindowSurfaceD3D::WindowSurfaceD3D(const egl::SurfaceState &state,
                                   RendererD3D *renderer,
                                   egl::Display *display,
                                   EGLNativeWindowType window,
                                   const egl::AttributeMap &attribs)
    : SurfaceD3D(state, renderer, display, window, 0, static_cast<EGLClientBuffer>(0), attribs)
{}

WindowSurfaceD3D::~WindowSurfaceD3D() {}

PbufferSurfaceD3D::PbufferSurfaceD3D(const egl::SurfaceState &state,
                                     RendererD3D *renderer,
                                     egl::Display *display,
                                     EGLenum buftype,
                                     EGLClientBuffer clientBuffer,
                                     const egl::AttributeMap &attribs)
    : SurfaceD3D(state,
                 renderer,
                 display,
                 static_cast<EGLNativeWindowType>(0),
                 buftype,
                 clientBuffer,
                 attribs)
{}

PbufferSurfaceD3D::~PbufferSurfaceD3D() {}

}  // namespace rx
