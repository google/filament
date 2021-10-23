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

#if SDL_VIDEO_DRIVER_KMSDRM

#include "SDL_kmsdrmvideo.h"
#include "SDL_kmsdrmevents.h"

#ifdef SDL_INPUT_LINUXEV
#include "../../core/linux/SDL_evdev.h"
#elif defined SDL_INPUT_WSCONS
#include "../../core/openbsd/SDL_wscons.h"
#endif

void KMSDRM_PumpEvents(_THIS)
{
#ifdef SDL_INPUT_LINUXEV
    SDL_EVDEV_Poll();
#elif defined SDL_INPUT_WSCONS
    SDL_WSCONS_PumpEvents();
#endif

}

#endif /* SDL_VIDEO_DRIVER_KMSDRM */

