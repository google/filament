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
#include "./SDL_internal.h"

#if defined(__WIN32__)
#include "core/windows/SDL_windows.h"
#elif defined(__OS2__)
#include <stdlib.h> /* For _exit() */
#elif !defined(__WINRT__)
#include <unistd.h> /* For _exit(), etc. */
#endif
#if defined(__OS2__)
#include "core/os2/SDL_os2.h"
#endif
#if SDL_THREAD_OS2
#include "thread/os2/SDL_systls_c.h"
#endif

/* this checks for HAVE_DBUS_DBUS_H internally. */
#include "core/linux/SDL_dbus.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

/* Initialization code for SDL */

#include "SDL.h"
#include "SDL_bits.h"
#include "SDL_revision.h"
#include "SDL_assert_c.h"
#include "events/SDL_events_c.h"
#include "haptic/SDL_haptic_c.h"
#include "joystick/SDL_joystick_c.h"
#include "sensor/SDL_sensor_c.h"

/* Initialization/Cleanup routines */
#if !SDL_TIMERS_DISABLED
# include "timer/SDL_timer_c.h"
#endif
#if SDL_VIDEO_DRIVER_WINDOWS
extern int SDL_HelperWindowCreate(void);
extern int SDL_HelperWindowDestroy(void);
#endif


/* This is not declared in any header, although it is shared between some
    parts of SDL, because we don't want anything calling it without an
    extremely good reason. */
extern SDL_NORETURN void SDL_ExitProcess(int exitcode);
SDL_NORETURN void SDL_ExitProcess(int exitcode)
{
#ifdef __WIN32__
    /* "if you do not know the state of all threads in your process, it is
       better to call TerminateProcess than ExitProcess"
       https://msdn.microsoft.com/en-us/library/windows/desktop/ms682658(v=vs.85).aspx */
    TerminateProcess(GetCurrentProcess(), exitcode);
    /* MingW doesn't have TerminateProcess marked as noreturn, so add an
       ExitProcess here that will never be reached but make MingW happy. */
    ExitProcess(exitcode);
#elif defined(__EMSCRIPTEN__)
    emscripten_cancel_main_loop();  /* this should "kill" the app. */
    emscripten_force_exit(exitcode);  /* this should "kill" the app. */
    exit(exitcode);
#elif defined(__HAIKU__)  /* Haiku has _Exit, but it's not marked noreturn. */
    _exit(exitcode);
#elif defined(HAVE__EXIT) /* Upper case _Exit() */
    _Exit(exitcode);
#else
    _exit(exitcode);
#endif
}


/* The initialized subsystems */
#ifdef SDL_MAIN_NEEDED
static SDL_bool SDL_MainIsReady = SDL_FALSE;
#else
static SDL_bool SDL_MainIsReady = SDL_TRUE;
#endif
static SDL_bool SDL_bInMainQuit = SDL_FALSE;
static Uint8 SDL_SubsystemRefCount[ 32 ];

/* Private helper to increment a subsystem's ref counter. */
static void
SDL_PrivateSubsystemRefCountIncr(Uint32 subsystem)
{
    int subsystem_index = SDL_MostSignificantBitIndex32(subsystem);
    SDL_assert(SDL_SubsystemRefCount[subsystem_index] < 255);
    ++SDL_SubsystemRefCount[subsystem_index];
}

/* Private helper to decrement a subsystem's ref counter. */
static void
SDL_PrivateSubsystemRefCountDecr(Uint32 subsystem)
{
    int subsystem_index = SDL_MostSignificantBitIndex32(subsystem);
    if (SDL_SubsystemRefCount[subsystem_index] > 0) {
        --SDL_SubsystemRefCount[subsystem_index];
    }
}

/* Private helper to check if a system needs init. */
static SDL_bool
SDL_PrivateShouldInitSubsystem(Uint32 subsystem)
{
    int subsystem_index = SDL_MostSignificantBitIndex32(subsystem);
    SDL_assert(SDL_SubsystemRefCount[subsystem_index] < 255);
    return (SDL_SubsystemRefCount[subsystem_index] == 0) ? SDL_TRUE : SDL_FALSE;
}

/* Private helper to check if a system needs to be quit. */
static SDL_bool
SDL_PrivateShouldQuitSubsystem(Uint32 subsystem) {
    int subsystem_index = SDL_MostSignificantBitIndex32(subsystem);
    if (SDL_SubsystemRefCount[subsystem_index] == 0) {
      return SDL_FALSE;
    }

    /* If we're in SDL_Quit, we shut down every subsystem, even if refcount
     * isn't zero.
     */
    return (SDL_SubsystemRefCount[subsystem_index] == 1 || SDL_bInMainQuit) ? SDL_TRUE : SDL_FALSE;
}

void
SDL_SetMainReady(void)
{
    SDL_MainIsReady = SDL_TRUE;
}

int
SDL_InitSubSystem(Uint32 flags)
{
    if (!SDL_MainIsReady) {
        SDL_SetError("Application didn't initialize properly, did you include SDL_main.h in the file containing your main() function?");
        return -1;
    }

    /* Clear the error message */
    SDL_ClearError();

#if SDL_USE_LIBDBUS
    SDL_DBus_Init();
#endif

    if ((flags & SDL_INIT_GAMECONTROLLER)) {
        /* game controller implies joystick */
        flags |= SDL_INIT_JOYSTICK;
    }

    if ((flags & (SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))) {
        /* video or joystick implies events */
        flags |= SDL_INIT_EVENTS;
    }

#if SDL_THREAD_OS2
    SDL_OS2TLSAlloc(); /* thread/os2/SDL_systls.c */
#endif

#if SDL_VIDEO_DRIVER_WINDOWS
    if ((flags & (SDL_INIT_HAPTIC|SDL_INIT_JOYSTICK))) {
        if (SDL_HelperWindowCreate() < 0) {
            return -1;
        }
    }
#endif

#if !SDL_TIMERS_DISABLED
    SDL_TicksInit();
#endif

    /* Initialize the event subsystem */
    if ((flags & SDL_INIT_EVENTS)) {
#if !SDL_EVENTS_DISABLED
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_EVENTS)) {
            if (SDL_EventsInit() < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_EVENTS);
#else
        return SDL_SetError("SDL not built with events support");
#endif
    }

    /* Initialize the timer subsystem */
    if ((flags & SDL_INIT_TIMER)){
#if !SDL_TIMERS_DISABLED
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_TIMER)) {
            if (SDL_TimerInit() < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_TIMER);
#else
        return SDL_SetError("SDL not built with timer support");
#endif
    }

    /* Initialize the video subsystem */
    if ((flags & SDL_INIT_VIDEO)){
#if !SDL_VIDEO_DISABLED
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_VIDEO)) {
            if (SDL_VideoInit(NULL) < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_VIDEO);
#else
        return SDL_SetError("SDL not built with video support");
#endif
    }

    /* Initialize the audio subsystem */
    if ((flags & SDL_INIT_AUDIO)){
#if !SDL_AUDIO_DISABLED
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_AUDIO)) {
            if (SDL_AudioInit(NULL) < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_AUDIO);
#else
        return SDL_SetError("SDL not built with audio support");
#endif
    }

    /* Initialize the joystick subsystem */
    if ((flags & SDL_INIT_JOYSTICK)){
#if !SDL_JOYSTICK_DISABLED
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_JOYSTICK)) {
           if (SDL_JoystickInit() < 0) {
               return (-1);
           }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_JOYSTICK);
#else
        return SDL_SetError("SDL not built with joystick support");
#endif
    }

    if ((flags & SDL_INIT_GAMECONTROLLER)){
#if !SDL_JOYSTICK_DISABLED
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_GAMECONTROLLER)) {
            if (SDL_GameControllerInit() < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_GAMECONTROLLER);
#else
        return SDL_SetError("SDL not built with joystick support");
#endif
    }

    /* Initialize the haptic subsystem */
    if ((flags & SDL_INIT_HAPTIC)){
#if !SDL_HAPTIC_DISABLED
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_HAPTIC)) {
            if (SDL_HapticInit() < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_HAPTIC);
#else
        return SDL_SetError("SDL not built with haptic (force feedback) support");
#endif
    }

    /* Initialize the sensor subsystem */
    if ((flags & SDL_INIT_SENSOR)){
#if !SDL_SENSOR_DISABLED
        if (SDL_PrivateShouldInitSubsystem(SDL_INIT_SENSOR)) {
            if (SDL_SensorInit() < 0) {
                return (-1);
            }
        }
        SDL_PrivateSubsystemRefCountIncr(SDL_INIT_SENSOR);
#else
        return SDL_SetError("SDL not built with sensor support");
#endif
    }

    return (0);
}

int
SDL_Init(Uint32 flags)
{
    return SDL_InitSubSystem(flags);
}

void
SDL_QuitSubSystem(Uint32 flags)
{
#if SDL_THREAD_OS2
    SDL_OS2TLSFree(); /* thread/os2/SDL_systls.c */
#endif
#if defined(__OS2__)
    SDL_OS2Quit();
#endif

    /* Shut down requested initialized subsystems */
#if !SDL_SENSOR_DISABLED
    if ((flags & SDL_INIT_SENSOR)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_SENSOR)) {
            SDL_SensorQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_SENSOR);
    }
#endif

#if !SDL_JOYSTICK_DISABLED
    if ((flags & SDL_INIT_GAMECONTROLLER)) {
        /* game controller implies joystick */
        flags |= SDL_INIT_JOYSTICK;

        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_GAMECONTROLLER)) {
            SDL_GameControllerQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_GAMECONTROLLER);
    }

    if ((flags & SDL_INIT_JOYSTICK)) {
        /* joystick implies events */
        flags |= SDL_INIT_EVENTS;

        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_JOYSTICK)) {
            SDL_JoystickQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_JOYSTICK);
    }
#endif

#if !SDL_HAPTIC_DISABLED
    if ((flags & SDL_INIT_HAPTIC)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_HAPTIC)) {
            SDL_HapticQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_HAPTIC);
    }
#endif

#if !SDL_AUDIO_DISABLED
    if ((flags & SDL_INIT_AUDIO)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_AUDIO)) {
            SDL_AudioQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_AUDIO);
    }
#endif

#if !SDL_VIDEO_DISABLED
    if ((flags & SDL_INIT_VIDEO)) {
        /* video implies events */
        flags |= SDL_INIT_EVENTS;

        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_VIDEO)) {
            SDL_VideoQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_VIDEO);
    }
#endif

#if !SDL_TIMERS_DISABLED
    if ((flags & SDL_INIT_TIMER)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_TIMER)) {
            SDL_TimerQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_TIMER);
    }
#endif

#if !SDL_EVENTS_DISABLED
    if ((flags & SDL_INIT_EVENTS)) {
        if (SDL_PrivateShouldQuitSubsystem(SDL_INIT_EVENTS)) {
            SDL_EventsQuit();
        }
        SDL_PrivateSubsystemRefCountDecr(SDL_INIT_EVENTS);
    }
#endif
}

Uint32
SDL_WasInit(Uint32 flags)
{
    int i;
    int num_subsystems = SDL_arraysize(SDL_SubsystemRefCount);
    Uint32 initialized = 0;

    /* Fast path for checking one flag */
    if (SDL_HasExactlyOneBitSet32(flags)) {
        int subsystem_index = SDL_MostSignificantBitIndex32(flags);
        return SDL_SubsystemRefCount[subsystem_index] ? flags : 0;
    }

    if (!flags) {
        flags = SDL_INIT_EVERYTHING;
    }

    num_subsystems = SDL_min(num_subsystems, SDL_MostSignificantBitIndex32(flags) + 1);

    /* Iterate over each bit in flags, and check the matching subsystem. */
    for (i = 0; i < num_subsystems; ++i) {
        if ((flags & 1) && SDL_SubsystemRefCount[i] > 0) {
            initialized |= (1 << i);
        }

        flags >>= 1;
    }

    return initialized;
}

void
SDL_Quit(void)
{
    SDL_bInMainQuit = SDL_TRUE;

    /* Quit all subsystems */
#if SDL_VIDEO_DRIVER_WINDOWS
    SDL_HelperWindowDestroy();
#endif
    SDL_QuitSubSystem(SDL_INIT_EVERYTHING);

#if !SDL_TIMERS_DISABLED
    SDL_TicksQuit();
#endif

    SDL_ClearHints();
    SDL_AssertionsQuit();
    SDL_LogResetPriorities();

#if SDL_USE_LIBDBUS
    SDL_DBus_Quit();
#endif

    /* Now that every subsystem has been quit, we reset the subsystem refcount
     * and the list of initialized subsystems.
     */
    SDL_memset( SDL_SubsystemRefCount, 0x0, sizeof(SDL_SubsystemRefCount) );

    SDL_bInMainQuit = SDL_FALSE;
}

/* Get the library version number */
void
SDL_GetVersion(SDL_version * ver)
{
    SDL_VERSION(ver);
}

/* Get the library source revision */
const char *
SDL_GetRevision(void)
{
    return SDL_REVISION;
}

/* Get the library source revision number */
int
SDL_GetRevisionNumber(void)
{
    return 0;  /* doesn't make sense without Mercurial. */
}

/* Get the name of the platform */
const char *
SDL_GetPlatform()
{
#if __AIX__
    return "AIX";
#elif __ANDROID__
    return "Android";
#elif __BSDI__
    return "BSDI";
#elif __DREAMCAST__
    return "Dreamcast";
#elif __EMSCRIPTEN__
    return "Emscripten";
#elif __FREEBSD__
    return "FreeBSD";
#elif __HAIKU__
    return "Haiku";
#elif __HPUX__
    return "HP-UX";
#elif __IRIX__
    return "Irix";
#elif __LINUX__
    return "Linux";
#elif __MINT__
    return "Atari MiNT";
#elif __MACOS__
    return "MacOS Classic";
#elif __MACOSX__
    return "Mac OS X";
#elif __NACL__
    return "NaCl";
#elif __NETBSD__
    return "NetBSD";
#elif __OPENBSD__
    return "OpenBSD";
#elif __OS2__
    return "OS/2";
#elif __OSF__
    return "OSF/1";
#elif __QNXNTO__
    return "QNX Neutrino";
#elif __RISCOS__
    return "RISC OS";
#elif __SOLARIS__
    return "Solaris";
#elif __WIN32__
    return "Windows";
#elif __WINRT__
    return "WinRT";
#elif __TVOS__
    return "tvOS";
#elif __IPHONEOS__
    return "iOS";
#elif __PSP__
    return "PlayStation Portable";
#elif __VITA__
    return "PlayStation Vita";
#else
    return "Unknown (see SDL_platform.h)";
#endif
}

SDL_bool
SDL_IsTablet()
{
#if __ANDROID__
    extern SDL_bool SDL_IsAndroidTablet(void);
    return SDL_IsAndroidTablet();
#elif __IPHONEOS__
    extern SDL_bool SDL_IsIPad(void);
    return SDL_IsIPad();
#else
    return SDL_FALSE;
#endif
}

#if defined(__WIN32__)

#if (!defined(HAVE_LIBC) || defined(__WATCOMC__)) && !defined(SDL_STATIC_LIB)
/* Need to include DllMain() on Watcom C for some reason.. */

BOOL APIENTRY
_DllMainCRTStartup(HANDLE hModule,
                   DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif /* Building DLL */

#endif /* __WIN32__ */

/* vi: set sts=4 ts=4 sw=4 expandtab: */
