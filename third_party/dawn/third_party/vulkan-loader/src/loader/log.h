/*
 *
 * Copyright (c) 2014-2021 The Khronos Group Inc.
 * Copyright (c) 2014-2021 Valve Corporation
 * Copyright (c) 2014-2021 LunarG, Inc.
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

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "vulkan/vulkan_core.h"

struct loader_instance;

enum vulkan_loader_debug_flags {
    VULKAN_LOADER_INFO_BIT = 0x01,
    VULKAN_LOADER_WARN_BIT = 0x02,
    VULKAN_LOADER_PERF_BIT = 0x04,
    VULKAN_LOADER_ERROR_BIT = 0x08,
    VULKAN_LOADER_DEBUG_BIT = 0x10,
    VULKAN_LOADER_LAYER_BIT = 0x20,
    VULKAN_LOADER_DRIVER_BIT = 0x40,
    VULKAN_LOADER_VALIDATION_BIT = 0x80,
    VULKAN_LOADER_FATAL_ERROR_BIT = 0x100,  // only forces the output to be printed to stderr, has no other effect
};

// Checks for the environment variable VK_LOADER_DEBUG and sets up the current debug level accordingly
// This should be called before any Vulkan API calls, eg in the initialization of the .dll or .so
void loader_init_global_debug_level(void);

// Sets the global debug level - used by global settings files
void loader_set_global_debug_level(uint32_t new_loader_debug);

// Writes a stringified version of enum vulkan_loader_debug_flags into a char array cmd_line_msg of length cmd_line_size
void generate_debug_flag_str(VkFlags msg_type, size_t cmd_line_size, char *cmd_line_msg);

// The asm declaration prevents name mangling which is necessary for macOS
#if defined(MODIFY_UNKNOWN_FUNCTION_DECLS)
#define ASM_NAME(name) __asm(name)
#else
#define ASM_NAME(name)
#endif

#if defined(__clang__)
#define DECORATE_PRINTF(_fmt_argnum, _first_param_num) __attribute__((format(printf, _fmt_argnum, _first_param_num)))
#elif defined(__GNUC__)
#define DECORATE_PRINTF(_fmt_argnum, _first_param_num) __attribute__((format(gnu_printf, _fmt_argnum, _first_param_num)))
#else
#define DECORATE_PRINTF(_fmt_num, _first_param_num)
#endif

// Logs a message to stderr
// May output to DebugUtils if the instance isn't null and the extension is enabled.
void DECORATE_PRINTF(4, 5) loader_log(const struct loader_instance *inst, VkFlags msg_type, int32_t msg_code, const char *format,
                                      ...) ASM_NAME("loader_log");

// Used for the assembly code to emit an specific error message
// This is a work around for linux 32 bit error handling not passing relocatable strings correctly
void loader_log_asm_function_not_supported(const struct loader_instance *inst, VkFlags msg_type, int32_t msg_code,
                                           const char *func_name) ASM_NAME("loader_log_asm_function_not_supported");

#undef ASM_NAME
