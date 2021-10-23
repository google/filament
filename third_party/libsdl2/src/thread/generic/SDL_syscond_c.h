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

#include "SDL_thread.h"

#ifndef SDL_syscond_generic_h_
#define SDL_syscond_generic_h_

#if SDL_THREAD_GENERIC_COND_SUFFIX

SDL_cond * SDL_CreateCond_generic(void);
void SDL_DestroyCond_generic(SDL_cond * cond);
int SDL_CondSignal_generic(SDL_cond * cond);
int SDL_CondBroadcast_generic(SDL_cond * cond);
int SDL_CondWait_generic(SDL_cond * cond, SDL_mutex * mutex);
int SDL_CondWaitTimeout_generic(SDL_cond * cond,
                                SDL_mutex * mutex, Uint32 ms);

#endif /* SDL_THREAD_GENERIC_COND_SUFFIX */

#endif /* SDL_syscond_generic_h_ */

/* vi: set ts=4 sw=4 expandtab: */
