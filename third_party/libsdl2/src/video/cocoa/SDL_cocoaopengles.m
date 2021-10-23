/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_COCOA && SDL_VIDEO_OPENGL_EGL

#include "SDL_cocoavideo.h"
#include "SDL_cocoaopengles.h"
#include "SDL_cocoaopengl.h"

/* EGL implementation of SDL OpenGL support */

int
Cocoa_GLES_LoadLibrary(_THIS, const char *path) {

    /* If the profile requested is not GL ES, switch over to WIN_GL functions  */
    if (_this->gl_config.profile_mask != SDL_GL_CONTEXT_PROFILE_ES) {
#if SDL_VIDEO_OPENGL_CGL
        Cocoa_GLES_UnloadLibrary(_this);
        _this->GL_LoadLibrary = Cocoa_GL_LoadLibrary;
        _this->GL_GetProcAddress = Cocoa_GL_GetProcAddress;
        _this->GL_UnloadLibrary = Cocoa_GL_UnloadLibrary;
        _this->GL_CreateContext = Cocoa_GL_CreateContext;
        _this->GL_MakeCurrent = Cocoa_GL_MakeCurrent;
        _this->GL_SetSwapInterval = Cocoa_GL_SetSwapInterval;
        _this->GL_GetSwapInterval = Cocoa_GL_GetSwapInterval;
        _this->GL_SwapWindow = Cocoa_GL_SwapWindow;
        _this->GL_DeleteContext = Cocoa_GL_DeleteContext;
        return Cocoa_GL_LoadLibrary(_this, path);
#else
        return SDL_SetError("SDL not configured with OpenGL/CGL support");
#endif
    }
    
    if (_this->egl_data == NULL) {
        return SDL_EGL_LoadLibrary(_this, NULL, EGL_DEFAULT_DISPLAY, 0);
    }

    return 0;
}

SDL_GLContext
Cocoa_GLES_CreateContext(_THIS, SDL_Window * window)
{
    SDL_GLContext context;
    SDL_WindowData *data = (SDL_WindowData *)window->driverdata;

#if SDL_VIDEO_OPENGL_CGL
    if (_this->gl_config.profile_mask != SDL_GL_CONTEXT_PROFILE_ES) {
        /* Switch to CGL based functions */
        Cocoa_GLES_UnloadLibrary(_this);
        _this->GL_LoadLibrary = Cocoa_GL_LoadLibrary;
        _this->GL_GetProcAddress = Cocoa_GL_GetProcAddress;
        _this->GL_UnloadLibrary = Cocoa_GL_UnloadLibrary;
        _this->GL_CreateContext = Cocoa_GL_CreateContext;
        _this->GL_MakeCurrent = Cocoa_GL_MakeCurrent;
        _this->GL_SetSwapInterval = Cocoa_GL_SetSwapInterval;
        _this->GL_GetSwapInterval = Cocoa_GL_GetSwapInterval;
        _this->GL_SwapWindow = Cocoa_GL_SwapWindow;
        _this->GL_DeleteContext = Cocoa_GL_DeleteContext;

        if (Cocoa_GL_LoadLibrary(_this, NULL) != 0) {
            return NULL;
        }

        return Cocoa_GL_CreateContext(_this, window);
    }
#endif

    context = SDL_EGL_CreateContext(_this, data->egl_surface);
    return context;
}

void
Cocoa_GLES_DeleteContext(_THIS, SDL_GLContext context)
{
    SDL_EGL_DeleteContext(_this, context);
    Cocoa_GLES_UnloadLibrary(_this);
}

SDL_EGL_SwapWindow_impl(Cocoa)
SDL_EGL_MakeCurrent_impl(Cocoa)

int
Cocoa_GLES_SetupWindow(_THIS, SDL_Window * window)
{
    /* The current context is lost in here; save it and reset it. */
    SDL_WindowData *windowdata = (SDL_WindowData *) window->driverdata;
    SDL_Window *current_win = SDL_GL_GetCurrentWindow();
    SDL_GLContext current_ctx = SDL_GL_GetCurrentContext();


    if (_this->egl_data == NULL) {
        SDL_assert(!_this->gl_config.driver_loaded);
        if (SDL_EGL_LoadLibrary(_this, NULL, EGL_DEFAULT_DISPLAY, 0) < 0) {
            SDL_EGL_UnloadLibrary(_this);
            return -1;
        }
        _this->gl_config.driver_loaded = 1;
    }
  
    /* Create the GLES window surface */
    NSView* v = windowdata->nswindow.contentView;
    windowdata->egl_surface = SDL_EGL_CreateSurface(_this, (NativeWindowType)[v layer]);

    if (windowdata->egl_surface == EGL_NO_SURFACE) {
        return SDL_SetError("Could not create GLES window surface");
    }

    return Cocoa_GLES_MakeCurrent(_this, current_win, current_ctx);
}

#endif /* SDL_VIDEO_DRIVER_COCOA && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
