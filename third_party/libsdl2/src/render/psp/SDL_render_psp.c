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
#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_PSP

#include "SDL_hints.h"
#include "../SDL_sysrender.h"

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pspge.h>
#include <stdarg.h>
#include <stdlib.h>
#include <vram.h>




/* PSP renderer implementation, based on the PGE  */

#define PSP_SCREEN_WIDTH    480
#define PSP_SCREEN_HEIGHT   272

#define PSP_FRAME_BUFFER_WIDTH  512
#define PSP_FRAME_BUFFER_SIZE   (PSP_FRAME_BUFFER_WIDTH*PSP_SCREEN_HEIGHT)

static unsigned int __attribute__((aligned(16))) DisplayList[262144];


#define COL5650(r,g,b,a)    ((r>>3) | ((g>>2)<<5) | ((b>>3)<<11))
#define COL5551(r,g,b,a)    ((r>>3) | ((g>>3)<<5) | ((b>>3)<<10) | (a>0?0x7000:0))
#define COL4444(r,g,b,a)    ((r>>4) | ((g>>4)<<4) | ((b>>4)<<8) | ((a>>4)<<12))
#define COL8888(r,g,b,a)    ((r) | ((g)<<8) | ((b)<<16) | ((a)<<24))


typedef struct
{
    void*           frontbuffer ;
    void*           backbuffer ;
    SDL_bool        initialized ;
    SDL_bool        displayListAvail ;
    unsigned int    psm ;
    unsigned int    bpp ;

    SDL_bool        vsync;
    unsigned int    currentColor;
    int             currentBlendMode;

} PSP_RenderData;


typedef struct
{
    void                *data;                              /**< Image data. */
    unsigned int        size;                               /**< Size of data in bytes. */
    unsigned int        width;                              /**< Image width. */
    unsigned int        height;                             /**< Image height. */
    unsigned int        textureWidth;                       /**< Texture width (power of two). */
    unsigned int        textureHeight;                      /**< Texture height (power of two). */
    unsigned int        bits;                               /**< Image bits per pixel. */
    unsigned int        format;                             /**< Image format - one of ::pgePixelFormat. */
    unsigned int        pitch;
    SDL_bool            swizzled;                           /**< Is image swizzled. */

} PSP_TextureData;

typedef struct
{
    float   x, y, z;
} VertV;


typedef struct
{
    float   u, v;
    float   x, y, z;

} VertTV;

#define PI   3.14159265358979f

#define radToDeg(x) ((x)*180.f/PI)
#define degToRad(x) ((x)*PI/180.f)

float MathAbs(float x)
{
    float result;

    __asm__ volatile (
        "mtv      %1, S000\n"
        "vabs.s   S000, S000\n"
        "mfv      %0, S000\n"
    : "=r"(result) : "r"(x));

    return result;
}

void MathSincos(float r, float *s, float *c)
{
    __asm__ volatile (
        "mtv      %2, S002\n"
        "vcst.s   S003, VFPU_2_PI\n"
        "vmul.s   S002, S002, S003\n"
        "vrot.p   C000, S002, [s, c]\n"
        "mfv      %0, S000\n"
        "mfv      %1, S001\n"
    : "=r"(*s), "=r"(*c): "r"(r));
}

void Swap(float *a, float *b)
{
    float n=*a;
    *a = *b;
    *b = n;
}

/* Return next power of 2 */
static int
TextureNextPow2(unsigned int w)
{
    if(w == 0)
        return 0;

    unsigned int n = 2;

    while(w > n)
        n <<= 1;

    return n;
}


static int
PixelFormatToPSPFMT(Uint32 format)
{
    switch (format) {
    case SDL_PIXELFORMAT_BGR565:
        return GU_PSM_5650;
    case SDL_PIXELFORMAT_ABGR1555:
        return GU_PSM_5551;
    case SDL_PIXELFORMAT_ABGR4444:
        return GU_PSM_4444;
    case SDL_PIXELFORMAT_ABGR8888:
        return GU_PSM_8888;
    default:
        return GU_PSM_8888;
    }
}

void
StartDrawing(SDL_Renderer * renderer)
{
    PSP_RenderData *data = (PSP_RenderData *) renderer->driverdata;
    if(data->displayListAvail)
        return;

    sceGuStart(GU_DIRECT, DisplayList);
    data->displayListAvail = SDL_TRUE;
}


int
TextureSwizzle(PSP_TextureData *psp_texture)
{
    if(psp_texture->swizzled)
        return 1;

    int bytewidth = psp_texture->textureWidth*(psp_texture->bits>>3);
    int height = psp_texture->size / bytewidth;

    int rowblocks = (bytewidth>>4);
    int rowblocksadd = (rowblocks-1)<<7;
    unsigned int blockaddress = 0;
    unsigned int *src = (unsigned int*) psp_texture->data;

    unsigned char *data = NULL;
    data = malloc(psp_texture->size);

    int j;

    for(j = 0; j < height; j++, blockaddress += 16)
    {
        unsigned int *block;

        block = (unsigned int*)&data[blockaddress];

        int i;

        for(i = 0; i < rowblocks; i++)
        {
            *block++ = *src++;
            *block++ = *src++;
            *block++ = *src++;
            *block++ = *src++;
            block += 28;
        }

        if((j & 0x7) == 0x7)
            blockaddress += rowblocksadd;
    }

    free(psp_texture->data);
    psp_texture->data = data;
    psp_texture->swizzled = SDL_TRUE;

    return 1;
}
int TextureUnswizzle(PSP_TextureData *psp_texture)
{
    if(!psp_texture->swizzled)
        return 1;

    int blockx, blocky;

    int bytewidth = psp_texture->textureWidth*(psp_texture->bits>>3);
    int height = psp_texture->size / bytewidth;

    int widthblocks = bytewidth/16;
    int heightblocks = height/8;

    int dstpitch = (bytewidth - 16)/4;
    int dstrow = bytewidth * 8;

    unsigned int *src = (unsigned int*) psp_texture->data;

    unsigned char *data = NULL;

    data = malloc(psp_texture->size);

    if(!data)
        return 0;

    sceKernelDcacheWritebackAll();

    int j;

    unsigned char *ydst = (unsigned char *)data;

    for(blocky = 0; blocky < heightblocks; ++blocky)
    {
        unsigned char *xdst = ydst;

        for(blockx = 0; blockx < widthblocks; ++blockx)
        {
            unsigned int *block;

            block = (unsigned int*)xdst;

            for(j = 0; j < 8; ++j)
            {
                *(block++) = *(src++);
                *(block++) = *(src++);
                *(block++) = *(src++);
                *(block++) = *(src++);
                block += dstpitch;
            }

            xdst += 16;
        }

        ydst += dstrow;
    }

    free(psp_texture->data);

    psp_texture->data = data;

    psp_texture->swizzled = SDL_FALSE;

    return 1;
}

static void
PSP_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
}


static int
PSP_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
/*      PSP_RenderData *renderdata = (PSP_RenderData *) renderer->driverdata; */
    PSP_TextureData* psp_texture = (PSP_TextureData*) SDL_calloc(1, sizeof(*psp_texture));

    if(!psp_texture)
        return -1;

    psp_texture->swizzled = SDL_FALSE;
    psp_texture->width = texture->w;
    psp_texture->height = texture->h;
    psp_texture->textureHeight = TextureNextPow2(texture->h);
    psp_texture->textureWidth = TextureNextPow2(texture->w);
    psp_texture->format = PixelFormatToPSPFMT(texture->format);

    switch(psp_texture->format)
    {
        case GU_PSM_5650:
        case GU_PSM_5551:
        case GU_PSM_4444:
            psp_texture->bits = 16;
            break;

        case GU_PSM_8888:
            psp_texture->bits = 32;
            break;

        default:
            return -1;
    }

    psp_texture->pitch = psp_texture->textureWidth * SDL_BYTESPERPIXEL(texture->format);
    psp_texture->size = psp_texture->textureHeight*psp_texture->pitch;
    psp_texture->data = SDL_calloc(1, psp_texture->size);

    if(!psp_texture->data)
    {
        SDL_free(psp_texture);
        return SDL_OutOfMemory();
    }
    texture->driverdata = psp_texture;

    return 0;
}

static int
PSP_SetTextureColorMod(SDL_Renderer * renderer, SDL_Texture * texture)
{
    return SDL_Unsupported();
}

void
TextureActivate(SDL_Texture * texture)
{
    PSP_TextureData *psp_texture = (PSP_TextureData *) texture->driverdata;
    int scaleMode = (texture->scaleMode == SDL_ScaleModeNearest) ? GU_NEAREST : GU_LINEAR;

    /* Swizzling is useless with small textures. */
    if (texture->w >= 16 || texture->h >= 16)
    {
        TextureSwizzle(psp_texture);
    }

    sceGuEnable(GU_TEXTURE_2D);
    sceGuTexWrap(GU_REPEAT, GU_REPEAT);
    sceGuTexMode(psp_texture->format, 0, 0, psp_texture->swizzled);
    sceGuTexFilter(scaleMode, scaleMode); /* GU_NEAREST good for tile-map */
                                          /* GU_LINEAR good for scaling */
    sceGuTexImage(0, psp_texture->textureWidth, psp_texture->textureHeight, psp_texture->textureWidth, psp_texture->data);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
}


static int
PSP_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                   const SDL_Rect * rect, const void *pixels, int pitch)
{
/*  PSP_TextureData *psp_texture = (PSP_TextureData *) texture->driverdata; */
    const Uint8 *src;
    Uint8 *dst;
    int row, length,dpitch;
    src = pixels;

    PSP_LockTexture(renderer, texture,rect,(void **)&dst, &dpitch);
    length = rect->w * SDL_BYTESPERPIXEL(texture->format);
    if (length == pitch && length == dpitch) {
        SDL_memcpy(dst, src, length*rect->h);
    } else {
        for (row = 0; row < rect->h; ++row) {
            SDL_memcpy(dst, src, length);
            src += pitch;
            dst += dpitch;
        }
    }

    sceKernelDcacheWritebackAll();
    return 0;
}

static int
PSP_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                 const SDL_Rect * rect, void **pixels, int *pitch)
{
    PSP_TextureData *psp_texture = (PSP_TextureData *) texture->driverdata;

    *pixels =
        (void *) ((Uint8 *) psp_texture->data + rect->y * psp_texture->pitch +
                  rect->x * SDL_BYTESPERPIXEL(texture->format));
    *pitch = psp_texture->pitch;
    return 0;
}

static void
PSP_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    PSP_TextureData *psp_texture = (PSP_TextureData *) texture->driverdata;
    SDL_Rect rect;

    /* We do whole texture updates, at least for now */
    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    PSP_UpdateTexture(renderer, texture, &rect, psp_texture->data, psp_texture->pitch);
}

static void
PSP_SetTextureScaleMode(SDL_Renderer * renderer, SDL_Texture * texture, SDL_ScaleMode scaleMode)
{
    /* Nothing to do because TextureActivate takes care of it */
}

static int
PSP_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture)
{
    return 0;
}

static int
PSP_QueueSetViewport(SDL_Renderer * renderer, SDL_RenderCommand *cmd)
{
    return 0;  /* nothing to do in this backend. */
}

static int
PSP_QueueDrawPoints(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FPoint * points, int count)
{
    VertV *verts = (VertV *) SDL_AllocateRenderVertices(renderer, count * sizeof (VertV), 4, &cmd->data.draw.first);
    int i;

    if (!verts) {
        return -1;
    }

    cmd->data.draw.count = count;

    for (i = 0; i < count; i++, verts++, points++) {
        verts->x = points->x;
        verts->y = points->y;
        verts->z = 0.0f;
    }

    return 0;
}

static int
PSP_QueueFillRects(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FRect * rects, int count)
{
    VertV *verts = (GLfloat *) SDL_AllocateRenderVertices(renderer, count * 2 * sizeof (VertV), 4, &cmd->data.draw.first);
    int i;

    if (!verts) {
        return -1;
    }

    cmd->data.draw.count = count;
    for (i = 0; i < count; i++, rects++) {
        const SDL_FRect *rect = &rects[i];
        verts->x = rect->x;
        verts->y = rect->y;
        verts->z = 0.0f;
        verts++;

        verts->x = rect->x + rect->w;
        verts->y = rect->y + rect->h;
        verts->z = 0.0f;
        verts++;
    }

    return 0;
}

static int
PSP_QueueCopy(SDL_Renderer * renderer, SDL_RenderCommand *cmd, SDL_Texture * texture,
             const SDL_Rect * srcrect, const SDL_FRect * dstrect)
{
    VertTV *verts;
    const float x = dstrect->x;
    const float y = dstrect->y;
    const float width = dstrect->w;
    const float height = dstrect->h;

    const float u0 = srcrect->x;
    const float v0 = srcrect->y;
    const float u1 = srcrect->x + srcrect->w;
    const float v1 = srcrect->y + srcrect->h;

    if((MathAbs(u1) - MathAbs(u0)) < 64.0f)
    {
        verts = (VertTV *) SDL_AllocateRenderVertices(renderer, 2 * sizeof (VertTV), 4, &cmd->data.draw.first);
        if (!verts) {
            return -1;
        }

        cmd->data.draw.count = 1;

        verts->u = u0;
        verts->v = v0;
        verts->x = x;
        verts->y = y;
        verts->z = 0;
        verts++;

        verts->u = u1;
        verts->v = v1;
        verts->x = x + width;
        verts->y = y + height;
        verts->z = 0;
        verts++;
    }
    else
    {
        float start, end;
        float curU = u0;
        float curX = x;
        const float endX = x + width;
        const float slice = 64.0f;
        const size_t count = SDL_ceilf(width / slice);
        size_t i;
        float ustep = (u1 - u0)/width * slice;

        if(ustep < 0.0f)
            ustep = -ustep;

        cmd->data.draw.count = count;

        verts = (VertTV *) SDL_AllocateRenderVertices(renderer, count * sizeof (VertTV), 4, &cmd->data.draw.first);
        if (!verts) {
            return -1;
        }


        for(i = 0, start = 0, end = width; i < count; i++, start += slice)
        {
            const float polyWidth = ((curX + slice) > endX) ? (endX - curX) : slice;
            const float sourceWidth = ((curU + ustep) > u1) ? (u1 - curU) : ustep;

            SDL_assert(start < end);

            verts->u = curU;
            verts->v = v0;
            verts->x = curX;
            verts->y = y;
            verts->z = 0;

            curU += sourceWidth;
            curX += polyWidth;

            verts->u = curU;
            verts->v = v1;
            verts->x = curX;
            verts->y = (y + height);
            verts->z = 0;
        }
    }

    return 0;
}

static int
PSP_QueueCopyEx(SDL_Renderer * renderer, SDL_RenderCommand *cmd, SDL_Texture * texture,
               const SDL_Rect * srcrect, const SDL_FRect * dstrect,
               const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    VertTV *verts = (VertTV *) SDL_AllocateRenderVertices(renderer, 4 * sizeof (VertTV), 4, &cmd->data.draw.first);
    const float centerx = center->x;
    const float centery = center->y;
    const float x = dstrect->x + centerx;
    const float y = dstrect->y + centery;
    const float width = dstrect->w - centerx;
    const float height = dstrect->h - centery;
    float s, c;

    float u0 = srcrect->x;
    float v0 = srcrect->y;
    float u1 = srcrect->x + srcrect->w;
    float v1 = srcrect->y + srcrect->h;


    if (!verts) {
        return -1;
    }

    cmd->data.draw.count = 1;

    MathSincos(degToRad(angle), &s, &c);

    const float cw = c * width;
    const float sw = s * width;
    const float ch = c * height;
    const float sh = s * height;

    if (flip & SDL_FLIP_VERTICAL) {
        Swap(&v0, &v1);
    }

    if (flip & SDL_FLIP_HORIZONTAL) {
        Swap(&u0, &u1);
    }

    verts->u = u0;
    verts->v = v0;
    verts->x = x - cw + sh;
    verts->y = y - sw - ch;
    verts->z = 0;
    verts++;

    verts->u = u0;
    verts->v = v1;
    verts->x = x - cw - sh;
    verts->y = y - sw + ch;
    verts->z = 0;
    verts++;

    verts->u = u1;
    verts->v = v1;
    verts->x = x + cw - sh;
    verts->y = y + sw + ch;
    verts->z = 0;
    verts++;

    verts->u = u1;
    verts->v = v0;
    verts->x = x + cw + sh;
    verts->y = y + sw - ch;
    verts->z = 0;
    verts++;

    return 0;
}

static void
PSP_SetBlendMode(SDL_Renderer * renderer, int blendMode)
{
    PSP_RenderData *data = (PSP_RenderData *) renderer->driverdata;
    if (blendMode != data-> currentBlendMode) {
        switch (blendMode) {
        case SDL_BLENDMODE_NONE:
                sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
                sceGuDisable(GU_BLEND);
            break;
        case SDL_BLENDMODE_BLEND:
                sceGuTexFunc(GU_TFX_MODULATE , GU_TCC_RGBA);
                sceGuEnable(GU_BLEND);
                sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
            break;
        case SDL_BLENDMODE_ADD:
                sceGuTexFunc(GU_TFX_MODULATE , GU_TCC_RGBA);
                sceGuEnable(GU_BLEND);
                sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_FIX, 0, 0x00FFFFFF );
            break;
        case SDL_BLENDMODE_MOD:
                sceGuTexFunc(GU_TFX_MODULATE , GU_TCC_RGBA);
                sceGuEnable(GU_BLEND);
                sceGuBlendFunc(GU_ADD, GU_FIX, GU_SRC_COLOR, 0, 0);
            break;
        case SDL_BLENDMODE_MUL:
                sceGuTexFunc(GU_TFX_MODULATE , GU_TCC_RGBA);
                sceGuEnable(GU_BLEND);
                sceGuBlendFunc(GU_ADD, GU_DST_COLOR, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
            break;
        }
        data->currentBlendMode = blendMode;
    }
}

static int
PSP_RunCommandQueue(SDL_Renderer * renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    PSP_RenderData *data = (PSP_RenderData *) renderer->driverdata;
    size_t i;

    StartDrawing(renderer);

    /* note that before the renderer interface change, this would do extrememly small
       batches with sceGuGetMemory()--a few vertices at a time--and it's not clear that
       this won't fail if you try to push 100,000 draw calls in a single batch.
       I don't know what the limits on PSP hardware are. It might be useful to have
       rendering backends report a reasonable maximum, so the higher level can flush
       if we appear to be exceeding that. */
    Uint8 *gpumem = (Uint8 *) sceGuGetMemory(vertsize);
    if (!gpumem) {
        return SDL_SetError("Couldn't obtain a %d-byte vertex buffer!", (int) vertsize);
    }
    SDL_memcpy(gpumem, vertices, vertsize);

    while (cmd) {
        switch (cmd->command) {
            case SDL_RENDERCMD_SETDRAWCOLOR: {
                break;  /* !!! FIXME: we could cache drawstate like color */
            }

            case SDL_RENDERCMD_SETVIEWPORT: {
                SDL_Rect *viewport = &data->drawstate.viewport;
                if (SDL_memcmp(viewport, &cmd->data.viewport.rect, sizeof (SDL_Rect)) != 0) {
                    SDL_memcpy(viewport, &cmd->data.viewport.rect, sizeof (SDL_Rect));
                    data->drawstate.viewport_dirty = SDL_TRUE;
                }
                break;
            }

            case SDL_RENDERCMD_SETCLIPRECT: {
                const SDL_Rect *rect = &cmd->data.cliprect.rect;
                if (data->drawstate.cliprect_enabled != cmd->data.cliprect.enabled) {
                    data->drawstate.cliprect_enabled = cmd->data.cliprect.enabled;
                    data->drawstate.cliprect_enabled_dirty = SDL_TRUE;
                }
                if (SDL_memcmp(&data->drawstate.cliprect, rect, sizeof (SDL_Rect)) != 0) {
                    SDL_memcpy(&data->drawstate.cliprect, rect, sizeof (SDL_Rect));
                    data->drawstate.cliprect_dirty = SDL_TRUE;
                }
                break;
            }

            case SDL_RENDERCMD_CLEAR: {
                const Uint8 r = cmd->data.color.r;
                const Uint8 g = cmd->data.color.g;
                const Uint8 b = cmd->data.color.b;
                const Uint8 a = cmd->data.color.a;
                const Uint32 color = ((a << 24) | (b << 16) | (g << 8) | r);
                /* !!! FIXME: we could cache drawstate like clear color */
                sceGuClearColor(color);
                sceGuClearDepth(0);
                sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT|GU_FAST_CLEAR_BIT);
                break;
            }

            case SDL_RENDERCMD_DRAW_POINTS: {
                const size_t count = cmd->data.draw.count;
                const VertV *verts = (VertV *) (gpumem + cmd->data.draw.first);
                const Uint8 r = cmd->data.draw.r;
                const Uint8 g = cmd->data.draw.g;
                const Uint8 b = cmd->data.draw.b;
                const Uint8 a = cmd->data.draw.a;
                const Uint32 color = ((a << 24) | (b << 16) | (g << 8) | r);
                /* !!! FIXME: we could cache draw state like color, texturing, etc */
                sceGuColor(color);
                sceGuDisable(GU_TEXTURE_2D);
                sceGuShadeModel(GU_FLAT);
                sceGuDrawArray(GU_POINTS, GU_VERTEX_32BITF|GU_TRANSFORM_2D, count, 0, verts);
                sceGuShadeModel(GU_SMOOTH);
                sceGuEnable(GU_TEXTURE_2D);
                break;
            }

            case SDL_RENDERCMD_DRAW_LINES: {
                const size_t count = cmd->data.draw.count;
                const VertV *verts = (VertV *) (gpumem + cmd->data.draw.first);
                const Uint8 r = cmd->data.draw.r;
                const Uint8 g = cmd->data.draw.g;
                const Uint8 b = cmd->data.draw.b;
                const Uint8 a = cmd->data.draw.a;
                const Uint32 color = ((a << 24) | (b << 16) | (g << 8) | r);
                /* !!! FIXME: we could cache draw state like color, texturing, etc */
                sceGuColor(color);
                sceGuDisable(GU_TEXTURE_2D);
                sceGuShadeModel(GU_FLAT);
                sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF|GU_TRANSFORM_2D, count, 0, verts);
                sceGuShadeModel(GU_SMOOTH);
                sceGuEnable(GU_TEXTURE_2D);
                break;
            }

            case SDL_RENDERCMD_FILL_RECTS: {
                const size_t count = cmd->data.draw.count;
                const VertV *verts = (VertV *) (gpumem + cmd->data.draw.first);
                const Uint8 r = cmd->data.draw.r;
                const Uint8 g = cmd->data.draw.g;
                const Uint8 b = cmd->data.draw.b;
                const Uint8 a = cmd->data.draw.a;
                const Uint32 color = ((a << 24) | (b << 16) | (g << 8) | r);
                /* !!! FIXME: we could cache draw state like color, texturing, etc */
                sceGuColor(color);
                sceGuDisable(GU_TEXTURE_2D);
                sceGuShadeModel(GU_FLAT);
                sceGuDrawArray(GU_SPRITES, GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2 * count, 0, verts);
                sceGuShadeModel(GU_SMOOTH);
                sceGuEnable(GU_TEXTURE_2D);
                break;
            }

            case SDL_RENDERCMD_COPY: {
                const size_t count = cmd->data.draw.count;
                const VertTV *verts = (VertTV *) (gpumem + cmd->data.draw.first);
                const Uint8 alpha = cmd->data.draw.a;
                TextureActivate(cmd->data.draw.texture);
                PSP_SetBlendMode(renderer, cmd->data.draw.blend);

                if(alpha != 255) {  /* !!! FIXME: is this right? */
                    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
                    sceGuColor(GU_RGBA(255, 255, 255, alpha));
                } else {
                    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
                    sceGuColor(0xFFFFFFFF);
                }

                sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2 * count, 0, verts);

                if(alpha != 255) {
                    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
                }
                break;
            }

            case SDL_RENDERCMD_COPY_EX: {
                const VertTV *verts = (VertTV *) (gpumem + cmd->data.draw.first);
                const Uint8 alpha = cmd->data.draw.a;
                TextureActivate(cmd->data.draw.texture);
                PSP_SetBlendMode(renderer, cmd->data.draw.blend);

                if(alpha != 255) {  /* !!! FIXME: is this right? */
                    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
                    sceGuColor(GU_RGBA(255, 255, 255, alpha));
                } else {
                    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
                    sceGuColor(0xFFFFFFFF);
                }

                sceGuDrawArray(GU_TRIANGLE_FAN, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, verts);

                if(alpha != 255) {
                    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
                }
                break;
            }

            case SDL_RENDERCMD_NO_OP:
                break;
        }

        cmd = cmd->next;
    }

    return 0;
}

static int
PSP_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                    Uint32 pixel_format, void * pixels, int pitch)
{
    return SDL_Unsupported();
}

static void
PSP_RenderPresent(SDL_Renderer * renderer)
{
    PSP_RenderData *data = (PSP_RenderData *) renderer->driverdata;
    if(!data->displayListAvail)
        return;

    data->displayListAvail = SDL_FALSE;
    sceGuFinish();
    sceGuSync(0,0);

/*  if(data->vsync) */
        sceDisplayWaitVblankStart();

    data->backbuffer = data->frontbuffer;
    data->frontbuffer = vabsptr(sceGuSwapBuffers());

}

static void
PSP_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    PSP_RenderData *renderdata = (PSP_RenderData *) renderer->driverdata;
    PSP_TextureData *psp_texture = (PSP_TextureData *) texture->driverdata;

    if (renderdata == 0)
        return;

    if(psp_texture == 0)
        return;

    SDL_free(psp_texture->data);
    SDL_free(psp_texture);
    texture->driverdata = NULL;
}

static void
PSP_DestroyRenderer(SDL_Renderer * renderer)
{
    PSP_RenderData *data = (PSP_RenderData *) renderer->driverdata;
    if (data) {
        if (!data->initialized)
            return;

        StartDrawing(renderer);

        sceGuTerm();
/*      vfree(data->backbuffer); */
/*      vfree(data->frontbuffer); */

        data->initialized = SDL_FALSE;
        data->displayListAvail = SDL_FALSE;
        SDL_free(data);
    }
    SDL_free(renderer);
}

SDL_Renderer *
PSP_CreateRenderer(SDL_Window * window, Uint32 flags)
{

    SDL_Renderer *renderer;
    PSP_RenderData *data;
        int pixelformat;
    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (PSP_RenderData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        PSP_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }


    renderer->WindowEvent = PSP_WindowEvent;
    renderer->CreateTexture = PSP_CreateTexture;
    renderer->SetTextureColorMod = PSP_SetTextureColorMod;
    renderer->UpdateTexture = PSP_UpdateTexture;
    renderer->LockTexture = PSP_LockTexture;
    renderer->UnlockTexture = PSP_UnlockTexture;
    renderer->SetTextureScaleMode = PSP_SetTextureScaleMode;
    renderer->SetRenderTarget = PSP_SetRenderTarget;
    renderer->QueueSetViewport = PSP_QueueSetViewport;
    renderer->QueueSetDrawColor = PSP_QueueSetViewport;  /* SetViewport and SetDrawColor are (currently) no-ops. */
    renderer->QueueDrawPoints = PSP_QueueDrawPoints;
    renderer->QueueDrawLines = PSP_QueueDrawPoints;  /* lines and points queue vertices the same way. */
    renderer->QueueFillRects = PSP_QueueFillRects;
    renderer->QueueCopy = PSP_QueueCopy;
    renderer->QueueCopyEx = PSP_QueueCopyEx;
    renderer->RunCommandQueue = PSP_RunCommandQueue;
    renderer->RenderReadPixels = PSP_RenderReadPixels;
    renderer->RenderPresent = PSP_RenderPresent;
    renderer->DestroyTexture = PSP_DestroyTexture;
    renderer->DestroyRenderer = PSP_DestroyRenderer;
    renderer->info = PSP_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;
    renderer->window = window;

    if (data->initialized != SDL_FALSE)
        return 0;
    data->initialized = SDL_TRUE;

    if (flags & SDL_RENDERER_PRESENTVSYNC) {
        data->vsync = SDL_TRUE;
    } else {
        data->vsync = SDL_FALSE;
    }

    pixelformat=PixelFormatToPSPFMT(SDL_GetWindowPixelFormat(window));
    switch(pixelformat)
    {
        case GU_PSM_4444:
        case GU_PSM_5650:
        case GU_PSM_5551:
            data->frontbuffer = (unsigned int *)(PSP_FRAME_BUFFER_SIZE<<1);
            data->backbuffer =  (unsigned int *)(0);
            data->bpp = 2;
            data->psm = pixelformat;
            break;
        default:
            data->frontbuffer = (unsigned int *)(PSP_FRAME_BUFFER_SIZE<<2);
            data->backbuffer =  (unsigned int *)(0);
            data->bpp = 4;
            data->psm = GU_PSM_8888;
            break;
    }

    sceGuInit();
    /* setup GU */
    sceGuStart(GU_DIRECT, DisplayList);
    sceGuDrawBuffer(data->psm, data->frontbuffer, PSP_FRAME_BUFFER_WIDTH);
    sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, data->backbuffer, PSP_FRAME_BUFFER_WIDTH);


    sceGuOffset(2048 - (PSP_SCREEN_WIDTH>>1), 2048 - (PSP_SCREEN_HEIGHT>>1));
    sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);

    data->frontbuffer = vabsptr(data->frontbuffer);
    data->backbuffer = vabsptr(data->backbuffer);

    /* Scissoring */
    sceGuScissor(0, 0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);

    /* Backface culling */
    sceGuFrontFace(GU_CCW);
    sceGuEnable(GU_CULL_FACE);

    /* Texturing */
    sceGuEnable(GU_TEXTURE_2D);
    sceGuShadeModel(GU_SMOOTH);
    sceGuTexWrap(GU_REPEAT, GU_REPEAT);

    /* Blending */
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

    sceGuTexFilter(GU_LINEAR,GU_LINEAR);

    sceGuFinish();
    sceGuSync(0,0);
    sceDisplayWaitVblankStartCB();
    sceGuDisplay(GU_TRUE);

    return renderer;
}

SDL_RenderDriver PSP_RenderDriver = {
    .CreateRenderer = PSP_CreateRenderer,
    .info = {
        .name = "PSP",
        .flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE,
        .num_texture_formats = 4,
        .texture_formats = { [0] = SDL_PIXELFORMAT_BGR565,
                                                 [1] = SDL_PIXELFORMAT_ABGR1555,
                                                 [2] = SDL_PIXELFORMAT_ABGR4444,
                                                 [3] = SDL_PIXELFORMAT_ABGR8888,
        },
        .max_texture_width = 512,
        .max_texture_height = 512,
     }
};

#endif /* SDL_VIDEO_RENDER_PSP */

/* vi: set ts=4 sw=4 expandtab: */

