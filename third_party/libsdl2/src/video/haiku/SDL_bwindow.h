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

#ifndef SDL_BWINDOW_H
#define SDL_BWINDOW_H


#include "../SDL_sysvideo.h"


extern int BE_CreateWindow(_THIS, SDL_Window *window);
extern int BE_CreateWindowFrom(_THIS, SDL_Window * window, const void *data);
extern void BE_SetWindowTitle(_THIS, SDL_Window * window);
extern void BE_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon);
extern void BE_SetWindowPosition(_THIS, SDL_Window * window);
extern void BE_SetWindowSize(_THIS, SDL_Window * window);
extern void BE_ShowWindow(_THIS, SDL_Window * window);
extern void BE_HideWindow(_THIS, SDL_Window * window);
extern void BE_RaiseWindow(_THIS, SDL_Window * window);
extern void BE_MaximizeWindow(_THIS, SDL_Window * window);
extern void BE_MinimizeWindow(_THIS, SDL_Window * window);
extern void BE_RestoreWindow(_THIS, SDL_Window * window);
extern void BE_SetWindowBordered(_THIS, SDL_Window * window, SDL_bool bordered);
extern void BE_SetWindowResizable(_THIS, SDL_Window * window, SDL_bool resizable);
extern void BE_SetWindowFullscreen(_THIS, SDL_Window * window, SDL_VideoDisplay * display, SDL_bool fullscreen);
extern int BE_SetWindowGammaRamp(_THIS, SDL_Window * window, const Uint16 * ramp);
extern int BE_GetWindowGammaRamp(_THIS, SDL_Window * window, Uint16 * ramp);
extern void BE_SetWindowGrab(_THIS, SDL_Window * window, SDL_bool grabbed);
extern void BE_DestroyWindow(_THIS, SDL_Window * window);
extern SDL_bool BE_GetWindowWMInfo(_THIS, SDL_Window * window,
                                    struct SDL_SysWMinfo *info);



#endif

/* vi: set ts=4 sw=4 expandtab: */
