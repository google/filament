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

#ifndef FILAMENT_BACKEND_ANDROIDNDK_H
#define FILAMENT_BACKEND_ANDROIDNDK_H

#include <android/native_window.h>
#include <android/hardware_buffer.h>

#define FILAMENT_REQUIRES_API(x) __attribute__((__availability__(android,introduced=x)))

#define FILAMENT_USE_DLSYM(api) (__ANDROID_API__ < (api))

namespace filament::backend {

class AndroidNdk {
public:
    AndroidNdk();

#if FILAMENT_USE_DLSYM(26)

    static void AHardwareBuffer_acquire(
            AHardwareBuffer* buffer) FILAMENT_REQUIRES_API(26) {
        ndk.AHardwareBuffer_acquire(buffer);
    }

    static void AHardwareBuffer_release(
            AHardwareBuffer* buffer) FILAMENT_REQUIRES_API(26) {
        ndk.AHardwareBuffer_release(buffer);
    }

    static void AHardwareBuffer_describe(
            AHardwareBuffer const* buffer, AHardwareBuffer_Desc* desc) FILAMENT_REQUIRES_API(26) {
        ndk.AHardwareBuffer_describe(buffer, desc);
    }

#else

    static void AHardwareBuffer_acquire(
            AHardwareBuffer* buffer) FILAMENT_REQUIRES_API(26) {
        ::AHardwareBuffer_acquire(buffer);
    }
    static void AHardwareBuffer_release(
            AHardwareBuffer* buffer) FILAMENT_REQUIRES_API(26) {
        ::AHardwareBuffer_release(buffer);
    }
    static void AHardwareBuffer_describe(
            AHardwareBuffer const* buffer, AHardwareBuffer_Desc* desc) FILAMENT_REQUIRES_API(26) {
        ::AHardwareBuffer_describe(buffer, desc);
    }

#endif

private:
#if FILAMENT_USE_DLSYM(26)
    static struct Ndk {
        void (*AHardwareBuffer_acquire)(AHardwareBuffer*);
        void (*AHardwareBuffer_release)(AHardwareBuffer*);
        void (*AHardwareBuffer_describe)(AHardwareBuffer const*, AHardwareBuffer_Desc*);
    } ndk;
#endif
};

} // filament::backend

#endif //FILAMENT_BACKEND_ANDROIDNDK_H
