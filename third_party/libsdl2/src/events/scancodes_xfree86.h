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

#ifndef scancodes_xfree86_h_
#define scancodes_xfree86_h_

#include "../../include/SDL_scancode.h"

/* XFree86 key code to SDL scancode mapping table
   Sources:
   - atKeyNames.h from XFree86 source code
*/
/* *INDENT-OFF* */
static const SDL_Scancode xfree86_scancode_table[] = {
    /*  0 */    SDL_SCANCODE_UNKNOWN,
    /*  1 */    SDL_SCANCODE_ESCAPE,
    /*  2 */    SDL_SCANCODE_1,
    /*  3 */    SDL_SCANCODE_2,
    /*  4 */    SDL_SCANCODE_3,
    /*  5 */    SDL_SCANCODE_4,
    /*  6 */    SDL_SCANCODE_5,
    /*  7 */    SDL_SCANCODE_6,
    /*  8 */    SDL_SCANCODE_7,
    /*  9 */    SDL_SCANCODE_8,
    /*  10 */   SDL_SCANCODE_9,
    /*  11 */   SDL_SCANCODE_0,
    /*  12 */   SDL_SCANCODE_MINUS,
    /*  13 */   SDL_SCANCODE_EQUALS,
    /*  14 */   SDL_SCANCODE_BACKSPACE,
    /*  15 */   SDL_SCANCODE_TAB,
    /*  16 */   SDL_SCANCODE_Q,
    /*  17 */   SDL_SCANCODE_W,
    /*  18 */   SDL_SCANCODE_E,
    /*  19 */   SDL_SCANCODE_R,
    /*  20 */   SDL_SCANCODE_T,
    /*  21 */   SDL_SCANCODE_Y,
    /*  22 */   SDL_SCANCODE_U,
    /*  23 */   SDL_SCANCODE_I,
    /*  24 */   SDL_SCANCODE_O,
    /*  25 */   SDL_SCANCODE_P,
    /*  26 */   SDL_SCANCODE_LEFTBRACKET,
    /*  27 */   SDL_SCANCODE_RIGHTBRACKET,
    /*  28 */   SDL_SCANCODE_RETURN,
    /*  29 */   SDL_SCANCODE_LCTRL,
    /*  30 */   SDL_SCANCODE_A,
    /*  31 */   SDL_SCANCODE_S,
    /*  32 */   SDL_SCANCODE_D,
    /*  33 */   SDL_SCANCODE_F,
    /*  34 */   SDL_SCANCODE_G,
    /*  35 */   SDL_SCANCODE_H,
    /*  36 */   SDL_SCANCODE_J,
    /*  37 */   SDL_SCANCODE_K,
    /*  38 */   SDL_SCANCODE_L,
    /*  39 */   SDL_SCANCODE_SEMICOLON,
    /*  40 */   SDL_SCANCODE_APOSTROPHE,
    /*  41 */   SDL_SCANCODE_GRAVE,
    /*  42 */   SDL_SCANCODE_LSHIFT,
    /*  43 */   SDL_SCANCODE_BACKSLASH,
    /*  44 */   SDL_SCANCODE_Z,
    /*  45 */   SDL_SCANCODE_X,
    /*  46 */   SDL_SCANCODE_C,
    /*  47 */   SDL_SCANCODE_V,
    /*  48 */   SDL_SCANCODE_B,
    /*  49 */   SDL_SCANCODE_N,
    /*  50 */   SDL_SCANCODE_M,
    /*  51 */   SDL_SCANCODE_COMMA,
    /*  52 */   SDL_SCANCODE_PERIOD,
    /*  53 */   SDL_SCANCODE_SLASH,
    /*  54 */   SDL_SCANCODE_RSHIFT,
    /*  55 */   SDL_SCANCODE_KP_MULTIPLY,
    /*  56 */   SDL_SCANCODE_LALT,
    /*  57 */   SDL_SCANCODE_SPACE,
    /*  58 */   SDL_SCANCODE_CAPSLOCK,
    /*  59 */   SDL_SCANCODE_F1,
    /*  60 */   SDL_SCANCODE_F2,
    /*  61 */   SDL_SCANCODE_F3,
    /*  62 */   SDL_SCANCODE_F4,
    /*  63 */   SDL_SCANCODE_F5,
    /*  64 */   SDL_SCANCODE_F6,
    /*  65 */   SDL_SCANCODE_F7,
    /*  66 */   SDL_SCANCODE_F8,
    /*  67 */   SDL_SCANCODE_F9,
    /*  68 */   SDL_SCANCODE_F10,
    /*  69 */   SDL_SCANCODE_NUMLOCKCLEAR,
    /*  70 */   SDL_SCANCODE_SCROLLLOCK,
    /*  71 */   SDL_SCANCODE_KP_7,
    /*  72 */   SDL_SCANCODE_KP_8,
    /*  73 */   SDL_SCANCODE_KP_9,
    /*  74 */   SDL_SCANCODE_KP_MINUS,
    /*  75 */   SDL_SCANCODE_KP_4,
    /*  76 */   SDL_SCANCODE_KP_5,
    /*  77 */   SDL_SCANCODE_KP_6,
    /*  78 */   SDL_SCANCODE_KP_PLUS,
    /*  79 */   SDL_SCANCODE_KP_1,
    /*  80 */   SDL_SCANCODE_KP_2,
    /*  81 */   SDL_SCANCODE_KP_3,
    /*  82 */   SDL_SCANCODE_KP_0,
    /*  83 */   SDL_SCANCODE_KP_PERIOD,
    /*  84 */   SDL_SCANCODE_SYSREQ,
    /*  85 */   SDL_SCANCODE_MODE,
    /*  86 */   SDL_SCANCODE_NONUSBACKSLASH,
    /*  87 */   SDL_SCANCODE_F11,
    /*  88 */   SDL_SCANCODE_F12,
    /*  89 */   SDL_SCANCODE_HOME,
    /*  90 */   SDL_SCANCODE_UP,
    /*  91 */   SDL_SCANCODE_PAGEUP,
    /*  92 */   SDL_SCANCODE_LEFT,
    /*  93 */   SDL_SCANCODE_BRIGHTNESSDOWN, /* on PowerBook G4 / KEY_Begin */
    /*  94 */   SDL_SCANCODE_RIGHT,
    /*  95 */   SDL_SCANCODE_END,
    /*  96 */   SDL_SCANCODE_DOWN,
    /*  97 */   SDL_SCANCODE_PAGEDOWN,
    /*  98 */   SDL_SCANCODE_INSERT,
    /*  99 */   SDL_SCANCODE_DELETE,
    /*  100 */  SDL_SCANCODE_KP_ENTER,
    /*  101 */  SDL_SCANCODE_RCTRL,
    /*  102 */  SDL_SCANCODE_PAUSE,
    /*  103 */  SDL_SCANCODE_PRINTSCREEN,
    /*  104 */  SDL_SCANCODE_KP_DIVIDE,
    /*  105 */  SDL_SCANCODE_RALT,
    /*  106 */  SDL_SCANCODE_UNKNOWN, /* BREAK */
    /*  107 */  SDL_SCANCODE_LGUI,
    /*  108 */  SDL_SCANCODE_RGUI,
    /*  109 */  SDL_SCANCODE_APPLICATION,
    /*  110 */  SDL_SCANCODE_F13,
    /*  111 */  SDL_SCANCODE_F14,
    /*  112 */  SDL_SCANCODE_F15,
    /*  113 */  SDL_SCANCODE_F16,
    /*  114 */  SDL_SCANCODE_F17,
    /*  115 */  SDL_SCANCODE_INTERNATIONAL1, /* \_ */
    /*  116 */  SDL_SCANCODE_UNKNOWN, /* is translated to XK_ISO_Level3_Shift by my X server, but I have no keyboard that generates this code, so I don't know what the correct SDL_SCANCODE_* for it is */
    /*  117 */  SDL_SCANCODE_UNKNOWN,
    /*  118 */  SDL_SCANCODE_KP_EQUALS,
    /*  119 */  SDL_SCANCODE_UNKNOWN,
    /*  120 */  SDL_SCANCODE_UNKNOWN,
    /*  121 */  SDL_SCANCODE_INTERNATIONAL4, /* Henkan_Mode */
    /*  122 */  SDL_SCANCODE_UNKNOWN,
    /*  123 */  SDL_SCANCODE_INTERNATIONAL5, /* Muhenkan */
    /*  124 */  SDL_SCANCODE_UNKNOWN,
    /*  125 */  SDL_SCANCODE_INTERNATIONAL3, /* Yen */
    /*  126 */  SDL_SCANCODE_UNKNOWN,
    /*  127 */  SDL_SCANCODE_UNKNOWN,
    /*  128 */  SDL_SCANCODE_UNKNOWN,
    /*  129 */  SDL_SCANCODE_UNKNOWN,
    /*  130 */  SDL_SCANCODE_UNKNOWN,
    /*  131 */  SDL_SCANCODE_UNKNOWN,
    /*  132 */  SDL_SCANCODE_POWER,
    /*  133 */  SDL_SCANCODE_MUTE,
    /*  134 */  SDL_SCANCODE_VOLUMEDOWN,
    /*  135 */  SDL_SCANCODE_VOLUMEUP,
    /*  136 */  SDL_SCANCODE_HELP,
    /*  137 */  SDL_SCANCODE_STOP,
    /*  138 */  SDL_SCANCODE_AGAIN,
    /*  139 */  SDL_SCANCODE_UNKNOWN, /* PROPS */
    /*  140 */  SDL_SCANCODE_UNDO,
    /*  141 */  SDL_SCANCODE_UNKNOWN, /* FRONT */
    /*  142 */  SDL_SCANCODE_COPY,
    /*  143 */  SDL_SCANCODE_UNKNOWN, /* OPEN */
    /*  144 */  SDL_SCANCODE_PASTE,
    /*  145 */  SDL_SCANCODE_FIND,
    /*  146 */  SDL_SCANCODE_CUT,
};

/* for wireless usb keyboard (manufacturer TRUST) without numpad. */
static const SDL_Scancode xfree86_scancode_table2[] = {
    /*  0 */    SDL_SCANCODE_UNKNOWN,
    /*  1 */    SDL_SCANCODE_ESCAPE,
    /*  2 */    SDL_SCANCODE_1,
    /*  3 */    SDL_SCANCODE_2,
    /*  4 */    SDL_SCANCODE_3,
    /*  5 */    SDL_SCANCODE_4,
    /*  6 */    SDL_SCANCODE_5,
    /*  7 */    SDL_SCANCODE_6,
    /*  8 */    SDL_SCANCODE_7,
    /*  9 */    SDL_SCANCODE_8,
    /*  10 */   SDL_SCANCODE_9,
    /*  11 */   SDL_SCANCODE_0,
    /*  12 */   SDL_SCANCODE_MINUS,
    /*  13 */   SDL_SCANCODE_EQUALS,
    /*  14 */   SDL_SCANCODE_BACKSPACE,
    /*  15 */   SDL_SCANCODE_TAB,
    /*  16 */   SDL_SCANCODE_Q,
    /*  17 */   SDL_SCANCODE_W,
    /*  18 */   SDL_SCANCODE_E,
    /*  19 */   SDL_SCANCODE_R,
    /*  20 */   SDL_SCANCODE_T,
    /*  21 */   SDL_SCANCODE_Y,
    /*  22 */   SDL_SCANCODE_U,
    /*  23 */   SDL_SCANCODE_I,
    /*  24 */   SDL_SCANCODE_O,
    /*  25 */   SDL_SCANCODE_P,
    /*  26 */   SDL_SCANCODE_LEFTBRACKET,
    /*  27 */   SDL_SCANCODE_RIGHTBRACKET,
    /*  28 */   SDL_SCANCODE_RETURN,
    /*  29 */   SDL_SCANCODE_LCTRL,
    /*  30 */   SDL_SCANCODE_A,
    /*  31 */   SDL_SCANCODE_S,
    /*  32 */   SDL_SCANCODE_D,
    /*  33 */   SDL_SCANCODE_F,
    /*  34 */   SDL_SCANCODE_G,
    /*  35 */   SDL_SCANCODE_H,
    /*  36 */   SDL_SCANCODE_J,
    /*  37 */   SDL_SCANCODE_K,
    /*  38 */   SDL_SCANCODE_L,
    /*  39 */   SDL_SCANCODE_SEMICOLON,
    /*  40 */   SDL_SCANCODE_APOSTROPHE,
    /*  41 */   SDL_SCANCODE_GRAVE,
    /*  42 */   SDL_SCANCODE_LSHIFT,
    /*  43 */   SDL_SCANCODE_BACKSLASH,
    /*  44 */   SDL_SCANCODE_Z,
    /*  45 */   SDL_SCANCODE_X,
    /*  46 */   SDL_SCANCODE_C,
    /*  47 */   SDL_SCANCODE_V,
    /*  48 */   SDL_SCANCODE_B,
    /*  49 */   SDL_SCANCODE_N,
    /*  50 */   SDL_SCANCODE_M,
    /*  51 */   SDL_SCANCODE_COMMA,
    /*  52 */   SDL_SCANCODE_PERIOD,
    /*  53 */   SDL_SCANCODE_SLASH,
    /*  54 */   SDL_SCANCODE_RSHIFT,
    /*  55 */   SDL_SCANCODE_KP_MULTIPLY,
    /*  56 */   SDL_SCANCODE_LALT,
    /*  57 */   SDL_SCANCODE_SPACE,
    /*  58 */   SDL_SCANCODE_CAPSLOCK,
    /*  59 */   SDL_SCANCODE_F1,
    /*  60 */   SDL_SCANCODE_F2,
    /*  61 */   SDL_SCANCODE_F3,
    /*  62 */   SDL_SCANCODE_F4,
    /*  63 */   SDL_SCANCODE_F5,
    /*  64 */   SDL_SCANCODE_F6,
    /*  65 */   SDL_SCANCODE_F7,
    /*  66 */   SDL_SCANCODE_F8,
    /*  67 */   SDL_SCANCODE_F9,
    /*  68 */   SDL_SCANCODE_F10,
    /*  69 */   SDL_SCANCODE_NUMLOCKCLEAR,
    /*  70 */   SDL_SCANCODE_SCROLLLOCK,
    /*  71 */   SDL_SCANCODE_KP_7,
    /*  72 */   SDL_SCANCODE_KP_8,
    /*  73 */   SDL_SCANCODE_KP_9,
    /*  74 */   SDL_SCANCODE_KP_MINUS,
    /*  75 */   SDL_SCANCODE_KP_4,
    /*  76 */   SDL_SCANCODE_KP_5,
    /*  77 */   SDL_SCANCODE_KP_6,
    /*  78 */   SDL_SCANCODE_KP_PLUS,
    /*  79 */   SDL_SCANCODE_KP_1,
    /*  80 */   SDL_SCANCODE_KP_2,
    /*  81 */   SDL_SCANCODE_KP_3,
    /*  82 */   SDL_SCANCODE_KP_0,
    /*  83 */   SDL_SCANCODE_KP_PERIOD,
    /*  84 */   SDL_SCANCODE_SYSREQ,    /* ???? */
    /*  85 */   SDL_SCANCODE_MODE,      /* ???? */
    /*  86 */   SDL_SCANCODE_NONUSBACKSLASH,
    /*  87 */   SDL_SCANCODE_F11,
    /*  88 */   SDL_SCANCODE_F12,
    /*  89 */   SDL_SCANCODE_INTERNATIONAL1, /* \_ */
    /*  90 */   SDL_SCANCODE_UNKNOWN,   /* Katakana */
    /*  91 */   SDL_SCANCODE_UNKNOWN,   /* Hiragana */
    /*  92 */   SDL_SCANCODE_INTERNATIONAL4, /* Henkan_Mode */
    /*  93 */   SDL_SCANCODE_INTERNATIONAL2, /* Hiragana_Katakana */
    /*  94 */   SDL_SCANCODE_INTERNATIONAL5, /* Muhenkan */
    /*  95 */   SDL_SCANCODE_UNKNOWN,
    /*  96 */   SDL_SCANCODE_KP_ENTER,
    /*  97 */   SDL_SCANCODE_RCTRL,
    /*  98 */   SDL_SCANCODE_KP_DIVIDE,
    /*  99 */   SDL_SCANCODE_PRINTSCREEN,
    /* 100 */   SDL_SCANCODE_RALT,      /* ISO_Level3_Shift, ALTGR, RALT */
    /* 101 */   SDL_SCANCODE_UNKNOWN,   /* Linefeed */
    /* 102 */   SDL_SCANCODE_HOME,
    /* 103 */   SDL_SCANCODE_UP,
    /* 104 */   SDL_SCANCODE_PAGEUP,
    /* 105 */   SDL_SCANCODE_LEFT,
    /* 106 */   SDL_SCANCODE_RIGHT,
    /* 107 */   SDL_SCANCODE_END,
    /* 108 */   SDL_SCANCODE_DOWN,
    /* 109 */   SDL_SCANCODE_PAGEDOWN,
    /* 110 */   SDL_SCANCODE_INSERT,
    /* 111 */   SDL_SCANCODE_DELETE,
    /* 112 */   SDL_SCANCODE_UNKNOWN,
    /* 113 */   SDL_SCANCODE_MUTE,
    /* 114 */   SDL_SCANCODE_VOLUMEDOWN,
    /* 115 */   SDL_SCANCODE_VOLUMEUP,
    /* 116 */   SDL_SCANCODE_POWER,
    /* 117 */   SDL_SCANCODE_KP_EQUALS,
    /* 118 */   SDL_SCANCODE_KP_PLUSMINUS, /* plusminus */
    /* 119 */   SDL_SCANCODE_PAUSE,
    /* 120 */   SDL_SCANCODE_UNKNOWN,   /* XF86LaunchA */
    /* 121 */   SDL_SCANCODE_KP_COMMA,  /* KP_Decimal */
    /* 122 */   SDL_SCANCODE_LANG1,     /* Hangul */
    /* 123 */   SDL_SCANCODE_LANG2,     /* Hangul_Hanja */
    /* 124 */   SDL_SCANCODE_INTERNATIONAL3, /* Yen */
    /* 125 */   SDL_SCANCODE_LGUI,
    /* 126 */   SDL_SCANCODE_RGUI,
    /* 127 */   SDL_SCANCODE_APPLICATION,
    /* 128 */   SDL_SCANCODE_CANCEL,
    /* 129 */   SDL_SCANCODE_AGAIN,
    /* 130 */   SDL_SCANCODE_UNKNOWN,   /* SunProps */
    /* 131 */   SDL_SCANCODE_UNDO,
    /* 132 */   SDL_SCANCODE_UNKNOWN,   /* SunFront */
    /* 133 */   SDL_SCANCODE_COPY,
    /* 134 */   SDL_SCANCODE_UNKNOWN,   /* SunOpen */
    /* 135 */   SDL_SCANCODE_PASTE,
    /* 136 */   SDL_SCANCODE_FIND,
    /* 137 */   SDL_SCANCODE_CUT,
    /* 138 */   SDL_SCANCODE_HELP,
    /* 139 */   SDL_SCANCODE_MENU,   /* XF86MenuKB */
    /* 140 */   SDL_SCANCODE_CALCULATOR,
    /* 141 */   SDL_SCANCODE_UNKNOWN,
    /* 142 */   SDL_SCANCODE_SLEEP,
    /* 143 */   SDL_SCANCODE_UNKNOWN,   /* XF86WakeUp */
    /* 144 */   SDL_SCANCODE_UNKNOWN,   /* XF86Explorer */
    /* 145 */   SDL_SCANCODE_UNKNOWN,   /* XF86Send */
    /* 146 */   SDL_SCANCODE_UNKNOWN,
    /* 147 */   SDL_SCANCODE_UNKNOWN,   /* XF86Xfer */
    /* 148 */   SDL_SCANCODE_APP1,      /* XF86Launch1 */
    /* 149 */   SDL_SCANCODE_APP2,      /* XF86Launch2 */
    /* 150 */   SDL_SCANCODE_WWW,
    /* 151 */   SDL_SCANCODE_UNKNOWN,   /* XF86DOS */
    /* 152 */   SDL_SCANCODE_UNKNOWN,   /* XF86ScreenSaver */
    /* 153 */   SDL_SCANCODE_UNKNOWN,
    /* 154 */   SDL_SCANCODE_UNKNOWN,   /* XF86RotateWindows */
    /* 155 */   SDL_SCANCODE_MAIL,
    /* 156 */   SDL_SCANCODE_AC_BOOKMARKS,   /* XF86Favorites */
    /* 157 */   SDL_SCANCODE_COMPUTER,
    /* 158 */   SDL_SCANCODE_AC_BACK,
    /* 159 */   SDL_SCANCODE_AC_FORWARD,
    /* 160 */   SDL_SCANCODE_UNKNOWN,
    /* 161 */   SDL_SCANCODE_EJECT,
    /* 162 */   SDL_SCANCODE_EJECT,
    /* 163 */   SDL_SCANCODE_AUDIONEXT,
    /* 164 */   SDL_SCANCODE_AUDIOPLAY,
    /* 165 */   SDL_SCANCODE_AUDIOPREV,
    /* 166 */   SDL_SCANCODE_AUDIOSTOP,
    /* 167 */   SDL_SCANCODE_UNKNOWN,   /* XF86AudioRecord */
    /* 168 */   SDL_SCANCODE_AUDIOREWIND,   /* XF86AudioRewind */
    /* 169 */   SDL_SCANCODE_UNKNOWN,   /* XF86Phone */
    /* 170 */   SDL_SCANCODE_UNKNOWN,
    /* 171 */   SDL_SCANCODE_F13,       /* XF86Tools */
    /* 172 */   SDL_SCANCODE_AC_HOME,
    /* 173 */   SDL_SCANCODE_AC_REFRESH,
    /* 174 */   SDL_SCANCODE_UNKNOWN,   /* XF86Close */
    /* 175 */   SDL_SCANCODE_UNKNOWN,
    /* 176 */   SDL_SCANCODE_UNKNOWN,
    /* 177 */   SDL_SCANCODE_UNKNOWN,   /* XF86ScrollUp */
    /* 178 */   SDL_SCANCODE_UNKNOWN,   /* XF86ScrollDown */
    /* 179 */   SDL_SCANCODE_KP_LEFTPAREN,  /* parenleft */
    /* 180 */   SDL_SCANCODE_KP_RIGHTPAREN, /* parenright */
    /* 181 */   SDL_SCANCODE_UNKNOWN,   /* XF86New */
    /* 182 */   SDL_SCANCODE_AGAIN,
    /* 183 */   SDL_SCANCODE_F13,       /* XF86Tools */
    /* 184 */   SDL_SCANCODE_F14,       /* XF86Launch5 */
    /* 185 */   SDL_SCANCODE_F15,       /* XF86Launch6 */
    /* 186 */   SDL_SCANCODE_F16,       /* XF86Launch7 */
    /* 187 */   SDL_SCANCODE_F17,       /* XF86Launch8 */
    /* 188 */   SDL_SCANCODE_F18,       /* XF86Launch9 */
    /* 189 */   SDL_SCANCODE_F19,       /* null keysym */
    /* 190 */   SDL_SCANCODE_F20,
    /* 191 */   SDL_SCANCODE_UNKNOWN,
    /* 192 */   SDL_SCANCODE_UNKNOWN,   /* XF86TouchpadToggle */
    /* 193 */   SDL_SCANCODE_UNKNOWN,
    /* 194 */   SDL_SCANCODE_UNKNOWN,
    /* 195 */   SDL_SCANCODE_MODE,
    /* 196 */   SDL_SCANCODE_UNKNOWN,
    /* 197 */   SDL_SCANCODE_UNKNOWN,
    /* 198 */   SDL_SCANCODE_UNKNOWN,
    /* 199 */   SDL_SCANCODE_UNKNOWN,
    /* 200 */   SDL_SCANCODE_AUDIOPLAY,
    /* 201 */   SDL_SCANCODE_UNKNOWN,   /* XF86AudioPause */
    /* 202 */   SDL_SCANCODE_UNKNOWN,   /* XF86Launch3 */
    /* 203 */   SDL_SCANCODE_UNKNOWN,   /* XF86Launch4 */
    /* 204 */   SDL_SCANCODE_UNKNOWN,   /* XF86LaunchB */
    /* 205 */   SDL_SCANCODE_UNKNOWN,   /* XF86Suspend */
    /* 206 */   SDL_SCANCODE_UNKNOWN,   /* XF86Close */
    /* 207 */   SDL_SCANCODE_AUDIOPLAY,
    /* 208 */   SDL_SCANCODE_AUDIOFASTFORWARD,
    /* 209 */   SDL_SCANCODE_UNKNOWN,
    /* 210 */   SDL_SCANCODE_PRINTSCREEN,
    /* 211 */   SDL_SCANCODE_UNKNOWN,
    /* 212 */   SDL_SCANCODE_UNKNOWN,   /* XF86WebCam */
    /* 213 */   SDL_SCANCODE_UNKNOWN,
    /* 214 */   SDL_SCANCODE_UNKNOWN,
    /* 215 */   SDL_SCANCODE_MAIL,
    /* 216 */   SDL_SCANCODE_UNKNOWN,
    /* 217 */   SDL_SCANCODE_AC_SEARCH,
    /* 218 */   SDL_SCANCODE_UNKNOWN,
    /* 219 */   SDL_SCANCODE_UNKNOWN,   /* XF86Finance */
    /* 220 */   SDL_SCANCODE_UNKNOWN,
    /* 221 */   SDL_SCANCODE_UNKNOWN,   /* XF86Shop */
    /* 222 */   SDL_SCANCODE_UNKNOWN,
    /* 223 */   SDL_SCANCODE_STOP,
    /* 224 */   SDL_SCANCODE_BRIGHTNESSDOWN,
    /* 225 */   SDL_SCANCODE_BRIGHTNESSUP,
    /* 226 */   SDL_SCANCODE_MEDIASELECT,
    /* 227 */   SDL_SCANCODE_DISPLAYSWITCH,
    /* 228 */   SDL_SCANCODE_KBDILLUMTOGGLE,
    /* 229 */   SDL_SCANCODE_KBDILLUMDOWN,
    /* 230 */   SDL_SCANCODE_KBDILLUMUP,
    /* 231 */   SDL_SCANCODE_UNKNOWN,   /* XF86Send */
    /* 232 */   SDL_SCANCODE_UNKNOWN,   /* XF86Reply */
    /* 233 */   SDL_SCANCODE_UNKNOWN,   /* XF86MailForward */
    /* 234 */   SDL_SCANCODE_UNKNOWN,   /* XF86Save */
    /* 235 */   SDL_SCANCODE_UNKNOWN,   /* XF86Documents */
    /* 236 */   SDL_SCANCODE_UNKNOWN,   /* XF86Battery */
    /* 237 */   SDL_SCANCODE_UNKNOWN,   /* XF86Bluetooth */
    /* 238 */   SDL_SCANCODE_UNKNOWN,   /* XF86WLAN */
};

/* Xvnc / Xtightvnc scancodes from xmodmap -pk */
static const SDL_Scancode xvnc_scancode_table[] = {
    /*  0 */    SDL_SCANCODE_LCTRL,
    /*  1 */    SDL_SCANCODE_RCTRL,
    /*  2 */    SDL_SCANCODE_LSHIFT,
    /*  3 */    SDL_SCANCODE_RSHIFT,
    /*  4 */    SDL_SCANCODE_UNKNOWN, /* Meta_L */
    /*  5 */    SDL_SCANCODE_UNKNOWN, /* Meta_R */
    /*  6 */    SDL_SCANCODE_LALT,
    /*  7 */    SDL_SCANCODE_RALT,
    /*  8 */    SDL_SCANCODE_SPACE,
    /*  9 */    SDL_SCANCODE_0,
    /*  10 */   SDL_SCANCODE_1,
    /*  11 */   SDL_SCANCODE_2,
    /*  12 */   SDL_SCANCODE_3,
    /*  13 */   SDL_SCANCODE_4,
    /*  14 */   SDL_SCANCODE_5,
    /*  15 */   SDL_SCANCODE_6,
    /*  16 */   SDL_SCANCODE_7,
    /*  17 */   SDL_SCANCODE_8,
    /*  18 */   SDL_SCANCODE_9,
    /*  19 */   SDL_SCANCODE_MINUS,
    /*  20 */   SDL_SCANCODE_EQUALS,
    /*  21 */   SDL_SCANCODE_LEFTBRACKET,
    /*  22 */   SDL_SCANCODE_RIGHTBRACKET,
    /*  23 */   SDL_SCANCODE_SEMICOLON,
    /*  24 */   SDL_SCANCODE_APOSTROPHE,
    /*  25 */   SDL_SCANCODE_GRAVE,
    /*  26 */   SDL_SCANCODE_COMMA,
    /*  27 */   SDL_SCANCODE_PERIOD,
    /*  28 */   SDL_SCANCODE_SLASH,
    /*  29 */   SDL_SCANCODE_BACKSLASH,
    /*  30 */   SDL_SCANCODE_A,
    /*  31 */   SDL_SCANCODE_B,
    /*  32 */   SDL_SCANCODE_C,
    /*  33 */   SDL_SCANCODE_D,
    /*  34 */   SDL_SCANCODE_E,
    /*  35 */   SDL_SCANCODE_F,
    /*  36 */   SDL_SCANCODE_G,
    /*  37 */   SDL_SCANCODE_H,
    /*  38 */   SDL_SCANCODE_I,
    /*  39 */   SDL_SCANCODE_J,
    /*  40 */   SDL_SCANCODE_K,
    /*  41 */   SDL_SCANCODE_L,
    /*  42 */   SDL_SCANCODE_M,
    /*  43 */   SDL_SCANCODE_N,
    /*  44 */   SDL_SCANCODE_O,
    /*  45 */   SDL_SCANCODE_P,
    /*  46 */   SDL_SCANCODE_Q,
    /*  47 */   SDL_SCANCODE_R,
    /*  48 */   SDL_SCANCODE_S,
    /*  49 */   SDL_SCANCODE_T,
    /*  50 */   SDL_SCANCODE_U,
    /*  51 */   SDL_SCANCODE_V,
    /*  52 */   SDL_SCANCODE_W,
    /*  53 */   SDL_SCANCODE_X,
    /*  54 */   SDL_SCANCODE_Y,
    /*  55 */   SDL_SCANCODE_Z,
    /*  56 */   SDL_SCANCODE_BACKSPACE,
    /*  57 */   SDL_SCANCODE_RETURN,
    /*  58 */   SDL_SCANCODE_TAB,
    /*  59 */   SDL_SCANCODE_ESCAPE,
    /*  60 */   SDL_SCANCODE_DELETE,
    /*  61 */   SDL_SCANCODE_HOME,
    /*  62 */   SDL_SCANCODE_END,
    /*  63 */   SDL_SCANCODE_PAGEUP,
    /*  64 */   SDL_SCANCODE_PAGEDOWN,
    /*  65 */   SDL_SCANCODE_UP,
    /*  66 */   SDL_SCANCODE_DOWN,
    /*  67 */   SDL_SCANCODE_LEFT,
    /*  68 */   SDL_SCANCODE_RIGHT,
    /*  69 */   SDL_SCANCODE_F1,
    /*  70 */   SDL_SCANCODE_F2,
    /*  71 */   SDL_SCANCODE_F3,
    /*  72 */   SDL_SCANCODE_F4,
    /*  73 */   SDL_SCANCODE_F5,
    /*  74 */   SDL_SCANCODE_F6,
    /*  75 */   SDL_SCANCODE_F7,
    /*  76 */   SDL_SCANCODE_F8,
    /*  77 */   SDL_SCANCODE_F9,
    /*  78 */   SDL_SCANCODE_F10,
    /*  79 */   SDL_SCANCODE_F11,
    /*  80 */   SDL_SCANCODE_F12,
};

#endif /* scancodes_xfree86_h_ */

/* *INDENT-ON* */
