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

#include <linux/input.h>

struct SDL_joylist_item;

/* The private structure used to keep track of a joystick */
struct joystick_hwdata
{
    int fd;
    struct SDL_joylist_item *item;
    SDL_JoystickGUID guid;
    char *fname;                /* Used in haptic subsystem */

    /* The current Linux joystick driver maps hats to two axes */
    struct hwdata_hat
    {
        int axis[2];
    } *hats;
    /* The current Linux joystick driver maps balls to two axes */
    struct hwdata_ball
    {
        int axis[2];
    } *balls;

    /* Support for the Linux 2.4 unified input interface */
    Uint8 key_map[KEY_MAX];
    Uint8 abs_map[ABS_MAX];
    struct axis_correct
    {
        int used;
        int coef[3];
    } abs_correct[ABS_MAX];

    int fresh;

    /* Steam Controller support */
    SDL_bool m_bSteamController;
};

/* vi: set ts=4 sw=4 expandtab: */
