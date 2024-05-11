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
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_EMSCRIPTEN && SDL_VIDEO_OPENGL_EGL

#include <emscripten/emscripten.h>
#include <GLES2/gl2.h>

#include "SDL_emscriptenvideo.h"
#include "SDL_emscriptenopengles.h"

#define LOAD_FUNC(NAME) _this->egl_data->NAME = NAME;

/* EGL implementation of SDL OpenGL support */

int
Emscripten_GLES_LoadLibrary(_THIS, const char *path) {
    /*we can't load EGL dynamically*/
    _this->egl_data = (struct SDL_EGL_VideoData *) SDL_calloc(1, sizeof(SDL_EGL_VideoData));
    if (!_this->egl_data) {
        return SDL_OutOfMemory();
    }

    /* Emscripten forces you to manually cast eglGetProcAddress to the real
       function type; grep for "__eglMustCastToProperFunctionPointerType" in
       Emscripten's egl.h for details. */
    _this->egl_data->eglGetProcAddress = (void *(EGLAPIENTRY *)(const char *)) eglGetProcAddress;

    LOAD_FUNC(eglGetDisplay);
    LOAD_FUNC(eglInitialize);
    LOAD_FUNC(eglTerminate);
    LOAD_FUNC(eglChooseConfig);
    LOAD_FUNC(eglGetConfigAttrib);
    LOAD_FUNC(eglCreateContext);
    LOAD_FUNC(eglDestroyContext);
    LOAD_FUNC(eglCreateWindowSurface);
    LOAD_FUNC(eglDestroySurface);
    LOAD_FUNC(eglMakeCurrent);
    LOAD_FUNC(eglSwapBuffers);
    LOAD_FUNC(eglSwapInterval);
    LOAD_FUNC(eglWaitNative);
    LOAD_FUNC(eglWaitGL);
    LOAD_FUNC(eglBindAPI);
    
    _this->egl_data->egl_display = _this->egl_data->eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!_this->egl_data->egl_display) {
        return SDL_SetError("Could not get EGL display");
    }
    
    if (_this->egl_data->eglInitialize(_this->egl_data->egl_display, NULL, NULL) != EGL_TRUE) {
        return SDL_SetError("Could not initialize EGL");
    }

    if (path) {
        SDL_strlcpy(_this->gl_config.driver_path, path, sizeof(_this->gl_config.driver_path) - 1);
    } else {
        *_this->gl_config.driver_path = '\0';
    }
    
    return 0;
}

void
Emscripten_GLES_DeleteContext(_THIS, SDL_GLContext context)
{
    /*
    WebGL contexts can't actually be deleted, so we need to reset it.
    ES2 renderer resets state on init anyway, clearing the canvas should be enough
    */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    SDL_EGL_DeleteContext(_this, context);
}

SDL_EGL_CreateContext_impl(Emscripten)
SDL_EGL_SwapWindow_impl(Emscripten)
SDL_EGL_MakeCurrent_impl(Emscripten)

void
Emscripten_GLES_GetDrawableSize(_THIS, SDL_Window * window, int * w, int * h)
{
    SDL_WindowData *data;
    if (window->driverdata) {
        data = (SDL_WindowData *) window->driverdata;

        if (w) {
            *w = window->w * data->pixel_ratio;
        }

        if (h) {
            *h = window->h * data->pixel_ratio;
        }
    }
}

#endif /* SDL_VIDEO_DRIVER_EMSCRIPTEN && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */

