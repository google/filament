//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceWGL.cpp: WGL implementation of egl::Surface

#include "libANGLE/renderer/gl/wgl/PbufferSurfaceWGL.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"
#include "libANGLE/renderer/gl/wgl/wgl_utils.h"

namespace rx
{

PbufferSurfaceWGL::PbufferSurfaceWGL(const egl::SurfaceState &state,
                                     EGLint width,
                                     EGLint height,
                                     EGLenum textureFormat,
                                     EGLenum textureTarget,
                                     bool largest,
                                     int pixelFormat,
                                     HDC deviceContext,
                                     const FunctionsWGL *functions)
    : SurfaceWGL(state),
      mWidth(width),
      mHeight(height),
      mLargest(largest),
      mTextureFormat(textureFormat),
      mTextureTarget(textureTarget),
      mPixelFormat(pixelFormat),
      mParentDeviceContext(deviceContext),
      mPbuffer(nullptr),
      mPbufferDeviceContext(nullptr),
      mFunctionsWGL(functions)
{}

PbufferSurfaceWGL::~PbufferSurfaceWGL()
{
    mFunctionsWGL->releasePbufferDCARB(mPbuffer, mPbufferDeviceContext);
    mPbufferDeviceContext = nullptr;

    mFunctionsWGL->destroyPbufferARB(mPbuffer);
    mPbuffer = nullptr;
}

static int GetWGLTextureType(EGLenum eglTextureType)
{
    switch (eglTextureType)
    {
        case EGL_NO_TEXTURE:
            return WGL_NO_TEXTURE_ARB;
        case EGL_TEXTURE_RGB:
            return WGL_TEXTURE_RGB_ARB;
        case EGL_TEXTURE_RGBA:
            return WGL_TEXTURE_RGBA_ARB;
        default:
            UNREACHABLE();
            return 0;
    }
}

static int GetWGLTextureTarget(EGLenum eglTextureTarget)
{
    switch (eglTextureTarget)
    {
        case EGL_NO_TEXTURE:
            return WGL_NO_TEXTURE_ARB;
        case EGL_TEXTURE_2D:
            return WGL_TEXTURE_2D_ARB;
        default:
            UNREACHABLE();
            return 0;
    }
}

egl::Error PbufferSurfaceWGL::initialize(const egl::Display *display)
{
    const int pbufferCreationAttributes[] = {
        WGL_PBUFFER_LARGEST_ARB,
        mLargest ? 1 : 0,
        WGL_TEXTURE_FORMAT_ARB,
        GetWGLTextureType(mTextureFormat),
        WGL_TEXTURE_TARGET_ARB,
        GetWGLTextureTarget(mTextureTarget),
        0,
        0,
    };

    mPbuffer = mFunctionsWGL->createPbufferARB(mParentDeviceContext, mPixelFormat, mWidth, mHeight,
                                               pbufferCreationAttributes);
    if (mPbuffer == nullptr)
    {
        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to create a native WGL pbuffer, "
                                  << gl::FmtErr(HRESULT_CODE(error));
    }

    // The returned pbuffer may not be as large as requested, update the size members.
    if (mFunctionsWGL->queryPbufferARB(mPbuffer, WGL_PBUFFER_WIDTH_ARB, &mWidth) != TRUE ||
        mFunctionsWGL->queryPbufferARB(mPbuffer, WGL_PBUFFER_HEIGHT_ARB, &mHeight) != TRUE)
    {
        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to query the WGL pbuffer's dimensions, "
                                  << gl::FmtErr(HRESULT_CODE(error));
    }

    mPbufferDeviceContext = mFunctionsWGL->getPbufferDCARB(mPbuffer);
    if (mPbufferDeviceContext == nullptr)
    {
        mFunctionsWGL->destroyPbufferARB(mPbuffer);
        mPbuffer = nullptr;

        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to get the WGL pbuffer handle, "
                                  << gl::FmtErr(HRESULT_CODE(error));
    }

    return egl::NoError();
}

egl::Error PbufferSurfaceWGL::makeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error PbufferSurfaceWGL::swap(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error PbufferSurfaceWGL::postSubBuffer(const gl::Context *context,
                                            EGLint x,
                                            EGLint y,
                                            EGLint width,
                                            EGLint height)
{
    return egl::NoError();
}

egl::Error PbufferSurfaceWGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    *value = nullptr;
    return egl::NoError();
}

static int GetWGLBufferBindTarget(EGLint buffer)
{
    switch (buffer)
    {
        case EGL_BACK_BUFFER:
            return WGL_BACK_LEFT_ARB;
        default:
            UNREACHABLE();
            return 0;
    }
}

egl::Error PbufferSurfaceWGL::bindTexImage(const gl::Context *context,
                                           gl::Texture *texture,
                                           EGLint buffer)
{
    if (!mFunctionsWGL->bindTexImageARB(mPbuffer, GetWGLBufferBindTarget(buffer)))
    {
        DWORD error = GetLastError();
        return egl::EglBadSurface()
               << "Failed to bind native wgl pbuffer, " << gl::FmtErr(HRESULT_CODE(error));
    }

    return egl::NoError();
}

egl::Error PbufferSurfaceWGL::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    if (!mFunctionsWGL->releaseTexImageARB(mPbuffer, GetWGLBufferBindTarget(buffer)))
    {
        DWORD error = GetLastError();
        return egl::EglBadSurface()
               << "Failed to unbind native wgl pbuffer, " << gl::FmtErr(HRESULT_CODE(error));
    }

    return egl::NoError();
}

void PbufferSurfaceWGL::setSwapInterval(const egl::Display *display, EGLint interval) {}

EGLint PbufferSurfaceWGL::getWidth() const
{
    return mWidth;
}

EGLint PbufferSurfaceWGL::getHeight() const
{
    return mHeight;
}

EGLint PbufferSurfaceWGL::isPostSubBufferSupported() const
{
    return EGL_FALSE;
}

EGLint PbufferSurfaceWGL::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

HDC PbufferSurfaceWGL::getDC() const
{
    return mPbufferDeviceContext;
}
}  // namespace rx
