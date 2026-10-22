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

#if SDL_VIDEO_DRIVER_WINDOWS

#include "SDL_assert.h"
#include "SDL_windowsvideo.h"

#include "../../events/SDL_mouse_c.h"


HCURSOR SDL_cursor = NULL;

static int rawInputEnableCount = 0;

static int 
ToggleRawInput(SDL_bool enabled)
{
    RAWINPUTDEVICE rawMouse = { 0x01, 0x02, 0, NULL }; /* Mouse: UsagePage = 1, Usage = 2 */

    if (enabled) {
        rawInputEnableCount++;
        if (rawInputEnableCount > 1) {
            return 0;  /* already done. */
        }
    } else {
        if (rawInputEnableCount == 0) {
            return 0;  /* already done. */
        }
        rawInputEnableCount--;
        if (rawInputEnableCount > 0) {
            return 0;  /* not time to disable yet */
        }
    }

    if (!enabled) {
        rawMouse.dwFlags |= RIDEV_REMOVE;
    }

    /* (Un)register raw input for mice */
    if (RegisterRawInputDevices(&rawMouse, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {

        /* Only return an error when registering. If we unregister and fail,
           then it's probably that we unregistered twice. That's OK. */
        if (enabled) {
            return SDL_Unsupported();
        }
    }
    return 0;
}


static SDL_Cursor *
WIN_CreateDefaultCursor()
{
    SDL_Cursor *cursor;

    cursor = SDL_calloc(1, sizeof(*cursor));
    if (cursor) {
        cursor->driverdata = LoadCursor(NULL, IDC_ARROW);
    } else {
        SDL_OutOfMemory();
    }

    return cursor;
}

static SDL_Cursor *
WIN_CreateCursor(SDL_Surface * surface, int hot_x, int hot_y)
{
    /* msdn says cursor mask has to be padded out to word alignment. Not sure
        if that means machine word or WORD, but this handles either case. */
    const size_t pad = (sizeof (size_t) * 8);  /* 32 or 64, or whatever. */
    SDL_Cursor *cursor;
    HICON hicon;
    HDC hdc;
    BITMAPV4HEADER bmh;
    LPVOID pixels;
    LPVOID maskbits;
    size_t maskbitslen;
    ICONINFO ii;

    SDL_zero(bmh);
    bmh.bV4Size = sizeof(bmh);
    bmh.bV4Width = surface->w;
    bmh.bV4Height = -surface->h; /* Invert the image */
    bmh.bV4Planes = 1;
    bmh.bV4BitCount = 32;
    bmh.bV4V4Compression = BI_BITFIELDS;
    bmh.bV4AlphaMask = 0xFF000000;
    bmh.bV4RedMask   = 0x00FF0000;
    bmh.bV4GreenMask = 0x0000FF00;
    bmh.bV4BlueMask  = 0x000000FF;

    maskbitslen = ((surface->w + (pad - (surface->w % pad))) / 8) * surface->h;
    maskbits = SDL_stack_alloc(Uint8,maskbitslen);
    if (maskbits == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    /* AND the cursor against full bits: no change. We already have alpha. */
    SDL_memset(maskbits, 0xFF, maskbitslen);

    hdc = GetDC(NULL);
    SDL_zero(ii);
    ii.fIcon = FALSE;
    ii.xHotspot = (DWORD)hot_x;
    ii.yHotspot = (DWORD)hot_y;
    ii.hbmColor = CreateDIBSection(hdc, (BITMAPINFO*)&bmh, DIB_RGB_COLORS, &pixels, NULL, 0);
    ii.hbmMask = CreateBitmap(surface->w, surface->h, 1, 1, maskbits);
    ReleaseDC(NULL, hdc);
    SDL_stack_free(maskbits);

    SDL_assert(surface->format->format == SDL_PIXELFORMAT_ARGB8888);
    SDL_assert(surface->pitch == surface->w * 4);
    SDL_memcpy(pixels, surface->pixels, surface->h * surface->pitch);

    hicon = CreateIconIndirect(&ii);

    DeleteObject(ii.hbmColor);
    DeleteObject(ii.hbmMask);

    if (!hicon) {
        WIN_SetError("CreateIconIndirect()");
        return NULL;
    }

    cursor = SDL_calloc(1, sizeof(*cursor));
    if (cursor) {
        cursor->driverdata = hicon;
    } else {
        DestroyIcon(hicon);
        SDL_OutOfMemory();
    }

    return cursor;
}

static SDL_Cursor *
WIN_CreateSystemCursor(SDL_SystemCursor id)
{
    SDL_Cursor *cursor;
    LPCTSTR name;

    switch(id)
    {
    default:
        SDL_assert(0);
        return NULL;
    case SDL_SYSTEM_CURSOR_ARROW:     name = IDC_ARROW; break;
    case SDL_SYSTEM_CURSOR_IBEAM:     name = IDC_IBEAM; break;
    case SDL_SYSTEM_CURSOR_WAIT:      name = IDC_WAIT; break;
    case SDL_SYSTEM_CURSOR_CROSSHAIR: name = IDC_CROSS; break;
    case SDL_SYSTEM_CURSOR_WAITARROW: name = IDC_WAIT; break;
    case SDL_SYSTEM_CURSOR_SIZENWSE:  name = IDC_SIZENWSE; break;
    case SDL_SYSTEM_CURSOR_SIZENESW:  name = IDC_SIZENESW; break;
    case SDL_SYSTEM_CURSOR_SIZEWE:    name = IDC_SIZEWE; break;
    case SDL_SYSTEM_CURSOR_SIZENS:    name = IDC_SIZENS; break;
    case SDL_SYSTEM_CURSOR_SIZEALL:   name = IDC_SIZEALL; break;
    case SDL_SYSTEM_CURSOR_NO:        name = IDC_NO; break;
    case SDL_SYSTEM_CURSOR_HAND:      name = IDC_HAND; break;
    }

    cursor = SDL_calloc(1, sizeof(*cursor));
    if (cursor) {
        HICON hicon;

        hicon = LoadCursor(NULL, name);

        cursor->driverdata = hicon;
    } else {
        SDL_OutOfMemory();
    }

    return cursor;
}

static void
WIN_FreeCursor(SDL_Cursor * cursor)
{
    HICON hicon = (HICON)cursor->driverdata;

    DestroyIcon(hicon);
    SDL_free(cursor);
}

static int
WIN_ShowCursor(SDL_Cursor * cursor)
{
    if (cursor) {
        SDL_cursor = (HCURSOR)cursor->driverdata;
    } else {
        SDL_cursor = NULL;
    }
    if (SDL_GetMouseFocus() != NULL) {
        SetCursor(SDL_cursor);
    }
    return 0;
}

static void
WIN_WarpMouse(SDL_Window * window, int x, int y)
{
    SDL_WindowData *data = (SDL_WindowData *)window->driverdata;
    HWND hwnd = data->hwnd;
    POINT pt;

    /* Don't warp the mouse while we're doing a modal interaction */
    if (data->in_title_click || data->focus_click_pending) {
        return;
    }

    pt.x = x;
    pt.y = y;
    ClientToScreen(hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
}

static int
WIN_WarpMouseGlobal(int x, int y)
{
    POINT pt;

    pt.x = x;
    pt.y = y;
    SetCursorPos(pt.x, pt.y);
    return 0;
}

static int
WIN_SetRelativeMouseMode(SDL_bool enabled)
{
    return ToggleRawInput(enabled);
}

static int
WIN_CaptureMouse(SDL_Window *window)
{
    if (!window) {
        SDL_Window *focusWin = SDL_GetKeyboardFocus();
        if (focusWin) {
            WIN_OnWindowEnter(SDL_GetVideoDevice(), focusWin);  /* make sure WM_MOUSELEAVE messages are (re)enabled. */
        }
    }

    /* While we were thinking of SetCapture() when designing this API in SDL,
       we didn't count on the fact that SetCapture() only tracks while the
       left mouse button is held down! Instead, we listen for raw mouse input
       and manually query the mouse when it leaves the window. :/ */
    return ToggleRawInput(window != NULL);
}

static Uint32
WIN_GetGlobalMouseState(int *x, int *y)
{
    Uint32 retval = 0;
    POINT pt = { 0, 0 };
    GetCursorPos(&pt);
    *x = (int) pt.x;
    *y = (int) pt.y;

    retval |= GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? SDL_BUTTON_LMASK : 0;
    retval |= GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? SDL_BUTTON_RMASK : 0;
    retval |= GetAsyncKeyState(VK_MBUTTON) & 0x8000 ? SDL_BUTTON_MMASK : 0;
    retval |= GetAsyncKeyState(VK_XBUTTON1) & 0x8000 ? SDL_BUTTON_X1MASK : 0;
    retval |= GetAsyncKeyState(VK_XBUTTON2) & 0x8000 ? SDL_BUTTON_X2MASK : 0;

    return retval;
}

void
WIN_InitMouse(_THIS)
{
    SDL_Mouse *mouse = SDL_GetMouse();

    mouse->CreateCursor = WIN_CreateCursor;
    mouse->CreateSystemCursor = WIN_CreateSystemCursor;
    mouse->ShowCursor = WIN_ShowCursor;
    mouse->FreeCursor = WIN_FreeCursor;
    mouse->WarpMouse = WIN_WarpMouse;
    mouse->WarpMouseGlobal = WIN_WarpMouseGlobal;
    mouse->SetRelativeMouseMode = WIN_SetRelativeMouseMode;
    mouse->CaptureMouse = WIN_CaptureMouse;
    mouse->GetGlobalMouseState = WIN_GetGlobalMouseState;

    SDL_SetDefaultCursor(WIN_CreateDefaultCursor());

    SDL_SetDoubleClickTime(GetDoubleClickTime());
}

void
WIN_QuitMouse(_THIS)
{
    if (rawInputEnableCount) {  /* force RAWINPUT off here. */
        rawInputEnableCount = 1;
        ToggleRawInput(SDL_FALSE);
    }
}

#endif /* SDL_VIDEO_DRIVER_WINDOWS */

/* vi: set ts=4 sw=4 expandtab: */
