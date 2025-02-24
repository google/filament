//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLSyncImpl.cpp: Implements the rx::EGLSyncImpl class.

#include "libANGLE/renderer/EGLReusableSync.h"

#include "angle_gl.h"

#include "common/utilities.h"

namespace rx
{

egl::Error EGLSyncImpl::signal(const egl::Display *display, const gl::Context *context, EGLint mode)
{
    UNREACHABLE();
    return egl::EglBadMatch();
}

egl::Error EGLSyncImpl::copyMetalSharedEventANGLE(const egl::Display *display,
                                                  void **eventOut) const
{
    UNREACHABLE();
    return egl::EglBadMatch();
}

egl::Error EGLSyncImpl::dupNativeFenceFD(const egl::Display *display, EGLint *fdOut) const
{
    UNREACHABLE();
    return egl::EglBadMatch();
}

}  // namespace rx
