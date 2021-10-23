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

#ifndef SDL_RENDER_VITA_GXM_MEMORY_H
#define SDL_RENDER_VITA_GXM_MEMORY_H

#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/kernel/sysmem.h>

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

void *mem_gpu_alloc(SceKernelMemBlockType type, unsigned int size, unsigned int alignment, unsigned int attribs, SceUID *uid);
void mem_gpu_free(SceUID uid);
void *mem_vertex_usse_alloc(unsigned int size, SceUID *uid, unsigned int *usse_offset);
void mem_vertex_usse_free(SceUID uid);
void *mem_fragment_usse_alloc(unsigned int size, SceUID *uid, unsigned int *usse_offset);
void mem_fragment_usse_free(SceUID uid);

#endif /* SDL_RENDER_VITA_GXM_MEMORY_H */

/* vi: set ts=4 sw=4 expandtab: */
