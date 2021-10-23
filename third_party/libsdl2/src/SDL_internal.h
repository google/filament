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
#ifndef SDL_internal_h_
#define SDL_internal_h_

/* Many of SDL's features require _GNU_SOURCE on various platforms */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* This is for a variable-length array at the end of a struct:
    struct x { int y; char z[SDL_VARIABLE_LENGTH_ARRAY]; };
   Use this because GCC 2 needs different magic than other compilers. */
#if (defined(__GNUC__) && (__GNUC__ <= 2)) || defined(__CC_ARM) || defined(__cplusplus)
#define SDL_VARIABLE_LENGTH_ARRAY 1
#else
#define SDL_VARIABLE_LENGTH_ARRAY
#endif

#define SDL_MAX_SMALL_ALLOC_STACKSIZE 128
#define SDL_small_alloc(type, count, pisstack) ( (*(pisstack) = ((sizeof(type)*(count)) < SDL_MAX_SMALL_ALLOC_STACKSIZE)), (*(pisstack) ? SDL_stack_alloc(type, count) : (type*)SDL_malloc(sizeof(type)*(count))) )
#define SDL_small_free(ptr, isstack) if ((isstack)) { SDL_stack_free(ptr); } else { SDL_free(ptr); }

#include "dynapi/SDL_dynapi.h"

#if SDL_DYNAMIC_API
#include "dynapi/SDL_dynapi_overrides.h"
/* force DECLSPEC and SDLCALL off...it's all internal symbols now.
   These will have actual #defines during SDL_dynapi.c only */
#define DECLSPEC
#define SDLCALL
#endif

#include "SDL_config.h"

/* A few #defines to reduce SDL2 footprint.
   Only effective when library is statically linked.
   You have to manually edit this file. */
#ifndef SDL_LEAN_AND_MEAN
#define SDL_LEAN_AND_MEAN               0
#endif

/* Optimized functions from 'SDL_blit_0.c'
   - blit with source BitsPerPixel < 8, palette */
#ifndef SDL_HAVE_BLIT_0
#define SDL_HAVE_BLIT_0                 !SDL_LEAN_AND_MEAN
#endif

/* Optimized functions from 'SDL_blit_1.c'
   - blit with source BytesPerPixel == 1, palette */
#ifndef SDL_HAVE_BLIT_1
#define SDL_HAVE_BLIT_1                 !SDL_LEAN_AND_MEAN
#endif

/* Optimized functions from 'SDL_blit_A.c'
   - blit with 'SDL_BLENDMODE_BLEND' blending mode */
#ifndef SDL_HAVE_BLIT_A
#define SDL_HAVE_BLIT_A                 !SDL_LEAN_AND_MEAN
#endif

/* Optimized functions from 'SDL_blit_N.c'
   - blit with COLORKEY mode, or nothing */
#ifndef SDL_HAVE_BLIT_N
#define SDL_HAVE_BLIT_N                 !SDL_LEAN_AND_MEAN
#endif

/* Optimized functions from 'SDL_blit_N.c'
   - RGB565 conversion with Lookup tables */
#ifndef SDL_HAVE_BLIT_N_RGB565
#define SDL_HAVE_BLIT_N_RGB565          !SDL_LEAN_AND_MEAN
#endif

/* Optimized functions from 'SDL_blit_AUTO.c'
   - blit with modulate color, modulate alpha, any blending mode
   - scaling or not */
#ifndef SDL_HAVE_BLIT_AUTO
#define SDL_HAVE_BLIT_AUTO              !SDL_LEAN_AND_MEAN
#endif

/* Run-Length-Encoding
   - SDL_SetColorKey() called with SDL_RLEACCEL flag */
#ifndef SDL_HAVE_RLE
#define SDL_HAVE_RLE                    !SDL_LEAN_AND_MEAN
#endif

/* Software SDL_Renderer
   - creation of software renderer
   - *not* general blitting functions
   - {blend,draw}{fillrect,line,point} internal functions */
#ifndef SDL_VIDEO_RENDER_SW
#define SDL_VIDEO_RENDER_SW             !SDL_LEAN_AND_MEAN
#endif

/* YUV formats
   - handling of YUV surfaces
   - blitting and conversion functions */
#ifndef SDL_HAVE_YUV
#define SDL_HAVE_YUV                    !SDL_LEAN_AND_MEAN
#endif

#include "SDL_assert.h"
#include "SDL_log.h"

#endif /* SDL_internal_h_ */

/* vi: set ts=4 sw=4 expandtab: */
