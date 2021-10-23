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
#include "../SDL_internal.h"

/* The high-level video driver subsystem */

#include "SDL.h"
#include "SDL_video.h"
#include "SDL_sysvideo.h"
#include "SDL_blit.h"
#include "SDL_pixels_c.h"
#include "SDL_rect_c.h"
#include "../events/SDL_events_c.h"
#include "../timer/SDL_timer_c.h"

#include "SDL_syswm.h"

#if SDL_VIDEO_OPENGL
#include "SDL_opengl.h"
#endif /* SDL_VIDEO_OPENGL */

#if SDL_VIDEO_OPENGL_ES && !SDL_VIDEO_OPENGL
#include "SDL_opengles.h"
#endif /* SDL_VIDEO_OPENGL_ES && !SDL_VIDEO_OPENGL */

/* GL and GLES2 headers conflict on Linux 32 bits */
#if SDL_VIDEO_OPENGL_ES2 && !SDL_VIDEO_OPENGL
#include "SDL_opengles2.h"
#endif /* SDL_VIDEO_OPENGL_ES2 && !SDL_VIDEO_OPENGL */

#if !SDL_VIDEO_OPENGL
#ifndef GL_CONTEXT_RELEASE_BEHAVIOR_KHR
#define GL_CONTEXT_RELEASE_BEHAVIOR_KHR 0x82FB
#endif
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* Available video drivers */
static VideoBootStrap *bootstrap[] = {
#if SDL_VIDEO_DRIVER_COCOA
    &COCOA_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_X11
    &X11_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_WAYLAND
    &Wayland_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_VIVANTE
    &VIVANTE_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_DIRECTFB
    &DirectFB_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_WINDOWS
    &WINDOWS_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_WINRT
    &WINRT_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_HAIKU
    &HAIKU_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_PANDORA
    &PND_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_UIKIT
    &UIKIT_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_ANDROID
    &Android_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_PSP
    &PSP_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_VITA
    &VITA_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_KMSDRM
    &KMSDRM_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_RPI
    &RPI_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_NACL
    &NACL_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_EMSCRIPTEN
    &Emscripten_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_QNX
    &QNX_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_OFFSCREEN
    &OFFSCREEN_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_OS2
    &OS2DIVE_bootstrap,
    &OS2VMAN_bootstrap,
#endif
#if SDL_VIDEO_DRIVER_DUMMY
    &DUMMY_bootstrap,
#endif
    NULL
};

static SDL_VideoDevice *_this = NULL;

#define CHECK_WINDOW_MAGIC(window, retval) \
    if (!_this) { \
        SDL_UninitializedVideo(); \
        return retval; \
    } \
    SDL_assert(window && window->magic == &_this->window_magic); \
    if (!window || window->magic != &_this->window_magic) { \
        SDL_SetError("Invalid window"); \
        return retval; \
    }

#define CHECK_DISPLAY_INDEX(displayIndex, retval) \
    if (!_this) { \
        SDL_UninitializedVideo(); \
        return retval; \
    } \
    SDL_assert(_this->displays != NULL); \
    SDL_assert(displayIndex >= 0 && displayIndex < _this->num_displays); \
    if (displayIndex < 0 || displayIndex >= _this->num_displays) { \
        SDL_SetError("displayIndex must be in the range 0 - %d", \
                     _this->num_displays - 1); \
        return retval; \
    }

#define FULLSCREEN_MASK (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN)

#ifdef __MACOSX__
/* Support for Mac OS X fullscreen spaces */
extern SDL_bool Cocoa_IsWindowInFullscreenSpace(SDL_Window * window);
extern SDL_bool Cocoa_SetWindowFullscreenSpace(SDL_Window * window, SDL_bool state);
#endif


/* Support for framebuffer emulation using an accelerated renderer */

#define SDL_WINDOWTEXTUREDATA   "_SDL_WindowTextureData"

typedef struct {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    void *pixels;
    int pitch;
    int bytes_per_pixel;
} SDL_WindowTextureData;

static SDL_bool
ShouldUseTextureFramebuffer()
{
    const char *hint;

    /* If there's no native framebuffer support then there's no option */
    if (!_this->CreateWindowFramebuffer) {
        return SDL_TRUE;
    }

    /* If this is the dummy driver there is no texture support */
    if (_this->is_dummy) {
        return SDL_FALSE;
    }

    /* If the user has specified a software renderer we can't use a
       texture framebuffer, or renderer creation will go recursive.
     */
    hint = SDL_GetHint(SDL_HINT_RENDER_DRIVER);
    if (hint && SDL_strcasecmp(hint, "software") == 0) {
        return SDL_FALSE;
    }

    /* See if the user or application wants a specific behavior */
    hint = SDL_GetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION);
    if (hint) {
        if (*hint == '0' || SDL_strcasecmp(hint, "false") == 0) {
            return SDL_FALSE;
        } else {
            return SDL_TRUE;
        }
    }

    /* Each platform has different performance characteristics */
#if defined(__WIN32__)
    /* GDI BitBlt() is way faster than Direct3D dynamic textures right now.
     */
    return SDL_FALSE;

#elif defined(__MACOSX__)
    /* Mac OS X uses OpenGL as the native fast path (for cocoa and X11) */
    return SDL_TRUE;

#elif defined(__LINUX__)
    /* Properly configured OpenGL drivers are faster than MIT-SHM */
#if SDL_VIDEO_OPENGL
    /* Ugh, find a way to cache this value! */
    {
        SDL_Window *window;
        SDL_GLContext context;
        SDL_bool hasAcceleratedOpenGL = SDL_FALSE;

        window = SDL_CreateWindow("OpenGL test", -32, -32, 32, 32, SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);
        if (window) {
            context = SDL_GL_CreateContext(window);
            if (context) {
                const GLubyte *(APIENTRY * glGetStringFunc) (GLenum);
                const char *vendor = NULL;

                glGetStringFunc = SDL_GL_GetProcAddress("glGetString");
                if (glGetStringFunc) {
                    vendor = (const char *) glGetStringFunc(GL_VENDOR);
                }
                /* Add more vendors here at will... */
                if (vendor &&
                    (SDL_strstr(vendor, "ATI Technologies") ||
                     SDL_strstr(vendor, "NVIDIA"))) {
                    hasAcceleratedOpenGL = SDL_TRUE;
                }
                SDL_GL_DeleteContext(context);
            }
            SDL_DestroyWindow(window);
        }
        return hasAcceleratedOpenGL;
    }
#elif SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2
    /* Let's be optimistic about this! */
    return SDL_TRUE;
#else
    return SDL_FALSE;
#endif

#else
    /* Play it safe, assume that if there is a framebuffer driver that it's
       optimized for the current platform.
    */
    return SDL_FALSE;
#endif
}

static int
SDL_CreateWindowTexture(SDL_VideoDevice *unused, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch)
{
    SDL_WindowTextureData *data;

    data = SDL_GetWindowData(window, SDL_WINDOWTEXTUREDATA);
    if (!data) {
        SDL_Renderer *renderer = NULL;
        int i;
        const char *hint = SDL_GetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION);

        /* Check to see if there's a specific driver requested */
        if (hint && *hint != '0' && *hint != '1' &&
            SDL_strcasecmp(hint, "true") != 0 &&
            SDL_strcasecmp(hint, "false") != 0 &&
            SDL_strcasecmp(hint, "software") != 0) {
            for (i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
                SDL_RendererInfo info;
                SDL_GetRenderDriverInfo(i, &info);
                if (SDL_strcasecmp(info.name, hint) == 0) {
                    renderer = SDL_CreateRenderer(window, i, 0);
                    break;
                }
            }
        }
        
        if (!renderer) {
            for (i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
                SDL_RendererInfo info;
                SDL_GetRenderDriverInfo(i, &info);
                if (SDL_strcmp(info.name, "software") != 0) {
                    renderer = SDL_CreateRenderer(window, i, 0);
                    if (renderer) {
                        break;
                    }
                }
            }
        }
        if (!renderer) {
            return SDL_SetError("No hardware accelerated renderers available");
        }

        /* Create the data after we successfully create the renderer (bug #1116) */
        data = (SDL_WindowTextureData *)SDL_calloc(1, sizeof(*data));
        if (!data) {
            SDL_DestroyRenderer(renderer);
            return SDL_OutOfMemory();
        }
        SDL_SetWindowData(window, SDL_WINDOWTEXTUREDATA, data);

        data->renderer = renderer;
    }

    /* Free any old texture and pixel data */
    if (data->texture) {
        SDL_DestroyTexture(data->texture);
        data->texture = NULL;
    }
    SDL_free(data->pixels);
    data->pixels = NULL;

    {
        SDL_RendererInfo info;
        Uint32 i;

        if (SDL_GetRendererInfo(data->renderer, &info) < 0) {
            return -1;
        }

        /* Find the first format without an alpha channel */
        *format = info.texture_formats[0];

        for (i = 0; i < info.num_texture_formats; ++i) {
            if (!SDL_ISPIXELFORMAT_FOURCC(info.texture_formats[i]) &&
                    !SDL_ISPIXELFORMAT_ALPHA(info.texture_formats[i])) {
                *format = info.texture_formats[i];
                break;
            }
        }
    }

    data->texture = SDL_CreateTexture(data->renderer, *format,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      window->w, window->h);
    if (!data->texture) {
        return -1;
    }

    /* Create framebuffer data */
    data->bytes_per_pixel = SDL_BYTESPERPIXEL(*format);
    data->pitch = (((window->w * data->bytes_per_pixel) + 3) & ~3);

    {
        /* Make static analysis happy about potential malloc(0) calls. */
        const size_t allocsize = window->h * data->pitch;
        data->pixels = SDL_malloc((allocsize > 0) ? allocsize : 1);
        if (!data->pixels) {
            return SDL_OutOfMemory();
        }
    }

    *pixels = data->pixels;
    *pitch = data->pitch;

    /* Make sure we're not double-scaling the viewport */
    SDL_RenderSetViewport(data->renderer, NULL);

    return 0;
}

static int
SDL_UpdateWindowTexture(SDL_VideoDevice *unused, SDL_Window * window, const SDL_Rect * rects, int numrects)
{
    SDL_WindowTextureData *data;
    SDL_Rect rect;
    void *src;

    data = SDL_GetWindowData(window, SDL_WINDOWTEXTUREDATA);
    if (!data || !data->texture) {
        return SDL_SetError("No window texture data");
    }

    /* Update a single rect that contains subrects for best DMA performance */
    if (SDL_GetSpanEnclosingRect(window->w, window->h, numrects, rects, &rect)) {
        src = (void *)((Uint8 *)data->pixels +
                        rect.y * data->pitch +
                        rect.x * data->bytes_per_pixel);
        if (SDL_UpdateTexture(data->texture, &rect, src, data->pitch) < 0) {
            return -1;
        }

        if (SDL_RenderCopy(data->renderer, data->texture, NULL, NULL) < 0) {
            return -1;
        }

        SDL_RenderPresent(data->renderer);
    }
    return 0;
}

static void
SDL_DestroyWindowTexture(SDL_VideoDevice *unused, SDL_Window * window)
{
    SDL_WindowTextureData *data;

    data = SDL_SetWindowData(window, SDL_WINDOWTEXTUREDATA, NULL);
    if (!data) {
        return;
    }
    if (data->texture) {
        SDL_DestroyTexture(data->texture);
    }
    if (data->renderer) {
        SDL_DestroyRenderer(data->renderer);
    }
    SDL_free(data->pixels);
    SDL_free(data);
}


static int
cmpmodes(const void *A, const void *B)
{
    const SDL_DisplayMode *a = (const SDL_DisplayMode *) A;
    const SDL_DisplayMode *b = (const SDL_DisplayMode *) B;
    if (a == b) {
        return 0;
    } else if (a->w != b->w) {
        return b->w - a->w;
    } else if (a->h != b->h) {
        return b->h - a->h;
    } else if (SDL_BITSPERPIXEL(a->format) != SDL_BITSPERPIXEL(b->format)) {
        return SDL_BITSPERPIXEL(b->format) - SDL_BITSPERPIXEL(a->format);
    } else if (SDL_PIXELLAYOUT(a->format) != SDL_PIXELLAYOUT(b->format)) {
        return SDL_PIXELLAYOUT(b->format) - SDL_PIXELLAYOUT(a->format);
    } else if (a->refresh_rate != b->refresh_rate) {
        return b->refresh_rate - a->refresh_rate;
    }
    return 0;
}

static int
SDL_UninitializedVideo()
{
    return SDL_SetError("Video subsystem has not been initialized");
}

int
SDL_GetNumVideoDrivers(void)
{
    return SDL_arraysize(bootstrap) - 1;
}

const char *
SDL_GetVideoDriver(int index)
{
    if (index >= 0 && index < SDL_GetNumVideoDrivers()) {
        return bootstrap[index]->name;
    }
    return NULL;
}

/*
 * Initialize the video and event subsystems -- determine native pixel format
 */
int
SDL_VideoInit(const char *driver_name)
{
    SDL_VideoDevice *video;
    int index;
    int i;

    /* Check to make sure we don't overwrite '_this' */
    if (_this != NULL) {
        SDL_VideoQuit();
    }

#if !SDL_TIMERS_DISABLED
    SDL_TicksInit();
#endif

    /* Start the event loop */
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0 ||
        SDL_KeyboardInit() < 0 ||
        SDL_MouseInit() < 0 ||
        SDL_TouchInit() < 0) {
        return -1;
    }

    /* Select the proper video driver */
    i = index = 0;
    video = NULL;
    if (driver_name == NULL) {
        driver_name = SDL_getenv("SDL_VIDEODRIVER");
    }
    if (driver_name != NULL) {
        const char *driver_attempt = driver_name;
        while(driver_attempt != NULL && *driver_attempt != 0 && video == NULL) {
            const char* driver_attempt_end = SDL_strchr(driver_attempt, ',');
            size_t driver_attempt_len = (driver_attempt_end != NULL) ? (driver_attempt_end - driver_attempt)
                                                                     : SDL_strlen(driver_attempt);

            for (i = 0; bootstrap[i]; ++i) {
                if ((driver_attempt_len == SDL_strlen(bootstrap[i]->name)) &&
                    (SDL_strncasecmp(bootstrap[i]->name, driver_attempt, driver_attempt_len) == 0)) {
                    video = bootstrap[i]->create(index);
                    break;
                }
            }

            driver_attempt = (driver_attempt_end != NULL) ? (driver_attempt_end + 1) : NULL;
        }
    } else {
        for (i = 0; bootstrap[i]; ++i) {
            video = bootstrap[i]->create(index);
            if (video != NULL) {
                break;
            }
        }
    }
    if (video == NULL) {
        if (driver_name) {
            return SDL_SetError("%s not available", driver_name);
        }
        return SDL_SetError("No available video device");
    }
    _this = video;
    _this->name = bootstrap[i]->name;
    _this->next_object_id = 1;


    /* Set some very sane GL defaults */
    _this->gl_config.driver_loaded = 0;
    _this->gl_config.dll_handle = NULL;
    SDL_GL_ResetAttributes();

    _this->current_glwin_tls = SDL_TLSCreate();
    _this->current_glctx_tls = SDL_TLSCreate();

    /* Initialize the video subsystem */
    if (_this->VideoInit(_this) < 0) {
        SDL_VideoQuit();
        return -1;
    }

    /* Make sure some displays were added */
    if (_this->num_displays == 0) {
        SDL_VideoQuit();
        return SDL_SetError("The video driver did not add any displays");
    }

    /* Add the renderer framebuffer emulation if desired */
    if (ShouldUseTextureFramebuffer()) {
        _this->CreateWindowFramebuffer = SDL_CreateWindowTexture;
        _this->UpdateWindowFramebuffer = SDL_UpdateWindowTexture;
        _this->DestroyWindowFramebuffer = SDL_DestroyWindowTexture;
    }

    /* Disable the screen saver by default. This is a change from <= 2.0.1,
       but most things using SDL are games or media players; you wouldn't
       want a screensaver to trigger if you're playing exclusively with a
       joystick, or passively watching a movie. Things that use SDL but
       function more like a normal desktop app should explicitly reenable the
       screensaver. */
    if (!SDL_GetHintBoolean(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, SDL_FALSE)) {
        SDL_DisableScreenSaver();
    }

    /* If we don't use a screen keyboard, turn on text input by default,
       otherwise programs that expect to get text events without enabling
       UNICODE input won't get any events.

       Actually, come to think of it, you needed to call SDL_EnableUNICODE(1)
       in SDL 1.2 before you got text input events.  Hmm...
     */
    if (!SDL_HasScreenKeyboardSupport()) {
        SDL_StartTextInput();
    }

    /* We're ready to go! */
    return 0;
}

const char *
SDL_GetCurrentVideoDriver()
{
    if (!_this) {
        SDL_UninitializedVideo();
        return NULL;
    }
    return _this->name;
}

SDL_VideoDevice *
SDL_GetVideoDevice(void)
{
    return _this;
}

int
SDL_AddBasicVideoDisplay(const SDL_DisplayMode * desktop_mode)
{
    SDL_VideoDisplay display;

    SDL_zero(display);
    if (desktop_mode) {
        display.desktop_mode = *desktop_mode;
    }
    display.current_mode = display.desktop_mode;

    return SDL_AddVideoDisplay(&display, SDL_FALSE);
}

int
SDL_AddVideoDisplay(const SDL_VideoDisplay * display, SDL_bool send_event)
{
    SDL_VideoDisplay *displays;
    int index = -1;

    displays =
        SDL_realloc(_this->displays,
                    (_this->num_displays + 1) * sizeof(*displays));
    if (displays) {
        index = _this->num_displays++;
        displays[index] = *display;
        displays[index].device = _this;
        _this->displays = displays;

        if (display->name) {
            displays[index].name = SDL_strdup(display->name);
        } else {
            char name[32];

            SDL_itoa(index, name, 10);
            displays[index].name = SDL_strdup(name);
        }

        if (send_event) {
            SDL_SendDisplayEvent(&_this->displays[index], SDL_DISPLAYEVENT_CONNECTED, 0);
        }
    } else {
        SDL_OutOfMemory();
    }
    return index;
}

void
SDL_DelVideoDisplay(int index)
{
    if (index < 0 || index >= _this->num_displays) {
        return;
    }

    SDL_SendDisplayEvent(&_this->displays[index], SDL_DISPLAYEVENT_DISCONNECTED, 0);

    if (index < (_this->num_displays - 1)) {
        SDL_memmove(&_this->displays[index], &_this->displays[index+1], (_this->num_displays - index - 1)*sizeof(_this->displays[index]));
    }
    --_this->num_displays;
}

int
SDL_GetNumVideoDisplays(void)
{
    if (!_this) {
        SDL_UninitializedVideo();
        return 0;
    }
    return _this->num_displays;
}

int
SDL_GetIndexOfDisplay(SDL_VideoDisplay *display)
{
    int displayIndex;

    for (displayIndex = 0; displayIndex < _this->num_displays; ++displayIndex) {
        if (display == &_this->displays[displayIndex]) {
            return displayIndex;
        }
    }

    /* Couldn't find the display, just use index 0 */
    return 0;
}

void *
SDL_GetDisplayDriverData(int displayIndex)
{
    CHECK_DISPLAY_INDEX(displayIndex, NULL);

    return _this->displays[displayIndex].driverdata;
}

SDL_bool
SDL_IsVideoContextExternal(void)
{
    return SDL_GetHintBoolean(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, SDL_FALSE);
}

const char *
SDL_GetDisplayName(int displayIndex)
{
    CHECK_DISPLAY_INDEX(displayIndex, NULL);

    return _this->displays[displayIndex].name;
}

int
SDL_GetDisplayBounds(int displayIndex, SDL_Rect * rect)
{
    CHECK_DISPLAY_INDEX(displayIndex, -1);

    if (rect) {
        SDL_VideoDisplay *display = &_this->displays[displayIndex];

        if (_this->GetDisplayBounds) {
            if (_this->GetDisplayBounds(_this, display, rect) == 0) {
                return 0;
            }
        }

        /* Assume that the displays are left to right */
        if (displayIndex == 0) {
            rect->x = 0;
            rect->y = 0;
        } else {
            SDL_GetDisplayBounds(displayIndex-1, rect);
            rect->x += rect->w;
        }
        rect->w = display->current_mode.w;
        rect->h = display->current_mode.h;
    }
    return 0;  /* !!! FIXME: should this be an error if (rect==NULL) ? */
}

static int
ParseDisplayUsableBoundsHint(SDL_Rect *rect)
{
    const char *hint = SDL_GetHint(SDL_HINT_DISPLAY_USABLE_BOUNDS);
    return hint && (SDL_sscanf(hint, "%d,%d,%d,%d", &rect->x, &rect->y, &rect->w, &rect->h) == 4);
}

int
SDL_GetDisplayUsableBounds(int displayIndex, SDL_Rect * rect)
{
    CHECK_DISPLAY_INDEX(displayIndex, -1);

    if (rect) {
        SDL_VideoDisplay *display = &_this->displays[displayIndex];

        if ((displayIndex == 0) && ParseDisplayUsableBoundsHint(rect)) {
            return 0;
        }

        if (_this->GetDisplayUsableBounds) {
            if (_this->GetDisplayUsableBounds(_this, display, rect) == 0) {
                return 0;
            }
        }

        /* Oh well, just give the entire display bounds. */
        return SDL_GetDisplayBounds(displayIndex, rect);
    }
    return 0;  /* !!! FIXME: should this be an error if (rect==NULL) ? */
}

int
SDL_GetDisplayDPI(int displayIndex, float * ddpi, float * hdpi, float * vdpi)
{
    SDL_VideoDisplay *display;

    CHECK_DISPLAY_INDEX(displayIndex, -1);

    display = &_this->displays[displayIndex];

    if (_this->GetDisplayDPI) {
        if (_this->GetDisplayDPI(_this, display, ddpi, hdpi, vdpi) == 0) {
            return 0;
        }
    } else {
        return SDL_Unsupported();
    }

    return -1;
}

SDL_DisplayOrientation
SDL_GetDisplayOrientation(int displayIndex)
{
    SDL_VideoDisplay *display;

    CHECK_DISPLAY_INDEX(displayIndex, SDL_ORIENTATION_UNKNOWN);

    display = &_this->displays[displayIndex];
    return display->orientation;
}

SDL_bool
SDL_AddDisplayMode(SDL_VideoDisplay * display,  const SDL_DisplayMode * mode)
{
    SDL_DisplayMode *modes;
    int i, nmodes;

    /* Make sure we don't already have the mode in the list */
    modes = display->display_modes;
    nmodes = display->num_display_modes;
    for (i = 0; i < nmodes; ++i) {
        if (cmpmodes(mode, &modes[i]) == 0) {
            return SDL_FALSE;
        }
    }

    /* Go ahead and add the new mode */
    if (nmodes == display->max_display_modes) {
        modes =
            SDL_realloc(modes,
                        (display->max_display_modes + 32) * sizeof(*modes));
        if (!modes) {
            return SDL_FALSE;
        }
        display->display_modes = modes;
        display->max_display_modes += 32;
    }
    modes[nmodes] = *mode;
    display->num_display_modes++;

    /* Re-sort video modes */
    SDL_qsort(display->display_modes, display->num_display_modes,
              sizeof(SDL_DisplayMode), cmpmodes);

    return SDL_TRUE;
}

static int
SDL_GetNumDisplayModesForDisplay(SDL_VideoDisplay * display)
{
    if (!display->num_display_modes && _this->GetDisplayModes) {
        _this->GetDisplayModes(_this, display);
        SDL_qsort(display->display_modes, display->num_display_modes,
                  sizeof(SDL_DisplayMode), cmpmodes);
    }
    return display->num_display_modes;
}

int
SDL_GetNumDisplayModes(int displayIndex)
{
    CHECK_DISPLAY_INDEX(displayIndex, -1);

    return SDL_GetNumDisplayModesForDisplay(&_this->displays[displayIndex]);
}

int
SDL_GetDisplayMode(int displayIndex, int index, SDL_DisplayMode * mode)
{
    SDL_VideoDisplay *display;

    CHECK_DISPLAY_INDEX(displayIndex, -1);

    display = &_this->displays[displayIndex];
    if (index < 0 || index >= SDL_GetNumDisplayModesForDisplay(display)) {
        return SDL_SetError("index must be in the range of 0 - %d",
                            SDL_GetNumDisplayModesForDisplay(display) - 1);
    }
    if (mode) {
        *mode = display->display_modes[index];
    }
    return 0;
}

int
SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode * mode)
{
    SDL_VideoDisplay *display;

    CHECK_DISPLAY_INDEX(displayIndex, -1);

    display = &_this->displays[displayIndex];
    if (mode) {
        *mode = display->desktop_mode;
    }
    return 0;
}

int
SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode * mode)
{
    SDL_VideoDisplay *display;

    CHECK_DISPLAY_INDEX(displayIndex, -1);

    display = &_this->displays[displayIndex];
    if (mode) {
        *mode = display->current_mode;
    }
    return 0;
}

static SDL_DisplayMode *
SDL_GetClosestDisplayModeForDisplay(SDL_VideoDisplay * display,
                                    const SDL_DisplayMode * mode,
                                    SDL_DisplayMode * closest)
{
    Uint32 target_format;
    int target_refresh_rate;
    int i;
    SDL_DisplayMode *current, *match;

    if (!mode || !closest) {
        SDL_SetError("Missing desired mode or closest mode parameter");
        return NULL;
    }

    /* Default to the desktop format */
    if (mode->format) {
        target_format = mode->format;
    } else {
        target_format = display->desktop_mode.format;
    }

    /* Default to the desktop refresh rate */
    if (mode->refresh_rate) {
        target_refresh_rate = mode->refresh_rate;
    } else {
        target_refresh_rate = display->desktop_mode.refresh_rate;
    }

    match = NULL;
    for (i = 0; i < SDL_GetNumDisplayModesForDisplay(display); ++i) {
        current = &display->display_modes[i];

        if (current->w && (current->w < mode->w)) {
            /* Out of sorted modes large enough here */
            break;
        }
        if (current->h && (current->h < mode->h)) {
            if (current->w && (current->w == mode->w)) {
                /* Out of sorted modes large enough here */
                break;
            }
            /* Wider, but not tall enough, due to a different
               aspect ratio. This mode must be skipped, but closer
               modes may still follow. */
            continue;
        }
        if (!match || current->w < match->w || current->h < match->h) {
            match = current;
            continue;
        }
        if (current->format != match->format) {
            /* Sorted highest depth to lowest */
            if (current->format == target_format ||
                (SDL_BITSPERPIXEL(current->format) >=
                 SDL_BITSPERPIXEL(target_format)
                 && SDL_PIXELTYPE(current->format) ==
                 SDL_PIXELTYPE(target_format))) {
                match = current;
            }
            continue;
        }
        if (current->refresh_rate != match->refresh_rate) {
            /* Sorted highest refresh to lowest */
            if (current->refresh_rate >= target_refresh_rate) {
                match = current;
            }
        }
    }
    if (match) {
        if (match->format) {
            closest->format = match->format;
        } else {
            closest->format = mode->format;
        }
        if (match->w && match->h) {
            closest->w = match->w;
            closest->h = match->h;
        } else {
            closest->w = mode->w;
            closest->h = mode->h;
        }
        if (match->refresh_rate) {
            closest->refresh_rate = match->refresh_rate;
        } else {
            closest->refresh_rate = mode->refresh_rate;
        }
        closest->driverdata = match->driverdata;

        /*
         * Pick some reasonable defaults if the app and driver don't
         * care
         */
        if (!closest->format) {
            closest->format = SDL_PIXELFORMAT_RGB888;
        }
        if (!closest->w) {
            closest->w = 640;
        }
        if (!closest->h) {
            closest->h = 480;
        }
        return closest;
    }
    return NULL;
}

SDL_DisplayMode *
SDL_GetClosestDisplayMode(int displayIndex,
                          const SDL_DisplayMode * mode,
                          SDL_DisplayMode * closest)
{
    SDL_VideoDisplay *display;

    CHECK_DISPLAY_INDEX(displayIndex, NULL);

    display = &_this->displays[displayIndex];
    return SDL_GetClosestDisplayModeForDisplay(display, mode, closest);
}

static int
SDL_SetDisplayModeForDisplay(SDL_VideoDisplay * display, const SDL_DisplayMode * mode)
{
    SDL_DisplayMode display_mode;
    SDL_DisplayMode current_mode;

    if (mode) {
        display_mode = *mode;

        /* Default to the current mode */
        if (!display_mode.format) {
            display_mode.format = display->current_mode.format;
        }
        if (!display_mode.w) {
            display_mode.w = display->current_mode.w;
        }
        if (!display_mode.h) {
            display_mode.h = display->current_mode.h;
        }
        if (!display_mode.refresh_rate) {
            display_mode.refresh_rate = display->current_mode.refresh_rate;
        }

        /* Get a good video mode, the closest one possible */
        if (!SDL_GetClosestDisplayModeForDisplay(display, &display_mode, &display_mode)) {
            return SDL_SetError("No video mode large enough for %dx%d",
                                display_mode.w, display_mode.h);
        }
    } else {
        display_mode = display->desktop_mode;
    }

    /* See if there's anything left to do */
    current_mode = display->current_mode;
    if (SDL_memcmp(&display_mode, &current_mode, sizeof(display_mode)) == 0) {
        return 0;
    }

    /* Actually change the display mode */
    if (!_this->SetDisplayMode) {
        return SDL_SetError("SDL video driver doesn't support changing display mode");
    }
    if (_this->SetDisplayMode(_this, display, &display_mode) < 0) {
        return -1;
    }
    display->current_mode = display_mode;
    return 0;
}

SDL_VideoDisplay *
SDL_GetDisplay(int displayIndex)
{
    CHECK_DISPLAY_INDEX(displayIndex, NULL);

    return &_this->displays[displayIndex];
}

int
SDL_GetWindowDisplayIndex(SDL_Window * window)
{
    int displayIndex;
    int i, dist;
    int closest = -1;
    int closest_dist = 0x7FFFFFFF;
    SDL_Point center;
    SDL_Point delta;
    SDL_Rect rect;

    CHECK_WINDOW_MAGIC(window, -1);

    if (SDL_WINDOWPOS_ISUNDEFINED(window->x) ||
        SDL_WINDOWPOS_ISCENTERED(window->x)) {
        displayIndex = (window->x & 0xFFFF);
        if (displayIndex >= _this->num_displays) {
            displayIndex = 0;
        }
        return displayIndex;
    }
    if (SDL_WINDOWPOS_ISUNDEFINED(window->y) ||
        SDL_WINDOWPOS_ISCENTERED(window->y)) {
        displayIndex = (window->y & 0xFFFF);
        if (displayIndex >= _this->num_displays) {
            displayIndex = 0;
        }
        return displayIndex;
    }

    /* Find the display containing the window */
    for (i = 0; i < _this->num_displays; ++i) {
        SDL_VideoDisplay *display = &_this->displays[i];

        if (display->fullscreen_window == window) {
            return i;
        }
    }
    center.x = window->x + window->w / 2;
    center.y = window->y + window->h / 2;
    for (i = 0; i < _this->num_displays; ++i) {
        SDL_GetDisplayBounds(i, &rect);
        if (SDL_EnclosePoints(&center, 1, &rect, NULL)) {
            return i;
        }

        delta.x = center.x - (rect.x + rect.w / 2);
        delta.y = center.y - (rect.y + rect.h / 2);
        dist = (delta.x*delta.x + delta.y*delta.y);
        if (dist < closest_dist) {
            closest = i;
            closest_dist = dist;
        }
    }
    if (closest < 0) {
        SDL_SetError("Couldn't find any displays");
    }
    return closest;
}

SDL_VideoDisplay *
SDL_GetDisplayForWindow(SDL_Window *window)
{
    int displayIndex = SDL_GetWindowDisplayIndex(window);
    if (displayIndex >= 0) {
        return &_this->displays[displayIndex];
    } else {
        return NULL;
    }
}

int
SDL_SetWindowDisplayMode(SDL_Window * window, const SDL_DisplayMode * mode)
{
    CHECK_WINDOW_MAGIC(window, -1);

    if (mode) {
        window->fullscreen_mode = *mode;
    } else {
        SDL_zero(window->fullscreen_mode);
    }

    if (FULLSCREEN_VISIBLE(window) && (window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_DisplayMode fullscreen_mode;
        if (SDL_GetWindowDisplayMode(window, &fullscreen_mode) == 0) {
            if (SDL_SetDisplayModeForDisplay(SDL_GetDisplayForWindow(window), &fullscreen_mode) == 0) {
#ifndef ANDROID
                /* Android may not resize the window to exactly what our fullscreen mode is, especially on
                 * windowed Android environments like the Chromebook or Samsung DeX.  Given this, we shouldn't
                 * use fullscreen_mode.w and fullscreen_mode.h, but rather get our current native size.  As such,
                 * Android's SetWindowFullscreen will generate the window event for us with the proper final size.
                 */
                SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESIZED, fullscreen_mode.w, fullscreen_mode.h);
#endif
            }
        }
    }
    return 0;
}

int
SDL_GetWindowDisplayMode(SDL_Window * window, SDL_DisplayMode * mode)
{
    SDL_DisplayMode fullscreen_mode;
    SDL_VideoDisplay *display;

    CHECK_WINDOW_MAGIC(window, -1);

    if (!mode) {
        return SDL_InvalidParamError("mode");
    }

    fullscreen_mode = window->fullscreen_mode;
    if (!fullscreen_mode.w) {
        fullscreen_mode.w = window->windowed.w;
    }
    if (!fullscreen_mode.h) {
        fullscreen_mode.h = window->windowed.h;
    }

    display = SDL_GetDisplayForWindow(window);

    /* if in desktop size mode, just return the size of the desktop */
    if ((window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP) {
        fullscreen_mode = display->desktop_mode;
    } else if (!SDL_GetClosestDisplayModeForDisplay(SDL_GetDisplayForWindow(window),
                                             &fullscreen_mode,
                                             &fullscreen_mode)) {
        return SDL_SetError("Couldn't find display mode match");
    }

    if (mode) {
        *mode = fullscreen_mode;
    }
    return 0;
}

Uint32
SDL_GetWindowPixelFormat(SDL_Window * window)
{
    SDL_VideoDisplay *display;

    CHECK_WINDOW_MAGIC(window, SDL_PIXELFORMAT_UNKNOWN);

    display = SDL_GetDisplayForWindow(window);
    return display->current_mode.format;
}

static void
SDL_RestoreMousePosition(SDL_Window *window)
{
    int x, y;

    if (window == SDL_GetMouseFocus()) {
        SDL_GetMouseState(&x, &y);
        SDL_WarpMouseInWindow(window, x, y);
    }
}

#if __WINRT__
extern Uint32 WINRT_DetectWindowFlags(SDL_Window * window);
#endif

static int
SDL_UpdateFullscreenMode(SDL_Window * window, SDL_bool fullscreen)
{
    SDL_VideoDisplay *display;
    SDL_Window *other;

    CHECK_WINDOW_MAGIC(window,-1);

    /* if we are in the process of hiding don't go back to fullscreen */
    if (window->is_hiding && fullscreen) {
        return 0;
    }

#ifdef __MACOSX__
    /* if the window is going away and no resolution change is necessary,
       do nothing, or else we may trigger an ugly double-transition
     */
    if (SDL_strcmp(_this->name, "cocoa") == 0) {  /* don't do this for X11, etc */
        if (window->is_destroying && (window->last_fullscreen_flags & FULLSCREEN_MASK) == SDL_WINDOW_FULLSCREEN_DESKTOP)
            return 0;
    
        /* If we're switching between a fullscreen Space and "normal" fullscreen, we need to get back to normal first. */
        if (fullscreen && ((window->last_fullscreen_flags & FULLSCREEN_MASK) == SDL_WINDOW_FULLSCREEN_DESKTOP) && ((window->flags & FULLSCREEN_MASK) == SDL_WINDOW_FULLSCREEN)) {
            if (!Cocoa_SetWindowFullscreenSpace(window, SDL_FALSE)) {
                return -1;
            }
        } else if (fullscreen && ((window->last_fullscreen_flags & FULLSCREEN_MASK) == SDL_WINDOW_FULLSCREEN) && ((window->flags & FULLSCREEN_MASK) == SDL_WINDOW_FULLSCREEN_DESKTOP)) {
            display = SDL_GetDisplayForWindow(window);
            SDL_SetDisplayModeForDisplay(display, NULL);
            if (_this->SetWindowFullscreen) {
                _this->SetWindowFullscreen(_this, window, display, SDL_FALSE);
            }
        }

        if (Cocoa_SetWindowFullscreenSpace(window, fullscreen)) {
            if (Cocoa_IsWindowInFullscreenSpace(window) != fullscreen) {
                return -1;
            }
            window->last_fullscreen_flags = window->flags;
            return 0;
        }
    }
#elif __WINRT__ && (NTDDI_VERSION < NTDDI_WIN10)
    /* HACK: WinRT 8.x apps can't choose whether or not they are fullscreen
       or not.  The user can choose this, via OS-provided UI, but this can't
       be set programmatically.

       Just look at what SDL's WinRT video backend code detected with regards
       to fullscreen (being active, or not), and figure out a return/error code
       from that.
    */
    if (fullscreen == !(WINRT_DetectWindowFlags(window) & FULLSCREEN_MASK)) {
        /* Uh oh, either:
            1. fullscreen was requested, and we're already windowed
            2. windowed-mode was requested, and we're already fullscreen

            WinRT 8.x can't resolve either programmatically, so we're
            giving up.
        */
        return -1;
    } else {
        /* Whatever was requested, fullscreen or windowed mode, is already
            in-place.
        */
        return 0;
    }
#endif

    display = SDL_GetDisplayForWindow(window);

    if (fullscreen) {
        /* Hide any other fullscreen windows */
        if (display->fullscreen_window &&
            display->fullscreen_window != window) {
            SDL_MinimizeWindow(display->fullscreen_window);
        }
    }

    /* See if anything needs to be done now */
    if ((display->fullscreen_window == window) == fullscreen) {
        if ((window->last_fullscreen_flags & FULLSCREEN_MASK) == (window->flags & FULLSCREEN_MASK)) {
            return 0;
        }
    }

    /* See if there are any fullscreen windows */
    for (other = _this->windows; other; other = other->next) {
        SDL_bool setDisplayMode = SDL_FALSE;

        if (other == window) {
            setDisplayMode = fullscreen;
        } else if (FULLSCREEN_VISIBLE(other) &&
                   SDL_GetDisplayForWindow(other) == display) {
            setDisplayMode = SDL_TRUE;
        }

        if (setDisplayMode) {
            SDL_DisplayMode fullscreen_mode;

            SDL_zero(fullscreen_mode);

            if (SDL_GetWindowDisplayMode(other, &fullscreen_mode) == 0) {
                SDL_bool resized = SDL_TRUE;

                if (other->w == fullscreen_mode.w && other->h == fullscreen_mode.h) {
                    resized = SDL_FALSE;
                }

                /* only do the mode change if we want exclusive fullscreen */
                if ((window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP) {
                    if (SDL_SetDisplayModeForDisplay(display, &fullscreen_mode) < 0) {
                        return -1;
                    }
                } else {
                    if (SDL_SetDisplayModeForDisplay(display, NULL) < 0) {
                        return -1;
                    }
                }

                if (_this->SetWindowFullscreen) {
                    _this->SetWindowFullscreen(_this, other, display, SDL_TRUE);
                }
                display->fullscreen_window = other;

                /* Generate a mode change event here */
                if (resized) {
#ifndef ANDROID
                    /* Android may not resize the window to exactly what our fullscreen mode is, especially on
                     * windowed Android environments like the Chromebook or Samsung DeX.  Given this, we shouldn't
                     * use fullscreen_mode.w and fullscreen_mode.h, but rather get our current native size.  As such,
                     * Android's SetWindowFullscreen will generate the window event for us with the proper final size.
                     */
                    SDL_SendWindowEvent(other, SDL_WINDOWEVENT_RESIZED,
                                        fullscreen_mode.w, fullscreen_mode.h);
#endif
                } else {
                    SDL_OnWindowResized(other);
                }

                SDL_RestoreMousePosition(other);

                window->last_fullscreen_flags = window->flags;
                return 0;
            }
        }
    }

    /* Nope, restore the desktop mode */
    SDL_SetDisplayModeForDisplay(display, NULL);

    if (_this->SetWindowFullscreen) {
        _this->SetWindowFullscreen(_this, window, display, SDL_FALSE);
    }
    display->fullscreen_window = NULL;

    /* Generate a mode change event here */
    SDL_OnWindowResized(window);

    /* Restore the cursor position */
    SDL_RestoreMousePosition(window);

    window->last_fullscreen_flags = window->flags;
    return 0;
}

#define CREATE_FLAGS \
    (SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_POPUP_MENU | SDL_WINDOW_UTILITY | SDL_WINDOW_TOOLTIP | SDL_WINDOW_VULKAN | SDL_WINDOW_MINIMIZED | SDL_WINDOW_METAL)

static SDL_INLINE SDL_bool
IsAcceptingDragAndDrop(void)
{
    if ((SDL_GetEventState(SDL_DROPFILE) == SDL_ENABLE) ||
        (SDL_GetEventState(SDL_DROPTEXT) == SDL_ENABLE)) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

/* prepare a newly-created window */
static SDL_INLINE void
PrepareDragAndDropSupport(SDL_Window *window)
{
    if (_this->AcceptDragAndDrop) {
        _this->AcceptDragAndDrop(window, IsAcceptingDragAndDrop());
    }
}

/* toggle d'n'd for all existing windows. */
void
SDL_ToggleDragAndDropSupport(void)
{
    if (_this && _this->AcceptDragAndDrop) {
        const SDL_bool enable = IsAcceptingDragAndDrop();
        SDL_Window *window;
        for (window = _this->windows; window; window = window->next) {
            _this->AcceptDragAndDrop(window, enable);
        }
    }
}

static void
SDL_FinishWindowCreation(SDL_Window *window, Uint32 flags)
{
    PrepareDragAndDropSupport(window);

    if (flags & SDL_WINDOW_MAXIMIZED) {
        SDL_MaximizeWindow(window);
    }
    if (flags & SDL_WINDOW_MINIMIZED) {
        SDL_MinimizeWindow(window);
    }
    if (flags & SDL_WINDOW_FULLSCREEN) {
        SDL_SetWindowFullscreen(window, flags);
    }
    if (flags & SDL_WINDOW_MOUSE_GRABBED) {
        /* We must specifically call SDL_SetWindowGrab() and not
           SDL_SetWindowMouseGrab() here because older applications may use
           this flag plus SDL_HINT_GRAB_KEYBOARD to indicate that they want
           the keyboard grabbed too and SDL_SetWindowMouseGrab() won't do that.
        */
        SDL_SetWindowGrab(window, SDL_TRUE);
    }
    if (flags & SDL_WINDOW_KEYBOARD_GRABBED) {
        SDL_SetWindowKeyboardGrab(window, SDL_TRUE);
    }
    if (!(flags & SDL_WINDOW_HIDDEN)) {
        SDL_ShowWindow(window);
    }
}

SDL_Window *
SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags)
{
    SDL_Window *window;
    Uint32 graphics_flags = flags & (SDL_WINDOW_OPENGL | SDL_WINDOW_METAL | SDL_WINDOW_VULKAN);

    if (!_this) {
        /* Initialize the video system if needed */
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            return NULL;
        }
    }

    if ((((flags & SDL_WINDOW_UTILITY) != 0) + ((flags & SDL_WINDOW_TOOLTIP) != 0) + ((flags & SDL_WINDOW_POPUP_MENU) != 0)) > 1) {
        SDL_SetError("Conflicting window flags specified");
        return NULL;
    }

    /* Some platforms can't create zero-sized windows */
    if (w < 1) {
        w = 1;
    }
    if (h < 1) {
        h = 1;
    }

    /* Some platforms blow up if the windows are too large. Raise it later? */
    if ((w > 16384) || (h > 16384)) {
        SDL_SetError("Window is too large.");
        return NULL;
    }

    /* Some platforms have certain graphics backends enabled by default */
    if (!_this->is_dummy && !graphics_flags && !SDL_IsVideoContextExternal()) {
#if (SDL_VIDEO_OPENGL && __MACOSX__) || (__IPHONEOS__ && !TARGET_OS_MACCATALYST) || __ANDROID__ || __NACL__
        flags |= SDL_WINDOW_OPENGL;
#endif
#if SDL_VIDEO_METAL && (TARGET_OS_MACCATALYST || __MACOSX__ || __IPHONEOS__)
        flags |= SDL_WINDOW_METAL;
#endif
    }

    if (flags & SDL_WINDOW_OPENGL) {
        if (!_this->GL_CreateContext) {
            SDL_SetError("OpenGL support is either not configured in SDL "
                         "or not available in current SDL video driver "
                         "(%s) or platform", _this->name);
            return NULL;
        }
        if (SDL_GL_LoadLibrary(NULL) < 0) {
            return NULL;
        }
    }

    if (flags & SDL_WINDOW_VULKAN) {
        if (!_this->Vulkan_CreateSurface) {
            SDL_SetError("Vulkan support is either not configured in SDL "
                         "or not available in current SDL video driver "
                         "(%s) or platform", _this->name);
            return NULL;
        }
        if (graphics_flags & SDL_WINDOW_OPENGL) {
            SDL_SetError("Vulkan and OpenGL not supported on same window");
            return NULL;
        }
        if (SDL_Vulkan_LoadLibrary(NULL) < 0) {
            return NULL;
        }
    }

    if (flags & SDL_WINDOW_METAL) {
        if (!_this->Metal_CreateView) {
            SDL_SetError("Metal support is either not configured in SDL "
                         "or not available in current SDL video driver "
                         "(%s) or platform", _this->name);
            return NULL;
        }
        /* 'flags' may have default flags appended, don't check against that. */
        if (graphics_flags & SDL_WINDOW_OPENGL) {
            SDL_SetError("Metal and OpenGL not supported on same window");
            return NULL;
        }
        if (graphics_flags & SDL_WINDOW_VULKAN) {
            SDL_SetError("Metal and Vulkan not supported on same window. "
                         "To use MoltenVK, set SDL_WINDOW_VULKAN only.");
            return NULL;
        }
    }

    /* Unless the user has specified the high-DPI disabling hint, respect the
     * SDL_WINDOW_ALLOW_HIGHDPI flag.
     */
    if (flags & SDL_WINDOW_ALLOW_HIGHDPI) {
        if (SDL_GetHintBoolean(SDL_HINT_VIDEO_HIGHDPI_DISABLED, SDL_FALSE)) {
            flags &= ~SDL_WINDOW_ALLOW_HIGHDPI;
        }
    }

    window = (SDL_Window *)SDL_calloc(1, sizeof(*window));
    if (!window) {
        SDL_OutOfMemory();
        return NULL;
    }
    window->magic = &_this->window_magic;
    window->id = _this->next_object_id++;
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
    if (SDL_WINDOWPOS_ISUNDEFINED(x) || SDL_WINDOWPOS_ISUNDEFINED(y) ||
        SDL_WINDOWPOS_ISCENTERED(x) || SDL_WINDOWPOS_ISCENTERED(y)) {
        SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
        int displayIndex;
        SDL_Rect bounds;

        displayIndex = SDL_GetIndexOfDisplay(display);
        SDL_GetDisplayBounds(displayIndex, &bounds);
        if (SDL_WINDOWPOS_ISUNDEFINED(x) || SDL_WINDOWPOS_ISCENTERED(x)) {
            window->x = bounds.x + (bounds.w - w) / 2;
        }
        if (SDL_WINDOWPOS_ISUNDEFINED(y) || SDL_WINDOWPOS_ISCENTERED(y)) {
            window->y = bounds.y + (bounds.h - h) / 2;
        }
    }
    window->windowed.x = window->x;
    window->windowed.y = window->y;
    window->windowed.w = window->w;
    window->windowed.h = window->h;

    if (flags & SDL_WINDOW_FULLSCREEN) {
        SDL_VideoDisplay *display = SDL_GetDisplayForWindow(window);
        int displayIndex;
        SDL_Rect bounds;

        displayIndex = SDL_GetIndexOfDisplay(display);
        SDL_GetDisplayBounds(displayIndex, &bounds);

        /* for real fullscreen we might switch the resolution, so get width and height
         * from closest supported mode and use that instead of current resolution
         */
        if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP
              && (bounds.w != w || bounds.h != h)) {
            SDL_DisplayMode fullscreen_mode, closest_mode;
            SDL_zero(fullscreen_mode);
            fullscreen_mode.w = w;
            fullscreen_mode.h = h;
            if (SDL_GetClosestDisplayModeForDisplay(display, &fullscreen_mode, &closest_mode) != NULL) {
                bounds.w = closest_mode.w;
                bounds.h = closest_mode.h;
            }
        }
        window->fullscreen_mode.w = bounds.w;
        window->fullscreen_mode.h = bounds.h;
        window->x = bounds.x;
        window->y = bounds.y;
        window->w = bounds.w;
        window->h = bounds.h;
    }

    window->flags = ((flags & CREATE_FLAGS) | SDL_WINDOW_HIDDEN);
    window->last_fullscreen_flags = window->flags;
    window->opacity = 1.0f;
    window->brightness = 1.0f;
    window->next = _this->windows;
    window->is_destroying = SDL_FALSE;

    if (_this->windows) {
        _this->windows->prev = window;
    }
    _this->windows = window;

    if (_this->CreateSDLWindow && _this->CreateSDLWindow(_this, window) < 0) {
        SDL_DestroyWindow(window);
        return NULL;
    }

    /* Clear minimized if not on windows, only windows handles it at create rather than FinishWindowCreation,
     * but it's important or window focus will get broken on windows!
     */
#if !defined(__WIN32__)
    if (window->flags & SDL_WINDOW_MINIMIZED) {
        window->flags &= ~SDL_WINDOW_MINIMIZED;
    }
#endif

#if __WINRT__ && (NTDDI_VERSION < NTDDI_WIN10)
    /* HACK: WinRT 8.x apps can't choose whether or not they are fullscreen
       or not.  The user can choose this, via OS-provided UI, but this can't
       be set programmatically.

       Just look at what SDL's WinRT video backend code detected with regards
       to fullscreen (being active, or not), and figure out a return/error code
       from that.
    */
    flags = window->flags;
#endif

    if (title) {
        SDL_SetWindowTitle(window, title);
    }
    SDL_FinishWindowCreation(window, flags);

    /* If the window was created fullscreen, make sure the mode code matches */
    SDL_UpdateFullscreenMode(window, FULLSCREEN_VISIBLE(window));

    return window;
}

SDL_Window *
SDL_CreateWindowFrom(const void *data)
{
    SDL_Window *window;

    if (!_this) {
        SDL_UninitializedVideo();
        return NULL;
    }
    if (!_this->CreateSDLWindowFrom) {
        SDL_Unsupported();
        return NULL;
    }
    window = (SDL_Window *)SDL_calloc(1, sizeof(*window));
    if (!window) {
        SDL_OutOfMemory();
        return NULL;
    }
    window->magic = &_this->window_magic;
    window->id = _this->next_object_id++;
    window->flags = SDL_WINDOW_FOREIGN;
    window->last_fullscreen_flags = window->flags;
    window->is_destroying = SDL_FALSE;
    window->opacity = 1.0f;
    window->brightness = 1.0f;
    window->next = _this->windows;
    if (_this->windows) {
        _this->windows->prev = window;
    }
    _this->windows = window;

    if (_this->CreateSDLWindowFrom(_this, window, data) < 0) {
        SDL_DestroyWindow(window);
        return NULL;
    }

    PrepareDragAndDropSupport(window);

    return window;
}

int
SDL_RecreateWindow(SDL_Window * window, Uint32 flags)
{
    SDL_bool loaded_opengl = SDL_FALSE;
    SDL_bool need_gl_unload = SDL_FALSE;
    SDL_bool need_gl_load = SDL_FALSE;
    SDL_bool loaded_vulkan = SDL_FALSE;
    SDL_bool need_vulkan_unload = SDL_FALSE;
    SDL_bool need_vulkan_load = SDL_FALSE;

    if ((flags & SDL_WINDOW_OPENGL) && !_this->GL_CreateContext) {
        return SDL_SetError("OpenGL support is either not configured in SDL "
                            "or not available in current SDL video driver "
                            "(%s) or platform", _this->name);
    }

    if (window->flags & SDL_WINDOW_FOREIGN) {
        /* Can't destroy and re-create foreign windows, hrm */
        flags |= SDL_WINDOW_FOREIGN;
    } else {
        flags &= ~SDL_WINDOW_FOREIGN;
    }

    /* Restore video mode, etc. */
    SDL_HideWindow(window);

    /* Tear down the old native window */
    if (window->surface) {
        window->surface->flags &= ~SDL_DONTFREE;
        SDL_FreeSurface(window->surface);
        window->surface = NULL;
        window->surface_valid = SDL_FALSE;
    }
    if (_this->DestroyWindowFramebuffer) {
        _this->DestroyWindowFramebuffer(_this, window);
    }
    if (_this->DestroyWindow && !(flags & SDL_WINDOW_FOREIGN)) {
        _this->DestroyWindow(_this, window);
    }

    if ((window->flags & SDL_WINDOW_OPENGL) != (flags & SDL_WINDOW_OPENGL)) {
        if (flags & SDL_WINDOW_OPENGL) {
            need_gl_load = SDL_TRUE;
        } else {
            need_gl_unload = SDL_TRUE;
        }
    } else if (window->flags & SDL_WINDOW_OPENGL) {
        need_gl_unload = SDL_TRUE;
        need_gl_load  = SDL_TRUE;
    }

    if ((window->flags & SDL_WINDOW_METAL) != (flags & SDL_WINDOW_METAL)) {
        if (flags & SDL_WINDOW_METAL) {
            need_gl_load = SDL_TRUE;
        } else {
            need_gl_unload = SDL_TRUE;
        }
    } else if (window->flags & SDL_WINDOW_METAL) {
        need_gl_unload = SDL_TRUE;
        need_gl_load  = SDL_TRUE;
    }

    if ((window->flags & SDL_WINDOW_VULKAN) != (flags & SDL_WINDOW_VULKAN)) {
        if (flags & SDL_WINDOW_VULKAN) {
            need_vulkan_load = SDL_TRUE;
        } else {
            need_vulkan_unload = SDL_TRUE;
        }
    } else if (window->flags & SDL_WINDOW_VULKAN) {
        need_vulkan_unload = SDL_TRUE;
        need_vulkan_load  = SDL_TRUE;
    }

    if ((flags & SDL_WINDOW_VULKAN) && (flags & SDL_WINDOW_OPENGL)) {
        SDL_SetError("Vulkan and OpenGL not supported on same window");
        return -1;
    }

    if ((flags & SDL_WINDOW_METAL) && (flags & SDL_WINDOW_OPENGL)) {
        SDL_SetError("Metal and OpenGL not supported on same window");
        return -1;
    }

    if ((flags & SDL_WINDOW_METAL) && (flags & SDL_WINDOW_VULKAN)) {
        SDL_SetError("Metal and Vulkan not supported on same window");
        return -1;
    }

    if (need_gl_unload) {
        SDL_GL_UnloadLibrary();
    }

    if (need_vulkan_unload) {
        SDL_Vulkan_UnloadLibrary();
    }

    if (need_gl_load) {
        if (SDL_GL_LoadLibrary(NULL) < 0) {
            return -1;
        }
        loaded_opengl = SDL_TRUE;
    }

    if (need_vulkan_load) {
        if (SDL_Vulkan_LoadLibrary(NULL) < 0) {
            return -1;
        }
        loaded_vulkan = SDL_TRUE;
    }

    window->flags = ((flags & CREATE_FLAGS) | SDL_WINDOW_HIDDEN);
    window->last_fullscreen_flags = window->flags;
    window->is_destroying = SDL_FALSE;

    if (_this->CreateSDLWindow && !(flags & SDL_WINDOW_FOREIGN)) {
        if (_this->CreateSDLWindow(_this, window) < 0) {
            if (loaded_opengl) {
                SDL_GL_UnloadLibrary();
                window->flags &= ~SDL_WINDOW_OPENGL;
            }
            if (loaded_vulkan) {
                SDL_Vulkan_UnloadLibrary();
                window->flags &= ~SDL_WINDOW_VULKAN;
            }
            return -1;
        }
    }

    if (flags & SDL_WINDOW_FOREIGN) {
        window->flags |= SDL_WINDOW_FOREIGN;
    }

    if (_this->SetWindowTitle && window->title) {
        _this->SetWindowTitle(_this, window);
    }

    if (_this->SetWindowIcon && window->icon) {
        _this->SetWindowIcon(_this, window, window->icon);
    }

    if (window->hit_test) {
        _this->SetWindowHitTest(window, SDL_TRUE);
    }

    SDL_FinishWindowCreation(window, flags);

    return 0;
}

SDL_bool
SDL_HasWindows(void)
{
    return (_this && _this->windows != NULL);
}

Uint32
SDL_GetWindowID(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, 0);

    return window->id;
}

SDL_Window *
SDL_GetWindowFromID(Uint32 id)
{
    SDL_Window *window;

    if (!_this) {
        return NULL;
    }
    for (window = _this->windows; window; window = window->next) {
        if (window->id == id) {
            return window;
        }
    }
    return NULL;
}

Uint32
SDL_GetWindowFlags(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, 0);

    return window->flags;
}

void
SDL_SetWindowTitle(SDL_Window * window, const char *title)
{
    CHECK_WINDOW_MAGIC(window,);

    if (title == window->title) {
        return;
    }
    SDL_free(window->title);

    window->title = SDL_strdup(title ? title : "");

    if (_this->SetWindowTitle) {
        _this->SetWindowTitle(_this, window);
    }
}

const char *
SDL_GetWindowTitle(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, "");

    return window->title ? window->title : "";
}

void
SDL_SetWindowIcon(SDL_Window * window, SDL_Surface * icon)
{
    CHECK_WINDOW_MAGIC(window,);

    if (!icon) {
        return;
    }

    SDL_FreeSurface(window->icon);

    /* Convert the icon into ARGB8888 */
    window->icon = SDL_ConvertSurfaceFormat(icon, SDL_PIXELFORMAT_ARGB8888, 0);
    if (!window->icon) {
        return;
    }

    if (_this->SetWindowIcon) {
        _this->SetWindowIcon(_this, window, window->icon);
    }
}

void*
SDL_SetWindowData(SDL_Window * window, const char *name, void *userdata)
{
    SDL_WindowUserData *prev, *data;

    CHECK_WINDOW_MAGIC(window, NULL);

    /* Input validation */
    if (name == NULL || name[0] == '\0') {
      SDL_InvalidParamError("name");
      return NULL;
    }

    /* See if the named data already exists */
    prev = NULL;
    for (data = window->data; data; prev = data, data = data->next) {
        if (data->name && SDL_strcmp(data->name, name) == 0) {
            void *last_value = data->data;

            if (userdata) {
                /* Set the new value */
                data->data = userdata;
            } else {
                /* Delete this value */
                if (prev) {
                    prev->next = data->next;
                } else {
                    window->data = data->next;
                }
                SDL_free(data->name);
                SDL_free(data);
            }
            return last_value;
        }
    }

    /* Add new data to the window */
    if (userdata) {
        data = (SDL_WindowUserData *)SDL_malloc(sizeof(*data));
        data->name = SDL_strdup(name);
        data->data = userdata;
        data->next = window->data;
        window->data = data;
    }
    return NULL;
}

void *
SDL_GetWindowData(SDL_Window * window, const char *name)
{
    SDL_WindowUserData *data;

    CHECK_WINDOW_MAGIC(window, NULL);

    /* Input validation */
    if (name == NULL || name[0] == '\0') {
      SDL_InvalidParamError("name");
      return NULL;
    }

    for (data = window->data; data; data = data->next) {
        if (data->name && SDL_strcmp(data->name, name) == 0) {
            return data->data;
        }
    }
    return NULL;
}

void
SDL_SetWindowPosition(SDL_Window * window, int x, int y)
{
    CHECK_WINDOW_MAGIC(window,);

    if (SDL_WINDOWPOS_ISCENTERED(x) || SDL_WINDOWPOS_ISCENTERED(y)) {
        int displayIndex = (x & 0xFFFF);
        SDL_Rect bounds;
        if (displayIndex >= _this->num_displays) {
            displayIndex = 0;
        }

        SDL_zero(bounds);

        SDL_GetDisplayBounds(displayIndex, &bounds);
        if (SDL_WINDOWPOS_ISCENTERED(x)) {
            x = bounds.x + (bounds.w - window->w) / 2;
        }
        if (SDL_WINDOWPOS_ISCENTERED(y)) {
            y = bounds.y + (bounds.h - window->h) / 2;
        }
    }

    if ((window->flags & SDL_WINDOW_FULLSCREEN)) {
        if (!SDL_WINDOWPOS_ISUNDEFINED(x)) {
            window->windowed.x = x;
        }
        if (!SDL_WINDOWPOS_ISUNDEFINED(y)) {
            window->windowed.y = y;
        }
    } else {
        if (!SDL_WINDOWPOS_ISUNDEFINED(x)) {
            window->x = x;
        }
        if (!SDL_WINDOWPOS_ISUNDEFINED(y)) {
            window->y = y;
        }

        if (_this->SetWindowPosition) {
            _this->SetWindowPosition(_this, window);
        }
    }
}

void
SDL_GetWindowPosition(SDL_Window * window, int *x, int *y)
{
    CHECK_WINDOW_MAGIC(window,);

    /* Fullscreen windows are always at their display's origin */
    if (window->flags & SDL_WINDOW_FULLSCREEN) {
        int displayIndex;
        
        if (x) {
            *x = 0;
        }
        if (y) {
            *y = 0;
        }

        /* Find the window's monitor and update to the
           monitor offset. */
        displayIndex = SDL_GetWindowDisplayIndex(window);
        if (displayIndex >= 0) {
            SDL_Rect bounds;

            SDL_zero(bounds);

            SDL_GetDisplayBounds(displayIndex, &bounds);
            if (x) {
                *x = bounds.x;
            }
            if (y) {
                *y = bounds.y;
            }
        }
    } else {
        if (x) {
            *x = window->x;
        }
        if (y) {
            *y = window->y;
        }
    }
}

void
SDL_SetWindowBordered(SDL_Window * window, SDL_bool bordered)
{
    CHECK_WINDOW_MAGIC(window,);
    if (!(window->flags & SDL_WINDOW_FULLSCREEN)) {
        const int want = (bordered != SDL_FALSE);  /* normalize the flag. */
        const int have = ((window->flags & SDL_WINDOW_BORDERLESS) == 0);
        if ((want != have) && (_this->SetWindowBordered)) {
            if (want) {
                window->flags &= ~SDL_WINDOW_BORDERLESS;
            } else {
                window->flags |= SDL_WINDOW_BORDERLESS;
            }
            _this->SetWindowBordered(_this, window, (SDL_bool) want);
        }
    }
}

void
SDL_SetWindowResizable(SDL_Window * window, SDL_bool resizable)
{
    CHECK_WINDOW_MAGIC(window,);
    if (!(window->flags & SDL_WINDOW_FULLSCREEN)) {
        const int want = (resizable != SDL_FALSE);  /* normalize the flag. */
        const int have = ((window->flags & SDL_WINDOW_RESIZABLE) != 0);
        if ((want != have) && (_this->SetWindowResizable)) {
            if (want) {
                window->flags |= SDL_WINDOW_RESIZABLE;
            } else {
                window->flags &= ~SDL_WINDOW_RESIZABLE;
            }
            _this->SetWindowResizable(_this, window, (SDL_bool) want);
        }
    }
}

void
SDL_SetWindowAlwaysOnTop(SDL_Window * window, SDL_bool on_top)
{
    CHECK_WINDOW_MAGIC(window,);
    if (!(window->flags & SDL_WINDOW_FULLSCREEN)) {
        const int want = (on_top != SDL_FALSE);  /* normalize the flag. */
        const int have = ((window->flags & SDL_WINDOW_ALWAYS_ON_TOP) != 0);
        if ((want != have) && (_this->SetWindowAlwaysOnTop)) {
            if (want) {
                window->flags |= SDL_WINDOW_ALWAYS_ON_TOP;
            } else {
                window->flags &= ~SDL_WINDOW_ALWAYS_ON_TOP;
            }
            
            _this->SetWindowAlwaysOnTop(_this, window, (SDL_bool) want);
        }
    }
}

void
SDL_SetWindowSize(SDL_Window * window, int w, int h)
{
    CHECK_WINDOW_MAGIC(window,);
    if (w <= 0) {
        SDL_InvalidParamError("w");
        return;
    }
    if (h <= 0) {
        SDL_InvalidParamError("h");
        return;
    }

    /* Make sure we don't exceed any window size limits */
    if (window->min_w && w < window->min_w) {
        w = window->min_w;
    }
    if (window->max_w && w > window->max_w) {
        w = window->max_w;
    }
    if (window->min_h && h < window->min_h) {
        h = window->min_h;
    }
    if (window->max_h && h > window->max_h) {
        h = window->max_h;
    }

    window->windowed.w = w;
    window->windowed.h = h;

    if (window->flags & SDL_WINDOW_FULLSCREEN) {
        if (FULLSCREEN_VISIBLE(window) && (window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP) {
            window->last_fullscreen_flags = 0;
            SDL_UpdateFullscreenMode(window, SDL_TRUE);
        }
    } else {
        window->w = w;
        window->h = h;
        if (_this->SetWindowSize) {
            _this->SetWindowSize(_this, window);
        }
        if (window->w == w && window->h == h) {
            /* We didn't get a SDL_WINDOWEVENT_RESIZED event (by design) */
            SDL_OnWindowResized(window);
        }
    }
}

void
SDL_GetWindowSize(SDL_Window * window, int *w, int *h)
{
    CHECK_WINDOW_MAGIC(window,);
    if (w) {
        *w = window->w;
    }
    if (h) {
        *h = window->h;
    }
}

int
SDL_GetWindowBordersSize(SDL_Window * window, int *top, int *left, int *bottom, int *right)
{
    int dummy = 0;

    if (!top) { top = &dummy; }
    if (!left) { left = &dummy; }
    if (!right) { right = &dummy; }
    if (!bottom) { bottom = &dummy; }

    /* Always initialize, so applications don't have to care */
    *top = *left = *bottom = *right = 0;

    CHECK_WINDOW_MAGIC(window, -1);

    if (!_this->GetWindowBordersSize) {
        return SDL_Unsupported();
    }

    return _this->GetWindowBordersSize(_this, window, top, left, bottom, right);
}

void
SDL_SetWindowMinimumSize(SDL_Window * window, int min_w, int min_h)
{
    CHECK_WINDOW_MAGIC(window,);
    if (min_w <= 0) {
        SDL_InvalidParamError("min_w");
        return;
    }
    if (min_h <= 0) {
        SDL_InvalidParamError("min_h");
        return;
    }

    if ((window->max_w && min_w > window->max_w) ||
        (window->max_h && min_h > window->max_h)) {
        SDL_SetError("SDL_SetWindowMinimumSize(): Tried to set minimum size larger than maximum size");
        return;
    }

    window->min_w = min_w;
    window->min_h = min_h;

    if (!(window->flags & SDL_WINDOW_FULLSCREEN)) {
        if (_this->SetWindowMinimumSize) {
            _this->SetWindowMinimumSize(_this, window);
        }
        /* Ensure that window is not smaller than minimal size */
        SDL_SetWindowSize(window, SDL_max(window->w, window->min_w), SDL_max(window->h, window->min_h));
    }
}

void
SDL_GetWindowMinimumSize(SDL_Window * window, int *min_w, int *min_h)
{
    CHECK_WINDOW_MAGIC(window,);
    if (min_w) {
        *min_w = window->min_w;
    }
    if (min_h) {
        *min_h = window->min_h;
    }
}

void
SDL_SetWindowMaximumSize(SDL_Window * window, int max_w, int max_h)
{
    CHECK_WINDOW_MAGIC(window,);
    if (max_w <= 0) {
        SDL_InvalidParamError("max_w");
        return;
    }
    if (max_h <= 0) {
        SDL_InvalidParamError("max_h");
        return;
    }

    if (max_w < window->min_w || max_h < window->min_h) {
        SDL_SetError("SDL_SetWindowMaximumSize(): Tried to set maximum size smaller than minimum size");
        return;
    }

    window->max_w = max_w;
    window->max_h = max_h;

    if (!(window->flags & SDL_WINDOW_FULLSCREEN)) {
        if (_this->SetWindowMaximumSize) {
            _this->SetWindowMaximumSize(_this, window);
        }
        /* Ensure that window is not larger than maximal size */
        SDL_SetWindowSize(window, SDL_min(window->w, window->max_w), SDL_min(window->h, window->max_h));
    }
}

void
SDL_GetWindowMaximumSize(SDL_Window * window, int *max_w, int *max_h)
{
    CHECK_WINDOW_MAGIC(window,);
    if (max_w) {
        *max_w = window->max_w;
    }
    if (max_h) {
        *max_h = window->max_h;
    }
}

void
SDL_ShowWindow(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window,);

    if (window->flags & SDL_WINDOW_SHOWN) {
        return;
    }

    if (_this->ShowWindow) {
        _this->ShowWindow(_this, window);
    }
    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_SHOWN, 0, 0);
}

void
SDL_HideWindow(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window,);

    if (!(window->flags & SDL_WINDOW_SHOWN)) {
        return;
    }

    window->is_hiding = SDL_TRUE;
    SDL_UpdateFullscreenMode(window, SDL_FALSE);

    if (_this->HideWindow) {
        _this->HideWindow(_this, window);
    }
    window->is_hiding = SDL_FALSE;
    SDL_SendWindowEvent(window, SDL_WINDOWEVENT_HIDDEN, 0, 0);
}

void
SDL_RaiseWindow(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window,);

    if (!(window->flags & SDL_WINDOW_SHOWN)) {
        return;
    }
    if (_this->RaiseWindow) {
        _this->RaiseWindow(_this, window);
    }
}

void
SDL_MaximizeWindow(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window,);

    if (window->flags & SDL_WINDOW_MAXIMIZED) {
        return;
    }

    /* !!! FIXME: should this check if the window is resizable? */

    if (_this->MaximizeWindow) {
        _this->MaximizeWindow(_this, window);
    }
}

static SDL_bool
CanMinimizeWindow(SDL_Window * window)
{
    if (!_this->MinimizeWindow) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

void
SDL_MinimizeWindow(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window,);

    if (window->flags & SDL_WINDOW_MINIMIZED) {
        return;
    }

    if (!CanMinimizeWindow(window)) {
        return;
    }

    SDL_UpdateFullscreenMode(window, SDL_FALSE);

    if (_this->MinimizeWindow) {
        _this->MinimizeWindow(_this, window);
    }
}

void
SDL_RestoreWindow(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window,);

    if (!(window->flags & (SDL_WINDOW_MAXIMIZED | SDL_WINDOW_MINIMIZED))) {
        return;
    }

    if (_this->RestoreWindow) {
        _this->RestoreWindow(_this, window);
    }
}

int
SDL_SetWindowFullscreen(SDL_Window * window, Uint32 flags)
{
    Uint32 oldflags;
    CHECK_WINDOW_MAGIC(window, -1);

    flags &= FULLSCREEN_MASK;

    if (flags == (window->flags & FULLSCREEN_MASK)) {
        return 0;
    }

    /* clear the previous flags and OR in the new ones */
    oldflags = window->flags & FULLSCREEN_MASK;
    window->flags &= ~FULLSCREEN_MASK;
    window->flags |= flags;

    if (SDL_UpdateFullscreenMode(window, FULLSCREEN_VISIBLE(window)) == 0) {
        return 0;
    }
    
    window->flags &= ~FULLSCREEN_MASK;
    window->flags |= oldflags;
    return -1;
}

static SDL_Surface *
SDL_CreateWindowFramebuffer(SDL_Window * window)
{
    Uint32 format;
    void *pixels;
    int pitch;
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;

    if (!_this->CreateWindowFramebuffer || !_this->UpdateWindowFramebuffer) {
        return NULL;
    }

    if (_this->CreateWindowFramebuffer(_this, window, &format, &pixels, &pitch) < 0) {
        return NULL;
    }

    if (window->surface) {
        return window->surface;
    }

    if (!SDL_PixelFormatEnumToMasks(format, &bpp, &Rmask, &Gmask, &Bmask, &Amask)) {
        return NULL;
    }

    return SDL_CreateRGBSurfaceFrom(pixels, window->w, window->h, bpp, pitch, Rmask, Gmask, Bmask, Amask);
}

SDL_Surface *
SDL_GetWindowSurface(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, NULL);

    if (!window->surface_valid) {
        if (window->surface) {
            window->surface->flags &= ~SDL_DONTFREE;
            SDL_FreeSurface(window->surface);
            window->surface = NULL;
        }
        window->surface = SDL_CreateWindowFramebuffer(window);
        if (window->surface) {
            window->surface_valid = SDL_TRUE;
            window->surface->flags |= SDL_DONTFREE;
        }
    }
    return window->surface;
}

int
SDL_UpdateWindowSurface(SDL_Window * window)
{
    SDL_Rect full_rect;

    CHECK_WINDOW_MAGIC(window, -1);

    full_rect.x = 0;
    full_rect.y = 0;
    full_rect.w = window->w;
    full_rect.h = window->h;
    return SDL_UpdateWindowSurfaceRects(window, &full_rect, 1);
}

int
SDL_UpdateWindowSurfaceRects(SDL_Window * window, const SDL_Rect * rects,
                             int numrects)
{
    CHECK_WINDOW_MAGIC(window, -1);

    if (!window->surface_valid) {
        return SDL_SetError("Window surface is invalid, please call SDL_GetWindowSurface() to get a new surface");
    }

    return _this->UpdateWindowFramebuffer(_this, window, rects, numrects);
}

int
SDL_SetWindowBrightness(SDL_Window * window, float brightness)
{
    Uint16 ramp[256];
    int status;

    CHECK_WINDOW_MAGIC(window, -1);

    SDL_CalculateGammaRamp(brightness, ramp);
    status = SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
    if (status == 0) {
        window->brightness = brightness;
    }
    return status;
}

float
SDL_GetWindowBrightness(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, 1.0f);

    return window->brightness;
}

int
SDL_SetWindowOpacity(SDL_Window * window, float opacity)
{
    int retval;
    CHECK_WINDOW_MAGIC(window, -1);

    if (!_this->SetWindowOpacity) {
        return SDL_Unsupported();
    }

    if (opacity < 0.0f) {
        opacity = 0.0f;
    } else if (opacity > 1.0f) {
        opacity = 1.0f;
    }

    retval = _this->SetWindowOpacity(_this, window, opacity);
    if (retval == 0) {
        window->opacity = opacity;
    }

    return retval;
}

int
SDL_GetWindowOpacity(SDL_Window * window, float * out_opacity)
{
    CHECK_WINDOW_MAGIC(window, -1);

    if (out_opacity) {
        *out_opacity = window->opacity;
    }

    return 0;
}

int
SDL_SetWindowModalFor(SDL_Window * modal_window, SDL_Window * parent_window)
{
    CHECK_WINDOW_MAGIC(modal_window, -1);
    CHECK_WINDOW_MAGIC(parent_window, -1);

    if (!_this->SetWindowModalFor) {
        return SDL_Unsupported();
    }
    
    return _this->SetWindowModalFor(_this, modal_window, parent_window);
}

int 
SDL_SetWindowInputFocus(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, -1);

    if (!_this->SetWindowInputFocus) {
        return SDL_Unsupported();
    }
    
    return _this->SetWindowInputFocus(_this, window);
}


int
SDL_SetWindowGammaRamp(SDL_Window * window, const Uint16 * red,
                                            const Uint16 * green,
                                            const Uint16 * blue)
{
    CHECK_WINDOW_MAGIC(window, -1);

    if (!_this->SetWindowGammaRamp) {
        return SDL_Unsupported();
    }

    if (!window->gamma) {
        if (SDL_GetWindowGammaRamp(window, NULL, NULL, NULL) < 0) {
            return -1;
        }
        SDL_assert(window->gamma != NULL);
    }

    if (red) {
        SDL_memcpy(&window->gamma[0*256], red, 256*sizeof(Uint16));
    }
    if (green) {
        SDL_memcpy(&window->gamma[1*256], green, 256*sizeof(Uint16));
    }
    if (blue) {
        SDL_memcpy(&window->gamma[2*256], blue, 256*sizeof(Uint16));
    }
    if (window->flags & SDL_WINDOW_INPUT_FOCUS) {
        return _this->SetWindowGammaRamp(_this, window, window->gamma);
    } else {
        return 0;
    }
}

int
SDL_GetWindowGammaRamp(SDL_Window * window, Uint16 * red,
                                            Uint16 * green,
                                            Uint16 * blue)
{
    CHECK_WINDOW_MAGIC(window, -1);

    if (!window->gamma) {
        int i;

        window->gamma = (Uint16 *)SDL_malloc(256*6*sizeof(Uint16));
        if (!window->gamma) {
            return SDL_OutOfMemory();
        }
        window->saved_gamma = window->gamma + 3*256;

        if (_this->GetWindowGammaRamp) {
            if (_this->GetWindowGammaRamp(_this, window, window->gamma) < 0) {
                return -1;
            }
        } else {
            /* Create an identity gamma ramp */
            for (i = 0; i < 256; ++i) {
                Uint16 value = (Uint16)((i << 8) | i);

                window->gamma[0*256+i] = value;
                window->gamma[1*256+i] = value;
                window->gamma[2*256+i] = value;
            }
        }
        SDL_memcpy(window->saved_gamma, window->gamma, 3*256*sizeof(Uint16));
    }

    if (red) {
        SDL_memcpy(red, &window->gamma[0*256], 256*sizeof(Uint16));
    }
    if (green) {
        SDL_memcpy(green, &window->gamma[1*256], 256*sizeof(Uint16));
    }
    if (blue) {
        SDL_memcpy(blue, &window->gamma[2*256], 256*sizeof(Uint16));
    }
    return 0;
}

void
SDL_UpdateWindowGrab(SDL_Window * window)
{
    SDL_bool keyboard_grabbed, mouse_grabbed;

    if (window->flags & SDL_WINDOW_INPUT_FOCUS) {
        if (SDL_GetMouse()->relative_mode || (window->flags & SDL_WINDOW_MOUSE_GRABBED)) {
            mouse_grabbed = SDL_TRUE;
        } else {
            mouse_grabbed = SDL_FALSE;
        }

        if (window->flags & SDL_WINDOW_KEYBOARD_GRABBED) {
            keyboard_grabbed = SDL_TRUE;
        } else {
            keyboard_grabbed = SDL_FALSE;
        }
    } else {
        mouse_grabbed = SDL_FALSE;
        keyboard_grabbed = SDL_FALSE;
    }

    if (mouse_grabbed || keyboard_grabbed) {
        if (_this->grabbed_window && (_this->grabbed_window != window)) {
            /* stealing a grab from another window! */
            _this->grabbed_window->flags &= ~(SDL_WINDOW_MOUSE_GRABBED | SDL_WINDOW_KEYBOARD_GRABBED);
            if (_this->SetWindowMouseGrab) {
                _this->SetWindowMouseGrab(_this, _this->grabbed_window, SDL_FALSE);
            }
            if (_this->SetWindowKeyboardGrab) {
                _this->SetWindowKeyboardGrab(_this, _this->grabbed_window, SDL_FALSE);
            }
        }
        _this->grabbed_window = window;
    } else if (_this->grabbed_window == window) {
        _this->grabbed_window = NULL;  /* ungrabbing input. */
    }

    if (_this->SetWindowMouseGrab) {
        _this->SetWindowMouseGrab(_this, window, mouse_grabbed);
    }
    if (_this->SetWindowKeyboardGrab) {
        _this->SetWindowKeyboardGrab(_this, window, keyboard_grabbed);
    }
}

void
SDL_SetWindowGrab(SDL_Window * window, SDL_bool grabbed)
{
    CHECK_WINDOW_MAGIC(window,);

    SDL_SetWindowMouseGrab(window, grabbed);

    if (SDL_GetHintBoolean(SDL_HINT_GRAB_KEYBOARD, SDL_FALSE)) {
        SDL_SetWindowKeyboardGrab(window, grabbed);
    }
}

void
SDL_SetWindowKeyboardGrab(SDL_Window * window, SDL_bool grabbed)
{
    CHECK_WINDOW_MAGIC(window,);

    if (!!grabbed == !!(window->flags & SDL_WINDOW_KEYBOARD_GRABBED)) {
        return;
    }
    if (grabbed) {
        window->flags |= SDL_WINDOW_KEYBOARD_GRABBED;
    } else {
        window->flags &= ~SDL_WINDOW_KEYBOARD_GRABBED;
    }
    SDL_UpdateWindowGrab(window);
}

void
SDL_SetWindowMouseGrab(SDL_Window * window, SDL_bool grabbed)
{
    CHECK_WINDOW_MAGIC(window,);

    if (!!grabbed == !!(window->flags & SDL_WINDOW_MOUSE_GRABBED)) {
        return;
    }
    if (grabbed) {
        window->flags |= SDL_WINDOW_MOUSE_GRABBED;
    } else {
        window->flags &= ~SDL_WINDOW_MOUSE_GRABBED;
    }
    SDL_UpdateWindowGrab(window);
}

SDL_bool
SDL_GetWindowGrab(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, SDL_FALSE);
    SDL_assert(!_this->grabbed_window ||
               ((_this->grabbed_window->flags & SDL_WINDOW_MOUSE_GRABBED) != 0) ||
               ((_this->grabbed_window->flags & SDL_WINDOW_KEYBOARD_GRABBED) != 0));
    return window == _this->grabbed_window;
}

SDL_bool
SDL_GetWindowKeyboardGrab(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, SDL_FALSE);
    return window == _this->grabbed_window &&
           ((_this->grabbed_window->flags & SDL_WINDOW_KEYBOARD_GRABBED) != 0);
}

SDL_bool
SDL_GetWindowMouseGrab(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, SDL_FALSE);
    return window == _this->grabbed_window &&
           ((_this->grabbed_window->flags & SDL_WINDOW_MOUSE_GRABBED) != 0);
}

SDL_Window *
SDL_GetGrabbedWindow(void)
{
    SDL_assert(!_this->grabbed_window ||
               ((_this->grabbed_window->flags & SDL_WINDOW_MOUSE_GRABBED) != 0) ||
               ((_this->grabbed_window->flags & SDL_WINDOW_KEYBOARD_GRABBED) != 0));
    return _this->grabbed_window;
}

int
SDL_FlashWindow(SDL_Window * window, SDL_FlashOperation operation)
{
    CHECK_WINDOW_MAGIC(window, -1);

    if (_this->FlashWindow) {
        return _this->FlashWindow(_this, window, operation);
    }

    return SDL_Unsupported();
}

void
SDL_OnWindowShown(SDL_Window * window)
{
    SDL_OnWindowRestored(window);
}

void
SDL_OnWindowHidden(SDL_Window * window)
{
    SDL_UpdateFullscreenMode(window, SDL_FALSE);
}

void
SDL_OnWindowResized(SDL_Window * window)
{
    window->surface_valid = SDL_FALSE;

    if (!window->is_destroying) {
        SDL_SendWindowEvent(window, SDL_WINDOWEVENT_SIZE_CHANGED, window->w, window->h);
    }
}

void
SDL_OnWindowMinimized(SDL_Window * window)
{
    SDL_UpdateFullscreenMode(window, SDL_FALSE);
}

void
SDL_OnWindowRestored(SDL_Window * window)
{
    /*
     * FIXME: Is this fine to just remove this, or should it be preserved just
     * for the fullscreen case? In principle it seems like just hiding/showing
     * windows shouldn't affect the stacking order; maybe the right fix is to
     * re-decouple OnWindowShown and OnWindowRestored.
     */
    /*SDL_RaiseWindow(window);*/

    if (FULLSCREEN_VISIBLE(window)) {
        SDL_UpdateFullscreenMode(window, SDL_TRUE);
    }
}

void
SDL_OnWindowEnter(SDL_Window * window)
{
    if (_this->OnWindowEnter) {
        _this->OnWindowEnter(_this, window);
    }
}

void
SDL_OnWindowLeave(SDL_Window * window)
{
}

void
SDL_OnWindowFocusGained(SDL_Window * window)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    if (window->gamma && _this->SetWindowGammaRamp) {
        _this->SetWindowGammaRamp(_this, window, window->gamma);
    }

    if (mouse && mouse->relative_mode) {
        SDL_SetMouseFocus(window);
        SDL_WarpMouseInWindow(window, window->w/2, window->h/2);
    }

    SDL_UpdateWindowGrab(window);
}

static SDL_bool
ShouldMinimizeOnFocusLoss(SDL_Window * window)
{
    const char *hint;

    if (!(window->flags & SDL_WINDOW_FULLSCREEN) || window->is_destroying) {
        return SDL_FALSE;
    }

#ifdef __MACOSX__
    if (SDL_strcmp(_this->name, "cocoa") == 0) {  /* don't do this for X11, etc */
        if (Cocoa_IsWindowInFullscreenSpace(window)) {
            return SDL_FALSE;
        }
    }
#endif

#ifdef __ANDROID__
    {
        extern SDL_bool Android_JNI_ShouldMinimizeOnFocusLoss(void);
        if (!Android_JNI_ShouldMinimizeOnFocusLoss()) {
            return SDL_FALSE;
        }
    }
#endif

    /* Real fullscreen windows should minimize on focus loss so the desktop video mode is restored */
    hint = SDL_GetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS);
    if (!hint || !*hint || SDL_strcasecmp(hint, "auto") == 0) {
        if ((window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP) {
            return SDL_FALSE;
        } else {
            return SDL_TRUE;
        }
    }
    return SDL_GetHintBoolean(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, SDL_FALSE);
}

void
SDL_OnWindowFocusLost(SDL_Window * window)
{
    if (window->gamma && _this->SetWindowGammaRamp) {
        _this->SetWindowGammaRamp(_this, window, window->saved_gamma);
    }

    SDL_UpdateWindowGrab(window);

    if (ShouldMinimizeOnFocusLoss(window)) {
        SDL_MinimizeWindow(window);
    }
}

/* !!! FIXME: is this different than SDL_GetKeyboardFocus()?
   !!! FIXME:  Also, SDL_GetKeyboardFocus() is O(1), this isn't. */
SDL_Window *
SDL_GetFocusWindow(void)
{
    SDL_Window *window;

    if (!_this) {
        return NULL;
    }
    for (window = _this->windows; window; window = window->next) {
        if (window->flags & SDL_WINDOW_INPUT_FOCUS) {
            return window;
        }
    }
    return NULL;
}

void
SDL_DestroyWindow(SDL_Window * window)
{
    SDL_VideoDisplay *display;

    CHECK_WINDOW_MAGIC(window,);

    window->is_destroying = SDL_TRUE;

    /* Restore video mode, etc. */
    SDL_HideWindow(window);

    /* Make sure this window no longer has focus */
    if (SDL_GetKeyboardFocus() == window) {
        SDL_SetKeyboardFocus(NULL);
    }
    if (SDL_GetMouseFocus() == window) {
        SDL_SetMouseFocus(NULL);
    }

    /* make no context current if this is the current context window. */
    if (window->flags & SDL_WINDOW_OPENGL) {
        if (_this->current_glwin == window) {
            SDL_GL_MakeCurrent(window, NULL);
        }
    }

    if (window->surface) {
        window->surface->flags &= ~SDL_DONTFREE;
        SDL_FreeSurface(window->surface);
        window->surface = NULL;
        window->surface_valid = SDL_FALSE;
    }
    if (_this->DestroyWindowFramebuffer) {
        _this->DestroyWindowFramebuffer(_this, window);
    }
    if (_this->DestroyWindow) {
        _this->DestroyWindow(_this, window);
    }
    if (window->flags & SDL_WINDOW_OPENGL) {
        SDL_GL_UnloadLibrary();
    }
    if (window->flags & SDL_WINDOW_VULKAN) {
        SDL_Vulkan_UnloadLibrary();
    }

    display = SDL_GetDisplayForWindow(window);
    if (display->fullscreen_window == window) {
        display->fullscreen_window = NULL;
    }

    /* Now invalidate magic */
    window->magic = NULL;

    /* Free memory associated with the window */
    SDL_free(window->title);
    SDL_FreeSurface(window->icon);
    SDL_free(window->gamma);
    while (window->data) {
        SDL_WindowUserData *data = window->data;

        window->data = data->next;
        SDL_free(data->name);
        SDL_free(data);
    }

    /* Unlink the window from the list */
    if (window->next) {
        window->next->prev = window->prev;
    }
    if (window->prev) {
        window->prev->next = window->next;
    } else {
        _this->windows = window->next;
    }

    SDL_free(window);
}

SDL_bool
SDL_IsScreenSaverEnabled()
{
    if (!_this) {
        return SDL_TRUE;
    }
    return _this->suspend_screensaver ? SDL_FALSE : SDL_TRUE;
}

void
SDL_EnableScreenSaver()
{
    if (!_this) {
        return;
    }
    if (!_this->suspend_screensaver) {
        return;
    }
    _this->suspend_screensaver = SDL_FALSE;
    if (_this->SuspendScreenSaver) {
        _this->SuspendScreenSaver(_this);
    }
}

void
SDL_DisableScreenSaver()
{
    if (!_this) {
        return;
    }
    if (_this->suspend_screensaver) {
        return;
    }
    _this->suspend_screensaver = SDL_TRUE;
    if (_this->SuspendScreenSaver) {
        _this->SuspendScreenSaver(_this);
    }
}

void
SDL_VideoQuit(void)
{
    int i, j;

    if (!_this) {
        return;
    }

    /* Halt event processing before doing anything else */
    SDL_TouchQuit();
    SDL_MouseQuit();
    SDL_KeyboardQuit();
    SDL_QuitSubSystem(SDL_INIT_EVENTS);

    SDL_EnableScreenSaver();

    /* Clean up the system video */
    while (_this->windows) {
        SDL_DestroyWindow(_this->windows);
    }
    _this->VideoQuit(_this);

    for (i = 0; i < _this->num_displays; ++i) {
        SDL_VideoDisplay *display = &_this->displays[i];
        for (j = display->num_display_modes; j--;) {
            SDL_free(display->display_modes[j].driverdata);
            display->display_modes[j].driverdata = NULL;
        }
        SDL_free(display->display_modes);
        display->display_modes = NULL;
        SDL_free(display->desktop_mode.driverdata);
        display->desktop_mode.driverdata = NULL;
        SDL_free(display->driverdata);
        display->driverdata = NULL;
    }
    if (_this->displays) {
        for (i = 0; i < _this->num_displays; ++i) {
            SDL_free(_this->displays[i].name);
        }
        SDL_free(_this->displays);
        _this->displays = NULL;
        _this->num_displays = 0;
    }
    SDL_free(_this->clipboard_text);
    _this->clipboard_text = NULL;
    _this->free(_this);
    _this = NULL;
}

int
SDL_GL_LoadLibrary(const char *path)
{
    int retval;

    if (!_this) {
        return SDL_UninitializedVideo();
    }
    if (_this->gl_config.driver_loaded) {
        if (path && SDL_strcmp(path, _this->gl_config.driver_path) != 0) {
            return SDL_SetError("OpenGL library already loaded");
        }
        retval = 0;
    } else {
        if (!_this->GL_LoadLibrary) {
            return SDL_SetError("No dynamic GL support in current SDL video driver (%s)", _this->name);
        }
        retval = _this->GL_LoadLibrary(_this, path);
    }
    if (retval == 0) {
        ++_this->gl_config.driver_loaded;
    } else {
        if (_this->GL_UnloadLibrary) {
            _this->GL_UnloadLibrary(_this);
        }
    }
    return (retval);
}

void *
SDL_GL_GetProcAddress(const char *proc)
{
    void *func;

    if (!_this) {
        SDL_UninitializedVideo();
        return NULL;
    }
    func = NULL;
    if (_this->GL_GetProcAddress) {
        if (_this->gl_config.driver_loaded) {
            func = _this->GL_GetProcAddress(_this, proc);
        } else {
            SDL_SetError("No GL driver has been loaded");
        }
    } else {
        SDL_SetError("No dynamic GL support in current SDL video driver (%s)", _this->name);
    }
    return func;
}

void
SDL_GL_UnloadLibrary(void)
{
    if (!_this) {
        SDL_UninitializedVideo();
        return;
    }
    if (_this->gl_config.driver_loaded > 0) {
        if (--_this->gl_config.driver_loaded > 0) {
            return;
        }
        if (_this->GL_UnloadLibrary) {
            _this->GL_UnloadLibrary(_this);
        }
    }
}

#if SDL_VIDEO_OPENGL || SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2
static SDL_INLINE SDL_bool
isAtLeastGL3(const char *verstr)
{
    return (verstr && (SDL_atoi(verstr) >= 3));
}
#endif

SDL_bool
SDL_GL_ExtensionSupported(const char *extension)
{
#if SDL_VIDEO_OPENGL || SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2
    const GLubyte *(APIENTRY * glGetStringFunc) (GLenum);
    const char *extensions;
    const char *start;
    const char *where, *terminator;

    /* Extension names should not have spaces. */
    where = SDL_strchr(extension, ' ');
    if (where || *extension == '\0') {
        return SDL_FALSE;
    }
    /* See if there's an environment variable override */
    start = SDL_getenv(extension);
    if (start && *start == '0') {
        return SDL_FALSE;
    }

    /* Lookup the available extensions */

    glGetStringFunc = SDL_GL_GetProcAddress("glGetString");
    if (!glGetStringFunc) {
        return SDL_FALSE;
    }

    if (isAtLeastGL3((const char *) glGetStringFunc(GL_VERSION))) {
        const GLubyte *(APIENTRY * glGetStringiFunc) (GLenum, GLuint);
        void (APIENTRY * glGetIntegervFunc) (GLenum pname, GLint * params);
        GLint num_exts = 0;
        GLint i;

        glGetStringiFunc = SDL_GL_GetProcAddress("glGetStringi");
        glGetIntegervFunc = SDL_GL_GetProcAddress("glGetIntegerv");
        if ((!glGetStringiFunc) || (!glGetIntegervFunc)) {
            return SDL_FALSE;
        }

        #ifndef GL_NUM_EXTENSIONS
        #define GL_NUM_EXTENSIONS 0x821D
        #endif
        glGetIntegervFunc(GL_NUM_EXTENSIONS, &num_exts);
        for (i = 0; i < num_exts; i++) {
            const char *thisext = (const char *) glGetStringiFunc(GL_EXTENSIONS, i);
            if (SDL_strcmp(thisext, extension) == 0) {
                return SDL_TRUE;
            }
        }

        return SDL_FALSE;
    }

    /* Try the old way with glGetString(GL_EXTENSIONS) ... */

    extensions = (const char *) glGetStringFunc(GL_EXTENSIONS);
    if (!extensions) {
        return SDL_FALSE;
    }
    /*
     * It takes a bit of care to be fool-proof about parsing the OpenGL
     * extensions string. Don't be fooled by sub-strings, etc.
     */

    start = extensions;

    for (;;) {
        where = SDL_strstr(start, extension);
        if (!where)
            break;

        terminator = where + SDL_strlen(extension);
        if (where == extensions || *(where - 1) == ' ')
            if (*terminator == ' ' || *terminator == '\0')
                return SDL_TRUE;

        start = terminator;
    }
    return SDL_FALSE;
#else
    return SDL_FALSE;
#endif
}

/* Deduce supported ES profile versions from the supported
   ARB_ES*_compatibility extensions. There is no direct query.
   
   This is normally only called when the OpenGL driver supports
   {GLX,WGL}_EXT_create_context_es2_profile.
 */
void
SDL_GL_DeduceMaxSupportedESProfile(int* major, int* minor)
{
/* THIS REQUIRES AN EXISTING GL CONTEXT THAT HAS BEEN MADE CURRENT. */
/*  Please refer to https://bugzilla.libsdl.org/show_bug.cgi?id=3725 for discussion. */
#if SDL_VIDEO_OPENGL || SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2
    /* XXX This is fragile; it will break in the event of release of
     * new versions of OpenGL ES.
     */
    if (SDL_GL_ExtensionSupported("GL_ARB_ES3_2_compatibility")) {
        *major = 3;
        *minor = 2;
    } else if (SDL_GL_ExtensionSupported("GL_ARB_ES3_1_compatibility")) {
        *major = 3;
        *minor = 1;
    } else if (SDL_GL_ExtensionSupported("GL_ARB_ES3_compatibility")) {
        *major = 3;
        *minor = 0;
    } else {
        *major = 2;
        *minor = 0;
    }
#endif
}

void
SDL_GL_ResetAttributes()
{
    if (!_this) {
        return;
    }

    _this->gl_config.red_size = 3;
    _this->gl_config.green_size = 3;
    _this->gl_config.blue_size = 2;
    _this->gl_config.alpha_size = 0;
    _this->gl_config.buffer_size = 0;
    _this->gl_config.depth_size = 16;
    _this->gl_config.stencil_size = 0;
    _this->gl_config.double_buffer = 1;
    _this->gl_config.accum_red_size = 0;
    _this->gl_config.accum_green_size = 0;
    _this->gl_config.accum_blue_size = 0;
    _this->gl_config.accum_alpha_size = 0;
    _this->gl_config.stereo = 0;
    _this->gl_config.multisamplebuffers = 0;
    _this->gl_config.multisamplesamples = 0;
    _this->gl_config.retained_backing = 1;
    _this->gl_config.accelerated = -1;  /* accelerated or not, both are fine */

#if SDL_VIDEO_OPENGL
    _this->gl_config.major_version = 2;
    _this->gl_config.minor_version = 1;
    _this->gl_config.profile_mask = 0;
#elif SDL_VIDEO_OPENGL_ES2
    _this->gl_config.major_version = 2;
    _this->gl_config.minor_version = 0;
    _this->gl_config.profile_mask = SDL_GL_CONTEXT_PROFILE_ES;
#elif SDL_VIDEO_OPENGL_ES
    _this->gl_config.major_version = 1;
    _this->gl_config.minor_version = 1;
    _this->gl_config.profile_mask = SDL_GL_CONTEXT_PROFILE_ES;
#endif

    if (_this->GL_DefaultProfileConfig) {
        _this->GL_DefaultProfileConfig(_this, &_this->gl_config.profile_mask,
                                       &_this->gl_config.major_version,
                                       &_this->gl_config.minor_version);
    }

    _this->gl_config.flags = 0;
    _this->gl_config.framebuffer_srgb_capable = 0;
    _this->gl_config.no_error = 0;
    _this->gl_config.release_behavior = SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH;
    _this->gl_config.reset_notification = SDL_GL_CONTEXT_RESET_NO_NOTIFICATION;

    _this->gl_config.share_with_current_context = 0;
}

int
SDL_GL_SetAttribute(SDL_GLattr attr, int value)
{
#if SDL_VIDEO_OPENGL || SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2
    int retval;

    if (!_this) {
        return SDL_UninitializedVideo();
    }
    retval = 0;
    switch (attr) {
    case SDL_GL_RED_SIZE:
        _this->gl_config.red_size = value;
        break;
    case SDL_GL_GREEN_SIZE:
        _this->gl_config.green_size = value;
        break;
    case SDL_GL_BLUE_SIZE:
        _this->gl_config.blue_size = value;
        break;
    case SDL_GL_ALPHA_SIZE:
        _this->gl_config.alpha_size = value;
        break;
    case SDL_GL_DOUBLEBUFFER:
        _this->gl_config.double_buffer = value;
        break;
    case SDL_GL_BUFFER_SIZE:
        _this->gl_config.buffer_size = value;
        break;
    case SDL_GL_DEPTH_SIZE:
        _this->gl_config.depth_size = value;
        break;
    case SDL_GL_STENCIL_SIZE:
        _this->gl_config.stencil_size = value;
        break;
    case SDL_GL_ACCUM_RED_SIZE:
        _this->gl_config.accum_red_size = value;
        break;
    case SDL_GL_ACCUM_GREEN_SIZE:
        _this->gl_config.accum_green_size = value;
        break;
    case SDL_GL_ACCUM_BLUE_SIZE:
        _this->gl_config.accum_blue_size = value;
        break;
    case SDL_GL_ACCUM_ALPHA_SIZE:
        _this->gl_config.accum_alpha_size = value;
        break;
    case SDL_GL_STEREO:
        _this->gl_config.stereo = value;
        break;
    case SDL_GL_MULTISAMPLEBUFFERS:
        _this->gl_config.multisamplebuffers = value;
        break;
    case SDL_GL_MULTISAMPLESAMPLES:
        _this->gl_config.multisamplesamples = value;
        break;
    case SDL_GL_ACCELERATED_VISUAL:
        _this->gl_config.accelerated = value;
        break;
    case SDL_GL_RETAINED_BACKING:
        _this->gl_config.retained_backing = value;
        break;
    case SDL_GL_CONTEXT_MAJOR_VERSION:
        _this->gl_config.major_version = value;
        break;
    case SDL_GL_CONTEXT_MINOR_VERSION:
        _this->gl_config.minor_version = value;
        break;
    case SDL_GL_CONTEXT_EGL:
        /* FIXME: SDL_GL_CONTEXT_EGL to be deprecated in SDL 2.1 */
        if (value != 0) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        } else {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
        };
        break;
    case SDL_GL_CONTEXT_FLAGS:
        if (value & ~(SDL_GL_CONTEXT_DEBUG_FLAG |
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG |
                      SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG |
                      SDL_GL_CONTEXT_RESET_ISOLATION_FLAG)) {
            retval = SDL_SetError("Unknown OpenGL context flag %d", value);
            break;
        }
        _this->gl_config.flags = value;
        break;
    case SDL_GL_CONTEXT_PROFILE_MASK:
        if (value != 0 &&
            value != SDL_GL_CONTEXT_PROFILE_CORE &&
            value != SDL_GL_CONTEXT_PROFILE_COMPATIBILITY &&
            value != SDL_GL_CONTEXT_PROFILE_ES) {
            retval = SDL_SetError("Unknown OpenGL context profile %d", value);
            break;
        }
        _this->gl_config.profile_mask = value;
        break;
    case SDL_GL_SHARE_WITH_CURRENT_CONTEXT:
        _this->gl_config.share_with_current_context = value;
        break;
    case SDL_GL_FRAMEBUFFER_SRGB_CAPABLE:
        _this->gl_config.framebuffer_srgb_capable = value;
        break;
    case SDL_GL_CONTEXT_RELEASE_BEHAVIOR:
        _this->gl_config.release_behavior = value;
        break;
    case SDL_GL_CONTEXT_RESET_NOTIFICATION:
        _this->gl_config.reset_notification = value;
        break;
    case SDL_GL_CONTEXT_NO_ERROR:
        _this->gl_config.no_error = value;
        break;
    default:
        retval = SDL_SetError("Unknown OpenGL attribute");
        break;
    }
    return retval;
#else
    return SDL_Unsupported();
#endif /* SDL_VIDEO_OPENGL */
}

int
SDL_GL_GetAttribute(SDL_GLattr attr, int *value)
{
#if SDL_VIDEO_OPENGL || SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2
    GLenum (APIENTRY *glGetErrorFunc) (void);
    GLenum attrib = 0;
    GLenum error = 0;

    /*
     * Some queries in Core Profile desktop OpenGL 3+ contexts require
     * glGetFramebufferAttachmentParameteriv instead of glGetIntegerv. Note that
     * the enums we use for the former function don't exist in OpenGL ES 2, and
     * the function itself doesn't exist prior to OpenGL 3 and OpenGL ES 2.
     */
#if SDL_VIDEO_OPENGL
    const GLubyte *(APIENTRY *glGetStringFunc) (GLenum name);
    void (APIENTRY *glGetFramebufferAttachmentParameterivFunc) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
    GLenum attachment = GL_BACK_LEFT;
    GLenum attachmentattrib = 0;
#endif

    if (!value) {
        return SDL_InvalidParamError("value");
    }

    /* Clear value in any case */
    *value = 0;

    if (!_this) {
        return SDL_UninitializedVideo();
    }

    switch (attr) {
    case SDL_GL_RED_SIZE:
#if SDL_VIDEO_OPENGL
        attachmentattrib = GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE;
#endif
        attrib = GL_RED_BITS;
        break;
    case SDL_GL_BLUE_SIZE:
#if SDL_VIDEO_OPENGL
        attachmentattrib = GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE;
#endif
        attrib = GL_BLUE_BITS;
        break;
    case SDL_GL_GREEN_SIZE:
#if SDL_VIDEO_OPENGL
        attachmentattrib = GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE;
#endif
        attrib = GL_GREEN_BITS;
        break;
    case SDL_GL_ALPHA_SIZE:
#if SDL_VIDEO_OPENGL
        attachmentattrib = GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE;
#endif
        attrib = GL_ALPHA_BITS;
        break;
    case SDL_GL_DOUBLEBUFFER:
#if SDL_VIDEO_OPENGL
        attrib = GL_DOUBLEBUFFER;
        break;
#else
        /* OpenGL ES 1.0 and above specifications have EGL_SINGLE_BUFFER      */
        /* parameter which switches double buffer to single buffer. OpenGL ES */
        /* SDL driver must set proper value after initialization              */
        *value = _this->gl_config.double_buffer;
        return 0;
#endif
    case SDL_GL_DEPTH_SIZE:
#if SDL_VIDEO_OPENGL
        attachment = GL_DEPTH;
        attachmentattrib = GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE;
#endif
        attrib = GL_DEPTH_BITS;
        break;
    case SDL_GL_STENCIL_SIZE:
#if SDL_VIDEO_OPENGL
        attachment = GL_STENCIL;
        attachmentattrib = GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE;
#endif
        attrib = GL_STENCIL_BITS;
        break;
#if SDL_VIDEO_OPENGL
    case SDL_GL_ACCUM_RED_SIZE:
        attrib = GL_ACCUM_RED_BITS;
        break;
    case SDL_GL_ACCUM_GREEN_SIZE:
        attrib = GL_ACCUM_GREEN_BITS;
        break;
    case SDL_GL_ACCUM_BLUE_SIZE:
        attrib = GL_ACCUM_BLUE_BITS;
        break;
    case SDL_GL_ACCUM_ALPHA_SIZE:
        attrib = GL_ACCUM_ALPHA_BITS;
        break;
    case SDL_GL_STEREO:
        attrib = GL_STEREO;
        break;
#else
    case SDL_GL_ACCUM_RED_SIZE:
    case SDL_GL_ACCUM_GREEN_SIZE:
    case SDL_GL_ACCUM_BLUE_SIZE:
    case SDL_GL_ACCUM_ALPHA_SIZE:
    case SDL_GL_STEREO:
        /* none of these are supported in OpenGL ES */
        *value = 0;
        return 0;
#endif
    case SDL_GL_MULTISAMPLEBUFFERS:
        attrib = GL_SAMPLE_BUFFERS;
        break;
    case SDL_GL_MULTISAMPLESAMPLES:
        attrib = GL_SAMPLES;
        break;
    case SDL_GL_CONTEXT_RELEASE_BEHAVIOR:
#if SDL_VIDEO_OPENGL
        attrib = GL_CONTEXT_RELEASE_BEHAVIOR;
#else
        attrib = GL_CONTEXT_RELEASE_BEHAVIOR_KHR;
#endif
        break;
    case SDL_GL_BUFFER_SIZE:
        {
            int rsize = 0, gsize = 0, bsize = 0, asize = 0;

            /* There doesn't seem to be a single flag in OpenGL for this! */
            if (SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &rsize) < 0) {
                return -1;
            }
            if (SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &gsize) < 0) {
                return -1;
            }
            if (SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &bsize) < 0) {
                return -1;
            }
            if (SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &asize) < 0) {
                return -1;
            }

            *value = rsize + gsize + bsize + asize;
            return 0;
        }
    case SDL_GL_ACCELERATED_VISUAL:
        {
            /* FIXME: How do we get this information? */
            *value = (_this->gl_config.accelerated != 0);
            return 0;
        }
    case SDL_GL_RETAINED_BACKING:
        {
            *value = _this->gl_config.retained_backing;
            return 0;
        }
    case SDL_GL_CONTEXT_MAJOR_VERSION:
        {
            *value = _this->gl_config.major_version;
            return 0;
        }
    case SDL_GL_CONTEXT_MINOR_VERSION:
        {
            *value = _this->gl_config.minor_version;
            return 0;
        }
    case SDL_GL_CONTEXT_EGL:
        /* FIXME: SDL_GL_CONTEXT_EGL to be deprecated in SDL 2.1 */
        {
            if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
                *value = 1;
            }
            else {
                *value = 0;
            }
            return 0;
        }
    case SDL_GL_CONTEXT_FLAGS:
        {
            *value = _this->gl_config.flags;
            return 0;
        }
    case SDL_GL_CONTEXT_PROFILE_MASK:
        {
            *value = _this->gl_config.profile_mask;
            return 0;
        }
    case SDL_GL_SHARE_WITH_CURRENT_CONTEXT:
        {
            *value = _this->gl_config.share_with_current_context;
            return 0;
        }
    case SDL_GL_FRAMEBUFFER_SRGB_CAPABLE:
        {
            *value = _this->gl_config.framebuffer_srgb_capable;
            return 0;
        }
    case SDL_GL_CONTEXT_NO_ERROR:
        {
            *value = _this->gl_config.no_error;
            return 0;
        }
    default:
        return SDL_SetError("Unknown OpenGL attribute");
    }

#if SDL_VIDEO_OPENGL
    glGetStringFunc = SDL_GL_GetProcAddress("glGetString");
    if (!glGetStringFunc) {
        return -1;
    }

    if (attachmentattrib && isAtLeastGL3((const char *) glGetStringFunc(GL_VERSION))) {
        glGetFramebufferAttachmentParameterivFunc = SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameteriv");

        if (glGetFramebufferAttachmentParameterivFunc) {
            glGetFramebufferAttachmentParameterivFunc(GL_FRAMEBUFFER, attachment, attachmentattrib, (GLint *) value);
        } else {
            return -1;
        }
    } else
#endif
    {
        void (APIENTRY *glGetIntegervFunc) (GLenum pname, GLint * params);
        glGetIntegervFunc = SDL_GL_GetProcAddress("glGetIntegerv");
        if (glGetIntegervFunc) {
            glGetIntegervFunc(attrib, (GLint *) value);
        } else {
            return -1;
        }
    }

    glGetErrorFunc = SDL_GL_GetProcAddress("glGetError");
    if (!glGetErrorFunc) {
        return -1;
    }

    error = glGetErrorFunc();
    if (error != GL_NO_ERROR) {
        if (error == GL_INVALID_ENUM) {
            return SDL_SetError("OpenGL error: GL_INVALID_ENUM");
        } else if (error == GL_INVALID_VALUE) {
            return SDL_SetError("OpenGL error: GL_INVALID_VALUE");
        }
        return SDL_SetError("OpenGL error: %08X", error);
    }
    return 0;
#else
    return SDL_Unsupported();
#endif /* SDL_VIDEO_OPENGL */
}

SDL_GLContext
SDL_GL_CreateContext(SDL_Window * window)
{
    SDL_GLContext ctx = NULL;
    CHECK_WINDOW_MAGIC(window, NULL);

    if (!(window->flags & SDL_WINDOW_OPENGL)) {
        SDL_SetError("The specified window isn't an OpenGL window");
        return NULL;
    }

    ctx = _this->GL_CreateContext(_this, window);

    /* Creating a context is assumed to make it current in the SDL driver. */
    if (ctx) {
        _this->current_glwin = window;
        _this->current_glctx = ctx;
        SDL_TLSSet(_this->current_glwin_tls, window, NULL);
        SDL_TLSSet(_this->current_glctx_tls, ctx, NULL);
    }
    return ctx;
}

int
SDL_GL_MakeCurrent(SDL_Window * window, SDL_GLContext ctx)
{
    int retval;

    if (window == SDL_GL_GetCurrentWindow() &&
        ctx == SDL_GL_GetCurrentContext()) {
        /* We're already current. */
        return 0;
    }

    if (!ctx) {
        window = NULL;
    } else if (window) {
        CHECK_WINDOW_MAGIC(window, -1);

        if (!(window->flags & SDL_WINDOW_OPENGL)) {
            return SDL_SetError("The specified window isn't an OpenGL window");
        }
    } else if (!_this->gl_allow_no_surface) {
        return SDL_SetError("Use of OpenGL without a window is not supported on this platform");
    }

    retval = _this->GL_MakeCurrent(_this, window, ctx);
    if (retval == 0) {
        _this->current_glwin = window;
        _this->current_glctx = ctx;
        SDL_TLSSet(_this->current_glwin_tls, window, NULL);
        SDL_TLSSet(_this->current_glctx_tls, ctx, NULL);
    }
    return retval;
}

SDL_Window *
SDL_GL_GetCurrentWindow(void)
{
    if (!_this) {
        SDL_UninitializedVideo();
        return NULL;
    }
    return (SDL_Window *)SDL_TLSGet(_this->current_glwin_tls);
}

SDL_GLContext
SDL_GL_GetCurrentContext(void)
{
    if (!_this) {
        SDL_UninitializedVideo();
        return NULL;
    }
    return (SDL_GLContext)SDL_TLSGet(_this->current_glctx_tls);
}

void SDL_GL_GetDrawableSize(SDL_Window * window, int *w, int *h)
{
    CHECK_WINDOW_MAGIC(window,);

    if (_this->GL_GetDrawableSize) {
        _this->GL_GetDrawableSize(_this, window, w, h);
    } else {
        SDL_GetWindowSize(window, w, h);
    }
}

int
SDL_GL_SetSwapInterval(int interval)
{
    if (!_this) {
        return SDL_UninitializedVideo();
    } else if (SDL_GL_GetCurrentContext() == NULL) {
        return SDL_SetError("No OpenGL context has been made current");
    } else if (_this->GL_SetSwapInterval) {
        return _this->GL_SetSwapInterval(_this, interval);
    } else {
        return SDL_SetError("Setting the swap interval is not supported");
    }
}

int
SDL_GL_GetSwapInterval(void)
{
    if (!_this) {
        return 0;
    } else if (SDL_GL_GetCurrentContext() == NULL) {
        return 0;
    } else if (_this->GL_GetSwapInterval) {
        return _this->GL_GetSwapInterval(_this);
    } else {
        return 0;
    }
}

void
SDL_GL_SwapWindow(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window,);

    if (!(window->flags & SDL_WINDOW_OPENGL)) {
        SDL_SetError("The specified window isn't an OpenGL window");
        return;
    }

    if (SDL_GL_GetCurrentWindow() != window) {
        SDL_SetError("The specified window has not been made current");
        return;
    }

    _this->GL_SwapWindow(_this, window);
}

void
SDL_GL_DeleteContext(SDL_GLContext context)
{
    if (!_this || !context) {
        return;
    }

    if (SDL_GL_GetCurrentContext() == context) {
        SDL_GL_MakeCurrent(NULL, NULL);
    }

    _this->GL_DeleteContext(_this, context);
}

#if 0                           /* FIXME */
/*
 * Utility function used by SDL_WM_SetIcon(); flags & 1 for color key, flags
 * & 2 for alpha channel.
 */
static void
CreateMaskFromColorKeyOrAlpha(SDL_Surface * icon, Uint8 * mask, int flags)
{
    int x, y;
    Uint32 colorkey;
#define SET_MASKBIT(icon, x, y, mask) \
    mask[(y*((icon->w+7)/8))+(x/8)] &= ~(0x01<<(7-(x%8)))

    colorkey = icon->format->colorkey;
    switch (icon->format->BytesPerPixel) {
    case 1:
        {
            Uint8 *pixels;
            for (y = 0; y < icon->h; ++y) {
                pixels = (Uint8 *) icon->pixels + y * icon->pitch;
                for (x = 0; x < icon->w; ++x) {
                    if (*pixels++ == colorkey) {
                        SET_MASKBIT(icon, x, y, mask);
                    }
                }
            }
        }
        break;

    case 2:
        {
            Uint16 *pixels;
            for (y = 0; y < icon->h; ++y) {
                pixels = (Uint16 *) icon->pixels + y * icon->pitch / 2;
                for (x = 0; x < icon->w; ++x) {
                    if ((flags & 1) && *pixels == colorkey) {
                        SET_MASKBIT(icon, x, y, mask);
                    } else if ((flags & 2)
                               && (*pixels & icon->format->Amask) == 0) {
                        SET_MASKBIT(icon, x, y, mask);
                    }
                    pixels++;
                }
            }
        }
        break;

    case 4:
        {
            Uint32 *pixels;
            for (y = 0; y < icon->h; ++y) {
                pixels = (Uint32 *) icon->pixels + y * icon->pitch / 4;
                for (x = 0; x < icon->w; ++x) {
                    if ((flags & 1) && *pixels == colorkey) {
                        SET_MASKBIT(icon, x, y, mask);
                    } else if ((flags & 2)
                               && (*pixels & icon->format->Amask) == 0) {
                        SET_MASKBIT(icon, x, y, mask);
                    }
                    pixels++;
                }
            }
        }
        break;
    }
}

/*
 * Sets the window manager icon for the display window.
 */
void
SDL_WM_SetIcon(SDL_Surface * icon, Uint8 * mask)
{
    if (icon && _this->SetIcon) {
        /* Generate a mask if necessary, and create the icon! */
        if (mask == NULL) {
            int mask_len = icon->h * (icon->w + 7) / 8;
            int flags = 0;
            mask = (Uint8 *) SDL_malloc(mask_len);
            if (mask == NULL) {
                return;
            }
            SDL_memset(mask, ~0, mask_len);
            if (icon->flags & SDL_SRCCOLORKEY)
                flags |= 1;
            if (icon->flags & SDL_SRCALPHA)
                flags |= 2;
            if (flags) {
                CreateMaskFromColorKeyOrAlpha(icon, mask, flags);
            }
            _this->SetIcon(_this, icon, mask);
            SDL_free(mask);
        } else {
            _this->SetIcon(_this, icon, mask);
        }
    }
}
#endif

SDL_bool
SDL_GetWindowWMInfo(SDL_Window * window, struct SDL_SysWMinfo *info)
{
    CHECK_WINDOW_MAGIC(window, SDL_FALSE);

    if (!info) {
        SDL_InvalidParamError("info");
        return SDL_FALSE;
    }
    info->subsystem = SDL_SYSWM_UNKNOWN;

    if (!_this->GetWindowWMInfo) {
        SDL_Unsupported();
        return SDL_FALSE;
    }
    return (_this->GetWindowWMInfo(_this, window, info));
}

void
SDL_StartTextInput(void)
{
    SDL_Window *window;

    /* First, enable text events */
    SDL_EventState(SDL_TEXTINPUT, SDL_ENABLE);
    SDL_EventState(SDL_TEXTEDITING, SDL_ENABLE);

    /* Then show the on-screen keyboard, if any */
    window = SDL_GetFocusWindow();
    if (window && _this && _this->ShowScreenKeyboard) {
        _this->ShowScreenKeyboard(_this, window);
    }

    /* Finally start the text input system */
    if (_this && _this->StartTextInput) {
        _this->StartTextInput(_this);
    }
}

SDL_bool
SDL_IsTextInputActive(void)
{
    return (SDL_GetEventState(SDL_TEXTINPUT) == SDL_ENABLE);
}

void
SDL_StopTextInput(void)
{
    SDL_Window *window;

    /* Stop the text input system */
    if (_this && _this->StopTextInput) {
        _this->StopTextInput(_this);
    }

    /* Hide the on-screen keyboard, if any */
    window = SDL_GetFocusWindow();
    if (window && _this && _this->HideScreenKeyboard) {
        _this->HideScreenKeyboard(_this, window);
    }

    /* Finally disable text events */
    SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
    SDL_EventState(SDL_TEXTEDITING, SDL_DISABLE);
}

void
SDL_SetTextInputRect(SDL_Rect *rect)
{
    if (_this && _this->SetTextInputRect) {
        _this->SetTextInputRect(_this, rect);
    }
}

SDL_bool
SDL_HasScreenKeyboardSupport(void)
{
    if (_this && _this->HasScreenKeyboardSupport) {
        return _this->HasScreenKeyboardSupport(_this);
    }
    return SDL_FALSE;
}

SDL_bool
SDL_IsScreenKeyboardShown(SDL_Window *window)
{
    if (window && _this && _this->IsScreenKeyboardShown) {
        return _this->IsScreenKeyboardShown(_this, window);
    }
    return SDL_FALSE;
}

#if SDL_VIDEO_DRIVER_ANDROID
#include "android/SDL_androidmessagebox.h"
#endif
#if SDL_VIDEO_DRIVER_WINDOWS
#include "windows/SDL_windowsmessagebox.h"
#endif
#if SDL_VIDEO_DRIVER_WINRT
#include "winrt/SDL_winrtmessagebox.h"
#endif
#if SDL_VIDEO_DRIVER_COCOA
#include "cocoa/SDL_cocoamessagebox.h"
#endif
#if SDL_VIDEO_DRIVER_UIKIT
#include "uikit/SDL_uikitmessagebox.h"
#endif
#if SDL_VIDEO_DRIVER_X11
#include "x11/SDL_x11messagebox.h"
#endif
#if SDL_VIDEO_DRIVER_WAYLAND
#include "wayland/SDL_waylandmessagebox.h"
#endif
#if SDL_VIDEO_DRIVER_HAIKU
#include "haiku/SDL_bmessagebox.h"
#endif
#if SDL_VIDEO_DRIVER_OS2
#include "os2/SDL_os2messagebox.h"
#endif
#if SDL_VIDEO_DRIVER_VITA
#include "vita/SDL_vitamessagebox.h"
#endif

#if SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT || SDL_VIDEO_DRIVER_COCOA || SDL_VIDEO_DRIVER_UIKIT || SDL_VIDEO_DRIVER_X11 || SDL_VIDEO_DRIVER_WAYLAND || SDL_VIDEO_DRIVER_HAIKU || SDL_VIDEO_DRIVER_OS2
static SDL_bool SDL_MessageboxValidForDriver(const SDL_MessageBoxData *messageboxdata, SDL_SYSWM_TYPE drivertype)
{
    SDL_SysWMinfo info;
    SDL_Window *window = messageboxdata->window;

    if (!window) {
        return SDL_TRUE;
    }

    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(window, &info)) {
        return SDL_TRUE;
    } else {
        return (info.subsystem == drivertype);
    }
}
#endif

int
SDL_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid)
{
    int dummybutton;
    int retval = -1;
    SDL_bool relative_mode;
    int show_cursor_prev;
    SDL_bool mouse_captured;
    SDL_Window *current_window;
    SDL_MessageBoxData mbdata;

    if (!messageboxdata) {
        return SDL_InvalidParamError("messageboxdata");
    } else if (messageboxdata->numbuttons < 0) {
        return SDL_SetError("Invalid number of buttons");
    }

    current_window = SDL_GetKeyboardFocus();
    mouse_captured = current_window && ((SDL_GetWindowFlags(current_window) & SDL_WINDOW_MOUSE_CAPTURE) != 0);
    relative_mode = SDL_GetRelativeMouseMode();
    SDL_CaptureMouse(SDL_FALSE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    show_cursor_prev = SDL_ShowCursor(1);
    SDL_ResetKeyboard();

    if (!buttonid) {
        buttonid = &dummybutton;
    }

    SDL_memcpy(&mbdata, messageboxdata, sizeof(*messageboxdata));
    if (!mbdata.title) mbdata.title = "";
    if (!mbdata.message) mbdata.message = "";
    messageboxdata = &mbdata;

    if (_this && _this->ShowMessageBox) {
        retval = _this->ShowMessageBox(_this, messageboxdata, buttonid);
    }

    /* It's completely fine to call this function before video is initialized */
#if SDL_VIDEO_DRIVER_ANDROID
    if (retval == -1 &&
        Android_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_WINDOWS
    if (retval == -1 &&
        SDL_MessageboxValidForDriver(messageboxdata, SDL_SYSWM_WINDOWS) &&
        WIN_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_WINRT
    if (retval == -1 &&
        SDL_MessageboxValidForDriver(messageboxdata, SDL_SYSWM_WINRT) &&
        WINRT_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_COCOA
    if (retval == -1 &&
        SDL_MessageboxValidForDriver(messageboxdata, SDL_SYSWM_COCOA) &&
        Cocoa_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_UIKIT
    if (retval == -1 &&
        SDL_MessageboxValidForDriver(messageboxdata, SDL_SYSWM_UIKIT) &&
        UIKit_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_X11
    if (retval == -1 &&
        SDL_MessageboxValidForDriver(messageboxdata, SDL_SYSWM_X11) &&
        X11_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_WAYLAND
    if (retval == -1 &&
        SDL_MessageboxValidForDriver(messageboxdata, SDL_SYSWM_WAYLAND) &&
        Wayland_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_HAIKU
    if (retval == -1 &&
        SDL_MessageboxValidForDriver(messageboxdata, SDL_SYSWM_HAIKU) &&
        HAIKU_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_OS2
    if (retval == -1 &&
        SDL_MessageboxValidForDriver(messageboxdata, SDL_SYSWM_OS2) &&
        OS2_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
#if SDL_VIDEO_DRIVER_VITA
    if (retval == -1 &&
        VITA_ShowMessageBox(messageboxdata, buttonid) == 0) {
        retval = 0;
    }
#endif
    if (retval == -1) {
        SDL_SetError("No message system available");
    }

    if (current_window) {
        SDL_RaiseWindow(current_window);
        if (mouse_captured) {
            SDL_CaptureMouse(SDL_TRUE);
        }
    }

    SDL_ShowCursor(show_cursor_prev);
    SDL_SetRelativeMouseMode(relative_mode);

    return retval;
}

int
SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *message, SDL_Window *window)
{
#ifdef __EMSCRIPTEN__
    /* !!! FIXME: propose a browser API for this, get this #ifdef out of here? */
    /* Web browsers don't (currently) have an API for a custom message box
       that can block, but for the most common case (SDL_ShowSimpleMessageBox),
       we can use the standard Javascript alert() function. */
    if (!title) title = "";
    if (!message) message = "";
    EM_ASM_({
        alert(UTF8ToString($0) + "\n\n" + UTF8ToString($1));
    }, title, message);
    return 0;
#else
    SDL_MessageBoxData data;
    SDL_MessageBoxButtonData button;

    SDL_zero(data);
    data.flags = flags;
    data.title = title;
    data.message = message;
    data.numbuttons = 1;
    data.buttons = &button;
    data.window = window;

    SDL_zero(button);
    button.flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    button.flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    button.text = "OK";

    return SDL_ShowMessageBox(&data, NULL);
#endif
}

SDL_bool
SDL_ShouldAllowTopmost(void)
{
    return SDL_GetHintBoolean(SDL_HINT_ALLOW_TOPMOST, SDL_TRUE);
}

int
SDL_SetWindowHitTest(SDL_Window * window, SDL_HitTest callback, void *userdata)
{
    CHECK_WINDOW_MAGIC(window, -1);

    if (!_this->SetWindowHitTest) {
        return SDL_Unsupported();
    } else if (_this->SetWindowHitTest(window, callback != NULL) == -1) {
        return -1;
    }

    window->hit_test = callback;
    window->hit_test_data = userdata;

    return 0;
}

float
SDL_ComputeDiagonalDPI(int hpix, int vpix, float hinches, float vinches)
{
    float den2 = hinches * hinches + vinches * vinches;
    if (den2 <= 0.0f) {
        return 0.0f;
    }

    return (float)(SDL_sqrt((double)hpix * (double)hpix + (double)vpix * (double)vpix) /
                   SDL_sqrt((double)den2));
}

/*
 * Functions used by iOS application delegates
 */
void SDL_OnApplicationWillTerminate(void)
{
    SDL_SendAppEvent(SDL_APP_TERMINATING);
}

void SDL_OnApplicationDidReceiveMemoryWarning(void)
{
    SDL_SendAppEvent(SDL_APP_LOWMEMORY);
}

void SDL_OnApplicationWillResignActive(void)
{
    if (_this) {
        SDL_Window *window;
        for (window = _this->windows; window != NULL; window = window->next) {
            SDL_SendWindowEvent(window, SDL_WINDOWEVENT_FOCUS_LOST, 0, 0);
            SDL_SendWindowEvent(window, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
        }
    }
    SDL_SendAppEvent(SDL_APP_WILLENTERBACKGROUND);
}

void SDL_OnApplicationDidEnterBackground(void)
{
    SDL_SendAppEvent(SDL_APP_DIDENTERBACKGROUND);
}

void SDL_OnApplicationWillEnterForeground(void)
{
    SDL_SendAppEvent(SDL_APP_WILLENTERFOREGROUND);
}

void SDL_OnApplicationDidBecomeActive(void)
{
    SDL_SendAppEvent(SDL_APP_DIDENTERFOREGROUND);

    if (_this) {
        SDL_Window *window;
        for (window = _this->windows; window != NULL; window = window->next) {
            SDL_SendWindowEvent(window, SDL_WINDOWEVENT_FOCUS_GAINED, 0, 0);
            SDL_SendWindowEvent(window, SDL_WINDOWEVENT_RESTORED, 0, 0);
        }
    }
}

#define NOT_A_VULKAN_WINDOW "The specified window isn't a Vulkan window"

int SDL_Vulkan_LoadLibrary(const char *path)
{
    int retval;
    if (!_this) {
        SDL_UninitializedVideo();
        return -1;
    }
    if (_this->vulkan_config.loader_loaded) {
        if (path && SDL_strcmp(path, _this->vulkan_config.loader_path) != 0) {
            return SDL_SetError("Vulkan loader library already loaded");
        }
        retval = 0;
    } else {
        if (!_this->Vulkan_LoadLibrary) {
            return SDL_SetError("Vulkan support is either not configured in SDL "
                                "or not available in current SDL video driver "
                                "(%s) or platform", _this->name);
        }
        retval = _this->Vulkan_LoadLibrary(_this, path);
    }
    if (retval == 0) {
        _this->vulkan_config.loader_loaded++;
    }
    return retval;
}

void *SDL_Vulkan_GetVkGetInstanceProcAddr(void)
{
    if (!_this) {
        SDL_UninitializedVideo();
        return NULL;
    }
    if (!_this->vulkan_config.loader_loaded) {
        SDL_SetError("No Vulkan loader has been loaded");
        return NULL;
    }
    return _this->vulkan_config.vkGetInstanceProcAddr;
}

void SDL_Vulkan_UnloadLibrary(void)
{
    if (!_this) {
        SDL_UninitializedVideo();
        return;
    }
    if (_this->vulkan_config.loader_loaded > 0) {
        if (--_this->vulkan_config.loader_loaded > 0) {
            return;
        }
        if (_this->Vulkan_UnloadLibrary) {
            _this->Vulkan_UnloadLibrary(_this);
        }
    }
}

SDL_bool SDL_Vulkan_GetInstanceExtensions(SDL_Window *window, unsigned *count, const char **names)
{
    if (window) {
        CHECK_WINDOW_MAGIC(window, SDL_FALSE);

        if (!(window->flags & SDL_WINDOW_VULKAN))
        {
            SDL_SetError(NOT_A_VULKAN_WINDOW);
            return SDL_FALSE;
        }
    }

    if (!count) {
        SDL_InvalidParamError("count");
        return SDL_FALSE;
    }

    return _this->Vulkan_GetInstanceExtensions(_this, window, count, names);
}

SDL_bool SDL_Vulkan_CreateSurface(SDL_Window *window,
                                  VkInstance instance,
                                  VkSurfaceKHR *surface)
{
    CHECK_WINDOW_MAGIC(window, SDL_FALSE);

    if (!(window->flags & SDL_WINDOW_VULKAN)) {
        SDL_SetError(NOT_A_VULKAN_WINDOW);
        return SDL_FALSE;
    }

    if (!instance) {
        SDL_InvalidParamError("instance");
        return SDL_FALSE;
    }

    if (!surface) {
        SDL_InvalidParamError("surface");
        return SDL_FALSE;
    }

    return _this->Vulkan_CreateSurface(_this, window, instance, surface);
}

void SDL_Vulkan_GetDrawableSize(SDL_Window * window, int *w, int *h)
{
    CHECK_WINDOW_MAGIC(window,);

    if (_this->Vulkan_GetDrawableSize) {
        _this->Vulkan_GetDrawableSize(_this, window, w, h);
    } else {
        SDL_GetWindowSize(window, w, h);
    }
}

SDL_MetalView
SDL_Metal_CreateView(SDL_Window * window)
{
    CHECK_WINDOW_MAGIC(window, NULL);

    if (!(window->flags & SDL_WINDOW_METAL)) {
        SDL_SetError("The specified window isn't a Metal window");
        return NULL;
    }

    if (_this->Metal_CreateView) {
        return _this->Metal_CreateView(_this, window);
    } else {
        SDL_SetError("Metal is not supported.");
        return NULL;
    }
}

void
SDL_Metal_DestroyView(SDL_MetalView view)
{
    if (_this && view && _this->Metal_DestroyView) {
        _this->Metal_DestroyView(_this, view);
    }
}

void *
SDL_Metal_GetLayer(SDL_MetalView view)
{
    if (_this && _this->Metal_GetLayer) {
        if (view) {
            return _this->Metal_GetLayer(_this, view);
        } else {
            SDL_InvalidParamError("view");
            return NULL;
        }
    } else {
        SDL_SetError("Metal is not supported.");
        return NULL;
    }
}

void SDL_Metal_GetDrawableSize(SDL_Window * window, int *w, int *h)
{
    CHECK_WINDOW_MAGIC(window,);

    if (_this->Metal_GetDrawableSize) {
        _this->Metal_GetDrawableSize(_this, window, w, h);
    } else {
        SDL_GetWindowSize(window, w, h);
    }
}

/* vi: set ts=4 sw=4 expandtab: */
