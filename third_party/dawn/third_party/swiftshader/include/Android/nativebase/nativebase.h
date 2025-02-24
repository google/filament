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

#include <cstdint>
#include <cstring>

// clang-format off
#define ANDROID_NATIVE_MAKE_CONSTANT(a, b, c, d) \
    ((static_cast<unsigned int>(a) << 24) | \
     (static_cast<unsigned int>(b) << 16) | \
     (static_cast<unsigned int>(c) <<  8) | \
     (static_cast<unsigned int>(d) <<  0))
// clang-format on

struct android_native_base_t {
    int magic;
    int version;
    void* reserved[4];
    void (*incRef)(android_native_base_t*);
    void (*decRef)(android_native_base_t*);
};

#define ANDROID_NATIVE_BUFFER_MAGIC ANDROID_NATIVE_MAKE_CONSTANT('_', 'b', 'f', 'r')

struct ANativeWindowBuffer {
    ANativeWindowBuffer() {
        common.magic = ANDROID_NATIVE_BUFFER_MAGIC;
        common.version = sizeof(ANativeWindowBuffer);
        memset(common.reserved, 0, sizeof(common.reserved));
    }

    android_native_base_t common;

    int width;
    int height;
    int stride;
    int format;
    int usage_deprecated;
    uintptr_t layerCount;

    void* reserved[1];

    const native_handle_t* handle;
    uint64_t usage;

    void* reserved_proc[8 - (sizeof(uint64_t) / sizeof(void*))];
};
