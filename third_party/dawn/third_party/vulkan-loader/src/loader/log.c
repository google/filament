/*
 *
 * Copyright (c) 2014-2022 The Khronos Group Inc.
 * Copyright (c) 2014-2022 Valve Corporation
 * Copyright (c) 2014-2022 LunarG, Inc.
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

#include "log.h"

#include <stdio.h>
#include <stdarg.h>

#include "debug_utils.h"
#include "loader_common.h"
#include "loader_environment.h"
#include "settings.h"
#include "vk_loader_platform.h"

uint32_t g_loader_debug = 0;

void loader_init_global_debug_level(void) {
    char *env, *orig;

    if (g_loader_debug > 0) return;

    g_loader_debug = 0;

    // Parse comma-separated debug options
    orig = env = loader_getenv("VK_LOADER_DEBUG", NULL);
    while (env) {
        char *p = strchr(env, ',');
        size_t len;

        if (p) {
            len = p - env;
        } else {
            len = strlen(env);
        }

        if (len > 0) {
            if (strncmp(env, "all", len) == 0) {
                g_loader_debug = ~0u;
            } else if (strncmp(env, "warn", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_WARN_BIT;
            } else if (strncmp(env, "info", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_INFO_BIT;
            } else if (strncmp(env, "perf", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_PERF_BIT;
            } else if (strncmp(env, "error", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_ERROR_BIT;
            } else if (strncmp(env, "debug", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_DEBUG_BIT;
            } else if (strncmp(env, "layer", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_LAYER_BIT;
            } else if (strncmp(env, "driver", len) == 0 || strncmp(env, "implem", len) == 0 || strncmp(env, "icd", len) == 0) {
                g_loader_debug |= VULKAN_LOADER_DRIVER_BIT;
            }
        }

        if (!p) break;

        env = p + 1;
    }

    loader_free_getenv(orig, NULL);
}

void loader_set_global_debug_level(uint32_t new_loader_debug) { g_loader_debug = new_loader_debug; }

void generate_debug_flag_str(VkFlags msg_type, size_t cmd_line_size, char *cmd_line_msg) {
    cmd_line_msg[0] = '\0';

    if ((msg_type & VULKAN_LOADER_ERROR_BIT) != 0) {
        loader_strncat(cmd_line_msg, cmd_line_size, "ERROR", sizeof("ERROR"));
    }
    if ((msg_type & VULKAN_LOADER_WARN_BIT) != 0) {
        if (strlen(cmd_line_msg) > 0) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "WARNING", sizeof("WARNING"));
    }
    if ((msg_type & VULKAN_LOADER_INFO_BIT) != 0) {
        if (strlen(cmd_line_msg) > 0) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "INFO", sizeof("INFO"));
    }
    if ((msg_type & VULKAN_LOADER_DEBUG_BIT) != 0) {
        if (strlen(cmd_line_msg) > 0) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "DEBUG", sizeof("DEBUG"));
    }
    if ((msg_type & VULKAN_LOADER_PERF_BIT) != 0) {
        if (strlen(cmd_line_msg) > 0) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "PERF", sizeof("PERF"));
    }
    if ((msg_type & VULKAN_LOADER_DRIVER_BIT) != 0) {
        if (strlen(cmd_line_msg) > 0) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "DRIVER", sizeof("DRIVER"));
    }
    if ((msg_type & VULKAN_LOADER_LAYER_BIT) != 0) {
        if (strlen(cmd_line_msg) > 0) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "LAYER", sizeof("LAYER"));
    }

#undef STRNCAT_TO_BUFFER
}

void DECORATE_PRINTF(4, 5)
    loader_log(const struct loader_instance *inst, VkFlags msg_type, int32_t msg_code, const char *format, ...) {
    (void)msg_code;
    char msg[512] = {0};

    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(msg, sizeof(msg), format, ap);
    if ((ret >= (int)sizeof(msg)) || ret < 0) {
        msg[sizeof(msg) - 1] = '\0';
    }
    va_end(ap);

    if (inst) {
        VkDebugUtilsMessageSeverityFlagBitsEXT severity = 0;
        VkDebugUtilsMessageTypeFlagsEXT type = 0;
        VkDebugUtilsMessengerCallbackDataEXT callback_data = {0};
        VkDebugUtilsObjectNameInfoEXT object_name = {0};

        if ((msg_type & VULKAN_LOADER_INFO_BIT) != 0) {
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_WARN_BIT) != 0) {
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_ERROR_BIT) != 0) {
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_DEBUG_BIT) != 0) {
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_LAYER_BIT) != 0 || (msg_type & VULKAN_LOADER_DRIVER_BIT) != 0) {
            // Just driver or just layer bit should be treated as an info message in debug utils.
            severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        }

        if ((msg_type & VULKAN_LOADER_PERF_BIT) != 0) {
            type = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        } else if ((msg_type & VULKAN_LOADER_VALIDATION_BIT) != 0) {
            // For loader logging, if it's a validation message, we still want to also keep the general flag as well
            // so messages of type validation can still be triggered for general message callbacks.
            type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        } else {
            type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        }

        callback_data.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
        callback_data.pMessageIdName = "Loader Message";
        callback_data.pMessage = msg;
        callback_data.objectCount = 1;
        callback_data.pObjects = &object_name;
        object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        object_name.objectType = VK_OBJECT_TYPE_INSTANCE;
        object_name.objectHandle = (uint64_t)(uintptr_t)inst;

        util_SubmitDebugUtilsMessageEXT(inst, severity, type, &callback_data);
    }

    // Always log to stderr if this is a fatal error
    if (0 == (msg_type & VULKAN_LOADER_FATAL_ERROR_BIT)) {
        if (inst && inst->settings.settings_active && inst->settings.debug_level > 0) {
            // Exit early if the current instance settings have some debugging options but do match the current msg_type
            if (0 == (msg_type & inst->settings.debug_level)) {
                return;
            }
            // Check the global settings and if that doesn't say to skip, check the environment variable
        } else if (0 == (msg_type & g_loader_debug)) {
            return;
        }
    }

#if defined(DEBUG)
    int debug_flag_mask =
        msg_type & (VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_WARN_BIT | VULKAN_LOADER_INFO_BIT | VULKAN_LOADER_DEBUG_BIT);
    assert((debug_flag_mask == 0 || debug_flag_mask == VULKAN_LOADER_ERROR_BIT || debug_flag_mask == VULKAN_LOADER_WARN_BIT ||
            debug_flag_mask == VULKAN_LOADER_INFO_BIT || debug_flag_mask == VULKAN_LOADER_DEBUG_BIT) &&
           "This log has more than one exclusive debug flags (error, warn, info, debug) set");
#endif

    // Only need enough space to create the filter description header for log messages
    // Also use the same header for all output
    char cmd_line_msg[64] = {0};
    size_t cmd_line_size = sizeof(cmd_line_msg);

    loader_strncat(cmd_line_msg, cmd_line_size, "[Vulkan Loader] ", sizeof("[Vulkan Loader] "));

    bool need_separator = false;
    if ((msg_type & VULKAN_LOADER_ERROR_BIT) != 0) {
        loader_strncat(cmd_line_msg, cmd_line_size, "ERROR", sizeof("ERROR"));
        need_separator = true;
    } else if ((msg_type & VULKAN_LOADER_WARN_BIT) != 0) {
        loader_strncat(cmd_line_msg, cmd_line_size, "WARNING", sizeof("WARNING"));
        need_separator = true;
    } else if ((msg_type & VULKAN_LOADER_INFO_BIT) != 0) {
        loader_strncat(cmd_line_msg, cmd_line_size, "INFO", sizeof("INFO"));
        need_separator = true;
    } else if ((msg_type & VULKAN_LOADER_DEBUG_BIT) != 0) {
        loader_strncat(cmd_line_msg, cmd_line_size, "DEBUG", sizeof("DEBUG"));
        need_separator = true;
    }

    if ((msg_type & VULKAN_LOADER_PERF_BIT) != 0) {
        if (need_separator) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "PERF", sizeof("PERF"));
    } else if ((msg_type & VULKAN_LOADER_DRIVER_BIT) != 0) {
        if (need_separator) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "DRIVER", sizeof("DRIVER"));
    } else if ((msg_type & VULKAN_LOADER_LAYER_BIT) != 0) {
        if (need_separator) {
            loader_strncat(cmd_line_msg, cmd_line_size, " | ", sizeof(" | "));
        }
        loader_strncat(cmd_line_msg, cmd_line_size, "LAYER", sizeof("LAYER"));
    }

    loader_strncat(cmd_line_msg, cmd_line_size, ": ", sizeof(": "));
    size_t num_used = strlen(cmd_line_msg);

    // Justifies the output to at least 29 spaces
    if (num_used < 32) {
        const char space_buffer[] = "                                ";
        // Only write (32 - num_used) spaces
        loader_strncat(cmd_line_msg, cmd_line_size, space_buffer, sizeof(space_buffer) - 1 - num_used);
    }
    // Assert that we didn't write more than what is available in cmd_line_msg
    assert(cmd_line_size > num_used);

    fputs(cmd_line_msg, stderr);
    fputs(msg, stderr);
    fputc('\n', stderr);
#if defined(WIN32)
    OutputDebugString(cmd_line_msg);
    OutputDebugString(msg);
    OutputDebugString("\n");
#endif
}

void loader_log_asm_function_not_supported(const struct loader_instance *inst, VkFlags msg_type, int32_t msg_code,
                                           const char *func_name) {
    loader_log(inst, msg_type, msg_code, "Function %s not supported for this physical device", func_name);
}
