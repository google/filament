# Copyright 2006-2021 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

# registry.tcl
#
# This is a simple human-readable database defining the EGL extension
# registry. For each extension, it includes an extension number, flags
# if the extension is public, and includes a path to the extension
# specification.
#
# The companion script 'regproc.tcl' uses this to build up the
# extensions portion of the public registry, by copying out only
# the public specifications.

extension EGL_KHR_config_attribs {
    number      1
    flags       public
    filename    extensions/KHR/EGL_KHR_config_attribs.txt
}
extension EGL_KHR_lock_surface {
    number      2
    flags       public
    filename    extensions/KHR/EGL_KHR_lock_surface.txt
}
extension EGL_KHR_image {
    number      3
    flags       public
    filename    extensions/KHR/EGL_KHR_image.txt
}
extension EGL_KHR_vg_parent_image {
    number      4
    flags       public
    filename    extensions/KHR/EGL_KHR_vg_parent_image.txt
}
extension EGL_KHR_gl_texture_2D_image {
    number      5
    flags       public
    filename    extensions/KHR/EGL_KHR_gl_image.txt
    alias       EGL_KHR_gl_texture_cubemap_image
    alias       EGL_KHR_gl_texture_3D_image
    alias       EGL_KHR_gl_renderbuffer_image
}
extension EGL_KHR_reusable_sync {
    number      6
    flags       public
    filename    extensions/KHR/EGL_KHR_reusable_sync.txt
}
extension EGL_SYMBIAN_image_preserved {
    number      7
    flags       private incomplete
    filename    extensions/SYMBIAN/EGL_SYMBIAN_image_preserved.txt
}
extension EGL_KHR_image_base {
    number      8
    flags       public
    filename    extensions/KHR/EGL_KHR_image_base.txt
}
extension EGL_KHR_image_pixmap {
    number      9
    flags       public
    filename    extensions/KHR/EGL_KHR_image_pixmap.txt
}
extension EGL_IMG_context_priority {
    number      10
    flags       public
    filename    extensions/IMG/EGL_IMG_context_priority.txt
}
extension EGL_NOK_hibernate_context {
    number      11
    flags       private
    filename    extensions/NOK/EGL_NOK_hibernate_context.txt
}
extension EGL_NOK_swap_region {
    number      12
    flags       private
    filename    extensions/NOK/EGL_NOK_swap_region.txt
}
extension EGL_NOK_resource_profiling {
    number      13
    flags       private
    filename    extensions/NOK/EGL_NOK_resource_profiling.txt
}
extension EGL_NOK_texture_from_pixmap {
    number      14
    flags       public
    filename    extensions/NOK/EGL_NOK_texture_from_pixmap.txt
}
extension EGL_NOK_resource_profiling2 {
    number      15
    flags       private
    filename    extensions/NOK/EGL_NOK_resource_profiling2.txt
}
extension EGL_KHR_lock_surface2 {
    number      16
    flags       public
    filename    extensions/KHR/EGL_KHR_lock_surface2.txt
}
extension EGL_NV_coverage_sample {
    number      17
    flags       public
    filename    extensions/NV/EGL_NV_coverage_sample.txt
}
extension EGL_NV_depth_nonlinear {
    number      18
    flags       public
    filename    extensions/NV/EGL_NV_depth_nonlinear.txt
}
extension EGL_NV_sync {
    number      19
    flags       public
    filename    extensions/NV/EGL_NV_sync.txt
}
extension EGL_KHR_fence_sync {
    number      20
    flags       public
    filename    extensions/KHR/EGL_KHR_fence_sync.txt
}
extension EGL_NOK_surface_scaling {
    number      21
    flags       private
    filename    extensions/NOK/EGL_NOK_surface_scaling.txt
}
extension EGL_NOK_image_shared {
    number      22
    flags       private
    filename    extensions/NOK/EGL_NOK_image_shared.txt
}
extension EGL_NOK_swap_region2 {
    number      23
    flags       public
    filename    extensions/NOK/EGL_NOK_swap_region2.txt
}
extension EGL_HI_clientpixmap {
    number      24
    flags       public
    filename    extensions/HI/EGL_HI_clientpixmap.txt
}
extension EGL_HI_colorformats {
    number      25
    flags       public
    filename    extensions/HI/EGL_HI_colorformats.txt
}
extension EGL_MESA_drm_image {
    number      26
    flags       public
    filename    extensions/MESA/EGL_MESA_drm_image.txt
}
extension EGL_NV_post_sub_buffer {
    number      27
    flags       public
    filename    extensions/NV/EGL_NV_post_sub_buffer.txt
}
extension EGL_ANGLE_query_surface_pointer {
    number      28
    flags       public
    filename    extensions/ANGLE/EGL_ANGLE_query_surface_pointer.txt
}
extension EGL_ANGLE_surface_d3d_texture_2d_share_handle {
    number      29
    flags       public
    filename    extensions/ANGLE/EGL_ANGLE_surface_d3d_texture_2d_share_handle.txt
}
extension EGL_NV_coverage_sample_resolve {
    number      30
    flags       public
    filename    extensions/NV/EGL_NV_coverage_sample_resolve.txt
}
extension EGL_NV_system_time {
    number      31
    flags       public
    filename    extensions/NV/EGL_NV_system_time.txt
}
extension EGL_KHR_stream {
    number      32
    flags       public
    filename    extensions/KHR/EGL_KHR_stream.txt
    alias       EGL_KHR_stream_attrib
}
extension EGL_KHR_stream_consumer_gltexture {
    number      33
    flags       public
    filename    extensions/KHR/EGL_KHR_stream_consumer_gltexture.txt
}
extension EGL_KHR_stream_producer_eglsurface {
    number      34
    flags       public
    filename    extensions/KHR/EGL_KHR_stream_producer_eglsurface.txt
}
extension EGL_KHR_stream_producer_aldatalocator {
    number      35
    flags       public
    filename    extensions/KHR/EGL_KHR_stream_producer_aldatalocator.txt
}
extension EGL_KHR_stream_fifo {
    number      36
    flags       public
    filename    extensions/KHR/EGL_KHR_stream_fifo.txt
}
extension EGL_EXT_create_context_robustness {
    number      37
    flags       public
    filename    extensions/EXT/EGL_EXT_create_context_robustness.txt
}
extension EGL_ANGLE_d3d_share_handle_client_buffer {
    number      38
    flags       public
    filename    extensions/ANGLE/EGL_ANGLE_d3d_share_handle_client_buffer.txt
}
extension EGL_KHR_create_context {
    number      39
    flags       public
    filename    extensions/KHR/EGL_KHR_create_context.txt
}
extension EGL_KHR_surfaceless_context {
    number      40
    flags       public
    filename    extensions/KHR/EGL_KHR_surfaceless_context.txt
}
extension EGL_KHR_stream_cross_process_fd {
    number      41
    flags       public
    filename    extensions/KHR/EGL_KHR_stream_cross_process_fd.txt
}
extension EGL_EXT_multiview_window {
    number      42
    flags       public
    filename    extensions/EXT/EGL_EXT_multiview_window.txt
}
extension EGL_KHR_wait_sync {
    number      43
    flags       public
    filename    extensions/KHR/EGL_KHR_wait_sync.txt
}
extension EGL_NV_post_convert_rounding {
    number      44
    flags       public
    filename    extensions/NV/EGL_NV_post_convert_rounding.txt
}
extension EGL_NV_native_query {
    number      45
    flags       public
    filename    extensions/NV/EGL_NV_native_query.txt
}
extension EGL_NV_3dvision_surface {
    number      46
    flags       public
    filename    extensions/NV/EGL_NV_3dvision_surface.txt
}
extension EGL_ANDROID_framebuffer_target {
    number      47
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_framebuffer_target.txt
}
extension EGL_ANDROID_blob_cache {
    number      48
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_blob_cache.txt
}
extension EGL_ANDROID_image_native_buffer {
    number      49
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_image_native_buffer.txt
}
extension EGL_ANDROID_native_fence_sync {
    number      50
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_native_fence_sync.txt
}
extension EGL_ANDROID_recordable {
    number      51
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_recordable.txt
}
extension EGL_EXT_buffer_age {
    number      52
    flags       public
    filename    extensions/EXT/EGL_EXT_buffer_age.txt
}
extension EGL_EXT_image_dma_buf_import {
    number      53
    flags       public
    filename    extensions/EXT/EGL_EXT_image_dma_buf_import.txt
}
extension EGL_ARM_pixmap_multisample_discard {
    number      54
    flags       public
    filename    extensions/ARM/EGL_ARM_pixmap_multisample_discard.txt
}
extension EGL_EXT_swap_buffers_with_damage {
    number      55
    flags       public
    filename    extensions/EXT/EGL_EXT_swap_buffers_with_damage.txt
}
extension EGL_NV_stream_sync {
    number      56
    flags       public
    filename    extensions/NV/EGL_NV_stream_sync.txt
}
extension EGL_EXT_platform_base {
    number      57
    flags       public
    filename    extensions/EXT/EGL_EXT_platform_base.txt
}
extension EGL_EXT_client_extensions {
    number      58
    flags       public
    filename    extensions/EXT/EGL_EXT_client_extensions.txt
}
extension EGL_EXT_platform_x11 {
    number      59
    flags       public
    filename    extensions/EXT/EGL_EXT_platform_x11.txt
}
extension EGL_KHR_cl_event {
    number      60
    flags       public
    filename    extensions/KHR/EGL_KHR_cl_event.txt
}
extension EGL_KHR_get_all_proc_addresses {
    number      61
    flags       public
    filename    extensions/KHR/EGL_KHR_get_all_proc_addresses.txt
    alias       EGL_KHR_client_get_all_proc_addresses
}
extension EGL_MESA_platform_gbm {
    number      62
    flags       public
    filename    extensions/MESA/EGL_MESA_platform_gbm.txt
}
extension EGL_EXT_platform_wayland {
    number      63
    flags       public
    filename    extensions/EXT/EGL_EXT_platform_wayland.txt
}
extension EGL_KHR_lock_surface3 {
    number      64
    flags       public
    filename    extensions/KHR/EGL_KHR_lock_surface3.txt
}
extension EGL_KHR_cl_event2 {
    number      65
    flags       public
    filename    extensions/KHR/EGL_KHR_cl_event2.txt
}
extension EGL_KHR_gl_colorspace {
    number      66
    flags       public
    filename    extensions/KHR/EGL_KHR_gl_colorspace.txt
}
extension EGL_EXT_protected_surface {
    number      67
    flags       public
    filename    extensions/EXT/EGL_EXT_protected_surface.txt
}
extension EGL_KHR_platform_android {
    number      68
    flags       public
    filename    extensions/KHR/EGL_KHR_platform_android.txt
}
extension EGL_KHR_platform_gbm {
    number      69
    flags       public
    filename    extensions/KHR/EGL_KHR_platform_gbm.txt
}
extension EGL_KHR_platform_wayland {
    number      70
    flags       public
    filename    extensions/KHR/EGL_KHR_platform_wayland.txt
}
extension EGL_KHR_platform_x11 {
    number      71
    flags       public
    filename    extensions/KHR/EGL_KHR_platform_x11.txt
}
extension EGL_EXT_device_base {
    number      72
    flags       public
    filename    extensions/EXT/EGL_EXT_device_base.txt
}
extension EGL_EXT_platform_device {
    number      73
    flags       public
    filename    extensions/EXT/EGL_EXT_platform_device.txt
}
extension EGL_NV_device_cuda {
    number      74
    flags       public
    filename    extensions/NV/EGL_NV_device_cuda.txt
}
extension EGL_NV_cuda_event {
    number      75
    flags       public
    filename    extensions/NV/EGL_NV_cuda_event.txt
}
extension EGL_TIZEN_image_native_buffer {
    number      76
    flags       public
    filename    extensions/TIZEN/EGL_TIZEN_image_native_buffer.txt
}
extension EGL_TIZEN_image_native_surface {
    number      77
    flags       public
    filename    extensions/TIZEN/EGL_TIZEN_image_native_surface.txt
}
extension EGL_EXT_output_base {
    number      78
    flags       public
    filename    extensions/EXT/EGL_EXT_output_base.txt
}
extension EGL_EXT_device_drm {
    number      79
    flags       public
    filename    extensions/EXT/EGL_EXT_device_drm.txt
    alias       EGL_EXT_output_drm
}
extension EGL_EXT_device_openwf {
    number      80
    flags       public
    filename    extensions/EXT/EGL_EXT_device_openwf.txt
    alias       EGL_EXT_output_openwf
}
extension EGL_EXT_stream_consumer_egloutput {
    number      81
    flags       public
    filename    extensions/EXT/EGL_EXT_stream_consumer_egloutput.txt
}
extension EGL_QCOM_gpu_perf {
    number      82
    flags       private
    filename    extensions/QCOM/EGL_QCOM_gpu_perf.txt
}
extension EGL_KHR_partial_update {
    number      83
    flags       public
    filename    extensions/KHR/EGL_KHR_partial_update.txt
}
extension EGL_KHR_swap_buffers_with_damage {
    number      84
    flags       public
    filename    extensions/KHR/EGL_KHR_swap_buffers_with_damage.txt
}
extension EGL_ANGLE_window_fixed_size {
    number      85
    flags       public
    filename    extensions/ANGLE/EGL_ANGLE_window_fixed_size.txt
}
extension EGL_EXT_yuv_surface {
    number      86
    flags       public
    filename    extensions/EXT/EGL_EXT_yuv_surface.txt
}
extension EGL_MESA_image_dma_buf_export {
    number      87
    flags       public
    filename    extensions/MESA/EGL_MESA_image_dma_buf_export.txt
}
extension EGL_EXT_device_enumeration {
    number      88
    flags       public
    filename    extensions/EXT/EGL_EXT_device_enumeration.txt
}
extension EGL_EXT_device_query {
    number      89
    flags       public
    filename    extensions/EXT/EGL_EXT_device_query.txt
}
extension EGL_ANGLE_device_d3d {
    number      90
    flags       public
    filename    extensions/ANGLE/EGL_ANGLE_device_d3d.txt
}
extension EGL_KHR_create_context_no_error {
    number      91
    flags       public
    filename    extensions/KHR/EGL_KHR_create_context_no_error.txt
}
extension EGL_KHR_debug {
    number      92
    flags       public
    filename    extensions/KHR/EGL_KHR_debug.txt
}
extension EGL_NV_stream_metadata {
    number      93
    flags       public
    filename    extensions/NV/EGL_NV_stream_metadata.txt
}
extension EGL_NV_stream_consumer_gltexture_yuv {
    number      94
    flags       public
    filename    extensions/NV/EGL_NV_stream_consumer_gltexture_yuv.txt
}
extension EGL_IMG_image_plane_attribs {
    number      95
    flags       public
    filename    extensions/IMG/EGL_IMG_image_plane_attribs.txt
}
extension EGL_KHR_mutable_render_buffer {
    number      96
    flags       public
    filename    extensions/KHR/EGL_KHR_mutable_render_buffer.txt
}
extension EGL_EXT_protected_content {
    number      97
    flags       public
    filename    extensions/EXT/EGL_EXT_protected_content.txt
}
extension EGL_ANDROID_presentation_time {
    number      98
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_presentation_time.txt
}
extension EGL_ANDROID_create_native_client_buffer {
    number      99
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_create_native_client_buffer.txt
}
extension EGL_ANDROID_front_buffer_auto_refresh {
    number      100
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_front_buffer_auto_refresh.txt
}
extension EGL_KHR_no_config_context {
    number      101
    flags       public
    filename    extensions/KHR/EGL_KHR_no_config_context.txt
}
extension EGL_KHR_context_flush_control {
    number      102
    flags       public
    filename    ../OpenGL/extensions/KHR/KHR_context_flush_control.txt
}
extension EGL_ARM_implicit_external_sync {
    number      103
    flags       public
    filename    extensions/ARM/EGL_ARM_implicit_external_sync.txt
}
extension EGL_MESA_platform_surfaceless {
    number      104
    flags       public
    filename    extensions/MESA/EGL_MESA_platform_surfaceless.txt
}
extension EGL_EXT_image_dma_buf_import_modifiers {
    number      105
    flags       public
    filename    extensions/EXT/EGL_EXT_image_dma_buf_import_modifiers.txt
}
extension EGL_EXT_pixel_format_float {
    number      106
    flags       public
    filename    extensions/EXT/EGL_EXT_pixel_format_float.txt
}
extension EGL_EXT_gl_colorspace_bt2020_linear {
    number      107
    flags       public
    filename    extensions/EXT/EGL_EXT_gl_colorspace_bt2020_linear.txt
    alias       EGL_EXT_gl_colorspace_bt2020_pq
}
extension EGL_EXT_gl_colorspace_scrgb_linear {
    number      108
    flags       public
    filename    extensions/EXT/EGL_EXT_gl_colorspace_scrgb_linear.txt
}
extension EGL_EXT_surface_SMPTE2086_metadata {
    number      109
    flags       public
    filename    extensions/EXT/EGL_EXT_surface_SMPTE2086_metadata.txt
}
extension EGL_NV_stream_fifo_next {
    number      110
    flags       public
    filename    extensions/NV/EGL_NV_stream_fifo_next.txt
}
extension EGL_NV_stream_fifo_synchronous {
    number      111
    flags       public
    filename    extensions/NV/EGL_NV_stream_fifo_synchronous.txt
}
extension EGL_NV_stream_reset {
    number      112
    flags       public
    filename    extensions/NV/EGL_NV_stream_reset.txt
}
extension EGL_NV_stream_frame_limits {
    number      113
    flags       public
    filename    extensions/NV/EGL_NV_stream_frame_limits.txt
}
extension EGL_NV_stream_remote {
    number      114
    flags       public
    filename    extensions/NV/EGL_NV_stream_remote.txt
    alias       EGL_NV_stream_cross_object
    alias       EGL_NV_stream_cross_display
    alias       EGL_NV_stream_cross_process
    alias       EGL_NV_stream_cross_partition
    alias       EGL_NV_stream_cross_system
}
extension EGL_NV_stream_socket {
    number      115
    flags       public
    filename    extensions/NV/EGL_NV_stream_socket.txt
    alias       EGL_NV_stream_socket_unix
    alias       EGL_NV_stream_socket_inet
}
extension EGL_EXT_compositor {
    number      116
    flags       public
    filename    extensions/EXT/EGL_EXT_compositor.txt
}
extension EGL_EXT_surface_CTA861_3_metadata {
    number      117
    flags       public
    filename    extensions/EXT/EGL_EXT_surface_CTA861_3_metadata.txt
}
extension EGL_EXT_gl_colorspace_display_p3 {
    number      118
    flags       public
    filename    extensions/EXT/EGL_EXT_colorspace_display_p3.txt
}
extension EGL_EXT_gl_colorspace_scrgb {
    number      119
    flags       public
    filename    extensions/EXT/EGL_EXT_gl_colorspace_scrgb.txt
}
extension EGL_EXT_image_implicit_sync_control {
    number      120
    flags       public
    filename    extensions/EXT/EGL_EXT_image_implicit_sync_control.txt
}
extension EGL_EXT_bind_to_front {
    number      121
    flags       public
    filename    extensions/EXT/EGL_EXT_bind_to_front.txt
}
extension EGL_ANDROID_get_frame_timestamps {
    number      122
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_get_frame_timestamps.txt
}
extension EGL_ANDROID_get_native_client_buffer {
    number      123
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_get_native_client_buffer.txt
}
extension EGL_NV_context_priority_realtime {
    number      124
    flags       public
    filename    extensions/NV/EGL_NV_context_priority_realtime.txt
}
extension EGL_EXT_image_gl_colorspace {
    number      125
    flags       public
    filename    extensions/EXT/EGL_EXT_image_gl_colorspace.txt
}
extension EGL_KHR_display_reference {
    number      126
    flags       public
    filename    extensions/KHR/EGL_KHR_display_reference.txt
}
extension EGL_NV_stream_flush {
    number      127
    flags       public
    filename    extensions/NV/EGL_NV_stream_flush.txt
}
extension EGL_EXT_sync_reuse {
    number      128
    flags       public
    filename    extensions/EXT/EGL_EXT_sync_reuse.txt
}
extension EGL_EXT_client_sync {
    number      129
    flags       public
    filename    extensions/EXT/EGL_EXT_client_sync.txt
}
extension EGL_EXT_gl_colorspace_display_p3_passthrough {
    number      130
    flags       public
    filename    extensions/EXT/EGL_EXT_gl_colorspace_display_p3_passthrough.txt
}
extension EGL_MESA_query_driver {
    number      131
    flags       public
    filename    extensions/MESA/EGL_MESA_query_driver.txt
}
extension EGL_ANDROID_GLES_layers {
    number      132
    flags       public
    filename    extensions/ANDROID/EGL_ANDROID_GLES_layers.txt
}
extension EGL_NV_n_buffer {
    number      133
    flags       public
    filename    extensions/NV/EGL_NV_n_buffer.txt
}
extension EGL_NV_stream_origin {
    number      134
    flags       public
    filename    extensions/NV/EGL_NV_stream_origin.txt
}
extension EGL_NV_stream_dma {
    number      135
    flags       public
    filename    extensions/NV/EGL_NV_stream_dma.txt
}
extension EGL_WL_bind_wayland_display {
    number      136
    flags       public
    filename    extensions/WL/EGL_WL_bind_wayland_display.txt
}
extension EGL_WL_create_wayland_buffer_from_image {
    number      137
    flags       public
    filename    extensions/WL/EGL_WL_create_wayland_buffer_from_image.txt
}
extension EGL_ARM_image_format {
    number      138
    flags       public
    filename    extensions/ARM/EGL_ARM_image_format.txt
}
extension EGL_NV_stream_consumer_eglimage {
    number      139
    flags       public
    filename    extensions/NV/EGL_NV_stream_consumer_eglimage.txt
}
extension EGL_NV_stream_consumer_eglimage {
    number      140
    flags       public
    filename    extensions/EXT/EGL_EXT_device_query_name.txt
}
extension EGL_EXT_platform_xcb {
    number      141
    flags       public
    filename    extensions/EXT/EGL_EXT_platform_xcb.txt
}
extension EGL_ANGLE_sync_control_rate {
    number      142
    flags       public
    filename    extensions/ANGLE/EGL_ANGLE_sync_control_rate.txt
}
extension EGL_EXT_device_persistent_id {
    number      143
    flags       public
    filename    extensions/EXT/EGL_EXT_device_persistent_id.txt
}
extension EGL_EXT_device_drm_render_node {
    number      144
    flags       public
    filename    extensions/EXT/EGL_EXT_device_drm_render_node.txt
}
extension EGL_EXT_config_select_group {
    number      145
    flags       public
    filename    extensions/EXT/EGL_EXT_config_select_group.txt
}
extension EGL_EXT_present_opaque {
    number      146
    flags       public
    filename    extensions/EXT/EGL_EXT_present_opaque.txt
}
extension EGL_EXT_surface_compression {
    number      147
    flags       public
    filename    extensions/EXT/EGL_EXT_surface_compression.txt
}
extension EGL_EXT_explicit_device {
    number      148
    flags       public
    filename    extensions/EXT/EGL_EXT_explicit_device.txt
}
# Next free extension number: 149
