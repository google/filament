// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See extension_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#include "vk_extension_helper.h"

vvl::Extension GetExtension(std::string extension) {
    static const vvl::unordered_map<std::string, vvl::Extension> extension_map{
        {"VK_KHR_surface", vvl::Extension::_VK_KHR_surface},
        {"VK_KHR_swapchain", vvl::Extension::_VK_KHR_swapchain},
        {"VK_KHR_display", vvl::Extension::_VK_KHR_display},
        {"VK_KHR_display_swapchain", vvl::Extension::_VK_KHR_display_swapchain},
        {"VK_KHR_xlib_surface", vvl::Extension::_VK_KHR_xlib_surface},
        {"VK_KHR_xcb_surface", vvl::Extension::_VK_KHR_xcb_surface},
        {"VK_KHR_wayland_surface", vvl::Extension::_VK_KHR_wayland_surface},
        {"VK_KHR_android_surface", vvl::Extension::_VK_KHR_android_surface},
        {"VK_KHR_win32_surface", vvl::Extension::_VK_KHR_win32_surface},
        {"VK_KHR_sampler_mirror_clamp_to_edge", vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge},
        {"VK_KHR_video_queue", vvl::Extension::_VK_KHR_video_queue},
        {"VK_KHR_video_decode_queue", vvl::Extension::_VK_KHR_video_decode_queue},
        {"VK_KHR_video_encode_h264", vvl::Extension::_VK_KHR_video_encode_h264},
        {"VK_KHR_video_encode_h265", vvl::Extension::_VK_KHR_video_encode_h265},
        {"VK_KHR_video_decode_h264", vvl::Extension::_VK_KHR_video_decode_h264},
        {"VK_KHR_dynamic_rendering", vvl::Extension::_VK_KHR_dynamic_rendering},
        {"VK_KHR_multiview", vvl::Extension::_VK_KHR_multiview},
        {"VK_KHR_get_physical_device_properties2", vvl::Extension::_VK_KHR_get_physical_device_properties2},
        {"VK_KHR_device_group", vvl::Extension::_VK_KHR_device_group},
        {"VK_KHR_shader_draw_parameters", vvl::Extension::_VK_KHR_shader_draw_parameters},
        {"VK_KHR_maintenance1", vvl::Extension::_VK_KHR_maintenance1},
        {"VK_KHR_device_group_creation", vvl::Extension::_VK_KHR_device_group_creation},
        {"VK_KHR_external_memory_capabilities", vvl::Extension::_VK_KHR_external_memory_capabilities},
        {"VK_KHR_external_memory", vvl::Extension::_VK_KHR_external_memory},
        {"VK_KHR_external_memory_win32", vvl::Extension::_VK_KHR_external_memory_win32},
        {"VK_KHR_external_memory_fd", vvl::Extension::_VK_KHR_external_memory_fd},
        {"VK_KHR_win32_keyed_mutex", vvl::Extension::_VK_KHR_win32_keyed_mutex},
        {"VK_KHR_external_semaphore_capabilities", vvl::Extension::_VK_KHR_external_semaphore_capabilities},
        {"VK_KHR_external_semaphore", vvl::Extension::_VK_KHR_external_semaphore},
        {"VK_KHR_external_semaphore_win32", vvl::Extension::_VK_KHR_external_semaphore_win32},
        {"VK_KHR_external_semaphore_fd", vvl::Extension::_VK_KHR_external_semaphore_fd},
        {"VK_KHR_push_descriptor", vvl::Extension::_VK_KHR_push_descriptor},
        {"VK_KHR_shader_float16_int8", vvl::Extension::_VK_KHR_shader_float16_int8},
        {"VK_KHR_16bit_storage", vvl::Extension::_VK_KHR_16bit_storage},
        {"VK_KHR_incremental_present", vvl::Extension::_VK_KHR_incremental_present},
        {"VK_KHR_descriptor_update_template", vvl::Extension::_VK_KHR_descriptor_update_template},
        {"VK_KHR_imageless_framebuffer", vvl::Extension::_VK_KHR_imageless_framebuffer},
        {"VK_KHR_create_renderpass2", vvl::Extension::_VK_KHR_create_renderpass2},
        {"VK_KHR_shared_presentable_image", vvl::Extension::_VK_KHR_shared_presentable_image},
        {"VK_KHR_external_fence_capabilities", vvl::Extension::_VK_KHR_external_fence_capabilities},
        {"VK_KHR_external_fence", vvl::Extension::_VK_KHR_external_fence},
        {"VK_KHR_external_fence_win32", vvl::Extension::_VK_KHR_external_fence_win32},
        {"VK_KHR_external_fence_fd", vvl::Extension::_VK_KHR_external_fence_fd},
        {"VK_KHR_performance_query", vvl::Extension::_VK_KHR_performance_query},
        {"VK_KHR_maintenance2", vvl::Extension::_VK_KHR_maintenance2},
        {"VK_KHR_get_surface_capabilities2", vvl::Extension::_VK_KHR_get_surface_capabilities2},
        {"VK_KHR_variable_pointers", vvl::Extension::_VK_KHR_variable_pointers},
        {"VK_KHR_get_display_properties2", vvl::Extension::_VK_KHR_get_display_properties2},
        {"VK_KHR_dedicated_allocation", vvl::Extension::_VK_KHR_dedicated_allocation},
        {"VK_KHR_storage_buffer_storage_class", vvl::Extension::_VK_KHR_storage_buffer_storage_class},
        {"VK_KHR_relaxed_block_layout", vvl::Extension::_VK_KHR_relaxed_block_layout},
        {"VK_KHR_get_memory_requirements2", vvl::Extension::_VK_KHR_get_memory_requirements2},
        {"VK_KHR_image_format_list", vvl::Extension::_VK_KHR_image_format_list},
        {"VK_KHR_sampler_ycbcr_conversion", vvl::Extension::_VK_KHR_sampler_ycbcr_conversion},
        {"VK_KHR_bind_memory2", vvl::Extension::_VK_KHR_bind_memory2},
        {"VK_KHR_portability_subset", vvl::Extension::_VK_KHR_portability_subset},
        {"VK_KHR_maintenance3", vvl::Extension::_VK_KHR_maintenance3},
        {"VK_KHR_draw_indirect_count", vvl::Extension::_VK_KHR_draw_indirect_count},
        {"VK_KHR_shader_subgroup_extended_types", vvl::Extension::_VK_KHR_shader_subgroup_extended_types},
        {"VK_KHR_8bit_storage", vvl::Extension::_VK_KHR_8bit_storage},
        {"VK_KHR_shader_atomic_int64", vvl::Extension::_VK_KHR_shader_atomic_int64},
        {"VK_KHR_shader_clock", vvl::Extension::_VK_KHR_shader_clock},
        {"VK_KHR_video_decode_h265", vvl::Extension::_VK_KHR_video_decode_h265},
        {"VK_KHR_global_priority", vvl::Extension::_VK_KHR_global_priority},
        {"VK_KHR_driver_properties", vvl::Extension::_VK_KHR_driver_properties},
        {"VK_KHR_shader_float_controls", vvl::Extension::_VK_KHR_shader_float_controls},
        {"VK_KHR_depth_stencil_resolve", vvl::Extension::_VK_KHR_depth_stencil_resolve},
        {"VK_KHR_swapchain_mutable_format", vvl::Extension::_VK_KHR_swapchain_mutable_format},
        {"VK_KHR_timeline_semaphore", vvl::Extension::_VK_KHR_timeline_semaphore},
        {"VK_KHR_vulkan_memory_model", vvl::Extension::_VK_KHR_vulkan_memory_model},
        {"VK_KHR_shader_terminate_invocation", vvl::Extension::_VK_KHR_shader_terminate_invocation},
        {"VK_KHR_fragment_shading_rate", vvl::Extension::_VK_KHR_fragment_shading_rate},
        {"VK_KHR_dynamic_rendering_local_read", vvl::Extension::_VK_KHR_dynamic_rendering_local_read},
        {"VK_KHR_shader_quad_control", vvl::Extension::_VK_KHR_shader_quad_control},
        {"VK_KHR_spirv_1_4", vvl::Extension::_VK_KHR_spirv_1_4},
        {"VK_KHR_surface_protected_capabilities", vvl::Extension::_VK_KHR_surface_protected_capabilities},
        {"VK_KHR_separate_depth_stencil_layouts", vvl::Extension::_VK_KHR_separate_depth_stencil_layouts},
        {"VK_KHR_present_wait", vvl::Extension::_VK_KHR_present_wait},
        {"VK_KHR_uniform_buffer_standard_layout", vvl::Extension::_VK_KHR_uniform_buffer_standard_layout},
        {"VK_KHR_buffer_device_address", vvl::Extension::_VK_KHR_buffer_device_address},
        {"VK_KHR_deferred_host_operations", vvl::Extension::_VK_KHR_deferred_host_operations},
        {"VK_KHR_pipeline_executable_properties", vvl::Extension::_VK_KHR_pipeline_executable_properties},
        {"VK_KHR_map_memory2", vvl::Extension::_VK_KHR_map_memory2},
        {"VK_KHR_shader_integer_dot_product", vvl::Extension::_VK_KHR_shader_integer_dot_product},
        {"VK_KHR_pipeline_library", vvl::Extension::_VK_KHR_pipeline_library},
        {"VK_KHR_shader_non_semantic_info", vvl::Extension::_VK_KHR_shader_non_semantic_info},
        {"VK_KHR_present_id", vvl::Extension::_VK_KHR_present_id},
        {"VK_KHR_video_encode_queue", vvl::Extension::_VK_KHR_video_encode_queue},
        {"VK_KHR_synchronization2", vvl::Extension::_VK_KHR_synchronization2},
        {"VK_KHR_fragment_shader_barycentric", vvl::Extension::_VK_KHR_fragment_shader_barycentric},
        {"VK_KHR_shader_subgroup_uniform_control_flow", vvl::Extension::_VK_KHR_shader_subgroup_uniform_control_flow},
        {"VK_KHR_zero_initialize_workgroup_memory", vvl::Extension::_VK_KHR_zero_initialize_workgroup_memory},
        {"VK_KHR_workgroup_memory_explicit_layout", vvl::Extension::_VK_KHR_workgroup_memory_explicit_layout},
        {"VK_KHR_copy_commands2", vvl::Extension::_VK_KHR_copy_commands2},
        {"VK_KHR_format_feature_flags2", vvl::Extension::_VK_KHR_format_feature_flags2},
        {"VK_KHR_ray_tracing_maintenance1", vvl::Extension::_VK_KHR_ray_tracing_maintenance1},
        {"VK_KHR_portability_enumeration", vvl::Extension::_VK_KHR_portability_enumeration},
        {"VK_KHR_maintenance4", vvl::Extension::_VK_KHR_maintenance4},
        {"VK_KHR_shader_subgroup_rotate", vvl::Extension::_VK_KHR_shader_subgroup_rotate},
        {"VK_KHR_shader_maximal_reconvergence", vvl::Extension::_VK_KHR_shader_maximal_reconvergence},
        {"VK_KHR_maintenance5", vvl::Extension::_VK_KHR_maintenance5},
        {"VK_KHR_ray_tracing_position_fetch", vvl::Extension::_VK_KHR_ray_tracing_position_fetch},
        {"VK_KHR_pipeline_binary", vvl::Extension::_VK_KHR_pipeline_binary},
        {"VK_KHR_cooperative_matrix", vvl::Extension::_VK_KHR_cooperative_matrix},
        {"VK_KHR_compute_shader_derivatives", vvl::Extension::_VK_KHR_compute_shader_derivatives},
        {"VK_KHR_video_decode_av1", vvl::Extension::_VK_KHR_video_decode_av1},
        {"VK_KHR_video_encode_av1", vvl::Extension::_VK_KHR_video_encode_av1},
        {"VK_KHR_video_maintenance1", vvl::Extension::_VK_KHR_video_maintenance1},
        {"VK_KHR_vertex_attribute_divisor", vvl::Extension::_VK_KHR_vertex_attribute_divisor},
        {"VK_KHR_load_store_op_none", vvl::Extension::_VK_KHR_load_store_op_none},
        {"VK_KHR_shader_float_controls2", vvl::Extension::_VK_KHR_shader_float_controls2},
        {"VK_KHR_index_type_uint8", vvl::Extension::_VK_KHR_index_type_uint8},
        {"VK_KHR_line_rasterization", vvl::Extension::_VK_KHR_line_rasterization},
        {"VK_KHR_calibrated_timestamps", vvl::Extension::_VK_KHR_calibrated_timestamps},
        {"VK_KHR_shader_expect_assume", vvl::Extension::_VK_KHR_shader_expect_assume},
        {"VK_KHR_maintenance6", vvl::Extension::_VK_KHR_maintenance6},
        {"VK_KHR_video_encode_quantization_map", vvl::Extension::_VK_KHR_video_encode_quantization_map},
        {"VK_KHR_shader_relaxed_extended_instruction", vvl::Extension::_VK_KHR_shader_relaxed_extended_instruction},
        {"VK_KHR_maintenance7", vvl::Extension::_VK_KHR_maintenance7},
        {"VK_KHR_maintenance8", vvl::Extension::_VK_KHR_maintenance8},
        {"VK_KHR_video_maintenance2", vvl::Extension::_VK_KHR_video_maintenance2},
        {"VK_KHR_depth_clamp_zero_one", vvl::Extension::_VK_KHR_depth_clamp_zero_one},
        {"VK_EXT_debug_report", vvl::Extension::_VK_EXT_debug_report},
        {"VK_NV_glsl_shader", vvl::Extension::_VK_NV_glsl_shader},
        {"VK_EXT_depth_range_unrestricted", vvl::Extension::_VK_EXT_depth_range_unrestricted},
        {"VK_IMG_filter_cubic", vvl::Extension::_VK_IMG_filter_cubic},
        {"VK_AMD_rasterization_order", vvl::Extension::_VK_AMD_rasterization_order},
        {"VK_AMD_shader_trinary_minmax", vvl::Extension::_VK_AMD_shader_trinary_minmax},
        {"VK_AMD_shader_explicit_vertex_parameter", vvl::Extension::_VK_AMD_shader_explicit_vertex_parameter},
        {"VK_EXT_debug_marker", vvl::Extension::_VK_EXT_debug_marker},
        {"VK_AMD_gcn_shader", vvl::Extension::_VK_AMD_gcn_shader},
        {"VK_NV_dedicated_allocation", vvl::Extension::_VK_NV_dedicated_allocation},
        {"VK_EXT_transform_feedback", vvl::Extension::_VK_EXT_transform_feedback},
        {"VK_NVX_binary_import", vvl::Extension::_VK_NVX_binary_import},
        {"VK_NVX_image_view_handle", vvl::Extension::_VK_NVX_image_view_handle},
        {"VK_AMD_draw_indirect_count", vvl::Extension::_VK_AMD_draw_indirect_count},
        {"VK_AMD_negative_viewport_height", vvl::Extension::_VK_AMD_negative_viewport_height},
        {"VK_AMD_gpu_shader_half_float", vvl::Extension::_VK_AMD_gpu_shader_half_float},
        {"VK_AMD_shader_ballot", vvl::Extension::_VK_AMD_shader_ballot},
        {"VK_AMD_texture_gather_bias_lod", vvl::Extension::_VK_AMD_texture_gather_bias_lod},
        {"VK_AMD_shader_info", vvl::Extension::_VK_AMD_shader_info},
        {"VK_AMD_shader_image_load_store_lod", vvl::Extension::_VK_AMD_shader_image_load_store_lod},
        {"VK_GGP_stream_descriptor_surface", vvl::Extension::_VK_GGP_stream_descriptor_surface},
        {"VK_NV_corner_sampled_image", vvl::Extension::_VK_NV_corner_sampled_image},
        {"VK_IMG_format_pvrtc", vvl::Extension::_VK_IMG_format_pvrtc},
        {"VK_NV_external_memory_capabilities", vvl::Extension::_VK_NV_external_memory_capabilities},
        {"VK_NV_external_memory", vvl::Extension::_VK_NV_external_memory},
        {"VK_NV_external_memory_win32", vvl::Extension::_VK_NV_external_memory_win32},
        {"VK_NV_win32_keyed_mutex", vvl::Extension::_VK_NV_win32_keyed_mutex},
        {"VK_EXT_validation_flags", vvl::Extension::_VK_EXT_validation_flags},
        {"VK_NN_vi_surface", vvl::Extension::_VK_NN_vi_surface},
        {"VK_EXT_shader_subgroup_ballot", vvl::Extension::_VK_EXT_shader_subgroup_ballot},
        {"VK_EXT_shader_subgroup_vote", vvl::Extension::_VK_EXT_shader_subgroup_vote},
        {"VK_EXT_texture_compression_astc_hdr", vvl::Extension::_VK_EXT_texture_compression_astc_hdr},
        {"VK_EXT_astc_decode_mode", vvl::Extension::_VK_EXT_astc_decode_mode},
        {"VK_EXT_pipeline_robustness", vvl::Extension::_VK_EXT_pipeline_robustness},
        {"VK_EXT_conditional_rendering", vvl::Extension::_VK_EXT_conditional_rendering},
        {"VK_NV_clip_space_w_scaling", vvl::Extension::_VK_NV_clip_space_w_scaling},
        {"VK_EXT_direct_mode_display", vvl::Extension::_VK_EXT_direct_mode_display},
        {"VK_EXT_acquire_xlib_display", vvl::Extension::_VK_EXT_acquire_xlib_display},
        {"VK_EXT_display_surface_counter", vvl::Extension::_VK_EXT_display_surface_counter},
        {"VK_EXT_display_control", vvl::Extension::_VK_EXT_display_control},
        {"VK_GOOGLE_display_timing", vvl::Extension::_VK_GOOGLE_display_timing},
        {"VK_NV_sample_mask_override_coverage", vvl::Extension::_VK_NV_sample_mask_override_coverage},
        {"VK_NV_geometry_shader_passthrough", vvl::Extension::_VK_NV_geometry_shader_passthrough},
        {"VK_NV_viewport_array2", vvl::Extension::_VK_NV_viewport_array2},
        {"VK_NVX_multiview_per_view_attributes", vvl::Extension::_VK_NVX_multiview_per_view_attributes},
        {"VK_NV_viewport_swizzle", vvl::Extension::_VK_NV_viewport_swizzle},
        {"VK_EXT_discard_rectangles", vvl::Extension::_VK_EXT_discard_rectangles},
        {"VK_EXT_conservative_rasterization", vvl::Extension::_VK_EXT_conservative_rasterization},
        {"VK_EXT_depth_clip_enable", vvl::Extension::_VK_EXT_depth_clip_enable},
        {"VK_EXT_swapchain_colorspace", vvl::Extension::_VK_EXT_swapchain_colorspace},
        {"VK_EXT_hdr_metadata", vvl::Extension::_VK_EXT_hdr_metadata},
        {"VK_IMG_relaxed_line_rasterization", vvl::Extension::_VK_IMG_relaxed_line_rasterization},
        {"VK_MVK_ios_surface", vvl::Extension::_VK_MVK_ios_surface},
        {"VK_MVK_macos_surface", vvl::Extension::_VK_MVK_macos_surface},
        {"VK_EXT_external_memory_dma_buf", vvl::Extension::_VK_EXT_external_memory_dma_buf},
        {"VK_EXT_queue_family_foreign", vvl::Extension::_VK_EXT_queue_family_foreign},
        {"VK_EXT_debug_utils", vvl::Extension::_VK_EXT_debug_utils},
        {"VK_ANDROID_external_memory_android_hardware_buffer", vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer},
        {"VK_EXT_sampler_filter_minmax", vvl::Extension::_VK_EXT_sampler_filter_minmax},
        {"VK_AMD_gpu_shader_int16", vvl::Extension::_VK_AMD_gpu_shader_int16},
        {"VK_AMDX_shader_enqueue", vvl::Extension::_VK_AMDX_shader_enqueue},
        {"VK_AMD_mixed_attachment_samples", vvl::Extension::_VK_AMD_mixed_attachment_samples},
        {"VK_AMD_shader_fragment_mask", vvl::Extension::_VK_AMD_shader_fragment_mask},
        {"VK_EXT_inline_uniform_block", vvl::Extension::_VK_EXT_inline_uniform_block},
        {"VK_EXT_shader_stencil_export", vvl::Extension::_VK_EXT_shader_stencil_export},
        {"VK_EXT_sample_locations", vvl::Extension::_VK_EXT_sample_locations},
        {"VK_EXT_blend_operation_advanced", vvl::Extension::_VK_EXT_blend_operation_advanced},
        {"VK_NV_fragment_coverage_to_color", vvl::Extension::_VK_NV_fragment_coverage_to_color},
        {"VK_NV_framebuffer_mixed_samples", vvl::Extension::_VK_NV_framebuffer_mixed_samples},
        {"VK_NV_fill_rectangle", vvl::Extension::_VK_NV_fill_rectangle},
        {"VK_NV_shader_sm_builtins", vvl::Extension::_VK_NV_shader_sm_builtins},
        {"VK_EXT_post_depth_coverage", vvl::Extension::_VK_EXT_post_depth_coverage},
        {"VK_EXT_image_drm_format_modifier", vvl::Extension::_VK_EXT_image_drm_format_modifier},
        {"VK_EXT_validation_cache", vvl::Extension::_VK_EXT_validation_cache},
        {"VK_EXT_descriptor_indexing", vvl::Extension::_VK_EXT_descriptor_indexing},
        {"VK_EXT_shader_viewport_index_layer", vvl::Extension::_VK_EXT_shader_viewport_index_layer},
        {"VK_NV_shading_rate_image", vvl::Extension::_VK_NV_shading_rate_image},
        {"VK_NV_ray_tracing", vvl::Extension::_VK_NV_ray_tracing},
        {"VK_NV_representative_fragment_test", vvl::Extension::_VK_NV_representative_fragment_test},
        {"VK_EXT_filter_cubic", vvl::Extension::_VK_EXT_filter_cubic},
        {"VK_QCOM_render_pass_shader_resolve", vvl::Extension::_VK_QCOM_render_pass_shader_resolve},
        {"VK_EXT_global_priority", vvl::Extension::_VK_EXT_global_priority},
        {"VK_EXT_external_memory_host", vvl::Extension::_VK_EXT_external_memory_host},
        {"VK_AMD_buffer_marker", vvl::Extension::_VK_AMD_buffer_marker},
        {"VK_AMD_pipeline_compiler_control", vvl::Extension::_VK_AMD_pipeline_compiler_control},
        {"VK_EXT_calibrated_timestamps", vvl::Extension::_VK_EXT_calibrated_timestamps},
        {"VK_AMD_shader_core_properties", vvl::Extension::_VK_AMD_shader_core_properties},
        {"VK_AMD_memory_overallocation_behavior", vvl::Extension::_VK_AMD_memory_overallocation_behavior},
        {"VK_EXT_vertex_attribute_divisor", vvl::Extension::_VK_EXT_vertex_attribute_divisor},
        {"VK_GGP_frame_token", vvl::Extension::_VK_GGP_frame_token},
        {"VK_EXT_pipeline_creation_feedback", vvl::Extension::_VK_EXT_pipeline_creation_feedback},
        {"VK_NV_shader_subgroup_partitioned", vvl::Extension::_VK_NV_shader_subgroup_partitioned},
        {"VK_NV_compute_shader_derivatives", vvl::Extension::_VK_NV_compute_shader_derivatives},
        {"VK_NV_mesh_shader", vvl::Extension::_VK_NV_mesh_shader},
        {"VK_NV_fragment_shader_barycentric", vvl::Extension::_VK_NV_fragment_shader_barycentric},
        {"VK_NV_shader_image_footprint", vvl::Extension::_VK_NV_shader_image_footprint},
        {"VK_NV_scissor_exclusive", vvl::Extension::_VK_NV_scissor_exclusive},
        {"VK_NV_device_diagnostic_checkpoints", vvl::Extension::_VK_NV_device_diagnostic_checkpoints},
        {"VK_INTEL_shader_integer_functions2", vvl::Extension::_VK_INTEL_shader_integer_functions2},
        {"VK_INTEL_performance_query", vvl::Extension::_VK_INTEL_performance_query},
        {"VK_EXT_pci_bus_info", vvl::Extension::_VK_EXT_pci_bus_info},
        {"VK_AMD_display_native_hdr", vvl::Extension::_VK_AMD_display_native_hdr},
        {"VK_FUCHSIA_imagepipe_surface", vvl::Extension::_VK_FUCHSIA_imagepipe_surface},
        {"VK_EXT_metal_surface", vvl::Extension::_VK_EXT_metal_surface},
        {"VK_EXT_fragment_density_map", vvl::Extension::_VK_EXT_fragment_density_map},
        {"VK_EXT_scalar_block_layout", vvl::Extension::_VK_EXT_scalar_block_layout},
        {"VK_GOOGLE_hlsl_functionality1", vvl::Extension::_VK_GOOGLE_hlsl_functionality1},
        {"VK_GOOGLE_decorate_string", vvl::Extension::_VK_GOOGLE_decorate_string},
        {"VK_EXT_subgroup_size_control", vvl::Extension::_VK_EXT_subgroup_size_control},
        {"VK_AMD_shader_core_properties2", vvl::Extension::_VK_AMD_shader_core_properties2},
        {"VK_AMD_device_coherent_memory", vvl::Extension::_VK_AMD_device_coherent_memory},
        {"VK_EXT_shader_image_atomic_int64", vvl::Extension::_VK_EXT_shader_image_atomic_int64},
        {"VK_EXT_memory_budget", vvl::Extension::_VK_EXT_memory_budget},
        {"VK_EXT_memory_priority", vvl::Extension::_VK_EXT_memory_priority},
        {"VK_NV_dedicated_allocation_image_aliasing", vvl::Extension::_VK_NV_dedicated_allocation_image_aliasing},
        {"VK_EXT_buffer_device_address", vvl::Extension::_VK_EXT_buffer_device_address},
        {"VK_EXT_tooling_info", vvl::Extension::_VK_EXT_tooling_info},
        {"VK_EXT_separate_stencil_usage", vvl::Extension::_VK_EXT_separate_stencil_usage},
        {"VK_EXT_validation_features", vvl::Extension::_VK_EXT_validation_features},
        {"VK_NV_cooperative_matrix", vvl::Extension::_VK_NV_cooperative_matrix},
        {"VK_NV_coverage_reduction_mode", vvl::Extension::_VK_NV_coverage_reduction_mode},
        {"VK_EXT_fragment_shader_interlock", vvl::Extension::_VK_EXT_fragment_shader_interlock},
        {"VK_EXT_ycbcr_image_arrays", vvl::Extension::_VK_EXT_ycbcr_image_arrays},
        {"VK_EXT_provoking_vertex", vvl::Extension::_VK_EXT_provoking_vertex},
        {"VK_EXT_full_screen_exclusive", vvl::Extension::_VK_EXT_full_screen_exclusive},
        {"VK_EXT_headless_surface", vvl::Extension::_VK_EXT_headless_surface},
        {"VK_EXT_line_rasterization", vvl::Extension::_VK_EXT_line_rasterization},
        {"VK_EXT_shader_atomic_float", vvl::Extension::_VK_EXT_shader_atomic_float},
        {"VK_EXT_host_query_reset", vvl::Extension::_VK_EXT_host_query_reset},
        {"VK_EXT_index_type_uint8", vvl::Extension::_VK_EXT_index_type_uint8},
        {"VK_EXT_extended_dynamic_state", vvl::Extension::_VK_EXT_extended_dynamic_state},
        {"VK_EXT_host_image_copy", vvl::Extension::_VK_EXT_host_image_copy},
        {"VK_EXT_map_memory_placed", vvl::Extension::_VK_EXT_map_memory_placed},
        {"VK_EXT_shader_atomic_float2", vvl::Extension::_VK_EXT_shader_atomic_float2},
        {"VK_EXT_surface_maintenance1", vvl::Extension::_VK_EXT_surface_maintenance1},
        {"VK_EXT_swapchain_maintenance1", vvl::Extension::_VK_EXT_swapchain_maintenance1},
        {"VK_EXT_shader_demote_to_helper_invocation", vvl::Extension::_VK_EXT_shader_demote_to_helper_invocation},
        {"VK_NV_device_generated_commands", vvl::Extension::_VK_NV_device_generated_commands},
        {"VK_NV_inherited_viewport_scissor", vvl::Extension::_VK_NV_inherited_viewport_scissor},
        {"VK_EXT_texel_buffer_alignment", vvl::Extension::_VK_EXT_texel_buffer_alignment},
        {"VK_QCOM_render_pass_transform", vvl::Extension::_VK_QCOM_render_pass_transform},
        {"VK_EXT_depth_bias_control", vvl::Extension::_VK_EXT_depth_bias_control},
        {"VK_EXT_device_memory_report", vvl::Extension::_VK_EXT_device_memory_report},
        {"VK_EXT_acquire_drm_display", vvl::Extension::_VK_EXT_acquire_drm_display},
        {"VK_EXT_robustness2", vvl::Extension::_VK_EXT_robustness2},
        {"VK_EXT_custom_border_color", vvl::Extension::_VK_EXT_custom_border_color},
        {"VK_GOOGLE_user_type", vvl::Extension::_VK_GOOGLE_user_type},
        {"VK_NV_present_barrier", vvl::Extension::_VK_NV_present_barrier},
        {"VK_EXT_private_data", vvl::Extension::_VK_EXT_private_data},
        {"VK_EXT_pipeline_creation_cache_control", vvl::Extension::_VK_EXT_pipeline_creation_cache_control},
        {"VK_NV_device_diagnostics_config", vvl::Extension::_VK_NV_device_diagnostics_config},
        {"VK_QCOM_render_pass_store_ops", vvl::Extension::_VK_QCOM_render_pass_store_ops},
        {"VK_NV_cuda_kernel_launch", vvl::Extension::_VK_NV_cuda_kernel_launch},
        {"VK_NV_low_latency", vvl::Extension::_VK_NV_low_latency},
        {"VK_EXT_metal_objects", vvl::Extension::_VK_EXT_metal_objects},
        {"VK_EXT_descriptor_buffer", vvl::Extension::_VK_EXT_descriptor_buffer},
        {"VK_EXT_graphics_pipeline_library", vvl::Extension::_VK_EXT_graphics_pipeline_library},
        {"VK_AMD_shader_early_and_late_fragment_tests", vvl::Extension::_VK_AMD_shader_early_and_late_fragment_tests},
        {"VK_NV_fragment_shading_rate_enums", vvl::Extension::_VK_NV_fragment_shading_rate_enums},
        {"VK_NV_ray_tracing_motion_blur", vvl::Extension::_VK_NV_ray_tracing_motion_blur},
        {"VK_EXT_ycbcr_2plane_444_formats", vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats},
        {"VK_EXT_fragment_density_map2", vvl::Extension::_VK_EXT_fragment_density_map2},
        {"VK_QCOM_rotated_copy_commands", vvl::Extension::_VK_QCOM_rotated_copy_commands},
        {"VK_EXT_image_robustness", vvl::Extension::_VK_EXT_image_robustness},
        {"VK_EXT_image_compression_control", vvl::Extension::_VK_EXT_image_compression_control},
        {"VK_EXT_attachment_feedback_loop_layout", vvl::Extension::_VK_EXT_attachment_feedback_loop_layout},
        {"VK_EXT_4444_formats", vvl::Extension::_VK_EXT_4444_formats},
        {"VK_EXT_device_fault", vvl::Extension::_VK_EXT_device_fault},
        {"VK_ARM_rasterization_order_attachment_access", vvl::Extension::_VK_ARM_rasterization_order_attachment_access},
        {"VK_EXT_rgba10x6_formats", vvl::Extension::_VK_EXT_rgba10x6_formats},
        {"VK_NV_acquire_winrt_display", vvl::Extension::_VK_NV_acquire_winrt_display},
        {"VK_EXT_directfb_surface", vvl::Extension::_VK_EXT_directfb_surface},
        {"VK_VALVE_mutable_descriptor_type", vvl::Extension::_VK_VALVE_mutable_descriptor_type},
        {"VK_EXT_vertex_input_dynamic_state", vvl::Extension::_VK_EXT_vertex_input_dynamic_state},
        {"VK_EXT_physical_device_drm", vvl::Extension::_VK_EXT_physical_device_drm},
        {"VK_EXT_device_address_binding_report", vvl::Extension::_VK_EXT_device_address_binding_report},
        {"VK_EXT_depth_clip_control", vvl::Extension::_VK_EXT_depth_clip_control},
        {"VK_EXT_primitive_topology_list_restart", vvl::Extension::_VK_EXT_primitive_topology_list_restart},
        {"VK_EXT_present_mode_fifo_latest_ready", vvl::Extension::_VK_EXT_present_mode_fifo_latest_ready},
        {"VK_FUCHSIA_external_memory", vvl::Extension::_VK_FUCHSIA_external_memory},
        {"VK_FUCHSIA_external_semaphore", vvl::Extension::_VK_FUCHSIA_external_semaphore},
        {"VK_FUCHSIA_buffer_collection", vvl::Extension::_VK_FUCHSIA_buffer_collection},
        {"VK_HUAWEI_subpass_shading", vvl::Extension::_VK_HUAWEI_subpass_shading},
        {"VK_HUAWEI_invocation_mask", vvl::Extension::_VK_HUAWEI_invocation_mask},
        {"VK_NV_external_memory_rdma", vvl::Extension::_VK_NV_external_memory_rdma},
        {"VK_EXT_pipeline_properties", vvl::Extension::_VK_EXT_pipeline_properties},
        {"VK_EXT_frame_boundary", vvl::Extension::_VK_EXT_frame_boundary},
        {"VK_EXT_multisampled_render_to_single_sampled", vvl::Extension::_VK_EXT_multisampled_render_to_single_sampled},
        {"VK_EXT_extended_dynamic_state2", vvl::Extension::_VK_EXT_extended_dynamic_state2},
        {"VK_QNX_screen_surface", vvl::Extension::_VK_QNX_screen_surface},
        {"VK_EXT_color_write_enable", vvl::Extension::_VK_EXT_color_write_enable},
        {"VK_EXT_primitives_generated_query", vvl::Extension::_VK_EXT_primitives_generated_query},
        {"VK_EXT_global_priority_query", vvl::Extension::_VK_EXT_global_priority_query},
        {"VK_EXT_image_view_min_lod", vvl::Extension::_VK_EXT_image_view_min_lod},
        {"VK_EXT_multi_draw", vvl::Extension::_VK_EXT_multi_draw},
        {"VK_EXT_image_2d_view_of_3d", vvl::Extension::_VK_EXT_image_2d_view_of_3d},
        {"VK_EXT_shader_tile_image", vvl::Extension::_VK_EXT_shader_tile_image},
        {"VK_EXT_opacity_micromap", vvl::Extension::_VK_EXT_opacity_micromap},
        {"VK_NV_displacement_micromap", vvl::Extension::_VK_NV_displacement_micromap},
        {"VK_EXT_load_store_op_none", vvl::Extension::_VK_EXT_load_store_op_none},
        {"VK_HUAWEI_cluster_culling_shader", vvl::Extension::_VK_HUAWEI_cluster_culling_shader},
        {"VK_EXT_border_color_swizzle", vvl::Extension::_VK_EXT_border_color_swizzle},
        {"VK_EXT_pageable_device_local_memory", vvl::Extension::_VK_EXT_pageable_device_local_memory},
        {"VK_ARM_shader_core_properties", vvl::Extension::_VK_ARM_shader_core_properties},
        {"VK_ARM_scheduling_controls", vvl::Extension::_VK_ARM_scheduling_controls},
        {"VK_EXT_image_sliced_view_of_3d", vvl::Extension::_VK_EXT_image_sliced_view_of_3d},
        {"VK_VALVE_descriptor_set_host_mapping", vvl::Extension::_VK_VALVE_descriptor_set_host_mapping},
        {"VK_EXT_depth_clamp_zero_one", vvl::Extension::_VK_EXT_depth_clamp_zero_one},
        {"VK_EXT_non_seamless_cube_map", vvl::Extension::_VK_EXT_non_seamless_cube_map},
        {"VK_ARM_render_pass_striped", vvl::Extension::_VK_ARM_render_pass_striped},
        {"VK_QCOM_fragment_density_map_offset", vvl::Extension::_VK_QCOM_fragment_density_map_offset},
        {"VK_NV_copy_memory_indirect", vvl::Extension::_VK_NV_copy_memory_indirect},
        {"VK_NV_memory_decompression", vvl::Extension::_VK_NV_memory_decompression},
        {"VK_NV_device_generated_commands_compute", vvl::Extension::_VK_NV_device_generated_commands_compute},
        {"VK_NV_ray_tracing_linear_swept_spheres", vvl::Extension::_VK_NV_ray_tracing_linear_swept_spheres},
        {"VK_NV_linear_color_attachment", vvl::Extension::_VK_NV_linear_color_attachment},
        {"VK_GOOGLE_surfaceless_query", vvl::Extension::_VK_GOOGLE_surfaceless_query},
        {"VK_EXT_image_compression_control_swapchain", vvl::Extension::_VK_EXT_image_compression_control_swapchain},
        {"VK_QCOM_image_processing", vvl::Extension::_VK_QCOM_image_processing},
        {"VK_EXT_nested_command_buffer", vvl::Extension::_VK_EXT_nested_command_buffer},
        {"VK_EXT_external_memory_acquire_unmodified", vvl::Extension::_VK_EXT_external_memory_acquire_unmodified},
        {"VK_EXT_extended_dynamic_state3", vvl::Extension::_VK_EXT_extended_dynamic_state3},
        {"VK_EXT_subpass_merge_feedback", vvl::Extension::_VK_EXT_subpass_merge_feedback},
        {"VK_LUNARG_direct_driver_loading", vvl::Extension::_VK_LUNARG_direct_driver_loading},
        {"VK_EXT_shader_module_identifier", vvl::Extension::_VK_EXT_shader_module_identifier},
        {"VK_EXT_rasterization_order_attachment_access", vvl::Extension::_VK_EXT_rasterization_order_attachment_access},
        {"VK_NV_optical_flow", vvl::Extension::_VK_NV_optical_flow},
        {"VK_EXT_legacy_dithering", vvl::Extension::_VK_EXT_legacy_dithering},
        {"VK_EXT_pipeline_protected_access", vvl::Extension::_VK_EXT_pipeline_protected_access},
        {"VK_ANDROID_external_format_resolve", vvl::Extension::_VK_ANDROID_external_format_resolve},
        {"VK_AMD_anti_lag", vvl::Extension::_VK_AMD_anti_lag},
        {"VK_EXT_shader_object", vvl::Extension::_VK_EXT_shader_object},
        {"VK_QCOM_tile_properties", vvl::Extension::_VK_QCOM_tile_properties},
        {"VK_SEC_amigo_profiling", vvl::Extension::_VK_SEC_amigo_profiling},
        {"VK_QCOM_multiview_per_view_viewports", vvl::Extension::_VK_QCOM_multiview_per_view_viewports},
        {"VK_NV_ray_tracing_invocation_reorder", vvl::Extension::_VK_NV_ray_tracing_invocation_reorder},
        {"VK_NV_cooperative_vector", vvl::Extension::_VK_NV_cooperative_vector},
        {"VK_NV_extended_sparse_address_space", vvl::Extension::_VK_NV_extended_sparse_address_space},
        {"VK_EXT_mutable_descriptor_type", vvl::Extension::_VK_EXT_mutable_descriptor_type},
        {"VK_EXT_legacy_vertex_attributes", vvl::Extension::_VK_EXT_legacy_vertex_attributes},
        {"VK_EXT_layer_settings", vvl::Extension::_VK_EXT_layer_settings},
        {"VK_ARM_shader_core_builtins", vvl::Extension::_VK_ARM_shader_core_builtins},
        {"VK_EXT_pipeline_library_group_handles", vvl::Extension::_VK_EXT_pipeline_library_group_handles},
        {"VK_EXT_dynamic_rendering_unused_attachments", vvl::Extension::_VK_EXT_dynamic_rendering_unused_attachments},
        {"VK_NV_low_latency2", vvl::Extension::_VK_NV_low_latency2},
        {"VK_QCOM_multiview_per_view_render_areas", vvl::Extension::_VK_QCOM_multiview_per_view_render_areas},
        {"VK_NV_per_stage_descriptor_set", vvl::Extension::_VK_NV_per_stage_descriptor_set},
        {"VK_QCOM_image_processing2", vvl::Extension::_VK_QCOM_image_processing2},
        {"VK_QCOM_filter_cubic_weights", vvl::Extension::_VK_QCOM_filter_cubic_weights},
        {"VK_QCOM_ycbcr_degamma", vvl::Extension::_VK_QCOM_ycbcr_degamma},
        {"VK_QCOM_filter_cubic_clamp", vvl::Extension::_VK_QCOM_filter_cubic_clamp},
        {"VK_EXT_attachment_feedback_loop_dynamic_state", vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state},
        {"VK_QNX_external_memory_screen_buffer", vvl::Extension::_VK_QNX_external_memory_screen_buffer},
        {"VK_MSFT_layered_driver", vvl::Extension::_VK_MSFT_layered_driver},
        {"VK_NV_descriptor_pool_overallocation", vvl::Extension::_VK_NV_descriptor_pool_overallocation},
        {"VK_NV_display_stereo", vvl::Extension::_VK_NV_display_stereo},
        {"VK_NV_raw_access_chains", vvl::Extension::_VK_NV_raw_access_chains},
        {"VK_NV_command_buffer_inheritance", vvl::Extension::_VK_NV_command_buffer_inheritance},
        {"VK_NV_shader_atomic_float16_vector", vvl::Extension::_VK_NV_shader_atomic_float16_vector},
        {"VK_EXT_shader_replicated_composites", vvl::Extension::_VK_EXT_shader_replicated_composites},
        {"VK_NV_ray_tracing_validation", vvl::Extension::_VK_NV_ray_tracing_validation},
        {"VK_NV_cluster_acceleration_structure", vvl::Extension::_VK_NV_cluster_acceleration_structure},
        {"VK_NV_partitioned_acceleration_structure", vvl::Extension::_VK_NV_partitioned_acceleration_structure},
        {"VK_EXT_device_generated_commands", vvl::Extension::_VK_EXT_device_generated_commands},
        {"VK_MESA_image_alignment_control", vvl::Extension::_VK_MESA_image_alignment_control},
        {"VK_EXT_depth_clamp_control", vvl::Extension::_VK_EXT_depth_clamp_control},
        {"VK_HUAWEI_hdr_vivid", vvl::Extension::_VK_HUAWEI_hdr_vivid},
        {"VK_NV_cooperative_matrix2", vvl::Extension::_VK_NV_cooperative_matrix2},
        {"VK_ARM_pipeline_opacity_micromap", vvl::Extension::_VK_ARM_pipeline_opacity_micromap},
        {"VK_EXT_external_memory_metal", vvl::Extension::_VK_EXT_external_memory_metal},
        {"VK_EXT_vertex_attribute_robustness", vvl::Extension::_VK_EXT_vertex_attribute_robustness},
        {"VK_KHR_acceleration_structure", vvl::Extension::_VK_KHR_acceleration_structure},
        {"VK_KHR_ray_tracing_pipeline", vvl::Extension::_VK_KHR_ray_tracing_pipeline},
        {"VK_KHR_ray_query", vvl::Extension::_VK_KHR_ray_query},
        {"VK_EXT_mesh_shader", vvl::Extension::_VK_EXT_mesh_shader},
    };
    const auto it = extension_map.find(extension);
    return (it == extension_map.end()) ? vvl::Extension::Empty : it->second;
}

const PromotedExtensionInfoMap& GetInstancePromotionInfoMap() {
    static const PromotedExtensionInfoMap promoted_map = {
        {VK_API_VERSION_1_1,
         {"VK_VERSION_1_1",
          {
              vvl::Extension::_VK_KHR_get_physical_device_properties2,
              vvl::Extension::_VK_KHR_device_group_creation,
              vvl::Extension::_VK_KHR_external_memory_capabilities,
              vvl::Extension::_VK_KHR_external_semaphore_capabilities,
              vvl::Extension::_VK_KHR_external_fence_capabilities,
          }}},

    };
    return promoted_map;
}

const PromotedExtensionInfoMap& GetDevicePromotionInfoMap() {
    static const PromotedExtensionInfoMap promoted_map = {
        {VK_API_VERSION_1_1,
         {"VK_VERSION_1_1",
          {
              vvl::Extension::_VK_KHR_multiview,
              vvl::Extension::_VK_KHR_device_group,
              vvl::Extension::_VK_KHR_shader_draw_parameters,
              vvl::Extension::_VK_KHR_maintenance1,
              vvl::Extension::_VK_KHR_external_memory,
              vvl::Extension::_VK_KHR_external_semaphore,
              vvl::Extension::_VK_KHR_16bit_storage,
              vvl::Extension::_VK_KHR_descriptor_update_template,
              vvl::Extension::_VK_KHR_external_fence,
              vvl::Extension::_VK_KHR_maintenance2,
              vvl::Extension::_VK_KHR_variable_pointers,
              vvl::Extension::_VK_KHR_dedicated_allocation,
              vvl::Extension::_VK_KHR_storage_buffer_storage_class,
              vvl::Extension::_VK_KHR_relaxed_block_layout,
              vvl::Extension::_VK_KHR_get_memory_requirements2,
              vvl::Extension::_VK_KHR_sampler_ycbcr_conversion,
              vvl::Extension::_VK_KHR_bind_memory2,
              vvl::Extension::_VK_KHR_maintenance3,
          }}},
        {VK_API_VERSION_1_2,
         {"VK_VERSION_1_2",
          {
              vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge,
              vvl::Extension::_VK_KHR_shader_float16_int8,
              vvl::Extension::_VK_KHR_imageless_framebuffer,
              vvl::Extension::_VK_KHR_create_renderpass2,
              vvl::Extension::_VK_KHR_image_format_list,
              vvl::Extension::_VK_KHR_draw_indirect_count,
              vvl::Extension::_VK_KHR_shader_subgroup_extended_types,
              vvl::Extension::_VK_KHR_8bit_storage,
              vvl::Extension::_VK_KHR_shader_atomic_int64,
              vvl::Extension::_VK_KHR_driver_properties,
              vvl::Extension::_VK_KHR_shader_float_controls,
              vvl::Extension::_VK_KHR_depth_stencil_resolve,
              vvl::Extension::_VK_KHR_timeline_semaphore,
              vvl::Extension::_VK_KHR_vulkan_memory_model,
              vvl::Extension::_VK_KHR_spirv_1_4,
              vvl::Extension::_VK_KHR_separate_depth_stencil_layouts,
              vvl::Extension::_VK_KHR_uniform_buffer_standard_layout,
              vvl::Extension::_VK_KHR_buffer_device_address,
              vvl::Extension::_VK_EXT_sampler_filter_minmax,
              vvl::Extension::_VK_EXT_descriptor_indexing,
              vvl::Extension::_VK_EXT_shader_viewport_index_layer,
              vvl::Extension::_VK_EXT_scalar_block_layout,
              vvl::Extension::_VK_EXT_separate_stencil_usage,
              vvl::Extension::_VK_EXT_host_query_reset,
          }}},
        {VK_API_VERSION_1_3,
         {"VK_VERSION_1_3",
          {
              vvl::Extension::_VK_KHR_dynamic_rendering,
              vvl::Extension::_VK_KHR_shader_terminate_invocation,
              vvl::Extension::_VK_KHR_shader_integer_dot_product,
              vvl::Extension::_VK_KHR_shader_non_semantic_info,
              vvl::Extension::_VK_KHR_synchronization2,
              vvl::Extension::_VK_KHR_zero_initialize_workgroup_memory,
              vvl::Extension::_VK_KHR_copy_commands2,
              vvl::Extension::_VK_KHR_format_feature_flags2,
              vvl::Extension::_VK_KHR_maintenance4,
              vvl::Extension::_VK_EXT_texture_compression_astc_hdr,
              vvl::Extension::_VK_EXT_inline_uniform_block,
              vvl::Extension::_VK_EXT_pipeline_creation_feedback,
              vvl::Extension::_VK_EXT_subgroup_size_control,
              vvl::Extension::_VK_EXT_tooling_info,
              vvl::Extension::_VK_EXT_extended_dynamic_state,
              vvl::Extension::_VK_EXT_shader_demote_to_helper_invocation,
              vvl::Extension::_VK_EXT_texel_buffer_alignment,
              vvl::Extension::_VK_EXT_private_data,
              vvl::Extension::_VK_EXT_pipeline_creation_cache_control,
              vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats,
              vvl::Extension::_VK_EXT_image_robustness,
              vvl::Extension::_VK_EXT_4444_formats,
              vvl::Extension::_VK_EXT_extended_dynamic_state2,
          }}},
        {VK_API_VERSION_1_4,
         {"VK_VERSION_1_4",
          {
              vvl::Extension::_VK_KHR_push_descriptor,
              vvl::Extension::_VK_KHR_global_priority,
              vvl::Extension::_VK_KHR_dynamic_rendering_local_read,
              vvl::Extension::_VK_KHR_map_memory2,
              vvl::Extension::_VK_KHR_shader_subgroup_rotate,
              vvl::Extension::_VK_KHR_maintenance5,
              vvl::Extension::_VK_KHR_vertex_attribute_divisor,
              vvl::Extension::_VK_KHR_load_store_op_none,
              vvl::Extension::_VK_KHR_shader_float_controls2,
              vvl::Extension::_VK_KHR_index_type_uint8,
              vvl::Extension::_VK_KHR_line_rasterization,
              vvl::Extension::_VK_KHR_shader_expect_assume,
              vvl::Extension::_VK_KHR_maintenance6,
              vvl::Extension::_VK_EXT_pipeline_robustness,
              vvl::Extension::_VK_EXT_host_image_copy,
              vvl::Extension::_VK_EXT_pipeline_protected_access,
          }}},

    };
    return promoted_map;
}

const InstanceExtensions::Info& GetInstanceVersionMap(const char* version) {
    static const InstanceExtensions::Info empty_info{nullptr, InstanceExtensions::RequirementVec()};
    static const vvl::unordered_map<std::string_view, InstanceExtensions::Info> version_map = {
        {"VK_VERSION_1_1", InstanceExtensions::Info(&InstanceExtensions::vk_feature_version_1_1, {})},
        {"VK_VERSION_1_2", InstanceExtensions::Info(&InstanceExtensions::vk_feature_version_1_2, {})},
        {"VK_VERSION_1_3", InstanceExtensions::Info(&InstanceExtensions::vk_feature_version_1_3, {})},
        {"VK_VERSION_1_4", InstanceExtensions::Info(&InstanceExtensions::vk_feature_version_1_4, {})},
    };
    const auto info = version_map.find(version);
    return (info != version_map.cend()) ? info->second : empty_info;
}

const DeviceExtensions::Info& GetDeviceVersionMap(const char* version) {
    static const DeviceExtensions::Info empty_info{nullptr, DeviceExtensions::RequirementVec()};
    static const vvl::unordered_map<std::string_view, DeviceExtensions::Info> version_map = {
        {"VK_VERSION_1_1", DeviceExtensions::Info(&DeviceExtensions::vk_feature_version_1_1, {})},
        {"VK_VERSION_1_2", DeviceExtensions::Info(&DeviceExtensions::vk_feature_version_1_2, {})},
        {"VK_VERSION_1_3", DeviceExtensions::Info(&DeviceExtensions::vk_feature_version_1_3, {})},
        {"VK_VERSION_1_4", DeviceExtensions::Info(&DeviceExtensions::vk_feature_version_1_4, {})},
    };
    const auto info = version_map.find(version);
    return (info != version_map.cend()) ? info->second : empty_info;
}

InstanceExtensions::InstanceExtensions(APIVersion requested_api_version, const VkInstanceCreateInfo* pCreateInfo) {
    // Initialize struct data, robust to invalid pCreateInfo
    api_version = NormalizeApiVersion(requested_api_version);
    if (!api_version.Valid()) return;

    const auto promotion_info_map = GetInstancePromotionInfoMap();
    for (const auto& version_it : promotion_info_map) {
        auto info = GetInstanceVersionMap(version_it.second.first);
        if (api_version >= version_it.first) {
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (const auto& extension : version_it.second.second) {
                info = GetInfo(extension);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
    }

    // CreateInfo takes precedence over promoted
    if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
            if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
            vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
            auto info = GetInfo(extension);
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
        }
    }
}

DeviceExtensions::DeviceExtensions(const InstanceExtensions& instance_ext, APIVersion requested_api_version,
                                   const VkDeviceCreateInfo* pCreateInfo)
    : InstanceExtensions(instance_ext) {
    auto api_version = NormalizeApiVersion(requested_api_version);
    if (!api_version.Valid()) return;

    const auto promotion_info_map = GetDevicePromotionInfoMap();
    for (const auto& version_it : promotion_info_map) {
        auto info = GetDeviceVersionMap(version_it.second.first);
        if (api_version >= version_it.first) {
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (const auto& extension : version_it.second.second) {
                info = GetInfo(extension);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
    }

    // CreateInfo takes precedence over promoted
    if (pCreateInfo && pCreateInfo->ppEnabledExtensionNames) {
        for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
            if (!pCreateInfo->ppEnabledExtensionNames[i]) continue;
            vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
            auto info = GetInfo(extension);
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
        }
    }

    // Workaround for functions being introduced by multiple extensions, until the layer is fixed to handle this correctly
    // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5579 and
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5600
    {
        constexpr std::array shader_object_interactions = {
            vvl::Extension::_VK_EXT_extended_dynamic_state,
            vvl::Extension::_VK_EXT_extended_dynamic_state2,
            vvl::Extension::_VK_EXT_extended_dynamic_state3,
            vvl::Extension::_VK_EXT_vertex_input_dynamic_state,
        };
        auto info = GetInfo(vvl::Extension::_VK_EXT_shader_object);
        if (info.state) {
            if (this->*(info.state) != kNotEnabled) {
                for (auto interaction_ext : shader_object_interactions) {
                    info = GetInfo(interaction_ext);
                    assert(info.state);
                    if (this->*(info.state) != kEnabledByCreateinfo) {
                        this->*(info.state) = kEnabledByInteraction;
                    }
                }
            }
        }
    }
}

DeviceExtensions::DeviceExtensions(const InstanceExtensions& instance_ext, APIVersion requested_api_version,
                                   const std::vector<VkExtensionProperties>& props)
    : InstanceExtensions(instance_ext) {
    auto api_version = NormalizeApiVersion(requested_api_version);
    if (!api_version.Valid()) return;

    const auto promotion_info_map = GetDevicePromotionInfoMap();
    for (const auto& version_it : promotion_info_map) {
        auto info = GetDeviceVersionMap(version_it.second.first);
        if (api_version >= version_it.first) {
            if (info.state) this->*(info.state) = kEnabledByCreateinfo;
            for (const auto& extension : version_it.second.second) {
                info = GetInfo(extension);
                assert(info.state);
                if (info.state) this->*(info.state) = kEnabledByApiLevel;
            }
        }
    }
    for (const auto& prop : props) {
        vvl::Extension extension = GetExtension(prop.extensionName);
        auto info = GetInfo(extension);
        if (info.state) this->*(info.state) = kEnabledByCreateinfo;
    }

    // Workaround for functions being introduced by multiple extensions, until the layer is fixed to handle this correctly
    // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5579 and
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5600
    {
        constexpr std::array shader_object_interactions = {
            vvl::Extension::_VK_EXT_extended_dynamic_state,
            vvl::Extension::_VK_EXT_extended_dynamic_state2,
            vvl::Extension::_VK_EXT_extended_dynamic_state3,
            vvl::Extension::_VK_EXT_vertex_input_dynamic_state,
        };
        auto info = GetInfo(vvl::Extension::_VK_EXT_shader_object);
        if (info.state) {
            if (this->*(info.state) != kNotEnabled) {
                for (auto interaction_ext : shader_object_interactions) {
                    info = GetInfo(interaction_ext);
                    assert(info.state);
                    if (this->*(info.state) != kEnabledByCreateinfo) {
                        this->*(info.state) = kEnabledByInteraction;
                    }
                }
            }
        }
    }
}

// NOLINTEND
