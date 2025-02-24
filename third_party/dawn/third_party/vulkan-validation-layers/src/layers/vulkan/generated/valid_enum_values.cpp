// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See valid_enum_values_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

//  Checking for values is a 2 part process
//    1. Check if is valid at all
//    2. If invalid, spend more time to figure out how and what info to report to the user
//
//  While this might not seem ideal to compute the enabled extensions every time this function is called, the
//  other solution would be to build a list at vkCreateDevice time of all the valid values. This adds much higher
//  memory overhead.
//
//  Another key point to consider is being able to tell the user a value is invalid because it "doesn't exist" vs
//  "forgot to enable an extension" is VERY important

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPipelineCacheHeaderVersion value) const {
    switch (value) {
        case VK_PIPELINE_CACHE_HEADER_VERSION_ONE:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkImageLayout value) const {
    switch (value) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_GENERAL:
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return ValidValue::Valid;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return IsExtEnabled(extensions.vk_khr_maintenance2) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            return IsExtEnabled(extensions.vk_khr_separate_depth_stencil_layouts) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
            return IsExtEnabled(extensions.vk_khr_synchronization2) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
            return IsExtEnabled(extensions.vk_khr_dynamic_rendering_local_read) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return IsExtEnabled(extensions.vk_khr_swapchain) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
            return IsExtEnabled(extensions.vk_khr_video_decode_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            return IsExtEnabled(extensions.vk_khr_shared_presentable_image) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            return IsExtEnabled(extensions.vk_ext_fragment_density_map) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            return IsExtEnabled(extensions.vk_khr_fragment_shading_rate) || IsExtEnabled(extensions.vk_nv_shading_rate_image)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
            return IsExtEnabled(extensions.vk_khr_video_encode_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            return IsExtEnabled(extensions.vk_ext_attachment_feedback_loop_layout) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
            return IsExtEnabled(extensions.vk_khr_video_encode_quantization_map) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkObjectType value) const {
    switch (value) {
        case VK_OBJECT_TYPE_UNKNOWN:
        case VK_OBJECT_TYPE_INSTANCE:
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
        case VK_OBJECT_TYPE_DEVICE:
        case VK_OBJECT_TYPE_QUEUE:
        case VK_OBJECT_TYPE_SEMAPHORE:
        case VK_OBJECT_TYPE_COMMAND_BUFFER:
        case VK_OBJECT_TYPE_FENCE:
        case VK_OBJECT_TYPE_DEVICE_MEMORY:
        case VK_OBJECT_TYPE_BUFFER:
        case VK_OBJECT_TYPE_IMAGE:
        case VK_OBJECT_TYPE_EVENT:
        case VK_OBJECT_TYPE_QUERY_POOL:
        case VK_OBJECT_TYPE_BUFFER_VIEW:
        case VK_OBJECT_TYPE_IMAGE_VIEW:
        case VK_OBJECT_TYPE_SHADER_MODULE:
        case VK_OBJECT_TYPE_PIPELINE_CACHE:
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
        case VK_OBJECT_TYPE_RENDER_PASS:
        case VK_OBJECT_TYPE_PIPELINE:
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
        case VK_OBJECT_TYPE_SAMPLER:
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
        case VK_OBJECT_TYPE_FRAMEBUFFER:
        case VK_OBJECT_TYPE_COMMAND_POOL:
            return ValidValue::Valid;
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
            return IsExtEnabled(extensions.vk_khr_sampler_ycbcr_conversion) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
            return IsExtEnabled(extensions.vk_khr_descriptor_update_template) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT:
            return IsExtEnabled(extensions.vk_ext_private_data) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_SURFACE_KHR:
            return IsExtEnabled(extensions.vk_khr_surface) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
            return IsExtEnabled(extensions.vk_khr_swapchain) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DISPLAY_KHR:
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
            return IsExtEnabled(extensions.vk_khr_display) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
            return IsExtEnabled(extensions.vk_ext_debug_report) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_VIDEO_SESSION_KHR:
        case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR:
            return IsExtEnabled(extensions.vk_khr_video_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_CU_MODULE_NVX:
        case VK_OBJECT_TYPE_CU_FUNCTION_NVX:
            return IsExtEnabled(extensions.vk_nvx_binary_import) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
            return IsExtEnabled(extensions.vk_ext_debug_utils) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
            return IsExtEnabled(extensions.vk_khr_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
            return IsExtEnabled(extensions.vk_ext_validation_cache) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
            return IsExtEnabled(extensions.vk_nv_ray_tracing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL:
            return IsExtEnabled(extensions.vk_intel_performance_query) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:
            return IsExtEnabled(extensions.vk_khr_deferred_host_operations) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
            return IsExtEnabled(extensions.vk_nv_device_generated_commands) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_CUDA_MODULE_NV:
        case VK_OBJECT_TYPE_CUDA_FUNCTION_NV:
            return IsExtEnabled(extensions.vk_nv_cuda_kernel_launch) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA:
            return IsExtEnabled(extensions.vk_fuchsia_buffer_collection) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_MICROMAP_EXT:
            return IsExtEnabled(extensions.vk_ext_opacity_micromap) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV:
            return IsExtEnabled(extensions.vk_nv_optical_flow) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_SHADER_EXT:
            return IsExtEnabled(extensions.vk_ext_shader_object) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_PIPELINE_BINARY_KHR:
            return IsExtEnabled(extensions.vk_khr_pipeline_binary) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT:
        case VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT:
            return IsExtEnabled(extensions.vk_ext_device_generated_commands) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkFormat value) const {
    switch (value) {
        case VK_FORMAT_UNDEFINED:
        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            return ValidValue::Valid;
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return IsExtEnabled(extensions.vk_khr_sampler_ycbcr_conversion) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
            return IsExtEnabled(extensions.vk_ext_ycbcr_2plane_444_formats) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return IsExtEnabled(extensions.vk_ext_4444_formats) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            return IsExtEnabled(extensions.vk_ext_texture_compression_astc_hdr) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16:
        case VK_FORMAT_A8_UNORM:
            return IsExtEnabled(extensions.vk_khr_maintenance5) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
            return IsExtEnabled(extensions.vk_img_format_pvrtc) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_FORMAT_R16G16_SFIXED5_NV:
            return IsExtEnabled(extensions.vk_nv_optical_flow) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkImageTiling value) const {
    switch (value) {
        case VK_IMAGE_TILING_OPTIMAL:
        case VK_IMAGE_TILING_LINEAR:
            return ValidValue::Valid;
        case VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT:
            return IsExtEnabled(extensions.vk_ext_image_drm_format_modifier) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkImageType value) const {
    switch (value) {
        case VK_IMAGE_TYPE_1D:
        case VK_IMAGE_TYPE_2D:
        case VK_IMAGE_TYPE_3D:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkQueryType value) const {
    switch (value) {
        case VK_QUERY_TYPE_OCCLUSION:
        case VK_QUERY_TYPE_PIPELINE_STATISTICS:
        case VK_QUERY_TYPE_TIMESTAMP:
            return ValidValue::Valid;
        case VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR:
            return IsExtEnabled(extensions.vk_khr_video_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
            return IsExtEnabled(extensions.vk_ext_transform_feedback) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
            return IsExtEnabled(extensions.vk_khr_performance_query) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
            return IsExtEnabled(extensions.vk_khr_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV:
            return IsExtEnabled(extensions.vk_nv_ray_tracing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL:
            return IsExtEnabled(extensions.vk_intel_performance_query) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR:
            return IsExtEnabled(extensions.vk_khr_video_encode_queue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT:
            return IsExtEnabled(extensions.vk_ext_mesh_shader) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
            return IsExtEnabled(extensions.vk_ext_primitives_generated_query) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
            return IsExtEnabled(extensions.vk_khr_ray_tracing_maintenance1) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT:
        case VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT:
            return IsExtEnabled(extensions.vk_ext_opacity_micromap) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkSharingMode value) const {
    switch (value) {
        case VK_SHARING_MODE_EXCLUSIVE:
        case VK_SHARING_MODE_CONCURRENT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkComponentSwizzle value) const {
    switch (value) {
        case VK_COMPONENT_SWIZZLE_IDENTITY:
        case VK_COMPONENT_SWIZZLE_ZERO:
        case VK_COMPONENT_SWIZZLE_ONE:
        case VK_COMPONENT_SWIZZLE_R:
        case VK_COMPONENT_SWIZZLE_G:
        case VK_COMPONENT_SWIZZLE_B:
        case VK_COMPONENT_SWIZZLE_A:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkImageViewType value) const {
    switch (value) {
        case VK_IMAGE_VIEW_TYPE_1D:
        case VK_IMAGE_VIEW_TYPE_2D:
        case VK_IMAGE_VIEW_TYPE_3D:
        case VK_IMAGE_VIEW_TYPE_CUBE:
        case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkBlendFactor value) const {
    switch (value) {
        case VK_BLEND_FACTOR_ZERO:
        case VK_BLEND_FACTOR_ONE:
        case VK_BLEND_FACTOR_SRC_COLOR:
        case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
        case VK_BLEND_FACTOR_DST_COLOR:
        case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
        case VK_BLEND_FACTOR_SRC_ALPHA:
        case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        case VK_BLEND_FACTOR_DST_ALPHA:
        case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
        case VK_BLEND_FACTOR_CONSTANT_COLOR:
        case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
        case VK_BLEND_FACTOR_CONSTANT_ALPHA:
        case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
        case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
        case VK_BLEND_FACTOR_SRC1_COLOR:
        case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
        case VK_BLEND_FACTOR_SRC1_ALPHA:
        case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkBlendOp value) const {
    switch (value) {
        case VK_BLEND_OP_ADD:
        case VK_BLEND_OP_SUBTRACT:
        case VK_BLEND_OP_REVERSE_SUBTRACT:
        case VK_BLEND_OP_MIN:
        case VK_BLEND_OP_MAX:
            return ValidValue::Valid;
        case VK_BLEND_OP_ZERO_EXT:
        case VK_BLEND_OP_SRC_EXT:
        case VK_BLEND_OP_DST_EXT:
        case VK_BLEND_OP_SRC_OVER_EXT:
        case VK_BLEND_OP_DST_OVER_EXT:
        case VK_BLEND_OP_SRC_IN_EXT:
        case VK_BLEND_OP_DST_IN_EXT:
        case VK_BLEND_OP_SRC_OUT_EXT:
        case VK_BLEND_OP_DST_OUT_EXT:
        case VK_BLEND_OP_SRC_ATOP_EXT:
        case VK_BLEND_OP_DST_ATOP_EXT:
        case VK_BLEND_OP_XOR_EXT:
        case VK_BLEND_OP_MULTIPLY_EXT:
        case VK_BLEND_OP_SCREEN_EXT:
        case VK_BLEND_OP_OVERLAY_EXT:
        case VK_BLEND_OP_DARKEN_EXT:
        case VK_BLEND_OP_LIGHTEN_EXT:
        case VK_BLEND_OP_COLORDODGE_EXT:
        case VK_BLEND_OP_COLORBURN_EXT:
        case VK_BLEND_OP_HARDLIGHT_EXT:
        case VK_BLEND_OP_SOFTLIGHT_EXT:
        case VK_BLEND_OP_DIFFERENCE_EXT:
        case VK_BLEND_OP_EXCLUSION_EXT:
        case VK_BLEND_OP_INVERT_EXT:
        case VK_BLEND_OP_INVERT_RGB_EXT:
        case VK_BLEND_OP_LINEARDODGE_EXT:
        case VK_BLEND_OP_LINEARBURN_EXT:
        case VK_BLEND_OP_VIVIDLIGHT_EXT:
        case VK_BLEND_OP_LINEARLIGHT_EXT:
        case VK_BLEND_OP_PINLIGHT_EXT:
        case VK_BLEND_OP_HARDMIX_EXT:
        case VK_BLEND_OP_HSL_HUE_EXT:
        case VK_BLEND_OP_HSL_SATURATION_EXT:
        case VK_BLEND_OP_HSL_COLOR_EXT:
        case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
        case VK_BLEND_OP_PLUS_EXT:
        case VK_BLEND_OP_PLUS_CLAMPED_EXT:
        case VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT:
        case VK_BLEND_OP_PLUS_DARKER_EXT:
        case VK_BLEND_OP_MINUS_EXT:
        case VK_BLEND_OP_MINUS_CLAMPED_EXT:
        case VK_BLEND_OP_CONTRAST_EXT:
        case VK_BLEND_OP_INVERT_OVG_EXT:
        case VK_BLEND_OP_RED_EXT:
        case VK_BLEND_OP_GREEN_EXT:
        case VK_BLEND_OP_BLUE_EXT:
            return IsExtEnabled(extensions.vk_ext_blend_operation_advanced) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCompareOp value) const {
    switch (value) {
        case VK_COMPARE_OP_NEVER:
        case VK_COMPARE_OP_LESS:
        case VK_COMPARE_OP_EQUAL:
        case VK_COMPARE_OP_LESS_OR_EQUAL:
        case VK_COMPARE_OP_GREATER:
        case VK_COMPARE_OP_NOT_EQUAL:
        case VK_COMPARE_OP_GREATER_OR_EQUAL:
        case VK_COMPARE_OP_ALWAYS:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDynamicState value) const {
    switch (value) {
        case VK_DYNAMIC_STATE_VIEWPORT:
        case VK_DYNAMIC_STATE_SCISSOR:
        case VK_DYNAMIC_STATE_LINE_WIDTH:
        case VK_DYNAMIC_STATE_DEPTH_BIAS:
        case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
        case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
        case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
        case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
            return ValidValue::Valid;
        case VK_DYNAMIC_STATE_CULL_MODE:
        case VK_DYNAMIC_STATE_FRONT_FACE:
        case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
        case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
        case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
        case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
        case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
        case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
        case VK_DYNAMIC_STATE_STENCIL_OP:
            return IsExtEnabled(extensions.vk_ext_extended_dynamic_state) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
        case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
        case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
        case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
            return IsExtEnabled(extensions.vk_ext_extended_dynamic_state2) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_LINE_STIPPLE:
            return IsExtEnabled(extensions.vk_khr_line_rasterization) || IsExtEnabled(extensions.vk_ext_line_rasterization)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            return IsExtEnabled(extensions.vk_nv_clip_space_w_scaling) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
            return IsExtEnabled(extensions.vk_ext_discard_rectangles) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            return IsExtEnabled(extensions.vk_ext_sample_locations) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
            return IsExtEnabled(extensions.vk_khr_ray_tracing_pipeline) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
        case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            return IsExtEnabled(extensions.vk_nv_shading_rate_image) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            return IsExtEnabled(extensions.vk_nv_scissor_exclusive) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            return IsExtEnabled(extensions.vk_khr_fragment_shading_rate) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
            return IsExtEnabled(extensions.vk_ext_vertex_input_dynamic_state) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
            return IsExtEnabled(extensions.vk_ext_color_write_enable) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
        case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
        case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
        case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
        case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
        case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
        case VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
        case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
        case VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
        case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
        case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
        case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
        case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
        case VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
        case VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
        case VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
            return IsExtEnabled(extensions.vk_ext_extended_dynamic_state3) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
            return IsExtEnabled(extensions.vk_ext_attachment_feedback_loop_dynamic_state) ? ValidValue::Valid
                                                                                          : ValidValue::NoExtension;
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT:
            return IsExtEnabled(extensions.vk_ext_depth_clamp_control) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkFrontFace value) const {
    switch (value) {
        case VK_FRONT_FACE_COUNTER_CLOCKWISE:
        case VK_FRONT_FACE_CLOCKWISE:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkVertexInputRate value) const {
    switch (value) {
        case VK_VERTEX_INPUT_RATE_VERTEX:
        case VK_VERTEX_INPUT_RATE_INSTANCE:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPrimitiveTopology value) const {
    switch (value) {
        case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
        case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
        case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
        case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
        case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPolygonMode value) const {
    switch (value) {
        case VK_POLYGON_MODE_FILL:
        case VK_POLYGON_MODE_LINE:
        case VK_POLYGON_MODE_POINT:
            return ValidValue::Valid;
        case VK_POLYGON_MODE_FILL_RECTANGLE_NV:
            return IsExtEnabled(extensions.vk_nv_fill_rectangle) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkStencilOp value) const {
    switch (value) {
        case VK_STENCIL_OP_KEEP:
        case VK_STENCIL_OP_ZERO:
        case VK_STENCIL_OP_REPLACE:
        case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
        case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
        case VK_STENCIL_OP_INVERT:
        case VK_STENCIL_OP_INCREMENT_AND_WRAP:
        case VK_STENCIL_OP_DECREMENT_AND_WRAP:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkLogicOp value) const {
    switch (value) {
        case VK_LOGIC_OP_CLEAR:
        case VK_LOGIC_OP_AND:
        case VK_LOGIC_OP_AND_REVERSE:
        case VK_LOGIC_OP_COPY:
        case VK_LOGIC_OP_AND_INVERTED:
        case VK_LOGIC_OP_NO_OP:
        case VK_LOGIC_OP_XOR:
        case VK_LOGIC_OP_OR:
        case VK_LOGIC_OP_NOR:
        case VK_LOGIC_OP_EQUIVALENT:
        case VK_LOGIC_OP_INVERT:
        case VK_LOGIC_OP_OR_REVERSE:
        case VK_LOGIC_OP_COPY_INVERTED:
        case VK_LOGIC_OP_OR_INVERTED:
        case VK_LOGIC_OP_NAND:
        case VK_LOGIC_OP_SET:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkBorderColor value) const {
    switch (value) {
        case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
        case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:
        case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
        case VK_BORDER_COLOR_INT_OPAQUE_BLACK:
        case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
        case VK_BORDER_COLOR_INT_OPAQUE_WHITE:
            return ValidValue::Valid;
        case VK_BORDER_COLOR_FLOAT_CUSTOM_EXT:
        case VK_BORDER_COLOR_INT_CUSTOM_EXT:
            return IsExtEnabled(extensions.vk_ext_custom_border_color) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkFilter value) const {
    switch (value) {
        case VK_FILTER_NEAREST:
        case VK_FILTER_LINEAR:
            return ValidValue::Valid;
        case VK_FILTER_CUBIC_EXT:
            return IsExtEnabled(extensions.vk_img_filter_cubic) || IsExtEnabled(extensions.vk_ext_filter_cubic)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkSamplerAddressMode value) const {
    switch (value) {
        case VK_SAMPLER_ADDRESS_MODE_REPEAT:
        case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
        case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
        case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
            return ValidValue::Valid;
        case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
            return IsExtEnabled(extensions.vk_khr_sampler_mirror_clamp_to_edge) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkSamplerMipmapMode value) const {
    switch (value) {
        case VK_SAMPLER_MIPMAP_MODE_NEAREST:
        case VK_SAMPLER_MIPMAP_MODE_LINEAR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDescriptorType value) const {
    switch (value) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return ValidValue::Valid;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            return IsExtEnabled(extensions.vk_ext_inline_uniform_block) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return IsExtEnabled(extensions.vk_khr_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            return IsExtEnabled(extensions.vk_nv_ray_tracing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
            return IsExtEnabled(extensions.vk_qcom_image_processing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
            return IsExtEnabled(extensions.vk_valve_mutable_descriptor_type) ||
                           IsExtEnabled(extensions.vk_ext_mutable_descriptor_type)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        case VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV:
            return IsExtEnabled(extensions.vk_nv_partitioned_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAttachmentLoadOp value) const {
    switch (value) {
        case VK_ATTACHMENT_LOAD_OP_LOAD:
        case VK_ATTACHMENT_LOAD_OP_CLEAR:
        case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
            return ValidValue::Valid;
        case VK_ATTACHMENT_LOAD_OP_NONE:
            return IsExtEnabled(extensions.vk_khr_load_store_op_none) || IsExtEnabled(extensions.vk_ext_load_store_op_none)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAttachmentStoreOp value) const {
    switch (value) {
        case VK_ATTACHMENT_STORE_OP_STORE:
        case VK_ATTACHMENT_STORE_OP_DONT_CARE:
            return ValidValue::Valid;
        case VK_ATTACHMENT_STORE_OP_NONE:
            return IsExtEnabled(extensions.vk_khr_dynamic_rendering) || IsExtEnabled(extensions.vk_khr_load_store_op_none) ||
                           IsExtEnabled(extensions.vk_qcom_render_pass_store_ops) ||
                           IsExtEnabled(extensions.vk_ext_load_store_op_none)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPipelineBindPoint value) const {
    switch (value) {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            return ValidValue::Valid;
        case VK_PIPELINE_BIND_POINT_EXECUTION_GRAPH_AMDX:
            return IsExtEnabled(extensions.vk_amdx_shader_enqueue) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return IsExtEnabled(extensions.vk_nv_ray_tracing) || IsExtEnabled(extensions.vk_khr_ray_tracing_pipeline)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        case VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI:
            return IsExtEnabled(extensions.vk_huawei_subpass_shading) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCommandBufferLevel value) const {
    switch (value) {
        case VK_COMMAND_BUFFER_LEVEL_PRIMARY:
        case VK_COMMAND_BUFFER_LEVEL_SECONDARY:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkIndexType value) const {
    switch (value) {
        case VK_INDEX_TYPE_UINT16:
        case VK_INDEX_TYPE_UINT32:
            return ValidValue::Valid;
        case VK_INDEX_TYPE_UINT8:
            return IsExtEnabled(extensions.vk_khr_index_type_uint8) || IsExtEnabled(extensions.vk_ext_index_type_uint8)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        case VK_INDEX_TYPE_NONE_KHR:
            return IsExtEnabled(extensions.vk_nv_ray_tracing) || IsExtEnabled(extensions.vk_khr_acceleration_structure)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkSubpassContents value) const {
    switch (value) {
        case VK_SUBPASS_CONTENTS_INLINE:
        case VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS:
            return ValidValue::Valid;
        case VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR:
            return IsExtEnabled(extensions.vk_khr_maintenance7) || IsExtEnabled(extensions.vk_ext_nested_command_buffer)
                       ? ValidValue::Valid
                       : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkTessellationDomainOrigin value) const {
    switch (value) {
        case VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT:
        case VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkSamplerYcbcrModelConversion value) const {
    switch (value) {
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY:
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY:
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709:
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601:
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkSamplerYcbcrRange value) const {
    switch (value) {
        case VK_SAMPLER_YCBCR_RANGE_ITU_FULL:
        case VK_SAMPLER_YCBCR_RANGE_ITU_NARROW:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkChromaLocation value) const {
    switch (value) {
        case VK_CHROMA_LOCATION_COSITED_EVEN:
        case VK_CHROMA_LOCATION_MIDPOINT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDescriptorUpdateTemplateType value) const {
    switch (value) {
        case VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET:
            return ValidValue::Valid;
        case VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS:
            return IsExtEnabled(extensions.vk_khr_push_descriptor) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkSamplerReductionMode value) const {
    switch (value) {
        case VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE:
        case VK_SAMPLER_REDUCTION_MODE_MIN:
        case VK_SAMPLER_REDUCTION_MODE_MAX:
            return ValidValue::Valid;
        case VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_RANGECLAMP_QCOM:
            return IsExtEnabled(extensions.vk_qcom_filter_cubic_clamp) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkSemaphoreType value) const {
    switch (value) {
        case VK_SEMAPHORE_TYPE_BINARY:
        case VK_SEMAPHORE_TYPE_TIMELINE:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPipelineRobustnessBufferBehavior value) const {
    switch (value) {
        case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT:
        case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED:
        case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS:
        case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPipelineRobustnessImageBehavior value) const {
    switch (value) {
        case VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT:
        case VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED:
        case VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS:
        case VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkQueueGlobalPriority value) const {
    switch (value) {
        case VK_QUEUE_GLOBAL_PRIORITY_LOW:
        case VK_QUEUE_GLOBAL_PRIORITY_MEDIUM:
        case VK_QUEUE_GLOBAL_PRIORITY_HIGH:
        case VK_QUEUE_GLOBAL_PRIORITY_REALTIME:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkLineRasterizationMode value) const {
    switch (value) {
        case VK_LINE_RASTERIZATION_MODE_DEFAULT:
        case VK_LINE_RASTERIZATION_MODE_RECTANGULAR:
        case VK_LINE_RASTERIZATION_MODE_BRESENHAM:
        case VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPresentModeKHR value) const {
    switch (value) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
        case VK_PRESENT_MODE_MAILBOX_KHR:
        case VK_PRESENT_MODE_FIFO_KHR:
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            return ValidValue::Valid;
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            return IsExtEnabled(extensions.vk_khr_shared_presentable_image) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_PRESENT_MODE_FIFO_LATEST_READY_EXT:
            return IsExtEnabled(extensions.vk_ext_present_mode_fifo_latest_ready) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkColorSpaceKHR value) const {
    switch (value) {
        case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
            return ValidValue::Valid;
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
        case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_BT709_LINEAR_EXT:
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
        case VK_COLOR_SPACE_DOLBYVISION_EXT:
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
        case VK_COLOR_SPACE_PASS_THROUGH_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
            return IsExtEnabled(extensions.vk_ext_swapchain_colorspace) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:
            return IsExtEnabled(extensions.vk_amd_display_native_hdr) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkFragmentShadingRateCombinerOpKHR value) const {
    switch (value) {
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR:
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR:
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MIN_KHR:
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR:
        case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkVideoEncodeTuningModeKHR value) const {
    switch (value) {
        case VK_VIDEO_ENCODE_TUNING_MODE_DEFAULT_KHR:
        case VK_VIDEO_ENCODE_TUNING_MODE_HIGH_QUALITY_KHR:
        case VK_VIDEO_ENCODE_TUNING_MODE_LOW_LATENCY_KHR:
        case VK_VIDEO_ENCODE_TUNING_MODE_ULTRA_LOW_LATENCY_KHR:
        case VK_VIDEO_ENCODE_TUNING_MODE_LOSSLESS_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkComponentTypeKHR value) const {
    switch (value) {
        case VK_COMPONENT_TYPE_FLOAT16_KHR:
        case VK_COMPONENT_TYPE_FLOAT32_KHR:
        case VK_COMPONENT_TYPE_FLOAT64_KHR:
        case VK_COMPONENT_TYPE_SINT8_KHR:
        case VK_COMPONENT_TYPE_SINT16_KHR:
        case VK_COMPONENT_TYPE_SINT32_KHR:
        case VK_COMPONENT_TYPE_SINT64_KHR:
        case VK_COMPONENT_TYPE_UINT8_KHR:
        case VK_COMPONENT_TYPE_UINT16_KHR:
        case VK_COMPONENT_TYPE_UINT32_KHR:
        case VK_COMPONENT_TYPE_UINT64_KHR:
        case VK_COMPONENT_TYPE_SINT8_PACKED_NV:
        case VK_COMPONENT_TYPE_UINT8_PACKED_NV:
        case VK_COMPONENT_TYPE_FLOAT_E4M3_NV:
        case VK_COMPONENT_TYPE_FLOAT_E5M2_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkVideoEncodeAV1PredictionModeKHR value) const {
    switch (value) {
        case VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_INTRA_ONLY_KHR:
        case VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR:
        case VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR:
        case VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkVideoEncodeAV1RateControlGroupKHR value) const {
    switch (value) {
        case VK_VIDEO_ENCODE_AV1_RATE_CONTROL_GROUP_INTRA_KHR:
        case VK_VIDEO_ENCODE_AV1_RATE_CONTROL_GROUP_PREDICTIVE_KHR:
        case VK_VIDEO_ENCODE_AV1_RATE_CONTROL_GROUP_BIPREDICTIVE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkTimeDomainKHR value) const {
    switch (value) {
        case VK_TIME_DOMAIN_DEVICE_KHR:
        case VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR:
        case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_KHR:
        case VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDebugReportObjectTypeEXT value) const {
    switch (value) {
        case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT:
            return ValidValue::Valid;
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT:
            return IsExtEnabled(extensions.vk_khr_sampler_ycbcr_conversion) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT:
            return IsExtEnabled(extensions.vk_khr_descriptor_update_template) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT:
            return IsExtEnabled(extensions.vk_nvx_binary_import) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT:
            return IsExtEnabled(extensions.vk_khr_acceleration_structure) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT:
            return IsExtEnabled(extensions.vk_nv_ray_tracing) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_MODULE_NV_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_FUNCTION_NV_EXT:
            return IsExtEnabled(extensions.vk_nv_cuda_kernel_launch) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT:
            return IsExtEnabled(extensions.vk_fuchsia_buffer_collection) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkRasterizationOrderAMD value) const {
    switch (value) {
        case VK_RASTERIZATION_ORDER_STRICT_AMD:
        case VK_RASTERIZATION_ORDER_RELAXED_AMD:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkShaderInfoTypeAMD value) const {
    switch (value) {
        case VK_SHADER_INFO_TYPE_STATISTICS_AMD:
        case VK_SHADER_INFO_TYPE_BINARY_AMD:
        case VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkValidationCheckEXT value) const {
    switch (value) {
        case VK_VALIDATION_CHECK_ALL_EXT:
        case VK_VALIDATION_CHECK_SHADERS_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDisplayPowerStateEXT value) const {
    switch (value) {
        case VK_DISPLAY_POWER_STATE_OFF_EXT:
        case VK_DISPLAY_POWER_STATE_SUSPEND_EXT:
        case VK_DISPLAY_POWER_STATE_ON_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDeviceEventTypeEXT value) const {
    switch (value) {
        case VK_DEVICE_EVENT_TYPE_DISPLAY_HOTPLUG_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDisplayEventTypeEXT value) const {
    switch (value) {
        case VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkViewportCoordinateSwizzleNV value) const {
    switch (value) {
        case VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_X_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Y_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_Z_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV:
        case VK_VIEWPORT_COORDINATE_SWIZZLE_NEGATIVE_W_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDiscardRectangleModeEXT value) const {
    switch (value) {
        case VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT:
        case VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkConservativeRasterizationModeEXT value) const {
    switch (value) {
        case VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT:
        case VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT:
        case VK_CONSERVATIVE_RASTERIZATION_MODE_UNDERESTIMATE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkBlendOverlapEXT value) const {
    switch (value) {
        case VK_BLEND_OVERLAP_UNCORRELATED_EXT:
        case VK_BLEND_OVERLAP_DISJOINT_EXT:
        case VK_BLEND_OVERLAP_CONJOINT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCoverageModulationModeNV value) const {
    switch (value) {
        case VK_COVERAGE_MODULATION_MODE_NONE_NV:
        case VK_COVERAGE_MODULATION_MODE_RGB_NV:
        case VK_COVERAGE_MODULATION_MODE_ALPHA_NV:
        case VK_COVERAGE_MODULATION_MODE_RGBA_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkShadingRatePaletteEntryNV value) const {
    switch (value) {
        case VK_SHADING_RATE_PALETTE_ENTRY_NO_INVOCATIONS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_16_INVOCATIONS_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_8_INVOCATIONS_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_4_INVOCATIONS_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_2_INVOCATIONS_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV:
        case VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCoarseSampleOrderTypeNV value) const {
    switch (value) {
        case VK_COARSE_SAMPLE_ORDER_TYPE_DEFAULT_NV:
        case VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV:
        case VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV:
        case VK_COARSE_SAMPLE_ORDER_TYPE_SAMPLE_MAJOR_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkRayTracingShaderGroupTypeKHR value) const {
    switch (value) {
        case VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR:
        case VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR:
        case VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkGeometryTypeKHR value) const {
    switch (value) {
        case VK_GEOMETRY_TYPE_TRIANGLES_KHR:
        case VK_GEOMETRY_TYPE_AABBS_KHR:
        case VK_GEOMETRY_TYPE_INSTANCES_KHR:
            return ValidValue::Valid;
        case VK_GEOMETRY_TYPE_SPHERES_NV:
        case VK_GEOMETRY_TYPE_LINEAR_SWEPT_SPHERES_NV:
            return IsExtEnabled(extensions.vk_nv_ray_tracing_linear_swept_spheres) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAccelerationStructureTypeKHR value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR:
        case VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR:
        case VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCopyAccelerationStructureModeKHR value) const {
    switch (value) {
        case VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR:
        case VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR:
        case VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR:
        case VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAccelerationStructureMemoryRequirementsTypeNV value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV:
        case VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV:
        case VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkMemoryOverallocationBehaviorAMD value) const {
    switch (value) {
        case VK_MEMORY_OVERALLOCATION_BEHAVIOR_DEFAULT_AMD:
        case VK_MEMORY_OVERALLOCATION_BEHAVIOR_ALLOWED_AMD:
        case VK_MEMORY_OVERALLOCATION_BEHAVIOR_DISALLOWED_AMD:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPerformanceConfigurationTypeINTEL value) const {
    switch (value) {
        case VK_PERFORMANCE_CONFIGURATION_TYPE_COMMAND_QUEUE_METRICS_DISCOVERY_ACTIVATED_INTEL:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkQueryPoolSamplingModeINTEL value) const {
    switch (value) {
        case VK_QUERY_POOL_SAMPLING_MODE_MANUAL_INTEL:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPerformanceOverrideTypeINTEL value) const {
    switch (value) {
        case VK_PERFORMANCE_OVERRIDE_TYPE_NULL_HARDWARE_INTEL:
        case VK_PERFORMANCE_OVERRIDE_TYPE_FLUSH_GPU_CACHES_INTEL:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPerformanceParameterTypeINTEL value) const {
    switch (value) {
        case VK_PERFORMANCE_PARAMETER_TYPE_HW_COUNTERS_SUPPORTED_INTEL:
        case VK_PERFORMANCE_PARAMETER_TYPE_STREAM_MARKER_VALID_BITS_INTEL:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkValidationFeatureEnableEXT value) const {
    switch (value) {
        case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT:
        case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT:
        case VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT:
        case VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT:
        case VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkValidationFeatureDisableEXT value) const {
    switch (value) {
        case VK_VALIDATION_FEATURE_DISABLE_ALL_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT:
        case VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCoverageReductionModeNV value) const {
    switch (value) {
        case VK_COVERAGE_REDUCTION_MODE_MERGE_NV:
        case VK_COVERAGE_REDUCTION_MODE_TRUNCATE_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkProvokingVertexModeEXT value) const {
    switch (value) {
        case VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT:
        case VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
template <>
ValidValue stateless::Context::IsValidEnumValue(VkFullScreenExclusiveEXT value) const {
    switch (value) {
        case VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT:
        case VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT:
        case VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT:
        case VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

template <>
ValidValue stateless::Context::IsValidEnumValue(VkIndirectCommandsTokenTypeNV value) const {
    switch (value) {
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SHADER_GROUP_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_STATE_FLAGS_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_TASKS_NV:
            return ValidValue::Valid;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV:
            return IsExtEnabled(extensions.vk_ext_mesh_shader) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PIPELINE_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_NV:
            return IsExtEnabled(extensions.vk_nv_device_generated_commands_compute) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDepthBiasRepresentationEXT value) const {
    switch (value) {
        case VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORMAT_EXT:
        case VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT:
        case VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkFragmentShadingRateTypeNV value) const {
    switch (value) {
        case VK_FRAGMENT_SHADING_RATE_TYPE_FRAGMENT_SIZE_NV:
        case VK_FRAGMENT_SHADING_RATE_TYPE_ENUMS_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkFragmentShadingRateNV value) const {
    switch (value) {
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_1X2_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X1_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X2_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_2X4_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_4X2_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_1_INVOCATION_PER_4X4_PIXELS_NV:
        case VK_FRAGMENT_SHADING_RATE_2_INVOCATIONS_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_4_INVOCATIONS_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_8_INVOCATIONS_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_16_INVOCATIONS_PER_PIXEL_NV:
        case VK_FRAGMENT_SHADING_RATE_NO_INVOCATIONS_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAccelerationStructureMotionInstanceTypeNV value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_STATIC_NV:
        case VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_MATRIX_MOTION_NV:
        case VK_ACCELERATION_STRUCTURE_MOTION_INSTANCE_TYPE_SRT_MOTION_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDeviceFaultAddressTypeEXT value) const {
    switch (value) {
        case VK_DEVICE_FAULT_ADDRESS_TYPE_NONE_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_READ_INVALID_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_WRITE_INVALID_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_EXECUTE_INVALID_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_UNKNOWN_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_INVALID_EXT:
        case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_FAULT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDeviceFaultVendorBinaryHeaderVersionEXT value) const {
    switch (value) {
        case VK_DEVICE_FAULT_VENDOR_BINARY_HEADER_VERSION_ONE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDeviceAddressBindingTypeEXT value) const {
    switch (value) {
        case VK_DEVICE_ADDRESS_BINDING_TYPE_BIND_EXT:
        case VK_DEVICE_ADDRESS_BINDING_TYPE_UNBIND_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkMicromapTypeEXT value) const {
    switch (value) {
        case VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT:
            return ValidValue::Valid;
        case VK_MICROMAP_TYPE_DISPLACEMENT_MICROMAP_NV:
            return IsExtEnabled(extensions.vk_nv_displacement_micromap) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkBuildMicromapModeEXT value) const {
    switch (value) {
        case VK_BUILD_MICROMAP_MODE_BUILD_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCopyMicromapModeEXT value) const {
    switch (value) {
        case VK_COPY_MICROMAP_MODE_CLONE_EXT:
        case VK_COPY_MICROMAP_MODE_SERIALIZE_EXT:
        case VK_COPY_MICROMAP_MODE_DESERIALIZE_EXT:
        case VK_COPY_MICROMAP_MODE_COMPACT_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAccelerationStructureCompatibilityKHR value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_COMPATIBILITY_COMPATIBLE_KHR:
        case VK_ACCELERATION_STRUCTURE_COMPATIBILITY_INCOMPATIBLE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAccelerationStructureBuildTypeKHR value) const {
    switch (value) {
        case VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR:
        case VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR:
        case VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_OR_DEVICE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkRayTracingLssIndexingModeNV value) const {
    switch (value) {
        case VK_RAY_TRACING_LSS_INDEXING_MODE_LIST_NV:
        case VK_RAY_TRACING_LSS_INDEXING_MODE_SUCCESSIVE_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkRayTracingLssPrimitiveEndCapsModeNV value) const {
    switch (value) {
        case VK_RAY_TRACING_LSS_PRIMITIVE_END_CAPS_MODE_NONE_NV:
        case VK_RAY_TRACING_LSS_PRIMITIVE_END_CAPS_MODE_CHAINED_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDirectDriverLoadingModeLUNARG value) const {
    switch (value) {
        case VK_DIRECT_DRIVER_LOADING_MODE_EXCLUSIVE_LUNARG:
        case VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkOpticalFlowPerformanceLevelNV value) const {
    switch (value) {
        case VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_UNKNOWN_NV:
        case VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_SLOW_NV:
        case VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_MEDIUM_NV:
        case VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_FAST_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkOpticalFlowSessionBindingPointNV value) const {
    switch (value) {
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_UNKNOWN_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_INPUT_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_REFERENCE_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_HINT_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_FLOW_VECTOR_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_BACKWARD_FLOW_VECTOR_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_COST_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_BACKWARD_COST_NV:
        case VK_OPTICAL_FLOW_SESSION_BINDING_POINT_GLOBAL_FLOW_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAntiLagModeAMD value) const {
    switch (value) {
        case VK_ANTI_LAG_MODE_DRIVER_CONTROL_AMD:
        case VK_ANTI_LAG_MODE_ON_AMD:
        case VK_ANTI_LAG_MODE_OFF_AMD:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkAntiLagStageAMD value) const {
    switch (value) {
        case VK_ANTI_LAG_STAGE_INPUT_AMD:
        case VK_ANTI_LAG_STAGE_PRESENT_AMD:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkShaderCodeTypeEXT value) const {
    switch (value) {
        case VK_SHADER_CODE_TYPE_BINARY_EXT:
        case VK_SHADER_CODE_TYPE_SPIRV_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDepthClampModeEXT value) const {
    switch (value) {
        case VK_DEPTH_CLAMP_MODE_VIEWPORT_RANGE_EXT:
        case VK_DEPTH_CLAMP_MODE_USER_DEFINED_RANGE_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCooperativeVectorMatrixLayoutNV value) const {
    switch (value) {
        case VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR_NV:
        case VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_COLUMN_MAJOR_NV:
        case VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_INFERENCING_OPTIMAL_NV:
        case VK_COOPERATIVE_VECTOR_MATRIX_LAYOUT_TRAINING_OPTIMAL_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkLayerSettingTypeEXT value) const {
    switch (value) {
        case VK_LAYER_SETTING_TYPE_BOOL32_EXT:
        case VK_LAYER_SETTING_TYPE_INT32_EXT:
        case VK_LAYER_SETTING_TYPE_INT64_EXT:
        case VK_LAYER_SETTING_TYPE_UINT32_EXT:
        case VK_LAYER_SETTING_TYPE_UINT64_EXT:
        case VK_LAYER_SETTING_TYPE_FLOAT32_EXT:
        case VK_LAYER_SETTING_TYPE_FLOAT64_EXT:
        case VK_LAYER_SETTING_TYPE_STRING_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkLatencyMarkerNV value) const {
    switch (value) {
        case VK_LATENCY_MARKER_SIMULATION_START_NV:
        case VK_LATENCY_MARKER_SIMULATION_END_NV:
        case VK_LATENCY_MARKER_RENDERSUBMIT_START_NV:
        case VK_LATENCY_MARKER_RENDERSUBMIT_END_NV:
        case VK_LATENCY_MARKER_PRESENT_START_NV:
        case VK_LATENCY_MARKER_PRESENT_END_NV:
        case VK_LATENCY_MARKER_INPUT_SAMPLE_NV:
        case VK_LATENCY_MARKER_TRIGGER_FLASH_NV:
        case VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV:
        case VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_END_NV:
        case VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_START_NV:
        case VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_END_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkOutOfBandQueueTypeNV value) const {
    switch (value) {
        case VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV:
        case VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkBlockMatchWindowCompareModeQCOM value) const {
    switch (value) {
        case VK_BLOCK_MATCH_WINDOW_COMPARE_MODE_MIN_QCOM:
        case VK_BLOCK_MATCH_WINDOW_COMPARE_MODE_MAX_QCOM:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkCubicFilterWeightsQCOM value) const {
    switch (value) {
        case VK_CUBIC_FILTER_WEIGHTS_CATMULL_ROM_QCOM:
        case VK_CUBIC_FILTER_WEIGHTS_ZERO_TANGENT_CARDINAL_QCOM:
        case VK_CUBIC_FILTER_WEIGHTS_B_SPLINE_QCOM:
        case VK_CUBIC_FILTER_WEIGHTS_MITCHELL_NETRAVALI_QCOM:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkDisplaySurfaceStereoTypeNV value) const {
    switch (value) {
        case VK_DISPLAY_SURFACE_STEREO_TYPE_NONE_NV:
        case VK_DISPLAY_SURFACE_STEREO_TYPE_ONBOARD_DIN_NV:
        case VK_DISPLAY_SURFACE_STEREO_TYPE_HDMI_3D_NV:
        case VK_DISPLAY_SURFACE_STEREO_TYPE_INBAND_DISPLAYPORT_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkClusterAccelerationStructureTypeNV value) const {
    switch (value) {
        case VK_CLUSTER_ACCELERATION_STRUCTURE_TYPE_CLUSTERS_BOTTOM_LEVEL_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_TYPE_TRIANGLE_CLUSTER_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_TYPE_TRIANGLE_CLUSTER_TEMPLATE_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkClusterAccelerationStructureOpTypeNV value) const {
    switch (value) {
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_MOVE_OBJECTS_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_CLUSTERS_BOTTOM_LEVEL_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_BUILD_TRIANGLE_CLUSTER_TEMPLATE_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_TYPE_INSTANTIATE_TRIANGLE_CLUSTER_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkClusterAccelerationStructureOpModeNV value) const {
    switch (value) {
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_IMPLICIT_DESTINATIONS_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_EXPLICIT_DESTINATIONS_NV:
        case VK_CLUSTER_ACCELERATION_STRUCTURE_OP_MODE_COMPUTE_SIZES_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkPartitionedAccelerationStructureOpTypeNV value) const {
    switch (value) {
        case VK_PARTITIONED_ACCELERATION_STRUCTURE_OP_TYPE_WRITE_INSTANCE_NV:
        case VK_PARTITIONED_ACCELERATION_STRUCTURE_OP_TYPE_UPDATE_INSTANCE_NV:
        case VK_PARTITIONED_ACCELERATION_STRUCTURE_OP_TYPE_WRITE_PARTITION_TRANSLATION_NV:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkIndirectExecutionSetInfoTypeEXT value) const {
    switch (value) {
        case VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT:
        case VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkIndirectCommandsTokenTypeEXT value) const {
    switch (value) {
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_EXECUTION_SET_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_COUNT_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_COUNT_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT:
            return ValidValue::Valid;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT:
            return IsExtEnabled(extensions.vk_nv_mesh_shader) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT:
            return IsExtEnabled(extensions.vk_ext_mesh_shader) ? ValidValue::Valid : ValidValue::NoExtension;
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT:
            return IsExtEnabled(extensions.vk_khr_ray_tracing_maintenance1) ? ValidValue::Valid : ValidValue::NoExtension;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkBuildAccelerationStructureModeKHR value) const {
    switch (value) {
        case VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR:
        case VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
ValidValue stateless::Context::IsValidEnumValue(VkShaderGroupShaderKHR value) const {
    switch (value) {
        case VK_SHADER_GROUP_SHADER_GENERAL_KHR:
        case VK_SHADER_GROUP_SHADER_CLOSEST_HIT_KHR:
        case VK_SHADER_GROUP_SHADER_ANY_HIT_KHR:
        case VK_SHADER_GROUP_SHADER_INTERSECTION_KHR:
            return ValidValue::Valid;
        default:
            return ValidValue::NotFound;
    };
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPipelineCacheHeaderVersion value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkPipelineCacheHeaderVersion value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkImageLayout value) const {
    switch (value) {
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return {vvl::Extension::_VK_KHR_maintenance2};
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            return {vvl::Extension::_VK_KHR_separate_depth_stencil_layouts};
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
            return {vvl::Extension::_VK_KHR_synchronization2};
        case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
            return {vvl::Extension::_VK_KHR_dynamic_rendering_local_read};
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return {vvl::Extension::_VK_KHR_swapchain};
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
            return {vvl::Extension::_VK_KHR_video_decode_queue};
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            return {vvl::Extension::_VK_KHR_shared_presentable_image};
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            return {vvl::Extension::_VK_EXT_fragment_density_map};
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            return {vvl::Extension::_VK_KHR_fragment_shading_rate, vvl::Extension::_VK_NV_shading_rate_image};
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
            return {vvl::Extension::_VK_KHR_video_encode_queue};
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            return {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout};
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
            return {vvl::Extension::_VK_KHR_video_encode_quantization_map};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkImageLayout value) const {
    return string_VkImageLayout(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkObjectType value) const {
    switch (value) {
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
            return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
            return {vvl::Extension::_VK_KHR_descriptor_update_template};
        case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT:
            return {vvl::Extension::_VK_EXT_private_data};
        case VK_OBJECT_TYPE_SURFACE_KHR:
            return {vvl::Extension::_VK_KHR_surface};
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
            return {vvl::Extension::_VK_KHR_swapchain};
        case VK_OBJECT_TYPE_DISPLAY_KHR:
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
            return {vvl::Extension::_VK_KHR_display};
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
            return {vvl::Extension::_VK_EXT_debug_report};
        case VK_OBJECT_TYPE_VIDEO_SESSION_KHR:
        case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR:
            return {vvl::Extension::_VK_KHR_video_queue};
        case VK_OBJECT_TYPE_CU_MODULE_NVX:
        case VK_OBJECT_TYPE_CU_FUNCTION_NVX:
            return {vvl::Extension::_VK_NVX_binary_import};
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
            return {vvl::Extension::_VK_EXT_debug_utils};
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
            return {vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
            return {vvl::Extension::_VK_EXT_validation_cache};
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
            return {vvl::Extension::_VK_NV_ray_tracing};
        case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL:
            return {vvl::Extension::_VK_INTEL_performance_query};
        case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR:
            return {vvl::Extension::_VK_KHR_deferred_host_operations};
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
            return {vvl::Extension::_VK_NV_device_generated_commands};
        case VK_OBJECT_TYPE_CUDA_MODULE_NV:
        case VK_OBJECT_TYPE_CUDA_FUNCTION_NV:
            return {vvl::Extension::_VK_NV_cuda_kernel_launch};
        case VK_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA:
            return {vvl::Extension::_VK_FUCHSIA_buffer_collection};
        case VK_OBJECT_TYPE_MICROMAP_EXT:
            return {vvl::Extension::_VK_EXT_opacity_micromap};
        case VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV:
            return {vvl::Extension::_VK_NV_optical_flow};
        case VK_OBJECT_TYPE_SHADER_EXT:
            return {vvl::Extension::_VK_EXT_shader_object};
        case VK_OBJECT_TYPE_PIPELINE_BINARY_KHR:
            return {vvl::Extension::_VK_KHR_pipeline_binary};
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_EXT:
        case VK_OBJECT_TYPE_INDIRECT_EXECUTION_SET_EXT:
            return {vvl::Extension::_VK_EXT_device_generated_commands};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkObjectType value) const {
    return string_VkObjectType(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkFormat value) const {
    switch (value) {
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
            return {vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats};
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return {vvl::Extension::_VK_EXT_4444_formats};
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            return {vvl::Extension::_VK_EXT_texture_compression_astc_hdr};
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16:
        case VK_FORMAT_A8_UNORM:
            return {vvl::Extension::_VK_KHR_maintenance5};
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
            return {vvl::Extension::_VK_IMG_format_pvrtc};
        case VK_FORMAT_R16G16_SFIXED5_NV:
            return {vvl::Extension::_VK_NV_optical_flow};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkFormat value) const {
    return string_VkFormat(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkImageTiling value) const {
    switch (value) {
        case VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT:
            return {vvl::Extension::_VK_EXT_image_drm_format_modifier};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkImageTiling value) const {
    return string_VkImageTiling(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkImageType value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkImageType value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkQueryType value) const {
    switch (value) {
        case VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR:
            return {vvl::Extension::_VK_KHR_video_queue};
        case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
            return {vvl::Extension::_VK_EXT_transform_feedback};
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR:
            return {vvl::Extension::_VK_KHR_performance_query};
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
            return {vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_NV:
            return {vvl::Extension::_VK_NV_ray_tracing};
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL:
            return {vvl::Extension::_VK_INTEL_performance_query};
        case VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR:
            return {vvl::Extension::_VK_KHR_video_encode_queue};
        case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT:
            return {vvl::Extension::_VK_EXT_mesh_shader};
        case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
            return {vvl::Extension::_VK_EXT_primitives_generated_query};
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
        case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
            return {vvl::Extension::_VK_KHR_ray_tracing_maintenance1};
        case VK_QUERY_TYPE_MICROMAP_SERIALIZATION_SIZE_EXT:
        case VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT:
            return {vvl::Extension::_VK_EXT_opacity_micromap};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkQueryType value) const {
    return string_VkQueryType(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkSharingMode value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkSharingMode value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkComponentSwizzle value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkComponentSwizzle value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkImageViewType value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkImageViewType value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkBlendFactor value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkBlendFactor value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkBlendOp value) const {
    switch (value) {
        case VK_BLEND_OP_ZERO_EXT:
        case VK_BLEND_OP_SRC_EXT:
        case VK_BLEND_OP_DST_EXT:
        case VK_BLEND_OP_SRC_OVER_EXT:
        case VK_BLEND_OP_DST_OVER_EXT:
        case VK_BLEND_OP_SRC_IN_EXT:
        case VK_BLEND_OP_DST_IN_EXT:
        case VK_BLEND_OP_SRC_OUT_EXT:
        case VK_BLEND_OP_DST_OUT_EXT:
        case VK_BLEND_OP_SRC_ATOP_EXT:
        case VK_BLEND_OP_DST_ATOP_EXT:
        case VK_BLEND_OP_XOR_EXT:
        case VK_BLEND_OP_MULTIPLY_EXT:
        case VK_BLEND_OP_SCREEN_EXT:
        case VK_BLEND_OP_OVERLAY_EXT:
        case VK_BLEND_OP_DARKEN_EXT:
        case VK_BLEND_OP_LIGHTEN_EXT:
        case VK_BLEND_OP_COLORDODGE_EXT:
        case VK_BLEND_OP_COLORBURN_EXT:
        case VK_BLEND_OP_HARDLIGHT_EXT:
        case VK_BLEND_OP_SOFTLIGHT_EXT:
        case VK_BLEND_OP_DIFFERENCE_EXT:
        case VK_BLEND_OP_EXCLUSION_EXT:
        case VK_BLEND_OP_INVERT_EXT:
        case VK_BLEND_OP_INVERT_RGB_EXT:
        case VK_BLEND_OP_LINEARDODGE_EXT:
        case VK_BLEND_OP_LINEARBURN_EXT:
        case VK_BLEND_OP_VIVIDLIGHT_EXT:
        case VK_BLEND_OP_LINEARLIGHT_EXT:
        case VK_BLEND_OP_PINLIGHT_EXT:
        case VK_BLEND_OP_HARDMIX_EXT:
        case VK_BLEND_OP_HSL_HUE_EXT:
        case VK_BLEND_OP_HSL_SATURATION_EXT:
        case VK_BLEND_OP_HSL_COLOR_EXT:
        case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
        case VK_BLEND_OP_PLUS_EXT:
        case VK_BLEND_OP_PLUS_CLAMPED_EXT:
        case VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT:
        case VK_BLEND_OP_PLUS_DARKER_EXT:
        case VK_BLEND_OP_MINUS_EXT:
        case VK_BLEND_OP_MINUS_CLAMPED_EXT:
        case VK_BLEND_OP_CONTRAST_EXT:
        case VK_BLEND_OP_INVERT_OVG_EXT:
        case VK_BLEND_OP_RED_EXT:
        case VK_BLEND_OP_GREEN_EXT:
        case VK_BLEND_OP_BLUE_EXT:
            return {vvl::Extension::_VK_EXT_blend_operation_advanced};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkBlendOp value) const {
    return string_VkBlendOp(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCompareOp value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCompareOp value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDynamicState value) const {
    switch (value) {
        case VK_DYNAMIC_STATE_CULL_MODE:
        case VK_DYNAMIC_STATE_FRONT_FACE:
        case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
        case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
        case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
        case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
        case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
        case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
        case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
        case VK_DYNAMIC_STATE_STENCIL_OP:
            return {vvl::Extension::_VK_EXT_extended_dynamic_state};
        case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
        case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
        case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
        case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
        case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
            return {vvl::Extension::_VK_EXT_extended_dynamic_state2};
        case VK_DYNAMIC_STATE_LINE_STIPPLE:
            return {vvl::Extension::_VK_KHR_line_rasterization, vvl::Extension::_VK_EXT_line_rasterization};
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV:
            return {vvl::Extension::_VK_NV_clip_space_w_scaling};
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
            return {vvl::Extension::_VK_EXT_discard_rectangles};
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
            return {vvl::Extension::_VK_EXT_sample_locations};
        case VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR:
            return {vvl::Extension::_VK_KHR_ray_tracing_pipeline};
        case VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV:
        case VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV:
            return {vvl::Extension::_VK_NV_shading_rate_image};
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV:
        case VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV:
            return {vvl::Extension::_VK_NV_scissor_exclusive};
        case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
            return {vvl::Extension::_VK_KHR_fragment_shading_rate};
        case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
            return {vvl::Extension::_VK_EXT_vertex_input_dynamic_state};
        case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
            return {vvl::Extension::_VK_EXT_color_write_enable};
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
        case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
        case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
        case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
        case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
        case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
        case VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT:
        case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
        case VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT:
        case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
        case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
        case VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT:
        case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
        case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
        case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
        case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
        case VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV:
        case VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV:
        case VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV:
        case VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV:
        case VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV:
            return {vvl::Extension::_VK_EXT_extended_dynamic_state3};
        case VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
            return {vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state};
        case VK_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT:
            return {vvl::Extension::_VK_EXT_depth_clamp_control};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkDynamicState value) const {
    return string_VkDynamicState(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkFrontFace value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkFrontFace value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkVertexInputRate value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkVertexInputRate value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPrimitiveTopology value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkPrimitiveTopology value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPolygonMode value) const {
    switch (value) {
        case VK_POLYGON_MODE_FILL_RECTANGLE_NV:
            return {vvl::Extension::_VK_NV_fill_rectangle};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkPolygonMode value) const {
    return string_VkPolygonMode(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkStencilOp value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkStencilOp value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkLogicOp value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkLogicOp value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkBorderColor value) const {
    switch (value) {
        case VK_BORDER_COLOR_FLOAT_CUSTOM_EXT:
        case VK_BORDER_COLOR_INT_CUSTOM_EXT:
            return {vvl::Extension::_VK_EXT_custom_border_color};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkBorderColor value) const {
    return string_VkBorderColor(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkFilter value) const {
    switch (value) {
        case VK_FILTER_CUBIC_EXT:
            return {vvl::Extension::_VK_IMG_filter_cubic, vvl::Extension::_VK_EXT_filter_cubic};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkFilter value) const {
    return string_VkFilter(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkSamplerAddressMode value) const {
    switch (value) {
        case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
            return {vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkSamplerAddressMode value) const {
    return string_VkSamplerAddressMode(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkSamplerMipmapMode value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkSamplerMipmapMode value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDescriptorType value) const {
    switch (value) {
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            return {vvl::Extension::_VK_EXT_inline_uniform_block};
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return {vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
            return {vvl::Extension::_VK_NV_ray_tracing};
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
            return {vvl::Extension::_VK_QCOM_image_processing};
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
            return {vvl::Extension::_VK_VALVE_mutable_descriptor_type, vvl::Extension::_VK_EXT_mutable_descriptor_type};
        case VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV:
            return {vvl::Extension::_VK_NV_partitioned_acceleration_structure};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkDescriptorType value) const {
    return string_VkDescriptorType(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAttachmentLoadOp value) const {
    switch (value) {
        case VK_ATTACHMENT_LOAD_OP_NONE:
            return {vvl::Extension::_VK_KHR_load_store_op_none, vvl::Extension::_VK_EXT_load_store_op_none};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkAttachmentLoadOp value) const {
    return string_VkAttachmentLoadOp(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAttachmentStoreOp value) const {
    switch (value) {
        case VK_ATTACHMENT_STORE_OP_NONE:
            return {vvl::Extension::_VK_KHR_dynamic_rendering, vvl::Extension::_VK_KHR_load_store_op_none,
                    vvl::Extension::_VK_QCOM_render_pass_store_ops, vvl::Extension::_VK_EXT_load_store_op_none};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkAttachmentStoreOp value) const {
    return string_VkAttachmentStoreOp(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPipelineBindPoint value) const {
    switch (value) {
        case VK_PIPELINE_BIND_POINT_EXECUTION_GRAPH_AMDX:
            return {vvl::Extension::_VK_AMDX_shader_enqueue};
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_ray_tracing_pipeline};
        case VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI:
            return {vvl::Extension::_VK_HUAWEI_subpass_shading};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkPipelineBindPoint value) const {
    return string_VkPipelineBindPoint(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCommandBufferLevel value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCommandBufferLevel value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkIndexType value) const {
    switch (value) {
        case VK_INDEX_TYPE_UINT8:
            return {vvl::Extension::_VK_KHR_index_type_uint8, vvl::Extension::_VK_EXT_index_type_uint8};
        case VK_INDEX_TYPE_NONE_KHR:
            return {vvl::Extension::_VK_NV_ray_tracing, vvl::Extension::_VK_KHR_acceleration_structure};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkIndexType value) const {
    return string_VkIndexType(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkSubpassContents value) const {
    switch (value) {
        case VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR:
            return {vvl::Extension::_VK_KHR_maintenance7, vvl::Extension::_VK_EXT_nested_command_buffer};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkSubpassContents value) const {
    return string_VkSubpassContents(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkTessellationDomainOrigin value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkTessellationDomainOrigin value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkSamplerYcbcrModelConversion value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkSamplerYcbcrModelConversion value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkSamplerYcbcrRange value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkSamplerYcbcrRange value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkChromaLocation value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkChromaLocation value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDescriptorUpdateTemplateType value) const {
    switch (value) {
        case VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS:
            return {vvl::Extension::_VK_KHR_push_descriptor};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkDescriptorUpdateTemplateType value) const {
    return string_VkDescriptorUpdateTemplateType(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkSamplerReductionMode value) const {
    switch (value) {
        case VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_RANGECLAMP_QCOM:
            return {vvl::Extension::_VK_QCOM_filter_cubic_clamp};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkSamplerReductionMode value) const {
    return string_VkSamplerReductionMode(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkSemaphoreType value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkSemaphoreType value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPipelineRobustnessBufferBehavior value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkPipelineRobustnessBufferBehavior value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPipelineRobustnessImageBehavior value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkPipelineRobustnessImageBehavior value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkQueueGlobalPriority value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkQueueGlobalPriority value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkLineRasterizationMode value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkLineRasterizationMode value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPresentModeKHR value) const {
    switch (value) {
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            return {vvl::Extension::_VK_KHR_shared_presentable_image};
        case VK_PRESENT_MODE_FIFO_LATEST_READY_EXT:
            return {vvl::Extension::_VK_EXT_present_mode_fifo_latest_ready};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkPresentModeKHR value) const {
    return string_VkPresentModeKHR(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkColorSpaceKHR value) const {
    switch (value) {
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
        case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
        case VK_COLOR_SPACE_BT709_LINEAR_EXT:
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
        case VK_COLOR_SPACE_DOLBYVISION_EXT:
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
        case VK_COLOR_SPACE_PASS_THROUGH_EXT:
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
            return {vvl::Extension::_VK_EXT_swapchain_colorspace};
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:
            return {vvl::Extension::_VK_AMD_display_native_hdr};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkColorSpaceKHR value) const {
    return string_VkColorSpaceKHR(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkFragmentShadingRateCombinerOpKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkFragmentShadingRateCombinerOpKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkVideoEncodeTuningModeKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkVideoEncodeTuningModeKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkComponentTypeKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkComponentTypeKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkVideoEncodeAV1PredictionModeKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkVideoEncodeAV1PredictionModeKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkVideoEncodeAV1RateControlGroupKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkVideoEncodeAV1RateControlGroupKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkTimeDomainKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkTimeDomainKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDebugReportObjectTypeEXT value) const {
    switch (value) {
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT:
            return {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion};
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT:
            return {vvl::Extension::_VK_KHR_descriptor_update_template};
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT:
            return {vvl::Extension::_VK_NVX_binary_import};
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT:
            return {vvl::Extension::_VK_KHR_acceleration_structure};
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT:
            return {vvl::Extension::_VK_NV_ray_tracing};
        case VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_MODULE_NV_EXT:
        case VK_DEBUG_REPORT_OBJECT_TYPE_CUDA_FUNCTION_NV_EXT:
            return {vvl::Extension::_VK_NV_cuda_kernel_launch};
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT:
            return {vvl::Extension::_VK_FUCHSIA_buffer_collection};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkDebugReportObjectTypeEXT value) const {
    return string_VkDebugReportObjectTypeEXT(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkRasterizationOrderAMD value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkRasterizationOrderAMD value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkShaderInfoTypeAMD value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkShaderInfoTypeAMD value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkValidationCheckEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkValidationCheckEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDisplayPowerStateEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDisplayPowerStateEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDeviceEventTypeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDeviceEventTypeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDisplayEventTypeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDisplayEventTypeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkViewportCoordinateSwizzleNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkViewportCoordinateSwizzleNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDiscardRectangleModeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDiscardRectangleModeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkConservativeRasterizationModeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkConservativeRasterizationModeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkBlendOverlapEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkBlendOverlapEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCoverageModulationModeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCoverageModulationModeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkShadingRatePaletteEntryNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkShadingRatePaletteEntryNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCoarseSampleOrderTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCoarseSampleOrderTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkRayTracingShaderGroupTypeKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkRayTracingShaderGroupTypeKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkGeometryTypeKHR value) const {
    switch (value) {
        case VK_GEOMETRY_TYPE_SPHERES_NV:
        case VK_GEOMETRY_TYPE_LINEAR_SWEPT_SPHERES_NV:
            return {vvl::Extension::_VK_NV_ray_tracing_linear_swept_spheres};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkGeometryTypeKHR value) const {
    return string_VkGeometryTypeKHR(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAccelerationStructureTypeKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkAccelerationStructureTypeKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCopyAccelerationStructureModeKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCopyAccelerationStructureModeKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAccelerationStructureMemoryRequirementsTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkAccelerationStructureMemoryRequirementsTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkMemoryOverallocationBehaviorAMD value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkMemoryOverallocationBehaviorAMD value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPerformanceConfigurationTypeINTEL value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkPerformanceConfigurationTypeINTEL value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkQueryPoolSamplingModeINTEL value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkQueryPoolSamplingModeINTEL value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPerformanceOverrideTypeINTEL value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkPerformanceOverrideTypeINTEL value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPerformanceParameterTypeINTEL value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkPerformanceParameterTypeINTEL value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkValidationFeatureEnableEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkValidationFeatureEnableEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkValidationFeatureDisableEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkValidationFeatureDisableEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCoverageReductionModeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCoverageReductionModeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkProvokingVertexModeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkProvokingVertexModeEXT value) const {
    return nullptr;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkFullScreenExclusiveEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkFullScreenExclusiveEXT value) const {
    return nullptr;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkIndirectCommandsTokenTypeNV value) const {
    switch (value) {
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV:
            return {vvl::Extension::_VK_EXT_mesh_shader};
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PIPELINE_NV:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_NV:
            return {vvl::Extension::_VK_NV_device_generated_commands_compute};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkIndirectCommandsTokenTypeNV value) const {
    return string_VkIndirectCommandsTokenTypeNV(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDepthBiasRepresentationEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDepthBiasRepresentationEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkFragmentShadingRateTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkFragmentShadingRateTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkFragmentShadingRateNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkFragmentShadingRateNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAccelerationStructureMotionInstanceTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkAccelerationStructureMotionInstanceTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDeviceFaultAddressTypeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDeviceFaultAddressTypeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDeviceFaultVendorBinaryHeaderVersionEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDeviceFaultVendorBinaryHeaderVersionEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDeviceAddressBindingTypeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDeviceAddressBindingTypeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkMicromapTypeEXT value) const {
    switch (value) {
        case VK_MICROMAP_TYPE_DISPLACEMENT_MICROMAP_NV:
            return {vvl::Extension::_VK_NV_displacement_micromap};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkMicromapTypeEXT value) const {
    return string_VkMicromapTypeEXT(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkBuildMicromapModeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkBuildMicromapModeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCopyMicromapModeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCopyMicromapModeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAccelerationStructureCompatibilityKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkAccelerationStructureCompatibilityKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAccelerationStructureBuildTypeKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkAccelerationStructureBuildTypeKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkRayTracingLssIndexingModeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkRayTracingLssIndexingModeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkRayTracingLssPrimitiveEndCapsModeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkRayTracingLssPrimitiveEndCapsModeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDirectDriverLoadingModeLUNARG value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDirectDriverLoadingModeLUNARG value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkOpticalFlowPerformanceLevelNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkOpticalFlowPerformanceLevelNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkOpticalFlowSessionBindingPointNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkOpticalFlowSessionBindingPointNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAntiLagModeAMD value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkAntiLagModeAMD value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkAntiLagStageAMD value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkAntiLagStageAMD value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkShaderCodeTypeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkShaderCodeTypeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDepthClampModeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDepthClampModeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCooperativeVectorMatrixLayoutNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCooperativeVectorMatrixLayoutNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkLayerSettingTypeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkLayerSettingTypeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkLatencyMarkerNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkLatencyMarkerNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkOutOfBandQueueTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkOutOfBandQueueTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkBlockMatchWindowCompareModeQCOM value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkBlockMatchWindowCompareModeQCOM value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkCubicFilterWeightsQCOM value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkCubicFilterWeightsQCOM value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkDisplaySurfaceStereoTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkDisplaySurfaceStereoTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkClusterAccelerationStructureTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkClusterAccelerationStructureTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkClusterAccelerationStructureOpTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkClusterAccelerationStructureOpTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkClusterAccelerationStructureOpModeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkClusterAccelerationStructureOpModeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkPartitionedAccelerationStructureOpTypeNV value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkPartitionedAccelerationStructureOpTypeNV value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkIndirectExecutionSetInfoTypeEXT value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkIndirectExecutionSetInfoTypeEXT value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkIndirectCommandsTokenTypeEXT value) const {
    switch (value) {
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_NV_EXT:
            return {vvl::Extension::_VK_NV_mesh_shader};
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_EXT:
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_COUNT_EXT:
            return {vvl::Extension::_VK_EXT_mesh_shader};
        case VK_INDIRECT_COMMANDS_TOKEN_TYPE_TRACE_RAYS2_EXT:
            return {vvl::Extension::_VK_KHR_ray_tracing_maintenance1};
        default:
            return {};
    };
}
template <>
const char* stateless::Context::DescribeEnum(VkIndirectCommandsTokenTypeEXT value) const {
    return string_VkIndirectCommandsTokenTypeEXT(value);
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkBuildAccelerationStructureModeKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkBuildAccelerationStructureModeKHR value) const {
    return nullptr;
}

template <>
vvl::Extensions stateless::Context::GetEnumExtensions(VkShaderGroupShaderKHR value) const {
    return {};
}
template <>
const char* stateless::Context::DescribeEnum(VkShaderGroupShaderKHR value) const {
    return nullptr;
}

// NOLINTEND
