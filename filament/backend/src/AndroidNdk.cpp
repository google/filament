/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "AndroidNdk.h"

#include <android/hardware_buffer.h>

#include <utils/compiler.h>

#include <mutex>

#include <dlfcn.h>

namespace filament::backend {

UTILS_UNUSED
static std::once_flag sInitOnce{};

template <typename T>
UTILS_UNUSED
static void loadSymbol(void* handle, const char* symbol, T& pfn) {
    pfn = T(dlsym(handle, symbol));
}

#if FILAMENT_USE_DLSYM(26)
AndroidNdk::Ndk AndroidNdk::ndk{};
#endif

AndroidNdk::AndroidNdk() {
#if FILAMENT_USE_DLSYM(26)
    std::call_once(sInitOnce, [] {
        // note: we don't need to dlclose() mNativeWindowLib here, because the library will be cleaned
        // when the process ends and dlopen() are ref-counted. dlclose() NDK documentation documents
        // not to call dlclose().
        if (__builtin_available(android 26, *)) {
            void* h = dlopen("libnativewindow.so", RTLD_LOCAL | RTLD_NOW);
            if (h) {
                loadSymbol(h, "AHardwareBuffer_acquire", ndk.AHardwareBuffer_acquire);
                loadSymbol(h, "AHardwareBuffer_release", ndk.AHardwareBuffer_release);
                loadSymbol(h, "AHardwareBuffer_describe", ndk.AHardwareBuffer_describe);
            }
        }
    });
#endif
}

} // filament::backend
