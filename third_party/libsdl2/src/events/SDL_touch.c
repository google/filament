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

/* General touch handling code for SDL */

#include "SDL_assert.h"
#include "SDL_events.h"
#include "SDL_events_c.h"
#include "../video/SDL_sysvideo.h"


static int SDL_num_touch = 0;
static SDL_Touch **SDL_touchDevices = NULL;


/* Public functions */
int
SDL_TouchInit(void)
{
    return (0);
}

int
SDL_GetNumTouchDevices(void)
{
    return SDL_num_touch;
}

SDL_TouchID
SDL_GetTouchDevice(int index)
{
    if (index < 0 || index >= SDL_num_touch) {
        SDL_SetError("Unknown touch device index %d", index);
        return 0;
    }
    return SDL_touchDevices[index]->id;
}

static int
SDL_GetTouchIndex(SDL_TouchID id)
{
    int index;
    SDL_Touch *touch;

    for (index = 0; index < SDL_num_touch; ++index) {
        touch = SDL_touchDevices[index];
        if (touch->id == id) {
            return index;
        }
    }
    return -1;
}

SDL_Touch *
SDL_GetTouch(SDL_TouchID id)
{
    int index = SDL_GetTouchIndex(id);
    if (index < 0 || index >= SDL_num_touch) {
        if (SDL_GetVideoDevice()->ResetTouch != NULL) {
            SDL_SetError("Unknown touch id %d, resetting", (int) id);
            (SDL_GetVideoDevice()->ResetTouch)(SDL_GetVideoDevice());
        } else {
            SDL_SetError("Unknown touch device id %d, cannot reset", (int) id);
        }
        return NULL;
    }
    return SDL_touchDevices[index];
}

static int
SDL_GetFingerIndex(const SDL_Touch * touch, SDL_FingerID fingerid)
{
    int index;
    for (index = 0; index < touch->num_fingers; ++index) {
        if (touch->fingers[index]->id == fingerid) {
            return index;
        }
    }
    return -1;
}

static SDL_Finger *
SDL_GetFinger(const SDL_Touch * touch, SDL_FingerID id)
{
    int index = SDL_GetFingerIndex(touch, id);
    if (index < 0 || index >= touch->num_fingers) {
        return NULL;
    }
    return touch->fingers[index];
}

int
SDL_GetNumTouchFingers(SDL_TouchID touchID)
{
    SDL_Touch *touch = SDL_GetTouch(touchID);
    if (touch) {
        return touch->num_fingers;
    }
    return 0;
}

SDL_Finger *
SDL_GetTouchFinger(SDL_TouchID touchID, int index)
{
    SDL_Touch *touch = SDL_GetTouch(touchID);
    if (!touch) {
        return NULL;
    }
    if (index < 0 || index >= touch->num_fingers) {
        SDL_SetError("Unknown touch finger");
        return NULL;
    }
    return touch->fingers[index];
}

int
SDL_AddTouch(SDL_TouchID touchID, const char *name)
{
    SDL_Touch **touchDevices;
    int index;

    index = SDL_GetTouchIndex(touchID);
    if (index >= 0) {
        return index;
    }

    /* Add the touch to the list of touch */
    touchDevices = (SDL_Touch **) SDL_realloc(SDL_touchDevices,
                                      (SDL_num_touch + 1) * sizeof(*touchDevices));
    if (!touchDevices) {
        return SDL_OutOfMemory();
    }

    SDL_touchDevices = touchDevices;
    index = SDL_num_touch;

    SDL_touchDevices[index] = (SDL_Touch *) SDL_malloc(sizeof(*SDL_touchDevices[index]));
    if (!SDL_touchDevices[index]) {
        return SDL_OutOfMemory();
    }

    /* Added touch to list */
    ++SDL_num_touch;

    /* we're setting the touch properties */
    SDL_touchDevices[index]->id = touchID;
    SDL_touchDevices[index]->num_fingers = 0;
    SDL_touchDevices[index]->max_fingers = 0;
    SDL_touchDevices[index]->fingers = NULL;

    /* Record this touch device for gestures */
    /* We could do this on the fly in the gesture code if we wanted */
    SDL_GestureAddTouch(touchID);

    return index;
}

static int
SDL_AddFinger(SDL_Touch *touch, SDL_FingerID fingerid, float x, float y, float pressure)
{
    SDL_Finger *finger;

    if (touch->num_fingers == touch->max_fingers) {
        SDL_Finger **new_fingers;
        new_fingers = (SDL_Finger **)SDL_realloc(touch->fingers, (touch->max_fingers+1)*sizeof(*touch->fingers));
        if (!new_fingers) {
            return SDL_OutOfMemory();
        }
        touch->fingers = new_fingers;
        touch->fingers[touch->max_fingers] = (SDL_Finger *)SDL_malloc(sizeof(*finger));
        if (!touch->fingers[touch->max_fingers]) {
            return SDL_OutOfMemory();
        }
        touch->max_fingers++;
    }

    finger = touch->fingers[touch->num_fingers++];
    finger->id = fingerid;
    finger->x = x;
    finger->y = y;
    finger->pressure = pressure;
    return 0;
}

static int
SDL_DelFinger(SDL_Touch* touch, SDL_FingerID fingerid)
{
    SDL_Finger *temp;

    int index = SDL_GetFingerIndex(touch, fingerid);
    if (index < 0) {
        return -1;
    }

    touch->num_fingers--;
    temp = touch->fingers[index];
    touch->fingers[index] = touch->fingers[touch->num_fingers];
    touch->fingers[touch->num_fingers] = temp;
    return 0;
}

int
SDL_SendTouch(SDL_TouchID id, SDL_FingerID fingerid,
              SDL_bool down, float x, float y, float pressure)
{
    int posted;
    SDL_Finger *finger;

    SDL_Touch* touch = SDL_GetTouch(id);
    if (!touch) {
        return -1;
    }

    finger = SDL_GetFinger(touch, fingerid);
    if (down) {
        if (finger) {
            /* This finger is already down */
            return 0;
        }

        if (SDL_AddFinger(touch, fingerid, x, y, pressure) < 0) {
            return 0;
        }

        posted = 0;
        if (SDL_GetEventState(SDL_FINGERDOWN) == SDL_ENABLE) {
            SDL_Event event;
            event.tfinger.type = SDL_FINGERDOWN;
            event.tfinger.touchId = id;
            event.tfinger.fingerId = fingerid;
            event.tfinger.x = x;
            event.tfinger.y = y;
            event.tfinger.dx = 0;
            event.tfinger.dy = 0;
            event.tfinger.pressure = pressure;
            posted = (SDL_PushEvent(&event) > 0);
        }
    } else {
        if (!finger) {
            /* This finger is already up */
            return 0;
        }

        posted = 0;
        if (SDL_GetEventState(SDL_FINGERUP) == SDL_ENABLE) {
            SDL_Event event;
            event.tfinger.type = SDL_FINGERUP;
            event.tfinger.touchId =  id;
            event.tfinger.fingerId = fingerid;
            /* I don't trust the coordinates passed on fingerUp */
            event.tfinger.x = finger->x;
            event.tfinger.y = finger->y;
            event.tfinger.dx = 0;
            event.tfinger.dy = 0;
            event.tfinger.pressure = pressure;
            posted = (SDL_PushEvent(&event) > 0);
        }

        SDL_DelFinger(touch, fingerid);
    }
    return posted;
}

int
SDL_SendTouchMotion(SDL_TouchID id, SDL_FingerID fingerid,
                    float x, float y, float pressure)
{
    SDL_Touch *touch;
    SDL_Finger *finger;
    int posted;
    float xrel, yrel, prel;

    touch = SDL_GetTouch(id);
    if (!touch) {
        return -1;
    }

    finger = SDL_GetFinger(touch,fingerid);
    if (!finger) {
        return SDL_SendTouch(id, fingerid, SDL_TRUE, x, y, pressure);
    }

    xrel = x - finger->x;
    yrel = y - finger->y;
    prel = pressure - finger->pressure;

    /* Drop events that don't change state */
    if (!xrel && !yrel && !prel) {
#if 0
        printf("Touch event didn't change state - dropped!\n");
#endif
        return 0;
    }

    /* Update internal touch coordinates */
    finger->x = x;
    finger->y = y;
    finger->pressure = pressure;

    /* Post the event, if desired */
    posted = 0;
    if (SDL_GetEventState(SDL_FINGERMOTION) == SDL_ENABLE) {
        SDL_Event event;
        event.tfinger.type = SDL_FINGERMOTION;
        event.tfinger.touchId = id;
        event.tfinger.fingerId = fingerid;
        event.tfinger.x = x;
        event.tfinger.y = y;
        event.tfinger.dx = xrel;
        event.tfinger.dy = yrel;
        event.tfinger.pressure = pressure;
        posted = (SDL_PushEvent(&event) > 0);
    }
    return posted;
}

void
SDL_DelTouch(SDL_TouchID id)
{
    int i;
    int index = SDL_GetTouchIndex(id);
    SDL_Touch *touch = SDL_GetTouch(id);

    if (!touch) {
        return;
    }

    for (i = 0; i < touch->max_fingers; ++i) {
        SDL_free(touch->fingers[i]);
    }
    SDL_free(touch->fingers);
    SDL_free(touch);

    SDL_num_touch--;
    SDL_touchDevices[index] = SDL_touchDevices[SDL_num_touch];

    /* Delete this touch device for gestures */
    SDL_GestureDelTouch(id);
}

void
SDL_TouchQuit(void)
{
    int i;

    for (i = SDL_num_touch; i--; ) {
        SDL_DelTouch(SDL_touchDevices[i]->id);
    }
    SDL_assert(SDL_num_touch == 0);

    SDL_free(SDL_touchDevices);
    SDL_touchDevices = NULL;
    SDL_GestureQuit();
}

/* vi: set ts=4 sw=4 expandtab: */
