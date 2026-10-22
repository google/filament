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

#ifndef SDL_MIR_MODULE
#define SDL_MIR_MODULE(modname)
#endif

#ifndef SDL_MIR_SYM
#define SDL_MIR_SYM(rc,fn,params)
#endif

#ifndef SDL_MIR_SYM_CONST
#define SDL_MIR_SYM_CONST(type, name)
#endif

SDL_MIR_MODULE(MIR_CLIENT)
SDL_MIR_SYM(MirWindow *,mir_create_window_sync,(MirWindowSpec* spec))
SDL_MIR_SYM(MirEGLNativeWindowType,mir_buffer_stream_get_egl_native_window,(MirBufferStream *surface))
SDL_MIR_SYM(bool,mir_buffer_stream_get_graphics_region,(MirBufferStream *stream, MirGraphicsRegion *graphics_region))
SDL_MIR_SYM(void,mir_buffer_stream_swap_buffers_sync,(MirBufferStream *stream))
SDL_MIR_SYM(void,mir_window_set_event_handler,(MirWindow* window, MirWindowEventCallback callback, void* context))
SDL_MIR_SYM(MirWindowSpec*,mir_create_normal_window_spec,(MirConnection *connection, int width, int height))
SDL_MIR_SYM(MirWindowSpec*,mir_create_window_spec,(MirConnection *connection))
SDL_MIR_SYM(void,mir_window_spec_set_buffer_usage,(MirWindowSpec *spec, MirBufferUsage usage))
SDL_MIR_SYM(void,mir_window_spec_set_name,(MirWindowSpec *spec, char const *name))
SDL_MIR_SYM(void,mir_window_spec_release,(MirWindowSpec *spec))
SDL_MIR_SYM(void,mir_window_spec_set_width,(MirWindowSpec *spec, unsigned width))
SDL_MIR_SYM(void,mir_window_spec_set_height,(MirWindowSpec *spec, unsigned height))
SDL_MIR_SYM(void,mir_window_spec_set_min_width,(MirWindowSpec *spec, unsigned min_width))
SDL_MIR_SYM(void,mir_window_spec_set_min_height,(MirWindowSpec *spec, unsigned min_height))
SDL_MIR_SYM(void,mir_window_spec_set_max_width,(MirWindowSpec *spec, unsigned max_width))
SDL_MIR_SYM(void,mir_window_spec_set_max_height,(MirWindowSpec *spec, unsigned max_height))
SDL_MIR_SYM(void,mir_window_spec_set_type,(MirWindowSpec *spec, MirWindowType type))
SDL_MIR_SYM(void,mir_window_spec_set_state,(MirWindowSpec *spec, MirWindowState state))
SDL_MIR_SYM(void,mir_window_spec_set_pointer_confinement,(MirWindowSpec *spec, MirPointerConfinementState state))
SDL_MIR_SYM(void,mir_window_spec_set_pixel_format,(MirWindowSpec *spec, MirPixelFormat pixel_format))
SDL_MIR_SYM(void,mir_window_spec_set_cursor_name,(MirWindowSpec *spec, char const* cursor_name))
SDL_MIR_SYM(void,mir_window_apply_spec,(MirWindow* window, MirWindowSpec* spec))
SDL_MIR_SYM(void,mir_window_get_parameters,(MirWindow *window, MirWindowParameters *params))
SDL_MIR_SYM(MirBufferStream*,mir_window_get_buffer_stream,(MirWindow* window))
SDL_MIR_SYM(MirCursorConfiguration*,mir_cursor_configuration_from_buffer_stream,(MirBufferStream const* stream, int hot_x, int hot_y))
SDL_MIR_SYM(MirBufferStream*,mir_connection_create_buffer_stream_sync,(MirConnection *connection, int w, int h, MirPixelFormat format, MirBufferUsage usage))
SDL_MIR_SYM(MirKeyboardAction,mir_keyboard_event_action,(MirKeyboardEvent const *event))
SDL_MIR_SYM(xkb_keysym_t,mir_keyboard_event_key_code,(MirKeyboardEvent const *event))
SDL_MIR_SYM(int,mir_keyboard_event_scan_code,(MirKeyboardEvent const *event))
SDL_MIR_SYM(bool,mir_pointer_event_button_state,(MirPointerEvent const *event, MirPointerButton button))
SDL_MIR_SYM(MirPointerButtons,mir_pointer_event_buttons,(MirPointerEvent const *event))
SDL_MIR_SYM(MirInputDeviceId,mir_input_event_get_device_id,(MirInputEvent const* ev))
SDL_MIR_SYM(MirTouchId,mir_touch_event_id,(MirTouchEvent const *event, size_t touch_index))
SDL_MIR_SYM(float,mir_touch_event_axis_value,(MirTouchEvent const *event, size_t touch_index, MirTouchAxis axis))
SDL_MIR_SYM(MirTouchAction,mir_touch_event_action,(MirTouchEvent const *event, size_t touch_index))
SDL_MIR_SYM(MirPointerAction,mir_pointer_event_action,(MirPointerEvent const *event))
SDL_MIR_SYM(float,mir_pointer_event_axis_value,(MirPointerEvent const *event, MirPointerAxis))
SDL_MIR_SYM(MirEventType,mir_event_get_type,(MirEvent const *event))
SDL_MIR_SYM(MirInputEventType,mir_input_event_get_type,(MirInputEvent const *event))
SDL_MIR_SYM(MirInputEvent const*,mir_event_get_input_event,(MirEvent const *event))
SDL_MIR_SYM(MirResizeEvent const*,mir_event_get_resize_event,(MirEvent const *event))
SDL_MIR_SYM(MirKeyboardEvent const*,mir_input_event_get_keyboard_event,(MirInputEvent const *event))
SDL_MIR_SYM(MirPointerEvent const*,mir_input_event_get_pointer_event,(MirInputEvent const *event))
SDL_MIR_SYM(MirTouchEvent const*,mir_input_event_get_touch_event,(MirInputEvent const *event))
SDL_MIR_SYM(MirWindowEvent const*,mir_event_get_window_event,(MirEvent const *event))
SDL_MIR_SYM(unsigned int,mir_touch_event_point_count,(MirTouchEvent const *event))
SDL_MIR_SYM(void,mir_connection_get_available_surface_formats,(MirConnection* connection, MirPixelFormat* formats, unsigned const int format_size, unsigned int *num_valid_formats))
SDL_MIR_SYM(MirEGLNativeDisplayType,mir_connection_get_egl_native_display,(MirConnection *connection))
SDL_MIR_SYM(bool,mir_connection_is_valid,(MirConnection *connection))
SDL_MIR_SYM(void,mir_connection_release,(MirConnection *connection))
SDL_MIR_SYM(MirPixelFormat,mir_connection_get_egl_pixel_format,(MirConnection* connection, void* egldisplay, void* eglconfig))
SDL_MIR_SYM(MirConnection *,mir_connect_sync,(char const *server, char const *app_name))
SDL_MIR_SYM(char const *,mir_window_get_error_message,(MirWindow *window))
SDL_MIR_SYM(bool,mir_window_is_valid,(MirWindow *window))
SDL_MIR_SYM(void,mir_window_release_sync,(MirWindow* window))
SDL_MIR_SYM(void,mir_buffer_stream_release_sync,(MirBufferStream *stream))
SDL_MIR_SYM(void,mir_window_configure_cursor,(MirWindow* window, MirCursorConfiguration const* conf))
SDL_MIR_SYM(void,mir_cursor_configuration_destroy,(MirCursorConfiguration* conf))
SDL_MIR_SYM(int,mir_resize_event_get_width,(MirResizeEvent const* resize_event))
SDL_MIR_SYM(int,mir_resize_event_get_height,(MirResizeEvent const* resize_event))
SDL_MIR_SYM(char const*,mir_connection_get_error_message,(MirConnection* connection))
SDL_MIR_SYM(MirWindowAttrib,mir_window_event_get_attribute,(MirWindowEvent const* event))
SDL_MIR_SYM(int,mir_window_event_get_attribute_value,(MirWindowEvent const* window_event))
SDL_MIR_SYM(MirDisplayConfig*,mir_connection_create_display_configuration,(MirConnection* connection))
SDL_MIR_SYM(void,mir_display_config_release,(MirDisplayConfig* config))
SDL_MIR_SYM(int,mir_display_config_get_num_outputs,(MirDisplayConfig const* config))
SDL_MIR_SYM(MirOutput*,mir_display_config_get_mutable_output,(MirDisplayConfig* config, size_t index))
SDL_MIR_SYM(int,mir_output_get_num_modes,(MirOutput const* output))
SDL_MIR_SYM(MirOutputMode const*,mir_output_get_current_mode,(MirOutput const* output))
SDL_MIR_SYM(MirPixelFormat,mir_output_get_current_pixel_format,(MirOutput const* output))
SDL_MIR_SYM(int,mir_output_get_position_x,(MirOutput const* output))
SDL_MIR_SYM(int,mir_output_get_position_y,(MirOutput const* output))
SDL_MIR_SYM(bool,mir_output_is_enabled,(MirOutput const* output))
SDL_MIR_SYM(MirOutputConnectionState,mir_output_get_connection_state,(MirOutput const* output))
SDL_MIR_SYM(size_t,mir_output_get_preferred_mode_index,(MirOutput const* output))
SDL_MIR_SYM(MirOutputType,mir_output_get_type,(MirOutput const* output))
SDL_MIR_SYM(char const*,mir_output_type_name,(MirOutputType type))
SDL_MIR_SYM(void,mir_output_set_current_mode,(MirOutput* output, MirOutputMode const* mode))
SDL_MIR_SYM(MirOutputMode const*,mir_output_get_mode,(MirOutput const* output, size_t index))
SDL_MIR_SYM(int,mir_output_mode_get_width,(MirOutputMode const* mode))
SDL_MIR_SYM(int,mir_output_mode_get_height,(MirOutputMode const* mode))
SDL_MIR_SYM(double,mir_output_mode_get_refresh_rate,(MirOutputMode const* mode))
SDL_MIR_SYM(bool,mir_output_is_gamma_supported,(MirOutput const* output))
SDL_MIR_SYM(uint32_t,mir_output_get_gamma_size,(MirOutput const* output))
SDL_MIR_SYM(void,mir_output_get_gamma,(MirOutput const* output, uint16_t* red, uint16_t* green, uint16_t* blue, uint32_t size))
SDL_MIR_SYM(void,mir_output_set_gamma,(MirOutput* output, uint16_t const* red, uint16_t const* green, uint16_t const* blue, uint32_t size))

SDL_MIR_SYM_CONST(char const*,mir_omnidirectional_resize_cursor_name)
SDL_MIR_SYM_CONST(char const*,mir_busy_cursor_name)
SDL_MIR_SYM_CONST(char const*,mir_arrow_cursor_name)
SDL_MIR_SYM_CONST(char const*,mir_caret_cursor_name)
SDL_MIR_SYM_CONST(char const*,mir_vertical_resize_cursor_name)
SDL_MIR_SYM_CONST(char const*,mir_horizontal_resize_cursor_name)
SDL_MIR_SYM_CONST(char const*,mir_open_hand_cursor_name)
SDL_MIR_SYM_CONST(char const*,mir_closed_hand_cursor_name)
SDL_MIR_SYM_CONST(char const*,mir_disabled_cursor_name)

SDL_MIR_MODULE(XKBCOMMON)
SDL_MIR_SYM(int,xkb_keysym_to_utf8,(xkb_keysym_t keysym, char *buffer, size_t size))

#undef SDL_MIR_MODULE
#undef SDL_MIR_SYM
#undef SDL_MIR_SYM_CONST

/* *INDENT-ON* */

/* vi: set ts=4 sw=4 expandtab: */
