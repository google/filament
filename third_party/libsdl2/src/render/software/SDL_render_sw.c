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

#if !SDL_RENDER_DISABLED

#include "../SDL_sysrender.h"
#include "SDL_render_sw_c.h"
#include "SDL_hints.h"

#include "SDL_draw.h"
#include "SDL_blendfillrect.h"
#include "SDL_blendline.h"
#include "SDL_blendpoint.h"
#include "SDL_drawline.h"
#include "SDL_drawpoint.h"
#include "SDL_rotate.h"

/* SDL surface based renderer implementation */

static SDL_Renderer *SW_CreateRenderer(SDL_Window * window, Uint32 flags);
static void SW_WindowEvent(SDL_Renderer * renderer,
                           const SDL_WindowEvent *event);
static int SW_GetOutputSize(SDL_Renderer * renderer, int *w, int *h);
static int SW_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int SW_SetTextureColorMod(SDL_Renderer * renderer,
                                 SDL_Texture * texture);
static int SW_SetTextureAlphaMod(SDL_Renderer * renderer,
                                 SDL_Texture * texture);
static int SW_SetTextureBlendMode(SDL_Renderer * renderer,
                                  SDL_Texture * texture);
static int SW_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                            const SDL_Rect * rect, const void *pixels,
                            int pitch);
static int SW_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                          const SDL_Rect * rect, void **pixels, int *pitch);
static void SW_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int SW_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture);
static int SW_UpdateViewport(SDL_Renderer * renderer);
static int SW_UpdateClipRect(SDL_Renderer * renderer);
static int SW_RenderClear(SDL_Renderer * renderer);
static int SW_RenderDrawPoints(SDL_Renderer * renderer,
                               const SDL_FPoint * points, int count);
static int SW_RenderDrawLines(SDL_Renderer * renderer,
                              const SDL_FPoint * points, int count);
static int SW_RenderFillRects(SDL_Renderer * renderer,
                              const SDL_FRect * rects, int count);
static int SW_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                         const SDL_Rect * srcrect, const SDL_FRect * dstrect);
static int SW_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                          const SDL_Rect * srcrect, const SDL_FRect * dstrect,
                          const double angle, const SDL_FPoint * center, const SDL_RendererFlip flip);
static int SW_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                               Uint32 format, void * pixels, int pitch);
static void SW_RenderPresent(SDL_Renderer * renderer);
static void SW_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static void SW_DestroyRenderer(SDL_Renderer * renderer);


SDL_RenderDriver SW_RenderDriver = {
    SW_CreateRenderer,
    {
     "software",
     SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE,
     8,
     {
      SDL_PIXELFORMAT_ARGB8888,
      SDL_PIXELFORMAT_ABGR8888,
      SDL_PIXELFORMAT_RGBA8888,
      SDL_PIXELFORMAT_BGRA8888,
      SDL_PIXELFORMAT_RGB888,
      SDL_PIXELFORMAT_BGR888,
      SDL_PIXELFORMAT_RGB565,
      SDL_PIXELFORMAT_RGB555
     },
     0,
     0}
};

typedef struct
{
    SDL_Surface *surface;
    SDL_Surface *window;
} SW_RenderData;


static SDL_Surface *
SW_ActivateRenderer(SDL_Renderer * renderer)
{
    SW_RenderData *data = (SW_RenderData *) renderer->driverdata;

    if (!data->surface) {
        data->surface = data->window;
    }
    if (!data->surface) {
        SDL_Surface *surface = SDL_GetWindowSurface(renderer->window);
        if (surface) {
            data->surface = data->window = surface;

            SW_UpdateViewport(renderer);
            SW_UpdateClipRect(renderer);
        }
    }
    return data->surface;
}

SDL_Renderer *
SW_CreateRendererForSurface(SDL_Surface * surface)
{
    SDL_Renderer *renderer;
    SW_RenderData *data;

    if (!surface) {
        SDL_SetError("Can't create renderer for NULL surface");
        return NULL;
    }

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (SW_RenderData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        SW_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }
    data->surface = surface;
    data->window = surface;

    renderer->WindowEvent = SW_WindowEvent;
    renderer->GetOutputSize = SW_GetOutputSize;
    renderer->CreateTexture = SW_CreateTexture;
    renderer->SetTextureColorMod = SW_SetTextureColorMod;
    renderer->SetTextureAlphaMod = SW_SetTextureAlphaMod;
    renderer->SetTextureBlendMode = SW_SetTextureBlendMode;
    renderer->UpdateTexture = SW_UpdateTexture;
    renderer->LockTexture = SW_LockTexture;
    renderer->UnlockTexture = SW_UnlockTexture;
    renderer->SetRenderTarget = SW_SetRenderTarget;
    renderer->UpdateViewport = SW_UpdateViewport;
    renderer->UpdateClipRect = SW_UpdateClipRect;
    renderer->RenderClear = SW_RenderClear;
    renderer->RenderDrawPoints = SW_RenderDrawPoints;
    renderer->RenderDrawLines = SW_RenderDrawLines;
    renderer->RenderFillRects = SW_RenderFillRects;
    renderer->RenderCopy = SW_RenderCopy;
    renderer->RenderCopyEx = SW_RenderCopyEx;
    renderer->RenderReadPixels = SW_RenderReadPixels;
    renderer->RenderPresent = SW_RenderPresent;
    renderer->DestroyTexture = SW_DestroyTexture;
    renderer->DestroyRenderer = SW_DestroyRenderer;
    renderer->info = SW_RenderDriver.info;
    renderer->driverdata = data;

    SW_ActivateRenderer(renderer);

    return renderer;
}

SDL_Renderer *
SW_CreateRenderer(SDL_Window * window, Uint32 flags)
{
    SDL_Surface *surface;

    surface = SDL_GetWindowSurface(window);
    if (!surface) {
        return NULL;
    }
    return SW_CreateRendererForSurface(surface);
}

static void
SW_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
    SW_RenderData *data = (SW_RenderData *) renderer->driverdata;

    if (event->event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        data->surface = NULL;
        data->window = NULL;
    }
}

static int
SW_GetOutputSize(SDL_Renderer * renderer, int *w, int *h)
{
    SDL_Surface *surface = SW_ActivateRenderer(renderer);

    if (surface) {
        if (w) {
            *w = surface->w;
        }
        if (h) {
            *h = surface->h;
        }
        return 0;
    } else {
        SDL_SetError("Software renderer doesn't have an output surface");
        return -1;
    }
}

static int
SW_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;

    if (!SDL_PixelFormatEnumToMasks
        (texture->format, &bpp, &Rmask, &Gmask, &Bmask, &Amask)) {
        return SDL_SetError("Unknown texture format");
    }

    texture->driverdata =
        SDL_CreateRGBSurface(0, texture->w, texture->h, bpp, Rmask, Gmask,
                             Bmask, Amask);
    SDL_SetSurfaceColorMod(texture->driverdata, texture->r, texture->g,
                           texture->b);
    SDL_SetSurfaceAlphaMod(texture->driverdata, texture->a);
    SDL_SetSurfaceBlendMode(texture->driverdata, texture->blendMode);

    /* Only RLE encode textures without an alpha channel since the RLE coder
     * discards the color values of pixels with an alpha value of zero.
     */
    if (texture->access == SDL_TEXTUREACCESS_STATIC && !Amask) {
        SDL_SetSurfaceRLE(texture->driverdata, 1);
    }

    if (!texture->driverdata) {
        return -1;
    }
    return 0;
}

static int
SW_SetTextureColorMod(SDL_Renderer * renderer, SDL_Texture * texture)
{
    SDL_Surface *surface = (SDL_Surface *) texture->driverdata;
    /* If the color mod is ever enabled (non-white), permanently disable RLE (which doesn't support
     * color mod) to avoid potentially frequent RLE encoding/decoding.
     */
    if ((texture->r & texture->g & texture->b) != 255) {
        SDL_SetSurfaceRLE(surface, 0);
    }
    return SDL_SetSurfaceColorMod(surface, texture->r, texture->g,
                                  texture->b);
}

static int
SW_SetTextureAlphaMod(SDL_Renderer * renderer, SDL_Texture * texture)
{
    SDL_Surface *surface = (SDL_Surface *) texture->driverdata;
    /* If the texture ever has multiple alpha values (surface alpha plus alpha channel), permanently
     * disable RLE (which doesn't support this) to avoid potentially frequent RLE encoding/decoding.
     */
    if (texture->a != 255 && surface->format->Amask) {
        SDL_SetSurfaceRLE(surface, 0);
    }
    return SDL_SetSurfaceAlphaMod(surface, texture->a);
}

static int
SW_SetTextureBlendMode(SDL_Renderer * renderer, SDL_Texture * texture)
{
    SDL_Surface *surface = (SDL_Surface *) texture->driverdata;
    /* If add or mod blending are ever enabled, permanently disable RLE (which doesn't support
     * them) to avoid potentially frequent RLE encoding/decoding.
     */
    if ((texture->blendMode == SDL_BLENDMODE_ADD || texture->blendMode == SDL_BLENDMODE_MOD)) {
        SDL_SetSurfaceRLE(surface, 0);
    }
    return SDL_SetSurfaceBlendMode(surface, texture->blendMode);
}

static int
SW_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                 const SDL_Rect * rect, const void *pixels, int pitch)
{
    SDL_Surface *surface = (SDL_Surface *) texture->driverdata;
    Uint8 *src, *dst;
    int row;
    size_t length;

    if(SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);
    src = (Uint8 *) pixels;
    dst = (Uint8 *) surface->pixels +
                        rect->y * surface->pitch +
                        rect->x * surface->format->BytesPerPixel;
    length = rect->w * surface->format->BytesPerPixel;
    for (row = 0; row < rect->h; ++row) {
        SDL_memcpy(dst, src, length);
        src += pitch;
        dst += surface->pitch;
    }
    if(SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);
    return 0;
}

static int
SW_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
               const SDL_Rect * rect, void **pixels, int *pitch)
{
    SDL_Surface *surface = (SDL_Surface *) texture->driverdata;

    *pixels =
        (void *) ((Uint8 *) surface->pixels + rect->y * surface->pitch +
                  rect->x * surface->format->BytesPerPixel);
    *pitch = surface->pitch;
    return 0;
}

static void
SW_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
}

static int
SW_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture)
{
    SW_RenderData *data = (SW_RenderData *) renderer->driverdata;

    if (texture ) {
        data->surface = (SDL_Surface *) texture->driverdata;
    } else {
        data->surface = data->window;
    }
    return 0;
}

static int
SW_UpdateViewport(SDL_Renderer * renderer)
{
    SW_RenderData *data = (SW_RenderData *) renderer->driverdata;
    SDL_Surface *surface = data->surface;

    if (!surface) {
        /* We'll update the viewport after we recreate the surface */
        return 0;
    }

    SDL_SetClipRect(data->surface, &renderer->viewport);
    return 0;
}

static int
SW_UpdateClipRect(SDL_Renderer * renderer)
{
    SW_RenderData *data = (SW_RenderData *) renderer->driverdata;
    SDL_Surface *surface = data->surface;
    if (surface) {
        if (renderer->clipping_enabled) {
            SDL_Rect clip_rect;
            clip_rect = renderer->clip_rect;
            clip_rect.x += renderer->viewport.x;
            clip_rect.y += renderer->viewport.y;
            SDL_IntersectRect(&renderer->viewport, &clip_rect, &clip_rect);
            SDL_SetClipRect(surface, &clip_rect);
        } else {
            SDL_SetClipRect(surface, &renderer->viewport);
        }
    }
    return 0;
}

static int
SW_RenderClear(SDL_Renderer * renderer)
{
    SDL_Surface *surface = SW_ActivateRenderer(renderer);
    Uint32 color;
    SDL_Rect clip_rect;

    if (!surface) {
        return -1;
    }

    color = SDL_MapRGBA(surface->format,
                        renderer->r, renderer->g, renderer->b, renderer->a);

    /* By definition the clear ignores the clip rect */
    clip_rect = surface->clip_rect;
    SDL_SetClipRect(surface, NULL);
    SDL_FillRect(surface, NULL, color);
    SDL_SetClipRect(surface, &clip_rect);
    return 0;
}

static int
SW_RenderDrawPoints(SDL_Renderer * renderer, const SDL_FPoint * points,
                    int count)
{
    SDL_Surface *surface = SW_ActivateRenderer(renderer);
    SDL_Point *final_points;
    int i, status;

    if (!surface) {
        return -1;
    }

    final_points = SDL_stack_alloc(SDL_Point, count);
    if (!final_points) {
        return SDL_OutOfMemory();
    }
    if (renderer->viewport.x || renderer->viewport.y) {
        int x = renderer->viewport.x;
        int y = renderer->viewport.y;

        for (i = 0; i < count; ++i) {
            final_points[i].x = (int)(x + points[i].x);
            final_points[i].y = (int)(y + points[i].y);
        }
    } else {
        for (i = 0; i < count; ++i) {
            final_points[i].x = (int)points[i].x;
            final_points[i].y = (int)points[i].y;
        }
    }

    /* Draw the points! */
    if (renderer->blendMode == SDL_BLENDMODE_NONE) {
        Uint32 color = SDL_MapRGBA(surface->format,
                                   renderer->r, renderer->g, renderer->b,
                                   renderer->a);

        status = SDL_DrawPoints(surface, final_points, count, color);
    } else {
        status = SDL_BlendPoints(surface, final_points, count,
                                renderer->blendMode,
                                renderer->r, renderer->g, renderer->b,
                                renderer->a);
    }
    SDL_stack_free(final_points);

    return status;
}

static int
SW_RenderDrawLines(SDL_Renderer * renderer, const SDL_FPoint * points,
                   int count)
{
    SDL_Surface *surface = SW_ActivateRenderer(renderer);
    SDL_Point *final_points;
    int i, status;

    if (!surface) {
        return -1;
    }

    final_points = SDL_stack_alloc(SDL_Point, count);
    if (!final_points) {
        return SDL_OutOfMemory();
    }
    if (renderer->viewport.x || renderer->viewport.y) {
        int x = renderer->viewport.x;
        int y = renderer->viewport.y;

        for (i = 0; i < count; ++i) {
            final_points[i].x = (int)(x + points[i].x);
            final_points[i].y = (int)(y + points[i].y);
        }
    } else {
        for (i = 0; i < count; ++i) {
            final_points[i].x = (int)points[i].x;
            final_points[i].y = (int)points[i].y;
        }
    }

    /* Draw the lines! */
    if (renderer->blendMode == SDL_BLENDMODE_NONE) {
        Uint32 color = SDL_MapRGBA(surface->format,
                                   renderer->r, renderer->g, renderer->b,
                                   renderer->a);

        status = SDL_DrawLines(surface, final_points, count, color);
    } else {
        status = SDL_BlendLines(surface, final_points, count,
                                renderer->blendMode,
                                renderer->r, renderer->g, renderer->b,
                                renderer->a);
    }
    SDL_stack_free(final_points);

    return status;
}

static int
SW_RenderFillRects(SDL_Renderer * renderer, const SDL_FRect * rects, int count)
{
    SDL_Surface *surface = SW_ActivateRenderer(renderer);
    SDL_Rect *final_rects;
    int i, status;

    if (!surface) {
        return -1;
    }

    final_rects = SDL_stack_alloc(SDL_Rect, count);
    if (!final_rects) {
        return SDL_OutOfMemory();
    }
    if (renderer->viewport.x || renderer->viewport.y) {
        int x = renderer->viewport.x;
        int y = renderer->viewport.y;

        for (i = 0; i < count; ++i) {
            final_rects[i].x = (int)(x + rects[i].x);
            final_rects[i].y = (int)(y + rects[i].y);
            final_rects[i].w = SDL_max((int)rects[i].w, 1);
            final_rects[i].h = SDL_max((int)rects[i].h, 1);
        }
    } else {
        for (i = 0; i < count; ++i) {
            final_rects[i].x = (int)rects[i].x;
            final_rects[i].y = (int)rects[i].y;
            final_rects[i].w = SDL_max((int)rects[i].w, 1);
            final_rects[i].h = SDL_max((int)rects[i].h, 1);
        }
    }

    if (renderer->blendMode == SDL_BLENDMODE_NONE) {
        Uint32 color = SDL_MapRGBA(surface->format,
                                   renderer->r, renderer->g, renderer->b,
                                   renderer->a);
        status = SDL_FillRects(surface, final_rects, count, color);
    } else {
        status = SDL_BlendFillRects(surface, final_rects, count,
                                    renderer->blendMode,
                                    renderer->r, renderer->g, renderer->b,
                                    renderer->a);
    }
    SDL_stack_free(final_rects);

    return status;
}

static int
SW_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
              const SDL_Rect * srcrect, const SDL_FRect * dstrect)
{
    SDL_Surface *surface = SW_ActivateRenderer(renderer);
    SDL_Surface *src = (SDL_Surface *) texture->driverdata;
    SDL_Rect final_rect;

    if (!surface) {
        return -1;
    }

    if (renderer->viewport.x || renderer->viewport.y) {
        final_rect.x = (int)(renderer->viewport.x + dstrect->x);
        final_rect.y = (int)(renderer->viewport.y + dstrect->y);
    } else {
        final_rect.x = (int)dstrect->x;
        final_rect.y = (int)dstrect->y;
    }
    final_rect.w = (int)dstrect->w;
    final_rect.h = (int)dstrect->h;

    if ( srcrect->w == final_rect.w && srcrect->h == final_rect.h ) {
        return SDL_BlitSurface(src, srcrect, surface, &final_rect);
    } else {
        /* If scaling is ever done, permanently disable RLE (which doesn't support scaling)
         * to avoid potentially frequent RLE encoding/decoding.
         */
        SDL_SetSurfaceRLE(surface, 0);
        return SDL_BlitScaled(src, srcrect, surface, &final_rect);
    }
}

static int
GetScaleQuality(void)
{
    const char *hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);

    if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
        return 0;
    } else {
        return 1;
    }
}

static int
SW_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                const SDL_Rect * srcrect, const SDL_FRect * dstrect,
                const double angle, const SDL_FPoint * center, const SDL_RendererFlip flip)
{
    SDL_Surface *surface = SW_ActivateRenderer(renderer);
    SDL_Surface *src = (SDL_Surface *) texture->driverdata;
    SDL_Rect final_rect, tmp_rect;
    SDL_Surface *src_clone, *src_rotated, *src_scaled;
    SDL_Surface *mask = NULL, *mask_rotated = NULL;
    int retval = 0, dstwidth, dstheight, abscenterx, abscentery;
    double cangle, sangle, px, py, p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y;
    SDL_BlendMode blendmode;
    Uint8 alphaMod, rMod, gMod, bMod;
    int applyModulation = SDL_FALSE;
    int blitRequired = SDL_FALSE;
    int isOpaque = SDL_FALSE;

    if (!surface) {
        return -1;
    }

    if (renderer->viewport.x || renderer->viewport.y) {
        final_rect.x = (int)(renderer->viewport.x + dstrect->x);
        final_rect.y = (int)(renderer->viewport.y + dstrect->y);
    } else {
        final_rect.x = (int)dstrect->x;
        final_rect.y = (int)dstrect->y;
    }
    final_rect.w = (int)dstrect->w;
    final_rect.h = (int)dstrect->h;

    tmp_rect = final_rect;
    tmp_rect.x = 0;
    tmp_rect.y = 0;

    /* It is possible to encounter an RLE encoded surface here and locking it is
     * necessary because this code is going to access the pixel buffer directly.
     */
    if (SDL_MUSTLOCK(src)) {
        SDL_LockSurface(src);
    }

    /* Clone the source surface but use its pixel buffer directly.
     * The original source surface must be treated as read-only.
     */
    src_clone = SDL_CreateRGBSurfaceFrom(src->pixels, src->w, src->h, src->format->BitsPerPixel, src->pitch,
                                         src->format->Rmask, src->format->Gmask,
                                         src->format->Bmask, src->format->Amask);
    if (src_clone == NULL) {
        if (SDL_MUSTLOCK(src)) {
            SDL_UnlockSurface(src);
        }
        return -1;
    }

    SDL_GetSurfaceBlendMode(src, &blendmode);
    SDL_GetSurfaceAlphaMod(src, &alphaMod);
    SDL_GetSurfaceColorMod(src, &rMod, &gMod, &bMod);

    /* SDLgfx_rotateSurface only accepts 32-bit surfaces with a 8888 layout. Everything else has to be converted. */
    if (src->format->BitsPerPixel != 32 || SDL_PIXELLAYOUT(src->format->format) != SDL_PACKEDLAYOUT_8888 || !src->format->Amask) {
        blitRequired = SDL_TRUE;
    }

    /* If scaling and cropping is necessary, it has to be taken care of before the rotation. */
    if (!(srcrect->w == final_rect.w && srcrect->h == final_rect.h && srcrect->x == 0 && srcrect->y == 0)) {
        blitRequired = SDL_TRUE;
    }

    /* The color and alpha modulation has to be applied before the rotation when using the NONE and MOD blend modes. */
    if ((blendmode == SDL_BLENDMODE_NONE || blendmode == SDL_BLENDMODE_MOD) && (alphaMod & rMod & gMod & bMod) != 255) {
        applyModulation = SDL_TRUE;
        SDL_SetSurfaceAlphaMod(src_clone, alphaMod);
        SDL_SetSurfaceColorMod(src_clone, rMod, gMod, bMod);
    }

    /* Opaque surfaces are much easier to handle with the NONE blend mode. */
    if (blendmode == SDL_BLENDMODE_NONE && !src->format->Amask && alphaMod == 255) {
        isOpaque = SDL_TRUE;
    }

    /* The NONE blend mode requires a mask for non-opaque surfaces. This mask will be used
     * to clear the pixels in the destination surface. The other steps are explained below.
     */
    if (blendmode == SDL_BLENDMODE_NONE && !isOpaque) {
        mask = SDL_CreateRGBSurface(0, final_rect.w, final_rect.h, 32,
                                    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
        if (mask == NULL) {
            retval = -1;
        } else {
            SDL_SetSurfaceBlendMode(mask, SDL_BLENDMODE_MOD);
        }
    }

    /* Create a new surface should there be a format mismatch or if scaling, cropping,
     * or modulation is required. It's possible to use the source surface directly otherwise.
     */
    if (!retval && (blitRequired || applyModulation)) {
        SDL_Rect scale_rect = tmp_rect;
        src_scaled = SDL_CreateRGBSurface(0, final_rect.w, final_rect.h, 32,
                                          0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
        if (src_scaled == NULL) {
            retval = -1;
        } else {
            SDL_SetSurfaceBlendMode(src_clone, SDL_BLENDMODE_NONE);
            retval = SDL_BlitScaled(src_clone, srcrect, src_scaled, &scale_rect);
            SDL_FreeSurface(src_clone);
            src_clone = src_scaled;
            src_scaled = NULL;
        }
    }

    /* SDLgfx_rotateSurface is going to make decisions depending on the blend mode. */
    SDL_SetSurfaceBlendMode(src_clone, blendmode);

    if (!retval) {
        SDLgfx_rotozoomSurfaceSizeTrig(tmp_rect.w, tmp_rect.h, angle, &dstwidth, &dstheight, &cangle, &sangle);
        src_rotated = SDLgfx_rotateSurface(src_clone, angle, dstwidth/2, dstheight/2, GetScaleQuality(), flip & SDL_FLIP_HORIZONTAL, flip & SDL_FLIP_VERTICAL, dstwidth, dstheight, cangle, sangle);
        if (src_rotated == NULL) {
            retval = -1;
        }
        if (!retval && mask != NULL) {
            /* The mask needed for the NONE blend mode gets rotated with the same parameters. */
            mask_rotated = SDLgfx_rotateSurface(mask, angle, dstwidth/2, dstheight/2, SDL_FALSE, 0, 0, dstwidth, dstheight, cangle, sangle);
            if (mask_rotated == NULL) {
                retval = -1;
            }
        }
        if (!retval) {
            /* Find out where the new origin is by rotating the four final_rect points around the center and then taking the extremes */
            abscenterx = final_rect.x + (int)center->x;
            abscentery = final_rect.y + (int)center->y;
            /* Compensate the angle inversion to match the behaviour of the other backends */
            sangle = -sangle;

            /* Top Left */
            px = final_rect.x - abscenterx;
            py = final_rect.y - abscentery;
            p1x = px * cangle - py * sangle + abscenterx;
            p1y = px * sangle + py * cangle + abscentery;

            /* Top Right */
            px = final_rect.x + final_rect.w - abscenterx;
            py = final_rect.y - abscentery;
            p2x = px * cangle - py * sangle + abscenterx;
            p2y = px * sangle + py * cangle + abscentery;

            /* Bottom Left */
            px = final_rect.x - abscenterx;
            py = final_rect.y + final_rect.h - abscentery;
            p3x = px * cangle - py * sangle + abscenterx;
            p3y = px * sangle + py * cangle + abscentery;

            /* Bottom Right */
            px = final_rect.x + final_rect.w - abscenterx;
            py = final_rect.y + final_rect.h - abscentery;
            p4x = px * cangle - py * sangle + abscenterx;
            p4y = px * sangle + py * cangle + abscentery;

            tmp_rect.x = (int)MIN(MIN(p1x, p2x), MIN(p3x, p4x));
            tmp_rect.y = (int)MIN(MIN(p1y, p2y), MIN(p3y, p4y));
            tmp_rect.w = dstwidth;
            tmp_rect.h = dstheight;

            /* The NONE blend mode needs some special care with non-opaque surfaces.
             * Other blend modes or opaque surfaces can be blitted directly.
             */
            if (blendmode != SDL_BLENDMODE_NONE || isOpaque) {
                if (applyModulation == SDL_FALSE) {
                    /* If the modulation wasn't already applied, make it happen now. */
                    SDL_SetSurfaceAlphaMod(src_rotated, alphaMod);
                    SDL_SetSurfaceColorMod(src_rotated, rMod, gMod, bMod);
                }
                retval = SDL_BlitSurface(src_rotated, NULL, surface, &tmp_rect);
            } else {
                /* The NONE blend mode requires three steps to get the pixels onto the destination surface.
                 * First, the area where the rotated pixels will be blitted to get set to zero.
                 * This is accomplished by simply blitting a mask with the NONE blend mode.
                 * The colorkey set by the rotate function will discard the correct pixels.
                 */
                SDL_Rect mask_rect = tmp_rect;
                SDL_SetSurfaceBlendMode(mask_rotated, SDL_BLENDMODE_NONE);
                retval = SDL_BlitSurface(mask_rotated, NULL, surface, &mask_rect);
                if (!retval) {
                    /* The next step copies the alpha value. This is done with the BLEND blend mode and
                     * by modulating the source colors with 0. Since the destination is all zeros, this
                     * will effectively set the destination alpha to the source alpha.
                     */
                    SDL_SetSurfaceColorMod(src_rotated, 0, 0, 0);
                    mask_rect = tmp_rect;
                    retval = SDL_BlitSurface(src_rotated, NULL, surface, &mask_rect);
                    if (!retval) {
                        /* The last step gets the color values in place. The ADD blend mode simply adds them to
                         * the destination (where the color values are all zero). However, because the ADD blend
                         * mode modulates the colors with the alpha channel, a surface without an alpha mask needs
                         * to be created. This makes all source pixels opaque and the colors get copied correctly.
                         */
                        SDL_Surface *src_rotated_rgb;
                        src_rotated_rgb = SDL_CreateRGBSurfaceFrom(src_rotated->pixels, src_rotated->w, src_rotated->h,
                                                                   src_rotated->format->BitsPerPixel, src_rotated->pitch,
                                                                   src_rotated->format->Rmask, src_rotated->format->Gmask,
                                                                   src_rotated->format->Bmask, 0);
                        if (src_rotated_rgb == NULL) {
                            retval = -1;
                        } else {
                            SDL_SetSurfaceBlendMode(src_rotated_rgb, SDL_BLENDMODE_ADD);
                            retval = SDL_BlitSurface(src_rotated_rgb, NULL, surface, &tmp_rect);
                            SDL_FreeSurface(src_rotated_rgb);
                        }
                    }
                }
                SDL_FreeSurface(mask_rotated);
            }
            if (src_rotated != NULL) {
                SDL_FreeSurface(src_rotated);
            }
        }
    }

    if (SDL_MUSTLOCK(src)) {
        SDL_UnlockSurface(src);
    }
    if (mask != NULL) {
        SDL_FreeSurface(mask);
    }
    if (src_clone != NULL) {
        SDL_FreeSurface(src_clone);
    }
    return retval;
}

static int
SW_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                    Uint32 format, void * pixels, int pitch)
{
    SDL_Surface *surface = SW_ActivateRenderer(renderer);
    Uint32 src_format;
    void *src_pixels;

    if (!surface) {
        return -1;
    }

    /* NOTE: The rect is already adjusted according to the viewport by
     * SDL_RenderReadPixels.
     */

    if (rect->x < 0 || rect->x+rect->w > surface->w ||
        rect->y < 0 || rect->y+rect->h > surface->h) {
        return SDL_SetError("Tried to read outside of surface bounds");
    }

    src_format = surface->format->format;
    src_pixels = (void*)((Uint8 *) surface->pixels +
                    rect->y * surface->pitch +
                    rect->x * surface->format->BytesPerPixel);

    return SDL_ConvertPixels(rect->w, rect->h,
                             src_format, src_pixels, surface->pitch,
                             format, pixels, pitch);
}

static void
SW_RenderPresent(SDL_Renderer * renderer)
{
    SDL_Window *window = renderer->window;

    if (window) {
        SDL_UpdateWindowSurface(window);
    }
}

static void
SW_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    SDL_Surface *surface = (SDL_Surface *) texture->driverdata;

    SDL_FreeSurface(surface);
}

static void
SW_DestroyRenderer(SDL_Renderer * renderer)
{
    SW_RenderData *data = (SW_RenderData *) renderer->driverdata;

    SDL_free(data);
    SDL_free(renderer);
}

#endif /* !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
