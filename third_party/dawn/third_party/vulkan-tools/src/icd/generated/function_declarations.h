/*
** Copyright (c) 2015-2025 The Khronos Group Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/*
** This header is generated from the Khronos Vulkan XML API Registry.
**
*/


#pragma once

#include <stdint.h>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace vkmock {

// Map of instance extension name to version
static const std::unordered_map<std::string, uint32_t> instance_extension_map = {
    {"VK_KHR_surface", 25},
    {"VK_KHR_display", 23},
    {"VK_KHR_xlib_surface", 6},
    {"VK_KHR_xcb_surface", 6},
    {"VK_KHR_wayland_surface", 6},
    {"VK_KHR_android_surface", 6},
    {"VK_KHR_win32_surface", 6},
    {"VK_EXT_debug_report", 10},
    {"VK_GGP_stream_descriptor_surface", 1},
    {"VK_NV_external_memory_capabilities", 1},
    {"VK_KHR_get_physical_device_properties2", 2},
    {"VK_EXT_validation_flags", 3},
    {"VK_NN_vi_surface", 1},
    {"VK_KHR_device_group_creation", 1},
    {"VK_KHR_external_memory_capabilities", 1},
    {"VK_KHR_external_semaphore_capabilities", 1},
    {"VK_EXT_direct_mode_display", 1},
    {"VK_EXT_acquire_xlib_display", 1},
    {"VK_EXT_display_surface_counter", 1},
    {"VK_EXT_swapchain_colorspace", 5},
    {"VK_KHR_external_fence_capabilities", 1},
    {"VK_KHR_get_surface_capabilities2", 1},
    {"VK_KHR_get_display_properties2", 1},
    {"VK_MVK_ios_surface", 3},
    {"VK_MVK_macos_surface", 3},
    {"VK_EXT_debug_utils", 2},
    {"VK_FUCHSIA_imagepipe_surface", 1},
    {"VK_EXT_metal_surface", 1},
    {"VK_KHR_surface_protected_capabilities", 1},
    {"VK_EXT_validation_features", 6},
    {"VK_EXT_headless_surface", 1},
    {"VK_EXT_surface_maintenance1", 1},
    {"VK_EXT_acquire_drm_display", 1},
    {"VK_EXT_directfb_surface", 1},
    {"VK_QNX_screen_surface", 1},
    {"VK_KHR_portability_enumeration", 1},
    {"VK_GOOGLE_surfaceless_query", 2},
    {"VK_LUNARG_direct_driver_loading", 1},
    {"VK_EXT_layer_settings", 2},
    {"VK_NV_display_stereo", 1},
};
// Map of device extension name to version
static const std::unordered_map<std::string, uint32_t> device_extension_map = {
    {"VK_KHR_swapchain", 70},
    {"VK_KHR_display_swapchain", 10},
    {"VK_NV_glsl_shader", 1},
    {"VK_EXT_depth_range_unrestricted", 1},
    {"VK_KHR_sampler_mirror_clamp_to_edge", 3},
    {"VK_IMG_filter_cubic", 1},
    {"VK_AMD_rasterization_order", 1},
    {"VK_AMD_shader_trinary_minmax", 1},
    {"VK_AMD_shader_explicit_vertex_parameter", 1},
    {"VK_EXT_debug_marker", 4},
    {"VK_KHR_video_queue", 8},
    {"VK_KHR_video_decode_queue", 8},
    {"VK_AMD_gcn_shader", 1},
    {"VK_NV_dedicated_allocation", 1},
    {"VK_EXT_transform_feedback", 1},
    {"VK_NVX_binary_import", 2},
    {"VK_NVX_image_view_handle", 3},
    {"VK_AMD_draw_indirect_count", 2},
    {"VK_AMD_negative_viewport_height", 1},
    {"VK_AMD_gpu_shader_half_float", 2},
    {"VK_AMD_shader_ballot", 1},
    {"VK_KHR_video_encode_h264", 14},
    {"VK_KHR_video_encode_h265", 14},
    {"VK_KHR_video_decode_h264", 9},
    {"VK_AMD_texture_gather_bias_lod", 1},
    {"VK_AMD_shader_info", 1},
    {"VK_KHR_dynamic_rendering", 1},
    {"VK_AMD_shader_image_load_store_lod", 1},
    {"VK_NV_corner_sampled_image", 2},
    {"VK_KHR_multiview", 1},
    {"VK_IMG_format_pvrtc", 1},
    {"VK_NV_external_memory", 1},
    {"VK_NV_external_memory_win32", 1},
    {"VK_NV_win32_keyed_mutex", 2},
    {"VK_KHR_device_group", 4},
    {"VK_KHR_shader_draw_parameters", 1},
    {"VK_EXT_shader_subgroup_ballot", 1},
    {"VK_EXT_shader_subgroup_vote", 1},
    {"VK_EXT_texture_compression_astc_hdr", 1},
    {"VK_EXT_astc_decode_mode", 1},
    {"VK_EXT_pipeline_robustness", 1},
    {"VK_KHR_maintenance1", 2},
    {"VK_KHR_external_memory", 1},
    {"VK_KHR_external_memory_win32", 1},
    {"VK_KHR_external_memory_fd", 1},
    {"VK_KHR_win32_keyed_mutex", 1},
    {"VK_KHR_external_semaphore", 1},
    {"VK_KHR_external_semaphore_win32", 1},
    {"VK_KHR_external_semaphore_fd", 1},
    {"VK_KHR_push_descriptor", 2},
    {"VK_EXT_conditional_rendering", 2},
    {"VK_KHR_shader_float16_int8", 1},
    {"VK_KHR_16bit_storage", 1},
    {"VK_KHR_incremental_present", 2},
    {"VK_KHR_descriptor_update_template", 1},
    {"VK_NV_clip_space_w_scaling", 1},
    {"VK_EXT_display_control", 1},
    {"VK_GOOGLE_display_timing", 1},
    {"VK_NV_sample_mask_override_coverage", 1},
    {"VK_NV_geometry_shader_passthrough", 1},
    {"VK_NV_viewport_array2", 1},
    {"VK_NVX_multiview_per_view_attributes", 1},
    {"VK_NV_viewport_swizzle", 1},
    {"VK_EXT_discard_rectangles", 2},
    {"VK_EXT_conservative_rasterization", 1},
    {"VK_EXT_depth_clip_enable", 1},
    {"VK_EXT_hdr_metadata", 3},
    {"VK_KHR_imageless_framebuffer", 1},
    {"VK_KHR_create_renderpass2", 1},
    {"VK_IMG_relaxed_line_rasterization", 1},
    {"VK_KHR_shared_presentable_image", 1},
    {"VK_KHR_external_fence", 1},
    {"VK_KHR_external_fence_win32", 1},
    {"VK_KHR_external_fence_fd", 1},
    {"VK_KHR_performance_query", 1},
    {"VK_KHR_maintenance2", 1},
    {"VK_KHR_variable_pointers", 1},
    {"VK_EXT_external_memory_dma_buf", 1},
    {"VK_EXT_queue_family_foreign", 1},
    {"VK_KHR_dedicated_allocation", 3},
    {"VK_ANDROID_external_memory_android_hardware_buffer", 5},
    {"VK_EXT_sampler_filter_minmax", 2},
    {"VK_KHR_storage_buffer_storage_class", 1},
    {"VK_AMD_gpu_shader_int16", 2},
    {"VK_AMDX_shader_enqueue", 2},
    {"VK_AMD_mixed_attachment_samples", 1},
    {"VK_AMD_shader_fragment_mask", 1},
    {"VK_EXT_inline_uniform_block", 1},
    {"VK_EXT_shader_stencil_export", 1},
    {"VK_EXT_sample_locations", 1},
    {"VK_KHR_relaxed_block_layout", 1},
    {"VK_KHR_get_memory_requirements2", 1},
    {"VK_KHR_image_format_list", 1},
    {"VK_EXT_blend_operation_advanced", 2},
    {"VK_NV_fragment_coverage_to_color", 1},
    {"VK_KHR_acceleration_structure", 13},
    {"VK_KHR_ray_tracing_pipeline", 1},
    {"VK_KHR_ray_query", 1},
    {"VK_NV_framebuffer_mixed_samples", 1},
    {"VK_NV_fill_rectangle", 1},
    {"VK_NV_shader_sm_builtins", 1},
    {"VK_EXT_post_depth_coverage", 1},
    {"VK_KHR_sampler_ycbcr_conversion", 14},
    {"VK_KHR_bind_memory2", 1},
    {"VK_EXT_image_drm_format_modifier", 2},
    {"VK_EXT_descriptor_indexing", 2},
    {"VK_EXT_shader_viewport_index_layer", 1},
    {"VK_NV_shading_rate_image", 3},
    {"VK_NV_ray_tracing", 3},
    {"VK_NV_representative_fragment_test", 2},
    {"VK_KHR_maintenance3", 1},
    {"VK_KHR_draw_indirect_count", 1},
    {"VK_EXT_filter_cubic", 3},
    {"VK_QCOM_render_pass_shader_resolve", 4},
    {"VK_EXT_global_priority", 2},
    {"VK_KHR_shader_subgroup_extended_types", 1},
    {"VK_KHR_8bit_storage", 1},
    {"VK_EXT_external_memory_host", 1},
    {"VK_AMD_buffer_marker", 1},
    {"VK_KHR_shader_atomic_int64", 1},
    {"VK_KHR_shader_clock", 1},
    {"VK_AMD_pipeline_compiler_control", 1},
    {"VK_EXT_calibrated_timestamps", 2},
    {"VK_AMD_shader_core_properties", 2},
    {"VK_KHR_video_decode_h265", 8},
    {"VK_KHR_global_priority", 1},
    {"VK_AMD_memory_overallocation_behavior", 1},
    {"VK_EXT_vertex_attribute_divisor", 3},
    {"VK_GGP_frame_token", 1},
    {"VK_EXT_pipeline_creation_feedback", 1},
    {"VK_KHR_driver_properties", 1},
    {"VK_KHR_shader_float_controls", 4},
    {"VK_NV_shader_subgroup_partitioned", 1},
    {"VK_KHR_depth_stencil_resolve", 1},
    {"VK_KHR_swapchain_mutable_format", 1},
    {"VK_NV_compute_shader_derivatives", 1},
    {"VK_NV_mesh_shader", 1},
    {"VK_NV_fragment_shader_barycentric", 1},
    {"VK_NV_shader_image_footprint", 2},
    {"VK_NV_scissor_exclusive", 2},
    {"VK_NV_device_diagnostic_checkpoints", 2},
    {"VK_KHR_timeline_semaphore", 2},
    {"VK_INTEL_shader_integer_functions2", 1},
    {"VK_INTEL_performance_query", 2},
    {"VK_KHR_vulkan_memory_model", 3},
    {"VK_EXT_pci_bus_info", 2},
    {"VK_AMD_display_native_hdr", 1},
    {"VK_KHR_shader_terminate_invocation", 1},
    {"VK_EXT_fragment_density_map", 2},
    {"VK_EXT_scalar_block_layout", 1},
    {"VK_GOOGLE_hlsl_functionality1", 1},
    {"VK_GOOGLE_decorate_string", 1},
    {"VK_EXT_subgroup_size_control", 2},
    {"VK_KHR_fragment_shading_rate", 2},
    {"VK_AMD_shader_core_properties2", 1},
    {"VK_AMD_device_coherent_memory", 1},
    {"VK_KHR_dynamic_rendering_local_read", 1},
    {"VK_EXT_shader_image_atomic_int64", 1},
    {"VK_KHR_shader_quad_control", 1},
    {"VK_KHR_spirv_1_4", 1},
    {"VK_EXT_memory_budget", 1},
    {"VK_EXT_memory_priority", 1},
    {"VK_NV_dedicated_allocation_image_aliasing", 1},
    {"VK_KHR_separate_depth_stencil_layouts", 1},
    {"VK_EXT_buffer_device_address", 2},
    {"VK_EXT_tooling_info", 1},
    {"VK_EXT_separate_stencil_usage", 1},
    {"VK_KHR_present_wait", 1},
    {"VK_NV_cooperative_matrix", 1},
    {"VK_NV_coverage_reduction_mode", 1},
    {"VK_EXT_fragment_shader_interlock", 1},
    {"VK_EXT_ycbcr_image_arrays", 1},
    {"VK_KHR_uniform_buffer_standard_layout", 1},
    {"VK_EXT_provoking_vertex", 1},
    {"VK_EXT_full_screen_exclusive", 4},
    {"VK_KHR_buffer_device_address", 1},
    {"VK_EXT_line_rasterization", 1},
    {"VK_EXT_shader_atomic_float", 1},
    {"VK_EXT_host_query_reset", 1},
    {"VK_EXT_index_type_uint8", 1},
    {"VK_EXT_extended_dynamic_state", 1},
    {"VK_KHR_deferred_host_operations", 4},
    {"VK_KHR_pipeline_executable_properties", 1},
    {"VK_EXT_host_image_copy", 1},
    {"VK_KHR_map_memory2", 1},
    {"VK_EXT_map_memory_placed", 1},
    {"VK_EXT_shader_atomic_float2", 1},
    {"VK_EXT_swapchain_maintenance1", 1},
    {"VK_EXT_shader_demote_to_helper_invocation", 1},
    {"VK_NV_device_generated_commands", 3},
    {"VK_NV_inherited_viewport_scissor", 1},
    {"VK_KHR_shader_integer_dot_product", 1},
    {"VK_EXT_texel_buffer_alignment", 1},
    {"VK_QCOM_render_pass_transform", 4},
    {"VK_EXT_depth_bias_control", 1},
    {"VK_EXT_device_memory_report", 2},
    {"VK_EXT_robustness2", 1},
    {"VK_EXT_custom_border_color", 12},
    {"VK_GOOGLE_user_type", 1},
    {"VK_KHR_pipeline_library", 1},
    {"VK_NV_present_barrier", 1},
    {"VK_KHR_shader_non_semantic_info", 1},
    {"VK_KHR_present_id", 1},
    {"VK_EXT_private_data", 1},
    {"VK_EXT_pipeline_creation_cache_control", 3},
    {"VK_KHR_video_encode_queue", 12},
    {"VK_NV_device_diagnostics_config", 2},
    {"VK_QCOM_render_pass_store_ops", 2},
    {"VK_NV_cuda_kernel_launch", 2},
    {"VK_NV_low_latency", 1},
    {"VK_EXT_metal_objects", 2},
    {"VK_KHR_synchronization2", 1},
    {"VK_EXT_descriptor_buffer", 1},
    {"VK_EXT_graphics_pipeline_library", 1},
    {"VK_AMD_shader_early_and_late_fragment_tests", 1},
    {"VK_KHR_fragment_shader_barycentric", 1},
    {"VK_KHR_shader_subgroup_uniform_control_flow", 1},
    {"VK_KHR_zero_initialize_workgroup_memory", 1},
    {"VK_NV_fragment_shading_rate_enums", 1},
    {"VK_NV_ray_tracing_motion_blur", 1},
    {"VK_EXT_mesh_shader", 1},
    {"VK_EXT_ycbcr_2plane_444_formats", 1},
    {"VK_EXT_fragment_density_map2", 1},
    {"VK_QCOM_rotated_copy_commands", 2},
    {"VK_EXT_image_robustness", 1},
    {"VK_KHR_workgroup_memory_explicit_layout", 1},
    {"VK_KHR_copy_commands2", 1},
    {"VK_EXT_image_compression_control", 1},
    {"VK_EXT_attachment_feedback_loop_layout", 2},
    {"VK_EXT_4444_formats", 1},
    {"VK_EXT_device_fault", 2},
    {"VK_ARM_rasterization_order_attachment_access", 1},
    {"VK_EXT_rgba10x6_formats", 1},
    {"VK_NV_acquire_winrt_display", 1},
    {"VK_VALVE_mutable_descriptor_type", 1},
    {"VK_EXT_vertex_input_dynamic_state", 2},
    {"VK_EXT_physical_device_drm", 1},
    {"VK_EXT_device_address_binding_report", 1},
    {"VK_EXT_depth_clip_control", 1},
    {"VK_EXT_primitive_topology_list_restart", 1},
    {"VK_KHR_format_feature_flags2", 2},
    {"VK_EXT_present_mode_fifo_latest_ready", 1},
    {"VK_FUCHSIA_external_memory", 1},
    {"VK_FUCHSIA_external_semaphore", 1},
    {"VK_FUCHSIA_buffer_collection", 2},
    {"VK_HUAWEI_subpass_shading", 3},
    {"VK_HUAWEI_invocation_mask", 1},
    {"VK_NV_external_memory_rdma", 1},
    {"VK_EXT_pipeline_properties", 1},
    {"VK_EXT_frame_boundary", 1},
    {"VK_EXT_multisampled_render_to_single_sampled", 1},
    {"VK_EXT_extended_dynamic_state2", 1},
    {"VK_EXT_color_write_enable", 1},
    {"VK_EXT_primitives_generated_query", 1},
    {"VK_KHR_ray_tracing_maintenance1", 1},
    {"VK_EXT_global_priority_query", 1},
    {"VK_EXT_image_view_min_lod", 1},
    {"VK_EXT_multi_draw", 1},
    {"VK_EXT_image_2d_view_of_3d", 1},
    {"VK_EXT_shader_tile_image", 1},
    {"VK_EXT_opacity_micromap", 2},
    {"VK_NV_displacement_micromap", 2},
    {"VK_EXT_load_store_op_none", 1},
    {"VK_HUAWEI_cluster_culling_shader", 3},
    {"VK_EXT_border_color_swizzle", 1},
    {"VK_EXT_pageable_device_local_memory", 1},
    {"VK_KHR_maintenance4", 2},
    {"VK_ARM_shader_core_properties", 1},
    {"VK_KHR_shader_subgroup_rotate", 2},
    {"VK_ARM_scheduling_controls", 1},
    {"VK_EXT_image_sliced_view_of_3d", 1},
    {"VK_VALVE_descriptor_set_host_mapping", 1},
    {"VK_EXT_depth_clamp_zero_one", 1},
    {"VK_EXT_non_seamless_cube_map", 1},
    {"VK_ARM_render_pass_striped", 1},
    {"VK_QCOM_fragment_density_map_offset", 2},
    {"VK_NV_copy_memory_indirect", 1},
    {"VK_NV_memory_decompression", 1},
    {"VK_NV_device_generated_commands_compute", 2},
    {"VK_NV_ray_tracing_linear_swept_spheres", 1},
    {"VK_NV_linear_color_attachment", 1},
    {"VK_KHR_shader_maximal_reconvergence", 1},
    {"VK_EXT_image_compression_control_swapchain", 1},
    {"VK_QCOM_image_processing", 1},
    {"VK_EXT_nested_command_buffer", 1},
    {"VK_EXT_external_memory_acquire_unmodified", 1},
    {"VK_EXT_extended_dynamic_state3", 2},
    {"VK_EXT_subpass_merge_feedback", 2},
    {"VK_EXT_shader_module_identifier", 1},
    {"VK_EXT_rasterization_order_attachment_access", 1},
    {"VK_NV_optical_flow", 1},
    {"VK_EXT_legacy_dithering", 2},
    {"VK_EXT_pipeline_protected_access", 1},
    {"VK_ANDROID_external_format_resolve", 1},
    {"VK_KHR_maintenance5", 1},
    {"VK_AMD_anti_lag", 1},
    {"VK_KHR_ray_tracing_position_fetch", 1},
    {"VK_EXT_shader_object", 1},
    {"VK_KHR_pipeline_binary", 1},
    {"VK_QCOM_tile_properties", 1},
    {"VK_SEC_amigo_profiling", 1},
    {"VK_QCOM_multiview_per_view_viewports", 1},
    {"VK_NV_ray_tracing_invocation_reorder", 1},
    {"VK_NV_cooperative_vector", 4},
    {"VK_NV_extended_sparse_address_space", 1},
    {"VK_EXT_mutable_descriptor_type", 1},
    {"VK_EXT_legacy_vertex_attributes", 1},
    {"VK_ARM_shader_core_builtins", 2},
    {"VK_EXT_pipeline_library_group_handles", 1},
    {"VK_EXT_dynamic_rendering_unused_attachments", 1},
    {"VK_NV_low_latency2", 2},
    {"VK_KHR_cooperative_matrix", 2},
    {"VK_QCOM_multiview_per_view_render_areas", 1},
    {"VK_KHR_compute_shader_derivatives", 1},
    {"VK_KHR_video_decode_av1", 1},
    {"VK_KHR_video_encode_av1", 1},
    {"VK_KHR_video_maintenance1", 1},
    {"VK_NV_per_stage_descriptor_set", 1},
    {"VK_QCOM_image_processing2", 1},
    {"VK_QCOM_filter_cubic_weights", 1},
    {"VK_QCOM_ycbcr_degamma", 1},
    {"VK_QCOM_filter_cubic_clamp", 1},
    {"VK_EXT_attachment_feedback_loop_dynamic_state", 1},
    {"VK_KHR_vertex_attribute_divisor", 1},
    {"VK_KHR_load_store_op_none", 1},
    {"VK_KHR_shader_float_controls2", 1},
    {"VK_QNX_external_memory_screen_buffer", 1},
    {"VK_MSFT_layered_driver", 1},
    {"VK_KHR_index_type_uint8", 1},
    {"VK_KHR_line_rasterization", 1},
    {"VK_KHR_calibrated_timestamps", 1},
    {"VK_KHR_shader_expect_assume", 1},
    {"VK_KHR_maintenance6", 1},
    {"VK_NV_descriptor_pool_overallocation", 1},
    {"VK_KHR_video_encode_quantization_map", 2},
    {"VK_NV_raw_access_chains", 1},
    {"VK_KHR_shader_relaxed_extended_instruction", 1},
    {"VK_NV_command_buffer_inheritance", 1},
    {"VK_KHR_maintenance7", 1},
    {"VK_NV_shader_atomic_float16_vector", 1},
    {"VK_EXT_shader_replicated_composites", 1},
    {"VK_NV_ray_tracing_validation", 1},
    {"VK_NV_cluster_acceleration_structure", 2},
    {"VK_NV_partitioned_acceleration_structure", 1},
    {"VK_EXT_device_generated_commands", 1},
    {"VK_KHR_maintenance8", 1},
    {"VK_MESA_image_alignment_control", 1},
    {"VK_EXT_depth_clamp_control", 1},
    {"VK_KHR_video_maintenance2", 1},
    {"VK_HUAWEI_hdr_vivid", 1},
    {"VK_NV_cooperative_matrix2", 1},
    {"VK_ARM_pipeline_opacity_micromap", 1},
    {"VK_EXT_external_memory_metal", 1},
    {"VK_KHR_depth_clamp_zero_one", 1},
    {"VK_EXT_vertex_attribute_robustness", 1},
};


static VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance);

static VKAPI_ATTR void VKAPI_CALL DestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties);

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName);

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(
    VkDevice                                    device,
    const char*                                 pName);

static VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice);

static VKAPI_ATTR void VKAPI_CALL DestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties);

static VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue);

static VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo*                         pSubmits,
    VkFence                                     fence);

static VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(
    VkQueue                                     queue);

static VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(
    VkDevice                                    device);

static VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory);

static VKAPI_ATTR void VKAPI_CALL FreeMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL MapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData);

static VKAPI_ATTR void VKAPI_CALL UnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory);

static VKAPI_ATTR VkResult VKAPI_CALL FlushMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges);

static VKAPI_ATTR VkResult VKAPI_CALL InvalidateMappedMemoryRanges(
    VkDevice                                    device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges);

static VKAPI_ATTR void VKAPI_CALL GetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes);

static VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset);

static VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory(
    VkDevice                                    device,
    VkImage                                     image,
    VkDeviceMemory                              memory,
    VkDeviceSize                                memoryOffset);

static VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements*            pSparseMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkSampleCountFlagBits                       samples,
    VkImageUsageFlags                           usage,
    VkImageTiling                               tiling,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties*              pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL QueueBindSparse(
    VkQueue                                     queue,
    uint32_t                                    bindInfoCount,
    const VkBindSparseInfo*                     pBindInfo,
    VkFence                                     fence);

static VKAPI_ATTR VkResult VKAPI_CALL CreateFence(
    VkDevice                                    device,
    const VkFenceCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence);

static VKAPI_ATTR void VKAPI_CALL DestroyFence(
    VkDevice                                    device,
    VkFence                                     fence,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL ResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences);

static VKAPI_ATTR VkResult VKAPI_CALL GetFenceStatus(
    VkDevice                                    device,
    VkFence                                     fence);

static VKAPI_ATTR VkResult VKAPI_CALL WaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout);

static VKAPI_ATTR VkResult VKAPI_CALL CreateSemaphore(
    VkDevice                                    device,
    const VkSemaphoreCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSemaphore*                                pSemaphore);

static VKAPI_ATTR void VKAPI_CALL DestroySemaphore(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateEvent(
    VkDevice                                    device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent);

static VKAPI_ATTR void VKAPI_CALL DestroyEvent(
    VkDevice                                    device,
    VkEvent                                     event,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event);

static VKAPI_ATTR VkResult VKAPI_CALL SetEvent(
    VkDevice                                    device,
    VkEvent                                     event);

static VKAPI_ATTR VkResult VKAPI_CALL ResetEvent(
    VkDevice                                    device,
    VkEvent                                     event);

static VKAPI_ATTR VkResult VKAPI_CALL CreateQueryPool(
    VkDevice                                    device,
    const VkQueryPoolCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkQueryPool*                                pQueryPool);

static VKAPI_ATTR void VKAPI_CALL DestroyQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetQueryPoolResults(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    size_t                                      dataSize,
    void*                                       pData,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags);

static VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer);

static VKAPI_ATTR void VKAPI_CALL DestroyBuffer(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateBufferView(
    VkDevice                                    device,
    const VkBufferViewCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferView*                               pView);

static VKAPI_ATTR void VKAPI_CALL DestroyBufferView(
    VkDevice                                    device,
    VkBufferView                                bufferView,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage);

static VKAPI_ATTR void VKAPI_CALL DestroyImage(
    VkDevice                                    device,
    VkImage                                     image,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource*                   pSubresource,
    VkSubresourceLayout*                        pLayout);

static VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView);

static VKAPI_ATTR void VKAPI_CALL DestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule);

static VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache);

static VKAPI_ATTR void VKAPI_CALL DestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    size_t*                                     pDataSize,
    void*                                       pData);

static VKAPI_ATTR VkResult VKAPI_CALL MergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             dstCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches);

static VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines);

static VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkComputePipelineCreateInfo*          pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines);

static VKAPI_ATTR void VKAPI_CALL DestroyPipeline(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout);

static VKAPI_ATTR void VKAPI_CALL DestroyPipelineLayout(
    VkDevice                                    device,
    VkPipelineLayout                            pipelineLayout,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateSampler(
    VkDevice                                    device,
    const VkSamplerCreateInfo*                  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSampler*                                  pSampler);

static VKAPI_ATTR void VKAPI_CALL DestroySampler(
    VkDevice                                    device,
    VkSampler                                   sampler,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout);

static VKAPI_ATTR void VKAPI_CALL DestroyDescriptorSetLayout(
    VkDevice                                    device,
    VkDescriptorSetLayout                       descriptorSetLayout,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool);

static VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags);

static VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets);

static VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets);

static VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies);

static VKAPI_ATTR VkResult VKAPI_CALL CreateFramebuffer(
    VkDevice                                    device,
    const VkFramebufferCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFramebuffer*                              pFramebuffer);

static VKAPI_ATTR void VKAPI_CALL DestroyFramebuffer(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass);

static VKAPI_ATTR void VKAPI_CALL DestroyRenderPass(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL GetRenderAreaGranularity(
    VkDevice                                    device,
    VkRenderPass                                renderPass,
    VkExtent2D*                                 pGranularity);

static VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool);

static VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags);

static VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers);

static VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers);

static VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo);

static VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(
    VkCommandBuffer                             commandBuffer);

static VKAPI_ATTR VkResult VKAPI_CALL ResetCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    VkCommandBufferResetFlags                   flags);

static VKAPI_ATTR void VKAPI_CALL CmdBindPipeline(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline);

static VKAPI_ATTR void VKAPI_CALL CmdSetViewport(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports);

static VKAPI_ATTR void VKAPI_CALL CmdSetScissor(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstScissor,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors);

static VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias(
    VkCommandBuffer                             commandBuffer,
    float                                       depthBiasConstantFactor,
    float                                       depthBiasClamp,
    float                                       depthBiasSlopeFactor);

static VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBounds(
    VkCommandBuffer                             commandBuffer,
    float                                       minDepthBounds,
    float                                       maxDepthBounds);

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilCompareMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    compareMask);

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilWriteMask(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    writeMask);

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilReference(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    uint32_t                                    reference);

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets);

static VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkIndexType                                 indexType);

static VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets);

static VKAPI_ATTR void VKAPI_CALL CmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance);

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexed(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    indexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstIndex,
    int32_t                                     vertexOffset,
    uint32_t                                    firstInstance);

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdDispatch(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ);

static VKAPI_ATTR void VKAPI_CALL CmdDispatchIndirect(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset);

static VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferCopy*                         pRegions);

static VKAPI_ATTR void VKAPI_CALL CmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions);

static VKAPI_ATTR void VKAPI_CALL CmdBlitImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageBlit*                          pRegions,
    VkFilter                                    filter);

static VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    srcBuffer,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions);

static VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkBuffer                                    dstBuffer,
    uint32_t                                    regionCount,
    const VkBufferImageCopy*                    pRegions);

static VKAPI_ATTR void VKAPI_CALL CmdUpdateBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                dataSize,
    const void*                                 pData);

static VKAPI_ATTR void VKAPI_CALL CmdFillBuffer(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                size,
    uint32_t                                    data);

static VKAPI_ATTR void VKAPI_CALL CmdClearColorImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearColorValue*                    pColor,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges);

static VKAPI_ATTR void VKAPI_CALL CmdClearDepthStencilImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     image,
    VkImageLayout                               imageLayout,
    const VkClearDepthStencilValue*             pDepthStencil,
    uint32_t                                    rangeCount,
    const VkImageSubresourceRange*              pRanges);

static VKAPI_ATTR void VKAPI_CALL CmdClearAttachments(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkClearAttachment*                    pAttachments,
    uint32_t                                    rectCount,
    const VkClearRect*                          pRects);

static VKAPI_ATTR void VKAPI_CALL CmdResolveImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageResolve*                       pRegions);

static VKAPI_ATTR void VKAPI_CALL CmdSetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask);

static VKAPI_ATTR void VKAPI_CALL CmdResetEvent(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags                        stageMask);

static VKAPI_ATTR void VKAPI_CALL CmdWaitEvents(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers);

static VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags                        srcStageMask,
    VkPipelineStageFlags                        dstStageMask,
    VkDependencyFlags                           dependencyFlags,
    uint32_t                                    memoryBarrierCount,
    const VkMemoryBarrier*                      pMemoryBarriers,
    uint32_t                                    bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    uint32_t                                    imageMemoryBarrierCount,
    const VkImageMemoryBarrier*                 pImageMemoryBarriers);

static VKAPI_ATTR void VKAPI_CALL CmdBeginQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags);

static VKAPI_ATTR void VKAPI_CALL CmdEndQuery(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query);

static VKAPI_ATTR void VKAPI_CALL CmdResetQueryPool(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount);

static VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query);

static VKAPI_ATTR void VKAPI_CALL CmdCopyQueryPoolResults(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    VkDeviceSize                                stride,
    VkQueryResultFlags                          flags);

static VKAPI_ATTR void VKAPI_CALL CmdPushConstants(
    VkCommandBuffer                             commandBuffer,
    VkPipelineLayout                            layout,
    VkShaderStageFlags                          stageFlags,
    uint32_t                                    offset,
    uint32_t                                    size,
    const void*                                 pValues);

static VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    VkSubpassContents                           contents);

static VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents);

static VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(
    VkCommandBuffer                             commandBuffer);

static VKAPI_ATTR void VKAPI_CALL CmdExecuteCommands(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    commandBufferCount,
    const VkCommandBuffer*                      pCommandBuffers);


static VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceVersion(
    uint32_t*                                   pApiVersion);

static VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos);

static VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos);

static VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeatures(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures);

static VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask);

static VKAPI_ATTR void VKAPI_CALL CmdDispatchBase(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ);

static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroups(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties);

static VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties);

static VKAPI_ATTR void VKAPI_CALL TrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags);

static VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue);

static VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversion(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion);

static VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversion(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate);

static VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFenceProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties);

static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupport(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport);


static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCount(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride);

static VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass);

static VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo);

static VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo);

static VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo);

static VKAPI_ATTR void VKAPI_CALL ResetQueryPool(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount);

static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreCounterValue(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue);

static VKAPI_ATTR VkResult VKAPI_CALL WaitSemaphores(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout);

static VKAPI_ATTR VkResult VKAPI_CALL SignalSemaphore(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo);

static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo);

static VKAPI_ATTR uint64_t VKAPI_CALL GetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo);

static VKAPI_ATTR uint64_t VKAPI_CALL GetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo);


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolProperties*             pToolProperties);

static VKAPI_ATTR VkResult VKAPI_CALL CreatePrivateDataSlot(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot);

static VKAPI_ATTR void VKAPI_CALL DestroyPrivateDataSlot(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL SetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data);

static VKAPI_ATTR void VKAPI_CALL GetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData);

static VKAPI_ATTR void VKAPI_CALL CmdSetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo);

static VKAPI_ATTR void VKAPI_CALL CmdResetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask);

static VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos);

static VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo);

static VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query);

static VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence);

static VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBlitImage2(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo);

static VKAPI_ATTR void VKAPI_CALL CmdResolveImage2(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBeginRendering(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo);

static VKAPI_ATTR void VKAPI_CALL CmdEndRendering(
    VkCommandBuffer                             commandBuffer);

static VKAPI_ATTR void VKAPI_CALL CmdSetCullMode(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode);

static VKAPI_ATTR void VKAPI_CALL CmdSetFrontFace(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace);

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopology(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology);

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports);

static VKAPI_ATTR void VKAPI_CALL CmdSetScissorWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors);

static VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOp(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilOp(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp);

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable);

static VKAPI_ATTR void VKAPI_CALL GetDeviceBufferMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageSparseMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements);


static VKAPI_ATTR void VKAPI_CALL CmdSetLineStipple(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern);

static VKAPI_ATTR VkResult VKAPI_CALL MapMemory2(
    VkDevice                                    device,
    const VkMemoryMapInfo*                      pMemoryMapInfo,
    void**                                      ppData);

static VKAPI_ATTR VkResult VKAPI_CALL UnmapMemory2(
    VkDevice                                    device,
    const VkMemoryUnmapInfo*                    pMemoryUnmapInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer2(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkIndexType                                 indexType);

static VKAPI_ATTR void VKAPI_CALL GetRenderingAreaGranularity(
    VkDevice                                    device,
    const VkRenderingAreaInfo*                  pRenderingAreaInfo,
    VkExtent2D*                                 pGranularity);

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageSubresourceLayout(
    VkDevice                                    device,
    const VkDeviceImageSubresourceInfo*         pInfo,
    VkSubresourceLayout2*                       pLayout);

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2*                  pSubresource,
    VkSubresourceLayout2*                       pLayout);

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSet(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites);

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplate(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData);

static VKAPI_ATTR void VKAPI_CALL CmdSetRenderingAttachmentLocations(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingAttachmentLocationInfo*    pLocationInfo);

static VKAPI_ATTR void VKAPI_CALL CmdSetRenderingInputAttachmentIndices(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInputAttachmentIndexInfo*  pInputAttachmentIndexInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets2(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorSetsInfo*             pBindDescriptorSetsInfo);

static VKAPI_ATTR void VKAPI_CALL CmdPushConstants2(
    VkCommandBuffer                             commandBuffer,
    const VkPushConstantsInfo*                  pPushConstantsInfo);

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSet2(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetInfo*              pPushDescriptorSetInfo);

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetWithTemplateInfo*  pPushDescriptorSetWithTemplateInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToImage(
    VkDevice                                    device,
    const VkCopyMemoryToImageInfo*              pCopyMemoryToImageInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyImageToMemory(
    VkDevice                                    device,
    const VkCopyImageToMemoryInfo*              pCopyImageToMemoryInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyImageToImage(
    VkDevice                                    device,
    const VkCopyImageToImageInfo*               pCopyImageToImageInfo);

static VKAPI_ATTR VkResult VKAPI_CALL TransitionImageLayout(
    VkDevice                                    device,
    uint32_t                                    transitionCount,
    const VkHostImageLayoutTransitionInfo*      pTransitions);


static VKAPI_ATTR void VKAPI_CALL DestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes);


static VKAPI_ATTR VkResult VKAPI_CALL CreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain);

static VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages);

static VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex);

static VKAPI_ATTR VkResult VKAPI_CALL QueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo);

static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupPresentCapabilitiesKHR(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHR*        pDeviceGroupPresentCapabilities);

static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModesKHR(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHR*           pModes);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects);

static VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImage2KHR(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHR*            pAcquireInfo,
    uint32_t*                                   pImageIndex);


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays);

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode);

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities);

static VKAPI_ATTR VkResult VKAPI_CALL CreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);


static VKAPI_ATTR VkResult VKAPI_CALL CreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains);

#ifdef VK_USE_PLATFORM_XLIB_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID);
#endif /* VK_USE_PLATFORM_XLIB_KHR */

#ifdef VK_USE_PLATFORM_XCB_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id);
#endif /* VK_USE_PLATFORM_XCB_KHR */

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display);
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */

#ifdef VK_USE_PLATFORM_ANDROID_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);
#endif /* VK_USE_PLATFORM_ANDROID_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL CreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex);
#endif /* VK_USE_PLATFORM_WIN32_KHR */



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkVideoProfileInfoKHR*                pVideoProfile,
    VkVideoCapabilitiesKHR*                     pCapabilities);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoFormatInfoKHR*   pVideoFormatInfo,
    uint32_t*                                   pVideoFormatPropertyCount,
    VkVideoFormatPropertiesKHR*                 pVideoFormatProperties);

static VKAPI_ATTR VkResult VKAPI_CALL CreateVideoSessionKHR(
    VkDevice                                    device,
    const VkVideoSessionCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionKHR*                          pVideoSession);

static VKAPI_ATTR void VKAPI_CALL DestroyVideoSessionKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetVideoSessionMemoryRequirementsKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t*                                   pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR*        pMemoryRequirements);

static VKAPI_ATTR VkResult VKAPI_CALL BindVideoSessionMemoryKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t                                    bindSessionMemoryInfoCount,
    const VkBindVideoSessionMemoryInfoKHR*      pBindSessionMemoryInfos);

static VKAPI_ATTR VkResult VKAPI_CALL CreateVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionParametersKHR*                pVideoSessionParameters);

static VKAPI_ATTR VkResult VKAPI_CALL UpdateVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo);

static VKAPI_ATTR void VKAPI_CALL DestroyVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL CmdBeginVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoBeginCodingInfoKHR*            pBeginInfo);

static VKAPI_ATTR void VKAPI_CALL CmdEndVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEndCodingInfoKHR*              pEndCodingInfo);

static VKAPI_ATTR void VKAPI_CALL CmdControlVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoCodingControlInfoKHR*          pCodingControlInfo);


static VKAPI_ATTR void VKAPI_CALL CmdDecodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoDecodeInfoKHR*                 pDecodeInfo);





static VKAPI_ATTR void VKAPI_CALL CmdBeginRenderingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo);

static VKAPI_ATTR void VKAPI_CALL CmdEndRenderingKHR(
    VkCommandBuffer                             commandBuffer);



static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2*             pProperties);


static VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures);

static VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask);

static VKAPI_ATTR void VKAPI_CALL CmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ);



static VKAPI_ATTR void VKAPI_CALL TrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags);


static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties);


static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties);


#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle);

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties);
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd);

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties);

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties*              pExternalSemaphoreProperties);


#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo);

static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle);
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo);

static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd);


static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites);

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData);





static VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate);

static VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData);



static VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass);

static VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo);

static VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo);

static VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo);


static VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain);


static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfo*    pExternalFenceInfo,
    VkExternalFenceProperties*                  pExternalFenceProperties);


#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL ImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo);

static VKAPI_ATTR VkResult VKAPI_CALL GetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle);
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR VkResult VKAPI_CALL ImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo);

static VKAPI_ATTR VkResult VKAPI_CALL GetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd);


static VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses);

static VKAPI_ATTR VkResult VKAPI_CALL AcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo);

static VKAPI_ATTR void VKAPI_CALL ReleaseProfilingLockKHR(
    VkDevice                                    device);



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats);



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayProperties2KHR*                    pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPlaneProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlaneProperties2KHR*               pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModeProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModeProperties2KHR*                pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkDisplayPlaneInfo2KHR*               pDisplayPlaneInfo,
    VkDisplayPlaneCapabilities2KHR*             pCapabilities);





static VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements);



static VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion);

static VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator);


static VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos);

static VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos);

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif /* VK_ENABLE_BETA_EXTENSIONS */


static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport);


static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride);












static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue);

static VKAPI_ATTR VkResult VKAPI_CALL WaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout);

static VKAPI_ATTR VkResult VKAPI_CALL SignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo);




static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR*     pFragmentShadingRates);

static VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateKHR(
    VkCommandBuffer                             commandBuffer,
    const VkExtent2D*                           pFragmentSize,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]);


static VKAPI_ATTR void VKAPI_CALL CmdSetRenderingAttachmentLocationsKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingAttachmentLocationInfo*    pLocationInfo);

static VKAPI_ATTR void VKAPI_CALL CmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInputAttachmentIndexInfo*  pInputAttachmentIndexInfo);






static VKAPI_ATTR VkResult VKAPI_CALL WaitForPresentKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    presentId,
    uint64_t                                    timeout);



static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo);

static VKAPI_ATTR uint64_t VKAPI_CALL GetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo);

static VKAPI_ATTR uint64_t VKAPI_CALL GetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo);


static VKAPI_ATTR VkResult VKAPI_CALL CreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation);

static VKAPI_ATTR void VKAPI_CALL DestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR uint32_t VKAPI_CALL GetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation);

static VKAPI_ATTR VkResult VKAPI_CALL GetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation);

static VKAPI_ATTR VkResult VKAPI_CALL DeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation);


static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics);

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations);


static VKAPI_ATTR VkResult VKAPI_CALL MapMemory2KHR(
    VkDevice                                    device,
    const VkMemoryMapInfo*                      pMemoryMapInfo,
    void**                                      ppData);

static VKAPI_ATTR VkResult VKAPI_CALL UnmapMemory2KHR(
    VkDevice                                    device,
    const VkMemoryUnmapInfo*                    pMemoryUnmapInfo);






static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR*     pQualityLevelProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetEncodedVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
    size_t*                                     pDataSize,
    void*                                       pData);

static VKAPI_ATTR void VKAPI_CALL CmdEncodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEncodeInfoKHR*                 pEncodeInfo);


static VKAPI_ATTR void VKAPI_CALL CmdSetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo);

static VKAPI_ATTR void VKAPI_CALL CmdResetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask);

static VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2KHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos);

static VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo);

static VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2KHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query);

static VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence);






static VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBlitImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo);

static VKAPI_ATTR void VKAPI_CALL CmdResolveImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo);



static VKAPI_ATTR void VKAPI_CALL CmdTraceRaysIndirect2KHR(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectDeviceAddress);



static VKAPI_ATTR void VKAPI_CALL GetDeviceBufferMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements);




static VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkIndexType                                 indexType);

static VKAPI_ATTR void VKAPI_CALL GetRenderingAreaGranularityKHR(
    VkDevice                                    device,
    const VkRenderingAreaInfo*                  pRenderingAreaInfo,
    VkExtent2D*                                 pGranularity);

static VKAPI_ATTR void VKAPI_CALL GetDeviceImageSubresourceLayoutKHR(
    VkDevice                                    device,
    const VkDeviceImageSubresourceInfo*         pInfo,
    VkSubresourceLayout2*                       pLayout);

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2KHR(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2*                  pSubresource,
    VkSubresourceLayout2*                       pLayout);



static VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineBinariesKHR(
    VkDevice                                    device,
    const VkPipelineBinaryCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineBinaryHandlesInfoKHR*             pBinaries);

static VKAPI_ATTR void VKAPI_CALL DestroyPipelineBinaryKHR(
    VkDevice                                    device,
    VkPipelineBinaryKHR                         pipelineBinary,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineKeyKHR(
    VkDevice                                    device,
    const VkPipelineCreateInfoKHR*              pPipelineCreateInfo,
    VkPipelineBinaryKeyKHR*                     pPipelineKey);

static VKAPI_ATTR VkResult VKAPI_CALL GetPipelineBinaryDataKHR(
    VkDevice                                    device,
    const VkPipelineBinaryDataInfoKHR*          pInfo,
    VkPipelineBinaryKeyKHR*                     pPipelineBinaryKey,
    size_t*                                     pPipelineBinaryDataSize,
    void*                                       pPipelineBinaryData);

static VKAPI_ATTR VkResult VKAPI_CALL ReleaseCapturedPipelineDataKHR(
    VkDevice                                    device,
    const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
    const VkAllocationCallbacks*                pAllocator);


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesKHR*           pProperties);










static VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern);


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainKHR*                            pTimeDomains);

static VKAPI_ATTR VkResult VKAPI_CALL GetCalibratedTimestampsKHR(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoKHR*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation);



static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorSetsInfo*             pBindDescriptorSetsInfo);

static VKAPI_ATTR void VKAPI_CALL CmdPushConstants2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushConstantsInfo*                  pPushConstantsInfo);

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSet2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetInfo*              pPushDescriptorSetInfo);

static VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetWithTemplateInfo*  pPushDescriptorSetWithTemplateInfo);

static VKAPI_ATTR void VKAPI_CALL CmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkSetDescriptorBufferOffsetsInfoEXT*  pSetDescriptorBufferOffsetsInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo);








static VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback);

static VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage);








static VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo);

static VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo);

static VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo);

static VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer);

static VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo);




static VKAPI_ATTR void VKAPI_CALL CmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes);

static VKAPI_ATTR void VKAPI_CALL CmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets);

static VKAPI_ATTR void VKAPI_CALL CmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets);

static VKAPI_ATTR void VKAPI_CALL CmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index);

static VKAPI_ATTR void VKAPI_CALL CmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index);

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride);


static VKAPI_ATTR VkResult VKAPI_CALL CreateCuModuleNVX(
    VkDevice                                    device,
    const VkCuModuleCreateInfoNVX*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuModuleNVX*                              pModule);

static VKAPI_ATTR VkResult VKAPI_CALL CreateCuFunctionNVX(
    VkDevice                                    device,
    const VkCuFunctionCreateInfoNVX*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuFunctionNVX*                            pFunction);

static VKAPI_ATTR void VKAPI_CALL DestroyCuModuleNVX(
    VkDevice                                    device,
    VkCuModuleNVX                               module,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL DestroyCuFunctionNVX(
    VkDevice                                    device,
    VkCuFunctionNVX                             function,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL CmdCuLaunchKernelNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCuLaunchInfoNVX*                    pLaunchInfo);


static VKAPI_ATTR uint32_t VKAPI_CALL GetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo);

static VKAPI_ATTR uint64_t VKAPI_CALL GetImageViewHandle64NVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo);

static VKAPI_ATTR VkResult VKAPI_CALL GetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties);


static VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride);






static VKAPI_ATTR VkResult VKAPI_CALL GetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo);


#ifdef VK_USE_PLATFORM_GGP

static VKAPI_ATTR VkResult VKAPI_CALL CreateStreamDescriptorSurfaceGGP(
    VkInstance                                  instance,
    const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);
#endif /* VK_USE_PLATFORM_GGP */




static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties);


#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle);
#endif /* VK_USE_PLATFORM_WIN32_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR
#endif /* VK_USE_PLATFORM_WIN32_KHR */


#ifdef VK_USE_PLATFORM_VI_NN

static VKAPI_ATTR VkResult VKAPI_CALL CreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);
#endif /* VK_USE_PLATFORM_VI_NN */







static VKAPI_ATTR void VKAPI_CALL CmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin);

static VKAPI_ATTR void VKAPI_CALL CmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer);


static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings);


static VKAPI_ATTR VkResult VKAPI_CALL ReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display);

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

static VKAPI_ATTR VkResult VKAPI_CALL AcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display);

static VKAPI_ATTR VkResult VKAPI_CALL GetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay);
#endif /* VK_USE_PLATFORM_XLIB_XRANDR_EXT */


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities);


static VKAPI_ATTR VkResult VKAPI_CALL DisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo);

static VKAPI_ATTR VkResult VKAPI_CALL RegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence);

static VKAPI_ATTR VkResult VKAPI_CALL RegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence);

static VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue);


static VKAPI_ATTR VkResult VKAPI_CALL GetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings);







static VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles);

static VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    discardRectangleEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkDiscardRectangleModeEXT                   discardRectangleMode);





static VKAPI_ATTR void VKAPI_CALL SetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata);


#ifdef VK_USE_PLATFORM_IOS_MVK

static VKAPI_ATTR VkResult VKAPI_CALL CreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);
#endif /* VK_USE_PLATFORM_IOS_MVK */

#ifdef VK_USE_PLATFORM_MACOS_MVK

static VKAPI_ATTR VkResult VKAPI_CALL CreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);
#endif /* VK_USE_PLATFORM_MACOS_MVK */




static VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo);

static VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo);

static VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo);

static VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue);

static VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo);

static VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer);

static VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger);

static VKAPI_ATTR void VKAPI_CALL DestroyDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessengerEXT                    messenger,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL SubmitDebugUtilsMessageEXT(
    VkInstance                                  instance,
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

#ifdef VK_USE_PLATFORM_ANDROID_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer);
#endif /* VK_USE_PLATFORM_ANDROID_KHR */



#ifdef VK_ENABLE_BETA_EXTENSIONS

static VKAPI_ATTR VkResult VKAPI_CALL CreateExecutionGraphPipelinesAMDX(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines);

static VKAPI_ATTR VkResult VKAPI_CALL GetExecutionGraphPipelineScratchSizeAMDX(
    VkDevice                                    device,
    VkPipeline                                  executionGraph,
    VkExecutionGraphPipelineScratchSizeAMDX*    pSizeInfo);

static VKAPI_ATTR VkResult VKAPI_CALL GetExecutionGraphPipelineNodeIndexAMDX(
    VkDevice                                    device,
    VkPipeline                                  executionGraph,
    const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
    uint32_t*                                   pNodeIndex);

static VKAPI_ATTR void VKAPI_CALL CmdInitializeGraphScratchMemoryAMDX(
    VkCommandBuffer                             commandBuffer,
    VkPipeline                                  executionGraph,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize);

static VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    const VkDispatchGraphCountInfoAMDX*         pCountInfo);

static VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphIndirectAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    const VkDispatchGraphCountInfoAMDX*         pCountInfo);

static VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphIndirectCountAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    VkDeviceAddress                             countInfo);
#endif /* VK_ENABLE_BETA_EXTENSIONS */






static VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo);

static VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties);








static VKAPI_ATTR VkResult VKAPI_CALL GetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties);


static VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache);

static VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches);

static VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData);




static VKAPI_ATTR void VKAPI_CALL CmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout);

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes);

static VKAPI_ATTR void VKAPI_CALL CmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders);


static VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure);

static VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements);

static VKAPI_ATTR VkResult VKAPI_CALL BindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos);

static VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset);

static VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkCopyAccelerationStructureModeKHR          mode);

static VKAPI_ATTR void VKAPI_CALL CmdTraceRaysNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    VkBuffer                                    callableShaderBindingTableBuffer,
    VkDeviceSize                                callableShaderBindingOffset,
    VkDeviceSize                                callableShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth);

static VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines);

static VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData);

static VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData);

static VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData);

static VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureNV*            pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery);

static VKAPI_ATTR VkResult VKAPI_CALL CompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader);






static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties);


static VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker);

static VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarker2AMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker);



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainKHR*                            pTimeDomains);

static VKAPI_ATTR VkResult VKAPI_CALL GetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoKHR*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation);




#ifdef VK_USE_PLATFORM_GGP
#endif /* VK_USE_PLATFORM_GGP */





static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask);

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride);




static VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorEnableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkBool32*                             pExclusiveScissorEnables);

static VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors);


static VKAPI_ATTR void VKAPI_CALL CmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker);

static VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData);

static VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointData2NV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointData2NV*                        pCheckpointData);



static VKAPI_ATTR VkResult VKAPI_CALL InitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo);

static VKAPI_ATTR void VKAPI_CALL UninitializePerformanceApiINTEL(
    VkDevice                                    device);

static VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo);

static VKAPI_ATTR VkResult VKAPI_CALL AcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration);

static VKAPI_ATTR VkResult VKAPI_CALL ReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration);

static VKAPI_ATTR VkResult VKAPI_CALL QueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration);

static VKAPI_ATTR VkResult VKAPI_CALL GetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue);



static VKAPI_ATTR void VKAPI_CALL SetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable);

#ifdef VK_USE_PLATFORM_FUCHSIA

static VKAPI_ATTR VkResult VKAPI_CALL CreateImagePipeSurfaceFUCHSIA(
    VkInstance                                  instance,
    const VkImagePipeSurfaceCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);
#endif /* VK_USE_PLATFORM_FUCHSIA */

#ifdef VK_USE_PLATFORM_METAL_EXT

static VKAPI_ATTR VkResult VKAPI_CALL CreateMetalSurfaceEXT(
    VkInstance                                  instance,
    const VkMetalSurfaceCreateInfoEXT*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);
#endif /* VK_USE_PLATFORM_METAL_EXT */













static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo);


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolPropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolProperties*             pToolProperties);




static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties);


static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations);




#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes);

static VKAPI_ATTR VkResult VKAPI_CALL AcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain);

static VKAPI_ATTR VkResult VKAPI_CALL ReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain);

static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModes2EXT(
    VkDevice                                    device,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkDeviceGroupPresentModeFlagsKHR*           pModes);
#endif /* VK_USE_PLATFORM_WIN32_KHR */


static VKAPI_ATTR VkResult VKAPI_CALL CreateHeadlessSurfaceEXT(
    VkInstance                                  instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);


static VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern);



static VKAPI_ATTR void VKAPI_CALL ResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount);



static VKAPI_ATTR void VKAPI_CALL CmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode);

static VKAPI_ATTR void VKAPI_CALL CmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace);

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology);

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports);

static VKAPI_ATTR void VKAPI_CALL CmdSetScissorWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors);

static VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers2EXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetStencilOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp);


static VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToImageEXT(
    VkDevice                                    device,
    const VkCopyMemoryToImageInfo*              pCopyMemoryToImageInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyImageToMemoryEXT(
    VkDevice                                    device,
    const VkCopyImageToMemoryInfo*              pCopyImageToMemoryInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyImageToImageEXT(
    VkDevice                                    device,
    const VkCopyImageToImageInfo*               pCopyImageToImageInfo);

static VKAPI_ATTR VkResult VKAPI_CALL TransitionImageLayoutEXT(
    VkDevice                                    device,
    uint32_t                                    transitionCount,
    const VkHostImageLayoutTransitionInfo*      pTransitions);

static VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2EXT(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2*                  pSubresource,
    VkSubresourceLayout2*                       pLayout);





static VKAPI_ATTR VkResult VKAPI_CALL ReleaseSwapchainImagesEXT(
    VkDevice                                    device,
    const VkReleaseSwapchainImagesInfoEXT*      pReleaseInfo);



static VKAPI_ATTR void VKAPI_CALL GetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL CmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo);

static VKAPI_ATTR void VKAPI_CALL CmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex);

static VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout);

static VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator);





static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkDepthBiasInfoEXT*                   pDepthBiasInfo);



static VKAPI_ATTR VkResult VKAPI_CALL AcquireDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    VkDisplayKHR                                display);

static VKAPI_ATTR VkResult VKAPI_CALL GetDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    uint32_t                                    connectorId,
    VkDisplayKHR*                               display);






static VKAPI_ATTR VkResult VKAPI_CALL CreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot);

static VKAPI_ATTR void VKAPI_CALL DestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL SetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data);

static VKAPI_ATTR void VKAPI_CALL GetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData);





static VKAPI_ATTR VkResult VKAPI_CALL CreateCudaModuleNV(
    VkDevice                                    device,
    const VkCudaModuleCreateInfoNV*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCudaModuleNV*                             pModule);

static VKAPI_ATTR VkResult VKAPI_CALL GetCudaModuleCacheNV(
    VkDevice                                    device,
    VkCudaModuleNV                              module,
    size_t*                                     pCacheSize,
    void*                                       pCacheData);

static VKAPI_ATTR VkResult VKAPI_CALL CreateCudaFunctionNV(
    VkDevice                                    device,
    const VkCudaFunctionCreateInfoNV*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCudaFunctionNV*                           pFunction);

static VKAPI_ATTR void VKAPI_CALL DestroyCudaModuleNV(
    VkDevice                                    device,
    VkCudaModuleNV                              module,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL DestroyCudaFunctionNV(
    VkDevice                                    device,
    VkCudaFunctionNV                            function,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL CmdCudaLaunchKernelNV(
    VkCommandBuffer                             commandBuffer,
    const VkCudaLaunchInfoNV*                   pLaunchInfo);


#ifdef VK_USE_PLATFORM_METAL_EXT

static VKAPI_ATTR void VKAPI_CALL ExportMetalObjectsEXT(
    VkDevice                                    device,
    VkExportMetalObjectsInfoEXT*                pMetalObjectsInfo);
#endif /* VK_USE_PLATFORM_METAL_EXT */


static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSizeEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    VkDeviceSize*                               pLayoutSizeInBytes);

static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutBindingOffsetEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    uint32_t                                    binding,
    VkDeviceSize*                               pOffset);

static VKAPI_ATTR void VKAPI_CALL GetDescriptorEXT(
    VkDevice                                    device,
    const VkDescriptorGetInfoEXT*               pDescriptorInfo,
    size_t                                      dataSize,
    void*                                       pDescriptor);

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    bufferCount,
    const VkDescriptorBufferBindingInfoEXT*     pBindingInfos);

static VKAPI_ATTR void VKAPI_CALL CmdSetDescriptorBufferOffsetsEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    setCount,
    const uint32_t*                             pBufferIndices,
    const VkDeviceSize*                         pOffsets);

static VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBufferEmbeddedSamplersEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set);

static VKAPI_ATTR VkResult VKAPI_CALL GetBufferOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData);

static VKAPI_ATTR VkResult VKAPI_CALL GetImageOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageCaptureDescriptorDataInfoEXT*  pInfo,
    void*                                       pData);

static VKAPI_ATTR VkResult VKAPI_CALL GetImageViewOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData);

static VKAPI_ATTR VkResult VKAPI_CALL GetSamplerOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData);

static VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData);




static VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateEnumNV(
    VkCommandBuffer                             commandBuffer,
    VkFragmentShadingRateNV                     shadingRate,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]);










static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceFaultInfoEXT(
    VkDevice                                    device,
    VkDeviceFaultCountsEXT*                     pFaultCounts,
    VkDeviceFaultInfoEXT*                       pFaultInfo);



#ifdef VK_USE_PLATFORM_WIN32_KHR

static VKAPI_ATTR VkResult VKAPI_CALL AcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display);

static VKAPI_ATTR VkResult VKAPI_CALL GetWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    deviceRelativeId,
    VkDisplayKHR*                               pDisplay);
#endif /* VK_USE_PLATFORM_WIN32_KHR */

#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

static VKAPI_ATTR VkResult VKAPI_CALL CreateDirectFBSurfaceEXT(
    VkInstance                                  instance,
    const VkDirectFBSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceDirectFBPresentationSupportEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    IDirectFB*                                  dfb);
#endif /* VK_USE_PLATFORM_DIRECTFB_EXT */



static VKAPI_ATTR void VKAPI_CALL CmdSetVertexInputEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT*  pVertexBindingDescriptions,
    uint32_t                                    vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions);






#ifdef VK_USE_PLATFORM_FUCHSIA

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkMemoryGetZirconHandleInfoFUCHSIA*   pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle);

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    zx_handle_t                                 zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA*      pMemoryZirconHandleProperties);
#endif /* VK_USE_PLATFORM_FUCHSIA */

#ifdef VK_USE_PLATFORM_FUCHSIA

static VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo);

static VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle);
#endif /* VK_USE_PLATFORM_FUCHSIA */

#ifdef VK_USE_PLATFORM_FUCHSIA

static VKAPI_ATTR VkResult VKAPI_CALL CreateBufferCollectionFUCHSIA(
    VkDevice                                    device,
    const VkBufferCollectionCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferCollectionFUCHSIA*                  pCollection);

static VKAPI_ATTR VkResult VKAPI_CALL SetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkImageConstraintsInfoFUCHSIA*        pImageConstraintsInfo);

static VKAPI_ATTR VkResult VKAPI_CALL SetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkBufferConstraintsInfoFUCHSIA*       pBufferConstraintsInfo);

static VKAPI_ATTR void VKAPI_CALL DestroyBufferCollectionFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetBufferCollectionPropertiesFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    VkBufferCollectionPropertiesFUCHSIA*        pProperties);
#endif /* VK_USE_PLATFORM_FUCHSIA */


static VKAPI_ATTR VkResult VKAPI_CALL GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(
    VkDevice                                    device,
    VkRenderPass                                renderpass,
    VkExtent2D*                                 pMaxWorkgroupSize);

static VKAPI_ATTR void VKAPI_CALL CmdSubpassShadingHUAWEI(
    VkCommandBuffer                             commandBuffer);


static VKAPI_ATTR void VKAPI_CALL CmdBindInvocationMaskHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout);


static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryRemoteAddressNV(
    VkDevice                                    device,
    const VkMemoryGetRemoteAddressInfoNV*       pMemoryGetRemoteAddressInfo,
    VkRemoteAddressNV*                          pAddress);


static VKAPI_ATTR VkResult VKAPI_CALL GetPipelinePropertiesEXT(
    VkDevice                                    device,
    const VkPipelineInfoEXT*                    pPipelineInfo,
    VkBaseOutStructure*                         pPipelineProperties);




static VKAPI_ATTR void VKAPI_CALL CmdSetPatchControlPointsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    patchControlPoints);

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkLogicOp                                   logicOp);

static VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable);

#ifdef VK_USE_PLATFORM_SCREEN_QNX

static VKAPI_ATTR VkResult VKAPI_CALL CreateScreenSurfaceQNX(
    VkInstance                                  instance,
    const VkScreenSurfaceCreateInfoQNX*         pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);

static VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceScreenPresentationSupportQNX(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct _screen_window*                      window);
#endif /* VK_USE_PLATFORM_SCREEN_QNX */


static VKAPI_ATTR void                                    VKAPI_CALL CmdSetColorWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorWriteEnables);





static VKAPI_ATTR void VKAPI_CALL CmdDrawMultiEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawInfoEXT*                   pVertexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdDrawMultiIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawIndexedInfoEXT*            pIndexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride,
    const int32_t*                              pVertexOffset);




static VKAPI_ATTR VkResult VKAPI_CALL CreateMicromapEXT(
    VkDevice                                    device,
    const VkMicromapCreateInfoEXT*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkMicromapEXT*                              pMicromap);

static VKAPI_ATTR void VKAPI_CALL DestroyMicromapEXT(
    VkDevice                                    device,
    VkMicromapEXT                               micromap,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL CmdBuildMicromapsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos);

static VKAPI_ATTR VkResult VKAPI_CALL BuildMicromapsEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos);

static VKAPI_ATTR VkResult VKAPI_CALL CopyMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapInfoEXT*                pInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyMicromapToMemoryEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo);

static VKAPI_ATTR VkResult VKAPI_CALL WriteMicromapsPropertiesEXT(
    VkDevice                                    device,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride);

static VKAPI_ATTR void VKAPI_CALL CmdCopyMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapInfoEXT*                pInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyMicromapToMemoryEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo);

static VKAPI_ATTR void VKAPI_CALL CmdWriteMicromapsPropertiesEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery);

static VKAPI_ATTR void VKAPI_CALL GetDeviceMicromapCompatibilityEXT(
    VkDevice                                    device,
    const VkMicromapVersionInfoEXT*             pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility);

static VKAPI_ATTR void VKAPI_CALL GetMicromapBuildSizesEXT(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkMicromapBuildInfoEXT*               pBuildInfo,
    VkMicromapBuildSizesInfoEXT*                pSizeInfo);

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif /* VK_ENABLE_BETA_EXTENSIONS */



static VKAPI_ATTR void VKAPI_CALL CmdDrawClusterHUAWEI(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ);

static VKAPI_ATTR void VKAPI_CALL CmdDrawClusterIndirectHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset);



static VKAPI_ATTR void VKAPI_CALL SetDeviceMemoryPriorityEXT(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    float                                       priority);





static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice                                    device,
    const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE*  pHostMapping);

static VKAPI_ATTR void VKAPI_CALL GetDescriptorSetHostMappingVALVE(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    void**                                      ppData);






static VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToImageIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    const VkImageSubresourceLayers*             pImageSubresources);


static VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    decompressRegionCount,
    const VkDecompressMemoryRegionNV*           pDecompressMemoryRegions);

static VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectCommandsAddress,
    VkDeviceAddress                             indirectCommandsCountAddress,
    uint32_t                                    stride);


static VKAPI_ATTR void VKAPI_CALL GetPipelineIndirectMemoryRequirementsNV(
    VkDevice                                    device,
    const VkComputePipelineCreateInfo*          pCreateInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL CmdUpdatePipelineIndirectBufferNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline);

static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetPipelineIndirectDeviceAddressNV(
    VkDevice                                    device,
    const VkPipelineIndirectDeviceAddressInfoNV* pInfo);









static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClampEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClampEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetPolygonModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkPolygonMode                               polygonMode);

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationSamplesEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       rasterizationSamples);

static VKAPI_ATTR void VKAPI_CALL CmdSetSampleMaskEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       samples,
    const VkSampleMask*                         pSampleMask);

static VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToCoverageEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToCoverageEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToOneEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToOneEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    logicOpEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorBlendEnables);

static VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEquationEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendEquationEXT*              pColorBlendEquations);

static VKAPI_ATTR void VKAPI_CALL CmdSetColorWriteMaskEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorComponentFlags*                pColorWriteMasks);

static VKAPI_ATTR void VKAPI_CALL CmdSetTessellationDomainOriginEXT(
    VkCommandBuffer                             commandBuffer,
    VkTessellationDomainOrigin                  domainOrigin);

static VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationStreamEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    rasterizationStream);

static VKAPI_ATTR void VKAPI_CALL CmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkConservativeRasterizationModeEXT          conservativeRasterizationMode);

static VKAPI_ATTR void VKAPI_CALL CmdSetExtraPrimitiveOverestimationSizeEXT(
    VkCommandBuffer                             commandBuffer,
    float                                       extraPrimitiveOverestimationSize);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClipEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    sampleLocationsEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendAdvancedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendAdvancedEXT*              pColorBlendAdvanced);

static VKAPI_ATTR void VKAPI_CALL CmdSetProvokingVertexModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkProvokingVertexModeEXT                    provokingVertexMode);

static VKAPI_ATTR void VKAPI_CALL CmdSetLineRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkLineRasterizationModeEXT                  lineRasterizationMode);

static VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stippledLineEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipNegativeOneToOneEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    negativeOneToOne);

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    viewportWScalingEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetViewportSwizzleNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportSwizzleNV*                  pViewportSwizzles);

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageToColorEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorLocationNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageToColorLocation);

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageModulationModeNV                  coverageModulationMode);

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageModulationTableEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageModulationTableCount,
    const float*                                pCoverageModulationTable);

static VKAPI_ATTR void VKAPI_CALL CmdSetShadingRateImageEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    shadingRateImageEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetRepresentativeFragmentTestEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    representativeFragmentTestEnable);

static VKAPI_ATTR void VKAPI_CALL CmdSetCoverageReductionModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageReductionModeNV                   coverageReductionMode);




static VKAPI_ATTR void VKAPI_CALL GetShaderModuleIdentifierEXT(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    VkShaderModuleIdentifierEXT*                pIdentifier);

static VKAPI_ATTR void VKAPI_CALL GetShaderModuleCreateInfoIdentifierEXT(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    VkShaderModuleIdentifierEXT*                pIdentifier);



static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice                            physicalDevice,
    const VkOpticalFlowImageFormatInfoNV*       pOpticalFlowImageFormatInfo,
    uint32_t*                                   pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV*       pImageFormatProperties);

static VKAPI_ATTR VkResult VKAPI_CALL CreateOpticalFlowSessionNV(
    VkDevice                                    device,
    const VkOpticalFlowSessionCreateInfoNV*     pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkOpticalFlowSessionNV*                     pSession);

static VKAPI_ATTR void VKAPI_CALL DestroyOpticalFlowSessionNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL BindOpticalFlowSessionImageNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    VkOpticalFlowSessionBindingPointNV          bindingPoint,
    VkImageView                                 view,
    VkImageLayout                               layout);

static VKAPI_ATTR void VKAPI_CALL CmdOpticalFlowExecuteNV(
    VkCommandBuffer                             commandBuffer,
    VkOpticalFlowSessionNV                      session,
    const VkOpticalFlowExecuteInfoNV*           pExecuteInfo);



#ifdef VK_USE_PLATFORM_ANDROID_KHR
#endif /* VK_USE_PLATFORM_ANDROID_KHR */


static VKAPI_ATTR void VKAPI_CALL AntiLagUpdateAMD(
    VkDevice                                    device,
    const VkAntiLagDataAMD*                     pData);


static VKAPI_ATTR VkResult VKAPI_CALL CreateShadersEXT(
    VkDevice                                    device,
    uint32_t                                    createInfoCount,
    const VkShaderCreateInfoEXT*                pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderEXT*                                pShaders);

static VKAPI_ATTR void VKAPI_CALL DestroyShaderEXT(
    VkDevice                                    device,
    VkShaderEXT                                 shader,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL GetShaderBinaryDataEXT(
    VkDevice                                    device,
    VkShaderEXT                                 shader,
    size_t*                                     pDataSize,
    void*                                       pData);

static VKAPI_ATTR void VKAPI_CALL CmdBindShadersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    stageCount,
    const VkShaderStageFlagBits*                pStages,
    const VkShaderEXT*                          pShaders);

static VKAPI_ATTR void VKAPI_CALL CmdSetDepthClampRangeEXT(
    VkCommandBuffer                             commandBuffer,
    VkDepthClampModeEXT                         depthClampMode,
    const VkDepthClampRangeEXT*                 pDepthClampRange);


static VKAPI_ATTR VkResult VKAPI_CALL GetFramebufferTilePropertiesQCOM(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    uint32_t*                                   pPropertiesCount,
    VkTilePropertiesQCOM*                       pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL GetDynamicRenderingTilePropertiesQCOM(
    VkDevice                                    device,
    const VkRenderingInfo*                      pRenderingInfo,
    VkTilePropertiesQCOM*                       pProperties);





static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeVectorPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeVectorPropertiesNV*            pProperties);

static VKAPI_ATTR VkResult VKAPI_CALL ConvertCooperativeVectorMatrixNV(
    VkDevice                                    device,
    const VkConvertCooperativeVectorMatrixInfoNV* pInfo);

static VKAPI_ATTR void VKAPI_CALL CmdConvertCooperativeVectorMatrixNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkConvertCooperativeVectorMatrixInfoNV* pInfos);









static VKAPI_ATTR VkResult VKAPI_CALL SetLatencySleepModeNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkLatencySleepModeInfoNV*             pSleepModeInfo);

static VKAPI_ATTR VkResult VKAPI_CALL LatencySleepNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkLatencySleepInfoNV*                 pSleepInfo);

static VKAPI_ATTR void VKAPI_CALL SetLatencyMarkerNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkSetLatencyMarkerInfoNV*             pLatencyMarkerInfo);

static VKAPI_ATTR void VKAPI_CALL GetLatencyTimingsNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkGetLatencyMarkerInfoNV*                   pLatencyMarkerInfo);

static VKAPI_ATTR void VKAPI_CALL QueueNotifyOutOfBandNV(
    VkQueue                                     queue,
    const VkOutOfBandQueueTypeInfoNV*           pQueueTypeInfo);








static VKAPI_ATTR void VKAPI_CALL CmdSetAttachmentFeedbackLoopEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkImageAspectFlags                          aspectMask);

#ifdef VK_USE_PLATFORM_SCREEN_QNX

static VKAPI_ATTR VkResult VKAPI_CALL GetScreenBufferPropertiesQNX(
    VkDevice                                    device,
    const struct _screen_buffer*                buffer,
    VkScreenBufferPropertiesQNX*                pProperties);
#endif /* VK_USE_PLATFORM_SCREEN_QNX */










static VKAPI_ATTR void VKAPI_CALL GetClusterAccelerationStructureBuildSizesNV(
    VkDevice                                    device,
    const VkClusterAccelerationStructureInputInfoNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer                             commandBuffer,
    const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos);


static VKAPI_ATTR void VKAPI_CALL GetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice                                    device,
    const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo);

static VKAPI_ATTR void VKAPI_CALL CmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer                             commandBuffer,
    const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo);


static VKAPI_ATTR void VKAPI_CALL GetGeneratedCommandsMemoryRequirementsEXT(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements);

static VKAPI_ATTR void VKAPI_CALL CmdPreprocessGeneratedCommandsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoEXT*           pGeneratedCommandsInfo,
    VkCommandBuffer                             stateCommandBuffer);

static VKAPI_ATTR void VKAPI_CALL CmdExecuteGeneratedCommandsEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoEXT*           pGeneratedCommandsInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutEXT(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutEXT*                pIndirectCommandsLayout);

static VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutEXT(
    VkDevice                                    device,
    VkIndirectCommandsLayoutEXT                 indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectExecutionSetEXT(
    VkDevice                                    device,
    const VkIndirectExecutionSetCreateInfoEXT*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectExecutionSetEXT*                  pIndirectExecutionSet);

static VKAPI_ATTR void VKAPI_CALL DestroyIndirectExecutionSetEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL UpdateIndirectExecutionSetPipelineEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    uint32_t                                    executionSetWriteCount,
    const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites);

static VKAPI_ATTR void VKAPI_CALL UpdateIndirectExecutionSetShaderEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    uint32_t                                    executionSetWriteCount,
    const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites);





static VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties);


#ifdef VK_USE_PLATFORM_METAL_EXT

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryMetalHandleEXT(
    VkDevice                                    device,
    const VkMemoryGetMetalHandleInfoEXT*        pGetMetalHandleInfo,
    void**                                      pHandle);

static VKAPI_ATTR VkResult VKAPI_CALL GetMemoryMetalHandlePropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHandle,
    VkMemoryMetalHandlePropertiesEXT*           pMemoryMetalHandleProperties);
#endif /* VK_USE_PLATFORM_METAL_EXT */



static VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure);

static VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator);

static VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructuresKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);

static VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress*                      pIndirectDeviceAddresses,
    const uint32_t*                             pIndirectStrides,
    const uint32_t* const*                      ppMaxPrimitiveCounts);

static VKAPI_ATTR VkResult VKAPI_CALL BuildAccelerationStructuresKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);

static VKAPI_ATTR VkResult VKAPI_CALL CopyAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo);

static VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo);

static VKAPI_ATTR VkResult VKAPI_CALL WriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride);

static VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo);

static VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo);

static VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo);

static VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery);

static VKAPI_ATTR void VKAPI_CALL GetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility);

static VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureBuildSizesKHR(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t*                             pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo);


static VKAPI_ATTR void VKAPI_CALL CmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth);

static VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines);

static VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData);

static VKAPI_ATTR void VKAPI_CALL CmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    VkDeviceAddress                             indirectDeviceAddress);

static VKAPI_ATTR VkDeviceSize VKAPI_CALL GetRayTracingShaderGroupStackSizeKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    group,
    VkShaderGroupShaderKHR                      groupShader);

static VKAPI_ATTR void VKAPI_CALL CmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize);



static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ);

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride);

static VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride);

// Map of all APIs to be intercepted by this layer
static const std::unordered_map<std::string, void*> name_to_funcptr_map = {
    {"vkCreateInstance", (void*)CreateInstance},
    {"vkDestroyInstance", (void*)DestroyInstance},
    {"vkEnumeratePhysicalDevices", (void*)EnumeratePhysicalDevices},
    {"vkGetPhysicalDeviceFeatures", (void*)GetPhysicalDeviceFeatures},
    {"vkGetPhysicalDeviceFormatProperties", (void*)GetPhysicalDeviceFormatProperties},
    {"vkGetPhysicalDeviceImageFormatProperties", (void*)GetPhysicalDeviceImageFormatProperties},
    {"vkGetPhysicalDeviceProperties", (void*)GetPhysicalDeviceProperties},
    {"vkGetPhysicalDeviceQueueFamilyProperties", (void*)GetPhysicalDeviceQueueFamilyProperties},
    {"vkGetPhysicalDeviceMemoryProperties", (void*)GetPhysicalDeviceMemoryProperties},
    {"vkGetInstanceProcAddr", (void*)GetInstanceProcAddr},
    {"vkGetDeviceProcAddr", (void*)GetDeviceProcAddr},
    {"vkCreateDevice", (void*)CreateDevice},
    {"vkDestroyDevice", (void*)DestroyDevice},
    {"vkEnumerateInstanceExtensionProperties", (void*)EnumerateInstanceExtensionProperties},
    {"vkEnumerateDeviceExtensionProperties", (void*)EnumerateDeviceExtensionProperties},
    {"vkEnumerateInstanceLayerProperties", (void*)EnumerateInstanceLayerProperties},
    {"vkEnumerateDeviceLayerProperties", (void*)EnumerateDeviceLayerProperties},
    {"vkGetDeviceQueue", (void*)GetDeviceQueue},
    {"vkQueueSubmit", (void*)QueueSubmit},
    {"vkQueueWaitIdle", (void*)QueueWaitIdle},
    {"vkDeviceWaitIdle", (void*)DeviceWaitIdle},
    {"vkAllocateMemory", (void*)AllocateMemory},
    {"vkFreeMemory", (void*)FreeMemory},
    {"vkMapMemory", (void*)MapMemory},
    {"vkUnmapMemory", (void*)UnmapMemory},
    {"vkFlushMappedMemoryRanges", (void*)FlushMappedMemoryRanges},
    {"vkInvalidateMappedMemoryRanges", (void*)InvalidateMappedMemoryRanges},
    {"vkGetDeviceMemoryCommitment", (void*)GetDeviceMemoryCommitment},
    {"vkBindBufferMemory", (void*)BindBufferMemory},
    {"vkBindImageMemory", (void*)BindImageMemory},
    {"vkGetBufferMemoryRequirements", (void*)GetBufferMemoryRequirements},
    {"vkGetImageMemoryRequirements", (void*)GetImageMemoryRequirements},
    {"vkGetImageSparseMemoryRequirements", (void*)GetImageSparseMemoryRequirements},
    {"vkGetPhysicalDeviceSparseImageFormatProperties", (void*)GetPhysicalDeviceSparseImageFormatProperties},
    {"vkQueueBindSparse", (void*)QueueBindSparse},
    {"vkCreateFence", (void*)CreateFence},
    {"vkDestroyFence", (void*)DestroyFence},
    {"vkResetFences", (void*)ResetFences},
    {"vkGetFenceStatus", (void*)GetFenceStatus},
    {"vkWaitForFences", (void*)WaitForFences},
    {"vkCreateSemaphore", (void*)CreateSemaphore},
    {"vkDestroySemaphore", (void*)DestroySemaphore},
    {"vkCreateEvent", (void*)CreateEvent},
    {"vkDestroyEvent", (void*)DestroyEvent},
    {"vkGetEventStatus", (void*)GetEventStatus},
    {"vkSetEvent", (void*)SetEvent},
    {"vkResetEvent", (void*)ResetEvent},
    {"vkCreateQueryPool", (void*)CreateQueryPool},
    {"vkDestroyQueryPool", (void*)DestroyQueryPool},
    {"vkGetQueryPoolResults", (void*)GetQueryPoolResults},
    {"vkCreateBuffer", (void*)CreateBuffer},
    {"vkDestroyBuffer", (void*)DestroyBuffer},
    {"vkCreateBufferView", (void*)CreateBufferView},
    {"vkDestroyBufferView", (void*)DestroyBufferView},
    {"vkCreateImage", (void*)CreateImage},
    {"vkDestroyImage", (void*)DestroyImage},
    {"vkGetImageSubresourceLayout", (void*)GetImageSubresourceLayout},
    {"vkCreateImageView", (void*)CreateImageView},
    {"vkDestroyImageView", (void*)DestroyImageView},
    {"vkCreateShaderModule", (void*)CreateShaderModule},
    {"vkDestroyShaderModule", (void*)DestroyShaderModule},
    {"vkCreatePipelineCache", (void*)CreatePipelineCache},
    {"vkDestroyPipelineCache", (void*)DestroyPipelineCache},
    {"vkGetPipelineCacheData", (void*)GetPipelineCacheData},
    {"vkMergePipelineCaches", (void*)MergePipelineCaches},
    {"vkCreateGraphicsPipelines", (void*)CreateGraphicsPipelines},
    {"vkCreateComputePipelines", (void*)CreateComputePipelines},
    {"vkDestroyPipeline", (void*)DestroyPipeline},
    {"vkCreatePipelineLayout", (void*)CreatePipelineLayout},
    {"vkDestroyPipelineLayout", (void*)DestroyPipelineLayout},
    {"vkCreateSampler", (void*)CreateSampler},
    {"vkDestroySampler", (void*)DestroySampler},
    {"vkCreateDescriptorSetLayout", (void*)CreateDescriptorSetLayout},
    {"vkDestroyDescriptorSetLayout", (void*)DestroyDescriptorSetLayout},
    {"vkCreateDescriptorPool", (void*)CreateDescriptorPool},
    {"vkDestroyDescriptorPool", (void*)DestroyDescriptorPool},
    {"vkResetDescriptorPool", (void*)ResetDescriptorPool},
    {"vkAllocateDescriptorSets", (void*)AllocateDescriptorSets},
    {"vkFreeDescriptorSets", (void*)FreeDescriptorSets},
    {"vkUpdateDescriptorSets", (void*)UpdateDescriptorSets},
    {"vkCreateFramebuffer", (void*)CreateFramebuffer},
    {"vkDestroyFramebuffer", (void*)DestroyFramebuffer},
    {"vkCreateRenderPass", (void*)CreateRenderPass},
    {"vkDestroyRenderPass", (void*)DestroyRenderPass},
    {"vkGetRenderAreaGranularity", (void*)GetRenderAreaGranularity},
    {"vkCreateCommandPool", (void*)CreateCommandPool},
    {"vkDestroyCommandPool", (void*)DestroyCommandPool},
    {"vkResetCommandPool", (void*)ResetCommandPool},
    {"vkAllocateCommandBuffers", (void*)AllocateCommandBuffers},
    {"vkFreeCommandBuffers", (void*)FreeCommandBuffers},
    {"vkBeginCommandBuffer", (void*)BeginCommandBuffer},
    {"vkEndCommandBuffer", (void*)EndCommandBuffer},
    {"vkResetCommandBuffer", (void*)ResetCommandBuffer},
    {"vkCmdBindPipeline", (void*)CmdBindPipeline},
    {"vkCmdSetViewport", (void*)CmdSetViewport},
    {"vkCmdSetScissor", (void*)CmdSetScissor},
    {"vkCmdSetLineWidth", (void*)CmdSetLineWidth},
    {"vkCmdSetDepthBias", (void*)CmdSetDepthBias},
    {"vkCmdSetBlendConstants", (void*)CmdSetBlendConstants},
    {"vkCmdSetDepthBounds", (void*)CmdSetDepthBounds},
    {"vkCmdSetStencilCompareMask", (void*)CmdSetStencilCompareMask},
    {"vkCmdSetStencilWriteMask", (void*)CmdSetStencilWriteMask},
    {"vkCmdSetStencilReference", (void*)CmdSetStencilReference},
    {"vkCmdBindDescriptorSets", (void*)CmdBindDescriptorSets},
    {"vkCmdBindIndexBuffer", (void*)CmdBindIndexBuffer},
    {"vkCmdBindVertexBuffers", (void*)CmdBindVertexBuffers},
    {"vkCmdDraw", (void*)CmdDraw},
    {"vkCmdDrawIndexed", (void*)CmdDrawIndexed},
    {"vkCmdDrawIndirect", (void*)CmdDrawIndirect},
    {"vkCmdDrawIndexedIndirect", (void*)CmdDrawIndexedIndirect},
    {"vkCmdDispatch", (void*)CmdDispatch},
    {"vkCmdDispatchIndirect", (void*)CmdDispatchIndirect},
    {"vkCmdCopyBuffer", (void*)CmdCopyBuffer},
    {"vkCmdCopyImage", (void*)CmdCopyImage},
    {"vkCmdBlitImage", (void*)CmdBlitImage},
    {"vkCmdCopyBufferToImage", (void*)CmdCopyBufferToImage},
    {"vkCmdCopyImageToBuffer", (void*)CmdCopyImageToBuffer},
    {"vkCmdUpdateBuffer", (void*)CmdUpdateBuffer},
    {"vkCmdFillBuffer", (void*)CmdFillBuffer},
    {"vkCmdClearColorImage", (void*)CmdClearColorImage},
    {"vkCmdClearDepthStencilImage", (void*)CmdClearDepthStencilImage},
    {"vkCmdClearAttachments", (void*)CmdClearAttachments},
    {"vkCmdResolveImage", (void*)CmdResolveImage},
    {"vkCmdSetEvent", (void*)CmdSetEvent},
    {"vkCmdResetEvent", (void*)CmdResetEvent},
    {"vkCmdWaitEvents", (void*)CmdWaitEvents},
    {"vkCmdPipelineBarrier", (void*)CmdPipelineBarrier},
    {"vkCmdBeginQuery", (void*)CmdBeginQuery},
    {"vkCmdEndQuery", (void*)CmdEndQuery},
    {"vkCmdResetQueryPool", (void*)CmdResetQueryPool},
    {"vkCmdWriteTimestamp", (void*)CmdWriteTimestamp},
    {"vkCmdCopyQueryPoolResults", (void*)CmdCopyQueryPoolResults},
    {"vkCmdPushConstants", (void*)CmdPushConstants},
    {"vkCmdBeginRenderPass", (void*)CmdBeginRenderPass},
    {"vkCmdNextSubpass", (void*)CmdNextSubpass},
    {"vkCmdEndRenderPass", (void*)CmdEndRenderPass},
    {"vkCmdExecuteCommands", (void*)CmdExecuteCommands},
    {"vkEnumerateInstanceVersion", (void*)EnumerateInstanceVersion},
    {"vkBindBufferMemory2", (void*)BindBufferMemory2},
    {"vkBindImageMemory2", (void*)BindImageMemory2},
    {"vkGetDeviceGroupPeerMemoryFeatures", (void*)GetDeviceGroupPeerMemoryFeatures},
    {"vkCmdSetDeviceMask", (void*)CmdSetDeviceMask},
    {"vkCmdDispatchBase", (void*)CmdDispatchBase},
    {"vkEnumeratePhysicalDeviceGroups", (void*)EnumeratePhysicalDeviceGroups},
    {"vkGetImageMemoryRequirements2", (void*)GetImageMemoryRequirements2},
    {"vkGetBufferMemoryRequirements2", (void*)GetBufferMemoryRequirements2},
    {"vkGetImageSparseMemoryRequirements2", (void*)GetImageSparseMemoryRequirements2},
    {"vkGetPhysicalDeviceFeatures2", (void*)GetPhysicalDeviceFeatures2},
    {"vkGetPhysicalDeviceProperties2", (void*)GetPhysicalDeviceProperties2},
    {"vkGetPhysicalDeviceFormatProperties2", (void*)GetPhysicalDeviceFormatProperties2},
    {"vkGetPhysicalDeviceImageFormatProperties2", (void*)GetPhysicalDeviceImageFormatProperties2},
    {"vkGetPhysicalDeviceQueueFamilyProperties2", (void*)GetPhysicalDeviceQueueFamilyProperties2},
    {"vkGetPhysicalDeviceMemoryProperties2", (void*)GetPhysicalDeviceMemoryProperties2},
    {"vkGetPhysicalDeviceSparseImageFormatProperties2", (void*)GetPhysicalDeviceSparseImageFormatProperties2},
    {"vkTrimCommandPool", (void*)TrimCommandPool},
    {"vkGetDeviceQueue2", (void*)GetDeviceQueue2},
    {"vkCreateSamplerYcbcrConversion", (void*)CreateSamplerYcbcrConversion},
    {"vkDestroySamplerYcbcrConversion", (void*)DestroySamplerYcbcrConversion},
    {"vkCreateDescriptorUpdateTemplate", (void*)CreateDescriptorUpdateTemplate},
    {"vkDestroyDescriptorUpdateTemplate", (void*)DestroyDescriptorUpdateTemplate},
    {"vkUpdateDescriptorSetWithTemplate", (void*)UpdateDescriptorSetWithTemplate},
    {"vkGetPhysicalDeviceExternalBufferProperties", (void*)GetPhysicalDeviceExternalBufferProperties},
    {"vkGetPhysicalDeviceExternalFenceProperties", (void*)GetPhysicalDeviceExternalFenceProperties},
    {"vkGetPhysicalDeviceExternalSemaphoreProperties", (void*)GetPhysicalDeviceExternalSemaphoreProperties},
    {"vkGetDescriptorSetLayoutSupport", (void*)GetDescriptorSetLayoutSupport},
    {"vkCmdDrawIndirectCount", (void*)CmdDrawIndirectCount},
    {"vkCmdDrawIndexedIndirectCount", (void*)CmdDrawIndexedIndirectCount},
    {"vkCreateRenderPass2", (void*)CreateRenderPass2},
    {"vkCmdBeginRenderPass2", (void*)CmdBeginRenderPass2},
    {"vkCmdNextSubpass2", (void*)CmdNextSubpass2},
    {"vkCmdEndRenderPass2", (void*)CmdEndRenderPass2},
    {"vkResetQueryPool", (void*)ResetQueryPool},
    {"vkGetSemaphoreCounterValue", (void*)GetSemaphoreCounterValue},
    {"vkWaitSemaphores", (void*)WaitSemaphores},
    {"vkSignalSemaphore", (void*)SignalSemaphore},
    {"vkGetBufferDeviceAddress", (void*)GetBufferDeviceAddress},
    {"vkGetBufferOpaqueCaptureAddress", (void*)GetBufferOpaqueCaptureAddress},
    {"vkGetDeviceMemoryOpaqueCaptureAddress", (void*)GetDeviceMemoryOpaqueCaptureAddress},
    {"vkGetPhysicalDeviceToolProperties", (void*)GetPhysicalDeviceToolProperties},
    {"vkCreatePrivateDataSlot", (void*)CreatePrivateDataSlot},
    {"vkDestroyPrivateDataSlot", (void*)DestroyPrivateDataSlot},
    {"vkSetPrivateData", (void*)SetPrivateData},
    {"vkGetPrivateData", (void*)GetPrivateData},
    {"vkCmdSetEvent2", (void*)CmdSetEvent2},
    {"vkCmdResetEvent2", (void*)CmdResetEvent2},
    {"vkCmdWaitEvents2", (void*)CmdWaitEvents2},
    {"vkCmdPipelineBarrier2", (void*)CmdPipelineBarrier2},
    {"vkCmdWriteTimestamp2", (void*)CmdWriteTimestamp2},
    {"vkQueueSubmit2", (void*)QueueSubmit2},
    {"vkCmdCopyBuffer2", (void*)CmdCopyBuffer2},
    {"vkCmdCopyImage2", (void*)CmdCopyImage2},
    {"vkCmdCopyBufferToImage2", (void*)CmdCopyBufferToImage2},
    {"vkCmdCopyImageToBuffer2", (void*)CmdCopyImageToBuffer2},
    {"vkCmdBlitImage2", (void*)CmdBlitImage2},
    {"vkCmdResolveImage2", (void*)CmdResolveImage2},
    {"vkCmdBeginRendering", (void*)CmdBeginRendering},
    {"vkCmdEndRendering", (void*)CmdEndRendering},
    {"vkCmdSetCullMode", (void*)CmdSetCullMode},
    {"vkCmdSetFrontFace", (void*)CmdSetFrontFace},
    {"vkCmdSetPrimitiveTopology", (void*)CmdSetPrimitiveTopology},
    {"vkCmdSetViewportWithCount", (void*)CmdSetViewportWithCount},
    {"vkCmdSetScissorWithCount", (void*)CmdSetScissorWithCount},
    {"vkCmdBindVertexBuffers2", (void*)CmdBindVertexBuffers2},
    {"vkCmdSetDepthTestEnable", (void*)CmdSetDepthTestEnable},
    {"vkCmdSetDepthWriteEnable", (void*)CmdSetDepthWriteEnable},
    {"vkCmdSetDepthCompareOp", (void*)CmdSetDepthCompareOp},
    {"vkCmdSetDepthBoundsTestEnable", (void*)CmdSetDepthBoundsTestEnable},
    {"vkCmdSetStencilTestEnable", (void*)CmdSetStencilTestEnable},
    {"vkCmdSetStencilOp", (void*)CmdSetStencilOp},
    {"vkCmdSetRasterizerDiscardEnable", (void*)CmdSetRasterizerDiscardEnable},
    {"vkCmdSetDepthBiasEnable", (void*)CmdSetDepthBiasEnable},
    {"vkCmdSetPrimitiveRestartEnable", (void*)CmdSetPrimitiveRestartEnable},
    {"vkGetDeviceBufferMemoryRequirements", (void*)GetDeviceBufferMemoryRequirements},
    {"vkGetDeviceImageMemoryRequirements", (void*)GetDeviceImageMemoryRequirements},
    {"vkGetDeviceImageSparseMemoryRequirements", (void*)GetDeviceImageSparseMemoryRequirements},
    {"vkCmdSetLineStipple", (void*)CmdSetLineStipple},
    {"vkMapMemory2", (void*)MapMemory2},
    {"vkUnmapMemory2", (void*)UnmapMemory2},
    {"vkCmdBindIndexBuffer2", (void*)CmdBindIndexBuffer2},
    {"vkGetRenderingAreaGranularity", (void*)GetRenderingAreaGranularity},
    {"vkGetDeviceImageSubresourceLayout", (void*)GetDeviceImageSubresourceLayout},
    {"vkGetImageSubresourceLayout2", (void*)GetImageSubresourceLayout2},
    {"vkCmdPushDescriptorSet", (void*)CmdPushDescriptorSet},
    {"vkCmdPushDescriptorSetWithTemplate", (void*)CmdPushDescriptorSetWithTemplate},
    {"vkCmdSetRenderingAttachmentLocations", (void*)CmdSetRenderingAttachmentLocations},
    {"vkCmdSetRenderingInputAttachmentIndices", (void*)CmdSetRenderingInputAttachmentIndices},
    {"vkCmdBindDescriptorSets2", (void*)CmdBindDescriptorSets2},
    {"vkCmdPushConstants2", (void*)CmdPushConstants2},
    {"vkCmdPushDescriptorSet2", (void*)CmdPushDescriptorSet2},
    {"vkCmdPushDescriptorSetWithTemplate2", (void*)CmdPushDescriptorSetWithTemplate2},
    {"vkCopyMemoryToImage", (void*)CopyMemoryToImage},
    {"vkCopyImageToMemory", (void*)CopyImageToMemory},
    {"vkCopyImageToImage", (void*)CopyImageToImage},
    {"vkTransitionImageLayout", (void*)TransitionImageLayout},
    {"vkDestroySurfaceKHR", (void*)DestroySurfaceKHR},
    {"vkGetPhysicalDeviceSurfaceSupportKHR", (void*)GetPhysicalDeviceSurfaceSupportKHR},
    {"vkGetPhysicalDeviceSurfaceCapabilitiesKHR", (void*)GetPhysicalDeviceSurfaceCapabilitiesKHR},
    {"vkGetPhysicalDeviceSurfaceFormatsKHR", (void*)GetPhysicalDeviceSurfaceFormatsKHR},
    {"vkGetPhysicalDeviceSurfacePresentModesKHR", (void*)GetPhysicalDeviceSurfacePresentModesKHR},
    {"vkCreateSwapchainKHR", (void*)CreateSwapchainKHR},
    {"vkDestroySwapchainKHR", (void*)DestroySwapchainKHR},
    {"vkGetSwapchainImagesKHR", (void*)GetSwapchainImagesKHR},
    {"vkAcquireNextImageKHR", (void*)AcquireNextImageKHR},
    {"vkQueuePresentKHR", (void*)QueuePresentKHR},
    {"vkGetDeviceGroupPresentCapabilitiesKHR", (void*)GetDeviceGroupPresentCapabilitiesKHR},
    {"vkGetDeviceGroupSurfacePresentModesKHR", (void*)GetDeviceGroupSurfacePresentModesKHR},
    {"vkGetPhysicalDevicePresentRectanglesKHR", (void*)GetPhysicalDevicePresentRectanglesKHR},
    {"vkAcquireNextImage2KHR", (void*)AcquireNextImage2KHR},
    {"vkGetPhysicalDeviceDisplayPropertiesKHR", (void*)GetPhysicalDeviceDisplayPropertiesKHR},
    {"vkGetPhysicalDeviceDisplayPlanePropertiesKHR", (void*)GetPhysicalDeviceDisplayPlanePropertiesKHR},
    {"vkGetDisplayPlaneSupportedDisplaysKHR", (void*)GetDisplayPlaneSupportedDisplaysKHR},
    {"vkGetDisplayModePropertiesKHR", (void*)GetDisplayModePropertiesKHR},
    {"vkCreateDisplayModeKHR", (void*)CreateDisplayModeKHR},
    {"vkGetDisplayPlaneCapabilitiesKHR", (void*)GetDisplayPlaneCapabilitiesKHR},
    {"vkCreateDisplayPlaneSurfaceKHR", (void*)CreateDisplayPlaneSurfaceKHR},
    {"vkCreateSharedSwapchainsKHR", (void*)CreateSharedSwapchainsKHR},
#ifdef VK_USE_PLATFORM_XLIB_KHR
    {"vkCreateXlibSurfaceKHR", (void*)CreateXlibSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    {"vkGetPhysicalDeviceXlibPresentationSupportKHR", (void*)GetPhysicalDeviceXlibPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    {"vkCreateXcbSurfaceKHR", (void*)CreateXcbSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    {"vkGetPhysicalDeviceXcbPresentationSupportKHR", (void*)GetPhysicalDeviceXcbPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    {"vkCreateWaylandSurfaceKHR", (void*)CreateWaylandSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    {"vkGetPhysicalDeviceWaylandPresentationSupportKHR", (void*)GetPhysicalDeviceWaylandPresentationSupportKHR},
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkCreateAndroidSurfaceKHR", (void*)CreateAndroidSurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkCreateWin32SurfaceKHR", (void*)CreateWin32SurfaceKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetPhysicalDeviceWin32PresentationSupportKHR", (void*)GetPhysicalDeviceWin32PresentationSupportKHR},
#endif
    {"vkGetPhysicalDeviceVideoCapabilitiesKHR", (void*)GetPhysicalDeviceVideoCapabilitiesKHR},
    {"vkGetPhysicalDeviceVideoFormatPropertiesKHR", (void*)GetPhysicalDeviceVideoFormatPropertiesKHR},
    {"vkCreateVideoSessionKHR", (void*)CreateVideoSessionKHR},
    {"vkDestroyVideoSessionKHR", (void*)DestroyVideoSessionKHR},
    {"vkGetVideoSessionMemoryRequirementsKHR", (void*)GetVideoSessionMemoryRequirementsKHR},
    {"vkBindVideoSessionMemoryKHR", (void*)BindVideoSessionMemoryKHR},
    {"vkCreateVideoSessionParametersKHR", (void*)CreateVideoSessionParametersKHR},
    {"vkUpdateVideoSessionParametersKHR", (void*)UpdateVideoSessionParametersKHR},
    {"vkDestroyVideoSessionParametersKHR", (void*)DestroyVideoSessionParametersKHR},
    {"vkCmdBeginVideoCodingKHR", (void*)CmdBeginVideoCodingKHR},
    {"vkCmdEndVideoCodingKHR", (void*)CmdEndVideoCodingKHR},
    {"vkCmdControlVideoCodingKHR", (void*)CmdControlVideoCodingKHR},
    {"vkCmdDecodeVideoKHR", (void*)CmdDecodeVideoKHR},
    {"vkCmdBeginRenderingKHR", (void*)CmdBeginRenderingKHR},
    {"vkCmdEndRenderingKHR", (void*)CmdEndRenderingKHR},
    {"vkGetPhysicalDeviceFeatures2KHR", (void*)GetPhysicalDeviceFeatures2KHR},
    {"vkGetPhysicalDeviceProperties2KHR", (void*)GetPhysicalDeviceProperties2KHR},
    {"vkGetPhysicalDeviceFormatProperties2KHR", (void*)GetPhysicalDeviceFormatProperties2KHR},
    {"vkGetPhysicalDeviceImageFormatProperties2KHR", (void*)GetPhysicalDeviceImageFormatProperties2KHR},
    {"vkGetPhysicalDeviceQueueFamilyProperties2KHR", (void*)GetPhysicalDeviceQueueFamilyProperties2KHR},
    {"vkGetPhysicalDeviceMemoryProperties2KHR", (void*)GetPhysicalDeviceMemoryProperties2KHR},
    {"vkGetPhysicalDeviceSparseImageFormatProperties2KHR", (void*)GetPhysicalDeviceSparseImageFormatProperties2KHR},
    {"vkGetDeviceGroupPeerMemoryFeaturesKHR", (void*)GetDeviceGroupPeerMemoryFeaturesKHR},
    {"vkCmdSetDeviceMaskKHR", (void*)CmdSetDeviceMaskKHR},
    {"vkCmdDispatchBaseKHR", (void*)CmdDispatchBaseKHR},
    {"vkTrimCommandPoolKHR", (void*)TrimCommandPoolKHR},
    {"vkEnumeratePhysicalDeviceGroupsKHR", (void*)EnumeratePhysicalDeviceGroupsKHR},
    {"vkGetPhysicalDeviceExternalBufferPropertiesKHR", (void*)GetPhysicalDeviceExternalBufferPropertiesKHR},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandleKHR", (void*)GetMemoryWin32HandleKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandlePropertiesKHR", (void*)GetMemoryWin32HandlePropertiesKHR},
#endif
    {"vkGetMemoryFdKHR", (void*)GetMemoryFdKHR},
    {"vkGetMemoryFdPropertiesKHR", (void*)GetMemoryFdPropertiesKHR},
    {"vkGetPhysicalDeviceExternalSemaphorePropertiesKHR", (void*)GetPhysicalDeviceExternalSemaphorePropertiesKHR},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkImportSemaphoreWin32HandleKHR", (void*)ImportSemaphoreWin32HandleKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetSemaphoreWin32HandleKHR", (void*)GetSemaphoreWin32HandleKHR},
#endif
    {"vkImportSemaphoreFdKHR", (void*)ImportSemaphoreFdKHR},
    {"vkGetSemaphoreFdKHR", (void*)GetSemaphoreFdKHR},
    {"vkCmdPushDescriptorSetKHR", (void*)CmdPushDescriptorSetKHR},
    {"vkCmdPushDescriptorSetWithTemplateKHR", (void*)CmdPushDescriptorSetWithTemplateKHR},
    {"vkCreateDescriptorUpdateTemplateKHR", (void*)CreateDescriptorUpdateTemplateKHR},
    {"vkDestroyDescriptorUpdateTemplateKHR", (void*)DestroyDescriptorUpdateTemplateKHR},
    {"vkUpdateDescriptorSetWithTemplateKHR", (void*)UpdateDescriptorSetWithTemplateKHR},
    {"vkCreateRenderPass2KHR", (void*)CreateRenderPass2KHR},
    {"vkCmdBeginRenderPass2KHR", (void*)CmdBeginRenderPass2KHR},
    {"vkCmdNextSubpass2KHR", (void*)CmdNextSubpass2KHR},
    {"vkCmdEndRenderPass2KHR", (void*)CmdEndRenderPass2KHR},
    {"vkGetSwapchainStatusKHR", (void*)GetSwapchainStatusKHR},
    {"vkGetPhysicalDeviceExternalFencePropertiesKHR", (void*)GetPhysicalDeviceExternalFencePropertiesKHR},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkImportFenceWin32HandleKHR", (void*)ImportFenceWin32HandleKHR},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetFenceWin32HandleKHR", (void*)GetFenceWin32HandleKHR},
#endif
    {"vkImportFenceFdKHR", (void*)ImportFenceFdKHR},
    {"vkGetFenceFdKHR", (void*)GetFenceFdKHR},
    {"vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR", (void*)EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR},
    {"vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR", (void*)GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR},
    {"vkAcquireProfilingLockKHR", (void*)AcquireProfilingLockKHR},
    {"vkReleaseProfilingLockKHR", (void*)ReleaseProfilingLockKHR},
    {"vkGetPhysicalDeviceSurfaceCapabilities2KHR", (void*)GetPhysicalDeviceSurfaceCapabilities2KHR},
    {"vkGetPhysicalDeviceSurfaceFormats2KHR", (void*)GetPhysicalDeviceSurfaceFormats2KHR},
    {"vkGetPhysicalDeviceDisplayProperties2KHR", (void*)GetPhysicalDeviceDisplayProperties2KHR},
    {"vkGetPhysicalDeviceDisplayPlaneProperties2KHR", (void*)GetPhysicalDeviceDisplayPlaneProperties2KHR},
    {"vkGetDisplayModeProperties2KHR", (void*)GetDisplayModeProperties2KHR},
    {"vkGetDisplayPlaneCapabilities2KHR", (void*)GetDisplayPlaneCapabilities2KHR},
    {"vkGetImageMemoryRequirements2KHR", (void*)GetImageMemoryRequirements2KHR},
    {"vkGetBufferMemoryRequirements2KHR", (void*)GetBufferMemoryRequirements2KHR},
    {"vkGetImageSparseMemoryRequirements2KHR", (void*)GetImageSparseMemoryRequirements2KHR},
    {"vkCreateSamplerYcbcrConversionKHR", (void*)CreateSamplerYcbcrConversionKHR},
    {"vkDestroySamplerYcbcrConversionKHR", (void*)DestroySamplerYcbcrConversionKHR},
    {"vkBindBufferMemory2KHR", (void*)BindBufferMemory2KHR},
    {"vkBindImageMemory2KHR", (void*)BindImageMemory2KHR},
    {"vkGetDescriptorSetLayoutSupportKHR", (void*)GetDescriptorSetLayoutSupportKHR},
    {"vkCmdDrawIndirectCountKHR", (void*)CmdDrawIndirectCountKHR},
    {"vkCmdDrawIndexedIndirectCountKHR", (void*)CmdDrawIndexedIndirectCountKHR},
    {"vkGetSemaphoreCounterValueKHR", (void*)GetSemaphoreCounterValueKHR},
    {"vkWaitSemaphoresKHR", (void*)WaitSemaphoresKHR},
    {"vkSignalSemaphoreKHR", (void*)SignalSemaphoreKHR},
    {"vkGetPhysicalDeviceFragmentShadingRatesKHR", (void*)GetPhysicalDeviceFragmentShadingRatesKHR},
    {"vkCmdSetFragmentShadingRateKHR", (void*)CmdSetFragmentShadingRateKHR},
    {"vkCmdSetRenderingAttachmentLocationsKHR", (void*)CmdSetRenderingAttachmentLocationsKHR},
    {"vkCmdSetRenderingInputAttachmentIndicesKHR", (void*)CmdSetRenderingInputAttachmentIndicesKHR},
    {"vkWaitForPresentKHR", (void*)WaitForPresentKHR},
    {"vkGetBufferDeviceAddressKHR", (void*)GetBufferDeviceAddressKHR},
    {"vkGetBufferOpaqueCaptureAddressKHR", (void*)GetBufferOpaqueCaptureAddressKHR},
    {"vkGetDeviceMemoryOpaqueCaptureAddressKHR", (void*)GetDeviceMemoryOpaqueCaptureAddressKHR},
    {"vkCreateDeferredOperationKHR", (void*)CreateDeferredOperationKHR},
    {"vkDestroyDeferredOperationKHR", (void*)DestroyDeferredOperationKHR},
    {"vkGetDeferredOperationMaxConcurrencyKHR", (void*)GetDeferredOperationMaxConcurrencyKHR},
    {"vkGetDeferredOperationResultKHR", (void*)GetDeferredOperationResultKHR},
    {"vkDeferredOperationJoinKHR", (void*)DeferredOperationJoinKHR},
    {"vkGetPipelineExecutablePropertiesKHR", (void*)GetPipelineExecutablePropertiesKHR},
    {"vkGetPipelineExecutableStatisticsKHR", (void*)GetPipelineExecutableStatisticsKHR},
    {"vkGetPipelineExecutableInternalRepresentationsKHR", (void*)GetPipelineExecutableInternalRepresentationsKHR},
    {"vkMapMemory2KHR", (void*)MapMemory2KHR},
    {"vkUnmapMemory2KHR", (void*)UnmapMemory2KHR},
    {"vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR", (void*)GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR},
    {"vkGetEncodedVideoSessionParametersKHR", (void*)GetEncodedVideoSessionParametersKHR},
    {"vkCmdEncodeVideoKHR", (void*)CmdEncodeVideoKHR},
    {"vkCmdSetEvent2KHR", (void*)CmdSetEvent2KHR},
    {"vkCmdResetEvent2KHR", (void*)CmdResetEvent2KHR},
    {"vkCmdWaitEvents2KHR", (void*)CmdWaitEvents2KHR},
    {"vkCmdPipelineBarrier2KHR", (void*)CmdPipelineBarrier2KHR},
    {"vkCmdWriteTimestamp2KHR", (void*)CmdWriteTimestamp2KHR},
    {"vkQueueSubmit2KHR", (void*)QueueSubmit2KHR},
    {"vkCmdCopyBuffer2KHR", (void*)CmdCopyBuffer2KHR},
    {"vkCmdCopyImage2KHR", (void*)CmdCopyImage2KHR},
    {"vkCmdCopyBufferToImage2KHR", (void*)CmdCopyBufferToImage2KHR},
    {"vkCmdCopyImageToBuffer2KHR", (void*)CmdCopyImageToBuffer2KHR},
    {"vkCmdBlitImage2KHR", (void*)CmdBlitImage2KHR},
    {"vkCmdResolveImage2KHR", (void*)CmdResolveImage2KHR},
    {"vkCmdTraceRaysIndirect2KHR", (void*)CmdTraceRaysIndirect2KHR},
    {"vkGetDeviceBufferMemoryRequirementsKHR", (void*)GetDeviceBufferMemoryRequirementsKHR},
    {"vkGetDeviceImageMemoryRequirementsKHR", (void*)GetDeviceImageMemoryRequirementsKHR},
    {"vkGetDeviceImageSparseMemoryRequirementsKHR", (void*)GetDeviceImageSparseMemoryRequirementsKHR},
    {"vkCmdBindIndexBuffer2KHR", (void*)CmdBindIndexBuffer2KHR},
    {"vkGetRenderingAreaGranularityKHR", (void*)GetRenderingAreaGranularityKHR},
    {"vkGetDeviceImageSubresourceLayoutKHR", (void*)GetDeviceImageSubresourceLayoutKHR},
    {"vkGetImageSubresourceLayout2KHR", (void*)GetImageSubresourceLayout2KHR},
    {"vkCreatePipelineBinariesKHR", (void*)CreatePipelineBinariesKHR},
    {"vkDestroyPipelineBinaryKHR", (void*)DestroyPipelineBinaryKHR},
    {"vkGetPipelineKeyKHR", (void*)GetPipelineKeyKHR},
    {"vkGetPipelineBinaryDataKHR", (void*)GetPipelineBinaryDataKHR},
    {"vkReleaseCapturedPipelineDataKHR", (void*)ReleaseCapturedPipelineDataKHR},
    {"vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR", (void*)GetPhysicalDeviceCooperativeMatrixPropertiesKHR},
    {"vkCmdSetLineStippleKHR", (void*)CmdSetLineStippleKHR},
    {"vkGetPhysicalDeviceCalibrateableTimeDomainsKHR", (void*)GetPhysicalDeviceCalibrateableTimeDomainsKHR},
    {"vkGetCalibratedTimestampsKHR", (void*)GetCalibratedTimestampsKHR},
    {"vkCmdBindDescriptorSets2KHR", (void*)CmdBindDescriptorSets2KHR},
    {"vkCmdPushConstants2KHR", (void*)CmdPushConstants2KHR},
    {"vkCmdPushDescriptorSet2KHR", (void*)CmdPushDescriptorSet2KHR},
    {"vkCmdPushDescriptorSetWithTemplate2KHR", (void*)CmdPushDescriptorSetWithTemplate2KHR},
    {"vkCmdSetDescriptorBufferOffsets2EXT", (void*)CmdSetDescriptorBufferOffsets2EXT},
    {"vkCmdBindDescriptorBufferEmbeddedSamplers2EXT", (void*)CmdBindDescriptorBufferEmbeddedSamplers2EXT},
    {"vkCreateDebugReportCallbackEXT", (void*)CreateDebugReportCallbackEXT},
    {"vkDestroyDebugReportCallbackEXT", (void*)DestroyDebugReportCallbackEXT},
    {"vkDebugReportMessageEXT", (void*)DebugReportMessageEXT},
    {"vkDebugMarkerSetObjectTagEXT", (void*)DebugMarkerSetObjectTagEXT},
    {"vkDebugMarkerSetObjectNameEXT", (void*)DebugMarkerSetObjectNameEXT},
    {"vkCmdDebugMarkerBeginEXT", (void*)CmdDebugMarkerBeginEXT},
    {"vkCmdDebugMarkerEndEXT", (void*)CmdDebugMarkerEndEXT},
    {"vkCmdDebugMarkerInsertEXT", (void*)CmdDebugMarkerInsertEXT},
    {"vkCmdBindTransformFeedbackBuffersEXT", (void*)CmdBindTransformFeedbackBuffersEXT},
    {"vkCmdBeginTransformFeedbackEXT", (void*)CmdBeginTransformFeedbackEXT},
    {"vkCmdEndTransformFeedbackEXT", (void*)CmdEndTransformFeedbackEXT},
    {"vkCmdBeginQueryIndexedEXT", (void*)CmdBeginQueryIndexedEXT},
    {"vkCmdEndQueryIndexedEXT", (void*)CmdEndQueryIndexedEXT},
    {"vkCmdDrawIndirectByteCountEXT", (void*)CmdDrawIndirectByteCountEXT},
    {"vkCreateCuModuleNVX", (void*)CreateCuModuleNVX},
    {"vkCreateCuFunctionNVX", (void*)CreateCuFunctionNVX},
    {"vkDestroyCuModuleNVX", (void*)DestroyCuModuleNVX},
    {"vkDestroyCuFunctionNVX", (void*)DestroyCuFunctionNVX},
    {"vkCmdCuLaunchKernelNVX", (void*)CmdCuLaunchKernelNVX},
    {"vkGetImageViewHandleNVX", (void*)GetImageViewHandleNVX},
    {"vkGetImageViewHandle64NVX", (void*)GetImageViewHandle64NVX},
    {"vkGetImageViewAddressNVX", (void*)GetImageViewAddressNVX},
    {"vkCmdDrawIndirectCountAMD", (void*)CmdDrawIndirectCountAMD},
    {"vkCmdDrawIndexedIndirectCountAMD", (void*)CmdDrawIndexedIndirectCountAMD},
    {"vkGetShaderInfoAMD", (void*)GetShaderInfoAMD},
#ifdef VK_USE_PLATFORM_GGP
    {"vkCreateStreamDescriptorSurfaceGGP", (void*)CreateStreamDescriptorSurfaceGGP},
#endif
    {"vkGetPhysicalDeviceExternalImageFormatPropertiesNV", (void*)GetPhysicalDeviceExternalImageFormatPropertiesNV},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetMemoryWin32HandleNV", (void*)GetMemoryWin32HandleNV},
#endif
#ifdef VK_USE_PLATFORM_VI_NN
    {"vkCreateViSurfaceNN", (void*)CreateViSurfaceNN},
#endif
    {"vkCmdBeginConditionalRenderingEXT", (void*)CmdBeginConditionalRenderingEXT},
    {"vkCmdEndConditionalRenderingEXT", (void*)CmdEndConditionalRenderingEXT},
    {"vkCmdSetViewportWScalingNV", (void*)CmdSetViewportWScalingNV},
    {"vkReleaseDisplayEXT", (void*)ReleaseDisplayEXT},
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    {"vkAcquireXlibDisplayEXT", (void*)AcquireXlibDisplayEXT},
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    {"vkGetRandROutputDisplayEXT", (void*)GetRandROutputDisplayEXT},
#endif
    {"vkGetPhysicalDeviceSurfaceCapabilities2EXT", (void*)GetPhysicalDeviceSurfaceCapabilities2EXT},
    {"vkDisplayPowerControlEXT", (void*)DisplayPowerControlEXT},
    {"vkRegisterDeviceEventEXT", (void*)RegisterDeviceEventEXT},
    {"vkRegisterDisplayEventEXT", (void*)RegisterDisplayEventEXT},
    {"vkGetSwapchainCounterEXT", (void*)GetSwapchainCounterEXT},
    {"vkGetRefreshCycleDurationGOOGLE", (void*)GetRefreshCycleDurationGOOGLE},
    {"vkGetPastPresentationTimingGOOGLE", (void*)GetPastPresentationTimingGOOGLE},
    {"vkCmdSetDiscardRectangleEXT", (void*)CmdSetDiscardRectangleEXT},
    {"vkCmdSetDiscardRectangleEnableEXT", (void*)CmdSetDiscardRectangleEnableEXT},
    {"vkCmdSetDiscardRectangleModeEXT", (void*)CmdSetDiscardRectangleModeEXT},
    {"vkSetHdrMetadataEXT", (void*)SetHdrMetadataEXT},
#ifdef VK_USE_PLATFORM_IOS_MVK
    {"vkCreateIOSSurfaceMVK", (void*)CreateIOSSurfaceMVK},
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    {"vkCreateMacOSSurfaceMVK", (void*)CreateMacOSSurfaceMVK},
#endif
    {"vkSetDebugUtilsObjectNameEXT", (void*)SetDebugUtilsObjectNameEXT},
    {"vkSetDebugUtilsObjectTagEXT", (void*)SetDebugUtilsObjectTagEXT},
    {"vkQueueBeginDebugUtilsLabelEXT", (void*)QueueBeginDebugUtilsLabelEXT},
    {"vkQueueEndDebugUtilsLabelEXT", (void*)QueueEndDebugUtilsLabelEXT},
    {"vkQueueInsertDebugUtilsLabelEXT", (void*)QueueInsertDebugUtilsLabelEXT},
    {"vkCmdBeginDebugUtilsLabelEXT", (void*)CmdBeginDebugUtilsLabelEXT},
    {"vkCmdEndDebugUtilsLabelEXT", (void*)CmdEndDebugUtilsLabelEXT},
    {"vkCmdInsertDebugUtilsLabelEXT", (void*)CmdInsertDebugUtilsLabelEXT},
    {"vkCreateDebugUtilsMessengerEXT", (void*)CreateDebugUtilsMessengerEXT},
    {"vkDestroyDebugUtilsMessengerEXT", (void*)DestroyDebugUtilsMessengerEXT},
    {"vkSubmitDebugUtilsMessageEXT", (void*)SubmitDebugUtilsMessageEXT},
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkGetAndroidHardwareBufferPropertiesANDROID", (void*)GetAndroidHardwareBufferPropertiesANDROID},
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {"vkGetMemoryAndroidHardwareBufferANDROID", (void*)GetMemoryAndroidHardwareBufferANDROID},
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    {"vkCreateExecutionGraphPipelinesAMDX", (void*)CreateExecutionGraphPipelinesAMDX},
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    {"vkGetExecutionGraphPipelineScratchSizeAMDX", (void*)GetExecutionGraphPipelineScratchSizeAMDX},
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    {"vkGetExecutionGraphPipelineNodeIndexAMDX", (void*)GetExecutionGraphPipelineNodeIndexAMDX},
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    {"vkCmdInitializeGraphScratchMemoryAMDX", (void*)CmdInitializeGraphScratchMemoryAMDX},
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    {"vkCmdDispatchGraphAMDX", (void*)CmdDispatchGraphAMDX},
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    {"vkCmdDispatchGraphIndirectAMDX", (void*)CmdDispatchGraphIndirectAMDX},
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    {"vkCmdDispatchGraphIndirectCountAMDX", (void*)CmdDispatchGraphIndirectCountAMDX},
#endif
    {"vkCmdSetSampleLocationsEXT", (void*)CmdSetSampleLocationsEXT},
    {"vkGetPhysicalDeviceMultisamplePropertiesEXT", (void*)GetPhysicalDeviceMultisamplePropertiesEXT},
    {"vkGetImageDrmFormatModifierPropertiesEXT", (void*)GetImageDrmFormatModifierPropertiesEXT},
    {"vkCreateValidationCacheEXT", (void*)CreateValidationCacheEXT},
    {"vkDestroyValidationCacheEXT", (void*)DestroyValidationCacheEXT},
    {"vkMergeValidationCachesEXT", (void*)MergeValidationCachesEXT},
    {"vkGetValidationCacheDataEXT", (void*)GetValidationCacheDataEXT},
    {"vkCmdBindShadingRateImageNV", (void*)CmdBindShadingRateImageNV},
    {"vkCmdSetViewportShadingRatePaletteNV", (void*)CmdSetViewportShadingRatePaletteNV},
    {"vkCmdSetCoarseSampleOrderNV", (void*)CmdSetCoarseSampleOrderNV},
    {"vkCreateAccelerationStructureNV", (void*)CreateAccelerationStructureNV},
    {"vkDestroyAccelerationStructureNV", (void*)DestroyAccelerationStructureNV},
    {"vkGetAccelerationStructureMemoryRequirementsNV", (void*)GetAccelerationStructureMemoryRequirementsNV},
    {"vkBindAccelerationStructureMemoryNV", (void*)BindAccelerationStructureMemoryNV},
    {"vkCmdBuildAccelerationStructureNV", (void*)CmdBuildAccelerationStructureNV},
    {"vkCmdCopyAccelerationStructureNV", (void*)CmdCopyAccelerationStructureNV},
    {"vkCmdTraceRaysNV", (void*)CmdTraceRaysNV},
    {"vkCreateRayTracingPipelinesNV", (void*)CreateRayTracingPipelinesNV},
    {"vkGetRayTracingShaderGroupHandlesKHR", (void*)GetRayTracingShaderGroupHandlesKHR},
    {"vkGetRayTracingShaderGroupHandlesNV", (void*)GetRayTracingShaderGroupHandlesNV},
    {"vkGetAccelerationStructureHandleNV", (void*)GetAccelerationStructureHandleNV},
    {"vkCmdWriteAccelerationStructuresPropertiesNV", (void*)CmdWriteAccelerationStructuresPropertiesNV},
    {"vkCompileDeferredNV", (void*)CompileDeferredNV},
    {"vkGetMemoryHostPointerPropertiesEXT", (void*)GetMemoryHostPointerPropertiesEXT},
    {"vkCmdWriteBufferMarkerAMD", (void*)CmdWriteBufferMarkerAMD},
    {"vkCmdWriteBufferMarker2AMD", (void*)CmdWriteBufferMarker2AMD},
    {"vkGetPhysicalDeviceCalibrateableTimeDomainsEXT", (void*)GetPhysicalDeviceCalibrateableTimeDomainsEXT},
    {"vkGetCalibratedTimestampsEXT", (void*)GetCalibratedTimestampsEXT},
    {"vkCmdDrawMeshTasksNV", (void*)CmdDrawMeshTasksNV},
    {"vkCmdDrawMeshTasksIndirectNV", (void*)CmdDrawMeshTasksIndirectNV},
    {"vkCmdDrawMeshTasksIndirectCountNV", (void*)CmdDrawMeshTasksIndirectCountNV},
    {"vkCmdSetExclusiveScissorEnableNV", (void*)CmdSetExclusiveScissorEnableNV},
    {"vkCmdSetExclusiveScissorNV", (void*)CmdSetExclusiveScissorNV},
    {"vkCmdSetCheckpointNV", (void*)CmdSetCheckpointNV},
    {"vkGetQueueCheckpointDataNV", (void*)GetQueueCheckpointDataNV},
    {"vkGetQueueCheckpointData2NV", (void*)GetQueueCheckpointData2NV},
    {"vkInitializePerformanceApiINTEL", (void*)InitializePerformanceApiINTEL},
    {"vkUninitializePerformanceApiINTEL", (void*)UninitializePerformanceApiINTEL},
    {"vkCmdSetPerformanceMarkerINTEL", (void*)CmdSetPerformanceMarkerINTEL},
    {"vkCmdSetPerformanceStreamMarkerINTEL", (void*)CmdSetPerformanceStreamMarkerINTEL},
    {"vkCmdSetPerformanceOverrideINTEL", (void*)CmdSetPerformanceOverrideINTEL},
    {"vkAcquirePerformanceConfigurationINTEL", (void*)AcquirePerformanceConfigurationINTEL},
    {"vkReleasePerformanceConfigurationINTEL", (void*)ReleasePerformanceConfigurationINTEL},
    {"vkQueueSetPerformanceConfigurationINTEL", (void*)QueueSetPerformanceConfigurationINTEL},
    {"vkGetPerformanceParameterINTEL", (void*)GetPerformanceParameterINTEL},
    {"vkSetLocalDimmingAMD", (void*)SetLocalDimmingAMD},
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkCreateImagePipeSurfaceFUCHSIA", (void*)CreateImagePipeSurfaceFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    {"vkCreateMetalSurfaceEXT", (void*)CreateMetalSurfaceEXT},
#endif
    {"vkGetBufferDeviceAddressEXT", (void*)GetBufferDeviceAddressEXT},
    {"vkGetPhysicalDeviceToolPropertiesEXT", (void*)GetPhysicalDeviceToolPropertiesEXT},
    {"vkGetPhysicalDeviceCooperativeMatrixPropertiesNV", (void*)GetPhysicalDeviceCooperativeMatrixPropertiesNV},
    {"vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV", (void*)GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetPhysicalDeviceSurfacePresentModes2EXT", (void*)GetPhysicalDeviceSurfacePresentModes2EXT},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkAcquireFullScreenExclusiveModeEXT", (void*)AcquireFullScreenExclusiveModeEXT},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkReleaseFullScreenExclusiveModeEXT", (void*)ReleaseFullScreenExclusiveModeEXT},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetDeviceGroupSurfacePresentModes2EXT", (void*)GetDeviceGroupSurfacePresentModes2EXT},
#endif
    {"vkCreateHeadlessSurfaceEXT", (void*)CreateHeadlessSurfaceEXT},
    {"vkCmdSetLineStippleEXT", (void*)CmdSetLineStippleEXT},
    {"vkResetQueryPoolEXT", (void*)ResetQueryPoolEXT},
    {"vkCmdSetCullModeEXT", (void*)CmdSetCullModeEXT},
    {"vkCmdSetFrontFaceEXT", (void*)CmdSetFrontFaceEXT},
    {"vkCmdSetPrimitiveTopologyEXT", (void*)CmdSetPrimitiveTopologyEXT},
    {"vkCmdSetViewportWithCountEXT", (void*)CmdSetViewportWithCountEXT},
    {"vkCmdSetScissorWithCountEXT", (void*)CmdSetScissorWithCountEXT},
    {"vkCmdBindVertexBuffers2EXT", (void*)CmdBindVertexBuffers2EXT},
    {"vkCmdSetDepthTestEnableEXT", (void*)CmdSetDepthTestEnableEXT},
    {"vkCmdSetDepthWriteEnableEXT", (void*)CmdSetDepthWriteEnableEXT},
    {"vkCmdSetDepthCompareOpEXT", (void*)CmdSetDepthCompareOpEXT},
    {"vkCmdSetDepthBoundsTestEnableEXT", (void*)CmdSetDepthBoundsTestEnableEXT},
    {"vkCmdSetStencilTestEnableEXT", (void*)CmdSetStencilTestEnableEXT},
    {"vkCmdSetStencilOpEXT", (void*)CmdSetStencilOpEXT},
    {"vkCopyMemoryToImageEXT", (void*)CopyMemoryToImageEXT},
    {"vkCopyImageToMemoryEXT", (void*)CopyImageToMemoryEXT},
    {"vkCopyImageToImageEXT", (void*)CopyImageToImageEXT},
    {"vkTransitionImageLayoutEXT", (void*)TransitionImageLayoutEXT},
    {"vkGetImageSubresourceLayout2EXT", (void*)GetImageSubresourceLayout2EXT},
    {"vkReleaseSwapchainImagesEXT", (void*)ReleaseSwapchainImagesEXT},
    {"vkGetGeneratedCommandsMemoryRequirementsNV", (void*)GetGeneratedCommandsMemoryRequirementsNV},
    {"vkCmdPreprocessGeneratedCommandsNV", (void*)CmdPreprocessGeneratedCommandsNV},
    {"vkCmdExecuteGeneratedCommandsNV", (void*)CmdExecuteGeneratedCommandsNV},
    {"vkCmdBindPipelineShaderGroupNV", (void*)CmdBindPipelineShaderGroupNV},
    {"vkCreateIndirectCommandsLayoutNV", (void*)CreateIndirectCommandsLayoutNV},
    {"vkDestroyIndirectCommandsLayoutNV", (void*)DestroyIndirectCommandsLayoutNV},
    {"vkCmdSetDepthBias2EXT", (void*)CmdSetDepthBias2EXT},
    {"vkAcquireDrmDisplayEXT", (void*)AcquireDrmDisplayEXT},
    {"vkGetDrmDisplayEXT", (void*)GetDrmDisplayEXT},
    {"vkCreatePrivateDataSlotEXT", (void*)CreatePrivateDataSlotEXT},
    {"vkDestroyPrivateDataSlotEXT", (void*)DestroyPrivateDataSlotEXT},
    {"vkSetPrivateDataEXT", (void*)SetPrivateDataEXT},
    {"vkGetPrivateDataEXT", (void*)GetPrivateDataEXT},
    {"vkCreateCudaModuleNV", (void*)CreateCudaModuleNV},
    {"vkGetCudaModuleCacheNV", (void*)GetCudaModuleCacheNV},
    {"vkCreateCudaFunctionNV", (void*)CreateCudaFunctionNV},
    {"vkDestroyCudaModuleNV", (void*)DestroyCudaModuleNV},
    {"vkDestroyCudaFunctionNV", (void*)DestroyCudaFunctionNV},
    {"vkCmdCudaLaunchKernelNV", (void*)CmdCudaLaunchKernelNV},
#ifdef VK_USE_PLATFORM_METAL_EXT
    {"vkExportMetalObjectsEXT", (void*)ExportMetalObjectsEXT},
#endif
    {"vkGetDescriptorSetLayoutSizeEXT", (void*)GetDescriptorSetLayoutSizeEXT},
    {"vkGetDescriptorSetLayoutBindingOffsetEXT", (void*)GetDescriptorSetLayoutBindingOffsetEXT},
    {"vkGetDescriptorEXT", (void*)GetDescriptorEXT},
    {"vkCmdBindDescriptorBuffersEXT", (void*)CmdBindDescriptorBuffersEXT},
    {"vkCmdSetDescriptorBufferOffsetsEXT", (void*)CmdSetDescriptorBufferOffsetsEXT},
    {"vkCmdBindDescriptorBufferEmbeddedSamplersEXT", (void*)CmdBindDescriptorBufferEmbeddedSamplersEXT},
    {"vkGetBufferOpaqueCaptureDescriptorDataEXT", (void*)GetBufferOpaqueCaptureDescriptorDataEXT},
    {"vkGetImageOpaqueCaptureDescriptorDataEXT", (void*)GetImageOpaqueCaptureDescriptorDataEXT},
    {"vkGetImageViewOpaqueCaptureDescriptorDataEXT", (void*)GetImageViewOpaqueCaptureDescriptorDataEXT},
    {"vkGetSamplerOpaqueCaptureDescriptorDataEXT", (void*)GetSamplerOpaqueCaptureDescriptorDataEXT},
    {"vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT", (void*)GetAccelerationStructureOpaqueCaptureDescriptorDataEXT},
    {"vkCmdSetFragmentShadingRateEnumNV", (void*)CmdSetFragmentShadingRateEnumNV},
    {"vkGetDeviceFaultInfoEXT", (void*)GetDeviceFaultInfoEXT},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkAcquireWinrtDisplayNV", (void*)AcquireWinrtDisplayNV},
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {"vkGetWinrtDisplayNV", (void*)GetWinrtDisplayNV},
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    {"vkCreateDirectFBSurfaceEXT", (void*)CreateDirectFBSurfaceEXT},
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    {"vkGetPhysicalDeviceDirectFBPresentationSupportEXT", (void*)GetPhysicalDeviceDirectFBPresentationSupportEXT},
#endif
    {"vkCmdSetVertexInputEXT", (void*)CmdSetVertexInputEXT},
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkGetMemoryZirconHandleFUCHSIA", (void*)GetMemoryZirconHandleFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkGetMemoryZirconHandlePropertiesFUCHSIA", (void*)GetMemoryZirconHandlePropertiesFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkImportSemaphoreZirconHandleFUCHSIA", (void*)ImportSemaphoreZirconHandleFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkGetSemaphoreZirconHandleFUCHSIA", (void*)GetSemaphoreZirconHandleFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkCreateBufferCollectionFUCHSIA", (void*)CreateBufferCollectionFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkSetBufferCollectionImageConstraintsFUCHSIA", (void*)SetBufferCollectionImageConstraintsFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkSetBufferCollectionBufferConstraintsFUCHSIA", (void*)SetBufferCollectionBufferConstraintsFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkDestroyBufferCollectionFUCHSIA", (void*)DestroyBufferCollectionFUCHSIA},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkGetBufferCollectionPropertiesFUCHSIA", (void*)GetBufferCollectionPropertiesFUCHSIA},
#endif
    {"vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", (void*)GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI},
    {"vkCmdSubpassShadingHUAWEI", (void*)CmdSubpassShadingHUAWEI},
    {"vkCmdBindInvocationMaskHUAWEI", (void*)CmdBindInvocationMaskHUAWEI},
    {"vkGetMemoryRemoteAddressNV", (void*)GetMemoryRemoteAddressNV},
    {"vkGetPipelinePropertiesEXT", (void*)GetPipelinePropertiesEXT},
    {"vkCmdSetPatchControlPointsEXT", (void*)CmdSetPatchControlPointsEXT},
    {"vkCmdSetRasterizerDiscardEnableEXT", (void*)CmdSetRasterizerDiscardEnableEXT},
    {"vkCmdSetDepthBiasEnableEXT", (void*)CmdSetDepthBiasEnableEXT},
    {"vkCmdSetLogicOpEXT", (void*)CmdSetLogicOpEXT},
    {"vkCmdSetPrimitiveRestartEnableEXT", (void*)CmdSetPrimitiveRestartEnableEXT},
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    {"vkCreateScreenSurfaceQNX", (void*)CreateScreenSurfaceQNX},
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    {"vkGetPhysicalDeviceScreenPresentationSupportQNX", (void*)GetPhysicalDeviceScreenPresentationSupportQNX},
#endif
    {"vkCmdSetColorWriteEnableEXT", (void*)CmdSetColorWriteEnableEXT},
    {"vkCmdDrawMultiEXT", (void*)CmdDrawMultiEXT},
    {"vkCmdDrawMultiIndexedEXT", (void*)CmdDrawMultiIndexedEXT},
    {"vkCreateMicromapEXT", (void*)CreateMicromapEXT},
    {"vkDestroyMicromapEXT", (void*)DestroyMicromapEXT},
    {"vkCmdBuildMicromapsEXT", (void*)CmdBuildMicromapsEXT},
    {"vkBuildMicromapsEXT", (void*)BuildMicromapsEXT},
    {"vkCopyMicromapEXT", (void*)CopyMicromapEXT},
    {"vkCopyMicromapToMemoryEXT", (void*)CopyMicromapToMemoryEXT},
    {"vkCopyMemoryToMicromapEXT", (void*)CopyMemoryToMicromapEXT},
    {"vkWriteMicromapsPropertiesEXT", (void*)WriteMicromapsPropertiesEXT},
    {"vkCmdCopyMicromapEXT", (void*)CmdCopyMicromapEXT},
    {"vkCmdCopyMicromapToMemoryEXT", (void*)CmdCopyMicromapToMemoryEXT},
    {"vkCmdCopyMemoryToMicromapEXT", (void*)CmdCopyMemoryToMicromapEXT},
    {"vkCmdWriteMicromapsPropertiesEXT", (void*)CmdWriteMicromapsPropertiesEXT},
    {"vkGetDeviceMicromapCompatibilityEXT", (void*)GetDeviceMicromapCompatibilityEXT},
    {"vkGetMicromapBuildSizesEXT", (void*)GetMicromapBuildSizesEXT},
    {"vkCmdDrawClusterHUAWEI", (void*)CmdDrawClusterHUAWEI},
    {"vkCmdDrawClusterIndirectHUAWEI", (void*)CmdDrawClusterIndirectHUAWEI},
    {"vkSetDeviceMemoryPriorityEXT", (void*)SetDeviceMemoryPriorityEXT},
    {"vkGetDescriptorSetLayoutHostMappingInfoVALVE", (void*)GetDescriptorSetLayoutHostMappingInfoVALVE},
    {"vkGetDescriptorSetHostMappingVALVE", (void*)GetDescriptorSetHostMappingVALVE},
    {"vkCmdCopyMemoryIndirectNV", (void*)CmdCopyMemoryIndirectNV},
    {"vkCmdCopyMemoryToImageIndirectNV", (void*)CmdCopyMemoryToImageIndirectNV},
    {"vkCmdDecompressMemoryNV", (void*)CmdDecompressMemoryNV},
    {"vkCmdDecompressMemoryIndirectCountNV", (void*)CmdDecompressMemoryIndirectCountNV},
    {"vkGetPipelineIndirectMemoryRequirementsNV", (void*)GetPipelineIndirectMemoryRequirementsNV},
    {"vkCmdUpdatePipelineIndirectBufferNV", (void*)CmdUpdatePipelineIndirectBufferNV},
    {"vkGetPipelineIndirectDeviceAddressNV", (void*)GetPipelineIndirectDeviceAddressNV},
    {"vkCmdSetDepthClampEnableEXT", (void*)CmdSetDepthClampEnableEXT},
    {"vkCmdSetPolygonModeEXT", (void*)CmdSetPolygonModeEXT},
    {"vkCmdSetRasterizationSamplesEXT", (void*)CmdSetRasterizationSamplesEXT},
    {"vkCmdSetSampleMaskEXT", (void*)CmdSetSampleMaskEXT},
    {"vkCmdSetAlphaToCoverageEnableEXT", (void*)CmdSetAlphaToCoverageEnableEXT},
    {"vkCmdSetAlphaToOneEnableEXT", (void*)CmdSetAlphaToOneEnableEXT},
    {"vkCmdSetLogicOpEnableEXT", (void*)CmdSetLogicOpEnableEXT},
    {"vkCmdSetColorBlendEnableEXT", (void*)CmdSetColorBlendEnableEXT},
    {"vkCmdSetColorBlendEquationEXT", (void*)CmdSetColorBlendEquationEXT},
    {"vkCmdSetColorWriteMaskEXT", (void*)CmdSetColorWriteMaskEXT},
    {"vkCmdSetTessellationDomainOriginEXT", (void*)CmdSetTessellationDomainOriginEXT},
    {"vkCmdSetRasterizationStreamEXT", (void*)CmdSetRasterizationStreamEXT},
    {"vkCmdSetConservativeRasterizationModeEXT", (void*)CmdSetConservativeRasterizationModeEXT},
    {"vkCmdSetExtraPrimitiveOverestimationSizeEXT", (void*)CmdSetExtraPrimitiveOverestimationSizeEXT},
    {"vkCmdSetDepthClipEnableEXT", (void*)CmdSetDepthClipEnableEXT},
    {"vkCmdSetSampleLocationsEnableEXT", (void*)CmdSetSampleLocationsEnableEXT},
    {"vkCmdSetColorBlendAdvancedEXT", (void*)CmdSetColorBlendAdvancedEXT},
    {"vkCmdSetProvokingVertexModeEXT", (void*)CmdSetProvokingVertexModeEXT},
    {"vkCmdSetLineRasterizationModeEXT", (void*)CmdSetLineRasterizationModeEXT},
    {"vkCmdSetLineStippleEnableEXT", (void*)CmdSetLineStippleEnableEXT},
    {"vkCmdSetDepthClipNegativeOneToOneEXT", (void*)CmdSetDepthClipNegativeOneToOneEXT},
    {"vkCmdSetViewportWScalingEnableNV", (void*)CmdSetViewportWScalingEnableNV},
    {"vkCmdSetViewportSwizzleNV", (void*)CmdSetViewportSwizzleNV},
    {"vkCmdSetCoverageToColorEnableNV", (void*)CmdSetCoverageToColorEnableNV},
    {"vkCmdSetCoverageToColorLocationNV", (void*)CmdSetCoverageToColorLocationNV},
    {"vkCmdSetCoverageModulationModeNV", (void*)CmdSetCoverageModulationModeNV},
    {"vkCmdSetCoverageModulationTableEnableNV", (void*)CmdSetCoverageModulationTableEnableNV},
    {"vkCmdSetCoverageModulationTableNV", (void*)CmdSetCoverageModulationTableNV},
    {"vkCmdSetShadingRateImageEnableNV", (void*)CmdSetShadingRateImageEnableNV},
    {"vkCmdSetRepresentativeFragmentTestEnableNV", (void*)CmdSetRepresentativeFragmentTestEnableNV},
    {"vkCmdSetCoverageReductionModeNV", (void*)CmdSetCoverageReductionModeNV},
    {"vkGetShaderModuleIdentifierEXT", (void*)GetShaderModuleIdentifierEXT},
    {"vkGetShaderModuleCreateInfoIdentifierEXT", (void*)GetShaderModuleCreateInfoIdentifierEXT},
    {"vkGetPhysicalDeviceOpticalFlowImageFormatsNV", (void*)GetPhysicalDeviceOpticalFlowImageFormatsNV},
    {"vkCreateOpticalFlowSessionNV", (void*)CreateOpticalFlowSessionNV},
    {"vkDestroyOpticalFlowSessionNV", (void*)DestroyOpticalFlowSessionNV},
    {"vkBindOpticalFlowSessionImageNV", (void*)BindOpticalFlowSessionImageNV},
    {"vkCmdOpticalFlowExecuteNV", (void*)CmdOpticalFlowExecuteNV},
    {"vkAntiLagUpdateAMD", (void*)AntiLagUpdateAMD},
    {"vkCreateShadersEXT", (void*)CreateShadersEXT},
    {"vkDestroyShaderEXT", (void*)DestroyShaderEXT},
    {"vkGetShaderBinaryDataEXT", (void*)GetShaderBinaryDataEXT},
    {"vkCmdBindShadersEXT", (void*)CmdBindShadersEXT},
    {"vkCmdSetDepthClampRangeEXT", (void*)CmdSetDepthClampRangeEXT},
    {"vkGetFramebufferTilePropertiesQCOM", (void*)GetFramebufferTilePropertiesQCOM},
    {"vkGetDynamicRenderingTilePropertiesQCOM", (void*)GetDynamicRenderingTilePropertiesQCOM},
    {"vkGetPhysicalDeviceCooperativeVectorPropertiesNV", (void*)GetPhysicalDeviceCooperativeVectorPropertiesNV},
    {"vkConvertCooperativeVectorMatrixNV", (void*)ConvertCooperativeVectorMatrixNV},
    {"vkCmdConvertCooperativeVectorMatrixNV", (void*)CmdConvertCooperativeVectorMatrixNV},
    {"vkSetLatencySleepModeNV", (void*)SetLatencySleepModeNV},
    {"vkLatencySleepNV", (void*)LatencySleepNV},
    {"vkSetLatencyMarkerNV", (void*)SetLatencyMarkerNV},
    {"vkGetLatencyTimingsNV", (void*)GetLatencyTimingsNV},
    {"vkQueueNotifyOutOfBandNV", (void*)QueueNotifyOutOfBandNV},
    {"vkCmdSetAttachmentFeedbackLoopEnableEXT", (void*)CmdSetAttachmentFeedbackLoopEnableEXT},
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    {"vkGetScreenBufferPropertiesQNX", (void*)GetScreenBufferPropertiesQNX},
#endif
    {"vkGetClusterAccelerationStructureBuildSizesNV", (void*)GetClusterAccelerationStructureBuildSizesNV},
    {"vkCmdBuildClusterAccelerationStructureIndirectNV", (void*)CmdBuildClusterAccelerationStructureIndirectNV},
    {"vkGetPartitionedAccelerationStructuresBuildSizesNV", (void*)GetPartitionedAccelerationStructuresBuildSizesNV},
    {"vkCmdBuildPartitionedAccelerationStructuresNV", (void*)CmdBuildPartitionedAccelerationStructuresNV},
    {"vkGetGeneratedCommandsMemoryRequirementsEXT", (void*)GetGeneratedCommandsMemoryRequirementsEXT},
    {"vkCmdPreprocessGeneratedCommandsEXT", (void*)CmdPreprocessGeneratedCommandsEXT},
    {"vkCmdExecuteGeneratedCommandsEXT", (void*)CmdExecuteGeneratedCommandsEXT},
    {"vkCreateIndirectCommandsLayoutEXT", (void*)CreateIndirectCommandsLayoutEXT},
    {"vkDestroyIndirectCommandsLayoutEXT", (void*)DestroyIndirectCommandsLayoutEXT},
    {"vkCreateIndirectExecutionSetEXT", (void*)CreateIndirectExecutionSetEXT},
    {"vkDestroyIndirectExecutionSetEXT", (void*)DestroyIndirectExecutionSetEXT},
    {"vkUpdateIndirectExecutionSetPipelineEXT", (void*)UpdateIndirectExecutionSetPipelineEXT},
    {"vkUpdateIndirectExecutionSetShaderEXT", (void*)UpdateIndirectExecutionSetShaderEXT},
    {"vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV", (void*)GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV},
#ifdef VK_USE_PLATFORM_METAL_EXT
    {"vkGetMemoryMetalHandleEXT", (void*)GetMemoryMetalHandleEXT},
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    {"vkGetMemoryMetalHandlePropertiesEXT", (void*)GetMemoryMetalHandlePropertiesEXT},
#endif
    {"vkCreateAccelerationStructureKHR", (void*)CreateAccelerationStructureKHR},
    {"vkDestroyAccelerationStructureKHR", (void*)DestroyAccelerationStructureKHR},
    {"vkCmdBuildAccelerationStructuresKHR", (void*)CmdBuildAccelerationStructuresKHR},
    {"vkCmdBuildAccelerationStructuresIndirectKHR", (void*)CmdBuildAccelerationStructuresIndirectKHR},
    {"vkBuildAccelerationStructuresKHR", (void*)BuildAccelerationStructuresKHR},
    {"vkCopyAccelerationStructureKHR", (void*)CopyAccelerationStructureKHR},
    {"vkCopyAccelerationStructureToMemoryKHR", (void*)CopyAccelerationStructureToMemoryKHR},
    {"vkCopyMemoryToAccelerationStructureKHR", (void*)CopyMemoryToAccelerationStructureKHR},
    {"vkWriteAccelerationStructuresPropertiesKHR", (void*)WriteAccelerationStructuresPropertiesKHR},
    {"vkCmdCopyAccelerationStructureKHR", (void*)CmdCopyAccelerationStructureKHR},
    {"vkCmdCopyAccelerationStructureToMemoryKHR", (void*)CmdCopyAccelerationStructureToMemoryKHR},
    {"vkCmdCopyMemoryToAccelerationStructureKHR", (void*)CmdCopyMemoryToAccelerationStructureKHR},
    {"vkGetAccelerationStructureDeviceAddressKHR", (void*)GetAccelerationStructureDeviceAddressKHR},
    {"vkCmdWriteAccelerationStructuresPropertiesKHR", (void*)CmdWriteAccelerationStructuresPropertiesKHR},
    {"vkGetDeviceAccelerationStructureCompatibilityKHR", (void*)GetDeviceAccelerationStructureCompatibilityKHR},
    {"vkGetAccelerationStructureBuildSizesKHR", (void*)GetAccelerationStructureBuildSizesKHR},
    {"vkCmdTraceRaysKHR", (void*)CmdTraceRaysKHR},
    {"vkCreateRayTracingPipelinesKHR", (void*)CreateRayTracingPipelinesKHR},
    {"vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", (void*)GetRayTracingCaptureReplayShaderGroupHandlesKHR},
    {"vkCmdTraceRaysIndirectKHR", (void*)CmdTraceRaysIndirectKHR},
    {"vkGetRayTracingShaderGroupStackSizeKHR", (void*)GetRayTracingShaderGroupStackSizeKHR},
    {"vkCmdSetRayTracingPipelineStackSizeKHR", (void*)CmdSetRayTracingPipelineStackSizeKHR},
    {"vkCmdDrawMeshTasksEXT", (void*)CmdDrawMeshTasksEXT},
    {"vkCmdDrawMeshTasksIndirectEXT", (void*)CmdDrawMeshTasksIndirectEXT},
    {"vkCmdDrawMeshTasksIndirectCountEXT", (void*)CmdDrawMeshTasksIndirectCountEXT},
};

} // namespace vkmock

