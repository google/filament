
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

#include <utils/Panic.h>

#include <dlfcn.h>

namespace bluevk {

static void* module = nullptr;

bool loadLibrary() {
#ifdef FILAMENT_VKLIBRARY_PATH
    const char* dylibPath = FILAMENT_VKLIBRARY_PATH;
#else
    const char* dylibPath = "libvulkan.1.dylib";
#endif

    module = dlopen(dylibPath, RTLD_NOW | RTLD_LOCAL);
    ASSERT_POSTCONDITION(module != nullptr,
            "BlueVK is unable to load entry points: %s.\n"
            "Install the LunarG SDK with 'System Global Installation' and reboot.\n",
            dlerror());
    return module != nullptr;
}

void* getInstanceProcAddr() {
    return dlsym(module, "vkGetInstanceProcAddr");
}

} // namespace bluevk
