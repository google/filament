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

#if SDL_VIDEO_DRIVER_WINDOWS

#include "SDL_main.h"
#include "SDL_video.h"
#include "SDL_hints.h"
#include "SDL_mouse.h"
#include "SDL_system.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"

#include "SDL_windowsvideo.h"
#include "SDL_windowsframebuffer.h"
#include "SDL_windowsshape.h"
#include "SDL_windowsvulkan.h"

/* Initialization/Query functions */
static int WIN_VideoInit(_THIS);
static void WIN_VideoQuit(_THIS);

/* Hints */
SDL_bool g_WindowsEnableMessageLoop = SDL_TRUE;
SDL_bool g_WindowFrameUsableWhileCursorHidden = SDL_TRUE;

static void SDLCALL
UpdateWindowsEnableMessageLoop(void *userdata, const char *name, const char *oldValue, const char *newValue)
{
    if (newValue && *newValue == '0') {
        g_WindowsEnableMessageLoop = SDL_FALSE;
    } else {
        g_WindowsEnableMessageLoop = SDL_TRUE;
    }
}

static void SDLCALL
UpdateWindowFrameUsableWhileCursorHidden(void *userdata, const char *name, const char *oldValue, const char *newValue)
{
    if (newValue && *newValue == '0') {
        g_WindowFrameUsableWhileCursorHidden = SDL_FALSE;
    } else {
        g_WindowFrameUsableWhileCursorHidden = SDL_TRUE;
    }
}

static void WIN_SuspendScreenSaver(_THIS)
{
    if (_this->suspend_screensaver) {
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
    } else {
        SetThreadExecutionState(ES_CONTINUOUS);
    }
}


/* Windows driver bootstrap functions */

static void
WIN_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_VideoData *data = (SDL_VideoData *) device->driverdata;

    SDL_UnregisterApp();
    if (data->userDLL) {
        SDL_UnloadObject(data->userDLL);
    }
    if (data->shcoreDLL) {
        SDL_UnloadObject(data->shcoreDLL);
    }
    if (device->wakeup_lock) {
        SDL_DestroyMutex(device->wakeup_lock);
    }
    SDL_free(device->driverdata);
    SDL_free(device);
}

static SDL_VideoDevice *
WIN_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;
    SDL_VideoData *data;

    SDL_RegisterApp(NULL, 0, NULL);

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device) {
        data = (struct SDL_VideoData *) SDL_calloc(1, sizeof(SDL_VideoData));
    } else {
        data = NULL;
    }
    if (!data) {
        SDL_free(device);
        SDL_OutOfMemory();
        return NULL;
    }
    device->driverdata = data;
    device->wakeup_lock = SDL_CreateMutex();

    data->userDLL = SDL_LoadObject("USER32.DLL");
    if (data->userDLL) {
        data->CloseTouchInputHandle = (BOOL (WINAPI *)(HTOUCHINPUT)) SDL_LoadFunction(data->userDLL, "CloseTouchInputHandle");
        data->GetTouchInputInfo = (BOOL (WINAPI *)(HTOUCHINPUT, UINT, PTOUCHINPUT, int)) SDL_LoadFunction(data->userDLL, "GetTouchInputInfo");
        data->RegisterTouchWindow = (BOOL (WINAPI *)(HWND, ULONG)) SDL_LoadFunction(data->userDLL, "RegisterTouchWindow");
    } else {
        SDL_ClearError();
    }

    data->shcoreDLL = SDL_LoadObject("SHCORE.DLL");
    if (data->shcoreDLL) {
        data->GetDpiForMonitor = (HRESULT (WINAPI *)(HMONITOR, MONITOR_DPI_TYPE, UINT *, UINT *)) SDL_LoadFunction(data->shcoreDLL, "GetDpiForMonitor");
    } else {
        SDL_ClearError();
    }

    /* Set the function pointers */
    device->VideoInit = WIN_VideoInit;
    device->VideoQuit = WIN_VideoQuit;
    device->GetDisplayBounds = WIN_GetDisplayBounds;
    device->GetDisplayUsableBounds = WIN_GetDisplayUsableBounds;
    device->GetDisplayDPI = WIN_GetDisplayDPI;
    device->GetDisplayModes = WIN_GetDisplayModes;
    device->SetDisplayMode = WIN_SetDisplayMode;
    device->PumpEvents = WIN_PumpEvents;
    device->WaitEventTimeout = WIN_WaitEventTimeout;
    device->SendWakeupEvent = WIN_SendWakeupEvent;
    device->SuspendScreenSaver = WIN_SuspendScreenSaver;

    device->CreateSDLWindow = WIN_CreateWindow;
    device->CreateSDLWindowFrom = WIN_CreateWindowFrom;
    device->SetWindowTitle = WIN_SetWindowTitle;
    device->SetWindowIcon = WIN_SetWindowIcon;
    device->SetWindowPosition = WIN_SetWindowPosition;
    device->SetWindowSize = WIN_SetWindowSize;
    device->GetWindowBordersSize = WIN_GetWindowBordersSize;
    device->SetWindowOpacity = WIN_SetWindowOpacity;
    device->ShowWindow = WIN_ShowWindow;
    device->HideWindow = WIN_HideWindow;
    device->RaiseWindow = WIN_RaiseWindow;
    device->MaximizeWindow = WIN_MaximizeWindow;
    device->MinimizeWindow = WIN_MinimizeWindow;
    device->RestoreWindow = WIN_RestoreWindow;
    device->SetWindowBordered = WIN_SetWindowBordered;
    device->SetWindowResizable = WIN_SetWindowResizable;
    device->SetWindowAlwaysOnTop = WIN_SetWindowAlwaysOnTop;
    device->SetWindowFullscreen = WIN_SetWindowFullscreen;
    device->SetWindowGammaRamp = WIN_SetWindowGammaRamp;
    device->GetWindowGammaRamp = WIN_GetWindowGammaRamp;
    device->SetWindowMouseGrab = WIN_SetWindowMouseGrab;
    device->SetWindowKeyboardGrab = WIN_SetWindowKeyboardGrab;
    device->DestroyWindow = WIN_DestroyWindow;
    device->GetWindowWMInfo = WIN_GetWindowWMInfo;
    device->CreateWindowFramebuffer = WIN_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = WIN_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = WIN_DestroyWindowFramebuffer;
    device->OnWindowEnter = WIN_OnWindowEnter;
    device->SetWindowHitTest = WIN_SetWindowHitTest;
    device->AcceptDragAndDrop = WIN_AcceptDragAndDrop;
    device->FlashWindow = WIN_FlashWindow;

    device->shape_driver.CreateShaper = Win32_CreateShaper;
    device->shape_driver.SetWindowShape = Win32_SetWindowShape;
    device->shape_driver.ResizeWindowShape = Win32_ResizeWindowShape;

#if SDL_VIDEO_OPENGL_WGL
    device->GL_LoadLibrary = WIN_GL_LoadLibrary;
    device->GL_GetProcAddress = WIN_GL_GetProcAddress;
    device->GL_UnloadLibrary = WIN_GL_UnloadLibrary;
    device->GL_CreateContext = WIN_GL_CreateContext;
    device->GL_MakeCurrent = WIN_GL_MakeCurrent;
    device->GL_SetSwapInterval = WIN_GL_SetSwapInterval;
    device->GL_GetSwapInterval = WIN_GL_GetSwapInterval;
    device->GL_SwapWindow = WIN_GL_SwapWindow;
    device->GL_DeleteContext = WIN_GL_DeleteContext;
#elif SDL_VIDEO_OPENGL_EGL
    /* Use EGL based functions */
    device->GL_LoadLibrary = WIN_GLES_LoadLibrary;
    device->GL_GetProcAddress = WIN_GLES_GetProcAddress;
    device->GL_UnloadLibrary = WIN_GLES_UnloadLibrary;
    device->GL_CreateContext = WIN_GLES_CreateContext;
    device->GL_MakeCurrent = WIN_GLES_MakeCurrent;
    device->GL_SetSwapInterval = WIN_GLES_SetSwapInterval;
    device->GL_GetSwapInterval = WIN_GLES_GetSwapInterval;
    device->GL_SwapWindow = WIN_GLES_SwapWindow;
    device->GL_DeleteContext = WIN_GLES_DeleteContext;
#endif
#if SDL_VIDEO_VULKAN
    device->Vulkan_LoadLibrary = WIN_Vulkan_LoadLibrary;
    device->Vulkan_UnloadLibrary = WIN_Vulkan_UnloadLibrary;
    device->Vulkan_GetInstanceExtensions = WIN_Vulkan_GetInstanceExtensions;
    device->Vulkan_CreateSurface = WIN_Vulkan_CreateSurface;
#endif

    device->StartTextInput = WIN_StartTextInput;
    device->StopTextInput = WIN_StopTextInput;
    device->SetTextInputRect = WIN_SetTextInputRect;

    device->SetClipboardText = WIN_SetClipboardText;
    device->GetClipboardText = WIN_GetClipboardText;
    device->HasClipboardText = WIN_HasClipboardText;

    device->free = WIN_DeleteDevice;

    return device;
}


VideoBootStrap WINDOWS_bootstrap = {
    "windows", "SDL Windows video driver", WIN_CreateDevice
};

int
WIN_VideoInit(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    if (WIN_InitModes(_this) < 0) {
        return -1;
    }

    WIN_InitKeyboard(_this);
    WIN_InitMouse(_this);

    SDL_AddHintCallback(SDL_HINT_WINDOWS_ENABLE_MESSAGELOOP, UpdateWindowsEnableMessageLoop, NULL);
    SDL_AddHintCallback(SDL_HINT_WINDOW_FRAME_USABLE_WHILE_CURSOR_HIDDEN, UpdateWindowFrameUsableWhileCursorHidden, NULL);

    data->_SDL_WAKEUP = RegisterWindowMessageA("_SDL_WAKEUP");

    return 0;
}

void
WIN_VideoQuit(_THIS)
{
    WIN_QuitModes(_this);
    WIN_QuitKeyboard(_this);
    WIN_QuitMouse(_this);
}


#define D3D_DEBUG_INFO
#include <d3d9.h>

SDL_bool
D3D_LoadDLL(void **pD3DDLL, IDirect3D9 **pDirect3D9Interface)
{
    *pD3DDLL = SDL_LoadObject("D3D9.DLL");
    if (*pD3DDLL) {
        typedef IDirect3D9 *(WINAPI *Direct3DCreate9_t) (UINT SDKVersion);
        Direct3DCreate9_t Direct3DCreate9Func;

        if (SDL_GetHintBoolean(SDL_HINT_WINDOWS_USE_D3D9EX, SDL_FALSE)) {
            typedef HRESULT(WINAPI* Direct3DCreate9Ex_t)(UINT SDKVersion, IDirect3D9Ex** ppD3D);
            Direct3DCreate9Ex_t Direct3DCreate9ExFunc;

            Direct3DCreate9ExFunc = (Direct3DCreate9Ex_t)SDL_LoadFunction(*pD3DDLL, "Direct3DCreate9Ex");
            if (Direct3DCreate9ExFunc) {
                IDirect3D9Ex* pDirect3D9ExInterface;
                HRESULT hr = Direct3DCreate9ExFunc(D3D_SDK_VERSION, &pDirect3D9ExInterface);
                if (SUCCEEDED(hr)) {
                    const GUID IDirect3D9_GUID = { 0x81bdcbca, 0x64d4, 0x426d, { 0xae, 0x8d, 0xad, 0x1, 0x47, 0xf4, 0x27, 0x5c } };
                    hr = IDirect3D9Ex_QueryInterface(pDirect3D9ExInterface, &IDirect3D9_GUID, (void**)pDirect3D9Interface);
                    IDirect3D9Ex_Release(pDirect3D9ExInterface);
                    if (SUCCEEDED(hr)) {
                        return SDL_TRUE;
                    }
                }
            }
        }

        Direct3DCreate9Func = (Direct3DCreate9_t)SDL_LoadFunction(*pD3DDLL, "Direct3DCreate9");
        if (Direct3DCreate9Func) {
            *pDirect3D9Interface = Direct3DCreate9Func(D3D_SDK_VERSION);
            if (*pDirect3D9Interface) {
                return SDL_TRUE;
            }
        }

        SDL_UnloadObject(*pD3DDLL);
        *pD3DDLL = NULL;
    }
    *pDirect3D9Interface = NULL;
    return SDL_FALSE;
}


int
SDL_Direct3D9GetAdapterIndex(int displayIndex)
{
    void *pD3DDLL;
    IDirect3D9 *pD3D;
    if (!D3D_LoadDLL(&pD3DDLL, &pD3D)) {
        SDL_SetError("Unable to create Direct3D interface");
        return D3DADAPTER_DEFAULT;
    } else {
        SDL_DisplayData *pData = (SDL_DisplayData *)SDL_GetDisplayDriverData(displayIndex);
        int adapterIndex = D3DADAPTER_DEFAULT;

        if (!pData) {
            SDL_SetError("Invalid display index");
            adapterIndex = -1; /* make sure we return something invalid */
        } else {
            char *displayName = WIN_StringToUTF8W(pData->DeviceName);
            unsigned int count = IDirect3D9_GetAdapterCount(pD3D);
            unsigned int i;
            for (i=0; i<count; i++) {
                D3DADAPTER_IDENTIFIER9 id;
                IDirect3D9_GetAdapterIdentifier(pD3D, i, 0, &id);

                if (SDL_strcmp(id.DeviceName, displayName) == 0) {
                    adapterIndex = i;
                    break;
                }
            }
            SDL_free(displayName);
        }

        /* free up the D3D stuff we inited */
        IDirect3D9_Release(pD3D);
        SDL_UnloadObject(pD3DDLL);

        return adapterIndex;
    }
}

#if HAVE_DXGI_H
#define CINTERFACE
#define COBJMACROS
#include <dxgi.h>

static SDL_bool
DXGI_LoadDLL(void **pDXGIDLL, IDXGIFactory **pDXGIFactory)
{
    *pDXGIDLL = SDL_LoadObject("DXGI.DLL");
    if (*pDXGIDLL) {
        HRESULT (WINAPI *CreateDXGI)(REFIID riid, void **ppFactory);

        CreateDXGI =
            (HRESULT (WINAPI *) (REFIID, void**)) SDL_LoadFunction(*pDXGIDLL,
            "CreateDXGIFactory");
        if (CreateDXGI) {
            GUID dxgiGUID = {0x7b7166ec,0x21c7,0x44ae,{0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69}};
            if (!SUCCEEDED(CreateDXGI(&dxgiGUID, (void**)pDXGIFactory))) {
                *pDXGIFactory = NULL;
            }
        }
        if (!*pDXGIFactory) {
            SDL_UnloadObject(*pDXGIDLL);
            *pDXGIDLL = NULL;
            return SDL_FALSE;
        }

        return SDL_TRUE;
    } else {
        *pDXGIFactory = NULL;
        return SDL_FALSE;
    }
}
#endif


SDL_bool
SDL_DXGIGetOutputInfo(int displayIndex, int *adapterIndex, int *outputIndex)
{
#if !HAVE_DXGI_H
    if (adapterIndex) *adapterIndex = -1;
    if (outputIndex) *outputIndex = -1;
    SDL_SetError("SDL was compiled without DXGI support due to missing dxgi.h header");
    return SDL_FALSE;
#else
    SDL_DisplayData *pData = (SDL_DisplayData *)SDL_GetDisplayDriverData(displayIndex);
    void *pDXGIDLL;
    char *displayName;
    int nAdapter, nOutput;
    IDXGIFactory *pDXGIFactory = NULL;
    IDXGIAdapter *pDXGIAdapter;
    IDXGIOutput* pDXGIOutput;

    if (!adapterIndex) {
        SDL_InvalidParamError("adapterIndex");
        return SDL_FALSE;
    }

    if (!outputIndex) {
        SDL_InvalidParamError("outputIndex");
        return SDL_FALSE;
    }

    *adapterIndex = -1;
    *outputIndex = -1;

    if (!pData) {
        SDL_SetError("Invalid display index");
        return SDL_FALSE;
    }

    if (!DXGI_LoadDLL(&pDXGIDLL, &pDXGIFactory)) {
        SDL_SetError("Unable to create DXGI interface");
        return SDL_FALSE;
    }

    displayName = WIN_StringToUTF8W(pData->DeviceName);
    nAdapter = 0;
    while (*adapterIndex == -1 && SUCCEEDED(IDXGIFactory_EnumAdapters(pDXGIFactory, nAdapter, &pDXGIAdapter))) {
        nOutput = 0;
        while (*adapterIndex == -1 && SUCCEEDED(IDXGIAdapter_EnumOutputs(pDXGIAdapter, nOutput, &pDXGIOutput))) {
            DXGI_OUTPUT_DESC outputDesc;
            if (SUCCEEDED(IDXGIOutput_GetDesc(pDXGIOutput, &outputDesc))) {
                char *outputName = WIN_StringToUTF8W(outputDesc.DeviceName);
                if (SDL_strcmp(outputName, displayName) == 0) {
                    *adapterIndex = nAdapter;
                    *outputIndex = nOutput;
                }
                SDL_free(outputName);
            }
            IDXGIOutput_Release(pDXGIOutput);
            nOutput++;
        }
        IDXGIAdapter_Release(pDXGIAdapter);
        nAdapter++;
    }
    SDL_free(displayName);

    /* free up the DXGI factory */
    IDXGIFactory_Release(pDXGIFactory);
    SDL_UnloadObject(pDXGIDLL);

    if (*adapterIndex == -1) {
        return SDL_FALSE;
    } else {
        return SDL_TRUE;
    }
#endif
}

#endif /* SDL_VIDEO_DRIVER_WINDOWS */

/* vim: set ts=4 sw=4 expandtab: */
