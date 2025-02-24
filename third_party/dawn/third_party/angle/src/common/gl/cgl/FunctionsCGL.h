//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsCGL.h: Exposing the soft-linked CGL interface.

#ifndef CGL_FUNCTIONS_H_
#define CGL_FUNCTIONS_H_

#include <OpenGL/OpenGL.h>

#include "common/apple/SoftLinking.h"

SOFT_LINK_FRAMEWORK_HEADER(OpenGL)

SOFT_LINK_FUNCTION_HEADER(OpenGL,
                          CGLChoosePixelFormat,
                          CGLError,
                          (const CGLPixelFormatAttribute *attribs,
                           CGLPixelFormatObj *pix,
                           GLint *npix),
                          (attribs, pix, npix))
SOFT_LINK_FUNCTION_HEADER(OpenGL,
                          CGLCreateContext,
                          CGLError,
                          (CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj *ctx),
                          (pix, share, ctx))
SOFT_LINK_FUNCTION_HEADER(
    OpenGL,
    CGLDescribePixelFormat,
    CGLError,
    (CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint *value),
    (pix, pix_num, attrib, value))
SOFT_LINK_FUNCTION_HEADER(
    OpenGL,
    CGLDescribeRenderer,
    CGLError,
    (CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint *value),
    (rend, rend_num, prop, value))
SOFT_LINK_FUNCTION_HEADER(OpenGL, CGLDestroyContext, CGLError, (CGLContextObj ctx), (ctx))
SOFT_LINK_FUNCTION_HEADER(OpenGL, CGLDestroyPixelFormat, CGLError, (CGLPixelFormatObj pix), (pix))
SOFT_LINK_FUNCTION_HEADER(OpenGL,
                          CGLDestroyRendererInfo,
                          CGLError,
                          (CGLRendererInfoObj rend),
                          (rend))
SOFT_LINK_FUNCTION_HEADER(OpenGL, CGLErrorString, const char *, (CGLError error), (error))
SOFT_LINK_FUNCTION_HEADER(OpenGL,
                          CGLQueryRendererInfo,
                          CGLError,
                          (GLuint display_mask, CGLRendererInfoObj *rend, GLint *nrend),
                          (display_mask, rend, nrend))
SOFT_LINK_FUNCTION_HEADER(OpenGL, CGLReleaseContext, void, (CGLContextObj ctx), (ctx))
SOFT_LINK_FUNCTION_HEADER(OpenGL, CGLGetCurrentContext, CGLContextObj, (void), ())
SOFT_LINK_FUNCTION_HEADER(OpenGL, CGLSetCurrentContext, CGLError, (CGLContextObj ctx), (ctx))
SOFT_LINK_FUNCTION_HEADER(OpenGL,
                          CGLSetVirtualScreen,
                          CGLError,
                          (CGLContextObj ctx, GLint screen),
                          (ctx, screen))
SOFT_LINK_FUNCTION_HEADER(
    OpenGL,
    CGLTexImageIOSurface2D,
    CGLError,
    (CGLContextObj ctx,
     GLenum target,
     GLenum internal_format,
     GLsizei width,
     GLsizei height,
     GLenum format,
     GLenum type,
     IOSurfaceRef ioSurface,
     GLuint plane),
    (ctx, target, internal_format, width, height, format, type, ioSurface, plane))
SOFT_LINK_FUNCTION_HEADER(OpenGL, CGLUpdateContext, CGLError, (CGLContextObj ctx), (ctx))

#endif  // CGL_FUNCTIONS_H_
