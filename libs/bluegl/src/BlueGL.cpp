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

#include <bluegl/BlueGL.h>

#include <iostream>
#include <mutex>

#include "private_BlueGL.h"

namespace bluegl {

// OS Dependent.
extern bool initBinder();
extern void* loadFunction(const char* name);
extern void shutdownBinder();

static void undefined() {
#ifndef NDEBUG
    std::cerr << "calling missing GL function!" << std::endl;
#endif
}

int bind() {
    std::lock_guard<std::mutex> lock(g_library_mutex);
    g_library_refcount++;

    if (g_library_refcount == 1) {
        if (initBinder()) {
            for (unsigned int i = 0; i < blueCoreNumFunctions; i++) {
                *g_gl_stubs[i].api_call = loadFunction(g_gl_stubs[i].api_name);
                if (*g_gl_stubs[i].api_call == nullptr) {
                    *g_gl_stubs[i].api_call = (void*)&undefined;
                }
            }
            return 0;
        } else {
            std::cerr << "Could not init binder." << std::endl;
        }
    } else {
        return 0;
    }

    return -1;
}

void unbind() {
    std::lock_guard<std::mutex> lock(g_library_mutex);
    uint32_t refcount = g_library_refcount;
    if (refcount > 0) {
        g_library_refcount--;
    }

    if (refcount == 1) {
        shutdownBinder();

        for (unsigned int i = 0; i < 2574; i++) {
            *g_gl_stubs[i].api_call = nullptr;
        }
    }
}

}; // namespace bluegl
