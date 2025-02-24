/*
 *
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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
 *
 *
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "vulkan/vulkan_core.h"

#include "log.h"
#include "vk_loader_platform.h"

struct loader_instance;
struct loader_layer_list;
struct loader_pointer_layer_list;
struct loader_envvar_all_filters;
typedef struct log_configuration log_configuration;

typedef enum loader_settings_layer_control {
    LOADER_SETTINGS_LAYER_CONTROL_DEFAULT,          // layer is not enabled by settings file but can be enabled through other means
    LOADER_SETTINGS_LAYER_CONTROL_ON,               // layer is enabled by settings file
    LOADER_SETTINGS_LAYER_CONTROL_OFF,              // layer is prevented from being enabled
    LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION  // special control indicating unspecified layers should go here. If this is not
                                                    // in the settings file, then the loader assume no other layers should be
                                                    // searched & loaded.
} loader_settings_layer_control;

// If a loader_settings_layer_configuration has a name of loader_settings_unknown_layers_location, then it specifies that the
// layer configuration it was found in shall be the location all layers not listed in the settings file that are enabled.
#define LOADER_SETTINGS_UNKNOWN_LAYERS_LOCATION "loader_settings_unknown_layers_location"

#define LOADER_SETTINGS_MAX_NAME_SIZE 256U;

typedef struct loader_settings_layer_configuration {
    char* name;
    char* path;
    loader_settings_layer_control control;
    bool treat_as_implicit_manifest;  // whether or not the layer should be parsed as if it is implicit

} loader_settings_layer_configuration;

typedef struct loader_settings {
    bool settings_active;
    bool has_unordered_layer_location;
    enum vulkan_loader_debug_flags debug_level;

    uint32_t layer_configuration_count;
    loader_settings_layer_configuration* layer_configurations;

    char* settings_file_path;
} loader_settings;

// Call this function to get the current settings that the loader should use.
// It will open up the current loader settings file and return a loader_settings in out_loader_settings if it.
// It should be called on every call to the global functions excluding vkGetInstanceProcAddr
// Caller is responsible for cleaning up by calling free_loader_settings()
VkResult get_loader_settings(const struct loader_instance* inst, loader_settings* out_loader_settings);

void free_loader_settings(const struct loader_instance* inst, loader_settings* loader_settings);

// Log the settings to the console
void log_settings(const struct loader_instance* inst, loader_settings* settings);

// Every global function needs to call this at startup to insure that
TEST_FUNCTION_EXPORT VkResult update_global_loader_settings(void);

// Needs to be called during startup -
void init_global_loader_settings(void);
void teardown_global_loader_settings(void);

// Check the global settings and return true if msg_type does not correspond to the active global loader settings
bool should_skip_logging_global_messages(VkFlags msg_type);

// Query the current settings (either global or per-instance) and return the list of layers contained within.
// should_search_for_other_layers tells the caller if the settings file should be used exclusively for layer searching or not
TEST_FUNCTION_EXPORT VkResult get_settings_layers(const struct loader_instance* inst, struct loader_layer_list* settings_layers,
                                                  bool* should_search_for_other_layers);

// Take the provided list of settings_layers and add in the layers from regular search paths
// Only adds layers that aren't already present in the settings_layers and in the location of the
// layer configuration with LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION set
VkResult combine_settings_layers_with_regular_layers(const struct loader_instance* inst, struct loader_layer_list* settings_layers,
                                                     struct loader_layer_list* regular_layers,
                                                     struct loader_layer_list* output_layers);

// Fill out activated_layer_list with the layers that should be activated, based on environment variables, VkInstanceCreateInfo, and
// the settings
VkResult enable_correct_layers_from_settings(const struct loader_instance* inst, const struct loader_envvar_all_filters* filters,
                                             uint32_t app_enabled_name_count, const char* const* app_enabled_names,
                                             const struct loader_layer_list* instance_layers,
                                             struct loader_pointer_layer_list* target_layer_list,
                                             struct loader_pointer_layer_list* activated_layer_list);
