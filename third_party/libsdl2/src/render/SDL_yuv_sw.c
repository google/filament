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
#include "../SDL_internal.h"

/* This is the software implementation of the YUV texture support */

#if SDL_HAVE_YUV


#include "SDL_yuv_sw_c.h"
#include "SDL_cpuinfo.h"


SDL_SW_YUVTexture *
SDL_SW_CreateYUVTexture(Uint32 format, int w, int h)
{
    SDL_SW_YUVTexture *swdata;

    switch (format) {
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_YUY2:
    case SDL_PIXELFORMAT_UYVY:
    case SDL_PIXELFORMAT_YVYU:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        break;
    default:
        SDL_SetError("Unsupported YUV format");
        return NULL;
    }

    swdata = (SDL_SW_YUVTexture *) SDL_calloc(1, sizeof(*swdata));
    if (!swdata) {
        SDL_OutOfMemory();
        return NULL;
    }

    swdata->format = format;
    swdata->target_format = SDL_PIXELFORMAT_UNKNOWN;
    swdata->w = w;
    swdata->h = h;
    {
        const int sz_plane         = w * h;
        const int sz_plane_chroma  = ((w + 1) / 2) * ((h + 1) / 2);
        const int sz_plane_packed  = ((w + 1) / 2) * h;
        int dst_size = 0;     
        switch(format) 
        {
            case SDL_PIXELFORMAT_YV12: /**< Planar mode: Y + V + U  (3 planes) */
            case SDL_PIXELFORMAT_IYUV: /**< Planar mode: Y + U + V  (3 planes) */
                dst_size = sz_plane + sz_plane_chroma + sz_plane_chroma;
                break;

            case SDL_PIXELFORMAT_YUY2: /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
            case SDL_PIXELFORMAT_UYVY: /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
            case SDL_PIXELFORMAT_YVYU: /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
                dst_size = 4 * sz_plane_packed;
                break;

            case SDL_PIXELFORMAT_NV12: /**< Planar mode: Y + U/V interleaved  (2 planes) */
            case SDL_PIXELFORMAT_NV21: /**< Planar mode: Y + V/U interleaved  (2 planes) */
                dst_size = sz_plane + sz_plane_chroma + sz_plane_chroma;
                break;

            default:
                SDL_assert(0 && "We should never get here (caught above)");
                break;
        }
        swdata->pixels = (Uint8 *) SDL_SIMDAlloc(dst_size);
        if (!swdata->pixels) {
            SDL_SW_DestroyYUVTexture(swdata);
            SDL_OutOfMemory();
            return NULL;
        }
    }

    /* Find the pitch and offset values for the texture */
    switch (format) {
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
        swdata->pitches[0] = w;
        swdata->pitches[1] = (swdata->pitches[0] + 1) / 2;
        swdata->pitches[2] = (swdata->pitches[0] + 1) / 2;
        swdata->planes[0] = swdata->pixels;
        swdata->planes[1] = swdata->planes[0] + swdata->pitches[0] * h;
        swdata->planes[2] = swdata->planes[1] + swdata->pitches[1] * ((h + 1) / 2);
        break;
    case SDL_PIXELFORMAT_YUY2:
    case SDL_PIXELFORMAT_UYVY:
    case SDL_PIXELFORMAT_YVYU:
        swdata->pitches[0] = ((w + 1) / 2) * 4;
        swdata->planes[0] = swdata->pixels;
        break;

    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        swdata->pitches[0] = w;
        swdata->pitches[1] = 2 * ((swdata->pitches[0] + 1) / 2);
        swdata->planes[0] = swdata->pixels;
        swdata->planes[1] = swdata->planes[0] + swdata->pitches[0] * h;
        break;

    default:
        SDL_assert(0 && "We should never get here (caught above)");
        break;
    }

    /* We're all done.. */
    return (swdata);
}

int
SDL_SW_QueryYUVTexturePixels(SDL_SW_YUVTexture * swdata, void **pixels,
                             int *pitch)
{
    *pixels = swdata->planes[0];
    *pitch = swdata->pitches[0];
    return 0;
}

int
SDL_SW_UpdateYUVTexture(SDL_SW_YUVTexture * swdata, const SDL_Rect * rect,
                        const void *pixels, int pitch)
{
    switch (swdata->format) {
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
        if (rect->x == 0 && rect->y == 0 &&
            rect->w == swdata->w && rect->h == swdata->h) {
                SDL_memcpy(swdata->pixels, pixels,
                           (swdata->h * swdata->w) + 2* ((swdata->h + 1) /2) * ((swdata->w + 1) / 2));
        } else {
            Uint8 *src, *dst;
            int row;
            size_t length;

            /* Copy the Y plane */
            src = (Uint8 *) pixels;
            dst = swdata->pixels + rect->y * swdata->w + rect->x;
            length = rect->w;
            for (row = 0; row < rect->h; ++row) {
                SDL_memcpy(dst, src, length);
                src += pitch;
                dst += swdata->w;
            }
            
            /* Copy the next plane */
            src = (Uint8 *) pixels + rect->h * pitch;
            dst = swdata->pixels + swdata->h * swdata->w;
            dst += rect->y/2 * ((swdata->w + 1) / 2) + rect->x/2;
            length = (rect->w + 1) / 2;
            for (row = 0; row < (rect->h + 1)/2; ++row) {
                SDL_memcpy(dst, src, length);
                src += (pitch + 1)/2;
                dst += (swdata->w + 1)/2;
            }

            /* Copy the next plane */
            src = (Uint8 *) pixels + rect->h * pitch + ((rect->h + 1) / 2) * ((pitch + 1) / 2);
            dst = swdata->pixels + swdata->h * swdata->w +
                  ((swdata->h + 1)/2) * ((swdata->w+1) / 2);
            dst += rect->y/2 * ((swdata->w + 1)/2) + rect->x/2;
            length = (rect->w + 1) / 2;
            for (row = 0; row < (rect->h + 1)/2; ++row) {
                SDL_memcpy(dst, src, length);
                src += (pitch + 1)/2;
                dst += (swdata->w + 1)/2;
            }
        }
        break;
    case SDL_PIXELFORMAT_YUY2:
    case SDL_PIXELFORMAT_UYVY:
    case SDL_PIXELFORMAT_YVYU:
        {
            Uint8 *src, *dst;
            int row;
            size_t length;

            src = (Uint8 *) pixels;
            dst =
                swdata->planes[0] + rect->y * swdata->pitches[0] +
                rect->x * 2;
            length = 4 * ((rect->w + 1) / 2);
            for (row = 0; row < rect->h; ++row) {
                SDL_memcpy(dst, src, length);
                src += pitch;
                dst += swdata->pitches[0];
            }
        }
        break;
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        {
            if (rect->x == 0 && rect->y == 0 && rect->w == swdata->w && rect->h == swdata->h) {
                SDL_memcpy(swdata->pixels, pixels,
                        (swdata->h * swdata->w) + 2* ((swdata->h + 1) /2) * ((swdata->w + 1) / 2));
            } else {

                Uint8 *src, *dst;
                int row;
                size_t length;

                /* Copy the Y plane */
                src = (Uint8 *) pixels;
                dst = swdata->pixels + rect->y * swdata->w + rect->x;
                length = rect->w;
                for (row = 0; row < rect->h; ++row) {
                    SDL_memcpy(dst, src, length);
                    src += pitch;
                    dst += swdata->w;
                }
                
                /* Copy the next plane */
                src = (Uint8 *) pixels + rect->h * pitch;
                dst = swdata->pixels + swdata->h * swdata->w;
                dst += 2 * ((rect->y + 1)/2) * ((swdata->w + 1) / 2) + 2 * (rect->x/2);
                length = 2 * ((rect->w + 1) / 2);
                for (row = 0; row < (rect->h + 1)/2; ++row) {
                    SDL_memcpy(dst, src, length);
                    src += 2 * ((pitch + 1)/2);
                    dst += 2 * ((swdata->w + 1)/2);
                }
            }
        }
    }
    return 0;
}

int
SDL_SW_UpdateYUVTexturePlanar(SDL_SW_YUVTexture * swdata, const SDL_Rect * rect,
                              const Uint8 *Yplane, int Ypitch,
                              const Uint8 *Uplane, int Upitch,
                              const Uint8 *Vplane, int Vpitch)
{
    const Uint8 *src;
    Uint8 *dst;
    int row;
    size_t length;

    /* Copy the Y plane */
    src = Yplane;
    dst = swdata->pixels + rect->y * swdata->w + rect->x;
    length = rect->w;
    for (row = 0; row < rect->h; ++row) {
        SDL_memcpy(dst, src, length);
        src += Ypitch;
        dst += swdata->w;
    }

    /* Copy the U plane */
    src = Uplane;
    if (swdata->format == SDL_PIXELFORMAT_IYUV) {
        dst = swdata->pixels + swdata->h * swdata->w;
    } else {
        dst = swdata->pixels + swdata->h * swdata->w +
              ((swdata->h + 1) / 2) * ((swdata->w + 1) / 2);
    }
    dst += rect->y/2 * ((swdata->w + 1)/2) + rect->x/2;
    length = (rect->w + 1) / 2;
    for (row = 0; row < (rect->h + 1)/2; ++row) {
        SDL_memcpy(dst, src, length);
        src += Upitch;
        dst += (swdata->w + 1)/2;
    }

    /* Copy the V plane */
    src = Vplane;
    if (swdata->format == SDL_PIXELFORMAT_YV12) {
        dst = swdata->pixels + swdata->h * swdata->w;
    } else {
        dst = swdata->pixels + swdata->h * swdata->w +
              ((swdata->h + 1) / 2) * ((swdata->w + 1) / 2);
    }
    dst += rect->y/2 * ((swdata->w + 1)/2) + rect->x/2;
    length = (rect->w + 1) / 2;
    for (row = 0; row < (rect->h + 1)/2; ++row) {
        SDL_memcpy(dst, src, length);
        src += Vpitch;
        dst += (swdata->w + 1)/2;
    }
    return 0;
}

int SDL_SW_UpdateNVTexturePlanar(SDL_SW_YUVTexture * swdata, const SDL_Rect * rect,
                                  const Uint8 *Yplane, int Ypitch,
                                  const Uint8 *UVplane, int UVpitch)
{
    const Uint8 *src;
    Uint8 *dst;
    int row;
    size_t length;

    /* Copy the Y plane */
    src = Yplane;
    dst = swdata->pixels + rect->y * swdata->w + rect->x;
    length = rect->w;
    for (row = 0; row < rect->h; ++row) {
        SDL_memcpy(dst, src, length);
        src += Ypitch;
        dst += swdata->w;
    }

    /* Copy the UV or VU plane */
    src = UVplane;
    dst = swdata->pixels + swdata->h * swdata->w;
    dst += rect->y * ((swdata->w + 1)/2) + rect->x;
    length = (rect->w + 1) / 2;
    length *= 2;
    for (row = 0; row < (rect->h + 1)/2; ++row) {
        SDL_memcpy(dst, src, length);
        src += UVpitch;
        dst += 2 * ((swdata->w + 1)/2);
    }

    return 0;
}

int
SDL_SW_LockYUVTexture(SDL_SW_YUVTexture * swdata, const SDL_Rect * rect,
                      void **pixels, int *pitch)
{
    switch (swdata->format) {
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        if (rect
            && (rect->x != 0 || rect->y != 0 || rect->w != swdata->w
                || rect->h != swdata->h)) {
            return SDL_SetError
                ("YV12, IYUV, NV12, NV21 textures only support full surface locks");
        }
        break;
    }

    if (rect) {
        *pixels = swdata->planes[0] + rect->y * swdata->pitches[0] + rect->x * 2;
    } else {
        *pixels = swdata->planes[0];
    }
    *pitch = swdata->pitches[0];
    return 0;
}

void
SDL_SW_UnlockYUVTexture(SDL_SW_YUVTexture * swdata)
{
}

int
SDL_SW_CopyYUVToRGB(SDL_SW_YUVTexture * swdata, const SDL_Rect * srcrect,
                    Uint32 target_format, int w, int h, void *pixels,
                    int pitch)
{
    int stretch;

    /* Make sure we're set up to display in the desired format */
    if (target_format != swdata->target_format && swdata->display) {
        SDL_FreeSurface(swdata->display);
        swdata->display = NULL;
    }

    stretch = 0;
    if (srcrect->x || srcrect->y || srcrect->w < swdata->w || srcrect->h < swdata->h) {
        /* The source rectangle has been clipped.
           Using a scratch surface is easier than adding clipped
           source support to all the blitters, plus that would
           slow them down in the general unclipped case.
         */
        stretch = 1;
    } else if ((srcrect->w != w) || (srcrect->h != h)) {
        stretch = 1;
    }
    if (stretch) {
        int bpp;
        Uint32 Rmask, Gmask, Bmask, Amask;

        if (swdata->display) {
            swdata->display->w = w;
            swdata->display->h = h;
            swdata->display->pixels = pixels;
            swdata->display->pitch = pitch;
        } else {
            /* This must have succeeded in SDL_SW_SetupYUVDisplay() earlier */
            SDL_PixelFormatEnumToMasks(target_format, &bpp, &Rmask, &Gmask,
                                       &Bmask, &Amask);
            swdata->display =
                SDL_CreateRGBSurfaceFrom(pixels, w, h, bpp, pitch, Rmask,
                                         Gmask, Bmask, Amask);
            if (!swdata->display) {
                return (-1);
            }
        }
        if (!swdata->stretch) {
            /* This must have succeeded in SDL_SW_SetupYUVDisplay() earlier */
            SDL_PixelFormatEnumToMasks(target_format, &bpp, &Rmask, &Gmask,
                                       &Bmask, &Amask);
            swdata->stretch =
                SDL_CreateRGBSurface(0, swdata->w, swdata->h, bpp, Rmask,
                                     Gmask, Bmask, Amask);
            if (!swdata->stretch) {
                return (-1);
            }
        }
        pixels = swdata->stretch->pixels;
        pitch = swdata->stretch->pitch;
    }
    if (SDL_ConvertPixels(swdata->w, swdata->h, swdata->format,
                          swdata->planes[0], swdata->pitches[0], 
                          target_format, pixels, pitch) < 0) {
        return -1;
    }
    if (stretch) {
        SDL_Rect rect = *srcrect;
        SDL_SoftStretch(swdata->stretch, &rect, swdata->display, NULL);
    }
    return 0;
}

void
SDL_SW_DestroyYUVTexture(SDL_SW_YUVTexture * swdata)
{
    if (swdata) {
        SDL_SIMDFree(swdata->pixels);
        SDL_FreeSurface(swdata->stretch);
        SDL_FreeSurface(swdata->display);
        SDL_free(swdata);
    }
}

#endif /* SDL_HAVE_YUV */

/* vi: set ts=4 sw=4 expandtab: */
