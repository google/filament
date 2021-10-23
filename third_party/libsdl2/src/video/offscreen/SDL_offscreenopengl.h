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

#ifndef _SDL_offscreenopengl_h
#define _SDL_offscreenopengl_h

#include "SDL_offscreenwindow.h"

#include "../SDL_egl_c.h"

#define OFFSCREEN_GL_DeleteContext   SDL_EGL_DeleteContext
#define OFFSCREEN_GL_GetSwapInterval SDL_EGL_GetSwapInterval
#define OFFSCREEN_GL_SetSwapInterval SDL_EGL_SetSwapInterval

extern int
OFFSCREEN_GL_SwapWindow(_THIS, SDL_Window* window);

extern int
OFFSCREEN_GL_MakeCurrent(_THIS, SDL_Window* window, SDL_GLContext context);

extern SDL_GLContext
OFFSCREEN_GL_CreateContext(_THIS, SDL_Window* window);

extern int
OFFSCREEN_GL_LoadLibrary(_THIS, const char* path);

extern void
OFFSCREEN_GL_UnloadLibrary(_THIS);

extern void*
OFFSCREEN_GL_GetProcAddress(_THIS, const char* proc);

#endif /* _SDL_offscreenopengl_h */

/* vi: set ts=4 sw=4 expandtab: */

