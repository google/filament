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

#include "SDL_video.h"

/* This is the software implementation of the YUV texture support */

struct SDL_SW_YUVTexture
{
    Uint32 format;
    Uint32 target_format;
    int w, h;
    Uint8 *pixels;

    /* These are just so we don't have to allocate them separately */
    Uint16 pitches[3];
    Uint8 *planes[3];

    /* This is a temporary surface in case we have to stretch copy */
    SDL_Surface *stretch;
    SDL_Surface *display;
};

typedef struct SDL_SW_YUVTexture SDL_SW_YUVTexture;

SDL_SW_YUVTexture *SDL_SW_CreateYUVTexture(Uint32 format, int w, int h);
int SDL_SW_QueryYUVTexturePixels(SDL_SW_YUVTexture * swdata, void **pixels,
                                 int *pitch);
int SDL_SW_UpdateYUVTexture(SDL_SW_YUVTexture * swdata, const SDL_Rect * rect,
                            const void *pixels, int pitch);
int SDL_SW_UpdateYUVTexturePlanar(SDL_SW_YUVTexture * swdata, const SDL_Rect * rect,
                                  const Uint8 *Yplane, int Ypitch,
                                  const Uint8 *Uplane, int Upitch,
                                  const Uint8 *Vplane, int Vpitch);
int SDL_SW_LockYUVTexture(SDL_SW_YUVTexture * swdata, const SDL_Rect * rect,
                          void **pixels, int *pitch);
void SDL_SW_UnlockYUVTexture(SDL_SW_YUVTexture * swdata);
int SDL_SW_CopyYUVToRGB(SDL_SW_YUVTexture * swdata, const SDL_Rect * srcrect,
                        Uint32 target_format, int w, int h, void *pixels,
                        int pitch);
void SDL_SW_DestroyYUVTexture(SDL_SW_YUVTexture * swdata);

/* FIXME: This breaks on various versions of GCC and should be rewritten using intrinsics */
#if 0 /* (__GNUC__ > 2) && defined(__i386__) && __OPTIMIZE__ && SDL_ASSEMBLY_ROUTINES && !defined(__clang__) */
#define USE_MMX_ASSEMBLY 1
#endif

/* vi: set ts=4 sw=4 expandtab: */
