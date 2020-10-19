
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

namespace bluevk {

#if defined(__ANDROID__)
static const char* VKLIBRARY_PATH = "libvulkan.so";
#elif defined(__linux__)
static const char* VKLIBRARY_PATH = "libvulkan.so.1";
#else
#error "This file should only be compiled for Android or Linux"
#endif

static void* module = nullptr;

bool loadLibrary() {
#ifndef FILAMENT_VKLIBRARY_PATH
    module = dlopen(VKLIBRARY_PATH, RTLD_NOW | RTLD_LOCAL);
#else
    module = dlopen(FILAMENT_VKLIBRARY_PATH, RTLD_NOW | RTLD_LOCAL);
#endif
    return module != nullptr;
}

void* getInstanceProcAddr() {
    return dlsym(module, "vkGetInstanceProcAddr");
}

} // namespace bluevk
