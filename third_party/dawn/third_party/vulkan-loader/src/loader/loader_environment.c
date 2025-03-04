/*
 *
 * Copyright (c) 2014-2023 The Khronos Group Inc.
 * Copyright (c) 2014-2023 Valve Corporation
 * Copyright (c) 2014-2023 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chia-I Wu <olv@lunarg.com>
 * Author: Mark Lobodzinski <mark@LunarG.com>
 * Author: Lenny Komow <lenny@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

#include "loader_environment.h"

#include "allocation.h"
#include "loader.h"
#include "log.h"
#include "stack_allocation.h"

#include <ctype.h>

// Environment variables
#if COMMON_UNIX_PLATFORMS

bool is_high_integrity() { return geteuid() != getuid() || getegid() != getgid(); }

char *loader_getenv(const char *name, const struct loader_instance *inst) {
    if (NULL == name) return NULL;
    // No allocation of memory necessary for Linux, but we should at least touch
    // the inst pointer to get rid of compiler warnings.
    (void)inst;
    return getenv(name);
}

char *loader_secure_getenv(const char *name, const struct loader_instance *inst) {
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    // Apple does not appear to have a secure getenv implementation.
    // The main difference between secure getenv and getenv is that secure getenv
    // returns NULL if the process is being run with elevated privileges by a normal user.
    // The idea is to prevent the reading of malicious environment variables by a process
    // that can do damage.
    // This algorithm is derived from glibc code that sets an internal
    // variable (__libc_enable_secure) if the process is running under setuid or setgid.
    return is_high_integrity() ? NULL : loader_getenv(name, inst);
#elif defined(__Fuchsia__)
    return loader_getenv(name, inst);
#else
    // Linux
    char *out;
#if defined(HAVE_SECURE_GETENV) && !defined(LOADER_USE_UNSAFE_FILE_SEARCH)
    (void)inst;
    out = secure_getenv(name);
#elif defined(HAVE___SECURE_GETENV) && !defined(LOADER_USE_UNSAFE_FILE_SEARCH)
    (void)inst;
    out = __secure_getenv(name);
#else
    out = loader_getenv(name, inst);
#if !defined(LOADER_USE_UNSAFE_FILE_SEARCH)
    loader_log(inst, VULKAN_LOADER_INFO_BIT, 0, "Loader is using non-secure environment variable lookup for %s", name);
#endif
#endif
    return out;
#endif
}

void loader_free_getenv(char *val, const struct loader_instance *inst) {
    // No freeing of memory necessary for Linux, but we should at least touch
    // the val and inst pointers to get rid of compiler warnings.
    (void)val;
    (void)inst;
}

#elif defined(WIN32)

bool is_high_integrity() {
    HANDLE process_token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &process_token)) {
        // Maximum possible size of SID_AND_ATTRIBUTES is maximum size of a SID + size of attributes DWORD.
        uint8_t mandatory_label_buffer[SECURITY_MAX_SID_SIZE + sizeof(DWORD)];
        DWORD buffer_size;
        if (GetTokenInformation(process_token, TokenIntegrityLevel, mandatory_label_buffer, sizeof(mandatory_label_buffer),
                                &buffer_size) != 0) {
            const TOKEN_MANDATORY_LABEL *mandatory_label = (const TOKEN_MANDATORY_LABEL *)mandatory_label_buffer;
            const DWORD sub_authority_count = *GetSidSubAuthorityCount(mandatory_label->Label.Sid);
            const DWORD integrity_level = *GetSidSubAuthority(mandatory_label->Label.Sid, sub_authority_count - 1);

            CloseHandle(process_token);
            return integrity_level >= SECURITY_MANDATORY_HIGH_RID;
        }

        CloseHandle(process_token);
    }

    return false;
}

char *loader_getenv(const char *name, const struct loader_instance *inst) {
    int name_utf16_size = MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
    if (name_utf16_size <= 0) {
        return NULL;
    }
    wchar_t *name_utf16 = (wchar_t *)loader_stack_alloc(name_utf16_size * sizeof(wchar_t));
    if (MultiByteToWideChar(CP_UTF8, 0, name, -1, name_utf16, name_utf16_size) != name_utf16_size) {
        return NULL;
    }

    DWORD val_size = GetEnvironmentVariableW(name_utf16, NULL, 0);
    // val_size DOES include the null terminator, so for any set variable
    // will always be at least 1. If it's 0, the variable wasn't set.
    if (val_size == 0) {
        return NULL;
    }

    wchar_t *val = (wchar_t *)loader_stack_alloc(val_size * sizeof(wchar_t));
    if (GetEnvironmentVariableW(name_utf16, val, val_size) != val_size - 1) {
        return NULL;
    }

    int val_utf8_size = WideCharToMultiByte(CP_UTF8, 0, val, -1, NULL, 0, NULL, NULL);
    if (val_utf8_size <= 0) {
        return NULL;
    }
    char *val_utf8 = (char *)loader_instance_heap_alloc(inst, val_utf8_size * sizeof(char), VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (val_utf8 == NULL) {
        return NULL;
    }
    if (WideCharToMultiByte(CP_UTF8, 0, val, -1, val_utf8, val_utf8_size, NULL, NULL) != val_utf8_size) {
        loader_instance_heap_free(inst, val_utf8);
        return NULL;
    }
    return val_utf8;
}

char *loader_secure_getenv(const char *name, const struct loader_instance *inst) {
    if (NULL == name) return NULL;
#if !defined(LOADER_USE_UNSAFE_FILE_SEARCH)
    if (is_high_integrity()) {
        loader_log(inst, VULKAN_LOADER_INFO_BIT, 0,
                   "Loader is running with elevated permissions. Environment variable %s will be ignored", name);
        return NULL;
    }
#endif

    return loader_getenv(name, inst);
}

void loader_free_getenv(char *val, const struct loader_instance *inst) { loader_instance_heap_free(inst, (void *)val); }

#else

#warning \
    "This platform does not support environment variables! If this is not intended, please implement the stubs functions loader_getenv and loader_free_getenv"

char *loader_getenv(const char *name, const struct loader_instance *inst) {
    // stub func
    (void)inst;
    (void)name;
    return NULL;
}
void loader_free_getenv(char *val, const struct loader_instance *inst) {
    // stub func
    (void)val;
    (void)inst;
}

#endif

// Determine the type of filter string based on the contents of it.
// This will properly check against:
//  - substrings "*string*"
//  - prefixes "string*"
//  - suffixes "*string"
//  - full string names "string"
// It will also return the correct start and finish to remove any star '*' characters for the actual string compare
void determine_filter_type(const char *filter_string, enum loader_filter_string_type *filter_type, const char **new_start,
                           size_t *new_length) {
    size_t filter_length = strlen(filter_string);
    bool star_begin = false;
    bool star_end = false;
    if ('~' == filter_string[0]) {
        // One of the special identifiers like: ~all~, ~implicit~, or ~explicit~
        *filter_type = FILTER_STRING_SPECIAL;
        *new_start = filter_string;
        *new_length = filter_length;
    } else {
        if ('*' == filter_string[0]) {
            // Only the * means everything
            if (filter_length == 1) {
                *filter_type = FILTER_STRING_SPECIAL;
                *new_start = filter_string;
                *new_length = filter_length;
            } else {
                star_begin = true;
            }
        }
        if ('*' == filter_string[filter_length - 1]) {
            // Not really valid, but just catch this case so if someone accidentally types "**" it will also mean everything
            if (filter_length == 2) {
                *filter_type = FILTER_STRING_SPECIAL;
                *new_start = filter_string;
                *new_length = filter_length;
            } else {
                star_end = true;
            }
        }
        if (star_begin && star_end) {
            *filter_type = FILTER_STRING_SUBSTRING;
            *new_start = &filter_string[1];
            *new_length = filter_length - 2;
        } else if (star_begin) {
            *new_start = &filter_string[1];
            *new_length = filter_length - 1;
            *filter_type = FILTER_STRING_SUFFIX;
        } else if (star_end) {
            *filter_type = FILTER_STRING_PREFIX;
            *new_start = filter_string;
            *new_length = filter_length - 1;
        } else {
            *filter_type = FILTER_STRING_FULLNAME;
            *new_start = filter_string;
            *new_length = filter_length;
        }
    }
}

// Parse the provided filter string provided by the envrionment variable into the appropriate filter
// struct variable.
VkResult parse_generic_filter_environment_var(const struct loader_instance *inst, const char *env_var_name,
                                              struct loader_envvar_filter *filter_struct) {
    VkResult result = VK_SUCCESS;
    memset(filter_struct, 0, sizeof(struct loader_envvar_filter));
    char *parsing_string = NULL;
    char *env_var_value = loader_secure_getenv(env_var_name, inst);
    if (NULL == env_var_value) {
        return result;
    }
    const size_t env_var_len = strlen(env_var_value);
    if (env_var_len == 0) {
        goto out;
    }
    // Allocate a separate string since scan_for_next_comma modifies the original string
    parsing_string = loader_instance_heap_calloc(inst, env_var_len + 1, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (NULL == parsing_string) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "parse_generic_filter_environment_var: Failed to allocate space for parsing env var \'%s\'", env_var_name);
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    for (uint32_t iii = 0; iii < env_var_len; ++iii) {
        parsing_string[iii] = (char)tolower(env_var_value[iii]);
    }
    parsing_string[env_var_len] = '\0';

    char *context = NULL;
    char *token = thread_safe_strtok(parsing_string, ",", &context);
    while (NULL != token) {
        enum loader_filter_string_type cur_filter_type;
        const char *actual_start;
        size_t actual_len;
        determine_filter_type(token, &cur_filter_type, &actual_start, &actual_len);
        if (actual_len > VK_MAX_EXTENSION_NAME_SIZE) {
            loader_strncpy(filter_struct->filters[filter_struct->count].value, VK_MAX_EXTENSION_NAME_SIZE, actual_start,
                           VK_MAX_EXTENSION_NAME_SIZE);
        } else {
            loader_strncpy(filter_struct->filters[filter_struct->count].value, VK_MAX_EXTENSION_NAME_SIZE, actual_start,
                           actual_len);
        }
        filter_struct->filters[filter_struct->count].length = actual_len;
        filter_struct->filters[filter_struct->count++].type = cur_filter_type;
        if (filter_struct->count >= MAX_ADDITIONAL_FILTERS) {
            break;
        }
        token = thread_safe_strtok(NULL, ",", &context);
    }

out:

    loader_instance_heap_free(inst, parsing_string);
    loader_free_getenv(env_var_value, inst);
    return result;
}

// Parse the disable layer string.  The layer disable has some special behavior because we allow it to disable
// all layers (either with "~all~", "*", or "**"), all implicit layers (with "~implicit~"), and all explicit layers
// (with "~explicit~"), in addition to the other layer filtering behavior.
VkResult parse_layers_disable_filter_environment_var(const struct loader_instance *inst,
                                                     struct loader_envvar_disable_layers_filter *disable_struct) {
    VkResult result = VK_SUCCESS;
    memset(disable_struct, 0, sizeof(struct loader_envvar_disable_layers_filter));
    char *parsing_string = NULL;
    char *env_var_value = loader_secure_getenv(VK_LAYERS_DISABLE_ENV_VAR, inst);
    if (NULL == env_var_value) {
        goto out;
    }
    const size_t env_var_len = strlen(env_var_value);
    if (env_var_len == 0) {
        goto out;
    }
    // Allocate a separate string since scan_for_next_comma modifies the original string
    parsing_string = loader_instance_heap_calloc(inst, env_var_len + 1, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (NULL == parsing_string) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "parse_layers_disable_filter_environment_var: Failed to allocate space for parsing env var "
                   "\'VK_LAYERS_DISABLE_ENV_VAR\'");
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    for (uint32_t iii = 0; iii < env_var_len; ++iii) {
        parsing_string[iii] = (char)tolower(env_var_value[iii]);
    }
    parsing_string[env_var_len] = '\0';

    char *context = NULL;
    char *token = thread_safe_strtok(parsing_string, ",", &context);
    while (NULL != token) {
        uint32_t cur_count = disable_struct->additional_filters.count;
        enum loader_filter_string_type cur_filter_type;
        const char *actual_start;
        size_t actual_len;
        determine_filter_type(token, &cur_filter_type, &actual_start, &actual_len);
        if (cur_filter_type == FILTER_STRING_SPECIAL) {
            if (!strcmp(VK_LOADER_DISABLE_ALL_LAYERS_VAR_1, token) || !strcmp(VK_LOADER_DISABLE_ALL_LAYERS_VAR_2, token) ||
                !strcmp(VK_LOADER_DISABLE_ALL_LAYERS_VAR_3, token)) {
                disable_struct->disable_all = true;
            } else if (!strcmp(VK_LOADER_DISABLE_IMPLICIT_LAYERS_VAR, token)) {
                disable_struct->disable_all_implicit = true;
            } else if (!strcmp(VK_LOADER_DISABLE_EXPLICIT_LAYERS_VAR, token)) {
                disable_struct->disable_all_explicit = true;
            }
        } else {
            if (actual_len > VK_MAX_EXTENSION_NAME_SIZE) {
                loader_strncpy(disable_struct->additional_filters.filters[cur_count].value, VK_MAX_EXTENSION_NAME_SIZE,
                               actual_start, VK_MAX_EXTENSION_NAME_SIZE);
            } else {
                loader_strncpy(disable_struct->additional_filters.filters[cur_count].value, VK_MAX_EXTENSION_NAME_SIZE,
                               actual_start, actual_len);
            }
            disable_struct->additional_filters.filters[cur_count].length = actual_len;
            disable_struct->additional_filters.filters[cur_count].type = cur_filter_type;
            disable_struct->additional_filters.count++;
            if (disable_struct->additional_filters.count >= MAX_ADDITIONAL_FILTERS) {
                break;
            }
        }
        token = thread_safe_strtok(NULL, ",", &context);
    }
out:
    loader_instance_heap_free(inst, parsing_string);
    loader_free_getenv(env_var_value, inst);
    return result;
}

// Parses the filter environment variables to determine if we have any special behavior
VkResult parse_layer_environment_var_filters(const struct loader_instance *inst, struct loader_envvar_all_filters *layer_filters) {
    VkResult res = parse_generic_filter_environment_var(inst, VK_LAYERS_ENABLE_ENV_VAR, &layer_filters->enable_filter);
    if (VK_SUCCESS != res) {
        return res;
    }
    res = parse_layers_disable_filter_environment_var(inst, &layer_filters->disable_filter);
    if (VK_SUCCESS != res) {
        return res;
    }
    res = parse_generic_filter_environment_var(inst, VK_LAYERS_ALLOW_ENV_VAR, &layer_filters->allow_filter);
    if (VK_SUCCESS != res) {
        return res;
    }
    return res;
}

// Check to see if the provided layer name matches any of the filter strings.
// This will properly check against:
//  - substrings "*string*"
//  - prefixes "string*"
//  - suffixes "*string"
//  - full string names "string"
bool check_name_matches_filter_environment_var(const char *name, const struct loader_envvar_filter *filter_struct) {
    bool ret_value = false;
    const size_t name_len = strlen(name);
    char lower_name[VK_MAX_EXTENSION_NAME_SIZE];
    for (uint32_t iii = 0; iii < name_len; ++iii) {
        lower_name[iii] = (char)tolower(name[iii]);
    }
    lower_name[name_len] = '\0';
    for (uint32_t filt = 0; filt < filter_struct->count; ++filt) {
        // Check if the filter name is longer (this is with all special characters removed), and if it is
        // continue since it can't match.
        if (filter_struct->filters[filt].length > name_len) {
            continue;
        }
        switch (filter_struct->filters[filt].type) {
            case FILTER_STRING_SPECIAL:
                if (!strcmp(VK_LOADER_DISABLE_ALL_LAYERS_VAR_1, filter_struct->filters[filt].value) ||
                    !strcmp(VK_LOADER_DISABLE_ALL_LAYERS_VAR_2, filter_struct->filters[filt].value) ||
                    !strcmp(VK_LOADER_DISABLE_ALL_LAYERS_VAR_3, filter_struct->filters[filt].value)) {
                    ret_value = true;
                }
                break;

            case FILTER_STRING_SUBSTRING:
                if (NULL != strstr(lower_name, filter_struct->filters[filt].value)) {
                    ret_value = true;
                }
                break;

            case FILTER_STRING_SUFFIX:
                if (0 == strncmp(lower_name + name_len - filter_struct->filters[filt].length, filter_struct->filters[filt].value,
                                 filter_struct->filters[filt].length)) {
                    ret_value = true;
                }
                break;

            case FILTER_STRING_PREFIX:
                if (0 == strncmp(lower_name, filter_struct->filters[filt].value, filter_struct->filters[filt].length)) {
                    ret_value = true;
                }
                break;

            case FILTER_STRING_FULLNAME:
                if (0 == strncmp(lower_name, filter_struct->filters[filt].value, name_len)) {
                    ret_value = true;
                }
                break;
        }
        if (ret_value) {
            break;
        }
    }
    return ret_value;
}

// Get the layer name(s) from the env_name environment variable. If layer is found in
// search_list then add it to layer_list.  But only add it to layer_list if type_flags matches.
VkResult loader_add_environment_layers(struct loader_instance *inst, const enum layer_type_flags type_flags,
                                       const struct loader_envvar_all_filters *filters,
                                       struct loader_pointer_layer_list *target_list,
                                       struct loader_pointer_layer_list *expanded_target_list,
                                       const struct loader_layer_list *source_list) {
    VkResult res = VK_SUCCESS;
    char *layer_env = loader_getenv(ENABLED_LAYERS_ENV, inst);

    // If the layer environment variable is present (i.e. VK_INSTANCE_LAYERS), we will always add it to the layer list.
    if (layer_env != NULL) {
        size_t layer_env_len = strlen(layer_env) + 1;
        char *name = loader_stack_alloc(layer_env_len);
        if (name != NULL) {
            loader_strncpy(name, layer_env_len, layer_env, layer_env_len);

            loader_log(inst, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_LAYER_BIT, 0, "env var \'%s\' defined and adding layers \"%s\"",
                       ENABLED_LAYERS_ENV, name);

            // First look for the old-fashion layers forced on with VK_INSTANCE_LAYERS
            while (name && *name) {
                char *next = loader_get_next_path(name);

                if (strlen(name) > 0) {
                    bool found = false;
                    for (uint32_t i = 0; i < source_list->count; i++) {
                        struct loader_layer_properties *source_prop = &source_list->list[i];

                        if (0 == strcmp(name, source_prop->info.layerName)) {
                            found = true;
                            // Only add it if it doesn't already appear in the layer list
                            if (!loader_find_layer_name_in_list(source_prop->info.layerName, target_list)) {
                                if (0 == (source_prop->type_flags & VK_LAYER_TYPE_FLAG_META_LAYER)) {
                                    source_prop->enabled_by_what = ENABLED_BY_WHAT_VK_INSTANCE_LAYERS;
                                    res = loader_add_layer_properties_to_list(inst, target_list, source_prop);
                                    if (res == VK_ERROR_OUT_OF_HOST_MEMORY) goto out;
                                    res = loader_add_layer_properties_to_list(inst, expanded_target_list, source_prop);
                                    if (res == VK_ERROR_OUT_OF_HOST_MEMORY) goto out;
                                } else {
                                    res = loader_add_meta_layer(inst, filters, source_prop, target_list, expanded_target_list,
                                                                source_list, NULL);
                                    if (res == VK_ERROR_OUT_OF_HOST_MEMORY) goto out;
                                }
                                break;
                            }
                        }
                    }
                    if (!found) {
                        loader_log(inst, VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_LAYER_BIT, 0,
                                   "Layer \"%s\" was not found but was requested by env var VK_INSTANCE_LAYERS!", name);
                    }
                }
                name = next;
            }
        }
    }

    // Loop through all the layers and check the enable/disable filters
    for (uint32_t i = 0; i < source_list->count; i++) {
        struct loader_layer_properties *source_prop = &source_list->list[i];

        // If it doesn't match the type, or the name isn't what we're looking for, just continue
        if ((source_prop->type_flags & type_flags) != type_flags) {
            continue;
        }

        // We found a layer we're interested in, but has it been disabled...
        bool adding = true;
        bool is_implicit = (0 == (source_prop->type_flags & VK_LAYER_TYPE_FLAG_EXPLICIT_LAYER));
        bool disabled_by_type =
            (is_implicit) ? (filters->disable_filter.disable_all_implicit) : (filters->disable_filter.disable_all_explicit);
        if ((filters->disable_filter.disable_all || disabled_by_type ||
             check_name_matches_filter_environment_var(source_prop->info.layerName, &filters->disable_filter.additional_filters)) &&
            !check_name_matches_filter_environment_var(source_prop->info.layerName, &filters->allow_filter)) {
            loader_log(inst, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_LAYER_BIT, 0,
                       "Layer \"%s\" ignored because it has been disabled by env var \'%s\'", source_prop->info.layerName,
                       VK_LAYERS_DISABLE_ENV_VAR);
            adding = false;
        }

        // If we are supposed to filter through all layers, we need to compare the layer name against the filter.
        // This can override the disable above, so we want to do it second.
        // Also make sure the layer isn't already in the output_list, skip adding it if it is.
        if (check_name_matches_filter_environment_var(source_prop->info.layerName, &filters->enable_filter) &&
            !loader_find_layer_name_in_list(source_prop->info.layerName, target_list)) {
            adding = true;
            // Only way is_substring is true is if there are enable variables.  If that's the case, and we're past the
            // above, we should indicate that it was forced on in this way.
            loader_log(inst, VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_LAYER_BIT, 0,
                       "Layer \"%s\" forced enabled due to env var \'%s\'", source_prop->info.layerName, VK_LAYERS_ENABLE_ENV_VAR);
        } else {
            adding = false;
        }

        if (!adding) {
            continue;
        }

        // If not a meta-layer, simply add it.
        if (0 == (source_prop->type_flags & VK_LAYER_TYPE_FLAG_META_LAYER)) {
            source_prop->enabled_by_what = ENABLED_BY_WHAT_VK_LOADER_LAYERS_ENABLE;
            res = loader_add_layer_properties_to_list(inst, target_list, source_prop);
            if (res == VK_ERROR_OUT_OF_HOST_MEMORY) goto out;
            res = loader_add_layer_properties_to_list(inst, expanded_target_list, source_prop);
            if (res == VK_ERROR_OUT_OF_HOST_MEMORY) goto out;
        } else {
            res = loader_add_meta_layer(inst, filters, source_prop, target_list, expanded_target_list, source_list, NULL);
            if (res == VK_ERROR_OUT_OF_HOST_MEMORY) goto out;
        }
    }

out:

    if (layer_env != NULL) {
        loader_free_getenv(layer_env, inst);
    }

    return res;
}
