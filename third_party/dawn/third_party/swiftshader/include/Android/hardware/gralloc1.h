/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <hardware/hardware.h>

#include <cutils/native_handle.h>

#define GRALLOC_MODULE_API_VERSION_1_0 HARDWARE_MAKE_API_VERSION(1, 0)

#define GRALLOC_HARDWARE_MODULE_ID "gralloc"

enum {
    GRALLOC1_ERROR_NONE = 0,
    GRALLOC1_ERROR_BAD_HANDLE = 2,
    GRALLOC1_ERROR_BAD_VALUE = 3,
    GRALLOC1_ERROR_UNDEFINED = 6,
};

enum {
    GRALLOC1_FUNCTION_LOCK = 18,
    GRALLOC1_FUNCTION_UNLOCK = 20,
};

enum {
    GRALLOC1_CONSUMER_USAGE_CPU_READ = 1ULL << 1,
    GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN = 1ULL << 2 | GRALLOC1_CONSUMER_USAGE_CPU_READ,
    GRALLOC1_CONSUMER_USAGE_CPU_WRITE = 1ULL << 5,
    GRALLOC1_CONSUMER_USAGE_CPU_WRITE_OFTEN = 1ULL << 6 | GRALLOC1_CONSUMER_USAGE_CPU_WRITE,
    GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE = 1ULL << 8,
};

enum {
    GRALLOC1_PRODUCER_USAGE_CPU_READ = 1ULL << 1,
    GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN = 1ULL << 2 | GRALLOC1_PRODUCER_USAGE_CPU_READ,
    GRALLOC1_PRODUCER_USAGE_CPU_WRITE = 1ULL << 5,
    GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN = 1ULL << 6 | GRALLOC1_PRODUCER_USAGE_CPU_WRITE,
    GRALLOC1_PRODUCER_USAGE_GPU_RENDER_TARGET = 1ULL << 9,
};

typedef void (*gralloc1_function_pointer_t)();

struct gralloc1_rect_t {
    int32_t left;
    int32_t top;
    int32_t width;
    int32_t height;
};

struct gralloc1_device_t {
    hw_device_t common;
    void (*getCapabilities)(gralloc1_device_t*, uint32_t*, int32_t*);
    gralloc1_function_pointer_t (*getFunction)(gralloc1_device_t*, int32_t);
};

typedef int32_t (*GRALLOC1_PFN_LOCK)(gralloc1_device_t*, buffer_handle_t, uint64_t, uint64_t,
                                     const gralloc1_rect_t*, void**, int32_t);
typedef int32_t (*GRALLOC1_PFN_UNLOCK)(gralloc1_device_t*, buffer_handle_t, int32_t*);

static inline int gralloc1_open(const hw_module_t* module, gralloc1_device_t** device) {
    return module->methods->open(module, GRALLOC_HARDWARE_MODULE_ID,
                                 reinterpret_cast<hw_device_t**>(device));
}
