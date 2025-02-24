/* Copyright (c) 2022-2024 The Khronos Group Inc.
 * Copyright (c) 2022-2024 Valve Corporation
 * Copyright (c) 2022-2024 LunarG, Inc.
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

#pragma once
#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/utility/vk_struct_helper.hpp>
#include "containers/custom_containers.h"

#define OBJECT_LAYER_NAME "VK_LAYER_KHRONOS_validation"

enum ValidationCheckDisables {
    VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE,
    VALIDATION_CHECK_DISABLE_OBJECT_IN_USE,
    VALIDATION_CHECK_DISABLE_QUERY_VALIDATION,
    VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION,
};

enum ValidationCheckEnables {
    VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM,
    VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD,
    VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG,
    VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA,
    VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL,
};

// CHECK_DISABLED and CHECK_ENABLED vectors are containers for bools that can opt in or out of specific classes of validation
// checks. Enum values can be specified via the vk_layer_settings.txt config file or at CreateInstance time via the
// VK_EXT_validation_features extension that can selectively disable or enable checks.
enum DisableFlags {
    command_buffer_state,
    object_in_use,
    query_validation,
    image_layout_validation,
    object_tracking,
    core_checks,
    thread_safety,
    stateless_checks,
    handle_wrapping,
    shader_validation,
    shader_validation_caching,
    // Insert new disables above this line
    kMaxDisableFlags,
};

enum EnableFlags {
    gpu_validation,
    gpu_validation_reserve_binding_slot,
    best_practices,
    vendor_specific_arm,
    vendor_specific_amd,
    vendor_specific_img,
    vendor_specific_nvidia,
    debug_printf_validation,
    sync_validation,
    // Insert new enables above this line
    kMaxEnableFlags,
};

using CHECK_DISABLED = std::array<bool, kMaxDisableFlags>;
using CHECK_ENABLED = std::array<bool, kMaxEnableFlags>;

// General settings to be used by all parts of the Validation Layers
struct GlobalSettings {
    bool fine_grained_locking = true;

    bool debug_disable_spirv_val = false;
};

class DebugReport;
struct GpuAVSettings;
struct SyncValSettings;
struct MessageFormatSettings;
struct ConfigAndEnvSettings {
    // Matches up with what is passed down to VK_EXT_layer_settings
    const char *layer_description;

    // Used so we can find things like VkValidationFeaturesEXT
    const VkInstanceCreateInfo *create_info;

    // Find grain way to turn off/on parts of validation
    CHECK_ENABLED &enables;
    CHECK_DISABLED &disables;

    // Settings for DebugReport
    DebugReport *debug_report;

    GlobalSettings* global_settings;

    // Individual settings for different internal layers
    GpuAVSettings *gpuav_settings;
    SyncValSettings *syncval_settings;
};
const std::vector<std::string> &GetDisableFlagNameHelper();
const std::vector<std::string> &GetEnableFlagNameHelper();

// Process validation features, flags and settings specified through extensions, a layer settings file, or environment variables
void ProcessConfigAndEnvSettings(ConfigAndEnvSettings *settings_data);

std::vector<std::pair<uint32_t, uint32_t>> &GetCustomStypeInfo();
