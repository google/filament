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

#ifndef SDL_mirvideo_h__
#define SDL_mirvideo_h__

#include <EGL/egl.h>
#include <mir_toolkit/mir_client_library.h>
#include "SDL_stdinc.h"

typedef struct MIR_Window MIR_Window;

typedef struct
{
    MirConnection*    connection;
    MirDisplayConfig* display_config;
    MIR_Window*       current_window;
    SDL_bool          software;
    MirPixelFormat    pixel_format;
} MIR_Data;

extern Uint32
MIR_GetSDLPixelFormat(MirPixelFormat format);

#endif /* SDL_mirvideo_h__ */

/* vi: set ts=4 sw=4 expandtab: */
