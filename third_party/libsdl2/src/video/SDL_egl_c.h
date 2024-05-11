/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../SDL_internal.h"

#ifndef SDL_egl_h_
#define SDL_egl_h_

#if SDL_VIDEO_OPENGL_EGL

#include "SDL_egl.h"

#include "SDL_sysvideo.h"

typedef struct SDL_EGL_VideoData
{
    void *egl_dll_handle, *dll_handle;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    int egl_swapinterval;
    int egl_surfacetype;
    
    EGLDisplay(EGLAPIENTRY *eglGetDisplay) (NativeDisplayType display);
    EGLDisplay(EGLAPIENTRY *eglGetPlatformDisplay) (EGLenum platform,
                                void *native_display,
                                const EGLint *attrib_list);
    EGLDisplay(EGLAPIENTRY *eglGetPlatformDisplayEXT) (EGLenum platform,
                                void *native_display,
                                const EGLint *attrib_list);
    EGLBoolean(EGLAPIENTRY *eglInitialize) (EGLDisplay dpy, EGLint * major,
                                EGLint * minor);
    EGLBoolean(EGLAPIENTRY  *eglTerminate) (EGLDisplay dpy);
    
    void *(EGLAPIENTRY *eglGetProcAddress) (const char * procName);
    
    EGLBoolean(EGLAPIENTRY *eglChooseConfig) (EGLDisplay dpy,
                                  const EGLint * attrib_list,
                                  EGLConfig * configs,
                                  EGLint config_size, EGLint * num_config);
    
    EGLContext(EGLAPIENTRY *eglCreateContext) (EGLDisplay dpy,
                                   EGLConfig config,
                                   EGLContext share_list,
                                   const EGLint * attrib_list);
    
    EGLBoolean(EGLAPIENTRY *eglDestroyContext) (EGLDisplay dpy, EGLContext ctx);
    
    EGLSurface(EGLAPIENTRY *eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config,
                                                     EGLint const* attrib_list);

    EGLSurface(EGLAPIENTRY *eglCreateWindowSurface) (EGLDisplay dpy,
                                         EGLConfig config,
                                         NativeWindowType window,
                                         const EGLint * attrib_list);
    EGLBoolean(EGLAPIENTRY *eglDestroySurface) (EGLDisplay dpy, EGLSurface surface);
    
    EGLBoolean(EGLAPIENTRY *eglMakeCurrent) (EGLDisplay dpy, EGLSurface draw,
                                 EGLSurface read, EGLContext ctx);
    
    EGLBoolean(EGLAPIENTRY *eglSwapBuffers) (EGLDisplay dpy, EGLSurface draw);
    
    EGLBoolean(EGLAPIENTRY *eglSwapInterval) (EGLDisplay dpy, EGLint interval);
    
    const char *(EGLAPIENTRY *eglQueryString) (EGLDisplay dpy, EGLint name);
    
    EGLBoolean(EGLAPIENTRY  *eglGetConfigAttrib) (EGLDisplay dpy, EGLConfig config,
                                     EGLint attribute, EGLint * value);
    
    EGLBoolean(EGLAPIENTRY *eglWaitNative) (EGLint  engine);

    EGLBoolean(EGLAPIENTRY *eglWaitGL)(void);
    
    EGLBoolean(EGLAPIENTRY *eglBindAPI)(EGLenum);

    EGLint(EGLAPIENTRY *eglGetError)(void);

} SDL_EGL_VideoData;

/* OpenGLES functions */
extern int SDL_EGL_GetAttribute(_THIS, SDL_GLattr attrib, int *value);
/* SDL_EGL_LoadLibrary can get a display for a specific platform (EGL_PLATFORM_*)
 * or, if 0 is passed, let the implementation decide.
 */
extern int SDL_EGL_LoadLibrary(_THIS, const char *path, NativeDisplayType native_display, EGLenum platform);
extern void *SDL_EGL_GetProcAddress(_THIS, const char *proc);
extern void SDL_EGL_UnloadLibrary(_THIS);
extern int SDL_EGL_ChooseConfig(_THIS);
extern int SDL_EGL_SetSwapInterval(_THIS, int interval);
extern int SDL_EGL_GetSwapInterval(_THIS);
extern void SDL_EGL_DeleteContext(_THIS, SDL_GLContext context);
extern EGLSurface *SDL_EGL_CreateSurface(_THIS, NativeWindowType nw);
extern void SDL_EGL_DestroySurface(_THIS, EGLSurface egl_surface);

/* These need to be wrapped to get the surface for the window by the platform GLES implementation */
extern SDL_GLContext SDL_EGL_CreateContext(_THIS, EGLSurface egl_surface);
extern int SDL_EGL_MakeCurrent(_THIS, EGLSurface egl_surface, SDL_GLContext context);
extern int SDL_EGL_SwapBuffers(_THIS, EGLSurface egl_surface);

/* SDL Error-reporting */
extern int SDL_EGL_SetErrorEx(const char * message, const char * eglFunctionName, EGLint eglErrorCode);
#define SDL_EGL_SetError(message, eglFunctionName) SDL_EGL_SetErrorEx(message, eglFunctionName, _this->egl_data->eglGetError())

/* A few of useful macros */

#define SDL_EGL_SwapWindow_impl(BACKEND) int \
BACKEND ## _GLES_SwapWindow(_THIS, SDL_Window * window) \
{\
    return SDL_EGL_SwapBuffers(_this, ((SDL_WindowData *) window->driverdata)->egl_surface);\
}

#define SDL_EGL_MakeCurrent_impl(BACKEND) int \
BACKEND ## _GLES_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context) \
{\
    if (window && context) { \
        return SDL_EGL_MakeCurrent(_this, ((SDL_WindowData *) window->driverdata)->egl_surface, context); \
    }\
    else {\
        return SDL_EGL_MakeCurrent(_this, NULL, NULL);\
    }\
}

#define SDL_EGL_CreateContext_impl(BACKEND) SDL_GLContext \
BACKEND ## _GLES_CreateContext(_THIS, SDL_Window * window) \
{\
    return SDL_EGL_CreateContext(_this, ((SDL_WindowData *) window->driverdata)->egl_surface);\
}

#endif /* SDL_VIDEO_OPENGL_EGL */

#endif /* SDL_egl_h_ */

/* vi: set ts=4 sw=4 expandtab: */
