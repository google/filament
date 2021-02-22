/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <utils/Log.h>

namespace bluevk {

static void* module = nullptr;

// Determines the file path to the Vulkan library and calls dlopen on it.
// Returns false on failure.
bool loadLibrary() {
    const char* path = nullptr;

    // For security, consult an environment variable only if Filament is built with this #define
    // enabled. Our weekly GitHub releases do not have this enabled.
#ifdef FILAMENT_VKLIBRARY_USE_ENV
    path = getenv("FILAMENT_VKLIBRARY_PATH");
#endif

    // If the environment variable is not set, fall back to a config-specified path, which is either
    // a custom path (common for SwiftShader), or "libvulkan.so" (common for Android).
    if (path == nullptr) {
#ifdef FILAMENT_VKLIBRARY_PATH
        path = FILAMENT_VKLIBRARY_PATH;
#elif defined(__ANDROID__)
        path = "libvulkan.so";
#elif defined(__linux__)
        path = "libvulkan.so.1";
#else
#error "This file should only be compiled for Android or Linux"
#endif
    }

    module = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (module == nullptr) {
        utils::slog.e << "Unable to load Vulkan from " << path << utils::io::endl;
    }
    return module != nullptr;
}

void* getInstanceProcAddr() {
    return dlsym(module, "vkGetInstanceProcAddr");
}

} // namespace bluevk
