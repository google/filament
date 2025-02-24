/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#pragma once

#include <dlfcn.h>
#include <stdlib.h>

#include <xcb/xcb.h>

typedef xcb_void_cookie_t (*PFN_xcb_destroy_window)(xcb_connection_t *c, xcb_window_t window);
typedef void (*PFN_xcb_disconnect)(xcb_connection_t *c);
typedef int (*PFN_xcb_flush)(xcb_connection_t *c);
typedef xcb_generic_event_t *(*PFN_xcb_wait_for_event)(xcb_connection_t *c);
typedef xcb_generic_event_t *(*PFN_xcb_poll_for_event)(xcb_connection_t *c);
typedef uint32_t (*PFN_xcb_generate_id)(xcb_connection_t *c);
typedef xcb_void_cookie_t (*PFN_xcb_create_window)(xcb_connection_t *c, uint8_t depth, xcb_window_t wid, xcb_window_t parent,
                                                   int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t border_width,
                                                   uint16_t _class, xcb_visualid_t visual, uint32_t value_mask,
                                                   const void *value_list);
typedef xcb_intern_atom_cookie_t (*PFN_xcb_intern_atom)(xcb_connection_t *c, uint8_t only_if_exists, uint16_t name_len,
                                                        const char *name);
typedef xcb_intern_atom_reply_t *(*PFN_xcb_intern_atom_reply)(xcb_connection_t *c, xcb_intern_atom_cookie_t cookie /**< */,
                                                              xcb_generic_error_t **e);
typedef xcb_void_cookie_t (*PFN_xcb_change_property)(xcb_connection_t *c, uint8_t mode, xcb_window_t window, xcb_atom_t property,
                                                     xcb_atom_t type, uint8_t format, uint32_t data_len, const void *data);
typedef xcb_void_cookie_t (*PFN_xcb_map_window)(xcb_connection_t *c, xcb_window_t window);
typedef xcb_void_cookie_t (*PFN_xcb_configure_window)(xcb_connection_t *c, xcb_window_t window, uint16_t value_mask,
                                                      const void *value_list);
typedef xcb_connection_t *(*PFN_xcb_connect)(const char *displayname, int *screenp);
typedef int (*PFN_xcb_connection_has_error)(xcb_connection_t *c);
typedef const struct xcb_setup_t *(*PFN_xcb_get_setup)(xcb_connection_t *c);
typedef xcb_screen_iterator_t (*PFN_xcb_setup_roots_iterator)(const xcb_setup_t *R);
typedef void (*PFN_xcb_screen_next)(xcb_screen_iterator_t *i);

static PFN_xcb_destroy_window cube_xcb_destroy_window = NULL;
static PFN_xcb_disconnect cube_xcb_disconnect = NULL;
static PFN_xcb_flush cube_xcb_flush = NULL;
static PFN_xcb_wait_for_event cube_xcb_wait_for_event = NULL;
static PFN_xcb_poll_for_event cube_xcb_poll_for_event = NULL;
static PFN_xcb_generate_id cube_xcb_generate_id = NULL;
static PFN_xcb_create_window cube_xcb_create_window = NULL;
static PFN_xcb_intern_atom cube_xcb_intern_atom = NULL;
static PFN_xcb_intern_atom_reply cube_xcb_intern_atom_reply = NULL;
static PFN_xcb_change_property cube_xcb_change_property = NULL;
static PFN_xcb_map_window cube_xcb_map_window = NULL;
static PFN_xcb_configure_window cube_xcb_configure_window = NULL;
static PFN_xcb_connect cube_xcb_connect = NULL;
static PFN_xcb_connection_has_error cube_xcb_connection_has_error = NULL;
static PFN_xcb_get_setup cube_xcb_get_setup = NULL;
static PFN_xcb_setup_roots_iterator cube_xcb_setup_roots_iterator = NULL;
static PFN_xcb_screen_next cube_xcb_screen_next = NULL;

#define xcb_destroy_window cube_xcb_destroy_window
#define xcb_disconnect cube_xcb_disconnect
#define xcb_flush cube_xcb_flush
#define xcb_wait_for_event cube_xcb_wait_for_event
#define xcb_poll_for_event cube_xcb_poll_for_event
#define xcb_generate_id cube_xcb_generate_id
#define xcb_create_window cube_xcb_create_window
#define xcb_intern_atom cube_xcb_intern_atom
#define xcb_intern_atom_reply cube_xcb_intern_atom_reply
#define xcb_change_property cube_xcb_change_property
#define xcb_map_window cube_xcb_map_window
#define xcb_configure_window cube_xcb_configure_window
#define xcb_connect cube_xcb_connect
#define xcb_connection_has_error cube_xcb_connection_has_error
#define xcb_get_setup cube_xcb_get_setup
#define xcb_setup_roots_iterator cube_xcb_setup_roots_iterator
#define xcb_screen_next cube_xcb_screen_next

void *initialize_xcb() {
    void *xcb_library = NULL;
#if defined(XCB_LIBRARY)
    xcb_library = dlopen(XCB_LIBRARY, RTLD_NOW | RTLD_LOCAL);
#endif
    if (NULL == xcb_library) {
        xcb_library = dlopen("libxcb.so.1", RTLD_NOW | RTLD_LOCAL);
    }
    if (NULL == xcb_library) {
        xcb_library = dlopen("libxcb.so", RTLD_NOW | RTLD_LOCAL);
    }
    if (NULL == xcb_library) {
        return NULL;
    }

#ifdef __cplusplus
#define TYPE_CONVERSION(type) reinterpret_cast<type>
#else
#define TYPE_CONVERSION(type)
#endif

    cube_xcb_destroy_window = TYPE_CONVERSION(PFN_xcb_destroy_window)(dlsym(xcb_library, "xcb_destroy_window"));
    cube_xcb_disconnect = TYPE_CONVERSION(PFN_xcb_disconnect)(dlsym(xcb_library, "xcb_disconnect"));
    cube_xcb_flush = TYPE_CONVERSION(PFN_xcb_flush)(dlsym(xcb_library, "xcb_flush"));
    cube_xcb_wait_for_event = TYPE_CONVERSION(PFN_xcb_wait_for_event)(dlsym(xcb_library, "xcb_wait_for_event"));
    cube_xcb_poll_for_event = TYPE_CONVERSION(PFN_xcb_poll_for_event)(dlsym(xcb_library, "xcb_poll_for_event"));
    cube_xcb_generate_id = TYPE_CONVERSION(PFN_xcb_generate_id)(dlsym(xcb_library, "xcb_generate_id"));
    cube_xcb_create_window = TYPE_CONVERSION(PFN_xcb_create_window)(dlsym(xcb_library, "xcb_create_window"));
    cube_xcb_intern_atom = TYPE_CONVERSION(PFN_xcb_intern_atom)(dlsym(xcb_library, "xcb_intern_atom"));
    cube_xcb_intern_atom_reply = TYPE_CONVERSION(PFN_xcb_intern_atom_reply)(dlsym(xcb_library, "xcb_intern_atom_reply"));
    cube_xcb_change_property = TYPE_CONVERSION(PFN_xcb_change_property)(dlsym(xcb_library, "xcb_change_property"));
    cube_xcb_map_window = TYPE_CONVERSION(PFN_xcb_map_window)(dlsym(xcb_library, "xcb_map_window"));
    cube_xcb_configure_window = TYPE_CONVERSION(PFN_xcb_configure_window)(dlsym(xcb_library, "xcb_configure_window"));
    cube_xcb_connect = TYPE_CONVERSION(PFN_xcb_connect)(dlsym(xcb_library, "xcb_connect"));
    cube_xcb_connection_has_error = TYPE_CONVERSION(PFN_xcb_connection_has_error)(dlsym(xcb_library, "xcb_connection_has_error"));
    cube_xcb_get_setup = TYPE_CONVERSION(PFN_xcb_get_setup)(dlsym(xcb_library, "xcb_get_setup"));
    cube_xcb_setup_roots_iterator = TYPE_CONVERSION(PFN_xcb_setup_roots_iterator)(dlsym(xcb_library, "xcb_setup_roots_iterator"));
    cube_xcb_screen_next = TYPE_CONVERSION(PFN_xcb_screen_next)(dlsym(xcb_library, "xcb_screen_next"));

    return xcb_library;
}
