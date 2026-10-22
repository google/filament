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

#ifndef SDL_mirwindow_h_
#define SDL_mirwindow_h_

#include "../SDL_sysvideo.h"
#include "SDL_syswm.h"

#include "SDL_mirvideo.h"

struct MIR_Window {
    SDL_Window* sdl_window;
    MIR_Data*   mir_data;

    MirWindow*  window;
    EGLSurface  egl_surface;
};


extern int
MIR_CreateWindow(_THIS, SDL_Window* window);

extern void
MIR_DestroyWindow(_THIS, SDL_Window* window);

extern void
MIR_SetWindowFullscreen(_THIS, SDL_Window* window,
                        SDL_VideoDisplay* display,
                        SDL_bool fullscreen);

extern void
MIR_MaximizeWindow(_THIS, SDL_Window* window);

extern void
MIR_MinimizeWindow(_THIS, SDL_Window* window);

extern void
MIR_RestoreWindow(_THIS, SDL_Window* window);

extern void
MIR_HideWindow(_THIS, SDL_Window* window);

extern SDL_bool
MIR_GetWindowWMInfo(_THIS, SDL_Window* window, SDL_SysWMinfo* info);

extern void
MIR_SetWindowSize(_THIS, SDL_Window* window);

extern void
MIR_SetWindowMinimumSize(_THIS, SDL_Window* window);

extern void
MIR_SetWindowMaximumSize(_THIS, SDL_Window* window);

extern void
MIR_SetWindowTitle(_THIS, SDL_Window* window);

extern void
MIR_SetWindowGrab(_THIS, SDL_Window* window, SDL_bool grabbed);

extern int
MIR_SetWindowGammaRamp(_THIS, SDL_Window* window, Uint16 const* ramp);

extern int
MIR_GetWindowGammaRamp(_THIS, SDL_Window* window, Uint16* ramp);

#endif /* SDL_mirwindow_h_ */

/* vi: set ts=4 sw=4 expandtab: */

