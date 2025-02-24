//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PbufferSurfaceEGL.h: EGL implementation of egl::Surface for pbuffers

#include "libANGLE/renderer/gl/egl/PbufferSurfaceEGL.h"

#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/egl/egl_utils.h"

namespace rx
{

PbufferSurfaceEGL::PbufferSurfaceEGL(const egl::SurfaceState &state,
                                     const FunctionsEGL *egl,
                                     EGLConfig config)
    : SurfaceEGL(state, egl, config)
{}

PbufferSurfaceEGL::~PbufferSurfaceEGL() {}

egl::Error PbufferSurfaceEGL::initialize(const egl::Display *display)
{
    constexpr EGLint kForwardedPBufferSurfaceAttributes[] = {
        EGL_WIDTH,          EGL_HEIGHT,         EGL_LARGEST_PBUFFER, EGL_TEXTURE_FORMAT,
        EGL_TEXTURE_TARGET, EGL_MIPMAP_TEXTURE, EGL_VG_COLORSPACE,   EGL_VG_ALPHA_FORMAT,
    };

    native_egl::AttributeVector nativeAttribs =
        native_egl::TrimAttributeMap(mState.attributes, kForwardedPBufferSurfaceAttributes);
    native_egl::FinalizeAttributeVector(&nativeAttribs);

    mSurface = mEGL->createPbufferSurface(mConfig, nativeAttribs.data());
    if (mSurface == EGL_NO_SURFACE)
    {
        return egl::Error(mEGL->getError(), "eglCreatePbufferSurface failed");
    }

    return egl::NoError();
}

}  // namespace rx
