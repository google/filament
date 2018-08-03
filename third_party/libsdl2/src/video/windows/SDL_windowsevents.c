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

#include "SDL_windowsvideo.h"
#include "SDL_windowsshape.h"
#include "SDL_system.h"
#include "SDL_syswm.h"
#include "SDL_timer.h"
#include "SDL_vkeys.h"
#include "SDL_hints.h"
#include "../../events/SDL_events_c.h"
#include "../../events/SDL_touch_c.h"
#include "../../events/scancodes_windows.h"
#include "SDL_assert.h"
#include "SDL_hints.h"

/* Dropfile support */
#include <shellapi.h>

/* For GET_X_LPARAM, GET_Y_LPARAM. */
#include <windowsx.h>

/* #define WMMSG_DEBUG */
#ifdef WMMSG_DEBUG
#include <stdio.h>
#include "wmmsg.h"
#endif

/* For processing mouse WM_*BUTTON* and WM_MOUSEMOVE message-data from GetMessageExtraInfo() */
#define MOUSEEVENTF_FROMTOUCH 0xFF515700

/* Masks for processing the windows KEYDOWN and KEYUP messages */
#define REPEATED_KEYMASK    (1<<30)
#define EXTENDED_KEYMASK    (1<<24)

#define VK_ENTER    10          /* Keypad Enter ... no VKEY defined? */
#ifndef VK_OEM_NEC_EQUAL
#define VK_OEM_NEC_EQUAL 0x92
#endif

/* Make sure XBUTTON stuff is defined that isn't in older Platform SDKs... */
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP 0x020C
#endif
#ifndef GET_XBUTTON_WPARAM
#define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#endif
#ifndef WM_INPUT
#define WM_INPUT 0x00ff
#endif
#ifndef WM_TOUCH
#define WM_TOUCH 0x0240
#endif
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef WM_UNICHAR
#define WM_UNICHAR 0x0109
#endif

static SDL_Scancode
VKeytoScancode(WPARAM vkey)
{
    switch (vkey) {
    case VK_CLEAR: return SDL_SCANCODE_CLEAR;
    case VK_MODECHANGE: return SDL_SCANCODE_MODE;
    case VK_SELECT: return SDL_SCANCODE_SELECT;
    case VK_EXECUTE: return SDL_SCANCODE_EXECUTE;
    case VK_HELP: return SDL_SCANCODE_HELP;
    case VK_PAUSE: return SDL_SCANCODE_PAUSE;
    case VK_NUMLOCK: return SDL_SCANCODE_NUMLOCKCLEAR;

    case VK_F13: return SDL_SCANCODE_F13;
    case VK_F14: return SDL_SCANCODE_F14;
    case VK_F15: return SDL_SCANCODE_F15;
    case VK_F16: return SDL_SCANCODE_F16;
    case VK_F17: return SDL_SCANCODE_F17;
    case VK_F18: return SDL_SCANCODE_F18;
    case VK_F19: return SDL_SCANCODE_F19;
    case VK_F20: return SDL_SCANCODE_F20;
    case VK_F21: return SDL_SCANCODE_F21;
    case VK_F22: return SDL_SCANCODE_F22;
    case VK_F23: return SDL_SCANCODE_F23;
    case VK_F24: return SDL_SCANCODE_F24;

    case VK_OEM_NEC_EQUAL: return SDL_SCANCODE_KP_EQUALS;
    case VK_BROWSER_BACK: return SDL_SCANCODE_AC_BACK;
    case VK_BROWSER_FORWARD: return SDL_SCANCODE_AC_FORWARD;
    case VK_BROWSER_REFRESH: return SDL_SCANCODE_AC_REFRESH;
    case VK_BROWSER_STOP: return SDL_SCANCODE_AC_STOP;
    case VK_BROWSER_SEARCH: return SDL_SCANCODE_AC_SEARCH;
    case VK_BROWSER_FAVORITES: return SDL_SCANCODE_AC_BOOKMARKS;
    case VK_BROWSER_HOME: return SDL_SCANCODE_AC_HOME;
    case VK_VOLUME_MUTE: return SDL_SCANCODE_AUDIOMUTE;
    case VK_VOLUME_DOWN: return SDL_SCANCODE_VOLUMEDOWN;
    case VK_VOLUME_UP: return SDL_SCANCODE_VOLUMEUP;

    case VK_MEDIA_NEXT_TRACK: return SDL_SCANCODE_AUDIONEXT;
    case VK_MEDIA_PREV_TRACK: return SDL_SCANCODE_AUDIOPREV;
    case VK_MEDIA_STOP: return SDL_SCANCODE_AUDIOSTOP;
    case VK_MEDIA_PLAY_PAUSE: return SDL_SCANCODE_AUDIOPLAY;
    case VK_LAUNCH_MAIL: return SDL_SCANCODE_MAIL;
    case VK_LAUNCH_MEDIA_SELECT: return SDL_SCANCODE_MEDIASELECT;

    case VK_OEM_102: return SDL_SCANCODE_NONUSBACKSLASH;

    case VK_ATTN: return SDL_SCANCODE_SYSREQ;
    case VK_CRSEL: return SDL_SCANCODE_CRSEL;
    case VK_EXSEL: return SDL_SCANCODE_EXSEL;
    case VK_OEM_CLEAR: return SDL_SCANCODE_CLEAR;

    case VK_LAUNCH_APP1: return SDL_SCANCODE_APP1;
    case VK_LAUNCH_APP2: return SDL_SCANCODE_APP2;

    default: return SDL_SCANCODE_UNKNOWN;
    }
}

static SDL_Scancode
WindowsScanCodeToSDLScanCode(LPARAM lParam, WPARAM wParam)
{
    SDL_Scancode code;
    int nScanCode = (lParam >> 16) & 0xFF;
    SDL_bool bIsExtended = (lParam & (1 << 24)) != 0;

    code = VKeytoScancode(wParam);

    if (code == SDL_SCANCODE_UNKNOWN && nScanCode <= 127) {
        code = windows_scancode_table[nScanCode];

        if (bIsExtended) {
            switch (code) {
            case SDL_SCANCODE_RETURN:
                code = SDL_SCANCODE_KP_ENTER;
                break;
            case SDL_SCANCODE_LALT:
                code = SDL_SCANCODE_RALT;
                break;
            case SDL_SCANCODE_LCTRL:
                code = SDL_SCANCODE_RCTRL;
                break;
            case SDL_SCANCODE_SLASH:
                code = SDL_SCANCODE_KP_DIVIDE;
                break;
            case SDL_SCANCODE_CAPSLOCK:
                code = SDL_SCANCODE_KP_PLUS;
                break;
            default:
                break;
            }
        } else {
            switch (code) {
            case SDL_SCANCODE_HOME:
                code = SDL_SCANCODE_KP_7;
                break;
            case SDL_SCANCODE_UP:
                code = SDL_SCANCODE_KP_8;
                break;
            case SDL_SCANCODE_PAGEUP:
                code = SDL_SCANCODE_KP_9;
                break;
            case SDL_SCANCODE_LEFT:
                code = SDL_SCANCODE_KP_4;
                break;
            case SDL_SCANCODE_RIGHT:
                code = SDL_SCANCODE_KP_6;
                break;
            case SDL_SCANCODE_END:
                code = SDL_SCANCODE_KP_1;
                break;
            case SDL_SCANCODE_DOWN:
                code = SDL_SCANCODE_KP_2;
                break;
            case SDL_SCANCODE_PAGEDOWN:
                code = SDL_SCANCODE_KP_3;
                break;
            case SDL_SCANCODE_INSERT:
                code = SDL_SCANCODE_KP_0;
                break;
            case SDL_SCANCODE_DELETE:
                code = SDL_SCANCODE_KP_PERIOD;
                break;
            case SDL_SCANCODE_PRINTSCREEN:
                code = SDL_SCANCODE_KP_MULTIPLY;
                break;
            default:
                break;
            }
        }
    }
    return code;
}

static SDL_bool
WIN_ShouldIgnoreFocusClick()
{
    return !SDL_GetHintBoolean(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, SDL_FALSE);
}

static void
WIN_CheckWParamMouseButton(SDL_bool bwParamMousePressed, SDL_bool bSDLMousePressed, SDL_WindowData *data, Uint8 button, SDL_MouseID mouseID)
{
    if (data->focus_click_pending & SDL_BUTTON(button)) {
        /* Ignore the button click for activation */
        if (!bwParamMousePressed) {
            data->focus_click_pending &= ~SDL_BUTTON(button);
            if (!data->focus_click_pending) {
                WIN_UpdateClipCursor(data->window);
            }
        }
        if (WIN_ShouldIgnoreFocusClick()) {
            return;
        }
    }

    if (bwParamMousePressed && !bSDLMousePressed) {
        SDL_SendMouseButton(data->window, mouseID, SDL_PRESSED, button);
    } else if (!bwParamMousePressed && bSDLMousePressed) {
        SDL_SendMouseButton(data->window, mouseID, SDL_RELEASED, button);
    }
}

/*
* Some windows systems fail to send a WM_LBUTTONDOWN sometimes, but each mouse move contains the current button state also
*  so this funciton reconciles our view of the world with the current buttons reported by windows
*/
static void
WIN_CheckWParamMouseButtons(WPARAM wParam, SDL_WindowData *data, SDL_MouseID mouseID)
{
    if (wParam != data->mouse_button_flags) {
        Uint32 mouseFlags = SDL_GetMouseState(NULL, NULL);
        WIN_CheckWParamMouseButton((wParam & MK_LBUTTON), (mouseFlags & SDL_BUTTON_LMASK), data, SDL_BUTTON_LEFT, mouseID);
        WIN_CheckWParamMouseButton((wParam & MK_MBUTTON), (mouseFlags & SDL_BUTTON_MMASK), data, SDL_BUTTON_MIDDLE, mouseID);
        WIN_CheckWParamMouseButton((wParam & MK_RBUTTON), (mouseFlags & SDL_BUTTON_RMASK), data, SDL_BUTTON_RIGHT, mouseID);
        WIN_CheckWParamMouseButton((wParam & MK_XBUTTON1), (mouseFlags & SDL_BUTTON_X1MASK), data, SDL_BUTTON_X1, mouseID);
        WIN_CheckWParamMouseButton((wParam & MK_XBUTTON2), (mouseFlags & SDL_BUTTON_X2MASK), data, SDL_BUTTON_X2, mouseID);
        data->mouse_button_flags = wParam;
    }
}


static void
WIN_CheckRawMouseButtons(ULONG rawButtons, SDL_WindowData *data)
{
    if (rawButtons != data->mouse_button_flags) {
        Uint32 mouseFlags = SDL_GetMouseState(NULL, NULL);
        if ((rawButtons & RI_MOUSE_BUTTON_1_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_1_DOWN), (mouseFlags & SDL_BUTTON_LMASK), data, SDL_BUTTON_LEFT, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_1_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_1_UP), (mouseFlags & SDL_BUTTON_LMASK), data, SDL_BUTTON_LEFT, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_2_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_2_DOWN), (mouseFlags & SDL_BUTTON_RMASK), data, SDL_BUTTON_RIGHT, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_2_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_2_UP), (mouseFlags & SDL_BUTTON_RMASK), data, SDL_BUTTON_RIGHT, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_3_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_3_DOWN), (mouseFlags & SDL_BUTTON_MMASK), data, SDL_BUTTON_MIDDLE, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_3_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_3_UP), (mouseFlags & SDL_BUTTON_MMASK), data, SDL_BUTTON_MIDDLE, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_4_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_4_DOWN), (mouseFlags & SDL_BUTTON_X1MASK), data, SDL_BUTTON_X1, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_4_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_4_UP), (mouseFlags & SDL_BUTTON_X1MASK), data, SDL_BUTTON_X1, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_5_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_5_DOWN), (mouseFlags & SDL_BUTTON_X2MASK), data, SDL_BUTTON_X2, 0);
        if ((rawButtons & RI_MOUSE_BUTTON_5_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_5_UP), (mouseFlags & SDL_BUTTON_X2MASK), data, SDL_BUTTON_X2, 0);
        data->mouse_button_flags = rawButtons;
    }
}

static void
WIN_CheckAsyncMouseRelease(SDL_WindowData *data)
{
    Uint32 mouseFlags;
    SHORT keyState;

    /* mouse buttons may have changed state here, we need to resync them,
       but we will get a WM_MOUSEMOVE right away which will fix things up if in non raw mode also
    */
    mouseFlags = SDL_GetMouseState(NULL, NULL);

    keyState = GetAsyncKeyState(VK_LBUTTON);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, (mouseFlags & SDL_BUTTON_LMASK), data, SDL_BUTTON_LEFT, 0);
    }
    keyState = GetAsyncKeyState(VK_RBUTTON);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, (mouseFlags & SDL_BUTTON_RMASK), data, SDL_BUTTON_RIGHT, 0);
    }
    keyState = GetAsyncKeyState(VK_MBUTTON);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, (mouseFlags & SDL_BUTTON_MMASK), data, SDL_BUTTON_MIDDLE, 0);
    }
    keyState = GetAsyncKeyState(VK_XBUTTON1);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, (mouseFlags & SDL_BUTTON_X1MASK), data, SDL_BUTTON_X1, 0);
    }
    keyState = GetAsyncKeyState(VK_XBUTTON2);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, (mouseFlags & SDL_BUTTON_X2MASK), data, SDL_BUTTON_X2, 0);
    }
    data->mouse_button_flags = 0;
}

static BOOL
WIN_ConvertUTF32toUTF8(UINT32 codepoint, char * text)
{
    if (codepoint <= 0x7F) {
        text[0] = (char) codepoint;
        text[1] = '\0';
    } else if (codepoint <= 0x7FF) {
        text[0] = 0xC0 | (char) ((codepoint >> 6) & 0x1F);
        text[1] = 0x80 | (char) (codepoint & 0x3F);
        text[2] = '\0';
    } else if (codepoint <= 0xFFFF) {
        text[0] = 0xE0 | (char) ((codepoint >> 12) & 0x0F);
        text[1] = 0x80 | (char) ((codepoint >> 6) & 0x3F);
        text[2] = 0x80 | (char) (codepoint & 0x3F);
        text[3] = '\0';
    } else if (codepoint <= 0x10FFFF) {
        text[0] = 0xF0 | (char) ((codepoint >> 18) & 0x0F);
        text[1] = 0x80 | (char) ((codepoint >> 12) & 0x3F);
        text[2] = 0x80 | (char) ((codepoint >> 6) & 0x3F);
        text[3] = 0x80 | (char) (codepoint & 0x3F);
        text[4] = '\0';
    } else {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static SDL_bool
ShouldGenerateWindowCloseOnAltF4(void)
{
    return !SDL_GetHintBoolean(SDL_HINT_WINDOWS_NO_CLOSE_ON_ALT_F4, SDL_FALSE);
}

/* Win10 "Fall Creators Update" introduced the bug that SetCursorPos() (as used by SDL_WarpMouseInWindow())
   doesn't reliably generate WM_MOUSEMOVE events anymore (see #3931) which breaks relative mouse mode via warping.
   This is used to implement a workaround.. */
static SDL_bool isWin10FCUorNewer = SDL_FALSE;

LRESULT CALLBACK
WIN_WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SDL_WindowData *data;
    LRESULT returnCode = -1;

    /* Send a SDL_SYSWMEVENT if the application wants them */
    if (SDL_GetEventState(SDL_SYSWMEVENT) == SDL_ENABLE) {
        SDL_SysWMmsg wmmsg;

        SDL_VERSION(&wmmsg.version);
        wmmsg.subsystem = SDL_SYSWM_WINDOWS;
        wmmsg.msg.win.hwnd = hwnd;
        wmmsg.msg.win.msg = msg;
        wmmsg.msg.win.wParam = wParam;
        wmmsg.msg.win.lParam = lParam;
        SDL_SendSysWMEvent(&wmmsg);
    }

    /* Get the window data for the window */
    data = (SDL_WindowData *) GetProp(hwnd, TEXT("SDL_WindowData"));
    if (!data) {
        return CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);
    }

#ifdef WMMSG_DEBUG
    {
        char message[1024];
        if (msg > MAX_WMMSG) {
            SDL_snprintf(message, sizeof(message), "Received windows message: %p UNKNOWN (%d) -- 0x%X, 0x%X\n", hwnd, msg, wParam, lParam);
        } else {
            SDL_snprintf(message, sizeof(message), "Received windows message: %p %s -- 0x%X, 0x%X\n", hwnd, wmtab[msg], wParam, lParam);
        }
        OutputDebugStringA(message);
    }
#endif /* WMMSG_DEBUG */

    if (IME_HandleMessage(hwnd, msg, wParam, &lParam, data->videodata))
        return 0;

    switch (msg) {

    case WM_SHOWWINDOW:
        {
            if (wParam) {
                SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_SHOWN, 0, 0);
            } else {
                SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_HIDDEN, 0, 0);
            }
        }
        break;

    case WM_ACTIVATE:
        {
            POINT cursorPos;
            BOOL minimized;

            minimized = HIWORD(wParam);
            if (!minimized && (LOWORD(wParam) != WA_INACTIVE)) {
                if (LOWORD(wParam) == WA_CLICKACTIVE) {
                    if (GetAsyncKeyState(VK_LBUTTON)) {
                        data->focus_click_pending |= SDL_BUTTON_LMASK;
                    }
                    if (GetAsyncKeyState(VK_RBUTTON)) {
                        data->focus_click_pending |= SDL_BUTTON_RMASK;
                    }
                    if (GetAsyncKeyState(VK_MBUTTON)) {
                        data->focus_click_pending |= SDL_BUTTON_MMASK;
                    }
                    if (GetAsyncKeyState(VK_XBUTTON1)) {
                        data->focus_click_pending |= SDL_BUTTON_X1MASK;
                    }
                    if (GetAsyncKeyState(VK_XBUTTON2)) {
                        data->focus_click_pending |= SDL_BUTTON_X2MASK;
                    }
                }
                
                SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_SHOWN, 0, 0);
                if (SDL_GetKeyboardFocus() != data->window) {
                    SDL_SetKeyboardFocus(data->window);
                }

                GetCursorPos(&cursorPos);
                ScreenToClient(hwnd, &cursorPos);
                SDL_SendMouseMotion(data->window, 0, 0, cursorPos.x, cursorPos.y);

                WIN_CheckAsyncMouseRelease(data);

                /*
                 * FIXME: Update keyboard state
                 */
                WIN_CheckClipboardUpdate(data->videodata);

                SDL_ToggleModState(KMOD_CAPS, (GetKeyState(VK_CAPITAL) & 0x0001) != 0);
                SDL_ToggleModState(KMOD_NUM, (GetKeyState(VK_NUMLOCK) & 0x0001) != 0);
            } else {
                data->in_window_deactivation = SDL_TRUE;

                if (SDL_GetKeyboardFocus() == data->window) {
                    SDL_SetKeyboardFocus(NULL);
                    WIN_ResetDeadKeys();
                }

                ClipCursor(NULL);

                data->in_window_deactivation = SDL_FALSE;
            }
        }
        returnCode = 0;
        break;

    case WM_MOUSEMOVE:
        {
            SDL_Mouse *mouse = SDL_GetMouse();
            if (!mouse->relative_mode || mouse->relative_mode_warp) {
                SDL_MouseID mouseID = (((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH) ? SDL_TOUCH_MOUSEID : 0);
                SDL_SendMouseMotion(data->window, mouseID, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                if (isWin10FCUorNewer && mouseID != SDL_TOUCH_MOUSEID && mouse->relative_mode_warp) {
                    /* To work around #3931, Win10 bug introduced in Fall Creators Update, where
                       SetCursorPos() (SDL_WarpMouseInWindow()) doesn't reliably generate mouse events anymore,
                       after each windows mouse event generate a fake event for the middle of the window
                       if relative_mode_warp is used */
                    int center_x = 0, center_y = 0;
                    SDL_GetWindowSize(data->window, &center_x, &center_y);
                    center_x /= 2;
                    center_y /= 2;
                    SDL_SendMouseMotion(data->window, mouseID, 0, center_x, center_y);
                }
            }
        }
        /* don't break here, fall through to check the wParam like the button presses */
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
        {
            SDL_Mouse *mouse = SDL_GetMouse();
            if (!mouse->relative_mode || mouse->relative_mode_warp) {
                SDL_MouseID mouseID = (((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH) ? SDL_TOUCH_MOUSEID : 0);
                WIN_CheckWParamMouseButtons(wParam, data, mouseID);
            }
        }
        break;

    case WM_INPUT:
        {
            SDL_Mouse *mouse = SDL_GetMouse();
            HRAWINPUT hRawInput = (HRAWINPUT)lParam;
            RAWINPUT inp;
            UINT size = sizeof(inp);
            const SDL_bool isRelative = mouse->relative_mode || mouse->relative_mode_warp;
            const SDL_bool isCapture = ((data->window->flags & SDL_WINDOW_MOUSE_CAPTURE) != 0);

            if (!isRelative || mouse->focus != data->window) {
                if (!isCapture) {
                    break;
                }
            }

            GetRawInputData(hRawInput, RID_INPUT, &inp, &size, sizeof(RAWINPUTHEADER));

            /* Mouse data */
            if (inp.header.dwType == RIM_TYPEMOUSE) {
                if (isRelative) {
                    RAWMOUSE* rawmouse = &inp.data.mouse;

                    if ((rawmouse->usFlags & 0x01) == MOUSE_MOVE_RELATIVE) {
                        SDL_SendMouseMotion(data->window, 0, 1, (int)rawmouse->lLastX, (int)rawmouse->lLastY);
                    } else {
                        /* synthesize relative moves from the abs position */
                        static SDL_Point initialMousePoint;
                        if (initialMousePoint.x == 0 && initialMousePoint.y == 0) {
                            initialMousePoint.x = rawmouse->lLastX;
                            initialMousePoint.y = rawmouse->lLastY;
                        }

                        SDL_SendMouseMotion(data->window, 0, 1, (int)(rawmouse->lLastX-initialMousePoint.x), (int)(rawmouse->lLastY-initialMousePoint.y));

                        initialMousePoint.x = rawmouse->lLastX;
                        initialMousePoint.y = rawmouse->lLastY;
                    }
                    WIN_CheckRawMouseButtons(rawmouse->usButtonFlags, data);
                } else if (isCapture) {
                    /* we check for where Windows thinks the system cursor lives in this case, so we don't really lose mouse accel, etc. */
                    POINT pt;
                    RECT hwndRect;
                    HWND currentHnd;

                    GetCursorPos(&pt);
                    currentHnd = WindowFromPoint(pt);
                    ScreenToClient(hwnd, &pt);
                    GetClientRect(hwnd, &hwndRect);

                    /* if in the window, WM_MOUSEMOVE, etc, will cover it. */
                    if(currentHnd != hwnd || pt.x < 0 || pt.y < 0 || pt.x > hwndRect.right || pt.y > hwndRect.right) {
                        SDL_SendMouseMotion(data->window, 0, 0, (int)pt.x, (int)pt.y);
                        SDL_SendMouseButton(data->window, 0, GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT);
                        SDL_SendMouseButton(data->window, 0, GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT);
                        SDL_SendMouseButton(data->window, 0, GetAsyncKeyState(VK_MBUTTON) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_MIDDLE);
                        SDL_SendMouseButton(data->window, 0, GetAsyncKeyState(VK_XBUTTON1) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_X1);
                        SDL_SendMouseButton(data->window, 0, GetAsyncKeyState(VK_XBUTTON2) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_X2);
                    }
                } else {
                    SDL_assert(0 && "Shouldn't happen");
                }
            }
        }
        break;

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
        {
            short amount = GET_WHEEL_DELTA_WPARAM(wParam);
            float fAmount = (float) amount / WHEEL_DELTA;
            if (msg == WM_MOUSEWHEEL)
                SDL_SendMouseWheel(data->window, 0, 0.0f, fAmount, SDL_MOUSEWHEEL_NORMAL);
            else
                SDL_SendMouseWheel(data->window, 0, fAmount, 0.0f, SDL_MOUSEWHEEL_NORMAL);
        }
        break;

#ifdef WM_MOUSELEAVE
    case WM_MOUSELEAVE:
        if (SDL_GetMouseFocus() == data->window && !SDL_GetMouse()->relative_mode && !(data->window->flags & SDL_WINDOW_MOUSE_CAPTURE)) {
            if (!IsIconic(hwnd)) {
                POINT cursorPos;
                GetCursorPos(&cursorPos);
                ScreenToClient(hwnd, &cursorPos);
                SDL_SendMouseMotion(data->window, 0, 0, cursorPos.x, cursorPos.y);
            }
            SDL_SetMouseFocus(NULL);
        }
        returnCode = 0;
        break;
#endif /* WM_MOUSELEAVE */

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        {
            SDL_Scancode code = WindowsScanCodeToSDLScanCode(lParam, wParam);
            const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

            /* Detect relevant keyboard shortcuts */
            if (keyboardState[SDL_SCANCODE_LALT] == SDL_PRESSED || keyboardState[SDL_SCANCODE_RALT] == SDL_PRESSED) {
                /* ALT+F4: Close window */
                if (code == SDL_SCANCODE_F4 && ShouldGenerateWindowCloseOnAltF4()) {
                    SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_CLOSE, 0, 0);
                }
            }

            if (code != SDL_SCANCODE_UNKNOWN) {
                SDL_SendKeyboardKey(SDL_PRESSED, code);
            }
        }

        returnCode = 0;
        break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
        {
            SDL_Scancode code = WindowsScanCodeToSDLScanCode(lParam, wParam);
            const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

            if (code != SDL_SCANCODE_UNKNOWN) {
                if (code == SDL_SCANCODE_PRINTSCREEN &&
                    keyboardState[code] == SDL_RELEASED) {
                    SDL_SendKeyboardKey(SDL_PRESSED, code);
                }
                SDL_SendKeyboardKey(SDL_RELEASED, code);
            }
        }
        returnCode = 0;
        break;

    case WM_UNICHAR:
        if ( wParam == UNICODE_NOCHAR ) {
            returnCode = 1;
            break;
        }
        /* otherwise fall through to below */
    case WM_CHAR:
        {
            char text[5];
            if ( WIN_ConvertUTF32toUTF8( (UINT32)wParam, text ) ) {
                SDL_SendKeyboardText( text );
            }
        }
        returnCode = 0;
        break;

#ifdef WM_INPUTLANGCHANGE
    case WM_INPUTLANGCHANGE:
        {
            WIN_UpdateKeymap();
            SDL_SendKeymapChangedEvent();
        }
        returnCode = 1;
        break;
#endif /* WM_INPUTLANGCHANGE */

    case WM_NCLBUTTONDOWN:
        {
            data->in_title_click = SDL_TRUE;
        }
        break;

    case WM_CAPTURECHANGED:
        {
            data->in_title_click = SDL_FALSE;

            /* The mouse may have been released during a modal loop */
            WIN_CheckAsyncMouseRelease(data);
        }
        break;

#ifdef WM_GETMINMAXINFO
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO *info;
            RECT size;
            int x, y;
            int w, h;
            int min_w, min_h;
            int max_w, max_h;
            BOOL constrain_max_size;

            if (SDL_IsShapedWindow(data->window))
                Win32_ResizeWindowShape(data->window);

            /* If this is an expected size change, allow it */
            if (data->expected_resize) {
                break;
            }

            /* Get the current position of our window */
            GetWindowRect(hwnd, &size);
            x = size.left;
            y = size.top;

            /* Calculate current size of our window */
            SDL_GetWindowSize(data->window, &w, &h);
            SDL_GetWindowMinimumSize(data->window, &min_w, &min_h);
            SDL_GetWindowMaximumSize(data->window, &max_w, &max_h);

            /* Store in min_w and min_h difference between current size and minimal
               size so we don't need to call AdjustWindowRectEx twice */
            min_w -= w;
            min_h -= h;
            if (max_w && max_h) {
                max_w -= w;
                max_h -= h;
                constrain_max_size = TRUE;
            } else {
                constrain_max_size = FALSE;
            }

            if (!(SDL_GetWindowFlags(data->window) & SDL_WINDOW_BORDERLESS)) {
                LONG style = GetWindowLong(hwnd, GWL_STYLE);
                /* DJM - according to the docs for GetMenu(), the
                   return value is undefined if hwnd is a child window.
                   Apparently it's too difficult for MS to check
                   inside their function, so I have to do it here.
                 */
                BOOL menu = (style & WS_CHILDWINDOW) ? FALSE : (GetMenu(hwnd) != NULL);
                size.top = 0;
                size.left = 0;
                size.bottom = h;
                size.right = w;

                AdjustWindowRectEx(&size, style, menu, 0);
                w = size.right - size.left;
                h = size.bottom - size.top;
            }

            /* Fix our size to the current size */
            info = (MINMAXINFO *) lParam;
            if (SDL_GetWindowFlags(data->window) & SDL_WINDOW_RESIZABLE) {
                info->ptMinTrackSize.x = w + min_w;
                info->ptMinTrackSize.y = h + min_h;
                if (constrain_max_size) {
                    info->ptMaxTrackSize.x = w + max_w;
                    info->ptMaxTrackSize.y = h + max_h;
                }
            } else {
                info->ptMaxSize.x = w;
                info->ptMaxSize.y = h;
                info->ptMaxPosition.x = x;
                info->ptMaxPosition.y = y;
                info->ptMinTrackSize.x = w;
                info->ptMinTrackSize.y = h;
                info->ptMaxTrackSize.x = w;
                info->ptMaxTrackSize.y = h;
            }
        }
        returnCode = 0;
        break;
#endif /* WM_GETMINMAXINFO */

    case WM_WINDOWPOSCHANGING:

        if (data->expected_resize) {
            returnCode = 0;
        }
        break;

    case WM_WINDOWPOSCHANGED:
        {
            RECT rect;
            int x, y;
            int w, h;

            if (data->initializing || data->in_border_change) {
                break;
            }

            if (!GetClientRect(hwnd, &rect) || IsRectEmpty(&rect)) {
                break;
            }
            ClientToScreen(hwnd, (LPPOINT) & rect);
            ClientToScreen(hwnd, (LPPOINT) & rect + 1);

            WIN_UpdateClipCursor(data->window);

            x = rect.left;
            y = rect.top;
            SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_MOVED, x, y);

            w = rect.right - rect.left;
            h = rect.bottom - rect.top;
            SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_RESIZED, w,
                                h);

            /* Forces a WM_PAINT event */
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_SIZE:
        {
            switch (wParam) {
            case SIZE_MAXIMIZED:
                SDL_SendWindowEvent(data->window,
                    SDL_WINDOWEVENT_RESTORED, 0, 0);
                SDL_SendWindowEvent(data->window,
                    SDL_WINDOWEVENT_MAXIMIZED, 0, 0);
                break;
            case SIZE_MINIMIZED:
                SDL_SendWindowEvent(data->window,
                    SDL_WINDOWEVENT_MINIMIZED, 0, 0);
                break;
            default:
                SDL_SendWindowEvent(data->window,
                    SDL_WINDOWEVENT_RESTORED, 0, 0);
                break;
            }
        }
        break;

    case WM_SETCURSOR:
        {
            Uint16 hittest;

            hittest = LOWORD(lParam);
            if (hittest == HTCLIENT) {
                SetCursor(SDL_cursor);
                returnCode = TRUE;
            } else if (!g_WindowFrameUsableWhileCursorHidden && !SDL_cursor) {
                SetCursor(NULL);
                returnCode = TRUE;
            }
        }
        break;

        /* We were occluded, refresh our display */
    case WM_PAINT:
        {
            RECT rect;
            if (GetUpdateRect(hwnd, &rect, FALSE)) {
                ValidateRect(hwnd, NULL);
                SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_EXPOSED,
                                    0, 0);
            }
        }
        returnCode = 0;
        break;

        /* We'll do our own drawing, prevent flicker */
    case WM_ERASEBKGND:
        {
        }
        return (1);

    case WM_SYSCOMMAND:
        {
            if ((wParam & 0xFFF0) == SC_KEYMENU) {
                return (0);
            }

#if defined(SC_SCREENSAVE) || defined(SC_MONITORPOWER)
            /* Don't start the screensaver or blank the monitor in fullscreen apps */
            if ((wParam & 0xFFF0) == SC_SCREENSAVE ||
                (wParam & 0xFFF0) == SC_MONITORPOWER) {
                if (SDL_GetVideoDevice()->suspend_screensaver) {
                    return (0);
                }
            }
#endif /* System has screensaver support */
        }
        break;

    case WM_CLOSE:
        {
            SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_CLOSE, 0, 0);
        }
        returnCode = 0;
        break;

    case WM_TOUCH:
        if (data->videodata->GetTouchInputInfo && data->videodata->CloseTouchInputHandle) {
            UINT i, num_inputs = LOWORD(wParam);
            PTOUCHINPUT inputs = SDL_stack_alloc(TOUCHINPUT, num_inputs);
            if (data->videodata->GetTouchInputInfo((HTOUCHINPUT)lParam, num_inputs, inputs, sizeof(TOUCHINPUT))) {
                RECT rect;
                float x, y;

                if (!GetClientRect(hwnd, &rect) ||
                    (rect.right == rect.left && rect.bottom == rect.top)) {
                    if (inputs) {
                        SDL_stack_free(inputs);
                    }
                    break;
                }
                ClientToScreen(hwnd, (LPPOINT) & rect);
                ClientToScreen(hwnd, (LPPOINT) & rect + 1);
                rect.top *= 100;
                rect.left *= 100;
                rect.bottom *= 100;
                rect.right *= 100;

                for (i = 0; i < num_inputs; ++i) {
                    PTOUCHINPUT input = &inputs[i];

                    const SDL_TouchID touchId = (SDL_TouchID)((size_t)input->hSource);
                    if (SDL_AddTouch(touchId, "") < 0) {
                        continue;
                    }

                    /* Get the normalized coordinates for the window */
                    x = (float)(input->x - rect.left)/(rect.right - rect.left);
                    y = (float)(input->y - rect.top)/(rect.bottom - rect.top);

                    if (input->dwFlags & TOUCHEVENTF_DOWN) {
                        SDL_SendTouch(touchId, input->dwID, SDL_TRUE, x, y, 1.0f);
                    }
                    if (input->dwFlags & TOUCHEVENTF_MOVE) {
                        SDL_SendTouchMotion(touchId, input->dwID, x, y, 1.0f);
                    }
                    if (input->dwFlags & TOUCHEVENTF_UP) {
                        SDL_SendTouch(touchId, input->dwID, SDL_FALSE, x, y, 1.0f);
                    }
                }
            }
            SDL_stack_free(inputs);

            data->videodata->CloseTouchInputHandle((HTOUCHINPUT)lParam);
            return 0;
        }
        break;

    case WM_DROPFILES:
        {
            UINT i;
            HDROP drop = (HDROP) wParam;
            UINT count = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);
            for (i = 0; i < count; ++i) {
                UINT size = DragQueryFile(drop, i, NULL, 0) + 1;
                LPTSTR buffer = SDL_stack_alloc(TCHAR, size);
                if (buffer) {
                    if (DragQueryFile(drop, i, buffer, size)) {
                        char *file = WIN_StringToUTF8(buffer);
                        SDL_SendDropFile(data->window, file);
                        SDL_free(file);
                    }
                    SDL_stack_free(buffer);
                }
            }
            SDL_SendDropComplete(data->window);
            DragFinish(drop);
            return 0;
        }
        break;

    case WM_NCCALCSIZE:
        {
            Uint32 window_flags = SDL_GetWindowFlags(data->window);
            if (wParam == TRUE && (window_flags & SDL_WINDOW_BORDERLESS) && !(window_flags & SDL_WINDOW_FULLSCREEN)) {
                /* When borderless, need to tell windows that the size of the non-client area is 0 */
                if (!(window_flags & SDL_WINDOW_RESIZABLE)) {
                    int w, h;
                    NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)lParam;
                    w = data->window->windowed.w;
                    h = data->window->windowed.h;
                    params->rgrc[0].right = params->rgrc[0].left + w;
                    params->rgrc[0].bottom = params->rgrc[0].top + h;
                }
                return 0;
            }
        }
        break;

    case WM_NCHITTEST:
        {
            SDL_Window *window = data->window;
            if (window->hit_test) {
                POINT winpoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                if (ScreenToClient(hwnd, &winpoint)) {
                    const SDL_Point point = { (int) winpoint.x, (int) winpoint.y };
                    const SDL_HitTestResult rc = window->hit_test(window, &point, window->hit_test_data);
                    switch (rc) {
                        #define POST_HIT_TEST(ret) { SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_HIT_TEST, 0, 0); return ret; }
                        case SDL_HITTEST_DRAGGABLE: POST_HIT_TEST(HTCAPTION);
                        case SDL_HITTEST_RESIZE_TOPLEFT: POST_HIT_TEST(HTTOPLEFT);
                        case SDL_HITTEST_RESIZE_TOP: POST_HIT_TEST(HTTOP);
                        case SDL_HITTEST_RESIZE_TOPRIGHT: POST_HIT_TEST(HTTOPRIGHT);
                        case SDL_HITTEST_RESIZE_RIGHT: POST_HIT_TEST(HTRIGHT);
                        case SDL_HITTEST_RESIZE_BOTTOMRIGHT: POST_HIT_TEST(HTBOTTOMRIGHT);
                        case SDL_HITTEST_RESIZE_BOTTOM: POST_HIT_TEST(HTBOTTOM);
                        case SDL_HITTEST_RESIZE_BOTTOMLEFT: POST_HIT_TEST(HTBOTTOMLEFT);
                        case SDL_HITTEST_RESIZE_LEFT: POST_HIT_TEST(HTLEFT);
                        #undef POST_HIT_TEST
                        case SDL_HITTEST_NORMAL: return HTCLIENT;
                    }
                }
                /* If we didn't return, this will call DefWindowProc below. */
            }
        }
        break;
    }

    /* If there's a window proc, assume it's going to handle messages */
    if (data->wndproc) {
        return CallWindowProc(data->wndproc, hwnd, msg, wParam, lParam);
    } else if (returnCode >= 0) {
        return returnCode;
    } else {
        return CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);
    }
}

/* A message hook called before TranslateMessage() */
static SDL_WindowsMessageHook g_WindowsMessageHook = NULL;
static void *g_WindowsMessageHookData = NULL;

void SDL_SetWindowsMessageHook(SDL_WindowsMessageHook callback, void *userdata)
{
    g_WindowsMessageHook = callback;
    g_WindowsMessageHookData = userdata;
}

void
WIN_PumpEvents(_THIS)
{
    const Uint8 *keystate;
    MSG msg;
    DWORD start_ticks = GetTickCount();

    if (g_WindowsEnableMessageLoop) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (g_WindowsMessageHook) {
                g_WindowsMessageHook(g_WindowsMessageHookData, msg.hwnd, msg.message, msg.wParam, msg.lParam);
            }

            /* Always translate the message in case it's a non-SDL window (e.g. with Qt integration) */
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            /* Make sure we don't busy loop here forever if there are lots of events coming in */
            if (SDL_TICKS_PASSED(msg.time, start_ticks)) {
                break;
            }
        }
    }

    /* Windows loses a shift KEYUP event when you have both pressed at once and let go of one.
       You won't get a KEYUP until both are released, and that keyup will only be for the second
       key you released. Take heroic measures and check the keystate as of the last handled event,
       and if we think a key is pressed when Windows doesn't, unstick it in SDL's state. */
    keystate = SDL_GetKeyboardState(NULL);
    if ((keystate[SDL_SCANCODE_LSHIFT] == SDL_PRESSED) && !(GetKeyState(VK_LSHIFT) & 0x8000)) {
        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_LSHIFT);
    }
    if ((keystate[SDL_SCANCODE_RSHIFT] == SDL_PRESSED) && !(GetKeyState(VK_RSHIFT) & 0x8000)) {
        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_RSHIFT);
    }
}

/* to work around #3931, a bug introduced in Win10 Fall Creators Update (build nr. 16299)
   we need to detect the windows version. this struct and the function below does that.
   usually this struct and the corresponding function (RtlGetVersion) are in <Ntddk.h>
   but here we just load it dynamically */
struct SDL_WIN_OSVERSIONINFOW {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR szCSDVersion[128];
};

static SDL_bool
IsWin10FCUorNewer(void)
{
    HMODULE handle = GetModuleHandleW(L"ntdll.dll");
    if (handle) {
        typedef LONG(WINAPI* RtlGetVersionPtr)(struct SDL_WIN_OSVERSIONINFOW*);
        RtlGetVersionPtr getVersionPtr = (RtlGetVersionPtr)GetProcAddress(handle, "RtlGetVersion");
        if (getVersionPtr != NULL) {
            struct SDL_WIN_OSVERSIONINFOW info;
            SDL_zero(info);
            info.dwOSVersionInfoSize = sizeof(info);
            if (getVersionPtr(&info) == 0) { /* STATUS_SUCCESS == 0 */
                if (   (info.dwMajorVersion == 10 && info.dwMinorVersion == 0 && info.dwBuildNumber >= 16299)
                    || (info.dwMajorVersion == 10 && info.dwMinorVersion > 0)
                    || (info.dwMajorVersion > 10) )
                {
                    return SDL_TRUE;
                }
            }
        }
    }
    return SDL_FALSE;
}

static int app_registered = 0;
LPTSTR SDL_Appname = NULL;
Uint32 SDL_Appstyle = 0;
HINSTANCE SDL_Instance = NULL;

/* Register the class for this application */
int
SDL_RegisterApp(char *name, Uint32 style, void *hInst)
{
    const char *hint;
    WNDCLASSEX wcex;
    TCHAR path[MAX_PATH];

    /* Only do this once... */
    if (app_registered) {
        ++app_registered;
        return (0);
    }
    if (!name && !SDL_Appname) {
        name = "SDL_app";
#if defined(CS_BYTEALIGNCLIENT) || defined(CS_OWNDC)
        SDL_Appstyle = (CS_BYTEALIGNCLIENT | CS_OWNDC);
#endif
        SDL_Instance = hInst ? hInst : GetModuleHandle(NULL);
    }

    if (name) {
        SDL_Appname = WIN_UTF8ToString(name);
        SDL_Appstyle = style;
        SDL_Instance = hInst ? hInst : GetModuleHandle(NULL);
    }

    /* Register the application class */
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.hCursor        = NULL;
    wcex.hIcon          = NULL;
    wcex.hIconSm        = NULL;
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = SDL_Appname;
    wcex.style          = SDL_Appstyle;
    wcex.hbrBackground  = NULL;
    wcex.lpfnWndProc    = WIN_WindowProc;
    wcex.hInstance      = SDL_Instance;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;

    hint = SDL_GetHint(SDL_HINT_WINDOWS_INTRESOURCE_ICON);
    if (hint && *hint) {
        wcex.hIcon = LoadIcon(SDL_Instance, MAKEINTRESOURCE(SDL_atoi(hint)));

        hint = SDL_GetHint(SDL_HINT_WINDOWS_INTRESOURCE_ICON_SMALL);
        if (hint && *hint) {
            wcex.hIconSm = LoadIcon(SDL_Instance, MAKEINTRESOURCE(SDL_atoi(hint)));
        }
    } else {
        /* Use the first icon as a default icon, like in the Explorer */
        GetModuleFileName(SDL_Instance, path, MAX_PATH);
        ExtractIconEx(path, 0, &wcex.hIcon, &wcex.hIconSm, 1);
    }

    if (!RegisterClassEx(&wcex)) {
        return SDL_SetError("Couldn't register application class");
    }

    isWin10FCUorNewer = IsWin10FCUorNewer();

    app_registered = 1;
    return 0;
}

/* Unregisters the windowclass registered in SDL_RegisterApp above. */
void
SDL_UnregisterApp()
{
    WNDCLASSEX wcex;

    /* SDL_RegisterApp might not have been called before */
    if (!app_registered) {
        return;
    }
    --app_registered;
    if (app_registered == 0) {
        /* Check for any registered window classes. */
        if (GetClassInfoEx(SDL_Instance, SDL_Appname, &wcex)) {
            UnregisterClass(SDL_Appname, SDL_Instance);
            if (wcex.hIcon) DestroyIcon(wcex.hIcon);
            if (wcex.hIconSm) DestroyIcon(wcex.hIconSm);
        }
        SDL_free(SDL_Appname);
        SDL_Appname = NULL;
    }
}

#endif /* SDL_VIDEO_DRIVER_WINDOWS */

/* vi: set ts=4 sw=4 expandtab: */
