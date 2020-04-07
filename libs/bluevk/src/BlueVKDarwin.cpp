
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

#include <utils/Path.h>

#include <dlfcn.h>

using utils::Path;

namespace bluevk {

#ifdef IOS
static const char* VKLIBRARY_PATH = "Frameworks/libMoltenVK.dylib";
#else
static const char* VKLIBRARY_PATH = "libvulkan.1.dylib";
#endif

static void* module = nullptr;

bool loadLibrary() {

#ifndef FILAMENT_VKLIBRARY_PATH
    // Rather than looking in the working directory, look for the dylib in the same folder that the
    // executable lives in. This allows MacOS users to run Vulkan-based Filament apps from anywhere.
    const Path executableFolder = Path::getCurrentExecutable().getParent();
    const Path dylibPath = executableFolder.concat(VKLIBRARY_PATH);

    // Provide a value for VK_ICD_FILENAMES only if it has not already been set.
    const char* icd = getenv("VK_ICD_FILENAMES");
    if (icd == nullptr) {
        const Path jsonPath = executableFolder.concat("MoltenVK_icd.json");
        setenv("VK_ICD_FILENAMES", jsonPath.c_str(), 1);
    }
#else
    const Path dylibPath = FILAMENT_VKLIBRARY_PATH;
#endif

    module = dlopen(dylibPath.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (module == nullptr) {
        printf("%s", dlerror());
    }
    return module != nullptr;
}

void* getInstanceProcAddr() {
    return dlsym(module, "vkGetInstanceProcAddr");
}

} // namespace bluevk
