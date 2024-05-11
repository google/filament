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

/* This is code that Windows uses to talk to WASAPI-related system APIs.
   This is for non-WinRT desktop apps. The C++/CX implementation of these
   functions, exclusive to WinRT, are in SDL_wasapi_winrt.cpp.
   The code in SDL_wasapi.c is used by both standard Windows and WinRT builds
   to deal with audio and calls into these functions. */

#if SDL_AUDIO_DRIVER_WASAPI && !defined(__WINRT__)

#include "../../core/windows/SDL_windows.h"
#include "SDL_audio.h"
#include "SDL_timer.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"
#include "SDL_assert.h"
#include "SDL_log.h"

#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "SDL_wasapi.h"

static const ERole SDL_WASAPI_role = eConsole;  /* !!! FIXME: should this be eMultimedia? Should be a hint? */

/* This is global to the WASAPI target, to handle hotplug and default device lookup. */
static IMMDeviceEnumerator *enumerator = NULL;

/* PropVariantInit() is an inline function/macro in PropIdl.h that calls the C runtime's memset() directly. Use ours instead, to avoid dependency. */
#ifdef PropVariantInit
#undef PropVariantInit
#endif
#define PropVariantInit(p) SDL_zerop(p)

/* handle to Avrt.dll--Vista and later!--for flagging the callback thread as "Pro Audio" (low latency). */
static HMODULE libavrt = NULL;
typedef HANDLE(WINAPI *pfnAvSetMmThreadCharacteristicsW)(LPWSTR, LPDWORD);
typedef BOOL(WINAPI *pfnAvRevertMmThreadCharacteristics)(HANDLE);
static pfnAvSetMmThreadCharacteristicsW pAvSetMmThreadCharacteristicsW = NULL;
static pfnAvRevertMmThreadCharacteristics pAvRevertMmThreadCharacteristics = NULL;

/* Some GUIDs we need to know without linking to libraries that aren't available before Vista. */
static const CLSID SDL_CLSID_MMDeviceEnumerator = { 0xbcde0395, 0xe52f, 0x467c,{ 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e } };
static const IID SDL_IID_IMMDeviceEnumerator = { 0xa95664d2, 0x9614, 0x4f35,{ 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6 } };
static const IID SDL_IID_IMMNotificationClient = { 0x7991eec9, 0x7e89, 0x4d85,{ 0x83, 0x90, 0x6c, 0x70, 0x3c, 0xec, 0x60, 0xc0 } };
static const IID SDL_IID_IMMEndpoint = { 0x1be09788, 0x6894, 0x4089,{ 0x85, 0x86, 0x9a, 0x2a, 0x6c, 0x26, 0x5a, 0xc5 } };
static const IID SDL_IID_IAudioClient = { 0x1cb9ad4c, 0xdbfa, 0x4c32,{ 0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2 } };
static const PROPERTYKEY SDL_PKEY_Device_FriendlyName = { { 0xa45c254e, 0xdf1c, 0x4efd,{ 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, } }, 14 };


static char *
GetWasapiDeviceName(IMMDevice *device)
{
    /* PKEY_Device_FriendlyName gives you "Speakers (SoundBlaster Pro)" which drives me nuts. I'd rather it be
       "SoundBlaster Pro (Speakers)" but I guess that's developers vs users. Windows uses the FriendlyName in
       its own UIs, like Volume Control, etc. */
    char *utf8dev = NULL;
    IPropertyStore *props = NULL;
    if (SUCCEEDED(IMMDevice_OpenPropertyStore(device, STGM_READ, &props))) {
        PROPVARIANT var;
        PropVariantInit(&var);
        if (SUCCEEDED(IPropertyStore_GetValue(props, &SDL_PKEY_Device_FriendlyName, &var))) {
            utf8dev = WIN_StringToUTF8(var.pwszVal);
        }
        PropVariantClear(&var);
        IPropertyStore_Release(props);
    }
    return utf8dev;
}


/* We need a COM subclass of IMMNotificationClient for hotplug support, which is
   easy in C++, but we have to tapdance more to make work in C.
   Thanks to this page for coaching on how to make this work:
     https://www.codeproject.com/Articles/13601/COM-in-plain-C */

typedef struct SDLMMNotificationClient
{
    const IMMNotificationClientVtbl *lpVtbl;
    SDL_atomic_t refcount;
} SDLMMNotificationClient;

static HRESULT STDMETHODCALLTYPE
SDLMMNotificationClient_QueryInterface(IMMNotificationClient *this, REFIID iid, void **ppv)
{
    if ((WIN_IsEqualIID(iid, &IID_IUnknown)) || (WIN_IsEqualIID(iid, &SDL_IID_IMMNotificationClient)))
    {
        *ppv = this;
        this->lpVtbl->AddRef(this);
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE
SDLMMNotificationClient_AddRef(IMMNotificationClient *ithis)
{
    SDLMMNotificationClient *this = (SDLMMNotificationClient *) ithis;
    return (ULONG) (SDL_AtomicIncRef(&this->refcount) + 1);
}

static ULONG STDMETHODCALLTYPE
SDLMMNotificationClient_Release(IMMNotificationClient *ithis)
{
    /* this is a static object; we don't ever free it. */
    SDLMMNotificationClient *this = (SDLMMNotificationClient *) ithis;
    const ULONG retval = SDL_AtomicDecRef(&this->refcount);
    if (retval == 0) {
        SDL_AtomicSet(&this->refcount, 0);  /* uhh... */
        return 0;
    }
    return retval - 1;
}

/* These are the entry points called when WASAPI device endpoints change. */
static HRESULT STDMETHODCALLTYPE
SDLMMNotificationClient_OnDefaultDeviceChanged(IMMNotificationClient *ithis, EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    if (role != SDL_WASAPI_role) {
        return S_OK;  /* ignore it. */
    }

    /* Increment the "generation," so opened devices will pick this up in their threads. */
    switch (flow) {
        case eRender:
            SDL_AtomicAdd(&WASAPI_DefaultPlaybackGeneration, 1);
            break;

        case eCapture:
            SDL_AtomicAdd(&WASAPI_DefaultCaptureGeneration, 1);
            break;

        case eAll:
            SDL_AtomicAdd(&WASAPI_DefaultPlaybackGeneration, 1);
            SDL_AtomicAdd(&WASAPI_DefaultCaptureGeneration, 1);
            break;

        default:
            SDL_assert(!"uhoh, unexpected OnDefaultDeviceChange flow!");
            break;
    }

    return S_OK;
}

static HRESULT STDMETHODCALLTYPE
SDLMMNotificationClient_OnDeviceAdded(IMMNotificationClient *ithis, LPCWSTR pwstrDeviceId)
{
    /* we ignore this; devices added here then progress to ACTIVE, if appropriate, in 
       OnDeviceStateChange, making that a better place to deal with device adds. More 
       importantly: the first time you plug in a USB audio device, this callback will 
       fire, but when you unplug it, it isn't removed (it's state changes to NOTPRESENT).
       Plugging it back in won't fire this callback again. */
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE
SDLMMNotificationClient_OnDeviceRemoved(IMMNotificationClient *ithis, LPCWSTR pwstrDeviceId)
{
    /* See notes in OnDeviceAdded handler about why we ignore this. */
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE
SDLMMNotificationClient_OnDeviceStateChanged(IMMNotificationClient *ithis, LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    IMMDevice *device = NULL;

    if (SUCCEEDED(IMMDeviceEnumerator_GetDevice(enumerator, pwstrDeviceId, &device))) {
        IMMEndpoint *endpoint = NULL;
        if (SUCCEEDED(IMMDevice_QueryInterface(device, &SDL_IID_IMMEndpoint, (void **) &endpoint))) {
            EDataFlow flow;
            if (SUCCEEDED(IMMEndpoint_GetDataFlow(endpoint, &flow))) {
                const SDL_bool iscapture = (flow == eCapture);
                if (dwNewState == DEVICE_STATE_ACTIVE) {
                    char *utf8dev = GetWasapiDeviceName(device);
                    if (utf8dev) {
                        WASAPI_AddDevice(iscapture, utf8dev, pwstrDeviceId);
                        SDL_free(utf8dev);
                    }
                } else {
                    WASAPI_RemoveDevice(iscapture, pwstrDeviceId);
                }
            }
            IMMEndpoint_Release(endpoint);
        }
        IMMDevice_Release(device);
    }

    return S_OK;
}

static HRESULT STDMETHODCALLTYPE
SDLMMNotificationClient_OnPropertyValueChanged(IMMNotificationClient *this, LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    return S_OK;  /* we don't care about these. */
}

static const IMMNotificationClientVtbl notification_client_vtbl = {
    SDLMMNotificationClient_QueryInterface,
    SDLMMNotificationClient_AddRef,
    SDLMMNotificationClient_Release,
    SDLMMNotificationClient_OnDeviceStateChanged,
    SDLMMNotificationClient_OnDeviceAdded,
    SDLMMNotificationClient_OnDeviceRemoved,
    SDLMMNotificationClient_OnDefaultDeviceChanged,
    SDLMMNotificationClient_OnPropertyValueChanged
};

static SDLMMNotificationClient notification_client = { &notification_client_vtbl, { 1 } };


int
WASAPI_PlatformInit(void)
{
    HRESULT ret;

    /* just skip the discussion with COM here. */
    if (!WIN_IsWindowsVistaOrGreater()) {
        return SDL_SetError("WASAPI support requires Windows Vista or later");
    }

    if (FAILED(WIN_CoInitialize())) {
        return SDL_SetError("WASAPI: CoInitialize() failed");
    }

    ret = CoCreateInstance(&SDL_CLSID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &SDL_IID_IMMDeviceEnumerator, (LPVOID) &enumerator);
    if (FAILED(ret)) {
        WIN_CoUninitialize();
        return WIN_SetErrorFromHRESULT("WASAPI CoCreateInstance(MMDeviceEnumerator)", ret);
    }

    libavrt = LoadLibraryW(L"avrt.dll");  /* this library is available in Vista and later. No WinXP, so have to LoadLibrary to use it for now! */
    if (libavrt) {
        pAvSetMmThreadCharacteristicsW = (pfnAvSetMmThreadCharacteristicsW) GetProcAddress(libavrt, "AvSetMmThreadCharacteristicsW");
        pAvRevertMmThreadCharacteristics = (pfnAvRevertMmThreadCharacteristics) GetProcAddress(libavrt, "AvRevertMmThreadCharacteristics");
    }

    return 0;
}

void
WASAPI_PlatformDeinit(void)
{
    if (enumerator) {
        IMMDeviceEnumerator_UnregisterEndpointNotificationCallback(enumerator, (IMMNotificationClient *) &notification_client);
        IMMDeviceEnumerator_Release(enumerator);
        enumerator = NULL;
    }

    if (libavrt) {
        FreeLibrary(libavrt);
        libavrt = NULL;
    }

    pAvSetMmThreadCharacteristicsW = NULL;
    pAvRevertMmThreadCharacteristics = NULL;

    WIN_CoUninitialize();
}

void
WASAPI_PlatformThreadInit(_THIS)
{
    /* this thread uses COM. */
    if (SUCCEEDED(WIN_CoInitialize())) {    /* can't report errors, hope it worked! */
        this->hidden->coinitialized = SDL_TRUE;
    }

    /* Set this thread to very high "Pro Audio" priority. */
    if (pAvSetMmThreadCharacteristicsW) {
        DWORD idx = 0;
        this->hidden->task = pAvSetMmThreadCharacteristicsW(TEXT("Pro Audio"), &idx);
    }
}

void
WASAPI_PlatformThreadDeinit(_THIS)
{
    /* Set this thread back to normal priority. */
    if (this->hidden->task && pAvRevertMmThreadCharacteristics) {
        pAvRevertMmThreadCharacteristics(this->hidden->task);
        this->hidden->task = NULL;
    }

    if (this->hidden->coinitialized) {
        WIN_CoUninitialize();
        this->hidden->coinitialized = SDL_FALSE;
    }
}

int
WASAPI_ActivateDevice(_THIS, const SDL_bool isrecovery)
{
    LPCWSTR devid = this->hidden->devid;
    IMMDevice *device = NULL;
    HRESULT ret;

    if (devid == NULL) {
        const EDataFlow dataflow = this->iscapture ? eCapture : eRender;
        ret = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, dataflow, SDL_WASAPI_role, &device);
    } else {
        ret = IMMDeviceEnumerator_GetDevice(enumerator, devid, &device);
    }

    if (FAILED(ret)) {
        SDL_assert(device == NULL);
        this->hidden->client = NULL;
        return WIN_SetErrorFromHRESULT("WASAPI can't find requested audio endpoint", ret);
    }

    /* this is not async in standard win32, yay! */
    ret = IMMDevice_Activate(device, &SDL_IID_IAudioClient, CLSCTX_ALL, NULL, (void **) &this->hidden->client);
    IMMDevice_Release(device);

    if (FAILED(ret)) {
        SDL_assert(this->hidden->client == NULL);
        return WIN_SetErrorFromHRESULT("WASAPI can't activate audio endpoint", ret);
    }

    SDL_assert(this->hidden->client != NULL);
    if (WASAPI_PrepDevice(this, isrecovery) == -1) {   /* not async, fire it right away. */
        return -1;
    }

    return 0;  /* good to go. */
}


static void
WASAPI_EnumerateEndpointsForFlow(const SDL_bool iscapture)
{
    IMMDeviceCollection *collection = NULL;
    UINT i, total;

    /* Note that WASAPI separates "adapter devices" from "audio endpoint devices"
       ...one adapter device ("SoundBlaster Pro") might have multiple endpoint devices ("Speakers", "Line-Out"). */

    if (FAILED(IMMDeviceEnumerator_EnumAudioEndpoints(enumerator, iscapture ? eCapture : eRender, DEVICE_STATE_ACTIVE, &collection))) {
        return;
    }

    if (FAILED(IMMDeviceCollection_GetCount(collection, &total))) {
        IMMDeviceCollection_Release(collection);
        return;
    }

    for (i = 0; i < total; i++) {
        IMMDevice *device = NULL;
        if (SUCCEEDED(IMMDeviceCollection_Item(collection, i, &device))) {
            LPWSTR devid = NULL;
            if (SUCCEEDED(IMMDevice_GetId(device, &devid))) {
                char *devname = GetWasapiDeviceName(device);
                if (devname) {
                    WASAPI_AddDevice(iscapture, devname, devid);
                    SDL_free(devname);
                }
                CoTaskMemFree(devid);
            }
            IMMDevice_Release(device);
        }
    }

    IMMDeviceCollection_Release(collection);
}

void
WASAPI_EnumerateEndpoints(void)
{
    WASAPI_EnumerateEndpointsForFlow(SDL_FALSE);  /* playback */
    WASAPI_EnumerateEndpointsForFlow(SDL_TRUE);  /* capture */

    /* if this fails, we just won't get hotplug events. Carry on anyhow. */
    IMMDeviceEnumerator_RegisterEndpointNotificationCallback(enumerator, (IMMNotificationClient *) &notification_client);
}

void
WASAPI_PlatformDeleteActivationHandler(void *handler)
{
    /* not asynchronous. */
    SDL_assert(!"This function should have only been called on WinRT.");
}

void
WASAPI_BeginLoopIteration(_THIS)
{
    /* no-op. */
}

#endif  /* SDL_AUDIO_DRIVER_WASAPI && !defined(__WINRT__) */

/* vi: set ts=4 sw=4 expandtab: */

