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
#ifndef WM_POINTERUPDATE
#define WM_POINTERUPDATE 0x0245
#endif
#ifndef WM_UNICHAR
#define WM_UNICHAR 0x0109
#endif

static SDL_Scancode
VKeytoScancodeFallback(WPARAM vkey)
{
    switch (vkey) {
    case VK_LEFT: return SDL_SCANCODE_LEFT;
    case VK_UP: return SDL_SCANCODE_UP;
    case VK_RIGHT: return SDL_SCANCODE_RIGHT;
    case VK_DOWN: return SDL_SCANCODE_DOWN;

    default: return SDL_SCANCODE_UNKNOWN;
    }
}

static SDL_Scancode
VKeytoScancode(WPARAM vkey)
{
    switch (vkey) {
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

    /* The on-screen keyboard can generate VK_LEFT and VK_RIGHT events without a scancode
     * value set, however we cannot simply map these in VKeytoScancode() or we will be
     * incorrectly handling the arrow keys on the number pad when NumLock is disabled
     * (which also generate VK_LEFT, VK_RIGHT, etc in that scenario). Instead, we'll only
     * map them if none of the above special number pad mappings applied. */
    if (code == SDL_SCANCODE_UNKNOWN) {
        code = VKeytoScancodeFallback(wParam);
    }

    return code;
}

static SDL_bool
WIN_ShouldIgnoreFocusClick()
{
    return !SDL_GetHintBoolean(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, SDL_FALSE);
}

static void
WIN_CheckWParamMouseButton(SDL_bool bwParamMousePressed, Uint32 mouseFlags, SDL_bool bSwapButtons, SDL_WindowData *data, Uint8 button, SDL_MouseID mouseID)
{
    if (bSwapButtons) {
        if (button == SDL_BUTTON_LEFT) {
            button = SDL_BUTTON_RIGHT;
        }
        else if (button == SDL_BUTTON_RIGHT) {
            button = SDL_BUTTON_LEFT;
        }
    }

    if (data->focus_click_pending & SDL_BUTTON(button)) {
        /* Ignore the button click for activation */
        if (!bwParamMousePressed) {
            data->focus_click_pending &= ~SDL_BUTTON(button);
            WIN_UpdateClipCursor(data->window);
        }
        if (WIN_ShouldIgnoreFocusClick()) {
            return;
        }
    }

    if (bwParamMousePressed && !(mouseFlags & SDL_BUTTON(button))) {
        SDL_SendMouseButton(data->window, mouseID, SDL_PRESSED, button);
    } else if (!bwParamMousePressed && (mouseFlags & SDL_BUTTON(button))) {
        SDL_SendMouseButton(data->window, mouseID, SDL_RELEASED, button);
    }
}

/*
* Some windows systems fail to send a WM_LBUTTONDOWN sometimes, but each mouse move contains the current button state also
*  so this function reconciles our view of the world with the current buttons reported by windows
*/
static void
WIN_CheckWParamMouseButtons(WPARAM wParam, SDL_WindowData *data, SDL_MouseID mouseID)
{
    if (wParam != data->mouse_button_flags) {
        Uint32 mouseFlags = SDL_GetMouseState(NULL, NULL);

        /* WM_LBUTTONDOWN and friends handle button swapping for us. No need to check SM_SWAPBUTTON here.  */
        WIN_CheckWParamMouseButton((wParam & MK_LBUTTON), mouseFlags, SDL_FALSE, data, SDL_BUTTON_LEFT, mouseID);
        WIN_CheckWParamMouseButton((wParam & MK_MBUTTON), mouseFlags, SDL_FALSE, data, SDL_BUTTON_MIDDLE, mouseID);
        WIN_CheckWParamMouseButton((wParam & MK_RBUTTON), mouseFlags, SDL_FALSE, data, SDL_BUTTON_RIGHT, mouseID);
        WIN_CheckWParamMouseButton((wParam & MK_XBUTTON1), mouseFlags, SDL_FALSE, data, SDL_BUTTON_X1, mouseID);
        WIN_CheckWParamMouseButton((wParam & MK_XBUTTON2), mouseFlags, SDL_FALSE, data, SDL_BUTTON_X2, mouseID);

        data->mouse_button_flags = wParam;
    }
}

static void
WIN_CheckRawMouseButtons(ULONG rawButtons, SDL_WindowData *data, SDL_MouseID mouseID)
{
    if (rawButtons != data->mouse_button_flags) {
        Uint32 mouseFlags = SDL_GetMouseState(NULL, NULL);
        SDL_bool swapButtons = GetSystemMetrics(SM_SWAPBUTTON) != 0;
        if ((rawButtons & RI_MOUSE_BUTTON_1_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_1_DOWN), mouseFlags, swapButtons, data, SDL_BUTTON_LEFT, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_1_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_1_UP), mouseFlags, swapButtons, data, SDL_BUTTON_LEFT, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_2_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_2_DOWN), mouseFlags, swapButtons, data, SDL_BUTTON_RIGHT, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_2_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_2_UP), mouseFlags, swapButtons, data, SDL_BUTTON_RIGHT, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_3_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_3_DOWN), mouseFlags, swapButtons, data, SDL_BUTTON_MIDDLE, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_3_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_3_UP), mouseFlags, swapButtons, data, SDL_BUTTON_MIDDLE, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_4_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_4_DOWN), mouseFlags, swapButtons, data, SDL_BUTTON_X1, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_4_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_4_UP), mouseFlags, swapButtons, data, SDL_BUTTON_X1, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_5_DOWN))
            WIN_CheckWParamMouseButton((rawButtons & RI_MOUSE_BUTTON_5_DOWN), mouseFlags, swapButtons, data, SDL_BUTTON_X2, mouseID);
        if ((rawButtons & RI_MOUSE_BUTTON_5_UP))
            WIN_CheckWParamMouseButton(!(rawButtons & RI_MOUSE_BUTTON_5_UP), mouseFlags, swapButtons, data, SDL_BUTTON_X2, mouseID);
        data->mouse_button_flags = rawButtons;
    }
}

static void
WIN_CheckAsyncMouseRelease(SDL_WindowData *data)
{
    Uint32 mouseFlags;
    SHORT keyState;
    SDL_bool swapButtons;

    /* mouse buttons may have changed state here, we need to resync them,
       but we will get a WM_MOUSEMOVE right away which will fix things up if in non raw mode also
    */
    mouseFlags = SDL_GetMouseState(NULL, NULL);
    swapButtons = GetSystemMetrics(SM_SWAPBUTTON) != 0;

    keyState = GetAsyncKeyState(VK_LBUTTON);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, mouseFlags, swapButtons, data, SDL_BUTTON_LEFT, 0);
    }
    keyState = GetAsyncKeyState(VK_RBUTTON);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, mouseFlags, swapButtons, data, SDL_BUTTON_RIGHT, 0);
    }
    keyState = GetAsyncKeyState(VK_MBUTTON);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, mouseFlags, swapButtons, data, SDL_BUTTON_MIDDLE, 0);
    }
    keyState = GetAsyncKeyState(VK_XBUTTON1);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, mouseFlags, swapButtons, data, SDL_BUTTON_X1, 0);
    }
    keyState = GetAsyncKeyState(VK_XBUTTON2);
    if (!(keyState & 0x8000)) {
        WIN_CheckWParamMouseButton(SDL_FALSE, mouseFlags, swapButtons, data, SDL_BUTTON_X2, 0);
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

/* We want to generate mouse events from mouse and pen, and touch events from touchscreens */
#define MI_WP_SIGNATURE         0xFF515700
#define MI_WP_SIGNATURE_MASK    0xFFFFFF00
#define IsTouchEvent(dw) ((dw) & MI_WP_SIGNATURE_MASK) == MI_WP_SIGNATURE

typedef enum
{
    SDL_MOUSE_EVENT_SOURCE_UNKNOWN,
    SDL_MOUSE_EVENT_SOURCE_MOUSE,
    SDL_MOUSE_EVENT_SOURCE_TOUCH,
    SDL_MOUSE_EVENT_SOURCE_PEN,
} SDL_MOUSE_EVENT_SOURCE;

static SDL_MOUSE_EVENT_SOURCE GetMouseMessageSource()
{
    LPARAM extrainfo = GetMessageExtraInfo();
    /* Mouse data (ignoring synthetic mouse events generated for touchscreens) */
    /* Versions below Vista will set the low 7 bits to the Mouse ID and don't use bit 7:
       Check bits 8-32 for the signature (which will indicate a Tablet PC Pen or Touch Device).
       Only check bit 7 when Vista and up(Cleared=Pen, Set=Touch(which we need to filter out)),
       when the signature is set. The Mouse ID will be zero for an actual mouse. */
    if (IsTouchEvent(extrainfo)) {
        if (extrainfo & 0x80) {
            return SDL_MOUSE_EVENT_SOURCE_TOUCH;
        } else {
            return SDL_MOUSE_EVENT_SOURCE_PEN;
        }
    }
    return SDL_MOUSE_EVENT_SOURCE_MOUSE;
}

static SDL_WindowData *
WIN_GetWindowDataFromHWND(HWND hwnd)
{
    SDL_VideoDevice *_this = SDL_GetVideoDevice();
    SDL_Window *window;

    if (_this) {
        for (window = _this->windows; window; window = window->next) {
            SDL_WindowData *data = (SDL_WindowData *)window->driverdata;
            if (data && data->hwnd == hwnd) {
                return data;
            }
        }
    }
    return NULL;
}

LRESULT CALLBACK
WIN_KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT* hookData = (KBDLLHOOKSTRUCT*)lParam;
    SDL_VideoData* data = SDL_GetVideoDevice()->driverdata;
    SDL_Scancode scanCode;

    if (nCode < 0 || nCode != HC_ACTION) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    switch (hookData->vkCode) {
    case VK_LWIN:
        scanCode = SDL_SCANCODE_LGUI;
        break;
    case VK_RWIN:
        scanCode = SDL_SCANCODE_RGUI;
        break;
    case VK_LMENU:
        scanCode = SDL_SCANCODE_LALT;
        break;
    case VK_RMENU:
        scanCode = SDL_SCANCODE_RALT;
        break;
    case VK_LCONTROL:
        scanCode = SDL_SCANCODE_LCTRL;
        break;
    case VK_RCONTROL:
        scanCode = SDL_SCANCODE_RCTRL;
        break;

    /* These are required to intercept Alt+Tab and Alt+Esc on Windows 7 */
    case VK_TAB:
        scanCode = SDL_SCANCODE_TAB;
        break;
    case VK_ESCAPE:
        scanCode = SDL_SCANCODE_ESCAPE;
        break;

    default:
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        SDL_SendKeyboardKey(SDL_PRESSED, scanCode);
    } else {
        SDL_SendKeyboardKey(SDL_RELEASED, scanCode);

        /* If the key was down prior to our hook being installed, allow the
           key up message to pass normally the first time. This ensures other
           windows have a consistent view of the key state, and avoids keys
           being stuck down in those windows if they are down when the grab
           happens and raised while grabbed. */
        if (hookData->vkCode <= 0xFF && data->pre_hook_key_state[hookData->vkCode]) {
            data->pre_hook_key_state[hookData->vkCode] = 0;
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
    }

    return 1;
}

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
    data = WIN_GetWindowDataFromHWND(hwnd);
    if (!data) {
        /* Fallback */
        data = (SDL_WindowData *) GetProp(hwnd, TEXT("SDL_WindowData"));
    }
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

    case WM_NCACTIVATE:
        {
            /* Don't immediately clip the cursor in case we're clicking minimize/maximize buttons */
            data->skip_update_clipcursor = SDL_TRUE;
        }
        break;

    case WM_ACTIVATE:
        {
            POINT cursorPos;
            BOOL minimized;

            minimized = HIWORD(wParam);
            if (!minimized && (LOWORD(wParam) != WA_INACTIVE)) {
                /* Don't mark the window as shown if it's activated before being shown */
                if (!IsWindowVisible(hwnd)) {
                    break;
                }
                if (LOWORD(wParam) == WA_CLICKACTIVE) {
                    SDL_bool swapButtons = GetSystemMetrics(SM_SWAPBUTTON) != 0;
                    if (GetAsyncKeyState(VK_LBUTTON)) {
                        data->focus_click_pending |= !swapButtons ? SDL_BUTTON_LMASK : SDL_BUTTON_RMASK;
                    }
                    if (GetAsyncKeyState(VK_RBUTTON)) {
                        data->focus_click_pending |= !swapButtons ? SDL_BUTTON_RMASK : SDL_BUTTON_LMASK;
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
                WIN_UpdateClipCursor(data->window);

                /*
                 * FIXME: Update keyboard state
                 */
                WIN_CheckClipboardUpdate(data->videodata);

                SDL_ToggleModState(KMOD_CAPS, (GetKeyState(VK_CAPITAL) & 0x0001) != 0);
                SDL_ToggleModState(KMOD_NUM, (GetKeyState(VK_NUMLOCK) & 0x0001) != 0);
            } else {
                RECT rect;

                data->in_window_deactivation = SDL_TRUE;

                if (SDL_GetKeyboardFocus() == data->window) {
                    SDL_SetKeyboardFocus(NULL);
                    WIN_ResetDeadKeys();
                }

                if (GetClipCursor(&rect) && SDL_memcmp(&rect, &data->cursor_clipped_rect, sizeof(rect)) == 0) {
                    ClipCursor(NULL);
                    SDL_zero(data->cursor_clipped_rect);
                }

                data->in_window_deactivation = SDL_FALSE;
            }
        }
        returnCode = 0;
        break;

    case WM_POINTERUPDATE:
        {
            data->last_pointer_update = lParam;
            break;
        }

    case WM_MOUSEMOVE:
        {
            SDL_Mouse *mouse = SDL_GetMouse();
            if (!mouse->relative_mode || mouse->relative_mode_warp) {
                /* Only generate mouse events for real mouse */
                if (GetMouseMessageSource() != SDL_MOUSE_EVENT_SOURCE_TOUCH &&
                    lParam != data->last_pointer_update) {
                    SDL_SendMouseMotion(data->window, 0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                    if (isWin10FCUorNewer && mouse->relative_mode_warp) {
                        /* To work around #3931, Win10 bug introduced in Fall Creators Update, where
                           SetCursorPos() (SDL_WarpMouseInWindow()) doesn't reliably generate mouse events anymore,
                           after each windows mouse event generate a fake event for the middle of the window
                           if relative_mode_warp is used */
                        int center_x = 0, center_y = 0;
                        SDL_GetWindowSize(data->window, &center_x, &center_y);
                        center_x /= 2;
                        center_y /= 2;
                        SDL_SendMouseMotion(data->window, 0, 0, center_x, center_y);
                    }
                }
            } else {
                /* We still need to update focus */
                SDL_SetMouseFocus(data->window);
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
                if (GetMouseMessageSource() != SDL_MOUSE_EVENT_SOURCE_TOUCH &&
                    lParam != data->last_pointer_update) {
                    WIN_CheckWParamMouseButtons(wParam, data, 0);
                }
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

            /* Mouse data (ignoring synthetic mouse events generated for touchscreens) */
            if (inp.header.dwType == RIM_TYPEMOUSE) {
                SDL_MouseID mouseID;
                if (SDL_GetNumTouchDevices() > 0 &&
                    (GetMouseMessageSource() == SDL_MOUSE_EVENT_SOURCE_TOUCH || (GetMessageExtraInfo() & 0x82) == 0x82)) {
                    break;
                }
                mouseID = (SDL_MouseID)(uintptr_t)inp.header.hDevice;
                if (isRelative) {
                    RAWMOUSE* rawmouse = &inp.data.mouse;

                    if ((rawmouse->usFlags & 0x01) == MOUSE_MOVE_RELATIVE) {
                        SDL_SendMouseMotion(data->window, mouseID, 1, (int)rawmouse->lLastX, (int)rawmouse->lLastY);
                    } else if (rawmouse->lLastX || rawmouse->lLastY) {
                        /* synthesize relative moves from the abs position */
                        static SDL_Point lastMousePoint;
                        SDL_bool virtual_desktop = (rawmouse->usFlags & MOUSE_VIRTUAL_DESKTOP) ? SDL_TRUE : SDL_FALSE;
                        int w = GetSystemMetrics(virtual_desktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
                        int h = GetSystemMetrics(virtual_desktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);
                        int x = (int)(((float)rawmouse->lLastX / 65535.0f) * w);
                        int y = (int)(((float)rawmouse->lLastY / 65535.0f) * h);

                        if (lastMousePoint.x == 0 && lastMousePoint.y == 0) {
                            lastMousePoint.x = x;
                            lastMousePoint.y = y;
                        }

                        SDL_SendMouseMotion(data->window, mouseID, 1, (int)(x-lastMousePoint.x), (int)(y-lastMousePoint.y));

                        lastMousePoint.x = x;
                        lastMousePoint.y = y;
                    }
                    WIN_CheckRawMouseButtons(rawmouse->usButtonFlags, data, mouseID);
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
                        SDL_bool swapButtons = GetSystemMetrics(SM_SWAPBUTTON) != 0;

                        SDL_SendMouseMotion(data->window, mouseID, 0, (int)pt.x, (int)pt.y);
                        SDL_SendMouseButton(data->window, mouseID, GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, !swapButtons ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT);
                        SDL_SendMouseButton(data->window, mouseID, GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, !swapButtons ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT);
                        SDL_SendMouseButton(data->window, mouseID, GetAsyncKeyState(VK_MBUTTON) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_MIDDLE);
                        SDL_SendMouseButton(data->window, mouseID, GetAsyncKeyState(VK_XBUTTON1) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_X1);
                        SDL_SendMouseButton(data->window, mouseID, GetAsyncKeyState(VK_XBUTTON2) & 0x8000 ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_X2);
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
                SDL_Mouse *mouse;
                POINT cursorPos;
                GetCursorPos(&cursorPos);
                ScreenToClient(hwnd, &cursorPos);
                mouse = SDL_GetMouse();
                if (!mouse->was_touch_mouse_events) { /* we're not a touch handler causing a mouse leave? */
                    SDL_SendMouseMotion(data->window, 0, 0, cursorPos.x, cursorPos.y);
                } else { /* touch handling? */
                    mouse->was_touch_mouse_events = SDL_FALSE; /* not anymore */
                    if (mouse->touch_mouse_events) { /* convert touch to mouse events */
                        SDL_SendMouseMotion(data->window, SDL_TOUCH_MOUSEID, 0, cursorPos.x, cursorPos.y);
                    } else { /* normal handling */
                        SDL_SendMouseMotion(data->window, 0, 0, cursorPos.x, cursorPos.y);
                    }
               }
            }
        }

        /* When WM_MOUSELEAVE is fired we can be assured that the cursor has left the window.
           Regardless of relative mode, it is important that mouse focus is reset as there is a potential
           race condition when in the process of leaving/entering relative mode, resulting in focus never
           being lost. This then causes a cascading failure where SDL_WINDOWEVENT_ENTER / SDL_WINDOWEVENT_LEAVE
           can stop firing permanently, due to the focus being in the wrong state and TrackMouseEvent never
           resubscribing. */
        SDL_SetMouseFocus(NULL);

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
        if (wParam == UNICODE_NOCHAR) {
            returnCode = 1;
            break;
        }
        /* otherwise fall through to below */
    case WM_CHAR:
        {
            char text[5];
            if (WIN_ConvertUTF32toUTF8((UINT32)wParam, text)) {
                SDL_SendKeyboardText(text);
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

            if (SDL_IsShapedWindow(data->window)) {
                Win32_ResizeWindowShape(data->window);
            }

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
                if (SDL_GetWindowFlags(data->window) & SDL_WINDOW_BORDERLESS) {
                    int screenW = GetSystemMetrics(SM_CXSCREEN);
                    int screenH = GetSystemMetrics(SM_CYSCREEN);
                    info->ptMaxSize.x = SDL_max(w, screenW);
                    info->ptMaxSize.y = SDL_max(h, screenH);
                    info->ptMaxPosition.x = SDL_min(0, ((screenW - w) / 2));
                    info->ptMaxPosition.y = SDL_min(0, ((screenH - h) / 2));
                }
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
            SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_RESIZED, w, h);

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
                SDL_SendWindowEvent(data->window, SDL_WINDOWEVENT_EXPOSED, 0, 0);
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
            SDL_bool isstack;
            PTOUCHINPUT inputs = SDL_small_alloc(TOUCHINPUT, num_inputs, &isstack);
            if (data->videodata->GetTouchInputInfo((HTOUCHINPUT)lParam, num_inputs, inputs, sizeof(TOUCHINPUT))) {
                RECT rect;
                float x, y;

                if (!GetClientRect(hwnd, &rect) ||
                    (rect.right == rect.left && rect.bottom == rect.top)) {
                    if (inputs) {
                        SDL_small_free(inputs, isstack);
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

                    /* TODO: Can we use GetRawInputDeviceInfo and HID info to
                       determine if this is a direct or indirect touch device?
                     */
                    if (SDL_AddTouch(touchId, SDL_TOUCH_DEVICE_DIRECT, "") < 0) {
                        continue;
                    }

                    /* Get the normalized coordinates for the window */
                    x = (float)(input->x - rect.left)/(rect.right - rect.left);
                    y = (float)(input->y - rect.top)/(rect.bottom - rect.top);

                    if (input->dwFlags & TOUCHEVENTF_DOWN) {
                        SDL_SendTouch(touchId, input->dwID, data->window, SDL_TRUE, x, y, 1.0f);
                    }
                    if (input->dwFlags & TOUCHEVENTF_MOVE) {
                        SDL_SendTouchMotion(touchId, input->dwID, data->window, x, y, 1.0f);
                    }
                    if (input->dwFlags & TOUCHEVENTF_UP) {
                        SDL_SendTouch(touchId, input->dwID, data->window, SDL_FALSE, x, y, 1.0f);
                    }
                }
            }
            SDL_small_free(inputs, isstack);

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
                SDL_bool isstack;
                UINT size = DragQueryFile(drop, i, NULL, 0) + 1;
                LPTSTR buffer = SDL_small_alloc(TCHAR, size, &isstack);
                if (buffer) {
                    if (DragQueryFile(drop, i, buffer, size)) {
                        char *file = WIN_StringToUTF8(buffer);
                        SDL_SendDropFile(data->window, file);
                        SDL_free(file);
                    }
                    SDL_small_free(buffer, isstack);
                }
            }
            SDL_SendDropComplete(data->window);
            DragFinish(drop);
            return 0;
        }
        break;

    case WM_DISPLAYCHANGE:
        {
            // Reacquire displays if any were added or removed
            WIN_RefreshDisplays(SDL_GetVideoDevice());
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

static void WIN_UpdateClipCursorForWindows()
{
    SDL_VideoDevice *_this = SDL_GetVideoDevice();
    SDL_Window *window;
    Uint32 now = SDL_GetTicks();
    const Uint32 CLIPCURSOR_UPDATE_INTERVAL_MS = 3000;

    if (_this) {
        for (window = _this->windows; window; window = window->next) {
            SDL_WindowData *data = (SDL_WindowData *)window->driverdata;
            if (data) {
                if (data->skip_update_clipcursor) {
                    data->skip_update_clipcursor = SDL_FALSE;
                    WIN_UpdateClipCursor(window);
                } else if ((now - data->last_updated_clipcursor) >= CLIPCURSOR_UPDATE_INTERVAL_MS) {
                    WIN_UpdateClipCursor(window);
                }
            }
        }
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

int
WIN_WaitEventTimeout(_THIS, int timeout)
{
    MSG msg;
    if (g_WindowsEnableMessageLoop) {
        BOOL message_result;
        UINT_PTR timer_id = 0;
        if (timeout > 0) {
            timer_id = SetTimer(NULL, 0, timeout, NULL);
            message_result = GetMessage(&msg, 0, 0, 0);
            KillTimer(NULL, timer_id);
        } else if (timeout == 0) {
            message_result = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        } else {
            message_result = GetMessage(&msg, 0, 0, 0);
        }
        if (message_result) {
            if (msg.message == WM_TIMER && msg.hwnd == NULL && msg.wParam == timer_id) {
                return 0;
            }
            if (g_WindowsMessageHook) {
                g_WindowsMessageHook(g_WindowsMessageHookData, msg.hwnd, msg.message, msg.wParam, msg.lParam);
            }
            /* Always translate the message in case it's a non-SDL window (e.g. with Qt integration) */
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            return 1;
        } else {
            return 0;
        }
    } else {
        /* Fail the wait so the caller falls back to polling */
        return -1;
    }
}

void
WIN_SendWakeupEvent(_THIS, SDL_Window *window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    PostMessage(data->hwnd, data->videodata->_SDL_WAKEUP, 0, 0);
}

void
WIN_PumpEvents(_THIS)
{
    const Uint8 *keystate;
    MSG msg;
    DWORD start_ticks = GetTickCount();
    int new_messages = 0;

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
                /* We might get a few new messages generated by the Steam overlay or other application hooks
                   In this case those messages will be processed before any pending input, so we want to continue after those messages.
                   (thanks to Peter Deayton for his investigation here)
                 */
                const int MAX_NEW_MESSAGES = 3;
                ++new_messages;
                if (new_messages > MAX_NEW_MESSAGES) {
                    break;
                }
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
    /* The Windows key state gets lost when using Windows+Space or Windows+G shortcuts */
    if ((keystate[SDL_SCANCODE_LGUI] == SDL_PRESSED) && !(GetKeyState(VK_LWIN) & 0x8000)) {
        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_LGUI);
    }
    if ((keystate[SDL_SCANCODE_RGUI] == SDL_PRESSED) && !(GetKeyState(VK_RWIN) & 0x8000)) {
        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_RGUI);
    }

    /* Update the clipping rect in case someone else has stolen it */
    WIN_UpdateClipCursorForWindows();
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
    HMODULE handle = GetModuleHandle(TEXT("ntdll.dll"));
    if (handle) {
        typedef LONG(WINAPI* RtlGetVersionPtr)(struct SDL_WIN_OSVERSIONINFOW*);
        RtlGetVersionPtr getVersionPtr = (RtlGetVersionPtr)GetProcAddress(handle, "RtlGetVersion");
        if (getVersionPtr != NULL) {
            struct SDL_WIN_OSVERSIONINFOW info;
            SDL_zero(info);
            info.dwOSVersionInfoSize = sizeof(info);
            if (getVersionPtr(&info) == 0) { /* STATUS_SUCCESS == 0 */
                if ((info.dwMajorVersion == 10 && info.dwMinorVersion == 0 && info.dwBuildNumber >= 16299) ||
                    (info.dwMajorVersion == 10 && info.dwMinorVersion > 0) ||
                    (info.dwMajorVersion > 10))
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
