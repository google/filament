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

#ifdef __LINUX__

#include "SDL_error.h"
#include "SDL_stdinc.h"
#include "SDL_thread.h"

#if !SDL_THREADS_DISABLED
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include "SDL_system.h"

/* RLIMIT_RTTIME requires kernel >= 2.6.25 and is in glibc >= 2.14 */
#ifndef RLIMIT_RTTIME
#define RLIMIT_RTTIME 15
#endif
/* SCHED_RESET_ON_FORK is in kernel >= 2.6.32. */
#ifndef SCHED_RESET_ON_FORK
#define SCHED_RESET_ON_FORK 0x40000000
#endif

#include "SDL_dbus.h"

#if SDL_USE_LIBDBUS
#include <sched.h>

/* d-bus queries to org.freedesktop.RealtimeKit1. */
#define RTKIT_DBUS_NODE "org.freedesktop.RealtimeKit1"
#define RTKIT_DBUS_PATH "/org/freedesktop/RealtimeKit1"
#define RTKIT_DBUS_INTERFACE "org.freedesktop.RealtimeKit1"

static pthread_once_t rtkit_initialize_once = PTHREAD_ONCE_INIT;
static Sint32 rtkit_min_nice_level = -20;
static Sint32 rtkit_max_realtime_priority = 99;
static Sint64 rtkit_max_rttime_usec = 200000;

static void
rtkit_initialize()
{
    SDL_DBusContext *dbus = SDL_DBus_GetContext();

    /* Try getting minimum nice level: this is often greater than PRIO_MIN (-20). */
    if (!dbus || !SDL_DBus_QueryPropertyOnConnection(dbus->system_conn, RTKIT_DBUS_NODE, RTKIT_DBUS_PATH, RTKIT_DBUS_INTERFACE, "MinNiceLevel",
                                            DBUS_TYPE_INT32, &rtkit_min_nice_level)) {
        rtkit_min_nice_level = -20;
    }

    /* Try getting maximum realtime priority: this can be less than the POSIX default (99). */
    if (!dbus || !SDL_DBus_QueryPropertyOnConnection(dbus->system_conn, RTKIT_DBUS_NODE, RTKIT_DBUS_PATH, RTKIT_DBUS_INTERFACE, "MaxRealtimePriority",
                                            DBUS_TYPE_INT32, &rtkit_max_realtime_priority)) {
        rtkit_max_realtime_priority = 99;
    }

    /* Try getting maximum rttime allowed by rtkit: exceeding this value will result in SIGKILL */
    if (!dbus || !SDL_DBus_QueryPropertyOnConnection(dbus->system_conn, RTKIT_DBUS_NODE, RTKIT_DBUS_PATH, RTKIT_DBUS_INTERFACE, "RTTimeUSecMax",
                                            DBUS_TYPE_INT64, &rtkit_max_rttime_usec)) {
        rtkit_max_rttime_usec = 200000;
    }
}

static SDL_bool
rtkit_initialize_realtime_thread()
{
    // Following is an excerpt from rtkit README that outlines the requirements
    // a thread must meet before making rtkit requests:
    //
    //   * Only clients with RLIMIT_RTTIME set will get RT scheduling
    //
    //   * RT scheduling will only be handed out to processes with
    //     SCHED_RESET_ON_FORK set to guarantee that the scheduling
    //     settings cannot 'leak' to child processes, thus making sure
    //     that 'RT fork bombs' cannot be used to bypass RLIMIT_RTTIME
    //     and take the system down.
    //
    //   * Limits are enforced on all user controllable resources, only
    //     a maximum number of users, processes, threads can request RT
    //     scheduling at the same time.
    //
    //   * Only a limited number of threads may be made RT in a
    //     specific time frame.
    //
    //   * Client authorization is verified with PolicyKit

    int err;
    struct rlimit rlimit;
    int nLimit = RLIMIT_RTTIME;
    pid_t nPid = 0; //self
    int nSchedPolicy = sched_getscheduler(nPid) | SCHED_RESET_ON_FORK;
    struct sched_param schedParam = {};

    // Requirement #1: Set RLIMIT_RTTIME
    err = getrlimit(nLimit, &rlimit);
    if (err)
    {
        return SDL_FALSE;
    }

    // Current rtkit allows a max of 200ms right now
    rlimit.rlim_max = rtkit_max_rttime_usec;
    rlimit.rlim_cur = rlimit.rlim_max / 2;
    err = setrlimit(nLimit, &rlimit);
    if (err)
    {
        return SDL_FALSE;
    }

    // Requirement #2: Add SCHED_RESET_ON_FORK to the scheduler policy
    err = sched_getparam(nPid, &schedParam);
    if (err)
    {
        return SDL_FALSE;
    }

    err = sched_setscheduler(nPid, nSchedPolicy, &schedParam);
    if (err)
    {
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

static SDL_bool
rtkit_setpriority_nice(pid_t thread, int nice_level)
{
    Uint64 ui64 = (Uint64)thread;
    Sint32 si32 = (Sint32)nice_level;
    SDL_DBusContext *dbus = SDL_DBus_GetContext();

    pthread_once(&rtkit_initialize_once, rtkit_initialize);

    if (si32 < rtkit_min_nice_level)
        si32 = rtkit_min_nice_level;

    if (!dbus || !SDL_DBus_CallMethodOnConnection(dbus->system_conn,
            RTKIT_DBUS_NODE, RTKIT_DBUS_PATH, RTKIT_DBUS_INTERFACE, "MakeThreadHighPriority",
            DBUS_TYPE_UINT64, &ui64, DBUS_TYPE_INT32, &si32, DBUS_TYPE_INVALID,
            DBUS_TYPE_INVALID)) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static SDL_bool
rtkit_setpriority_realtime(pid_t thread, int rt_priority)
{
    Uint64 ui64 = (Uint64)thread;
    Uint32 ui32 = (Uint32)rt_priority;
    SDL_DBusContext *dbus = SDL_DBus_GetContext();

    pthread_once(&rtkit_initialize_once, rtkit_initialize);

    if (ui32 > rtkit_max_realtime_priority)
        ui32 = rtkit_max_realtime_priority;

    // We always perform the thread state changes necessary for rtkit.
    // This wastes some system calls if the state is already set but
    // typically code sets a thread priority and leaves it so it's
    // not expected that this wasted effort will be an issue.
    // We also do not quit if this fails, we let the rtkit request
    // go through to determine whether it really needs to fail or not.
    rtkit_initialize_realtime_thread();

    if (!dbus || !SDL_DBus_CallMethodOnConnection(dbus->system_conn,
            RTKIT_DBUS_NODE, RTKIT_DBUS_PATH, RTKIT_DBUS_INTERFACE, "MakeThreadRealtime",
            DBUS_TYPE_UINT64, &ui64, DBUS_TYPE_UINT32, &ui32, DBUS_TYPE_INVALID,
            DBUS_TYPE_INVALID)) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}
#else

#define rtkit_max_realtime_priority 99

#endif /* dbus */
#endif /* threads */

/* this is a public symbol, so it has to exist even if threads are disabled. */
int
SDL_LinuxSetThreadPriority(Sint64 threadID, int priority)
{
#if SDL_THREADS_DISABLED
    return SDL_Unsupported();
#else
    if (setpriority(PRIO_PROCESS, (id_t)threadID, priority) == 0) {
        return 0;
    }

#if SDL_USE_LIBDBUS
    /* Note that this fails you most likely:
         * Have your process's scheduler incorrectly configured.
           See the requirements at:
           http://git.0pointer.net/rtkit.git/tree/README#n16
         * Encountered dbus/polkit security restrictions. Note
           that the RealtimeKit1 dbus endpoint is inaccessible
           over ssh connections for most common distro configs.
           You might want to check your local config for details:
           /usr/share/polkit-1/actions/org.freedesktop.RealtimeKit1.policy

       README and sample code at: http://git.0pointer.net/rtkit.git
    */
    if (rtkit_setpriority_nice((pid_t)threadID, priority)) {
        return 0;
    }
#endif

    return SDL_SetError("setpriority() failed");
#endif
}

/* this is a public symbol, so it has to exist even if threads are disabled. */
int
SDL_LinuxSetThreadPriorityAndPolicy(Sint64 threadID, int sdlPriority, int schedPolicy)
{
#if SDL_THREADS_DISABLED
    return SDL_Unsupported();
#else
    int osPriority;

    if (schedPolicy == SCHED_RR || schedPolicy == SCHED_FIFO) {
        if (sdlPriority == SDL_THREAD_PRIORITY_LOW) {
            osPriority = 1;
        } else if (sdlPriority == SDL_THREAD_PRIORITY_HIGH) {
            osPriority = rtkit_max_realtime_priority * 3 / 4;
        } else if (sdlPriority == SDL_THREAD_PRIORITY_TIME_CRITICAL) {
            osPriority = rtkit_max_realtime_priority;
        } else {
            osPriority = rtkit_max_realtime_priority / 2;
        }
    } else {
        if (sdlPriority == SDL_THREAD_PRIORITY_LOW) {
            osPriority = 19;
        } else if (sdlPriority == SDL_THREAD_PRIORITY_HIGH) {
            osPriority = -10;
        } else if (sdlPriority == SDL_THREAD_PRIORITY_TIME_CRITICAL) {
            osPriority = -20;
        } else {
            osPriority = 0;
        }

        if (setpriority(PRIO_PROCESS, (id_t)threadID, osPriority) == 0) {
            return 0;
        }
    }

#if SDL_USE_LIBDBUS
    /* Note that this fails you most likely:
     * Have your process's scheduler incorrectly configured.
       See the requirements at:
       http://git.0pointer.net/rtkit.git/tree/README#n16
     * Encountered dbus/polkit security restrictions. Note
       that the RealtimeKit1 dbus endpoint is inaccessible
       over ssh connections for most common distro configs.
       You might want to check your local config for details:
       /usr/share/polkit-1/actions/org.freedesktop.RealtimeKit1.policy

       README and sample code at: http://git.0pointer.net/rtkit.git
    */
    if (schedPolicy == SCHED_RR || schedPolicy == SCHED_FIFO) {
        if (rtkit_setpriority_realtime((pid_t)threadID, osPriority)) {
            return 0;
        }
    } else {
        if (rtkit_setpriority_nice((pid_t)threadID, osPriority)) {
            return 0;
        }
    }
#endif

    return SDL_SetError("setpriority() failed");
#endif
}

#endif  /* __LINUX__ */

/* vi: set ts=4 sw=4 expandtab: */
