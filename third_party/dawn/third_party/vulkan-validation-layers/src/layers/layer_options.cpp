/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "layer_options.h"
#include <vulkan/vulkan_core.h>
#include "chassis/validation_object.h"
#include "error_message/log_message_type.h"
#include "error_message/logging.h"
#include "error_message/error_location.h"
#include "generated/error_location_helper.h"
#include "utils/hash_util.h"
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>
#include <vulkan/layer/vk_layer_settings.hpp>

#include "gpuav/core/gpuav_settings.h"

#include "sync/sync_settings.h"
#include "vk_layer_config.h"

// Include new / delete overrides if using mimalloc. This needs to be include exactly once in a file that is
// part of the VVL but not the layer utils library.
#if defined(USE_MIMALLOC) && defined(_WIN64)
#include "mimalloc-new-delete.h"
#endif

const auto &VkValFeatureDisableLookup() {
    static const vvl::unordered_map<std::string, VkValidationFeatureDisableEXT> vk_val_feature_disable_lookup = {
        {"VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT", VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT},
        {"VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT", VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT},
        {"VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT", VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT},
        {"VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT", VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT},
        {"VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT", VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT},
        {"VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT", VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT},
        {"VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT", VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT},
        {"VK_VALIDATION_FEATURE_DISABLE_ALL_EXT", VK_VALIDATION_FEATURE_DISABLE_ALL_EXT},
    };
    return vk_val_feature_disable_lookup;
}

const auto &VkValFeatureEnableLookup() {
    static const vvl::unordered_map<std::string, VkValidationFeatureEnableEXT> vk_val_feature_enable_lookup = {
        {"VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT", VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT},
        {"VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT",
         VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT},
        {"VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT", VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT},
        {"VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT", VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT},
        {"VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT",
         VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT},
    };
    return vk_val_feature_enable_lookup;
}

const auto &ValidationDisableLookup() {
    static const vvl::unordered_map<std::string, ValidationCheckDisables> validation_disable_lookup = {
        {"VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE", VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE},
        {"VALIDATION_CHECK_DISABLE_OBJECT_IN_USE", VALIDATION_CHECK_DISABLE_OBJECT_IN_USE},
        {"VALIDATION_CHECK_DISABLE_QUERY_VALIDATION", VALIDATION_CHECK_DISABLE_QUERY_VALIDATION},
        {"VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION", VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION},
    };
    return validation_disable_lookup;
}

const auto &ValidationEnableLookup() {
    static const vvl::unordered_map<std::string, ValidationCheckEnables> validation_enable_lookup = {
        {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM},
        {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD},
        {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG},
        {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA},
        {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL},
    };
    return validation_enable_lookup;
}

// This should mirror the 'DisableFlags' enumerated type
const std::vector<std::string> &GetDisableFlagNameHelper() {
    static const std::vector<std::string> disable_flag_name_helper = {
        "VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE",                // command_buffer_state,
        "VALIDATION_CHECK_DISABLE_OBJECT_IN_USE",                       // object_in_use,
        "VALIDATION_CHECK_DISABLE_QUERY_VALIDATION",                    // query_validation,
        "VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION",             // image_layout_validation,
        "VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT",           // object_tracking,
        "VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT",                // core_checks,
        "VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT",              // thread_safety,
        "VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT",             // stateless_checks,
        "VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT",             // handle_wrapping,
        "VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT",                    // shader_validation,
        "VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHING_EXT",  // shader_validation_caching
    };
    return disable_flag_name_helper;
}

const std::vector<std::string> &GetEnableFlagNameHelper() {
    // This should mirror the 'EnableFlags' enumerated type
    static const std::vector<std::string> enable_flag_name_helper = {
        "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT",                       // gpu_validation,
        "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT",  // gpu_validation_reserve_binding_slot,
        "VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT",                     // best_practices,
        "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM",                         // vendor_specific_arm,
        "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD",                         // vendor_specific_amd,
        "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG",                         // vendor_specific_img,
        "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA",                      // vendor_specific_nvidia,
        "VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT",                       // debug_printf,
        "VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT",         // sync_validation,
    };
    return enable_flag_name_helper;
}

// To enable "my_setting",
// Set env var VK_LAYER_MY_SETTING to 1
//
// The ["VK_LAYER_" + toUpper(key)] logic is don in vk_layer_setting (VUL)
//
// To quickly be able to find the env var corresponding to a setting,
// the following `const char*` holding setting names match their corresponding environment variable

// Corresponding to VkValidationFeatureEnableEXT
// ---
const char *VK_LAYER_ENABLES = "enables";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES = "validate_best_practices";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES_ARM = "validate_best_practices_arm";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES_AMD = "validate_best_practices_amd";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES_IMG = "validate_best_practices_img";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES_NVIDIA = "validate_best_practices_nvidia";
const char *VK_LAYER_VALIDATE_SYNC = "validate_sync";
// These were deprecated after the 1.3.296 SDK release (because it was a flag and now is a boolean)
const char *DEPRECATED_VK_LAYER_VALIDATE_GPU_BASED = "validate_gpu_based";

// Corresponding to VkValidationFeatureDisableEXT
// ---
const char *VK_LAYER_DISABLES = "disables";
const char *VK_LAYER_CHECK_SHADERS = "check_shaders";
const char *VK_LAYER_THREAD_SAFETY = "thread_safety";
const char *VK_LAYER_STATELESS_PARAM = "stateless_param";
const char *VK_LAYER_OBJECT_LIFETIME = "object_lifetime";
const char *VK_LAYER_VALIDATE_CORE = "validate_core";
const char *VK_LAYER_UNIQUE_HANDLES = "unique_handles";
const char *VK_LAYER_CHECK_SHADERS_CACHING = "check_shaders_caching";

// Additional checks exposed in vkconfig, but not in VkValidationFeatureDisableEXT
// ---
const char *VK_LAYER_CHECK_COMMAND_BUFFER = "check_command_buffer";
const char *VK_LAYER_CHECK_OBJECT_IN_USE = "check_object_in_use";
const char *VK_LAYER_CHECK_QUERY = "check_query";
const char *VK_LAYER_CHECK_IMAGE_LAYOUT = "check_image_layout";

// Options related to debug reporting
// ---
const char *VK_LAYER_MESSAGE_ID_FILTER = "message_id_filter";
const char *VK_LAYER_CUSTOM_STYPE_LIST = "custom_stype_list";
const char *VK_LAYER_ENABLE_MESSAGE_LIMIT = "enable_message_limit";
const char *VK_LAYER_DUPLICATE_MESSAGE_LIMIT = "duplicate_message_limit";

// GloablSettings
// ---
const char *VK_LAYER_FINE_GRAINED_LOCKING = "fine_grained_locking";
// Debug settings used for internal development
const char *VK_LAYER_DEBUG_DISABLE_SPIRV_VAL = "debug_disable_spirv_val";
const char *VK_LAYER_DEBUG_STABLE_MESSAGES = "debug_stable_messages";

// DebugPrintf (which is now part of GPU-AV internally)
// ---
// Quick, single setting to turn on DebugPrintf
const char *VK_LAYER_PRINTF_ONLY_PRESET = "printf_only_preset";
// Was added a new way to set things without having to use VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
const char *VK_LAYER_PRINTF_ENABLE = "printf_enable";
const char *VK_LAYER_PRINTF_TO_STDOUT = "printf_to_stdout";
const char *VK_LAYER_PRINTF_VERBOSE = "printf_verbose";
const char *VK_LAYER_PRINTF_BUFFER_SIZE = "printf_buffer_size";

// GPU-AV
// ---
const char *VK_LAYER_GPUAV_ENABLE = "gpuav_enable";
const char *VK_LAYER_GPUAV_SHADER_INSTRUMENTATION = "gpuav_shader_instrumentation";
const char *VK_LAYER_GPUAV_DESCRIPTOR_CHECKS = "gpuav_descriptor_checks";
const char *VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB = "gpuav_warn_on_robust_oob";
const char *VK_LAYER_GPUAV_BUFFER_ADDRESS_OOB = "gpuav_buffer_address_oob";
const char *VK_LAYER_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES = "gpuav_max_buffer_device_addresses";
const char *VK_LAYER_GPUAV_VALIDATE_RAY_QUERY = "gpuav_validate_ray_query";
// Post Process are designed to allow the user to "assume" the access is valid and want to know after the GPU executes what
// happened. These are much lighter checks and can be used while the rest of GPU-AV is turned off
const char *VK_LAYER_GPUAV_POST_PROCESS_DESCRIPTOR_INDEXING = "gpuav_post_process_descriptor_indexing";
const char *VK_LAYER_GPUAV_VERTEX_ATTRIBUTE_FETCH_OOB = "gpuav_vertex_attribute_fetch_oob";
const char *VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS = "gpuav_select_instrumented_shaders";

const char *VK_LAYER_GPUAV_BUFFERS_VALIDATION = "gpuav_buffers_validation";
const char *VK_LAYER_GPUAV_INDIRECT_DRAWS_BUFFERS = "gpuav_indirect_draws_buffers";
const char *VK_LAYER_GPUAV_INDIRECT_DISPATCHES_BUFFERS = "gpuav_indirect_dispatches_buffers";
const char *VK_LAYER_GPUAV_INDIRECT_TRACE_RAYS_BUFFERS = "gpuav_indirect_trace_rays_buffers";
const char *VK_LAYER_GPUAV_BUFFER_COPIES = "gpuav_buffer_copies";
const char *VK_LAYER_GPUAV_INDEX_BUFFERS = "gpuav_index_buffers";
const char *VK_LAYER_GPUAV_IMAGE_LAYOUT = "gpuav_image_layout";

const char *VK_LAYER_GPUAV_RESERVE_BINDING_SLOT = "gpuav_reserve_binding_slot";
const char *VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT = "gpuav_vma_linear_output";

const char *VK_LAYER_GPUAV_DEBUG_DISABLE_ALL = "gpuav_debug_disable_all";
const char *VK_LAYER_GPUAV_DEBUG_VALIDATE_INSTRUMENTED_SHADERS = "gpuav_debug_validate_instrumented_shaders";
const char *VK_LAYER_GPUAV_DEBUG_DUMP_INSTRUMENTED_SHADERS = "gpuav_debug_dump_instrumented_shaders";
const char *VK_LAYER_GPUAV_DEBUG_MAX_INSTRUMENTATIONS_COUNT = "gpuav_debug_max_instrumentations_count";
const char *VK_LAYER_GPUAV_DEBUG_PRINT_INSTRUMENTATION_INFO = "gpuav_debug_print_instrumentation_info";

// SyncVal
// ---
const char *VK_LAYER_SYNCVAL_SUBMIT_TIME_VALIDATION = "syncval_submit_time_validation";
const char *VK_LAYER_SYNCVAL_SHADER_ACCESSES_HEURISTIC = "syncval_shader_accesses_heuristic";
const char *VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES = "syncval_message_extra_properties";
const char *VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES_PRETTY_PRINT = "syncval_message_extra_properties_pretty_print";

// Message Formatting
// ---
const char *VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME = "message_format_display_application_name";
// Until post 1.3.290 SDK release, these were not possible to set via environment variables
const char *VK_LAYER_LOG_FILENAME = "log_filename";
const char *VK_LAYER_DEBUG_ACTION = "debug_action";
const char *VK_LAYER_REPORT_FLAGS = "report_flags";

// These were deprecated after the 1.3.280 SDK release
const char *DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES = "gpuav_validate_copies";
const char *DEPRECATED_VK_LAYER_GPUAV_VALIDATE_INDIRECT_BUFFER = "gpuav_validate_indirect_buffer";
const char *DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT = "reserve_binding_slot";
const char *DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT = "vma_linear_output";
const char *DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB = "warn_on_robust_oob";
const char *DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS = "select_instrumented_shaders";

// These were deprecated after the 1.3.283 SDK release
const char *DEPRECATED_VK_LAYER_VALIDATE_SYNC_QUEUE_SUBMIT = "sync_queue_submit";

// Don't need any setting helper when using self vvl and don't want unused function warnings
#if !defined(BUILD_SELF_VVL)

// Set the local disable flag for the appropriate VALIDATION_CHECK_DISABLE enum
void SetValidationDisable(CHECK_DISABLED &disable_data, const ValidationCheckDisables disable_id) {
    switch (disable_id) {
        case VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE:
            disable_data[command_buffer_state] = true;
            break;
        case VALIDATION_CHECK_DISABLE_OBJECT_IN_USE:
            disable_data[object_in_use] = true;
            break;
        case VALIDATION_CHECK_DISABLE_QUERY_VALIDATION:
            disable_data[query_validation] = true;
            break;
        case VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION:
            disable_data[image_layout_validation] = true;
            break;
        default:
            assert(false);
    }
}

// Set the local disable flag for a single VK_VALIDATION_FEATURE_DISABLE_* flag
void SetValidationFeatureDisable(CHECK_DISABLED &disable_data, const VkValidationFeatureDisableEXT feature_disable) {
    switch (feature_disable) {
        case VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT:
            disable_data[shader_validation] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT:
            disable_data[thread_safety] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT:
            disable_data[stateless_checks] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT:
            disable_data[object_tracking] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT:
            disable_data[core_checks] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT:
            disable_data[handle_wrapping] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT:
            disable_data[shader_validation_caching] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_ALL_EXT:
            // Set all disabled flags to true
            std::fill(disable_data.begin(), disable_data.end(), true);
            break;
        default:
            break;
    }
}

// Set the local enable flag for the appropriate VALIDATION_CHECK_ENABLE enum
void SetValidationEnable(CHECK_ENABLED &enable_data, const ValidationCheckEnables enable_id) {
    switch (enable_id) {
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM:
            enable_data[vendor_specific_arm] = true;
            break;
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD:
            enable_data[vendor_specific_amd] = true;
            break;
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG:
            enable_data[vendor_specific_img] = true;
            break;
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA:
            enable_data[vendor_specific_nvidia] = true;
            break;
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL:
            enable_data[vendor_specific_arm] = true;
            enable_data[vendor_specific_amd] = true;
            enable_data[vendor_specific_img] = true;
            enable_data[vendor_specific_nvidia] = true;
            break;
        default:
            assert(false);
    }
}

// Set the local enable flag for a single VK_VALIDATION_FEATURE_ENABLE_* flag
void SetValidationFeatureEnable(CHECK_ENABLED &enable_data, const VkValidationFeatureEnableEXT feature_enable) {
    switch (feature_enable) {
        case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT:
            enable_data[gpu_validation] = true;
            break;
        case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT:
            enable_data[gpu_validation_reserve_binding_slot] = true;
            break;
        case VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT:
            enable_data[best_practices] = true;
            break;
        case VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT:
            enable_data[debug_printf_validation] = true;
            break;
        case VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT:
            enable_data[sync_validation] = true;
            break;
        default:
            break;
    }
}

// Set the local disable flag for settings specified through the VK_EXT_validation_flags extension
void SetValidationFlags(CHECK_DISABLED &disables, const VkValidationFlagsEXT *val_flags_struct) {
    for (uint32_t i = 0; i < val_flags_struct->disabledValidationCheckCount; ++i) {
        switch (val_flags_struct->pDisabledValidationChecks[i]) {
            case VK_VALIDATION_CHECK_SHADERS_EXT:
                disables[shader_validation] = true;
                break;
            case VK_VALIDATION_CHECK_ALL_EXT:
                // Set all disabled flags to true
                disables[shader_validation] = true;
                break;
            default:
                break;
        }
    }
}

// Process Validation Features flags specified through the ValidationFeature extension
void SetValidationFeatures(CHECK_DISABLED &disable_data, CHECK_ENABLED &enable_data,
                           const VkValidationFeaturesEXT *val_features_struct) {
    for (uint32_t i = 0; i < val_features_struct->disabledValidationFeatureCount; ++i) {
        SetValidationFeatureDisable(disable_data, val_features_struct->pDisabledValidationFeatures[i]);
    }
    for (uint32_t i = 0; i < val_features_struct->enabledValidationFeatureCount; ++i) {
        SetValidationFeatureEnable(enable_data, val_features_struct->pEnabledValidationFeatures[i]);
    }
}

std::string GetNextToken(std::string *token_list, const std::string &delimiter, size_t *pos) {
    std::string token;
    *pos = token_list->find(delimiter);
    if (*pos != std::string::npos) {
        token = token_list->substr(0, *pos);
    } else {
        *pos = token_list->length() - delimiter.length();
        token = *token_list;
    }
    token_list->erase(0, *pos + delimiter.length());

    // Remove quotes from quoted strings
    if ((token.length() > 0) && (token[0] == '\"')) {
        token.erase(token.begin());
        if ((token.length() > 0) && (token[token.length() - 1] == '\"')) {
            token.erase(--token.end());
        }
    }
    return token;
}

// Given a string representation of a list of enable enum values, call the appropriate setter function
void SetLocalEnableSetting(std::string list_of_enables, const std::string &delimiter, CHECK_ENABLED &enables) {
    size_t pos = 0;
    std::string token;
    while (list_of_enables.length() != 0) {
        token = GetNextToken(&list_of_enables, delimiter, &pos);
        if (token.find("VK_VALIDATION_FEATURE_ENABLE_") != std::string::npos) {
            auto result = VkValFeatureEnableLookup().find(token);
            if (result != VkValFeatureEnableLookup().end()) {
                SetValidationFeatureEnable(enables, result->second);
            }
        } else if (token.find("VALIDATION_CHECK_ENABLE_") != std::string::npos) {
            auto result = ValidationEnableLookup().find(token);
            if (result != ValidationEnableLookup().end()) {
                SetValidationEnable(enables, result->second);
            }
        }
    }
}

// Given a string representation of a list of disable enum values, call the appropriate setter function
void SetLocalDisableSetting(std::string list_of_disables, const std::string &delimiter, CHECK_DISABLED &disables) {
    size_t pos = 0;
    std::string token;
    while (list_of_disables.length() != 0) {
        token = GetNextToken(&list_of_disables, delimiter, &pos);
        if (token.find("VK_VALIDATION_FEATURE_DISABLE_") != std::string::npos) {
            auto result = VkValFeatureDisableLookup().find(token);
            if (result != VkValFeatureDisableLookup().end()) {
                SetValidationFeatureDisable(disables, result->second);
            }
        } else if (token.find("VALIDATION_CHECK_DISABLE_") != std::string::npos) {
            auto result = ValidationDisableLookup().find(token);
            if (result != ValidationDisableLookup().end()) {
                SetValidationDisable(disables, result->second);
            }
        }
    }
}

uint32_t TokenToUint(std::string &token) {
    uint32_t int_id = 0;
    if ((token.find("0x") == 0) || token.find("0X") == 0) {  // Handle hex format
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}

void CreateFilterMessageIdList(std::string raw_id_list, const std::string &delimiter, vvl::unordered_set<uint32_t> &filter_list) {
    size_t pos = 0;
    std::string token;
    while (raw_id_list.length() != 0) {
        token = GetNextToken(&raw_id_list, delimiter, &pos);
        uint32_t int_id = TokenToUint(token);
        if (int_id == 0) {
            const uint32_t id_hash = hash_util::VuidHash(token);
            if (id_hash != 0) {
                int_id = id_hash;
            }
        }
        if ((int_id != 0) && filter_list.find(int_id) == filter_list.end()) {
            filter_list.insert(int_id);
        }
    }
}

// Because VkLayerSettingsCreateInfoEXT/VkLayerSettingEXT are passed in and used before everything else, need to do the stateless
// validation here as a special exception
//
// We want to use the DebugReport object to print VU error messages in here, but we need to parse the settings in
// VkLayerSettingsCreateInfoEXT in order to configure the DebugReport object. Therefor we just settle with printf in here. These are
// very unlikely things to hit validation layers messages (as everything after will likely crumble) so should be ok.
//
// Returns if valid
static bool ValidateLayerSettingsCreateInfo(const VkLayerSettingsCreateInfoEXT *layer_settings) {
    bool valid = true;
    if (!layer_settings) return valid;
    const Location loc(vvl::Func::vkCreateInstance, vvl::Field::pCreateInfo);
    const Location create_info_loc = loc.pNext(vvl::Struct::VkLayerSettingsCreateInfoEXT);
    std::stringstream ss;

    if (layer_settings->pSettings) {
        for (const auto [i, setting] : vvl::enumerate(layer_settings->pSettings, layer_settings->settingCount)) {
            const Location setting_loc = create_info_loc.dot(vvl::Field::pSettings, i);
            if (setting.valueCount > 0 && !setting.pValues) {
                ss << "[ VUID-VkLayerSettingEXT-valueCount-10070 ] " << setting_loc.dot(vvl::Field::pValues).Message()
                   << " is NULL";
                printf("Validation Layer Error: %s\n", ss.str().c_str());
                valid = false;
            }
            if (!setting.pLayerName) {
                ss << "[ VUID-VkLayerSettingEXT-pLayerName-parameter ] " << setting_loc.dot(vvl::Field::pLayerName).Message()
                   << " is NULL";
                printf("Validation Layer Error: %s\n", ss.str().c_str());
                valid = false;
            }
            if (!setting.pSettingName) {
                ss << "[ VUID-VkLayerSettingEXT-pSettingName-parameter ] " << setting_loc.dot(vvl::Field::pSettingName).Message()
                   << " is NULL";
                printf("Validation Layer Error: %s\n", ss.str().c_str());
                valid = false;
            }
        }
    } else if (layer_settings->settingCount > 0) {
        ss << "[ VUID-VkLayerSettingsCreateInfoEXT-pSettings-parameter ] " << create_info_loc.dot(vvl::Field::pSettings).Message()
           << " is NULL";
        printf("Validation Layer Error: %s\n", ss.str().c_str());
        valid = false;
    }
    return valid;
}

// TODO - This should be in https://github.com/KhronosGroup/Vulkan-Utility-Libraries/issues/254
// Doing here in VVL until added in VUL
// If it will not be done in VUL for a while, we should at least generate this from the JSON
static void ValidateLayerSettingsProvided(const VkLayerSettingsCreateInfoEXT *layer_setting_create_info,
                                          VkuLayerSettingSet layer_setting_set, std::vector<std::string> &setting_warnings) {
    if (!layer_setting_create_info) return;
    // Found that a set of <const char*> doesn't detect duplicates on all compilers
    vvl::unordered_set<std::string> used_settings;

    for (uint32_t i = 0; i < layer_setting_create_info->settingCount; i++) {
        const VkLayerSettingEXT &setting = layer_setting_create_info->pSettings[i];
        if (strcmp(OBJECT_LAYER_NAME, setting.pLayerName) != 0) continue;

        // used as a backup for settings not listed below
        VkLayerSettingTypeEXT required_type = VK_LAYER_SETTING_TYPE_MAX_ENUM_EXT;

        // Debugging settings are not added here, those are for internal development and not designed for an app to use via
        // VkLayerSettings API
        if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES_ARM, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES_AMD, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES_IMG, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES_NVIDIA, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_VALIDATE_SYNC, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_CHECK_SHADERS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_THREAD_SAFETY, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_STATELESS_PARAM, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_OBJECT_LIFETIME, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_VALIDATE_CORE, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_UNIQUE_HANDLES, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_CHECK_SHADERS_CACHING, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_CHECK_COMMAND_BUFFER, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_CHECK_OBJECT_IN_USE, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_CHECK_QUERY, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_CHECK_IMAGE_LAYOUT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_FINE_GRAINED_LOCKING, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_MESSAGE_ID_FILTER, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_STRING_EXT;
        } else if (strcmp(VK_LAYER_CUSTOM_STYPE_LIST, setting.pSettingName) == 0) {
            // TODO - Need to decide on a type for this
        } else if (strcmp(VK_LAYER_ENABLE_MESSAGE_LIMIT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_DUPLICATE_MESSAGE_LIMIT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_UINT32_EXT;
        } else if (strcmp(VK_LAYER_PRINTF_ONLY_PRESET, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_PRINTF_ENABLE, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_PRINTF_TO_STDOUT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_PRINTF_VERBOSE, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_PRINTF_BUFFER_SIZE, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_UINT32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_ENABLE, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(DEPRECATED_VK_LAYER_VALIDATE_GPU_BASED, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_SHADER_INSTRUMENTATION, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_DESCRIPTOR_CHECKS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_BUFFER_ADDRESS_OOB, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_UINT32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_VALIDATE_RAY_QUERY, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_POST_PROCESS_DESCRIPTOR_INDEXING, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_BUFFERS_VALIDATION, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_INDIRECT_DRAWS_BUFFERS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_INDIRECT_DISPATCHES_BUFFERS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_INDIRECT_TRACE_RAYS_BUFFERS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_BUFFER_COPIES, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_INDEX_BUFFERS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_IMAGE_LAYOUT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_RESERVE_BINDING_SLOT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_SYNCVAL_SUBMIT_TIME_VALIDATION, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_SYNCVAL_SHADER_ACCESSES_HEURISTIC, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES_PRETTY_PRINT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(VK_LAYER_LOG_FILENAME, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_STRING_EXT;
        } else if (strcmp(VK_LAYER_DEBUG_ACTION, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_STRING_EXT;
        } else if (strcmp(VK_LAYER_REPORT_FLAGS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_STRING_EXT;
        } else if (strcmp(VK_LAYER_ENABLES, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_STRING_EXT;
        } else if (strcmp(VK_LAYER_DISABLES, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_STRING_EXT;
        } else if (strcmp(DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(DEPRECATED_VK_LAYER_GPUAV_VALIDATE_INDIRECT_BUFFER, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else if (strcmp(DEPRECATED_VK_LAYER_VALIDATE_SYNC_QUEUE_SUBMIT, setting.pSettingName) == 0) {
            required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
        } else {
            setting_warnings.emplace_back("The setting " + std::string(setting.pSettingName) +
                                          " in VkLayerSettingsCreateInfoEXT was not recognize by the Validation Layers. Please "
                                          "view the VkLayer_khronos_validation.json for a list of all settings.");
        }

        if (required_type != VK_LAYER_SETTING_TYPE_MAX_ENUM_EXT && setting.type != required_type) {
            setting_warnings.emplace_back(
                "The setting " + std::string(setting.pSettingName) + " in VkLayerSettingsCreateInfoEXT was set to type " +
                std::string(string_VkLayerSettingTypeEXT(setting.type)) + " but requires type " +
                std::string(string_VkLayerSettingTypeEXT(required_type)) + " and the value may be parsed incorrectly.");
        }

        if (used_settings.count(setting.pSettingName)) {
            setting_warnings.emplace_back(
                "The setting " + std::string(setting.pSettingName) +
                " in VkLayerSettingsCreateInfoEXT was listed twice and only the first one listed will be recognized.");
        }
        used_settings.insert(setting.pSettingName);
    }
}

static void SetValidationSetting(VkuLayerSettingSet layer_setting_set, CHECK_DISABLED &disable_data,
                                 const DisableFlags feature_disable, const char *setting) {
    if (vkuHasLayerSetting(layer_setting_set, setting)) {
        bool enabled = true;
        vkuGetLayerSettingValue(layer_setting_set, setting, enabled);
        disable_data[feature_disable] = !enabled;
    }
}

static void SetValidationSetting(VkuLayerSettingSet layer_setting_set, CHECK_ENABLED &enable_data, const EnableFlags feature_enable,
                                 const char *setting) {
    if (vkuHasLayerSetting(layer_setting_set, setting)) {
        bool enabled = true;
        vkuGetLayerSettingValue(layer_setting_set, setting, enabled);
        enable_data[feature_enable] = enabled;
    }
}

static std::string Merge(const std::vector<std::string> &strings) {
    std::string result;

    for (std::size_t i = 0, n = strings.size(); i < n; ++i) {
        if (!result.empty()) {
            result += ",";
        }
        result += strings[i];
    }

    return result;
}

// If log_filename is NULL or stdout, return stdout, otherwise try to open log_filename
// as a filename. If successful, return file handle, otherwise stdout
FILE *GetLayerLogOutput(const char *log_filename, std::vector<std::string> &setting_warnings) {
    FILE *log_output = NULL;
    if (!log_filename || !strcmp("stdout", log_filename)) {
        log_output = stdout;
    } else {
        log_output = fopen(log_filename, "w");
        if (log_output == NULL) {
            if (log_filename) {
                setting_warnings.emplace_back("log_filename (" + std::string(log_filename) +
                                              ") could not be opened, falling back to stdout instead.");
            }
            log_output = stdout;
        }
    }
    return log_output;
}

// Definitions for Debug Actions
enum VkLayerDbgActionBits {
    VK_DBG_LAYER_ACTION_IGNORE = 0x00000000,
    VK_DBG_LAYER_ACTION_CALLBACK = 0x00000001,
    VK_DBG_LAYER_ACTION_LOG_MSG = 0x00000002,
    VK_DBG_LAYER_ACTION_BREAK = 0x00000004,
    VK_DBG_LAYER_ACTION_DEBUG_OUTPUT = 0x00000008,
    VK_DBG_LAYER_ACTION_DEFAULT = 0x40000000,
};
using VkLayerDbgActionFlags = VkFlags;

static void ProcessDebugReportSettings(ConfigAndEnvSettings *settings_data, VkuLayerSettingSet &layer_setting_set,
                                       std::vector<std::string> &setting_warnings) {
    DebugReport *debug_report = settings_data->debug_report;
    // Message ID Filtering
    std::vector<std::string> message_id_filter;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_MESSAGE_ID_FILTER)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_MESSAGE_ID_FILTER, message_id_filter);
    }
    const std::string string_message_id_filter = Merge(message_id_filter);
    CreateFilterMessageIdList(string_message_id_filter, ",", debug_report->filter_message_ids);

    // Duplicate message limit
    bool enable_message_limit = true;       // default in JSON
    uint32_t duplicate_message_limit = 10;  // default in JSON
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_ENABLE_MESSAGE_LIMIT)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_ENABLE_MESSAGE_LIMIT, enable_message_limit);
    }
    if (enable_message_limit) {
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_DUPLICATE_MESSAGE_LIMIT)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_DUPLICATE_MESSAGE_LIMIT, duplicate_message_limit);
        }
    } else {
        duplicate_message_limit = 0;
    }
    debug_report->duplicate_message_limit = duplicate_message_limit;

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME,
                                debug_report->message_format_settings.display_application_name);
    }

    std::string log_filename = "stdout";  // Default
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_LOG_FILENAME)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_LOG_FILENAME, log_filename);
    }
    const bool is_stdout = log_filename.compare("stdout") == 0;

    // Debug mode to simplify comparison of error messages between application runs
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_DEBUG_STABLE_MESSAGES)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_DEBUG_STABLE_MESSAGES, debug_report->debug_stable_messages);
    }

    // Default
    std::vector<std::string> debug_actions_list = {"VK_DBG_LAYER_ACTION_DEFAULT", "VK_DBG_LAYER_ACTION_LOG_MSG"};
#ifdef WIN32
    // For Windows, enable message logging AND OutputDebugString
    debug_actions_list.push_back("VK_DBG_LAYER_ACTION_DEBUG_OUTPUT");
#endif  // WIN32

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_DEBUG_ACTION)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_DEBUG_ACTION, debug_actions_list);
    }

    VkLayerDbgActionFlags debug_action = 0;
    const vvl::unordered_map<std::string, VkFlags> debug_actions_option = {
        {std::string("VK_DBG_LAYER_ACTION_IGNORE"), VK_DBG_LAYER_ACTION_IGNORE},
        {std::string("VK_DBG_LAYER_ACTION_CALLBACK"), VK_DBG_LAYER_ACTION_CALLBACK},
        {std::string("VK_DBG_LAYER_ACTION_LOG_MSG"), VK_DBG_LAYER_ACTION_LOG_MSG},
        {std::string("VK_DBG_LAYER_ACTION_BREAK"), VK_DBG_LAYER_ACTION_BREAK},
        {std::string("VK_DBG_LAYER_ACTION_DEBUG_OUTPUT"), VK_DBG_LAYER_ACTION_DEBUG_OUTPUT},
        {std::string("VK_DBG_LAYER_ACTION_DEFAULT"), VK_DBG_LAYER_ACTION_DEFAULT}};
    for (const auto &element : debug_actions_list) {
        auto enum_value = debug_actions_option.find(element);
        if (enum_value != debug_actions_option.end()) {
            debug_action |= enum_value->second;
        } else {
            if (element.find(',') != std::string::npos) {
                setting_warnings.emplace_back("\"" + element +
                                              "\" was not a valid option for VK_LAYER_DEBUG_ACTION (ignoring).\nIf using "
                                              "VkLayerSettings, each string needs to be its own VkLayerSettingEXT::pValues.");
            } else {
                setting_warnings.emplace_back(
                    "\"" + element +
                    "\" was not a valid option for VK_LAYER_DEBUG_ACTION (ignoring).\nValid options are "
                    "[VK_DBG_LAYER_ACTION_IGNORE, VK_DBG_LAYER_ACTION_CALLBACK, VK_DBG_LAYER_ACTION_LOG_MSG, "
                    "VK_DBG_LAYER_ACTION_BREAK, VK_DBG_LAYER_ACTION_DEBUG_OUTPUT, VK_DBG_LAYER_ACTION_DEFAULT]");
            }
        }
    }

    std::vector<std::string> report_flags_list = {"error"};  // Default
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_REPORT_FLAGS)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_REPORT_FLAGS, report_flags_list);
    }

    VkLayerDbgActionFlags report_flags = 0;
    const vvl::unordered_map<std::string, VkFlags> report_flags_options = {{std::string("warn"), kWarningBit},
                                                                           {std::string("info"), kInformationBit},
                                                                           {std::string("perf"), kPerformanceWarningBit},
                                                                           {std::string("error"), kErrorBit},
                                                                           {std::string("verbose"), kVerboseBit}};
    for (const auto &element : report_flags_list) {
        auto enum_value = report_flags_options.find(element);
        if (enum_value != report_flags_options.end()) {
            report_flags |= enum_value->second;
        } else {
            if (element.find(',') != std::string::npos) {
                setting_warnings.emplace_back("\"" + element +
                                              "\" was not a valid option for VK_LAYER_REPORT_FLAGS (ignoring)\nIf using "
                                              "VkLayerSettings, each string needs to be its own VkLayerSettingEXT::pValues.");
            } else {
                setting_warnings.emplace_back("\"" + element +
                                              "\" was not a valid option for VK_LAYER_REPORT_FLAGS (ignoring)\nValid options are "
                                              "[error, warn, info, perf, verbose].");
            }
        }
    }

    // Before creating the debug callback, see if other settings interfere
    if (settings_data->gpuav_settings->debug_printf_enabled) {
        if (settings_data->gpuav_settings->debug_printf_to_stdout && (debug_action & VK_DBG_LAYER_ACTION_LOG_MSG)) {
            if (is_stdout) {
                setting_warnings.emplace_back(
                    "The debug callback is already logging to stdout, but " + std::string(VK_LAYER_PRINTF_TO_STDOUT) +
                    " is also enabled. DebugPrintf will skip the debug callback in favor of a direct stdout write.");
            } else {
                setting_warnings.emplace_back("The logging to " + log_filename + " will not contain any DebugPrintf info because " +
                                              std::string(VK_LAYER_PRINTF_TO_STDOUT) + " is enabled.");
            }
        }
        if (!settings_data->gpuav_settings->debug_printf_to_stdout && ((report_flags & kInformationBit) == 0)) {
            // Normally it is a lot of spam to use kInformationBit, but if only using DebugPrintf, it should be minimal information
            // printed
            setting_warnings.emplace_back(
                "DebugPrintf logs to the Information message severity, enabling Information level logging otherwise the message "
                "will not be seen.");
            report_flags |= kInformationBit;
        }
        // If any non-stdout DebugPrintf is being used, just turn off duplicate_message_limit, it will prevent people thinking
        // DebugPrintf is broken because nothing is printing.
        if (!settings_data->gpuav_settings->debug_printf_to_stdout && debug_report->duplicate_message_limit != 0) {
            debug_report->duplicate_message_limit = 0;
            setting_warnings.emplace_back("DebugPrintf logs can possibly print many times, but duplicate_message_limit is set to " +
                                          std::to_string(debug_report->duplicate_message_limit) +
                                          ", setting enable_message_limit to false so all logs are printed.");
        }
    }

    // Flag as default if these settings are not from a vk_layer_settings.txt file
    const bool default_layer_callback = (debug_action & VK_DBG_LAYER_ACTION_DEFAULT) != 0;

    VkDebugUtilsMessengerCreateInfoEXT dbg_create_info = vku::InitStructHelper();
    dbg_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    if (report_flags & kErrorBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    }
    if (report_flags & kWarningBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    }
    if (report_flags & kPerformanceWarningBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        dbg_create_info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    }
    if (report_flags & kInformationBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    }
    if (report_flags & kVerboseBit) {
        dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    }

    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    if (debug_action & VK_DBG_LAYER_ACTION_LOG_MSG) {
        FILE *log_output = GetLayerLogOutput(log_filename.c_str(), setting_warnings);
        if (log_output != stdout) {
            // This particular warning is designed to show the user where the debug callback is going (which is important to know!),
            // so it makes no sense to put the warning in the callback location. For this one only we attempt to print to the
            // everywhere else possible
            const std::string tmp = "Validation Layer Info - Logging validation error to " + log_filename + "\n";
            const char *cstr = tmp.c_str();
            printf("%s", cstr);
#ifdef VK_USE_PLATFORM_WIN32_KHR
            OutputDebugString(cstr);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            __android_log_print(ANDROID_LOG_INFO, "VALIDATION", "%s", cstr);
#endif
        }
        dbg_create_info.pfnUserCallback = MessengerLogCallback;
        dbg_create_info.pUserData = (void *)log_output;
        LayerCreateMessengerCallback(debug_report, default_layer_callback, &dbg_create_info, &messenger);
    } else if (!is_stdout) {
        setting_warnings.emplace_back("The log_filename was set to " + log_filename +
                                      " but VK_DBG_LAYER_ACTION_LOG_MSG was not set, so it won't be sent to the file.");
    }

    messenger = VK_NULL_HANDLE;
    if (debug_action & VK_DBG_LAYER_ACTION_DEBUG_OUTPUT) {
        dbg_create_info.pfnUserCallback = MessengerWin32DebugOutputMsg;
        dbg_create_info.pUserData = nullptr;
        LayerCreateMessengerCallback(debug_report, default_layer_callback, &dbg_create_info, &messenger);
    }

    messenger = VK_NULL_HANDLE;
    if (debug_action & VK_DBG_LAYER_ACTION_BREAK) {
        dbg_create_info.pfnUserCallback = MessengerBreakCallback;
        dbg_create_info.pUserData = nullptr;
        LayerCreateMessengerCallback(debug_report, default_layer_callback, &dbg_create_info, &messenger);
    }
}

static const char *GetDefaultPrefix() {
#ifdef __ANDROID__
    return "vvl";
#else
    return "LAYER";
#endif
}
#endif  // !defined(BUILD_SELF_VVL)

// Global list of sType,size identifiers
std::vector<std::pair<uint32_t, uint32_t>> &GetCustomStypeInfo() {
    static std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info{};
    return custom_stype_info;
}

// Process enables and disables set though the vk_layer_settings.txt config file or through an environment variable
void ProcessConfigAndEnvSettings(ConfigAndEnvSettings *settings_data) {
    // When compiling a build for self validation, ProcessConfigAndEnvSettings immediately returns,
    // so that the layer always defaults to the standard validation options we want,
    // and does not try to process option coming from the VVL we are debugging
#if defined(BUILD_SELF_VVL)
    // Setup default messenger callback to stdout and just error validation messages
    FILE *log_output = stdout;
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    VkDebugUtilsMessengerCreateInfoEXT dbg_create_info = vku::InitStructHelper();
    dbg_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    dbg_create_info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbg_create_info.pfnUserCallback = MessengerLogCallback;
    dbg_create_info.pUserData = (void *)log_output;
    LayerCreateMessengerCallback(settings_data->debug_report, true, &dbg_create_info, &messenger);

#ifdef WIN32
    messenger = VK_NULL_HANDLE;
    dbg_create_info.pfnUserCallback = MessengerWin32DebugOutputMsg;
    dbg_create_info.pUserData = nullptr;
    LayerCreateMessengerCallback(settings_data->debug_report, true, &dbg_create_info, &messenger);
#endif  // WIN32

    return;
#else

    // We need to send all warnings through the DebugCallback (printf will go into the void and likely is not seen for people), but
    // we also need to get settings setup before creating the DebugCallback. This is the only spot that needs this, so we create a
    // temp list of strings here and shove them through DebugCallback right after we call it. This function should not be a
    // bottleneck so some extra string building should be ok here.
    std::vector<std::string> setting_warnings;

    // If not cleared, garbage has been seen in some Android run effecting the error message
    GetCustomStypeInfo().clear();

    VkuLayerSettingSet layer_setting_set = VK_NULL_HANDLE;
    auto layer_setting_create_info = vkuFindLayerSettingsCreateInfo(settings_data->create_info);
    if (!ValidateLayerSettingsCreateInfo(layer_setting_create_info)) {
        return;  // nullptr will crash things
    }
    vkuCreateLayerSettingSet(OBJECT_LAYER_NAME, layer_setting_create_info, nullptr, nullptr, &layer_setting_set);

    vkuSetLayerSettingCompatibilityNamespace(layer_setting_set, GetDefaultPrefix());

    ValidateLayerSettingsProvided(layer_setting_create_info, layer_setting_set, setting_warnings);

    // Read legacy "enables" flags for backward compatibility
    std::vector<std::string> enables;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_ENABLES)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_ENABLES, enables);
    }
    const std::string string_enables = Merge(enables);
    SetLocalEnableSetting(string_enables, ",", settings_data->enables);

    // Read legacy "disables" flags for backward compatibility
    std::vector<std::string> disables;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_DISABLES)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_DISABLES, disables);
    }
    const std::string string_disables = Merge(disables);
    SetLocalDisableSetting(string_disables, ",", settings_data->disables);

    GlobalSettings &global_settings = *settings_data->global_settings;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_FINE_GRAINED_LOCKING)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_FINE_GRAINED_LOCKING, global_settings.fine_grained_locking);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_DEBUG_DISABLE_SPIRV_VAL)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_DEBUG_DISABLE_SPIRV_VAL, global_settings.debug_disable_spirv_val);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_CUSTOM_STYPE_LIST)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_CUSTOM_STYPE_LIST, GetCustomStypeInfo());
    }

    GpuAVSettings &gpuav_settings = *settings_data->gpuav_settings;
    bool shader_instrumentation_enabled = true;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_SHADER_INSTRUMENTATION)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_SHADER_INSTRUMENTATION, shader_instrumentation_enabled);
    }

    if (!shader_instrumentation_enabled) {
        gpuav_settings.DisableShaderInstrumentationAndOptions();
    } else {
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_DESCRIPTOR_CHECKS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_DESCRIPTOR_CHECKS,
                                    gpuav_settings.shader_instrumentation.descriptor_checks);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_POST_PROCESS_DESCRIPTOR_INDEXING)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_POST_PROCESS_DESCRIPTOR_INDEXING,
                                    gpuav_settings.shader_instrumentation.post_process_descriptor_index);
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VERTEX_ATTRIBUTE_FETCH_OOB)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VERTEX_ATTRIBUTE_FETCH_OOB,
                                    gpuav_settings.shader_instrumentation.vertex_attribute_fetch_oob);
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB, gpuav_settings.warn_on_robust_oob);
        } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB)) {
            vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB, gpuav_settings.warn_on_robust_oob);
            setting_warnings.emplace_back("Deprecated " + std::string(DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB) +
                                          " setting was set, use " + std::string(VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB) + " instead.");
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_BUFFER_ADDRESS_OOB)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_BUFFER_ADDRESS_OOB,
                                    gpuav_settings.shader_instrumentation.buffer_device_address);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES)) {
            const uint32_t default_max_bda_in_use = gpuav_settings.max_bda_in_use;
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES, gpuav_settings.max_bda_in_use);
            if (gpuav_settings.max_bda_in_use == 0) {
                gpuav_settings.max_bda_in_use = default_max_bda_in_use;
                setting_warnings.emplace_back(std::string(VK_LAYER_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES) +
                                              " was set to zero, which is invalid, setting to the default of " +
                                              std::to_string(default_max_bda_in_use));
            }
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_RAY_QUERY)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_RAY_QUERY,
                                    gpuav_settings.shader_instrumentation.ray_query);
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS,
                                    gpuav_settings.select_instrumented_shaders);
        } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS)) {
            vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS,
                                    gpuav_settings.select_instrumented_shaders);
            setting_warnings.emplace_back("Deprecated " + std::string(DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS) +
                                          " setting was set, use " + std::string(VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS) +
                                          " instead.");
        }

        // No need to enable shader instrumentation options is no instrumentation is done
        if (!gpuav_settings.IsShaderInstrumentationEnabled()) {
            gpuav_settings.DisableShaderInstrumentationAndOptions();
        }
    }

    bool buffers_validation_enabled = true;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_BUFFERS_VALIDATION)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_BUFFERS_VALIDATION, buffers_validation_enabled);
    }
    if (!buffers_validation_enabled) {
        gpuav_settings.SetBufferValidationEnabled(false);
    } else {
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_INDIRECT_DRAWS_BUFFERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_INDIRECT_DRAWS_BUFFERS,
                                    gpuav_settings.validate_indirect_draws_buffers);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_INDIRECT_DISPATCHES_BUFFERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_INDIRECT_DISPATCHES_BUFFERS,
                                    gpuav_settings.validate_indirect_dispatches_buffers);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_INDIRECT_TRACE_RAYS_BUFFERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_INDIRECT_TRACE_RAYS_BUFFERS,
                                    gpuav_settings.validate_indirect_trace_rays_buffers);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_BUFFER_COPIES)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_BUFFER_COPIES, gpuav_settings.validate_buffer_copies);
        } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES)) {
            vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES,
                                    gpuav_settings.validate_buffer_copies);
            setting_warnings.emplace_back("Deprecated " + std::string(DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES) +
                                          " setting was set, use " + std::string(VK_LAYER_GPUAV_BUFFER_COPIES) + " instead.");
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_INDEX_BUFFERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_INDEX_BUFFERS, gpuav_settings.validate_index_buffers);
        }
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_IMAGE_LAYOUT)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_IMAGE_LAYOUT, gpuav_settings.validate_image_layout);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_RESERVE_BINDING_SLOT)) {
        SetValidationSetting(layer_setting_set, settings_data->enables, gpu_validation_reserve_binding_slot,
                             VK_LAYER_GPUAV_RESERVE_BINDING_SLOT);
    } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT)) {
        SetValidationSetting(layer_setting_set, settings_data->enables, gpu_validation_reserve_binding_slot,
                             DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT);
        setting_warnings.emplace_back("Deprecated " + std::string(DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT) +
                                      " setting was set, use " + std::string(VK_LAYER_GPUAV_RESERVE_BINDING_SLOT) + " instead.");
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT, gpuav_settings.vma_linear_output);
    } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT)) {
        vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT, gpuav_settings.vma_linear_output);
        setting_warnings.emplace_back("Deprecated " + std::string(DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT) + " setting was set, use " +
                                      std::string(VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT) + " instead.");
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_DEBUG_VALIDATE_INSTRUMENTED_SHADERS)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_DEBUG_VALIDATE_INSTRUMENTED_SHADERS,
                                gpuav_settings.debug_validate_instrumented_shaders);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_DEBUG_DUMP_INSTRUMENTED_SHADERS)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_DEBUG_DUMP_INSTRUMENTED_SHADERS,
                                gpuav_settings.debug_dump_instrumented_shaders);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_DEBUG_MAX_INSTRUMENTATIONS_COUNT)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_DEBUG_MAX_INSTRUMENTATIONS_COUNT,
                                gpuav_settings.debug_max_instrumentations_count);
    }

    // Debug Printf - (which we bundle into GPU-AV internally)
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_PRINTF_ENABLE)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_PRINTF_ENABLE, gpuav_settings.debug_printf_enabled);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_DEBUG_PRINT_INSTRUMENTATION_INFO)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_DEBUG_PRINT_INSTRUMENTATION_INFO,
                                gpuav_settings.debug_print_instrumentation_info);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_PRINTF_TO_STDOUT)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_PRINTF_TO_STDOUT, gpuav_settings.debug_printf_to_stdout);
    }

    // This option was published when DebugPrintf came out, leave to not break people's flow
    // Deprecated right after the 1.3.280 SDK release
    if (!GetEnvironment("DEBUG_PRINTF_TO_STDOUT").empty()) {
        setting_warnings.emplace_back("DEBUG_PRINTF_TO_STDOUT was set, this is deprecated, please use VK_LAYER_PRINTF_TO_STDOUT");
        gpuav_settings.debug_printf_to_stdout = true;
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_PRINTF_VERBOSE)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_PRINTF_VERBOSE, gpuav_settings.debug_printf_verbose);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_PRINTF_BUFFER_SIZE)) {
        const uint32_t default_buffer_size = gpuav_settings.debug_printf_buffer_size;
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_PRINTF_BUFFER_SIZE, gpuav_settings.debug_printf_buffer_size);
        if (gpuav_settings.debug_printf_buffer_size == 0) {
            gpuav_settings.debug_printf_buffer_size = default_buffer_size;
            setting_warnings.emplace_back(std::string(VK_LAYER_PRINTF_BUFFER_SIZE) +
                                          " was set to zero, which is invalid, setting to the default of " +
                                          std::to_string(default_buffer_size));
        }
    }

    SyncValSettings &syncval_settings = *settings_data->syncval_settings;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_SYNCVAL_SUBMIT_TIME_VALIDATION)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_SYNCVAL_SUBMIT_TIME_VALIDATION,
                                syncval_settings.submit_time_validation);
    } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_VK_LAYER_VALIDATE_SYNC_QUEUE_SUBMIT)) {
        vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_VK_LAYER_VALIDATE_SYNC_QUEUE_SUBMIT,
                                syncval_settings.submit_time_validation);
        setting_warnings.emplace_back("Deprecated " + std::string(DEPRECATED_VK_LAYER_VALIDATE_SYNC_QUEUE_SUBMIT) +
                                      " setting was set, use " + std::string(VK_LAYER_SYNCVAL_SUBMIT_TIME_VALIDATION) +
                                      " instead.");
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_SYNCVAL_SHADER_ACCESSES_HEURISTIC)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_SYNCVAL_SHADER_ACCESSES_HEURISTIC,
                                syncval_settings.shader_accesses_heuristic);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES,
                                syncval_settings.message_extra_properties);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES_PRETTY_PRINT)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES_PRETTY_PRINT,
                                syncval_settings.message_extra_properties_pretty_print);
    }

    const auto *validation_features_ext = vku::FindStructInPNextChain<VkValidationFeaturesEXT>(settings_data->create_info);
    if (validation_features_ext) {
        SetValidationFeatures(settings_data->disables, settings_data->enables, validation_features_ext);
    }
    const auto *validation_flags_ext = vku::FindStructInPNextChain<VkValidationFlagsEXT>(settings_data->create_info);
    if (validation_flags_ext) {
        SetValidationFlags(settings_data->disables, validation_flags_ext);
    }

    const bool use_fine_grained_settings = disables.empty() && enables.empty();

    // Only read the legacy enables flags when used, not their replacement.
    // Avoid Android C.I. performance regression from reading Android env variables
    if (use_fine_grained_settings) {
        SetValidationSetting(layer_setting_set, settings_data->enables, best_practices, VK_LAYER_VALIDATE_BEST_PRACTICES);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_arm, VK_LAYER_VALIDATE_BEST_PRACTICES_ARM);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_amd, VK_LAYER_VALIDATE_BEST_PRACTICES_AMD);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_img, VK_LAYER_VALIDATE_BEST_PRACTICES_IMG);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_nvidia,
                             VK_LAYER_VALIDATE_BEST_PRACTICES_NVIDIA);
        SetValidationSetting(layer_setting_set, settings_data->enables, sync_validation, VK_LAYER_VALIDATE_SYNC);

        // These were deprecated after the 1.3.296 SDK release
        // Before GPU-AV and DebugPrintf were merged, we used this enum to set GPU-AV and DebugPrintf in vkconfig.
        // This code should in theory be dead since removing it from vkconfig, but keep just incase for a bit
        if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_VK_LAYER_VALIDATE_GPU_BASED)) {
            std::string setting_value;
            vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_VK_LAYER_VALIDATE_GPU_BASED, setting_value);
            if (setting_value == "GPU_BASED_GPU_ASSISTED") {
                settings_data->enables[gpu_validation] = true;
                setting_warnings.emplace_back("Deprecated " + std::string(DEPRECATED_VK_LAYER_VALIDATE_GPU_BASED) +
                                              " setting was set, use " + std::string(VK_LAYER_GPUAV_ENABLE) + " instead.");
            } else if (setting_value == "GPU_BASED_DEBUG_PRINTF") {
                settings_data->enables[debug_printf_validation] = true;
                setting_warnings.emplace_back("Deprecated " + std::string(DEPRECATED_VK_LAYER_VALIDATE_GPU_BASED) +
                                              " setting was set, use " + std::string(VK_LAYER_PRINTF_ENABLE) + " instead.");
            }
        }
    }

    // Only read the legacy disables flags when used, not their replacement.
    // Avoid Android C.I. performance regression from reading Android env variables
    if (use_fine_grained_settings) {
        SetValidationSetting(layer_setting_set, settings_data->disables, stateless_checks, VK_LAYER_STATELESS_PARAM);
        SetValidationSetting(layer_setting_set, settings_data->disables, thread_safety, VK_LAYER_THREAD_SAFETY);
        SetValidationSetting(layer_setting_set, settings_data->disables, core_checks, VK_LAYER_VALIDATE_CORE);
        SetValidationSetting(layer_setting_set, settings_data->disables, command_buffer_state, VK_LAYER_CHECK_COMMAND_BUFFER);
        SetValidationSetting(layer_setting_set, settings_data->disables, object_in_use, VK_LAYER_CHECK_OBJECT_IN_USE);
        SetValidationSetting(layer_setting_set, settings_data->disables, query_validation, VK_LAYER_CHECK_QUERY);
        SetValidationSetting(layer_setting_set, settings_data->disables, image_layout_validation, VK_LAYER_CHECK_IMAGE_LAYOUT);
        SetValidationSetting(layer_setting_set, settings_data->disables, handle_wrapping, VK_LAYER_UNIQUE_HANDLES);
        SetValidationSetting(layer_setting_set, settings_data->disables, object_tracking, VK_LAYER_OBJECT_LIFETIME);
        SetValidationSetting(layer_setting_set, settings_data->disables, shader_validation, VK_LAYER_CHECK_SHADERS);
        SetValidationSetting(layer_setting_set, settings_data->disables, shader_validation_caching, VK_LAYER_CHECK_SHADERS_CACHING);
    }

    // This is the "original" way to use DebugPrintf before you could use it with GPU-AV
    // In this case, we want to emulate supporting only for DebugPrintf with GPU-AV disabled
    if (settings_data->enables[debug_printf_validation]) {
        gpuav_settings.debug_printf_enabled = true;
        if (!settings_data->enables[gpu_validation]) {
            gpuav_settings.SetOnlyDebugPrintf();
        }
    } else if (gpuav_settings.debug_printf_enabled) {
        // enabled the new way, but chassis uses this to create Validation Object
        settings_data->enables[debug_printf_validation] = true;
    }

    // New way to override everything to make it easy to use DebugPrintf when VkConfig isn't available
    bool printf_only_preset = false;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_PRINTF_ONLY_PRESET)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_PRINTF_ONLY_PRESET, printf_only_preset);
        if (printf_only_preset) {
            gpuav_settings.SetOnlyDebugPrintf();
            // chassis uses this to create Validation Object
            settings_data->enables[debug_printf_validation] = true;
            settings_data->enables[gpu_validation] = false;

            // Tried to use VK_VALIDATION_FEATURE_DISABLE_ALL_EXT but when running on Android
            // (--gtest_filter="NegativeGpuAVDebugPrintf.BasicLayerSettingsPrintfPreset:NegativeWsi.UseDestroyedSwapchain")
            // Was getting asserts in VulkanTypedHandle::Cast() Simple fix was to just not disable
            // VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT
            SetValidationFeatureDisable(settings_data->disables, VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT);
            SetValidationFeatureDisable(settings_data->disables, VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT);
            SetValidationFeatureDisable(settings_data->disables, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT);
            SetValidationFeatureDisable(settings_data->disables, VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT);
            SetValidationFeatureDisable(settings_data->disables, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT);
            SetValidationFeatureDisable(settings_data->disables, VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT);
            setting_warnings.emplace_back(
                "Disabling as much of normal validation as possible so that only DebugPrintf will be running.");
        }
    }

    // This is the "new" way to enable GPU-AV
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_ENABLE)) {
        bool gpuav_enable = false;
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_ENABLE, gpuav_enable);
        if (printf_only_preset) {
            setting_warnings.emplace_back(std::string(VK_LAYER_PRINTF_ONLY_PRESET) + " was set, so ignoring " + std::string(VK_LAYER_GPUAV_ENABLE) + ".");
        } else if (gpuav_enable) {
            // enabled the new way, but chassis uses this to create Validation Object
            settings_data->enables[gpu_validation] = true;
        }
    }

    // if app is setting VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT, we can use this to disable it for debugging
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_DEBUG_DISABLE_ALL)) {
        bool disable_gpuav = false;
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_DEBUG_DISABLE_ALL, disable_gpuav);
        if (disable_gpuav) {
            settings_data->enables[gpu_validation] = false;
        }
    }

    if (settings_data->enables[gpu_validation] && !settings_data->disables[core_checks]) {
        setting_warnings.emplace_back(
            "Both GPU Assisted Validation and Normal Core Check Validation are enabled, this is not recommend as it  will be very "
            "slow. Once all "
            "errors in Core Check are solved, please disable, then only use GPU-AV for best performance.");
    }

    // Last as previous settings are needed so we can make sure they line up with the DebugReport settings
    ProcessDebugReportSettings(settings_data, layer_setting_set, setting_warnings);

    // Grab application name here while we have access to it and know if to save it or not
    if (settings_data->debug_report->message_format_settings.display_application_name) {
        const VkApplicationInfo *app_info = settings_data->create_info->pApplicationInfo;
        settings_data->debug_report->message_format_settings.application_name =
            (app_info && app_info->pApplicationName) ? app_info->pApplicationName : "";
    }

    for (const auto &warning : setting_warnings) {
        settings_data->debug_report->DebugLogMsg(kWarningBit, {}, warning.c_str(), "VALIDATION-SETTINGS");
    }

    vkuDestroyLayerSettingSet(layer_setting_set, nullptr);
#endif  // !BUILD_SELF_VVL
}
