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

/* *INDENT-OFF* */

#ifndef SDL_WAYLAND_MODULE
#define SDL_WAYLAND_MODULE(modname)
#endif

#ifndef SDL_WAYLAND_SYM
#define SDL_WAYLAND_SYM(rc,fn,params)
#endif

#ifndef SDL_WAYLAND_INTERFACE
#define SDL_WAYLAND_INTERFACE(iface)
#endif

SDL_WAYLAND_MODULE(WAYLAND_CLIENT)
SDL_WAYLAND_SYM(void, wl_proxy_marshal, (struct wl_proxy *, uint32_t, ...))
SDL_WAYLAND_SYM(struct wl_proxy *, wl_proxy_create, (struct wl_proxy *, const struct wl_interface *))
SDL_WAYLAND_SYM(void, wl_proxy_destroy, (struct wl_proxy *))
SDL_WAYLAND_SYM(int, wl_proxy_add_listener, (struct wl_proxy *, void (**)(void), void *))
SDL_WAYLAND_SYM(void, wl_proxy_set_user_data, (struct wl_proxy *, void *))
SDL_WAYLAND_SYM(void *, wl_proxy_get_user_data, (struct wl_proxy *))
SDL_WAYLAND_SYM(uint32_t, wl_proxy_get_id, (struct wl_proxy *))
SDL_WAYLAND_SYM(const char *, wl_proxy_get_class, (struct wl_proxy *))
SDL_WAYLAND_SYM(void, wl_proxy_set_queue, (struct wl_proxy *, struct wl_event_queue *))
SDL_WAYLAND_SYM(struct wl_display *, wl_display_connect, (const char *))
SDL_WAYLAND_SYM(struct wl_display *, wl_display_connect_to_fd, (int))
SDL_WAYLAND_SYM(void, wl_display_disconnect, (struct wl_display *))
SDL_WAYLAND_SYM(int, wl_display_get_fd, (struct wl_display *))
SDL_WAYLAND_SYM(int, wl_display_dispatch, (struct wl_display *))
SDL_WAYLAND_SYM(int, wl_display_dispatch_queue, (struct wl_display *, struct wl_event_queue *))
SDL_WAYLAND_SYM(int, wl_display_dispatch_queue_pending, (struct wl_display *, struct wl_event_queue *))
SDL_WAYLAND_SYM(int, wl_display_dispatch_pending, (struct wl_display *))
SDL_WAYLAND_SYM(int, wl_display_get_error, (struct wl_display *))
SDL_WAYLAND_SYM(int, wl_display_flush, (struct wl_display *))
SDL_WAYLAND_SYM(int, wl_display_roundtrip, (struct wl_display *))
SDL_WAYLAND_SYM(struct wl_event_queue *, wl_display_create_queue, (struct wl_display *))
SDL_WAYLAND_SYM(void, wl_log_set_handler_client, (wl_log_func_t))
SDL_WAYLAND_SYM(void, wl_list_init, (struct wl_list *))
SDL_WAYLAND_SYM(void, wl_list_insert, (struct wl_list *, struct wl_list *) )
SDL_WAYLAND_SYM(void, wl_list_remove, (struct wl_list *))
SDL_WAYLAND_SYM(int, wl_list_length, (const struct wl_list *))
SDL_WAYLAND_SYM(int, wl_list_empty, (const struct wl_list *))
SDL_WAYLAND_SYM(void, wl_list_insert_list, (struct wl_list *, struct wl_list *))

/* These functions are available in Wayland >= 1.4 */
SDL_WAYLAND_MODULE(WAYLAND_CLIENT_1_4)
SDL_WAYLAND_SYM(struct wl_proxy *, wl_proxy_marshal_constructor, (struct wl_proxy *, uint32_t opcode, const struct wl_interface *interface, ...))

SDL_WAYLAND_MODULE(WAYLAND_CLIENT_1_10)
SDL_WAYLAND_SYM(struct wl_proxy *, wl_proxy_marshal_constructor_versioned, (struct wl_proxy *proxy, uint32_t opcode, const struct wl_interface *interface, uint32_t version, ...))

SDL_WAYLAND_INTERFACE(wl_seat_interface)
SDL_WAYLAND_INTERFACE(wl_surface_interface)
SDL_WAYLAND_INTERFACE(wl_shm_pool_interface)
SDL_WAYLAND_INTERFACE(wl_buffer_interface)
SDL_WAYLAND_INTERFACE(wl_registry_interface)
SDL_WAYLAND_INTERFACE(wl_shell_surface_interface)
SDL_WAYLAND_INTERFACE(wl_region_interface)
SDL_WAYLAND_INTERFACE(wl_pointer_interface)
SDL_WAYLAND_INTERFACE(wl_keyboard_interface)
SDL_WAYLAND_INTERFACE(wl_compositor_interface)
SDL_WAYLAND_INTERFACE(wl_output_interface)
SDL_WAYLAND_INTERFACE(wl_shell_interface)
SDL_WAYLAND_INTERFACE(wl_shm_interface)
SDL_WAYLAND_INTERFACE(wl_data_device_interface)
SDL_WAYLAND_INTERFACE(wl_data_source_interface)
SDL_WAYLAND_INTERFACE(wl_data_offer_interface)
SDL_WAYLAND_INTERFACE(wl_data_device_manager_interface)

SDL_WAYLAND_MODULE(WAYLAND_EGL)
SDL_WAYLAND_SYM(struct wl_egl_window *, wl_egl_window_create, (struct wl_surface *, int, int))
SDL_WAYLAND_SYM(void, wl_egl_window_destroy, (struct wl_egl_window *))
SDL_WAYLAND_SYM(void, wl_egl_window_resize, (struct wl_egl_window *, int, int, int, int))
SDL_WAYLAND_SYM(void, wl_egl_window_get_attached_size, (struct wl_egl_window *, int *, int *))

SDL_WAYLAND_MODULE(WAYLAND_CURSOR)
SDL_WAYLAND_SYM(struct wl_cursor_theme *, wl_cursor_theme_load, (const char *, int , struct wl_shm *))
SDL_WAYLAND_SYM(void, wl_cursor_theme_destroy, (struct wl_cursor_theme *))
SDL_WAYLAND_SYM(struct wl_cursor *, wl_cursor_theme_get_cursor, (struct wl_cursor_theme *, const char *))
SDL_WAYLAND_SYM(struct wl_buffer *, wl_cursor_image_get_buffer, (struct wl_cursor_image *))
SDL_WAYLAND_SYM(int, wl_cursor_frame, (struct wl_cursor *, uint32_t))

SDL_WAYLAND_MODULE(WAYLAND_XKB)
SDL_WAYLAND_SYM(int, xkb_state_key_get_syms, (struct xkb_state *, xkb_keycode_t, const xkb_keysym_t **))
SDL_WAYLAND_SYM(int, xkb_keysym_to_utf8, (xkb_keysym_t, char *, size_t) )
SDL_WAYLAND_SYM(struct xkb_keymap *, xkb_keymap_new_from_string, (struct xkb_context *, const char *, enum xkb_keymap_format, enum xkb_keymap_compile_flags))
SDL_WAYLAND_SYM(struct xkb_state *, xkb_state_new, (struct xkb_keymap *) )
SDL_WAYLAND_SYM(void, xkb_keymap_unref, (struct xkb_keymap *) )
SDL_WAYLAND_SYM(void, xkb_state_unref, (struct xkb_state *) )
SDL_WAYLAND_SYM(void, xkb_context_unref, (struct xkb_context *) )
SDL_WAYLAND_SYM(struct xkb_context *, xkb_context_new, (enum xkb_context_flags flags) )
SDL_WAYLAND_SYM(enum xkb_state_component, xkb_state_update_mask, (struct xkb_state *state,\
                      xkb_mod_mask_t depressed_mods,\
                      xkb_mod_mask_t latched_mods,\
                      xkb_mod_mask_t locked_mods,\
                      xkb_layout_index_t depressed_layout,\
                      xkb_layout_index_t latched_layout,\
                      xkb_layout_index_t locked_layout) )

#undef SDL_WAYLAND_MODULE
#undef SDL_WAYLAND_SYM
#undef SDL_WAYLAND_INTERFACE

/* *INDENT-ON* */

/* vi: set ts=4 sw=4 expandtab: */
