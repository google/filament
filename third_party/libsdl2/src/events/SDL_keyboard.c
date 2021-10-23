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

/* General keyboard handling code for SDL */

#include "SDL_hints.h"
#include "SDL_timer.h"
#include "SDL_events.h"
#include "SDL_events_c.h"
#include "../video/SDL_sysvideo.h"


/* #define DEBUG_KEYBOARD */

/* Global keyboard information */

#define KEYBOARD_HARDWARE       0x01
#define KEYBOARD_AUTORELEASE    0x02

typedef struct SDL_Keyboard SDL_Keyboard;

struct SDL_Keyboard
{
    /* Data common to all keyboards */
    SDL_Window *focus;
    Uint16 modstate;
    Uint8 keysource[SDL_NUM_SCANCODES];
    Uint8 keystate[SDL_NUM_SCANCODES];
    SDL_Keycode keymap[SDL_NUM_SCANCODES];
    SDL_bool autorelease_pending;
};

static SDL_Keyboard SDL_keyboard;

static const SDL_Keycode SDL_default_keymap[SDL_NUM_SCANCODES] = {
    0, 0, 0, 0,
    'a',
    'b',
    'c',
    'd',
    'e',
    'f',
    'g',
    'h',
    'i',
    'j',
    'k',
    'l',
    'm',
    'n',
    'o',
    'p',
    'q',
    'r',
    's',
    't',
    'u',
    'v',
    'w',
    'x',
    'y',
    'z',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    SDLK_RETURN,
    SDLK_ESCAPE,
    SDLK_BACKSPACE,
    SDLK_TAB,
    SDLK_SPACE,
    '-',
    '=',
    '[',
    ']',
    '\\',
    '#',
    ';',
    '\'',
    '`',
    ',',
    '.',
    '/',
    SDLK_CAPSLOCK,
    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
    SDLK_PRINTSCREEN,
    SDLK_SCROLLLOCK,
    SDLK_PAUSE,
    SDLK_INSERT,
    SDLK_HOME,
    SDLK_PAGEUP,
    SDLK_DELETE,
    SDLK_END,
    SDLK_PAGEDOWN,
    SDLK_RIGHT,
    SDLK_LEFT,
    SDLK_DOWN,
    SDLK_UP,
    SDLK_NUMLOCKCLEAR,
    SDLK_KP_DIVIDE,
    SDLK_KP_MULTIPLY,
    SDLK_KP_MINUS,
    SDLK_KP_PLUS,
    SDLK_KP_ENTER,
    SDLK_KP_1,
    SDLK_KP_2,
    SDLK_KP_3,
    SDLK_KP_4,
    SDLK_KP_5,
    SDLK_KP_6,
    SDLK_KP_7,
    SDLK_KP_8,
    SDLK_KP_9,
    SDLK_KP_0,
    SDLK_KP_PERIOD,
    0,
    SDLK_APPLICATION,
    SDLK_POWER,
    SDLK_KP_EQUALS,
    SDLK_F13,
    SDLK_F14,
    SDLK_F15,
    SDLK_F16,
    SDLK_F17,
    SDLK_F18,
    SDLK_F19,
    SDLK_F20,
    SDLK_F21,
    SDLK_F22,
    SDLK_F23,
    SDLK_F24,
    SDLK_EXECUTE,
    SDLK_HELP,
    SDLK_MENU,
    SDLK_SELECT,
    SDLK_STOP,
    SDLK_AGAIN,
    SDLK_UNDO,
    SDLK_CUT,
    SDLK_COPY,
    SDLK_PASTE,
    SDLK_FIND,
    SDLK_MUTE,
    SDLK_VOLUMEUP,
    SDLK_VOLUMEDOWN,
    0, 0, 0,
    SDLK_KP_COMMA,
    SDLK_KP_EQUALSAS400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_ALTERASE,
    SDLK_SYSREQ,
    SDLK_CANCEL,
    SDLK_CLEAR,
    SDLK_PRIOR,
    SDLK_RETURN2,
    SDLK_SEPARATOR,
    SDLK_OUT,
    SDLK_OPER,
    SDLK_CLEARAGAIN,
    SDLK_CRSEL,
    SDLK_EXSEL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_KP_00,
    SDLK_KP_000,
    SDLK_THOUSANDSSEPARATOR,
    SDLK_DECIMALSEPARATOR,
    SDLK_CURRENCYUNIT,
    SDLK_CURRENCYSUBUNIT,
    SDLK_KP_LEFTPAREN,
    SDLK_KP_RIGHTPAREN,
    SDLK_KP_LEFTBRACE,
    SDLK_KP_RIGHTBRACE,
    SDLK_KP_TAB,
    SDLK_KP_BACKSPACE,
    SDLK_KP_A,
    SDLK_KP_B,
    SDLK_KP_C,
    SDLK_KP_D,
    SDLK_KP_E,
    SDLK_KP_F,
    SDLK_KP_XOR,
    SDLK_KP_POWER,
    SDLK_KP_PERCENT,
    SDLK_KP_LESS,
    SDLK_KP_GREATER,
    SDLK_KP_AMPERSAND,
    SDLK_KP_DBLAMPERSAND,
    SDLK_KP_VERTICALBAR,
    SDLK_KP_DBLVERTICALBAR,
    SDLK_KP_COLON,
    SDLK_KP_HASH,
    SDLK_KP_SPACE,
    SDLK_KP_AT,
    SDLK_KP_EXCLAM,
    SDLK_KP_MEMSTORE,
    SDLK_KP_MEMRECALL,
    SDLK_KP_MEMCLEAR,
    SDLK_KP_MEMADD,
    SDLK_KP_MEMSUBTRACT,
    SDLK_KP_MEMMULTIPLY,
    SDLK_KP_MEMDIVIDE,
    SDLK_KP_PLUSMINUS,
    SDLK_KP_CLEAR,
    SDLK_KP_CLEARENTRY,
    SDLK_KP_BINARY,
    SDLK_KP_OCTAL,
    SDLK_KP_DECIMAL,
    SDLK_KP_HEXADECIMAL,
    0, 0,
    SDLK_LCTRL,
    SDLK_LSHIFT,
    SDLK_LALT,
    SDLK_LGUI,
    SDLK_RCTRL,
    SDLK_RSHIFT,
    SDLK_RALT,
    SDLK_RGUI,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    SDLK_MODE,
    SDLK_AUDIONEXT,
    SDLK_AUDIOPREV,
    SDLK_AUDIOSTOP,
    SDLK_AUDIOPLAY,
    SDLK_AUDIOMUTE,
    SDLK_MEDIASELECT,
    SDLK_WWW,
    SDLK_MAIL,
    SDLK_CALCULATOR,
    SDLK_COMPUTER,
    SDLK_AC_SEARCH,
    SDLK_AC_HOME,
    SDLK_AC_BACK,
    SDLK_AC_FORWARD,
    SDLK_AC_STOP,
    SDLK_AC_REFRESH,
    SDLK_AC_BOOKMARKS,
    SDLK_BRIGHTNESSDOWN,
    SDLK_BRIGHTNESSUP,
    SDLK_DISPLAYSWITCH,
    SDLK_KBDILLUMTOGGLE,
    SDLK_KBDILLUMDOWN,
    SDLK_KBDILLUMUP,
    SDLK_EJECT,
    SDLK_SLEEP,
    SDLK_APP1,
    SDLK_APP2,
    SDLK_AUDIOREWIND,
    SDLK_AUDIOFASTFORWARD,
};

static const char *SDL_scancode_names[SDL_NUM_SCANCODES] = {
    NULL, NULL, NULL, NULL,
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "Return",
    "Escape",
    "Backspace",
    "Tab",
    "Space",
    "-",
    "=",
    "[",
    "]",
    "\\",
    "#",
    ";",
    "'",
    "`",
    ",",
    ".",
    "/",
    "CapsLock",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "PrintScreen",
    "ScrollLock",
    "Pause",
    "Insert",
    "Home",
    "PageUp",
    "Delete",
    "End",
    "PageDown",
    "Right",
    "Left",
    "Down",
    "Up",
    "Numlock",
    "Keypad /",
    "Keypad *",
    "Keypad -",
    "Keypad +",
    "Keypad Enter",
    "Keypad 1",
    "Keypad 2",
    "Keypad 3",
    "Keypad 4",
    "Keypad 5",
    "Keypad 6",
    "Keypad 7",
    "Keypad 8",
    "Keypad 9",
    "Keypad 0",
    "Keypad .",
    NULL,
    "Application",
    "Power",
    "Keypad =",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "Execute",
    "Help",
    "Menu",
    "Select",
    "Stop",
    "Again",
    "Undo",
    "Cut",
    "Copy",
    "Paste",
    "Find",
    "Mute",
    "VolumeUp",
    "VolumeDown",
    NULL, NULL, NULL,
    "Keypad ,",
    "Keypad = (AS400)",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL,
    "AltErase",
    "SysReq",
    "Cancel",
    "Clear",
    "Prior",
    "Return",
    "Separator",
    "Out",
    "Oper",
    "Clear / Again",
    "CrSel",
    "ExSel",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "Keypad 00",
    "Keypad 000",
    "ThousandsSeparator",
    "DecimalSeparator",
    "CurrencyUnit",
    "CurrencySubUnit",
    "Keypad (",
    "Keypad )",
    "Keypad {",
    "Keypad }",
    "Keypad Tab",
    "Keypad Backspace",
    "Keypad A",
    "Keypad B",
    "Keypad C",
    "Keypad D",
    "Keypad E",
    "Keypad F",
    "Keypad XOR",
    "Keypad ^",
    "Keypad %",
    "Keypad <",
    "Keypad >",
    "Keypad &",
    "Keypad &&",
    "Keypad |",
    "Keypad ||",
    "Keypad :",
    "Keypad #",
    "Keypad Space",
    "Keypad @",
    "Keypad !",
    "Keypad MemStore",
    "Keypad MemRecall",
    "Keypad MemClear",
    "Keypad MemAdd",
    "Keypad MemSubtract",
    "Keypad MemMultiply",
    "Keypad MemDivide",
    "Keypad +/-",
    "Keypad Clear",
    "Keypad ClearEntry",
    "Keypad Binary",
    "Keypad Octal",
    "Keypad Decimal",
    "Keypad Hexadecimal",
    NULL, NULL,
    "Left Ctrl",
    "Left Shift",
    "Left Alt",
    "Left GUI",
    "Right Ctrl",
    "Right Shift",
    "Right Alt",
    "Right GUI",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL,
    "ModeSwitch",
    "AudioNext",
    "AudioPrev",
    "AudioStop",
    "AudioPlay",
    "AudioMute",
    "MediaSelect",
    "WWW",
    "Mail",
    "Calculator",
    "Computer",
    "AC Search",
    "AC Home",
    "AC Back",
    "AC Forward",
    "AC Stop",
    "AC Refresh",
    "AC Bookmarks",
    "BrightnessDown",
    "BrightnessUp",
    "DisplaySwitch",
    "KBDIllumToggle",
    "KBDIllumDown",
    "KBDIllumUp",
    "Eject",
    "Sleep",
    "App1",
    "App2",
    "AudioRewind",
    "AudioFastForward",
};

/* Taken from SDL_iconv() */
char *
SDL_UCS4ToUTF8(Uint32 ch, char *dst)
{
    Uint8 *p = (Uint8 *) dst;
    if (ch <= 0x7F) {
        *p = (Uint8) ch;
        ++dst;
    } else if (ch <= 0x7FF) {
        p[0] = 0xC0 | (Uint8) ((ch >> 6) & 0x1F);
        p[1] = 0x80 | (Uint8) (ch & 0x3F);
        dst += 2;
    } else if (ch <= 0xFFFF) {
        p[0] = 0xE0 | (Uint8) ((ch >> 12) & 0x0F);
        p[1] = 0x80 | (Uint8) ((ch >> 6) & 0x3F);
        p[2] = 0x80 | (Uint8) (ch & 0x3F);
        dst += 3;
    } else {
        p[0] = 0xF0 | (Uint8) ((ch >> 18) & 0x07);
        p[1] = 0x80 | (Uint8) ((ch >> 12) & 0x3F);
        p[2] = 0x80 | (Uint8) ((ch >> 6) & 0x3F);
        p[3] = 0x80 | (Uint8) (ch & 0x3F);
        dst += 4;
    }
    return dst;
}

/* Public functions */
int
SDL_KeyboardInit(void)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;

    /* Set the default keymap */
    SDL_memcpy(keyboard->keymap, SDL_default_keymap, sizeof(SDL_default_keymap));
    return (0);
}

void
SDL_ResetKeyboard(void)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    SDL_Scancode scancode;

#ifdef DEBUG_KEYBOARD
    printf("Resetting keyboard\n");
#endif
    for (scancode = (SDL_Scancode) 0; scancode < SDL_NUM_SCANCODES; ++scancode) {
        if (keyboard->keystate[scancode] == SDL_PRESSED) {
            SDL_SendKeyboardKey(SDL_RELEASED, scancode);
        }
    }
}

void
SDL_GetDefaultKeymap(SDL_Keycode * keymap)
{
    SDL_memcpy(keymap, SDL_default_keymap, sizeof(SDL_default_keymap));
}

void
SDL_SetKeymap(int start, SDL_Keycode * keys, int length)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    SDL_Scancode scancode;

    if (start < 0 || start + length > SDL_NUM_SCANCODES) {
        return;
    }

    SDL_memcpy(&keyboard->keymap[start], keys, sizeof(*keys) * length);

    /* The number key scancodes always map to the number key keycodes.
     * On AZERTY layouts these technically are symbols, but users (and games)
     * always think of them and view them in UI as number keys.
     */
    keyboard->keymap[SDL_SCANCODE_0] = SDLK_0;
    for (scancode = SDL_SCANCODE_1; scancode <= SDL_SCANCODE_9; ++scancode) {
        keyboard->keymap[scancode] = SDLK_1 + (scancode - SDL_SCANCODE_1);
    }
}

void
SDL_SetScancodeName(SDL_Scancode scancode, const char *name)
{
    if (scancode >= SDL_NUM_SCANCODES) {
        return;
    }
    SDL_scancode_names[scancode] = name;
}

SDL_Window *
SDL_GetKeyboardFocus(void)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;

    return keyboard->focus;
}

void
SDL_SetKeyboardFocus(SDL_Window * window)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;

    if (keyboard->focus && !window) {
        /* We won't get anymore keyboard messages, so reset keyboard state */
        SDL_ResetKeyboard();
    }

    /* See if the current window has lost focus */
    if (keyboard->focus && keyboard->focus != window) {

        /* new window shouldn't think it has mouse captured. */
        SDL_assert(!window || !(window->flags & SDL_WINDOW_MOUSE_CAPTURE));

        /* old window must lose an existing mouse capture. */
        if (keyboard->focus->flags & SDL_WINDOW_MOUSE_CAPTURE) {
            SDL_CaptureMouse(SDL_FALSE);  /* drop the capture. */
            SDL_assert(!(keyboard->focus->flags & SDL_WINDOW_MOUSE_CAPTURE));
        }

        SDL_SendWindowEvent(keyboard->focus, SDL_WINDOWEVENT_FOCUS_LOST,
                            0, 0);

        /* Ensures IME compositions are committed */
        if (SDL_EventState(SDL_TEXTINPUT, SDL_QUERY)) {
            SDL_VideoDevice *video = SDL_GetVideoDevice();
            if (video && video->StopTextInput) {
                video->StopTextInput(video);
            }
        }
    }

    keyboard->focus = window;

    if (keyboard->focus) {
        SDL_SendWindowEvent(keyboard->focus, SDL_WINDOWEVENT_FOCUS_GAINED,
                            0, 0);

        if (SDL_EventState(SDL_TEXTINPUT, SDL_QUERY)) {
            SDL_VideoDevice *video = SDL_GetVideoDevice();
            if (video && video->StartTextInput) {
                video->StartTextInput(video);
            }
        }
    }
}

static int
SDL_SendKeyboardKeyInternal(Uint8 source, Uint8 state, SDL_Scancode scancode)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    int posted;
    SDL_Keymod modifier;
    SDL_Keycode keycode;
    Uint32 type;
    Uint8 repeat = SDL_FALSE;

    if (scancode == SDL_SCANCODE_UNKNOWN || scancode >= SDL_NUM_SCANCODES) {
        return 0;
    }

#ifdef DEBUG_KEYBOARD
    printf("The '%s' key has been %s\n", SDL_GetScancodeName(scancode),
           state == SDL_PRESSED ? "pressed" : "released");
#endif

    /* Figure out what type of event this is */
    switch (state) {
    case SDL_PRESSED:
        type = SDL_KEYDOWN;
        break;
    case SDL_RELEASED:
        type = SDL_KEYUP;
        break;
    default:
        /* Invalid state -- bail */
        return 0;
    }

    /* Drop events that don't change state */
    if (state) {
        if (keyboard->keystate[scancode]) {
            if (!(keyboard->keysource[scancode] & source)) {
                keyboard->keysource[scancode] |= source;
                return 0;
            }
            repeat = SDL_TRUE;
        }
        keyboard->keysource[scancode] |= source;
    } else {
        if (!keyboard->keystate[scancode]) {
            return 0;
        }
        keyboard->keysource[scancode] = 0;
    }

    /* Update internal keyboard state */
    keyboard->keystate[scancode] = state;

    keycode = keyboard->keymap[scancode];

    if (source == KEYBOARD_AUTORELEASE) {
        keyboard->autorelease_pending = SDL_TRUE;
    }

    /* Update modifiers state if applicable */
    switch (keycode) {
    case SDLK_LCTRL:
        modifier = KMOD_LCTRL;
        break;
    case SDLK_RCTRL:
        modifier = KMOD_RCTRL;
        break;
    case SDLK_LSHIFT:
        modifier = KMOD_LSHIFT;
        break;
    case SDLK_RSHIFT:
        modifier = KMOD_RSHIFT;
        break;
    case SDLK_LALT:
        modifier = KMOD_LALT;
        break;
    case SDLK_RALT:
        modifier = KMOD_RALT;
        break;
    case SDLK_LGUI:
        modifier = KMOD_LGUI;
        break;
    case SDLK_RGUI:
        modifier = KMOD_RGUI;
        break;
    case SDLK_MODE:
        modifier = KMOD_MODE;
        break;
    default:
        modifier = KMOD_NONE;
        break;
    }
    if (SDL_KEYDOWN == type) {
        switch (keycode) {
        case SDLK_NUMLOCKCLEAR:
            keyboard->modstate ^= KMOD_NUM;
            break;
        case SDLK_CAPSLOCK:
            keyboard->modstate ^= KMOD_CAPS;
            break;
        default:
            keyboard->modstate |= modifier;
            break;
        }
    } else {
        keyboard->modstate &= ~modifier;
    }

    /* Post the event, if desired */
    posted = 0;
    if (SDL_GetEventState(type) == SDL_ENABLE) {
        SDL_Event event;
        event.key.type = type;
        event.key.state = state;
        event.key.repeat = repeat;
        event.key.keysym.scancode = scancode;
        event.key.keysym.sym = keycode;
        event.key.keysym.mod = keyboard->modstate;
        event.key.windowID = keyboard->focus ? keyboard->focus->id : 0;
        posted = (SDL_PushEvent(&event) > 0);
    }

    /* If the keyboard is grabbed and the grabbed window is in full-screen,
       minimize the window when we receive Alt+Tab, unless the application
       has explicitly opted out of this behavior. */
    if (keycode == SDLK_TAB &&
        state == SDL_PRESSED &&
        (keyboard->modstate & KMOD_ALT) &&
        keyboard->focus &&
        (keyboard->focus->flags & SDL_WINDOW_KEYBOARD_GRABBED) &&
        (keyboard->focus->flags & SDL_WINDOW_FULLSCREEN) &&
        SDL_GetHintBoolean(SDL_HINT_ALLOW_ALT_TAB_WHILE_GRABBED, SDL_TRUE)) {
        /* We will temporarily forfeit our grab by minimizing our window, 
           allowing the user to escape the application */
        SDL_MinimizeWindow(keyboard->focus);
    }

    return (posted);
}

int
SDL_SendKeyboardKey(Uint8 state, SDL_Scancode scancode)
{
    return SDL_SendKeyboardKeyInternal(KEYBOARD_HARDWARE, state, scancode);
}

int
SDL_SendKeyboardKeyAutoRelease(SDL_Scancode scancode)
{
    return SDL_SendKeyboardKeyInternal(KEYBOARD_AUTORELEASE, SDL_PRESSED, scancode);
}

void
SDL_ReleaseAutoReleaseKeys(void)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    SDL_Scancode scancode;

    if (keyboard->autorelease_pending) {
        for (scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
            if (keyboard->keysource[scancode] == KEYBOARD_AUTORELEASE) {
                SDL_SendKeyboardKeyInternal(KEYBOARD_AUTORELEASE, SDL_RELEASED, scancode);
            }
        }
        keyboard->autorelease_pending = SDL_FALSE;
    }
}

SDL_bool
SDL_HardwareKeyboardKeyPressed(void)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    SDL_Scancode scancode;

    for (scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++scancode) {
        if ((keyboard->keysource[scancode] & KEYBOARD_HARDWARE) != 0) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

int
SDL_SendKeyboardText(const char *text)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    int posted;

    /* Don't post text events for unprintable characters */
    if ((unsigned char)*text < ' ' || *text == 127) {
        return 0;
    }

    /* Post the event, if desired */
    posted = 0;
    if (SDL_GetEventState(SDL_TEXTINPUT) == SDL_ENABLE) {
        SDL_Event event;
        event.text.type = SDL_TEXTINPUT;
        event.text.windowID = keyboard->focus ? keyboard->focus->id : 0;
        SDL_utf8strlcpy(event.text.text, text, SDL_arraysize(event.text.text));
        posted = (SDL_PushEvent(&event) > 0);
    }
    return (posted);
}

int
SDL_SendEditingText(const char *text, int start, int length)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    int posted;

    /* Post the event, if desired */
    posted = 0;
    if (SDL_GetEventState(SDL_TEXTEDITING) == SDL_ENABLE) {
        SDL_Event event;
        event.edit.type = SDL_TEXTEDITING;
        event.edit.windowID = keyboard->focus ? keyboard->focus->id : 0;
        event.edit.start = start;
        event.edit.length = length;
        SDL_utf8strlcpy(event.edit.text, text, SDL_arraysize(event.edit.text));
        posted = (SDL_PushEvent(&event) > 0);
    }
    return (posted);
}

void
SDL_KeyboardQuit(void)
{
}

const Uint8 *
SDL_GetKeyboardState(int *numkeys)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;

    if (numkeys != (int *) 0) {
        *numkeys = SDL_NUM_SCANCODES;
    }
    return keyboard->keystate;
}

SDL_Keymod
SDL_GetModState(void)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;

    return (SDL_Keymod) keyboard->modstate;
}

void
SDL_SetModState(SDL_Keymod modstate)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;

    keyboard->modstate = modstate;
}

/* Note that SDL_ToggleModState() is not a public API. SDL_SetModState() is. */
void
SDL_ToggleModState(const SDL_Keymod modstate, const SDL_bool toggle)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    if (toggle) {
        keyboard->modstate |= modstate;
    } else {
        keyboard->modstate &= ~modstate;
    }
}


SDL_Keycode
SDL_GetKeyFromScancode(SDL_Scancode scancode)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;

    if (((int)scancode) < SDL_SCANCODE_UNKNOWN || scancode >= SDL_NUM_SCANCODES) {
          SDL_InvalidParamError("scancode");
          return 0;
    }

    return keyboard->keymap[scancode];
}

SDL_Scancode
SDL_GetScancodeFromKey(SDL_Keycode key)
{
    SDL_Keyboard *keyboard = &SDL_keyboard;
    SDL_Scancode scancode;

    for (scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES;
         ++scancode) {
        if (keyboard->keymap[scancode] == key) {
            return scancode;
        }
    }
    return SDL_SCANCODE_UNKNOWN;
}

const char *
SDL_GetScancodeName(SDL_Scancode scancode)
{
    const char *name;
    if (((int)scancode) < SDL_SCANCODE_UNKNOWN || scancode >= SDL_NUM_SCANCODES) {
          SDL_InvalidParamError("scancode");
          return "";
    }

    name = SDL_scancode_names[scancode];
    if (name)
        return name;
    else
        return "";
}

SDL_Scancode SDL_GetScancodeFromName(const char *name)
{
    int i;

    if (!name || !*name) {
            SDL_InvalidParamError("name");
        return SDL_SCANCODE_UNKNOWN;
    }

    for (i = 0; i < SDL_arraysize(SDL_scancode_names); ++i) {
        if (!SDL_scancode_names[i]) {
            continue;
        }
        if (SDL_strcasecmp(name, SDL_scancode_names[i]) == 0) {
            return (SDL_Scancode)i;
        }
    }

    SDL_InvalidParamError("name");
    return SDL_SCANCODE_UNKNOWN;
}

const char *
SDL_GetKeyName(SDL_Keycode key)
{
    static char name[8];
    char *end;

    if (key & SDLK_SCANCODE_MASK) {
        return
            SDL_GetScancodeName((SDL_Scancode) (key & ~SDLK_SCANCODE_MASK));
    }

    switch (key) {
    case SDLK_RETURN:
        return SDL_GetScancodeName(SDL_SCANCODE_RETURN);
    case SDLK_ESCAPE:
        return SDL_GetScancodeName(SDL_SCANCODE_ESCAPE);
    case SDLK_BACKSPACE:
        return SDL_GetScancodeName(SDL_SCANCODE_BACKSPACE);
    case SDLK_TAB:
        return SDL_GetScancodeName(SDL_SCANCODE_TAB);
    case SDLK_SPACE:
        return SDL_GetScancodeName(SDL_SCANCODE_SPACE);
    case SDLK_DELETE:
        return SDL_GetScancodeName(SDL_SCANCODE_DELETE);
    default:
        /* Unaccented letter keys on latin keyboards are normally
           labeled in upper case (and probably on others like Greek or
           Cyrillic too, so if you happen to know for sure, please
           adapt this). */
        if (key >= 'a' && key <= 'z') {
            key -= 32;
        }

        end = SDL_UCS4ToUTF8((Uint32) key, name);
        *end = '\0';
        return name;
    }
}

SDL_Keycode
SDL_GetKeyFromName(const char *name)
{
    SDL_Keycode key;

    /* Check input */
    if (name == NULL) {
        return SDLK_UNKNOWN;
    }

    /* If it's a single UTF-8 character, then that's the keycode itself */
    key = *(const unsigned char *)name;
    if (key >= 0xF0) {
        if (SDL_strlen(name) == 4) {
            int i = 0;
            key  = (Uint16)(name[i]&0x07) << 18;
            key |= (Uint16)(name[++i]&0x3F) << 12;
            key |= (Uint16)(name[++i]&0x3F) << 6;
            key |= (Uint16)(name[++i]&0x3F);
            return key;
        }
        return SDLK_UNKNOWN;
    } else if (key >= 0xE0) {
        if (SDL_strlen(name) == 3) {
            int i = 0;
            key  = (Uint16)(name[i]&0x0F) << 12;
            key |= (Uint16)(name[++i]&0x3F) << 6;
            key |= (Uint16)(name[++i]&0x3F);
            return key;
        }
        return SDLK_UNKNOWN;
    } else if (key >= 0xC0) {
        if (SDL_strlen(name) == 2) {
            int i = 0;
            key  = (Uint16)(name[i]&0x1F) << 6;
            key |= (Uint16)(name[++i]&0x3F);
            return key;
        }
        return SDLK_UNKNOWN;
    } else {
        if (SDL_strlen(name) == 1) {
            if (key >= 'A' && key <= 'Z') {
                key += 32;
            }
            return key;
        }

        /* Get the scancode for this name, and the associated keycode */
        return SDL_default_keymap[SDL_GetScancodeFromName(name)];
    }
}

/* vi: set ts=4 sw=4 expandtab: */
