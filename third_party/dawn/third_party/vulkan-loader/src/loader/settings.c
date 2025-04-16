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

#include "settings.h"

#include "allocation.h"
#include "cJSON.h"
#include "loader.h"
#include "loader_environment.h"
#include "loader_json.h"
#if defined(WIN32)
#include "loader_windows.h"
#endif
#include "log.h"
#include "stack_allocation.h"
#include "vk_loader_platform.h"

loader_platform_thread_mutex global_loader_settings_lock;
loader_settings global_loader_settings;

void free_layer_configuration(const struct loader_instance* inst, loader_settings_layer_configuration* layer_configuration) {
    loader_instance_heap_free(inst, layer_configuration->name);
    loader_instance_heap_free(inst, layer_configuration->path);
    memset(layer_configuration, 0, sizeof(loader_settings_layer_configuration));
}

void free_loader_settings(const struct loader_instance* inst, loader_settings* settings) {
    if (NULL != settings->layer_configurations) {
        for (uint32_t i = 0; i < settings->layer_configuration_count; i++) {
            free_layer_configuration(inst, &settings->layer_configurations[i]);
        }
    }
    loader_instance_heap_free(inst, settings->layer_configurations);
    loader_instance_heap_free(inst, settings->settings_file_path);
    memset(settings, 0, sizeof(loader_settings));
}

loader_settings_layer_control parse_control_string(char* control_string) {
    loader_settings_layer_control layer_control = LOADER_SETTINGS_LAYER_CONTROL_DEFAULT;
    if (strcmp(control_string, "auto") == 0)
        layer_control = LOADER_SETTINGS_LAYER_CONTROL_DEFAULT;
    else if (strcmp(control_string, "on") == 0)
        layer_control = LOADER_SETTINGS_LAYER_CONTROL_ON;
    else if (strcmp(control_string, "off") == 0)
        layer_control = LOADER_SETTINGS_LAYER_CONTROL_OFF;
    else if (strcmp(control_string, "unordered_layer_location") == 0)
        layer_control = LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION;
    return layer_control;
}

const char* loader_settings_layer_control_to_string(loader_settings_layer_control control) {
    switch (control) {
        case (LOADER_SETTINGS_LAYER_CONTROL_DEFAULT):
            return "auto";
        case (LOADER_SETTINGS_LAYER_CONTROL_ON):
            return "on";
        case (LOADER_SETTINGS_LAYER_CONTROL_OFF):
            return "off";
        case (LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION):
            return "unordered_layer_location";
        default:
            return "UNKNOWN_LAYER_CONTROl";
    }
}

uint32_t parse_log_filters_from_strings(struct loader_string_list* log_filters) {
    uint32_t filters = 0;
    for (uint32_t i = 0; i < log_filters->count; i++) {
        if (strcmp(log_filters->list[i], "all") == 0)
            filters |= VULKAN_LOADER_INFO_BIT | VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_PERF_BIT | VULKAN_LOADER_ERROR_BIT |
                       VULKAN_LOADER_DEBUG_BIT | VULKAN_LOADER_LAYER_BIT | VULKAN_LOADER_DRIVER_BIT | VULKAN_LOADER_VALIDATION_BIT;
        else if (strcmp(log_filters->list[i], "info") == 0)
            filters |= VULKAN_LOADER_INFO_BIT;
        else if (strcmp(log_filters->list[i], "warn") == 0)
            filters |= VULKAN_LOADER_WARN_BIT;
        else if (strcmp(log_filters->list[i], "perf") == 0)
            filters |= VULKAN_LOADER_PERF_BIT;
        else if (strcmp(log_filters->list[i], "error") == 0)
            filters |= VULKAN_LOADER_ERROR_BIT;
        else if (strcmp(log_filters->list[i], "debug") == 0)
            filters |= VULKAN_LOADER_DEBUG_BIT;
        else if (strcmp(log_filters->list[i], "layer") == 0)
            filters |= VULKAN_LOADER_LAYER_BIT;
        else if (strcmp(log_filters->list[i], "driver") == 0)
            filters |= VULKAN_LOADER_DRIVER_BIT;
        else if (strcmp(log_filters->list[i], "validation") == 0)
            filters |= VULKAN_LOADER_VALIDATION_BIT;
    }
    return filters;
}

bool parse_json_enable_disable_option(cJSON* object) {
    char* str = loader_cJSON_GetStringValue(object);
    if (NULL == str) {
        return false;
    }
    bool enable = false;
    if (strcmp(str, "enabled") == 0) {
        enable = true;
    }
    return enable;
}

VkResult parse_layer_configuration(const struct loader_instance* inst, cJSON* layer_configuration_json,
                                   loader_settings_layer_configuration* layer_configuration) {
    char* control_string = NULL;
    VkResult res = loader_parse_json_string(layer_configuration_json, "control", &control_string);
    if (res != VK_SUCCESS) {
        goto out;
    }
    layer_configuration->control = parse_control_string(control_string);
    loader_instance_heap_free(inst, control_string);

    // If that is the only value - do no further parsing
    if (layer_configuration->control == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
        goto out;
    }

    res = loader_parse_json_string(layer_configuration_json, "name", &(layer_configuration->name));
    if (res != VK_SUCCESS) {
        goto out;
    }

    res = loader_parse_json_string(layer_configuration_json, "path", &(layer_configuration->path));
    if (res != VK_SUCCESS) {
        goto out;
    }

    cJSON* treat_as_implicit_manifest = loader_cJSON_GetObjectItem(layer_configuration_json, "treat_as_implicit_manifest");
    if (treat_as_implicit_manifest && treat_as_implicit_manifest->type == cJSON_True) {
        layer_configuration->treat_as_implicit_manifest = true;
    }
out:
    if (VK_SUCCESS != res) {
        free_layer_configuration(inst, layer_configuration);
    }
    return res;
}

VkResult parse_layer_configurations(const struct loader_instance* inst, cJSON* settings_object, loader_settings* loader_settings) {
    VkResult res = VK_SUCCESS;

    cJSON* layer_configurations = loader_cJSON_GetObjectItem(settings_object, "layers");
    // If the layers object isn't present, return early with success to allow the settings file to still apply
    if (NULL == layer_configurations) {
        return VK_SUCCESS;
    }

    uint32_t layer_configurations_count = loader_cJSON_GetArraySize(layer_configurations);
    if (layer_configurations_count == 0) {
        return VK_SUCCESS;
    }

    loader_settings->layer_configuration_count = layer_configurations_count;

    loader_settings->layer_configurations = loader_instance_heap_calloc(
        inst, sizeof(loader_settings_layer_configuration) * layer_configurations_count, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (NULL == loader_settings->layer_configurations) {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    cJSON* layer = NULL;
    size_t i = 0;
    cJSON_ArrayForEach(layer, layer_configurations) {
        if (layer->type != cJSON_Object) {
            res = VK_ERROR_INITIALIZATION_FAILED;
            goto out;
        }
        res = parse_layer_configuration(inst, layer, &(loader_settings->layer_configurations[i++]));
        if (VK_SUCCESS != res) {
            goto out;
        }
    }
out:
    if (res != VK_SUCCESS) {
        if (loader_settings->layer_configurations) {
            for (size_t index = 0; index < loader_settings->layer_configuration_count; index++) {
                free_layer_configuration(inst, &(loader_settings->layer_configurations[index]));
            }
            loader_settings->layer_configuration_count = 0;
            loader_instance_heap_free(inst, loader_settings->layer_configurations);
            loader_settings->layer_configurations = NULL;
        }
    }

    return res;
}

VkResult check_if_settings_path_exists(const struct loader_instance* inst, const char* base, const char* suffix,
                                       char** settings_file_path) {
    if (NULL == base || NULL == suffix) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    size_t base_len = strlen(base);
    size_t suffix_len = strlen(suffix);
    size_t path_len = base_len + suffix_len + 1;
    *settings_file_path = loader_instance_heap_calloc(inst, path_len, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (NULL == *settings_file_path) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    loader_strncpy(*settings_file_path, path_len, base, base_len);
    loader_strncat(*settings_file_path, path_len, suffix, suffix_len);

    if (!loader_platform_file_exists(*settings_file_path)) {
        loader_instance_heap_free(inst, *settings_file_path);
        *settings_file_path = NULL;
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}
VkResult get_unix_settings_path(const struct loader_instance* inst, char** settings_file_path) {
    VkResult res =
        check_if_settings_path_exists(inst, loader_secure_getenv("HOME", inst),
                                      "/.local/share/vulkan/loader_settings.d/" VK_LOADER_SETTINGS_FILENAME, settings_file_path);
    if (res == VK_SUCCESS) {
        return res;
    }
    // If HOME isn't set, fallback to XDG_DATA_HOME
    res = check_if_settings_path_exists(inst, loader_secure_getenv("XDG_DATA_HOME", inst),
                                        "/vulkan/loader_settings.d/" VK_LOADER_SETTINGS_FILENAME, settings_file_path);
    if (res == VK_SUCCESS) {
        return res;
    }
    // if XDG_DATA_HOME isn't set, fallback to /etc.
    // note that the settings_fil_path_suffix stays the same since its the same layout as for XDG_DATA_HOME
    return check_if_settings_path_exists(inst, "/etc", "/vulkan/loader_settings.d/" VK_LOADER_SETTINGS_FILENAME,
                                         settings_file_path);
}

bool check_if_settings_are_equal(loader_settings* a, loader_settings* b) {
    // If either pointer is null, return true
    if (NULL == a || NULL == b) return false;
    bool are_equal = true;
    are_equal &= a->settings_active == b->settings_active;
    are_equal &= a->has_unordered_layer_location == b->has_unordered_layer_location;
    are_equal &= a->debug_level == b->debug_level;
    are_equal &= a->layer_configuration_count == b->layer_configuration_count;
    if (!are_equal) return false;
    for (uint32_t i = 0; i < a->layer_configuration_count && i < b->layer_configuration_count; i++) {
        if (a->layer_configurations[i].name && b->layer_configurations[i].name) {
            are_equal &= 0 == strcmp(a->layer_configurations[i].name, b->layer_configurations[i].name);
        } else {
            are_equal = false;
        }
        if (a->layer_configurations[i].path && b->layer_configurations[i].path) {
            are_equal &= 0 == strcmp(a->layer_configurations[i].path, b->layer_configurations[i].path);
        } else {
            are_equal = false;
        }
        are_equal &= a->layer_configurations[i].control == b->layer_configurations[i].control;
    }
    return are_equal;
}

void log_settings(const struct loader_instance* inst, loader_settings* settings) {
    if (settings == NULL) {
        return;
    }
    loader_log(inst, VULKAN_LOADER_INFO_BIT, 0, "Using layer configurations found in loader settings from %s",
               settings->settings_file_path);

    char cmd_line_msg[64] = {0};
    size_t cmd_line_size = sizeof(cmd_line_msg);

    cmd_line_msg[0] = '\0';

    generate_debug_flag_str(settings->debug_level, cmd_line_size, cmd_line_msg);
    if (strlen(cmd_line_msg)) {
        loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Loader Settings Filters for Logging to Standard Error: %s", cmd_line_msg);
    }

    loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Layer Configurations count = %d", settings->layer_configuration_count);
    for (uint32_t i = 0; i < settings->layer_configuration_count; i++) {
        loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "---- Layer Configuration [%d] ----", i);
        if (settings->layer_configurations[i].control != LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Name: %s", settings->layer_configurations[i].name);
            loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Path: %s", settings->layer_configurations[i].path);
            loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Layer Type: %s",
                       settings->layer_configurations[i].treat_as_implicit_manifest ? "Implicit" : "Explicit");
        }
        loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "Control: %s",
                   loader_settings_layer_control_to_string(settings->layer_configurations[i].control));
    }
    loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0, "---------------------------------");
}

// Loads the vk_loader_settings.json file
// Returns VK_SUCCESS if it was found & was successfully parsed. Otherwise, it returns VK_ERROR_INITIALIZATION_FAILED if it
// wasn't found or failed to parse, and returns VK_ERROR_OUT_OF_HOST_MEMORY if it was unable to allocate enough memory.
VkResult get_loader_settings(const struct loader_instance* inst, loader_settings* loader_settings) {
    VkResult res = VK_SUCCESS;
    cJSON* json = NULL;
    char* file_format_version_string = NULL;
    char* settings_file_path = NULL;
#if defined(WIN32)
    res = windows_get_loader_settings_file_path(inst, &settings_file_path);
    if (res != VK_SUCCESS) {
        loader_log(inst, VULKAN_LOADER_INFO_BIT, 0,
                   "No valid vk_loader_settings.json file found, no loader settings will be active");
        goto out;
    }

#elif COMMON_UNIX_PLATFORMS
    res = get_unix_settings_path(inst, &settings_file_path);
    if (res != VK_SUCCESS) {
        loader_log(inst, VULKAN_LOADER_INFO_BIT, 0,
                   "No valid vk_loader_settings.json file found, no loader settings will be active");
        goto out;
    }
#else
#warning "Unsupported platform - must specify platform specific location for vk_loader_settings.json"
#endif

    res = loader_get_json(inst, settings_file_path, &json);
    // Make sure sure the top level json value is an object
    if (res != VK_SUCCESS || NULL == json || json->type != cJSON_Object) {
        goto out;
    }

    res = loader_parse_json_string(json, "file_format_version", &file_format_version_string);
    if (res != VK_SUCCESS) {
        if (res != VK_ERROR_OUT_OF_HOST_MEMORY) {
            loader_log(
                inst, VULKAN_LOADER_DEBUG_BIT, 0,
                "Loader settings file from %s missing required field file_format_version - no loader settings will be active",
                settings_file_path);
        }
        goto out;
    }

    // Because the file may contain either a "settings_array" or a single "settings" object, we need to create a cJSON so that we
    // can iterate on both cases with common code
    cJSON settings_iter_parent = {0};

    cJSON* settings_array = loader_cJSON_GetObjectItem(json, "settings_array");
    cJSON* single_settings_object = loader_cJSON_GetObjectItem(json, "settings");
    if (NULL != settings_array) {
        memcpy(&settings_iter_parent, settings_array, sizeof(cJSON));
    } else if (NULL != single_settings_object) {
        settings_iter_parent.child = single_settings_object;
    } else if (settings_array == NULL && single_settings_object) {
        loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0,
                   "Loader settings file from %s missing required settings objects: Either one of the \"settings\" or "
                   "\"settings_array\" objects must be present - no loader settings will be active",
                   settings_file_path);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }

    // Corresponds to the settings object that has no app keys
    cJSON* global_settings = NULL;
    // Corresponds to the settings object which has a matching app key
    cJSON* settings_to_use = NULL;

    char current_process_path[1024];
    bool valid_exe_path = NULL != loader_platform_executable_path(current_process_path, 1024);

    cJSON* settings_object_iter = NULL;
    cJSON_ArrayForEach(settings_object_iter, &settings_iter_parent) {
        if (settings_object_iter->type != cJSON_Object) {
            loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0,
                       "Loader settings file from %s has a settings element that is not an object", settings_file_path);
            break;
        }

        cJSON* app_keys = loader_cJSON_GetObjectItem(settings_object_iter, "app_keys");
        if (NULL == app_keys) {
            // use the first 'global' settings that has no app keys as the global one
            if (global_settings == NULL) {
                global_settings = settings_object_iter;
            }
            continue;
        }
        // No sense iterating if we couldn't get the executable path
        if (!valid_exe_path) {
            break;
        }
        cJSON* app_key = NULL;
        cJSON_ArrayForEach(app_key, app_keys) {
            char* app_key_str = loader_cJSON_GetStringValue(app_key);
            if (app_key_str && strcmp(current_process_path, app_key_str) == 0) {
                settings_to_use = settings_object_iter;
                break;
            }
        }

        // Break if we have found a matching current_process_path
        if (NULL != settings_to_use) {
            break;
        }
    }

    // No app specific settings match - either use global settings or exit
    if (settings_to_use == NULL) {
        if (global_settings == NULL) {
            loader_log(inst, VULKAN_LOADER_DEBUG_BIT, 0,
                       "Loader settings file from %s missing global settings and none of the app specific settings matched the "
                       "current application - no loader settings will be active",
                       settings_file_path);
            goto out;  // No global settings were found - exit
        } else {
            settings_to_use = global_settings;  // Global settings are present - use it
        }
    }

    // optional
    cJSON* stderr_filter = loader_cJSON_GetObjectItem(settings_to_use, "stderr_log");
    if (NULL != stderr_filter) {
        struct loader_string_list stderr_log = {0};
        res = loader_parse_json_array_of_strings(inst, settings_to_use, "stderr_log", &stderr_log);
        if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
            goto out;
        }
        loader_settings->debug_level = parse_log_filters_from_strings(&stderr_log);
        free_string_list(inst, &stderr_log);
    }

    // optional
    cJSON* logs_to_use = loader_cJSON_GetObjectItem(settings_to_use, "log_locations");
    if (NULL != logs_to_use) {
        cJSON* log_element = NULL;
        cJSON_ArrayForEach(log_element, logs_to_use) {
            // bool is_valid = true;
            struct loader_string_list log_destinations = {0};
            res = loader_parse_json_array_of_strings(inst, log_element, "destinations", &log_destinations);
            if (res != VK_SUCCESS) {
                // is_valid = false;
            }
            free_string_list(inst, &log_destinations);
            struct loader_string_list log_filters = {0};
            res = loader_parse_json_array_of_strings(inst, log_element, "filters", &log_filters);
            if (res != VK_SUCCESS) {
                // is_valid = false;
            }
            free_string_list(inst, &log_filters);
        }
    }

    res = parse_layer_configurations(inst, settings_to_use, loader_settings);
    if (res != VK_SUCCESS) {
        goto out;
    }

    // Determine if there exists a layer configuration indicating where to put layers not contained in the settings file
    // LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION
    for (uint32_t i = 0; i < loader_settings->layer_configuration_count; i++) {
        if (loader_settings->layer_configurations[i].control == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            loader_settings->has_unordered_layer_location = true;
            break;
        }
    }

    loader_settings->settings_file_path = settings_file_path;
    settings_file_path = NULL;
    loader_settings->settings_active = true;
out:
    if (NULL != json) {
        loader_cJSON_Delete(json);
    }

    loader_instance_heap_free(inst, settings_file_path);

    loader_instance_heap_free(inst, file_format_version_string);
    return res;
}

TEST_FUNCTION_EXPORT VkResult update_global_loader_settings(void) {
    loader_settings settings = {0};
    VkResult res = get_loader_settings(NULL, &settings);
    loader_platform_thread_lock_mutex(&global_loader_settings_lock);

    free_loader_settings(NULL, &global_loader_settings);
    if (res == VK_SUCCESS) {
        if (!check_if_settings_are_equal(&settings, &global_loader_settings)) {
            log_settings(NULL, &settings);
        }

        memcpy(&global_loader_settings, &settings, sizeof(loader_settings));
        if (global_loader_settings.settings_active && global_loader_settings.debug_level > 0) {
            loader_set_global_debug_level(global_loader_settings.debug_level);
        }
    }
    loader_platform_thread_unlock_mutex(&global_loader_settings_lock);
    return res;
}

void init_global_loader_settings(void) {
    loader_platform_thread_create_mutex(&global_loader_settings_lock);
    // Free out the global settings in case the process was loaded & unloaded
    free_loader_settings(NULL, &global_loader_settings);
}
void teardown_global_loader_settings(void) {
    free_loader_settings(NULL, &global_loader_settings);
    loader_platform_thread_delete_mutex(&global_loader_settings_lock);
}

bool should_skip_logging_global_messages(VkFlags msg_type) {
    loader_platform_thread_lock_mutex(&global_loader_settings_lock);
    bool should_skip = global_loader_settings.settings_active && 0 != (msg_type & global_loader_settings.debug_level);
    loader_platform_thread_unlock_mutex(&global_loader_settings_lock);
    return should_skip;
}

// Use this function to get the correct settings to use based on the context
// If inst is NULL - use the global settings and lock the mutex
// Else return the settings local to the instance - but do nto lock the mutex
const loader_settings* get_current_settings_and_lock(const struct loader_instance* inst) {
    if (inst) {
        return &inst->settings;
    }
    loader_platform_thread_lock_mutex(&global_loader_settings_lock);
    return &global_loader_settings;
}
// Release the global settings lock if we are using the global settings - aka if inst is NULL
void release_current_settings_lock(const struct loader_instance* inst) {
    if (inst == NULL) {
        loader_platform_thread_unlock_mutex(&global_loader_settings_lock);
    }
}

TEST_FUNCTION_EXPORT VkResult get_settings_layers(const struct loader_instance* inst, struct loader_layer_list* settings_layers,
                                                  bool* should_search_for_other_layers) {
    VkResult res = VK_SUCCESS;
    *should_search_for_other_layers = true;  // default to true

    const loader_settings* settings = get_current_settings_and_lock(inst);

    if (NULL == settings || !settings->settings_active) {
        goto out;
    }

    // Assume the list doesn't contain LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION at first
    *should_search_for_other_layers = false;

    for (uint32_t i = 0; i < settings->layer_configuration_count; i++) {
        loader_settings_layer_configuration* layer_config = &settings->layer_configurations[i];

        // If we encountered a layer that should be forced off, we add it to the settings_layers list but only
        // with the data required to compare it with layers not in the settings file (aka name and manifest path)
        if (layer_config->control == LOADER_SETTINGS_LAYER_CONTROL_OFF) {
            struct loader_layer_properties props = {0};
            props.settings_control_value = LOADER_SETTINGS_LAYER_CONTROL_OFF;
            loader_strncpy(props.info.layerName, VK_MAX_EXTENSION_NAME_SIZE, layer_config->name, VK_MAX_EXTENSION_NAME_SIZE);
            props.info.layerName[VK_MAX_EXTENSION_NAME_SIZE - 1] = '\0';
            res = loader_copy_to_new_str(inst, layer_config->path, &props.manifest_file_name);
            if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
                goto out;
            }
            res = loader_append_layer_property(inst, settings_layers, &props);
            if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
                loader_free_layer_properties(inst, &props);
                goto out;
            }
            continue;
        }

        // The special layer location that indicates where unordered layers should go only should have the
        // settings_control_value set - everything else should be NULL
        if (layer_config->control == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            struct loader_layer_properties props = {0};
            props.settings_control_value = LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION;
            res = loader_append_layer_property(inst, settings_layers, &props);
            if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
                loader_free_layer_properties(inst, &props);
                goto out;
            }
            *should_search_for_other_layers = true;
            continue;
        }

        if (layer_config->path == NULL) {
            continue;
        }

        cJSON* json = NULL;
        VkResult local_res = loader_get_json(inst, layer_config->path, &json);
        if (VK_ERROR_OUT_OF_HOST_MEMORY == local_res) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto out;
        } else if (VK_SUCCESS != local_res || NULL == json) {
            continue;
        }

        // Makes it possible to know if a new layer was added or not, since the only return value is VkResult
        size_t count_before_adding = settings_layers->count;

        local_res =
            loader_add_layer_properties(inst, settings_layers, json, layer_config->treat_as_implicit_manifest, layer_config->path);
        loader_cJSON_Delete(json);

        // If the error is anything other than out of memory we still want to try to load the other layers
        if (VK_ERROR_OUT_OF_HOST_MEMORY == local_res) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto out;
        } else if (local_res != VK_SUCCESS || count_before_adding == settings_layers->count) {
            // Indicates something was wrong with the layer, can't add it to the list
            continue;
        }

        struct loader_layer_properties* newly_added_layer = &settings_layers->list[settings_layers->count - 1];
        newly_added_layer->settings_control_value = layer_config->control;
        // If the manifest file found has a name that differs from the one in the settings, remove this layer from
        // consideration
        bool should_remove = false;
        if (strncmp(newly_added_layer->info.layerName, layer_config->name, VK_MAX_EXTENSION_NAME_SIZE) != 0) {
            should_remove = true;
            loader_remove_layer_in_list(inst, settings_layers, settings_layers->count - 1);
        }
        // Make sure the layer isn't already in the list
        for (uint32_t j = 0; settings_layers->count > 0 && j < settings_layers->count - 1; j++) {
            if (0 ==
                strncmp(settings_layers->list[j].info.layerName, newly_added_layer->info.layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
                if (0 == (newly_added_layer->type_flags & VK_LAYER_TYPE_FLAG_META_LAYER) &&
                    strcmp(settings_layers->list[j].lib_name, newly_added_layer->lib_name) == 0) {
                    should_remove = true;
                    break;
                }
            }
        }
        if (should_remove) {
            loader_remove_layer_in_list(inst, settings_layers, settings_layers->count - 1);
        }
    }

out:
    release_current_settings_lock(inst);
    return res;
}

// Check if layers has an element with the same name.
// LAYER_CONTROL_OFF layers are missing some fields, just make sure the layerName is the same
// If layer_property is a meta layer, just make sure the layerName is the same
// Skip comparing to UNORDERED_LAYER_LOCATION
// If layer_property is a regular layer, check if the lib_path is the same.
// Make sure that the lib_name pointers are non-null before calling strcmp.
bool check_if_layer_is_in_list(struct loader_layer_list* layer_list, struct loader_layer_properties* layer_property) {
    // If the layer is a meta layer, just check against the name
    for (uint32_t i = 0; i < layer_list->count; i++) {
        if (0 == strncmp(layer_list->list[i].info.layerName, layer_property->info.layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
            if (layer_list->list[i].settings_control_value == LOADER_SETTINGS_LAYER_CONTROL_OFF) {
                return true;
            }
            if (VK_LAYER_TYPE_FLAG_META_LAYER == (layer_property->type_flags & VK_LAYER_TYPE_FLAG_META_LAYER)) {
                return true;
            }
            if (layer_list->list[i].lib_name && layer_property->lib_name) {
                return strcmp(layer_list->list[i].lib_name, layer_property->lib_name) == 0;
            }
        }
    }
    return false;
}

VkResult combine_settings_layers_with_regular_layers(const struct loader_instance* inst, struct loader_layer_list* settings_layers,
                                                     struct loader_layer_list* regular_layers,
                                                     struct loader_layer_list* output_layers) {
    VkResult res = VK_SUCCESS;
    bool has_unordered_layer_location = false;
    uint32_t unordered_layer_location_index = 0;
    // Location to put layers that aren't known to the settings file
    // Find it here so we dont have to pass in a loader_settings struct
    for (uint32_t i = 0; i < settings_layers->count; i++) {
        if (settings_layers->list[i].settings_control_value == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            has_unordered_layer_location = true;
            unordered_layer_location_index = i;
            break;
        }
    }

    if (settings_layers->count == 0 && regular_layers->count == 0) {
        // No layers to combine
        goto out;
    } else if (settings_layers->count == 0) {
        // No settings layers - just copy regular to output_layers - memset regular layers to prevent double frees
        *output_layers = *regular_layers;
        memset(regular_layers, 0, sizeof(struct loader_layer_list));
        goto out;
    } else if (regular_layers->count == 0 || !has_unordered_layer_location) {
        // No regular layers or has_unordered_layer_location is false - just copy settings to output_layers -
        // memset settings layers to prevent double frees
        *output_layers = *settings_layers;
        memset(settings_layers, 0, sizeof(struct loader_layer_list));
        goto out;
    }

    res = loader_init_generic_list(inst, (struct loader_generic_list*)output_layers,
                                   (settings_layers->count + regular_layers->count) * sizeof(struct loader_layer_properties));
    if (VK_SUCCESS != res) {
        goto out;
    }

    // Insert the settings layers into output_layers up to unordered_layer_index
    for (uint32_t i = 0; i < unordered_layer_location_index; i++) {
        if (!check_if_layer_is_in_list(output_layers, &settings_layers->list[i])) {
            res = loader_append_layer_property(inst, output_layers, &settings_layers->list[i]);
            if (VK_SUCCESS != res) {
                goto out;
            }
        }
    }

    for (uint32_t i = 0; i < regular_layers->count; i++) {
        // Check if its already been put in the output_layers list as well as the remaining settings_layers
        bool regular_layer_is_ordered = check_if_layer_is_in_list(output_layers, &regular_layers->list[i]) ||
                                        check_if_layer_is_in_list(settings_layers, &regular_layers->list[i]);
        // If it isn't found, add it
        if (!regular_layer_is_ordered) {
            res = loader_append_layer_property(inst, output_layers, &regular_layers->list[i]);
            if (VK_SUCCESS != res) {
                goto out;
            }
        } else {
            // layer is already ordered and can be safely freed
            loader_free_layer_properties(inst, &regular_layers->list[i]);
        }
    }

    // Insert the rest of the settings layers into combined_layers from  unordered_layer_index to the end
    // start at one after the unordered_layer_index
    for (uint32_t i = unordered_layer_location_index + 1; i < settings_layers->count; i++) {
        res = loader_append_layer_property(inst, output_layers, &settings_layers->list[i]);
        if (VK_SUCCESS != res) {
            goto out;
        }
    }

out:
    if (res != VK_SUCCESS) {
        loader_delete_layer_list_and_properties(inst, output_layers);
    }

    return res;
}

VkResult enable_correct_layers_from_settings(const struct loader_instance* inst, const struct loader_envvar_all_filters* filters,
                                             uint32_t app_enabled_name_count, const char* const* app_enabled_names,
                                             const struct loader_layer_list* instance_layers,
                                             struct loader_pointer_layer_list* target_layer_list,
                                             struct loader_pointer_layer_list* activated_layer_list) {
    VkResult res = VK_SUCCESS;
    char* vk_instance_layers_env = loader_getenv(ENABLED_LAYERS_ENV, inst);
    size_t vk_instance_layers_env_len = 0;
    char* vk_instance_layers_env_copy = NULL;
    if (vk_instance_layers_env != NULL) {
        vk_instance_layers_env_len = strlen(vk_instance_layers_env) + 1;
        vk_instance_layers_env_copy = loader_stack_alloc(vk_instance_layers_env_len);
        memset(vk_instance_layers_env_copy, 0, vk_instance_layers_env_len);

        loader_log(inst, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_LAYER_BIT, 0, "env var \'%s\' defined and adding layers: %s",
                   ENABLED_LAYERS_ENV, vk_instance_layers_env);
    }
    for (uint32_t i = 0; i < instance_layers->count; i++) {
        bool enable_layer = false;
        struct loader_layer_properties* props = &instance_layers->list[i];

        // Skip the sentinel unordered layer location
        if (props->settings_control_value == LOADER_SETTINGS_LAYER_UNORDERED_LAYER_LOCATION) {
            continue;
        }

        // Do not enable the layer if the settings have it set as off
        if (props->settings_control_value == LOADER_SETTINGS_LAYER_CONTROL_OFF) {
            continue;
        }
        // Force enable it based on settings
        if (props->settings_control_value == LOADER_SETTINGS_LAYER_CONTROL_ON) {
            enable_layer = true;
            props->enabled_by_what = ENABLED_BY_WHAT_LOADER_SETTINGS_FILE;
        } else {
            // Check if disable filter needs to skip the layer
            if ((filters->disable_filter.disable_all || filters->disable_filter.disable_all_implicit ||
                 check_name_matches_filter_environment_var(props->info.layerName, &filters->disable_filter.additional_filters)) &&
                !check_name_matches_filter_environment_var(props->info.layerName, &filters->allow_filter)) {
                // Report a message that we've forced off a layer if it would have been enabled normally.
                loader_log(inst, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_LAYER_BIT, 0,
                           "Layer \"%s\" forced disabled because name matches filter of env var \'%s\'.", props->info.layerName,
                           VK_LAYERS_DISABLE_ENV_VAR);

                continue;
            }
        }
        // Check the enable filter
        if (!enable_layer && check_name_matches_filter_environment_var(props->info.layerName, &filters->enable_filter)) {
            enable_layer = true;
            props->enabled_by_what = ENABLED_BY_WHAT_VK_LOADER_LAYERS_ENABLE;
            loader_log(inst, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_LAYER_BIT, 0,
                       "Layer \"%s\" forced enabled due to env var \'%s\'.", props->info.layerName, VK_LAYERS_ENABLE_ENV_VAR);
        }

        // First look for the old-fashion layers forced on with VK_INSTANCE_LAYERS
        if (!enable_layer && vk_instance_layers_env && vk_instance_layers_env_copy && vk_instance_layers_env_len > 0) {
            // Copy the env-var on each iteration, so that loader_get_next_path can correctly find the separators
            // This solution only needs one stack allocation ahead of time rather than an allocation per layer in the
            // env-var
            loader_strncpy(vk_instance_layers_env_copy, vk_instance_layers_env_len, vk_instance_layers_env,
                           vk_instance_layers_env_len);

            char* instance_layers_env_iter = vk_instance_layers_env_copy;
            while (instance_layers_env_iter && *instance_layers_env_iter) {
                char* next = loader_get_next_path(instance_layers_env_iter);
                if (0 == strcmp(instance_layers_env_iter, props->info.layerName)) {
                    enable_layer = true;
                    props->enabled_by_what = ENABLED_BY_WHAT_VK_INSTANCE_LAYERS;
                    break;
                }
                instance_layers_env_iter = next;
            }
        }

        // Check if it should be enabled by the application
        if (!enable_layer) {
            for (uint32_t j = 0; j < app_enabled_name_count; j++) {
                if (strcmp(props->info.layerName, app_enabled_names[j]) == 0) {
                    enable_layer = true;
                    props->enabled_by_what = ENABLED_BY_WHAT_IN_APPLICATION_API;
                    break;
                }
            }
        }

        // Check if its an implicit layers and thus enabled by default
        if (!enable_layer && (0 == (props->type_flags & VK_LAYER_TYPE_FLAG_EXPLICIT_LAYER)) &&
            loader_implicit_layer_is_enabled(inst, filters, props)) {
            enable_layer = true;
            props->enabled_by_what = ENABLED_BY_WHAT_IMPLICIT_LAYER;
        }

        if (enable_layer) {
            // Check if the layer is a meta layer reuse the existing function to add the meta layer
            if (props->type_flags & VK_LAYER_TYPE_FLAG_META_LAYER) {
                res = loader_add_meta_layer(inst, filters, props, target_layer_list, activated_layer_list, instance_layers, NULL);
                if (res == VK_ERROR_OUT_OF_HOST_MEMORY) goto out;
            } else {
                res = loader_add_layer_properties_to_list(inst, target_layer_list, props);
                if (res != VK_SUCCESS) {
                    goto out;
                }
                res = loader_add_layer_properties_to_list(inst, activated_layer_list, props);
                if (res != VK_SUCCESS) {
                    goto out;
                }
            }
        }
    }
out:
    return res;
}
