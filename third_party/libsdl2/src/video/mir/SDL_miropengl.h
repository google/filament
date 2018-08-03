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

#ifndef SDL_miropengl_h_
#define SDL_miropengl_h_

#include "SDL_mirwindow.h"

#include "../SDL_egl_c.h"

#define MIR_GL_DeleteContext   SDL_EGL_DeleteContext
#define MIR_GL_GetSwapInterval SDL_EGL_GetSwapInterval
#define MIR_GL_SetSwapInterval SDL_EGL_SetSwapInterval
#define MIR_GL_UnloadLibrary   SDL_EGL_UnloadLibrary
#define MIR_GL_GetProcAddress  SDL_EGL_GetProcAddress

extern int
MIR_GL_SwapWindow(_THIS, SDL_Window* window);

extern int
MIR_GL_MakeCurrent(_THIS, SDL_Window* window, SDL_GLContext context);

extern SDL_GLContext
MIR_GL_CreateContext(_THIS, SDL_Window* window);

extern int
MIR_GL_LoadLibrary(_THIS, const char* path);

#endif /* SDL_miropengl_h_ */

/* vi: set ts=4 sw=4 expandtab: */
