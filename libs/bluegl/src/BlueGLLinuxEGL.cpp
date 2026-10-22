/*
 * Copyright (C) 2022 The Android Open Source Project
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
#include <string.h>

namespace bluegl {

struct Driver {
    void* (*eglGetProcAddress)(const char*);
    void* library;
} g_driver = {nullptr, nullptr};

bool initBinder() {
    const char* library_name = "libEGL.so.1";
    g_driver.library = dlopen(library_name, RTLD_GLOBAL | RTLD_NOW);

    if (!g_driver.library) {
        return false;
    }

    g_driver.eglGetProcAddress = (void *(*)(const char *))
            dlsym(g_driver.library, "eglGetProcAddress");

    return g_driver.eglGetProcAddress;
}

void* loadFunction(const char* name) {
  return (void*) g_driver.eglGetProcAddress((const char*) name);
}

void shutdownBinder() {
    dlclose(g_driver.library);
    memset(&g_driver, 0, sizeof(g_driver));
}

} // namespace bluegl
