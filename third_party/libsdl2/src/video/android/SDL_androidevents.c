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
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_ANDROID

/* We're going to do this by default */
#define SDL_ANDROID_BLOCK_ON_PAUSE  1

#include "SDL_androidevents.h"
#include "SDL_events.h"
#include "SDL_androidwindow.h"

#if !SDL_AUDIO_DISABLED
/* Can't include sysaudio "../../audio/android/SDL_androidaudio.h"
 * because of THIS redefinition */
extern void ANDROIDAUDIO_ResumeDevices(void);
extern void ANDROIDAUDIO_PauseDevices(void);
#else
static void ANDROIDAUDIO_ResumeDevices(void) {}
static void ANDROIDAUDIO_PauseDevices(void) {}
#endif

static void 
android_egl_context_restore() 
{
    SDL_Event event;
    SDL_WindowData *data = (SDL_WindowData *) Android_Window->driverdata;
    if (SDL_GL_MakeCurrent(Android_Window, (SDL_GLContext) data->egl_context) < 0) {
        /* The context is no longer valid, create a new one */
        data->egl_context = (EGLContext) SDL_GL_CreateContext(Android_Window);
        SDL_GL_MakeCurrent(Android_Window, (SDL_GLContext) data->egl_context);
        event.type = SDL_RENDER_DEVICE_RESET;
        SDL_PushEvent(&event);
    }
}

static void 
android_egl_context_backup() 
{
    /* Keep a copy of the EGL Context so we can try to restore it when we resume */
    SDL_WindowData *data = (SDL_WindowData *) Android_Window->driverdata;
    data->egl_context = SDL_GL_GetCurrentContext();
    /* We need to do this so the EGLSurface can be freed */
    SDL_GL_MakeCurrent(Android_Window, NULL);
}

void
Android_PumpEvents(_THIS)
{
    static int isPaused = 0;
#if SDL_ANDROID_BLOCK_ON_PAUSE
    static int isPausing = 0;
#endif
    /* No polling necessary */

    /*
     * Android_ResumeSem and Android_PauseSem are signaled from Java_org_libsdl_app_SDLActivity_nativePause and Java_org_libsdl_app_SDLActivity_nativeResume
     * When the pause semaphore is signaled, if SDL_ANDROID_BLOCK_ON_PAUSE is defined the event loop will block until the resume signal is emitted.
     */

#if SDL_ANDROID_BLOCK_ON_PAUSE
    if (isPaused && !isPausing) {
        /* Make sure this is the last thing we do before pausing */
        android_egl_context_backup();
        ANDROIDAUDIO_PauseDevices();
        if(SDL_SemWait(Android_ResumeSem) == 0) {
#else
    if (isPaused) {
        if(SDL_SemTryWait(Android_ResumeSem) == 0) {
#endif
            isPaused = 0;
            ANDROIDAUDIO_ResumeDevices();
            /* Restore the GL Context from here, as this operation is thread dependent */
            if (!SDL_HasEvent(SDL_QUIT)) {
                android_egl_context_restore();
            }
        }
    }
    else {
#if SDL_ANDROID_BLOCK_ON_PAUSE
        if( isPausing || SDL_SemTryWait(Android_PauseSem) == 0 ) {
            /* We've been signaled to pause, but before we block ourselves, 
            we need to make sure that certain key events have reached the app */
            if (SDL_HasEvent(SDL_WINDOWEVENT) || SDL_HasEvent(SDL_APP_WILLENTERBACKGROUND) || SDL_HasEvent(SDL_APP_DIDENTERBACKGROUND) ) {
                isPausing = 1;
            }
            else {
                isPausing = 0;
                isPaused = 1;
            }
        }
#else
        if(SDL_SemTryWait(Android_PauseSem) == 0) {
            android_egl_context_backup();
            ANDROIDAUDIO_PauseDevices();
            isPaused = 1;
        }
#endif
    }
}

#endif /* SDL_VIDEO_DRIVER_ANDROID */

/* vi: set ts=4 sw=4 expandtab: */
