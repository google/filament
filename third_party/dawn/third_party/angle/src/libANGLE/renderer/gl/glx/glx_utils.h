//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// glx_utils.h: Utility routines specific to the G:X->EGL implementation.

#ifndef LIBANGLE_RENDERER_GL_GLX_GLXUTILS_H_
#define LIBANGLE_RENDERER_GL_GLX_GLXUTILS_H_

#include <string>

#include "common/platform.h"
#include "libANGLE/renderer/gl/glx/FunctionsGLX.h"

namespace rx
{

namespace x11
{

std::string XErrorToString(Display *display, int status);

}  // namespace x11

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_GLX_GLXUTILS_H_
