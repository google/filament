// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See valid_flag_values_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "stateless/stateless_validation.h"
#include <vulkan/vk_enum_string_helper.h>

// For flags, we can't use the VkFlag as it can't be templated (since it all resolves to a int).
// It is simpler for the caller to already check for both
//    - if zero is valid value or not
//    - if the value is even found in the API
// so the this file is only focused on checking for extensions being supported

vvl::Extensions stateless::Context::IsValidFlagValue(vvl::FlagBitmask flag_bitmask, VkFlags value) const {
    switch (flag_bitmask) {
        case vvl::FlagBitmask::VkAccessFlagBits:
            if (value & (VK_ACCESS_NONE)) {
                if (!IsExtEnabled(extensions.vk_khr_synchronization2)) {
                    return {vvl::Extension::_VK_KHR_synchronization2};
                }
            }
            if (value & (VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT |
                         VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_transform_feedback)) {
                    return {vvl::Extension::_VK_EXT_transform_feedback};
                }
            }
            if (value & (VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_conditional_rendering)) {
                    return {vvl::Extension::_VK_EXT_conditional_rendering};
                }
            }
            if (value & (VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_blend_operation_advanced)) {
                    return {vvl::Extension::_VK_EXT_blend_operation_advanced};
                }
            }
            if (value & (VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing) && !IsExtEnabled(extensions.vk_khr_acceleration_structure)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_acceleration_structure};
                }
            }
            if (value & (VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_fragment_shading_rate) && !IsExtEnabled(extensions.vk_nv_shading_rate_image)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate, vvl::Extension::_VK_NV_shading_rate_image};
                }
            }
            if (value & (VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV | VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_device_generated_commands) &&
                    !IsExtEnabled(extensions.vk_ext_device_generated_commands)) {
                    return {vvl::Extension::_VK_NV_device_generated_commands, vvl::Extension::_VK_EXT_device_generated_commands};
                }
            }
            return {};
        case vvl::FlagBitmask::VkImageAspectFlagBits:
            if (value & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_sampler_ycbcr_conversion)) {
                    return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
                }
            }
            if (value & (VK_IMAGE_ASPECT_NONE)) {
                if (!IsExtEnabled(extensions.vk_khr_maintenance4)) {
                    return {vvl::Extension::_VK_KHR_maintenance4};
                }
            }
            if (value & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                         VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_image_drm_format_modifier)) {
                    return {vvl::Extension::_VK_EXT_image_drm_format_modifier};
                }
            }
            return {};
        case vvl::FlagBitmask::VkFormatFeatureFlagBits:
            if (value & (VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_maintenance1};
                }
            }
            if (value & (VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT |
                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT |
                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT |
                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT |
                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT |
                         VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_sampler_ycbcr_conversion)) {
                    return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
                }
            }
            if (value & (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT)) {
                if (!IsExtEnabled(extensions.vk_ext_sampler_filter_minmax)) {
                    return {vvl::Extension::_VK_EXT_sampler_filter_minmax};
                }
            }
            if (value & (VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR | VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value & (VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_acceleration_structure)) {
                    return {vvl::Extension::_VK_KHR_acceleration_structure};
                }
            }
            if (value & (VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_img_filter_cubic) && !IsExtEnabled(extensions.vk_ext_filter_cubic)) {
                    return {vvl::Extension::_VK_IMG_filter_cubic, vvl::Extension::_VK_EXT_filter_cubic};
                }
            }
            if (value & (VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_fragment_shading_rate)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate};
                }
            }
            if (value & (VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR | VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            return {};
        case vvl::FlagBitmask::VkImageCreateFlagBits:
            if (value & (VK_IMAGE_CREATE_ALIAS_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_bind_memory2)) {
                    return {vvl::Extension::_VK_KHR_bind_memory2};
                }
            }
            if (value & (VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_device_group)) {
                    return {vvl::Extension::_VK_KHR_device_group};
                }
            }
            if (value & (VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_maintenance1};
                }
            }
            if (value & (VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_maintenance2)) {
                    return {vvl::Extension::_VK_KHR_maintenance2};
                }
            }
            if (value & (VK_IMAGE_CREATE_DISJOINT_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_sampler_ycbcr_conversion)) {
                    return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
                }
            }
            if (value & (VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_corner_sampled_image)) {
                    return {vvl::Extension::_VK_NV_corner_sampled_image};
                }
            }
            if (value & (VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_sample_locations)) {
                    return {vvl::Extension::_VK_EXT_sample_locations};
                }
            }
            if (value & (VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_multisampled_render_to_single_sampled)) {
                    return {vvl::Extension::_VK_EXT_multisampled_render_to_single_sampled};
                }
            }
            if (value & (VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_image_2d_view_of_3d)) {
                    return {vvl::Extension::_VK_EXT_image_2d_view_of_3d};
                }
            }
            if (value & (VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM)) {
                if (!IsExtEnabled(extensions.vk_qcom_fragment_density_map_offset)) {
                    return {vvl::Extension::_VK_QCOM_fragment_density_map_offset};
                }
            }
            if (value & (VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_video_maintenance1};
                }
            }
            return {};
        case vvl::FlagBitmask::VkImageUsageFlagBits:
            if (value & (VK_IMAGE_USAGE_HOST_TRANSFER_BIT)) {
                if (!IsExtEnabled(extensions.vk_ext_host_image_copy)) {
                    return {vvl::Extension::_VK_EXT_host_image_copy};
                }
            }
            if (value & (VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR |
                         VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value & (VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_fragment_shading_rate) && !IsExtEnabled(extensions.vk_nv_shading_rate_image)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate, vvl::Extension::_VK_NV_shading_rate_image};
                }
            }
            if (value & (VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR |
                         VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_attachment_feedback_loop_layout)) {
                    return {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout};
                }
            }
            if (value & (VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI)) {
                if (!IsExtEnabled(extensions.vk_huawei_invocation_mask)) {
                    return {vvl::Extension::_VK_HUAWEI_invocation_mask};
                }
            }
            if (value & (VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM | VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM)) {
                if (!IsExtEnabled(extensions.vk_qcom_image_processing)) {
                    return {vvl::Extension::_VK_QCOM_image_processing};
                }
            }
            if (value &
                (VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_quantization_map)) {
                    return {vvl::Extension::_VK_KHR_video_encode_quantization_map};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineStageFlagBits:
            if (value & (VK_PIPELINE_STAGE_NONE)) {
                if (!IsExtEnabled(extensions.vk_khr_synchronization2)) {
                    return {vvl::Extension::_VK_KHR_synchronization2};
                }
            }
            if (value & (VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_transform_feedback)) {
                    return {vvl::Extension::_VK_EXT_transform_feedback};
                }
            }
            if (value & (VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_conditional_rendering)) {
                    return {vvl::Extension::_VK_EXT_conditional_rendering};
                }
            }
            if (value & (VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing) && !IsExtEnabled(extensions.vk_khr_acceleration_structure)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_acceleration_structure};
                }
            }
            if (value & (VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing) && !IsExtEnabled(extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_fragment_shading_rate) && !IsExtEnabled(extensions.vk_nv_shading_rate_image)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate, vvl::Extension::_VK_NV_shading_rate_image};
                }
            }
            if (value & (VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_device_generated_commands) &&
                    !IsExtEnabled(extensions.vk_ext_device_generated_commands)) {
                    return {vvl::Extension::_VK_NV_device_generated_commands, vvl::Extension::_VK_EXT_device_generated_commands};
                }
            }
            if (value & (VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_nv_mesh_shader) && !IsExtEnabled(extensions.vk_ext_mesh_shader)) {
                    return {vvl::Extension::_VK_NV_mesh_shader, vvl::Extension::_VK_EXT_mesh_shader};
                }
            }
            return {};
        case vvl::FlagBitmask::VkMemoryMapFlagBits:
            if (value & (VK_MEMORY_MAP_PLACED_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_map_memory_placed)) {
                    return {vvl::Extension::_VK_EXT_map_memory_placed};
                }
            }
            return {};
        case vvl::FlagBitmask::VkEventCreateFlagBits:
            if (value & (VK_EVENT_CREATE_DEVICE_ONLY_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_synchronization2)) {
                    return {vvl::Extension::_VK_KHR_synchronization2};
                }
            }
            return {};
        case vvl::FlagBitmask::VkQueryPipelineStatisticFlagBits:
            if (value & (VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT |
                         VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_mesh_shader)) {
                    return {vvl::Extension::_VK_EXT_mesh_shader};
                }
            }
            if (value & (VK_QUERY_PIPELINE_STATISTIC_CLUSTER_CULLING_SHADER_INVOCATIONS_BIT_HUAWEI)) {
                if (!IsExtEnabled(extensions.vk_huawei_cluster_culling_shader)) {
                    return {vvl::Extension::_VK_HUAWEI_cluster_culling_shader};
                }
            }
            return {};
        case vvl::FlagBitmask::VkQueryResultFlagBits:
            if (value & (VK_QUERY_RESULT_WITH_STATUS_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_queue)) {
                    return {vvl::Extension::_VK_KHR_video_queue};
                }
            }
            return {};
        case vvl::FlagBitmask::VkBufferCreateFlagBits:
            if (value & (VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_buffer_device_address) &&
                    !IsExtEnabled(extensions.vk_ext_buffer_device_address)) {
                    return {vvl::Extension::_VK_KHR_buffer_device_address, vvl::Extension::_VK_EXT_buffer_device_address};
                }
            }
            if (value & (VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_video_maintenance1};
                }
            }
            return {};
        case vvl::FlagBitmask::VkBufferUsageFlagBits:
            if (value & (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_buffer_device_address) &&
                    !IsExtEnabled(extensions.vk_ext_buffer_device_address)) {
                    return {vvl::Extension::_VK_KHR_buffer_device_address, vvl::Extension::_VK_EXT_buffer_device_address};
                }
            }
            if (value & (VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR | VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value &
                (VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_transform_feedback)) {
                    return {vvl::Extension::_VK_EXT_transform_feedback};
                }
            }
            if (value & (VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_conditional_rendering)) {
                    return {vvl::Extension::_VK_EXT_conditional_rendering};
                }
            }
            if (value & (VK_BUFFER_USAGE_EXECUTION_GRAPH_SCRATCH_BIT_AMDX)) {
                if (!IsExtEnabled(extensions.vk_amdx_shader_enqueue)) {
                    return {vvl::Extension::_VK_AMDX_shader_enqueue};
                }
            }
            if (value & (VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                         VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_acceleration_structure)) {
                    return {vvl::Extension::_VK_KHR_acceleration_structure};
                }
            }
            if (value & (VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing) && !IsExtEnabled(extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR | VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                         VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT | VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            return {};
        case vvl::FlagBitmask::VkImageViewCreateFlagBits:
            if (value & (VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map2)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map2};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineCacheCreateFlagBits:
            if (value & (VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT)) {
                if (!IsExtEnabled(extensions.vk_ext_pipeline_creation_cache_control)) {
                    return {vvl::Extension::_VK_EXT_pipeline_creation_cache_control};
                }
            }
            if (value & (VK_PIPELINE_CACHE_CREATE_INTERNALLY_SYNCHRONIZED_MERGE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_maintenance8)) {
                    return {vvl::Extension::_VK_KHR_maintenance8};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineCreateFlagBits:
            if (value & (VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_device_group)) {
                    return {vvl::Extension::_VK_KHR_device_group};
                }
            }
            if (value &
                (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT | VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT)) {
                if (!IsExtEnabled(extensions.vk_ext_pipeline_creation_cache_control)) {
                    return {vvl::Extension::_VK_EXT_pipeline_creation_cache_control};
                }
            }
            if (value & (VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT)) {
                if (!IsExtEnabled(extensions.vk_ext_pipeline_protected_access)) {
                    return {vvl::Extension::_VK_EXT_pipeline_protected_access};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR | VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR |
                         VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing)) {
                    return {vvl::Extension::_VK_NV_ray_tracing};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_fragment_shading_rate)) {
                    return {vvl::Extension::_VK_KHR_fragment_shading_rate};
                }
            }
            if (value &
                (VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_pipeline_executable_properties)) {
                    return {vvl::Extension::_VK_KHR_pipeline_executable_properties};
                }
            }
            if (value & (VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_device_generated_commands)) {
                    return {vvl::Extension::_VK_NV_device_generated_commands};
                }
            }
            if (value & (VK_PIPELINE_CREATE_LIBRARY_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_pipeline_library)) {
                    return {vvl::Extension::_VK_KHR_pipeline_library};
                }
            }
            if (value & (VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT |
                         VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_graphics_pipeline_library)) {
                    return {vvl::Extension::_VK_EXT_graphics_pipeline_library};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing_motion_blur)) {
                    return {vvl::Extension::_VK_NV_ray_tracing_motion_blur};
                }
            }
            if (value & (VK_PIPELINE_CREATE_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
                         VK_PIPELINE_CREATE_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_attachment_feedback_loop_layout)) {
                    return {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            if (value & (VK_PIPELINE_CREATE_RAY_TRACING_DISPLACEMENT_MICROMAP_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_displacement_micromap)) {
                    return {vvl::Extension::_VK_NV_displacement_micromap};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineShaderStageCreateFlagBits:
            if (value & (VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT |
                         VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT)) {
                if (!IsExtEnabled(extensions.vk_ext_subgroup_size_control)) {
                    return {vvl::Extension::_VK_EXT_subgroup_size_control};
                }
            }
            return {};
        case vvl::FlagBitmask::VkShaderStageFlagBits:
            if (value == VK_SHADER_STAGE_ALL_GRAPHICS) {
                return {};
            }
            if (value == VK_SHADER_STAGE_ALL) {
                return {};
            }
            if (value & (VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                         VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing) && !IsExtEnabled(extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_nv_mesh_shader) && !IsExtEnabled(extensions.vk_ext_mesh_shader)) {
                    return {vvl::Extension::_VK_NV_mesh_shader, vvl::Extension::_VK_EXT_mesh_shader};
                }
            }
            if (value & (VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI)) {
                if (!IsExtEnabled(extensions.vk_huawei_subpass_shading)) {
                    return {vvl::Extension::_VK_HUAWEI_subpass_shading};
                }
            }
            if (value & (VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI)) {
                if (!IsExtEnabled(extensions.vk_huawei_cluster_culling_shader)) {
                    return {vvl::Extension::_VK_HUAWEI_cluster_culling_shader};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineDepthStencilStateCreateFlagBits:
            if (value & (VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
                         VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_arm_rasterization_order_attachment_access) &&
                    !IsExtEnabled(extensions.vk_ext_rasterization_order_attachment_access)) {
                    return {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
                            vvl::Extension::_VK_EXT_rasterization_order_attachment_access};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineColorBlendStateCreateFlagBits:
            if (value & (VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_arm_rasterization_order_attachment_access) &&
                    !IsExtEnabled(extensions.vk_ext_rasterization_order_attachment_access)) {
                    return {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
                            vvl::Extension::_VK_EXT_rasterization_order_attachment_access};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineLayoutCreateFlagBits:
            if (value & (VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_graphics_pipeline_library)) {
                    return {vvl::Extension::_VK_EXT_graphics_pipeline_library};
                }
            }
            return {};
        case vvl::FlagBitmask::VkSamplerCreateFlagBits:
            if (value & (VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT | VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_fragment_density_map)) {
                    return {vvl::Extension::_VK_EXT_fragment_density_map};
                }
            }
            if (value & (VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_non_seamless_cube_map)) {
                    return {vvl::Extension::_VK_EXT_non_seamless_cube_map};
                }
            }
            if (value & (VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM)) {
                if (!IsExtEnabled(extensions.vk_qcom_image_processing)) {
                    return {vvl::Extension::_VK_QCOM_image_processing};
                }
            }
            return {};
        case vvl::FlagBitmask::VkDescriptorPoolCreateFlagBits:
            if (value & (VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_indexing)) {
                    return {vvl::Extension::_VK_EXT_descriptor_indexing};
                }
            }
            if (value & (VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_valve_mutable_descriptor_type) &&
                    !IsExtEnabled(extensions.vk_ext_mutable_descriptor_type)) {
                    return {vvl::Extension::_VK_VALVE_mutable_descriptor_type, vvl::Extension::_VK_EXT_mutable_descriptor_type};
                }
            }
            if (value & (VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV |
                         VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_POOLS_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_descriptor_pool_overallocation)) {
                    return {vvl::Extension::_VK_NV_descriptor_pool_overallocation};
                }
            }
            return {};
        case vvl::FlagBitmask::VkDescriptorSetLayoutCreateFlagBits:
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_indexing)) {
                    return {vvl::Extension::_VK_EXT_descriptor_indexing};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_push_descriptor)) {
                    return {vvl::Extension::_VK_KHR_push_descriptor};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                         VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_INDIRECT_BINDABLE_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_device_generated_commands_compute)) {
                    return {vvl::Extension::_VK_NV_device_generated_commands_compute};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_valve_mutable_descriptor_type) &&
                    !IsExtEnabled(extensions.vk_ext_mutable_descriptor_type)) {
                    return {vvl::Extension::_VK_VALVE_mutable_descriptor_type, vvl::Extension::_VK_EXT_mutable_descriptor_type};
                }
            }
            if (value & (VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_per_stage_descriptor_set)) {
                    return {vvl::Extension::_VK_NV_per_stage_descriptor_set};
                }
            }
            return {};
        case vvl::FlagBitmask::VkDependencyFlagBits:
            if (value & (VK_DEPENDENCY_DEVICE_GROUP_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_device_group)) {
                    return {vvl::Extension::_VK_KHR_device_group};
                }
            }
            if (value & (VK_DEPENDENCY_VIEW_LOCAL_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_multiview)) {
                    return {vvl::Extension::_VK_KHR_multiview};
                }
            }
            if (value & (VK_DEPENDENCY_FEEDBACK_LOOP_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_attachment_feedback_loop_layout)) {
                    return {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout};
                }
            }
            if (value & (VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_maintenance8)) {
                    return {vvl::Extension::_VK_KHR_maintenance8};
                }
            }
            return {};
        case vvl::FlagBitmask::VkFramebufferCreateFlagBits:
            if (value & (VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_imageless_framebuffer)) {
                    return {vvl::Extension::_VK_KHR_imageless_framebuffer};
                }
            }
            return {};
        case vvl::FlagBitmask::VkRenderPassCreateFlagBits:
            if (value & (VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM)) {
                if (!IsExtEnabled(extensions.vk_qcom_render_pass_transform)) {
                    return {vvl::Extension::_VK_QCOM_render_pass_transform};
                }
            }
            return {};
        case vvl::FlagBitmask::VkSubpassDescriptionFlagBits:
            if (value &
                (VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX | VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX)) {
                if (!IsExtEnabled(extensions.vk_nvx_multiview_per_view_attributes)) {
                    return {vvl::Extension::_VK_NVX_multiview_per_view_attributes};
                }
            }
            if (value & (VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM | VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM)) {
                if (!IsExtEnabled(extensions.vk_qcom_render_pass_shader_resolve)) {
                    return {vvl::Extension::_VK_QCOM_render_pass_shader_resolve};
                }
            }
            if (value & (VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_EXT |
                         VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
                         VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_arm_rasterization_order_attachment_access) &&
                    !IsExtEnabled(extensions.vk_ext_rasterization_order_attachment_access)) {
                    return {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
                            vvl::Extension::_VK_EXT_rasterization_order_attachment_access};
                }
            }
            if (value & (VK_SUBPASS_DESCRIPTION_ENABLE_LEGACY_DITHERING_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_legacy_dithering)) {
                    return {vvl::Extension::_VK_EXT_legacy_dithering};
                }
            }
            return {};
        case vvl::FlagBitmask::VkMemoryAllocateFlagBits:
            if (value & (VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
                if (!IsExtEnabled(extensions.vk_khr_buffer_device_address)) {
                    return {vvl::Extension::_VK_KHR_buffer_device_address};
                }
            }
            return {};
        case vvl::FlagBitmask::VkExternalMemoryHandleTypeFlagBits:
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_external_memory_dma_buf)) {
                    return {vvl::Extension::_VK_EXT_external_memory_dma_buf};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
                if (!IsExtEnabled(extensions.vk_android_external_memory_android_hardware_buffer)) {
                    return {vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT |
                         VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_external_memory_host)) {
                    return {vvl::Extension::_VK_EXT_external_memory_host};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA)) {
                if (!IsExtEnabled(extensions.vk_fuchsia_external_memory)) {
                    return {vvl::Extension::_VK_FUCHSIA_external_memory};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_RDMA_ADDRESS_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_external_memory_rdma)) {
                    return {vvl::Extension::_VK_NV_external_memory_rdma};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_SCREEN_BUFFER_BIT_QNX)) {
                if (!IsExtEnabled(extensions.vk_qnx_external_memory_screen_buffer)) {
                    return {vvl::Extension::_VK_QNX_external_memory_screen_buffer};
                }
            }
            if (value & (VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLBUFFER_BIT_EXT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT |
                         VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLHEAP_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_external_memory_metal)) {
                    return {vvl::Extension::_VK_EXT_external_memory_metal};
                }
            }
            return {};
        case vvl::FlagBitmask::VkExternalSemaphoreHandleTypeFlagBits:
            if (value & (VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA)) {
                if (!IsExtEnabled(extensions.vk_fuchsia_external_semaphore)) {
                    return {vvl::Extension::_VK_FUCHSIA_external_semaphore};
                }
            }
            return {};
        case vvl::FlagBitmask::VkResolveModeFlagBits:
            if (value & (VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID)) {
                if (!IsExtEnabled(extensions.vk_android_external_format_resolve)) {
                    return {vvl::Extension::_VK_ANDROID_external_format_resolve};
                }
            }
            return {};
        case vvl::FlagBitmask::VkRenderingFlagBits:
            if (value & (VK_RENDERING_ENABLE_LEGACY_DITHERING_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_legacy_dithering)) {
                    return {vvl::Extension::_VK_EXT_legacy_dithering};
                }
            }
            if (value & (VK_RENDERING_CONTENTS_INLINE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_maintenance7) && !IsExtEnabled(extensions.vk_ext_nested_command_buffer)) {
                    return {vvl::Extension::_VK_KHR_maintenance7, vvl::Extension::_VK_EXT_nested_command_buffer};
                }
            }
            return {};
        case vvl::FlagBitmask::VkMemoryUnmapFlagBits:
            if (value & (VK_MEMORY_UNMAP_RESERVE_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_map_memory_placed)) {
                    return {vvl::Extension::_VK_EXT_map_memory_placed};
                }
            }
            return {};
        case vvl::FlagBitmask::VkSwapchainCreateFlagBitsKHR:
            if (value & (VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_device_group)) {
                    return {vvl::Extension::_VK_KHR_device_group};
                }
            }
            if (value & (VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_swapchain_mutable_format)) {
                    return {vvl::Extension::_VK_KHR_swapchain_mutable_format};
                }
            }
            if (value & (VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_swapchain_maintenance1)) {
                    return {vvl::Extension::_VK_EXT_swapchain_maintenance1};
                }
            }
            return {};
        case vvl::FlagBitmask::VkVideoSessionCreateFlagBitsKHR:
            if (value & (VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_PARAMETER_OPTIMIZATIONS_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_video_maintenance1};
                }
            }
            if (value & (VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR |
                         VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_quantization_map)) {
                    return {vvl::Extension::_VK_KHR_video_encode_quantization_map};
                }
            }
            if (value & (VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_maintenance2)) {
                    return {vvl::Extension::_VK_KHR_video_maintenance2};
                }
            }
            return {};
        case vvl::FlagBitmask::VkVideoSessionParametersCreateFlagBitsKHR:
            if (value & (VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_quantization_map)) {
                    return {vvl::Extension::_VK_KHR_video_encode_quantization_map};
                }
            }
            return {};
        case vvl::FlagBitmask::VkVideoCodingControlFlagBitsKHR:
            if (value &
                (VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR | VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            return {};
        case vvl::FlagBitmask::VkVideoEncodeFlagBitsKHR:
            if (value & (VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR | VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_quantization_map)) {
                    return {vvl::Extension::_VK_KHR_video_encode_quantization_map};
                }
            }
            return {};
        case vvl::FlagBitmask::VkDebugUtilsMessageTypeFlagBitsEXT:
            if (value & (VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_device_address_binding_report)) {
                    return {vvl::Extension::_VK_EXT_device_address_binding_report};
                }
            }
            return {};
        case vvl::FlagBitmask::VkGeometryInstanceFlagBitsKHR:
            if (value &
                (VK_GEOMETRY_INSTANCE_FORCE_OPACITY_MICROMAP_2_STATE_EXT | VK_GEOMETRY_INSTANCE_DISABLE_OPACITY_MICROMAPS_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            return {};
        case vvl::FlagBitmask::VkBuildAccelerationStructureFlagBitsKHR:
            if (value & (VK_BUILD_ACCELERATION_STRUCTURE_MOTION_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing_motion_blur)) {
                    return {vvl::Extension::_VK_NV_ray_tracing_motion_blur};
                }
            }
            if (value & (VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_UPDATE_EXT |
                         VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DISABLE_OPACITY_MICROMAPS_EXT |
                         VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_OPACITY_MICROMAP_DATA_UPDATE_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            if (value & (VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DISPLACEMENT_MICROMAP_UPDATE_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_displacement_micromap)) {
                    return {vvl::Extension::_VK_NV_displacement_micromap};
                }
            }
            if (value & (VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DATA_ACCESS_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_ray_tracing_position_fetch)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_position_fetch};
                }
            }
            return {};
        case vvl::FlagBitmask::VkShaderCreateFlagBitsEXT:
            if (value & (VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_device_generated_commands)) {
                    return {vvl::Extension::_VK_EXT_device_generated_commands};
                }
            }
            return {};
        case vvl::FlagBitmask::VkAccelerationStructureCreateFlagBitsKHR:
            if (value & (VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_ACCELERATION_STRUCTURE_CREATE_MOTION_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing_motion_blur)) {
                    return {vvl::Extension::_VK_NV_ray_tracing_motion_blur};
                }
            }
            return {};
        default:
            return {};
    }
}

vvl::Extensions stateless::Context::IsValidFlag64Value(vvl::FlagBitmask flag_bitmask, VkFlags64 value) const {
    switch (flag_bitmask) {
        case vvl::FlagBitmask::VkPipelineStageFlagBits2:
            if (value & (VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_SUBPASS_SHADER_BIT_HUAWEI)) {
                if (!IsExtEnabled(extensions.vk_huawei_subpass_shading)) {
                    return {vvl::Extension::_VK_HUAWEI_subpass_shading};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_INVOCATION_MASK_BIT_HUAWEI)) {
                if (!IsExtEnabled(extensions.vk_huawei_invocation_mask)) {
                    return {vvl::Extension::_VK_HUAWEI_invocation_mask};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_ray_tracing_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_maintenance1};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_CLUSTER_CULLING_SHADER_BIT_HUAWEI)) {
                if (!IsExtEnabled(extensions.vk_huawei_cluster_culling_shader)) {
                    return {vvl::Extension::_VK_HUAWEI_cluster_culling_shader};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_OPTICAL_FLOW_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_optical_flow)) {
                    return {vvl::Extension::_VK_NV_optical_flow};
                }
            }
            if (value & (VK_PIPELINE_STAGE_2_CONVERT_COOPERATIVE_VECTOR_MATRIX_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_cooperative_vector)) {
                    return {vvl::Extension::_VK_NV_cooperative_vector};
                }
            }
            return {};
        case vvl::FlagBitmask::VkAccessFlagBits2:
            if (value & (VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_decode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_decode_queue};
                }
            }
            if (value & (VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_video_encode_queue)) {
                    return {vvl::Extension::_VK_KHR_video_encode_queue};
                }
            }
            if (value & (VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_descriptor_buffer)) {
                    return {vvl::Extension::_VK_EXT_descriptor_buffer};
                }
            }
            if (value & (VK_ACCESS_2_INVOCATION_MASK_READ_BIT_HUAWEI)) {
                if (!IsExtEnabled(extensions.vk_huawei_invocation_mask)) {
                    return {vvl::Extension::_VK_HUAWEI_invocation_mask};
                }
            }
            if (value & (VK_ACCESS_2_SHADER_BINDING_TABLE_READ_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_ray_tracing_maintenance1)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_maintenance1};
                }
            }
            if (value & (VK_ACCESS_2_MICROMAP_READ_BIT_EXT | VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_opacity_micromap)) {
                    return {vvl::Extension::_VK_EXT_opacity_micromap};
                }
            }
            if (value & (VK_ACCESS_2_OPTICAL_FLOW_READ_BIT_NV | VK_ACCESS_2_OPTICAL_FLOW_WRITE_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_optical_flow)) {
                    return {vvl::Extension::_VK_NV_optical_flow};
                }
            }
            return {};
        case vvl::FlagBitmask::VkPipelineCreateFlagBits2:
            if (value & (VK_PIPELINE_CREATE_2_EXECUTION_GRAPH_BIT_AMDX)) {
                if (!IsExtEnabled(extensions.vk_amdx_shader_enqueue)) {
                    return {vvl::Extension::_VK_AMDX_shader_enqueue};
                }
            }
            if (value & (VK_PIPELINE_CREATE_2_RAY_TRACING_ALLOW_SPHERES_AND_LINEAR_SWEPT_SPHERES_BIT_NV)) {
                if (!IsExtEnabled(extensions.vk_nv_ray_tracing_linear_swept_spheres)) {
                    return {vvl::Extension::_VK_NV_ray_tracing_linear_swept_spheres};
                }
            }
            if (value & (VK_PIPELINE_CREATE_2_ENABLE_LEGACY_DITHERING_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_legacy_dithering)) {
                    return {vvl::Extension::_VK_EXT_legacy_dithering};
                }
            }
            if (value & (VK_PIPELINE_CREATE_2_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_ray_tracing_pipeline)) {
                    return {vvl::Extension::_VK_KHR_ray_tracing_pipeline};
                }
            }
            if (value & (VK_PIPELINE_CREATE_2_DISALLOW_OPACITY_MICROMAP_BIT_ARM)) {
                if (!IsExtEnabled(extensions.vk_arm_pipeline_opacity_micromap)) {
                    return {vvl::Extension::_VK_ARM_pipeline_opacity_micromap};
                }
            }
            if (value & (VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR)) {
                if (!IsExtEnabled(extensions.vk_khr_pipeline_binary)) {
                    return {vvl::Extension::_VK_KHR_pipeline_binary};
                }
            }
            if (value & (VK_PIPELINE_CREATE_2_INDIRECT_BINDABLE_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_device_generated_commands)) {
                    return {vvl::Extension::_VK_EXT_device_generated_commands};
                }
            }
            return {};
        case vvl::FlagBitmask::VkBufferUsageFlagBits2:
            if (value & (VK_BUFFER_USAGE_2_EXECUTION_GRAPH_SCRATCH_BIT_AMDX)) {
                if (!IsExtEnabled(extensions.vk_amdx_shader_enqueue)) {
                    return {vvl::Extension::_VK_AMDX_shader_enqueue};
                }
            }
            if (value & (VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT)) {
                if (!IsExtEnabled(extensions.vk_ext_device_generated_commands)) {
                    return {vvl::Extension::_VK_EXT_device_generated_commands};
                }
            }
            return {};
        default:
            return {};
    }
}

std::string stateless::Context::DescribeFlagBitmaskValue(vvl::FlagBitmask flag_bitmask, VkFlags value) const {
    switch (flag_bitmask) {
        case vvl::FlagBitmask::VkAccessFlagBits:
            return string_VkAccessFlags(value);
        case vvl::FlagBitmask::VkImageAspectFlagBits:
            return string_VkImageAspectFlags(value);
        case vvl::FlagBitmask::VkFormatFeatureFlagBits:
            return string_VkFormatFeatureFlags(value);
        case vvl::FlagBitmask::VkImageCreateFlagBits:
            return string_VkImageCreateFlags(value);
        case vvl::FlagBitmask::VkSampleCountFlagBits:
            return string_VkSampleCountFlags(value);
        case vvl::FlagBitmask::VkImageUsageFlagBits:
            return string_VkImageUsageFlags(value);
        case vvl::FlagBitmask::VkDeviceQueueCreateFlagBits:
            return string_VkDeviceQueueCreateFlags(value);
        case vvl::FlagBitmask::VkPipelineStageFlagBits:
            return string_VkPipelineStageFlags(value);
        case vvl::FlagBitmask::VkMemoryMapFlagBits:
            return string_VkMemoryMapFlags(value);
        case vvl::FlagBitmask::VkSparseMemoryBindFlagBits:
            return string_VkSparseMemoryBindFlags(value);
        case vvl::FlagBitmask::VkFenceCreateFlagBits:
            return string_VkFenceCreateFlags(value);
        case vvl::FlagBitmask::VkEventCreateFlagBits:
            return string_VkEventCreateFlags(value);
        case vvl::FlagBitmask::VkQueryPipelineStatisticFlagBits:
            return string_VkQueryPipelineStatisticFlags(value);
        case vvl::FlagBitmask::VkQueryResultFlagBits:
            return string_VkQueryResultFlags(value);
        case vvl::FlagBitmask::VkBufferCreateFlagBits:
            return string_VkBufferCreateFlags(value);
        case vvl::FlagBitmask::VkBufferUsageFlagBits:
            return string_VkBufferUsageFlags(value);
        case vvl::FlagBitmask::VkImageViewCreateFlagBits:
            return string_VkImageViewCreateFlags(value);
        case vvl::FlagBitmask::VkPipelineCacheCreateFlagBits:
            return string_VkPipelineCacheCreateFlags(value);
        case vvl::FlagBitmask::VkColorComponentFlagBits:
            return string_VkColorComponentFlags(value);
        case vvl::FlagBitmask::VkPipelineCreateFlagBits:
            return string_VkPipelineCreateFlags(value);
        case vvl::FlagBitmask::VkPipelineShaderStageCreateFlagBits:
            return string_VkPipelineShaderStageCreateFlags(value);
        case vvl::FlagBitmask::VkShaderStageFlagBits:
            return string_VkShaderStageFlags(value);
        case vvl::FlagBitmask::VkCullModeFlagBits:
            return string_VkCullModeFlags(value);
        case vvl::FlagBitmask::VkPipelineDepthStencilStateCreateFlagBits:
            return string_VkPipelineDepthStencilStateCreateFlags(value);
        case vvl::FlagBitmask::VkPipelineColorBlendStateCreateFlagBits:
            return string_VkPipelineColorBlendStateCreateFlags(value);
        case vvl::FlagBitmask::VkPipelineLayoutCreateFlagBits:
            return string_VkPipelineLayoutCreateFlags(value);
        case vvl::FlagBitmask::VkSamplerCreateFlagBits:
            return string_VkSamplerCreateFlags(value);
        case vvl::FlagBitmask::VkDescriptorPoolCreateFlagBits:
            return string_VkDescriptorPoolCreateFlags(value);
        case vvl::FlagBitmask::VkDescriptorSetLayoutCreateFlagBits:
            return string_VkDescriptorSetLayoutCreateFlags(value);
        case vvl::FlagBitmask::VkAttachmentDescriptionFlagBits:
            return string_VkAttachmentDescriptionFlags(value);
        case vvl::FlagBitmask::VkDependencyFlagBits:
            return string_VkDependencyFlags(value);
        case vvl::FlagBitmask::VkFramebufferCreateFlagBits:
            return string_VkFramebufferCreateFlags(value);
        case vvl::FlagBitmask::VkRenderPassCreateFlagBits:
            return string_VkRenderPassCreateFlags(value);
        case vvl::FlagBitmask::VkSubpassDescriptionFlagBits:
            return string_VkSubpassDescriptionFlags(value);
        case vvl::FlagBitmask::VkCommandPoolCreateFlagBits:
            return string_VkCommandPoolCreateFlags(value);
        case vvl::FlagBitmask::VkCommandPoolResetFlagBits:
            return string_VkCommandPoolResetFlags(value);
        case vvl::FlagBitmask::VkCommandBufferUsageFlagBits:
            return string_VkCommandBufferUsageFlags(value);
        case vvl::FlagBitmask::VkQueryControlFlagBits:
            return string_VkQueryControlFlags(value);
        case vvl::FlagBitmask::VkCommandBufferResetFlagBits:
            return string_VkCommandBufferResetFlags(value);
        case vvl::FlagBitmask::VkStencilFaceFlagBits:
            return string_VkStencilFaceFlags(value);
        case vvl::FlagBitmask::VkPeerMemoryFeatureFlagBits:
            return string_VkPeerMemoryFeatureFlags(value);
        case vvl::FlagBitmask::VkMemoryAllocateFlagBits:
            return string_VkMemoryAllocateFlags(value);
        case vvl::FlagBitmask::VkExternalMemoryHandleTypeFlagBits:
            return string_VkExternalMemoryHandleTypeFlags(value);
        case vvl::FlagBitmask::VkExternalFenceHandleTypeFlagBits:
            return string_VkExternalFenceHandleTypeFlags(value);
        case vvl::FlagBitmask::VkFenceImportFlagBits:
            return string_VkFenceImportFlags(value);
        case vvl::FlagBitmask::VkSemaphoreImportFlagBits:
            return string_VkSemaphoreImportFlags(value);
        case vvl::FlagBitmask::VkExternalSemaphoreHandleTypeFlagBits:
            return string_VkExternalSemaphoreHandleTypeFlags(value);
        case vvl::FlagBitmask::VkResolveModeFlagBits:
            return string_VkResolveModeFlags(value);
        case vvl::FlagBitmask::VkDescriptorBindingFlagBits:
            return string_VkDescriptorBindingFlags(value);
        case vvl::FlagBitmask::VkSemaphoreWaitFlagBits:
            return string_VkSemaphoreWaitFlags(value);
        case vvl::FlagBitmask::VkSubmitFlagBits:
            return string_VkSubmitFlags(value);
        case vvl::FlagBitmask::VkRenderingFlagBits:
            return string_VkRenderingFlags(value);
        case vvl::FlagBitmask::VkMemoryUnmapFlagBits:
            return string_VkMemoryUnmapFlags(value);
        case vvl::FlagBitmask::VkHostImageCopyFlagBits:
            return string_VkHostImageCopyFlags(value);
        case vvl::FlagBitmask::VkSurfaceTransformFlagBitsKHR:
            return string_VkSurfaceTransformFlagsKHR(value);
        case vvl::FlagBitmask::VkCompositeAlphaFlagBitsKHR:
            return string_VkCompositeAlphaFlagsKHR(value);
        case vvl::FlagBitmask::VkSwapchainCreateFlagBitsKHR:
            return string_VkSwapchainCreateFlagsKHR(value);
        case vvl::FlagBitmask::VkDeviceGroupPresentModeFlagBitsKHR:
            return string_VkDeviceGroupPresentModeFlagsKHR(value);
        case vvl::FlagBitmask::VkDisplayPlaneAlphaFlagBitsKHR:
            return string_VkDisplayPlaneAlphaFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoChromaSubsamplingFlagBitsKHR:
            return string_VkVideoChromaSubsamplingFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoComponentBitDepthFlagBitsKHR:
            return string_VkVideoComponentBitDepthFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoSessionCreateFlagBitsKHR:
            return string_VkVideoSessionCreateFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoSessionParametersCreateFlagBitsKHR:
            return string_VkVideoSessionParametersCreateFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoCodingControlFlagBitsKHR:
            return string_VkVideoCodingControlFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoDecodeUsageFlagBitsKHR:
            return string_VkVideoDecodeUsageFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoEncodeH264RateControlFlagBitsKHR:
            return string_VkVideoEncodeH264RateControlFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoEncodeH265RateControlFlagBitsKHR:
            return string_VkVideoEncodeH265RateControlFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoDecodeH264PictureLayoutFlagBitsKHR:
            return string_VkVideoDecodeH264PictureLayoutFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoEncodeFlagBitsKHR:
            return string_VkVideoEncodeFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoEncodeRateControlModeFlagBitsKHR:
            return string_VkVideoEncodeRateControlModeFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoEncodeFeedbackFlagBitsKHR:
            return string_VkVideoEncodeFeedbackFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoEncodeUsageFlagBitsKHR:
            return string_VkVideoEncodeUsageFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoEncodeContentFlagBitsKHR:
            return string_VkVideoEncodeContentFlagsKHR(value);
        case vvl::FlagBitmask::VkVideoEncodeAV1RateControlFlagBitsKHR:
            return string_VkVideoEncodeAV1RateControlFlagsKHR(value);
        case vvl::FlagBitmask::VkDebugReportFlagBitsEXT:
            return string_VkDebugReportFlagsEXT(value);
        case vvl::FlagBitmask::VkExternalMemoryHandleTypeFlagBitsNV:
            return string_VkExternalMemoryHandleTypeFlagsNV(value);
        case vvl::FlagBitmask::VkConditionalRenderingFlagBitsEXT:
            return string_VkConditionalRenderingFlagsEXT(value);
        case vvl::FlagBitmask::VkSurfaceCounterFlagBitsEXT:
            return string_VkSurfaceCounterFlagsEXT(value);
        case vvl::FlagBitmask::VkDebugUtilsMessageSeverityFlagBitsEXT:
            return string_VkDebugUtilsMessageSeverityFlagsEXT(value);
        case vvl::FlagBitmask::VkDebugUtilsMessageTypeFlagBitsEXT:
            return string_VkDebugUtilsMessageTypeFlagsEXT(value);
        case vvl::FlagBitmask::VkGeometryFlagBitsKHR:
            return string_VkGeometryFlagsKHR(value);
        case vvl::FlagBitmask::VkGeometryInstanceFlagBitsKHR:
            return string_VkGeometryInstanceFlagsKHR(value);
        case vvl::FlagBitmask::VkBuildAccelerationStructureFlagBitsKHR:
            return string_VkBuildAccelerationStructureFlagsKHR(value);
        case vvl::FlagBitmask::VkPresentScalingFlagBitsEXT:
            return string_VkPresentScalingFlagsEXT(value);
        case vvl::FlagBitmask::VkPresentGravityFlagBitsEXT:
            return string_VkPresentGravityFlagsEXT(value);
        case vvl::FlagBitmask::VkIndirectStateFlagBitsNV:
            return string_VkIndirectStateFlagsNV(value);
        case vvl::FlagBitmask::VkIndirectCommandsLayoutUsageFlagBitsNV:
            return string_VkIndirectCommandsLayoutUsageFlagsNV(value);
        case vvl::FlagBitmask::VkDeviceDiagnosticsConfigFlagBitsNV:
            return string_VkDeviceDiagnosticsConfigFlagsNV(value);
#ifdef VK_USE_PLATFORM_METAL_EXT
        case vvl::FlagBitmask::VkExportMetalObjectTypeFlagBitsEXT:
            return string_VkExportMetalObjectTypeFlagsEXT(value);
#endif  // VK_USE_PLATFORM_METAL_EXT
        case vvl::FlagBitmask::VkGraphicsPipelineLibraryFlagBitsEXT:
            return string_VkGraphicsPipelineLibraryFlagsEXT(value);
        case vvl::FlagBitmask::VkImageCompressionFlagBitsEXT:
            return string_VkImageCompressionFlagsEXT(value);
        case vvl::FlagBitmask::VkImageCompressionFixedRateFlagBitsEXT:
            return string_VkImageCompressionFixedRateFlagsEXT(value);
        case vvl::FlagBitmask::VkDeviceAddressBindingFlagBitsEXT:
            return string_VkDeviceAddressBindingFlagsEXT(value);
#ifdef VK_USE_PLATFORM_FUCHSIA
        case vvl::FlagBitmask::VkImageConstraintsInfoFlagBitsFUCHSIA:
            return string_VkImageConstraintsInfoFlagsFUCHSIA(value);
#endif  // VK_USE_PLATFORM_FUCHSIA
        case vvl::FlagBitmask::VkFrameBoundaryFlagBitsEXT:
            return string_VkFrameBoundaryFlagsEXT(value);
        case vvl::FlagBitmask::VkBuildMicromapFlagBitsEXT:
            return string_VkBuildMicromapFlagsEXT(value);
        case vvl::FlagBitmask::VkMicromapCreateFlagBitsEXT:
            return string_VkMicromapCreateFlagsEXT(value);
        case vvl::FlagBitmask::VkOpticalFlowGridSizeFlagBitsNV:
            return string_VkOpticalFlowGridSizeFlagsNV(value);
        case vvl::FlagBitmask::VkOpticalFlowUsageFlagBitsNV:
            return string_VkOpticalFlowUsageFlagsNV(value);
        case vvl::FlagBitmask::VkOpticalFlowSessionCreateFlagBitsNV:
            return string_VkOpticalFlowSessionCreateFlagsNV(value);
        case vvl::FlagBitmask::VkOpticalFlowExecuteFlagBitsNV:
            return string_VkOpticalFlowExecuteFlagsNV(value);
        case vvl::FlagBitmask::VkShaderCreateFlagBitsEXT:
            return string_VkShaderCreateFlagsEXT(value);
        case vvl::FlagBitmask::VkClusterAccelerationStructureAddressResolutionFlagBitsNV:
            return string_VkClusterAccelerationStructureAddressResolutionFlagsNV(value);
        case vvl::FlagBitmask::VkClusterAccelerationStructureClusterFlagBitsNV:
            return string_VkClusterAccelerationStructureClusterFlagsNV(value);
        case vvl::FlagBitmask::VkPartitionedAccelerationStructureInstanceFlagBitsNV:
            return string_VkPartitionedAccelerationStructureInstanceFlagsNV(value);
        case vvl::FlagBitmask::VkIndirectCommandsInputModeFlagBitsEXT:
            return string_VkIndirectCommandsInputModeFlagsEXT(value);
        case vvl::FlagBitmask::VkIndirectCommandsLayoutUsageFlagBitsEXT:
            return string_VkIndirectCommandsLayoutUsageFlagsEXT(value);
        case vvl::FlagBitmask::VkAccelerationStructureCreateFlagBitsKHR:
            return string_VkAccelerationStructureCreateFlagsKHR(value);

        default:
            std::stringstream ss;
            ss << "0x" << std::hex << value;
            return ss.str();
    }
}

std::string stateless::Context::DescribeFlagBitmaskValue64(vvl::FlagBitmask flag_bitmask, VkFlags64 value) const {
    switch (flag_bitmask) {
        case vvl::FlagBitmask::VkPipelineStageFlagBits2:
            return string_VkPipelineStageFlags2(value);
        case vvl::FlagBitmask::VkAccessFlagBits2:
            return string_VkAccessFlags2(value);
        case vvl::FlagBitmask::VkPipelineCreateFlagBits2:
            return string_VkPipelineCreateFlags2(value);
        case vvl::FlagBitmask::VkBufferUsageFlagBits2:
            return string_VkBufferUsageFlags2(value);
        case vvl::FlagBitmask::VkAccessFlagBits3KHR:
            return string_VkAccessFlags3KHR(value);
        case vvl::FlagBitmask::VkPhysicalDeviceSchedulingControlsFlagBitsARM:
            return string_VkPhysicalDeviceSchedulingControlsFlagsARM(value);
        case vvl::FlagBitmask::VkMemoryDecompressionMethodFlagBitsNV:
            return string_VkMemoryDecompressionMethodFlagsNV(value);

        default:
            std::stringstream ss;
            ss << "0x" << std::hex << value;
            return ss.str();
    }
}

// NOLINTEND
