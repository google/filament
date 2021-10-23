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

#include "SDL_loadso.h"
#include "SDL_windowsvideo.h"
#include "SDL_windowsopengles.h"
#include "SDL_hints.h"

/* WGL implementation of SDL OpenGL support */

#if SDL_VIDEO_OPENGL_WGL
#include "SDL_opengl.h"

#define DEFAULT_OPENGL "OPENGL32.DLL"

#ifndef WGL_ARB_create_context
#define WGL_ARB_create_context
#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB     0x2093
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB       0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#ifndef WGL_ARB_create_context_profile
#define WGL_ARB_create_context_profile
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif

#ifndef WGL_ARB_create_context_robustness
#define WGL_ARB_create_context_robustness
#define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB         0x00000004
#define WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB     0x8256
#define WGL_NO_RESET_NOTIFICATION_ARB                   0x8261
#define WGL_LOSE_CONTEXT_ON_RESET_ARB                   0x8252
#endif
#endif

#ifndef WGL_EXT_create_context_es2_profile
#define WGL_EXT_create_context_es2_profile
#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT           0x00000004
#endif

#ifndef WGL_EXT_create_context_es_profile
#define WGL_EXT_create_context_es_profile
#define WGL_CONTEXT_ES_PROFILE_BIT_EXT            0x00000004
#endif

#ifndef WGL_ARB_framebuffer_sRGB
#define WGL_ARB_framebuffer_sRGB
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB                0x20A9
#endif

#ifndef WGL_ARB_context_flush_control
#define WGL_ARB_context_flush_control
#define WGL_CONTEXT_RELEASE_BEHAVIOR_ARB   0x2097
#define WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB           0x0000
#define WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB          0x2098
#endif

#ifndef WGL_ARB_create_context_no_error
#define WGL_ARB_create_context_no_error
#define WGL_CONTEXT_OPENGL_NO_ERROR_ARB                 0x31B3
#endif

typedef HGLRC(APIENTRYP PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC,
                                                            HGLRC
                                                            hShareContext,
                                                            const int
                                                            *attribList);

int
WIN_GL_LoadLibrary(_THIS, const char *path)
{
    void *handle;

    if (path == NULL) {
        path = SDL_getenv("SDL_OPENGL_LIBRARY");
    }
    if (path == NULL) {
        path = DEFAULT_OPENGL;
    }
    _this->gl_config.dll_handle = SDL_LoadObject(path);
    if (!_this->gl_config.dll_handle) {
        return -1;
    }
    SDL_strlcpy(_this->gl_config.driver_path, path,
                SDL_arraysize(_this->gl_config.driver_path));

    /* Allocate OpenGL memory */
    _this->gl_data = (struct SDL_GLDriverData *) SDL_calloc(1, sizeof(struct SDL_GLDriverData));
    if (!_this->gl_data) {
        return SDL_OutOfMemory();
    }

    /* Load function pointers */
    handle = _this->gl_config.dll_handle;
    _this->gl_data->wglGetProcAddress = (void *(WINAPI *) (const char *))
        SDL_LoadFunction(handle, "wglGetProcAddress");
    _this->gl_data->wglCreateContext = (HGLRC(WINAPI *) (HDC))
        SDL_LoadFunction(handle, "wglCreateContext");
    _this->gl_data->wglDeleteContext = (BOOL(WINAPI *) (HGLRC))
        SDL_LoadFunction(handle, "wglDeleteContext");
    _this->gl_data->wglMakeCurrent = (BOOL(WINAPI *) (HDC, HGLRC))
        SDL_LoadFunction(handle, "wglMakeCurrent");
    _this->gl_data->wglShareLists = (BOOL(WINAPI *) (HGLRC, HGLRC))
        SDL_LoadFunction(handle, "wglShareLists");

    if (!_this->gl_data->wglGetProcAddress ||
        !_this->gl_data->wglCreateContext ||
        !_this->gl_data->wglDeleteContext ||
        !_this->gl_data->wglMakeCurrent) {
        return SDL_SetError("Could not retrieve OpenGL functions");
    }

    /* XXX Too sleazy? WIN_GL_InitExtensions looks for certain OpenGL
       extensions via SDL_GL_DeduceMaxSupportedESProfile. This uses
       SDL_GL_ExtensionSupported which in turn calls SDL_GL_GetProcAddress.
       However SDL_GL_GetProcAddress will fail if the library is not
       loaded; it checks for gl_config.driver_loaded > 0. To avoid this
       test failing, increment driver_loaded around the call to
       WIN_GLInitExtensions.

       Successful loading of the library is normally indicated by
       SDL_GL_LoadLibrary incrementing driver_loaded immediately after
       this function returns 0 to it.

       Alternatives to this are:
       - moving SDL_GL_DeduceMaxSupportedESProfile to both the WIN and
         X11 platforms while adding a function equivalent to
         SDL_GL_ExtensionSupported but which directly calls
         glGetProcAddress(). Having 3 copies of the
         SDL_GL_ExtensionSupported makes this alternative unattractive.
       - moving SDL_GL_DeduceMaxSupportedESProfile to a new file shared
         by the WIN and X11 platforms while adding a function equivalent
         to SDL_GL_ExtensionSupported. This is unattractive due to the
         number of project files that will need updating, plus there
         will be 2 copies of the SDL_GL_ExtensionSupported code.
       - Add a private equivalent of SDL_GL_ExtensionSupported to
         SDL_video.c.
       - Move the call to WIN_GL_InitExtensions back to WIN_CreateWindow
         and add a flag to gl_data to avoid multiple calls to this
         expensive function. This is probably the least objectionable
         alternative if this increment/decrement trick is unacceptable.

       Note that the driver_loaded > 0 check needs to remain in
       SDL_GL_ExtensionSupported and SDL_GL_GetProcAddress as they are
       public API functions.
    */
    ++_this->gl_config.driver_loaded;
    WIN_GL_InitExtensions(_this);
    --_this->gl_config.driver_loaded;

    return 0;
}

void *
WIN_GL_GetProcAddress(_THIS, const char *proc)
{
    void *func;

    /* This is to pick up extensions */
    func = _this->gl_data->wglGetProcAddress(proc);
    if (!func) {
        /* This is probably a normal GL function */
        func = GetProcAddress(_this->gl_config.dll_handle, proc);
    }
    return func;
}

void
WIN_GL_UnloadLibrary(_THIS)
{
    SDL_UnloadObject(_this->gl_config.dll_handle);
    _this->gl_config.dll_handle = NULL;

    /* Free OpenGL memory */
    SDL_free(_this->gl_data);
    _this->gl_data = NULL;
}

static void
WIN_GL_SetupPixelFormat(_THIS, PIXELFORMATDESCRIPTOR * pfd)
{
    SDL_zerop(pfd);
    pfd->nSize = sizeof(*pfd);
    pfd->nVersion = 1;
    pfd->dwFlags = (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL);
    if (_this->gl_config.double_buffer) {
        pfd->dwFlags |= PFD_DOUBLEBUFFER;
    }
    if (_this->gl_config.stereo) {
        pfd->dwFlags |= PFD_STEREO;
    }
    pfd->iLayerType = PFD_MAIN_PLANE;
    pfd->iPixelType = PFD_TYPE_RGBA;
    pfd->cRedBits = _this->gl_config.red_size;
    pfd->cGreenBits = _this->gl_config.green_size;
    pfd->cBlueBits = _this->gl_config.blue_size;
    pfd->cAlphaBits = _this->gl_config.alpha_size;
    if (_this->gl_config.buffer_size) {
        pfd->cColorBits =
            _this->gl_config.buffer_size - _this->gl_config.alpha_size;
    } else {
        pfd->cColorBits = (pfd->cRedBits + pfd->cGreenBits + pfd->cBlueBits);
    }
    pfd->cAccumRedBits = _this->gl_config.accum_red_size;
    pfd->cAccumGreenBits = _this->gl_config.accum_green_size;
    pfd->cAccumBlueBits = _this->gl_config.accum_blue_size;
    pfd->cAccumAlphaBits = _this->gl_config.accum_alpha_size;
    pfd->cAccumBits =
        (pfd->cAccumRedBits + pfd->cAccumGreenBits + pfd->cAccumBlueBits +
         pfd->cAccumAlphaBits);
    pfd->cDepthBits = _this->gl_config.depth_size;
    pfd->cStencilBits = _this->gl_config.stencil_size;
}

/* Choose the closest pixel format that meets or exceeds the target.
   FIXME: Should we weight any particular attribute over any other?
*/
static int
WIN_GL_ChoosePixelFormat(HDC hdc, PIXELFORMATDESCRIPTOR * target)
{
    PIXELFORMATDESCRIPTOR pfd;
    int count, index, best = 0;
    unsigned int dist, best_dist = ~0U;

    count = DescribePixelFormat(hdc, 1, sizeof(pfd), NULL);

    for (index = 1; index <= count; index++) {

        if (!DescribePixelFormat(hdc, index, sizeof(pfd), &pfd)) {
            continue;
        }

        if ((pfd.dwFlags & target->dwFlags) != target->dwFlags) {
            continue;
        }

        if (pfd.iLayerType != target->iLayerType) {
            continue;
        }
        if (pfd.iPixelType != target->iPixelType) {
            continue;
        }

        dist = 0;

        if (pfd.cColorBits < target->cColorBits) {
            continue;
        } else {
            dist += (pfd.cColorBits - target->cColorBits);
        }
        if (pfd.cRedBits < target->cRedBits) {
            continue;
        } else {
            dist += (pfd.cRedBits - target->cRedBits);
        }
        if (pfd.cGreenBits < target->cGreenBits) {
            continue;
        } else {
            dist += (pfd.cGreenBits - target->cGreenBits);
        }
        if (pfd.cBlueBits < target->cBlueBits) {
            continue;
        } else {
            dist += (pfd.cBlueBits - target->cBlueBits);
        }
        if (pfd.cAlphaBits < target->cAlphaBits) {
            continue;
        } else {
            dist += (pfd.cAlphaBits - target->cAlphaBits);
        }
        if (pfd.cAccumBits < target->cAccumBits) {
            continue;
        } else {
            dist += (pfd.cAccumBits - target->cAccumBits);
        }
        if (pfd.cAccumRedBits < target->cAccumRedBits) {
            continue;
        } else {
            dist += (pfd.cAccumRedBits - target->cAccumRedBits);
        }
        if (pfd.cAccumGreenBits < target->cAccumGreenBits) {
            continue;
        } else {
            dist += (pfd.cAccumGreenBits - target->cAccumGreenBits);
        }
        if (pfd.cAccumBlueBits < target->cAccumBlueBits) {
            continue;
        } else {
            dist += (pfd.cAccumBlueBits - target->cAccumBlueBits);
        }
        if (pfd.cAccumAlphaBits < target->cAccumAlphaBits) {
            continue;
        } else {
            dist += (pfd.cAccumAlphaBits - target->cAccumAlphaBits);
        }
        if (pfd.cDepthBits < target->cDepthBits) {
            continue;
        } else {
            dist += (pfd.cDepthBits - target->cDepthBits);
        }
        if (pfd.cStencilBits < target->cStencilBits) {
            continue;
        } else {
            dist += (pfd.cStencilBits - target->cStencilBits);
        }

        if (dist < best_dist) {
            best = index;
            best_dist = dist;
        }
    }

    return best;
}

static SDL_bool
HasExtension(const char *extension, const char *extensions)
{
    const char *start;
    const char *where, *terminator;

    /* Extension names should not have spaces. */
    where = SDL_strchr(extension, ' ');
    if (where || *extension == '\0')
        return SDL_FALSE;

    if (!extensions)
        return SDL_FALSE;

    /* It takes a bit of care to be fool-proof about parsing the
     * OpenGL extensions string. Don't be fooled by sub-strings,
     * etc. */

    start = extensions;

    for (;;) {
        where = SDL_strstr(start, extension);
        if (!where)
            break;

        terminator = where + SDL_strlen(extension);
        if (where == start || *(where - 1) == ' ')
            if (*terminator == ' ' || *terminator == '\0')
                return SDL_TRUE;

        start = terminator;
    }
    return SDL_FALSE;
}

void
WIN_GL_InitExtensions(_THIS)
{
    const char *(WINAPI * wglGetExtensionsStringARB) (HDC) = 0;
    const char *extensions;
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;
    PIXELFORMATDESCRIPTOR pfd;

    if (!_this->gl_data) {
        return;
    }

    hwnd =
        CreateWindow(SDL_Appname, SDL_Appname, (WS_POPUP | WS_DISABLED), 0, 0,
        10, 10, NULL, NULL, SDL_Instance, NULL);
    if (!hwnd) {
        return;
    }
    WIN_PumpEvents(_this);

    hdc = GetDC(hwnd);

    WIN_GL_SetupPixelFormat(_this, &pfd);

    SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);

    hglrc = _this->gl_data->wglCreateContext(hdc);
    if (!hglrc) {
        return;
    }
    _this->gl_data->wglMakeCurrent(hdc, hglrc);

    wglGetExtensionsStringARB = (const char *(WINAPI *) (HDC))
        _this->gl_data->wglGetProcAddress("wglGetExtensionsStringARB");
    if (wglGetExtensionsStringARB) {
        extensions = wglGetExtensionsStringARB(hdc);
    } else {
        extensions = NULL;
    }

    /* Check for WGL_ARB_pixel_format */
    _this->gl_data->HAS_WGL_ARB_pixel_format = SDL_FALSE;
    if (HasExtension("WGL_ARB_pixel_format", extensions)) {
        _this->gl_data->wglChoosePixelFormatARB = (BOOL(WINAPI *)
                                                   (HDC, const int *,
                                                    const FLOAT *, UINT,
                                                    int *, UINT *))
            WIN_GL_GetProcAddress(_this, "wglChoosePixelFormatARB");
        _this->gl_data->wglGetPixelFormatAttribivARB =
            (BOOL(WINAPI *) (HDC, int, int, UINT, const int *, int *))
            WIN_GL_GetProcAddress(_this, "wglGetPixelFormatAttribivARB");

        if ((_this->gl_data->wglChoosePixelFormatARB != NULL) &&
            (_this->gl_data->wglGetPixelFormatAttribivARB != NULL)) {
            _this->gl_data->HAS_WGL_ARB_pixel_format = SDL_TRUE;
        }
    }

    /* Check for WGL_EXT_swap_control */
    _this->gl_data->HAS_WGL_EXT_swap_control_tear = SDL_FALSE;
    if (HasExtension("WGL_EXT_swap_control", extensions)) {
        _this->gl_data->wglSwapIntervalEXT =
            WIN_GL_GetProcAddress(_this, "wglSwapIntervalEXT");
        _this->gl_data->wglGetSwapIntervalEXT =
            WIN_GL_GetProcAddress(_this, "wglGetSwapIntervalEXT");
        if (HasExtension("WGL_EXT_swap_control_tear", extensions)) {
            _this->gl_data->HAS_WGL_EXT_swap_control_tear = SDL_TRUE;
        }
    } else {
        _this->gl_data->wglSwapIntervalEXT = NULL;
        _this->gl_data->wglGetSwapIntervalEXT = NULL;
    }

    /* Check for WGL_EXT_create_context_es2_profile */
    if (HasExtension("WGL_EXT_create_context_es2_profile", extensions)) {
        SDL_GL_DeduceMaxSupportedESProfile(
            &_this->gl_data->es_profile_max_supported_version.major,
            &_this->gl_data->es_profile_max_supported_version.minor
        );
    }

    /* Check for WGL_ARB_context_flush_control */
    if (HasExtension("WGL_ARB_context_flush_control", extensions)) {
        _this->gl_data->HAS_WGL_ARB_context_flush_control = SDL_TRUE;
    }

    /* Check for WGL_ARB_create_context_robustness */
    if (HasExtension("WGL_ARB_create_context_robustness", extensions)) {
        _this->gl_data->HAS_WGL_ARB_create_context_robustness = SDL_TRUE;
    }

    /* Check for WGL_ARB_create_context_no_error */
    if (HasExtension("WGL_ARB_create_context_no_error", extensions)) {
        _this->gl_data->HAS_WGL_ARB_create_context_no_error = SDL_TRUE;
    }

    _this->gl_data->wglMakeCurrent(hdc, NULL);
    _this->gl_data->wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    WIN_PumpEvents(_this);
}

static int
WIN_GL_ChoosePixelFormatARB(_THIS, int *iAttribs, float *fAttribs)
{
    HWND hwnd;
    HDC hdc;
    PIXELFORMATDESCRIPTOR pfd;
    HGLRC hglrc;
    int pixel_format = 0;
    unsigned int matching;

    hwnd =
        CreateWindow(SDL_Appname, SDL_Appname, (WS_POPUP | WS_DISABLED), 0, 0,
                     10, 10, NULL, NULL, SDL_Instance, NULL);
    WIN_PumpEvents(_this);

    hdc = GetDC(hwnd);

    WIN_GL_SetupPixelFormat(_this, &pfd);

    SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);

    hglrc = _this->gl_data->wglCreateContext(hdc);
    if (hglrc) {
        _this->gl_data->wglMakeCurrent(hdc, hglrc);

        if (_this->gl_data->HAS_WGL_ARB_pixel_format) {
            _this->gl_data->wglChoosePixelFormatARB(hdc, iAttribs, fAttribs,
                                                    1, &pixel_format,
                                                    &matching);
        }

        _this->gl_data->wglMakeCurrent(hdc, NULL);
        _this->gl_data->wglDeleteContext(hglrc);
    }
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    WIN_PumpEvents(_this);

    return pixel_format;
}

/* actual work of WIN_GL_SetupWindow() happens here. */
static int
WIN_GL_SetupWindowInternal(_THIS, SDL_Window * window)
{
    HDC hdc = ((SDL_WindowData *) window->driverdata)->hdc;
    PIXELFORMATDESCRIPTOR pfd;
    int pixel_format = 0;
    int iAttribs[64];
    int *iAttr;
    int *iAccelAttr;
    float fAttribs[1] = { 0 };

    WIN_GL_SetupPixelFormat(_this, &pfd);

    /* setup WGL_ARB_pixel_format attribs */
    iAttr = &iAttribs[0];

    *iAttr++ = WGL_DRAW_TO_WINDOW_ARB;
    *iAttr++ = GL_TRUE;
    *iAttr++ = WGL_RED_BITS_ARB;
    *iAttr++ = _this->gl_config.red_size;
    *iAttr++ = WGL_GREEN_BITS_ARB;
    *iAttr++ = _this->gl_config.green_size;
    *iAttr++ = WGL_BLUE_BITS_ARB;
    *iAttr++ = _this->gl_config.blue_size;

    if (_this->gl_config.alpha_size) {
        *iAttr++ = WGL_ALPHA_BITS_ARB;
        *iAttr++ = _this->gl_config.alpha_size;
    }

    *iAttr++ = WGL_DOUBLE_BUFFER_ARB;
    *iAttr++ = _this->gl_config.double_buffer;

    *iAttr++ = WGL_DEPTH_BITS_ARB;
    *iAttr++ = _this->gl_config.depth_size;

    if (_this->gl_config.stencil_size) {
        *iAttr++ = WGL_STENCIL_BITS_ARB;
        *iAttr++ = _this->gl_config.stencil_size;
    }

    if (_this->gl_config.accum_red_size) {
        *iAttr++ = WGL_ACCUM_RED_BITS_ARB;
        *iAttr++ = _this->gl_config.accum_red_size;
    }

    if (_this->gl_config.accum_green_size) {
        *iAttr++ = WGL_ACCUM_GREEN_BITS_ARB;
        *iAttr++ = _this->gl_config.accum_green_size;
    }

    if (_this->gl_config.accum_blue_size) {
        *iAttr++ = WGL_ACCUM_BLUE_BITS_ARB;
        *iAttr++ = _this->gl_config.accum_blue_size;
    }

    if (_this->gl_config.accum_alpha_size) {
        *iAttr++ = WGL_ACCUM_ALPHA_BITS_ARB;
        *iAttr++ = _this->gl_config.accum_alpha_size;
    }

    if (_this->gl_config.stereo) {
        *iAttr++ = WGL_STEREO_ARB;
        *iAttr++ = GL_TRUE;
    }

    if (_this->gl_config.multisamplebuffers) {
        *iAttr++ = WGL_SAMPLE_BUFFERS_ARB;
        *iAttr++ = _this->gl_config.multisamplebuffers;
    }

    if (_this->gl_config.multisamplesamples) {
        *iAttr++ = WGL_SAMPLES_ARB;
        *iAttr++ = _this->gl_config.multisamplesamples;
    }

    if (_this->gl_config.framebuffer_srgb_capable) {
        *iAttr++ = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
        *iAttr++ = _this->gl_config.framebuffer_srgb_capable;
    }

    /* We always choose either FULL or NO accel on Windows, because of flaky
       drivers. If the app didn't specify, we use FULL, because that's
       probably what they wanted (and if you didn't care and got FULL, that's
       a perfectly valid result in any case). */
    *iAttr++ = WGL_ACCELERATION_ARB;
    iAccelAttr = iAttr;
    if (_this->gl_config.accelerated) {
        *iAttr++ = WGL_FULL_ACCELERATION_ARB;
    } else {
        *iAttr++ = WGL_NO_ACCELERATION_ARB;
    }

    *iAttr = 0;

    /* Choose and set the closest available pixel format */
    pixel_format = WIN_GL_ChoosePixelFormatARB(_this, iAttribs, fAttribs);

    /* App said "don't care about accel" and FULL accel failed. Try NO. */
    if ( ( !pixel_format ) && ( _this->gl_config.accelerated < 0 ) ) {
        *iAccelAttr = WGL_NO_ACCELERATION_ARB;
        pixel_format = WIN_GL_ChoosePixelFormatARB(_this, iAttribs, fAttribs);
        *iAccelAttr = WGL_FULL_ACCELERATION_ARB;  /* if we try again. */
    }
    if (!pixel_format) {
        pixel_format = WIN_GL_ChoosePixelFormat(hdc, &pfd);
    }
    if (!pixel_format) {
        return SDL_SetError("No matching GL pixel format available");
    }
    if (!SetPixelFormat(hdc, pixel_format, &pfd)) {
        return WIN_SetError("SetPixelFormat()");
    }
    return 0;
}

int
WIN_GL_SetupWindow(_THIS, SDL_Window * window)
{
    /* The current context is lost in here; save it and reset it. */
    SDL_Window *current_win = SDL_GL_GetCurrentWindow();
    SDL_GLContext current_ctx = SDL_GL_GetCurrentContext();
    const int retval = WIN_GL_SetupWindowInternal(_this, window);
    WIN_GL_MakeCurrent(_this, current_win, current_ctx);
    return retval;
}

SDL_bool
WIN_GL_UseEGL(_THIS)
{
    SDL_assert(_this->gl_data != NULL);
    SDL_assert(_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES);

    return (SDL_GetHintBoolean(SDL_HINT_OPENGL_ES_DRIVER, SDL_FALSE)
            || _this->gl_config.major_version == 1 /* No WGL extension for OpenGL ES 1.x profiles. */
            || _this->gl_config.major_version > _this->gl_data->es_profile_max_supported_version.major
            || (_this->gl_config.major_version == _this->gl_data->es_profile_max_supported_version.major
                && _this->gl_config.minor_version > _this->gl_data->es_profile_max_supported_version.minor));
}

SDL_GLContext
WIN_GL_CreateContext(_THIS, SDL_Window * window)
{
    HDC hdc = ((SDL_WindowData *) window->driverdata)->hdc;
    HGLRC context, share_context;

    if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES && WIN_GL_UseEGL(_this)) {
#if SDL_VIDEO_OPENGL_EGL        
        /* Switch to EGL based functions */
        WIN_GL_UnloadLibrary(_this);
        _this->GL_LoadLibrary = WIN_GLES_LoadLibrary;
        _this->GL_GetProcAddress = WIN_GLES_GetProcAddress;
        _this->GL_UnloadLibrary = WIN_GLES_UnloadLibrary;
        _this->GL_CreateContext = WIN_GLES_CreateContext;
        _this->GL_MakeCurrent = WIN_GLES_MakeCurrent;
        _this->GL_SetSwapInterval = WIN_GLES_SetSwapInterval;
        _this->GL_GetSwapInterval = WIN_GLES_GetSwapInterval;
        _this->GL_SwapWindow = WIN_GLES_SwapWindow;
        _this->GL_DeleteContext = WIN_GLES_DeleteContext;
        
        if (WIN_GLES_LoadLibrary(_this, NULL) != 0) {
            return NULL;
        }
        
        return WIN_GLES_CreateContext(_this, window);
#else
        SDL_SetError("SDL not configured with EGL support");
        return NULL;
#endif
    }

    if (_this->gl_config.share_with_current_context) {
        share_context = (HGLRC)SDL_GL_GetCurrentContext();
    } else {
        share_context = 0;
    }

    if (_this->gl_config.major_version < 3 &&
        _this->gl_config.profile_mask == 0 &&
        _this->gl_config.flags == 0) {
        /* Create legacy context */
        context = _this->gl_data->wglCreateContext(hdc);
        if( share_context != 0 ) {
            _this->gl_data->wglShareLists(share_context, context);
        }
    } else {
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
        HGLRC temp_context = _this->gl_data->wglCreateContext(hdc);
        if (!temp_context) {
            SDL_SetError("Could not create GL context");
            return NULL;
        }

        /* Make the context current */
        if (WIN_GL_MakeCurrent(_this, window, temp_context) < 0) {
            WIN_GL_DeleteContext(_this, temp_context);
            return NULL;
        }

        wglCreateContextAttribsARB =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC) _this->gl_data->
            wglGetProcAddress("wglCreateContextAttribsARB");
        if (!wglCreateContextAttribsARB) {
            SDL_SetError("GL 3.x is not supported");
            context = temp_context;
        } else {
            int attribs[15];  /* max 14 attributes plus terminator */
            int iattr = 0;

            attribs[iattr++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
            attribs[iattr++] = _this->gl_config.major_version;
            attribs[iattr++] = WGL_CONTEXT_MINOR_VERSION_ARB;
            attribs[iattr++] = _this->gl_config.minor_version;

            /* SDL profile bits match WGL profile bits */
            if (_this->gl_config.profile_mask != 0) {
                attribs[iattr++] = WGL_CONTEXT_PROFILE_MASK_ARB;
                attribs[iattr++] = _this->gl_config.profile_mask;
            }

            /* SDL flags match WGL flags */
            if (_this->gl_config.flags != 0) {
                attribs[iattr++] = WGL_CONTEXT_FLAGS_ARB;
                attribs[iattr++] = _this->gl_config.flags;
            }

            /* only set if wgl extension is available */
            if (_this->gl_data->HAS_WGL_ARB_context_flush_control) {
                attribs[iattr++] = WGL_CONTEXT_RELEASE_BEHAVIOR_ARB;
                attribs[iattr++] = _this->gl_config.release_behavior ?
                                    WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB :
                                    WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB;
            }

            /* only set if wgl extension is available */
            if (_this->gl_data->HAS_WGL_ARB_create_context_robustness) {
                attribs[iattr++] = WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB;
                attribs[iattr++] = _this->gl_config.reset_notification ?
                                    WGL_LOSE_CONTEXT_ON_RESET_ARB :
                                    WGL_NO_RESET_NOTIFICATION_ARB;
            }

            /* only set if wgl extension is available */
            if (_this->gl_data->HAS_WGL_ARB_create_context_no_error) {
                attribs[iattr++] = WGL_CONTEXT_OPENGL_NO_ERROR_ARB;
                attribs[iattr++] = _this->gl_config.no_error;
            }

            attribs[iattr++] = 0;

            /* Create the GL 3.x context */
            context = wglCreateContextAttribsARB(hdc, share_context, attribs);
            /* Delete the GL 2.x context */
            _this->gl_data->wglDeleteContext(temp_context);
        }
    }

    if (!context) {
        WIN_SetError("Could not create GL context");
        return NULL;
    }

    if (WIN_GL_MakeCurrent(_this, window, context) < 0) {
        WIN_GL_DeleteContext(_this, context);
        return NULL;
    }

    return context;
}

int
WIN_GL_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context)
{
    HDC hdc;

    if (!_this->gl_data) {
        return SDL_SetError("OpenGL not initialized");
    }

    /* sanity check that higher level handled this. */
    SDL_assert(window || (!window && !context));

    /* Some Windows drivers freak out if hdc is NULL, even when context is
       NULL, against spec. Since hdc is _supposed_ to be ignored if context
       is NULL, we either use the current GL window, or do nothing if we
       already have no current context. */
    if (!window) {
        window = SDL_GL_GetCurrentWindow();
        if (!window) {
            SDL_assert(SDL_GL_GetCurrentContext() == NULL);
            return 0;  /* already done. */
        }
    }

    hdc = ((SDL_WindowData *) window->driverdata)->hdc;
    if (!_this->gl_data->wglMakeCurrent(hdc, (HGLRC) context)) {
        return WIN_SetError("wglMakeCurrent()");
    }
    return 0;
}

int
WIN_GL_SetSwapInterval(_THIS, int interval)
{
    if ((interval < 0) && (!_this->gl_data->HAS_WGL_EXT_swap_control_tear)) {
        return SDL_SetError("Negative swap interval unsupported in this GL");
    } else if (_this->gl_data->wglSwapIntervalEXT) {
        if (_this->gl_data->wglSwapIntervalEXT(interval) != TRUE) {
            return WIN_SetError("wglSwapIntervalEXT()");
        }
    } else {
        return SDL_Unsupported();
    }
    return 0;
}

int
WIN_GL_GetSwapInterval(_THIS)
{
    int retval = 0;
    if (_this->gl_data->wglGetSwapIntervalEXT) {
        retval = _this->gl_data->wglGetSwapIntervalEXT();
    }
    return retval;
}

int
WIN_GL_SwapWindow(_THIS, SDL_Window * window)
{
    HDC hdc = ((SDL_WindowData *) window->driverdata)->hdc;

    if (!SwapBuffers(hdc)) {
        return WIN_SetError("SwapBuffers()");
    }
    return 0;
}

void
WIN_GL_DeleteContext(_THIS, SDL_GLContext context)
{
    if (!_this->gl_data) {
        return;
    }
    _this->gl_data->wglDeleteContext((HGLRC) context);
}


SDL_bool
WIN_GL_SetPixelFormatFrom(_THIS, SDL_Window * fromWindow, SDL_Window * toWindow)
{
    HDC hfromdc = ((SDL_WindowData *) fromWindow->driverdata)->hdc;
    HDC htodc = ((SDL_WindowData *) toWindow->driverdata)->hdc;
    BOOL result;

    /* get the pixel format of the fromWindow */
    int pixel_format = GetPixelFormat(hfromdc);
    PIXELFORMATDESCRIPTOR pfd;
    SDL_memset(&pfd, 0, sizeof(pfd));
    DescribePixelFormat(hfromdc, pixel_format, sizeof(pfd), &pfd);

    /* set the pixel format of the toWindow */
    result = SetPixelFormat(htodc, pixel_format, &pfd);

    return result ? SDL_TRUE : SDL_FALSE;
}

#endif /* SDL_VIDEO_OPENGL_WGL */

#endif /* SDL_VIDEO_DRIVER_WINDOWS */

/* vi: set ts=4 sw=4 expandtab: */
