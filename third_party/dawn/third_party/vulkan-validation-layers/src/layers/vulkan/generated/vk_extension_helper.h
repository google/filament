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

#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cassert>

#include <vulkan/vulkan.h>
#include "containers/custom_containers.h"
#include "generated/vk_api_version.h"
#include "generated/error_location_helper.h"

// Extensions (unlike functions, struct, etc) are passed in as strings.
// The goal is to turn the string to a enum and pass that around the layers.
// Only when we need to print an error do we try and turn it back into a string
vvl::Extension GetExtension(std::string extension);

enum ExtEnabled : unsigned char {
    kNotEnabled,
    kEnabledByCreateinfo,   // Extension is passed at vkCreateDevice/vkCreateInstance time
    kEnabledByApiLevel,     // the API version implicitly enabled it
    kEnabledByInteraction,  // is implicity enabled by anthoer extension
};

// Map of promoted extension information per version (a separate map exists for instance and device extensions).
// The map is keyed by the version number (e.g. VK_API_VERSION_1_1) and each value is a pair consisting of the
// version string (e.g. "VK_VERSION_1_1") and the set of name of the promoted extensions.
typedef vvl::unordered_map<uint32_t, std::pair<const char *, vvl::unordered_set<vvl::Extension>>> PromotedExtensionInfoMap;
const PromotedExtensionInfoMap &GetInstancePromotionInfoMap();
const PromotedExtensionInfoMap &GetDevicePromotionInfoMap();

/*
This function is a helper to know if the extension is enabled.

Times to use it
- To determine the VUID
- The VU mentions the use of the extension
- Extension exposes property limits being validated
- Checking not enabled
    - if (!IsExtEnabled(...)) { }
- Special extensions that being EXPOSED alters the VUs
    - IsExtEnabled(extensions.vk_khr_portability_subset)
- Special extensions that alter behaviour of enabled
    - IsExtEnabled(extensions.vk_khr_maintenance*)

Times to NOT use it
    - If checking if a struct or enum is being used. There are a stateless checks
      to make sure the new Structs/Enums are not being used without this enabled.
    - If checking if the extension's feature enable status, because if the feature
      is enabled, then we already validated that extension is enabled.
    - Some variables (ex. viewMask) require the extension to be used if non-zero
*/
[[maybe_unused]] static bool IsExtEnabled(ExtEnabled extension) { return (extension != kNotEnabled); }

[[maybe_unused]] static bool IsExtEnabledByCreateinfo(ExtEnabled extension) { return (extension == kEnabledByCreateinfo); }

struct InstanceExtensions {
    APIVersion api_version{};
    ExtEnabled vk_feature_version_1_1{kNotEnabled};
    ExtEnabled vk_feature_version_1_2{kNotEnabled};
    ExtEnabled vk_feature_version_1_3{kNotEnabled};
    ExtEnabled vk_feature_version_1_4{kNotEnabled};
    ExtEnabled vk_khr_surface{kNotEnabled};
    ExtEnabled vk_khr_display{kNotEnabled};
    ExtEnabled vk_khr_xlib_surface{kNotEnabled};
    ExtEnabled vk_khr_xcb_surface{kNotEnabled};
    ExtEnabled vk_khr_wayland_surface{kNotEnabled};
    ExtEnabled vk_khr_android_surface{kNotEnabled};
    ExtEnabled vk_khr_win32_surface{kNotEnabled};
    ExtEnabled vk_khr_get_physical_device_properties2{kNotEnabled};
    ExtEnabled vk_khr_device_group_creation{kNotEnabled};
    ExtEnabled vk_khr_external_memory_capabilities{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore_capabilities{kNotEnabled};
    ExtEnabled vk_khr_external_fence_capabilities{kNotEnabled};
    ExtEnabled vk_khr_get_surface_capabilities2{kNotEnabled};
    ExtEnabled vk_khr_get_display_properties2{kNotEnabled};
    ExtEnabled vk_khr_surface_protected_capabilities{kNotEnabled};
    ExtEnabled vk_khr_portability_enumeration{kNotEnabled};
    ExtEnabled vk_ext_debug_report{kNotEnabled};
    ExtEnabled vk_ggp_stream_descriptor_surface{kNotEnabled};
    ExtEnabled vk_nv_external_memory_capabilities{kNotEnabled};
    ExtEnabled vk_ext_validation_flags{kNotEnabled};
    ExtEnabled vk_nn_vi_surface{kNotEnabled};
    ExtEnabled vk_ext_direct_mode_display{kNotEnabled};
    ExtEnabled vk_ext_acquire_xlib_display{kNotEnabled};
    ExtEnabled vk_ext_display_surface_counter{kNotEnabled};
    ExtEnabled vk_ext_swapchain_colorspace{kNotEnabled};
    ExtEnabled vk_mvk_ios_surface{kNotEnabled};
    ExtEnabled vk_mvk_macos_surface{kNotEnabled};
    ExtEnabled vk_ext_debug_utils{kNotEnabled};
    ExtEnabled vk_fuchsia_imagepipe_surface{kNotEnabled};
    ExtEnabled vk_ext_metal_surface{kNotEnabled};
    ExtEnabled vk_ext_validation_features{kNotEnabled};
    ExtEnabled vk_ext_headless_surface{kNotEnabled};
    ExtEnabled vk_ext_surface_maintenance1{kNotEnabled};
    ExtEnabled vk_ext_acquire_drm_display{kNotEnabled};
    ExtEnabled vk_ext_directfb_surface{kNotEnabled};
    ExtEnabled vk_qnx_screen_surface{kNotEnabled};
    ExtEnabled vk_google_surfaceless_query{kNotEnabled};
    ExtEnabled vk_lunarg_direct_driver_loading{kNotEnabled};
    ExtEnabled vk_ext_layer_settings{kNotEnabled};
    ExtEnabled vk_nv_display_stereo{kNotEnabled};

    struct Requirement {
        const ExtEnabled InstanceExtensions::*enabled;
        const char *name;
    };
    typedef std::vector<Requirement> RequirementVec;
    struct Info {
        Info(ExtEnabled InstanceExtensions::*state_, const RequirementVec requirements_)
            : state(state_), requirements(requirements_) {}
        ExtEnabled InstanceExtensions::*state;
        RequirementVec requirements;
    };

    typedef vvl::unordered_map<vvl::Extension, Info> InfoMap;
    static const InfoMap &GetInfoMap() {
        static const InfoMap info_map = {
            {vvl::Extension::_VK_KHR_surface, Info(&InstanceExtensions::vk_khr_surface, {})},
            {vvl::Extension::_VK_KHR_display,
             Info(&InstanceExtensions::vk_khr_display, {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_XLIB_KHR
            {vvl::Extension::_VK_KHR_xlib_surface, Info(&InstanceExtensions::vk_khr_xlib_surface,
                                                        {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
            {vvl::Extension::_VK_KHR_xcb_surface, Info(&InstanceExtensions::vk_khr_xcb_surface,
                                                       {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
            {vvl::Extension::_VK_KHR_wayland_surface,
             Info(&InstanceExtensions::vk_khr_wayland_surface,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            {vvl::Extension::_VK_KHR_android_surface,
             Info(&InstanceExtensions::vk_khr_android_surface,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_win32_surface, Info(&InstanceExtensions::vk_khr_win32_surface,
                                                         {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_get_physical_device_properties2,
             Info(&InstanceExtensions::vk_khr_get_physical_device_properties2, {})},
            {vvl::Extension::_VK_KHR_device_group_creation, Info(&InstanceExtensions::vk_khr_device_group_creation, {})},
            {vvl::Extension::_VK_KHR_external_memory_capabilities,
             Info(&InstanceExtensions::vk_khr_external_memory_capabilities,
                  {{{&InstanceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_external_semaphore_capabilities,
             Info(&InstanceExtensions::vk_khr_external_semaphore_capabilities,
                  {{{&InstanceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_external_fence_capabilities,
             Info(&InstanceExtensions::vk_khr_external_fence_capabilities,
                  {{{&InstanceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_get_surface_capabilities2,
             Info(&InstanceExtensions::vk_khr_get_surface_capabilities2,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_get_display_properties2,
             Info(&InstanceExtensions::vk_khr_get_display_properties2,
                  {{{&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_surface_protected_capabilities,
             Info(&InstanceExtensions::vk_khr_surface_protected_capabilities,
                  {{{&InstanceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                    {&InstanceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_portability_enumeration, Info(&InstanceExtensions::vk_khr_portability_enumeration, {})},
            {vvl::Extension::_VK_EXT_debug_report, Info(&InstanceExtensions::vk_ext_debug_report, {})},
#ifdef VK_USE_PLATFORM_GGP
            {vvl::Extension::_VK_GGP_stream_descriptor_surface,
             Info(&InstanceExtensions::vk_ggp_stream_descriptor_surface,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_GGP
            {vvl::Extension::_VK_NV_external_memory_capabilities,
             Info(&InstanceExtensions::vk_nv_external_memory_capabilities, {})},
            {vvl::Extension::_VK_EXT_validation_flags, Info(&InstanceExtensions::vk_ext_validation_flags, {})},
#ifdef VK_USE_PLATFORM_VI_NN
            {vvl::Extension::_VK_NN_vi_surface,
             Info(&InstanceExtensions::vk_nn_vi_surface, {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_VI_NN
            {vvl::Extension::_VK_EXT_direct_mode_display,
             Info(&InstanceExtensions::vk_ext_direct_mode_display,
                  {{{&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
            {vvl::Extension::_VK_EXT_acquire_xlib_display,
             Info(&InstanceExtensions::vk_ext_acquire_xlib_display,
                  {{{&InstanceExtensions::vk_ext_direct_mode_display, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
            {vvl::Extension::_VK_EXT_display_surface_counter,
             Info(&InstanceExtensions::vk_ext_display_surface_counter,
                  {{{&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_swapchain_colorspace,
             Info(&InstanceExtensions::vk_ext_swapchain_colorspace,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_IOS_MVK
            {vvl::Extension::_VK_MVK_ios_surface, Info(&InstanceExtensions::vk_mvk_ios_surface,
                                                       {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
            {vvl::Extension::_VK_MVK_macos_surface, Info(&InstanceExtensions::vk_mvk_macos_surface,
                                                         {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_MACOS_MVK
            {vvl::Extension::_VK_EXT_debug_utils, Info(&InstanceExtensions::vk_ext_debug_utils, {})},
#ifdef VK_USE_PLATFORM_FUCHSIA
            {vvl::Extension::_VK_FUCHSIA_imagepipe_surface,
             Info(&InstanceExtensions::vk_fuchsia_imagepipe_surface,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
            {vvl::Extension::_VK_EXT_metal_surface, Info(&InstanceExtensions::vk_ext_metal_surface,
                                                         {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_METAL_EXT
            {vvl::Extension::_VK_EXT_validation_features, Info(&InstanceExtensions::vk_ext_validation_features, {})},
            {vvl::Extension::_VK_EXT_headless_surface,
             Info(&InstanceExtensions::vk_ext_headless_surface,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_surface_maintenance1,
             Info(&InstanceExtensions::vk_ext_surface_maintenance1,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME},
                    {&InstanceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_acquire_drm_display,
             Info(&InstanceExtensions::vk_ext_acquire_drm_display,
                  {{{&InstanceExtensions::vk_ext_direct_mode_display, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
            {vvl::Extension::_VK_EXT_directfb_surface,
             Info(&InstanceExtensions::vk_ext_directfb_surface,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            {vvl::Extension::_VK_QNX_screen_surface,
             Info(&InstanceExtensions::vk_qnx_screen_surface,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_SCREEN_QNX
            {vvl::Extension::_VK_GOOGLE_surfaceless_query,
             Info(&InstanceExtensions::vk_google_surfaceless_query,
                  {{{&InstanceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_LUNARG_direct_driver_loading, Info(&InstanceExtensions::vk_lunarg_direct_driver_loading, {})},
            {vvl::Extension::_VK_EXT_layer_settings, Info(&InstanceExtensions::vk_ext_layer_settings, {})},
            {vvl::Extension::_VK_NV_display_stereo,
             Info(&InstanceExtensions::vk_nv_display_stereo,
                  {{{&InstanceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME},
                    {&InstanceExtensions::vk_khr_get_display_properties2, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME}}})},

        };
        return info_map;
    }

    static const Info &GetInfo(vvl::Extension extension_name) {
        static const Info empty_info{nullptr, RequirementVec()};
        const auto &ext_map = InstanceExtensions::GetInfoMap();
        const auto info = ext_map.find(extension_name);
        return (info != ext_map.cend()) ? info->second : empty_info;
    }

    InstanceExtensions() = default;
    InstanceExtensions(APIVersion requested_api_version, const VkInstanceCreateInfo *pCreateInfo);
};

struct DeviceExtensions : public InstanceExtensions {
    ExtEnabled vk_feature_version_1_1{kNotEnabled};
    ExtEnabled vk_feature_version_1_2{kNotEnabled};
    ExtEnabled vk_feature_version_1_3{kNotEnabled};
    ExtEnabled vk_feature_version_1_4{kNotEnabled};
    ExtEnabled vk_khr_swapchain{kNotEnabled};
    ExtEnabled vk_khr_display_swapchain{kNotEnabled};
    ExtEnabled vk_khr_sampler_mirror_clamp_to_edge{kNotEnabled};
    ExtEnabled vk_khr_video_queue{kNotEnabled};
    ExtEnabled vk_khr_video_decode_queue{kNotEnabled};
    ExtEnabled vk_khr_video_encode_h264{kNotEnabled};
    ExtEnabled vk_khr_video_encode_h265{kNotEnabled};
    ExtEnabled vk_khr_video_decode_h264{kNotEnabled};
    ExtEnabled vk_khr_dynamic_rendering{kNotEnabled};
    ExtEnabled vk_khr_multiview{kNotEnabled};
    ExtEnabled vk_khr_device_group{kNotEnabled};
    ExtEnabled vk_khr_shader_draw_parameters{kNotEnabled};
    ExtEnabled vk_khr_maintenance1{kNotEnabled};
    ExtEnabled vk_khr_external_memory{kNotEnabled};
    ExtEnabled vk_khr_external_memory_win32{kNotEnabled};
    ExtEnabled vk_khr_external_memory_fd{kNotEnabled};
    ExtEnabled vk_khr_win32_keyed_mutex{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore_win32{kNotEnabled};
    ExtEnabled vk_khr_external_semaphore_fd{kNotEnabled};
    ExtEnabled vk_khr_push_descriptor{kNotEnabled};
    ExtEnabled vk_khr_shader_float16_int8{kNotEnabled};
    ExtEnabled vk_khr_16bit_storage{kNotEnabled};
    ExtEnabled vk_khr_incremental_present{kNotEnabled};
    ExtEnabled vk_khr_descriptor_update_template{kNotEnabled};
    ExtEnabled vk_khr_imageless_framebuffer{kNotEnabled};
    ExtEnabled vk_khr_create_renderpass2{kNotEnabled};
    ExtEnabled vk_khr_shared_presentable_image{kNotEnabled};
    ExtEnabled vk_khr_external_fence{kNotEnabled};
    ExtEnabled vk_khr_external_fence_win32{kNotEnabled};
    ExtEnabled vk_khr_external_fence_fd{kNotEnabled};
    ExtEnabled vk_khr_performance_query{kNotEnabled};
    ExtEnabled vk_khr_maintenance2{kNotEnabled};
    ExtEnabled vk_khr_variable_pointers{kNotEnabled};
    ExtEnabled vk_khr_dedicated_allocation{kNotEnabled};
    ExtEnabled vk_khr_storage_buffer_storage_class{kNotEnabled};
    ExtEnabled vk_khr_relaxed_block_layout{kNotEnabled};
    ExtEnabled vk_khr_get_memory_requirements2{kNotEnabled};
    ExtEnabled vk_khr_image_format_list{kNotEnabled};
    ExtEnabled vk_khr_sampler_ycbcr_conversion{kNotEnabled};
    ExtEnabled vk_khr_bind_memory2{kNotEnabled};
    ExtEnabled vk_khr_portability_subset{kNotEnabled};
    ExtEnabled vk_khr_maintenance3{kNotEnabled};
    ExtEnabled vk_khr_draw_indirect_count{kNotEnabled};
    ExtEnabled vk_khr_shader_subgroup_extended_types{kNotEnabled};
    ExtEnabled vk_khr_8bit_storage{kNotEnabled};
    ExtEnabled vk_khr_shader_atomic_int64{kNotEnabled};
    ExtEnabled vk_khr_shader_clock{kNotEnabled};
    ExtEnabled vk_khr_video_decode_h265{kNotEnabled};
    ExtEnabled vk_khr_global_priority{kNotEnabled};
    ExtEnabled vk_khr_driver_properties{kNotEnabled};
    ExtEnabled vk_khr_shader_float_controls{kNotEnabled};
    ExtEnabled vk_khr_depth_stencil_resolve{kNotEnabled};
    ExtEnabled vk_khr_swapchain_mutable_format{kNotEnabled};
    ExtEnabled vk_khr_timeline_semaphore{kNotEnabled};
    ExtEnabled vk_khr_vulkan_memory_model{kNotEnabled};
    ExtEnabled vk_khr_shader_terminate_invocation{kNotEnabled};
    ExtEnabled vk_khr_fragment_shading_rate{kNotEnabled};
    ExtEnabled vk_khr_dynamic_rendering_local_read{kNotEnabled};
    ExtEnabled vk_khr_shader_quad_control{kNotEnabled};
    ExtEnabled vk_khr_spirv_1_4{kNotEnabled};
    ExtEnabled vk_khr_separate_depth_stencil_layouts{kNotEnabled};
    ExtEnabled vk_khr_present_wait{kNotEnabled};
    ExtEnabled vk_khr_uniform_buffer_standard_layout{kNotEnabled};
    ExtEnabled vk_khr_buffer_device_address{kNotEnabled};
    ExtEnabled vk_khr_deferred_host_operations{kNotEnabled};
    ExtEnabled vk_khr_pipeline_executable_properties{kNotEnabled};
    ExtEnabled vk_khr_map_memory2{kNotEnabled};
    ExtEnabled vk_khr_shader_integer_dot_product{kNotEnabled};
    ExtEnabled vk_khr_pipeline_library{kNotEnabled};
    ExtEnabled vk_khr_shader_non_semantic_info{kNotEnabled};
    ExtEnabled vk_khr_present_id{kNotEnabled};
    ExtEnabled vk_khr_video_encode_queue{kNotEnabled};
    ExtEnabled vk_khr_synchronization2{kNotEnabled};
    ExtEnabled vk_khr_fragment_shader_barycentric{kNotEnabled};
    ExtEnabled vk_khr_shader_subgroup_uniform_control_flow{kNotEnabled};
    ExtEnabled vk_khr_zero_initialize_workgroup_memory{kNotEnabled};
    ExtEnabled vk_khr_workgroup_memory_explicit_layout{kNotEnabled};
    ExtEnabled vk_khr_copy_commands2{kNotEnabled};
    ExtEnabled vk_khr_format_feature_flags2{kNotEnabled};
    ExtEnabled vk_khr_ray_tracing_maintenance1{kNotEnabled};
    ExtEnabled vk_khr_maintenance4{kNotEnabled};
    ExtEnabled vk_khr_shader_subgroup_rotate{kNotEnabled};
    ExtEnabled vk_khr_shader_maximal_reconvergence{kNotEnabled};
    ExtEnabled vk_khr_maintenance5{kNotEnabled};
    ExtEnabled vk_khr_ray_tracing_position_fetch{kNotEnabled};
    ExtEnabled vk_khr_pipeline_binary{kNotEnabled};
    ExtEnabled vk_khr_cooperative_matrix{kNotEnabled};
    ExtEnabled vk_khr_compute_shader_derivatives{kNotEnabled};
    ExtEnabled vk_khr_video_decode_av1{kNotEnabled};
    ExtEnabled vk_khr_video_encode_av1{kNotEnabled};
    ExtEnabled vk_khr_video_maintenance1{kNotEnabled};
    ExtEnabled vk_khr_vertex_attribute_divisor{kNotEnabled};
    ExtEnabled vk_khr_load_store_op_none{kNotEnabled};
    ExtEnabled vk_khr_shader_float_controls2{kNotEnabled};
    ExtEnabled vk_khr_index_type_uint8{kNotEnabled};
    ExtEnabled vk_khr_line_rasterization{kNotEnabled};
    ExtEnabled vk_khr_calibrated_timestamps{kNotEnabled};
    ExtEnabled vk_khr_shader_expect_assume{kNotEnabled};
    ExtEnabled vk_khr_maintenance6{kNotEnabled};
    ExtEnabled vk_khr_video_encode_quantization_map{kNotEnabled};
    ExtEnabled vk_khr_shader_relaxed_extended_instruction{kNotEnabled};
    ExtEnabled vk_khr_maintenance7{kNotEnabled};
    ExtEnabled vk_khr_maintenance8{kNotEnabled};
    ExtEnabled vk_khr_video_maintenance2{kNotEnabled};
    ExtEnabled vk_khr_depth_clamp_zero_one{kNotEnabled};
    ExtEnabled vk_nv_glsl_shader{kNotEnabled};
    ExtEnabled vk_ext_depth_range_unrestricted{kNotEnabled};
    ExtEnabled vk_img_filter_cubic{kNotEnabled};
    ExtEnabled vk_amd_rasterization_order{kNotEnabled};
    ExtEnabled vk_amd_shader_trinary_minmax{kNotEnabled};
    ExtEnabled vk_amd_shader_explicit_vertex_parameter{kNotEnabled};
    ExtEnabled vk_ext_debug_marker{kNotEnabled};
    ExtEnabled vk_amd_gcn_shader{kNotEnabled};
    ExtEnabled vk_nv_dedicated_allocation{kNotEnabled};
    ExtEnabled vk_ext_transform_feedback{kNotEnabled};
    ExtEnabled vk_nvx_binary_import{kNotEnabled};
    ExtEnabled vk_nvx_image_view_handle{kNotEnabled};
    ExtEnabled vk_amd_draw_indirect_count{kNotEnabled};
    ExtEnabled vk_amd_negative_viewport_height{kNotEnabled};
    ExtEnabled vk_amd_gpu_shader_half_float{kNotEnabled};
    ExtEnabled vk_amd_shader_ballot{kNotEnabled};
    ExtEnabled vk_amd_texture_gather_bias_lod{kNotEnabled};
    ExtEnabled vk_amd_shader_info{kNotEnabled};
    ExtEnabled vk_amd_shader_image_load_store_lod{kNotEnabled};
    ExtEnabled vk_nv_corner_sampled_image{kNotEnabled};
    ExtEnabled vk_img_format_pvrtc{kNotEnabled};
    ExtEnabled vk_nv_external_memory{kNotEnabled};
    ExtEnabled vk_nv_external_memory_win32{kNotEnabled};
    ExtEnabled vk_nv_win32_keyed_mutex{kNotEnabled};
    ExtEnabled vk_ext_shader_subgroup_ballot{kNotEnabled};
    ExtEnabled vk_ext_shader_subgroup_vote{kNotEnabled};
    ExtEnabled vk_ext_texture_compression_astc_hdr{kNotEnabled};
    ExtEnabled vk_ext_astc_decode_mode{kNotEnabled};
    ExtEnabled vk_ext_pipeline_robustness{kNotEnabled};
    ExtEnabled vk_ext_conditional_rendering{kNotEnabled};
    ExtEnabled vk_nv_clip_space_w_scaling{kNotEnabled};
    ExtEnabled vk_ext_display_control{kNotEnabled};
    ExtEnabled vk_google_display_timing{kNotEnabled};
    ExtEnabled vk_nv_sample_mask_override_coverage{kNotEnabled};
    ExtEnabled vk_nv_geometry_shader_passthrough{kNotEnabled};
    ExtEnabled vk_nv_viewport_array2{kNotEnabled};
    ExtEnabled vk_nvx_multiview_per_view_attributes{kNotEnabled};
    ExtEnabled vk_nv_viewport_swizzle{kNotEnabled};
    ExtEnabled vk_ext_discard_rectangles{kNotEnabled};
    ExtEnabled vk_ext_conservative_rasterization{kNotEnabled};
    ExtEnabled vk_ext_depth_clip_enable{kNotEnabled};
    ExtEnabled vk_ext_hdr_metadata{kNotEnabled};
    ExtEnabled vk_img_relaxed_line_rasterization{kNotEnabled};
    ExtEnabled vk_ext_external_memory_dma_buf{kNotEnabled};
    ExtEnabled vk_ext_queue_family_foreign{kNotEnabled};
    ExtEnabled vk_android_external_memory_android_hardware_buffer{kNotEnabled};
    ExtEnabled vk_ext_sampler_filter_minmax{kNotEnabled};
    ExtEnabled vk_amd_gpu_shader_int16{kNotEnabled};
    ExtEnabled vk_amdx_shader_enqueue{kNotEnabled};
    ExtEnabled vk_amd_mixed_attachment_samples{kNotEnabled};
    ExtEnabled vk_amd_shader_fragment_mask{kNotEnabled};
    ExtEnabled vk_ext_inline_uniform_block{kNotEnabled};
    ExtEnabled vk_ext_shader_stencil_export{kNotEnabled};
    ExtEnabled vk_ext_sample_locations{kNotEnabled};
    ExtEnabled vk_ext_blend_operation_advanced{kNotEnabled};
    ExtEnabled vk_nv_fragment_coverage_to_color{kNotEnabled};
    ExtEnabled vk_nv_framebuffer_mixed_samples{kNotEnabled};
    ExtEnabled vk_nv_fill_rectangle{kNotEnabled};
    ExtEnabled vk_nv_shader_sm_builtins{kNotEnabled};
    ExtEnabled vk_ext_post_depth_coverage{kNotEnabled};
    ExtEnabled vk_ext_image_drm_format_modifier{kNotEnabled};
    ExtEnabled vk_ext_validation_cache{kNotEnabled};
    ExtEnabled vk_ext_descriptor_indexing{kNotEnabled};
    ExtEnabled vk_ext_shader_viewport_index_layer{kNotEnabled};
    ExtEnabled vk_nv_shading_rate_image{kNotEnabled};
    ExtEnabled vk_nv_ray_tracing{kNotEnabled};
    ExtEnabled vk_nv_representative_fragment_test{kNotEnabled};
    ExtEnabled vk_ext_filter_cubic{kNotEnabled};
    ExtEnabled vk_qcom_render_pass_shader_resolve{kNotEnabled};
    ExtEnabled vk_ext_global_priority{kNotEnabled};
    ExtEnabled vk_ext_external_memory_host{kNotEnabled};
    ExtEnabled vk_amd_buffer_marker{kNotEnabled};
    ExtEnabled vk_amd_pipeline_compiler_control{kNotEnabled};
    ExtEnabled vk_ext_calibrated_timestamps{kNotEnabled};
    ExtEnabled vk_amd_shader_core_properties{kNotEnabled};
    ExtEnabled vk_amd_memory_overallocation_behavior{kNotEnabled};
    ExtEnabled vk_ext_vertex_attribute_divisor{kNotEnabled};
    ExtEnabled vk_ggp_frame_token{kNotEnabled};
    ExtEnabled vk_ext_pipeline_creation_feedback{kNotEnabled};
    ExtEnabled vk_nv_shader_subgroup_partitioned{kNotEnabled};
    ExtEnabled vk_nv_compute_shader_derivatives{kNotEnabled};
    ExtEnabled vk_nv_mesh_shader{kNotEnabled};
    ExtEnabled vk_nv_fragment_shader_barycentric{kNotEnabled};
    ExtEnabled vk_nv_shader_image_footprint{kNotEnabled};
    ExtEnabled vk_nv_scissor_exclusive{kNotEnabled};
    ExtEnabled vk_nv_device_diagnostic_checkpoints{kNotEnabled};
    ExtEnabled vk_intel_shader_integer_functions2{kNotEnabled};
    ExtEnabled vk_intel_performance_query{kNotEnabled};
    ExtEnabled vk_ext_pci_bus_info{kNotEnabled};
    ExtEnabled vk_amd_display_native_hdr{kNotEnabled};
    ExtEnabled vk_ext_fragment_density_map{kNotEnabled};
    ExtEnabled vk_ext_scalar_block_layout{kNotEnabled};
    ExtEnabled vk_google_hlsl_functionality1{kNotEnabled};
    ExtEnabled vk_google_decorate_string{kNotEnabled};
    ExtEnabled vk_ext_subgroup_size_control{kNotEnabled};
    ExtEnabled vk_amd_shader_core_properties2{kNotEnabled};
    ExtEnabled vk_amd_device_coherent_memory{kNotEnabled};
    ExtEnabled vk_ext_shader_image_atomic_int64{kNotEnabled};
    ExtEnabled vk_ext_memory_budget{kNotEnabled};
    ExtEnabled vk_ext_memory_priority{kNotEnabled};
    ExtEnabled vk_nv_dedicated_allocation_image_aliasing{kNotEnabled};
    ExtEnabled vk_ext_buffer_device_address{kNotEnabled};
    ExtEnabled vk_ext_tooling_info{kNotEnabled};
    ExtEnabled vk_ext_separate_stencil_usage{kNotEnabled};
    ExtEnabled vk_nv_cooperative_matrix{kNotEnabled};
    ExtEnabled vk_nv_coverage_reduction_mode{kNotEnabled};
    ExtEnabled vk_ext_fragment_shader_interlock{kNotEnabled};
    ExtEnabled vk_ext_ycbcr_image_arrays{kNotEnabled};
    ExtEnabled vk_ext_provoking_vertex{kNotEnabled};
    ExtEnabled vk_ext_full_screen_exclusive{kNotEnabled};
    ExtEnabled vk_ext_line_rasterization{kNotEnabled};
    ExtEnabled vk_ext_shader_atomic_float{kNotEnabled};
    ExtEnabled vk_ext_host_query_reset{kNotEnabled};
    ExtEnabled vk_ext_index_type_uint8{kNotEnabled};
    ExtEnabled vk_ext_extended_dynamic_state{kNotEnabled};
    ExtEnabled vk_ext_host_image_copy{kNotEnabled};
    ExtEnabled vk_ext_map_memory_placed{kNotEnabled};
    ExtEnabled vk_ext_shader_atomic_float2{kNotEnabled};
    ExtEnabled vk_ext_swapchain_maintenance1{kNotEnabled};
    ExtEnabled vk_ext_shader_demote_to_helper_invocation{kNotEnabled};
    ExtEnabled vk_nv_device_generated_commands{kNotEnabled};
    ExtEnabled vk_nv_inherited_viewport_scissor{kNotEnabled};
    ExtEnabled vk_ext_texel_buffer_alignment{kNotEnabled};
    ExtEnabled vk_qcom_render_pass_transform{kNotEnabled};
    ExtEnabled vk_ext_depth_bias_control{kNotEnabled};
    ExtEnabled vk_ext_device_memory_report{kNotEnabled};
    ExtEnabled vk_ext_robustness2{kNotEnabled};
    ExtEnabled vk_ext_custom_border_color{kNotEnabled};
    ExtEnabled vk_google_user_type{kNotEnabled};
    ExtEnabled vk_nv_present_barrier{kNotEnabled};
    ExtEnabled vk_ext_private_data{kNotEnabled};
    ExtEnabled vk_ext_pipeline_creation_cache_control{kNotEnabled};
    ExtEnabled vk_nv_device_diagnostics_config{kNotEnabled};
    ExtEnabled vk_qcom_render_pass_store_ops{kNotEnabled};
    ExtEnabled vk_nv_cuda_kernel_launch{kNotEnabled};
    ExtEnabled vk_nv_low_latency{kNotEnabled};
    ExtEnabled vk_ext_metal_objects{kNotEnabled};
    ExtEnabled vk_ext_descriptor_buffer{kNotEnabled};
    ExtEnabled vk_ext_graphics_pipeline_library{kNotEnabled};
    ExtEnabled vk_amd_shader_early_and_late_fragment_tests{kNotEnabled};
    ExtEnabled vk_nv_fragment_shading_rate_enums{kNotEnabled};
    ExtEnabled vk_nv_ray_tracing_motion_blur{kNotEnabled};
    ExtEnabled vk_ext_ycbcr_2plane_444_formats{kNotEnabled};
    ExtEnabled vk_ext_fragment_density_map2{kNotEnabled};
    ExtEnabled vk_qcom_rotated_copy_commands{kNotEnabled};
    ExtEnabled vk_ext_image_robustness{kNotEnabled};
    ExtEnabled vk_ext_image_compression_control{kNotEnabled};
    ExtEnabled vk_ext_attachment_feedback_loop_layout{kNotEnabled};
    ExtEnabled vk_ext_4444_formats{kNotEnabled};
    ExtEnabled vk_ext_device_fault{kNotEnabled};
    ExtEnabled vk_arm_rasterization_order_attachment_access{kNotEnabled};
    ExtEnabled vk_ext_rgba10x6_formats{kNotEnabled};
    ExtEnabled vk_nv_acquire_winrt_display{kNotEnabled};
    ExtEnabled vk_valve_mutable_descriptor_type{kNotEnabled};
    ExtEnabled vk_ext_vertex_input_dynamic_state{kNotEnabled};
    ExtEnabled vk_ext_physical_device_drm{kNotEnabled};
    ExtEnabled vk_ext_device_address_binding_report{kNotEnabled};
    ExtEnabled vk_ext_depth_clip_control{kNotEnabled};
    ExtEnabled vk_ext_primitive_topology_list_restart{kNotEnabled};
    ExtEnabled vk_ext_present_mode_fifo_latest_ready{kNotEnabled};
    ExtEnabled vk_fuchsia_external_memory{kNotEnabled};
    ExtEnabled vk_fuchsia_external_semaphore{kNotEnabled};
    ExtEnabled vk_fuchsia_buffer_collection{kNotEnabled};
    ExtEnabled vk_huawei_subpass_shading{kNotEnabled};
    ExtEnabled vk_huawei_invocation_mask{kNotEnabled};
    ExtEnabled vk_nv_external_memory_rdma{kNotEnabled};
    ExtEnabled vk_ext_pipeline_properties{kNotEnabled};
    ExtEnabled vk_ext_frame_boundary{kNotEnabled};
    ExtEnabled vk_ext_multisampled_render_to_single_sampled{kNotEnabled};
    ExtEnabled vk_ext_extended_dynamic_state2{kNotEnabled};
    ExtEnabled vk_ext_color_write_enable{kNotEnabled};
    ExtEnabled vk_ext_primitives_generated_query{kNotEnabled};
    ExtEnabled vk_ext_global_priority_query{kNotEnabled};
    ExtEnabled vk_ext_image_view_min_lod{kNotEnabled};
    ExtEnabled vk_ext_multi_draw{kNotEnabled};
    ExtEnabled vk_ext_image_2d_view_of_3d{kNotEnabled};
    ExtEnabled vk_ext_shader_tile_image{kNotEnabled};
    ExtEnabled vk_ext_opacity_micromap{kNotEnabled};
    ExtEnabled vk_nv_displacement_micromap{kNotEnabled};
    ExtEnabled vk_ext_load_store_op_none{kNotEnabled};
    ExtEnabled vk_huawei_cluster_culling_shader{kNotEnabled};
    ExtEnabled vk_ext_border_color_swizzle{kNotEnabled};
    ExtEnabled vk_ext_pageable_device_local_memory{kNotEnabled};
    ExtEnabled vk_arm_shader_core_properties{kNotEnabled};
    ExtEnabled vk_arm_scheduling_controls{kNotEnabled};
    ExtEnabled vk_ext_image_sliced_view_of_3d{kNotEnabled};
    ExtEnabled vk_valve_descriptor_set_host_mapping{kNotEnabled};
    ExtEnabled vk_ext_depth_clamp_zero_one{kNotEnabled};
    ExtEnabled vk_ext_non_seamless_cube_map{kNotEnabled};
    ExtEnabled vk_arm_render_pass_striped{kNotEnabled};
    ExtEnabled vk_qcom_fragment_density_map_offset{kNotEnabled};
    ExtEnabled vk_nv_copy_memory_indirect{kNotEnabled};
    ExtEnabled vk_nv_memory_decompression{kNotEnabled};
    ExtEnabled vk_nv_device_generated_commands_compute{kNotEnabled};
    ExtEnabled vk_nv_ray_tracing_linear_swept_spheres{kNotEnabled};
    ExtEnabled vk_nv_linear_color_attachment{kNotEnabled};
    ExtEnabled vk_ext_image_compression_control_swapchain{kNotEnabled};
    ExtEnabled vk_qcom_image_processing{kNotEnabled};
    ExtEnabled vk_ext_nested_command_buffer{kNotEnabled};
    ExtEnabled vk_ext_external_memory_acquire_unmodified{kNotEnabled};
    ExtEnabled vk_ext_extended_dynamic_state3{kNotEnabled};
    ExtEnabled vk_ext_subpass_merge_feedback{kNotEnabled};
    ExtEnabled vk_ext_shader_module_identifier{kNotEnabled};
    ExtEnabled vk_ext_rasterization_order_attachment_access{kNotEnabled};
    ExtEnabled vk_nv_optical_flow{kNotEnabled};
    ExtEnabled vk_ext_legacy_dithering{kNotEnabled};
    ExtEnabled vk_ext_pipeline_protected_access{kNotEnabled};
    ExtEnabled vk_android_external_format_resolve{kNotEnabled};
    ExtEnabled vk_amd_anti_lag{kNotEnabled};
    ExtEnabled vk_ext_shader_object{kNotEnabled};
    ExtEnabled vk_qcom_tile_properties{kNotEnabled};
    ExtEnabled vk_sec_amigo_profiling{kNotEnabled};
    ExtEnabled vk_qcom_multiview_per_view_viewports{kNotEnabled};
    ExtEnabled vk_nv_ray_tracing_invocation_reorder{kNotEnabled};
    ExtEnabled vk_nv_cooperative_vector{kNotEnabled};
    ExtEnabled vk_nv_extended_sparse_address_space{kNotEnabled};
    ExtEnabled vk_ext_mutable_descriptor_type{kNotEnabled};
    ExtEnabled vk_ext_legacy_vertex_attributes{kNotEnabled};
    ExtEnabled vk_arm_shader_core_builtins{kNotEnabled};
    ExtEnabled vk_ext_pipeline_library_group_handles{kNotEnabled};
    ExtEnabled vk_ext_dynamic_rendering_unused_attachments{kNotEnabled};
    ExtEnabled vk_nv_low_latency2{kNotEnabled};
    ExtEnabled vk_qcom_multiview_per_view_render_areas{kNotEnabled};
    ExtEnabled vk_nv_per_stage_descriptor_set{kNotEnabled};
    ExtEnabled vk_qcom_image_processing2{kNotEnabled};
    ExtEnabled vk_qcom_filter_cubic_weights{kNotEnabled};
    ExtEnabled vk_qcom_ycbcr_degamma{kNotEnabled};
    ExtEnabled vk_qcom_filter_cubic_clamp{kNotEnabled};
    ExtEnabled vk_ext_attachment_feedback_loop_dynamic_state{kNotEnabled};
    ExtEnabled vk_qnx_external_memory_screen_buffer{kNotEnabled};
    ExtEnabled vk_msft_layered_driver{kNotEnabled};
    ExtEnabled vk_nv_descriptor_pool_overallocation{kNotEnabled};
    ExtEnabled vk_nv_raw_access_chains{kNotEnabled};
    ExtEnabled vk_nv_command_buffer_inheritance{kNotEnabled};
    ExtEnabled vk_nv_shader_atomic_float16_vector{kNotEnabled};
    ExtEnabled vk_ext_shader_replicated_composites{kNotEnabled};
    ExtEnabled vk_nv_ray_tracing_validation{kNotEnabled};
    ExtEnabled vk_nv_cluster_acceleration_structure{kNotEnabled};
    ExtEnabled vk_nv_partitioned_acceleration_structure{kNotEnabled};
    ExtEnabled vk_ext_device_generated_commands{kNotEnabled};
    ExtEnabled vk_mesa_image_alignment_control{kNotEnabled};
    ExtEnabled vk_ext_depth_clamp_control{kNotEnabled};
    ExtEnabled vk_huawei_hdr_vivid{kNotEnabled};
    ExtEnabled vk_nv_cooperative_matrix2{kNotEnabled};
    ExtEnabled vk_arm_pipeline_opacity_micromap{kNotEnabled};
    ExtEnabled vk_ext_external_memory_metal{kNotEnabled};
    ExtEnabled vk_ext_vertex_attribute_robustness{kNotEnabled};
    ExtEnabled vk_khr_acceleration_structure{kNotEnabled};
    ExtEnabled vk_khr_ray_tracing_pipeline{kNotEnabled};
    ExtEnabled vk_khr_ray_query{kNotEnabled};
    ExtEnabled vk_ext_mesh_shader{kNotEnabled};

    struct Requirement {
        const ExtEnabled DeviceExtensions::*enabled;
        const char *name;
    };
    typedef std::vector<Requirement> RequirementVec;
    struct Info {
        Info(ExtEnabled DeviceExtensions::*state_, const RequirementVec requirements_)
            : state(state_), requirements(requirements_) {}
        ExtEnabled DeviceExtensions::*state;
        RequirementVec requirements;
    };

    typedef vvl::unordered_map<vvl::Extension, Info> InfoMap;
    static const InfoMap &GetInfoMap() {
        static const InfoMap info_map = {
            {vvl::Extension::_VK_KHR_swapchain,
             Info(&DeviceExtensions::vk_khr_swapchain, {{{&DeviceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_display_swapchain,
             Info(&DeviceExtensions::vk_khr_display_swapchain,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_display, VK_KHR_DISPLAY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge,
             Info(&DeviceExtensions::vk_khr_sampler_mirror_clamp_to_edge, {})},
            {vvl::Extension::_VK_KHR_video_queue,
             Info(&DeviceExtensions::vk_khr_video_queue,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_decode_queue,
             Info(&DeviceExtensions::vk_khr_video_decode_queue,
                  {{{&DeviceExtensions::vk_khr_video_queue, VK_KHR_VIDEO_QUEUE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_encode_h264,
             Info(&DeviceExtensions::vk_khr_video_encode_h264,
                  {{{&DeviceExtensions::vk_khr_video_encode_queue, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_encode_h265,
             Info(&DeviceExtensions::vk_khr_video_encode_h265,
                  {{{&DeviceExtensions::vk_khr_video_encode_queue, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_decode_h264,
             Info(&DeviceExtensions::vk_khr_video_decode_h264,
                  {{{&DeviceExtensions::vk_khr_video_decode_queue, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_dynamic_rendering,
             Info(&DeviceExtensions::vk_khr_dynamic_rendering,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_depth_stencil_resolve, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_multiview,
             Info(&DeviceExtensions::vk_khr_multiview, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                          VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_device_group,
             Info(&DeviceExtensions::vk_khr_device_group,
                  {{{&DeviceExtensions::vk_khr_device_group_creation, VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_draw_parameters, Info(&DeviceExtensions::vk_khr_shader_draw_parameters, {})},
            {vvl::Extension::_VK_KHR_maintenance1, Info(&DeviceExtensions::vk_khr_maintenance1, {})},
            {vvl::Extension::_VK_KHR_external_memory,
             Info(&DeviceExtensions::vk_khr_external_memory, {{{&DeviceExtensions::vk_khr_external_memory_capabilities,
                                                                VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_external_memory_win32,
             Info(&DeviceExtensions::vk_khr_external_memory_win32,
                  {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_external_memory_fd,
             Info(&DeviceExtensions::vk_khr_external_memory_fd,
                  {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_win32_keyed_mutex,
             Info(&DeviceExtensions::vk_khr_win32_keyed_mutex,
                  {{{&DeviceExtensions::vk_khr_external_memory_win32, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_external_semaphore,
             Info(&DeviceExtensions::vk_khr_external_semaphore, {{{&DeviceExtensions::vk_khr_external_semaphore_capabilities,
                                                                   VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_external_semaphore_win32,
             Info(&DeviceExtensions::vk_khr_external_semaphore_win32,
                  {{{&DeviceExtensions::vk_khr_external_semaphore, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_external_semaphore_fd,
             Info(&DeviceExtensions::vk_khr_external_semaphore_fd,
                  {{{&DeviceExtensions::vk_khr_external_semaphore, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_push_descriptor,
             Info(&DeviceExtensions::vk_khr_push_descriptor, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_float16_int8,
             Info(&DeviceExtensions::vk_khr_shader_float16_int8, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_16bit_storage,
             Info(&DeviceExtensions::vk_khr_16bit_storage, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                              VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                                                             {&DeviceExtensions::vk_khr_storage_buffer_storage_class,
                                                              VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_incremental_present,
             Info(&DeviceExtensions::vk_khr_incremental_present,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_descriptor_update_template, Info(&DeviceExtensions::vk_khr_descriptor_update_template, {})},
            {vvl::Extension::_VK_KHR_imageless_framebuffer,
             Info(&DeviceExtensions::vk_khr_imageless_framebuffer,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_maintenance2, VK_KHR_MAINTENANCE_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_image_format_list, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_create_renderpass2,
             Info(&DeviceExtensions::vk_khr_create_renderpass2,
                  {{{&DeviceExtensions::vk_khr_multiview, VK_KHR_MULTIVIEW_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_maintenance2, VK_KHR_MAINTENANCE_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shared_presentable_image,
             Info(&DeviceExtensions::vk_khr_shared_presentable_image,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_external_fence,
             Info(&DeviceExtensions::vk_khr_external_fence,
                  {{{&DeviceExtensions::vk_khr_external_fence_capabilities, VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_external_fence_win32,
             Info(&DeviceExtensions::vk_khr_external_fence_win32,
                  {{{&DeviceExtensions::vk_khr_external_fence, VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_KHR_external_fence_fd,
             Info(&DeviceExtensions::vk_khr_external_fence_fd,
                  {{{&DeviceExtensions::vk_khr_external_fence, VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_performance_query,
             Info(&DeviceExtensions::vk_khr_performance_query, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_maintenance2, Info(&DeviceExtensions::vk_khr_maintenance2, {})},
            {vvl::Extension::_VK_KHR_variable_pointers,
             Info(&DeviceExtensions::vk_khr_variable_pointers, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                                                                 {&DeviceExtensions::vk_khr_storage_buffer_storage_class,
                                                                  VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_dedicated_allocation,
             Info(&DeviceExtensions::vk_khr_dedicated_allocation,
                  {{{&DeviceExtensions::vk_khr_get_memory_requirements2, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_storage_buffer_storage_class,
             Info(&DeviceExtensions::vk_khr_storage_buffer_storage_class, {})},
            {vvl::Extension::_VK_KHR_relaxed_block_layout, Info(&DeviceExtensions::vk_khr_relaxed_block_layout, {})},
            {vvl::Extension::_VK_KHR_get_memory_requirements2, Info(&DeviceExtensions::vk_khr_get_memory_requirements2, {})},
            {vvl::Extension::_VK_KHR_image_format_list, Info(&DeviceExtensions::vk_khr_image_format_list, {})},
            {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion,
             Info(&DeviceExtensions::vk_khr_sampler_ycbcr_conversion,
                  {{{&DeviceExtensions::vk_khr_maintenance1, VK_KHR_MAINTENANCE_1_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_bind_memory2, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_memory_requirements2, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_bind_memory2, Info(&DeviceExtensions::vk_khr_bind_memory2, {})},
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {vvl::Extension::_VK_KHR_portability_subset,
             Info(&DeviceExtensions::vk_khr_portability_subset, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#endif  // VK_ENABLE_BETA_EXTENSIONS
            {vvl::Extension::_VK_KHR_maintenance3,
             Info(&DeviceExtensions::vk_khr_maintenance3, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_draw_indirect_count, Info(&DeviceExtensions::vk_khr_draw_indirect_count, {})},
            {vvl::Extension::_VK_KHR_shader_subgroup_extended_types,
             Info(&DeviceExtensions::vk_khr_shader_subgroup_extended_types,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_KHR_8bit_storage,
             Info(&DeviceExtensions::vk_khr_8bit_storage, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                                                            {&DeviceExtensions::vk_khr_storage_buffer_storage_class,
                                                             VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_atomic_int64,
             Info(&DeviceExtensions::vk_khr_shader_atomic_int64, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_clock,
             Info(&DeviceExtensions::vk_khr_shader_clock, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_decode_h265,
             Info(&DeviceExtensions::vk_khr_video_decode_h265,
                  {{{&DeviceExtensions::vk_khr_video_decode_queue, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_global_priority,
             Info(&DeviceExtensions::vk_khr_global_priority, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_driver_properties,
             Info(&DeviceExtensions::vk_khr_driver_properties, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_float_controls,
             Info(&DeviceExtensions::vk_khr_shader_float_controls, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_depth_stencil_resolve,
             Info(&DeviceExtensions::vk_khr_depth_stencil_resolve,
                  {{{&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_swapchain_mutable_format,
             Info(&DeviceExtensions::vk_khr_swapchain_mutable_format,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_maintenance2, VK_KHR_MAINTENANCE_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_image_format_list, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_timeline_semaphore,
             Info(&DeviceExtensions::vk_khr_timeline_semaphore, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_vulkan_memory_model,
             Info(&DeviceExtensions::vk_khr_vulkan_memory_model, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_terminate_invocation,
             Info(&DeviceExtensions::vk_khr_shader_terminate_invocation,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_fragment_shading_rate,
             Info(&DeviceExtensions::vk_khr_fragment_shading_rate,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_dynamic_rendering_local_read,
             Info(&DeviceExtensions::vk_khr_dynamic_rendering_local_read,
                  {{{&DeviceExtensions::vk_khr_dynamic_rendering, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_quad_control,
             Info(
                 &DeviceExtensions::vk_khr_shader_quad_control,
                 {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                   {&DeviceExtensions::vk_khr_vulkan_memory_model, VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME},
                   {&DeviceExtensions::vk_khr_shader_maximal_reconvergence, VK_KHR_SHADER_MAXIMAL_RECONVERGENCE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_spirv_1_4,
             Info(&DeviceExtensions::vk_khr_spirv_1_4,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                    {&DeviceExtensions::vk_khr_shader_float_controls, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_separate_depth_stencil_layouts,
             Info(&DeviceExtensions::vk_khr_separate_depth_stencil_layouts,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_present_wait,
             Info(&DeviceExtensions::vk_khr_present_wait,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_present_id, VK_KHR_PRESENT_ID_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_uniform_buffer_standard_layout,
             Info(&DeviceExtensions::vk_khr_uniform_buffer_standard_layout,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_buffer_device_address,
             Info(&DeviceExtensions::vk_khr_buffer_device_address,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_device_group, VK_KHR_DEVICE_GROUP_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_deferred_host_operations, Info(&DeviceExtensions::vk_khr_deferred_host_operations, {})},
            {vvl::Extension::_VK_KHR_pipeline_executable_properties,
             Info(&DeviceExtensions::vk_khr_pipeline_executable_properties,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_map_memory2, Info(&DeviceExtensions::vk_khr_map_memory2, {})},
            {vvl::Extension::_VK_KHR_shader_integer_dot_product,
             Info(&DeviceExtensions::vk_khr_shader_integer_dot_product,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_pipeline_library, Info(&DeviceExtensions::vk_khr_pipeline_library, {})},
            {vvl::Extension::_VK_KHR_shader_non_semantic_info, Info(&DeviceExtensions::vk_khr_shader_non_semantic_info, {})},
            {vvl::Extension::_VK_KHR_present_id,
             Info(&DeviceExtensions::vk_khr_present_id, {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                                                          {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_encode_queue,
             Info(&DeviceExtensions::vk_khr_video_encode_queue,
                  {{{&DeviceExtensions::vk_khr_video_queue, VK_KHR_VIDEO_QUEUE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_synchronization2,
             Info(&DeviceExtensions::vk_khr_synchronization2, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_fragment_shader_barycentric,
             Info(&DeviceExtensions::vk_khr_fragment_shader_barycentric,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_subgroup_uniform_control_flow,
             Info(&DeviceExtensions::vk_khr_shader_subgroup_uniform_control_flow,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_KHR_zero_initialize_workgroup_memory,
             Info(&DeviceExtensions::vk_khr_zero_initialize_workgroup_memory,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_workgroup_memory_explicit_layout,
             Info(&DeviceExtensions::vk_khr_workgroup_memory_explicit_layout,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_copy_commands2,
             Info(&DeviceExtensions::vk_khr_copy_commands2, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_format_feature_flags2,
             Info(&DeviceExtensions::vk_khr_format_feature_flags2, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_ray_tracing_maintenance1,
             Info(&DeviceExtensions::vk_khr_ray_tracing_maintenance1,
                  {{{&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_maintenance4,
             Info(&DeviceExtensions::vk_khr_maintenance4, {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_KHR_shader_subgroup_rotate, Info(&DeviceExtensions::vk_khr_shader_subgroup_rotate, {})},
            {vvl::Extension::_VK_KHR_shader_maximal_reconvergence,
             Info(&DeviceExtensions::vk_khr_shader_maximal_reconvergence,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_KHR_maintenance5,
             Info(&DeviceExtensions::vk_khr_maintenance5,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                    {&DeviceExtensions::vk_khr_dynamic_rendering, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_ray_tracing_position_fetch,
             Info(&DeviceExtensions::vk_khr_ray_tracing_position_fetch,
                  {{{&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_pipeline_binary,
             Info(&DeviceExtensions::vk_khr_pipeline_binary,
                  {{{&DeviceExtensions::vk_khr_maintenance5, VK_KHR_MAINTENANCE_5_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_cooperative_matrix,
             Info(&DeviceExtensions::vk_khr_cooperative_matrix, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_compute_shader_derivatives,
             Info(&DeviceExtensions::vk_khr_compute_shader_derivatives,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_decode_av1,
             Info(&DeviceExtensions::vk_khr_video_decode_av1,
                  {{{&DeviceExtensions::vk_khr_video_decode_queue, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_encode_av1,
             Info(&DeviceExtensions::vk_khr_video_encode_av1,
                  {{{&DeviceExtensions::vk_khr_video_encode_queue, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_video_maintenance1,
             Info(&DeviceExtensions::vk_khr_video_maintenance1,
                  {{{&DeviceExtensions::vk_khr_video_queue, VK_KHR_VIDEO_QUEUE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_vertex_attribute_divisor, Info(&DeviceExtensions::vk_khr_vertex_attribute_divisor,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_load_store_op_none, Info(&DeviceExtensions::vk_khr_load_store_op_none, {})},
            {vvl::Extension::_VK_KHR_shader_float_controls2,
             Info(&DeviceExtensions::vk_khr_shader_float_controls2,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                    {&DeviceExtensions::vk_khr_shader_float_controls, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_index_type_uint8,
             Info(&DeviceExtensions::vk_khr_index_type_uint8, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_line_rasterization,
             Info(&DeviceExtensions::vk_khr_line_rasterization, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_calibrated_timestamps,
             Info(&DeviceExtensions::vk_khr_calibrated_timestamps, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_expect_assume,
             Info(&DeviceExtensions::vk_khr_shader_expect_assume, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_maintenance6,
             Info(&DeviceExtensions::vk_khr_maintenance6, {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_KHR_video_encode_quantization_map,
             Info(&DeviceExtensions::vk_khr_video_encode_quantization_map,
                  {{{&DeviceExtensions::vk_khr_video_encode_queue, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_format_feature_flags2, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_shader_relaxed_extended_instruction,
             Info(&DeviceExtensions::vk_khr_shader_relaxed_extended_instruction, {})},
            {vvl::Extension::_VK_KHR_maintenance7,
             Info(&DeviceExtensions::vk_khr_maintenance7, {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_KHR_maintenance8,
             Info(&DeviceExtensions::vk_khr_maintenance8, {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_KHR_video_maintenance2,
             Info(&DeviceExtensions::vk_khr_video_maintenance2,
                  {{{&DeviceExtensions::vk_khr_video_decode_queue, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_video_encode_queue, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_depth_clamp_zero_one,
             Info(&DeviceExtensions::vk_khr_depth_clamp_zero_one, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_glsl_shader, Info(&DeviceExtensions::vk_nv_glsl_shader, {})},
            {vvl::Extension::_VK_EXT_depth_range_unrestricted, Info(&DeviceExtensions::vk_ext_depth_range_unrestricted, {})},
            {vvl::Extension::_VK_IMG_filter_cubic, Info(&DeviceExtensions::vk_img_filter_cubic, {})},
            {vvl::Extension::_VK_AMD_rasterization_order, Info(&DeviceExtensions::vk_amd_rasterization_order, {})},
            {vvl::Extension::_VK_AMD_shader_trinary_minmax, Info(&DeviceExtensions::vk_amd_shader_trinary_minmax, {})},
            {vvl::Extension::_VK_AMD_shader_explicit_vertex_parameter,
             Info(&DeviceExtensions::vk_amd_shader_explicit_vertex_parameter, {})},
            {vvl::Extension::_VK_EXT_debug_marker,
             Info(&DeviceExtensions::vk_ext_debug_marker,
                  {{{&DeviceExtensions::vk_ext_debug_report, VK_EXT_DEBUG_REPORT_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_gcn_shader, Info(&DeviceExtensions::vk_amd_gcn_shader, {})},
            {vvl::Extension::_VK_NV_dedicated_allocation, Info(&DeviceExtensions::vk_nv_dedicated_allocation, {})},
            {vvl::Extension::_VK_EXT_transform_feedback,
             Info(&DeviceExtensions::vk_ext_transform_feedback, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NVX_binary_import, Info(&DeviceExtensions::vk_nvx_binary_import, {})},
            {vvl::Extension::_VK_NVX_image_view_handle, Info(&DeviceExtensions::vk_nvx_image_view_handle, {})},
            {vvl::Extension::_VK_AMD_draw_indirect_count, Info(&DeviceExtensions::vk_amd_draw_indirect_count, {})},
            {vvl::Extension::_VK_AMD_negative_viewport_height, Info(&DeviceExtensions::vk_amd_negative_viewport_height, {})},
            {vvl::Extension::_VK_AMD_gpu_shader_half_float, Info(&DeviceExtensions::vk_amd_gpu_shader_half_float, {})},
            {vvl::Extension::_VK_AMD_shader_ballot, Info(&DeviceExtensions::vk_amd_shader_ballot, {})},
            {vvl::Extension::_VK_AMD_texture_gather_bias_lod,
             Info(&DeviceExtensions::vk_amd_texture_gather_bias_lod, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_shader_info, Info(&DeviceExtensions::vk_amd_shader_info, {})},
            {vvl::Extension::_VK_AMD_shader_image_load_store_lod, Info(&DeviceExtensions::vk_amd_shader_image_load_store_lod, {})},
            {vvl::Extension::_VK_NV_corner_sampled_image,
             Info(&DeviceExtensions::vk_nv_corner_sampled_image, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_IMG_format_pvrtc, Info(&DeviceExtensions::vk_img_format_pvrtc, {})},
            {vvl::Extension::_VK_NV_external_memory,
             Info(&DeviceExtensions::vk_nv_external_memory,
                  {{{&DeviceExtensions::vk_nv_external_memory_capabilities, VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_NV_external_memory_win32,
             Info(&DeviceExtensions::vk_nv_external_memory_win32,
                  {{{&DeviceExtensions::vk_nv_external_memory, VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_win32_keyed_mutex,
             Info(&DeviceExtensions::vk_nv_win32_keyed_mutex,
                  {{{&DeviceExtensions::vk_nv_external_memory_win32, VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_EXT_shader_subgroup_ballot, Info(&DeviceExtensions::vk_ext_shader_subgroup_ballot, {})},
            {vvl::Extension::_VK_EXT_shader_subgroup_vote, Info(&DeviceExtensions::vk_ext_shader_subgroup_vote, {})},
            {vvl::Extension::_VK_EXT_texture_compression_astc_hdr,
             Info(&DeviceExtensions::vk_ext_texture_compression_astc_hdr,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_astc_decode_mode,
             Info(&DeviceExtensions::vk_ext_astc_decode_mode, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_pipeline_robustness,
             Info(&DeviceExtensions::vk_ext_pipeline_robustness, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_conditional_rendering,
             Info(&DeviceExtensions::vk_ext_conditional_rendering, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_clip_space_w_scaling, Info(&DeviceExtensions::vk_nv_clip_space_w_scaling, {})},
            {vvl::Extension::_VK_EXT_display_control,
             Info(&DeviceExtensions::vk_ext_display_control,
                  {{{&DeviceExtensions::vk_ext_display_surface_counter, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_GOOGLE_display_timing,
             Info(&DeviceExtensions::vk_google_display_timing,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_sample_mask_override_coverage,
             Info(&DeviceExtensions::vk_nv_sample_mask_override_coverage, {})},
            {vvl::Extension::_VK_NV_geometry_shader_passthrough, Info(&DeviceExtensions::vk_nv_geometry_shader_passthrough, {})},
            {vvl::Extension::_VK_NV_viewport_array2, Info(&DeviceExtensions::vk_nv_viewport_array2, {})},
            {vvl::Extension::_VK_NVX_multiview_per_view_attributes,
             Info(&DeviceExtensions::vk_nvx_multiview_per_view_attributes,
                  {{{&DeviceExtensions::vk_khr_multiview, VK_KHR_MULTIVIEW_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_viewport_swizzle, Info(&DeviceExtensions::vk_nv_viewport_swizzle, {})},
            {vvl::Extension::_VK_EXT_discard_rectangles,
             Info(&DeviceExtensions::vk_ext_discard_rectangles, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_conservative_rasterization,
             Info(&DeviceExtensions::vk_ext_conservative_rasterization,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_depth_clip_enable,
             Info(&DeviceExtensions::vk_ext_depth_clip_enable, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_hdr_metadata,
             Info(&DeviceExtensions::vk_ext_hdr_metadata,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_IMG_relaxed_line_rasterization,
             Info(&DeviceExtensions::vk_img_relaxed_line_rasterization,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_external_memory_dma_buf,
             Info(&DeviceExtensions::vk_ext_external_memory_dma_buf,
                  {{{&DeviceExtensions::vk_khr_external_memory_fd, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_queue_family_foreign,
             Info(&DeviceExtensions::vk_ext_queue_family_foreign,
                  {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            {vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer,
             Info(&DeviceExtensions::vk_android_external_memory_android_hardware_buffer,
                  {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_dedicated_allocation, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_queue_family_foreign, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_ANDROID_KHR
            {vvl::Extension::_VK_EXT_sampler_filter_minmax,
             Info(&DeviceExtensions::vk_ext_sampler_filter_minmax, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_gpu_shader_int16, Info(&DeviceExtensions::vk_amd_gpu_shader_int16, {})},
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {vvl::Extension::_VK_AMDX_shader_enqueue,
             Info(&DeviceExtensions::vk_amdx_shader_enqueue,
                  {{{&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_spirv_1_4, VK_KHR_SPIRV_1_4_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_extended_dynamic_state, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_maintenance5, VK_KHR_MAINTENANCE_5_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_pipeline_library, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME}}})},
#endif  // VK_ENABLE_BETA_EXTENSIONS
            {vvl::Extension::_VK_AMD_mixed_attachment_samples, Info(&DeviceExtensions::vk_amd_mixed_attachment_samples, {})},
            {vvl::Extension::_VK_AMD_shader_fragment_mask, Info(&DeviceExtensions::vk_amd_shader_fragment_mask, {})},
            {vvl::Extension::_VK_EXT_inline_uniform_block,
             Info(&DeviceExtensions::vk_ext_inline_uniform_block,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_maintenance1, VK_KHR_MAINTENANCE_1_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_shader_stencil_export, Info(&DeviceExtensions::vk_ext_shader_stencil_export, {})},
            {vvl::Extension::_VK_EXT_sample_locations,
             Info(&DeviceExtensions::vk_ext_sample_locations, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_blend_operation_advanced, Info(&DeviceExtensions::vk_ext_blend_operation_advanced,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_fragment_coverage_to_color, Info(&DeviceExtensions::vk_nv_fragment_coverage_to_color, {})},
            {vvl::Extension::_VK_NV_framebuffer_mixed_samples, Info(&DeviceExtensions::vk_nv_framebuffer_mixed_samples, {})},
            {vvl::Extension::_VK_NV_fill_rectangle, Info(&DeviceExtensions::vk_nv_fill_rectangle, {})},
            {vvl::Extension::_VK_NV_shader_sm_builtins,
             Info(&DeviceExtensions::vk_nv_shader_sm_builtins, {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_EXT_post_depth_coverage, Info(&DeviceExtensions::vk_ext_post_depth_coverage, {})},
            {vvl::Extension::_VK_EXT_image_drm_format_modifier,
             Info(&DeviceExtensions::vk_ext_image_drm_format_modifier,
                  {{{&DeviceExtensions::vk_khr_bind_memory2, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_image_format_list, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_validation_cache, Info(&DeviceExtensions::vk_ext_validation_cache, {})},
            {vvl::Extension::_VK_EXT_descriptor_indexing,
             Info(&DeviceExtensions::vk_ext_descriptor_indexing,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_maintenance3, VK_KHR_MAINTENANCE_3_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_shader_viewport_index_layer, Info(&DeviceExtensions::vk_ext_shader_viewport_index_layer, {})},
            {vvl::Extension::_VK_NV_shading_rate_image,
             Info(&DeviceExtensions::vk_nv_shading_rate_image, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_ray_tracing,
             Info(&DeviceExtensions::vk_nv_ray_tracing,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_memory_requirements2, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_representative_fragment_test,
             Info(&DeviceExtensions::vk_nv_representative_fragment_test,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_filter_cubic, Info(&DeviceExtensions::vk_ext_filter_cubic, {})},
            {vvl::Extension::_VK_QCOM_render_pass_shader_resolve, Info(&DeviceExtensions::vk_qcom_render_pass_shader_resolve, {})},
            {vvl::Extension::_VK_EXT_global_priority, Info(&DeviceExtensions::vk_ext_global_priority, {})},
            {vvl::Extension::_VK_EXT_external_memory_host,
             Info(&DeviceExtensions::vk_ext_external_memory_host,
                  {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_buffer_marker, Info(&DeviceExtensions::vk_amd_buffer_marker, {})},
            {vvl::Extension::_VK_AMD_pipeline_compiler_control, Info(&DeviceExtensions::vk_amd_pipeline_compiler_control, {})},
            {vvl::Extension::_VK_EXT_calibrated_timestamps,
             Info(&DeviceExtensions::vk_ext_calibrated_timestamps, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_shader_core_properties,
             Info(&DeviceExtensions::vk_amd_shader_core_properties, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_memory_overallocation_behavior,
             Info(&DeviceExtensions::vk_amd_memory_overallocation_behavior, {})},
            {vvl::Extension::_VK_EXT_vertex_attribute_divisor, Info(&DeviceExtensions::vk_ext_vertex_attribute_divisor,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_GGP
            {vvl::Extension::_VK_GGP_frame_token,
             Info(&DeviceExtensions::vk_ggp_frame_token,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ggp_stream_descriptor_surface, VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_GGP
            {vvl::Extension::_VK_EXT_pipeline_creation_feedback, Info(&DeviceExtensions::vk_ext_pipeline_creation_feedback, {})},
            {vvl::Extension::_VK_NV_shader_subgroup_partitioned,
             Info(&DeviceExtensions::vk_nv_shader_subgroup_partitioned,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_NV_compute_shader_derivatives, Info(&DeviceExtensions::vk_nv_compute_shader_derivatives,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_mesh_shader,
             Info(&DeviceExtensions::vk_nv_mesh_shader, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_fragment_shader_barycentric,
             Info(&DeviceExtensions::vk_nv_fragment_shader_barycentric,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_shader_image_footprint,
             Info(&DeviceExtensions::vk_nv_shader_image_footprint, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_scissor_exclusive,
             Info(&DeviceExtensions::vk_nv_scissor_exclusive, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_device_diagnostic_checkpoints,
             Info(&DeviceExtensions::vk_nv_device_diagnostic_checkpoints,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_INTEL_shader_integer_functions2,
             Info(&DeviceExtensions::vk_intel_shader_integer_functions2,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_INTEL_performance_query, Info(&DeviceExtensions::vk_intel_performance_query, {})},
            {vvl::Extension::_VK_EXT_pci_bus_info,
             Info(&DeviceExtensions::vk_ext_pci_bus_info, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_display_native_hdr,
             Info(&DeviceExtensions::vk_amd_display_native_hdr,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_fragment_density_map,
             Info(&DeviceExtensions::vk_ext_fragment_density_map, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_scalar_block_layout,
             Info(&DeviceExtensions::vk_ext_scalar_block_layout, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_GOOGLE_hlsl_functionality1, Info(&DeviceExtensions::vk_google_hlsl_functionality1, {})},
            {vvl::Extension::_VK_GOOGLE_decorate_string, Info(&DeviceExtensions::vk_google_decorate_string, {})},
            {vvl::Extension::_VK_EXT_subgroup_size_control,
             Info(&DeviceExtensions::vk_ext_subgroup_size_control,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_AMD_shader_core_properties2,
             Info(&DeviceExtensions::vk_amd_shader_core_properties2,
                  {{{&DeviceExtensions::vk_amd_shader_core_properties, VK_AMD_SHADER_CORE_PROPERTIES_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_device_coherent_memory,
             Info(&DeviceExtensions::vk_amd_device_coherent_memory, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_shader_image_atomic_int64, Info(&DeviceExtensions::vk_ext_shader_image_atomic_int64,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_memory_budget,
             Info(&DeviceExtensions::vk_ext_memory_budget, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                              VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_memory_priority,
             Info(&DeviceExtensions::vk_ext_memory_priority, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_dedicated_allocation_image_aliasing,
             Info(&DeviceExtensions::vk_nv_dedicated_allocation_image_aliasing,
                  {{{&DeviceExtensions::vk_khr_dedicated_allocation, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_buffer_device_address,
             Info(&DeviceExtensions::vk_ext_buffer_device_address, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_tooling_info, Info(&DeviceExtensions::vk_ext_tooling_info, {})},
            {vvl::Extension::_VK_EXT_separate_stencil_usage, Info(&DeviceExtensions::vk_ext_separate_stencil_usage, {})},
            {vvl::Extension::_VK_NV_cooperative_matrix,
             Info(&DeviceExtensions::vk_nv_cooperative_matrix, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_coverage_reduction_mode,
             Info(&DeviceExtensions::vk_nv_coverage_reduction_mode,
                  {{{&DeviceExtensions::vk_nv_framebuffer_mixed_samples, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_fragment_shader_interlock, Info(&DeviceExtensions::vk_ext_fragment_shader_interlock,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_ycbcr_image_arrays,
             Info(&DeviceExtensions::vk_ext_ycbcr_image_arrays,
                  {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_provoking_vertex,
             Info(&DeviceExtensions::vk_ext_provoking_vertex, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_EXT_full_screen_exclusive,
             Info(&DeviceExtensions::vk_ext_full_screen_exclusive,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_EXT_line_rasterization,
             Info(&DeviceExtensions::vk_ext_line_rasterization, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_shader_atomic_float,
             Info(&DeviceExtensions::vk_ext_shader_atomic_float, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_host_query_reset,
             Info(&DeviceExtensions::vk_ext_host_query_reset, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_index_type_uint8,
             Info(&DeviceExtensions::vk_ext_index_type_uint8, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_extended_dynamic_state,
             Info(&DeviceExtensions::vk_ext_extended_dynamic_state, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_host_image_copy,
             Info(&DeviceExtensions::vk_ext_host_image_copy,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_copy_commands2, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_format_feature_flags2, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_map_memory_placed,
             Info(&DeviceExtensions::vk_ext_map_memory_placed,
                  {{{&DeviceExtensions::vk_khr_map_memory2, VK_KHR_MAP_MEMORY_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_shader_atomic_float2,
             Info(&DeviceExtensions::vk_ext_shader_atomic_float2,
                  {{{&DeviceExtensions::vk_ext_shader_atomic_float, VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_swapchain_maintenance1,
             Info(&DeviceExtensions::vk_ext_swapchain_maintenance1,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_surface_maintenance1, VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_shader_demote_to_helper_invocation,
             Info(&DeviceExtensions::vk_ext_shader_demote_to_helper_invocation,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_device_generated_commands,
             Info(&DeviceExtensions::vk_nv_device_generated_commands,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                    {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_inherited_viewport_scissor, Info(&DeviceExtensions::vk_nv_inherited_viewport_scissor,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_texel_buffer_alignment,
             Info(&DeviceExtensions::vk_ext_texel_buffer_alignment, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_render_pass_transform, Info(&DeviceExtensions::vk_qcom_render_pass_transform, {})},
            {vvl::Extension::_VK_EXT_depth_bias_control,
             Info(&DeviceExtensions::vk_ext_depth_bias_control, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_device_memory_report,
             Info(&DeviceExtensions::vk_ext_device_memory_report, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_robustness2,
             Info(&DeviceExtensions::vk_ext_robustness2, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_custom_border_color,
             Info(&DeviceExtensions::vk_ext_custom_border_color, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_GOOGLE_user_type, Info(&DeviceExtensions::vk_google_user_type, {})},
            {vvl::Extension::_VK_NV_present_barrier,
             Info(&DeviceExtensions::vk_nv_present_barrier,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_surface, VK_KHR_SURFACE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_surface_capabilities2, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_private_data,
             Info(&DeviceExtensions::vk_ext_private_data, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_pipeline_creation_cache_control,
             Info(&DeviceExtensions::vk_ext_pipeline_creation_cache_control,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_device_diagnostics_config, Info(&DeviceExtensions::vk_nv_device_diagnostics_config,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_render_pass_store_ops, Info(&DeviceExtensions::vk_qcom_render_pass_store_ops, {})},
            {vvl::Extension::_VK_NV_cuda_kernel_launch, Info(&DeviceExtensions::vk_nv_cuda_kernel_launch, {})},
            {vvl::Extension::_VK_NV_low_latency, Info(&DeviceExtensions::vk_nv_low_latency, {})},
#ifdef VK_USE_PLATFORM_METAL_EXT
            {vvl::Extension::_VK_EXT_metal_objects, Info(&DeviceExtensions::vk_ext_metal_objects, {})},
#endif  // VK_USE_PLATFORM_METAL_EXT
            {vvl::Extension::_VK_EXT_descriptor_buffer,
             Info(&DeviceExtensions::vk_ext_descriptor_buffer,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_descriptor_indexing, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_graphics_pipeline_library,
             Info(&DeviceExtensions::vk_ext_graphics_pipeline_library,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_pipeline_library, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_AMD_shader_early_and_late_fragment_tests,
             Info(&DeviceExtensions::vk_amd_shader_early_and_late_fragment_tests,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_fragment_shading_rate_enums,
             Info(&DeviceExtensions::vk_nv_fragment_shading_rate_enums,
                  {{{&DeviceExtensions::vk_khr_fragment_shading_rate, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_ray_tracing_motion_blur,
             Info(&DeviceExtensions::vk_nv_ray_tracing_motion_blur,
                  {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats,
             Info(&DeviceExtensions::vk_ext_ycbcr_2plane_444_formats,
                  {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_fragment_density_map2,
             Info(&DeviceExtensions::vk_ext_fragment_density_map2,
                  {{{&DeviceExtensions::vk_ext_fragment_density_map, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_rotated_copy_commands,
             Info(&DeviceExtensions::vk_qcom_rotated_copy_commands,
                  {{{&DeviceExtensions::vk_khr_copy_commands2, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_image_robustness,
             Info(&DeviceExtensions::vk_ext_image_robustness, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_image_compression_control, Info(&DeviceExtensions::vk_ext_image_compression_control,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_attachment_feedback_loop_layout,
             Info(&DeviceExtensions::vk_ext_attachment_feedback_loop_layout,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_4444_formats,
             Info(&DeviceExtensions::vk_ext_4444_formats, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_device_fault,
             Info(&DeviceExtensions::vk_ext_device_fault, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                             VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_ARM_rasterization_order_attachment_access,
             Info(&DeviceExtensions::vk_arm_rasterization_order_attachment_access,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_rgba10x6_formats,
             Info(&DeviceExtensions::vk_ext_rgba10x6_formats,
                  {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_NV_acquire_winrt_display,
             Info(&DeviceExtensions::vk_nv_acquire_winrt_display,
                  {{{&DeviceExtensions::vk_ext_direct_mode_display, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_WIN32_KHR
            {vvl::Extension::_VK_VALVE_mutable_descriptor_type,
             Info(&DeviceExtensions::vk_valve_mutable_descriptor_type,
                  {{{&DeviceExtensions::vk_khr_maintenance3, VK_KHR_MAINTENANCE_3_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_vertex_input_dynamic_state,
             Info(&DeviceExtensions::vk_ext_vertex_input_dynamic_state,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_physical_device_drm,
             Info(&DeviceExtensions::vk_ext_physical_device_drm, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_device_address_binding_report,
             Info(&DeviceExtensions::vk_ext_device_address_binding_report,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_debug_utils, VK_EXT_DEBUG_UTILS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_depth_clip_control,
             Info(&DeviceExtensions::vk_ext_depth_clip_control, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_primitive_topology_list_restart,
             Info(&DeviceExtensions::vk_ext_primitive_topology_list_restart,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_present_mode_fifo_latest_ready,
             Info(&DeviceExtensions::vk_ext_present_mode_fifo_latest_ready,
                  {{{&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_FUCHSIA
            {vvl::Extension::_VK_FUCHSIA_external_memory,
             Info(&DeviceExtensions::vk_fuchsia_external_memory,
                  {{{&DeviceExtensions::vk_khr_external_memory_capabilities, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_FUCHSIA_external_semaphore,
             Info(&DeviceExtensions::vk_fuchsia_external_semaphore,
                  {{{&DeviceExtensions::vk_khr_external_semaphore_capabilities,
                     VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_external_semaphore, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_FUCHSIA_buffer_collection,
             Info(&DeviceExtensions::vk_fuchsia_buffer_collection,
                  {{{&DeviceExtensions::vk_fuchsia_external_memory, VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_FUCHSIA
            {vvl::Extension::_VK_HUAWEI_subpass_shading,
             Info(&DeviceExtensions::vk_huawei_subpass_shading,
                  {{{&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_HUAWEI_invocation_mask,
             Info(&DeviceExtensions::vk_huawei_invocation_mask,
                  {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_external_memory_rdma,
             Info(&DeviceExtensions::vk_nv_external_memory_rdma,
                  {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_pipeline_properties,
             Info(&DeviceExtensions::vk_ext_pipeline_properties, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_frame_boundary, Info(&DeviceExtensions::vk_ext_frame_boundary, {})},
            {vvl::Extension::_VK_EXT_multisampled_render_to_single_sampled,
             Info(&DeviceExtensions::vk_ext_multisampled_render_to_single_sampled,
                  {{{&DeviceExtensions::vk_khr_create_renderpass2, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_depth_stencil_resolve, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_extended_dynamic_state2,
             Info(&DeviceExtensions::vk_ext_extended_dynamic_state2, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_color_write_enable,
             Info(&DeviceExtensions::vk_ext_color_write_enable, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_primitives_generated_query,
             Info(&DeviceExtensions::vk_ext_primitives_generated_query,
                  {{{&DeviceExtensions::vk_ext_transform_feedback, VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_global_priority_query,
             Info(&DeviceExtensions::vk_ext_global_priority_query,
                  {{{&DeviceExtensions::vk_ext_global_priority, VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_image_view_min_lod,
             Info(&DeviceExtensions::vk_ext_image_view_min_lod, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_multi_draw,
             Info(&DeviceExtensions::vk_ext_multi_draw, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                           VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_image_2d_view_of_3d,
             Info(&DeviceExtensions::vk_ext_image_2d_view_of_3d,
                  {{{&DeviceExtensions::vk_khr_maintenance1, VK_KHR_MAINTENANCE_1_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_shader_tile_image,
             Info(&DeviceExtensions::vk_ext_shader_tile_image, {{{&DeviceExtensions::vk_feature_version_1_3, "VK_VERSION_1_3"}}})},
            {vvl::Extension::_VK_EXT_opacity_micromap,
             Info(&DeviceExtensions::vk_ext_opacity_micromap,
                  {{{&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {vvl::Extension::_VK_NV_displacement_micromap,
             Info(&DeviceExtensions::vk_nv_displacement_micromap,
                  {{{&DeviceExtensions::vk_ext_opacity_micromap, VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME}}})},
#endif  // VK_ENABLE_BETA_EXTENSIONS
            {vvl::Extension::_VK_EXT_load_store_op_none, Info(&DeviceExtensions::vk_ext_load_store_op_none, {})},
            {vvl::Extension::_VK_HUAWEI_cluster_culling_shader, Info(&DeviceExtensions::vk_huawei_cluster_culling_shader,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_border_color_swizzle,
             Info(&DeviceExtensions::vk_ext_border_color_swizzle,
                  {{{&DeviceExtensions::vk_ext_custom_border_color, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_pageable_device_local_memory,
             Info(&DeviceExtensions::vk_ext_pageable_device_local_memory,
                  {{{&DeviceExtensions::vk_ext_memory_priority, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_ARM_shader_core_properties,
             Info(&DeviceExtensions::vk_arm_shader_core_properties,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_ARM_scheduling_controls,
             Info(&DeviceExtensions::vk_arm_scheduling_controls,
                  {{{&DeviceExtensions::vk_arm_shader_core_builtins, VK_ARM_SHADER_CORE_BUILTINS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_image_sliced_view_of_3d,
             Info(&DeviceExtensions::vk_ext_image_sliced_view_of_3d,
                  {{{&DeviceExtensions::vk_khr_maintenance1, VK_KHR_MAINTENANCE_1_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_VALVE_descriptor_set_host_mapping,
             Info(&DeviceExtensions::vk_valve_descriptor_set_host_mapping,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_depth_clamp_zero_one,
             Info(&DeviceExtensions::vk_ext_depth_clamp_zero_one, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_non_seamless_cube_map,
             Info(&DeviceExtensions::vk_ext_non_seamless_cube_map, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_ARM_render_pass_striped,
             Info(&DeviceExtensions::vk_arm_render_pass_striped,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_fragment_density_map_offset,
             Info(&DeviceExtensions::vk_qcom_fragment_density_map_offset,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_fragment_density_map, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_copy_memory_indirect,
             Info(&DeviceExtensions::vk_nv_copy_memory_indirect,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_memory_decompression,
             Info(&DeviceExtensions::vk_nv_memory_decompression,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_device_generated_commands_compute,
             Info(&DeviceExtensions::vk_nv_device_generated_commands_compute,
                  {{{&DeviceExtensions::vk_nv_device_generated_commands, VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_ray_tracing_linear_swept_spheres,
             Info(&DeviceExtensions::vk_nv_ray_tracing_linear_swept_spheres,
                  {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_linear_color_attachment,
             Info(&DeviceExtensions::vk_nv_linear_color_attachment, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_image_compression_control_swapchain,
             Info(&DeviceExtensions::vk_ext_image_compression_control_swapchain,
                  {{{&DeviceExtensions::vk_ext_image_compression_control, VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_image_processing,
             Info(&DeviceExtensions::vk_qcom_image_processing,
                  {{{&DeviceExtensions::vk_khr_format_feature_flags2, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_nested_command_buffer,
             Info(&DeviceExtensions::vk_ext_nested_command_buffer, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_external_memory_acquire_unmodified,
             Info(&DeviceExtensions::vk_ext_external_memory_acquire_unmodified,
                  {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_extended_dynamic_state3,
             Info(&DeviceExtensions::vk_ext_extended_dynamic_state3, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_subpass_merge_feedback,
             Info(&DeviceExtensions::vk_ext_subpass_merge_feedback, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_shader_module_identifier,
             Info(&DeviceExtensions::vk_ext_shader_module_identifier, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                         VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                                                                        {&DeviceExtensions::vk_ext_pipeline_creation_cache_control,
                                                                         VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_rasterization_order_attachment_access,
             Info(&DeviceExtensions::vk_ext_rasterization_order_attachment_access,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_optical_flow,
             Info(&DeviceExtensions::vk_nv_optical_flow,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_format_feature_flags2, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_synchronization2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_legacy_dithering,
             Info(&DeviceExtensions::vk_ext_legacy_dithering, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_pipeline_protected_access, Info(&DeviceExtensions::vk_ext_pipeline_protected_access,
                                                                     {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            {vvl::Extension::_VK_ANDROID_external_format_resolve,
             Info(&DeviceExtensions::vk_android_external_format_resolve,
                  {{{&DeviceExtensions::vk_android_external_memory_android_hardware_buffer,
                     VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_ANDROID_KHR
            {vvl::Extension::_VK_AMD_anti_lag, Info(&DeviceExtensions::vk_amd_anti_lag, {})},
            {vvl::Extension::_VK_EXT_shader_object,
             Info(&DeviceExtensions::vk_ext_shader_object,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_dynamic_rendering, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_tile_properties,
             Info(&DeviceExtensions::vk_qcom_tile_properties, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                 VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_SEC_amigo_profiling,
             Info(&DeviceExtensions::vk_sec_amigo_profiling, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_multiview_per_view_viewports,
             Info(&DeviceExtensions::vk_qcom_multiview_per_view_viewports,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_ray_tracing_invocation_reorder,
             Info(&DeviceExtensions::vk_nv_ray_tracing_invocation_reorder,
                  {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_cooperative_vector, Info(&DeviceExtensions::vk_nv_cooperative_vector, {})},
            {vvl::Extension::_VK_NV_extended_sparse_address_space,
             Info(&DeviceExtensions::vk_nv_extended_sparse_address_space, {})},
            {vvl::Extension::_VK_EXT_mutable_descriptor_type,
             Info(&DeviceExtensions::vk_ext_mutable_descriptor_type,
                  {{{&DeviceExtensions::vk_khr_maintenance3, VK_KHR_MAINTENANCE_3_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_legacy_vertex_attributes,
             Info(&DeviceExtensions::vk_ext_legacy_vertex_attributes,
                  {{{&DeviceExtensions::vk_ext_vertex_input_dynamic_state, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_ARM_shader_core_builtins,
             Info(&DeviceExtensions::vk_arm_shader_core_builtins, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_pipeline_library_group_handles,
             Info(&DeviceExtensions::vk_ext_pipeline_library_group_handles,
                  {{{&DeviceExtensions::vk_khr_ray_tracing_pipeline, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_pipeline_library, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_dynamic_rendering_unused_attachments,
             Info(&DeviceExtensions::vk_ext_dynamic_rendering_unused_attachments,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_dynamic_rendering, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_low_latency2,
             Info(&DeviceExtensions::vk_nv_low_latency2,
                  {{{&DeviceExtensions::vk_khr_timeline_semaphore, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_multiview_per_view_render_areas,
             Info(&DeviceExtensions::vk_qcom_multiview_per_view_render_areas, {})},
            {vvl::Extension::_VK_NV_per_stage_descriptor_set,
             Info(&DeviceExtensions::vk_nv_per_stage_descriptor_set,
                  {{{&DeviceExtensions::vk_khr_maintenance6, VK_KHR_MAINTENANCE_6_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_image_processing2,
             Info(&DeviceExtensions::vk_qcom_image_processing2,
                  {{{&DeviceExtensions::vk_qcom_image_processing, VK_QCOM_IMAGE_PROCESSING_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_filter_cubic_weights,
             Info(&DeviceExtensions::vk_qcom_filter_cubic_weights,
                  {{{&DeviceExtensions::vk_ext_filter_cubic, VK_EXT_FILTER_CUBIC_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_QCOM_ycbcr_degamma, Info(&DeviceExtensions::vk_qcom_ycbcr_degamma, {})},
            {vvl::Extension::_VK_QCOM_filter_cubic_clamp,
             Info(&DeviceExtensions::vk_qcom_filter_cubic_clamp,
                  {{{&DeviceExtensions::vk_ext_filter_cubic, VK_EXT_FILTER_CUBIC_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_sampler_filter_minmax, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state,
             Info(&DeviceExtensions::vk_ext_attachment_feedback_loop_dynamic_state,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_attachment_feedback_loop_layout,
                     VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            {vvl::Extension::_VK_QNX_external_memory_screen_buffer,
             Info(&DeviceExtensions::vk_qnx_external_memory_screen_buffer,
                  {{{&DeviceExtensions::vk_khr_sampler_ycbcr_conversion, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_dedicated_allocation, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_queue_family_foreign, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_SCREEN_QNX
            {vvl::Extension::_VK_MSFT_layered_driver,
             Info(&DeviceExtensions::vk_msft_layered_driver, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_descriptor_pool_overallocation,
             Info(&DeviceExtensions::vk_nv_descriptor_pool_overallocation,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"}}})},
            {vvl::Extension::_VK_NV_raw_access_chains, Info(&DeviceExtensions::vk_nv_raw_access_chains, {})},
            {vvl::Extension::_VK_NV_command_buffer_inheritance, Info(&DeviceExtensions::vk_nv_command_buffer_inheritance, {})},
            {vvl::Extension::_VK_NV_shader_atomic_float16_vector, Info(&DeviceExtensions::vk_nv_shader_atomic_float16_vector, {})},
            {vvl::Extension::_VK_EXT_shader_replicated_composites,
             Info(&DeviceExtensions::vk_ext_shader_replicated_composites, {})},
            {vvl::Extension::_VK_NV_ray_tracing_validation, Info(&DeviceExtensions::vk_nv_ray_tracing_validation, {})},
            {vvl::Extension::_VK_NV_cluster_acceleration_structure,
             Info(&DeviceExtensions::vk_nv_cluster_acceleration_structure,
                  {{{&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_partitioned_acceleration_structure,
             Info(&DeviceExtensions::vk_nv_partitioned_acceleration_structure,
                  {{{&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_device_generated_commands,
             Info(&DeviceExtensions::vk_ext_device_generated_commands,
                  {{{&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_maintenance5, VK_KHR_MAINTENANCE_5_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_MESA_image_alignment_control, Info(&DeviceExtensions::vk_mesa_image_alignment_control,
                                                                    {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                       VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_depth_clamp_control,
             Info(&DeviceExtensions::vk_ext_depth_clamp_control, {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_HUAWEI_hdr_vivid,
             Info(&DeviceExtensions::vk_huawei_hdr_vivid,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_swapchain, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
                    {&DeviceExtensions::vk_ext_hdr_metadata, VK_EXT_HDR_METADATA_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_NV_cooperative_matrix2,
             Info(&DeviceExtensions::vk_nv_cooperative_matrix2,
                  {{{&DeviceExtensions::vk_khr_cooperative_matrix, VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_ARM_pipeline_opacity_micromap,
             Info(&DeviceExtensions::vk_arm_pipeline_opacity_micromap,
                  {{{&DeviceExtensions::vk_ext_opacity_micromap, VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME}}})},
#ifdef VK_USE_PLATFORM_METAL_EXT
            {vvl::Extension::_VK_EXT_external_memory_metal,
             Info(&DeviceExtensions::vk_ext_external_memory_metal,
                  {{{&DeviceExtensions::vk_khr_external_memory, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME}}})},
#endif  // VK_USE_PLATFORM_METAL_EXT
            {vvl::Extension::_VK_EXT_vertex_attribute_robustness,
             Info(&DeviceExtensions::vk_ext_vertex_attribute_robustness,
                  {{{&DeviceExtensions::vk_khr_get_physical_device_properties2,
                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_acceleration_structure,
             Info(&DeviceExtensions::vk_khr_acceleration_structure,
                  {{{&DeviceExtensions::vk_feature_version_1_1, "VK_VERSION_1_1"},
                    {&DeviceExtensions::vk_ext_descriptor_indexing, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_buffer_device_address, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_deferred_host_operations, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_ray_tracing_pipeline,
             Info(&DeviceExtensions::vk_khr_ray_tracing_pipeline,
                  {{{&DeviceExtensions::vk_khr_spirv_1_4, VK_KHR_SPIRV_1_4_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_KHR_ray_query,
             Info(&DeviceExtensions::vk_khr_ray_query,
                  {{{&DeviceExtensions::vk_khr_spirv_1_4, VK_KHR_SPIRV_1_4_EXTENSION_NAME},
                    {&DeviceExtensions::vk_khr_acceleration_structure, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME}}})},
            {vvl::Extension::_VK_EXT_mesh_shader, Info(&DeviceExtensions::vk_ext_mesh_shader,
                                                       {{{&DeviceExtensions::vk_khr_spirv_1_4, VK_KHR_SPIRV_1_4_EXTENSION_NAME}}})},

        };

        return info_map;
    }

    static const Info &GetInfo(vvl::Extension extension_name) {
        static const Info empty_info{nullptr, RequirementVec()};
        const auto &ext_map = DeviceExtensions::GetInfoMap();
        const auto info = ext_map.find(extension_name);
        return (info != ext_map.cend()) ? info->second : empty_info;
    }

    DeviceExtensions() = default;
    DeviceExtensions(const InstanceExtensions &instance_ext) : InstanceExtensions(instance_ext) {}

    DeviceExtensions(const InstanceExtensions &instance_extensions, APIVersion requested_api_version,
                     const VkDeviceCreateInfo *pCreateInfo = nullptr);
    DeviceExtensions(const InstanceExtensions &instance_ext, APIVersion requested_api_version,
                     const std::vector<VkExtensionProperties> &props);
};

const InstanceExtensions::Info &GetInstanceVersionMap(const char *version);
const DeviceExtensions::Info &GetDeviceVersionMap(const char *version);

constexpr bool IsInstanceExtension(vvl::Extension extension) {
    switch (extension) {
        case vvl::Extension::_VK_KHR_surface:
        case vvl::Extension::_VK_KHR_display:
        case vvl::Extension::_VK_KHR_xlib_surface:
        case vvl::Extension::_VK_KHR_xcb_surface:
        case vvl::Extension::_VK_KHR_wayland_surface:
        case vvl::Extension::_VK_KHR_android_surface:
        case vvl::Extension::_VK_KHR_win32_surface:
        case vvl::Extension::_VK_KHR_get_physical_device_properties2:
        case vvl::Extension::_VK_KHR_device_group_creation:
        case vvl::Extension::_VK_KHR_external_memory_capabilities:
        case vvl::Extension::_VK_KHR_external_semaphore_capabilities:
        case vvl::Extension::_VK_KHR_external_fence_capabilities:
        case vvl::Extension::_VK_KHR_get_surface_capabilities2:
        case vvl::Extension::_VK_KHR_get_display_properties2:
        case vvl::Extension::_VK_KHR_surface_protected_capabilities:
        case vvl::Extension::_VK_KHR_portability_enumeration:
        case vvl::Extension::_VK_EXT_debug_report:
        case vvl::Extension::_VK_GGP_stream_descriptor_surface:
        case vvl::Extension::_VK_NV_external_memory_capabilities:
        case vvl::Extension::_VK_EXT_validation_flags:
        case vvl::Extension::_VK_NN_vi_surface:
        case vvl::Extension::_VK_EXT_direct_mode_display:
        case vvl::Extension::_VK_EXT_acquire_xlib_display:
        case vvl::Extension::_VK_EXT_display_surface_counter:
        case vvl::Extension::_VK_EXT_swapchain_colorspace:
        case vvl::Extension::_VK_MVK_ios_surface:
        case vvl::Extension::_VK_MVK_macos_surface:
        case vvl::Extension::_VK_EXT_debug_utils:
        case vvl::Extension::_VK_FUCHSIA_imagepipe_surface:
        case vvl::Extension::_VK_EXT_metal_surface:
        case vvl::Extension::_VK_EXT_validation_features:
        case vvl::Extension::_VK_EXT_headless_surface:
        case vvl::Extension::_VK_EXT_surface_maintenance1:
        case vvl::Extension::_VK_EXT_acquire_drm_display:
        case vvl::Extension::_VK_EXT_directfb_surface:
        case vvl::Extension::_VK_QNX_screen_surface:
        case vvl::Extension::_VK_GOOGLE_surfaceless_query:
        case vvl::Extension::_VK_LUNARG_direct_driver_loading:
        case vvl::Extension::_VK_EXT_layer_settings:
        case vvl::Extension::_VK_NV_display_stereo:
            return true;
        default:
            return false;
    }
}

constexpr bool IsDeviceExtension(vvl::Extension extension) {
    switch (extension) {
        case vvl::Extension::_VK_KHR_swapchain:
        case vvl::Extension::_VK_KHR_display_swapchain:
        case vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge:
        case vvl::Extension::_VK_KHR_video_queue:
        case vvl::Extension::_VK_KHR_video_decode_queue:
        case vvl::Extension::_VK_KHR_video_encode_h264:
        case vvl::Extension::_VK_KHR_video_encode_h265:
        case vvl::Extension::_VK_KHR_video_decode_h264:
        case vvl::Extension::_VK_KHR_dynamic_rendering:
        case vvl::Extension::_VK_KHR_multiview:
        case vvl::Extension::_VK_KHR_device_group:
        case vvl::Extension::_VK_KHR_shader_draw_parameters:
        case vvl::Extension::_VK_KHR_maintenance1:
        case vvl::Extension::_VK_KHR_external_memory:
        case vvl::Extension::_VK_KHR_external_memory_win32:
        case vvl::Extension::_VK_KHR_external_memory_fd:
        case vvl::Extension::_VK_KHR_win32_keyed_mutex:
        case vvl::Extension::_VK_KHR_external_semaphore:
        case vvl::Extension::_VK_KHR_external_semaphore_win32:
        case vvl::Extension::_VK_KHR_external_semaphore_fd:
        case vvl::Extension::_VK_KHR_push_descriptor:
        case vvl::Extension::_VK_KHR_shader_float16_int8:
        case vvl::Extension::_VK_KHR_16bit_storage:
        case vvl::Extension::_VK_KHR_incremental_present:
        case vvl::Extension::_VK_KHR_descriptor_update_template:
        case vvl::Extension::_VK_KHR_imageless_framebuffer:
        case vvl::Extension::_VK_KHR_create_renderpass2:
        case vvl::Extension::_VK_KHR_shared_presentable_image:
        case vvl::Extension::_VK_KHR_external_fence:
        case vvl::Extension::_VK_KHR_external_fence_win32:
        case vvl::Extension::_VK_KHR_external_fence_fd:
        case vvl::Extension::_VK_KHR_performance_query:
        case vvl::Extension::_VK_KHR_maintenance2:
        case vvl::Extension::_VK_KHR_variable_pointers:
        case vvl::Extension::_VK_KHR_dedicated_allocation:
        case vvl::Extension::_VK_KHR_storage_buffer_storage_class:
        case vvl::Extension::_VK_KHR_relaxed_block_layout:
        case vvl::Extension::_VK_KHR_get_memory_requirements2:
        case vvl::Extension::_VK_KHR_image_format_list:
        case vvl::Extension::_VK_KHR_sampler_ycbcr_conversion:
        case vvl::Extension::_VK_KHR_bind_memory2:
        case vvl::Extension::_VK_KHR_portability_subset:
        case vvl::Extension::_VK_KHR_maintenance3:
        case vvl::Extension::_VK_KHR_draw_indirect_count:
        case vvl::Extension::_VK_KHR_shader_subgroup_extended_types:
        case vvl::Extension::_VK_KHR_8bit_storage:
        case vvl::Extension::_VK_KHR_shader_atomic_int64:
        case vvl::Extension::_VK_KHR_shader_clock:
        case vvl::Extension::_VK_KHR_video_decode_h265:
        case vvl::Extension::_VK_KHR_global_priority:
        case vvl::Extension::_VK_KHR_driver_properties:
        case vvl::Extension::_VK_KHR_shader_float_controls:
        case vvl::Extension::_VK_KHR_depth_stencil_resolve:
        case vvl::Extension::_VK_KHR_swapchain_mutable_format:
        case vvl::Extension::_VK_KHR_timeline_semaphore:
        case vvl::Extension::_VK_KHR_vulkan_memory_model:
        case vvl::Extension::_VK_KHR_shader_terminate_invocation:
        case vvl::Extension::_VK_KHR_fragment_shading_rate:
        case vvl::Extension::_VK_KHR_dynamic_rendering_local_read:
        case vvl::Extension::_VK_KHR_shader_quad_control:
        case vvl::Extension::_VK_KHR_spirv_1_4:
        case vvl::Extension::_VK_KHR_separate_depth_stencil_layouts:
        case vvl::Extension::_VK_KHR_present_wait:
        case vvl::Extension::_VK_KHR_uniform_buffer_standard_layout:
        case vvl::Extension::_VK_KHR_buffer_device_address:
        case vvl::Extension::_VK_KHR_deferred_host_operations:
        case vvl::Extension::_VK_KHR_pipeline_executable_properties:
        case vvl::Extension::_VK_KHR_map_memory2:
        case vvl::Extension::_VK_KHR_shader_integer_dot_product:
        case vvl::Extension::_VK_KHR_pipeline_library:
        case vvl::Extension::_VK_KHR_shader_non_semantic_info:
        case vvl::Extension::_VK_KHR_present_id:
        case vvl::Extension::_VK_KHR_video_encode_queue:
        case vvl::Extension::_VK_KHR_synchronization2:
        case vvl::Extension::_VK_KHR_fragment_shader_barycentric:
        case vvl::Extension::_VK_KHR_shader_subgroup_uniform_control_flow:
        case vvl::Extension::_VK_KHR_zero_initialize_workgroup_memory:
        case vvl::Extension::_VK_KHR_workgroup_memory_explicit_layout:
        case vvl::Extension::_VK_KHR_copy_commands2:
        case vvl::Extension::_VK_KHR_format_feature_flags2:
        case vvl::Extension::_VK_KHR_ray_tracing_maintenance1:
        case vvl::Extension::_VK_KHR_maintenance4:
        case vvl::Extension::_VK_KHR_shader_subgroup_rotate:
        case vvl::Extension::_VK_KHR_shader_maximal_reconvergence:
        case vvl::Extension::_VK_KHR_maintenance5:
        case vvl::Extension::_VK_KHR_ray_tracing_position_fetch:
        case vvl::Extension::_VK_KHR_pipeline_binary:
        case vvl::Extension::_VK_KHR_cooperative_matrix:
        case vvl::Extension::_VK_KHR_compute_shader_derivatives:
        case vvl::Extension::_VK_KHR_video_decode_av1:
        case vvl::Extension::_VK_KHR_video_encode_av1:
        case vvl::Extension::_VK_KHR_video_maintenance1:
        case vvl::Extension::_VK_KHR_vertex_attribute_divisor:
        case vvl::Extension::_VK_KHR_load_store_op_none:
        case vvl::Extension::_VK_KHR_shader_float_controls2:
        case vvl::Extension::_VK_KHR_index_type_uint8:
        case vvl::Extension::_VK_KHR_line_rasterization:
        case vvl::Extension::_VK_KHR_calibrated_timestamps:
        case vvl::Extension::_VK_KHR_shader_expect_assume:
        case vvl::Extension::_VK_KHR_maintenance6:
        case vvl::Extension::_VK_KHR_video_encode_quantization_map:
        case vvl::Extension::_VK_KHR_shader_relaxed_extended_instruction:
        case vvl::Extension::_VK_KHR_maintenance7:
        case vvl::Extension::_VK_KHR_maintenance8:
        case vvl::Extension::_VK_KHR_video_maintenance2:
        case vvl::Extension::_VK_KHR_depth_clamp_zero_one:
        case vvl::Extension::_VK_NV_glsl_shader:
        case vvl::Extension::_VK_EXT_depth_range_unrestricted:
        case vvl::Extension::_VK_IMG_filter_cubic:
        case vvl::Extension::_VK_AMD_rasterization_order:
        case vvl::Extension::_VK_AMD_shader_trinary_minmax:
        case vvl::Extension::_VK_AMD_shader_explicit_vertex_parameter:
        case vvl::Extension::_VK_EXT_debug_marker:
        case vvl::Extension::_VK_AMD_gcn_shader:
        case vvl::Extension::_VK_NV_dedicated_allocation:
        case vvl::Extension::_VK_EXT_transform_feedback:
        case vvl::Extension::_VK_NVX_binary_import:
        case vvl::Extension::_VK_NVX_image_view_handle:
        case vvl::Extension::_VK_AMD_draw_indirect_count:
        case vvl::Extension::_VK_AMD_negative_viewport_height:
        case vvl::Extension::_VK_AMD_gpu_shader_half_float:
        case vvl::Extension::_VK_AMD_shader_ballot:
        case vvl::Extension::_VK_AMD_texture_gather_bias_lod:
        case vvl::Extension::_VK_AMD_shader_info:
        case vvl::Extension::_VK_AMD_shader_image_load_store_lod:
        case vvl::Extension::_VK_NV_corner_sampled_image:
        case vvl::Extension::_VK_IMG_format_pvrtc:
        case vvl::Extension::_VK_NV_external_memory:
        case vvl::Extension::_VK_NV_external_memory_win32:
        case vvl::Extension::_VK_NV_win32_keyed_mutex:
        case vvl::Extension::_VK_EXT_shader_subgroup_ballot:
        case vvl::Extension::_VK_EXT_shader_subgroup_vote:
        case vvl::Extension::_VK_EXT_texture_compression_astc_hdr:
        case vvl::Extension::_VK_EXT_astc_decode_mode:
        case vvl::Extension::_VK_EXT_pipeline_robustness:
        case vvl::Extension::_VK_EXT_conditional_rendering:
        case vvl::Extension::_VK_NV_clip_space_w_scaling:
        case vvl::Extension::_VK_EXT_display_control:
        case vvl::Extension::_VK_GOOGLE_display_timing:
        case vvl::Extension::_VK_NV_sample_mask_override_coverage:
        case vvl::Extension::_VK_NV_geometry_shader_passthrough:
        case vvl::Extension::_VK_NV_viewport_array2:
        case vvl::Extension::_VK_NVX_multiview_per_view_attributes:
        case vvl::Extension::_VK_NV_viewport_swizzle:
        case vvl::Extension::_VK_EXT_discard_rectangles:
        case vvl::Extension::_VK_EXT_conservative_rasterization:
        case vvl::Extension::_VK_EXT_depth_clip_enable:
        case vvl::Extension::_VK_EXT_hdr_metadata:
        case vvl::Extension::_VK_IMG_relaxed_line_rasterization:
        case vvl::Extension::_VK_EXT_external_memory_dma_buf:
        case vvl::Extension::_VK_EXT_queue_family_foreign:
        case vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer:
        case vvl::Extension::_VK_EXT_sampler_filter_minmax:
        case vvl::Extension::_VK_AMD_gpu_shader_int16:
        case vvl::Extension::_VK_AMDX_shader_enqueue:
        case vvl::Extension::_VK_AMD_mixed_attachment_samples:
        case vvl::Extension::_VK_AMD_shader_fragment_mask:
        case vvl::Extension::_VK_EXT_inline_uniform_block:
        case vvl::Extension::_VK_EXT_shader_stencil_export:
        case vvl::Extension::_VK_EXT_sample_locations:
        case vvl::Extension::_VK_EXT_blend_operation_advanced:
        case vvl::Extension::_VK_NV_fragment_coverage_to_color:
        case vvl::Extension::_VK_NV_framebuffer_mixed_samples:
        case vvl::Extension::_VK_NV_fill_rectangle:
        case vvl::Extension::_VK_NV_shader_sm_builtins:
        case vvl::Extension::_VK_EXT_post_depth_coverage:
        case vvl::Extension::_VK_EXT_image_drm_format_modifier:
        case vvl::Extension::_VK_EXT_validation_cache:
        case vvl::Extension::_VK_EXT_descriptor_indexing:
        case vvl::Extension::_VK_EXT_shader_viewport_index_layer:
        case vvl::Extension::_VK_NV_shading_rate_image:
        case vvl::Extension::_VK_NV_ray_tracing:
        case vvl::Extension::_VK_NV_representative_fragment_test:
        case vvl::Extension::_VK_EXT_filter_cubic:
        case vvl::Extension::_VK_QCOM_render_pass_shader_resolve:
        case vvl::Extension::_VK_EXT_global_priority:
        case vvl::Extension::_VK_EXT_external_memory_host:
        case vvl::Extension::_VK_AMD_buffer_marker:
        case vvl::Extension::_VK_AMD_pipeline_compiler_control:
        case vvl::Extension::_VK_EXT_calibrated_timestamps:
        case vvl::Extension::_VK_AMD_shader_core_properties:
        case vvl::Extension::_VK_AMD_memory_overallocation_behavior:
        case vvl::Extension::_VK_EXT_vertex_attribute_divisor:
        case vvl::Extension::_VK_GGP_frame_token:
        case vvl::Extension::_VK_EXT_pipeline_creation_feedback:
        case vvl::Extension::_VK_NV_shader_subgroup_partitioned:
        case vvl::Extension::_VK_NV_compute_shader_derivatives:
        case vvl::Extension::_VK_NV_mesh_shader:
        case vvl::Extension::_VK_NV_fragment_shader_barycentric:
        case vvl::Extension::_VK_NV_shader_image_footprint:
        case vvl::Extension::_VK_NV_scissor_exclusive:
        case vvl::Extension::_VK_NV_device_diagnostic_checkpoints:
        case vvl::Extension::_VK_INTEL_shader_integer_functions2:
        case vvl::Extension::_VK_INTEL_performance_query:
        case vvl::Extension::_VK_EXT_pci_bus_info:
        case vvl::Extension::_VK_AMD_display_native_hdr:
        case vvl::Extension::_VK_EXT_fragment_density_map:
        case vvl::Extension::_VK_EXT_scalar_block_layout:
        case vvl::Extension::_VK_GOOGLE_hlsl_functionality1:
        case vvl::Extension::_VK_GOOGLE_decorate_string:
        case vvl::Extension::_VK_EXT_subgroup_size_control:
        case vvl::Extension::_VK_AMD_shader_core_properties2:
        case vvl::Extension::_VK_AMD_device_coherent_memory:
        case vvl::Extension::_VK_EXT_shader_image_atomic_int64:
        case vvl::Extension::_VK_EXT_memory_budget:
        case vvl::Extension::_VK_EXT_memory_priority:
        case vvl::Extension::_VK_NV_dedicated_allocation_image_aliasing:
        case vvl::Extension::_VK_EXT_buffer_device_address:
        case vvl::Extension::_VK_EXT_tooling_info:
        case vvl::Extension::_VK_EXT_separate_stencil_usage:
        case vvl::Extension::_VK_NV_cooperative_matrix:
        case vvl::Extension::_VK_NV_coverage_reduction_mode:
        case vvl::Extension::_VK_EXT_fragment_shader_interlock:
        case vvl::Extension::_VK_EXT_ycbcr_image_arrays:
        case vvl::Extension::_VK_EXT_provoking_vertex:
        case vvl::Extension::_VK_EXT_full_screen_exclusive:
        case vvl::Extension::_VK_EXT_line_rasterization:
        case vvl::Extension::_VK_EXT_shader_atomic_float:
        case vvl::Extension::_VK_EXT_host_query_reset:
        case vvl::Extension::_VK_EXT_index_type_uint8:
        case vvl::Extension::_VK_EXT_extended_dynamic_state:
        case vvl::Extension::_VK_EXT_host_image_copy:
        case vvl::Extension::_VK_EXT_map_memory_placed:
        case vvl::Extension::_VK_EXT_shader_atomic_float2:
        case vvl::Extension::_VK_EXT_swapchain_maintenance1:
        case vvl::Extension::_VK_EXT_shader_demote_to_helper_invocation:
        case vvl::Extension::_VK_NV_device_generated_commands:
        case vvl::Extension::_VK_NV_inherited_viewport_scissor:
        case vvl::Extension::_VK_EXT_texel_buffer_alignment:
        case vvl::Extension::_VK_QCOM_render_pass_transform:
        case vvl::Extension::_VK_EXT_depth_bias_control:
        case vvl::Extension::_VK_EXT_device_memory_report:
        case vvl::Extension::_VK_EXT_robustness2:
        case vvl::Extension::_VK_EXT_custom_border_color:
        case vvl::Extension::_VK_GOOGLE_user_type:
        case vvl::Extension::_VK_NV_present_barrier:
        case vvl::Extension::_VK_EXT_private_data:
        case vvl::Extension::_VK_EXT_pipeline_creation_cache_control:
        case vvl::Extension::_VK_NV_device_diagnostics_config:
        case vvl::Extension::_VK_QCOM_render_pass_store_ops:
        case vvl::Extension::_VK_NV_cuda_kernel_launch:
        case vvl::Extension::_VK_NV_low_latency:
        case vvl::Extension::_VK_EXT_metal_objects:
        case vvl::Extension::_VK_EXT_descriptor_buffer:
        case vvl::Extension::_VK_EXT_graphics_pipeline_library:
        case vvl::Extension::_VK_AMD_shader_early_and_late_fragment_tests:
        case vvl::Extension::_VK_NV_fragment_shading_rate_enums:
        case vvl::Extension::_VK_NV_ray_tracing_motion_blur:
        case vvl::Extension::_VK_EXT_ycbcr_2plane_444_formats:
        case vvl::Extension::_VK_EXT_fragment_density_map2:
        case vvl::Extension::_VK_QCOM_rotated_copy_commands:
        case vvl::Extension::_VK_EXT_image_robustness:
        case vvl::Extension::_VK_EXT_image_compression_control:
        case vvl::Extension::_VK_EXT_attachment_feedback_loop_layout:
        case vvl::Extension::_VK_EXT_4444_formats:
        case vvl::Extension::_VK_EXT_device_fault:
        case vvl::Extension::_VK_ARM_rasterization_order_attachment_access:
        case vvl::Extension::_VK_EXT_rgba10x6_formats:
        case vvl::Extension::_VK_NV_acquire_winrt_display:
        case vvl::Extension::_VK_VALVE_mutable_descriptor_type:
        case vvl::Extension::_VK_EXT_vertex_input_dynamic_state:
        case vvl::Extension::_VK_EXT_physical_device_drm:
        case vvl::Extension::_VK_EXT_device_address_binding_report:
        case vvl::Extension::_VK_EXT_depth_clip_control:
        case vvl::Extension::_VK_EXT_primitive_topology_list_restart:
        case vvl::Extension::_VK_EXT_present_mode_fifo_latest_ready:
        case vvl::Extension::_VK_FUCHSIA_external_memory:
        case vvl::Extension::_VK_FUCHSIA_external_semaphore:
        case vvl::Extension::_VK_FUCHSIA_buffer_collection:
        case vvl::Extension::_VK_HUAWEI_subpass_shading:
        case vvl::Extension::_VK_HUAWEI_invocation_mask:
        case vvl::Extension::_VK_NV_external_memory_rdma:
        case vvl::Extension::_VK_EXT_pipeline_properties:
        case vvl::Extension::_VK_EXT_frame_boundary:
        case vvl::Extension::_VK_EXT_multisampled_render_to_single_sampled:
        case vvl::Extension::_VK_EXT_extended_dynamic_state2:
        case vvl::Extension::_VK_EXT_color_write_enable:
        case vvl::Extension::_VK_EXT_primitives_generated_query:
        case vvl::Extension::_VK_EXT_global_priority_query:
        case vvl::Extension::_VK_EXT_image_view_min_lod:
        case vvl::Extension::_VK_EXT_multi_draw:
        case vvl::Extension::_VK_EXT_image_2d_view_of_3d:
        case vvl::Extension::_VK_EXT_shader_tile_image:
        case vvl::Extension::_VK_EXT_opacity_micromap:
        case vvl::Extension::_VK_NV_displacement_micromap:
        case vvl::Extension::_VK_EXT_load_store_op_none:
        case vvl::Extension::_VK_HUAWEI_cluster_culling_shader:
        case vvl::Extension::_VK_EXT_border_color_swizzle:
        case vvl::Extension::_VK_EXT_pageable_device_local_memory:
        case vvl::Extension::_VK_ARM_shader_core_properties:
        case vvl::Extension::_VK_ARM_scheduling_controls:
        case vvl::Extension::_VK_EXT_image_sliced_view_of_3d:
        case vvl::Extension::_VK_VALVE_descriptor_set_host_mapping:
        case vvl::Extension::_VK_EXT_depth_clamp_zero_one:
        case vvl::Extension::_VK_EXT_non_seamless_cube_map:
        case vvl::Extension::_VK_ARM_render_pass_striped:
        case vvl::Extension::_VK_QCOM_fragment_density_map_offset:
        case vvl::Extension::_VK_NV_copy_memory_indirect:
        case vvl::Extension::_VK_NV_memory_decompression:
        case vvl::Extension::_VK_NV_device_generated_commands_compute:
        case vvl::Extension::_VK_NV_ray_tracing_linear_swept_spheres:
        case vvl::Extension::_VK_NV_linear_color_attachment:
        case vvl::Extension::_VK_EXT_image_compression_control_swapchain:
        case vvl::Extension::_VK_QCOM_image_processing:
        case vvl::Extension::_VK_EXT_nested_command_buffer:
        case vvl::Extension::_VK_EXT_external_memory_acquire_unmodified:
        case vvl::Extension::_VK_EXT_extended_dynamic_state3:
        case vvl::Extension::_VK_EXT_subpass_merge_feedback:
        case vvl::Extension::_VK_EXT_shader_module_identifier:
        case vvl::Extension::_VK_EXT_rasterization_order_attachment_access:
        case vvl::Extension::_VK_NV_optical_flow:
        case vvl::Extension::_VK_EXT_legacy_dithering:
        case vvl::Extension::_VK_EXT_pipeline_protected_access:
        case vvl::Extension::_VK_ANDROID_external_format_resolve:
        case vvl::Extension::_VK_AMD_anti_lag:
        case vvl::Extension::_VK_EXT_shader_object:
        case vvl::Extension::_VK_QCOM_tile_properties:
        case vvl::Extension::_VK_SEC_amigo_profiling:
        case vvl::Extension::_VK_QCOM_multiview_per_view_viewports:
        case vvl::Extension::_VK_NV_ray_tracing_invocation_reorder:
        case vvl::Extension::_VK_NV_cooperative_vector:
        case vvl::Extension::_VK_NV_extended_sparse_address_space:
        case vvl::Extension::_VK_EXT_mutable_descriptor_type:
        case vvl::Extension::_VK_EXT_legacy_vertex_attributes:
        case vvl::Extension::_VK_ARM_shader_core_builtins:
        case vvl::Extension::_VK_EXT_pipeline_library_group_handles:
        case vvl::Extension::_VK_EXT_dynamic_rendering_unused_attachments:
        case vvl::Extension::_VK_NV_low_latency2:
        case vvl::Extension::_VK_QCOM_multiview_per_view_render_areas:
        case vvl::Extension::_VK_NV_per_stage_descriptor_set:
        case vvl::Extension::_VK_QCOM_image_processing2:
        case vvl::Extension::_VK_QCOM_filter_cubic_weights:
        case vvl::Extension::_VK_QCOM_ycbcr_degamma:
        case vvl::Extension::_VK_QCOM_filter_cubic_clamp:
        case vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state:
        case vvl::Extension::_VK_QNX_external_memory_screen_buffer:
        case vvl::Extension::_VK_MSFT_layered_driver:
        case vvl::Extension::_VK_NV_descriptor_pool_overallocation:
        case vvl::Extension::_VK_NV_raw_access_chains:
        case vvl::Extension::_VK_NV_command_buffer_inheritance:
        case vvl::Extension::_VK_NV_shader_atomic_float16_vector:
        case vvl::Extension::_VK_EXT_shader_replicated_composites:
        case vvl::Extension::_VK_NV_ray_tracing_validation:
        case vvl::Extension::_VK_NV_cluster_acceleration_structure:
        case vvl::Extension::_VK_NV_partitioned_acceleration_structure:
        case vvl::Extension::_VK_EXT_device_generated_commands:
        case vvl::Extension::_VK_MESA_image_alignment_control:
        case vvl::Extension::_VK_EXT_depth_clamp_control:
        case vvl::Extension::_VK_HUAWEI_hdr_vivid:
        case vvl::Extension::_VK_NV_cooperative_matrix2:
        case vvl::Extension::_VK_ARM_pipeline_opacity_micromap:
        case vvl::Extension::_VK_EXT_external_memory_metal:
        case vvl::Extension::_VK_EXT_vertex_attribute_robustness:
        case vvl::Extension::_VK_KHR_acceleration_structure:
        case vvl::Extension::_VK_KHR_ray_tracing_pipeline:
        case vvl::Extension::_VK_KHR_ray_query:
        case vvl::Extension::_VK_EXT_mesh_shader:
            return true;
        default:
            return false;
    }
}

// NOLINTEND
