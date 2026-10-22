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

#ifndef SDL_waylanddyn_h_
#define SDL_waylanddyn_h_

#include "../../SDL_internal.h"

/* We can't include wayland-client.h here 
 * but we need some structs from it
 */
struct wl_interface;
struct wl_proxy;
struct wl_event_queue;
struct wl_display;
struct wl_surface;
struct wl_shm;

#include <stdint.h>
#include "wayland-cursor.h"
#include "wayland-util.h"
#include "xkbcommon/xkbcommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

int SDL_WAYLAND_LoadSymbols(void);
void SDL_WAYLAND_UnloadSymbols(void);

#define SDL_WAYLAND_MODULE(modname) extern int SDL_WAYLAND_HAVE_##modname;
#define SDL_WAYLAND_SYM(rc,fn,params) \
    typedef rc (*SDL_DYNWAYLANDFN_##fn) params; \
    extern SDL_DYNWAYLANDFN_##fn WAYLAND_##fn;
#define SDL_WAYLAND_INTERFACE(iface) extern const struct wl_interface *WAYLAND_##iface;
#include "SDL_waylandsym.h"


#ifdef __cplusplus
}
#endif

#ifdef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC

#ifdef _WAYLAND_CLIENT_H
#error Do not include wayland-client ahead of SDL_waylanddyn.h in dynamic loading mode
#endif

/* wayland-client-protocol.h included from wayland-client.h
 * has inline functions that require these to be defined in dynamic loading mode
 */

#define wl_proxy_create (*WAYLAND_wl_proxy_create)
#define wl_proxy_destroy (*WAYLAND_wl_proxy_destroy)
#define wl_proxy_marshal (*WAYLAND_wl_proxy_marshal)
#define wl_proxy_set_user_data (*WAYLAND_wl_proxy_set_user_data)
#define wl_proxy_get_user_data (*WAYLAND_wl_proxy_get_user_data)
#define wl_proxy_add_listener (*WAYLAND_wl_proxy_add_listener)
#define wl_proxy_marshal_constructor (*WAYLAND_wl_proxy_marshal_constructor)
#define wl_proxy_marshal_constructor_versioned (*WAYLAND_wl_proxy_marshal_constructor_versioned)

#define wl_seat_interface (*WAYLAND_wl_seat_interface)
#define wl_surface_interface (*WAYLAND_wl_surface_interface)
#define wl_shm_pool_interface (*WAYLAND_wl_shm_pool_interface)
#define wl_buffer_interface (*WAYLAND_wl_buffer_interface)
#define wl_registry_interface (*WAYLAND_wl_registry_interface)
#define wl_shell_surface_interface (*WAYLAND_wl_shell_surface_interface)
#define wl_region_interface (*WAYLAND_wl_region_interface)
#define wl_pointer_interface (*WAYLAND_wl_pointer_interface)
#define wl_keyboard_interface (*WAYLAND_wl_keyboard_interface)
#define wl_compositor_interface (*WAYLAND_wl_compositor_interface)
#define wl_output_interface (*WAYLAND_wl_output_interface)
#define wl_shell_interface (*WAYLAND_wl_shell_interface)
#define wl_shm_interface (*WAYLAND_wl_shm_interface)
#define wl_data_device_interface (*WAYLAND_wl_data_device_interface)
#define wl_data_offer_interface (*WAYLAND_wl_data_offer_interface)
#define wl_data_source_interface (*WAYLAND_wl_data_source_interface)
#define wl_data_device_manager_interface (*WAYLAND_wl_data_device_manager_interface)

#endif /* SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC */

#include "wayland-client-core.h"
#include "wayland-client-protocol.h"
#include "wayland-egl.h"

#endif /* SDL_waylanddyn_h_ */

/* vi: set ts=4 sw=4 expandtab: */
