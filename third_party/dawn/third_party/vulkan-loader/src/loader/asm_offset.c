/*
 * Copyright (c) 2017-2024 The Khronos Group Inc.
 * Copyright (c) 2017-2024 Valve Corporation
 * Copyright (c) 2017-2024 LunarG, Inc.
 * Copyright (c) 2021 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
 * Author: Lenny Komow <lenny@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

// This code generates an assembly file which provides offsets to get struct members from assembly code.

// __USE_MINGW_ANSI_STDIO is needed to use the %zu format specifier with mingw-w64.
// Otherwise the compiler will complain about an unknown format specifier.
#if defined(__MINGW32__) && !defined(__USE_MINGW_ANSI_STDIO)
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdio.h>
#include "loader_common.h"
#include "log.h"

#if defined(__GNUC__) || defined(__clang__)
void produce_asm_define() {
    // GCC and clang make it easy to print easy to regex for values
    __asm__("# VULKAN_LOADER_ERROR_BIT = %c0" : : "i"(VULKAN_LOADER_ERROR_BIT));
    __asm__("# PTR_SIZE = %c0" : : "i"(sizeof(void *)));
    __asm__("# CHAR_PTR_SIZE = %c0" : : "i"(sizeof(char *)));
    __asm__("# FUNCTION_OFFSET_INSTANCE = %c0" : : "i"(offsetof(struct loader_instance, phys_dev_ext_disp_functions)));
    __asm__("# PHYS_DEV_OFFSET_INST_DISPATCH = %c0" : : "i"(offsetof(struct loader_instance_dispatch_table, phys_dev_ext)));
    __asm__("# PHYS_DEV_OFFSET_PHYS_DEV_TRAMP = %c0" : : "i"(offsetof(struct loader_physical_device_tramp, phys_dev)));
    __asm__("# ICD_TERM_OFFSET_PHYS_DEV_TERM = %c0" : : "i"(offsetof(struct loader_physical_device_term, this_icd_term)));
    __asm__("# PHYS_DEV_OFFSET_PHYS_DEV_TERM = %c0" : : "i"(offsetof(struct loader_physical_device_term, phys_dev)));
    __asm__("# INSTANCE_OFFSET_ICD_TERM = %c0" : : "i"(offsetof(struct loader_icd_term, this_instance)));
    __asm__("# DISPATCH_OFFSET_ICD_TERM = %c0" : : "i"(offsetof(struct loader_icd_term, phys_dev_ext)));
    __asm__("# EXT_OFFSET_DEVICE_DISPATCH = %c0" : : "i"(offsetof(struct loader_dev_dispatch_table, ext_dispatch)));
}
#elif defined(_WIN32)
// MSVC will print the name of the value and the value in hex
// Must disable optimization for this translation unit, otherwise the compiler strips out the variables
static const uint32_t PTR_SIZE = sizeof(void *);
static const uint32_t CHAR_PTR_SIZE = sizeof(char *);
static const uint32_t FUNCTION_OFFSET_INSTANCE = offsetof(struct loader_instance, phys_dev_ext_disp_functions);
static const uint32_t PHYS_DEV_OFFSET_INST_DISPATCH = offsetof(struct loader_instance_dispatch_table, phys_dev_ext);
static const uint32_t PHYS_DEV_OFFSET_PHYS_DEV_TRAMP = offsetof(struct loader_physical_device_tramp, phys_dev);
static const uint32_t ICD_TERM_OFFSET_PHYS_DEV_TERM = offsetof(struct loader_physical_device_term, this_icd_term);
static const uint32_t PHYS_DEV_OFFSET_PHYS_DEV_TERM = offsetof(struct loader_physical_device_term, phys_dev);
static const uint32_t INSTANCE_OFFSET_ICD_TERM = offsetof(struct loader_icd_term, this_instance);
static const uint32_t DISPATCH_OFFSET_ICD_TERM = offsetof(struct loader_icd_term, phys_dev_ext);
static const uint32_t EXT_OFFSET_DEVICE_DISPATCH = offsetof(struct loader_dev_dispatch_table, ext_dispatch);
#else
#warning asm_offset.c variable declarations need to be defined for this platform
#endif

#if !defined(_MSC_VER) || (_MSC_VER >= 1900)
#define SIZE_T_FMT "%-8zu"
#elif defined(__GNUC__) || defined(__clang__)
#define SIZE_T_FMT "%-8lu"
#else
#warning asm_offset.c SIZE_T_FMT must be defined for this platform
#endif

struct ValueInfo {
    const char *name;
    size_t value;
    const char *comment;
};

enum Assembler {
    UNKNOWN = 0,
    MASM = 1,
    MARMASM = 2,
    GAS = 3,
};

// This file can both be executed to produce gen_defines.asm and contains all the relevant data which
// the parse_asm_values.py script needs to write gen_defines.asm, necessary for cross compilation
int main(int argc, char **argv) {
    enum Assembler assembler = UNKNOWN;
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "MASM")) {
            assembler = MASM;
        } else if (!strcmp(argv[i], "MARMASM")) {
            assembler = MARMASM;
        } else if (!strcmp(argv[i], "GAS")) {
            assembler = GAS;
        }
    }
    if (assembler == UNKNOWN) {
        return -1;
    }

    struct ValueInfo values[] = {
        // clang-format off
        { .name = "VULKAN_LOADER_ERROR_BIT", .value = (size_t) VULKAN_LOADER_ERROR_BIT,
            .comment = "The numerical value of the enum value 'VULKAN_LOADER_ERROR_BIT'" },
        { .name = "PTR_SIZE", .value = sizeof(void*),
                    .comment = "The size of a pointer" },
        { .name = "CHAR_PTR_SIZE", .value = sizeof(char *),
            .comment = "The size of a 'const char *' struct" },
        { .name = "FUNCTION_OFFSET_INSTANCE", .value = offsetof(struct loader_instance, phys_dev_ext_disp_functions),
            .comment = "The offset of 'phys_dev_ext_disp_functions' within a 'loader_instance' struct" },
        { .name = "PHYS_DEV_OFFSET_INST_DISPATCH", .value = offsetof(struct loader_instance_dispatch_table, phys_dev_ext),
            .comment = "The offset of 'phys_dev_ext' within in 'loader_instance_dispatch_table' struct" },
        { .name = "PHYS_DEV_OFFSET_PHYS_DEV_TRAMP", .value = offsetof(struct loader_physical_device_tramp, phys_dev),
            .comment = "The offset of 'phys_dev' within a 'loader_physical_device_tramp' struct" },
        { .name = "ICD_TERM_OFFSET_PHYS_DEV_TERM", .value = offsetof(struct loader_physical_device_term, this_icd_term),
            .comment = "The offset of 'this_icd_term' within a 'loader_physical_device_term' struct" },
        { .name = "PHYS_DEV_OFFSET_PHYS_DEV_TERM", .value = offsetof(struct loader_physical_device_term, phys_dev),
            .comment = "The offset of 'phys_dev' within a 'loader_physical_device_term' struct" },
        { .name = "INSTANCE_OFFSET_ICD_TERM", .value = offsetof(struct loader_icd_term, this_instance),
            .comment = "The offset of 'this_instance' within a 'loader_icd_term' struct" },
        { .name = "DISPATCH_OFFSET_ICD_TERM", .value = offsetof(struct loader_icd_term, phys_dev_ext),
            .comment = "The offset of 'phys_dev_ext' within a 'loader_icd_term' struct" },
        { .name = "EXT_OFFSET_DEVICE_DISPATCH", .value = offsetof(struct loader_dev_dispatch_table, ext_dispatch),
            .comment = "The offset of 'ext_dispatch' within a 'loader_dev_dispatch_table' struct" },
        // clang-format on
    };

    FILE *file = loader_fopen("gen_defines.asm", "w");
    fprintf(file, "\n");
    if (assembler == MASM) {
        for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
            fprintf(file, "%-32s equ " SIZE_T_FMT "; %s\n", values[i].name, values[i].value, values[i].comment);
        }
    } else if (assembler == MARMASM) {
        fprintf(file, "    AREA    loader_structs_details, DATA,READONLY\n");
#if defined(__aarch64__) || defined(_M_ARM64)
        fprintf(file, "AARCH_64 EQU 1\n");
#else
        fprintf(file, "AARCH_64 EQU 0\n");
#endif
        for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
            fprintf(file, "%-32s EQU " SIZE_T_FMT "; %s\n", values[i].name, values[i].value, values[i].comment);
        }
        fprintf(file, "    END\n");
    } else if (assembler == GAS) {
#if defined(__x86_64__) || defined(__i386__)
        const char *comment_delimiter = "#";
#if defined(__x86_64__)
        fprintf(file, ".set X86_64, 1\n");
#endif  // defined(__x86_64__)
#elif defined(__aarch64__) || defined(__arm__)
        const char *comment_delimiter = "//";
#if defined(__aarch64__)
        fprintf(file, ".set AARCH_64, 1\n");
#else
        fprintf(file, ".set AARCH_64, 0\n");
#endif
#else
        // Default comment delimiter
        const char *comment_delimiter = "#";
#endif
        for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
            fprintf(file, ".set %-32s, " SIZE_T_FMT "%s %s\n", values[i].name, values[i].value, comment_delimiter,
                    values[i].comment);
        }
    }
    return fclose(file);
}
