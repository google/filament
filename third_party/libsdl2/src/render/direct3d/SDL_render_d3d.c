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

#include "SDL_render.h"
#include "SDL_system.h"

#if SDL_VIDEO_RENDER_D3D && !SDL_RENDER_DISABLED

#include "../../core/windows/SDL_windows.h"

#include "SDL_hints.h"
#include "SDL_loadso.h"
#include "SDL_syswm.h"
#include "../SDL_sysrender.h"
#include "../SDL_d3dmath.h"
#include "../../video/windows/SDL_windowsvideo.h"

#if SDL_VIDEO_RENDER_D3D
#define D3D_DEBUG_INFO
#include <d3d9.h>
#endif

#include "SDL_shaders_d3d.h"


/* Direct3D renderer implementation */

static SDL_Renderer *D3D_CreateRenderer(SDL_Window * window, Uint32 flags);
static void D3D_WindowEvent(SDL_Renderer * renderer,
                            const SDL_WindowEvent *event);
static SDL_bool D3D_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode);
static int D3D_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int D3D_RecreateTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int D3D_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                             const SDL_Rect * rect, const void *pixels,
                             int pitch);
static int D3D_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                                const SDL_Rect * rect,
                                const Uint8 *Yplane, int Ypitch,
                                const Uint8 *Uplane, int Upitch,
                                const Uint8 *Vplane, int Vpitch);
static int D3D_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                           const SDL_Rect * rect, void **pixels, int *pitch);
static void D3D_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int D3D_SetRenderTargetInternal(SDL_Renderer * renderer, SDL_Texture * texture);
static int D3D_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture);
static int D3D_UpdateViewport(SDL_Renderer * renderer);
static int D3D_UpdateClipRect(SDL_Renderer * renderer);
static int D3D_RenderClear(SDL_Renderer * renderer);
static int D3D_RenderDrawPoints(SDL_Renderer * renderer,
                                const SDL_FPoint * points, int count);
static int D3D_RenderDrawLines(SDL_Renderer * renderer,
                               const SDL_FPoint * points, int count);
static int D3D_RenderFillRects(SDL_Renderer * renderer,
                               const SDL_FRect * rects, int count);
static int D3D_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                          const SDL_Rect * srcrect, const SDL_FRect * dstrect);
static int D3D_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                          const SDL_Rect * srcrect, const SDL_FRect * dstrect,
                          const double angle, const SDL_FPoint * center, const SDL_RendererFlip flip);
static int D3D_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                                Uint32 format, void * pixels, int pitch);
static void D3D_RenderPresent(SDL_Renderer * renderer);
static void D3D_DestroyTexture(SDL_Renderer * renderer,
                               SDL_Texture * texture);
static void D3D_DestroyRenderer(SDL_Renderer * renderer);


SDL_RenderDriver D3D_RenderDriver = {
    D3D_CreateRenderer,
    {
     "direct3d",
     (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE),
     1,
     {SDL_PIXELFORMAT_ARGB8888},
     0,
     0}
};

typedef struct
{
    void* d3dDLL;
    IDirect3D9 *d3d;
    IDirect3DDevice9 *device;
    UINT adapter;
    D3DPRESENT_PARAMETERS pparams;
    SDL_bool updateSize;
    SDL_bool beginScene;
    SDL_bool enableSeparateAlphaBlend;
    D3DTEXTUREFILTERTYPE scaleMode[8];
    IDirect3DSurface9 *defaultRenderTarget;
    IDirect3DSurface9 *currentRenderTarget;
    void* d3dxDLL;
    LPDIRECT3DPIXELSHADER9 shaders[NUM_SHADERS];
} D3D_RenderData;

typedef struct
{
    SDL_bool dirty;
    int w, h;
    DWORD usage;
    Uint32 format;
    D3DFORMAT d3dfmt;
    IDirect3DTexture9 *texture;
    IDirect3DTexture9 *staging;
} D3D_TextureRep;

typedef struct
{
    D3D_TextureRep texture;
    D3DTEXTUREFILTERTYPE scaleMode;

    /* YV12 texture support */
    SDL_bool yuv;
    D3D_TextureRep utexture;
    D3D_TextureRep vtexture;
    Uint8 *pixels;
    int pitch;
    SDL_Rect locked_rect;
} D3D_TextureData;

typedef struct
{
    float x, y, z;
    DWORD color;
    float u, v;
} Vertex;

static int
D3D_SetError(const char *prefix, HRESULT result)
{
    const char *error;

    switch (result) {
    case D3DERR_WRONGTEXTUREFORMAT:
        error = "WRONGTEXTUREFORMAT";
        break;
    case D3DERR_UNSUPPORTEDCOLOROPERATION:
        error = "UNSUPPORTEDCOLOROPERATION";
        break;
    case D3DERR_UNSUPPORTEDCOLORARG:
        error = "UNSUPPORTEDCOLORARG";
        break;
    case D3DERR_UNSUPPORTEDALPHAOPERATION:
        error = "UNSUPPORTEDALPHAOPERATION";
        break;
    case D3DERR_UNSUPPORTEDALPHAARG:
        error = "UNSUPPORTEDALPHAARG";
        break;
    case D3DERR_TOOMANYOPERATIONS:
        error = "TOOMANYOPERATIONS";
        break;
    case D3DERR_CONFLICTINGTEXTUREFILTER:
        error = "CONFLICTINGTEXTUREFILTER";
        break;
    case D3DERR_UNSUPPORTEDFACTORVALUE:
        error = "UNSUPPORTEDFACTORVALUE";
        break;
    case D3DERR_CONFLICTINGRENDERSTATE:
        error = "CONFLICTINGRENDERSTATE";
        break;
    case D3DERR_UNSUPPORTEDTEXTUREFILTER:
        error = "UNSUPPORTEDTEXTUREFILTER";
        break;
    case D3DERR_CONFLICTINGTEXTUREPALETTE:
        error = "CONFLICTINGTEXTUREPALETTE";
        break;
    case D3DERR_DRIVERINTERNALERROR:
        error = "DRIVERINTERNALERROR";
        break;
    case D3DERR_NOTFOUND:
        error = "NOTFOUND";
        break;
    case D3DERR_MOREDATA:
        error = "MOREDATA";
        break;
    case D3DERR_DEVICELOST:
        error = "DEVICELOST";
        break;
    case D3DERR_DEVICENOTRESET:
        error = "DEVICENOTRESET";
        break;
    case D3DERR_NOTAVAILABLE:
        error = "NOTAVAILABLE";
        break;
    case D3DERR_OUTOFVIDEOMEMORY:
        error = "OUTOFVIDEOMEMORY";
        break;
    case D3DERR_INVALIDDEVICE:
        error = "INVALIDDEVICE";
        break;
    case D3DERR_INVALIDCALL:
        error = "INVALIDCALL";
        break;
    case D3DERR_DRIVERINVALIDCALL:
        error = "DRIVERINVALIDCALL";
        break;
    case D3DERR_WASSTILLDRAWING:
        error = "WASSTILLDRAWING";
        break;
    default:
        error = "UNKNOWN";
        break;
    }
    return SDL_SetError("%s: %s", prefix, error);
}

static D3DFORMAT
PixelFormatToD3DFMT(Uint32 format)
{
    switch (format) {
    case SDL_PIXELFORMAT_RGB565:
        return D3DFMT_R5G6B5;
    case SDL_PIXELFORMAT_RGB888:
        return D3DFMT_X8R8G8B8;
    case SDL_PIXELFORMAT_ARGB8888:
        return D3DFMT_A8R8G8B8;
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        return D3DFMT_L8;
    default:
        return D3DFMT_UNKNOWN;
    }
}

static Uint32
D3DFMTToPixelFormat(D3DFORMAT format)
{
    switch (format) {
    case D3DFMT_R5G6B5:
        return SDL_PIXELFORMAT_RGB565;
    case D3DFMT_X8R8G8B8:
        return SDL_PIXELFORMAT_RGB888;
    case D3DFMT_A8R8G8B8:
        return SDL_PIXELFORMAT_ARGB8888;
    default:
        return SDL_PIXELFORMAT_UNKNOWN;
    }
}

static void
D3D_InitRenderState(D3D_RenderData *data)
{
    D3DMATRIX matrix;

    IDirect3DDevice9 *device = data->device;

    IDirect3DDevice9_SetVertexShader(device, NULL);
    IDirect3DDevice9_SetFVF(device, D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, D3DZB_FALSE);
    IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE);
    IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE);

    /* Enable color modulation by diffuse color */
    IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLOROP,
                                          D3DTOP_MODULATE);
    IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG1,
                                          D3DTA_TEXTURE);
    IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG2,
                                          D3DTA_DIFFUSE);

    /* Enable alpha modulation by diffuse alpha */
    IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAOP,
                                          D3DTOP_MODULATE);
    IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAARG1,
                                          D3DTA_TEXTURE);
    IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAARG2,
                                          D3DTA_DIFFUSE);

    /* Enable separate alpha blend function, if possible */
    if (data->enableSeparateAlphaBlend) {
        IDirect3DDevice9_SetRenderState(device, D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
    }

    /* Disable second texture stage, since we're done */
    IDirect3DDevice9_SetTextureStageState(device, 1, D3DTSS_COLOROP,
                                          D3DTOP_DISABLE);
    IDirect3DDevice9_SetTextureStageState(device, 1, D3DTSS_ALPHAOP,
                                          D3DTOP_DISABLE);

    /* Set an identity world and view matrix */
    matrix.m[0][0] = 1.0f;
    matrix.m[0][1] = 0.0f;
    matrix.m[0][2] = 0.0f;
    matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = 0.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[1][2] = 0.0f;
    matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = 0.0f;
    matrix.m[2][1] = 0.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = 0.0f;
    matrix.m[3][1] = 0.0f;
    matrix.m[3][2] = 0.0f;
    matrix.m[3][3] = 1.0f;
    IDirect3DDevice9_SetTransform(device, D3DTS_WORLD, &matrix);
    IDirect3DDevice9_SetTransform(device, D3DTS_VIEW, &matrix);

    /* Reset our current scale mode */
    SDL_memset(data->scaleMode, 0xFF, sizeof(data->scaleMode));

    /* Start the render with beginScene */
    data->beginScene = SDL_TRUE;
}

static int
D3D_Reset(SDL_Renderer * renderer)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    HRESULT result;
    SDL_Texture *texture;

    /* Release the default render target before reset */
    if (data->defaultRenderTarget) {
        IDirect3DSurface9_Release(data->defaultRenderTarget);
        data->defaultRenderTarget = NULL;
    }
    if (data->currentRenderTarget != NULL) {
        IDirect3DSurface9_Release(data->currentRenderTarget);
        data->currentRenderTarget = NULL;
    }

    /* Release application render targets */
    for (texture = renderer->textures; texture; texture = texture->next) {
        if (texture->access == SDL_TEXTUREACCESS_TARGET) {
            D3D_DestroyTexture(renderer, texture);
        } else {
            D3D_RecreateTexture(renderer, texture);
        }
    }

    result = IDirect3DDevice9_Reset(data->device, &data->pparams);
    if (FAILED(result)) {
        if (result == D3DERR_DEVICELOST) {
            /* Don't worry about it, we'll reset later... */
            return 0;
        } else {
            return D3D_SetError("Reset()", result);
        }
    }

    /* Allocate application render targets */
    for (texture = renderer->textures; texture; texture = texture->next) {
        if (texture->access == SDL_TEXTUREACCESS_TARGET) {
            D3D_CreateTexture(renderer, texture);
        }
    }

    IDirect3DDevice9_GetRenderTarget(data->device, 0, &data->defaultRenderTarget);
    D3D_InitRenderState(data);
    D3D_SetRenderTargetInternal(renderer, renderer->target);
    D3D_UpdateViewport(renderer);

    /* Let the application know that render targets were reset */
    {
        SDL_Event event;
        event.type = SDL_RENDER_TARGETS_RESET;
        SDL_PushEvent(&event);
    }

    return 0;
}

static int
D3D_ActivateRenderer(SDL_Renderer * renderer)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    HRESULT result;

    if (data->updateSize) {
        SDL_Window *window = renderer->window;
        int w, h;
        Uint32 window_flags = SDL_GetWindowFlags(window);

        SDL_GetWindowSize(window, &w, &h);
        data->pparams.BackBufferWidth = w;
        data->pparams.BackBufferHeight = h;
        if (window_flags & SDL_WINDOW_FULLSCREEN && (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP) {
            SDL_DisplayMode fullscreen_mode;
            SDL_GetWindowDisplayMode(window, &fullscreen_mode);
            data->pparams.Windowed = FALSE;
            data->pparams.BackBufferFormat = PixelFormatToD3DFMT(fullscreen_mode.format);
            data->pparams.FullScreen_RefreshRateInHz = fullscreen_mode.refresh_rate;
        } else {
            data->pparams.Windowed = TRUE;
            data->pparams.BackBufferFormat = D3DFMT_UNKNOWN;
            data->pparams.FullScreen_RefreshRateInHz = 0;
        }
        if (D3D_Reset(renderer) < 0) {
            return -1;
        }

        data->updateSize = SDL_FALSE;
    }
    if (data->beginScene) {
        result = IDirect3DDevice9_BeginScene(data->device);
        if (result == D3DERR_DEVICELOST) {
            if (D3D_Reset(renderer) < 0) {
                return -1;
            }
            result = IDirect3DDevice9_BeginScene(data->device);
        }
        if (FAILED(result)) {
            return D3D_SetError("BeginScene()", result);
        }
        data->beginScene = SDL_FALSE;
    }
    return 0;
}

SDL_Renderer *
D3D_CreateRenderer(SDL_Window * window, Uint32 flags)
{
    SDL_Renderer *renderer;
    D3D_RenderData *data;
    SDL_SysWMinfo windowinfo;
    HRESULT result;
    D3DPRESENT_PARAMETERS pparams;
    IDirect3DSwapChain9 *chain;
    D3DCAPS9 caps;
    DWORD device_flags;
    Uint32 window_flags;
    int w, h;
    SDL_DisplayMode fullscreen_mode;
    int displayIndex;

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (D3D_RenderData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        SDL_free(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    if (!D3D_LoadDLL(&data->d3dDLL, &data->d3d)) {
        SDL_free(renderer);
        SDL_free(data);
        SDL_SetError("Unable to create Direct3D interface");
        return NULL;
    }

    renderer->WindowEvent = D3D_WindowEvent;
    renderer->SupportsBlendMode = D3D_SupportsBlendMode;
    renderer->CreateTexture = D3D_CreateTexture;
    renderer->UpdateTexture = D3D_UpdateTexture;
    renderer->UpdateTextureYUV = D3D_UpdateTextureYUV;
    renderer->LockTexture = D3D_LockTexture;
    renderer->UnlockTexture = D3D_UnlockTexture;
    renderer->SetRenderTarget = D3D_SetRenderTarget;
    renderer->UpdateViewport = D3D_UpdateViewport;
    renderer->UpdateClipRect = D3D_UpdateClipRect;
    renderer->RenderClear = D3D_RenderClear;
    renderer->RenderDrawPoints = D3D_RenderDrawPoints;
    renderer->RenderDrawLines = D3D_RenderDrawLines;
    renderer->RenderFillRects = D3D_RenderFillRects;
    renderer->RenderCopy = D3D_RenderCopy;
    renderer->RenderCopyEx = D3D_RenderCopyEx;
    renderer->RenderReadPixels = D3D_RenderReadPixels;
    renderer->RenderPresent = D3D_RenderPresent;
    renderer->DestroyTexture = D3D_DestroyTexture;
    renderer->DestroyRenderer = D3D_DestroyRenderer;
    renderer->info = D3D_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;

    SDL_VERSION(&windowinfo.version);
    SDL_GetWindowWMInfo(window, &windowinfo);

    window_flags = SDL_GetWindowFlags(window);
    SDL_GetWindowSize(window, &w, &h);
    SDL_GetWindowDisplayMode(window, &fullscreen_mode);

    SDL_zero(pparams);
    pparams.hDeviceWindow = windowinfo.info.win.window;
    pparams.BackBufferWidth = w;
    pparams.BackBufferHeight = h;
    pparams.BackBufferCount = 1;
    pparams.SwapEffect = D3DSWAPEFFECT_DISCARD;

    if (window_flags & SDL_WINDOW_FULLSCREEN && (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP) {
        pparams.Windowed = FALSE;
        pparams.BackBufferFormat = PixelFormatToD3DFMT(fullscreen_mode.format);
        pparams.FullScreen_RefreshRateInHz = fullscreen_mode.refresh_rate;
    } else {
        pparams.Windowed = TRUE;
        pparams.BackBufferFormat = D3DFMT_UNKNOWN;
        pparams.FullScreen_RefreshRateInHz = 0;
    }
    if (flags & SDL_RENDERER_PRESENTVSYNC) {
        pparams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    } else {
        pparams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }

    /* Get the adapter for the display that the window is on */
    displayIndex = SDL_GetWindowDisplayIndex(window);
    data->adapter = SDL_Direct3D9GetAdapterIndex(displayIndex);

    IDirect3D9_GetDeviceCaps(data->d3d, data->adapter, D3DDEVTYPE_HAL, &caps);

    device_flags = D3DCREATE_FPU_PRESERVE;
    if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
        device_flags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    } else {
        device_flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    if (SDL_GetHintBoolean(SDL_HINT_RENDER_DIRECT3D_THREADSAFE, SDL_FALSE)) {
        device_flags |= D3DCREATE_MULTITHREADED;
    }

    result = IDirect3D9_CreateDevice(data->d3d, data->adapter,
                                     D3DDEVTYPE_HAL,
                                     pparams.hDeviceWindow,
                                     device_flags,
                                     &pparams, &data->device);
    if (FAILED(result)) {
        D3D_DestroyRenderer(renderer);
        D3D_SetError("CreateDevice()", result);
        return NULL;
    }

    /* Get presentation parameters to fill info */
    result = IDirect3DDevice9_GetSwapChain(data->device, 0, &chain);
    if (FAILED(result)) {
        D3D_DestroyRenderer(renderer);
        D3D_SetError("GetSwapChain()", result);
        return NULL;
    }
    result = IDirect3DSwapChain9_GetPresentParameters(chain, &pparams);
    if (FAILED(result)) {
        IDirect3DSwapChain9_Release(chain);
        D3D_DestroyRenderer(renderer);
        D3D_SetError("GetPresentParameters()", result);
        return NULL;
    }
    IDirect3DSwapChain9_Release(chain);
    if (pparams.PresentationInterval == D3DPRESENT_INTERVAL_ONE) {
        renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
    }
    data->pparams = pparams;

    IDirect3DDevice9_GetDeviceCaps(data->device, &caps);
    renderer->info.max_texture_width = caps.MaxTextureWidth;
    renderer->info.max_texture_height = caps.MaxTextureHeight;
    if (caps.NumSimultaneousRTs >= 2) {
        renderer->info.flags |= SDL_RENDERER_TARGETTEXTURE;
    }

    if (caps.PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND) {
        data->enableSeparateAlphaBlend = SDL_TRUE;
    }

    /* Store the default render target */
    IDirect3DDevice9_GetRenderTarget(data->device, 0, &data->defaultRenderTarget);
    data->currentRenderTarget = NULL;

    /* Set up parameters for rendering */
    D3D_InitRenderState(data);

    if (caps.MaxSimultaneousTextures >= 3) {
        int i;
        for (i = 0; i < SDL_arraysize(data->shaders); ++i) {
            result = D3D9_CreatePixelShader(data->device, (D3D9_Shader)i, &data->shaders[i]);
            if (FAILED(result)) {
                D3D_SetError("CreatePixelShader()", result);
            }
        }
        if (data->shaders[SHADER_YUV_JPEG] && data->shaders[SHADER_YUV_BT601] && data->shaders[SHADER_YUV_BT709]) {
            renderer->info.texture_formats[renderer->info.num_texture_formats++] = SDL_PIXELFORMAT_YV12;
            renderer->info.texture_formats[renderer->info.num_texture_formats++] = SDL_PIXELFORMAT_IYUV;
        }
    }
    return renderer;
}

static void
D3D_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;

    if (event->event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        data->updateSize = SDL_TRUE;
    }
}

static D3DBLEND GetBlendFunc(SDL_BlendFactor factor)
{
    switch (factor) {
    case SDL_BLENDFACTOR_ZERO:
        return D3DBLEND_ZERO;
    case SDL_BLENDFACTOR_ONE:
        return D3DBLEND_ONE;
    case SDL_BLENDFACTOR_SRC_COLOR:
        return D3DBLEND_SRCCOLOR;
    case SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR:
        return D3DBLEND_INVSRCCOLOR;
    case SDL_BLENDFACTOR_SRC_ALPHA:
        return D3DBLEND_SRCALPHA;
    case SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA:
        return D3DBLEND_INVSRCALPHA;
    case SDL_BLENDFACTOR_DST_COLOR:
        return D3DBLEND_DESTCOLOR;
    case SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR:
        return D3DBLEND_INVDESTCOLOR;
    case SDL_BLENDFACTOR_DST_ALPHA:
        return D3DBLEND_DESTALPHA;
    case SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA:
        return D3DBLEND_INVDESTALPHA;
    default:
        return (D3DBLEND)0;
    }
}

static SDL_bool
D3D_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    SDL_BlendFactor srcColorFactor = SDL_GetBlendModeSrcColorFactor(blendMode);
    SDL_BlendFactor srcAlphaFactor = SDL_GetBlendModeSrcAlphaFactor(blendMode);
    SDL_BlendOperation colorOperation = SDL_GetBlendModeColorOperation(blendMode);
    SDL_BlendFactor dstColorFactor = SDL_GetBlendModeDstColorFactor(blendMode);
    SDL_BlendFactor dstAlphaFactor = SDL_GetBlendModeDstAlphaFactor(blendMode);
    SDL_BlendOperation alphaOperation = SDL_GetBlendModeAlphaOperation(blendMode);

    if (!GetBlendFunc(srcColorFactor) || !GetBlendFunc(srcAlphaFactor) ||
        !GetBlendFunc(dstColorFactor) || !GetBlendFunc(dstAlphaFactor)) {
        return SDL_FALSE;
    }
    if ((srcColorFactor != srcAlphaFactor || dstColorFactor != dstAlphaFactor) && !data->enableSeparateAlphaBlend) {
        return SDL_FALSE;
    }
    if (colorOperation != SDL_BLENDOPERATION_ADD || alphaOperation != SDL_BLENDOPERATION_ADD) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static D3DTEXTUREFILTERTYPE
GetScaleQuality(void)
{
    const char *hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);

    if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
        return D3DTEXF_POINT;
    } else /* if (*hint == '1' || SDL_strcasecmp(hint, "linear") == 0) */ {
        return D3DTEXF_LINEAR;
    }
}

static int
D3D_CreateTextureRep(IDirect3DDevice9 *device, D3D_TextureRep *texture, DWORD usage, Uint32 format, D3DFORMAT d3dfmt, int w, int h)
{
    HRESULT result;

    texture->dirty = SDL_FALSE;
    texture->w = w;
    texture->h = h;
    texture->usage = usage;
    texture->format = format;
    texture->d3dfmt = d3dfmt;

    result = IDirect3DDevice9_CreateTexture(device, w, h, 1, usage,
        PixelFormatToD3DFMT(format),
        D3DPOOL_DEFAULT, &texture->texture, NULL);
    if (FAILED(result)) {
        return D3D_SetError("CreateTexture(D3DPOOL_DEFAULT)", result);
    }
    return 0;
}


static int
D3D_CreateStagingTexture(IDirect3DDevice9 *device, D3D_TextureRep *texture)
{
    HRESULT result;

    if (texture->staging == NULL) {
        result = IDirect3DDevice9_CreateTexture(device, texture->w, texture->h, 1, 0,
            texture->d3dfmt, D3DPOOL_SYSTEMMEM, &texture->staging, NULL);
        if (FAILED(result)) {
            return D3D_SetError("CreateTexture(D3DPOOL_SYSTEMMEM)", result);
        }
    }
    return 0;
}

static int
D3D_BindTextureRep(IDirect3DDevice9 *device, D3D_TextureRep *texture, DWORD sampler)
{
    HRESULT result;

    if (texture->dirty && texture->staging) {
        if (!texture->texture) {
            result = IDirect3DDevice9_CreateTexture(device, texture->w, texture->h, 1, texture->usage,
                PixelFormatToD3DFMT(texture->format), D3DPOOL_DEFAULT, &texture->texture, NULL);
            if (FAILED(result)) {
                return D3D_SetError("CreateTexture(D3DPOOL_DEFAULT)", result);
            }
        }

        result = IDirect3DDevice9_UpdateTexture(device, (IDirect3DBaseTexture9 *)texture->staging, (IDirect3DBaseTexture9 *)texture->texture);
        if (FAILED(result)) {
            return D3D_SetError("UpdateTexture()", result);
        }
        texture->dirty = SDL_FALSE;
    }
    result = IDirect3DDevice9_SetTexture(device, sampler, (IDirect3DBaseTexture9 *)texture->texture);
    if (FAILED(result)) {
        return D3D_SetError("SetTexture()", result);
    }
    return 0;
}

static int
D3D_RecreateTextureRep(IDirect3DDevice9 *device, D3D_TextureRep *texture)
{
    if (texture->texture) {
        IDirect3DTexture9_Release(texture->texture);
        texture->texture = NULL;
    }
    if (texture->staging) {
        IDirect3DTexture9_AddDirtyRect(texture->staging, NULL);
        texture->dirty = SDL_TRUE;
    }
    return 0;
}

static int
D3D_UpdateTextureRep(IDirect3DDevice9 *device, D3D_TextureRep *texture, int x, int y, int w, int h, const void *pixels, int pitch)
{
    RECT d3drect;
    D3DLOCKED_RECT locked;
    const Uint8 *src;
    Uint8 *dst;
    int row, length;
    HRESULT result;

    if (D3D_CreateStagingTexture(device, texture) < 0) {
        return -1;
    }

    d3drect.left = x;
    d3drect.right = x + w;
    d3drect.top = y;
    d3drect.bottom = y + h;
    
    result = IDirect3DTexture9_LockRect(texture->staging, 0, &locked, &d3drect, 0);
    if (FAILED(result)) {
        return D3D_SetError("LockRect()", result);
    }

    src = (const Uint8 *)pixels;
    dst = (Uint8 *)locked.pBits;
    length = w * SDL_BYTESPERPIXEL(texture->format);
    if (length == pitch && length == locked.Pitch) {
        SDL_memcpy(dst, src, length*h);
    } else {
        if (length > pitch) {
            length = pitch;
        }
        if (length > locked.Pitch) {
            length = locked.Pitch;
        }
        for (row = 0; row < h; ++row) {
            SDL_memcpy(dst, src, length);
            src += pitch;
            dst += locked.Pitch;
        }
    }
    result = IDirect3DTexture9_UnlockRect(texture->staging, 0);
    if (FAILED(result)) {
        return D3D_SetError("UnlockRect()", result);
    }
    texture->dirty = SDL_TRUE;

    return 0;
}

static void
D3D_DestroyTextureRep(D3D_TextureRep *texture)
{
    if (texture->texture) {
        IDirect3DTexture9_Release(texture->texture);
        texture->texture = NULL;
    }
    if (texture->staging) {
        IDirect3DTexture9_Release(texture->staging);
        texture->staging = NULL;
    }
}

static int
D3D_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    D3D_TextureData *texturedata;
    DWORD usage;

    texturedata = (D3D_TextureData *) SDL_calloc(1, sizeof(*texturedata));
    if (!texturedata) {
        return SDL_OutOfMemory();
    }
    texturedata->scaleMode = GetScaleQuality();

    texture->driverdata = texturedata;

    if (texture->access == SDL_TEXTUREACCESS_TARGET) {
        usage = D3DUSAGE_RENDERTARGET;
    } else {
        usage = 0;
    }

    if (D3D_CreateTextureRep(data->device, &texturedata->texture, usage, texture->format, PixelFormatToD3DFMT(texture->format), texture->w, texture->h) < 0) {
        return -1;
    }

    if (texture->format == SDL_PIXELFORMAT_YV12 ||
        texture->format == SDL_PIXELFORMAT_IYUV) {
        texturedata->yuv = SDL_TRUE;

        if (D3D_CreateTextureRep(data->device, &texturedata->utexture, usage, texture->format, PixelFormatToD3DFMT(texture->format), (texture->w + 1) / 2, (texture->h + 1) / 2) < 0) {
            return -1;
        }

        if (D3D_CreateTextureRep(data->device, &texturedata->vtexture, usage, texture->format, PixelFormatToD3DFMT(texture->format), (texture->w + 1) / 2, (texture->h + 1) / 2) < 0) {
            return -1;
        }
    }
    return 0;
}

static int
D3D_RecreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    D3D_RenderData *data = (D3D_RenderData *)renderer->driverdata;
    D3D_TextureData *texturedata = (D3D_TextureData *)texture->driverdata;

    if (!texturedata) {
        return 0;
    }

    if (D3D_RecreateTextureRep(data->device, &texturedata->texture) < 0) {
        return -1;
    }

    if (texturedata->yuv) {
        if (D3D_RecreateTextureRep(data->device, &texturedata->utexture) < 0) {
            return -1;
        }

        if (D3D_RecreateTextureRep(data->device, &texturedata->vtexture) < 0) {
            return -1;
        }
    }
    return 0;
}

static int
D3D_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                  const SDL_Rect * rect, const void *pixels, int pitch)
{
    D3D_RenderData *data = (D3D_RenderData *)renderer->driverdata;
    D3D_TextureData *texturedata = (D3D_TextureData *) texture->driverdata;

    if (!texturedata) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    if (D3D_UpdateTextureRep(data->device, &texturedata->texture, rect->x, rect->y, rect->w, rect->h, pixels, pitch) < 0) {
        return -1;
    }

    if (texturedata->yuv) {
        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + rect->h * pitch);

        if (D3D_UpdateTextureRep(data->device, texture->format == SDL_PIXELFORMAT_YV12 ? &texturedata->vtexture : &texturedata->utexture, rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2, pixels, (pitch + 1) / 2) < 0) {
            return -1;
        }

        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + ((rect->h + 1) / 2) * ((pitch + 1) / 2));
        if (D3D_UpdateTextureRep(data->device, texture->format == SDL_PIXELFORMAT_YV12 ? &texturedata->utexture : &texturedata->vtexture, rect->x / 2, (rect->y + 1) / 2, (rect->w + 1) / 2, (rect->h + 1) / 2, pixels, (pitch + 1) / 2) < 0) {
            return -1;
        }
    }
    return 0;
}

static int
D3D_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                     const SDL_Rect * rect,
                     const Uint8 *Yplane, int Ypitch,
                     const Uint8 *Uplane, int Upitch,
                     const Uint8 *Vplane, int Vpitch)
{
    D3D_RenderData *data = (D3D_RenderData *)renderer->driverdata;
    D3D_TextureData *texturedata = (D3D_TextureData *) texture->driverdata;

    if (!texturedata) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    if (D3D_UpdateTextureRep(data->device, &texturedata->texture, rect->x, rect->y, rect->w, rect->h, Yplane, Ypitch) < 0) {
        return -1;
    }
    if (D3D_UpdateTextureRep(data->device, &texturedata->utexture, rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2, Uplane, Upitch) < 0) {
        return -1;
    }
    if (D3D_UpdateTextureRep(data->device, &texturedata->vtexture, rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2, Vplane, Vpitch) < 0) {
        return -1;
    }
    return 0;
}

static int
D3D_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                const SDL_Rect * rect, void **pixels, int *pitch)
{
    D3D_RenderData *data = (D3D_RenderData *)renderer->driverdata;
    D3D_TextureData *texturedata = (D3D_TextureData *)texture->driverdata;
    IDirect3DDevice9 *device = data->device;

    if (!texturedata) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    texturedata->locked_rect = *rect;

    if (texturedata->yuv) {
        /* It's more efficient to upload directly... */
        if (!texturedata->pixels) {
            texturedata->pitch = texture->w;
            texturedata->pixels = (Uint8 *)SDL_malloc((texture->h * texturedata->pitch * 3) / 2);
            if (!texturedata->pixels) {
                return SDL_OutOfMemory();
            }
        }
        *pixels =
            (void *) ((Uint8 *) texturedata->pixels + rect->y * texturedata->pitch +
                      rect->x * SDL_BYTESPERPIXEL(texture->format));
        *pitch = texturedata->pitch;
    } else {
        RECT d3drect;
        D3DLOCKED_RECT locked;
        HRESULT result;

        if (D3D_CreateStagingTexture(device, &texturedata->texture) < 0) {
            return -1;
        }

        d3drect.left = rect->x;
        d3drect.right = rect->x + rect->w;
        d3drect.top = rect->y;
        d3drect.bottom = rect->y + rect->h;

        result = IDirect3DTexture9_LockRect(texturedata->texture.staging, 0, &locked, &d3drect, 0);
        if (FAILED(result)) {
            return D3D_SetError("LockRect()", result);
        }
        *pixels = locked.pBits;
        *pitch = locked.Pitch;
    }
    return 0;
}

static void
D3D_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    /*D3D_RenderData *data = (D3D_RenderData *)renderer->driverdata;*/
    D3D_TextureData *texturedata = (D3D_TextureData *)texture->driverdata;

    if (!texturedata) {
        return;
    }

    if (texturedata->yuv) {
        const SDL_Rect *rect = &texturedata->locked_rect;
        void *pixels =
            (void *) ((Uint8 *) texturedata->pixels + rect->y * texturedata->pitch +
                      rect->x * SDL_BYTESPERPIXEL(texture->format));
        D3D_UpdateTexture(renderer, texture, rect, pixels, texturedata->pitch);
    } else {
        IDirect3DTexture9_UnlockRect(texturedata->texture.staging, 0);
        texturedata->texture.dirty = SDL_TRUE;
   }
}

static int
D3D_SetRenderTargetInternal(SDL_Renderer * renderer, SDL_Texture * texture)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    D3D_TextureData *texturedata;
    D3D_TextureRep *texturerep;
    HRESULT result;
    IDirect3DDevice9 *device = data->device;

    /* Release the previous render target if it wasn't the default one */
    if (data->currentRenderTarget != NULL) {
        IDirect3DSurface9_Release(data->currentRenderTarget);
        data->currentRenderTarget = NULL;
    }

    if (texture == NULL) {
        IDirect3DDevice9_SetRenderTarget(data->device, 0, data->defaultRenderTarget);
        return 0;
    }

    texturedata = (D3D_TextureData *)texture->driverdata;
    if (!texturedata) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    /* Make sure the render target is updated if it was locked and written to */
    texturerep = &texturedata->texture;
    if (texturerep->dirty && texturerep->staging) {
        if (!texturerep->texture) {
            result = IDirect3DDevice9_CreateTexture(device, texturerep->w, texturerep->h, 1, texturerep->usage,
                PixelFormatToD3DFMT(texturerep->format), D3DPOOL_DEFAULT, &texturerep->texture, NULL);
            if (FAILED(result)) {
                return D3D_SetError("CreateTexture(D3DPOOL_DEFAULT)", result);
            }
        }

        result = IDirect3DDevice9_UpdateTexture(device, (IDirect3DBaseTexture9 *)texturerep->staging, (IDirect3DBaseTexture9 *)texturerep->texture);
        if (FAILED(result)) {
            return D3D_SetError("UpdateTexture()", result);
        }
        texturerep->dirty = SDL_FALSE;
    }

    result = IDirect3DTexture9_GetSurfaceLevel(texturedata->texture.texture, 0, &data->currentRenderTarget);
    if(FAILED(result)) {
        return D3D_SetError("GetSurfaceLevel()", result);
    }
    result = IDirect3DDevice9_SetRenderTarget(data->device, 0, data->currentRenderTarget);
    if(FAILED(result)) {
        return D3D_SetError("SetRenderTarget()", result);
    }

    return 0;
}

static int
D3D_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture)
{
    if (D3D_ActivateRenderer(renderer) < 0) {
        return -1;
    }

    return D3D_SetRenderTargetInternal(renderer, texture);
}

static int
D3D_UpdateViewport(SDL_Renderer * renderer)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    D3DVIEWPORT9 viewport;
    D3DMATRIX matrix;

    /* Set the viewport */
    viewport.X = renderer->viewport.x;
    viewport.Y = renderer->viewport.y;
    viewport.Width = renderer->viewport.w;
    viewport.Height = renderer->viewport.h;
    viewport.MinZ = 0.0f;
    viewport.MaxZ = 1.0f;
    IDirect3DDevice9_SetViewport(data->device, &viewport);

    /* Set an orthographic projection matrix */
    if (renderer->viewport.w && renderer->viewport.h) {
        matrix.m[0][0] = 2.0f / renderer->viewport.w;
        matrix.m[0][1] = 0.0f;
        matrix.m[0][2] = 0.0f;
        matrix.m[0][3] = 0.0f;
        matrix.m[1][0] = 0.0f;
        matrix.m[1][1] = -2.0f / renderer->viewport.h;
        matrix.m[1][2] = 0.0f;
        matrix.m[1][3] = 0.0f;
        matrix.m[2][0] = 0.0f;
        matrix.m[2][1] = 0.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[2][3] = 0.0f;
        matrix.m[3][0] = -1.0f;
        matrix.m[3][1] = 1.0f;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        IDirect3DDevice9_SetTransform(data->device, D3DTS_PROJECTION, &matrix);
    }

    return 0;
}

static int
D3D_UpdateClipRect(SDL_Renderer * renderer)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;

    if (renderer->clipping_enabled) {
        const SDL_Rect *rect = &renderer->clip_rect;
        RECT r;
        HRESULT result;

        IDirect3DDevice9_SetRenderState(data->device, D3DRS_SCISSORTESTENABLE, TRUE);
        r.left = renderer->viewport.x + rect->x;
        r.top = renderer->viewport.y + rect->y;
        r.right = renderer->viewport.x + rect->x + rect->w;
        r.bottom = renderer->viewport.y + rect->y + rect->h;

        result = IDirect3DDevice9_SetScissorRect(data->device, &r);
        if (result != D3D_OK) {
            D3D_SetError("SetScissor()", result);
            return -1;
        }
    } else {
        IDirect3DDevice9_SetRenderState(data->device, D3DRS_SCISSORTESTENABLE, FALSE);
    }
    return 0;
}

static int
D3D_RenderClear(SDL_Renderer * renderer)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    DWORD color;
    HRESULT result;
    int BackBufferWidth, BackBufferHeight;

    if (D3D_ActivateRenderer(renderer) < 0) {
        return -1;
    }

    color = D3DCOLOR_ARGB(renderer->a, renderer->r, renderer->g, renderer->b);

    if (renderer->target) {
        BackBufferWidth = renderer->target->w;
        BackBufferHeight = renderer->target->h;
    } else {
        BackBufferWidth = data->pparams.BackBufferWidth;
        BackBufferHeight = data->pparams.BackBufferHeight;
    }

    if (renderer->clipping_enabled) {
        IDirect3DDevice9_SetRenderState(data->device, D3DRS_SCISSORTESTENABLE, FALSE);
    }

    /* Don't reset the viewport if we don't have to! */
    if (!renderer->viewport.x && !renderer->viewport.y &&
        renderer->viewport.w == BackBufferWidth &&
        renderer->viewport.h == BackBufferHeight) {
        result = IDirect3DDevice9_Clear(data->device, 0, NULL, D3DCLEAR_TARGET, color, 0.0f, 0);
    } else {
        D3DVIEWPORT9 viewport;

        /* Clear is defined to clear the entire render target */
        viewport.X = 0;
        viewport.Y = 0;
        viewport.Width = BackBufferWidth;
        viewport.Height = BackBufferHeight;
        viewport.MinZ = 0.0f;
        viewport.MaxZ = 1.0f;
        IDirect3DDevice9_SetViewport(data->device, &viewport);

        result = IDirect3DDevice9_Clear(data->device, 0, NULL, D3DCLEAR_TARGET, color, 0.0f, 0);

        /* Reset the viewport */
        viewport.X = renderer->viewport.x;
        viewport.Y = renderer->viewport.y;
        viewport.Width = renderer->viewport.w;
        viewport.Height = renderer->viewport.h;
        viewport.MinZ = 0.0f;
        viewport.MaxZ = 1.0f;
        IDirect3DDevice9_SetViewport(data->device, &viewport);
    }

    if (renderer->clipping_enabled) {
        IDirect3DDevice9_SetRenderState(data->device, D3DRS_SCISSORTESTENABLE, TRUE);
    }

    if (FAILED(result)) {
        return D3D_SetError("Clear()", result);
    }
    return 0;
}

static void
D3D_SetBlendMode(D3D_RenderData * data, SDL_BlendMode blendMode)
{
    if (blendMode == SDL_BLENDMODE_NONE) {
        IDirect3DDevice9_SetRenderState(data->device, D3DRS_ALPHABLENDENABLE, FALSE);
    } else {
        IDirect3DDevice9_SetRenderState(data->device, D3DRS_ALPHABLENDENABLE, TRUE);
        IDirect3DDevice9_SetRenderState(data->device, D3DRS_SRCBLEND,
                                        GetBlendFunc(SDL_GetBlendModeSrcColorFactor(blendMode)));
        IDirect3DDevice9_SetRenderState(data->device, D3DRS_DESTBLEND,
                                        GetBlendFunc(SDL_GetBlendModeDstColorFactor(blendMode)));
        if (data->enableSeparateAlphaBlend) {
            IDirect3DDevice9_SetRenderState(data->device, D3DRS_SRCBLENDALPHA,
                                            GetBlendFunc(SDL_GetBlendModeSrcAlphaFactor(blendMode)));
            IDirect3DDevice9_SetRenderState(data->device, D3DRS_DESTBLENDALPHA,
                                            GetBlendFunc(SDL_GetBlendModeDstAlphaFactor(blendMode)));
        }
    }
}

static int
D3D_RenderDrawPoints(SDL_Renderer * renderer, const SDL_FPoint * points,
                     int count)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    DWORD color;
    Vertex *vertices;
    int i;
    HRESULT result;

    if (D3D_ActivateRenderer(renderer) < 0) {
        return -1;
    }

    D3D_SetBlendMode(data, renderer->blendMode);

    result =
        IDirect3DDevice9_SetTexture(data->device, 0,
                                    (IDirect3DBaseTexture9 *) 0);
    if (FAILED(result)) {
        return D3D_SetError("SetTexture()", result);
    }

    color = D3DCOLOR_ARGB(renderer->a, renderer->r, renderer->g, renderer->b);

    vertices = SDL_stack_alloc(Vertex, count);
    for (i = 0; i < count; ++i) {
        vertices[i].x = points[i].x;
        vertices[i].y = points[i].y;
        vertices[i].z = 0.0f;
        vertices[i].color = color;
        vertices[i].u = 0.0f;
        vertices[i].v = 0.0f;
    }
    result =
        IDirect3DDevice9_DrawPrimitiveUP(data->device, D3DPT_POINTLIST, count,
                                         vertices, sizeof(*vertices));
    SDL_stack_free(vertices);
    if (FAILED(result)) {
        return D3D_SetError("DrawPrimitiveUP()", result);
    }
    return 0;
}

static int
D3D_RenderDrawLines(SDL_Renderer * renderer, const SDL_FPoint * points,
                    int count)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    DWORD color;
    Vertex *vertices;
    int i;
    HRESULT result;

    if (D3D_ActivateRenderer(renderer) < 0) {
        return -1;
    }

    D3D_SetBlendMode(data, renderer->blendMode);

    result =
        IDirect3DDevice9_SetTexture(data->device, 0,
                                    (IDirect3DBaseTexture9 *) 0);
    if (FAILED(result)) {
        return D3D_SetError("SetTexture()", result);
    }

    color = D3DCOLOR_ARGB(renderer->a, renderer->r, renderer->g, renderer->b);

    vertices = SDL_stack_alloc(Vertex, count);
    for (i = 0; i < count; ++i) {
        vertices[i].x = points[i].x;
        vertices[i].y = points[i].y;
        vertices[i].z = 0.0f;
        vertices[i].color = color;
        vertices[i].u = 0.0f;
        vertices[i].v = 0.0f;
    }
    result =
        IDirect3DDevice9_DrawPrimitiveUP(data->device, D3DPT_LINESTRIP, count-1,
                                         vertices, sizeof(*vertices));

    /* DirectX 9 has the same line rasterization semantics as GDI,
       so we need to close the endpoint of the line */
    if (count == 2 ||
        points[0].x != points[count-1].x || points[0].y != points[count-1].y) {
        vertices[0].x = points[count-1].x;
        vertices[0].y = points[count-1].y;
        result = IDirect3DDevice9_DrawPrimitiveUP(data->device, D3DPT_POINTLIST, 1, vertices, sizeof(*vertices));
    }

    SDL_stack_free(vertices);
    if (FAILED(result)) {
        return D3D_SetError("DrawPrimitiveUP()", result);
    }
    return 0;
}

static int
D3D_RenderFillRects(SDL_Renderer * renderer, const SDL_FRect * rects,
                    int count)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    DWORD color;
    int i;
    float minx, miny, maxx, maxy;
    Vertex vertices[4];
    HRESULT result;

    if (D3D_ActivateRenderer(renderer) < 0) {
        return -1;
    }

    D3D_SetBlendMode(data, renderer->blendMode);

    result =
        IDirect3DDevice9_SetTexture(data->device, 0,
                                    (IDirect3DBaseTexture9 *) 0);
    if (FAILED(result)) {
        return D3D_SetError("SetTexture()", result);
    }

    color = D3DCOLOR_ARGB(renderer->a, renderer->r, renderer->g, renderer->b);

    for (i = 0; i < count; ++i) {
        const SDL_FRect *rect = &rects[i];

        minx = rect->x;
        miny = rect->y;
        maxx = rect->x + rect->w;
        maxy = rect->y + rect->h;

        vertices[0].x = minx;
        vertices[0].y = miny;
        vertices[0].z = 0.0f;
        vertices[0].color = color;
        vertices[0].u = 0.0f;
        vertices[0].v = 0.0f;

        vertices[1].x = maxx;
        vertices[1].y = miny;
        vertices[1].z = 0.0f;
        vertices[1].color = color;
        vertices[1].u = 0.0f;
        vertices[1].v = 0.0f;

        vertices[2].x = maxx;
        vertices[2].y = maxy;
        vertices[2].z = 0.0f;
        vertices[2].color = color;
        vertices[2].u = 0.0f;
        vertices[2].v = 0.0f;

        vertices[3].x = minx;
        vertices[3].y = maxy;
        vertices[3].z = 0.0f;
        vertices[3].color = color;
        vertices[3].u = 0.0f;
        vertices[3].v = 0.0f;

        result =
            IDirect3DDevice9_DrawPrimitiveUP(data->device, D3DPT_TRIANGLEFAN,
                                             2, vertices, sizeof(*vertices));
        if (FAILED(result)) {
            return D3D_SetError("DrawPrimitiveUP()", result);
        }
    }
    return 0;
}

static void
D3D_UpdateTextureScaleMode(D3D_RenderData *data, D3D_TextureData *texturedata, unsigned index)
{
    if (texturedata->scaleMode != data->scaleMode[index]) {
        IDirect3DDevice9_SetSamplerState(data->device, index, D3DSAMP_MINFILTER,
                                         texturedata->scaleMode);
        IDirect3DDevice9_SetSamplerState(data->device, index, D3DSAMP_MAGFILTER,
                                         texturedata->scaleMode);
        IDirect3DDevice9_SetSamplerState(data->device, index, D3DSAMP_ADDRESSU,
                                         D3DTADDRESS_CLAMP);
        IDirect3DDevice9_SetSamplerState(data->device, index, D3DSAMP_ADDRESSV,
                                         D3DTADDRESS_CLAMP);
        data->scaleMode[index] = texturedata->scaleMode;
    }
}

static int
D3D_RenderSetupTextureState(SDL_Renderer * renderer, SDL_Texture * texture, LPDIRECT3DPIXELSHADER9 *shader)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    D3D_TextureData *texturedata;

    *shader = NULL;

    texturedata = (D3D_TextureData *)texture->driverdata;
    if (!texturedata) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    D3D_UpdateTextureScaleMode(data, texturedata, 0);

    if (D3D_BindTextureRep(data->device, &texturedata->texture, 0) < 0) {
        return -1;
    }

    if (texturedata->yuv) {
        switch (SDL_GetYUVConversionModeForResolution(texture->w, texture->h)) {
        case SDL_YUV_CONVERSION_JPEG:
            *shader = data->shaders[SHADER_YUV_JPEG];
            break;
        case SDL_YUV_CONVERSION_BT601:
            *shader = data->shaders[SHADER_YUV_BT601];
            break;
        case SDL_YUV_CONVERSION_BT709:
            *shader = data->shaders[SHADER_YUV_BT709];
            break;
        default:
            return SDL_SetError("Unsupported YUV conversion mode");
        }

        D3D_UpdateTextureScaleMode(data, texturedata, 1);
        D3D_UpdateTextureScaleMode(data, texturedata, 2);

        if (D3D_BindTextureRep(data->device, &texturedata->utexture, 1) < 0) {
            return -1;
        }
        if (D3D_BindTextureRep(data->device, &texturedata->vtexture, 2) < 0) {
            return -1;
        }
    }
    return 0;
}

static int
D3D_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
               const SDL_Rect * srcrect, const SDL_FRect * dstrect)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    LPDIRECT3DPIXELSHADER9 shader;
    float minx, miny, maxx, maxy;
    float minu, maxu, minv, maxv;
    DWORD color;
    Vertex vertices[4];
    HRESULT result;

    if (D3D_ActivateRenderer(renderer) < 0) {
        return -1;
    }

    minx = dstrect->x - 0.5f;
    miny = dstrect->y - 0.5f;
    maxx = dstrect->x + dstrect->w - 0.5f;
    maxy = dstrect->y + dstrect->h - 0.5f;

    minu = (float) srcrect->x / texture->w;
    maxu = (float) (srcrect->x + srcrect->w) / texture->w;
    minv = (float) srcrect->y / texture->h;
    maxv = (float) (srcrect->y + srcrect->h) / texture->h;

    color = D3DCOLOR_ARGB(texture->a, texture->r, texture->g, texture->b);

    vertices[0].x = minx;
    vertices[0].y = miny;
    vertices[0].z = 0.0f;
    vertices[0].color = color;
    vertices[0].u = minu;
    vertices[0].v = minv;

    vertices[1].x = maxx;
    vertices[1].y = miny;
    vertices[1].z = 0.0f;
    vertices[1].color = color;
    vertices[1].u = maxu;
    vertices[1].v = minv;

    vertices[2].x = maxx;
    vertices[2].y = maxy;
    vertices[2].z = 0.0f;
    vertices[2].color = color;
    vertices[2].u = maxu;
    vertices[2].v = maxv;

    vertices[3].x = minx;
    vertices[3].y = maxy;
    vertices[3].z = 0.0f;
    vertices[3].color = color;
    vertices[3].u = minu;
    vertices[3].v = maxv;

    D3D_SetBlendMode(data, texture->blendMode);

    if (D3D_RenderSetupTextureState(renderer, texture, &shader) < 0) {
        return -1;
    }
    
    if (shader) {
        result = IDirect3DDevice9_SetPixelShader(data->device, shader);
        if (FAILED(result)) {
            return D3D_SetError("SetShader()", result);
        }
    }
    result = IDirect3DDevice9_DrawPrimitiveUP(data->device, D3DPT_TRIANGLEFAN, 2,
                                              vertices, sizeof(*vertices));
    if (FAILED(result)) {
        D3D_SetError("DrawPrimitiveUP()", result);
    }
    if (shader) {
        IDirect3DDevice9_SetPixelShader(data->device, NULL);
    }
    return FAILED(result) ? -1 : 0;
}


static int
D3D_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
               const SDL_Rect * srcrect, const SDL_FRect * dstrect,
               const double angle, const SDL_FPoint * center, const SDL_RendererFlip flip)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    LPDIRECT3DPIXELSHADER9 shader = NULL;
    float minx, miny, maxx, maxy;
    float minu, maxu, minv, maxv;
    float centerx, centery;
    DWORD color;
    Vertex vertices[4];
    Float4X4 modelMatrix;
    HRESULT result;

    if (D3D_ActivateRenderer(renderer) < 0) {
        return -1;
    }

    centerx = center->x;
    centery = center->y;

    minx = -centerx;
    maxx = dstrect->w - centerx;
    miny = -centery;
    maxy = dstrect->h - centery;

    minu = (float) srcrect->x / texture->w;
    maxu = (float) (srcrect->x + srcrect->w) / texture->w;
    minv = (float) srcrect->y / texture->h;
    maxv = (float) (srcrect->y + srcrect->h) / texture->h;

    if (flip & SDL_FLIP_HORIZONTAL) {
        float tmp = maxu;
        maxu = minu;
        minu = tmp;
    }
    if (flip & SDL_FLIP_VERTICAL) {
        float tmp = maxv;
        maxv = minv;
        minv = tmp;
    }

    color = D3DCOLOR_ARGB(texture->a, texture->r, texture->g, texture->b);

    vertices[0].x = minx;
    vertices[0].y = miny;
    vertices[0].z = 0.0f;
    vertices[0].color = color;
    vertices[0].u = minu;
    vertices[0].v = minv;

    vertices[1].x = maxx;
    vertices[1].y = miny;
    vertices[1].z = 0.0f;
    vertices[1].color = color;
    vertices[1].u = maxu;
    vertices[1].v = minv;

    vertices[2].x = maxx;
    vertices[2].y = maxy;
    vertices[2].z = 0.0f;
    vertices[2].color = color;
    vertices[2].u = maxu;
    vertices[2].v = maxv;

    vertices[3].x = minx;
    vertices[3].y = maxy;
    vertices[3].z = 0.0f;
    vertices[3].color = color;
    vertices[3].u = minu;
    vertices[3].v = maxv;

    D3D_SetBlendMode(data, texture->blendMode);

    if (D3D_RenderSetupTextureState(renderer, texture, &shader) < 0) {
        return -1;
    }

    /* Rotate and translate */
    modelMatrix = MatrixMultiply(
            MatrixRotationZ((float)(M_PI * (float) angle / 180.0f)),
            MatrixTranslation(dstrect->x + center->x - 0.5f, dstrect->y + center->y - 0.5f, 0));
    IDirect3DDevice9_SetTransform(data->device, D3DTS_VIEW, (D3DMATRIX*)&modelMatrix);
    
    if (shader) {
        result = IDirect3DDevice9_SetPixelShader(data->device, shader);
        if (FAILED(result)) {
            D3D_SetError("SetShader()", result);
            goto done;
        }
    }
    result = IDirect3DDevice9_DrawPrimitiveUP(data->device, D3DPT_TRIANGLEFAN, 2,
                                              vertices, sizeof(*vertices));
    if (FAILED(result)) {
        D3D_SetError("DrawPrimitiveUP()", result);
    }
done:
    if (shader) {
        IDirect3DDevice9_SetPixelShader(data->device, NULL);
    }

    modelMatrix = MatrixIdentity();
    IDirect3DDevice9_SetTransform(data->device, D3DTS_VIEW, (D3DMATRIX*)&modelMatrix);

    return FAILED(result) ? -1 : 0;
}

static int
D3D_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                     Uint32 format, void * pixels, int pitch)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    D3DSURFACE_DESC desc;
    LPDIRECT3DSURFACE9 backBuffer;
    LPDIRECT3DSURFACE9 surface;
    RECT d3drect;
    D3DLOCKED_RECT locked;
    HRESULT result;

    if (data->currentRenderTarget) {
        backBuffer = data->currentRenderTarget;
    } else {
        backBuffer = data->defaultRenderTarget;
    }

    result = IDirect3DSurface9_GetDesc(backBuffer, &desc);
    if (FAILED(result)) {
        IDirect3DSurface9_Release(backBuffer);
        return D3D_SetError("GetDesc()", result);
    }

    result = IDirect3DDevice9_CreateOffscreenPlainSurface(data->device, desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &surface, NULL);
    if (FAILED(result)) {
        IDirect3DSurface9_Release(backBuffer);
        return D3D_SetError("CreateOffscreenPlainSurface()", result);
    }

    result = IDirect3DDevice9_GetRenderTargetData(data->device, backBuffer, surface);
    if (FAILED(result)) {
        IDirect3DSurface9_Release(surface);
        IDirect3DSurface9_Release(backBuffer);
        return D3D_SetError("GetRenderTargetData()", result);
    }

    d3drect.left = rect->x;
    d3drect.right = rect->x + rect->w;
    d3drect.top = rect->y;
    d3drect.bottom = rect->y + rect->h;

    result = IDirect3DSurface9_LockRect(surface, &locked, &d3drect, D3DLOCK_READONLY);
    if (FAILED(result)) {
        IDirect3DSurface9_Release(surface);
        IDirect3DSurface9_Release(backBuffer);
        return D3D_SetError("LockRect()", result);
    }

    SDL_ConvertPixels(rect->w, rect->h,
                      D3DFMTToPixelFormat(desc.Format), locked.pBits, locked.Pitch,
                      format, pixels, pitch);

    IDirect3DSurface9_UnlockRect(surface);

    IDirect3DSurface9_Release(surface);

    return 0;
}

static void
D3D_RenderPresent(SDL_Renderer * renderer)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
    HRESULT result;

    if (!data->beginScene) {
        IDirect3DDevice9_EndScene(data->device);
        data->beginScene = SDL_TRUE;
    }

    result = IDirect3DDevice9_TestCooperativeLevel(data->device);
    if (result == D3DERR_DEVICELOST) {
        /* We'll reset later */
        return;
    }
    if (result == D3DERR_DEVICENOTRESET) {
        D3D_Reset(renderer);
    }
    result = IDirect3DDevice9_Present(data->device, NULL, NULL, NULL, NULL);
    if (FAILED(result)) {
        D3D_SetError("Present()", result);
    }
}

static void
D3D_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    D3D_TextureData *data = (D3D_TextureData *) texture->driverdata;

    if (!data) {
        return;
    }
    D3D_DestroyTextureRep(&data->texture);
    D3D_DestroyTextureRep(&data->utexture);
    D3D_DestroyTextureRep(&data->vtexture);
    SDL_free(data->pixels);
    SDL_free(data);
    texture->driverdata = NULL;
}

static void
D3D_DestroyRenderer(SDL_Renderer * renderer)
{
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;

    if (data) {
        int i;

        /* Release the render target */
        if (data->defaultRenderTarget) {
            IDirect3DSurface9_Release(data->defaultRenderTarget);
            data->defaultRenderTarget = NULL;
        }
        if (data->currentRenderTarget != NULL) {
            IDirect3DSurface9_Release(data->currentRenderTarget);
            data->currentRenderTarget = NULL;
        }
        for (i = 0; i < SDL_arraysize(data->shaders); ++i) {
            if (data->shaders[i]) {
                IDirect3DPixelShader9_Release(data->shaders[i]);
                data->shaders[i] = NULL;
            }
        }
        if (data->device) {
            IDirect3DDevice9_Release(data->device);
            data->device = NULL;
        }
        if (data->d3d) {
            IDirect3D9_Release(data->d3d);
            SDL_UnloadObject(data->d3dDLL);
        }
        SDL_free(data);
    }
    SDL_free(renderer);
}
#endif /* SDL_VIDEO_RENDER_D3D && !SDL_RENDER_DISABLED */

#ifdef __WIN32__
/* This function needs to always exist on Windows, for the Dynamic API. */
IDirect3DDevice9 *
SDL_RenderGetD3D9Device(SDL_Renderer * renderer)
{
    IDirect3DDevice9 *device = NULL;

#if SDL_VIDEO_RENDER_D3D && !SDL_RENDER_DISABLED
    D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;

    /* Make sure that this is a D3D renderer */
    if (renderer->DestroyRenderer != D3D_DestroyRenderer) {
        SDL_SetError("Renderer is not a D3D renderer");
        return NULL;
    }

    device = data->device;
    if (device) {
        IDirect3DDevice9_AddRef(device);
    }
#endif /* SDL_VIDEO_RENDER_D3D && !SDL_RENDER_DISABLED */

    return device;
}
#endif /* __WIN32__ */

/* vi: set ts=4 sw=4 expandtab: */
