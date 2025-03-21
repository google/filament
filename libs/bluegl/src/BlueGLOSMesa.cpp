/*
 * Copyright (C) 2024 The Android Open Source Project
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

#if defined(__linux__)

#include <osmesa.h>

// This is to ensure that linking during compilation will not fail even if
// OSMesaGetProcAddress is not linked.
__attribute__((weak)) OSMESAproc OSMesaGetProcAddress(char const*);

#elif defined(__APPLE__)

#include <GL/osmesa.h>

#endif // __linux__

#if defined(__linux__)
#endif

namespace bluegl {

namespace {
using ProcAddressFunc = void*(*)(char const* funcName);
}

struct Driver {
    ProcAddressFunc OSMesaGetProcAddress;
    void* library;
} g_driver = {nullptr, nullptr};

bool initBinder() {
    static constexpr char const* libraryNames[] = {
#if defined(__linux__)
        "libOSMesa.so",
        "libosmesa.so",
#elif defined(__APPLE__)
        "libOSMesa.dylib",
#endif
    };
    for (char const* name: libraryNames) {
        g_driver.library = dlopen(name, RTLD_GLOBAL | RTLD_NOW);
        if (g_driver.library) {
            break;
        }
    }
    if (g_driver.library) {
        // Linking against a libosmesa.so.
        g_driver.OSMesaGetProcAddress =
                (ProcAddressFunc) dlsym(g_driver.library, "OSMesaGetProcAddress");
    }
#if defined(__linux__)
    else {
        // If Filament was built as a dynamic library.
        g_driver.OSMesaGetProcAddress = (ProcAddressFunc) dlsym(RTLD_LOCAL, "OSMesaGetProcAddress");
    }
    if (!g_driver.OSMesaGetProcAddress) {
        // If statically linking OSMesa.
        g_driver.OSMesaGetProcAddress = (ProcAddressFunc) OSMesaGetProcAddress;
    }
#endif

    return g_driver.OSMesaGetProcAddress;
}

void* loadFunction(const char* name) {
    return (void*) g_driver.OSMesaGetProcAddress(name);
}

void shutdownBinder() {
    if (g_driver.library) {
        dlclose(g_driver.library);
    }
    memset(&g_driver, 0, sizeof(g_driver));
}

} // namespace bluegl
