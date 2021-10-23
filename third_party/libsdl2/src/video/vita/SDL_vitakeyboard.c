/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_DRIVER_VITA

#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/hid.h>

#include "SDL_events.h"
#include "SDL_log.h"
#include "SDL_vitavideo.h"
#include "SDL_vitakeyboard.h"
#include "../../events/SDL_keyboard_c.h"

SceHidKeyboardReport k_reports[SCE_HID_MAX_REPORT];
int keyboard_hid_handle = 0;
Uint8 prev_keys[6] = {0};
Uint8 prev_modifiers = 0;
Uint8 locks = 0;
Uint8 lock_key_down = 0;

void 
VITA_InitKeyboard(void)
{
    sceHidKeyboardEnumerate(&keyboard_hid_handle, 1);
}

void 
VITA_PollKeyboard(void)
{
    // We skip polling keyboard if no window is created
    if (Vita_Window == NULL)
        return;

    if (keyboard_hid_handle > 0)
    {
        int numReports = sceHidKeyboardRead(keyboard_hid_handle, (SceHidKeyboardReport**)&k_reports, SCE_HID_MAX_REPORT);

        if (numReports < 0) {
            keyboard_hid_handle = 0;
        }
        else if (numReports) {
            // Numlock and Capslock state changes only on a SDL_PRESSED event
            // The k_report only reports the state of the LED
            if (k_reports[numReports - 1].modifiers[1] & 0x1) {
                if (!(locks & 0x1)) {
                    SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_NUMLOCKCLEAR);
                    locks |= 0x1;
                }
            }
            else {
                if (locks & 0x1) {
                    SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_NUMLOCKCLEAR);
                    SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_NUMLOCKCLEAR);
                    SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_NUMLOCKCLEAR);
                    locks &= ~0x1;
                }
            }

            if (k_reports[numReports - 1].modifiers[1] & 0x2) {
                if (!(locks & 0x2)) {
                    SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_CAPSLOCK);
                    locks |= 0x2;
                }
            }
            else {
                if (locks & 0x2) {
                    SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_CAPSLOCK);
                    SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_CAPSLOCK);
                    SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_CAPSLOCK);
                    locks &= ~0x2;
                }
            }

            if (k_reports[numReports - 1].modifiers[1] & 0x4) {
                if (!(locks & 0x4)) {
                    SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_SCROLLLOCK);
                    locks |= 0x4;
                }
            }
            else {
                if (locks & 0x4) {
                    SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_SCROLLLOCK);
                    locks &= ~0x4;
                }
            }

            {
                Uint8 changed_modifiers = k_reports[numReports - 1].modifiers[0] ^ prev_modifiers;

                if (changed_modifiers & 0x01) {
                    if (prev_modifiers & 0x01) {
                        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_LCTRL);
                    }
                    else {
                        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_LCTRL);
                    }
                }
                if (changed_modifiers & 0x02) {
                    if (prev_modifiers & 0x02) {
                        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_LSHIFT);
                    }
                else {
                        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_LSHIFT);
                    }
                }
                if (changed_modifiers & 0x04) {
                    if (prev_modifiers & 0x04) {
                        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_LALT);
                    }
                    else {
                        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_LALT);
                    }
                }
                if (changed_modifiers & 0x08) {
                    if (prev_modifiers & 0x08) {
                        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_LGUI);
                    }
                    else {
                        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_LGUI);
                    }
                }
                if (changed_modifiers & 0x10) {
                    if (prev_modifiers & 0x10) {
                        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_RCTRL);
                    }
                    else {
                        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_RCTRL);
                    }
            }
                if (changed_modifiers & 0x20) {
                    if (prev_modifiers & 0x20) {
                        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_RSHIFT);
                    }
                    else {
                        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_RSHIFT);
                    }
                }
                if (changed_modifiers & 0x40) {
                    if (prev_modifiers & 0x40) {
                        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_RALT);
                    }
                    else {
                        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_RALT);
                    }
                }
                if (changed_modifiers & 0x80) {
                    if (prev_modifiers & 0x80) {
                        SDL_SendKeyboardKey(SDL_RELEASED, SDL_SCANCODE_RGUI);
                    }
                    else {
                        SDL_SendKeyboardKey(SDL_PRESSED, SDL_SCANCODE_RGUI);
                    }
                }
            }

            prev_modifiers = k_reports[numReports - 1].modifiers[0];

            for (int i = 0; i < 6; i++) {

                int keyCode = k_reports[numReports - 1].keycodes[i];

                if (keyCode != prev_keys[i]) {

                    if (prev_keys[i]) {
                        SDL_SendKeyboardKey(SDL_RELEASED, prev_keys[i]);
                    }
                    if (keyCode) {
                        SDL_SendKeyboardKey(SDL_PRESSED, keyCode);
                    }
                    prev_keys[i] = keyCode;
                }
            }
        }
    }
}

#endif /* SDL_VIDEO_DRIVER_VITA */

/* vi: set ts=4 sw=4 expandtab: */
