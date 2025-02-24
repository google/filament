//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsWGL.h: Implements the FuntionsWGL class.

#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"

#include <algorithm>

#include "common/string_utils.h"

namespace rx
{

template <typename T>
static void GetWGLProcAddress(HMODULE glModule,
                              PFNWGLGETPROCADDRESSPROC getProcAddressWGL,
                              const std::string &procName,
                              T *outProcAddress)
{
    T proc = nullptr;
    if (getProcAddressWGL)
    {
        proc = reinterpret_cast<T>(getProcAddressWGL(procName.c_str()));
    }

    if (!proc)
    {
        proc = reinterpret_cast<T>(GetProcAddress(glModule, procName.c_str()));
    }

    *outProcAddress = proc;
}

template <typename T>
static void GetWGLExtensionProcAddress(HMODULE glModule,
                                       PFNWGLGETPROCADDRESSPROC getProcAddressWGL,
                                       const std::vector<std::string> &extensions,
                                       const std::string &extensionName,
                                       const std::string &procName,
                                       T *outProcAddress)
{
    T proc = nullptr;
    if (std::find(extensions.begin(), extensions.end(), extensionName) != extensions.end())
    {
        GetWGLProcAddress(glModule, getProcAddressWGL, procName, &proc);
    }

    *outProcAddress = proc;
}

FunctionsWGL::FunctionsWGL()
    : copyContext(nullptr),
      createContext(nullptr),
      createLayerContext(nullptr),
      deleteContext(nullptr),
      getCurrentContext(nullptr),
      getCurrentDC(nullptr),
      getProcAddress(nullptr),
      makeCurrent(nullptr),
      shareLists(nullptr),
      useFontBitmapsA(nullptr),
      useFontBitmapsW(nullptr),
      swapBuffers(nullptr),
      useFontOutlinesA(nullptr),
      useFontOutlinesW(nullptr),
      describeLayerPlane(nullptr),
      setLayerPaletteEntries(nullptr),
      getLayerPaletteEntries(nullptr),
      realizeLayerPalette(nullptr),
      swapLayerBuffers(nullptr),
      swapMultipleBuffers(nullptr),
      getExtensionStringEXT(nullptr),
      getExtensionStringARB(nullptr),
      createContextAttribsARB(nullptr),
      getPixelFormatAttribivARB(nullptr),
      getPixelFormatAttribfvARB(nullptr),
      choosePixelFormatARB(nullptr),
      swapIntervalEXT(nullptr),
      createPbufferARB(nullptr),
      getPbufferDCARB(nullptr),
      releasePbufferDCARB(nullptr),
      destroyPbufferARB(nullptr),
      queryPbufferARB(nullptr),
      bindTexImageARB(nullptr),
      releaseTexImageARB(nullptr),
      setPbufferAttribARB(nullptr),
      dxSetResourceShareHandleNV(nullptr),
      dxOpenDeviceNV(nullptr),
      dxCloseDeviceNV(nullptr),
      dxRegisterObjectNV(nullptr),
      dxUnregisterObjectNV(nullptr),
      dxObjectAccessNV(nullptr),
      dxLockObjectsNV(nullptr),
      dxUnlockObjectsNV(nullptr)
{}

FunctionsWGL::~FunctionsWGL() {}

void FunctionsWGL::initialize(HMODULE glModule, HDC context)
{
    // First grab the wglGetProcAddress function from the gl module
    GetWGLProcAddress(glModule, nullptr, "wglGetProcAddress", &getProcAddress);

    // Load the core wgl functions
    GetWGLProcAddress(glModule, getProcAddress, "wglCopyContext", &copyContext);
    GetWGLProcAddress(glModule, getProcAddress, "wglCreateContext", &createContext);
    GetWGLProcAddress(glModule, getProcAddress, "wglCreateLayerContext", &createLayerContext);
    GetWGLProcAddress(glModule, getProcAddress, "wglDeleteContext", &deleteContext);
    GetWGLProcAddress(glModule, getProcAddress, "wglGetCurrentContext", &getCurrentContext);
    GetWGLProcAddress(glModule, getProcAddress, "wglGetCurrentDC", &getCurrentDC);
    GetWGLProcAddress(glModule, getProcAddress, "wglMakeCurrent", &makeCurrent);
    GetWGLProcAddress(glModule, getProcAddress, "wglShareLists", &shareLists);
    GetWGLProcAddress(glModule, getProcAddress, "wglUseFontBitmapsA", &useFontBitmapsA);
    GetWGLProcAddress(glModule, getProcAddress, "wglUseFontBitmapsW", &useFontBitmapsW);
    swapBuffers = SwapBuffers;  // SwapBuffers is statically linked from GDI
    GetWGLProcAddress(glModule, getProcAddress, "wglUseFontOutlinesA", &useFontOutlinesA);
    GetWGLProcAddress(glModule, getProcAddress, "wglUseFontOutlinesW", &useFontOutlinesW);
    GetWGLProcAddress(glModule, getProcAddress, "wglDescribeLayerPlane", &describeLayerPlane);
    GetWGLProcAddress(glModule, getProcAddress, "wglSetLayerPaletteEntries",
                      &setLayerPaletteEntries);
    GetWGLProcAddress(glModule, getProcAddress, "wglGetLayerPaletteEntries",
                      &getLayerPaletteEntries);
    GetWGLProcAddress(glModule, getProcAddress, "wglRealizeLayerPalette", &realizeLayerPalette);
    GetWGLProcAddress(glModule, getProcAddress, "wglSwapLayerBuffers", &swapLayerBuffers);
    GetWGLProcAddress(glModule, getProcAddress, "wglSwapMultipleBuffers", &swapMultipleBuffers);

    // Load extension string getter functions
    GetWGLProcAddress(glModule, getProcAddress, "wglGetExtensionsStringEXT",
                      &getExtensionStringEXT);
    GetWGLProcAddress(glModule, getProcAddress, "wglGetExtensionsStringARB",
                      &getExtensionStringARB);

    std::string extensionString = "";
    if (getExtensionStringEXT)
    {
        extensionString = getExtensionStringEXT();
    }
    else if (getExtensionStringARB && context)
    {
        extensionString = getExtensionStringARB(context);
    }
    angle::SplitStringAlongWhitespace(extensionString, &extensions);

    // Load the wgl extension functions by checking if the context supports the extension first

    // WGL_ARB_create_context
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_create_context",
                               "wglCreateContextAttribsARB", &createContextAttribsARB);

    // WGL_ARB_pixel_format
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_pixel_format",
                               "wglGetPixelFormatAttribivARB", &getPixelFormatAttribivARB);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_pixel_format",
                               "wglGetPixelFormatAttribfvARB", &getPixelFormatAttribfvARB);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_pixel_format",
                               "wglChoosePixelFormatARB", &choosePixelFormatARB);

    // WGL_EXT_swap_control
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_EXT_swap_control",
                               "wglSwapIntervalEXT", &swapIntervalEXT);

    // WGL_ARB_pbuffer
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_pbuffer",
                               "wglCreatePbufferARB", &createPbufferARB);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_pbuffer",
                               "wglGetPbufferDCARB", &getPbufferDCARB);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_pbuffer",
                               "wglReleasePbufferDCARB", &releasePbufferDCARB);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_pbuffer",
                               "wglDestroyPbufferARB", &destroyPbufferARB);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_pbuffer",
                               "wglQueryPbufferARB", &queryPbufferARB);

    // WGL_ARB_render_texture
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_render_texture",
                               "wglBindTexImageARB", &bindTexImageARB);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_render_texture",
                               "wglReleaseTexImageARB", &releaseTexImageARB);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_ARB_render_texture",
                               "wglSetPbufferAttribARB", &setPbufferAttribARB);

    // WGL_NV_DX_interop
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_NV_DX_interop",
                               "wglDXSetResourceShareHandleNV", &dxSetResourceShareHandleNV);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_NV_DX_interop",
                               "wglDXOpenDeviceNV", &dxOpenDeviceNV);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_NV_DX_interop",
                               "wglDXCloseDeviceNV", &dxCloseDeviceNV);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_NV_DX_interop",
                               "wglDXRegisterObjectNV", &dxRegisterObjectNV);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_NV_DX_interop",
                               "wglDXUnregisterObjectNV", &dxUnregisterObjectNV);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_NV_DX_interop",
                               "wglDXObjectAccessNV", &dxObjectAccessNV);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_NV_DX_interop",
                               "wglDXLockObjectsNV", &dxLockObjectsNV);
    GetWGLExtensionProcAddress(glModule, getProcAddress, extensions, "WGL_NV_DX_interop",
                               "wglDXUnlockObjectsNV", &dxUnlockObjectsNV);
}

bool FunctionsWGL::hasExtension(const std::string &ext) const
{
    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}
}  // namespace rx
