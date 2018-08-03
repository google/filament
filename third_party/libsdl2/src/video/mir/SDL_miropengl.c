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

/*
  Contributed by Brandon Schaefer, <brandon.schaefer@canonical.com>
*/

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MIR

#include "SDL_miropengl.h"

#include "SDL_mirdyn.h"

int
MIR_GL_SwapWindow(_THIS, SDL_Window* window)
{
    MIR_Window* mir_wind = window->driverdata;

    return SDL_EGL_SwapBuffers(_this, mir_wind->egl_surface);
}

int
MIR_GL_MakeCurrent(_THIS, SDL_Window* window, SDL_GLContext context)
{
  if (window) {
      EGLSurface egl_surface = ((MIR_Window*)window->driverdata)->egl_surface;
      return SDL_EGL_MakeCurrent(_this, egl_surface, context);
  }

  return SDL_EGL_MakeCurrent(_this, NULL, NULL);
}

SDL_GLContext
MIR_GL_CreateContext(_THIS, SDL_Window* window)
{
    MIR_Window* mir_window = window->driverdata;

    SDL_GLContext context;
    context = SDL_EGL_CreateContext(_this, mir_window->egl_surface);

    return context;
}

int
MIR_GL_LoadLibrary(_THIS, const char* path)
{
    MIR_Data* mir_data = _this->driverdata;

    SDL_EGL_LoadLibrary(_this, path, MIR_mir_connection_get_egl_native_display(mir_data->connection), 0);

    SDL_EGL_ChooseConfig(_this);

    return 0;
}

#endif /* SDL_VIDEO_DRIVER_MIR */

/* vi: set ts=4 sw=4 expandtab: */
