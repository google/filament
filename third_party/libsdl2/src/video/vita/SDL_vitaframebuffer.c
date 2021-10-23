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

#if SDL_VIDEO_DRIVER_VITA

#include "SDL_vitavideo.h"

#include <psp2/kernel/sysmem.h>

#define SCREEN_W 960
#define SCREEN_H 544
#define ALIGN(x, a)     (((x) + ((a) - 1)) & ~((a) - 1))
#define DISPLAY_PIXEL_FORMAT SCE_DISPLAY_PIXELFORMAT_A8B8G8R8

void *vita_gpu_alloc(SceKernelMemBlockType type, unsigned int size, SceUID *uid)
{
    void *mem;

    if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW) {
        size = ALIGN(size, 256*1024);
    } else {
        size = ALIGN(size, 4*1024);
    }

    *uid = sceKernelAllocMemBlock("gpu_mem", type, size, NULL);

    if (*uid < 0)
        return NULL;

    if (sceKernelGetMemBlockBase(*uid, &mem) < 0)
        return NULL;

    return mem;
}

void vita_gpu_free(SceUID uid)
{
    void *mem = NULL;
    if (sceKernelGetMemBlockBase(uid, &mem) < 0)
        return;
    sceKernelFreeMemBlock(uid);
}

int VITA_CreateWindowFramebuffer(_THIS, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SceDisplayFrameBuf framebuf;

    *format = SDL_PIXELFORMAT_ABGR8888;
    *pitch = SCREEN_W * 4;

    data->buffer = vita_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
        4 * SCREEN_W * SCREEN_H,
        &data->buffer_uid
    );

    // memset the buffer to black
    SDL_memset(data->buffer, 0x0, SCREEN_W*SCREEN_H*4);

    SDL_memset(&framebuf, 0x00, sizeof(SceDisplayFrameBuf));
    framebuf.size        = sizeof(SceDisplayFrameBuf);
    framebuf.base        = data->buffer;
    framebuf.pitch       = SCREEN_W;
    framebuf.pixelformat = DISPLAY_PIXEL_FORMAT;
    framebuf.width       = SCREEN_W;
    framebuf.height      = SCREEN_H;
    sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);

    *pixels = data->buffer;

    return 0;
}

int VITA_UpdateWindowFramebuffer(_THIS, SDL_Window * window, const SDL_Rect * rects, int numrects)
{
    // do nothing
    return 0;
}

void VITA_DestroyWindowFramebuffer(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;

    if (!data) {
        /* The window wasn't fully initialized */
        return;
    }

    vita_gpu_free(data->buffer_uid);
    data->buffer = NULL;
    return;
}

#endif /* SDL_VIDEO_DRIVER_VITA */

/* vi: set ts=4 sw=4 expandtab: */
