/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
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
 */

#pragma once

#include "error_message/error_location.h"

namespace vvl {

// Set of VUID that need to go between drawdispatch_validation.cpp and rest of CoreChecks
struct DrawDispatchVuid {
    // Save the action command here for reverse lookup so don't need to pass around both items
    const Func function;
    DrawDispatchVuid(Func func) : function(func){};
    Location loc() const { return Location(function); }

    const char* pipeline_bound_08606 = kVUIDUndefined;
    const char* pipeline_or_shaders_bound_08607 = kVUIDUndefined;
    const char* index_binding_07312 = kVUIDUndefined;
    const char* vertex_binding_04007 = kVUIDUndefined;
    const char* vertex_binding_null_04008 = kVUIDUndefined;
    const char* compatible_pipeline_08600 = kVUIDUndefined;
    const char* render_pass_compatible_02684 = kVUIDUndefined;
    const char* render_pass_began_08876 = kVUIDUndefined;
    const char* subpass_index_02685 = kVUIDUndefined;
    const char* sample_location_02689 = kVUIDUndefined;
    const char* linear_filter_sampler_04553 = kVUIDUndefined;
    const char* linear_mipmap_sampler_04770 = kVUIDUndefined;
    const char* linear_filter_sampler_09598 = kVUIDUndefined;
    const char* unnormalized_coordinates_09635 = kVUIDUndefined;
    const char* linear_mipmap_sampler_09599 = kVUIDUndefined;
    const char* cubic_sampler_02692 = kVUIDUndefined;
    const char* indirect_protected_cb_02711 = kVUIDUndefined;
    const char* indirect_contiguous_memory_02708 = kVUIDUndefined;
    const char* indirect_count_contiguous_memory_02714 = kVUIDUndefined;
    const char* indirect_buffer_bit_02290 = kVUIDUndefined;
    const char* indirect_count_buffer_bit_02715 = kVUIDUndefined;
    const char* indirect_count_offset_04129 = kVUIDUndefined;
    const char* viewport_count_03417 = kVUIDUndefined;
    const char* scissor_count_03418 = kVUIDUndefined;
    const char* viewport_scissor_count_03419 = kVUIDUndefined;
    const char* primitive_topology_class_07500 = kVUIDUndefined;
    const char* primitive_topology_patch_list_10286 = kVUIDUndefined;
    const char* corner_sampled_address_mode_02696 = kVUIDUndefined;
    const char* imageview_atomic_02691 = kVUIDUndefined;
    const char* bufferview_atomic_07888 = kVUIDUndefined;
    const char* push_constants_set_08602 = kVUIDUndefined;
    const char* image_subresources_render_pass_write_06537 = kVUIDUndefined;
    const char* image_subresources_subpass_read_09003 = kVUIDUndefined;
    const char* image_subresources_subpass_write_06539 = kVUIDUndefined;
    const char* sampler_imageview_type_08609 = kVUIDUndefined;
    const char* sampler_implicitLod_dref_proj_08610 = kVUIDUndefined;
    const char* sampler_bias_offset_08611 = kVUIDUndefined;
    const char* vertex_binding_attribute_02721 = kVUIDUndefined;
    const char* dynamic_state_setting_commands_08608 = kVUIDUndefined;
    const char* msrtss_rasterization_samples_07284 = kVUIDUndefined;
    const char* unprotected_command_buffer_02707 = kVUIDUndefined;
    const char* protected_command_buffer_02712 = kVUIDUndefined;
    const char* ray_query_protected_cb_03635 = kVUIDUndefined;
    const char* ray_query_04617 = kVUIDUndefined;
    // TODO: Some instance values are in VkBuffer. The validation in those Cmds is skipped.
    const char* max_multiview_instance_index_02688 = kVUIDUndefined;
    const char* img_filter_cubic_02693 = kVUIDUndefined;
    const char* filter_cubic_02694 = kVUIDUndefined;
    const char* filter_cubic_min_max_02695 = kVUIDUndefined;
    const char* viewport_count_primitive_shading_rate_04552 = kVUIDUndefined;
    const char* patch_control_points_04875 = kVUIDUndefined;
    const char* rasterizer_discard_enable_04876 = kVUIDUndefined;
    const char* depth_bias_enable_04877 = kVUIDUndefined;
    const char* logic_op_04878 = kVUIDUndefined;
    const char* primitive_restart_enable_04879 = kVUIDUndefined;
    const char* primitive_restart_list_09637 = kVUIDUndefined;
    const char* vertex_input_binding_stride_04913 = kVUIDUndefined;
    const char* vertex_input_04914 = kVUIDUndefined;
    const char* vertex_input_08734 = kVUIDUndefined;
    const char* blend_enable_04727 = kVUIDUndefined;
    const char* blend_dual_source_09239 = kVUIDUndefined;
    const char* dynamic_discard_rectangle_07751 = kVUIDUndefined;
    const char* dynamic_discard_rectangle_enable_07880 = kVUIDUndefined;
    const char* dynamic_discard_rectangle_mode_07881 = kVUIDUndefined;
    const char* dynamic_exclusive_scissor_enable_07878 = kVUIDUndefined;
    const char* dynamic_exclusive_scissor_07879 = kVUIDUndefined;
    const char* dynamic_color_write_enable_07749 = kVUIDUndefined;
    const char* dynamic_color_write_enable_count_07750 = kVUIDUndefined;
    const char* dynamic_attachment_feedback_loop_08877 = kVUIDUndefined;
    const char* dynamic_rendering_view_mask_06178 = kVUIDUndefined;
    const char* dynamic_rendering_color_count_06179 = kVUIDUndefined;
    const char* dynamic_rendering_color_formats_08910 = kVUIDUndefined;
    const char* dynamic_rendering_unused_attachments_08911 = kVUIDUndefined;
    const char* dynamic_rendering_undefined_color_formats_08912 = kVUIDUndefined;
    const char* dynamic_rendering_undefined_depth_format_08916 = kVUIDUndefined;
    const char* dynamic_rendering_undefined_stencil_format_08916 = kVUIDUndefined;
    const char* dynamic_rendering_depth_format_08914 = kVUIDUndefined;
    const char* dynamic_rendering_unused_attachments_08915 = kVUIDUndefined;
    const char* dynamic_rendering_stencil_format_08917 = kVUIDUndefined;
    const char* dynamic_rendering_unused_attachments_08918 = kVUIDUndefined;
    const char* dynamic_rendering_fsr_06183 = kVUIDUndefined;
    const char* dynamic_rendering_fdm_06184 = kVUIDUndefined;
    const char* dynamic_rendering_color_sample_06185 = kVUIDUndefined;
    const char* dynamic_rendering_depth_sample_06186 = kVUIDUndefined;
    const char* dynamic_rendering_stencil_sample_06187 = kVUIDUndefined;
    const char* dynamic_rendering_06198 = kVUIDUndefined;
    const char* dynamic_rendering_07285 = kVUIDUndefined;
    const char* dynamic_rendering_07286 = kVUIDUndefined;
    const char* dynamic_rendering_07287 = kVUIDUndefined;
    const char* dynamic_rendering_local_location_09548 = kVUIDUndefined;
    const char* dynamic_rendering_local_index_09549 = kVUIDUndefined;
    const char* dynamic_rendering_dithering_09642 = kVUIDUndefined;
    const char* dynamic_rendering_dithering_09643 = kVUIDUndefined;
    const char* image_view_access_64_04470 = kVUIDUndefined;
    const char* image_view_access_32_04471 = kVUIDUndefined;
    const char* image_view_sparse_64_04474 = kVUIDUndefined;
    const char* buffer_view_access_64_04472 = kVUIDUndefined;
    const char* buffer_view_access_32_04473 = kVUIDUndefined;
    const char* storage_image_read_without_format_07028 = kVUIDUndefined;
    const char* storage_image_write_without_format_07027 = kVUIDUndefined;
    const char* storage_texel_buffer_read_without_format_07030 = kVUIDUndefined;
    const char* storage_texel_buffer_write_without_format_07029 = kVUIDUndefined;
    const char* storage_image_write_texel_count_08795 = kVUIDUndefined;
    const char* storage_image_write_texel_count_08796 = kVUIDUndefined;
    const char* storage_texel_buffer_write_texel_count_04469 = kVUIDUndefined;
    const char* depth_compare_sample_06479 = kVUIDUndefined;
    const char* depth_read_only_06886 = kVUIDUndefined;
    const char* stencil_read_only_06887 = kVUIDUndefined;
    const char* dynamic_sample_locations_06666 = kVUIDUndefined;
    const char* dynamic_tessellation_domain_origin_07619 = kVUIDUndefined;
    const char* dynamic_depth_clamp_enable_07620 = kVUIDUndefined;
    const char* dynamic_polygon_mode_07621 = kVUIDUndefined;
    const char* dynamic_rasterization_samples_07622 = kVUIDUndefined;
    const char* dynamic_sample_mask_07623 = kVUIDUndefined;
    const char* dynamic_alpha_to_coverage_enable_07624 = kVUIDUndefined;
    const char* dynamic_alpha_to_coverage_component_08919 = kVUIDUndefined;
    const char* dynamic_alpha_to_one_enable_07625 = kVUIDUndefined;
    const char* dynamic_logic_op_enable_07626 = kVUIDUndefined;
    const char* dynamic_color_blend_enable_07476 = kVUIDUndefined;
    const char* dynamic_color_blend_equation_07477 = kVUIDUndefined;
    const char* dynamic_color_write_mask_07478 = kVUIDUndefined;
    const char* dynamic_rasterization_stream_07630 = kVUIDUndefined;
    const char* dynamic_conservative_rasterization_mode_07631 = kVUIDUndefined;
    const char* dynamic_extra_primitive_overestimation_size_07632 = kVUIDUndefined;
    const char* dynamic_depth_clip_enable_07633 = kVUIDUndefined;
    const char* dynamic_sample_locations_enable_07634 = kVUIDUndefined;
    const char* dynamic_color_blend_advanced_07479 = kVUIDUndefined;
    const char* dynamic_provoking_vertex_mode_07636 = kVUIDUndefined;
    const char* dynamic_line_rasterization_mode_07637 = kVUIDUndefined;
    const char* dynamic_line_stipple_enable_07638 = kVUIDUndefined;
    const char* dynamic_depth_clip_negative_one_to_one_07639 = kVUIDUndefined;
    const char* dynamic_viewport_w_scaling_enable_07640 = kVUIDUndefined;
    const char* dynamic_viewport_swizzle_07641 = kVUIDUndefined;
    const char* dynamic_coverage_to_color_enable_07642 = kVUIDUndefined;
    const char* dynamic_coverage_to_color_location_07643 = kVUIDUndefined;
    const char* dynamic_coverage_modulation_mode_07644 = kVUIDUndefined;
    const char* dynamic_coverage_modulation_table_enable_07645 = kVUIDUndefined;
    const char* dynamic_coverage_modulation_table_07646 = kVUIDUndefined;
    const char* dynamic_shading_rate_image_enable_07647 = kVUIDUndefined;
    const char* dynamic_representative_fragment_test_enable_07648 = kVUIDUndefined;
    const char* dynamic_coverage_reduction_mode_07649 = kVUIDUndefined;
    const char* dynamic_depth_clamp_control_09650 = kVUIDUndefined;
    const char* dynamic_viewport_07831 = kVUIDUndefined;
    const char* dynamic_scissor_07832 = kVUIDUndefined;
    const char* dynamic_depth_bias_07834 = kVUIDUndefined;
    const char* dynamic_line_width_07833 = kVUIDUndefined;
    const char* dynamic_line_stipple_ext_07849 = kVUIDUndefined;
    const char* dynamic_blend_constants_07835 = kVUIDUndefined;
    const char* dynamic_depth_bounds_07836 = kVUIDUndefined;
    const char* dynamic_depth_enable_08715 = kVUIDUndefined;
    const char* dynamic_stencil_compare_mask_07837 = kVUIDUndefined;
    const char* dynamic_stencil_write_mask_07838 = kVUIDUndefined;
    const char* dynamic_stencil_write_mask_08716 = kVUIDUndefined;
    const char* dynamic_stencil_reference_07839 = kVUIDUndefined;
    const char* dynamic_state_inherited_07850 = kVUIDUndefined;
    const char* dynamic_cull_mode_07840 = kVUIDUndefined;
    const char* dynamic_front_face_07841 = kVUIDUndefined;
    const char* dynamic_primitive_topology_07842 = kVUIDUndefined;
    const char* dynamic_depth_test_enable_07843 = kVUIDUndefined;
    const char* dynamic_depth_write_enable_07844 = kVUIDUndefined;
    const char* dynamic_depth_compare_op_07845 = kVUIDUndefined;
    const char* dynamic_depth_bound_test_enable_07846 = kVUIDUndefined;
    const char* dynamic_stencil_test_enable_07847 = kVUIDUndefined;
    const char* dynamic_stencil_op_07848 = kVUIDUndefined;
    const char* primitives_generated_06708 = kVUIDUndefined;
    const char* primitives_generated_streams_06709 = kVUIDUndefined;
    const char* mesh_shader_stages_06480 = kVUIDUndefined;
    const char* invalid_mesh_shader_stages_06481 = kVUIDUndefined;
    const char* missing_mesh_shader_stages_07080 = kVUIDUndefined;
    const char* descriptor_buffer_bit_set_08114 = kVUIDUndefined;
    const char* descriptor_buffer_bit_not_set_08115 = kVUIDUndefined;
    const char* descriptor_buffer_set_offset_missing_08117 = kVUIDUndefined;
    const char* image_ycbcr_sampled_06550 = kVUIDUndefined;
    const char* image_ycbcr_offset_06551 = kVUIDUndefined;
    const char* image_view_dim_07752 = kVUIDUndefined;
    const char* image_view_numeric_format_07753 = kVUIDUndefined;
    const char* stippled_rectangular_lines_07495 = kVUIDUndefined;
    const char* stippled_bresenham_lines_07496 = kVUIDUndefined;
    const char* stippled_smooth_lines_07497 = kVUIDUndefined;
    const char* stippled_default_strict_07498 = kVUIDUndefined;
    const char* viewport_and_scissor_with_count_03419 = kVUIDUndefined;
    const char* viewport_w_scaling_08636 = kVUIDUndefined;
    const char* shading_rate_palette_08637 = kVUIDUndefined;
    const char* external_format_resolve_09362 = kVUIDUndefined;
    const char* external_format_resolve_09363 = kVUIDUndefined;
    const char* external_format_resolve_09364 = kVUIDUndefined;
    const char* external_format_resolve_09365 = kVUIDUndefined;
    const char* external_format_resolve_09368 = kVUIDUndefined;
    const char* external_format_resolve_09369 = kVUIDUndefined;
    const char* external_format_resolve_09372 = kVUIDUndefined;
    const char* external_format_resolve_09366 = kVUIDUndefined;
    const char* external_format_resolve_09367 = kVUIDUndefined;
    const char* external_format_resolve_09370 = kVUIDUndefined;
    const char* external_format_resolve_09371 = kVUIDUndefined;
    const char* set_color_blend_enable_08643 = kVUIDUndefined;
    const char* set_rasterization_samples_08644 = kVUIDUndefined;
    const char* set_color_write_enable_08646 = kVUIDUndefined;
    const char* set_color_write_enable_08647 = kVUIDUndefined;
    const char* set_color_blend_enable_08657 = kVUIDUndefined;
    const char* set_color_blend_equation_08658 = kVUIDUndefined;
    const char* set_color_write_mask_08659 = kVUIDUndefined;
    const char* set_blend_operation_advance_09416 = kVUIDUndefined;
    const char* set_line_rasterization_mode_08666 = kVUIDUndefined;
    const char* set_line_rasterization_mode_08667 = kVUIDUndefined;
    const char* set_line_rasterization_mode_08668 = kVUIDUndefined;
    const char* set_line_stipple_enable_08669 = kVUIDUndefined;
    const char* set_line_stipple_enable_08670 = kVUIDUndefined;
    const char* set_line_stipple_enable_08671 = kVUIDUndefined;
    const char* vertex_shader_08684 = kVUIDUndefined;
    const char* tessellation_control_shader_08685 = kVUIDUndefined;
    const char* tessellation_evaluation_shader_08686 = kVUIDUndefined;
    const char* geometry_shader_08687 = kVUIDUndefined;
    const char* fragment_shader_08688 = kVUIDUndefined;
    const char* task_shader_08689 = kVUIDUndefined;
    const char* mesh_shader_08690 = kVUIDUndefined;
    const char* vert_mesh_shader_08693 = kVUIDUndefined;
    const char* task_mesh_shader_08694 = kVUIDUndefined;
    const char* task_mesh_shader_08695 = kVUIDUndefined;
    const char* vert_task_mesh_shader_08696 = kVUIDUndefined;
    const char* linked_shaders_08698 = kVUIDUndefined;
    const char* linked_shaders_08699 = kVUIDUndefined;
    const char* shaders_push_constants_08878 = kVUIDUndefined;
    const char* shaders_descriptor_layouts_08879 = kVUIDUndefined;
    const char* draw_shaders_no_task_mesh_08885 = kVUIDUndefined;
    const char* set_line_width_08617 = kVUIDUndefined;
    const char* set_line_width_08618 = kVUIDUndefined;
    const char* set_blend_constants_08621 = kVUIDUndefined;
    const char* set_line_width_08619 = kVUIDUndefined;
    const char* set_viewport_with_count_08642 = kVUIDUndefined;
    const char* alpha_component_word_08920 = kVUIDUndefined;
    const char* color_write_mask_09116 = kVUIDUndefined;
    const char* vertex_input_format_08936 = kVUIDUndefined;
    const char* vertex_input_format_08937 = kVUIDUndefined;
    const char* vertex_input_format_09203 = kVUIDUndefined;
    const char* vertex_input_format_07939 = kVUIDUndefined;
    const char* set_clip_space_w_scaling_04138 = kVUIDUndefined;
    const char* set_discard_rectangle_09236 = kVUIDUndefined;
    const char* set_viewport_coarse_sample_order_09233 = kVUIDUndefined;
    const char* set_viewport_shading_rate_palette_09234 = kVUIDUndefined;
    const char* set_fragment_shading_rate_09238 = kVUIDUndefined;
    const char* rasterization_samples_07935 = kVUIDUndefined;
    const char* mesh_shader_queries_07073 = kVUIDUndefined;
    const char* blend_advanced_07480 = kVUIDUndefined;
    const char* blend_feature_07470 = kVUIDUndefined;
    const char* color_attachment_08963 = kVUIDUndefined;
    const char* depth_attachment_08964 = kVUIDUndefined;
    const char* stencil_attachment_08965 = kVUIDUndefined;
    const char* sample_locations_07482 = kVUIDUndefined;
    const char* sample_locations_07483 = kVUIDUndefined;
    const char* sample_locations_07471 = kVUIDUndefined;
    const char* sample_locations_enable_07936 = kVUIDUndefined;
    const char* sample_locations_enable_07937 = kVUIDUndefined;
    const char* sample_locations_enable_07938 = kVUIDUndefined;
    const char* set_blend_advanced_09417 = kVUIDUndefined;
    const char* set_blend_equation_09418 = kVUIDUndefined;
    const char* set_color_write_09419 = kVUIDUndefined;
    const char* set_coverage_to_color_location_09420 = kVUIDUndefined;
    const char* set_viewport_swizzle_09421 = kVUIDUndefined;
    const char* convervative_rasterization_07499 = kVUIDUndefined;
    const char* sample_mask_07472 = kVUIDUndefined;
    const char* sample_mask_07473 = kVUIDUndefined;
    const char* rasterization_sampled_07474 = kVUIDUndefined;
    const char* sample_locations_enable_07484 = kVUIDUndefined;
    const char* sample_locations_enable_07485 = kVUIDUndefined;
    const char* sample_locations_enable_07486 = kVUIDUndefined;
    const char* sample_locations_enable_07487 = kVUIDUndefined;
    const char* xfb_queries_07074 = kVUIDUndefined;
    const char* pg_queries_07075 = kVUIDUndefined;
    const char* rasterization_samples_09211 = kVUIDUndefined;
    const char* dynamic_rendering_undefined_depth_format_08913 = kVUIDUndefined;
    const char* primitives_generated_query_07481 = kVUIDUndefined;
    const char* vertex_input_09461 = kVUIDUndefined;
    const char* vertex_input_09462 = kVUIDUndefined;
    const char* color_blend_enable_07627 = kVUIDUndefined;
    const char* color_blend_equation_07628 = kVUIDUndefined;
    const char* color_write_mask_07629 = kVUIDUndefined;
    const char* color_blend_advanced_07635 = kVUIDUndefined;
    const char* image_layout_09600 = kVUIDUndefined;
    // Ray tracing
    const char* ray_tracing_pipeline_stack_size_09458 = kVUIDUndefined;
};

const DrawDispatchVuid& GetDrawDispatchVuid(vvl::Func function);
}
