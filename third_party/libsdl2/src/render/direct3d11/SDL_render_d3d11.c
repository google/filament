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

#if SDL_VIDEO_RENDER_D3D11 && !SDL_RENDER_DISABLED

#define COBJMACROS
#include "../../core/windows/SDL_windows.h"
#include "SDL_hints.h"
#include "SDL_loadso.h"
#include "SDL_syswm.h"
#include "../SDL_sysrender.h"
#include "../SDL_d3dmath.h"

#include <d3d11_1.h>

#include "SDL_shaders_d3d11.h"

#ifdef __WINRT__

#if NTDDI_VERSION > NTDDI_WIN8
#include <DXGI1_3.h>
#endif

#include "SDL_render_winrt.h"

#if WINAPI_FAMILY == WINAPI_FAMILY_APP
#include <windows.ui.xaml.media.dxinterop.h>
/* TODO, WinRT, XAML: get the ISwapChainBackgroundPanelNative from something other than a global var */
extern ISwapChainBackgroundPanelNative * WINRT_GlobalSwapChainBackgroundPanelNative;
#endif  /* WINAPI_FAMILY == WINAPI_FAMILY_APP */

#endif  /* __WINRT__ */


#define SDL_COMPOSE_ERROR(str) SDL_STRINGIFY_ARG(__FUNCTION__) ", " str

#define SAFE_RELEASE(X) if ((X)) { IUnknown_Release(SDL_static_cast(IUnknown*, X)); X = NULL; }


/* Vertex shader, common values */
typedef struct
{
    Float4X4 model;
    Float4X4 projectionAndView;
} VertexShaderConstants;

/* Per-vertex data */
typedef struct
{
    Float3 pos;
    Float2 tex;
    Float4 color;
} VertexPositionColor;

/* Per-texture data */
typedef struct
{
    ID3D11Texture2D *mainTexture;
    ID3D11ShaderResourceView *mainTextureResourceView;
    ID3D11RenderTargetView *mainTextureRenderTargetView;
    ID3D11Texture2D *stagingTexture;
    int lockedTexturePositionX;
    int lockedTexturePositionY;
    D3D11_FILTER scaleMode;

    /* YV12 texture support */
    SDL_bool yuv;
    ID3D11Texture2D *mainTextureU;
    ID3D11ShaderResourceView *mainTextureResourceViewU;
    ID3D11Texture2D *mainTextureV;
    ID3D11ShaderResourceView *mainTextureResourceViewV;

    /* NV12 texture support */
    SDL_bool nv12;
    ID3D11Texture2D *mainTextureNV;
    ID3D11ShaderResourceView *mainTextureResourceViewNV;

    Uint8 *pixels;
    int pitch;
    SDL_Rect locked_rect;
} D3D11_TextureData;

/* Blend mode data */
typedef struct
{
    SDL_BlendMode blendMode;
    ID3D11BlendState *blendState;
} D3D11_BlendMode;

/* Private renderer data */
typedef struct
{
    void *hDXGIMod;
    void *hD3D11Mod;
    IDXGIFactory2 *dxgiFactory;
    IDXGIAdapter *dxgiAdapter;
    ID3D11Device1 *d3dDevice;
    ID3D11DeviceContext1 *d3dContext;
    IDXGISwapChain1 *swapChain;
    DXGI_SWAP_EFFECT swapEffect;
    ID3D11RenderTargetView *mainRenderTargetView;
    ID3D11RenderTargetView *currentOffscreenRenderTargetView;
    ID3D11InputLayout *inputLayout;
    ID3D11Buffer *vertexBuffer;
    ID3D11VertexShader *vertexShader;
    ID3D11PixelShader *pixelShaders[NUM_SHADERS];
    int blendModesCount;
    D3D11_BlendMode *blendModes;
    ID3D11SamplerState *nearestPixelSampler;
    ID3D11SamplerState *linearSampler;
    D3D_FEATURE_LEVEL featureLevel;

    /* Rasterizers */
    ID3D11RasterizerState *mainRasterizer;
    ID3D11RasterizerState *clippedRasterizer;

    /* Vertex buffer constants */
    VertexShaderConstants vertexShaderConstantsData;
    ID3D11Buffer *vertexShaderConstants;

    /* Cached renderer properties */
    DXGI_MODE_ROTATION rotation;
    ID3D11RenderTargetView *currentRenderTargetView;
    ID3D11RasterizerState *currentRasterizerState;
    ID3D11BlendState *currentBlendState;
    ID3D11PixelShader *currentShader;
    ID3D11ShaderResourceView *currentShaderResource;
    ID3D11SamplerState *currentSampler;
} D3D11_RenderData;


/* Define D3D GUIDs here so we don't have to include uuid.lib.
*
* Fix for SDL bug https://bugzilla.libsdl.org/show_bug.cgi?id=3437:
* The extra 'SDL_' was added to the start of each IID's name, in order
* to prevent build errors on both MinGW-w64 and WinRT/UWP.
* (SDL bug https://bugzilla.libsdl.org/show_bug.cgi?id=3336 led to
* linker errors in WinRT/UWP builds.)
*/

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif

static const GUID SDL_IID_IDXGIFactory2 = { 0x50c83a1c, 0xe072, 0x4c48, { 0x87, 0xb0, 0x36, 0x30, 0xfa, 0x36, 0xa6, 0xd0 } };
static const GUID SDL_IID_IDXGIDevice1 = { 0x77db970f, 0x6276, 0x48ba, { 0xba, 0x28, 0x07, 0x01, 0x43, 0xb4, 0x39, 0x2c } };
static const GUID SDL_IID_IDXGIDevice3 = { 0x6007896c, 0x3244, 0x4afd, { 0xbf, 0x18, 0xa6, 0xd3, 0xbe, 0xda, 0x50, 0x23 } };
static const GUID SDL_IID_ID3D11Texture2D = { 0x6f15aaf2, 0xd208, 0x4e89, { 0x9a, 0xb4, 0x48, 0x95, 0x35, 0xd3, 0x4f, 0x9c } };
static const GUID SDL_IID_ID3D11Device1 = { 0xa04bfb29, 0x08ef, 0x43d6, { 0xa4, 0x9c, 0xa9, 0xbd, 0xbd, 0xcb, 0xe6, 0x86 } };
static const GUID SDL_IID_ID3D11DeviceContext1 = { 0xbb2c6faa, 0xb5fb, 0x4082, { 0x8e, 0x6b, 0x38, 0x8b, 0x8c, 0xfa, 0x90, 0xe1 } };
static const GUID SDL_IID_ID3D11Debug = { 0x79cf2233, 0x7536, 0x4948, { 0x9d, 0x36, 0x1e, 0x46, 0x92, 0xdc, 0x57, 0x60 } };

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


/* Direct3D 11.1 renderer implementation */
static SDL_Renderer *D3D11_CreateRenderer(SDL_Window * window, Uint32 flags);
static void D3D11_WindowEvent(SDL_Renderer * renderer,
                            const SDL_WindowEvent *event);
static SDL_bool D3D11_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode);
static int D3D11_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int D3D11_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                             const SDL_Rect * rect, const void *srcPixels,
                             int srcPitch);
static int D3D11_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                                  const SDL_Rect * rect,
                                  const Uint8 *Yplane, int Ypitch,
                                  const Uint8 *Uplane, int Upitch,
                                  const Uint8 *Vplane, int Vpitch);
static int D3D11_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                             const SDL_Rect * rect, void **pixels, int *pitch);
static void D3D11_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int D3D11_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture);
static int D3D11_UpdateViewport(SDL_Renderer * renderer);
static int D3D11_UpdateClipRect(SDL_Renderer * renderer);
static int D3D11_RenderClear(SDL_Renderer * renderer);
static int D3D11_RenderDrawPoints(SDL_Renderer * renderer,
                                  const SDL_FPoint * points, int count);
static int D3D11_RenderDrawLines(SDL_Renderer * renderer,
                                 const SDL_FPoint * points, int count);
static int D3D11_RenderFillRects(SDL_Renderer * renderer,
                                 const SDL_FRect * rects, int count);
static int D3D11_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                            const SDL_Rect * srcrect, const SDL_FRect * dstrect);
static int D3D11_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                              const SDL_Rect * srcrect, const SDL_FRect * dstrect,
                              const double angle, const SDL_FPoint * center, const SDL_RendererFlip flip);
static int D3D11_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                                  Uint32 format, void * pixels, int pitch);
static void D3D11_RenderPresent(SDL_Renderer * renderer);
static void D3D11_DestroyTexture(SDL_Renderer * renderer,
                                 SDL_Texture * texture);
static void D3D11_DestroyRenderer(SDL_Renderer * renderer);

/* Direct3D 11.1 Internal Functions */
static HRESULT D3D11_CreateDeviceResources(SDL_Renderer * renderer);
static HRESULT D3D11_CreateWindowSizeDependentResources(SDL_Renderer * renderer);
static HRESULT D3D11_UpdateForWindowSizeChange(SDL_Renderer * renderer);
static HRESULT D3D11_HandleDeviceLost(SDL_Renderer * renderer);
static void D3D11_ReleaseMainRenderTargetView(SDL_Renderer * renderer);

SDL_RenderDriver D3D11_RenderDriver = {
    D3D11_CreateRenderer,
    {
        "direct3d11",
        (
            SDL_RENDERER_ACCELERATED |
            SDL_RENDERER_PRESENTVSYNC |
            SDL_RENDERER_TARGETTEXTURE
        ),                          /* flags.  see SDL_RendererFlags */
        6,                          /* num_texture_formats */
        {                           /* texture_formats */
            SDL_PIXELFORMAT_ARGB8888,
            SDL_PIXELFORMAT_RGB888,
            SDL_PIXELFORMAT_YV12,
            SDL_PIXELFORMAT_IYUV,
            SDL_PIXELFORMAT_NV12,
            SDL_PIXELFORMAT_NV21
        },
        0,                          /* max_texture_width: will be filled in later */
        0                           /* max_texture_height: will be filled in later */
    }
};


Uint32
D3D11_DXGIFormatToSDLPixelFormat(DXGI_FORMAT dxgiFormat)
{
    switch (dxgiFormat) {
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return SDL_PIXELFORMAT_ARGB8888;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return SDL_PIXELFORMAT_RGB888;
        default:
            return SDL_PIXELFORMAT_UNKNOWN;
    }
}

static DXGI_FORMAT
SDLPixelFormatToDXGIFormat(Uint32 sdlFormat)
{
    switch (sdlFormat) {
        case SDL_PIXELFORMAT_ARGB8888:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case SDL_PIXELFORMAT_RGB888:
            return DXGI_FORMAT_B8G8R8X8_UNORM;
        case SDL_PIXELFORMAT_YV12:
        case SDL_PIXELFORMAT_IYUV:
        case SDL_PIXELFORMAT_NV12:  /* For the Y texture */
        case SDL_PIXELFORMAT_NV21:  /* For the Y texture */
            return DXGI_FORMAT_R8_UNORM;
        default:
            return DXGI_FORMAT_UNKNOWN;
    }
}

SDL_Renderer *
D3D11_CreateRenderer(SDL_Window * window, Uint32 flags)
{
    SDL_Renderer *renderer;
    D3D11_RenderData *data;

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (D3D11_RenderData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = D3D11_WindowEvent;
    renderer->SupportsBlendMode = D3D11_SupportsBlendMode;
    renderer->CreateTexture = D3D11_CreateTexture;
    renderer->UpdateTexture = D3D11_UpdateTexture;
    renderer->UpdateTextureYUV = D3D11_UpdateTextureYUV;
    renderer->LockTexture = D3D11_LockTexture;
    renderer->UnlockTexture = D3D11_UnlockTexture;
    renderer->SetRenderTarget = D3D11_SetRenderTarget;
    renderer->UpdateViewport = D3D11_UpdateViewport;
    renderer->UpdateClipRect = D3D11_UpdateClipRect;
    renderer->RenderClear = D3D11_RenderClear;
    renderer->RenderDrawPoints = D3D11_RenderDrawPoints;
    renderer->RenderDrawLines = D3D11_RenderDrawLines;
    renderer->RenderFillRects = D3D11_RenderFillRects;
    renderer->RenderCopy = D3D11_RenderCopy;
    renderer->RenderCopyEx = D3D11_RenderCopyEx;
    renderer->RenderReadPixels = D3D11_RenderReadPixels;
    renderer->RenderPresent = D3D11_RenderPresent;
    renderer->DestroyTexture = D3D11_DestroyTexture;
    renderer->DestroyRenderer = D3D11_DestroyRenderer;
    renderer->info = D3D11_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
    /* VSync is required in Windows Phone, at least for Win Phone 8.0 and 8.1.
     * Failure to use it seems to either result in:
     *
     *  - with the D3D11 debug runtime turned OFF, vsync seemingly gets turned
     *    off (framerate doesn't get capped), but nothing appears on-screen
     *
     *  - with the D3D11 debug runtime turned ON, vsync gets automatically
     *    turned back on, and the following gets output to the debug console:
     *    
     *    DXGI ERROR: IDXGISwapChain::Present: Interval 0 is not supported, changed to Interval 1. [ UNKNOWN ERROR #1024: ] 
     */
    renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
#else
    if ((flags & SDL_RENDERER_PRESENTVSYNC)) {
        renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
    }
#endif

    /* HACK: make sure the SDL_Renderer references the SDL_Window data now, in
     * order to give init functions access to the underlying window handle:
     */
    renderer->window = window;

    /* Initialize Direct3D resources */
    if (FAILED(D3D11_CreateDeviceResources(renderer))) {
        D3D11_DestroyRenderer(renderer);
        return NULL;
    }
    if (FAILED(D3D11_CreateWindowSizeDependentResources(renderer))) {
        D3D11_DestroyRenderer(renderer);
        return NULL;
    }

    return renderer;
}

static void
D3D11_ReleaseAll(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    SDL_Texture *texture = NULL;

    /* Release all textures */
    for (texture = renderer->textures; texture; texture = texture->next) {
        D3D11_DestroyTexture(renderer, texture);
    }

    /* Release/reset everything else */
    if (data) {
        int i;

        SAFE_RELEASE(data->dxgiFactory);
        SAFE_RELEASE(data->dxgiAdapter);
        SAFE_RELEASE(data->d3dDevice);
        SAFE_RELEASE(data->d3dContext);
        SAFE_RELEASE(data->swapChain);
        SAFE_RELEASE(data->mainRenderTargetView);
        SAFE_RELEASE(data->currentOffscreenRenderTargetView);
        SAFE_RELEASE(data->inputLayout);
        SAFE_RELEASE(data->vertexBuffer);
        SAFE_RELEASE(data->vertexShader);
        for (i = 0; i < SDL_arraysize(data->pixelShaders); ++i) {
            SAFE_RELEASE(data->pixelShaders[i]);
        }
        if (data->blendModesCount > 0) {
            for (i = 0; i < data->blendModesCount; ++i) {
                SAFE_RELEASE(data->blendModes[i].blendState);
            }
            SDL_free(data->blendModes);

            data->blendModesCount = 0;
        }
        SAFE_RELEASE(data->nearestPixelSampler);
        SAFE_RELEASE(data->linearSampler);
        SAFE_RELEASE(data->mainRasterizer);
        SAFE_RELEASE(data->clippedRasterizer);
        SAFE_RELEASE(data->vertexShaderConstants);

        data->swapEffect = (DXGI_SWAP_EFFECT) 0;
        data->rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
        data->currentRenderTargetView = NULL;
        data->currentRasterizerState = NULL;
        data->currentBlendState = NULL;
        data->currentShader = NULL;
        data->currentShaderResource = NULL;
        data->currentSampler = NULL;

        /* Unload the D3D libraries.  This should be done last, in order
         * to prevent IUnknown::Release() calls from crashing.
         */
        if (data->hD3D11Mod) {
            SDL_UnloadObject(data->hD3D11Mod);
            data->hD3D11Mod = NULL;
        }
        if (data->hDXGIMod) {
            SDL_UnloadObject(data->hDXGIMod);
            data->hDXGIMod = NULL;
        }
    }
}

static void
D3D11_DestroyRenderer(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    D3D11_ReleaseAll(renderer);
    if (data) {
        SDL_free(data);
    }
    SDL_free(renderer);
}

static D3D11_BLEND GetBlendFunc(SDL_BlendFactor factor)
{
    switch (factor) {
    case SDL_BLENDFACTOR_ZERO:
        return D3D11_BLEND_ZERO;
    case SDL_BLENDFACTOR_ONE:
        return D3D11_BLEND_ONE;
    case SDL_BLENDFACTOR_SRC_COLOR:
        return D3D11_BLEND_SRC_COLOR;
    case SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR:
        return D3D11_BLEND_INV_SRC_COLOR;
    case SDL_BLENDFACTOR_SRC_ALPHA:
        return D3D11_BLEND_SRC_ALPHA;
    case SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA:
        return D3D11_BLEND_INV_SRC_ALPHA;
    case SDL_BLENDFACTOR_DST_COLOR:
        return D3D11_BLEND_DEST_COLOR;
    case SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR:
        return D3D11_BLEND_INV_DEST_COLOR;
    case SDL_BLENDFACTOR_DST_ALPHA:
        return D3D11_BLEND_DEST_ALPHA;
    case SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA:
        return D3D11_BLEND_INV_DEST_ALPHA;
    default:
        return (D3D11_BLEND)0;
    }
}

static D3D11_BLEND_OP GetBlendEquation(SDL_BlendOperation operation)
{
    switch (operation) {
    case SDL_BLENDOPERATION_ADD:
        return D3D11_BLEND_OP_ADD;
    case SDL_BLENDOPERATION_SUBTRACT:
        return D3D11_BLEND_OP_SUBTRACT;
    case SDL_BLENDOPERATION_REV_SUBTRACT:
        return D3D11_BLEND_OP_REV_SUBTRACT;
    case SDL_BLENDOPERATION_MINIMUM:
        return D3D11_BLEND_OP_MIN;
    case SDL_BLENDOPERATION_MAXIMUM:
        return D3D11_BLEND_OP_MAX;
    default:
        return (D3D11_BLEND_OP)0;
    }
}

static SDL_bool
D3D11_CreateBlendState(SDL_Renderer * renderer, SDL_BlendMode blendMode)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    SDL_BlendFactor srcColorFactor = SDL_GetBlendModeSrcColorFactor(blendMode);
    SDL_BlendFactor srcAlphaFactor = SDL_GetBlendModeSrcAlphaFactor(blendMode);
    SDL_BlendOperation colorOperation = SDL_GetBlendModeColorOperation(blendMode);
    SDL_BlendFactor dstColorFactor = SDL_GetBlendModeDstColorFactor(blendMode);
    SDL_BlendFactor dstAlphaFactor = SDL_GetBlendModeDstAlphaFactor(blendMode);
    SDL_BlendOperation alphaOperation = SDL_GetBlendModeAlphaOperation(blendMode);
    ID3D11BlendState *blendState = NULL;
    D3D11_BlendMode *blendModes;
    HRESULT result = S_OK;

    D3D11_BLEND_DESC blendDesc;
    SDL_zero(blendDesc);
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = GetBlendFunc(srcColorFactor);
    blendDesc.RenderTarget[0].DestBlend = GetBlendFunc(dstColorFactor);
    blendDesc.RenderTarget[0].BlendOp = GetBlendEquation(colorOperation);
    blendDesc.RenderTarget[0].SrcBlendAlpha = GetBlendFunc(srcAlphaFactor);
    blendDesc.RenderTarget[0].DestBlendAlpha = GetBlendFunc(dstAlphaFactor);
    blendDesc.RenderTarget[0].BlendOpAlpha = GetBlendEquation(alphaOperation);
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    result = ID3D11Device_CreateBlendState(data->d3dDevice, &blendDesc, &blendState);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateBlendState"), result);
        return SDL_FALSE;
    }

    blendModes = (D3D11_BlendMode *)SDL_realloc(data->blendModes, (data->blendModesCount + 1) * sizeof(*blendModes));
    if (!blendModes) {
        SAFE_RELEASE(blendState);
        SDL_OutOfMemory();
        return SDL_FALSE;
    }
    blendModes[data->blendModesCount].blendMode = blendMode;
    blendModes[data->blendModesCount].blendState = blendState;
    data->blendModes = blendModes;
    ++data->blendModesCount;

    return SDL_TRUE;
}

/* Create resources that depend on the device. */
static HRESULT
D3D11_CreateDeviceResources(SDL_Renderer * renderer)
{
    typedef HRESULT(WINAPI *PFN_CREATE_DXGI_FACTORY)(REFIID riid, void **ppFactory);
    PFN_CREATE_DXGI_FACTORY CreateDXGIFactoryFunc;
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    PFN_D3D11_CREATE_DEVICE D3D11CreateDeviceFunc;
    ID3D11Device *d3dDevice = NULL;
    ID3D11DeviceContext *d3dContext = NULL;
    IDXGIDevice1 *dxgiDevice = NULL;
    HRESULT result = S_OK;
    UINT creationFlags;
    int i;

    /* This array defines the set of DirectX hardware feature levels this app will support.
     * Note the ordering should be preserved.
     * Don't forget to declare your application's minimum required feature level in its
     * description.  All applications are assumed to support 9.1 unless otherwise stated.
     */
    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    D3D11_BUFFER_DESC constantBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_RASTERIZER_DESC rasterDesc;

#ifdef __WINRT__
    CreateDXGIFactoryFunc = CreateDXGIFactory1;
    D3D11CreateDeviceFunc = D3D11CreateDevice;
#else
    data->hDXGIMod = SDL_LoadObject("dxgi.dll");
    if (!data->hDXGIMod) {
        result = E_FAIL;
        goto done;
    }

    CreateDXGIFactoryFunc = (PFN_CREATE_DXGI_FACTORY)SDL_LoadFunction(data->hDXGIMod, "CreateDXGIFactory");
    if (!CreateDXGIFactoryFunc) {
        result = E_FAIL;
        goto done;
    }

    data->hD3D11Mod = SDL_LoadObject("d3d11.dll");
    if (!data->hD3D11Mod) {
        result = E_FAIL;
        goto done;
    }

    D3D11CreateDeviceFunc = (PFN_D3D11_CREATE_DEVICE)SDL_LoadFunction(data->hD3D11Mod, "D3D11CreateDevice");
    if (!D3D11CreateDeviceFunc) {
        result = E_FAIL;
        goto done;
    }
#endif /* __WINRT__ */

    result = CreateDXGIFactoryFunc(&SDL_IID_IDXGIFactory2, (void **)&data->dxgiFactory);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("CreateDXGIFactory"), result);
        goto done;
    }

    /* FIXME: Should we use the default adapter? */
    result = IDXGIFactory2_EnumAdapters(data->dxgiFactory, 0, &data->dxgiAdapter);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("D3D11CreateDevice"), result);
        goto done;
    }

    /* This flag adds support for surfaces with a different color channel ordering
     * than the API default. It is required for compatibility with Direct2D.
     */
    creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    /* Make sure Direct3D's debugging feature gets used, if the app requests it. */
    if (SDL_GetHintBoolean(SDL_HINT_RENDER_DIRECT3D11_DEBUG, SDL_FALSE)) {
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }

    /* Create the Direct3D 11 API device object and a corresponding context. */
    result = D3D11CreateDeviceFunc(
        data->dxgiAdapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        NULL,
        creationFlags, /* Set set debug and Direct2D compatibility flags. */
        featureLevels, /* List of feature levels this app can support. */
        SDL_arraysize(featureLevels),
        D3D11_SDK_VERSION, /* Always set this to D3D11_SDK_VERSION for Windows Store apps. */
        &d3dDevice, /* Returns the Direct3D device created. */
        &data->featureLevel, /* Returns feature level of device created. */
        &d3dContext /* Returns the device immediate context. */
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("D3D11CreateDevice"), result);
        goto done;
    }

    result = ID3D11Device_QueryInterface(d3dDevice, &SDL_IID_ID3D11Device1, (void **)&data->d3dDevice);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device to ID3D11Device1"), result);
        goto done;
    }

    result = ID3D11DeviceContext_QueryInterface(d3dContext, &SDL_IID_ID3D11DeviceContext1, (void **)&data->d3dContext);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11DeviceContext to ID3D11DeviceContext1"), result);
        goto done;
    }

    result = ID3D11Device_QueryInterface(d3dDevice, &SDL_IID_IDXGIDevice1, (void **)&dxgiDevice);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device to IDXGIDevice1"), result);
        goto done;
    }

    /* Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
     * ensures that the application will only render after each VSync, minimizing power consumption.
     */
    result = IDXGIDevice1_SetMaximumFrameLatency(dxgiDevice, 1);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGIDevice1::SetMaximumFrameLatency"), result);
        goto done;
    }

    /* Make note of the maximum texture size
     * Max texture sizes are documented on MSDN, at:
     * http://msdn.microsoft.com/en-us/library/windows/apps/ff476876.aspx
     */
    switch (data->featureLevel) {
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_11_0:
            renderer->info.max_texture_width = renderer->info.max_texture_height = 16384;
            break;

        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_10_0:
            renderer->info.max_texture_width = renderer->info.max_texture_height = 8192;
            break;

        case D3D_FEATURE_LEVEL_9_3:
            renderer->info.max_texture_width = renderer->info.max_texture_height = 4096;
            break;

        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_1:
            renderer->info.max_texture_width = renderer->info.max_texture_height = 2048;
            break;

        default:
            SDL_SetError("%s, Unexpected feature level: %d", __FUNCTION__, data->featureLevel);
            result = E_FAIL;
            goto done;
    }

    if (D3D11_CreateVertexShader(data->d3dDevice, &data->vertexShader, &data->inputLayout) < 0) {
        goto done;
    }

    for (i = 0; i < SDL_arraysize(data->pixelShaders); ++i) {
        if (D3D11_CreatePixelShader(data->d3dDevice, (D3D11_Shader)i, &data->pixelShaders[i]) < 0) {
            goto done;
        }
    }

    /* Setup space to hold vertex shader constants: */
    SDL_zero(constantBufferDesc);
    constantBufferDesc.ByteWidth = sizeof(VertexShaderConstants);
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    result = ID3D11Device_CreateBuffer(data->d3dDevice,
        &constantBufferDesc,
        NULL,
        &data->vertexShaderConstants
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateBuffer [vertex shader constants]"), result);
        goto done;
    }

    /* Create samplers to use when drawing textures: */
    SDL_zero(samplerDesc);
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    result = ID3D11Device_CreateSamplerState(data->d3dDevice,
        &samplerDesc,
        &data->nearestPixelSampler
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateSamplerState [nearest-pixel filter]"), result);
        goto done;
    }

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    result = ID3D11Device_CreateSamplerState(data->d3dDevice,
        &samplerDesc,
        &data->linearSampler
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateSamplerState [linear filter]"), result);
        goto done;
    }

    /* Setup Direct3D rasterizer states */
    SDL_zero(rasterDesc);
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.ScissorEnable = FALSE;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    result = ID3D11Device_CreateRasterizerState(data->d3dDevice, &rasterDesc, &data->mainRasterizer);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateRasterizerState [main rasterizer]"), result);
        goto done;
    }

    rasterDesc.ScissorEnable = TRUE;
    result = ID3D11Device_CreateRasterizerState(data->d3dDevice, &rasterDesc, &data->clippedRasterizer);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateRasterizerState [clipped rasterizer]"), result);
        goto done;
    }

    /* Create blending states: */
    if (!D3D11_CreateBlendState(renderer, SDL_BLENDMODE_BLEND) ||
        !D3D11_CreateBlendState(renderer, SDL_BLENDMODE_ADD) ||
        !D3D11_CreateBlendState(renderer, SDL_BLENDMODE_MOD)) {
        /* D3D11_CreateBlendMode will set the SDL error, if it fails */
        goto done;
    }

    /* Setup render state that doesn't change */
    ID3D11DeviceContext_IASetInputLayout(data->d3dContext, data->inputLayout);
    ID3D11DeviceContext_VSSetShader(data->d3dContext, data->vertexShader, NULL, 0);
    ID3D11DeviceContext_VSSetConstantBuffers(data->d3dContext, 0, 1, &data->vertexShaderConstants);

done:
    SAFE_RELEASE(d3dDevice);
    SAFE_RELEASE(d3dContext);
    SAFE_RELEASE(dxgiDevice);
    return result;
}

#ifdef __WIN32__

static DXGI_MODE_ROTATION
D3D11_GetCurrentRotation()
{
    /* FIXME */
    return DXGI_MODE_ROTATION_IDENTITY;
}

#endif /* __WIN32__ */

static BOOL
D3D11_IsDisplayRotated90Degrees(DXGI_MODE_ROTATION rotation)
{
    switch (rotation) {
        case DXGI_MODE_ROTATION_ROTATE90:
        case DXGI_MODE_ROTATION_ROTATE270:
            return TRUE;
        default:
            return FALSE;
    }
}

static int
D3D11_GetRotationForCurrentRenderTarget(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *)renderer->driverdata;
    if (data->currentOffscreenRenderTargetView) {
        return DXGI_MODE_ROTATION_IDENTITY;
    } else {
        return data->rotation;
    }
}

static int
D3D11_GetViewportAlignedD3DRect(SDL_Renderer * renderer, const SDL_Rect * sdlRect, D3D11_RECT * outRect, BOOL includeViewportOffset)
{
    const int rotation = D3D11_GetRotationForCurrentRenderTarget(renderer);
    switch (rotation) {
        case DXGI_MODE_ROTATION_IDENTITY:
            outRect->left = sdlRect->x;
            outRect->right = sdlRect->x + sdlRect->w;
            outRect->top = sdlRect->y;
            outRect->bottom = sdlRect->y + sdlRect->h;
            if (includeViewportOffset) {
                outRect->left += renderer->viewport.x;
                outRect->right += renderer->viewport.x;
                outRect->top += renderer->viewport.y;
                outRect->bottom += renderer->viewport.y;
            }
            break;
        case DXGI_MODE_ROTATION_ROTATE270:
            outRect->left = sdlRect->y;
            outRect->right = sdlRect->y + sdlRect->h;
            outRect->top = renderer->viewport.w - sdlRect->x - sdlRect->w;
            outRect->bottom = renderer->viewport.w - sdlRect->x;
            break;
        case DXGI_MODE_ROTATION_ROTATE180:
            outRect->left = renderer->viewport.w - sdlRect->x - sdlRect->w;
            outRect->right = renderer->viewport.w - sdlRect->x;
            outRect->top = renderer->viewport.h - sdlRect->y - sdlRect->h;
            outRect->bottom = renderer->viewport.h - sdlRect->y;
            break;
        case DXGI_MODE_ROTATION_ROTATE90:
            outRect->left = renderer->viewport.h - sdlRect->y - sdlRect->h;
            outRect->right = renderer->viewport.h - sdlRect->y;
            outRect->top = sdlRect->x;
            outRect->bottom = sdlRect->x + sdlRect->h;
            break;
        default:
            return SDL_SetError("The physical display is in an unknown or unsupported rotation");
    }
    return 0;
}

static HRESULT
D3D11_CreateSwapChain(SDL_Renderer * renderer, int w, int h)
{
    D3D11_RenderData *data = (D3D11_RenderData *)renderer->driverdata;
#ifdef __WINRT__
    IUnknown *coreWindow = D3D11_GetCoreWindowFromSDLRenderer(renderer);
    const BOOL usingXAML = (coreWindow == NULL);
#else
    IUnknown *coreWindow = NULL;
    const BOOL usingXAML = FALSE;
#endif
    HRESULT result = S_OK;

    /* Create a swap chain using the same adapter as the existing Direct3D device. */
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    SDL_zero(swapChainDesc);
    swapChainDesc.Width = w;
    swapChainDesc.Height = h;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; /* This is the most common swap chain format. */
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1; /* Don't use multi-sampling. */
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2; /* Use double-buffering to minimize latency. */
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; /* On phone, only stretch and aspect-ratio stretch scaling are allowed. */
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; /* On phone, no swap effects are supported. */
    /* TODO, WinRT: see if Win 8.x DXGI_SWAP_CHAIN_DESC1 settings are available on Windows Phone 8.1, and if there's any advantage to having them on */
#else
    if (usingXAML) {
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    } else {
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
    }
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; /* All Windows Store apps must use this SwapEffect. */
#endif
    swapChainDesc.Flags = 0;

    if (coreWindow) {
        result = IDXGIFactory2_CreateSwapChainForCoreWindow(data->dxgiFactory,
            (IUnknown *)data->d3dDevice,
            coreWindow,
            &swapChainDesc,
            NULL, /* Allow on all displays. */
            &data->swapChain
            );
        if (FAILED(result)) {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGIFactory2::CreateSwapChainForCoreWindow"), result);
            goto done;
        }
    } else if (usingXAML) {
        result = IDXGIFactory2_CreateSwapChainForComposition(data->dxgiFactory,
            (IUnknown *)data->d3dDevice,
            &swapChainDesc,
            NULL,
            &data->swapChain);
        if (FAILED(result)) {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGIFactory2::CreateSwapChainForComposition"), result);
            goto done;
        }

#if WINAPI_FAMILY == WINAPI_FAMILY_APP
        result = ISwapChainBackgroundPanelNative_SetSwapChain(WINRT_GlobalSwapChainBackgroundPanelNative, (IDXGISwapChain *) data->swapChain);
        if (FAILED(result)) {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ISwapChainBackgroundPanelNative::SetSwapChain"), result);
            goto done;
        }
#else
        SDL_SetError(SDL_COMPOSE_ERROR("XAML support is not yet available for Windows Phone"));
        result = E_FAIL;
        goto done;
#endif
    } else {
#ifdef __WIN32__
        SDL_SysWMinfo windowinfo;
        SDL_VERSION(&windowinfo.version);
        SDL_GetWindowWMInfo(renderer->window, &windowinfo);

        result = IDXGIFactory2_CreateSwapChainForHwnd(data->dxgiFactory,
            (IUnknown *)data->d3dDevice,
            windowinfo.info.win.window,
            &swapChainDesc,
            NULL,
            NULL, /* Allow on all displays. */
            &data->swapChain
            );
        if (FAILED(result)) {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGIFactory2::CreateSwapChainForHwnd"), result);
            goto done;
        }

        IDXGIFactory_MakeWindowAssociation(data->dxgiFactory, windowinfo.info.win.window, DXGI_MWA_NO_WINDOW_CHANGES);
#else
        SDL_SetError(__FUNCTION__", Unable to find something to attach a swap chain to");
        goto done;
#endif  /* ifdef __WIN32__ / else */
    }
    data->swapEffect = swapChainDesc.SwapEffect;

done:
    SAFE_RELEASE(coreWindow);
    return result;
}


/* Initialize all resources that change when the window's size changes. */
static HRESULT
D3D11_CreateWindowSizeDependentResources(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *)renderer->driverdata;
    ID3D11Texture2D *backBuffer = NULL;
    HRESULT result = S_OK;
    int w, h;

    /* Release the previous render target view */
    D3D11_ReleaseMainRenderTargetView(renderer);

    /* The width and height of the swap chain must be based on the display's
     * non-rotated size.
     */
    SDL_GetWindowSize(renderer->window, &w, &h);
    data->rotation = D3D11_GetCurrentRotation();
    /* SDL_Log("%s: windowSize={%d,%d}, orientation=%d\n", __FUNCTION__, w, h, (int)data->rotation); */
    if (D3D11_IsDisplayRotated90Degrees(data->rotation)) {
        int tmp = w;
        w = h;
        h = tmp;
    }

    if (data->swapChain) {
        /* IDXGISwapChain::ResizeBuffers is not available on Windows Phone 8. */
#if !defined(__WINRT__) || (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
        /* If the swap chain already exists, resize it. */
        result = IDXGISwapChain_ResizeBuffers(data->swapChain,
            0,
            w, h,
            DXGI_FORMAT_UNKNOWN,
            0
            );
        if (result == DXGI_ERROR_DEVICE_REMOVED) {
            /* If the device was removed for any reason, a new device and swap chain will need to be created. */
            D3D11_HandleDeviceLost(renderer);

            /* Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
             * and correctly set up the new device.
             */
            goto done;
        } else if (FAILED(result)) {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGISwapChain::ResizeBuffers"), result);
            goto done;
        }
#endif
    } else {
        result = D3D11_CreateSwapChain(renderer, w, h);
        if (FAILED(result)) {
            goto done;
        }
    }
    
#if WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
    /* Set the proper rotation for the swap chain.
     *
     * To note, the call for this, IDXGISwapChain1::SetRotation, is not necessary
     * on Windows Phone 8.0, nor is it supported there.
     *
     * IDXGISwapChain1::SetRotation does seem to be available on Windows Phone 8.1,
     * however I've yet to find a way to make it work.  It might have something to
     * do with IDXGISwapChain::ResizeBuffers appearing to not being available on
     * Windows Phone 8.1 (it wasn't on Windows Phone 8.0), but I'm not 100% sure of this.
     * The call doesn't appear to be entirely necessary though, and is a performance-related
     * call, at least according to the following page on MSDN:
     * http://code.msdn.microsoft.com/windowsapps/DXGI-swap-chain-rotation-21d13d71
     *   -- David L.
     *
     * TODO, WinRT: reexamine the docs for IDXGISwapChain1::SetRotation, see if might be available, usable, and prudent-to-call on WinPhone 8.1
     */
    if (data->swapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL) {
        result = IDXGISwapChain1_SetRotation(data->swapChain, data->rotation);
        if (FAILED(result)) {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGISwapChain1::SetRotation"), result);
            goto done;
        }
    }
#endif

    result = IDXGISwapChain_GetBuffer(data->swapChain,
        0,
        &SDL_IID_ID3D11Texture2D,
        (void **)&backBuffer
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGISwapChain::GetBuffer [back-buffer]"), result);
        goto done;
    }

    /* Create a render target view of the swap chain back buffer. */
    result = ID3D11Device_CreateRenderTargetView(data->d3dDevice,
        (ID3D11Resource *)backBuffer,
        NULL,
        &data->mainRenderTargetView
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device::CreateRenderTargetView"), result);
        goto done;
    }

    if (D3D11_UpdateViewport(renderer) != 0) {
        /* D3D11_UpdateViewport will set the SDL error if it fails. */
        result = E_FAIL;
        goto done;
    }

done:
    SAFE_RELEASE(backBuffer);
    return result;
}

/* This method is called when the window's size changes. */
static HRESULT
D3D11_UpdateForWindowSizeChange(SDL_Renderer * renderer)
{
    return D3D11_CreateWindowSizeDependentResources(renderer);
}

HRESULT
D3D11_HandleDeviceLost(SDL_Renderer * renderer)
{
    HRESULT result = S_OK;

    D3D11_ReleaseAll(renderer);

    result = D3D11_CreateDeviceResources(renderer);
    if (FAILED(result)) {
        /* D3D11_CreateDeviceResources will set the SDL error */
        return result;
    }

    result = D3D11_UpdateForWindowSizeChange(renderer);
    if (FAILED(result)) {
        /* D3D11_UpdateForWindowSizeChange will set the SDL error */
        return result;
    }

    /* Let the application know that the device has been reset */
    {
        SDL_Event event;
        event.type = SDL_RENDER_DEVICE_RESET;
        SDL_PushEvent(&event);
    }

    return S_OK;
}

void
D3D11_Trim(SDL_Renderer * renderer)
{
#ifdef __WINRT__
#if NTDDI_VERSION > NTDDI_WIN8
    D3D11_RenderData *data = (D3D11_RenderData *)renderer->driverdata;
    HRESULT result = S_OK;
    IDXGIDevice3 *dxgiDevice = NULL;

    result = ID3D11Device_QueryInterface(data->d3dDevice, &SDL_IID_IDXGIDevice3, &dxgiDevice);
    if (FAILED(result)) {
        //WIN_SetErrorFromHRESULT(__FUNCTION__ ", ID3D11Device to IDXGIDevice3", result);
        return;
    }

    IDXGIDevice3_Trim(dxgiDevice);
    SAFE_RELEASE(dxgiDevice);
#endif
#endif
}

static void
D3D11_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
    if (event->event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        D3D11_UpdateForWindowSizeChange(renderer);
    }
}

static SDL_bool
D3D11_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode)
{
    SDL_BlendFactor srcColorFactor = SDL_GetBlendModeSrcColorFactor(blendMode);
    SDL_BlendFactor srcAlphaFactor = SDL_GetBlendModeSrcAlphaFactor(blendMode);
    SDL_BlendOperation colorOperation = SDL_GetBlendModeColorOperation(blendMode);
    SDL_BlendFactor dstColorFactor = SDL_GetBlendModeDstColorFactor(blendMode);
    SDL_BlendFactor dstAlphaFactor = SDL_GetBlendModeDstAlphaFactor(blendMode);
    SDL_BlendOperation alphaOperation = SDL_GetBlendModeAlphaOperation(blendMode);

    if (!GetBlendFunc(srcColorFactor) || !GetBlendFunc(srcAlphaFactor) ||
        !GetBlendEquation(colorOperation) ||
        !GetBlendFunc(dstColorFactor) || !GetBlendFunc(dstAlphaFactor) ||
        !GetBlendEquation(alphaOperation)) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static D3D11_FILTER
GetScaleQuality(void)
{
    const char *hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
    if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
        return D3D11_FILTER_MIN_MAG_MIP_POINT;
    } else /* if (*hint == '1' || SDL_strcasecmp(hint, "linear") == 0) */ {
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
}

static int
D3D11_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_TextureData *textureData;
    HRESULT result;
    DXGI_FORMAT textureFormat = SDLPixelFormatToDXGIFormat(texture->format);
    D3D11_TEXTURE2D_DESC textureDesc;
    D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;

    if (textureFormat == DXGI_FORMAT_UNKNOWN) {
        return SDL_SetError("%s, An unsupported SDL pixel format (0x%x) was specified",
            __FUNCTION__, texture->format);
    }

    textureData = (D3D11_TextureData*) SDL_calloc(1, sizeof(*textureData));
    if (!textureData) {
        SDL_OutOfMemory();
        return -1;
    }
    textureData->scaleMode = GetScaleQuality();

    texture->driverdata = textureData;

    SDL_zero(textureDesc);
    textureDesc.Width = texture->w;
    textureDesc.Height = texture->h;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = textureFormat;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.MiscFlags = 0;

    if (texture->access == SDL_TEXTUREACCESS_STREAMING) {
        textureDesc.Usage = D3D11_USAGE_DYNAMIC;
        textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    } else {
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.CPUAccessFlags = 0;
    }

    if (texture->access == SDL_TEXTUREACCESS_TARGET) {
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    } else {
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    }

    result = ID3D11Device_CreateTexture2D(rendererData->d3dDevice,
        &textureDesc,
        NULL,
        &textureData->mainTexture
        );
    if (FAILED(result)) {
        D3D11_DestroyTexture(renderer, texture);
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateTexture2D"), result);
        return -1;
    }

    if (texture->format == SDL_PIXELFORMAT_YV12 ||
        texture->format == SDL_PIXELFORMAT_IYUV) {
        textureData->yuv = SDL_TRUE;

        textureDesc.Width = (textureDesc.Width + 1) / 2;
        textureDesc.Height = (textureDesc.Height + 1) / 2;

        result = ID3D11Device_CreateTexture2D(rendererData->d3dDevice,
            &textureDesc,
            NULL,
            &textureData->mainTextureU
            );
        if (FAILED(result)) {
            D3D11_DestroyTexture(renderer, texture);
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateTexture2D"), result);
            return -1;
        }

        result = ID3D11Device_CreateTexture2D(rendererData->d3dDevice,
            &textureDesc,
            NULL,
            &textureData->mainTextureV
            );
        if (FAILED(result)) {
            D3D11_DestroyTexture(renderer, texture);
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateTexture2D"), result);
            return -1;
        }
    }

    if (texture->format == SDL_PIXELFORMAT_NV12 ||
        texture->format == SDL_PIXELFORMAT_NV21) {
        D3D11_TEXTURE2D_DESC nvTextureDesc = textureDesc;

        textureData->nv12 = SDL_TRUE;

        nvTextureDesc.Format = DXGI_FORMAT_R8G8_UNORM;
        nvTextureDesc.Width = (textureDesc.Width + 1) / 2;
        nvTextureDesc.Height = (textureDesc.Height + 1) / 2;

        result = ID3D11Device_CreateTexture2D(rendererData->d3dDevice,
            &nvTextureDesc,
            NULL,
            &textureData->mainTextureNV
            );
        if (FAILED(result)) {
            D3D11_DestroyTexture(renderer, texture);
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateTexture2D"), result);
            return -1;
        }
    }

    resourceViewDesc.Format = textureDesc.Format;
    resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resourceViewDesc.Texture2D.MostDetailedMip = 0;
    resourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
    result = ID3D11Device_CreateShaderResourceView(rendererData->d3dDevice,
        (ID3D11Resource *)textureData->mainTexture,
        &resourceViewDesc,
        &textureData->mainTextureResourceView
        );
    if (FAILED(result)) {
        D3D11_DestroyTexture(renderer, texture);
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateShaderResourceView"), result);
        return -1;
    }

    if (textureData->yuv) {
        result = ID3D11Device_CreateShaderResourceView(rendererData->d3dDevice,
            (ID3D11Resource *)textureData->mainTextureU,
            &resourceViewDesc,
            &textureData->mainTextureResourceViewU
            );
        if (FAILED(result)) {
            D3D11_DestroyTexture(renderer, texture);
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateShaderResourceView"), result);
            return -1;
        }
        result = ID3D11Device_CreateShaderResourceView(rendererData->d3dDevice,
            (ID3D11Resource *)textureData->mainTextureV,
            &resourceViewDesc,
            &textureData->mainTextureResourceViewV
            );
        if (FAILED(result)) {
            D3D11_DestroyTexture(renderer, texture);
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateShaderResourceView"), result);
            return -1;
        }
    }

    if (textureData->nv12) {
        D3D11_SHADER_RESOURCE_VIEW_DESC nvResourceViewDesc = resourceViewDesc;

        nvResourceViewDesc.Format = DXGI_FORMAT_R8G8_UNORM;

        result = ID3D11Device_CreateShaderResourceView(rendererData->d3dDevice,
            (ID3D11Resource *)textureData->mainTextureNV,
            &nvResourceViewDesc,
            &textureData->mainTextureResourceViewNV
            );
        if (FAILED(result)) {
            D3D11_DestroyTexture(renderer, texture);
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateShaderResourceView"), result);
            return -1;
        }
    }

    if (texture->access & SDL_TEXTUREACCESS_TARGET) {
        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
        renderTargetViewDesc.Format = textureDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;

        result = ID3D11Device_CreateRenderTargetView(rendererData->d3dDevice,
            (ID3D11Resource *)textureData->mainTexture,
            &renderTargetViewDesc,
            &textureData->mainTextureRenderTargetView);
        if (FAILED(result)) {
            D3D11_DestroyTexture(renderer, texture);
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateRenderTargetView"), result);
            return -1;
        }
    }

    return 0;
}

static void
D3D11_DestroyTexture(SDL_Renderer * renderer,
                     SDL_Texture * texture)
{
    D3D11_TextureData *data = (D3D11_TextureData *)texture->driverdata;

    if (!data) {
        return;
    }

    SAFE_RELEASE(data->mainTexture);
    SAFE_RELEASE(data->mainTextureResourceView);
    SAFE_RELEASE(data->mainTextureRenderTargetView);
    SAFE_RELEASE(data->stagingTexture);
    SAFE_RELEASE(data->mainTextureU);
    SAFE_RELEASE(data->mainTextureResourceViewU);
    SAFE_RELEASE(data->mainTextureV);
    SAFE_RELEASE(data->mainTextureResourceViewV);
    SDL_free(data->pixels);
    SDL_free(data);
    texture->driverdata = NULL;
}

static int
D3D11_UpdateTextureInternal(D3D11_RenderData *rendererData, ID3D11Texture2D *texture, int bpp, int x, int y, int w, int h, const void *pixels, int pitch)
{
    ID3D11Texture2D *stagingTexture;
    const Uint8 *src;
    Uint8 *dst;
    int row;
    UINT length;
    HRESULT result;
    D3D11_TEXTURE2D_DESC stagingTextureDesc;
    D3D11_MAPPED_SUBRESOURCE textureMemory;

    /* Create a 'staging' texture, which will be used to write to a portion of the main texture. */
    ID3D11Texture2D_GetDesc(texture, &stagingTextureDesc);
    stagingTextureDesc.Width = w;
    stagingTextureDesc.Height = h;
    stagingTextureDesc.BindFlags = 0;
    stagingTextureDesc.MiscFlags = 0;
    stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
    result = ID3D11Device_CreateTexture2D(rendererData->d3dDevice,
        &stagingTextureDesc,
        NULL,
        &stagingTexture);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateTexture2D [create staging texture]"), result);
        return -1;
    }

    /* Get a write-only pointer to data in the staging texture: */
    result = ID3D11DeviceContext_Map(rendererData->d3dContext,
        (ID3D11Resource *)stagingTexture,
        0,
        D3D11_MAP_WRITE,
        0,
        &textureMemory
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11DeviceContext1::Map [map staging texture]"), result);
        SAFE_RELEASE(stagingTexture);
        return -1;
    }

    src = (const Uint8 *)pixels;
    dst = textureMemory.pData;
    length = w * bpp;
    if (length == pitch && length == textureMemory.RowPitch) {
        SDL_memcpy(dst, src, length*h);
    } else {
        if (length > (UINT)pitch) {
            length = pitch;
        }
        if (length > textureMemory.RowPitch) {
            length = textureMemory.RowPitch;
        }
        for (row = 0; row < h; ++row) {
            SDL_memcpy(dst, src, length);
            src += pitch;
            dst += textureMemory.RowPitch;
        }
    }

    /* Commit the pixel buffer's changes back to the staging texture: */
    ID3D11DeviceContext_Unmap(rendererData->d3dContext,
        (ID3D11Resource *)stagingTexture,
        0);

    /* Copy the staging texture's contents back to the texture: */
    ID3D11DeviceContext_CopySubresourceRegion(rendererData->d3dContext,
        (ID3D11Resource *)texture,
        0,
        x,
        y,
        0,
        (ID3D11Resource *)stagingTexture,
        0,
        NULL);

    SAFE_RELEASE(stagingTexture);

    return 0;
}

static int
D3D11_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                    const SDL_Rect * rect, const void * srcPixels,
                    int srcPitch)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *)renderer->driverdata;
    D3D11_TextureData *textureData = (D3D11_TextureData *)texture->driverdata;

    if (!textureData) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    if (D3D11_UpdateTextureInternal(rendererData, textureData->mainTexture, SDL_BYTESPERPIXEL(texture->format), rect->x, rect->y, rect->w, rect->h, srcPixels, srcPitch) < 0) {
        return -1;
    }

    if (textureData->yuv) {
        /* Skip to the correct offset into the next texture */
        srcPixels = (const void*)((const Uint8*)srcPixels + rect->h * srcPitch);

        if (D3D11_UpdateTextureInternal(rendererData, texture->format == SDL_PIXELFORMAT_YV12 ? textureData->mainTextureV : textureData->mainTextureU, SDL_BYTESPERPIXEL(texture->format), rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2, srcPixels, (srcPitch + 1) / 2) < 0) {
            return -1;
        }

        /* Skip to the correct offset into the next texture */
        srcPixels = (const void*)((const Uint8*)srcPixels + ((rect->h + 1) / 2) * ((srcPitch + 1) / 2));
        if (D3D11_UpdateTextureInternal(rendererData, texture->format == SDL_PIXELFORMAT_YV12 ? textureData->mainTextureU : textureData->mainTextureV, SDL_BYTESPERPIXEL(texture->format), rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2, srcPixels, (srcPitch + 1) / 2) < 0) {
            return -1;
        }
    }

    if (textureData->nv12) {
        /* Skip to the correct offset into the next texture */
        srcPixels = (const void*)((const Uint8*)srcPixels + rect->h * srcPitch);

        if (D3D11_UpdateTextureInternal(rendererData, textureData->mainTextureNV, 2, rect->x / 2, rect->y / 2, ((rect->w + 1) / 2), (rect->h + 1) / 2, srcPixels, 2*((srcPitch + 1) / 2)) < 0) {
            return -1;
        }
    }
    return 0;
}

static int
D3D11_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                       const SDL_Rect * rect,
                       const Uint8 *Yplane, int Ypitch,
                       const Uint8 *Uplane, int Upitch,
                       const Uint8 *Vplane, int Vpitch)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *)renderer->driverdata;
    D3D11_TextureData *textureData = (D3D11_TextureData *)texture->driverdata;

    if (!textureData) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    if (D3D11_UpdateTextureInternal(rendererData, textureData->mainTexture, SDL_BYTESPERPIXEL(texture->format), rect->x, rect->y, rect->w, rect->h, Yplane, Ypitch) < 0) {
        return -1;
    }
    if (D3D11_UpdateTextureInternal(rendererData, textureData->mainTextureU, SDL_BYTESPERPIXEL(texture->format), rect->x / 2, rect->y / 2, rect->w / 2, rect->h / 2, Uplane, Upitch) < 0) {
        return -1;
    }
    if (D3D11_UpdateTextureInternal(rendererData, textureData->mainTextureV, SDL_BYTESPERPIXEL(texture->format), rect->x / 2, rect->y / 2, rect->w / 2, rect->h / 2, Vplane, Vpitch) < 0) {
        return -1;
    }
    return 0;
}

static int
D3D11_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                  const SDL_Rect * rect, void **pixels, int *pitch)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_TextureData *textureData = (D3D11_TextureData *) texture->driverdata;
    HRESULT result = S_OK;
    D3D11_TEXTURE2D_DESC stagingTextureDesc;
    D3D11_MAPPED_SUBRESOURCE textureMemory;

    if (!textureData) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    if (textureData->yuv || textureData->nv12) {
        /* It's more efficient to upload directly... */
        if (!textureData->pixels) {
            textureData->pitch = texture->w;
            textureData->pixels = (Uint8 *)SDL_malloc((texture->h * textureData->pitch * 3) / 2);
            if (!textureData->pixels) {
                return SDL_OutOfMemory();
            }
        }
        textureData->locked_rect = *rect;
        *pixels =
            (void *)((Uint8 *)textureData->pixels + rect->y * textureData->pitch +
            rect->x * SDL_BYTESPERPIXEL(texture->format));
        *pitch = textureData->pitch;
        return 0;
    }

    if (textureData->stagingTexture) {
        return SDL_SetError("texture is already locked");
    }
    
    /* Create a 'staging' texture, which will be used to write to a portion
     * of the main texture.  This is necessary, as Direct3D 11.1 does not
     * have the ability to write a CPU-bound pixel buffer to a rectangular
     * subrect of a texture.  Direct3D 11.1 can, however, write a pixel
     * buffer to an entire texture, hence the use of a staging texture.
     *
     * TODO, WinRT: consider avoiding the use of a staging texture in D3D11_LockTexture if/when the entire texture is being updated
     */
    ID3D11Texture2D_GetDesc(textureData->mainTexture, &stagingTextureDesc);
    stagingTextureDesc.Width = rect->w;
    stagingTextureDesc.Height = rect->h;
    stagingTextureDesc.BindFlags = 0;
    stagingTextureDesc.MiscFlags = 0;
    stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
    result = ID3D11Device_CreateTexture2D(rendererData->d3dDevice,
        &stagingTextureDesc,
        NULL,
        &textureData->stagingTexture);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateTexture2D [create staging texture]"), result);
        return -1;
    }

    /* Get a write-only pointer to data in the staging texture: */
    result = ID3D11DeviceContext_Map(rendererData->d3dContext,
        (ID3D11Resource *)textureData->stagingTexture,
        0,
        D3D11_MAP_WRITE,
        0,
        &textureMemory
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11DeviceContext1::Map [map staging texture]"), result);
        SAFE_RELEASE(textureData->stagingTexture);
        return -1;
    }

    /* Make note of where the staging texture will be written to 
     * (on a call to SDL_UnlockTexture):
     */
    textureData->lockedTexturePositionX = rect->x;
    textureData->lockedTexturePositionY = rect->y;

    /* Make sure the caller has information on the texture's pixel buffer,
     * then return:
     */
    *pixels = textureMemory.pData;
    *pitch = textureMemory.RowPitch;
    return 0;
}

static void
D3D11_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_TextureData *textureData = (D3D11_TextureData *) texture->driverdata;
    
    if (!textureData) {
        return;
    }

    if (textureData->yuv || textureData->nv12) {
        const SDL_Rect *rect = &textureData->locked_rect;
        void *pixels =
            (void *) ((Uint8 *) textureData->pixels + rect->y * textureData->pitch +
                      rect->x * SDL_BYTESPERPIXEL(texture->format));
        D3D11_UpdateTexture(renderer, texture, rect, pixels, textureData->pitch);
        return;
    }

    /* Commit the pixel buffer's changes back to the staging texture: */
    ID3D11DeviceContext_Unmap(rendererData->d3dContext,
        (ID3D11Resource *)textureData->stagingTexture,
        0);

    /* Copy the staging texture's contents back to the main texture: */
    ID3D11DeviceContext_CopySubresourceRegion(rendererData->d3dContext,
        (ID3D11Resource *)textureData->mainTexture,
        0,
        textureData->lockedTexturePositionX,
        textureData->lockedTexturePositionY,
        0,
        (ID3D11Resource *)textureData->stagingTexture,
        0,
        NULL);

    SAFE_RELEASE(textureData->stagingTexture);
}

static int
D3D11_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_TextureData *textureData = NULL;

    if (texture == NULL) {
        rendererData->currentOffscreenRenderTargetView = NULL;
        return 0;
    }

    textureData = (D3D11_TextureData *) texture->driverdata;

    if (!textureData->mainTextureRenderTargetView) {
        return SDL_SetError("specified texture is not a render target");
    }

    rendererData->currentOffscreenRenderTargetView = textureData->mainTextureRenderTargetView;

    return 0;
}

static void
D3D11_SetModelMatrix(SDL_Renderer *renderer, const Float4X4 *matrix)
{
    D3D11_RenderData *data = (D3D11_RenderData *)renderer->driverdata;

    if (matrix) {
        data->vertexShaderConstantsData.model = *matrix;
    } else {
        data->vertexShaderConstantsData.model = MatrixIdentity();
    }

    ID3D11DeviceContext_UpdateSubresource(data->d3dContext,
        (ID3D11Resource *)data->vertexShaderConstants,
        0,
        NULL,
        &data->vertexShaderConstantsData,
        0,
        0
        );
}

static int
D3D11_UpdateViewport(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    Float4X4 projection;
    Float4X4 view;
    SDL_FRect orientationAlignedViewport;
    BOOL swapDimensions;
    D3D11_VIEWPORT viewport;
    const int rotation = D3D11_GetRotationForCurrentRenderTarget(renderer);

    if (renderer->viewport.w == 0 || renderer->viewport.h == 0) {
        /* If the viewport is empty, assume that it is because
         * SDL_CreateRenderer is calling it, and will call it again later
         * with a non-empty viewport.
         */
        /* SDL_Log("%s, no viewport was set!\n", __FUNCTION__); */
        return 0;
    }

    /* Make sure the SDL viewport gets rotated to that of the physical display's rotation.
     * Keep in mind here that the Y-axis will be been inverted (from Direct3D's
     * default coordinate system) so rotations will be done in the opposite
     * direction of the DXGI_MODE_ROTATION enumeration.
     */
    switch (rotation) {
        case DXGI_MODE_ROTATION_IDENTITY:
            projection = MatrixIdentity();
            break;
        case DXGI_MODE_ROTATION_ROTATE270:
            projection = MatrixRotationZ(SDL_static_cast(float, M_PI * 0.5f));
            break;
        case DXGI_MODE_ROTATION_ROTATE180:
            projection = MatrixRotationZ(SDL_static_cast(float, M_PI));
            break;
        case DXGI_MODE_ROTATION_ROTATE90:
            projection = MatrixRotationZ(SDL_static_cast(float, -M_PI * 0.5f));
            break;
        default:
            return SDL_SetError("An unknown DisplayOrientation is being used");
    }

    /* Update the view matrix */
    view.m[0][0] = 2.0f / renderer->viewport.w;
    view.m[0][1] = 0.0f;
    view.m[0][2] = 0.0f;
    view.m[0][3] = 0.0f;
    view.m[1][0] = 0.0f;
    view.m[1][1] = -2.0f / renderer->viewport.h;
    view.m[1][2] = 0.0f;
    view.m[1][3] = 0.0f;
    view.m[2][0] = 0.0f;
    view.m[2][1] = 0.0f;
    view.m[2][2] = 1.0f;
    view.m[2][3] = 0.0f;
    view.m[3][0] = -1.0f;
    view.m[3][1] = 1.0f;
    view.m[3][2] = 0.0f;
    view.m[3][3] = 1.0f;

    /* Combine the projection + view matrix together now, as both only get
     * set here (as of this writing, on Dec 26, 2013).  When done, store it
     * for eventual transfer to the GPU.
     */
    data->vertexShaderConstantsData.projectionAndView = MatrixMultiply(
            view,
            projection);

    /* Reset the model matrix */
    D3D11_SetModelMatrix(renderer, NULL);

    /* Update the Direct3D viewport, which seems to be aligned to the
     * swap buffer's coordinate space, which is always in either
     * a landscape mode, for all Windows 8/RT devices, or a portrait mode,
     * for Windows Phone devices.
     */
    swapDimensions = D3D11_IsDisplayRotated90Degrees(rotation);
    if (swapDimensions) {
        orientationAlignedViewport.x = (float) renderer->viewport.y;
        orientationAlignedViewport.y = (float) renderer->viewport.x;
        orientationAlignedViewport.w = (float) renderer->viewport.h;
        orientationAlignedViewport.h = (float) renderer->viewport.w;
    } else {
        orientationAlignedViewport.x = (float) renderer->viewport.x;
        orientationAlignedViewport.y = (float) renderer->viewport.y;
        orientationAlignedViewport.w = (float) renderer->viewport.w;
        orientationAlignedViewport.h = (float) renderer->viewport.h;
    }
    /* TODO, WinRT: get custom viewports working with non-Landscape modes (Portrait, PortraitFlipped, and LandscapeFlipped) */

    viewport.TopLeftX = orientationAlignedViewport.x;
    viewport.TopLeftY = orientationAlignedViewport.y;
    viewport.Width = orientationAlignedViewport.w;
    viewport.Height = orientationAlignedViewport.h;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    /* SDL_Log("%s: D3D viewport = {%f,%f,%f,%f}\n", __FUNCTION__, viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height); */
    ID3D11DeviceContext_RSSetViewports(data->d3dContext, 1, &viewport);

    return 0;
}

static int
D3D11_UpdateClipRect(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;

    if (!renderer->clipping_enabled) {
        ID3D11DeviceContext_RSSetScissorRects(data->d3dContext, 0, NULL);
    } else {
        D3D11_RECT scissorRect;
        if (D3D11_GetViewportAlignedD3DRect(renderer, &renderer->clip_rect, &scissorRect, TRUE) != 0) {
            /* D3D11_GetViewportAlignedD3DRect will have set the SDL error */
            return -1;
        }
        ID3D11DeviceContext_RSSetScissorRects(data->d3dContext, 1, &scissorRect);
    }

    return 0;
}

static void
D3D11_ReleaseMainRenderTargetView(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *)renderer->driverdata;
    ID3D11DeviceContext_OMSetRenderTargets(data->d3dContext, 0, NULL, NULL);
    SAFE_RELEASE(data->mainRenderTargetView);
}

static ID3D11RenderTargetView *
D3D11_GetCurrentRenderTargetView(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    if (data->currentOffscreenRenderTargetView) {
        return data->currentOffscreenRenderTargetView;
    } else {
        return data->mainRenderTargetView;
    }
}

static int
D3D11_RenderClear(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    const float colorRGBA[] = {
        (renderer->r / 255.0f),
        (renderer->g / 255.0f),
        (renderer->b / 255.0f),
        (renderer->a / 255.0f)
    };
    ID3D11DeviceContext_ClearRenderTargetView(data->d3dContext,
        D3D11_GetCurrentRenderTargetView(renderer),
        colorRGBA
        );
    return 0;
}

static int
D3D11_UpdateVertexBuffer(SDL_Renderer *renderer,
                         const void * vertexData, size_t dataSizeInBytes)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_BUFFER_DESC vertexBufferDesc;
    HRESULT result = S_OK;
    D3D11_SUBRESOURCE_DATA vertexBufferData;
    const UINT stride = sizeof(VertexPositionColor);
    const UINT offset = 0;

    if (rendererData->vertexBuffer) {
        ID3D11Buffer_GetDesc(rendererData->vertexBuffer, &vertexBufferDesc);
    } else {
        SDL_zero(vertexBufferDesc);
    }

    if (rendererData->vertexBuffer && vertexBufferDesc.ByteWidth >= dataSizeInBytes) {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        result = ID3D11DeviceContext_Map(rendererData->d3dContext,
            (ID3D11Resource *)rendererData->vertexBuffer,
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mappedResource
            );
        if (FAILED(result)) {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11DeviceContext1::Map [vertex buffer]"), result);
            return -1;
        }
        SDL_memcpy(mappedResource.pData, vertexData, dataSizeInBytes);
        ID3D11DeviceContext_Unmap(rendererData->d3dContext, (ID3D11Resource *)rendererData->vertexBuffer, 0);
    } else {
        SAFE_RELEASE(rendererData->vertexBuffer);

        vertexBufferDesc.ByteWidth = (UINT) dataSizeInBytes;
        vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        SDL_zero(vertexBufferData);
        vertexBufferData.pSysMem = vertexData;
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        result = ID3D11Device_CreateBuffer(rendererData->d3dDevice,
            &vertexBufferDesc,
            &vertexBufferData,
            &rendererData->vertexBuffer
            );
        if (FAILED(result)) {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateBuffer [vertex buffer]"), result);
            return -1;
        }

        ID3D11DeviceContext_IASetVertexBuffers(rendererData->d3dContext,
            0,
            1,
            &rendererData->vertexBuffer,
            &stride,
            &offset
            );
    }

    return 0;
}

static void
D3D11_RenderStartDrawOp(SDL_Renderer * renderer)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *)renderer->driverdata;
    ID3D11RasterizerState *rasterizerState;
    ID3D11RenderTargetView *renderTargetView = D3D11_GetCurrentRenderTargetView(renderer);
    if (renderTargetView != rendererData->currentRenderTargetView) {
        ID3D11DeviceContext_OMSetRenderTargets(rendererData->d3dContext,
            1,
            &renderTargetView,
            NULL
            );
        rendererData->currentRenderTargetView = renderTargetView;
    }

    if (!renderer->clipping_enabled) {
        rasterizerState = rendererData->mainRasterizer;
    } else {
        rasterizerState = rendererData->clippedRasterizer;
    }
    if (rasterizerState != rendererData->currentRasterizerState) {
        ID3D11DeviceContext_RSSetState(rendererData->d3dContext, rasterizerState);
        rendererData->currentRasterizerState = rasterizerState;
    }
}

static void
D3D11_RenderSetBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *)renderer->driverdata;
    ID3D11BlendState *blendState = NULL;
    if (blendMode != SDL_BLENDMODE_NONE) {
        int i;
        for (i = 0; i < rendererData->blendModesCount; ++i) {
            if (blendMode == rendererData->blendModes[i].blendMode) {
                blendState = rendererData->blendModes[i].blendState;
                break;
            }
        }
        if (!blendState) {
            if (D3D11_CreateBlendState(renderer, blendMode)) {
                /* Successfully created the blend state, try again */
                D3D11_RenderSetBlendMode(renderer, blendMode);
            }
            return;
        }
    }
    if (blendState != rendererData->currentBlendState) {
        ID3D11DeviceContext_OMSetBlendState(rendererData->d3dContext, blendState, 0, 0xFFFFFFFF);
        rendererData->currentBlendState = blendState;
    }
}

static void
D3D11_SetPixelShader(SDL_Renderer * renderer,
                     ID3D11PixelShader * shader,
                     int numShaderResources,
                     ID3D11ShaderResourceView ** shaderResources,
                     ID3D11SamplerState * sampler)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    ID3D11ShaderResourceView *shaderResource;
    if (shader != rendererData->currentShader) {
        ID3D11DeviceContext_PSSetShader(rendererData->d3dContext, shader, NULL, 0);
        rendererData->currentShader = shader;
    }
    if (numShaderResources > 0) {
        shaderResource = shaderResources[0];
    } else {
        shaderResource = NULL;
    }
    if (shaderResource != rendererData->currentShaderResource) {
        ID3D11DeviceContext_PSSetShaderResources(rendererData->d3dContext, 0, numShaderResources, shaderResources);
        rendererData->currentShaderResource = shaderResource;
    }
    if (sampler != rendererData->currentSampler) {
        ID3D11DeviceContext_PSSetSamplers(rendererData->d3dContext, 0, 1, &sampler);
        rendererData->currentSampler = sampler;
    }
}

static void
D3D11_RenderFinishDrawOp(SDL_Renderer * renderer,
                         D3D11_PRIMITIVE_TOPOLOGY primitiveTopology,
                         UINT vertexCount)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;

    ID3D11DeviceContext_IASetPrimitiveTopology(rendererData->d3dContext, primitiveTopology);
    ID3D11DeviceContext_Draw(rendererData->d3dContext, vertexCount, 0);
}

static int
D3D11_RenderDrawPoints(SDL_Renderer * renderer,
                       const SDL_FPoint * points, int count)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    float r, g, b, a;
    VertexPositionColor *vertices;
    int i;

    r = (float)(renderer->r / 255.0f);
    g = (float)(renderer->g / 255.0f);
    b = (float)(renderer->b / 255.0f);
    a = (float)(renderer->a / 255.0f);

    vertices = SDL_stack_alloc(VertexPositionColor, count);
    for (i = 0; i < count; ++i) {
        const VertexPositionColor v = { { points[i].x + 0.5f, points[i].y + 0.5f, 0.0f }, { 0.0f, 0.0f }, { r, g, b, a } };
        vertices[i] = v;
    }

    D3D11_RenderStartDrawOp(renderer);
    D3D11_RenderSetBlendMode(renderer, renderer->blendMode);
    if (D3D11_UpdateVertexBuffer(renderer, vertices, (unsigned int)count * sizeof(VertexPositionColor)) != 0) {
        SDL_stack_free(vertices);
        return -1;
    }

    D3D11_SetPixelShader(
        renderer,
        rendererData->pixelShaders[SHADER_SOLID],
        0,
        NULL,
        NULL);

    D3D11_RenderFinishDrawOp(renderer, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, count);
    SDL_stack_free(vertices);
    return 0;
}

static int
D3D11_RenderDrawLines(SDL_Renderer * renderer,
                      const SDL_FPoint * points, int count)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    float r, g, b, a;
    VertexPositionColor *vertices;
    int i;

    r = (float)(renderer->r / 255.0f);
    g = (float)(renderer->g / 255.0f);
    b = (float)(renderer->b / 255.0f);
    a = (float)(renderer->a / 255.0f);

    vertices = SDL_stack_alloc(VertexPositionColor, count);
    for (i = 0; i < count; ++i) {
        const VertexPositionColor v = { { points[i].x + 0.5f, points[i].y + 0.5f, 0.0f }, { 0.0f, 0.0f }, { r, g, b, a } };
        vertices[i] = v;
    }

    D3D11_RenderStartDrawOp(renderer);
    D3D11_RenderSetBlendMode(renderer, renderer->blendMode);
    if (D3D11_UpdateVertexBuffer(renderer, vertices, (unsigned int)count * sizeof(VertexPositionColor)) != 0) {
        SDL_stack_free(vertices);
        return -1;
    }

    D3D11_SetPixelShader(
        renderer,
        rendererData->pixelShaders[SHADER_SOLID],
        0,
        NULL,
        NULL);

    D3D11_RenderFinishDrawOp(renderer, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, count);

    if (points[0].x != points[count - 1].x || points[0].y != points[count - 1].y) {
        ID3D11DeviceContext_IASetPrimitiveTopology(rendererData->d3dContext, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        ID3D11DeviceContext_Draw(rendererData->d3dContext, 1, count - 1);
    }

    SDL_stack_free(vertices);
    return 0;
}

static int
D3D11_RenderFillRects(SDL_Renderer * renderer,
                      const SDL_FRect * rects, int count)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    float r, g, b, a;
    int i;

    r = (float)(renderer->r / 255.0f);
    g = (float)(renderer->g / 255.0f);
    b = (float)(renderer->b / 255.0f);
    a = (float)(renderer->a / 255.0f);

    for (i = 0; i < count; ++i) {
        VertexPositionColor vertices[] = {
            { { rects[i].x, rects[i].y, 0.0f },                             { 0.0f, 0.0f}, {r, g, b, a} },
            { { rects[i].x, rects[i].y + rects[i].h, 0.0f },                { 0.0f, 0.0f }, { r, g, b, a } },
            { { rects[i].x + rects[i].w, rects[i].y, 0.0f },                { 0.0f, 0.0f }, { r, g, b, a } },
            { { rects[i].x + rects[i].w, rects[i].y + rects[i].h, 0.0f },   { 0.0f, 0.0f }, { r, g, b, a } },
        };

        D3D11_RenderStartDrawOp(renderer);
        D3D11_RenderSetBlendMode(renderer, renderer->blendMode);
        if (D3D11_UpdateVertexBuffer(renderer, vertices, sizeof(vertices)) != 0) {
            return -1;
        }

        D3D11_SetPixelShader(
            renderer,
            rendererData->pixelShaders[SHADER_SOLID],
            0,
            NULL,
            NULL);

        D3D11_RenderFinishDrawOp(renderer, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, SDL_arraysize(vertices));
    }

    return 0;
}

static int
D3D11_RenderSetupSampler(SDL_Renderer * renderer, SDL_Texture * texture)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_TextureData *textureData = (D3D11_TextureData *) texture->driverdata;
    ID3D11SamplerState *textureSampler;

    switch (textureData->scaleMode) {
    case D3D11_FILTER_MIN_MAG_MIP_POINT:
        textureSampler = rendererData->nearestPixelSampler;
        break;
    case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
        textureSampler = rendererData->linearSampler;
        break;
    default:
        return SDL_SetError("Unknown scale mode: %d\n", textureData->scaleMode);
    }

    if (textureData->yuv) {
        ID3D11ShaderResourceView *shaderResources[] = {
            textureData->mainTextureResourceView,
            textureData->mainTextureResourceViewU,
            textureData->mainTextureResourceViewV
        };
        D3D11_Shader shader;

        switch (SDL_GetYUVConversionModeForResolution(texture->w, texture->h)) {
        case SDL_YUV_CONVERSION_JPEG:
            shader = SHADER_YUV_JPEG;
            break;
        case SDL_YUV_CONVERSION_BT601:
            shader = SHADER_YUV_BT601;
            break;
        case SDL_YUV_CONVERSION_BT709:
            shader = SHADER_YUV_BT709;
            break;
        default:
            return SDL_SetError("Unsupported YUV conversion mode");
        }

        D3D11_SetPixelShader(
            renderer,
            rendererData->pixelShaders[shader],
            SDL_arraysize(shaderResources),
            shaderResources,
            textureSampler);

    } else if (textureData->nv12) {
        ID3D11ShaderResourceView *shaderResources[] = {
            textureData->mainTextureResourceView,
            textureData->mainTextureResourceViewNV,
        };
        D3D11_Shader shader;

        switch (SDL_GetYUVConversionModeForResolution(texture->w, texture->h)) {
        case SDL_YUV_CONVERSION_JPEG:
            shader = texture->format == SDL_PIXELFORMAT_NV12 ? SHADER_NV12_JPEG : SHADER_NV21_JPEG;
            break;
        case SDL_YUV_CONVERSION_BT601:
            shader = texture->format == SDL_PIXELFORMAT_NV12 ? SHADER_NV12_BT601 : SHADER_NV21_BT601;
            break;
        case SDL_YUV_CONVERSION_BT709:
            shader = texture->format == SDL_PIXELFORMAT_NV12 ? SHADER_NV12_BT709 : SHADER_NV21_BT709;
            break;
        default:
            return SDL_SetError("Unsupported YUV conversion mode");
        }

        D3D11_SetPixelShader(
            renderer,
            rendererData->pixelShaders[shader],
            SDL_arraysize(shaderResources),
            shaderResources,
            textureSampler);

    } else {
        D3D11_SetPixelShader(
            renderer,
            rendererData->pixelShaders[SHADER_RGB],
            1,
            &textureData->mainTextureResourceView,
            textureSampler);
    }

    return 0;
}

static int
D3D11_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                 const SDL_Rect * srcrect, const SDL_FRect * dstrect)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_TextureData *textureData = (D3D11_TextureData *) texture->driverdata;
    float minu, maxu, minv, maxv;
    Float4 color;
    VertexPositionColor vertices[4];

    D3D11_RenderStartDrawOp(renderer);
    D3D11_RenderSetBlendMode(renderer, texture->blendMode);

    minu = (float) srcrect->x / texture->w;
    maxu = (float) (srcrect->x + srcrect->w) / texture->w;
    minv = (float) srcrect->y / texture->h;
    maxv = (float) (srcrect->y + srcrect->h) / texture->h;

    color.x = 1.0f;     /* red */
    color.y = 1.0f;     /* green */
    color.z = 1.0f;     /* blue */
    color.w = 1.0f;     /* alpha */
    if (texture->modMode & SDL_TEXTUREMODULATE_COLOR) {
        color.x = (float)(texture->r / 255.0f);     /* red */
        color.y = (float)(texture->g / 255.0f);     /* green */
        color.z = (float)(texture->b / 255.0f);     /* blue */
    }
    if (texture->modMode & SDL_TEXTUREMODULATE_ALPHA) {
        color.w = (float)(texture->a / 255.0f);     /* alpha */
    }

    vertices[0].pos.x = dstrect->x;
    vertices[0].pos.y = dstrect->y;
    vertices[0].pos.z = 0.0f;
    vertices[0].tex.x = minu;
    vertices[0].tex.y = minv;
    vertices[0].color = color;

    vertices[1].pos.x = dstrect->x;
    vertices[1].pos.y = dstrect->y + dstrect->h;
    vertices[1].pos.z = 0.0f;
    vertices[1].tex.x = minu;
    vertices[1].tex.y = maxv;
    vertices[1].color = color;

    vertices[2].pos.x = dstrect->x + dstrect->w;
    vertices[2].pos.y = dstrect->y;
    vertices[2].pos.z = 0.0f;
    vertices[2].tex.x = maxu;
    vertices[2].tex.y = minv;
    vertices[2].color = color;

    vertices[3].pos.x = dstrect->x + dstrect->w;
    vertices[3].pos.y = dstrect->y + dstrect->h;
    vertices[3].pos.z = 0.0f;
    vertices[3].tex.x = maxu;
    vertices[3].tex.y = maxv;
    vertices[3].color = color;

    if (D3D11_UpdateVertexBuffer(renderer, vertices, sizeof(vertices)) != 0) {
        return -1;
    }

    if (D3D11_RenderSetupSampler(renderer, texture) < 0) {
        return -1;
    }

    D3D11_RenderFinishDrawOp(renderer, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, sizeof(vertices) / sizeof(VertexPositionColor));

    return 0;
}

static int
D3D11_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                   const SDL_Rect * srcrect, const SDL_FRect * dstrect,
                   const double angle, const SDL_FPoint * center, const SDL_RendererFlip flip)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_TextureData *textureData = (D3D11_TextureData *) texture->driverdata;
    float minu, maxu, minv, maxv;
    Float4 color;
    Float4X4 modelMatrix;
    float minx, maxx, miny, maxy;
    VertexPositionColor vertices[4];

    D3D11_RenderStartDrawOp(renderer);
    D3D11_RenderSetBlendMode(renderer, texture->blendMode);

    minu = (float) srcrect->x / texture->w;
    maxu = (float) (srcrect->x + srcrect->w) / texture->w;
    minv = (float) srcrect->y / texture->h;
    maxv = (float) (srcrect->y + srcrect->h) / texture->h;

    color.x = 1.0f;     /* red */
    color.y = 1.0f;     /* green */
    color.z = 1.0f;     /* blue */
    color.w = 1.0f;     /* alpha */
    if (texture->modMode & SDL_TEXTUREMODULATE_COLOR) {
        color.x = (float)(texture->r / 255.0f);     /* red */
        color.y = (float)(texture->g / 255.0f);     /* green */
        color.z = (float)(texture->b / 255.0f);     /* blue */
    }
    if (texture->modMode & SDL_TEXTUREMODULATE_ALPHA) {
        color.w = (float)(texture->a / 255.0f);     /* alpha */
    }

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

    modelMatrix = MatrixMultiply(
            MatrixRotationZ((float)(M_PI * (float) angle / 180.0f)),
            MatrixTranslation(dstrect->x + center->x, dstrect->y + center->y, 0)
            );
    D3D11_SetModelMatrix(renderer, &modelMatrix);

    minx = -center->x;
    maxx = dstrect->w - center->x;
    miny = -center->y;
    maxy = dstrect->h - center->y;

    vertices[0].pos.x = minx;
    vertices[0].pos.y = miny;
    vertices[0].pos.z = 0.0f;
    vertices[0].tex.x = minu;
    vertices[0].tex.y = minv;
    vertices[0].color = color;
    
    vertices[1].pos.x = minx;
    vertices[1].pos.y = maxy;
    vertices[1].pos.z = 0.0f;
    vertices[1].tex.x = minu;
    vertices[1].tex.y = maxv;
    vertices[1].color = color;
    
    vertices[2].pos.x = maxx;
    vertices[2].pos.y = miny;
    vertices[2].pos.z = 0.0f;
    vertices[2].tex.x = maxu;
    vertices[2].tex.y = minv;
    vertices[2].color = color;
    
    vertices[3].pos.x = maxx;
    vertices[3].pos.y = maxy;
    vertices[3].pos.z = 0.0f;
    vertices[3].tex.x = maxu;
    vertices[3].tex.y = maxv;
    vertices[3].color = color;

    if (D3D11_UpdateVertexBuffer(renderer, vertices, sizeof(vertices)) != 0) {
        return -1;
    }

    if (D3D11_RenderSetupSampler(renderer, texture) < 0) {
        return -1;
    }

    D3D11_RenderFinishDrawOp(renderer, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, sizeof(vertices) / sizeof(VertexPositionColor));

    D3D11_SetModelMatrix(renderer, NULL);

    return 0;
}

static int
D3D11_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                       Uint32 format, void * pixels, int pitch)
{
    D3D11_RenderData * data = (D3D11_RenderData *) renderer->driverdata;
    ID3D11Texture2D *backBuffer = NULL;
    ID3D11Texture2D *stagingTexture = NULL;
    HRESULT result;
    int status = -1;
    D3D11_TEXTURE2D_DESC stagingTextureDesc;
    D3D11_RECT srcRect = {0, 0, 0, 0};
    D3D11_BOX srcBox;
    D3D11_MAPPED_SUBRESOURCE textureMemory;

    /* Retrieve a pointer to the back buffer: */
    result = IDXGISwapChain_GetBuffer(data->swapChain,
        0,
        &SDL_IID_ID3D11Texture2D,
        (void **)&backBuffer
        );
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGISwapChain1::GetBuffer [get back buffer]"), result);
        goto done;
    }

    /* Create a staging texture to copy the screen's data to: */
    ID3D11Texture2D_GetDesc(backBuffer, &stagingTextureDesc);
    stagingTextureDesc.Width = rect->w;
    stagingTextureDesc.Height = rect->h;
    stagingTextureDesc.BindFlags = 0;
    stagingTextureDesc.MiscFlags = 0;
    stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
    result = ID3D11Device_CreateTexture2D(data->d3dDevice,
        &stagingTextureDesc,
        NULL,
        &stagingTexture);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11Device1::CreateTexture2D [create staging texture]"), result);
        goto done;
    }

    /* Copy the desired portion of the back buffer to the staging texture: */
    if (D3D11_GetViewportAlignedD3DRect(renderer, rect, &srcRect, FALSE) != 0) {
        /* D3D11_GetViewportAlignedD3DRect will have set the SDL error */
        goto done;
    }

    srcBox.left = srcRect.left;
    srcBox.right = srcRect.right;
    srcBox.top = srcRect.top;
    srcBox.bottom = srcRect.bottom;
    srcBox.front = 0;
    srcBox.back = 1;
    ID3D11DeviceContext_CopySubresourceRegion(data->d3dContext,
        (ID3D11Resource *)stagingTexture,
        0,
        0, 0, 0,
        (ID3D11Resource *)backBuffer,
        0,
        &srcBox);

    /* Map the staging texture's data to CPU-accessible memory: */
    result = ID3D11DeviceContext_Map(data->d3dContext,
        (ID3D11Resource *)stagingTexture,
        0,
        D3D11_MAP_READ,
        0,
        &textureMemory);
    if (FAILED(result)) {
        WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("ID3D11DeviceContext1::Map [map staging texture]"), result);
        goto done;
    }

    /* Copy the data into the desired buffer, converting pixels to the
     * desired format at the same time:
     */
    if (SDL_ConvertPixels(
        rect->w, rect->h,
        D3D11_DXGIFormatToSDLPixelFormat(stagingTextureDesc.Format),
        textureMemory.pData,
        textureMemory.RowPitch,
        format,
        pixels,
        pitch) != 0) {
        /* When SDL_ConvertPixels fails, it'll have already set the format.
         * Get the error message, and attach some extra data to it.
         */
        char errorMessage[1024];
        SDL_snprintf(errorMessage, sizeof(errorMessage), "%s, Convert Pixels failed: %s", __FUNCTION__, SDL_GetError());
        SDL_SetError("%s", errorMessage);
        goto done;
    }

    /* Unmap the texture: */
    ID3D11DeviceContext_Unmap(data->d3dContext,
        (ID3D11Resource *)stagingTexture,
        0);

    status = 0;

done:
    SAFE_RELEASE(backBuffer);
    SAFE_RELEASE(stagingTexture);
    return status;
}

static void
D3D11_RenderPresent(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    UINT syncInterval;
    UINT presentFlags;
    HRESULT result;
    DXGI_PRESENT_PARAMETERS parameters;

    SDL_zero(parameters);

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
    syncInterval = 1;
    presentFlags = 0;
    result = IDXGISwapChain_Present(data->swapChain, syncInterval, presentFlags);
#else
    if (renderer->info.flags & SDL_RENDERER_PRESENTVSYNC) {
        syncInterval = 1;
        presentFlags = 0;
    } else {
        syncInterval = 0;
        presentFlags = DXGI_PRESENT_DO_NOT_WAIT;
    }

    /* The application may optionally specify "dirty" or "scroll"
     * rects to improve efficiency in certain scenarios.
     * This option is not available on Windows Phone 8, to note.
     */
    result = IDXGISwapChain1_Present1(data->swapChain, syncInterval, presentFlags, &parameters);
#endif

    /* Discard the contents of the render target.
     * This is a valid operation only when the existing contents will be entirely
     * overwritten. If dirty or scroll rects are used, this call should be removed.
     */
    ID3D11DeviceContext1_DiscardView(data->d3dContext, (ID3D11View*)data->mainRenderTargetView);

    /* When the present flips, it unbinds the current view, so bind it again on the next draw call */
    data->currentRenderTargetView = NULL;

    if (FAILED(result) && result != DXGI_ERROR_WAS_STILL_DRAWING) {
        /* If the device was removed either by a disconnect or a driver upgrade, we 
         * must recreate all device resources.
         *
         * TODO, WinRT: consider throwing an exception if D3D11_RenderPresent fails, especially if there is a way to salvage debug info from users' machines
         */
        if ( result == DXGI_ERROR_DEVICE_REMOVED ) {
            D3D11_HandleDeviceLost(renderer);
        } else if (result == DXGI_ERROR_INVALID_CALL) {
            /* We probably went through a fullscreen <-> windowed transition */
            D3D11_CreateWindowSizeDependentResources(renderer);
        } else {
            WIN_SetErrorFromHRESULT(SDL_COMPOSE_ERROR("IDXGISwapChain::Present"), result);
        }
    }
}

#endif /* SDL_VIDEO_RENDER_D3D11 && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
