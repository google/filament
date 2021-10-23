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

#if SDL_VIDEO_DRIVER_OFFSCREEN

#include "SDL_offscreenopengl.h"

#include "SDL_opengl.h"

int
OFFSCREEN_GL_SwapWindow(_THIS, SDL_Window* window)
{
    OFFSCREEN_Window* offscreen_wind = window->driverdata;

    SDL_EGL_SwapBuffers(_this, offscreen_wind->egl_surface);
    return 0;
}

int
OFFSCREEN_GL_MakeCurrent(_THIS, SDL_Window* window, SDL_GLContext context)
{
  if (window) {
      EGLSurface egl_surface = ((OFFSCREEN_Window*)window->driverdata)->egl_surface;
      return SDL_EGL_MakeCurrent(_this, egl_surface, context);
  }

  return SDL_EGL_MakeCurrent(_this, NULL, NULL);
}

SDL_GLContext
OFFSCREEN_GL_CreateContext(_THIS, SDL_Window* window)
{
    OFFSCREEN_Window* offscreen_window = window->driverdata;

    SDL_GLContext context;
    context = SDL_EGL_CreateContext(_this, offscreen_window->egl_surface);

    return context;
}

int
OFFSCREEN_GL_LoadLibrary(_THIS, const char* path)
{
    int ret = SDL_EGL_LoadLibraryOnly(_this, path);
    if (ret != 0) {
        return ret;
    }

    ret = SDL_EGL_InitializeOffscreen(_this, 0);
    if (ret != 0) {
        return ret;
    }

    ret = SDL_EGL_ChooseConfig(_this);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

void
OFFSCREEN_GL_UnloadLibrary(_THIS)
{
    SDL_EGL_UnloadLibrary(_this);
}

void*
OFFSCREEN_GL_GetProcAddress(_THIS, const char* proc)
{
    void* proc_addr = SDL_EGL_GetProcAddress(_this, proc);

    if (!proc_addr) {
        SDL_SetError("Failed to find proc address!");
    }

    return proc_addr;
}

#endif /* SDL_VIDEO_DRIVER_OFFSCREEN */

/* vi: set ts=4 sw=4 expandtab: */
