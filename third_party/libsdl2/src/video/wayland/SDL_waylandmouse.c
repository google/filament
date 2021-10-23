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

#if SDL_VIDEO_DRIVER_WAYLAND

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "../SDL_sysvideo.h"

#include "SDL_mouse.h"
#include "../../events/SDL_mouse_c.h"
#include "SDL_waylandvideo.h"
#include "SDL_waylandevents_c.h"

#include "wayland-cursor.h"



typedef struct {
    struct wl_buffer   *buffer;
    struct wl_surface  *surface;

    int                hot_x, hot_y;
    int                w, h;

    /* Either a preloaded cursor, or one we created ourselves */
    struct wl_cursor   *cursor;
    void               *shm_data;
} Wayland_CursorData;

static int
wayland_create_tmp_file(off_t size)
{
    static const char template[] = "/sdl-shared-XXXXXX";
    char *xdg_path;
    char tmp_path[PATH_MAX];
    int fd;

    xdg_path = SDL_getenv("XDG_RUNTIME_DIR");
    if (!xdg_path) {
        return -1;
    }

    SDL_strlcpy(tmp_path, xdg_path, PATH_MAX);
    SDL_strlcat(tmp_path, template, PATH_MAX);

    fd = mkostemp(tmp_path, O_CLOEXEC);
    if (fd < 0)
        return -1;

    if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static void
mouse_buffer_release(void *data, struct wl_buffer *buffer)
{
}

static const struct wl_buffer_listener mouse_buffer_listener = {
    mouse_buffer_release
};

static int
create_buffer_from_shm(Wayland_CursorData *d,
                       int width,
                       int height,
                       uint32_t format)
{
    SDL_VideoDevice *vd = SDL_GetVideoDevice();
    SDL_VideoData *data = (SDL_VideoData *) vd->driverdata;
    struct wl_shm_pool *shm_pool;

    int stride = width * 4;
    int size = stride * height;

    int shm_fd;

    shm_fd = wayland_create_tmp_file(size);
    if (shm_fd < 0)
    {
        return SDL_SetError("Creating mouse cursor buffer failed.");
    }

    d->shm_data = mmap(NULL,
                       size,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED,
                       shm_fd,
                       0);
    if (d->shm_data == MAP_FAILED) {
        d->shm_data = NULL;
        close (shm_fd);
        return SDL_SetError("mmap() failed.");
    }

    SDL_assert(d->shm_data != NULL);

    shm_pool = wl_shm_create_pool(data->shm, shm_fd, size);
    d->buffer = wl_shm_pool_create_buffer(shm_pool,
                                          0,
                                          width,
                                          height,
                                          stride,
                                          format);
    wl_buffer_add_listener(d->buffer,
                           &mouse_buffer_listener,
                           d);

    wl_shm_pool_destroy (shm_pool);
    close (shm_fd);

    return 0;
}

static SDL_Cursor *
Wayland_CreateCursor(SDL_Surface *surface, int hot_x, int hot_y)
{
    SDL_Cursor *cursor;

    cursor = SDL_calloc(1, sizeof (*cursor));
    if (cursor) {
        SDL_VideoDevice *vd = SDL_GetVideoDevice ();
        SDL_VideoData *wd = (SDL_VideoData *) vd->driverdata;
        Wayland_CursorData *data = SDL_calloc (1, sizeof (Wayland_CursorData));
        if (!data) {
            SDL_OutOfMemory();
            SDL_free(cursor);
            return NULL;
        }
        cursor->driverdata = (void *) data;

        /* Assume ARGB8888 */
        SDL_assert(surface->format->format == SDL_PIXELFORMAT_ARGB8888);
        SDL_assert(surface->pitch == surface->w * 4);

        /* Allocate shared memory buffer for this cursor */
        if (create_buffer_from_shm (data,
                                    surface->w,
                                    surface->h,
                                    WL_SHM_FORMAT_ARGB8888) < 0)
        {
            SDL_free (cursor->driverdata);
            SDL_free (cursor);
            return NULL;
        }

        SDL_memcpy(data->shm_data,
                   surface->pixels,
                   surface->h * surface->pitch);

        data->surface = wl_compositor_create_surface(wd->compositor);
        wl_surface_set_user_data(data->surface, NULL);

        data->hot_x = hot_x;
        data->hot_y = hot_y;
        data->w = surface->w;
        data->h = surface->h;
    } else {
        SDL_OutOfMemory();
    }

    return cursor;
}

static SDL_Cursor *
CreateCursorFromWlCursor(SDL_VideoData *d, struct wl_cursor *wlcursor)
{
    SDL_Cursor *cursor;

    cursor = SDL_calloc(1, sizeof (*cursor));
    if (cursor) {
        Wayland_CursorData *data = SDL_calloc (1, sizeof (Wayland_CursorData));
        if (!data) {
            SDL_OutOfMemory();
            SDL_free(cursor);
            return NULL;
        }
        cursor->driverdata = (void *) data;

        data->buffer = WAYLAND_wl_cursor_image_get_buffer(wlcursor->images[0]);
        data->surface = wl_compositor_create_surface(d->compositor);
        wl_surface_set_user_data(data->surface, NULL);
        data->hot_x = wlcursor->images[0]->hotspot_x;
        data->hot_y = wlcursor->images[0]->hotspot_y;
        data->w = wlcursor->images[0]->width;
        data->h = wlcursor->images[0]->height;
        data->cursor= wlcursor;
    } else {
        SDL_OutOfMemory ();
    }

    return cursor;
}

static SDL_Cursor *
Wayland_CreateDefaultCursor()
{
    SDL_VideoDevice *device = SDL_GetVideoDevice();
    SDL_VideoData *data = device->driverdata;

    return CreateCursorFromWlCursor (data,
                                     WAYLAND_wl_cursor_theme_get_cursor(data->cursor_theme,
                                                                "left_ptr"));
}

static SDL_Cursor *
Wayland_CreateSystemCursor(SDL_SystemCursor id)
{
    SDL_VideoDevice *vd = SDL_GetVideoDevice();
    SDL_VideoData *d = vd->driverdata;

    struct wl_cursor *cursor = NULL;

    switch(id)
    {
    default:
        SDL_assert(0);
        return NULL;
    case SDL_SYSTEM_CURSOR_ARROW:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "left_ptr");
        break;
    case SDL_SYSTEM_CURSOR_IBEAM:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "xterm");
        break;
    case SDL_SYSTEM_CURSOR_WAIT:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "watch");
        break;
    case SDL_SYSTEM_CURSOR_CROSSHAIR:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "hand1");
        break;
    case SDL_SYSTEM_CURSOR_WAITARROW:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "watch");
        break;
    case SDL_SYSTEM_CURSOR_SIZENWSE:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "hand1");
        break;
    case SDL_SYSTEM_CURSOR_SIZENESW:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "hand1");
        break;
    case SDL_SYSTEM_CURSOR_SIZEWE:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "hand1");
        break;
    case SDL_SYSTEM_CURSOR_SIZENS:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "hand1");
        break;
    case SDL_SYSTEM_CURSOR_SIZEALL:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "hand1");
        break;
    case SDL_SYSTEM_CURSOR_NO:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "xterm");
        break;
    case SDL_SYSTEM_CURSOR_HAND:
        cursor = WAYLAND_wl_cursor_theme_get_cursor(d->cursor_theme, "hand1");
        break;
    }

    return CreateCursorFromWlCursor(d, cursor);
}

static void
Wayland_FreeCursor(SDL_Cursor *cursor)
{
    Wayland_CursorData *d;

    if (!cursor)
        return;

    d = cursor->driverdata;

    /* Probably not a cursor we own */
    if (!d)
        return;

    if (d->buffer && !d->cursor)
        wl_buffer_destroy(d->buffer);

    if (d->surface)
        wl_surface_destroy(d->surface);

    /* Not sure what's meant to happen to shm_data */
    SDL_free (cursor->driverdata);
    SDL_free(cursor);
}

static int
Wayland_ShowCursor(SDL_Cursor *cursor)
{
    SDL_VideoDevice *vd = SDL_GetVideoDevice();
    SDL_VideoData *d = vd->driverdata;
    struct SDL_WaylandInput *input = d->input;

    struct wl_pointer *pointer = d->pointer;

    if (!pointer)
        return -1;

    if (cursor)
    {
        Wayland_CursorData *data = cursor->driverdata;

        wl_pointer_set_cursor (pointer,
                               input->pointer_enter_serial,
                               data->surface,
                               data->hot_x,
                               data->hot_y);
        wl_surface_attach(data->surface, data->buffer, 0, 0);
        wl_surface_damage(data->surface, 0, 0, data->w, data->h);
        wl_surface_commit(data->surface);
    }
    else
    {
        wl_pointer_set_cursor (pointer,
                               input->pointer_enter_serial,
                               NULL,
                               0,
                               0);
    }
    
    return 0;
}

static void
Wayland_WarpMouse(SDL_Window *window, int x, int y)
{
    SDL_Unsupported();
}

static int
Wayland_WarpMouseGlobal(int x, int y)
{
    return SDL_Unsupported();
}

static int
Wayland_SetRelativeMouseMode(SDL_bool enabled)
{
    SDL_VideoDevice *vd = SDL_GetVideoDevice();
    SDL_VideoData *data = (SDL_VideoData *) vd->driverdata;

    if (enabled)
        return Wayland_input_lock_pointer(data->input);
    else
        return Wayland_input_unlock_pointer(data->input);
}

void
Wayland_InitMouse(void)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    mouse->CreateCursor = Wayland_CreateCursor;
    mouse->CreateSystemCursor = Wayland_CreateSystemCursor;
    mouse->ShowCursor = Wayland_ShowCursor;
    mouse->FreeCursor = Wayland_FreeCursor;
    mouse->WarpMouse = Wayland_WarpMouse;
    mouse->WarpMouseGlobal = Wayland_WarpMouseGlobal;
    mouse->SetRelativeMouseMode = Wayland_SetRelativeMouseMode;

    SDL_SetDefaultCursor(Wayland_CreateDefaultCursor());
}

void
Wayland_FiniMouse(void)
{
    /* This effectively assumes that nobody else
     * touches SDL_Mouse which is effectively
     * a singleton */
}
#endif  /* SDL_VIDEO_DRIVER_WAYLAND */
