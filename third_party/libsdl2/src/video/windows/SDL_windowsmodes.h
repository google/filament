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

#ifndef SDL_windowsmodes_h_
#define SDL_windowsmodes_h_

typedef struct
{
    WCHAR DeviceName[32];
    HMONITOR MonitorHandle;
    SDL_bool IsValid;
} SDL_DisplayData;

typedef struct
{
    DEVMODE DeviceMode;
} SDL_DisplayModeData;

extern int WIN_InitModes(_THIS);
extern int WIN_GetDisplayBounds(_THIS, SDL_VideoDisplay * display, SDL_Rect * rect);
extern int WIN_GetDisplayUsableBounds(_THIS, SDL_VideoDisplay * display, SDL_Rect * rect);
extern int WIN_GetDisplayDPI(_THIS, SDL_VideoDisplay * display, float * ddpi, float * hdpi, float * vdpi);
extern void WIN_GetDisplayModes(_THIS, SDL_VideoDisplay * display);
extern int WIN_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
extern void WIN_RefreshDisplays(_THIS);
extern void WIN_QuitModes(_THIS);

#endif /* SDL_windowsmodes_h_ */

/* vi: set ts=4 sw=4 expandtab: */
