//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_API_H_
#define LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_API_H_

#include "libANGLE/renderer/DisplayImpl.h"

namespace rx
{
DisplayImpl *CreateGLXDisplay(const egl::DisplayState &state);
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_GLX_DISPLAYGLX_API_H_
