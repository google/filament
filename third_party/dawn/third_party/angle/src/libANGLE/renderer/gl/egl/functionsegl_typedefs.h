//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// functionsegl_typedefs.h: Typedefs of EGL functions.

#ifndef LIBANGLE_RENDERER_GL_EGL_FUNCTIONSEGLTYPEDEFS_H_
#define LIBANGLE_RENDERER_GL_EGL_FUNCTIONSEGLTYPEDEFS_H_

#include <EGL/egl.h>

namespace rx
{
// EGL 1.0
typedef EGLBoolean (*PFNEGLCHOOSECONFIGPROC)(EGLDisplay dpy,
                                             const EGLint *attrib_list,
                                             EGLConfig *configs,
                                             EGLint config_size,
                                             EGLint *num_config);
typedef EGLBoolean (*PFNEGLCOPYBUFFERSPROC)(EGLDisplay dpy,
                                            EGLSurface surface,
                                            EGLNativePixmapType target);
typedef EGLContext (*PFNEGLCREATECONTEXTPROC)(EGLDisplay dpy,
                                              EGLConfig config,
                                              EGLContext share_context,
                                              const EGLint *attrib_list);
typedef EGLSurface (*PFNEGLCREATEPBUFFERSURFACEPROC)(EGLDisplay dpy,
                                                     EGLConfig config,
                                                     const EGLint *attrib_list);
typedef EGLSurface (*PFNEGLCREATEPIXMAPSURFACEPROC)(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    EGLNativePixmapType pixmap,
                                                    const EGLint *attrib_list);
typedef EGLSurface (*PFNEGLCREATEWINDOWSURFACEPROC)(EGLDisplay dpy,
                                                    EGLConfig config,
                                                    EGLNativeWindowType win,
                                                    const EGLint *attrib_list);
typedef EGLBoolean (*PFNEGLDESTROYCONTEXTPROC)(EGLDisplay dpy, EGLContext ctx);
typedef EGLBoolean (*PFNEGLDESTROYSURFACEPROC)(EGLDisplay dpy, EGLSurface surface);
typedef EGLBoolean (*PFNEGLGETCONFIGATTRIBPROC)(EGLDisplay dpy,
                                                EGLConfig config,
                                                EGLint attribute,
                                                EGLint *value);
typedef EGLBoolean (*PFNEGLGETCONFIGSPROC)(EGLDisplay dpy,
                                           EGLConfig *configs,
                                           EGLint config_size,
                                           EGLint *num_config);
typedef EGLDisplay (*PFNEGLGETCURRENTDISPLAYPROC)(void);
typedef EGLSurface (*PFNEGLGETCURRENTSURFACEPROC)(EGLint readdraw);
typedef EGLDisplay (*PFNEGLGETDISPLAYPROC)(EGLNativeDisplayType display_id);
typedef EGLint (*PFNEGLGETERRORPROC)(void);
typedef __eglMustCastToProperFunctionPointerType (*PFNEGLGETPROCADDRESSPROC)(const char *procname);
typedef EGLBoolean (*PFNEGLINITIALIZEPROC)(EGLDisplay dpy, EGLint *major, EGLint *minor);
typedef EGLBoolean (*PFNEGLMAKECURRENTPROC)(EGLDisplay dpy,
                                            EGLSurface draw,
                                            EGLSurface read,
                                            EGLContext ctx);
typedef EGLBoolean (*PFNEGLQUERYCONTEXTPROC)(EGLDisplay dpy,
                                             EGLContext ctx,
                                             EGLint attribute,
                                             EGLint *value);
typedef const char *(*PFNEGLQUERYSTRINGPROC)(EGLDisplay dpy, EGLint name);
typedef EGLBoolean (*PFNEGLQUERYSURFACEPROC)(EGLDisplay dpy,
                                             EGLSurface surface,
                                             EGLint attribute,
                                             EGLint *value);
typedef EGLBoolean (*PFNEGLSWAPBUFFERSPROC)(EGLDisplay dpy, EGLSurface surface);
typedef EGLBoolean (*PFNEGLTERMINATEPROC)(EGLDisplay dpy);
typedef EGLBoolean (*PFNEGLWAITGLPROC)(void);
typedef EGLBoolean (*PFNEGLWAITNATIVEPROC)(EGLint engine);

// EGL 1.1
typedef EGLBoolean (*PFNEGLBINDTEXIMAGEPROC)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
typedef EGLBoolean (*PFNEGLRELEASETEXIMAGEPROC)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
typedef EGLBoolean (*PFNEGLSURFACEATTRIBPROC)(EGLDisplay dpy,
                                              EGLSurface surface,
                                              EGLint attribute,
                                              EGLint value);
typedef EGLBoolean (*PFNEGLSWAPINTERVALPROC)(EGLDisplay dpy, EGLint interval);

// EGL 1.2
typedef EGLBoolean (*PFNEGLBINDAPIPROC)(EGLenum api);
typedef EGLenum (*PFNEGLQUERYAPIPROC)(void);
typedef EGLSurface (*PFNEGLCREATEPBUFFERFROMCLIENTBUFFERPROC)(EGLDisplay dpy,
                                                              EGLenum buftype,
                                                              EGLClientBuffer buffer,
                                                              EGLConfig config,
                                                              const EGLint *attrib_list);
typedef EGLBoolean (*PFNEGLRELEASETHREADPROC)(void);
typedef EGLBoolean (*PFNEGLWAITCLIENTPROC)(void);

// EGL 1.3

// EGL 1.4
typedef EGLContext (*PFNEGLGETCURRENTCONTEXTPROC)(void);

// EGL 1.5
typedef EGLSync (*PFNEGLCREATESYNCPROC)(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list);
typedef EGLBoolean (*PFNEGLDESTROYSYNCPROC)(EGLDisplay dpy, EGLSync sync);
typedef EGLint (*PFNEGLCLIENTWAITSYNCPROC)(EGLDisplay dpy,
                                           EGLSync sync,
                                           EGLint flags,
                                           EGLTime timeout);
typedef EGLBoolean (*PFNEGLGETSYNCATTRIBPROC)(EGLDisplay dpy,
                                              EGLSync sync,
                                              EGLint attribute,
                                              EGLAttrib *value);
typedef EGLImage (*PFNEGLCREATEIMAGEPROC)(EGLDisplay dpy,
                                          EGLContext ctx,
                                          EGLenum target,
                                          EGLClientBuffer buffer,
                                          const EGLAttrib *attrib_list);
typedef EGLBoolean (*PFNEGLDESTROYIMAGEPROC)(EGLDisplay dpy, EGLImage image);
typedef EGLDisplay (*PFNEGLGETPLATFORMDISPLAYPROC)(EGLenum platform,
                                                   void *native_display,
                                                   const EGLAttrib *attrib_list);
typedef EGLSurface (*PFNEGLCREATEPLATFORMWINDOWSURFACEPROC)(EGLDisplay dpy,
                                                            EGLConfig config,
                                                            void *native_window,
                                                            const EGLAttrib *attrib_list);
typedef EGLSurface (*PFNEGLCREATEPLATFORMPIXMAPSURFACEPROC)(EGLDisplay dpy,
                                                            EGLConfig config,
                                                            void *native_pixmap,
                                                            const EGLAttrib *attrib_list);
typedef EGLBoolean (*PFNEGLWAITSYNCPROC)(EGLDisplay dpy, EGLSync sync, EGLint flags);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_FUNCTIONSEGLTYPEDEFS_H_
