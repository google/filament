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

// This is C++/CX code that the WinRT port uses to talk to WASAPI-related
//  system APIs. The C implementation of these functions, for non-WinRT apps,
//  is in SDL_wasapi_win32.c. The code in SDL_wasapi.c is used by both standard
//  Windows and WinRT builds to deal with audio and calls into these functions.

#if SDL_AUDIO_DRIVER_WASAPI && defined(__WINRT__)

#include <Windows.h>
#include <windows.ui.core.h>
#include <windows.devices.enumeration.h>
#include <windows.media.devices.h>
#include <wrl/implements.h>

extern "C" {
#include "../../core/windows/SDL_windows.h"
#include "SDL_audio.h"
#include "SDL_timer.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"
#include "SDL_assert.h"
#include "SDL_log.h"
}

#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "SDL_wasapi.h"

using namespace Windows::Devices::Enumeration;
using namespace Windows::Media::Devices;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;

class SDL_WasapiDeviceEventHandler
{
public:
    SDL_WasapiDeviceEventHandler(const SDL_bool _iscapture);
    ~SDL_WasapiDeviceEventHandler();
    void OnDeviceAdded(DeviceWatcher^ sender, DeviceInformation^ args);
    void OnDeviceRemoved(DeviceWatcher^ sender, DeviceInformationUpdate^ args);
    void OnDeviceUpdated(DeviceWatcher^ sender, DeviceInformationUpdate^ args);
    void OnDefaultRenderDeviceChanged(Platform::Object^ sender, DefaultAudioRenderDeviceChangedEventArgs^ args);
    void OnDefaultCaptureDeviceChanged(Platform::Object^ sender, DefaultAudioCaptureDeviceChangedEventArgs^ args);

private:
    const SDL_bool iscapture;
    DeviceWatcher^ watcher;
    Windows::Foundation::EventRegistrationToken added_handler;
    Windows::Foundation::EventRegistrationToken removed_handler;
    Windows::Foundation::EventRegistrationToken updated_handler;
    Windows::Foundation::EventRegistrationToken default_changed_handler;
};

SDL_WasapiDeviceEventHandler::SDL_WasapiDeviceEventHandler(const SDL_bool _iscapture)
    : iscapture(_iscapture)
    , watcher(DeviceInformation::CreateWatcher(_iscapture ? DeviceClass::AudioCapture : DeviceClass::AudioRender))
{
    if (!watcher)
        return;  // uhoh.

    // !!! FIXME: this doesn't need a lambda here, I think, if I make SDL_WasapiDeviceEventHandler a proper C++/CX class. --ryan.
    added_handler = watcher->Added += ref new TypedEventHandler<DeviceWatcher^, DeviceInformation^>([this](DeviceWatcher^ sender, DeviceInformation^ args) { OnDeviceAdded(sender, args); } );
    removed_handler = watcher->Removed += ref new TypedEventHandler<DeviceWatcher^, DeviceInformationUpdate^>([this](DeviceWatcher^ sender, DeviceInformationUpdate^ args) { OnDeviceRemoved(sender, args); } );
    updated_handler = watcher->Updated += ref new TypedEventHandler<DeviceWatcher^, DeviceInformationUpdate^>([this](DeviceWatcher^ sender, DeviceInformationUpdate^ args) { OnDeviceUpdated(sender, args); } );
    if (iscapture) {
        default_changed_handler = MediaDevice::DefaultAudioCaptureDeviceChanged += ref new TypedEventHandler<Platform::Object^, DefaultAudioCaptureDeviceChangedEventArgs^>([this](Platform::Object^ sender, DefaultAudioCaptureDeviceChangedEventArgs^ args) { OnDefaultCaptureDeviceChanged(sender, args); } );
    } else {
        default_changed_handler = MediaDevice::DefaultAudioRenderDeviceChanged += ref new TypedEventHandler<Platform::Object^, DefaultAudioRenderDeviceChangedEventArgs^>([this](Platform::Object^ sender, DefaultAudioRenderDeviceChangedEventArgs^ args) { OnDefaultRenderDeviceChanged(sender, args); } );
    }
    watcher->Start();
}

SDL_WasapiDeviceEventHandler::~SDL_WasapiDeviceEventHandler()
{
    if (watcher) {
        watcher->Added -= added_handler;
        watcher->Removed -= removed_handler;
        watcher->Updated -= updated_handler;
        watcher->Stop();
        watcher = nullptr;
    }

    if (iscapture) {
        MediaDevice::DefaultAudioCaptureDeviceChanged -= default_changed_handler;
    } else {
        MediaDevice::DefaultAudioRenderDeviceChanged -= default_changed_handler;
    }
}

void
SDL_WasapiDeviceEventHandler::OnDeviceAdded(DeviceWatcher^ sender, DeviceInformation^ info)
{
    SDL_assert(sender == this->watcher);
    char *utf8dev = WIN_StringToUTF8(info->Name->Data());
    if (utf8dev) {
        WASAPI_AddDevice(this->iscapture, utf8dev, info->Id->Data());
        SDL_free(utf8dev);
    }
}

void
SDL_WasapiDeviceEventHandler::OnDeviceRemoved(DeviceWatcher^ sender, DeviceInformationUpdate^ info)
{
    SDL_assert(sender == this->watcher);
    WASAPI_RemoveDevice(this->iscapture, info->Id->Data());
}

void
SDL_WasapiDeviceEventHandler::OnDeviceUpdated(DeviceWatcher^ sender, DeviceInformationUpdate^ args)
{
    SDL_assert(sender == this->watcher);
}

void
SDL_WasapiDeviceEventHandler::OnDefaultRenderDeviceChanged(Platform::Object^ sender, DefaultAudioRenderDeviceChangedEventArgs^ args)
{
    SDL_assert(this->iscapture);
    SDL_AtomicAdd(&WASAPI_DefaultPlaybackGeneration, 1);
}

void
SDL_WasapiDeviceEventHandler::OnDefaultCaptureDeviceChanged(Platform::Object^ sender, DefaultAudioCaptureDeviceChangedEventArgs^ args)
{
    SDL_assert(!this->iscapture);
    SDL_AtomicAdd(&WASAPI_DefaultCaptureGeneration, 1);
}


static SDL_WasapiDeviceEventHandler *playback_device_event_handler;
static SDL_WasapiDeviceEventHandler *capture_device_event_handler;

int WASAPI_PlatformInit(void)
{
    return 0;
}

void WASAPI_PlatformDeinit(void)
{
    delete playback_device_event_handler;
    playback_device_event_handler = nullptr;
    delete capture_device_event_handler;
    capture_device_event_handler = nullptr;
}

void WASAPI_EnumerateEndpoints(void)
{
    // DeviceWatchers will fire an Added event for each existing device at
    //  startup, so we don't need to enumerate them separately before
    //  listening for updates.
    playback_device_event_handler = new SDL_WasapiDeviceEventHandler(SDL_FALSE);
    capture_device_event_handler = new SDL_WasapiDeviceEventHandler(SDL_TRUE);
}

struct SDL_WasapiActivationHandler : public RuntimeClass< RuntimeClassFlags< ClassicCom >, FtmBase, IActivateAudioInterfaceCompletionHandler >
{
    SDL_WasapiActivationHandler() : device(nullptr) {}
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation *operation);
    SDL_AudioDevice *device;
};

HRESULT
SDL_WasapiActivationHandler::ActivateCompleted(IActivateAudioInterfaceAsyncOperation *async)
{
    HRESULT result = S_OK;
    IUnknown *iunknown = nullptr;
    const HRESULT ret = async->GetActivateResult(&result, &iunknown);

    if (SUCCEEDED(ret) && SUCCEEDED(result)) {
        iunknown->QueryInterface(IID_PPV_ARGS(&device->hidden->client));
        if (device->hidden->client) {
            // Just set a flag, since we're probably in a different thread. We'll pick it up and init everything on our own thread to prevent races.
            SDL_AtomicSet(&device->hidden->just_activated, 1);
        }
    }

    WASAPI_UnrefDevice(device);

    return S_OK;
}

void
WASAPI_PlatformDeleteActivationHandler(void *handler)
{
    ((SDL_WasapiActivationHandler *) handler)->Release();
}

int
WASAPI_ActivateDevice(_THIS, const SDL_bool isrecovery)
{
    LPCWSTR devid = _this->hidden->devid;
    Platform::String^ defdevid;

    if (devid == nullptr) {
        defdevid = _this->iscapture ? MediaDevice::GetDefaultAudioCaptureId(AudioDeviceRole::Default) : MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);
        if (defdevid) {
            devid = defdevid->Data();
        }
    }

    SDL_AtomicSet(&_this->hidden->just_activated, 0);

    ComPtr<SDL_WasapiActivationHandler> handler = Make<SDL_WasapiActivationHandler>();
    if (handler == nullptr) {
        return SDL_SetError("Failed to allocate WASAPI activation handler");
    }

    handler.Get()->AddRef();  // we hold a reference after ComPtr destructs on return, causing a Release, and Release ourselves in WASAPI_PlatformDeleteActivationHandler(), etc.
    handler.Get()->device = _this;
    _this->hidden->activation_handler = handler.Get();

    WASAPI_RefDevice(_this);  /* completion handler will unref it. */
    IActivateAudioInterfaceAsyncOperation *async = nullptr;
    const HRESULT ret = ActivateAudioInterfaceAsync(devid, __uuidof(IAudioClient), nullptr, handler.Get(), &async);

    if (async != nullptr) {
        async->Release();
    }

    if (FAILED(ret)) {
        handler.Get()->Release();
        WASAPI_UnrefDevice(_this);
        return WIN_SetErrorFromHRESULT("WASAPI can't activate requested audio endpoint", ret);
    }

    return 0;
}

void
WASAPI_BeginLoopIteration(_THIS)
{
    if (SDL_AtomicCAS(&_this->hidden->just_activated, 1, 0)) {
        if (WASAPI_PrepDevice(_this, SDL_TRUE) == -1) {
            SDL_OpenedAudioDeviceDisconnected(_this);
        } 
    }
}

void
WASAPI_PlatformThreadInit(_THIS)
{
    // !!! FIXME: set this thread to "Pro Audio" priority.
}

void
WASAPI_PlatformThreadDeinit(_THIS)
{
    // !!! FIXME: set this thread to "Pro Audio" priority.
}

#endif  // SDL_AUDIO_DRIVER_WASAPI && defined(__WINRT__)

/* vi: set ts=4 sw=4 expandtab: */
