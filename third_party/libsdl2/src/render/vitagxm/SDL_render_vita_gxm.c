/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_RENDER_VITA_GXM

#include "SDL_hints.h"
#include "../SDL_sysrender.h"
#include "SDL_log.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "SDL_render_vita_gxm_types.h"
#include "SDL_render_vita_gxm_tools.h"
#include "SDL_render_vita_gxm_memory.h"

#include <psp2/common_dialog.h>

/* #define DEBUG_RAZOR */

#if DEBUG_RAZOR
#include <psp2/sysmodule.h>
#endif

static SDL_Renderer *VITA_GXM_CreateRenderer(SDL_Window *window, Uint32 flags);

static void VITA_GXM_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event);

static SDL_bool VITA_GXM_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode);

static int VITA_GXM_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture);

static int VITA_GXM_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture,
    const SDL_Rect *rect, const void *pixels, int pitch);

static int VITA_GXM_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
    const SDL_Rect * rect,
    const Uint8 *Yplane, int Ypitch,
    const Uint8 *Uplane, int Upitch,
    const Uint8 *Vplane, int Vpitch);

static int VITA_GXM_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture,
    const SDL_Rect *rect, void **pixels, int *pitch);

static void VITA_GXM_UnlockTexture(SDL_Renderer *renderer,
    SDL_Texture *texture);

static void VITA_GXM_SetTextureScaleMode(SDL_Renderer * renderer, SDL_Texture * texture, SDL_ScaleMode scaleMode);

static int VITA_GXM_SetRenderTarget(SDL_Renderer *renderer,
    SDL_Texture *texture);


static int VITA_GXM_QueueSetViewport(SDL_Renderer * renderer, SDL_RenderCommand *cmd);

static int VITA_GXM_QueueSetDrawColor(SDL_Renderer * renderer, SDL_RenderCommand *cmd);


static int VITA_GXM_QueueDrawPoints(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FPoint * points, int count);
static int VITA_GXM_QueueDrawLines(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FPoint * points, int count);

static int VITA_GXM_QueueCopy(SDL_Renderer * renderer, SDL_RenderCommand *cmd, SDL_Texture * texture,
    const SDL_Rect * srcrect, const SDL_FRect * dstrect);

static int VITA_GXM_QueueCopyEx(SDL_Renderer * renderer, SDL_RenderCommand *cmd, SDL_Texture * texture,
    const SDL_Rect * srcrect, const SDL_FRect * dstrect,
    const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip);

static int VITA_GXM_RenderClear(SDL_Renderer *renderer, SDL_RenderCommand *cmd);

static int VITA_GXM_RenderDrawPoints(SDL_Renderer *renderer, const SDL_RenderCommand *cmd);

static int VITA_GXM_RenderDrawLines(SDL_Renderer *renderer, const SDL_RenderCommand *cmd);

static int VITA_GXM_QueueFillRects(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FRect * rects, int count);

static int VITA_GXM_RenderFillRects(SDL_Renderer *renderer, const SDL_RenderCommand *cmd);


static int VITA_GXM_RunCommandQueue(SDL_Renderer * renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize);

static int VITA_GXM_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect,
    Uint32 pixel_format, void *pixels, int pitch);


static void VITA_GXM_RenderPresent(SDL_Renderer *renderer);
static void VITA_GXM_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture);
static void VITA_GXM_DestroyRenderer(SDL_Renderer *renderer);


SDL_RenderDriver VITA_GXM_RenderDriver = {
    .CreateRenderer = VITA_GXM_CreateRenderer,
    .info = {
        .name = "VITA gxm",
        .flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE,
        .num_texture_formats = 4,
        .texture_formats = {
            [0] = SDL_PIXELFORMAT_ABGR8888,
            [1] = SDL_PIXELFORMAT_ARGB8888,
            [2] = SDL_PIXELFORMAT_RGB565,
            [3] = SDL_PIXELFORMAT_BGR565
        },
        .max_texture_width = 4096,
        .max_texture_height = 4096,
     }
};

static int
PixelFormatToVITAFMT(Uint32 format)
{
    switch (format) {
    case SDL_PIXELFORMAT_ARGB8888:
        return SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ARGB;
    case SDL_PIXELFORMAT_RGB888:
        return SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ARGB;
    case SDL_PIXELFORMAT_BGR888:
        return SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR;
    case SDL_PIXELFORMAT_ABGR8888:
        return SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR;
    case SDL_PIXELFORMAT_RGB565:
        return SCE_GXM_TEXTURE_FORMAT_U5U6U5_RGB;
    case SDL_PIXELFORMAT_BGR565:
        return SCE_GXM_TEXTURE_FORMAT_U5U6U5_BGR;
    default:
        return SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR;
    }
}

void
StartDrawing(SDL_Renderer *renderer)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;
    if (data->drawing) {
        return;
    }

    data->drawstate.texture = NULL;
    data->drawstate.vertex_program = NULL;
    data->drawstate.fragment_program = NULL;
    data->drawstate.last_command = -1;
    data->drawstate.texture_color = 0xFFFFFFFF;
    data->drawstate.viewport_dirty = SDL_TRUE;

    // reset blend mode
//    data->currentBlendMode = SDL_BLENDMODE_BLEND;
//    fragment_programs *in = &data->blendFragmentPrograms.blend_mode_blend;
//    data->colorFragmentProgram = in->color;
//    data->textureFragmentProgram = in->texture;
//    data->textureTintFragmentProgram = in->textureTint;

    if (renderer->target == NULL) {
        sceGxmBeginScene(
            data->gxm_context,
            0,
            data->renderTarget,
            NULL,
            NULL,
            data->displayBufferSync[data->backBufferIndex],
            &data->displaySurface[data->backBufferIndex],
            &data->depthSurface
        );
    } else {
        VITA_GXM_TextureData *vita_texture = (VITA_GXM_TextureData *) renderer->target->driverdata;

        sceGxmBeginScene(
            data->gxm_context,
            0,
            vita_texture->tex->gxm_rendertarget,
            NULL,
            NULL,
            NULL,
            &vita_texture->tex->gxm_colorsurface,
            &vita_texture->tex->gxm_depthstencil
        );
    }

//    unset_clip_rectangle(data);

    data->drawing = SDL_TRUE;
}

SDL_Renderer *
VITA_GXM_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer;
    VITA_GXM_RenderData *data;

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (VITA_GXM_RenderData *) SDL_calloc(1, sizeof(VITA_GXM_RenderData));
    if (!data) {
        VITA_GXM_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = VITA_GXM_WindowEvent;
    renderer->SupportsBlendMode = VITA_GXM_SupportsBlendMode;
    renderer->CreateTexture = VITA_GXM_CreateTexture;
    renderer->UpdateTexture = VITA_GXM_UpdateTexture;
    renderer->UpdateTextureYUV = VITA_GXM_UpdateTextureYUV;
    renderer->LockTexture = VITA_GXM_LockTexture;
    renderer->UnlockTexture = VITA_GXM_UnlockTexture;
    renderer->SetTextureScaleMode = VITA_GXM_SetTextureScaleMode;
    renderer->SetRenderTarget = VITA_GXM_SetRenderTarget;
    renderer->QueueSetViewport = VITA_GXM_QueueSetViewport;
    renderer->QueueSetDrawColor = VITA_GXM_QueueSetDrawColor;
    renderer->QueueDrawPoints = VITA_GXM_QueueDrawPoints;
    renderer->QueueDrawLines = VITA_GXM_QueueDrawLines;
    renderer->QueueFillRects = VITA_GXM_QueueFillRects;
    renderer->QueueCopy = VITA_GXM_QueueCopy;
    renderer->QueueCopyEx = VITA_GXM_QueueCopyEx;
    renderer->RunCommandQueue = VITA_GXM_RunCommandQueue;
    renderer->RenderReadPixels = VITA_GXM_RenderReadPixels;
    renderer->RenderPresent = VITA_GXM_RenderPresent;
    renderer->DestroyTexture = VITA_GXM_DestroyTexture;
    renderer->DestroyRenderer = VITA_GXM_DestroyRenderer;

    renderer->info = VITA_GXM_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;
    renderer->window = window;

    if (data->initialized != SDL_FALSE)
        return 0;
    data->initialized = SDL_TRUE;

    if (flags & SDL_RENDERER_PRESENTVSYNC) {
        data->displayData.wait_vblank = SDL_TRUE;
    } else {
        data->displayData.wait_vblank = SDL_FALSE;
    }

#if DEBUG_RAZOR
    sceSysmoduleLoadModule( SCE_SYSMODULE_RAZOR_HUD );
    sceSysmoduleLoadModule( SCE_SYSMODULE_RAZOR_CAPTURE );
#endif

    if (gxm_init(renderer) != 0)
    {
        return NULL;
    }

    return renderer;
}

static void
VITA_GXM_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event)
{
}

static SDL_bool
VITA_GXM_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode)
{
    // only for custom modes. we build all modes on init, so no custom modes, sorry
    return SDL_FALSE;
}

static int
VITA_GXM_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;
    VITA_GXM_TextureData* vita_texture = (VITA_GXM_TextureData*) SDL_calloc(1, sizeof(VITA_GXM_TextureData));

    if (!vita_texture) {
        return SDL_OutOfMemory();
    }

    vita_texture->tex = create_gxm_texture(data, texture->w, texture->h, PixelFormatToVITAFMT(texture->format), (texture->access == SDL_TEXTUREACCESS_TARGET));

    if (!vita_texture->tex) {
        SDL_free(vita_texture);
        return SDL_OutOfMemory();
    }

    texture->driverdata = vita_texture;

    VITA_GXM_SetTextureScaleMode(renderer, texture, texture->scaleMode);

    vita_texture->w = gxm_texture_get_width(vita_texture->tex);
    vita_texture->h = gxm_texture_get_height(vita_texture->tex);
    vita_texture->pitch = gxm_texture_get_stride(vita_texture->tex);

    return 0;
}


static int
VITA_GXM_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture,
    const SDL_Rect *rect, const void *pixels, int pitch)
{
    const Uint8 *src;
    Uint8 *dst;
    int row, length,dpitch;
    src = pixels;

    VITA_GXM_LockTexture(renderer, texture, rect, (void **)&dst, &dpitch);
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

    return 0;
}

static int
VITA_GXM_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
    const SDL_Rect * rect,
    const Uint8 *Yplane, int Ypitch,
    const Uint8 *Uplane, int Upitch,
    const Uint8 *Vplane, int Vpitch)
{
    return 0;
}

static int
VITA_GXM_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture,
    const SDL_Rect *rect, void **pixels, int *pitch)
{
    VITA_GXM_TextureData *vita_texture = (VITA_GXM_TextureData *) texture->driverdata;

    *pixels =
        (void *) ((Uint8 *) gxm_texture_get_datap(vita_texture->tex)
            + (rect->y * vita_texture->pitch) + rect->x * SDL_BYTESPERPIXEL(texture->format));
    *pitch = vita_texture->pitch;
    return 0;
}

static void
VITA_GXM_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    // No need to update texture data on ps vita.
    // VITA_GXM_LockTexture already returns a pointer to the texture pixels buffer.
    // This really improves framerate when using lock/unlock.
}

static void
VITA_GXM_SetTextureScaleMode(SDL_Renderer * renderer, SDL_Texture * texture, SDL_ScaleMode scaleMode)
{
    VITA_GXM_TextureData *vita_texture = (VITA_GXM_TextureData *) texture->driverdata;

    /*
     set texture filtering according to scaleMode
     suported hint values are nearest (0, default) or linear (1)
     vitaScaleMode is either SCE_GXM_TEXTURE_FILTER_POINT (good for tile-map)
     or SCE_GXM_TEXTURE_FILTER_LINEAR (good for scaling)
     */

    int vitaScaleMode = (scaleMode == SDL_ScaleModeNearest
                        ? SCE_GXM_TEXTURE_FILTER_POINT
                        : SCE_GXM_TEXTURE_FILTER_LINEAR);
    gxm_texture_set_filters(vita_texture->tex, vitaScaleMode, vitaScaleMode);

    return;
}

static int
VITA_GXM_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture)
{
    return 0;
}

static void
VITA_GXM_SetBlendMode(VITA_GXM_RenderData *data, int blendMode)
{
    if (blendMode != data->currentBlendMode)
    {
        fragment_programs *in = &data->blendFragmentPrograms.blend_mode_blend;

        switch (blendMode)
        {
            case SDL_BLENDMODE_NONE:
                in = &data->blendFragmentPrograms.blend_mode_none;
                break;
            case SDL_BLENDMODE_BLEND:
                in = &data->blendFragmentPrograms.blend_mode_blend;
                break;
            case SDL_BLENDMODE_ADD:
                in = &data->blendFragmentPrograms.blend_mode_add;
                break;
            case SDL_BLENDMODE_MOD:
                in = &data->blendFragmentPrograms.blend_mode_mod;
                break;
            case SDL_BLENDMODE_MUL:
                in = &data->blendFragmentPrograms.blend_mode_mul;
                break;
        }
        data->colorFragmentProgram = in->color;
        data->textureFragmentProgram = in->texture;
        data->textureTintFragmentProgram = in->textureTint;
        data->currentBlendMode = blendMode;
    }
}

static int
VITA_GXM_QueueSetViewport(SDL_Renderer * renderer, SDL_RenderCommand *cmd)
{
    return 0;
}

static int
VITA_GXM_QueueSetDrawColor(SDL_Renderer * renderer, SDL_RenderCommand *cmd)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    const Uint8 r = cmd->data.color.r;
    const Uint8 g = cmd->data.color.g;
    const Uint8 b = cmd->data.color.b;
    const Uint8 a = cmd->data.color.a;
    data->drawstate.color = ((a << 24) | (b << 16) | (g << 8) | r);

    return 0;
}

static int
VITA_GXM_QueueDrawPoints(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FPoint * points, int count)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    int color = data->drawstate.color;

    color_vertex *vertex = (color_vertex *)pool_memalign(
        data,
        count * sizeof(color_vertex),
        sizeof(color_vertex)
    );

    cmd->data.draw.first = (size_t)vertex;
    cmd->data.draw.count = count;

    for (int i = 0; i < count; i++)
    {
        vertex[i].x = points[i].x;
        vertex[i].y = points[i].y;
        vertex[i].z = +0.5f;
        vertex[i].color = color;
    }

    return 0;
}

static int
VITA_GXM_QueueDrawLines(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FPoint * points, int count)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;
    int color = data->drawstate.color;

    color_vertex *vertex = (color_vertex *)pool_memalign(
        data,
        (count-1) * 2 * sizeof(color_vertex),
        sizeof(color_vertex)
    );

    cmd->data.draw.first = (size_t)vertex;
    cmd->data.draw.count = (count-1) * 2;

    for (int i = 0; i < count - 1; i++)
    {
        vertex[i*2].x = points[i].x;
        vertex[i*2].y = points[i].y;
        vertex[i*2].z = +0.5f;
        vertex[i*2].color = color;

        vertex[i*2+1].x = points[i+1].x;
        vertex[i*2+1].y = points[i+1].y;
        vertex[i*2+1].z = +0.5f;
        vertex[i*2+1].color = color;
    }

    return 0;
}

static int
VITA_GXM_QueueFillRects(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FRect * rects, int count)
{
    int color;
    color_vertex *vertices;
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    cmd->data.draw.count = count;
    color = data->drawstate.color;

    vertices = (color_vertex *)pool_memalign(
            data,
            4 * count * sizeof(color_vertex), // 4 vertices * count
            sizeof(color_vertex));

    for (int i =0; i < count; i++)
    {
        const SDL_FRect *rect = &rects[i];

        vertices[4*i+0].x = rect->x;
        vertices[4*i+0].y = rect->y;
        vertices[4*i+0].z = +0.5f;
        vertices[4*i+0].color = color;

        vertices[4*i+1].x = rect->x + rect->w;
        vertices[4*i+1].y = rect->y;
        vertices[4*i+1].z = +0.5f;
        vertices[4*i+1].color = color;

        vertices[4*i+2].x = rect->x;
        vertices[4*i+2].y = rect->y + rect->h;
        vertices[4*i+2].z = +0.5f;
        vertices[4*i+2].color = color;

        vertices[4*i+3].x = rect->x + rect->w;
        vertices[4*i+3].y = rect->y + rect->h;
        vertices[4*i+3].z = +0.5f;
        vertices[4*i+3].color = color;
    }

    cmd->data.draw.first = (size_t)vertices;

    return 0;
}

#define degToRad(x) ((x)*M_PI/180.f)

void MathSincos(float r, float *s, float *c)
{
    *s = SDL_sin(r);
    *c = SDL_cos(r);
}

static int
VITA_GXM_QueueCopy(SDL_Renderer * renderer, SDL_RenderCommand *cmd, SDL_Texture * texture,
    const SDL_Rect * srcrect, const SDL_FRect * dstrect)
{
    texture_vertex *vertices;
    float u0, v0, u1, v1;
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    cmd->data.draw.count = 1;

    vertices = (texture_vertex *)pool_memalign(
            data,
            4 * sizeof(texture_vertex), // 4 vertices
            sizeof(texture_vertex));

    cmd->data.draw.first = (size_t)vertices;
    cmd->data.draw.texture = texture;

    u0 = (float)srcrect->x / (float)texture->w;
    v0 = (float)srcrect->y / (float)texture->h;
    u1 = (float)(srcrect->x + srcrect->w) / (float)texture->w;
    v1 = (float)(srcrect->y + srcrect->h) / (float)texture->h;

    vertices[0].x = dstrect->x;
    vertices[0].y = dstrect->y;
    vertices[0].z = +0.5f;
    vertices[0].u = u0;
    vertices[0].v = v0;

    vertices[1].x = dstrect->x + dstrect->w;
    vertices[1].y = dstrect->y;
    vertices[1].z = +0.5f;
    vertices[1].u = u1;
    vertices[1].v = v0;

    vertices[2].x = dstrect->x;
    vertices[2].y = dstrect->y + dstrect->h;
    vertices[2].z = +0.5f;
    vertices[2].u = u0;
    vertices[2].v = v1;

    vertices[3].x = dstrect->x + dstrect->w;
    vertices[3].y = dstrect->y + dstrect->h;
    vertices[3].z = +0.5f;
    vertices[3].u = u1;
    vertices[3].v = v1;

    return 0;
}

static int
VITA_GXM_QueueCopyEx(SDL_Renderer * renderer, SDL_RenderCommand *cmd, SDL_Texture * texture,
    const SDL_Rect * srcrect, const SDL_FRect * dstrect,
    const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    texture_vertex *vertices;
    float u0, v0, u1, v1;
    float x0, y0, x1, y1;
    float s, c;
    const float centerx = center->x + dstrect->x;
    const float centery = center->y + dstrect->y;

    cmd->data.draw.count = 1;

    vertices = (texture_vertex *)pool_memalign(
            data,
            4 * sizeof(texture_vertex), // 4 vertices
            sizeof(texture_vertex));

    cmd->data.draw.first = (size_t)vertices;
    cmd->data.draw.texture = texture;

    if (flip & SDL_FLIP_HORIZONTAL) {
        x0 = dstrect->x + dstrect->w;
        x1 = dstrect->x;
    } else {
        x0 = dstrect->x;
        x1 = dstrect->x + dstrect->w;
    }

    if (flip & SDL_FLIP_VERTICAL) {
        y0 = dstrect->y + dstrect->h;
        y1 = dstrect->y;
    } else {
        y0 = dstrect->y;
        y1 = dstrect->y + dstrect->h;
    }

    u0 = (float)srcrect->x / (float)texture->w;
    v0 = (float)srcrect->y / (float)texture->h;
    u1 = (float)(srcrect->x + srcrect->w) / (float)texture->w;
    v1 = (float)(srcrect->y + srcrect->h) / (float)texture->h;

    MathSincos(degToRad(angle), &s, &c);

    vertices[0].x = x0 - centerx;
    vertices[0].y = y0 - centery;
    vertices[0].z = +0.5f;
    vertices[0].u = u0;
    vertices[0].v = v0;

    vertices[1].x = x1 - centerx;
    vertices[1].y = y0 - centery;
    vertices[1].z = +0.5f;
    vertices[1].u = u1;
    vertices[1].v = v0;


    vertices[2].x = x0 - centerx;
    vertices[2].y = y1 - centery;
    vertices[2].z = +0.5f;
    vertices[2].u = u0;
    vertices[2].v = v1;

    vertices[3].x = x1 - centerx;
    vertices[3].y = y1 - centery;
    vertices[3].z = +0.5f;
    vertices[3].u = u1;
    vertices[3].v = v1;

    for (int i = 0; i < 4; ++i)
    {
        float _x = vertices[i].x;
        float _y = vertices[i].y;
        vertices[i].x = _x * c - _y * s + centerx;
        vertices[i].y = _x * s + _y * c + centery;
    }

    return 0;
}


static int
VITA_GXM_RenderClear(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    void *color_buffer;
    float clear_color[4];

    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;
    unset_clip_rectangle(data);

    clear_color[0] = (cmd->data.color.r)/255.0f;
    clear_color[1] = (cmd->data.color.g)/255.0f;
    clear_color[2] = (cmd->data.color.b)/255.0f;
    clear_color[3] = (cmd->data.color.a)/255.0f;

    // set clear shaders
    data->drawstate.fragment_program = data->clearFragmentProgram;
    data->drawstate.vertex_program = data->clearVertexProgram;
    sceGxmSetVertexProgram(data->gxm_context, data->clearVertexProgram);
    sceGxmSetFragmentProgram(data->gxm_context, data->clearFragmentProgram);

    // set the clear color
    sceGxmReserveFragmentDefaultUniformBuffer(data->gxm_context, &color_buffer);
    sceGxmSetUniformDataF(color_buffer, data->clearClearColorParam, 0, 4, clear_color);

    // draw the clear triangle
    sceGxmSetVertexStream(data->gxm_context, 0, data->clearVertices);
    sceGxmDraw(data->gxm_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, data->linearIndices, 3);

    data->drawstate.cliprect_dirty = SDL_TRUE;
    return 0;
}


static int
VITA_GXM_RenderDrawPoints(SDL_Renderer *renderer, const SDL_RenderCommand *cmd)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    sceGxmSetFrontPolygonMode(data->gxm_context, SCE_GXM_POLYGON_MODE_POINT);
    sceGxmDraw(data->gxm_context, SCE_GXM_PRIMITIVE_POINTS, SCE_GXM_INDEX_FORMAT_U16, data->linearIndices, cmd->data.draw.count);
    sceGxmSetFrontPolygonMode(data->gxm_context, SCE_GXM_POLYGON_MODE_TRIANGLE_FILL);

    return 0;
}

static int
VITA_GXM_RenderDrawLines(SDL_Renderer *renderer, const SDL_RenderCommand *cmd)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    sceGxmSetFrontPolygonMode(data->gxm_context, SCE_GXM_POLYGON_MODE_LINE);
    sceGxmDraw(data->gxm_context, SCE_GXM_PRIMITIVE_LINES, SCE_GXM_INDEX_FORMAT_U16, data->linearIndices, cmd->data.draw.count);
    sceGxmSetFrontPolygonMode(data->gxm_context, SCE_GXM_POLYGON_MODE_TRIANGLE_FILL);
    return 0;
}


static int
VITA_GXM_RenderFillRects(SDL_Renderer *renderer, const SDL_RenderCommand *cmd)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    sceGxmSetVertexStream(data->gxm_context, 0, (const void*)cmd->data.draw.first);
    sceGxmDraw(data->gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, data->linearIndices, 4 * cmd->data.draw.count);

    return 0;
}


static int
SetDrawState(VITA_GXM_RenderData *data, const SDL_RenderCommand *cmd, SDL_bool solid)
{
    SDL_Texture *texture = cmd->data.draw.texture;
    const SDL_BlendMode blend = cmd->data.draw.blend;
    SceGxmFragmentProgram *fragment_program;
    SceGxmVertexProgram *vertex_program;
    SDL_bool matrix_updated = SDL_FALSE;
    SDL_bool program_updated = SDL_FALSE;
    Uint32 texture_color;

    Uint8 r, g, b, a;
    r = cmd->data.draw.r;
    g = cmd->data.draw.g;
    b = cmd->data.draw.b;
    a = cmd->data.draw.a;

    if (data->drawstate.viewport_dirty) {
        const SDL_Rect *viewport = &data->drawstate.viewport;


        float sw = viewport->w  / 2.;
        float sh = viewport->h / 2.;

        float x_scale = sw;
        float x_off = viewport->x + sw;
        float y_scale = -(sh);
        float y_off = viewport->y + sh;

        sceGxmSetViewport(data->gxm_context, x_off, x_scale, y_off, y_scale, 0.5f, 0.5f);

        if (viewport->w && viewport->h) {
            init_orthographic_matrix(data->ortho_matrix,
                (float) 0,
                (float) viewport->w,
                (float) viewport->h,
                (float) 0,
                0.0f, 1.0f);
            matrix_updated = SDL_TRUE;
        }

        data->drawstate.viewport_dirty = SDL_FALSE;
    }

    if (data->drawstate.cliprect_enabled_dirty) {
        if (!data->drawstate.cliprect_enabled) {
            unset_clip_rectangle(data);
        }
        data->drawstate.cliprect_enabled_dirty = SDL_FALSE;
    }

    if (data->drawstate.cliprect_enabled && data->drawstate.cliprect_dirty) {
        const SDL_Rect *rect = &data->drawstate.cliprect;
        set_clip_rectangle(data, rect->x, rect->y, rect->x + rect->w, rect->y + rect->h);
        data->drawstate.cliprect_dirty = SDL_FALSE;
    }

    VITA_GXM_SetBlendMode(data, blend); // do that first, to select appropriate shaders

    if (texture) {
        vertex_program = data->textureVertexProgram;
        if(cmd->data.draw.r == 255 && cmd->data.draw.g == 255 && cmd->data.draw.b == 255 && cmd->data.draw.a == 255) {
            fragment_program = data->textureFragmentProgram;
        } else {
            fragment_program = data->textureTintFragmentProgram;
        }
    } else {
        vertex_program = data->colorVertexProgram;
        fragment_program = data->colorFragmentProgram;
    }

    if (data->drawstate.vertex_program != vertex_program) {
        data->drawstate.vertex_program = vertex_program;
        sceGxmSetVertexProgram(data->gxm_context, vertex_program);
        program_updated = SDL_TRUE;
    }

    if (data->drawstate.fragment_program != fragment_program) {
        data->drawstate.fragment_program = fragment_program;
        sceGxmSetFragmentProgram(data->gxm_context, fragment_program);
        program_updated = SDL_TRUE;
    }

    texture_color = ((a << 24) | (b << 16) | (g << 8) | r);

    if (program_updated || matrix_updated) {
        if (data->drawstate.fragment_program == data->textureFragmentProgram) {
            void *vertex_wvp_buffer;
            sceGxmReserveVertexDefaultUniformBuffer(data->gxm_context, &vertex_wvp_buffer);
            sceGxmSetUniformDataF(vertex_wvp_buffer, data->textureWvpParam, 0, 16, data->ortho_matrix);
        } else if (data->drawstate.fragment_program == data->textureTintFragmentProgram) {
            void *vertex_wvp_buffer;
            void *texture_tint_color_buffer;
            float *tint_color;
            sceGxmReserveVertexDefaultUniformBuffer(data->gxm_context, &vertex_wvp_buffer);
            sceGxmSetUniformDataF(vertex_wvp_buffer, data->textureWvpParam, 0, 16, data->ortho_matrix);

            sceGxmReserveFragmentDefaultUniformBuffer(data->gxm_context, &texture_tint_color_buffer);

            tint_color = pool_memalign(
                data,
                4 * sizeof(float), // RGBA
                sizeof(float)
            );

            tint_color[0] = r / 255.0f;
            tint_color[1] = g / 255.0f;
            tint_color[2] = b / 255.0f;
            tint_color[3] = a / 255.0f;
            sceGxmSetUniformDataF(texture_tint_color_buffer, data->textureTintColorParam, 0, 4, tint_color);
            data->drawstate.texture_color = texture_color;
        } else { // color
            void *vertexDefaultBuffer;
            sceGxmReserveVertexDefaultUniformBuffer(data->gxm_context, &vertexDefaultBuffer);
            sceGxmSetUniformDataF(vertexDefaultBuffer, data->colorWvpParam, 0, 16, data->ortho_matrix);
        }
    } else {
        if (data->drawstate.fragment_program == data->textureTintFragmentProgram && data->drawstate.texture_color != texture_color) {
            void *texture_tint_color_buffer;
            float *tint_color;
            sceGxmReserveFragmentDefaultUniformBuffer(data->gxm_context, &texture_tint_color_buffer);

            tint_color = pool_memalign(
                data,
                4 * sizeof(float), // RGBA
                sizeof(float)
            );

            tint_color[0] = r / 255.0f;
            tint_color[1] = g / 255.0f;
            tint_color[2] = b / 255.0f;
            tint_color[3] = a / 255.0f;
            sceGxmSetUniformDataF(texture_tint_color_buffer, data->textureTintColorParam, 0, 4, tint_color);
            data->drawstate.texture_color = texture_color;
        }
    }

    if (texture != data->drawstate.texture) {
        if (texture) {
            VITA_GXM_TextureData *vita_texture = (VITA_GXM_TextureData *) cmd->data.draw.texture->driverdata;
            sceGxmSetFragmentTexture(data->gxm_context, 0, &vita_texture->tex->gxm_tex);
        }
        data->drawstate.texture = texture;
    }

    /* all drawing commands use this */
    sceGxmSetVertexStream(data->gxm_context, 0, (const void*)cmd->data.draw.first);

    return 0;
}

static int
SetCopyState(SDL_Renderer *renderer, const SDL_RenderCommand *cmd)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    return SetDrawState(data, cmd, SDL_FALSE);
}

static int
VITA_GXM_RunCommandQueue(SDL_Renderer * renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;
    StartDrawing(renderer);

    data->drawstate.target = renderer->target;
    if (!data->drawstate.target) {
        SDL_GL_GetDrawableSize(renderer->window, &data->drawstate.drawablew, &data->drawstate.drawableh);
    }

    while (cmd) {
        switch (cmd->command) {

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

            case SDL_RENDERCMD_SETDRAWCOLOR: {
                break;
            }

            case SDL_RENDERCMD_CLEAR: {
                VITA_GXM_RenderClear(renderer, cmd);
                break;
            }

            case SDL_RENDERCMD_DRAW_POINTS: {
                SetDrawState(data, cmd, SDL_FALSE);
                VITA_GXM_RenderDrawPoints(renderer, cmd);
                break;
            }

            case SDL_RENDERCMD_DRAW_LINES: {
                SetDrawState(data, cmd, SDL_FALSE);
                VITA_GXM_RenderDrawLines(renderer, cmd);
                break;
            }

            case SDL_RENDERCMD_FILL_RECTS: {
                SetDrawState(data, cmd, SDL_FALSE);
                VITA_GXM_RenderFillRects(renderer, cmd);
                break;
            }

            case SDL_RENDERCMD_COPY:
            case SDL_RENDERCMD_COPY_EX: {
                SetCopyState(renderer, cmd);
                sceGxmDraw(data->gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, data->linearIndices, 4 * cmd->data.draw.count);

                break;
            }

            case SDL_RENDERCMD_NO_OP:
                break;
        }
        data->drawstate.last_command = cmd->command;
        cmd = cmd->next;
    }

    return 0;
}

void read_pixels(int x, int y, size_t width, size_t height, void *data) {
    SceDisplayFrameBuf pParam;
    int i, j;
    Uint32 *out32;
    Uint32 *in32;

    pParam.size = sizeof(SceDisplayFrameBuf);

    sceDisplayGetFrameBuf(&pParam, SCE_DISPLAY_SETBUF_NEXTFRAME);

    out32 = (Uint32 *)data;
    in32 = (Uint32 *)pParam.base;

    in32 += (x + y * pParam.pitch);

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            out32[(height - (i + 1)) * width + j] = in32[j];
        }
        in32 += pParam.pitch;
    }
}


static int
VITA_GXM_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect,
    Uint32 pixel_format, void *pixels, int pitch)
{
    Uint32 temp_format = renderer->target ? renderer->target->format : SDL_PIXELFORMAT_ABGR8888;
    size_t buflen;
    void *temp_pixels;
    int temp_pitch;
    Uint8 *src, *dst, *tmp;
    int w, h, length, rows;
    int status;

    // TODO: read from texture rendertarget. Although no-one sane should do it.
    if (renderer->target) {
        return SDL_Unsupported();
    }


    temp_pitch = rect->w * SDL_BYTESPERPIXEL(temp_format);
    buflen = rect->h * temp_pitch;
    if (buflen == 0) {
        return 0;  /* nothing to do. */
    }

    temp_pixels = SDL_malloc(buflen);
    if (!temp_pixels) {
        return SDL_OutOfMemory();
    }

    SDL_GetRendererOutputSize(renderer, &w, &h);

    read_pixels(rect->x, renderer->target ? rect->y : (h-rect->y)-rect->h,
                       rect->w, rect->h, temp_pixels);

    /* Flip the rows to be top-down if necessary */

    if (!renderer->target) {
        SDL_bool isstack;
        length = rect->w * SDL_BYTESPERPIXEL(temp_format);
        src = (Uint8*)temp_pixels + (rect->h-1)*temp_pitch;
        dst = (Uint8*)temp_pixels;
        tmp = SDL_small_alloc(Uint8, length, &isstack);
        rows = rect->h / 2;
        while (rows--) {
            SDL_memcpy(tmp, dst, length);
            SDL_memcpy(dst, src, length);
            SDL_memcpy(src, tmp, length);
            dst += temp_pitch;
            src -= temp_pitch;
        }
        SDL_small_free(tmp, isstack);
    }

    status = SDL_ConvertPixels(rect->w, rect->h,
                               temp_format, temp_pixels, temp_pitch,
                               pixel_format, pixels, pitch);
    SDL_free(temp_pixels);

    return status;
}


static void
VITA_GXM_RenderPresent(SDL_Renderer *renderer)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;
    SceCommonDialogUpdateParam updateParam;

    if (data->drawing) {
        sceGxmEndScene(data->gxm_context, NULL, NULL);
        if (data->displayData.wait_vblank) {
            sceGxmFinish(data->gxm_context);
        }
    }

    data->displayData.address = data->displayBufferData[data->backBufferIndex];

    SDL_memset(&updateParam, 0, sizeof(updateParam));

    updateParam.renderTarget.colorFormat    = VITA_GXM_COLOR_FORMAT;
    updateParam.renderTarget.surfaceType    = SCE_GXM_COLOR_SURFACE_LINEAR;
    updateParam.renderTarget.width          = VITA_GXM_SCREEN_WIDTH;
    updateParam.renderTarget.height         = VITA_GXM_SCREEN_HEIGHT;
    updateParam.renderTarget.strideInPixels = VITA_GXM_SCREEN_STRIDE;

    updateParam.renderTarget.colorSurfaceData = data->displayBufferData[data->backBufferIndex];
    updateParam.renderTarget.depthSurfaceData = data->depthBufferData;

    updateParam.displaySyncObject = (SceGxmSyncObject *)data->displayBufferSync[data->backBufferIndex];

    sceCommonDialogUpdate(&updateParam);

#if DEBUG_RAZOR
    sceGxmPadHeartbeat(
        (const SceGxmColorSurface *)&data->displaySurface[data->backBufferIndex],
        (SceGxmSyncObject *)data->displayBufferSync[data->backBufferIndex]
    );
#endif

    sceGxmDisplayQueueAddEntry(
        data->displayBufferSync[data->frontBufferIndex],    // OLD fb
        data->displayBufferSync[data->backBufferIndex],     // NEW fb
        &data->displayData
    );

    // update buffer indices
    data->frontBufferIndex = data->backBufferIndex;
    data->backBufferIndex = (data->backBufferIndex + 1) % VITA_GXM_BUFFERS;
    data->pool_index = 0;

    data->current_pool = (data->current_pool + 1) % 2;

    data->drawing = SDL_FALSE;
}

static void
VITA_GXM_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;
    VITA_GXM_TextureData *vita_texture = (VITA_GXM_TextureData *) texture->driverdata;

    if (data == 0)
        return;

    if(vita_texture == 0)
        return;

    if(vita_texture->tex == 0)
        return;

    sceGxmFinish(data->gxm_context);

    free_gxm_texture(vita_texture->tex);

    SDL_free(vita_texture);

    texture->driverdata = NULL;
}

static void
VITA_GXM_DestroyRenderer(SDL_Renderer *renderer)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;
    if (data) {
        if (!data->initialized)
            return;

        gxm_finish(renderer);

        data->initialized = SDL_FALSE;
        data->drawing = SDL_FALSE;
        SDL_free(data);
    }
    SDL_free(renderer);
}

#endif /* SDL_VIDEO_RENDER_VITA_GXM */

/* vi: set ts=4 sw=4 expandtab: */
