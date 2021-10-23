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
#include "SDL_mouse.h"
#include "SDL_vitavideo.h"
#include "SDL_vitamouse_c.h"
#include "../../events/SDL_mouse_c.h"

SceHidMouseReport m_reports[SCE_HID_MAX_REPORT];
int mouse_hid_handle = 0;
Uint8 prev_buttons = 0;

void 
VITA_InitMouse(void)
{
    sceHidMouseEnumerate(&mouse_hid_handle, 1);
}

void 
VITA_PollMouse(void)
{
    // We skip polling mouse if no window is created
    if (Vita_Window == NULL)
        return;

    if (mouse_hid_handle > 0)
    {
        int numReports = sceHidMouseRead(mouse_hid_handle, (SceHidMouseReport**)&m_reports, SCE_HID_MAX_REPORT);
        if (numReports > 0)
        {
            for (int i = 0; i <= numReports - 1; i++)
            {
                Uint8 changed_buttons = m_reports[i].buttons ^ prev_buttons;

                if (changed_buttons & 0x1) {
                    if (prev_buttons & 0x1)
                        SDL_SendMouseButton(Vita_Window, 0, SDL_RELEASED, SDL_BUTTON_LEFT);
                    else
                        SDL_SendMouseButton(Vita_Window, 0, SDL_PRESSED, SDL_BUTTON_LEFT);
                }
                if (changed_buttons & 0x2) {
                    if (prev_buttons & 0x2)
                        SDL_SendMouseButton(Vita_Window, 0, SDL_RELEASED, SDL_BUTTON_RIGHT);
                    else
                        SDL_SendMouseButton(Vita_Window, 0, SDL_PRESSED, SDL_BUTTON_RIGHT);
                }
                if (changed_buttons & 0x4) {
                    if (prev_buttons & 0x4)
                        SDL_SendMouseButton(Vita_Window, 0, SDL_RELEASED, SDL_BUTTON_MIDDLE);
                    else
                        SDL_SendMouseButton(Vita_Window, 0, SDL_PRESSED, SDL_BUTTON_MIDDLE);
                }

                prev_buttons = m_reports[i].buttons;

                if (m_reports[i].rel_x || m_reports[i].rel_y)
                {
                    SDL_SendMouseMotion(Vita_Window, 0, 1, m_reports[i].rel_x, m_reports[i].rel_y);
                }
            }
        }
    }
}

#endif /* SDL_VIDEO_DRIVER_VITA */

/* vi: set ts=4 sw=4 expandtab: */
