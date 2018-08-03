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

#include "SDL_x11video.h"

#include "../../events/SDL_keyboard_c.h"
#include "../../events/scancodes_darwin.h"
#include "../../events/scancodes_xfree86.h"

#include <X11/keysym.h>
#include <X11/XKBlib.h>

#include "imKStoUCS.h"

#ifdef X_HAVE_UTF8_STRING
#include <locale.h>
#endif

/* *INDENT-OFF* */
static const struct {
    KeySym keysym;
    SDL_Scancode scancode;
} KeySymToSDLScancode[] = {
    { XK_Return, SDL_SCANCODE_RETURN },
    { XK_Escape, SDL_SCANCODE_ESCAPE },
    { XK_BackSpace, SDL_SCANCODE_BACKSPACE },
    { XK_Tab, SDL_SCANCODE_TAB },
    { XK_Caps_Lock, SDL_SCANCODE_CAPSLOCK },
    { XK_F1, SDL_SCANCODE_F1 },
    { XK_F2, SDL_SCANCODE_F2 },
    { XK_F3, SDL_SCANCODE_F3 },
    { XK_F4, SDL_SCANCODE_F4 },
    { XK_F5, SDL_SCANCODE_F5 },
    { XK_F6, SDL_SCANCODE_F6 },
    { XK_F7, SDL_SCANCODE_F7 },
    { XK_F8, SDL_SCANCODE_F8 },
    { XK_F9, SDL_SCANCODE_F9 },
    { XK_F10, SDL_SCANCODE_F10 },
    { XK_F11, SDL_SCANCODE_F11 },
    { XK_F12, SDL_SCANCODE_F12 },
    { XK_Print, SDL_SCANCODE_PRINTSCREEN },
    { XK_Scroll_Lock, SDL_SCANCODE_SCROLLLOCK },
    { XK_Pause, SDL_SCANCODE_PAUSE },
    { XK_Insert, SDL_SCANCODE_INSERT },
    { XK_Home, SDL_SCANCODE_HOME },
    { XK_Prior, SDL_SCANCODE_PAGEUP },
    { XK_Delete, SDL_SCANCODE_DELETE },
    { XK_End, SDL_SCANCODE_END },
    { XK_Next, SDL_SCANCODE_PAGEDOWN },
    { XK_Right, SDL_SCANCODE_RIGHT },
    { XK_Left, SDL_SCANCODE_LEFT },
    { XK_Down, SDL_SCANCODE_DOWN },
    { XK_Up, SDL_SCANCODE_UP },
    { XK_Num_Lock, SDL_SCANCODE_NUMLOCKCLEAR },
    { XK_KP_Divide, SDL_SCANCODE_KP_DIVIDE },
    { XK_KP_Multiply, SDL_SCANCODE_KP_MULTIPLY },
    { XK_KP_Subtract, SDL_SCANCODE_KP_MINUS },
    { XK_KP_Add, SDL_SCANCODE_KP_PLUS },
    { XK_KP_Enter, SDL_SCANCODE_KP_ENTER },
    { XK_KP_Delete, SDL_SCANCODE_KP_PERIOD },
    { XK_KP_End, SDL_SCANCODE_KP_1 },
    { XK_KP_Down, SDL_SCANCODE_KP_2 },
    { XK_KP_Next, SDL_SCANCODE_KP_3 },
    { XK_KP_Left, SDL_SCANCODE_KP_4 },
    { XK_KP_Begin, SDL_SCANCODE_KP_5 },
    { XK_KP_Right, SDL_SCANCODE_KP_6 },
    { XK_KP_Home, SDL_SCANCODE_KP_7 },
    { XK_KP_Up, SDL_SCANCODE_KP_8 },
    { XK_KP_Prior, SDL_SCANCODE_KP_9 },
    { XK_KP_Insert, SDL_SCANCODE_KP_0 },
    { XK_KP_Decimal, SDL_SCANCODE_KP_PERIOD },
    { XK_KP_1, SDL_SCANCODE_KP_1 },
    { XK_KP_2, SDL_SCANCODE_KP_2 },
    { XK_KP_3, SDL_SCANCODE_KP_3 },
    { XK_KP_4, SDL_SCANCODE_KP_4 },
    { XK_KP_5, SDL_SCANCODE_KP_5 },
    { XK_KP_6, SDL_SCANCODE_KP_6 },
    { XK_KP_7, SDL_SCANCODE_KP_7 },
    { XK_KP_8, SDL_SCANCODE_KP_8 },
    { XK_KP_9, SDL_SCANCODE_KP_9 },
    { XK_KP_0, SDL_SCANCODE_KP_0 },
    { XK_KP_Decimal, SDL_SCANCODE_KP_PERIOD },
    { XK_Hyper_R, SDL_SCANCODE_APPLICATION },
    { XK_KP_Equal, SDL_SCANCODE_KP_EQUALS },
    { XK_F13, SDL_SCANCODE_F13 },
    { XK_F14, SDL_SCANCODE_F14 },
    { XK_F15, SDL_SCANCODE_F15 },
    { XK_F16, SDL_SCANCODE_F16 },
    { XK_F17, SDL_SCANCODE_F17 },
    { XK_F18, SDL_SCANCODE_F18 },
    { XK_F19, SDL_SCANCODE_F19 },
    { XK_F20, SDL_SCANCODE_F20 },
    { XK_F21, SDL_SCANCODE_F21 },
    { XK_F22, SDL_SCANCODE_F22 },
    { XK_F23, SDL_SCANCODE_F23 },
    { XK_F24, SDL_SCANCODE_F24 },
    { XK_Execute, SDL_SCANCODE_EXECUTE },
    { XK_Help, SDL_SCANCODE_HELP },
    { XK_Menu, SDL_SCANCODE_MENU },
    { XK_Select, SDL_SCANCODE_SELECT },
    { XK_Cancel, SDL_SCANCODE_STOP },
    { XK_Redo, SDL_SCANCODE_AGAIN },
    { XK_Undo, SDL_SCANCODE_UNDO },
    { XK_Find, SDL_SCANCODE_FIND },
    { XK_KP_Separator, SDL_SCANCODE_KP_COMMA },
    { XK_Sys_Req, SDL_SCANCODE_SYSREQ },
    { XK_Control_L, SDL_SCANCODE_LCTRL },
    { XK_Shift_L, SDL_SCANCODE_LSHIFT },
    { XK_Alt_L, SDL_SCANCODE_LALT },
    { XK_Meta_L, SDL_SCANCODE_LGUI },
    { XK_Super_L, SDL_SCANCODE_LGUI },
    { XK_Control_R, SDL_SCANCODE_RCTRL },
    { XK_Shift_R, SDL_SCANCODE_RSHIFT },
    { XK_Alt_R, SDL_SCANCODE_RALT },
    { XK_ISO_Level3_Shift, SDL_SCANCODE_RALT },
    { XK_Meta_R, SDL_SCANCODE_RGUI },
    { XK_Super_R, SDL_SCANCODE_RGUI },
    { XK_Mode_switch, SDL_SCANCODE_MODE },
    { XK_period, SDL_SCANCODE_PERIOD },
    { XK_comma, SDL_SCANCODE_COMMA },
    { XK_slash, SDL_SCANCODE_SLASH },
    { XK_backslash, SDL_SCANCODE_BACKSLASH },
    { XK_minus, SDL_SCANCODE_MINUS },
    { XK_equal, SDL_SCANCODE_EQUALS },
    { XK_space, SDL_SCANCODE_SPACE },
    { XK_grave, SDL_SCANCODE_GRAVE },
    { XK_apostrophe, SDL_SCANCODE_APOSTROPHE },
    { XK_bracketleft, SDL_SCANCODE_LEFTBRACKET },
    { XK_bracketright, SDL_SCANCODE_RIGHTBRACKET },
};

static const struct
{
    SDL_Scancode const *table;
    int table_size;
} scancode_set[] = {
    { darwin_scancode_table, SDL_arraysize(darwin_scancode_table) },
    { xfree86_scancode_table, SDL_arraysize(xfree86_scancode_table) },
    { xfree86_scancode_table2, SDL_arraysize(xfree86_scancode_table2) },
    { xvnc_scancode_table, SDL_arraysize(xvnc_scancode_table) },
};
/* *INDENT-OFF* */

/* This function only works for keyboards in US QWERTY layout */
static SDL_Scancode
X11_KeyCodeToSDLScancode(_THIS, KeyCode keycode)
{
    KeySym keysym;
    int i;

    keysym = X11_KeyCodeToSym(_this, keycode, 0);
    if (keysym == NoSymbol) {
        return SDL_SCANCODE_UNKNOWN;
    }

    if (keysym >= XK_a && keysym <= XK_z) {
        return SDL_SCANCODE_A + (keysym - XK_a);
    }
    if (keysym >= XK_A && keysym <= XK_Z) {
        return SDL_SCANCODE_A + (keysym - XK_A);
    }

    if (keysym == XK_0) {
        return SDL_SCANCODE_0;
    }
    if (keysym >= XK_1 && keysym <= XK_9) {
        return SDL_SCANCODE_1 + (keysym - XK_1);
    }

    for (i = 0; i < SDL_arraysize(KeySymToSDLScancode); ++i) {
        if (keysym == KeySymToSDLScancode[i].keysym) {
            return KeySymToSDLScancode[i].scancode;
        }
    }
    return SDL_SCANCODE_UNKNOWN;
}

static Uint32
X11_KeyCodeToUcs4(_THIS, KeyCode keycode, unsigned char group)
{
    KeySym keysym = X11_KeyCodeToSym(_this, keycode, group);

    if (keysym == NoSymbol) {
        return 0;
    }

    return X11_KeySymToUcs4(keysym);
}

KeySym
X11_KeyCodeToSym(_THIS, KeyCode keycode, unsigned char group)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    KeySym keysym;

#if SDL_VIDEO_DRIVER_X11_HAS_XKBKEYCODETOKEYSYM
    if (data->xkb) {
        int num_groups     = XkbKeyNumGroups(data->xkb, keycode);
        unsigned char info = XkbKeyGroupInfo(data->xkb, keycode);
        
        if (num_groups && group >= num_groups) {
        
            int action = XkbOutOfRangeGroupAction(info);
            
            if (action == XkbRedirectIntoRange) {
                if ((group = XkbOutOfRangeGroupNumber(info)) >= num_groups) {
                    group = 0;
                }
            } else if (action == XkbClampIntoRange) {
                group = num_groups - 1;
            } else {
                group %= num_groups;
            }
        }
        keysym = X11_XkbKeycodeToKeysym(data->display, keycode, group, 0);
    } else {
        keysym = X11_XKeycodeToKeysym(data->display, keycode, 0);
    }
#else
    keysym = X11_XKeycodeToKeysym(data->display, keycode, 0);
#endif

    return keysym;
}

int
X11_InitKeyboard(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    int i = 0;
    int j = 0;
    int min_keycode, max_keycode;
    struct {
        SDL_Scancode scancode;
        KeySym keysym;
        int value;
    } fingerprint[] = {
        { SDL_SCANCODE_HOME, XK_Home, 0 },
        { SDL_SCANCODE_PAGEUP, XK_Prior, 0 },
        { SDL_SCANCODE_UP, XK_Up, 0 },
        { SDL_SCANCODE_LEFT, XK_Left, 0 },
        { SDL_SCANCODE_DELETE, XK_Delete, 0 },
        { SDL_SCANCODE_KP_ENTER, XK_KP_Enter, 0 },
    };
    int best_distance;
    int best_index;
    int distance;
    BOOL xkb_repeat = 0;
    
    X11_XAutoRepeatOn(data->display);

#if SDL_VIDEO_DRIVER_X11_HAS_XKBKEYCODETOKEYSYM
    {
        int xkb_major = XkbMajorVersion;
        int xkb_minor = XkbMinorVersion;

        if (X11_XkbQueryExtension(data->display, NULL, NULL, NULL, &xkb_major, &xkb_minor)) {
            data->xkb = X11_XkbGetMap(data->display, XkbAllClientInfoMask, XkbUseCoreKbd);
        }

        /* This will remove KeyRelease events for held keys */
        X11_XkbSetDetectableAutoRepeat(data->display, True, &xkb_repeat);
    }
#endif
    
    /* Open a connection to the X input manager */
#ifdef X_HAVE_UTF8_STRING
    if (SDL_X11_HAVE_UTF8) {
        /* Set the locale, and call XSetLocaleModifiers before XOpenIM so that 
           Compose keys will work correctly. */
        char *prev_locale = setlocale(LC_ALL, NULL);
        char *prev_xmods  = X11_XSetLocaleModifiers(NULL);
        const char *new_xmods = "";
#if defined(HAVE_IBUS_IBUS_H) || defined(HAVE_FCITX_FRONTEND_H)
        const char *env_xmods = SDL_getenv("XMODIFIERS");
#endif
        SDL_bool has_dbus_ime_support = SDL_FALSE;

        if (prev_locale) {
            prev_locale = SDL_strdup(prev_locale);
        }

        if (prev_xmods) {
            prev_xmods = SDL_strdup(prev_xmods);
        }

        /* IBus resends some key events that were filtered by XFilterEvents
           when it is used via XIM which causes issues. Prevent this by forcing
           @im=none if XMODIFIERS contains @im=ibus. IBus can still be used via 
           the DBus implementation, which also has support for pre-editing. */
#ifdef HAVE_IBUS_IBUS_H
        if (env_xmods && SDL_strstr(env_xmods, "@im=ibus") != NULL) {
            has_dbus_ime_support = SDL_TRUE;
        }
#endif
#ifdef HAVE_FCITX_FRONTEND_H
        if (env_xmods && SDL_strstr(env_xmods, "@im=fcitx") != NULL) {
            has_dbus_ime_support = SDL_TRUE;
        }
#endif
        if (has_dbus_ime_support || !xkb_repeat) {
            new_xmods = "@im=none";
        }

        setlocale(LC_ALL, "");
        X11_XSetLocaleModifiers(new_xmods);

        data->im = X11_XOpenIM(data->display, NULL, data->classname, data->classname);

        /* Reset the locale + X locale modifiers back to how they were,
           locale first because the X locale modifiers depend on it. */
        setlocale(LC_ALL, prev_locale);
        X11_XSetLocaleModifiers(prev_xmods);

        if (prev_locale) {
            SDL_free(prev_locale);
        }

        if (prev_xmods) {
            SDL_free(prev_xmods);
        }
    }
#endif
    /* Try to determine which scancodes are being used based on fingerprint */
    best_distance = SDL_arraysize(fingerprint) + 1;
    best_index = -1;
    X11_XDisplayKeycodes(data->display, &min_keycode, &max_keycode);
    for (i = 0; i < SDL_arraysize(fingerprint); ++i) {
        fingerprint[i].value =
            X11_XKeysymToKeycode(data->display, fingerprint[i].keysym) -
            min_keycode;
    }
    for (i = 0; i < SDL_arraysize(scancode_set); ++i) {
        /* Make sure the scancode set isn't too big */
        if ((max_keycode - min_keycode + 1) <= scancode_set[i].table_size) {
            continue;
        }
        distance = 0;
        for (j = 0; j < SDL_arraysize(fingerprint); ++j) {
            if (fingerprint[j].value < 0
                || fingerprint[j].value >= scancode_set[i].table_size) {
                distance += 1;
            } else if (scancode_set[i].table[fingerprint[j].value] != fingerprint[j].scancode) {
                distance += 1;
            }
        }
        if (distance < best_distance) {
            best_distance = distance;
            best_index = i;
        }
    }
    if (best_index >= 0 && best_distance <= 2) {
#ifdef DEBUG_KEYBOARD
        printf("Using scancode set %d, min_keycode = %d, max_keycode = %d, table_size = %d\n", best_index, min_keycode, max_keycode, scancode_set[best_index].table_size);
#endif
        SDL_memcpy(&data->key_layout[min_keycode], scancode_set[best_index].table,
                   sizeof(SDL_Scancode) * scancode_set[best_index].table_size);
    } else {
        SDL_Keycode keymap[SDL_NUM_SCANCODES];

        printf
            ("Keyboard layout unknown, please report the following to the SDL forums/mailing list (https://discourse.libsdl.org/):\n");

        /* Determine key_layout - only works on US QWERTY layout */
        SDL_GetDefaultKeymap(keymap);
        for (i = min_keycode; i <= max_keycode; ++i) {
            KeySym sym;
            sym = X11_KeyCodeToSym(_this, (KeyCode) i, 0);
            if (sym != NoSymbol) {
                SDL_Scancode scancode;
                printf("code = %d, sym = 0x%X (%s) ", i - min_keycode,
                       (unsigned int) sym, X11_XKeysymToString(sym));
                scancode = X11_KeyCodeToSDLScancode(_this, i);
                data->key_layout[i] = scancode;
                if (scancode == SDL_SCANCODE_UNKNOWN) {
                    printf("scancode not found\n");
                } else {
                    printf("scancode = %d (%s)\n", scancode, SDL_GetScancodeName(scancode));
                }
            }
        }
    }

    X11_UpdateKeymap(_this);

    SDL_SetScancodeName(SDL_SCANCODE_APPLICATION, "Menu");

#ifdef SDL_USE_IME
    SDL_IME_Init();
#endif

    return 0;
}

void
X11_UpdateKeymap(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    int i;
    SDL_Scancode scancode;
    SDL_Keycode keymap[SDL_NUM_SCANCODES];
    unsigned char group = 0;

    SDL_GetDefaultKeymap(keymap);

#if SDL_VIDEO_DRIVER_X11_HAS_XKBKEYCODETOKEYSYM
    if (data->xkb) {
        XkbStateRec state;
        X11_XkbGetUpdatedMap(data->display, XkbAllClientInfoMask, data->xkb);

        if (X11_XkbGetState(data->display, XkbUseCoreKbd, &state) == Success) {
            group = state.group;
        }
    }
#endif


    for (i = 0; i < SDL_arraysize(data->key_layout); i++) {
        Uint32 key;

        /* Make sure this is a valid scancode */
        scancode = data->key_layout[i];
        if (scancode == SDL_SCANCODE_UNKNOWN) {
            continue;
        }

        /* See if there is a UCS keycode for this scancode */
        key = X11_KeyCodeToUcs4(_this, (KeyCode)i, group);
        if (key) {
            keymap[scancode] = key;
        } else {
            SDL_Scancode keyScancode = X11_KeyCodeToSDLScancode(_this, (KeyCode)i);

            switch (keyScancode) {
                case SDL_SCANCODE_RETURN:
                    keymap[scancode] = SDLK_RETURN;
                    break;
                case SDL_SCANCODE_ESCAPE:
                    keymap[scancode] = SDLK_ESCAPE;
                    break;
                case SDL_SCANCODE_BACKSPACE:
                    keymap[scancode] = SDLK_BACKSPACE;
                    break;
                case SDL_SCANCODE_TAB:
                    keymap[scancode] = SDLK_TAB;
                    break;
                case SDL_SCANCODE_DELETE:
                    keymap[scancode] = SDLK_DELETE;
                    break;
                default:
                    keymap[scancode] = SDL_SCANCODE_TO_KEYCODE(keyScancode);
                    break;
            }
        }
    }
    SDL_SetKeymap(0, keymap, SDL_NUM_SCANCODES);
}

void
X11_QuitKeyboard(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

#if SDL_VIDEO_DRIVER_X11_HAS_XKBKEYCODETOKEYSYM
    if (data->xkb) {
        X11_XkbFreeKeyboard(data->xkb, 0, True);
        data->xkb = NULL;
    }
#endif

#ifdef SDL_USE_IME
    SDL_IME_Quit();
#endif
}

static void
X11_ResetXIM(_THIS)
{
#ifdef X_HAVE_UTF8_STRING
    SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
    int i;

    if (videodata && videodata->windowlist) {
        for (i = 0; i < videodata->numwindows; ++i) {
            SDL_WindowData *data = videodata->windowlist[i];
            if (data && data->ic) {
                /* Clear any partially entered dead keys */
                char *contents = X11_Xutf8ResetIC(data->ic);
                if (contents) {
                    X11_XFree(contents);
                }
            }
        }
    }
#endif
}

void
X11_StartTextInput(_THIS)
{
    X11_ResetXIM(_this);
}

void
X11_StopTextInput(_THIS)
{
    X11_ResetXIM(_this);
#ifdef SDL_USE_IME
    SDL_IME_Reset();
#endif
}

void
X11_SetTextInputRect(_THIS, SDL_Rect *rect)
{
    if (!rect) {
        SDL_InvalidParamError("rect");
        return;
    }
       
#ifdef SDL_USE_IME
    SDL_IME_UpdateTextRect(rect);
#endif
}

#endif /* SDL_VIDEO_DRIVER_X11 */

/* vi: set ts=4 sw=4 expandtab: */
