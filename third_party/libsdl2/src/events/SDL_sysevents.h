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
#include "../SDL_internal.h"

#include "../video/SDL_sysvideo.h"

/* Useful functions and variables from SDL_sysevents.c */

#if defined(__HAIKU__)
/* The Haiku event loops run in a separate thread */
#define MUST_THREAD_EVENTS
#endif

#ifdef __WIN32__              /* Windows doesn't allow a separate event thread */
#define CANT_THREAD_EVENTS
#endif

/* vi: set ts=4 sw=4 expandtab: */
