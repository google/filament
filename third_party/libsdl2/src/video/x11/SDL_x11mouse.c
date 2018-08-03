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

#if SDL_VIDEO_DRIVER_X11

#include <X11/cursorfont.h>
#include "SDL_assert.h"
#include "SDL_x11video.h"
#include "SDL_x11mouse.h"
#include "SDL_x11xinput2.h"
#include "../../events/SDL_mouse_c.h"


/* FIXME: Find a better place to put this... */
static Cursor x11_empty_cursor = None;

static Display *
GetDisplay(void)
{
    return ((SDL_VideoData *)SDL_GetVideoDevice()->driverdata)->display;
}

static Cursor
X11_CreateEmptyCursor()
{
    if (x11_empty_cursor == None) {
        Display *display = GetDisplay();
        char data[1];
        XColor color;
        Pixmap pixmap;

        SDL_zero(data);
        color.red = color.green = color.blue = 0;
        pixmap = X11_XCreateBitmapFromData(display, DefaultRootWindow(display),
                                       data, 1, 1);
        if (pixmap) {
            x11_empty_cursor = X11_XCreatePixmapCursor(display, pixmap, pixmap,
                                                   &color, &color, 0, 0);
            X11_XFreePixmap(display, pixmap);
        }
    }
    return x11_empty_cursor;
}

static void
X11_DestroyEmptyCursor(void)
{
    if (x11_empty_cursor != None) {
        X11_XFreeCursor(GetDisplay(), x11_empty_cursor);
        x11_empty_cursor = None;
    }
}

static SDL_Cursor *
X11_CreateDefaultCursor()
{
    SDL_Cursor *cursor;

    cursor = SDL_calloc(1, sizeof(*cursor));
    if (cursor) {
        /* None is used to indicate the default cursor */
        cursor->driverdata = (void*)None;
    } else {
        SDL_OutOfMemory();
    }

    return cursor;
}

#if SDL_VIDEO_DRIVER_X11_XCURSOR
static Cursor
X11_CreateXCursorCursor(SDL_Surface * surface, int hot_x, int hot_y)
{
    Display *display = GetDisplay();
    Cursor cursor = None;
    XcursorImage *image;

    image = X11_XcursorImageCreate(surface->w, surface->h);
    if (!image) {
        SDL_OutOfMemory();
        return None;
    }
    image->xhot = hot_x;
    image->yhot = hot_y;
    image->delay = 0;

    SDL_assert(surface->format->format == SDL_PIXELFORMAT_ARGB8888);
    SDL_assert(surface->pitch == surface->w * 4);
    SDL_memcpy(image->pixels, surface->pixels, surface->h * surface->pitch);

    cursor = X11_XcursorImageLoadCursor(display, image);

    X11_XcursorImageDestroy(image);

    return cursor;
}
#endif /* SDL_VIDEO_DRIVER_X11_XCURSOR */

static Cursor
X11_CreatePixmapCursor(SDL_Surface * surface, int hot_x, int hot_y)
{
    Display *display = GetDisplay();
    XColor fg, bg;
    Cursor cursor = None;
    Uint32 *ptr;
    Uint8 *data_bits, *mask_bits;
    Pixmap data_pixmap, mask_pixmap;
    int x, y;
    unsigned int rfg, gfg, bfg, rbg, gbg, bbg, fgBits, bgBits;
    unsigned int width_bytes = ((surface->w + 7) & ~7) / 8;

    data_bits = SDL_calloc(1, surface->h * width_bytes);
    if (!data_bits) {
        SDL_OutOfMemory();
        return None;
    }

    mask_bits = SDL_calloc(1, surface->h * width_bytes);
    if (!mask_bits) {
        SDL_free(data_bits);
        SDL_OutOfMemory();
        return None;
    }

    /* Code below assumes ARGB pixel format */
    SDL_assert(surface->format->format == SDL_PIXELFORMAT_ARGB8888);

    rfg = gfg = bfg = rbg = gbg = bbg = fgBits = bgBits = 0;
    for (y = 0; y < surface->h; ++y) {
        ptr = (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch);
        for (x = 0; x < surface->w; ++x) {
            int alpha = (*ptr >> 24) & 0xff;
            int red   = (*ptr >> 16) & 0xff;
            int green = (*ptr >> 8) & 0xff;
            int blue  = (*ptr >> 0) & 0xff;
            if (alpha > 25) {
                mask_bits[y * width_bytes + x / 8] |= (0x01 << (x % 8));

                if ((red + green + blue) > 0x40) {
                    fgBits++;
                    rfg += red;
                    gfg += green;
                    bfg += blue;
                    data_bits[y * width_bytes + x / 8] |= (0x01 << (x % 8));
                } else {
                    bgBits++;
                    rbg += red;
                    gbg += green;
                    bbg += blue;
                }
            }
            ++ptr;
        }
    }

    if (fgBits) {
        fg.red   = rfg * 257 / fgBits;
        fg.green = gfg * 257 / fgBits;
        fg.blue  = bfg * 257 / fgBits;
    }
    else fg.red = fg.green = fg.blue = 0;

    if (bgBits) {
        bg.red   = rbg * 257 / bgBits;
        bg.green = gbg * 257 / bgBits;
        bg.blue  = bbg * 257 / bgBits;
    }
    else bg.red = bg.green = bg.blue = 0;

    data_pixmap = X11_XCreateBitmapFromData(display, DefaultRootWindow(display),
                                        (char*)data_bits,
                                        surface->w, surface->h);
    mask_pixmap = X11_XCreateBitmapFromData(display, DefaultRootWindow(display),
                                        (char*)mask_bits,
                                        surface->w, surface->h);
    cursor = X11_XCreatePixmapCursor(display, data_pixmap, mask_pixmap,
                                 &fg, &bg, hot_x, hot_y);
    X11_XFreePixmap(display, data_pixmap);
    X11_XFreePixmap(display, mask_pixmap);

    return cursor;
}

static SDL_Cursor *
X11_CreateCursor(SDL_Surface * surface, int hot_x, int hot_y)
{
    SDL_Cursor *cursor;

    cursor = SDL_calloc(1, sizeof(*cursor));
    if (cursor) {
        Cursor x11_cursor = None;

#if SDL_VIDEO_DRIVER_X11_XCURSOR
        if (SDL_X11_HAVE_XCURSOR) {
            x11_cursor = X11_CreateXCursorCursor(surface, hot_x, hot_y);
        }
#endif
        if (x11_cursor == None) {
            x11_cursor = X11_CreatePixmapCursor(surface, hot_x, hot_y);
        }
        cursor->driverdata = (void*)x11_cursor;
    } else {
        SDL_OutOfMemory();
    }

    return cursor;
}

static SDL_Cursor *
X11_CreateSystemCursor(SDL_SystemCursor id)
{
    SDL_Cursor *cursor;
    unsigned int shape;

    switch(id)
    {
    default:
        SDL_assert(0);
        return NULL;
    /* X Font Cursors reference: */
    /*   http://tronche.com/gui/x/xlib/appendix/b/ */
    case SDL_SYSTEM_CURSOR_ARROW:     shape = XC_left_ptr; break;
    case SDL_SYSTEM_CURSOR_IBEAM:     shape = XC_xterm; break;
    case SDL_SYSTEM_CURSOR_WAIT:      shape = XC_watch; break;
    case SDL_SYSTEM_CURSOR_CROSSHAIR: shape = XC_tcross; break;
    case SDL_SYSTEM_CURSOR_WAITARROW: shape = XC_watch; break;
    case SDL_SYSTEM_CURSOR_SIZENWSE:  shape = XC_fleur; break;
    case SDL_SYSTEM_CURSOR_SIZENESW:  shape = XC_fleur; break;
    case SDL_SYSTEM_CURSOR_SIZEWE:    shape = XC_sb_h_double_arrow; break;
    case SDL_SYSTEM_CURSOR_SIZENS:    shape = XC_sb_v_double_arrow; break;
    case SDL_SYSTEM_CURSOR_SIZEALL:   shape = XC_fleur; break;
    case SDL_SYSTEM_CURSOR_NO:        shape = XC_pirate; break;
    case SDL_SYSTEM_CURSOR_HAND:      shape = XC_hand2; break;
    }

    cursor = SDL_calloc(1, sizeof(*cursor));
    if (cursor) {
        Cursor x11_cursor;

        x11_cursor = X11_XCreateFontCursor(GetDisplay(), shape);

        cursor->driverdata = (void*)x11_cursor;
    } else {
        SDL_OutOfMemory();
    }

    return cursor;
}

static void
X11_FreeCursor(SDL_Cursor * cursor)
{
    Cursor x11_cursor = (Cursor)cursor->driverdata;

    if (x11_cursor != None) {
        X11_XFreeCursor(GetDisplay(), x11_cursor);
    }
    SDL_free(cursor);
}

static int
X11_ShowCursor(SDL_Cursor * cursor)
{
    Cursor x11_cursor = 0;

    if (cursor) {
        x11_cursor = (Cursor)cursor->driverdata;
    } else {
        x11_cursor = X11_CreateEmptyCursor();
    }

    /* FIXME: Is there a better way than this? */
    {
        SDL_VideoDevice *video = SDL_GetVideoDevice();
        Display *display = GetDisplay();
        SDL_Window *window;
        SDL_WindowData *data;

        for (window = video->windows; window; window = window->next) {
            data = (SDL_WindowData *)window->driverdata;
            if (x11_cursor != None) {
                X11_XDefineCursor(display, data->xwindow, x11_cursor);
            } else {
                X11_XUndefineCursor(display, data->xwindow);
            }
        }
        X11_XFlush(display);
    }
    return 0;
}

static void
WarpMouseInternal(Window xwindow, const int x, const int y)
{
    SDL_VideoData *videodata = (SDL_VideoData *) SDL_GetVideoDevice()->driverdata;
    Display *display = videodata->display;
    X11_XWarpPointer(display, None, xwindow, 0, 0, 0, 0, x, y);
    X11_XSync(display, False);
    videodata->global_mouse_changed = SDL_TRUE;
}

static void
X11_WarpMouse(SDL_Window * window, int x, int y)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    WarpMouseInternal(data->xwindow, x, y);
}

static int
X11_WarpMouseGlobal(int x, int y)
{
    WarpMouseInternal(DefaultRootWindow(GetDisplay()), x, y);
    return 0;
}

static int
X11_SetRelativeMouseMode(SDL_bool enabled)
{
#if SDL_VIDEO_DRIVER_X11_XINPUT2
    if(X11_Xinput2IsInitialized())
        return 0;
#else
    SDL_Unsupported();
#endif
    return -1;
}

static int
X11_CaptureMouse(SDL_Window *window)
{
    Display *display = GetDisplay();

    if (window) {
        SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
        const unsigned int mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
        const int rc = X11_XGrabPointer(display, data->xwindow, False,
                                        mask, GrabModeAsync, GrabModeAsync,
                                        None, None, CurrentTime);
        if (rc != GrabSuccess) {
            return SDL_SetError("X server refused mouse capture");
        }
    } else {
        X11_XUngrabPointer(display, CurrentTime);
    }

    X11_XSync(display, False);

    return 0;
}

static Uint32
X11_GetGlobalMouseState(int *x, int *y)
{
    SDL_VideoData *videodata = (SDL_VideoData *) SDL_GetVideoDevice()->driverdata;
    Display *display = GetDisplay();
    const int num_screens = SDL_GetNumVideoDisplays();
    int i;

    /* !!! FIXME: should we XSync() here first? */

#if !SDL_VIDEO_DRIVER_X11_XINPUT2
    videodata->global_mouse_changed = SDL_TRUE;
#endif

    /* check if we have this cached since XInput last saw the mouse move. */
    /* !!! FIXME: can we just calculate this from XInput's events? */
    if (videodata->global_mouse_changed) {
        for (i = 0; i < num_screens; i++) {
            SDL_DisplayData *data = (SDL_DisplayData *) SDL_GetDisplayDriverData(i);
            if (data != NULL) {
                Window root, child;
                int rootx, rooty, winx, winy;
                unsigned int mask;
                if (X11_XQueryPointer(display, RootWindow(display, data->screen), &root, &child, &rootx, &rooty, &winx, &winy, &mask)) {
                    XWindowAttributes root_attrs;
                    Uint32 buttons = 0;
                    buttons |= (mask & Button1Mask) ? SDL_BUTTON_LMASK : 0;
                    buttons |= (mask & Button2Mask) ? SDL_BUTTON_MMASK : 0;
                    buttons |= (mask & Button3Mask) ? SDL_BUTTON_RMASK : 0;
                    /* SDL_DisplayData->x,y point to screen origin, and adding them to mouse coordinates relative to root window doesn't do the right thing
                     * (observed on dual monitor setup with primary display being the rightmost one - mouse was offset to the right).
                     *
                     * Adding root position to root-relative coordinates seems to be a better way to get absolute position. */
                    X11_XGetWindowAttributes(display, root, &root_attrs);
                    videodata->global_mouse_position.x = root_attrs.x + rootx;
                    videodata->global_mouse_position.y = root_attrs.y + rooty;
                    videodata->global_mouse_buttons = buttons;
                    videodata->global_mouse_changed = SDL_FALSE;
                    break;
                }
            }
        }
    }

    SDL_assert(!videodata->global_mouse_changed);  /* The pointer wasn't on any X11 screen?! */

    *x = videodata->global_mouse_position.x;
    *y = videodata->global_mouse_position.y;
    return videodata->global_mouse_buttons;
}


void
X11_InitMouse(_THIS)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    mouse->CreateCursor = X11_CreateCursor;
    mouse->CreateSystemCursor = X11_CreateSystemCursor;
    mouse->ShowCursor = X11_ShowCursor;
    mouse->FreeCursor = X11_FreeCursor;
    mouse->WarpMouse = X11_WarpMouse;
    mouse->WarpMouseGlobal = X11_WarpMouseGlobal;
    mouse->SetRelativeMouseMode = X11_SetRelativeMouseMode;
    mouse->CaptureMouse = X11_CaptureMouse;
    mouse->GetGlobalMouseState = X11_GetGlobalMouseState;

    SDL_SetDefaultCursor(X11_CreateDefaultCursor());
}

void
X11_QuitMouse(_THIS)
{
    X11_DestroyEmptyCursor();
}

#endif /* SDL_VIDEO_DRIVER_X11 */

/* vi: set ts=4 sw=4 expandtab: */
