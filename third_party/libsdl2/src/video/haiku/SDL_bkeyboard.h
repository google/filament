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

#ifndef SDL_BKEYBOARD_H
#define SDL_BKEYBOARD_H

#include <SupportDefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../include/SDL_keyboard.h"

extern void BE_InitOSKeymap(void);
extern SDL_Scancode BE_GetScancodeFromBeKey(int32 bkey);
extern int8 BE_GetKeyState(int32 bkey);
extern void BE_SetKeyState(int32 bkey, int8 state);

#ifdef __cplusplus
}
#endif

#endif

/* vi: set ts=4 sw=4 expandtab: */
