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

#include "../../events/SDL_mouse_c.h"
#include "../SDL_sysvideo.h"
#include "SDL_assert.h"

#include "SDL_mirdyn.h"

#include "SDL_mirvideo.h"
#include "SDL_mirmouse.h"
#include "SDL_mirwindow.h"

typedef struct
{
    MirCursorConfiguration* conf;
    MirBufferStream*        stream;
    char const*             name;
} MIR_Cursor;

static SDL_Cursor*
MIR_CreateDefaultCursor()
{
    SDL_Cursor* cursor;

    cursor = SDL_calloc(1, sizeof(SDL_Cursor));
    if (cursor) {

        MIR_Cursor* mir_cursor = SDL_calloc(1, sizeof(MIR_Cursor));
        if (mir_cursor) {
            mir_cursor->conf   = NULL;
            mir_cursor->stream = NULL;
            mir_cursor->name   = NULL;
            cursor->driverdata = mir_cursor;
        }
        else {
            SDL_OutOfMemory();
            SDL_free(cursor);
            cursor = NULL;
        }
    }
    else {
        SDL_OutOfMemory();
    }

    return cursor;
}

static void
CopySurfacePixelsToMirStream(SDL_Surface* surface, MirBufferStream* stream)
{
    char* dest, *pixels;
    int i, s_w, s_h, r_stride, p_stride, bytes_per_pixel, bytes_per_row;

    MirGraphicsRegion region;
    MIR_mir_buffer_stream_get_graphics_region(stream, &region);

    s_w = surface->w;
    s_h = surface->h;

    bytes_per_pixel = surface->format->BytesPerPixel;
    bytes_per_row   = bytes_per_pixel * s_w;

    dest = region.vaddr;
    pixels = (char*)surface->pixels;

    r_stride = region.stride;
    p_stride = surface->pitch;

    for (i = 0; i < s_h; i++)
    {
        SDL_memcpy(dest, pixels, bytes_per_row);
        dest   += r_stride;
        pixels += p_stride;
    }
}

static SDL_Cursor*
MIR_CreateCursor(SDL_Surface* surface, int hot_x, int hot_y)
{
    MirCursorConfiguration* conf;
    MirBufferStream*        stream;

    int s_w = surface->w;
    int s_h = surface->h;

    MIR_Data* mir_data     = (MIR_Data*)SDL_GetVideoDevice()->driverdata;
    SDL_Cursor* cursor     = MIR_CreateDefaultCursor();
    MIR_Cursor* mir_cursor;

    if (!cursor) {
        return NULL;
    }

    mir_cursor = (MIR_Cursor*)cursor->driverdata;

    stream = MIR_mir_connection_create_buffer_stream_sync(mir_data->connection,
                                                          s_w, s_h, mir_data->pixel_format,
                                                          mir_buffer_usage_software);

    conf = MIR_mir_cursor_configuration_from_buffer_stream(stream, hot_x, hot_y);

    CopySurfacePixelsToMirStream(surface, stream);
    MIR_mir_buffer_stream_swap_buffers_sync(stream);

    mir_cursor->conf   = conf;
    mir_cursor->stream = stream;

    return cursor;
}

static SDL_Cursor*
MIR_CreateSystemCursor(SDL_SystemCursor id)
{
    char const* cursor_name = NULL;
    SDL_Cursor* cursor;
    MIR_Cursor* mir_cursor;

    switch(id) {
        case SDL_SYSTEM_CURSOR_ARROW:
            cursor_name = MIR_mir_arrow_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_IBEAM:
            cursor_name = MIR_mir_caret_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_WAIT:
            cursor_name = MIR_mir_busy_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_CROSSHAIR:
            /* Unsupported */
            cursor_name = MIR_mir_arrow_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_WAITARROW:
            cursor_name = MIR_mir_busy_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_SIZENWSE:
            cursor_name = MIR_mir_omnidirectional_resize_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_SIZENESW:
            cursor_name = MIR_mir_omnidirectional_resize_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_SIZEWE:
            cursor_name = MIR_mir_horizontal_resize_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_SIZENS:
            cursor_name = MIR_mir_vertical_resize_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_SIZEALL:
            cursor_name = MIR_mir_omnidirectional_resize_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_NO:
            /* Unsupported */
            cursor_name = MIR_mir_closed_hand_cursor_name;
            break;
        case SDL_SYSTEM_CURSOR_HAND:
            cursor_name = MIR_mir_open_hand_cursor_name;
            break;
        default:
            SDL_assert(0);
            return NULL;
    }

    cursor = MIR_CreateDefaultCursor();
    if (!cursor) {
        return NULL;
    }

    mir_cursor = (MIR_Cursor*)cursor->driverdata;
    mir_cursor->name = cursor_name;

    return cursor;
}

static void
MIR_FreeCursor(SDL_Cursor* cursor)
{
    if (cursor) {

        if (cursor->driverdata) {
            MIR_Cursor* mir_cursor = (MIR_Cursor*)cursor->driverdata;

            if (mir_cursor->conf)
                MIR_mir_cursor_configuration_destroy(mir_cursor->conf);
            if (mir_cursor->stream)
                MIR_mir_buffer_stream_release_sync(mir_cursor->stream);

            SDL_free(mir_cursor);
        }

        SDL_free(cursor);
    }
}

static int
MIR_ShowCursor(SDL_Cursor* cursor)
{
    MIR_Data* mir_data      = (MIR_Data*)SDL_GetVideoDevice()->driverdata;
    MIR_Window* mir_window  = mir_data->current_window;

    if (cursor && cursor->driverdata) {
        if (mir_window && MIR_mir_window_is_valid(mir_window->window)) {
            MIR_Cursor* mir_cursor = (MIR_Cursor*)cursor->driverdata;

            if (mir_cursor->name != NULL) {
                MirWindowSpec* spec = MIR_mir_create_window_spec(mir_data->connection);
                MIR_mir_window_spec_set_cursor_name(spec, mir_cursor->name);
                MIR_mir_window_apply_spec(mir_window->window, spec);
                MIR_mir_window_spec_release(spec);
            }

            if (mir_cursor->conf) {
                MIR_mir_window_configure_cursor(mir_window->window, mir_cursor->conf);
            }
        }
    }
    else if(mir_window && MIR_mir_window_is_valid(mir_window->window)) {
        MIR_mir_window_configure_cursor(mir_window->window, NULL);
    }

    return 0;
}

static void
MIR_WarpMouse(SDL_Window* window, int x, int y)
{
    SDL_Unsupported();
}

static int
MIR_WarpMouseGlobal(int x, int y)
{
    return SDL_Unsupported();
}

static int
MIR_SetRelativeMouseMode(SDL_bool enabled)
{
    return 0;
}

/* TODO Actually implement the cursor, need to wait for mir support */
void
MIR_InitMouse()
{
    SDL_Mouse* mouse = SDL_GetMouse();

    mouse->CreateCursor         = MIR_CreateCursor;
    mouse->ShowCursor           = MIR_ShowCursor;
    mouse->FreeCursor           = MIR_FreeCursor;
    mouse->WarpMouse            = MIR_WarpMouse;
    mouse->WarpMouseGlobal      = MIR_WarpMouseGlobal;
    mouse->CreateSystemCursor   = MIR_CreateSystemCursor;
    mouse->SetRelativeMouseMode = MIR_SetRelativeMouseMode;

    SDL_SetDefaultCursor(MIR_CreateDefaultCursor());
}

void
MIR_FiniMouse()
{
}

#endif /* SDL_VIDEO_DRIVER_MIR */

/* vi: set ts=4 sw=4 expandtab: */

