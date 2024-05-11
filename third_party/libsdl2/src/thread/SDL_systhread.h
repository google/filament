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

/* These are functions that need to be implemented by a port of SDL */

#ifndef SDL_systhread_h_
#define SDL_systhread_h_

#include "SDL_thread.h"
#include "SDL_thread_c.h"

/* This function creates a thread, passing args to SDL_RunThread(),
   saves a system-dependent thread id in thread->id, and returns 0
   on success.
*/
#ifdef SDL_PASSED_BEGINTHREAD_ENDTHREAD
extern int SDL_SYS_CreateThread(SDL_Thread * thread, void *args,
                                pfnSDL_CurrentBeginThread pfnBeginThread,
                                pfnSDL_CurrentEndThread pfnEndThread);
#else
extern int SDL_SYS_CreateThread(SDL_Thread * thread, void *args);
#endif

/* This function does any necessary setup in the child thread */
extern void SDL_SYS_SetupThread(const char *name);

/* This function sets the current thread priority */
extern int SDL_SYS_SetThreadPriority(SDL_ThreadPriority priority);

/* This function waits for the thread to finish and frees any data
   allocated by SDL_SYS_CreateThread()
 */
extern void SDL_SYS_WaitThread(SDL_Thread * thread);

/* Mark thread as cleaned up as soon as it exits, without joining. */
extern void SDL_SYS_DetachThread(SDL_Thread * thread);

/* Get the thread local storage for this thread */
extern SDL_TLSData *SDL_SYS_GetTLSData(void);

/* Set the thread local storage for this thread */
extern int SDL_SYS_SetTLSData(SDL_TLSData *data);

/* This is for internal SDL use, so we don't need #ifdefs everywhere. */
extern SDL_Thread *
SDL_CreateThreadInternal(int (SDLCALL * fn) (void *), const char *name,
                         const size_t stacksize, void *data);

#endif /* SDL_systhread_h_ */

/* vi: set ts=4 sw=4 expandtab: */
