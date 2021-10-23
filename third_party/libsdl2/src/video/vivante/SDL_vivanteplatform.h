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

#ifndef SDL_vivanteplatform_h_
#define SDL_vivanteplatform_h_

#if SDL_VIDEO_DRIVER_VIVANTE

#include "SDL_vivantevideo.h"

#if defined(CAVIUM)
#define VIVANTE_PLATFORM_CAVIUM
#elif defined(MARVELL)
#define VIVANTE_PLATFORM_MARVELL
#else
#define VIVANTE_PLATFORM_GENERIC
#endif

extern int VIVANTE_SetupPlatform(_THIS);
extern char *VIVANTE_GetDisplayName(_THIS);
extern void VIVANTE_UpdateDisplayScale(_THIS);
extern void VIVANTE_CleanupPlatform(_THIS);

#endif /* SDL_VIDEO_DRIVER_VIVANTE */

#endif /* SDL_vivanteplatform_h_ */

/* vi: set ts=4 sw=4 expandtab: */
