//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// functionswgl_typedefs.h: Typedefs of WGL functions.

#ifndef LIBANGLE_RENDERER_GL_WGL_FUNCTIONSWGLTYPEDEFS_H_
#define LIBANGLE_RENDERER_GL_WGL_FUNCTIONSWGLTYPEDEFS_H_

#include "common/platform.h"

// This header must be included before wglext.h.
#include <angle_gl.h>

#include <GL/wglext.h>

namespace rx
{

typedef BOOL(WINAPI *PFNWGLCOPYCONTEXTPROC)(HGLRC, HGLRC, UINT);
typedef HGLRC(WINAPI *PFNWGLCREATECONTEXTPROC)(HDC);
typedef HGLRC(WINAPI *PFNWGLCREATELAYERCONTEXTPROC)(HDC, int);
typedef BOOL(WINAPI *PFNWGLDELETECONTEXTPROC)(HGLRC);
typedef HGLRC(WINAPI *PFNWGLGETCURRENTCONTEXTPROC)(VOID);
typedef HDC(WINAPI *PFNWGLGETCURRENTDCPROC)(VOID);
typedef PROC(WINAPI *PFNWGLGETPROCADDRESSPROC)(LPCSTR);
typedef BOOL(WINAPI *PFNWGLMAKECURRENTPROC)(HDC, HGLRC);
typedef BOOL(WINAPI *PFNWGLSHARELISTSPROC)(HGLRC, HGLRC);
typedef BOOL(WINAPI *PFNWGLUSEFONTBITMAPSAPROC)(HDC, DWORD, DWORD, DWORD);
typedef BOOL(WINAPI *PFNWGLUSEFONTBITMAPSWPROC)(HDC, DWORD, DWORD, DWORD);
typedef BOOL(WINAPI *PFNSWAPBUFFERSPROC)(HDC);
typedef BOOL(WINAPI *PFNWGLUSEFONTOUTLINESAPROC)(HDC,
                                                 DWORD,
                                                 DWORD,
                                                 DWORD,
                                                 FLOAT,
                                                 FLOAT,
                                                 int,
                                                 LPGLYPHMETRICSFLOAT);
typedef BOOL(WINAPI *PFNWGLUSEFONTOUTLINESWPROC)(HDC,
                                                 DWORD,
                                                 DWORD,
                                                 DWORD,
                                                 FLOAT,
                                                 FLOAT,
                                                 int,
                                                 LPGLYPHMETRICSFLOAT);
typedef BOOL(WINAPI *PFNWGLDESCRIBELAYERPLANEPROC)(HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR);
typedef int(WINAPI *PFNWGLSETLAYERPALETTEENTRIESPROC)(HDC, int, int, int, CONST COLORREF *);
typedef int(WINAPI *PFNWGLGETLAYERPALETTEENTRIESPROC)(HDC, int, int, int, COLORREF *);
typedef BOOL(WINAPI *PFNWGLREALIZELAYERPALETTEPROC)(HDC, int, BOOL);
typedef BOOL(WINAPI *PFNWGLSWAPLAYERBUFFERSPROC)(HDC, UINT);
typedef DWORD(WINAPI *PFNWGLSWAPMULTIPLEBUFFERSPROC)(UINT, CONST WGLSWAP *);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_WGL_FUNCTIONSWGLTYPEDEFS_H_
