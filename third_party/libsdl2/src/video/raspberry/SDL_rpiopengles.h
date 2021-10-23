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

#ifndef SDL_rpiopengles_h_
#define SDL_rpiopengles_h_

#if SDL_VIDEO_DRIVER_RPI && SDL_VIDEO_OPENGL_EGL

#include "../SDL_sysvideo.h"
#include "../SDL_egl_c.h"

/* OpenGLES functions */
#define RPI_GLES_GetAttribute SDL_EGL_GetAttribute
#define RPI_GLES_GetProcAddress SDL_EGL_GetProcAddress
#define RPI_GLES_UnloadLibrary SDL_EGL_UnloadLibrary
#define RPI_GLES_SetSwapInterval SDL_EGL_SetSwapInterval
#define RPI_GLES_GetSwapInterval SDL_EGL_GetSwapInterval
#define RPI_GLES_DeleteContext SDL_EGL_DeleteContext

extern int RPI_GLES_LoadLibrary(_THIS, const char *path);
extern SDL_GLContext RPI_GLES_CreateContext(_THIS, SDL_Window * window);
extern int RPI_GLES_SwapWindow(_THIS, SDL_Window * window);
extern int RPI_GLES_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context);
extern void RPI_GLES_DefaultProfileConfig(_THIS, int *mask, int *major, int *minor);

#endif /* SDL_VIDEO_DRIVER_RPI && SDL_VIDEO_OPENGL_EGL */

#endif /* SDL_rpiopengles_h_ */

/* vi: set ts=4 sw=4 expandtab: */
