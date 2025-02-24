//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsWGL.h: Defines the FuntionsWGL class to contain loaded WGL functions

#ifndef LIBANGLE_RENDERER_GL_WGL_FUNCTIONS_WGL
#define LIBANGLE_RENDERER_GL_WGL_FUNCTIONS_WGL

#include "common/angleutils.h"
#include "libANGLE/renderer/gl/wgl/functionswgl_typedefs.h"

namespace rx
{

class FunctionsWGL : angle::NonCopyable
{
  public:
    FunctionsWGL();
    ~FunctionsWGL();

    // Loads all available wgl functions, may be called multiple times
    void initialize(HMODULE glModule, HDC context);

    // Extension information
    std::vector<std::string> extensions;
    bool hasExtension(const std::string &ext) const;

    // Base WGL functions
    PFNWGLCOPYCONTEXTPROC copyContext;
    PFNWGLCREATECONTEXTPROC createContext;
    PFNWGLCREATELAYERCONTEXTPROC createLayerContext;
    PFNWGLDELETECONTEXTPROC deleteContext;
    PFNWGLGETCURRENTCONTEXTPROC getCurrentContext;
    PFNWGLGETCURRENTDCPROC getCurrentDC;
    PFNWGLGETPROCADDRESSPROC getProcAddress;
    PFNWGLMAKECURRENTPROC makeCurrent;
    PFNWGLSHARELISTSPROC shareLists;
    PFNWGLUSEFONTBITMAPSAPROC useFontBitmapsA;
    PFNWGLUSEFONTBITMAPSWPROC useFontBitmapsW;
    PFNSWAPBUFFERSPROC swapBuffers;
    PFNWGLUSEFONTOUTLINESAPROC useFontOutlinesA;
    PFNWGLUSEFONTOUTLINESWPROC useFontOutlinesW;
    PFNWGLDESCRIBELAYERPLANEPROC describeLayerPlane;
    PFNWGLSETLAYERPALETTEENTRIESPROC setLayerPaletteEntries;
    PFNWGLGETLAYERPALETTEENTRIESPROC getLayerPaletteEntries;
    PFNWGLREALIZELAYERPALETTEPROC realizeLayerPalette;
    PFNWGLSWAPLAYERBUFFERSPROC swapLayerBuffers;
    PFNWGLSWAPMULTIPLEBUFFERSPROC swapMultipleBuffers;

    // WGL_EXT_extensions_string
    PFNWGLGETEXTENSIONSSTRINGEXTPROC getExtensionStringEXT;

    // WGL_ARB_extensions_string
    PFNWGLGETEXTENSIONSSTRINGARBPROC getExtensionStringARB;

    // WGL_ARB_create_context
    PFNWGLCREATECONTEXTATTRIBSARBPROC createContextAttribsARB;

    // WGL_ARB_pixel_format
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC getPixelFormatAttribivARB;
    PFNWGLGETPIXELFORMATATTRIBFVARBPROC getPixelFormatAttribfvARB;
    PFNWGLCHOOSEPIXELFORMATARBPROC choosePixelFormatARB;

    // WGL_EXT_swap_control
    PFNWGLSWAPINTERVALEXTPROC swapIntervalEXT;

    // WGL_ARB_pbuffer
    PFNWGLCREATEPBUFFERARBPROC createPbufferARB;
    PFNWGLGETPBUFFERDCARBPROC getPbufferDCARB;
    PFNWGLRELEASEPBUFFERDCARBPROC releasePbufferDCARB;
    PFNWGLDESTROYPBUFFERARBPROC destroyPbufferARB;
    PFNWGLQUERYPBUFFERARBPROC queryPbufferARB;

    // WGL_ARB_render_texture
    PFNWGLBINDTEXIMAGEARBPROC bindTexImageARB;
    PFNWGLRELEASETEXIMAGEARBPROC releaseTexImageARB;
    PFNWGLSETPBUFFERATTRIBARBPROC setPbufferAttribARB;

    // WGL_NV_DX_interop
    PFNWGLDXSETRESOURCESHAREHANDLENVPROC dxSetResourceShareHandleNV;
    PFNWGLDXOPENDEVICENVPROC dxOpenDeviceNV;
    PFNWGLDXCLOSEDEVICENVPROC dxCloseDeviceNV;
    PFNWGLDXREGISTEROBJECTNVPROC dxRegisterObjectNV;
    PFNWGLDXUNREGISTEROBJECTNVPROC dxUnregisterObjectNV;
    PFNWGLDXOBJECTACCESSNVPROC dxObjectAccessNV;
    PFNWGLDXLOCKOBJECTSNVPROC dxLockObjectsNV;
    PFNWGLDXUNLOCKOBJECTSNVPROC dxUnlockObjectsNV;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_FUNCTIONS_WGL
