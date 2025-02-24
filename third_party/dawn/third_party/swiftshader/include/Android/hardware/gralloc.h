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

#include <cutils/native_handle.h>

#include <hardware/hardware.h>

struct android_ycbcr;

enum {
    GRALLOC_USAGE_SW_READ_OFTEN = 0x00000003U,
    GRALLOC_USAGE_SW_WRITE_OFTEN = 0x00000030U,
    GRALLOC_USAGE_HW_TEXTURE = 0x00000100U,
    GRALLOC_USAGE_HW_RENDER = 0x00000200U,
};

struct gralloc_module_t {
    hw_module_t common;
    int (*registerBuffer)(gralloc_module_t const*, buffer_handle_t);
    int (*unregisterBuffer)(gralloc_module_t const*, buffer_handle_t);
    int (*lock)(gralloc_module_t const*, buffer_handle_t, int, int, int, int, int, void**);
    int (*unlock)(gralloc_module_t const*, buffer_handle_t);
    int (*perform)(gralloc_module_t const*, int, ...);
    int (*lock_ycbcr)(gralloc_module_t const*, buffer_handle_t, int, int, int, int, int,
                      android_ycbcr*);
    int (*lockAsync)(gralloc_module_t const*, buffer_handle_t, int, int, int, int, int, void**, int);
    int (*unlockAsync)(gralloc_module_t const*, buffer_handle_t, int*);
    int (*lockAsync_ycbcr)(gralloc_module_t const*, buffer_handle_t, int, int, int, int, int,
                           android_ycbcr*, int);
    int32_t (*getTransportSize)(gralloc_module_t const*, buffer_handle_t, uint32_t, uint32_t);
    int32_t (*validateBufferSize)(gralloc_module_t const*, buffer_handle_t, uint32_t, uint32_t, int32_t, int, uint32_t);

    void* reserved_proc[1];
};
