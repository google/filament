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
#include "SDL_blit.h"


#ifdef __SSE__
/* *INDENT-OFF* */

#if defined(_MSC_VER) && !defined(__clang__)
#define SSE_BEGIN \
    __m128 c128; \
    c128.m128_u32[0] = color; \
    c128.m128_u32[1] = color; \
    c128.m128_u32[2] = color; \
    c128.m128_u32[3] = color;
#else
#define SSE_BEGIN \
    __m128 c128; \
    DECLARE_ALIGNED(Uint32, cccc[4], 16); \
    cccc[0] = color; \
    cccc[1] = color; \
    cccc[2] = color; \
    cccc[3] = color; \
    c128 = *(__m128 *)cccc;
#endif

#define SSE_WORK \
    for (i = n / 64; i--;) { \
        _mm_stream_ps((float *)(p+0), c128); \
        _mm_stream_ps((float *)(p+16), c128); \
        _mm_stream_ps((float *)(p+32), c128); \
        _mm_stream_ps((float *)(p+48), c128); \
        p += 64; \
    }

#define SSE_END

#define DEFINE_SSE_FILLRECT(bpp, type) \
static void \
SDL_FillRect##bpp##SSE(Uint8 *pixels, int pitch, Uint32 color, int w, int h) \
{ \
    int i, n; \
    Uint8 *p = NULL; \
 \
    SSE_BEGIN; \
 \
    while (h--) { \
        n = w * bpp; \
        p = pixels; \
 \
        if (n > 63) { \
            int adjust = 16 - ((uintptr_t)p & 15); \
            if (adjust < 16) { \
                n -= adjust; \
                adjust /= bpp; \
                while (adjust--) { \
                    *((type *)p) = (type)color; \
                    p += bpp; \
                } \
            } \
            SSE_WORK; \
        } \
        if (n & 63) { \
            int remainder = (n & 63); \
            remainder /= bpp; \
            while (remainder--) { \
                *((type *)p) = (type)color; \
                p += bpp; \
            } \
        } \
        pixels += pitch; \
    } \
 \
    SSE_END; \
}

static void
SDL_FillRect1SSE(Uint8 *pixels, int pitch, Uint32 color, int w, int h)
{
    int i, n;

    SSE_BEGIN;
    while (h--) {
        Uint8 *p = pixels;
        n = w;

        if (n > 63) {
            int adjust = 16 - ((uintptr_t)p & 15);
            if (adjust) {
                n -= adjust;
                SDL_memset(p, color, adjust);
                p += adjust;
            }
            SSE_WORK;
        }
        if (n & 63) {
            int remainder = (n & 63);
            SDL_memset(p, color, remainder);
        }
        pixels += pitch;
    }

    SSE_END;
}
/* DEFINE_SSE_FILLRECT(1, Uint8) */
DEFINE_SSE_FILLRECT(2, Uint16)
DEFINE_SSE_FILLRECT(4, Uint32)

/* *INDENT-ON* */
#endif /* __SSE__ */

static void
SDL_FillRect1(Uint8 * pixels, int pitch, Uint32 color, int w, int h)
{
    int n;
    Uint8 *p = NULL;
    
    while (h--) {
        n = w;
        p = pixels;

        if (n > 3) {
            switch ((uintptr_t) p & 3) {
            case 1:
                *p++ = (Uint8) color;
                --n;                    /* fallthrough */
            case 2:
                *p++ = (Uint8) color;
                --n;                    /* fallthrough */
            case 3:
                *p++ = (Uint8) color;
                --n;                    /* fallthrough */
            }
            SDL_memset4(p, color, (n >> 2));
        }
        if (n & 3) {
            p += (n & ~3);
            switch (n & 3) {
            case 3:
                *p++ = (Uint8) color;   /* fallthrough */
            case 2:
                *p++ = (Uint8) color;   /* fallthrough */
            case 1:
                *p++ = (Uint8) color;   /* fallthrough */
            }
        }
        pixels += pitch;
    }
}

static void
SDL_FillRect2(Uint8 * pixels, int pitch, Uint32 color, int w, int h)
{
    int n;
    Uint16 *p = NULL;
    
    while (h--) {
        n = w;
        p = (Uint16 *) pixels;

        if (n > 1) {
            if ((uintptr_t) p & 2) {
                *p++ = (Uint16) color;
                --n;
            }
            SDL_memset4(p, color, (n >> 1));
        }
        if (n & 1) {
            p[n - 1] = (Uint16) color;
        }
        pixels += pitch;
    }
}

static void
SDL_FillRect3(Uint8 * pixels, int pitch, Uint32 color, int w, int h)
{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    Uint8 b1 = (Uint8) (color & 0xFF);
    Uint8 b2 = (Uint8) ((color >> 8) & 0xFF);
    Uint8 b3 = (Uint8) ((color >> 16) & 0xFF);
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint8 b1 = (Uint8) ((color >> 16) & 0xFF);
    Uint8 b2 = (Uint8) ((color >> 8) & 0xFF);
    Uint8 b3 = (Uint8) (color & 0xFF);
#endif
    int n;
    Uint8 *p = NULL;

    while (h--) {
        n = w;
        p = pixels;

        while (n--) {
            *p++ = b1;
            *p++ = b2;
            *p++ = b3;
        }
        pixels += pitch;
    }
}

static void
SDL_FillRect4(Uint8 * pixels, int pitch, Uint32 color, int w, int h)
{
    while (h--) {
        SDL_memset4(pixels, color, w);
        pixels += pitch;
    }
}

/* 
 * This function performs a fast fill of the given rectangle with 'color'
 */
int
SDL_FillRect(SDL_Surface * dst, const SDL_Rect * rect, Uint32 color)
{
    SDL_Rect clipped;
    Uint8 *pixels;

    if (!dst) {
        return SDL_SetError("Passed NULL destination surface");
    }

    /* This function doesn't work on surfaces < 8 bpp */
    if (dst->format->BitsPerPixel < 8) {
        return SDL_SetError("SDL_FillRect(): Unsupported surface format");
    }

    /* If 'rect' == NULL, then fill the whole surface */
    if (rect) {
        /* Perform clipping */
        if (!SDL_IntersectRect(rect, &dst->clip_rect, &clipped)) {
            return 0;
        }
        rect = &clipped;
    } else {
        rect = &dst->clip_rect;
        /* Don't attempt to fill if the surface's clip_rect is empty */
        if (SDL_RectEmpty(rect)) {
            return 0;
        }
    }

    /* Perform software fill */
    if (!dst->pixels) {
        return SDL_SetError("SDL_FillRect(): You must lock the surface");
    }

    pixels = (Uint8 *) dst->pixels + rect->y * dst->pitch +
                                     rect->x * dst->format->BytesPerPixel;

    switch (dst->format->BytesPerPixel) {
    case 1:
        {
            color |= (color << 8);
            color |= (color << 16);
#ifdef __SSE__
            if (SDL_HasSSE()) {
                SDL_FillRect1SSE(pixels, dst->pitch, color, rect->w, rect->h);
                break;
            }
#endif
            SDL_FillRect1(pixels, dst->pitch, color, rect->w, rect->h);
            break;
        }

    case 2:
        {
            color |= (color << 16);
#ifdef __SSE__
            if (SDL_HasSSE()) {
                SDL_FillRect2SSE(pixels, dst->pitch, color, rect->w, rect->h);
                break;
            }
#endif
            SDL_FillRect2(pixels, dst->pitch, color, rect->w, rect->h);
            break;
        }

    case 3:
        /* 24-bit RGB is a slow path, at least for now. */
        {
            SDL_FillRect3(pixels, dst->pitch, color, rect->w, rect->h);
            break;
        }

    case 4:
        {
#ifdef __SSE__
            if (SDL_HasSSE()) {
                SDL_FillRect4SSE(pixels, dst->pitch, color, rect->w, rect->h);
                break;
            }
#endif
            SDL_FillRect4(pixels, dst->pitch, color, rect->w, rect->h);
            break;
        }
    }

    /* We're done! */
    return 0;
}

int
SDL_FillRects(SDL_Surface * dst, const SDL_Rect * rects, int count,
              Uint32 color)
{
    int i;
    int status = 0;

    if (!rects) {
        return SDL_SetError("SDL_FillRects() passed NULL rects");
    }

    for (i = 0; i < count; ++i) {
        status += SDL_FillRect(dst, &rects[i], color);
    }
    return status;
}

/* vi: set ts=4 sw=4 expandtab: */
