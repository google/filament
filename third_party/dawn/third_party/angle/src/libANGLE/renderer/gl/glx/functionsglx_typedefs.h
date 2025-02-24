//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// functionsglx_typedefs.h: Typedefs of GLX functions.

#ifndef LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLXTYPEDEFS_H_
#define LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLXTYPEDEFS_H_

#include "libANGLE/renderer/gl/glx/platform_glx.h"

namespace rx
{

// Only the functions of GLX 1.2 and earlier need to be typdefed; the other
// functions are already typedefed in glx.h

// GLX 1.0
typedef XVisualInfo *(*PFNGLXCHOOSEVISUALPROC)(Display *dpy, int screen, int *attribList);
typedef GLXContext (*PFNGLXCREATECONTEXTPROC)(Display *dpy,
                                              XVisualInfo *vis,
                                              GLXContext shareList,
                                              Bool direct);
typedef void (*PFNGLXDESTROYCONTEXTPROC)(Display *dpy, GLXContext ctx);
typedef Bool (*PFNGLXMAKECURRENTPROC)(Display *dpy, GLXDrawable drawable, GLXContext ctx);
typedef void (*PFNGLXCOPYCONTEXTPROC)(Display *dpy,
                                      GLXContext src,
                                      GLXContext dst,
                                      unsigned long mask);
typedef void (*PFNGLXSWAPBUFFERSPROC)(Display *dpy, GLXDrawable drawable);
typedef GLXPixmap (*PFNGLXCREATEGLXPIXMAPPROC)(Display *dpy, XVisualInfo *visual, Pixmap pixmap);
typedef void (*PFNGLXDESTROYGLXPIXMAPPROC)(Display *dpy, GLXPixmap pixmap);
typedef Bool (*PFNGLXQUERYEXTENSIONPROC)(Display *dpy, int *errorb, int *event);
typedef Bool (*PFNGLXQUERYVERSIONPROC)(Display *dpy, int *maj, int *min);
typedef Bool (*PFNGLXISDIRECTPROC)(Display *dpy, GLXContext ctx);
typedef int (*PFNGLXGETCONFIGPROC)(Display *dpy, XVisualInfo *visual, int attrib, int *value);
typedef GLXContext (*PFNGLXGETCURRENTCONTEXTPROC)();
typedef GLXDrawable (*PFNGLXGETCURRENTDRAWABLEPROC)();
typedef GLXContext (*PFNGLXGETCURRENTCONTEXTPROC)();
typedef GLXDrawable (*PFNGLXGETCURRENTDRAWABLEPROC)();
typedef void (*PFNGLXWAITGLPROC)();
typedef void (*PFNGLXWAITXPROC)();
typedef void (*PFNGLXUSEXFONT)(Font font, int first, int count, int list);

// GLX 1.1
typedef const char *(*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display *dpy, int screen);
typedef const char *(*PFNGLXQUERYSERVERSTRINGPROC)(Display *dpy, int screen, int name);
typedef const char *(*PFNGLXGETCLIENTSTRINGPROC)(Display *dpy, int name);

// GLX 1.2
typedef Display *(*PFNGLXGETCURRENTDISPLAYPROC)();

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_GLX_FUNCTIONSGLXTYPEDEFS_H_
