/*
 *  Simple DirectMedia Layer
 *  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>
 * 
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 * 
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 * 
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 */
#include "../SDL_internal.h"

#if SDL_VIDEO_OPENGL_EGL

#if SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT
#include "../core/windows/SDL_windows.h"
#endif
#if SDL_VIDEO_DRIVER_ANDROID
#include <android/native_window.h>
#include "../core/android/SDL_android.h"
#include "../video/android/SDL_androidvideo.h"
#endif

#include "SDL_sysvideo.h"
#include "SDL_egl_c.h"
#include "SDL_loadso.h"
#include "SDL_hints.h"

#ifdef EGL_KHR_create_context
/* EGL_OPENGL_ES3_BIT_KHR was added in version 13 of the extension. */
#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR 0x00000040
#endif
#endif /* EGL_KHR_create_context */

#if SDL_VIDEO_DRIVER_RPI
/* Raspbian places the OpenGL ES/EGL binaries in a non standard path */
#define DEFAULT_EGL ( vc4 ? "libEGL.so.1" : "libbrcmEGL.so" )
#define DEFAULT_OGL_ES2 ( vc4 ? "libGLESv2.so.2" : "libbrcmGLESv2.so" )
#define ALT_EGL "libEGL.so"
#define ALT_OGL_ES2 "libGLESv2.so"
#define DEFAULT_OGL_ES_PVR ( vc4 ? "libGLES_CM.so.1" : "libbrcmGLESv2.so" )
#define DEFAULT_OGL_ES ( vc4 ? "libGLESv1_CM.so.1" : "libbrcmGLESv2.so" )

#elif SDL_VIDEO_DRIVER_ANDROID || SDL_VIDEO_DRIVER_VIVANTE
/* Android */
#define DEFAULT_EGL "libEGL.so"
#define DEFAULT_OGL_ES2 "libGLESv2.so"
#define DEFAULT_OGL_ES_PVR "libGLES_CM.so"
#define DEFAULT_OGL_ES "libGLESv1_CM.so"

#elif SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT
/* EGL AND OpenGL ES support via ANGLE */
#define DEFAULT_EGL "libEGL.dll"
#define DEFAULT_OGL_ES2 "libGLESv2.dll"
#define DEFAULT_OGL_ES_PVR "libGLES_CM.dll"
#define DEFAULT_OGL_ES "libGLESv1_CM.dll"

#elif SDL_VIDEO_DRIVER_COCOA
/* EGL AND OpenGL ES support via ANGLE */
#define DEFAULT_EGL "libEGL.dylib"
#define DEFAULT_OGL_ES2 "libGLESv2.dylib"
#define DEFAULT_OGL_ES_PVR "libGLES_CM.dylib"   //???
#define DEFAULT_OGL_ES "libGLESv1_CM.dylib"     //???

#elif defined(__OpenBSD__)
/* OpenBSD */
#define DEFAULT_OGL "libGL.so"
#define DEFAULT_EGL "libEGL.so"
#define DEFAULT_OGL_ES2 "libGLESv2.so"
#define DEFAULT_OGL_ES_PVR "libGLES_CM.so"
#define DEFAULT_OGL_ES "libGLESv1_CM.so"

#else
/* Desktop Linux/Unix-like */
#define DEFAULT_OGL "libGL.so.1"
#define DEFAULT_EGL "libEGL.so.1"
#define ALT_OGL "libOpenGL.so.0"
#define DEFAULT_OGL_ES2 "libGLESv2.so.2"
#define DEFAULT_OGL_ES_PVR "libGLES_CM.so.1"
#define DEFAULT_OGL_ES "libGLESv1_CM.so.1"
#endif /* SDL_VIDEO_DRIVER_RPI */

#if SDL_VIDEO_OPENGL
#include "SDL_opengl.h"
#endif

/** If we happen to not have this defined because of an older EGL version, just define it 0x0
    as eglGetPlatformDisplayEXT will most likely be NULL if this is missing
*/
#ifndef EGL_PLATFORM_DEVICE_EXT
#define EGL_PLATFORM_DEVICE_EXT 0x0
#endif

#ifdef SDL_VIDEO_STATIC_ANGLE
#define LOAD_FUNC(NAME) \
_this->egl_data->NAME = (void *)NAME;
#else
#define LOAD_FUNC(NAME) \
_this->egl_data->NAME = SDL_LoadFunction(_this->egl_data->dll_handle, #NAME); \
if (!_this->egl_data->NAME) \
{ \
    return SDL_SetError("Could not retrieve EGL function " #NAME); \
}
#endif

/* it is allowed to not have some of the EGL extensions on start - attempts to use them will fail later. */
#define LOAD_FUNC_EGLEXT(NAME) \
    _this->egl_data->NAME = _this->egl_data->eglGetProcAddress(#NAME);


static const char * SDL_EGL_GetErrorName(EGLint eglErrorCode)
{
#define SDL_EGL_ERROR_TRANSLATE(e) case e: return #e;
    switch (eglErrorCode) {
        SDL_EGL_ERROR_TRANSLATE(EGL_SUCCESS);
        SDL_EGL_ERROR_TRANSLATE(EGL_NOT_INITIALIZED);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_ACCESS);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_ALLOC);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_ATTRIBUTE);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_CONTEXT);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_CONFIG);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_CURRENT_SURFACE);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_DISPLAY);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_SURFACE);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_MATCH);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_PARAMETER);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_NATIVE_PIXMAP);
        SDL_EGL_ERROR_TRANSLATE(EGL_BAD_NATIVE_WINDOW);
        SDL_EGL_ERROR_TRANSLATE(EGL_CONTEXT_LOST);
    }
    return "";
}

int SDL_EGL_SetErrorEx(const char * message, const char * eglFunctionName, EGLint eglErrorCode)
{
    const char * errorText = SDL_EGL_GetErrorName(eglErrorCode);
    char altErrorText[32];
    if (errorText[0] == '\0') {
        /* An unknown-to-SDL error code was reported.  Report its hexadecimal value, instead of its name. */
        SDL_snprintf(altErrorText, SDL_arraysize(altErrorText), "0x%x", (unsigned int)eglErrorCode);
        errorText = altErrorText;
    }
    return SDL_SetError("%s (call to %s failed, reporting an error of %s)", message, eglFunctionName, errorText);
}

/* EGL implementation of SDL OpenGL ES support */

SDL_bool SDL_EGL_HasExtension(_THIS, SDL_EGL_ExtensionType type, const char *ext)
{
    size_t ext_len;
    const char *ext_override;
    const char *egl_extstr;
    const char *ext_start;

    /* Invalid extensions can be rejected early */
    if (ext == NULL || *ext == 0 || SDL_strchr(ext, ' ') != NULL) {
        /* SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "SDL_EGL_HasExtension: Invalid EGL extension"); */
        return SDL_FALSE;
    }

    /* Extensions can be masked with an environment variable.
     * Unlike the OpenGL override, this will use the set bits of an integer
     * to disable the extension.
     *  Bit   Action
     *  0     If set, the display extension is masked and not present to SDL.
     *  1     If set, the client extension is masked and not present to SDL.
     */
    ext_override = SDL_getenv(ext);
    if (ext_override != NULL) {
        int disable_ext = SDL_atoi(ext_override);
        if (disable_ext & 0x01 && type == SDL_EGL_DISPLAY_EXTENSION) {
            return SDL_FALSE;
        } else if (disable_ext & 0x02 && type == SDL_EGL_CLIENT_EXTENSION) {
            return SDL_FALSE;
        }
    }

    ext_len = SDL_strlen(ext);
    switch (type) {
    case SDL_EGL_DISPLAY_EXTENSION:
        egl_extstr = _this->egl_data->eglQueryString(_this->egl_data->egl_display, EGL_EXTENSIONS);
        break;
    case SDL_EGL_CLIENT_EXTENSION:
        /* EGL_EXT_client_extensions modifies eglQueryString to return client extensions
         * if EGL_NO_DISPLAY is passed. Implementations without it are required to return NULL.
         * This behavior is included in EGL 1.5.
         */
        egl_extstr = _this->egl_data->eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
        break;
    default:
        /* SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "SDL_EGL_HasExtension: Invalid extension type"); */
        return SDL_FALSE;
    }

    if (egl_extstr != NULL) {
        ext_start = egl_extstr;

        while (*ext_start) {
            ext_start = SDL_strstr(ext_start, ext);
            if (ext_start == NULL) {
                return SDL_FALSE;
            }
            /* Check if the match is not just a substring of one of the extensions */
            if (ext_start == egl_extstr || *(ext_start - 1) == ' ') {
                if (ext_start[ext_len] == ' ' || ext_start[ext_len] == 0) {
                    return SDL_TRUE;
                }
            }
            /* If the search stopped in the middle of an extension, skip to the end of it */
            ext_start += ext_len;
            while (*ext_start != ' ' && *ext_start != 0) {
                ext_start++;
            }
        }
    }

    return SDL_FALSE;
}

void *
SDL_EGL_GetProcAddress(_THIS, const char *proc)
{
    const Uint32 eglver = (((Uint32) _this->egl_data->egl_version_major) << 16) | ((Uint32) _this->egl_data->egl_version_minor);
    const SDL_bool is_egl_15_or_later = eglver >= ((((Uint32) 1) << 16) | 5);
    void *retval = NULL;

    /* EGL 1.5 can use eglGetProcAddress() for any symbol. 1.4 and earlier can't use it for core entry points. */
    if (!retval && is_egl_15_or_later && _this->egl_data->eglGetProcAddress) {
        retval = _this->egl_data->eglGetProcAddress(proc);
    }

    #ifndef __EMSCRIPTEN__  /* LoadFunction isn't needed on Emscripten and will call dlsym(), causing other problems. */
    /* Try SDL_LoadFunction() first for EGL <= 1.4, or as a fallback for >= 1.5. */
    if (!retval) {
        static char procname[64];
        retval = SDL_LoadFunction(_this->egl_data->egl_dll_handle, proc);
        /* just in case you need an underscore prepended... */
        if (!retval && (SDL_strlen(proc) < (sizeof (procname) - 1))) {
            procname[0] = '_';
            SDL_strlcpy(procname + 1, proc, sizeof (procname) - 1);
            retval = SDL_LoadFunction(_this->egl_data->egl_dll_handle, procname);
        }
    }
    #endif

    /* Try eglGetProcAddress if we're on <= 1.4 and still searching... */
    if (!retval && !is_egl_15_or_later && _this->egl_data->eglGetProcAddress) {
        retval = _this->egl_data->eglGetProcAddress(proc);
        if (retval) {
            return retval;
        }
    }

    return retval;
}

void
SDL_EGL_UnloadLibrary(_THIS)
{
    if (_this->egl_data) {
        if (_this->egl_data->egl_display) {
            _this->egl_data->eglTerminate(_this->egl_data->egl_display);
            _this->egl_data->egl_display = NULL;
        }

        if (_this->egl_data->dll_handle) {
            SDL_UnloadObject(_this->egl_data->dll_handle);
            _this->egl_data->dll_handle = NULL;
        }
        if (_this->egl_data->egl_dll_handle) {
            SDL_UnloadObject(_this->egl_data->egl_dll_handle);
            _this->egl_data->egl_dll_handle = NULL;
        }
        
        SDL_free(_this->egl_data);
        _this->egl_data = NULL;
    }
}

int
SDL_EGL_LoadLibraryOnly(_THIS, const char *egl_path)
{
    void *dll_handle = NULL, *egl_dll_handle = NULL; /* The naming is counter intuitive, but hey, I just work here -- Gabriel */
    const char *path = NULL;
#if SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT
    const char *d3dcompiler;
#endif
#if SDL_VIDEO_DRIVER_RPI
    SDL_bool vc4 = (0 == access("/sys/module/vc4/", F_OK));
#endif

    if (_this->egl_data) {
        return SDL_SetError("EGL context already created");
    }

    _this->egl_data = (struct SDL_EGL_VideoData *) SDL_calloc(1, sizeof(SDL_EGL_VideoData));
    if (!_this->egl_data) {
        return SDL_OutOfMemory();
    }

#if SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT
    d3dcompiler = SDL_GetHint(SDL_HINT_VIDEO_WIN_D3DCOMPILER);
    if (d3dcompiler) {
        if (SDL_strcasecmp(d3dcompiler, "none") != 0) {
            if (SDL_LoadObject(d3dcompiler) == NULL) {
                SDL_ClearError();
            }
        }
    } else {
        if (WIN_IsWindowsVistaOrGreater()) {
            /* Try the newer d3d compilers first */
            const char *d3dcompiler_list[] = {
                "d3dcompiler_47.dll", "d3dcompiler_46.dll",
            };
            int i;

            for (i = 0; i < SDL_arraysize(d3dcompiler_list); ++i) {
                if (SDL_LoadObject(d3dcompiler_list[i]) != NULL) {
                    break;
                }
                SDL_ClearError();
            }
        } else {
            if (SDL_LoadObject("d3dcompiler_43.dll") == NULL) {
                SDL_ClearError();
            }
        }
    }
#endif

#ifndef SDL_VIDEO_STATIC_ANGLE
    /* A funny thing, loading EGL.so first does not work on the Raspberry, so we load libGL* first */
    path = SDL_getenv("SDL_VIDEO_GL_DRIVER");
    if (path != NULL) {
        egl_dll_handle = SDL_LoadObject(path);
    }

    if (egl_dll_handle == NULL) {
        if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
            if (_this->gl_config.major_version > 1) {
                path = DEFAULT_OGL_ES2;
                egl_dll_handle = SDL_LoadObject(path);
#ifdef ALT_OGL_ES2
                if (egl_dll_handle == NULL && !vc4) {
                    path = ALT_OGL_ES2;
                    egl_dll_handle = SDL_LoadObject(path);
                }
#endif

            } else {
                path = DEFAULT_OGL_ES;
                egl_dll_handle = SDL_LoadObject(path);
                if (egl_dll_handle == NULL) {
                    path = DEFAULT_OGL_ES_PVR;
                    egl_dll_handle = SDL_LoadObject(path);
                }
#ifdef ALT_OGL_ES2
                if (egl_dll_handle == NULL && !vc4) {
                    path = ALT_OGL_ES2;
                    egl_dll_handle = SDL_LoadObject(path);
                }
#endif
            }
        }
#ifdef DEFAULT_OGL         
        else {
            path = DEFAULT_OGL;
            egl_dll_handle = SDL_LoadObject(path);
#ifdef ALT_OGL
            if (egl_dll_handle == NULL) {
                path = ALT_OGL;
                egl_dll_handle = SDL_LoadObject(path);
            }
#endif
        }
#endif        
    }
    _this->egl_data->egl_dll_handle = egl_dll_handle;

    if (egl_dll_handle == NULL) {
        return SDL_SetError("Could not initialize OpenGL / GLES library");
    }

    /* Loading libGL* in the previous step took care of loading libEGL.so, but we future proof by double checking */
    if (egl_path != NULL) {
        dll_handle = SDL_LoadObject(egl_path);
    }   
    /* Try loading a EGL symbol, if it does not work try the default library paths */
    if (dll_handle == NULL || SDL_LoadFunction(dll_handle, "eglChooseConfig") == NULL) {
        if (dll_handle != NULL) {
            SDL_UnloadObject(dll_handle);
        }
        path = SDL_getenv("SDL_VIDEO_EGL_DRIVER");
        if (path == NULL) {
            path = DEFAULT_EGL;
        }
        dll_handle = SDL_LoadObject(path);

#ifdef ALT_EGL
        if (dll_handle == NULL && !vc4) {
            path = ALT_EGL;
            dll_handle = SDL_LoadObject(path);
        }
#endif

        if (dll_handle == NULL || SDL_LoadFunction(dll_handle, "eglChooseConfig") == NULL) {
            if (dll_handle != NULL) {
                SDL_UnloadObject(dll_handle);
            }
            return SDL_SetError("Could not load EGL library");
        }
        SDL_ClearError();
    }
#endif

    _this->egl_data->dll_handle = dll_handle;

    /* Load new function pointers */
    LOAD_FUNC(eglGetDisplay);
    LOAD_FUNC(eglInitialize);
    LOAD_FUNC(eglTerminate);
    LOAD_FUNC(eglGetProcAddress);
    LOAD_FUNC(eglChooseConfig);
    LOAD_FUNC(eglGetConfigAttrib);
    LOAD_FUNC(eglCreateContext);
    LOAD_FUNC(eglDestroyContext);
    LOAD_FUNC(eglCreatePbufferSurface);
    LOAD_FUNC(eglCreateWindowSurface);
    LOAD_FUNC(eglDestroySurface);
    LOAD_FUNC(eglMakeCurrent);
    LOAD_FUNC(eglSwapBuffers);
    LOAD_FUNC(eglSwapInterval);
    LOAD_FUNC(eglWaitNative);
    LOAD_FUNC(eglWaitGL);
    LOAD_FUNC(eglBindAPI);
    LOAD_FUNC(eglQueryAPI);
    LOAD_FUNC(eglQueryString);
    LOAD_FUNC(eglGetError);
    LOAD_FUNC_EGLEXT(eglQueryDevicesEXT);
    LOAD_FUNC_EGLEXT(eglGetPlatformDisplayEXT);
    /* Atomic functions */
    LOAD_FUNC_EGLEXT(eglCreateSyncKHR);
    LOAD_FUNC_EGLEXT(eglDestroySyncKHR);
    LOAD_FUNC_EGLEXT(eglDupNativeFenceFDANDROID);
    LOAD_FUNC_EGLEXT(eglWaitSyncKHR);
    LOAD_FUNC_EGLEXT(eglClientWaitSyncKHR);
    /* Atomic functions end */

    if (path) {
        SDL_strlcpy(_this->gl_config.driver_path, path, sizeof(_this->gl_config.driver_path) - 1);
    } else {
        *_this->gl_config.driver_path = '\0';
    }

    return 0;
}

static void
SDL_EGL_GetVersion(_THIS) {
    if (_this->egl_data->eglQueryString) {
        const char *egl_version = _this->egl_data->eglQueryString(_this->egl_data->egl_display, EGL_VERSION);
        if (egl_version) {
            int major = 0, minor = 0;
            if (SDL_sscanf(egl_version, "%d.%d", &major, &minor) == 2) {
                _this->egl_data->egl_version_major = major;
                _this->egl_data->egl_version_minor = minor;
            } else {
                SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Could not parse EGL version string: %s", egl_version);
            }
        }
    }
}

int
SDL_EGL_LoadLibrary(_THIS, const char *egl_path, NativeDisplayType native_display, EGLenum platform)
{
    int library_load_retcode = SDL_EGL_LoadLibraryOnly(_this, egl_path);
    if (library_load_retcode != 0) {
        return library_load_retcode;
    }

    _this->egl_data->egl_display = EGL_NO_DISPLAY;

#if !defined(__WINRT__)
    if (platform) {
        /* EGL 1.5 allows querying for client version with EGL_NO_DISPLAY
         * --
         * Khronos doc: "EGL_BAD_DISPLAY is generated if display is not an EGL display connection, unless display is EGL_NO_DISPLAY and name is EGL_EXTENSIONS."
         * Therefore SDL_EGL_GetVersion() shouldn't work with uninitialized display.
         * - it actually doesn't work on Android that has 1.5 egl client
         * - it works on desktop X11 (using SDL_VIDEO_X11_FORCE_EGL=1) */
        SDL_EGL_GetVersion(_this);

        if (_this->egl_data->egl_version_major == 1 && _this->egl_data->egl_version_minor == 5) {
            LOAD_FUNC(eglGetPlatformDisplay);
        }

        if (_this->egl_data->eglGetPlatformDisplay) {
            _this->egl_data->egl_display = _this->egl_data->eglGetPlatformDisplay(platform, (void *)(uintptr_t)native_display, NULL);
        } else {
            if (SDL_EGL_HasExtension(_this, SDL_EGL_CLIENT_EXTENSION, "EGL_EXT_platform_base")) {
                _this->egl_data->eglGetPlatformDisplayEXT = SDL_EGL_GetProcAddress(_this, "eglGetPlatformDisplayEXT");
                if (_this->egl_data->eglGetPlatformDisplayEXT) {
                    _this->egl_data->egl_display = _this->egl_data->eglGetPlatformDisplayEXT(platform, (void *)(uintptr_t)native_display, NULL);
                }
            }
        }
    }
    /* Try the implementation-specific eglGetDisplay even if eglGetPlatformDisplay fails */
    if (_this->egl_data->egl_display == EGL_NO_DISPLAY) {
        _this->egl_data->egl_display = _this->egl_data->eglGetDisplay(native_display);
    }
    if (_this->egl_data->egl_display == EGL_NO_DISPLAY) {
        _this->gl_config.driver_loaded = 0;
        *_this->gl_config.driver_path = '\0';
        return SDL_SetError("Could not get EGL display");
    }

    if (_this->egl_data->eglInitialize(_this->egl_data->egl_display, NULL, NULL) != EGL_TRUE) {
        _this->gl_config.driver_loaded = 0;
        *_this->gl_config.driver_path = '\0';
        return SDL_SetError("Could not initialize EGL");
    }
#endif

    /* Get the EGL version with a valid egl_display, for EGL <= 1.4 */
    SDL_EGL_GetVersion(_this);

    _this->egl_data->is_offscreen = 0;

    return 0;
}

/**
   On multi GPU machines EGL device 0 is not always the first valid GPU.
   Container environments can restrict access to some GPUs that are still listed in the EGL
   device list. If the requested device is a restricted GPU and cannot be used
   (eglInitialize() will fail) then attempt to automatically and silently select the next
   valid available GPU for EGL to use.
*/

int
SDL_EGL_InitializeOffscreen(_THIS, int device)
{
    void *egl_devices[SDL_EGL_MAX_DEVICES];
    EGLint num_egl_devices = 0;
    const char *egl_device_hint;

    if (_this->gl_config.driver_loaded != 1) {
        return SDL_SetError("SDL_EGL_LoadLibraryOnly() has not been called or has failed.");
    }

    /* Check for all extensions that are optional until used and fail if any is missing */
    if (_this->egl_data->eglQueryDevicesEXT == NULL) {
        return SDL_SetError("eglQueryDevicesEXT is missing (EXT_device_enumeration not supported by the drivers?)");
    }

    if (_this->egl_data->eglGetPlatformDisplayEXT == NULL) {
        return SDL_SetError("eglGetPlatformDisplayEXT is missing (EXT_platform_base not supported by the drivers?)");
    }

    if (_this->egl_data->eglQueryDevicesEXT(SDL_EGL_MAX_DEVICES, egl_devices, &num_egl_devices) != EGL_TRUE) {
        return SDL_SetError("eglQueryDevicesEXT() failed");
    }

    egl_device_hint = SDL_GetHint("SDL_HINT_EGL_DEVICE");
    if (egl_device_hint) {
        device = SDL_atoi(egl_device_hint);

        if (device >= num_egl_devices) {
            return SDL_SetError("Invalid EGL device is requested.");
        }

        _this->egl_data->egl_display = _this->egl_data->eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, egl_devices[device], NULL);

        if (_this->egl_data->egl_display == EGL_NO_DISPLAY) {
            return SDL_SetError("eglGetPlatformDisplayEXT() failed.");
        }

        if (_this->egl_data->eglInitialize(_this->egl_data->egl_display, NULL, NULL) != EGL_TRUE) {
            return SDL_SetError("Could not initialize EGL");
        }
    }
    else {
        int i;
        SDL_bool found = SDL_FALSE;
        EGLDisplay attempted_egl_display;

        /* If no hint is provided lets look for the first device/display that will allow us to eglInit */
        for (i = 0; i < num_egl_devices; i++) {
            attempted_egl_display = _this->egl_data->eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, egl_devices[i], NULL);

            if (attempted_egl_display == EGL_NO_DISPLAY) {
                continue;
            }

            if (_this->egl_data->eglInitialize(attempted_egl_display, NULL, NULL) != EGL_TRUE) {
                _this->egl_data->eglTerminate(attempted_egl_display);
                continue;
            }

            /* We did not fail, we'll pick this one! */
            _this->egl_data->egl_display = attempted_egl_display;
            found = SDL_TRUE;

            break;
        }

        if (!found) {
            return SDL_SetError("Could not find a valid EGL device to initialize");
        }
    }

    /* Get the EGL version with a valid egl_display, for EGL <= 1.4 */
    SDL_EGL_GetVersion(_this);

    _this->egl_data->is_offscreen = 1;

    return 0;
}

void
SDL_EGL_SetRequiredVisualId(_THIS, int visual_id) 
{
    _this->egl_data->egl_required_visual_id=visual_id;
}

#ifdef DUMP_EGL_CONFIG

#define ATTRIBUTE(_attr) { _attr, #_attr }

typedef struct {
    EGLint attribute;
    char const* name;
} Attribute;

static
Attribute all_attributes[] = {
        ATTRIBUTE( EGL_BUFFER_SIZE ),
        ATTRIBUTE( EGL_ALPHA_SIZE ),
        ATTRIBUTE( EGL_BLUE_SIZE ),
        ATTRIBUTE( EGL_GREEN_SIZE ),
        ATTRIBUTE( EGL_RED_SIZE ),
        ATTRIBUTE( EGL_DEPTH_SIZE ),
        ATTRIBUTE( EGL_STENCIL_SIZE ),
        ATTRIBUTE( EGL_CONFIG_CAVEAT ),
        ATTRIBUTE( EGL_CONFIG_ID ),
        ATTRIBUTE( EGL_LEVEL ),
        ATTRIBUTE( EGL_MAX_PBUFFER_HEIGHT ),
        ATTRIBUTE( EGL_MAX_PBUFFER_WIDTH ),
        ATTRIBUTE( EGL_MAX_PBUFFER_PIXELS ),
        ATTRIBUTE( EGL_NATIVE_RENDERABLE ),
        ATTRIBUTE( EGL_NATIVE_VISUAL_ID ),
        ATTRIBUTE( EGL_NATIVE_VISUAL_TYPE ),
        ATTRIBUTE( EGL_SAMPLES ),
        ATTRIBUTE( EGL_SAMPLE_BUFFERS ),
        ATTRIBUTE( EGL_SURFACE_TYPE ),
        ATTRIBUTE( EGL_TRANSPARENT_TYPE ),
        ATTRIBUTE( EGL_TRANSPARENT_BLUE_VALUE ),
        ATTRIBUTE( EGL_TRANSPARENT_GREEN_VALUE ),
        ATTRIBUTE( EGL_TRANSPARENT_RED_VALUE ),
        ATTRIBUTE( EGL_BIND_TO_TEXTURE_RGB ),
        ATTRIBUTE( EGL_BIND_TO_TEXTURE_RGBA ),
        ATTRIBUTE( EGL_MIN_SWAP_INTERVAL ),
        ATTRIBUTE( EGL_MAX_SWAP_INTERVAL ),
        ATTRIBUTE( EGL_LUMINANCE_SIZE ),
        ATTRIBUTE( EGL_ALPHA_MASK_SIZE ),
        ATTRIBUTE( EGL_COLOR_BUFFER_TYPE ),
        ATTRIBUTE( EGL_RENDERABLE_TYPE ),
        ATTRIBUTE( EGL_MATCH_NATIVE_PIXMAP ),
        ATTRIBUTE( EGL_CONFORMANT ),
};


static void dumpconfig(_THIS, EGLConfig config)
{
    int attr;
    for (attr = 0 ; attr<sizeof(all_attributes)/sizeof(Attribute) ; attr++) {
        EGLint value;
        _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display, config, all_attributes[attr].attribute, &value);
        SDL_Log("\t%-32s: %10d (0x%08x)\n", all_attributes[attr].name, value, value);
    }
}

#endif /* DUMP_EGL_CONFIG */

static int
SDL_EGL_PrivateChooseConfig(_THIS, SDL_bool set_config_caveat_none)
{
    /* 64 seems nice. */
    EGLint attribs[64];
    EGLint found_configs = 0, value;
    /* 128 seems even nicer here */
    EGLConfig configs[128];
    SDL_bool has_matching_format = SDL_FALSE;
    int i, j, best_bitdiff = -1, best_truecolor_bitdiff = -1;
    int truecolor_config_idx = -1;

    /* Get a valid EGL configuration */
    i = 0;
    attribs[i++] = EGL_RED_SIZE;
    attribs[i++] = _this->gl_config.red_size;
    attribs[i++] = EGL_GREEN_SIZE;
    attribs[i++] = _this->gl_config.green_size;
    attribs[i++] = EGL_BLUE_SIZE;
    attribs[i++] = _this->gl_config.blue_size;

    if (set_config_caveat_none) {
        attribs[i++] = EGL_CONFIG_CAVEAT;
        attribs[i++] = EGL_NONE;
    }

    if (_this->gl_config.alpha_size) {
        attribs[i++] = EGL_ALPHA_SIZE;
        attribs[i++] = _this->gl_config.alpha_size;
    }

    if (_this->gl_config.buffer_size) {
        attribs[i++] = EGL_BUFFER_SIZE;
        attribs[i++] = _this->gl_config.buffer_size;
    }

    if (_this->gl_config.depth_size) {
        attribs[i++] = EGL_DEPTH_SIZE;
        attribs[i++] = _this->gl_config.depth_size;
    }

    if (_this->gl_config.stencil_size) {
        attribs[i++] = EGL_STENCIL_SIZE;
        attribs[i++] = _this->gl_config.stencil_size;
    }

    if (_this->gl_config.multisamplebuffers) {
        attribs[i++] = EGL_SAMPLE_BUFFERS;
        attribs[i++] = _this->gl_config.multisamplebuffers;
    }

    if (_this->gl_config.multisamplesamples) {
        attribs[i++] = EGL_SAMPLES;
        attribs[i++] = _this->gl_config.multisamplesamples;
    }

    if (_this->egl_data->is_offscreen) {
        attribs[i++] = EGL_SURFACE_TYPE;
        attribs[i++] = EGL_PBUFFER_BIT;
    }

    attribs[i++] = EGL_RENDERABLE_TYPE;
    if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
#ifdef EGL_KHR_create_context
        if (_this->gl_config.major_version >= 3 &&
            SDL_EGL_HasExtension(_this, SDL_EGL_DISPLAY_EXTENSION, "EGL_KHR_create_context")) {
            attribs[i++] = EGL_OPENGL_ES3_BIT_KHR;
        } else
#endif
        if (_this->gl_config.major_version >= 2) {
            attribs[i++] = EGL_OPENGL_ES2_BIT;
        } else {
            attribs[i++] = EGL_OPENGL_ES_BIT;
        }
        _this->egl_data->eglBindAPI(EGL_OPENGL_ES_API);
    } else {
        attribs[i++] = EGL_OPENGL_BIT;
        _this->egl_data->eglBindAPI(EGL_OPENGL_API);
    }

    if (_this->egl_data->egl_surfacetype) {
        attribs[i++] = EGL_SURFACE_TYPE;
        attribs[i++] = _this->egl_data->egl_surfacetype;
    }

    attribs[i++] = EGL_NONE;

    SDL_assert(i < SDL_arraysize(attribs));

    if (_this->egl_data->eglChooseConfig(_this->egl_data->egl_display,
        attribs,
        configs, SDL_arraysize(configs),
        &found_configs) == EGL_FALSE ||
        found_configs == 0) {
        return -1;
    }

    /* first ensure that a found config has a matching format, or the function will fall through. */
    if (_this->egl_data->egl_required_visual_id)
    {
        for (i = 0; i < found_configs; i++ ) {
            EGLint format;
            _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display,
                                            configs[i],
                                            EGL_NATIVE_VISUAL_ID, &format);
            if (_this->egl_data->egl_required_visual_id == format) {
                has_matching_format = SDL_TRUE;
                break;
            }
        }
    }

    /* eglChooseConfig returns a number of configurations that match or exceed the requested attribs. */
    /* From those, we select the one that matches our requirements more closely via a makeshift algorithm */

    for (i = 0; i < found_configs; i++ ) {
        SDL_bool is_truecolor = SDL_FALSE;
        int bitdiff = 0;

        if (has_matching_format && _this->egl_data->egl_required_visual_id) {
            EGLint format;
            _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display,
                                            configs[i],
                                            EGL_NATIVE_VISUAL_ID, &format);
            if (_this->egl_data->egl_required_visual_id != format) {
                continue;
            }
        }

        _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display, configs[i], EGL_RED_SIZE, &value);
        if (value == 8) {
            _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display, configs[i], EGL_GREEN_SIZE, &value);
            if (value == 8) {
                _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display, configs[i], EGL_BLUE_SIZE, &value);
                if (value == 8) {
                    is_truecolor = SDL_TRUE;
                }
            }
        }

        for (j = 0; j < SDL_arraysize(attribs) - 1; j += 2) {
            if (attribs[j] == EGL_NONE) {
               break;
            }

            if ( attribs[j+1] != EGL_DONT_CARE && (
                attribs[j] == EGL_RED_SIZE ||
                attribs[j] == EGL_GREEN_SIZE ||
                attribs[j] == EGL_BLUE_SIZE ||
                attribs[j] == EGL_ALPHA_SIZE ||
                attribs[j] == EGL_DEPTH_SIZE ||
                attribs[j] == EGL_STENCIL_SIZE)) {
                _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display, configs[i], attribs[j], &value);
                bitdiff += value - attribs[j + 1]; /* value is always >= attrib */
            }
        }

        if ((bitdiff < best_bitdiff) || (best_bitdiff == -1)) {
            _this->egl_data->egl_config = configs[i];
            best_bitdiff = bitdiff;
        }

        if (is_truecolor && ((bitdiff < best_truecolor_bitdiff) || (best_truecolor_bitdiff == -1))) {
            truecolor_config_idx = i;
            best_truecolor_bitdiff = bitdiff;
        }
    }

    #define FAVOR_TRUECOLOR 1
    #if FAVOR_TRUECOLOR
    /* Some apps request a low color depth, either because they _assume_
       they'll get a larger one but don't want to fail if only smaller ones
       are available, or they just never called SDL_GL_SetAttribute at all and
       got a tiny default. For these cases, a game that would otherwise run
       at 24-bit color might get dithered down to something smaller, which is
       worth avoiding. If the app requested <= 16 bit color and an exact 24-bit
       match is available, favor that. Otherwise, we look for the closest
       match. Note that while the API promises what you request _or better_,
       it's feasible this can be disastrous for performance for custom software
       on small hardware that all expected to actually get 16-bit color. In this
       case, turn off FAVOR_TRUECOLOR (and maybe send a patch to make this more
       flexible). */
    if ( ((_this->gl_config.red_size + _this->gl_config.blue_size + _this->gl_config.green_size) <= 16) ) {
        if (truecolor_config_idx != -1) {
            _this->egl_data->egl_config = configs[truecolor_config_idx];
        }
    }
    #endif

#ifdef DUMP_EGL_CONFIG
    dumpconfig(_this, _this->egl_data->egl_config);
#endif

    return 0;
}

int
SDL_EGL_ChooseConfig(_THIS)
{
    int ret;

    if (!_this->egl_data) {
        /* The EGL library wasn't loaded, SDL_GetError() should have info */
        return -1;
    }

    /* Try with EGL_CONFIG_CAVEAT set to EGL_NONE, to avoid any EGL_SLOW_CONFIG or EGL_NON_CONFORMANT_CONFIG */
    ret = SDL_EGL_PrivateChooseConfig(_this, SDL_TRUE);
    if (ret == 0) {
        return 0;
    }

    /* Fallback with all configs */
    ret = SDL_EGL_PrivateChooseConfig(_this, SDL_FALSE);
    if (ret == 0) {
        SDL_Log("SDL_EGL_ChooseConfig: found a slow EGL config");
        return 0;
    }

    return SDL_EGL_SetError("Couldn't find matching EGL config", "eglChooseConfig");
}

SDL_GLContext
SDL_EGL_CreateContext(_THIS, EGLSurface egl_surface)
{
    /* max 14 values plus terminator. */
    EGLint attribs[15];
    int attr = 0;

    EGLContext egl_context, share_context = EGL_NO_CONTEXT;
    EGLint profile_mask = _this->gl_config.profile_mask;
    EGLint major_version = _this->gl_config.major_version;
    EGLint minor_version = _this->gl_config.minor_version;
    SDL_bool profile_es = (profile_mask == SDL_GL_CONTEXT_PROFILE_ES);

    if (!_this->egl_data) {
        /* The EGL library wasn't loaded, SDL_GetError() should have info */
        return NULL;
    }

    if (_this->gl_config.share_with_current_context) {
        share_context = (EGLContext)SDL_GL_GetCurrentContext();
    }

#if SDL_VIDEO_DRIVER_ANDROID
    if ((_this->gl_config.flags & SDL_GL_CONTEXT_DEBUG_FLAG) != 0) {
        /* If SDL_GL_CONTEXT_DEBUG_FLAG is set but EGL_KHR_debug unsupported, unset.
         * This is required because some Android devices like to complain about it
         * by "silently" failing, logging a hint which could be easily overlooked:
         * E/libEGL  (26984): validate_display:255 error 3008 (EGL_BAD_DISPLAY)
         * The following explicitly checks for EGL_KHR_debug before EGL 1.5
         */
        int egl_version_major = _this->egl_data->egl_version_major;
        int egl_version_minor = _this->egl_data->egl_version_minor;
        if (((egl_version_major < 1) || (egl_version_major == 1 && egl_version_minor < 5)) &&
            !SDL_EGL_HasExtension(_this, SDL_EGL_DISPLAY_EXTENSION, "EGL_KHR_debug")) {
            /* SDL profile bits match EGL profile bits. */
            _this->gl_config.flags &= ~SDL_GL_CONTEXT_DEBUG_FLAG;
        }
    }
#endif

    /* Set the context version and other attributes. */
    if ((major_version < 3 || (minor_version == 0 && profile_es)) &&
        _this->gl_config.flags == 0 &&
        (profile_mask == 0 || profile_es)) {
        /* Create a context without using EGL_KHR_create_context attribs.
         * When creating a GLES context without EGL_KHR_create_context we can
         * only specify the major version. When creating a desktop GL context
         * we can't specify any version, so we only try in that case when the
         * version is less than 3.0 (matches SDL's GLX/WGL behavior.)
         */
        if (profile_es) {
            attribs[attr++] = EGL_CONTEXT_CLIENT_VERSION;
            attribs[attr++] = SDL_max(major_version, 1);
        }
    } else {
#ifdef EGL_KHR_create_context
        /* The Major/minor version, context profiles, and context flags can
         * only be specified when this extension is available.
         */
        if (SDL_EGL_HasExtension(_this, SDL_EGL_DISPLAY_EXTENSION, "EGL_KHR_create_context")) {
            attribs[attr++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
            attribs[attr++] = major_version;
            attribs[attr++] = EGL_CONTEXT_MINOR_VERSION_KHR;
            attribs[attr++] = minor_version;

            /* SDL profile bits match EGL profile bits. */
            if (profile_mask != 0 && profile_mask != SDL_GL_CONTEXT_PROFILE_ES) {
                attribs[attr++] = EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR;
                attribs[attr++] = profile_mask;
            }

            /* SDL flags match EGL flags. */
            if (_this->gl_config.flags != 0) {
                attribs[attr++] = EGL_CONTEXT_FLAGS_KHR;
                attribs[attr++] = _this->gl_config.flags;
            }
        } else
#endif /* EGL_KHR_create_context */
        {
            SDL_SetError("Could not create EGL context (context attributes are not supported)");
            return NULL;
        }
    }

    if (_this->gl_config.no_error) {
#ifdef EGL_KHR_create_context_no_error
        if (SDL_EGL_HasExtension(_this, SDL_EGL_DISPLAY_EXTENSION, "EGL_KHR_create_context_no_error")) {
            attribs[attr++] = EGL_CONTEXT_OPENGL_NO_ERROR_KHR;
            attribs[attr++] = _this->gl_config.no_error;
        } else
#endif
        {
            SDL_SetError("EGL implementation does not support no_error contexts");
            return NULL;
        }
    }

    attribs[attr++] = EGL_NONE;

    /* Bind the API */
    if (profile_es) {
        _this->egl_data->eglBindAPI(EGL_OPENGL_ES_API);
    } else {
        _this->egl_data->eglBindAPI(EGL_OPENGL_API);
    }

    egl_context = _this->egl_data->eglCreateContext(_this->egl_data->egl_display,
                                      _this->egl_data->egl_config,
                                      share_context, attribs);

    if (egl_context == EGL_NO_CONTEXT) {
        SDL_EGL_SetError("Could not create EGL context", "eglCreateContext");
        return NULL;
    }

    _this->egl_data->egl_swapinterval = 0;

    if (SDL_EGL_MakeCurrent(_this, egl_surface, egl_context) < 0) {
        /* Save the SDL error set by SDL_EGL_MakeCurrent */
        char errorText[1024];
        SDL_strlcpy(errorText, SDL_GetError(), SDL_arraysize(errorText));

        /* Delete the context, which may alter the value returned by SDL_GetError() */
        SDL_EGL_DeleteContext(_this, egl_context);

        /* Restore the SDL error */
        SDL_SetError("%s", errorText);

        return NULL;
    }

    /* Check whether making contexts current without a surface is supported.
     * First condition: EGL must support it. That's the case for EGL 1.5
     * or later, or if the EGL_KHR_surfaceless_context extension is present. */
    if ((_this->egl_data->egl_version_major > 1) ||
        ((_this->egl_data->egl_version_major == 1) && (_this->egl_data->egl_version_minor >= 5)) ||
        SDL_EGL_HasExtension(_this, SDL_EGL_DISPLAY_EXTENSION, "EGL_KHR_surfaceless_context"))
    {
        /* Secondary condition: The client API must support it. */
        if (profile_es) {
            /* On OpenGL ES, the GL_OES_surfaceless_context extension must be
             * present. */
            if (SDL_GL_ExtensionSupported("GL_OES_surfaceless_context")) {
                _this->gl_allow_no_surface = SDL_TRUE;
            }
#if SDL_VIDEO_OPENGL
        } else {
            /* Desktop OpenGL supports it by default from version 3.0 on. */
            void (APIENTRY * glGetIntegervFunc) (GLenum pname, GLint * params);
            glGetIntegervFunc = SDL_GL_GetProcAddress("glGetIntegerv");
            if (glGetIntegervFunc) {
                GLint v = 0;
                glGetIntegervFunc(GL_MAJOR_VERSION, &v);
                if (v >= 3) {
                    _this->gl_allow_no_surface = SDL_TRUE;
                }
            }
#endif
        }
    }

    return (SDL_GLContext) egl_context;
}

int
SDL_EGL_MakeCurrent(_THIS, EGLSurface egl_surface, SDL_GLContext context)
{
    EGLContext egl_context = (EGLContext) context;

    if (!_this->egl_data) {
        return SDL_SetError("OpenGL not initialized");
    }

    if (!_this->egl_data->eglMakeCurrent) {
        if (!egl_surface && !context) {
            /* Can't do the nothing there is to do? Probably trying to cleanup a failed startup, just return. */
            return 0;
        } else {
            return SDL_SetError("OpenGL not initialized");  /* something clearly went wrong somewhere. */
        }
    }

    /* The android emulator crashes badly if you try to eglMakeCurrent 
     * with a valid context and invalid surface, so we have to check for both here.
     */
    if (!egl_context || (!egl_surface && !_this->gl_allow_no_surface)) {
         _this->egl_data->eglMakeCurrent(_this->egl_data->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    } else {
        if (!_this->egl_data->eglMakeCurrent(_this->egl_data->egl_display,
            egl_surface, egl_surface, egl_context)) {
            return SDL_EGL_SetError("Unable to make EGL context current", "eglMakeCurrent");
        }
    }

    return 0;
}

int
SDL_EGL_SetSwapInterval(_THIS, int interval)
{
    EGLBoolean status;
    
    if (!_this->egl_data) {
        return SDL_SetError("EGL not initialized");
    }

    /* FIXME: Revisit this check when EGL_EXT_swap_control_tear is published:
     * https://github.com/KhronosGroup/EGL-Registry/pull/113
     */
    if (interval < 0) {
        return SDL_SetError("Late swap tearing currently unsupported");
    }
    
    status = _this->egl_data->eglSwapInterval(_this->egl_data->egl_display, interval);
    if (status == EGL_TRUE) {
        _this->egl_data->egl_swapinterval = interval;
        return 0;
    }
    
    return SDL_EGL_SetError("Unable to set the EGL swap interval", "eglSwapInterval");
}

int
SDL_EGL_GetSwapInterval(_THIS)
{
    if (!_this->egl_data) {
        SDL_SetError("EGL not initialized");
        return 0;
    }
    
    return _this->egl_data->egl_swapinterval;
}

int
SDL_EGL_SwapBuffers(_THIS, EGLSurface egl_surface)
{
    if (!_this->egl_data->eglSwapBuffers(_this->egl_data->egl_display, egl_surface)) {
        return SDL_EGL_SetError("unable to show color buffer in an OS-native window", "eglSwapBuffers");
    }
    return 0;
}

void
SDL_EGL_DeleteContext(_THIS, SDL_GLContext context)
{
    EGLContext egl_context = (EGLContext) context;

    /* Clean up GLES and EGL */
    if (!_this->egl_data) {
        return;
    }
    
    if (egl_context != NULL && egl_context != EGL_NO_CONTEXT) {
        _this->egl_data->eglDestroyContext(_this->egl_data->egl_display, egl_context);
    }
        
}

EGLSurface *
SDL_EGL_CreateSurface(_THIS, NativeWindowType nw) 
{
#if SDL_VIDEO_DRIVER_ANDROID
    EGLint format_wanted;
    EGLint format_got;
#endif
    /* max 2 values plus terminator. */
    EGLint attribs[3];
    int attr = 0;

    EGLSurface * surface;

    if (SDL_EGL_ChooseConfig(_this) != 0) {
        return EGL_NO_SURFACE;
    }

#if SDL_VIDEO_DRIVER_ANDROID
    /* On Android, EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry(). */
    _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display,
            _this->egl_data->egl_config,
            EGL_NATIVE_VISUAL_ID, &format_wanted);

    /* Format based on selected egl config. */
    ANativeWindow_setBuffersGeometry(nw, 0, 0, format_wanted);
#endif

    if (_this->gl_config.framebuffer_srgb_capable) {
#ifdef EGL_KHR_gl_colorspace
        if (SDL_EGL_HasExtension(_this, SDL_EGL_DISPLAY_EXTENSION, "EGL_KHR_gl_colorspace")) {
            attribs[attr++] = EGL_GL_COLORSPACE_KHR;
            attribs[attr++] = EGL_GL_COLORSPACE_SRGB_KHR;
        } else
#endif
        {
            SDL_SetError("EGL implementation does not support sRGB system framebuffers");
            return EGL_NO_SURFACE;
        }
    }

    attribs[attr++] = EGL_NONE;
    
    surface = _this->egl_data->eglCreateWindowSurface(
            _this->egl_data->egl_display,
            _this->egl_data->egl_config,
            nw, &attribs[0]);
    if (surface == EGL_NO_SURFACE) {
        SDL_EGL_SetError("unable to create an EGL window surface", "eglCreateWindowSurface");
    }

#if SDL_VIDEO_DRIVER_ANDROID
    format_got = ANativeWindow_getFormat(nw);
    Android_SetFormat(format_wanted, format_got);
#endif

    return surface;
}

EGLSurface
SDL_EGL_CreateOffscreenSurface(_THIS, int width, int height)
{
    EGLint attributes[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };

    if (SDL_EGL_ChooseConfig(_this) != 0) {
        return EGL_NO_SURFACE;
    }

    return _this->egl_data->eglCreatePbufferSurface(
        _this->egl_data->egl_display,
        _this->egl_data->egl_config,
        attributes);
}

void
SDL_EGL_DestroySurface(_THIS, EGLSurface egl_surface) 
{
    if (!_this->egl_data) {
        return;
    }
    
    if (egl_surface != EGL_NO_SURFACE) {
        _this->egl_data->eglDestroySurface(_this->egl_data->egl_display, egl_surface);
    }
}

#endif /* SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */

