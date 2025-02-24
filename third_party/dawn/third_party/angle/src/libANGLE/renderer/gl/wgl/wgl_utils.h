//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// wgl_utils.h: Utility routines specific to the WGL->EGL implementation.

#ifndef LIBANGLE_RENDERER_GL_WGL_WGLUTILS_H_
#define LIBANGLE_RENDERER_GL_WGL_WGLUTILS_H_

#include <vector>

#include "common/platform.h"

namespace rx
{

class FunctionsWGL;

namespace wgl
{

PIXELFORMATDESCRIPTOR GetDefaultPixelFormatDescriptor();
std::vector<int> GetDefaultPixelFormatAttributes(bool preservedSwap);

int QueryWGLFormatAttrib(HDC dc, int format, int attribName, const FunctionsWGL *functions);
}  // namespace wgl

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_WGLUTILS_H_
