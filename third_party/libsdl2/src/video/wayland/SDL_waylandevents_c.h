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

#ifndef SDL_waylandevents_h_
#define SDL_waylandevents_h_

#include "SDL_waylandvideo.h"
#include "SDL_waylandwindow.h"
#include "SDL_waylanddatamanager.h"

struct SDL_WaylandInput;

extern void Wayland_PumpEvents(_THIS);

extern void Wayland_display_add_input(SDL_VideoData *d, uint32_t id);
extern void Wayland_display_destroy_input(SDL_VideoData *d);

extern SDL_WaylandDataDevice* Wayland_get_data_device(struct SDL_WaylandInput *input);

extern void Wayland_display_add_pointer_constraints(SDL_VideoData *d, uint32_t id);
extern void Wayland_display_destroy_pointer_constraints(SDL_VideoData *d);

extern int Wayland_input_lock_pointer(struct SDL_WaylandInput *input);
extern int Wayland_input_unlock_pointer(struct SDL_WaylandInput *input);

extern void Wayland_display_add_relative_pointer_manager(SDL_VideoData *d, uint32_t id);
extern void Wayland_display_destroy_relative_pointer_manager(SDL_VideoData *d);

#endif /* SDL_waylandevents_h_ */

/* vi: set ts=4 sw=4 expandtab: */
