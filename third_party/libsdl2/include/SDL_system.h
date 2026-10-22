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

/**
 *  \file SDL_system.h
 *
 *  Include file for platform specific SDL API functions
 */

#ifndef SDL_system_h_
#define SDL_system_h_

#include "SDL_stdinc.h"
#include "SDL_keyboard.h"
#include "SDL_render.h"
#include "SDL_video.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif


/* Platform specific functions for Windows */
#ifdef __WIN32__
	
/**
   \brief Set a function that is called for every windows message, before TranslateMessage()
*/
typedef void (SDLCALL * SDL_WindowsMessageHook)(void *userdata, void *hWnd, unsigned int message, Uint64 wParam, Sint64 lParam);
extern DECLSPEC void SDLCALL SDL_SetWindowsMessageHook(SDL_WindowsMessageHook callback, void *userdata);

/**
   \brief Returns the D3D9 adapter index that matches the specified display index.

   This adapter index can be passed to IDirect3D9::CreateDevice and controls
   on which monitor a full screen application will appear.
*/
extern DECLSPEC int SDLCALL SDL_Direct3D9GetAdapterIndex( int displayIndex );

typedef struct IDirect3DDevice9 IDirect3DDevice9;
/**
   \brief Returns the D3D device associated with a renderer, or NULL if it's not a D3D renderer.

   Once you are done using the device, you should release it to avoid a resource leak.
 */
extern DECLSPEC IDirect3DDevice9* SDLCALL SDL_RenderGetD3D9Device(SDL_Renderer * renderer);

/**
   \brief Returns the DXGI Adapter and Output indices for the specified display index.

   These can be passed to EnumAdapters and EnumOutputs respectively to get the objects
   required to create a DX10 or DX11 device and swap chain.
 */
extern DECLSPEC SDL_bool SDLCALL SDL_DXGIGetOutputInfo( int displayIndex, int *adapterIndex, int *outputIndex );

#endif /* __WIN32__ */


/* Platform specific functions for iOS */
#if defined(__IPHONEOS__) && __IPHONEOS__

#define SDL_iOSSetAnimationCallback(window, interval, callback, callbackParam) SDL_iPhoneSetAnimationCallback(window, interval, callback, callbackParam)
extern DECLSPEC int SDLCALL SDL_iPhoneSetAnimationCallback(SDL_Window * window, int interval, void (*callback)(void*), void *callbackParam);

#define SDL_iOSSetEventPump(enabled) SDL_iPhoneSetEventPump(enabled)
extern DECLSPEC void SDLCALL SDL_iPhoneSetEventPump(SDL_bool enabled);

#endif /* __IPHONEOS__ */


/* Platform specific functions for Android */
#if defined(__ANDROID__) && __ANDROID__

/**
   \brief Get the JNI environment for the current thread

   This returns JNIEnv*, but the prototype is void* so we don't need jni.h
 */
extern DECLSPEC void * SDLCALL SDL_AndroidGetJNIEnv(void);

/**
   \brief Get the SDL Activity object for the application

   This returns jobject, but the prototype is void* so we don't need jni.h
   The jobject returned by SDL_AndroidGetActivity is a local reference.
   It is the caller's responsibility to properly release it
   (using env->Push/PopLocalFrame or manually with env->DeleteLocalRef)
 */
extern DECLSPEC void * SDLCALL SDL_AndroidGetActivity(void);

/**
   \brief Return true if the application is running on Android TV
 */
extern DECLSPEC SDL_bool SDLCALL SDL_IsAndroidTV(void);

/**
   See the official Android developer guide for more information:
   http://developer.android.com/guide/topics/data/data-storage.html
*/
#define SDL_ANDROID_EXTERNAL_STORAGE_READ   0x01
#define SDL_ANDROID_EXTERNAL_STORAGE_WRITE  0x02

/**
   \brief Get the path used for internal storage for this application.

   This path is unique to your application and cannot be written to
   by other applications.
 */
extern DECLSPEC const char * SDLCALL SDL_AndroidGetInternalStoragePath(void);

/**
   \brief Get the current state of external storage, a bitmask of these values:
    SDL_ANDROID_EXTERNAL_STORAGE_READ
    SDL_ANDROID_EXTERNAL_STORAGE_WRITE

   If external storage is currently unavailable, this will return 0.
*/
extern DECLSPEC int SDLCALL SDL_AndroidGetExternalStorageState(void);

/**
   \brief Get the path used for external storage for this application.

   This path is unique to your application, but is public and can be
   written to by other applications.
 */
extern DECLSPEC const char * SDLCALL SDL_AndroidGetExternalStoragePath(void);

#endif /* __ANDROID__ */

/* Platform specific functions for WinRT */
#if defined(__WINRT__) && __WINRT__

/**
 *  \brief WinRT / Windows Phone path types
 */
typedef enum
{
    /** \brief The installed app's root directory.
        Files here are likely to be read-only. */
    SDL_WINRT_PATH_INSTALLED_LOCATION,

    /** \brief The app's local data store.  Files may be written here */
    SDL_WINRT_PATH_LOCAL_FOLDER,

    /** \brief The app's roaming data store.  Unsupported on Windows Phone.
        Files written here may be copied to other machines via a network
        connection.
    */
    SDL_WINRT_PATH_ROAMING_FOLDER,

    /** \brief The app's temporary data store.  Unsupported on Windows Phone.
        Files written here may be deleted at any time. */
    SDL_WINRT_PATH_TEMP_FOLDER
} SDL_WinRT_Path;


/**
 *  \brief WinRT Device Family
 */
typedef enum
{
    /** \brief Unknown family  */
    SDL_WINRT_DEVICEFAMILY_UNKNOWN,

    /** \brief Desktop family*/
    SDL_WINRT_DEVICEFAMILY_DESKTOP,

    /** \brief Mobile family (for example smartphone) */
    SDL_WINRT_DEVICEFAMILY_MOBILE,

    /** \brief XBox family */
    SDL_WINRT_DEVICEFAMILY_XBOX,
} SDL_WinRT_DeviceFamily;


/**
 *  \brief Retrieves a WinRT defined path on the local file system
 *
 *  \note Documentation on most app-specific path types on WinRT
 *      can be found on MSDN, at the URL:
 *      http://msdn.microsoft.com/en-us/library/windows/apps/hh464917.aspx
 *
 *  \param pathType The type of path to retrieve.
 *  \return A UCS-2 string (16-bit, wide-char) containing the path, or NULL
 *      if the path is not available for any reason.  Not all paths are
 *      available on all versions of Windows.  This is especially true on
 *      Windows Phone.  Check the documentation for the given
 *      SDL_WinRT_Path for more information on which path types are
 *      supported where.
 */
extern DECLSPEC const wchar_t * SDLCALL SDL_WinRTGetFSPathUNICODE(SDL_WinRT_Path pathType);

/**
 *  \brief Retrieves a WinRT defined path on the local file system
 *
 *  \note Documentation on most app-specific path types on WinRT
 *      can be found on MSDN, at the URL:
 *      http://msdn.microsoft.com/en-us/library/windows/apps/hh464917.aspx
 *
 *  \param pathType The type of path to retrieve.
 *  \return A UTF-8 string (8-bit, multi-byte) containing the path, or NULL
 *      if the path is not available for any reason.  Not all paths are
 *      available on all versions of Windows.  This is especially true on
 *      Windows Phone.  Check the documentation for the given
 *      SDL_WinRT_Path for more information on which path types are
 *      supported where.
 */
extern DECLSPEC const char * SDLCALL SDL_WinRTGetFSPathUTF8(SDL_WinRT_Path pathType);

/**
 *  \brief Detects the device family of WinRT plattform on runtime
 *
 *  \return Device family
 */
extern DECLSPEC SDL_WinRT_DeviceFamily SDLCALL SDL_WinRTGetDeviceFamily();

#endif /* __WINRT__ */

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* SDL_system_h_ */

/* vi: set ts=4 sw=4 expandtab: */
