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
#include "../../include/SDL_scancode.h"

/* Linux virtual key code to SDL_Keycode mapping table
   Sources:
   - Linux kernel source input.h
*/
/* *INDENT-OFF* */
static SDL_Scancode const linux_scancode_table[] = {
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
    /*  10 */    SDL_SCANCODE_9,
    /*  11 */    SDL_SCANCODE_0,
    /*  12 */    SDL_SCANCODE_MINUS,
    /*  13 */    SDL_SCANCODE_EQUALS,
    /*  14 */    SDL_SCANCODE_BACKSPACE,
    /*  15 */    SDL_SCANCODE_TAB,
    /*  16 */    SDL_SCANCODE_Q,
    /*  17 */    SDL_SCANCODE_W,
    /*  18 */    SDL_SCANCODE_E,
    /*  19 */    SDL_SCANCODE_R,
    /*  20 */    SDL_SCANCODE_T,
    /*  21 */    SDL_SCANCODE_Y,
    /*  22 */    SDL_SCANCODE_U,
    /*  23 */    SDL_SCANCODE_I,
    /*  24 */    SDL_SCANCODE_O,
    /*  25 */    SDL_SCANCODE_P,
    /*  26 */    SDL_SCANCODE_LEFTBRACKET,
    /*  27 */    SDL_SCANCODE_RIGHTBRACKET,
    /*  28 */    SDL_SCANCODE_RETURN,
    /*  29 */    SDL_SCANCODE_LCTRL,
    /*  30 */    SDL_SCANCODE_A,
    /*  31 */    SDL_SCANCODE_S,
    /*  32 */    SDL_SCANCODE_D,
    /*  33 */    SDL_SCANCODE_F,
    /*  34 */    SDL_SCANCODE_G,
    /*  35 */    SDL_SCANCODE_H,
    /*  36 */    SDL_SCANCODE_J,
    /*  37 */    SDL_SCANCODE_K,
    /*  38 */    SDL_SCANCODE_L,
    /*  39 */    SDL_SCANCODE_SEMICOLON,
    /*  40 */    SDL_SCANCODE_APOSTROPHE,
    /*  41 */    SDL_SCANCODE_GRAVE,
    /*  42 */    SDL_SCANCODE_LSHIFT,
    /*  43 */    SDL_SCANCODE_BACKSLASH,
    /*  44 */    SDL_SCANCODE_Z,
    /*  45 */    SDL_SCANCODE_X,
    /*  46 */    SDL_SCANCODE_C,
    /*  47 */    SDL_SCANCODE_V,
    /*  48 */    SDL_SCANCODE_B,
    /*  49 */    SDL_SCANCODE_N,
    /*  50 */    SDL_SCANCODE_M,
    /*  51 */    SDL_SCANCODE_COMMA,
    /*  52 */    SDL_SCANCODE_PERIOD,
    /*  53 */    SDL_SCANCODE_SLASH,
    /*  54 */    SDL_SCANCODE_RSHIFT,
    /*  55 */    SDL_SCANCODE_KP_MULTIPLY,
    /*  56 */    SDL_SCANCODE_LALT,
    /*  57 */    SDL_SCANCODE_SPACE,
    /*  58 */    SDL_SCANCODE_CAPSLOCK,
    /*  59 */    SDL_SCANCODE_F1,
    /*  60 */    SDL_SCANCODE_F2,
    /*  61 */    SDL_SCANCODE_F3,
    /*  62 */    SDL_SCANCODE_F4,
    /*  63 */    SDL_SCANCODE_F5,
    /*  64 */    SDL_SCANCODE_F6,
    /*  65 */    SDL_SCANCODE_F7,
    /*  66 */    SDL_SCANCODE_F8,
    /*  67 */    SDL_SCANCODE_F9,
    /*  68 */    SDL_SCANCODE_F10,
    /*  69 */    SDL_SCANCODE_NUMLOCKCLEAR,
    /*  70 */    SDL_SCANCODE_SCROLLLOCK,
    /*  71 */    SDL_SCANCODE_KP_7,
    /*  72 */    SDL_SCANCODE_KP_8,
    /*  73 */    SDL_SCANCODE_KP_9,
    /*  74 */    SDL_SCANCODE_KP_MINUS,
    /*  75 */    SDL_SCANCODE_KP_4,
    /*  76 */    SDL_SCANCODE_KP_5,
    /*  77 */    SDL_SCANCODE_KP_6,
    /*  78 */    SDL_SCANCODE_KP_PLUS,
    /*  79 */    SDL_SCANCODE_KP_1,
    /*  80 */    SDL_SCANCODE_KP_2,
    /*  81 */    SDL_SCANCODE_KP_3,
    /*  82 */    SDL_SCANCODE_KP_0,
    /*  83 */    SDL_SCANCODE_KP_PERIOD,
    0,
    /*  85 */    SDL_SCANCODE_LANG5, /* KEY_ZENKAKUHANKAKU */
    /*  86 */    SDL_SCANCODE_NONUSBACKSLASH, /* KEY_102ND */
    /*  87 */    SDL_SCANCODE_F11,
    /*  88 */    SDL_SCANCODE_F12,
    /*  89 */    SDL_SCANCODE_INTERNATIONAL1, /* KEY_RO */
    /*  90 */    SDL_SCANCODE_LANG3, /* KEY_KATAKANA */
    /*  91 */    SDL_SCANCODE_LANG4, /* KEY_HIRAGANA */
    /*  92 */    SDL_SCANCODE_INTERNATIONAL4, /* KEY_HENKAN */
    /*  93 */    SDL_SCANCODE_INTERNATIONAL2, /* KEY_KATAKANAHIRAGANA */
    /*  94 */    SDL_SCANCODE_INTERNATIONAL5, /* KEY_MUHENKAN */
    /*  95 */    SDL_SCANCODE_INTERNATIONAL5, /* KEY_KPJPCOMMA */
    /*  96 */    SDL_SCANCODE_KP_ENTER,
    /*  97 */    SDL_SCANCODE_RCTRL,
    /*  98 */    SDL_SCANCODE_KP_DIVIDE,
    /*  99 */    SDL_SCANCODE_SYSREQ,
    /*  100 */    SDL_SCANCODE_RALT,
    /*  101 */    SDL_SCANCODE_UNKNOWN, /* KEY_LINEFEED */
    /*  102 */    SDL_SCANCODE_HOME,
    /*  103 */    SDL_SCANCODE_UP,
    /*  104 */    SDL_SCANCODE_PAGEUP,
    /*  105 */    SDL_SCANCODE_LEFT,
    /*  106 */    SDL_SCANCODE_RIGHT,
    /*  107 */    SDL_SCANCODE_END,
    /*  108 */    SDL_SCANCODE_DOWN,
    /*  109 */    SDL_SCANCODE_PAGEDOWN,
    /*  110 */    SDL_SCANCODE_INSERT,
    /*  111 */    SDL_SCANCODE_DELETE,
    /*  112 */    SDL_SCANCODE_UNKNOWN, /* KEY_MACRO */
    /*  113 */    SDL_SCANCODE_MUTE,
    /*  114 */    SDL_SCANCODE_VOLUMEDOWN,
    /*  115 */    SDL_SCANCODE_VOLUMEUP,
    /*  116 */    SDL_SCANCODE_POWER,
    /*  117 */    SDL_SCANCODE_KP_EQUALS,
    /*  118 */    SDL_SCANCODE_KP_PLUSMINUS,
    /*  119 */    SDL_SCANCODE_PAUSE,
    0,
    /*  121 */    SDL_SCANCODE_KP_COMMA,
    /*  122 */    SDL_SCANCODE_LANG1, /* KEY_HANGUEL */
    /*  123 */    SDL_SCANCODE_LANG2, /* KEY_HANJA */
    /*  124 */    SDL_SCANCODE_INTERNATIONAL3, /* KEY_YEN */
    /*  125 */    SDL_SCANCODE_LGUI,
    /*  126 */    SDL_SCANCODE_RGUI,
    /*  127 */    SDL_SCANCODE_APPLICATION, /* KEY_COMPOSE */
    /*  128 */    SDL_SCANCODE_STOP,
    /*  129 */    SDL_SCANCODE_AGAIN,
    /*  130 */    SDL_SCANCODE_UNKNOWN, /* KEY_PROPS */
    /*  131 */    SDL_SCANCODE_UNDO,
    /*  132 */    SDL_SCANCODE_UNKNOWN, /* KEY_FRONT */
    /*  133 */    SDL_SCANCODE_COPY,
    /*  134 */    SDL_SCANCODE_UNKNOWN, /* KEY_OPEN */
    /*  135 */    SDL_SCANCODE_PASTE,
    /*  136 */    SDL_SCANCODE_FIND,
    /*  137 */    SDL_SCANCODE_CUT,
    /*  138 */    SDL_SCANCODE_HELP,
    /*  139 */    SDL_SCANCODE_MENU,
    /*  140 */    SDL_SCANCODE_CALCULATOR,
    /*  141 */    SDL_SCANCODE_UNKNOWN, /* KEY_SETUP */
    /*  142 */    SDL_SCANCODE_SLEEP,
    /*  143 */    SDL_SCANCODE_UNKNOWN, /* KEY_WAKEUP */
    /*  144 */    SDL_SCANCODE_UNKNOWN, /* KEY_FILE */
    /*  145 */    SDL_SCANCODE_UNKNOWN, /* KEY_SENDFILE */
    /*  146 */    SDL_SCANCODE_UNKNOWN, /* KEY_DELETEFILE */
    /*  147 */    SDL_SCANCODE_UNKNOWN, /* KEY_XFER */
    /*  148 */    SDL_SCANCODE_APP1, /* KEY_PROG1 */
    /*  149 */    SDL_SCANCODE_APP2, /* KEY_PROG2 */
    /*  150 */    SDL_SCANCODE_WWW, /* KEY_WWW */
    /*  151 */    SDL_SCANCODE_UNKNOWN, /* KEY_MSDOS */
    /*  152 */    SDL_SCANCODE_UNKNOWN, /* KEY_COFFEE */
    /*  153 */    SDL_SCANCODE_UNKNOWN, /* KEY_DIRECTION */
    /*  154 */    SDL_SCANCODE_UNKNOWN, /* KEY_CYCLEWINDOWS */
    /*  155 */    SDL_SCANCODE_MAIL,
    /*  156 */    SDL_SCANCODE_AC_BOOKMARKS,
    /*  157 */    SDL_SCANCODE_COMPUTER,
    /*  158 */    SDL_SCANCODE_AC_BACK,
    /*  159 */    SDL_SCANCODE_AC_FORWARD,
    /*  160 */    SDL_SCANCODE_UNKNOWN, /* KEY_CLOSECD */
    /*  161 */    SDL_SCANCODE_EJECT, /* KEY_EJECTCD */
    /*  162 */    SDL_SCANCODE_UNKNOWN, /* KEY_EJECTCLOSECD */
    /*  163 */    SDL_SCANCODE_AUDIONEXT, /* KEY_NEXTSONG */
    /*  164 */    SDL_SCANCODE_AUDIOPLAY, /* KEY_PLAYPAUSE */
    /*  165 */    SDL_SCANCODE_AUDIOPREV, /* KEY_PREVIOUSSONG */
    /*  166 */    SDL_SCANCODE_AUDIOSTOP, /* KEY_STOPCD */
    /*  167 */    SDL_SCANCODE_UNKNOWN, /* KEY_RECORD */
    /*  168 */    SDL_SCANCODE_AUDIOREWIND, /* KEY_REWIND */
    /*  169 */    SDL_SCANCODE_UNKNOWN, /* KEY_PHONE */
    /*  170 */    SDL_SCANCODE_UNKNOWN, /* KEY_ISO */
    /*  171 */    SDL_SCANCODE_UNKNOWN, /* KEY_CONFIG */
    /*  172 */    SDL_SCANCODE_AC_HOME,
    /*  173 */    SDL_SCANCODE_AC_REFRESH,
    /*  174 */    SDL_SCANCODE_UNKNOWN, /* KEY_EXIT */
    /*  175 */    SDL_SCANCODE_UNKNOWN, /* KEY_MOVE */
    /*  176 */    SDL_SCANCODE_UNKNOWN, /* KEY_EDIT */
    /*  177 */    SDL_SCANCODE_UNKNOWN, /* KEY_SCROLLUP */
    /*  178 */    SDL_SCANCODE_UNKNOWN, /* KEY_SCROLLDOWN */
    /*  179 */    SDL_SCANCODE_KP_LEFTPAREN,
    /*  180 */    SDL_SCANCODE_KP_RIGHTPAREN,
    /*  181 */    SDL_SCANCODE_UNKNOWN, /* KEY_NEW */
    /*  182 */    SDL_SCANCODE_UNKNOWN, /* KEY_REDO */
    /*  183 */    SDL_SCANCODE_F13,
    /*  184 */    SDL_SCANCODE_F14,
    /*  185 */    SDL_SCANCODE_F15,
    /*  186 */    SDL_SCANCODE_F16,
    /*  187 */    SDL_SCANCODE_F17,
    /*  188 */    SDL_SCANCODE_F18,
    /*  189 */    SDL_SCANCODE_F19,
    /*  190 */    SDL_SCANCODE_F20,
    /*  191 */    SDL_SCANCODE_F21,
    /*  192 */    SDL_SCANCODE_F22,
    /*  193 */    SDL_SCANCODE_F23,
    /*  194 */    SDL_SCANCODE_F24,
    0, 0, 0, 0, 0,
    /*  200 */    SDL_SCANCODE_UNKNOWN, /* KEY_PLAYCD */
    /*  201 */    SDL_SCANCODE_UNKNOWN, /* KEY_PAUSECD */
    /*  202 */    SDL_SCANCODE_UNKNOWN, /* KEY_PROG3 */
    /*  203 */    SDL_SCANCODE_UNKNOWN, /* KEY_PROG4 */
    0,
    /*  205 */    SDL_SCANCODE_UNKNOWN, /* KEY_SUSPEND */
    /*  206 */    SDL_SCANCODE_UNKNOWN, /* KEY_CLOSE */
    /*  207 */    SDL_SCANCODE_UNKNOWN, /* KEY_PLAY */
    /*  208 */    SDL_SCANCODE_AUDIOFASTFORWARD, /* KEY_FASTFORWARD */
    /*  209 */    SDL_SCANCODE_UNKNOWN, /* KEY_BASSBOOST */
    /*  210 */    SDL_SCANCODE_UNKNOWN, /* KEY_PRINT */
    /*  211 */    SDL_SCANCODE_UNKNOWN, /* KEY_HP */
    /*  212 */    SDL_SCANCODE_UNKNOWN, /* KEY_CAMERA */
    /*  213 */    SDL_SCANCODE_UNKNOWN, /* KEY_SOUND */
    /*  214 */    SDL_SCANCODE_UNKNOWN, /* KEY_QUESTION */
    /*  215 */    SDL_SCANCODE_UNKNOWN, /* KEY_EMAIL */
    /*  216 */    SDL_SCANCODE_UNKNOWN, /* KEY_CHAT */
    /*  217 */    SDL_SCANCODE_AC_SEARCH,
    /*  218 */    SDL_SCANCODE_UNKNOWN, /* KEY_CONNECT */
    /*  219 */    SDL_SCANCODE_UNKNOWN, /* KEY_FINANCE */
    /*  220 */    SDL_SCANCODE_UNKNOWN, /* KEY_SPORT */
    /*  221 */    SDL_SCANCODE_UNKNOWN, /* KEY_SHOP */
    /*  222 */    SDL_SCANCODE_ALTERASE,
    /*  223 */    SDL_SCANCODE_CANCEL,
    /*  224 */    SDL_SCANCODE_BRIGHTNESSDOWN,
    /*  225 */    SDL_SCANCODE_BRIGHTNESSUP,
    /*  226 */    SDL_SCANCODE_UNKNOWN, /* KEY_MEDIA */
    /*  227 */    SDL_SCANCODE_DISPLAYSWITCH, /* KEY_SWITCHVIDEOMODE */
    /*  228 */    SDL_SCANCODE_KBDILLUMTOGGLE,
    /*  229 */    SDL_SCANCODE_KBDILLUMDOWN,
    /*  230 */    SDL_SCANCODE_KBDILLUMUP,
    /*  231 */    SDL_SCANCODE_UNKNOWN, /* KEY_SEND */
    /*  232 */    SDL_SCANCODE_UNKNOWN, /* KEY_REPLY */
    /*  233 */    SDL_SCANCODE_UNKNOWN, /* KEY_FORWARDMAIL */
    /*  234 */    SDL_SCANCODE_UNKNOWN, /* KEY_SAVE */
    /*  235 */    SDL_SCANCODE_UNKNOWN, /* KEY_DOCUMENTS */
    /*  236 */    SDL_SCANCODE_UNKNOWN, /* KEY_BATTERY */
};
/* *INDENT-ON* */
