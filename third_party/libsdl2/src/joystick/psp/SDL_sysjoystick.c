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

#if SDL_JOYSTICK_PSP

/* This is the PSP implementation of the SDL joystick API */
#include <pspctrl.h>
#include <pspkernel.h>

#include <stdio.h>      /* For the definition of NULL */
#include <stdlib.h>

#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

#include "SDL_events.h"
#include "SDL_error.h"
#include "SDL_mutex.h"
#include "SDL_timer.h"
#include "../../thread/SDL_systhread.h"

/* Current pad state */
static SceCtrlData pad = { .Lx = 0, .Ly = 0, .Buttons = 0 };
static SDL_sem *pad_sem = NULL;
static SDL_Thread *thread = NULL;
static int running = 0;
static const enum PspCtrlButtons button_map[] = {
    PSP_CTRL_TRIANGLE, PSP_CTRL_CIRCLE, PSP_CTRL_CROSS, PSP_CTRL_SQUARE,
    PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER,
    PSP_CTRL_DOWN, PSP_CTRL_LEFT, PSP_CTRL_UP, PSP_CTRL_RIGHT,
    PSP_CTRL_SELECT, PSP_CTRL_START, PSP_CTRL_HOME, PSP_CTRL_HOLD };
static int analog_map[256];  /* Map analog inputs to -32768 -> 32767 */

typedef struct
{
  int x;
  int y;
} point;

/* 4 points define the bezier-curve. */
static point a = { 0, 0 };
static point b = { 50, 0  };
static point c = { 78, 32767 };
static point d = { 128, 32767 };

/* simple linear interpolation between two points */
static SDL_INLINE void lerp (point *dest, point *a, point *b, float t)
{
    dest->x = a->x + (b->x - a->x)*t;
    dest->y = a->y + (b->y - a->y)*t;
}

/* evaluate a point on a bezier-curve. t goes from 0 to 1.0 */
static int calc_bezier_y(float t)
{
    point ab, bc, cd, abbc, bccd, dest;
    lerp (&ab, &a, &b, t);           /* point between a and b */
    lerp (&bc, &b, &c, t);           /* point between b and c */
    lerp (&cd, &c, &d, t);           /* point between c and d */
    lerp (&abbc, &ab, &bc, t);       /* point between ab and bc */
    lerp (&bccd, &bc, &cd, t);       /* point between bc and cd */
    lerp (&dest, &abbc, &bccd, t);   /* point on the bezier-curve */
    return dest.y;
}

/*
 * Collect pad data about once per frame
 */
int JoystickUpdate(void *data)
{
    while (running) {
        SDL_SemWait(pad_sem);
        sceCtrlPeekBufferPositive(&pad, 1);
        SDL_SemPost(pad_sem);
        /* Delay 1/60th of a second */
        sceKernelDelayThread(1000000 / 60);
    }
    return 0;
}



/* Function to scan the system for joysticks.
 * Joystick 0 should be the system default joystick.
 * It should return number of joysticks, or -1 on an unrecoverable fatal error.
 */
int SDL_SYS_JoystickInit(void)
{
    int i;

    /* Setup input */
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    /* Start thread to read data */
    if((pad_sem =  SDL_CreateSemaphore(1)) == NULL) {
        return SDL_SetError("Can't create input semaphore");
    }
    running = 1;
    if((thread = SDL_CreateThreadInternal(JoystickUpdate, "JoystickThread", 4096, NULL)) == NULL) {
        return SDL_SetError("Can't create input thread");
    }

    /* Create an accurate map from analog inputs (0 to 255)
       to SDL joystick positions (-32768 to 32767) */
    for (i = 0; i < 128; i++)
    {
        float t = (float)i/127.0f;
        analog_map[i+128] = calc_bezier_y(t);
        analog_map[127-i] = -1 * analog_map[i+128];
    }

    return 1;
}

int SDL_SYS_NumJoysticks(void)
{
    return 1;
}

void SDL_SYS_JoystickDetect(void)
{
}

/* Function to get the device-dependent name of a joystick */
const char * SDL_SYS_JoystickNameForDeviceIndex(int device_index)
{
    return "PSP builtin joypad";
}

/* Function to perform the mapping from device index to the instance id for this index */
SDL_JoystickID SDL_SYS_GetInstanceIdOfDeviceIndex(int device_index)
{
    return device_index;
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
    if (index == 0)
        return "PSP controller";

    SDL_SetError("No joystick available with that index");
    return(NULL);
}

/* Function to open a joystick for use.
   The joystick to open is specified by the device index.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int SDL_SYS_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    joystick->nbuttons = 14;
    joystick->naxes = 2;
    joystick->nhats = 0;

    return 0;
}

/* Function to determine if this joystick is attached to the system right now */
SDL_bool SDL_SYS_JoystickAttached(SDL_Joystick *joystick)
{
    return SDL_TRUE;
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
    int i;
    enum PspCtrlButtons buttons;
    enum PspCtrlButtons changed;
    unsigned char x, y;
    static enum PspCtrlButtons old_buttons = 0;
    static unsigned char old_x = 0, old_y = 0;

    SDL_SemWait(pad_sem);
    buttons = pad.Buttons;
    x = pad.Lx;
    y = pad.Ly;
    SDL_SemPost(pad_sem);

    /* Axes */
    if(old_x != x) {
        SDL_PrivateJoystickAxis(joystick, 0, analog_map[x]);
        old_x = x;
    }
    if(old_y != y) {
        SDL_PrivateJoystickAxis(joystick, 1, analog_map[y]);
        old_y = y;
    }

    /* Buttons */
    changed = old_buttons ^ buttons;
    old_buttons = buttons;
    if(changed) {
        for(i=0; i<sizeof(button_map)/sizeof(button_map[0]); i++) {
            if(changed & button_map[i]) {
                SDL_PrivateJoystickButton(
                    joystick, i,
                    (buttons & button_map[i]) ?
                    SDL_PRESSED : SDL_RELEASED);
            }
        }
    }

    sceKernelDelayThread(0);
}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
    /* Cleanup Threads and Semaphore. */
    running = 0;
    SDL_WaitThread(thread, NULL);
    SDL_DestroySemaphore(pad_sem);
}

SDL_JoystickGUID SDL_SYS_JoystickGetDeviceGUID( int device_index )
{
    SDL_JoystickGUID guid;
    /* the GUID is just the first 16 chars of the name for now */
    const char *name = SDL_SYS_JoystickNameForDeviceIndex( device_index );
    SDL_zero( guid );
    SDL_memcpy( &guid, name, SDL_min( sizeof(guid), SDL_strlen( name ) ) );
    return guid;
}

SDL_JoystickGUID SDL_SYS_JoystickGetGUID(SDL_Joystick * joystick)
{
    SDL_JoystickGUID guid;
    /* the GUID is just the first 16 chars of the name for now */
    const char *name = joystick->name;
    SDL_zero( guid );
    SDL_memcpy( &guid, name, SDL_min( sizeof(guid), SDL_strlen( name ) ) );
    return guid;
}

#endif /* SDL_JOYSTICK_PSP */

/* vim: ts=4 sw=4
 */
