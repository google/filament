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

#include <android/log.h>

#include "SDL_hints.h"
#include "SDL_events.h"
#include "SDL_log.h"
#include "SDL_androidtouch.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_touch_c.h"
#include "../../core/android/SDL_android.h"

#define ACTION_DOWN 0
#define ACTION_UP 1
#define ACTION_MOVE 2
#define ACTION_CANCEL 3
#define ACTION_OUTSIDE 4
#define ACTION_POINTER_DOWN 5
#define ACTION_POINTER_UP 6

static void Android_GetWindowCoordinates(float x, float y,
                                         int *window_x, int *window_y)
{
    int window_w, window_h;

    SDL_GetWindowSize(Android_Window, &window_w, &window_h);
    *window_x = (int)(x * window_w);
    *window_y = (int)(y * window_h);
}

static SDL_bool separate_mouse_and_touch = SDL_FALSE;

static void SDLCALL
SeparateEventsHintWatcher(void *userdata, const char *name,
                          const char *oldValue, const char *newValue)
{
    separate_mouse_and_touch = (newValue && (SDL_strcmp(newValue, "1") == 0));

    Android_JNI_SetSeparateMouseAndTouch(separate_mouse_and_touch);
}

void Android_InitTouch(void)
{
    int i;
    int* ids;
    const int number = Android_JNI_GetTouchDeviceIds(&ids);

    SDL_AddHintCallback(SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH,
                        SeparateEventsHintWatcher, NULL);

    if (0 < number) {
        for (i = 0; i < number; ++i) {
            SDL_AddTouch((SDL_TouchID) ids[i], ""); /* no error handling */
        }
        SDL_free(ids);
    }
}

void Android_QuitTouch(void)
{
    SDL_DelHintCallback(SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH,
                        SeparateEventsHintWatcher, NULL);
    separate_mouse_and_touch = SDL_FALSE;
}

void Android_OnTouch(int touch_device_id_in, int pointer_finger_id_in, int action, float x, float y, float p)
{
    SDL_TouchID touchDeviceId = 0;
    SDL_FingerID fingerId = 0;
    int window_x, window_y;
    static SDL_FingerID pointerFingerID = 0;

    if (!Android_Window) {
        return;
    }

    touchDeviceId = (SDL_TouchID)touch_device_id_in;
    if (SDL_AddTouch(touchDeviceId, "") < 0) {
        SDL_Log("error: can't add touch %s, %d", __FILE__, __LINE__);
    }

    fingerId = (SDL_FingerID)pointer_finger_id_in;
    switch (action) {
        case ACTION_DOWN:
            /* Primary pointer down */
            if (!separate_mouse_and_touch) {
                Android_GetWindowCoordinates(x, y, &window_x, &window_y);
                /* send moved event */
                SDL_SendMouseMotion(Android_Window, SDL_TOUCH_MOUSEID, 0, window_x, window_y);
                /* send mouse down event */
                SDL_SendMouseButton(Android_Window, SDL_TOUCH_MOUSEID, SDL_PRESSED, SDL_BUTTON_LEFT);
            }
            pointerFingerID = fingerId;
        case ACTION_POINTER_DOWN:
            /* Non primary pointer down */
            SDL_SendTouch(touchDeviceId, fingerId, SDL_TRUE, x, y, p);
            break;

        case ACTION_MOVE:
            if (!pointerFingerID) {
                if (!separate_mouse_and_touch) {
                    Android_GetWindowCoordinates(x, y, &window_x, &window_y);
                    /* send moved event */
                    SDL_SendMouseMotion(Android_Window, SDL_TOUCH_MOUSEID, 0, window_x, window_y);
                }
            }
            SDL_SendTouchMotion(touchDeviceId, fingerId, x, y, p);
            break;

        case ACTION_UP:
            /* Primary pointer up */
            if (!separate_mouse_and_touch) {
                /* send mouse up */
                SDL_SendMouseButton(Android_Window, SDL_TOUCH_MOUSEID, SDL_RELEASED, SDL_BUTTON_LEFT);
            }
            pointerFingerID = (SDL_FingerID) 0;
        case ACTION_POINTER_UP:
            /* Non primary pointer up */
            SDL_SendTouch(touchDeviceId, fingerId, SDL_FALSE, x, y, p);
            break;

        default:
            break;
    }
}

#endif /* SDL_VIDEO_DRIVER_ANDROID */

/* vi: set ts=4 sw=4 expandtab: */
