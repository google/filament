//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsCGL.cpp: Exposing the soft-linked CGL interface.

#include "common/gl/cgl/FunctionsCGL.h"
#include "common/platform.h"

SOFT_LINK_FRAMEWORK_SOURCE(OpenGL)

SOFT_LINK_FUNCTION_SOURCE(OpenGL,
                          CGLChoosePixelFormat,
                          CGLError,
                          (const CGLPixelFormatAttribute *attribs,
                           CGLPixelFormatObj *pix,
                           GLint *npix),
                          (attribs, pix, npix))
SOFT_LINK_FUNCTION_SOURCE(OpenGL,
                          CGLCreateContext,
                          CGLError,
                          (CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj *ctx),
                          (pix, share, ctx))
SOFT_LINK_FUNCTION_SOURCE(
    OpenGL,
    CGLDescribePixelFormat,
    CGLError,
    (CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint *value),
    (pix, pix_num, attrib, value))
SOFT_LINK_FUNCTION_SOURCE(OpenGL, CGLDestroyContext, CGLError, (CGLContextObj ctx), (ctx))
SOFT_LINK_FUNCTION_SOURCE(OpenGL, CGLDestroyPixelFormat, CGLError, (CGLPixelFormatObj pix), (pix))
SOFT_LINK_FUNCTION_SOURCE(OpenGL, CGLErrorString, const char *, (CGLError error), (error))
SOFT_LINK_FUNCTION_SOURCE(OpenGL, CGLReleaseContext, void, (CGLContextObj ctx), (ctx))
SOFT_LINK_FUNCTION_SOURCE(OpenGL, CGLGetCurrentContext, CGLContextObj, (void), ())
SOFT_LINK_FUNCTION_SOURCE(OpenGL, CGLSetCurrentContext, CGLError, (CGLContextObj ctx), (ctx))
SOFT_LINK_FUNCTION_SOURCE(OpenGL,
                          CGLSetVirtualScreen,
                          CGLError,
                          (CGLContextObj ctx, GLint screen),
                          (ctx, screen))
SOFT_LINK_FUNCTION_SOURCE(
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
SOFT_LINK_FUNCTION_SOURCE(OpenGL, CGLUpdateContext, CGLError, (CGLContextObj ctx), (ctx))

SOFT_LINK_FUNCTION_SOURCE(
    OpenGL,
    CGLDescribeRenderer,
    CGLError,
    (CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint *value),
    (rend, rend_num, prop, value))
SOFT_LINK_FUNCTION_SOURCE(OpenGL,
                          CGLDestroyRendererInfo,
                          CGLError,
                          (CGLRendererInfoObj rend),
                          (rend))
SOFT_LINK_FUNCTION_SOURCE(OpenGL,
                          CGLQueryRendererInfo,
                          CGLError,
                          (GLuint display_mask, CGLRendererInfoObj *rend, GLint *nrend),
                          (display_mask, rend, nrend))
