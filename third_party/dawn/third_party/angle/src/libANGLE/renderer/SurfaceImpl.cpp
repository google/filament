//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceImpl.cpp: Implementation of Surface stub method class

#include "libANGLE/renderer/SurfaceImpl.h"

namespace rx
{

SurfaceImpl::SurfaceImpl(const egl::SurfaceState &state) : mState(state) {}

SurfaceImpl::~SurfaceImpl() {}

egl::Error SurfaceImpl::makeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error SurfaceImpl::unMakeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error SurfaceImpl::prepareSwap(const gl::Context *)
{
    return angle::ResultToEGL(angle::Result::Continue);
}

egl::Error SurfaceImpl::swapWithDamage(const gl::Context *context,
                                       const EGLint *rects,
                                       EGLint n_rects)
{
    UNREACHABLE();
    return egl::EglBadSurface() << "swapWithDamage implementation missing.";
}

egl::Error SurfaceImpl::swapWithFrameToken(const gl::Context *context,
                                           EGLFrameTokenANGLE frameToken)
{
    UNREACHABLE();
    return egl::EglBadDisplay();
}

egl::Error SurfaceImpl::postSubBuffer(const gl::Context *context,
                                      EGLint x,
                                      EGLint y,
                                      EGLint width,
                                      EGLint height)
{
    UNREACHABLE();
    return egl::EglBadSurface() << "getMscRate implementation missing.";
}

egl::Error SurfaceImpl::setPresentationTime(EGLnsecsANDROID time)
{
    UNREACHABLE();
    return egl::EglBadSurface() << "setPresentationTime implementation missing.";
}

egl::Error SurfaceImpl::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNREACHABLE();
    return egl::EglBadSurface() << "querySurfacePointerANGLE implementation missing.";
}

egl::Error SurfaceImpl::getSyncValues(EGLuint64KHR *ust, EGLuint64KHR *msc, EGLuint64KHR *sbc)
{
    UNREACHABLE();
    return egl::EglBadSurface() << "getSyncValues implementation missing.";
}

egl::Error SurfaceImpl::getMscRate(EGLint *numerator, EGLint *denominator)
{
    UNREACHABLE();
    return egl::EglBadSurface() << "getMscRate implementation missing.";
}

void SurfaceImpl::setFixedWidth(EGLint width)
{
    UNREACHABLE();
}

void SurfaceImpl::setFixedHeight(EGLint height)
{
    UNREACHABLE();
}

void SurfaceImpl::setTimestampsEnabled(bool enabled)
{
    UNREACHABLE();
}

const angle::Format *SurfaceImpl::getD3DTextureColorFormat() const
{
    UNREACHABLE();
    return nullptr;
}

egl::SupportedCompositorTimings SurfaceImpl::getSupportedCompositorTimings() const
{
    UNREACHABLE();
    return egl::SupportedCompositorTimings();
}

egl::Error SurfaceImpl::getCompositorTiming(EGLint numTimestamps,
                                            const EGLint *names,
                                            EGLnsecsANDROID *values) const
{
    UNREACHABLE();
    return egl::EglBadDisplay();
}

egl::Error SurfaceImpl::getNextFrameId(EGLuint64KHR *frameId) const
{
    UNREACHABLE();
    return egl::EglBadDisplay();
}

egl::SupportedTimestamps SurfaceImpl::getSupportedTimestamps() const
{
    UNREACHABLE();
    return egl::SupportedTimestamps();
}

egl::Error SurfaceImpl::getFrameTimestamps(EGLuint64KHR frameId,
                                           EGLint numTimestamps,
                                           const EGLint *timestamps,
                                           EGLnsecsANDROID *values) const
{
    UNREACHABLE();
    return egl::EglBadDisplay();
}
egl::Error SurfaceImpl::getUserWidth(const egl::Display *display, EGLint *value) const
{
    *value = getWidth();
    return egl::NoError();
}

egl::Error SurfaceImpl::getUserHeight(const egl::Display *display, EGLint *value) const
{
    *value = getHeight();
    return egl::NoError();
}

EGLint SurfaceImpl::isPostSubBufferSupported() const
{
    UNREACHABLE();
    return EGL_FALSE;
}

egl::Error SurfaceImpl::getBufferAge(const gl::Context *context, EGLint *age)
{
    *age = 0;
    return egl::NoError();
}

egl::Error SurfaceImpl::setAutoRefreshEnabled(bool enabled)
{
    return egl::EglBadMatch();
}

egl::Error SurfaceImpl::lockSurface(const egl::Display *display,
                                    EGLint usageHint,
                                    bool preservePixels,
                                    uint8_t **bufferPtrOut,
                                    EGLint *bufferPitchOut)
{
    UNREACHABLE();
    return egl::EglBadMatch();
}

egl::Error SurfaceImpl::unlockSurface(const egl::Display *display, bool preservePixels)
{
    UNREACHABLE();
    return egl::EglBadMatch();
}

EGLint SurfaceImpl::origin() const
{
    return EGL_LOWER_LEFT_KHR;
}

egl::Error SurfaceImpl::setRenderBuffer(EGLint renderBuffer)
{
    return egl::NoError();
}

EGLint SurfaceImpl::getCompressionRate(const egl::Display *display) const
{
    UNREACHABLE();
    return EGL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT;
}

}  // namespace rx
