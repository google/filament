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

#include "../SDL_egl_c.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_keyboard_c.h"

#include "SDL_mirevents.h"
#include "SDL_mirwindow.h"

#include "SDL_mirdyn.h"

static int
IsMirWindowValid(MIR_Window* mir_window)
{
    if (!MIR_mir_window_is_valid(mir_window->window)) {
        const char* error = MIR_mir_window_get_error_message(mir_window->window);
        return SDL_SetError("Failed to create a mir surface: %s", error);
    }

    return 1;
}

static MirPixelFormat
FindValidPixelFormat(MIR_Data* mir_data)
{
    unsigned int pf_size = 32;
    unsigned int valid_formats;
    unsigned int f;

    MirPixelFormat formats[pf_size];
    MIR_mir_connection_get_available_surface_formats(mir_data->connection, formats,
                                                     pf_size, &valid_formats);

    for (f = 0; f < valid_formats; f++) {
        MirPixelFormat cur_pf = formats[f];

        if (cur_pf == mir_pixel_format_abgr_8888 ||
            cur_pf == mir_pixel_format_xbgr_8888 ||
            cur_pf == mir_pixel_format_argb_8888 ||
            cur_pf == mir_pixel_format_xrgb_8888) {

            return cur_pf;
        }
    }

    return mir_pixel_format_invalid;
}

int
MIR_CreateWindow(_THIS, SDL_Window* window)
{
    MIR_Window* mir_window;
    MIR_Data* mir_data;
    MirPixelFormat pixel_format;
    MirBufferUsage buffer_usage;

    MirWindowSpec* spec;

    mir_window = SDL_calloc(1, sizeof(MIR_Window));
    if (!mir_window)
        return SDL_OutOfMemory();

    mir_data = _this->driverdata;
    window->driverdata = mir_window;

    if (window->x == SDL_WINDOWPOS_UNDEFINED)
        window->x = 0;

    if (window->y == SDL_WINDOWPOS_UNDEFINED)
        window->y = 0;

    mir_window->mir_data = mir_data;
    mir_window->sdl_window = window;

    if (window->flags & SDL_WINDOW_OPENGL) {
        pixel_format = MIR_mir_connection_get_egl_pixel_format(mir_data->connection,
                                                               _this->egl_data->egl_display,
                                                               _this->egl_data->egl_config);
    }
    else {
        pixel_format = FindValidPixelFormat(mir_data);
    }

    mir_data->pixel_format = pixel_format;
    if (pixel_format == mir_pixel_format_invalid) {
        return SDL_SetError("Failed to find a valid pixel format.");
    }

    buffer_usage = mir_buffer_usage_hardware;
    if (mir_data->software)
        buffer_usage = mir_buffer_usage_software;

    spec = MIR_mir_create_normal_window_spec(mir_data->connection,
                                             window->w,
                                             window->h);

    MIR_mir_window_spec_set_buffer_usage(spec, buffer_usage);
    MIR_mir_window_spec_set_name(spec, "Mir surface");
    MIR_mir_window_spec_set_pixel_format(spec, pixel_format);

    if (window->flags & SDL_WINDOW_INPUT_FOCUS)
        SDL_SetKeyboardFocus(window);

    mir_window->window = MIR_mir_create_window_sync(spec);
    MIR_mir_window_set_event_handler(mir_window->window, MIR_HandleEvent, window);

    MIR_mir_window_spec_release(spec);

    if (!MIR_mir_window_is_valid(mir_window->window)) {
        return SDL_SetError("Failed to create a mir surface: %s",
            MIR_mir_window_get_error_message(mir_window->window));
    }

    if (window->flags & SDL_WINDOW_OPENGL) {
        EGLNativeWindowType egl_native_window =
                        (EGLNativeWindowType)MIR_mir_buffer_stream_get_egl_native_window(
                                                       MIR_mir_window_get_buffer_stream(mir_window->window));

        mir_window->egl_surface = SDL_EGL_CreateSurface(_this, egl_native_window);

        if (mir_window->egl_surface == EGL_NO_SURFACE) {
            return SDL_SetError("Failed to create a window surface %p",
                                _this->egl_data->egl_display);
        }
    }
    else {
        mir_window->egl_surface = EGL_NO_SURFACE;
    }

    mir_data->current_window = mir_window;

    return 0;
}

void
MIR_DestroyWindow(_THIS, SDL_Window* window)
{
    MIR_Data* mir_data     = _this->driverdata;
    MIR_Window* mir_window = window->driverdata;

    if (mir_data) {
        SDL_EGL_DestroySurface(_this, mir_window->egl_surface);
        MIR_mir_window_release_sync(mir_window->window);

        mir_data->current_window = NULL;

        SDL_free(mir_window);
    }
    window->driverdata = NULL;
}

SDL_bool
MIR_GetWindowWMInfo(_THIS, SDL_Window* window, SDL_SysWMinfo* info)
{
    if (info->version.major == SDL_MAJOR_VERSION &&
        info->version.minor == SDL_MINOR_VERSION) {
        MIR_Window* mir_window = window->driverdata;

        info->subsystem = SDL_SYSWM_MIR;
        info->info.mir.connection = mir_window->mir_data->connection;
        // Cannot change this to window due to it being in the public API
        info->info.mir.surface = mir_window->window;

        return SDL_TRUE;
    }

    return SDL_FALSE;
}

static void
UpdateMirWindowState(MIR_Data* mir_data, MIR_Window* mir_window, MirWindowState window_state)
{
    if (IsMirWindowValid(mir_window)) {
        MirWindowSpec* spec = MIR_mir_create_window_spec(mir_data->connection);
        MIR_mir_window_spec_set_state(spec, window_state);

        MIR_mir_window_apply_spec(mir_window->window, spec);
        MIR_mir_window_spec_release(spec);
    }
}

void
MIR_SetWindowFullscreen(_THIS, SDL_Window* window,
                        SDL_VideoDisplay* display,
                        SDL_bool fullscreen)
{
    if (IsMirWindowValid(window->driverdata)) {
        MirWindowState state;

        if (fullscreen) {
            state = mir_window_state_fullscreen;
        }
        else {
            state = mir_window_state_restored;
        }

        UpdateMirWindowState(_this->driverdata, window->driverdata, state);
    }
}

void
MIR_MaximizeWindow(_THIS, SDL_Window* window)
{
    UpdateMirWindowState(_this->driverdata, window->driverdata, mir_window_state_maximized);
}

void
MIR_MinimizeWindow(_THIS, SDL_Window* window)
{
    UpdateMirWindowState(_this->driverdata, window->driverdata, mir_window_state_minimized);
}

void
MIR_RestoreWindow(_THIS, SDL_Window * window)
{
    UpdateMirWindowState(_this->driverdata, window->driverdata, mir_window_state_restored);
}

void
MIR_HideWindow(_THIS, SDL_Window* window)
{
    UpdateMirWindowState(_this->driverdata, window->driverdata, mir_window_state_hidden);
}

void
MIR_SetWindowSize(_THIS, SDL_Window* window)
{
    MIR_Data* mir_data     = _this->driverdata;
    MIR_Window* mir_window = window->driverdata;

    if (IsMirWindowValid(mir_window)) {
        MirWindowSpec* spec = MIR_mir_create_window_spec(mir_data->connection);
        MIR_mir_window_spec_set_width (spec, window->w);
        MIR_mir_window_spec_set_height(spec, window->h);

        MIR_mir_window_apply_spec(mir_window->window, spec);
    }
}

void
MIR_SetWindowMinimumSize(_THIS, SDL_Window* window)
{
    MIR_Data* mir_data     = _this->driverdata;
    MIR_Window* mir_window = window->driverdata;

    if (IsMirWindowValid(mir_window)) {
        MirWindowSpec* spec = MIR_mir_create_window_spec(mir_data->connection);
        MIR_mir_window_spec_set_min_width (spec, window->min_w);
        MIR_mir_window_spec_set_min_height(spec, window->min_h);

        MIR_mir_window_apply_spec(mir_window->window, spec);
    }
}

void
MIR_SetWindowMaximumSize(_THIS, SDL_Window* window)
{
    MIR_Data* mir_data     = _this->driverdata;
    MIR_Window* mir_window = window->driverdata;

    if (IsMirWindowValid(mir_window)) {
        MirWindowSpec* spec = MIR_mir_create_window_spec(mir_data->connection);
        MIR_mir_window_spec_set_max_width (spec, window->max_w);
        MIR_mir_window_spec_set_max_height(spec, window->max_h);

        MIR_mir_window_apply_spec(mir_window->window, spec);
    }
}

void
MIR_SetWindowTitle(_THIS, SDL_Window* window)
{
    MIR_Data*   mir_data   = _this->driverdata;
    MIR_Window* mir_window = window->driverdata;
    char const* title = window->title ? window->title : "";
    MirWindowSpec* spec;

    if (IsMirWindowValid(mir_window) < 0)
        return;

    spec = MIR_mir_create_window_spec(mir_data->connection);
    MIR_mir_window_spec_set_name(spec, title);

    MIR_mir_window_apply_spec(mir_window->window, spec);
    MIR_mir_window_spec_release(spec);
}

void
MIR_SetWindowGrab(_THIS, SDL_Window* window, SDL_bool grabbed)
{
    MIR_Data*   mir_data   = _this->driverdata;
    MIR_Window* mir_window = window->driverdata;
    MirPointerConfinementState confined = mir_pointer_unconfined;
    MirWindowSpec* spec;

    if (grabbed)
        confined = mir_pointer_confined_to_window;

    spec = MIR_mir_create_window_spec(mir_data->connection);
    MIR_mir_window_spec_set_pointer_confinement(spec, confined);

    MIR_mir_window_apply_spec(mir_window->window, spec);
    MIR_mir_window_spec_release(spec);
}

int
MIR_SetWindowGammaRamp(_THIS, SDL_Window* window, Uint16 const* ramp)
{
    MirOutput* output = SDL_GetDisplayForWindow(window)->driverdata;
    Uint32 ramp_size = 256;

    // FIXME Need to apply the changes to the output, once that public API function is around
    if (MIR_mir_output_is_gamma_supported(output) == mir_output_gamma_supported) {
        MIR_mir_output_set_gamma(output,
                                 ramp + ramp_size * 0,
                                 ramp + ramp_size * 1,
                                 ramp + ramp_size * 2,
                                 ramp_size);
        return 0;
    }

    return -1;
}

int
MIR_GetWindowGammaRamp(_THIS, SDL_Window* window, Uint16* ramp)
{
    MirOutput* output = SDL_GetDisplayForWindow(window)->driverdata;
    Uint32 ramp_size = 256;

    if (MIR_mir_output_is_gamma_supported(output) == mir_output_gamma_supported) {
        if (MIR_mir_output_get_gamma_size(output) == ramp_size) {
            MIR_mir_output_get_gamma(output,
                                     ramp + ramp_size * 0,
                                     ramp + ramp_size * 1,
                                     ramp + ramp_size * 2,
                                     ramp_size);
            return 0;
        }
    }

    return -1;
}

#endif /* SDL_VIDEO_DRIVER_MIR */

/* vi: set ts=4 sw=4 expandtab: */
