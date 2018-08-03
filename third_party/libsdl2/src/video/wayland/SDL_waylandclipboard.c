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

#if SDL_VIDEO_DRIVER_WAYLAND

#include "SDL_waylanddatamanager.h"
#include "SDL_waylandevents_c.h"

int
Wayland_SetClipboardText(_THIS, const char *text)
{
    SDL_VideoData *video_data = NULL;
    SDL_WaylandDataDevice *data_device = NULL;
    
    int status = 0;
 
    if (_this == NULL || _this->driverdata == NULL) {
        status = SDL_SetError("Video driver uninitialized");
    } else {
        video_data = _this->driverdata;
        /* TODO: Support more than one seat */ 
        data_device = Wayland_get_data_device(video_data->input);
        if (text[0] != '\0') {
            SDL_WaylandDataSource* source = Wayland_data_source_create(_this);
            Wayland_data_source_add_data(source, TEXT_MIME, text,
                                         strlen(text) + 1); 

            status = Wayland_data_device_set_selection(data_device, source);
            if (status != 0) {
                Wayland_data_source_destroy(source);
            }
        } else {
            status = Wayland_data_device_clear_selection(data_device);
        }
    }

    return status;
}

char *
Wayland_GetClipboardText(_THIS)
{
    SDL_VideoData *video_data = NULL;
    SDL_WaylandDataDevice *data_device = NULL;

    char *text = NULL;

    void *buffer = NULL;
    size_t length = 0;
 
    if (_this == NULL || _this->driverdata == NULL) {
        SDL_SetError("Video driver uninitialized");
    } else {
        video_data = _this->driverdata;
        /* TODO: Support more than one seat */ 
        data_device = Wayland_get_data_device(video_data->input);
        if (data_device->selection_offer != NULL) {
            buffer = Wayland_data_offer_receive(data_device->selection_offer,
                                                &length, TEXT_MIME, SDL_TRUE);
            if (length > 0) {
                text = (char*) buffer;
            } 
        } else if (data_device->selection_source != NULL) {
            buffer = Wayland_data_source_get_data(data_device->selection_source,
                                                  &length, TEXT_MIME, SDL_TRUE);
            if (length > 0) {
                text = (char*) buffer;
            } 
        }
    }

    if (text == NULL) {
        text = SDL_strdup("");
    }

    return text;
}

SDL_bool
Wayland_HasClipboardText(_THIS)
{
    SDL_VideoData *video_data = NULL;
    SDL_WaylandDataDevice *data_device = NULL;

    SDL_bool result = SDL_FALSE;    
    if (_this == NULL || _this->driverdata == NULL) {
        SDL_SetError("Video driver uninitialized");
    } else {
        video_data = _this->driverdata;
        data_device = Wayland_get_data_device(video_data->input);
        if (data_device != NULL && Wayland_data_offer_has_mime(
                data_device->selection_offer, TEXT_MIME)) {
            result = SDL_TRUE;
        } else if(data_device != NULL && Wayland_data_source_has_mime(
                data_device->selection_source, TEXT_MIME)) {
            result = SDL_TRUE;
        }
    }
    return result;
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND */

/* vi: set ts=4 sw=4 expandtab: */
