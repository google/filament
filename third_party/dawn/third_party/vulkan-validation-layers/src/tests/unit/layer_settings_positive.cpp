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
 */
// stype-check off

#include <array>
#include "../framework/layer_validation_tests.h"

class PositiveLayerSettings : public VkLayerTest {};

// When adding a new setting, add here to make sure it is tested
// (internal debug settings and deprecated are excluded from here)
TEST_F(PositiveLayerSettings, AllSettings) {
    const char* some_string = "placeholder";
    const char* action_ignore = "VK_DBG_LAYER_ACTION_IGNORE";
    const char* warning = "warn";
    const VkBool32 disable = VK_FALSE;
    const uint32_t one = 1;
    const uint32_t one_k = 1024;
    std::vector<VkLayerSettingEXT> settings = {{
        {OBJECT_LAYER_NAME, "enables", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &some_string},
        {OBJECT_LAYER_NAME, "validate_best_practices", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "validate_best_practices_arm", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "validate_best_practices_amd", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "validate_best_practices_img", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "validate_best_practices_nvidia", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "disables", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &some_string},
        {OBJECT_LAYER_NAME, "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "thread_safety", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "stateless_param", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "object_lifetime", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "unique_handles", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "check_shaders_caching", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "check_command_buffer", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "check_object_in_use", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "check_query", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "check_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &some_string},
        {OBJECT_LAYER_NAME, "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &one},
        {OBJECT_LAYER_NAME, "fine_grained_locking", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "printf_only_preset", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "printf_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "printf_to_stdout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "printf_verbose", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "printf_buffer_size", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &one_k},
        {OBJECT_LAYER_NAME, "gpuav_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_shader_instrumentation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_descriptor_checks", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_warn_on_robust_oob", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_buffer_address_oob", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_max_buffer_device_addresses", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &one_k},
        {OBJECT_LAYER_NAME, "gpuav_validate_ray_query", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_post_process_descriptor_indexing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_buffers_validation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_indirect_draws_buffers", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_indirect_dispatches_buffers", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_indirect_trace_rays_buffers", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_buffer_copies", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_index_buffers", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_reserve_binding_slot", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "gpuav_vma_linear_output", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "syncval_submit_time_validation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "syncval_shader_accesses_heuristic", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "syncval_message_extra_properties", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "syncval_message_extra_properties_pretty_print", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "message_format_display_application_name", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable},
        {OBJECT_LAYER_NAME, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &action_ignore},
        {OBJECT_LAYER_NAME, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &warning},
    }};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                (uint32_t)settings.size(), settings.data()};
    Monitor().ExpectSuccess(kErrorBit | kWarningBit);
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(PositiveLayerSettings, ReportFlags) {
    const char* report_flags[3] = {"error", "warn", "info"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, 3, report_flags};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().ExpectSuccess(kErrorBit | kWarningBit);
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(PositiveLayerSettings, DebugAction) {
    const char* actions[2] = {"VK_DBG_LAYER_ACTION_CALLBACK", "VK_DBG_LAYER_ACTION_DEFAULT"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 2, actions};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().ExpectSuccess(kErrorBit | kWarningBit);
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}
