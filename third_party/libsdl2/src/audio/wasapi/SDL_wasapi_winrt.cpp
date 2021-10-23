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
#include <collection.h>

extern "C" {
#include "../../core/windows/SDL_windows.h"
#include "SDL_audio.h"
#include "SDL_timer.h"
#include "../SDL_audio_c.h"
#include "../SDL_sysaudio.h"
}

#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "SDL_wasapi.h"

using namespace Windows::Devices::Enumeration;
using namespace Windows::Media::Devices;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;

static Platform::String^ SDL_PKEY_AudioEngine_DeviceFormat = L"{f19f064d-082c-4e27-bc73-6882a1bb8e4c} 0";

class SDL_WasapiDeviceEventHandler
{
public:
    SDL_WasapiDeviceEventHandler(const SDL_bool _iscapture);
    ~SDL_WasapiDeviceEventHandler();
    void OnDeviceAdded(DeviceWatcher^ sender, DeviceInformation^ args);
    void OnDeviceRemoved(DeviceWatcher^ sender, DeviceInformationUpdate^ args);
    void OnDeviceUpdated(DeviceWatcher^ sender, DeviceInformationUpdate^ args);
    void OnEnumerationCompleted(DeviceWatcher^ sender, Platform::Object^ args);
    void OnDefaultRenderDeviceChanged(Platform::Object^ sender, DefaultAudioRenderDeviceChangedEventArgs^ args);
    void OnDefaultCaptureDeviceChanged(Platform::Object^ sender, DefaultAudioCaptureDeviceChangedEventArgs^ args);
    SDL_semaphore* completed;

private:
    const SDL_bool iscapture;
    DeviceWatcher^ watcher;
    Windows::Foundation::EventRegistrationToken added_handler;
    Windows::Foundation::EventRegistrationToken removed_handler;
    Windows::Foundation::EventRegistrationToken updated_handler;
    Windows::Foundation::EventRegistrationToken completed_handler;
    Windows::Foundation::EventRegistrationToken default_changed_handler;
};

SDL_WasapiDeviceEventHandler::SDL_WasapiDeviceEventHandler(const SDL_bool _iscapture)
    : iscapture(_iscapture)
    , completed(SDL_CreateSemaphore(0))
{
    if (!completed)
        return;  // uhoh.

    Platform::String^ selector = _iscapture ? MediaDevice::GetAudioCaptureSelector() :
                                              MediaDevice::GetAudioRenderSelector();
    Platform::Collections::Vector<Platform::String^> properties;
    properties.Append(SDL_PKEY_AudioEngine_DeviceFormat);
    watcher = DeviceInformation::CreateWatcher(selector, properties.GetView());
    if (!watcher)
        return;  // uhoh.

    // !!! FIXME: this doesn't need a lambda here, I think, if I make SDL_WasapiDeviceEventHandler a proper C++/CX class. --ryan.
    added_handler = watcher->Added += ref new TypedEventHandler<DeviceWatcher^, DeviceInformation^>([this](DeviceWatcher^ sender, DeviceInformation^ args) { OnDeviceAdded(sender, args); } );
    removed_handler = watcher->Removed += ref new TypedEventHandler<DeviceWatcher^, DeviceInformationUpdate^>([this](DeviceWatcher^ sender, DeviceInformationUpdate^ args) { OnDeviceRemoved(sender, args); } );
    updated_handler = watcher->Updated += ref new TypedEventHandler<DeviceWatcher^, DeviceInformationUpdate^>([this](DeviceWatcher^ sender, DeviceInformationUpdate^ args) { OnDeviceUpdated(sender, args); } );
    completed_handler = watcher->EnumerationCompleted += ref new TypedEventHandler<DeviceWatcher^, Platform::Object^>([this](DeviceWatcher^ sender, Platform::Object^ args) { OnEnumerationCompleted(sender, args); } );
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
        watcher->EnumerationCompleted -= completed_handler;
        watcher->Stop();
        watcher = nullptr;
    }
    if (completed) {
        SDL_DestroySemaphore(completed);
        completed = nullptr;
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
        WAVEFORMATEXTENSIBLE fmt;
        Platform::Object^ obj = info->Properties->Lookup(SDL_PKEY_AudioEngine_DeviceFormat);
        if (obj) {
            IPropertyValue^ property = (IPropertyValue^) obj;
            Platform::Array<unsigned char>^ data;
            property->GetUInt8Array(&data);
            SDL_memcpy(&fmt, data->Data, SDL_min(data->Length, sizeof(WAVEFORMATEXTENSIBLE)));
        } else {
            SDL_zero(fmt);
        }

        WASAPI_AddDevice(this->iscapture, utf8dev, &fmt, info->Id->Data());
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
SDL_WasapiDeviceEventHandler::OnEnumerationCompleted(DeviceWatcher^ sender, Platform::Object^ args)
{
    SDL_assert(sender == this->watcher);
    SDL_SemPost(this->completed);
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
    SDL_SemWait(playback_device_event_handler->completed);
    SDL_SemWait(capture_device_event_handler->completed);
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
    // Just set a flag, since we're probably in a different thread. We'll pick it up and init everything on our own thread to prevent races.
    SDL_AtomicSet(&device->hidden->just_activated, 1);
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

    if (FAILED(ret) || async == nullptr) {
        if (async != nullptr) {
            async->Release();
        }
        handler.Get()->Release();
        WASAPI_UnrefDevice(_this);
        return WIN_SetErrorFromHRESULT("WASAPI can't activate requested audio endpoint", ret);
    }

    /* Spin until the async operation is complete.
     * If we don't PrepDevice before leaving this function, the bug list gets LONG:
     * - device.spec is not filled with the correct information
     * - The 'obtained' spec will be wrong for ALLOW_CHANGE properties
     * - SDL_AudioStreams will/will not be allocated at the right time
     * - SDL_assert(device->callbackspec.size == device->spec.size) will fail
     * - When the assert is ignored, skipping or a buffer overflow will occur
     */
    while (!SDL_AtomicCAS(&_this->hidden->just_activated, 1, 0)) {
        SDL_Delay(1);
    }

    HRESULT activateRes = S_OK;
    IUnknown *iunknown = nullptr;
    const HRESULT getActivateRes = async->GetActivateResult(&activateRes, &iunknown);
    async->Release();
    if (FAILED(getActivateRes)) {
        return WIN_SetErrorFromHRESULT("Failed to get WASAPI activate result", getActivateRes);
    } else if (FAILED(activateRes)) {
        return WIN_SetErrorFromHRESULT("Failed to activate WASAPI device", activateRes);
    }

    iunknown->QueryInterface(IID_PPV_ARGS(&_this->hidden->client));
    if (!_this->hidden->client) {
        return SDL_SetError("Failed to query WASAPI client interface");
    }

    if (WASAPI_PrepDevice(_this, isrecovery) == -1) {
        return -1;
    }

    return 0;
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
