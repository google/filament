//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// wgl_utils.cpp: Utility routines specific to the WGL->EGL implementation.

#include "libANGLE/renderer/gl/wgl/wgl_utils.h"

#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"

namespace rx
{

namespace wgl
{

PIXELFORMATDESCRIPTOR GetDefaultPixelFormatDescriptor()
{
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {};
    pixelFormatDescriptor.nSize                 = sizeof(pixelFormatDescriptor);
    pixelFormatDescriptor.nVersion              = 1;
    pixelFormatDescriptor.dwFlags =
        PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDescriptor.iPixelType   = PFD_TYPE_RGBA;
    pixelFormatDescriptor.cColorBits   = 24;
    pixelFormatDescriptor.cAlphaBits   = 8;
    pixelFormatDescriptor.cDepthBits   = 24;
    pixelFormatDescriptor.cStencilBits = 8;
    pixelFormatDescriptor.iLayerType   = PFD_MAIN_PLANE;

    return pixelFormatDescriptor;
}

std::vector<int> GetDefaultPixelFormatAttributes(bool preservedSwap)
{
    std::vector<int> attribs;
    attribs.push_back(WGL_DRAW_TO_WINDOW_ARB);
    attribs.push_back(TRUE);

    attribs.push_back(WGL_ACCELERATION_ARB);
    attribs.push_back(WGL_FULL_ACCELERATION_ARB);

    attribs.push_back(WGL_SUPPORT_OPENGL_ARB);
    attribs.push_back(TRUE);

    attribs.push_back(WGL_DOUBLE_BUFFER_ARB);
    attribs.push_back(TRUE);

    attribs.push_back(WGL_PIXEL_TYPE_ARB);
    attribs.push_back(WGL_TYPE_RGBA_ARB);

    attribs.push_back(WGL_COLOR_BITS_ARB);
    attribs.push_back(24);

    attribs.push_back(WGL_ALPHA_BITS_ARB);
    attribs.push_back(8);

    attribs.push_back(WGL_DEPTH_BITS_ARB);
    attribs.push_back(24);

    attribs.push_back(WGL_STENCIL_BITS_ARB);
    attribs.push_back(8);

    attribs.push_back(WGL_SWAP_METHOD_ARB);
    attribs.push_back(preservedSwap ? WGL_SWAP_COPY_ARB : WGL_SWAP_UNDEFINED_ARB);

    attribs.push_back(0);

    return attribs;
}

int QueryWGLFormatAttrib(HDC dc, int format, int attribName, const FunctionsWGL *functions)
{
    int result = 0;
    if (functions->getPixelFormatAttribivARB == nullptr ||
        !functions->getPixelFormatAttribivARB(dc, format, 0, 1, &attribName, &result))
    {
        return 0;
    }
    return result;
}
}  // namespace wgl

}  // namespace rx
