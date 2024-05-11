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
#include "SDL_hints.h"
#include "SDL_log.h"

#if SDL_VIDEO_DRIVER_RPI && SDL_VIDEO_OPENGL_EGL

#include "SDL_rpivideo.h"
#include "SDL_rpiopengles.h"

/* EGL implementation of SDL OpenGL support */

void
RPI_GLES_DefaultProfileConfig(_THIS, int *mask, int *major, int *minor)
{
    *mask = SDL_GL_CONTEXT_PROFILE_ES;
    *major = 2;
    *minor = 0;
}

int
RPI_GLES_LoadLibrary(_THIS, const char *path) {
    return SDL_EGL_LoadLibrary(_this, path, EGL_DEFAULT_DISPLAY, 0);
}

int
RPI_GLES_SwapWindow(_THIS, SDL_Window * window) {
    SDL_WindowData *wdata = ((SDL_WindowData *) window->driverdata);

    if (!(_this->egl_data->eglSwapBuffers(_this->egl_data->egl_display, wdata->egl_surface))) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "eglSwapBuffers failed.");
        return 0;
    }

    /* Wait immediately for vsync (as if we only had two buffers), for low input-lag scenarios.
     * Run your SDL2 program with "SDL_RPI_DOUBLE_BUFFER=1 <program_name>" to enable this. */
    if (wdata->double_buffer) {
        SDL_LockMutex(wdata->vsync_cond_mutex);
        SDL_CondWait(wdata->vsync_cond, wdata->vsync_cond_mutex);
        SDL_UnlockMutex(wdata->vsync_cond_mutex);
    }

    return 0;
}

SDL_EGL_CreateContext_impl(RPI)
SDL_EGL_MakeCurrent_impl(RPI)

#endif /* SDL_VIDEO_DRIVER_RPI && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */

