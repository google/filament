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

/*
  Contributed by Brandon Schaefer, <brandon.schaefer@canonical.com>
*/

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MIR

#include "SDL_assert.h"
#include "SDL_log.h"

#include "SDL_mirwindow.h"
#include "SDL_video.h"

#include "SDL_mirframebuffer.h"
#include "SDL_mirmouse.h"
#include "SDL_miropengl.h"
#include "SDL_mirvideo.h"
#include "SDL_mirvulkan.h"

#include "SDL_mirdyn.h"

#define MIR_DRIVER_NAME "mir"

static const Uint32 mir_pixel_format_to_sdl_format[] = {
    SDL_PIXELFORMAT_UNKNOWN,  /* mir_pixel_format_invalid   */
    SDL_PIXELFORMAT_ABGR8888, /* mir_pixel_format_abgr_8888 */
    SDL_PIXELFORMAT_BGR888,   /* mir_pixel_format_xbgr_8888 */
    SDL_PIXELFORMAT_ARGB8888, /* mir_pixel_format_argb_8888 */
    SDL_PIXELFORMAT_RGB888,   /* mir_pixel_format_xrgb_8888 */
    SDL_PIXELFORMAT_BGR24,    /* mir_pixel_format_bgr_888   */
    SDL_PIXELFORMAT_RGB24,    /* mir_pixel_format_rgb_888   */
    SDL_PIXELFORMAT_RGB565,   /* mir_pixel_format_rgb_565   */
    SDL_PIXELFORMAT_RGBA5551, /* mir_pixel_format_rgba_5551 */
    SDL_PIXELFORMAT_RGBA4444  /* mir_pixel_format_rgba_4444 */
};

Uint32
MIR_GetSDLPixelFormat(MirPixelFormat format)
{
    return mir_pixel_format_to_sdl_format[format];
}

static int
MIR_VideoInit(_THIS);

static void
MIR_VideoQuit(_THIS);

static int
MIR_GetDisplayBounds(_THIS, SDL_VideoDisplay* display, SDL_Rect* rect);

static void
MIR_GetDisplayModes(_THIS, SDL_VideoDisplay* sdl_display);

static int
MIR_SetDisplayMode(_THIS, SDL_VideoDisplay* sdl_display, SDL_DisplayMode* mode);

static SDL_WindowShaper*
MIR_CreateShaper(SDL_Window* window)
{
    /* FIXME Im not sure if mir support this atm, will have to come back to this */
    return NULL;
}

static int
MIR_SetWindowShape(SDL_WindowShaper* shaper, SDL_Surface* shape, SDL_WindowShapeMode* shape_mode)
{
    return SDL_Unsupported();
}

static int
MIR_ResizeWindowShape(SDL_Window* window)
{
    return SDL_Unsupported();
}

static int
MIR_Available()
{
    int available = 0;

    if (SDL_MIR_LoadSymbols()) {

        /* Lets ensure we can connect to the mir server */
        MirConnection* connection = MIR_mir_connect_sync(NULL, SDL_FUNCTION);

        if (!MIR_mir_connection_is_valid(connection)) {
            SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Unable to connect to the mir server %s",
                MIR_mir_connection_get_error_message(connection));

            return available;
        }

        MIR_mir_connection_release(connection);

        available = 1;
        SDL_MIR_UnloadSymbols();
    }

    return available;
}

static void
MIR_DeleteDevice(SDL_VideoDevice* device)
{
    SDL_free(device);
    SDL_MIR_UnloadSymbols();
}

static void
MIR_PumpEvents(_THIS)
{
}

static SDL_VideoDevice*
MIR_CreateDevice(int device_index)
{
    MIR_Data* mir_data;
    SDL_VideoDevice* device = NULL;

    if (!SDL_MIR_LoadSymbols()) {
        return NULL;
    }

    device = SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_MIR_UnloadSymbols();
        SDL_OutOfMemory();
        return NULL;
    }

    mir_data = SDL_calloc(1, sizeof(MIR_Data));
    if (!mir_data) {
        SDL_free(device);
        SDL_MIR_UnloadSymbols();
        SDL_OutOfMemory();
        return NULL;
    }

    device->driverdata = mir_data;

    /* mirvideo */
    device->VideoInit        = MIR_VideoInit;
    device->VideoQuit        = MIR_VideoQuit;
    device->GetDisplayBounds = MIR_GetDisplayBounds;
    device->GetDisplayModes  = MIR_GetDisplayModes;
    device->SetDisplayMode   = MIR_SetDisplayMode;
    device->free             = MIR_DeleteDevice;

    /* miropengles */
    device->GL_SwapWindow      = MIR_GL_SwapWindow;
    device->GL_MakeCurrent     = MIR_GL_MakeCurrent;
    device->GL_CreateContext   = MIR_GL_CreateContext;
    device->GL_DeleteContext   = MIR_GL_DeleteContext;
    device->GL_LoadLibrary     = MIR_GL_LoadLibrary;
    device->GL_UnloadLibrary   = MIR_GL_UnloadLibrary;
    device->GL_GetSwapInterval = MIR_GL_GetSwapInterval;
    device->GL_SetSwapInterval = MIR_GL_SetSwapInterval;
    device->GL_GetProcAddress  = MIR_GL_GetProcAddress;

    /* mirwindow */
    device->CreateSDLWindow         = MIR_CreateWindow;
    device->DestroyWindow        = MIR_DestroyWindow;
    device->GetWindowWMInfo      = MIR_GetWindowWMInfo;
    device->SetWindowFullscreen  = MIR_SetWindowFullscreen;
    device->MaximizeWindow       = MIR_MaximizeWindow;
    device->MinimizeWindow       = MIR_MinimizeWindow;
    device->RestoreWindow        = MIR_RestoreWindow;
    device->ShowWindow           = MIR_RestoreWindow;
    device->HideWindow           = MIR_HideWindow;
    device->SetWindowSize        = MIR_SetWindowSize;
    device->SetWindowMinimumSize = MIR_SetWindowMinimumSize;
    device->SetWindowMaximumSize = MIR_SetWindowMaximumSize;
    device->SetWindowTitle       = MIR_SetWindowTitle;
    device->SetWindowGrab        = MIR_SetWindowGrab;
    device->SetWindowGammaRamp   = MIR_SetWindowGammaRamp;
    device->GetWindowGammaRamp   = MIR_GetWindowGammaRamp;

    device->CreateSDLWindowFrom     = NULL;
    device->SetWindowIcon        = NULL;
    device->RaiseWindow          = NULL;
    device->SetWindowBordered    = NULL;
    device->SetWindowResizable   = NULL;
    device->OnWindowEnter        = NULL;
    device->SetWindowPosition    = NULL;

    /* mirframebuffer */
    device->CreateWindowFramebuffer  = MIR_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer  = MIR_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = MIR_DestroyWindowFramebuffer;

    device->shape_driver.CreateShaper      = MIR_CreateShaper;
    device->shape_driver.SetWindowShape    = MIR_SetWindowShape;
    device->shape_driver.ResizeWindowShape = MIR_ResizeWindowShape;

    device->PumpEvents = MIR_PumpEvents;

    device->SuspendScreenSaver = NULL;

    device->StartTextInput   = NULL;
    device->StopTextInput    = NULL;
    device->SetTextInputRect = NULL;

    device->HasScreenKeyboardSupport = NULL;
    device->ShowScreenKeyboard       = NULL;
    device->HideScreenKeyboard       = NULL;
    device->IsScreenKeyboardShown    = NULL;

    device->SetClipboardText = NULL;
    device->GetClipboardText = NULL;
    device->HasClipboardText = NULL;

    device->ShowMessageBox = NULL;

#if SDL_VIDEO_VULKAN
    device->Vulkan_LoadLibrary = MIR_Vulkan_LoadLibrary;
    device->Vulkan_UnloadLibrary = MIR_Vulkan_UnloadLibrary;
    device->Vulkan_GetInstanceExtensions = MIR_Vulkan_GetInstanceExtensions;
    device->Vulkan_CreateSurface = MIR_Vulkan_CreateSurface;
#endif

    return device;
}

VideoBootStrap MIR_bootstrap = {
    MIR_DRIVER_NAME, "SDL Mir video driver",
    MIR_Available, MIR_CreateDevice
};

static SDL_DisplayMode
MIR_ConvertModeToSDLMode(MirOutputMode const* mode, MirPixelFormat format)
{
    SDL_DisplayMode sdl_mode  = {
        .format = MIR_GetSDLPixelFormat(format),
        .w      = MIR_mir_output_mode_get_width(mode),
        .h      = MIR_mir_output_mode_get_height(mode),
        .refresh_rate = MIR_mir_output_mode_get_refresh_rate(mode),
        .driverdata   = NULL
    };

    return sdl_mode;
}

static void
MIR_AddModeToDisplay(SDL_VideoDisplay* display, MirOutputMode const* mode, MirPixelFormat format)
{
    SDL_DisplayMode sdl_mode = MIR_ConvertModeToSDLMode(mode, format);
    SDL_AddDisplayMode(display, &sdl_mode);
}

static void
MIR_InitDisplayFromOutput(_THIS, MirOutput* output)
{
    SDL_VideoDisplay display;
    int m;

    MirPixelFormat format = MIR_mir_output_get_current_pixel_format(output);
    int num_modes         = MIR_mir_output_get_num_modes(output);
    SDL_DisplayMode current_mode = MIR_ConvertModeToSDLMode(MIR_mir_output_get_current_mode(output), format);

    SDL_zero(display);

    // Unfortunate cast, but SDL_AddVideoDisplay will strdup this pointer so its read-only in this case.
    display.name = (char*)MIR_mir_output_type_name(MIR_mir_output_get_type(output));

    for (m = 0; m < num_modes; m++) {
        MirOutputMode const* mode = MIR_mir_output_get_mode(output, m);
        MIR_AddModeToDisplay(&display, mode, format);
    }

    display.desktop_mode = current_mode;
    display.current_mode = current_mode;

    display.driverdata = output;
    SDL_AddVideoDisplay(&display);
}

static void
MIR_InitDisplays(_THIS)
{
    MIR_Data* mir_data = _this->driverdata;
    int num_outputs    = MIR_mir_display_config_get_num_outputs(mir_data->display_config);
    int d;

    for (d = 0; d < num_outputs; d++) {
        MirOutput* output = MIR_mir_display_config_get_mutable_output(mir_data->display_config, d);
        SDL_bool enabled  = MIR_mir_output_is_enabled(output);
        MirOutputConnectionState state = MIR_mir_output_get_connection_state(output);

        if (enabled && state == mir_output_connection_state_connected) {
            MIR_InitDisplayFromOutput(_this, output);
        }
    }
}

static int
MIR_VideoInit(_THIS)
{
    MIR_Data* mir_data = _this->driverdata;

    mir_data->connection     = MIR_mir_connect_sync(NULL, SDL_FUNCTION);
    mir_data->current_window = NULL;
    mir_data->software       = SDL_FALSE;
    mir_data->pixel_format   = mir_pixel_format_invalid;

    if (!MIR_mir_connection_is_valid(mir_data->connection)) {
        return SDL_SetError("Failed to connect to the mir server: %s",
            MIR_mir_connection_get_error_message(mir_data->connection));
    }

    mir_data->display_config = MIR_mir_connection_create_display_configuration(mir_data->connection);

    MIR_InitDisplays(_this);
    MIR_InitMouse();

    return 0;
}

static void
MIR_CleanUpDisplayConfig(_THIS)
{
    MIR_Data* mir_data = _this->driverdata;
    int i;

    // SDL_VideoQuit frees the display driverdata, we own it not them
    for (i = 0; i < _this->num_displays; ++i) {
        _this->displays[i].driverdata = NULL;
    }

    MIR_mir_display_config_release(mir_data->display_config);
}

static void
MIR_VideoQuit(_THIS)
{
    MIR_Data* mir_data = _this->driverdata;

    MIR_CleanUpDisplayConfig(_this);

    MIR_FiniMouse();

    MIR_GL_DeleteContext(_this, NULL);
    MIR_GL_UnloadLibrary(_this);

    MIR_mir_connection_release(mir_data->connection);

    SDL_free(mir_data);
    _this->driverdata = NULL;
}

static int
MIR_GetDisplayBounds(_THIS, SDL_VideoDisplay* display, SDL_Rect* rect)
{
    MirOutput const* output = display->driverdata;

    rect->x = MIR_mir_output_get_position_x(output);
    rect->y = MIR_mir_output_get_position_y(output);
    rect->w = display->current_mode.w;
    rect->h = display->current_mode.h;

    return 0;
}

static void
MIR_GetDisplayModes(_THIS, SDL_VideoDisplay* display)
{
}

static int
MIR_SetDisplayMode(_THIS, SDL_VideoDisplay* display, SDL_DisplayMode* mode)
{
    int m;
    MirOutput* output = display->driverdata;
    int num_modes     = MIR_mir_output_get_num_modes(output);
    Uint32 sdl_format = MIR_GetSDLPixelFormat(
                            MIR_mir_output_get_current_pixel_format(output));

    for (m = 0; m < num_modes; m++) {
        MirOutputMode const* mir_mode = MIR_mir_output_get_mode(output, m);
        int width  = MIR_mir_output_mode_get_width(mir_mode);
        int height = MIR_mir_output_mode_get_height(mir_mode);
        double refresh_rate = MIR_mir_output_mode_get_refresh_rate(mir_mode);

        if (mode->format == sdl_format &&
            mode->w      == width &&
            mode->h      == height &&
            mode->refresh_rate == refresh_rate) {

            // FIXME Currently wont actually *set* anything. Need to wait for applying display changes
            MIR_mir_output_set_current_mode(output, mir_mode);
            return 0;
        }
    }

    return -1;
}

#endif /* SDL_VIDEO_DRIVER_MIR */

/* vi: set ts=4 sw=4 expandtab: */

