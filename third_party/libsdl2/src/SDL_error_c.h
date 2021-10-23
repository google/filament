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

/* This file defines a structure that carries language-independent
   error messages
*/

#ifndef SDL_error_c_h_
#define SDL_error_c_h_

#define ERR_MAX_STRLEN  128

typedef struct SDL_error
{
    int error; /* This is a numeric value corresponding to the current error */
    char str[ERR_MAX_STRLEN];
} SDL_error;

/* Defined in SDL_thread.c */
extern SDL_error *SDL_GetErrBuf(void);

#endif /* SDL_error_c_h_ */

/* vi: set ts=4 sw=4 expandtab: */
