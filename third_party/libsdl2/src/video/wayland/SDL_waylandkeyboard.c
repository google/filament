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

#if SDL_VIDEO_DRIVER_WAYLAND

#include "../SDL_sysvideo.h"
#include "SDL_waylandvideo.h"
#include "SDL_waylandevents_c.h"
#include "text-input-unstable-v3-client-protocol.h"

int
Wayland_InitKeyboard(_THIS)
{
#ifdef SDL_USE_IME
    SDL_VideoData *driverdata = _this->driverdata;
    if (driverdata->text_input_manager == NULL) {
        SDL_IME_Init();
    }
#endif

    return 0;
}

void
Wayland_QuitKeyboard(_THIS)
{
#ifdef SDL_USE_IME
    SDL_VideoData *driverdata = _this->driverdata;
    if (driverdata->text_input_manager == NULL) {
        SDL_IME_Quit();
    }
#endif
}

void
Wayland_StartTextInput(_THIS)
{
    SDL_VideoData *driverdata = _this->driverdata;

    if (driverdata->text_input_manager) {
        struct SDL_WaylandInput *input = driverdata->input;
        if (input != NULL && input->text_input) {
            const SDL_Rect *rect = &input->text_input->cursor_rect;

            /* For some reason this has to be done twice, it appears to be a
             * bug in mutter? Maybe?
             * -flibit
             */
            zwp_text_input_v3_enable(input->text_input->text_input);
            zwp_text_input_v3_commit(input->text_input->text_input);
            zwp_text_input_v3_enable(input->text_input->text_input);
            zwp_text_input_v3_commit(input->text_input->text_input);

            /* Now that it's enabled, set the input properties */
            zwp_text_input_v3_set_content_type(input->text_input->text_input,
                                               ZWP_TEXT_INPUT_V3_CONTENT_HINT_NONE,
                                               ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL);
            if (!SDL_RectEmpty(rect)) {
                /* This gets reset on enable so we have to cache it */
                zwp_text_input_v3_set_cursor_rectangle(input->text_input->text_input,
                                                       rect->x,
                                                       rect->y,
                                                       rect->w,
                                                       rect->h);
            }
            zwp_text_input_v3_commit(input->text_input->text_input);
        }
    }
}

void
Wayland_StopTextInput(_THIS)
{
    SDL_VideoData *driverdata = _this->driverdata;

    if (driverdata->text_input_manager) {
        struct SDL_WaylandInput *input = driverdata->input;
        if (input != NULL && input->text_input) {
            zwp_text_input_v3_disable(input->text_input->text_input);
            zwp_text_input_v3_commit(input->text_input->text_input);
        }
    }

#ifdef SDL_USE_IME
    else {
        SDL_IME_Reset();
    }
#endif
}

void
Wayland_SetTextInputRect(_THIS, SDL_Rect *rect)
{
    SDL_VideoData *driverdata = _this->driverdata;

    if (!rect) {
        SDL_InvalidParamError("rect");
        return;
    }

    if (driverdata->text_input_manager) {
        struct SDL_WaylandInput *input = driverdata->input;
        if (input != NULL && input->text_input) {
            SDL_memcpy(&input->text_input->cursor_rect, rect, sizeof(SDL_Rect));
            zwp_text_input_v3_set_cursor_rectangle(input->text_input->text_input,
                                                   rect->x,
                                                   rect->y,
                                                   rect->w,
                                                   rect->h);
            zwp_text_input_v3_commit(input->text_input->text_input);
        }
    }

#ifdef SDL_USE_IME
    else {
        SDL_IME_UpdateTextRect(rect);
    }
#endif
}

SDL_bool
Wayland_HasScreenKeyboardSupport(_THIS)
{
    SDL_VideoData *driverdata = _this->driverdata;
    return (driverdata->text_input_manager != NULL);
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND */

/* vi: set ts=4 sw=4 expandtab: */
