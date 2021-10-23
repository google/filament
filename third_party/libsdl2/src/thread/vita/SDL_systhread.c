/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2015 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_THREAD_VITA

/* VITA thread management routines for SDL */

#include <stdio.h>
#include <stdlib.h>

#include "SDL_error.h"
#include "SDL_thread.h"
#include "../SDL_systhread.h"
#include "../SDL_thread_c.h"
#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>


static int ThreadEntry(SceSize args, void *argp)
{
    SDL_RunThread(*(SDL_Thread **) argp);
    return 0;
}

int SDL_SYS_CreateThread(SDL_Thread *thread)
{
    SceKernelThreadInfo info;
    int priority = 32;

    /* Set priority of new thread to the same as the current thread */
    info.size = sizeof(SceKernelThreadInfo);
    if (sceKernelGetThreadInfo(sceKernelGetThreadId(), &info) == 0) {
        priority = info.currentPriority;
    }

    thread->handle = sceKernelCreateThread("SDL thread", ThreadEntry,
                           priority, 0x10000, 0, 0, NULL);

    if (thread->handle < 0) {
        return SDL_SetError("sceKernelCreateThread() failed");
    }

    sceKernelStartThread(thread->handle, 4, &thread);
    return 0;
}

void SDL_SYS_SetupThread(const char *name)
{
    /* Do nothing. */
}

SDL_threadID SDL_ThreadID(void)
{
    return (SDL_threadID) sceKernelGetThreadId();
}

void SDL_SYS_WaitThread(SDL_Thread *thread)
{
    sceKernelWaitThreadEnd(thread->handle, NULL, NULL);
    sceKernelDeleteThread(thread->handle);
}

void SDL_SYS_DetachThread(SDL_Thread *thread)
{
    /* !!! FIXME: is this correct? */
    sceKernelDeleteThread(thread->handle);
}

void SDL_SYS_KillThread(SDL_Thread *thread)
{
    sceKernelDeleteThread(thread->handle);
}

int SDL_SYS_SetThreadPriority(SDL_ThreadPriority priority)
{
    int value;

    if (priority == SDL_THREAD_PRIORITY_LOW) {
        value = 19;
    } else if (priority == SDL_THREAD_PRIORITY_HIGH) {
        value = -20;
    } else {
        value = 0;
    }

    return sceKernelChangeThreadPriority(sceKernelGetThreadId(),value);

}

#endif /* SDL_THREAD_VITA */

/* vi: set ts=4 sw=4 expandtab: */
