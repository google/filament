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

#include <psp2/kernel/processmgr.h>
#include <psp2/appmgr.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/types.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/message_dialog.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "SDL_render_vita_gxm_tools.h"
#include "SDL_render_vita_gxm_types.h"
#include "SDL_render_vita_gxm_memory.h"
#include "SDL_render_vita_gxm_shaders.h"

void
init_orthographic_matrix(float *m, float left, float right, float bottom, float top, float near, float far)
{
    m[0x0] = 2.0f/(right-left);
    m[0x4] = 0.0f;
    m[0x8] = 0.0f;
    m[0xC] = -(right+left)/(right-left);

    m[0x1] = 0.0f;
    m[0x5] = 2.0f/(top-bottom);
    m[0x9] = 0.0f;
    m[0xD] = -(top+bottom)/(top-bottom);

    m[0x2] = 0.0f;
    m[0x6] = 0.0f;
    m[0xA] = -2.0f/(far-near);
    m[0xE] = (far+near)/(far-near);

    m[0x3] = 0.0f;
    m[0x7] = 0.0f;
    m[0xB] = 0.0f;
    m[0xF] = 1.0f;
}

static void *
patcher_host_alloc(void *user_data, unsigned int size)
{
    void *mem = SDL_malloc(size);
    (void)user_data;
    return mem;
}

static void
patcher_host_free(void *user_data, void *mem)
{
    (void)user_data;
    SDL_free(mem);
}

void *
pool_malloc(VITA_GXM_RenderData *data, unsigned int size)
{

    if ((data->pool_index + size) < VITA_GXM_POOL_SIZE) {
        void *addr = (void *)((unsigned int)data->pool_addr[data->current_pool] + data->pool_index);
        data->pool_index += size;
        return addr;
    }
    SDL_LogError(SDL_LOG_CATEGORY_RENDER, "POOL OVERFLOW\n");
    return NULL;
}

void *
pool_memalign(VITA_GXM_RenderData *data, unsigned int size, unsigned int alignment)
{
    unsigned int new_index = (data->pool_index + alignment - 1) & ~(alignment - 1);
    if ((new_index + size) < VITA_GXM_POOL_SIZE) {
        void *addr = (void *)((unsigned int)data->pool_addr[data->current_pool] + new_index);
        data->pool_index = new_index + size;
        return addr;
    }
    SDL_LogError(SDL_LOG_CATEGORY_RENDER, "POOL OVERFLOW\n");
    return NULL;
}

static int
tex_format_to_bytespp(SceGxmTextureFormat format)
{
    switch (format & 0x9f000000U) {
    case SCE_GXM_TEXTURE_BASE_FORMAT_U8:
    case SCE_GXM_TEXTURE_BASE_FORMAT_S8:
    case SCE_GXM_TEXTURE_BASE_FORMAT_P8:
        return 1;
    case SCE_GXM_TEXTURE_BASE_FORMAT_U4U4U4U4:
    case SCE_GXM_TEXTURE_BASE_FORMAT_U8U3U3U2:
    case SCE_GXM_TEXTURE_BASE_FORMAT_U1U5U5U5:
    case SCE_GXM_TEXTURE_BASE_FORMAT_U5U6U5:
    case SCE_GXM_TEXTURE_BASE_FORMAT_S5S5U6:
    case SCE_GXM_TEXTURE_BASE_FORMAT_U8U8:
    case SCE_GXM_TEXTURE_BASE_FORMAT_S8S8:
        return 2;
    case SCE_GXM_TEXTURE_BASE_FORMAT_U8U8U8:
    case SCE_GXM_TEXTURE_BASE_FORMAT_S8S8S8:
        return 3;
    case SCE_GXM_TEXTURE_BASE_FORMAT_U8U8U8U8:
    case SCE_GXM_TEXTURE_BASE_FORMAT_S8S8S8S8:
    case SCE_GXM_TEXTURE_BASE_FORMAT_F32:
    case SCE_GXM_TEXTURE_BASE_FORMAT_U32:
    case SCE_GXM_TEXTURE_BASE_FORMAT_S32:
    default:
        return 4;
    }
}

static void
display_callback(const void *callback_data)
{
    SceDisplayFrameBuf framebuf;
    const VITA_GXM_DisplayData *display_data = (const VITA_GXM_DisplayData *)callback_data;

    SDL_memset(&framebuf, 0x00, sizeof(SceDisplayFrameBuf));
    framebuf.size        = sizeof(SceDisplayFrameBuf);
    framebuf.base        = display_data->address;
    framebuf.pitch       = VITA_GXM_SCREEN_STRIDE;
    framebuf.pixelformat = VITA_GXM_PIXEL_FORMAT;
    framebuf.width       = VITA_GXM_SCREEN_WIDTH;
    framebuf.height      = VITA_GXM_SCREEN_HEIGHT;
    sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);

    if (display_data->wait_vblank) {
        sceDisplayWaitVblankStart();
    }
}

static void
free_fragment_programs(VITA_GXM_RenderData *data, fragment_programs *out)
{
    sceGxmShaderPatcherReleaseFragmentProgram(data->shaderPatcher, out->color);
    sceGxmShaderPatcherReleaseFragmentProgram(data->shaderPatcher, out->texture);
    sceGxmShaderPatcherReleaseFragmentProgram(data->shaderPatcher, out->textureTint);
}

static void
make_fragment_programs(VITA_GXM_RenderData *data, fragment_programs *out,
    const SceGxmBlendInfo *blend_info)
{
    int err;

    err = sceGxmShaderPatcherCreateFragmentProgram(
        data->shaderPatcher,
        data->colorFragmentProgramId,
        SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
        0,
        blend_info,
        colorVertexProgramGxp,
        &out->color
    );

    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Patcher create fragment failed: %d\n", err);
        return;
    }

    err = sceGxmShaderPatcherCreateFragmentProgram(
        data->shaderPatcher,
        data->textureFragmentProgramId,
        SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
        0,
        blend_info,
        textureVertexProgramGxp,
        &out->texture
    );

    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Patcher create fragment failed: %d\n", err);
        return;
    }

    err = sceGxmShaderPatcherCreateFragmentProgram(
        data->shaderPatcher,
        data->textureTintFragmentProgramId,
        SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
        0,
        blend_info,
        textureVertexProgramGxp,
        &out->textureTint
    );

    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Patcher create fragment failed: %d\n", err);
        return;
    }
}


static void
set_stencil_mask(VITA_GXM_RenderData *data, float x, float y, float w, float h)
{
    void *vertexDefaultBuffer;
    color_vertex *vertices = (color_vertex *)pool_memalign(
        data,
        4 * sizeof(color_vertex), // 4 vertices
        sizeof(color_vertex)
    );

    vertices[0].x = x;
    vertices[0].y = y;
    vertices[0].z = +0.5f;
    vertices[0].color = 0;

    vertices[1].x = x + w;
    vertices[1].y = y;
    vertices[1].z = +0.5f;
    vertices[1].color = 0;

    vertices[2].x = x;
    vertices[2].y = y + h;
    vertices[2].z = +0.5f;
    vertices[2].color = 0;

    vertices[3].x = x + w;
    vertices[3].y = y + h;
    vertices[3].z = +0.5f;
    vertices[3].color = 0;

    data->drawstate.fragment_program = data->colorFragmentProgram;
    data->drawstate.vertex_program = data->colorVertexProgram;
    sceGxmSetVertexProgram(data->gxm_context, data->colorVertexProgram);
    sceGxmSetFragmentProgram(data->gxm_context, data->colorFragmentProgram);

    sceGxmReserveVertexDefaultUniformBuffer(data->gxm_context, &vertexDefaultBuffer);
    sceGxmSetUniformDataF(vertexDefaultBuffer, data->colorWvpParam, 0, 16, data->ortho_matrix);

    sceGxmSetVertexStream(data->gxm_context, 0, vertices);
    sceGxmDraw(data->gxm_context, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, data->linearIndices, 4);
}


void
set_clip_rectangle(VITA_GXM_RenderData *data, int x_min, int y_min, int x_max, int y_max)
{
    if(data->drawing) {
        // clear the stencil buffer to 0
        sceGxmSetFrontStencilFunc(
            data->gxm_context,
            SCE_GXM_STENCIL_FUNC_NEVER,
            SCE_GXM_STENCIL_OP_ZERO,
            SCE_GXM_STENCIL_OP_ZERO,
            SCE_GXM_STENCIL_OP_ZERO,
            0xFF,
            0xFF
        );

        set_stencil_mask(data, 0, 0, VITA_GXM_SCREEN_WIDTH, VITA_GXM_SCREEN_HEIGHT);

        // set the stencil to 1 in the desired region
        sceGxmSetFrontStencilFunc(
            data->gxm_context,
            SCE_GXM_STENCIL_FUNC_NEVER,
            SCE_GXM_STENCIL_OP_REPLACE,
            SCE_GXM_STENCIL_OP_REPLACE,
            SCE_GXM_STENCIL_OP_REPLACE,
            0xFF,
            0xFF
        );

        set_stencil_mask(data, x_min, y_min, x_max - x_min, y_max - y_min);

        // set the stencil function to only accept pixels where the stencil is 1
        sceGxmSetFrontStencilFunc(
            data->gxm_context,
            SCE_GXM_STENCIL_FUNC_EQUAL,
            SCE_GXM_STENCIL_OP_KEEP,
            SCE_GXM_STENCIL_OP_KEEP,
            SCE_GXM_STENCIL_OP_KEEP,
            0xFF,
            0xFF
        );
    }
}

void
unset_clip_rectangle(VITA_GXM_RenderData *data)
{
    sceGxmSetFrontStencilFunc(
        data->gxm_context,
        SCE_GXM_STENCIL_FUNC_ALWAYS,
        SCE_GXM_STENCIL_OP_KEEP,
        SCE_GXM_STENCIL_OP_KEEP,
        SCE_GXM_STENCIL_OP_KEEP,
        0xFF,
        0xFF
    );
}

int
gxm_init(SDL_Renderer *renderer)
{
    unsigned int i, x, y;
    int err;
    void *vdmRingBuffer;
    void *vertexRingBuffer;
    void *fragmentRingBuffer;
    unsigned int fragmentUsseRingBufferOffset;
    void *fragmentUsseRingBuffer;
    unsigned int patcherVertexUsseOffset;
    unsigned int patcherFragmentUsseOffset;
    void *patcherBuffer;
    void *patcherVertexUsse;
    void *patcherFragmentUsse;

    SceGxmRenderTargetParams renderTargetParams;
    SceGxmShaderPatcherParams patcherParams;

    // compute the memory footprint of the depth buffer
    const unsigned int alignedWidth = ALIGN(VITA_GXM_SCREEN_WIDTH, SCE_GXM_TILE_SIZEX);
    const unsigned int alignedHeight = ALIGN(VITA_GXM_SCREEN_HEIGHT, SCE_GXM_TILE_SIZEY);

    unsigned int sampleCount = alignedWidth * alignedHeight;
    unsigned int depthStrideInSamples = alignedWidth;

    // set buffer sizes for this sample
    const unsigned int patcherBufferSize        = 64*1024;
    const unsigned int patcherVertexUsseSize    = 64*1024;
    const unsigned int patcherFragmentUsseSize  = 64*1024;

    // Fill SceGxmBlendInfo
    static const SceGxmBlendInfo blend_info_none = {
        .colorFunc = SCE_GXM_BLEND_FUNC_NONE,
        .alphaFunc = SCE_GXM_BLEND_FUNC_NONE,
        .colorSrc  = SCE_GXM_BLEND_FACTOR_ZERO,
        .colorDst  = SCE_GXM_BLEND_FACTOR_ZERO,
        .alphaSrc  = SCE_GXM_BLEND_FACTOR_ZERO,
        .alphaDst  = SCE_GXM_BLEND_FACTOR_ZERO,
        .colorMask = SCE_GXM_COLOR_MASK_ALL
    };

    static const SceGxmBlendInfo blend_info_blend = {
        .colorFunc = SCE_GXM_BLEND_FUNC_ADD,
        .alphaFunc = SCE_GXM_BLEND_FUNC_ADD,
        .colorSrc  = SCE_GXM_BLEND_FACTOR_SRC_ALPHA,
        .colorDst  = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaSrc  = SCE_GXM_BLEND_FACTOR_ONE,
        .alphaDst  = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorMask = SCE_GXM_COLOR_MASK_ALL
    };

    static const SceGxmBlendInfo blend_info_add = {
        .colorFunc = SCE_GXM_BLEND_FUNC_ADD,
        .alphaFunc = SCE_GXM_BLEND_FUNC_ADD,
        .colorSrc  = SCE_GXM_BLEND_FACTOR_SRC_ALPHA,
        .colorDst  = SCE_GXM_BLEND_FACTOR_ONE,
        .alphaSrc  = SCE_GXM_BLEND_FACTOR_ZERO,
        .alphaDst  = SCE_GXM_BLEND_FACTOR_ONE,
        .colorMask = SCE_GXM_COLOR_MASK_ALL
    };

    static const SceGxmBlendInfo blend_info_mod = {
        .colorFunc = SCE_GXM_BLEND_FUNC_ADD,
        .alphaFunc = SCE_GXM_BLEND_FUNC_ADD,

        .colorSrc  = SCE_GXM_BLEND_FACTOR_ZERO,
        .colorDst  = SCE_GXM_BLEND_FACTOR_SRC_COLOR,

        .alphaSrc  = SCE_GXM_BLEND_FACTOR_ZERO,
        .alphaDst  = SCE_GXM_BLEND_FACTOR_ONE,
        .colorMask = SCE_GXM_COLOR_MASK_ALL
    };

    static const SceGxmBlendInfo blend_info_mul = {
        .colorFunc = SCE_GXM_BLEND_FUNC_ADD,
        .alphaFunc = SCE_GXM_BLEND_FUNC_ADD,
        .colorSrc  = SCE_GXM_BLEND_FACTOR_DST_COLOR,
        .colorDst  = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaSrc  = SCE_GXM_BLEND_FACTOR_DST_ALPHA,
        .alphaDst  = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorMask = SCE_GXM_COLOR_MASK_ALL
    };

    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    SceGxmInitializeParams initializeParams;
    SDL_memset(&initializeParams, 0, sizeof(SceGxmInitializeParams));
    initializeParams.flags                          = 0;
    initializeParams.displayQueueMaxPendingCount    = VITA_GXM_PENDING_SWAPS;
    initializeParams.displayQueueCallback           = display_callback;
    initializeParams.displayQueueCallbackDataSize   = sizeof(VITA_GXM_DisplayData);
    initializeParams.parameterBufferSize            = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;

    err = sceGxmInitialize(&initializeParams);

    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "gxm init failed: %d\n", err);
        return err;
    }

    // allocate ring buffer memory using default sizes
    vdmRingBuffer = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
        SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE,
        4,
        SCE_GXM_MEMORY_ATTRIB_READ,
        &data->vdmRingBufferUid);

    vertexRingBuffer = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
        SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE,
        4,
        SCE_GXM_MEMORY_ATTRIB_READ,
        &data->vertexRingBufferUid);

    fragmentRingBuffer = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
        SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE,
        4,
        SCE_GXM_MEMORY_ATTRIB_READ,
        &data->fragmentRingBufferUid);

    fragmentUsseRingBuffer = mem_fragment_usse_alloc(
        SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE,
        &data->fragmentUsseRingBufferUid,
        &fragmentUsseRingBufferOffset);

    SDL_memset(&data->contextParams, 0, sizeof(SceGxmContextParams));
    data->contextParams.hostMem                       = SDL_malloc(SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE);
    data->contextParams.hostMemSize                   = SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE;
    data->contextParams.vdmRingBufferMem              = vdmRingBuffer;
    data->contextParams.vdmRingBufferMemSize          = SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE;
    data->contextParams.vertexRingBufferMem           = vertexRingBuffer;
    data->contextParams.vertexRingBufferMemSize       = SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE;
    data->contextParams.fragmentRingBufferMem         = fragmentRingBuffer;
    data->contextParams.fragmentRingBufferMemSize     = SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE;
    data->contextParams.fragmentUsseRingBufferMem     = fragmentUsseRingBuffer;
    data->contextParams.fragmentUsseRingBufferMemSize = SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE;
    data->contextParams.fragmentUsseRingBufferOffset  = fragmentUsseRingBufferOffset;

    err = sceGxmCreateContext(&data->contextParams, &data->gxm_context);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "create context failed: %d\n", err);
        return err;
    }

    // set up parameters
    SDL_memset(&renderTargetParams, 0, sizeof(SceGxmRenderTargetParams));
    renderTargetParams.flags                = 0;
    renderTargetParams.width                = VITA_GXM_SCREEN_WIDTH;
    renderTargetParams.height               = VITA_GXM_SCREEN_HEIGHT;
    renderTargetParams.scenesPerFrame       = 1;
    renderTargetParams.multisampleMode      = 0;
    renderTargetParams.multisampleLocations = 0;
    renderTargetParams.driverMemBlock       = -1; // Invalid UID

    // create the render target
    err = sceGxmCreateRenderTarget(&renderTargetParams, &data->renderTarget);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "render target creation failed: %d\n", err);
        return err;
    }

    // allocate memory and sync objects for display buffers
    for (i = 0; i < VITA_GXM_BUFFERS; i++) {

        // allocate memory for display
        data->displayBufferData[i] = mem_gpu_alloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
            4 * VITA_GXM_SCREEN_STRIDE * VITA_GXM_SCREEN_HEIGHT,
            SCE_GXM_COLOR_SURFACE_ALIGNMENT,
            SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
            &data->displayBufferUid[i]);

        // memset the buffer to black
        for (y = 0; y < VITA_GXM_SCREEN_HEIGHT; y++) {
            unsigned int *row = (unsigned int *)data->displayBufferData[i] + y * VITA_GXM_SCREEN_STRIDE;
            for (x = 0; x < VITA_GXM_SCREEN_WIDTH; x++) {
                row[x] = 0xff000000;
            }
        }

        // initialize a color surface for this display buffer
        err = sceGxmColorSurfaceInit(
            &data->displaySurface[i],
            VITA_GXM_COLOR_FORMAT,
            SCE_GXM_COLOR_SURFACE_LINEAR,
            SCE_GXM_COLOR_SURFACE_SCALE_NONE,
            SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT,
            VITA_GXM_SCREEN_WIDTH,
            VITA_GXM_SCREEN_HEIGHT,
            VITA_GXM_SCREEN_STRIDE,
            data->displayBufferData[i]
        );

        if (err != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "color surface init failed: %d\n", err);
            return err;
        }


        // create a sync object that we will associate with this buffer
        err = sceGxmSyncObjectCreate(&data->displayBufferSync[i]);
        if (err != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "sync object creation failed: %d\n", err);
            return err;
        }

    }


    // allocate the depth buffer
    data->depthBufferData = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
        4 * sampleCount,
        SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT,
        SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
        &data->depthBufferUid);

    // allocate the stencil buffer
    data->stencilBufferData = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
        4 * sampleCount,
        SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT,
        SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
        &data->stencilBufferUid);

    // create the SceGxmDepthStencilSurface structure
    err = sceGxmDepthStencilSurfaceInit(
        &data->depthSurface,
        SCE_GXM_DEPTH_STENCIL_FORMAT_S8D24,
        SCE_GXM_DEPTH_STENCIL_SURFACE_TILED,
        depthStrideInSamples,
        data->depthBufferData,
        data->stencilBufferData);

    // set the stencil test reference (this is currently assumed to always remain 1 after here for region clipping)
    sceGxmSetFrontStencilRef(data->gxm_context, 1);


    // set the stencil function (this wouldn't actually be needed, as the set clip rectangle function has to call this at the begginning of every scene)
    sceGxmSetFrontStencilFunc(
        data->gxm_context,
        SCE_GXM_STENCIL_FUNC_ALWAYS,
        SCE_GXM_STENCIL_OP_KEEP,
        SCE_GXM_STENCIL_OP_KEEP,
        SCE_GXM_STENCIL_OP_KEEP,
        0xFF,
        0xFF);


    // allocate memory for buffers and USSE code
    patcherBuffer = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
        patcherBufferSize,
        4,
        SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
        &data->patcherBufferUid);

    patcherVertexUsse = mem_vertex_usse_alloc(
        patcherVertexUsseSize,
        &data->patcherVertexUsseUid,
        &patcherVertexUsseOffset);

    patcherFragmentUsse = mem_fragment_usse_alloc(
        patcherFragmentUsseSize,
        &data->patcherFragmentUsseUid,
        &patcherFragmentUsseOffset);

    // create a shader patcher
    SDL_memset(&patcherParams, 0, sizeof(SceGxmShaderPatcherParams));
    patcherParams.userData                  = NULL;
    patcherParams.hostAllocCallback         = &patcher_host_alloc;
    patcherParams.hostFreeCallback          = &patcher_host_free;
    patcherParams.bufferAllocCallback       = NULL;
    patcherParams.bufferFreeCallback        = NULL;
    patcherParams.bufferMem                 = patcherBuffer;
    patcherParams.bufferMemSize             = patcherBufferSize;
    patcherParams.vertexUsseAllocCallback   = NULL;
    patcherParams.vertexUsseFreeCallback    = NULL;
    patcherParams.vertexUsseMem             = patcherVertexUsse;
    patcherParams.vertexUsseMemSize         = patcherVertexUsseSize;
    patcherParams.vertexUsseOffset          = patcherVertexUsseOffset;
    patcherParams.fragmentUsseAllocCallback = NULL;
    patcherParams.fragmentUsseFreeCallback  = NULL;
    patcherParams.fragmentUsseMem           = patcherFragmentUsse;
    patcherParams.fragmentUsseMemSize       = patcherFragmentUsseSize;
    patcherParams.fragmentUsseOffset        = patcherFragmentUsseOffset;

    err = sceGxmShaderPatcherCreate(&patcherParams, &data->shaderPatcher);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "shader patcher creation failed: %d\n", err);
        return err;
    }


    // check the shaders
    err = sceGxmProgramCheck(clearVertexProgramGxp);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "check program (clear vertex) failed: %d\n", err);
        return err;
    }

    err = sceGxmProgramCheck(clearFragmentProgramGxp);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "check program (clear fragment) failed: %d\n", err);
        return err;
    }

    err = sceGxmProgramCheck(colorVertexProgramGxp);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "check program (color vertex) failed: %d\n", err);
        return err;
    }

    err = sceGxmProgramCheck(colorFragmentProgramGxp);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "check program (color fragment) failed: %d\n", err);
        return err;
    }

    err = sceGxmProgramCheck(textureVertexProgramGxp);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "check program (texture vertex) failed: %d\n", err);
        return err;
    }

    err = sceGxmProgramCheck(textureFragmentProgramGxp);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "check program (texture fragment) failed: %d\n", err);
        return err;
    }

    err = sceGxmProgramCheck(textureTintFragmentProgramGxp);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "check program (texture tint fragment) failed: %d\n", err);
        return err;
    }

    // register programs with the patcher
    err = sceGxmShaderPatcherRegisterProgram(data->shaderPatcher, clearVertexProgramGxp, &data->clearVertexProgramId);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "register program (clear vertex) failed: %d\n", err);
        return err;
    }

    err = sceGxmShaderPatcherRegisterProgram(data->shaderPatcher, clearFragmentProgramGxp, &data->clearFragmentProgramId);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "register program (clear fragment) failed: %d\n", err);
        return err;
    }

    err = sceGxmShaderPatcherRegisterProgram(data->shaderPatcher, colorVertexProgramGxp, &data->colorVertexProgramId);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "register program (color vertex) failed: %d\n", err);
        return err;
    }

    err = sceGxmShaderPatcherRegisterProgram(data->shaderPatcher, colorFragmentProgramGxp, &data->colorFragmentProgramId);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "register program (color fragment) failed: %d\n", err);
        return err;
    }

    err = sceGxmShaderPatcherRegisterProgram(data->shaderPatcher, textureVertexProgramGxp, &data->textureVertexProgramId);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "register program (texture vertex) failed: %d\n", err);
        return err;
    }

    err = sceGxmShaderPatcherRegisterProgram(data->shaderPatcher, textureFragmentProgramGxp, &data->textureFragmentProgramId);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "register program (texture fragment) failed: %d\n", err);
        return err;
    }

    err = sceGxmShaderPatcherRegisterProgram(data->shaderPatcher, textureTintFragmentProgramGxp, &data->textureTintFragmentProgramId);
    if (err != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "register program (texture tint fragment) failed: %d\n", err);
        return err;
    }


    {
        // get attributes by name to create vertex format bindings
        const SceGxmProgramParameter *paramClearPositionAttribute = sceGxmProgramFindParameterByName(clearVertexProgramGxp, "aPosition");

        // create clear vertex format
        SceGxmVertexAttribute clearVertexAttributes[1];
        SceGxmVertexStream clearVertexStreams[1];
        clearVertexAttributes[0].streamIndex    = 0;
        clearVertexAttributes[0].offset         = 0;
        clearVertexAttributes[0].format         = SCE_GXM_ATTRIBUTE_FORMAT_F32;
        clearVertexAttributes[0].componentCount = 2;
        clearVertexAttributes[0].regIndex       = sceGxmProgramParameterGetResourceIndex(paramClearPositionAttribute);
        clearVertexStreams[0].stride            = sizeof(clear_vertex);
        clearVertexStreams[0].indexSource       = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

        // create clear programs
        err = sceGxmShaderPatcherCreateVertexProgram(
            data->shaderPatcher,
            data->clearVertexProgramId,
            clearVertexAttributes,
            1,
            clearVertexStreams,
            1,
            &data->clearVertexProgram
        );
        if (err != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "create program (clear vertex) failed: %d\n", err);
            return err;
        }

        err = sceGxmShaderPatcherCreateFragmentProgram(
            data->shaderPatcher,
            data->clearFragmentProgramId,
            SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
            0,
            NULL,
            clearVertexProgramGxp,
            &data->clearFragmentProgram
        );
        if (err != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "create program (clear fragment) failed: %d\n", err);
            return err;
        }

        // create the clear triangle vertex/index data
        data->clearVertices = (clear_vertex *)mem_gpu_alloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
            3*sizeof(clear_vertex),
            4,
            SCE_GXM_MEMORY_ATTRIB_READ,
            &data->clearVerticesUid
        );
    }

    // Allocate a 64k * 2 bytes = 128 KiB buffer and store all possible
    // 16-bit indices in linear ascending order, so we can use this for
    // all drawing operations where we don't want to use indexing.
    data->linearIndices = (uint16_t *)mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
        UINT16_MAX*sizeof(uint16_t),
        sizeof(uint16_t),
        SCE_GXM_MEMORY_ATTRIB_READ,
        &data->linearIndicesUid
    );

    for (i = 0; i <= UINT16_MAX; ++i)
    {
        data->linearIndices[i] = i;
    }

    data->clearVertices[0].x = -1.0f;
    data->clearVertices[0].y = -1.0f;
    data->clearVertices[1].x =  3.0f;
    data->clearVertices[1].y = -1.0f;
    data->clearVertices[2].x = -1.0f;
    data->clearVertices[2].y =  3.0f;

    {
        const SceGxmProgramParameter *paramColorPositionAttribute = sceGxmProgramFindParameterByName(colorVertexProgramGxp, "aPosition");

        const SceGxmProgramParameter *paramColorColorAttribute = sceGxmProgramFindParameterByName(colorVertexProgramGxp, "aColor");

        // create color vertex format
        SceGxmVertexAttribute colorVertexAttributes[2];
        SceGxmVertexStream colorVertexStreams[1];
        /* x,y,z: 3 float 32 bits */
        colorVertexAttributes[0].streamIndex = 0;
        colorVertexAttributes[0].offset = 0;
        colorVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
        colorVertexAttributes[0].componentCount = 3; // (x, y, z)
        colorVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramColorPositionAttribute);
        /* color: 4 unsigned char  = 32 bits */
        colorVertexAttributes[1].streamIndex = 0;
        colorVertexAttributes[1].offset = 12; // (x, y, z) * 4 = 12 bytes
        colorVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N;
        colorVertexAttributes[1].componentCount = 4; // (color)
        colorVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramColorColorAttribute);
        // 16 bit (short) indices
        colorVertexStreams[0].stride = sizeof(color_vertex);
        colorVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

        // create color shaders
        err = sceGxmShaderPatcherCreateVertexProgram(
            data->shaderPatcher,
            data->colorVertexProgramId,
            colorVertexAttributes,
            2,
            colorVertexStreams,
            1,
            &data->colorVertexProgram
        );
        if (err != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "create program (color vertex) failed: %d\n", err);
            return err;
        }

    }


    {
        const SceGxmProgramParameter *paramTexturePositionAttribute = sceGxmProgramFindParameterByName(textureVertexProgramGxp, "aPosition");
        const SceGxmProgramParameter *paramTextureTexcoordAttribute = sceGxmProgramFindParameterByName(textureVertexProgramGxp, "aTexcoord");

        // create texture vertex format
        SceGxmVertexAttribute textureVertexAttributes[2];
        SceGxmVertexStream textureVertexStreams[1];
        /* x,y,z: 3 float 32 bits */
        textureVertexAttributes[0].streamIndex = 0;
        textureVertexAttributes[0].offset = 0;
        textureVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
        textureVertexAttributes[0].componentCount = 3; // (x, y, z)
        textureVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramTexturePositionAttribute);
        /* u,v: 2 floats 32 bits */
        textureVertexAttributes[1].streamIndex = 0;
        textureVertexAttributes[1].offset = 12; // (x, y, z) * 4 = 12 bytes
        textureVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
        textureVertexAttributes[1].componentCount = 2; // (u, v)
        textureVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramTextureTexcoordAttribute);
        // 16 bit (short) indices
        textureVertexStreams[0].stride = sizeof(texture_vertex);
        textureVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

        // create texture shaders
        err = sceGxmShaderPatcherCreateVertexProgram(
            data->shaderPatcher,
            data->textureVertexProgramId,
            textureVertexAttributes,
            2,
            textureVertexStreams,
            1,
            &data->textureVertexProgram
        );
        if (err != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "create program (texture vertex) failed: %d\n", err);
            return err;
        }

    }

    // Create variations of the fragment program based on blending mode
    make_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_none, &blend_info_none);
    make_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_blend, &blend_info_blend);
    make_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_add, &blend_info_add);
    make_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_mod, &blend_info_mod);
    make_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_mul, &blend_info_mul);

    {
        // Default to blend blending mode
        fragment_programs *in = &data->blendFragmentPrograms.blend_mode_blend;

        data->colorFragmentProgram = in->color;
        data->textureFragmentProgram = in->texture;
        data->textureTintFragmentProgram = in->textureTint;

    }

    // find vertex uniforms by name and cache parameter information
    data->clearClearColorParam = (SceGxmProgramParameter *)sceGxmProgramFindParameterByName(clearFragmentProgramGxp, "uClearColor");
    data->colorWvpParam = (SceGxmProgramParameter *)sceGxmProgramFindParameterByName(colorVertexProgramGxp, "wvp");
    data->textureWvpParam = (SceGxmProgramParameter *)sceGxmProgramFindParameterByName(textureVertexProgramGxp, "wvp");
    data->textureTintColorParam = (SceGxmProgramParameter *)sceGxmProgramFindParameterByName(textureTintFragmentProgramGxp, "uTintColor");

    // Allocate memory for the memory pool
    data->pool_addr[0] = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
        VITA_GXM_POOL_SIZE,
        sizeof(void *),
        SCE_GXM_MEMORY_ATTRIB_READ,
        &data->poolUid[0]
    );

    data->pool_addr[1] = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
        VITA_GXM_POOL_SIZE,
        sizeof(void *),
        SCE_GXM_MEMORY_ATTRIB_READ,
        &data->poolUid[1]
    );

    init_orthographic_matrix(data->ortho_matrix, 0.0f, VITA_GXM_SCREEN_WIDTH, VITA_GXM_SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);

    data->backBufferIndex = 0;
    data->frontBufferIndex = 0;
    data->pool_index = 0;
    data->current_pool = 0;
    data->currentBlendMode = SDL_BLENDMODE_BLEND;

    return 0;
}

void gxm_finish(SDL_Renderer *renderer)
{
    VITA_GXM_RenderData *data = (VITA_GXM_RenderData *) renderer->driverdata;

    // wait until rendering is done
    sceGxmFinish(data->gxm_context);

    // clean up allocations
    sceGxmShaderPatcherReleaseFragmentProgram(data->shaderPatcher, data->clearFragmentProgram);
    sceGxmShaderPatcherReleaseVertexProgram(data->shaderPatcher, data->clearVertexProgram);
    sceGxmShaderPatcherReleaseVertexProgram(data->shaderPatcher, data->colorVertexProgram);
    sceGxmShaderPatcherReleaseVertexProgram(data->shaderPatcher, data->textureVertexProgram);


    free_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_none);
    free_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_blend);
    free_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_add);
    free_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_mod);
    free_fragment_programs(data, &data->blendFragmentPrograms.blend_mode_mul);

    mem_gpu_free(data->linearIndicesUid);
    mem_gpu_free(data->clearVerticesUid);

    // wait until display queue is finished before deallocating display buffers
    sceGxmDisplayQueueFinish();

    // clean up display queue
    mem_gpu_free(data->depthBufferUid);

    for (size_t i = 0; i < VITA_GXM_BUFFERS; i++)
    {
        // clear the buffer then deallocate
        SDL_memset(data->displayBufferData[i], 0, VITA_GXM_SCREEN_HEIGHT * VITA_GXM_SCREEN_STRIDE * 4);
        mem_gpu_free(data->displayBufferUid[i]);

        // destroy the sync object
        sceGxmSyncObjectDestroy(data->displayBufferSync[i]);
    }

    // free the depth and stencil buffer
    mem_gpu_free(data->depthBufferUid);
    mem_gpu_free(data->stencilBufferUid);

    // unregister programs and destroy shader patcher
    sceGxmShaderPatcherUnregisterProgram(data->shaderPatcher, data->clearFragmentProgramId);
    sceGxmShaderPatcherUnregisterProgram(data->shaderPatcher, data->clearVertexProgramId);
    sceGxmShaderPatcherUnregisterProgram(data->shaderPatcher, data->colorFragmentProgramId);
    sceGxmShaderPatcherUnregisterProgram(data->shaderPatcher, data->colorVertexProgramId);
    sceGxmShaderPatcherUnregisterProgram(data->shaderPatcher, data->textureFragmentProgramId);
    sceGxmShaderPatcherUnregisterProgram(data->shaderPatcher, data->textureTintFragmentProgramId);
    sceGxmShaderPatcherUnregisterProgram(data->shaderPatcher, data->textureVertexProgramId);

    sceGxmShaderPatcherDestroy(data->shaderPatcher);
    mem_fragment_usse_free(data->patcherFragmentUsseUid);
    mem_vertex_usse_free(data->patcherVertexUsseUid);
    mem_gpu_free(data->patcherBufferUid);

    // destroy the render target
    sceGxmDestroyRenderTarget(data->renderTarget);

    // destroy the gxm context
    sceGxmDestroyContext(data->gxm_context);
    mem_fragment_usse_free(data->fragmentUsseRingBufferUid);
    mem_gpu_free(data->fragmentRingBufferUid);
    mem_gpu_free(data->vertexRingBufferUid);
    mem_gpu_free(data->vdmRingBufferUid);
    SDL_free(data->contextParams.hostMem);

    mem_gpu_free(data->poolUid[0]);
    mem_gpu_free(data->poolUid[1]);

    // terminate libgxm
    sceGxmTerminate();
}

// textures

void
free_gxm_texture(gxm_texture *texture)
{
    if (texture) {
        if (texture->gxm_rendertarget) {
            sceGxmDestroyRenderTarget(texture->gxm_rendertarget);
        }
        if (texture->depth_UID) {
            mem_gpu_free(texture->depth_UID);
        }
        mem_gpu_free(texture->data_UID);
        SDL_free(texture);
    }
}

SceGxmTextureFormat
gxm_texture_get_format(const gxm_texture *texture)
{
    return sceGxmTextureGetFormat(&texture->gxm_tex);
}

unsigned int
gxm_texture_get_width(const gxm_texture *texture)
{
    return sceGxmTextureGetWidth(&texture->gxm_tex);
}

unsigned int
gxm_texture_get_height(const gxm_texture *texture)
{
    return sceGxmTextureGetHeight(&texture->gxm_tex);
}

unsigned int
gxm_texture_get_stride(const gxm_texture *texture)
{
    return ((gxm_texture_get_width(texture) + 7) & ~7)
        * tex_format_to_bytespp(gxm_texture_get_format(texture));
}

void *
gxm_texture_get_datap(const gxm_texture *texture)
{
    return sceGxmTextureGetData(&texture->gxm_tex);
}

gxm_texture *
create_gxm_texture(VITA_GXM_RenderData *data, unsigned int w, unsigned int h, SceGxmTextureFormat format, unsigned int isRenderTarget)
{
    gxm_texture *texture = SDL_malloc(sizeof(gxm_texture));
    const int tex_size =  ((w + 7) & ~ 7) * h * tex_format_to_bytespp(format);
    void *texture_data;

    if (!texture)
        return NULL;

    /* Allocate a GPU buffer for the texture */
    texture_data = mem_gpu_alloc(
        SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
        tex_size,
        SCE_GXM_TEXTURE_ALIGNMENT,
        SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
        &texture->data_UID
    );

    if (!texture_data) {
        free(texture);
        return NULL;
    }

    /* Clear the texture */
    SDL_memset(texture_data, 0, tex_size);

    /* Create the gxm texture */
    sceGxmTextureInitLinear( &texture->gxm_tex, texture_data, format, w, h, 0);

    if (isRenderTarget) {
        void *depthBufferData;
        const uint32_t alignedWidth = ALIGN(w, SCE_GXM_TILE_SIZEX);
        const uint32_t alignedHeight = ALIGN(h, SCE_GXM_TILE_SIZEY);
        uint32_t sampleCount = alignedWidth*alignedHeight;
        uint32_t depthStrideInSamples = alignedWidth;

        int err = sceGxmColorSurfaceInit(
            &texture->gxm_colorsurface,
            SCE_GXM_COLOR_FORMAT_A8B8G8R8,
            SCE_GXM_COLOR_SURFACE_LINEAR,
            SCE_GXM_COLOR_SURFACE_SCALE_NONE,
            SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT,
            w,
            h,
            w,
            texture_data
        );

        if (err < 0) {
            free_gxm_texture(texture);
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "color surface init failed: %d\n", err);
            return NULL;
        }

        // allocate it
        depthBufferData = mem_gpu_alloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
            4*sampleCount,
            SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT,
            SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
            &texture->depth_UID);

        // create the SceGxmDepthStencilSurface structure
        err = sceGxmDepthStencilSurfaceInit(
            &texture->gxm_depthstencil,
            SCE_GXM_DEPTH_STENCIL_FORMAT_S8D24,
            SCE_GXM_DEPTH_STENCIL_SURFACE_TILED,
            depthStrideInSamples,
            depthBufferData,
            NULL);

        if (err < 0) {
            free_gxm_texture(texture);
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "depth stencil init failed: %d\n", err);
            return NULL;
        }

        {
            SceGxmRenderTarget *tgt = NULL;

            // set up parameters
            SceGxmRenderTargetParams renderTargetParams;
            memset(&renderTargetParams, 0, sizeof(SceGxmRenderTargetParams));
            renderTargetParams.flags = 0;
            renderTargetParams.width = w;
            renderTargetParams.height = h;
            renderTargetParams.scenesPerFrame = 1;
            renderTargetParams.multisampleMode = SCE_GXM_MULTISAMPLE_NONE;
            renderTargetParams.multisampleLocations = 0;
            renderTargetParams.driverMemBlock = -1;

            // create the render target
            err = sceGxmCreateRenderTarget(&renderTargetParams, &tgt);

            texture->gxm_rendertarget = tgt;

            if (err < 0) {
                free_gxm_texture(texture);
                SDL_LogError(SDL_LOG_CATEGORY_RENDER, "create render target failed: %d\n", err);
                return NULL;
            }
        }

    }

    return texture;
}

void
gxm_texture_set_filters(gxm_texture *texture, SceGxmTextureFilter min_filter, SceGxmTextureFilter mag_filter)
{
    sceGxmTextureSetMinFilter(&texture->gxm_tex, min_filter);
    sceGxmTextureSetMagFilter(&texture->gxm_tex, mag_filter);
}

static unsigned int back_buffer_index_for_common_dialog = 0;
static unsigned int front_buffer_index_for_common_dialog = 0;
struct
{
    VITA_GXM_DisplayData displayData;
    SceGxmSyncObject* sync;
    SceGxmColorSurface surf;
    SceUID uid;
} buffer_for_common_dialog[VITA_GXM_BUFFERS];

void gxm_minimal_init_for_common_dialog(void)
{
    SceGxmInitializeParams initializeParams;
    SDL_zero(initializeParams);
    initializeParams.flags                          = 0;
    initializeParams.displayQueueMaxPendingCount    = VITA_GXM_PENDING_SWAPS;
    initializeParams.displayQueueCallback           = display_callback;
    initializeParams.displayQueueCallbackDataSize   = sizeof(VITA_GXM_DisplayData);
    initializeParams.parameterBufferSize            = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;
    sceGxmInitialize(&initializeParams);
}

void gxm_minimal_term_for_common_dialog(void)
{
    sceGxmTerminate();
}

void gxm_init_for_common_dialog(void)
{
    for (int i = 0; i < VITA_GXM_BUFFERS; i += 1)
    {
        buffer_for_common_dialog[i].displayData.wait_vblank = SDL_TRUE;
        buffer_for_common_dialog[i].displayData.address = mem_gpu_alloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
            4 * VITA_GXM_SCREEN_STRIDE * VITA_GXM_SCREEN_HEIGHT,
            SCE_GXM_COLOR_SURFACE_ALIGNMENT,
            SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
            &buffer_for_common_dialog[i].uid);
        sceGxmColorSurfaceInit(
            &buffer_for_common_dialog[i].surf,
            VITA_GXM_PIXEL_FORMAT,
            SCE_GXM_COLOR_SURFACE_LINEAR,
            SCE_GXM_COLOR_SURFACE_SCALE_NONE,
            SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT,
            VITA_GXM_SCREEN_WIDTH,
            VITA_GXM_SCREEN_HEIGHT,
            VITA_GXM_SCREEN_STRIDE,
            buffer_for_common_dialog[i].displayData.address
        );
        sceGxmSyncObjectCreate(&buffer_for_common_dialog[i].sync);
    }
    sceGxmDisplayQueueFinish();
}

void gxm_swap_for_common_dialog(void)
{
    SceCommonDialogUpdateParam updateParam;
    SDL_zero(updateParam);
    updateParam.renderTarget.colorFormat    = VITA_GXM_PIXEL_FORMAT;
    updateParam.renderTarget.surfaceType    = SCE_GXM_COLOR_SURFACE_LINEAR;
    updateParam.renderTarget.width          = VITA_GXM_SCREEN_WIDTH;
    updateParam.renderTarget.height         = VITA_GXM_SCREEN_HEIGHT;
    updateParam.renderTarget.strideInPixels = VITA_GXM_SCREEN_STRIDE;

    updateParam.renderTarget.colorSurfaceData = buffer_for_common_dialog[back_buffer_index_for_common_dialog].displayData.address;

    updateParam.displaySyncObject = buffer_for_common_dialog[back_buffer_index_for_common_dialog].sync;
    SDL_memset(buffer_for_common_dialog[back_buffer_index_for_common_dialog].displayData.address, 0, 4 * VITA_GXM_SCREEN_STRIDE * VITA_GXM_SCREEN_HEIGHT);
    sceCommonDialogUpdate(&updateParam);

    sceGxmDisplayQueueAddEntry(buffer_for_common_dialog[front_buffer_index_for_common_dialog].sync, buffer_for_common_dialog[back_buffer_index_for_common_dialog].sync, &buffer_for_common_dialog[back_buffer_index_for_common_dialog].displayData);
    front_buffer_index_for_common_dialog = back_buffer_index_for_common_dialog;
    back_buffer_index_for_common_dialog = (back_buffer_index_for_common_dialog + 1) % VITA_GXM_BUFFERS;
}

void gxm_term_for_common_dialog(void)
{
    sceGxmDisplayQueueFinish();
    for (int i = 0; i < VITA_GXM_BUFFERS; i += 1)
    {
        mem_gpu_free(buffer_for_common_dialog[i].uid);
        sceGxmSyncObjectDestroy(buffer_for_common_dialog[i].sync);
    }
}

#endif /* SDL_VIDEO_RENDER_VITA_GXM */

/* vi: set ts=4 sw=4 expandtab: */
