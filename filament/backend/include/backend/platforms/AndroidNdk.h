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

#include <android/hardware_buffer.h>
#include <android/native_window.h>

#include <stdint.h>

#define FILAMENT_REQUIRES_API(x) __attribute__((__availability__(android,introduced=x)))

#define FILAMENT_USE_DLSYM(api) (__ANDROID_API__ < (api))

namespace filament::backend {

#if __ANDROID__ && __ANDROID_API__ < 37
extern "C" {
int32_t ANativeWindow_setProducerThrottlingEnabled(ANativeWindow* window,
        bool enabled);
int32_t ANativeWindow_isProducerThrottlingEnabled(ANativeWindow* window,
        bool* outEnabled);
}
#endif

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

#if FILAMENT_USE_DLSYM(30)
    static int32_t ANativeWindow_setFrameRate(ANativeWindow* window,
            float frameRate, int8_t compatibility) FILAMENT_REQUIRES_API(30) {
        return ndk.ANativeWindow_setFrameRate(window, frameRate, compatibility);
    }
#else
    static int32_t ANativeWindow_setFrameRate(ANativeWindow* window,
            float frameRate, int8_t compatibility) FILAMENT_REQUIRES_API(30) {
        return ::ANativeWindow_setFrameRate(window, frameRate, compatibility);
    }
#endif

#if FILAMENT_USE_DLSYM(31)
    static int32_t ANativeWindow_setFrameRateWithChangeStrategy(ANativeWindow* window,
            float frameRate, int8_t compatibility,
            int8_t strategy) FILAMENT_REQUIRES_API(31) {
        return ndk.ANativeWindow_setFrameRateWithChangeStrategy(
                window, frameRate, compatibility, strategy);
    }
#else
    static int32_t ANativeWindow_setFrameRateWithChangeStrategy(ANativeWindow* window,
            float frameRate, int8_t compatibility,
            int8_t strategy) FILAMENT_REQUIRES_API(31) {
        return ::ANativeWindow_setFrameRateWithChangeStrategy(
                window, frameRate, compatibility, strategy);
    }
#endif

#if FILAMENT_USE_DLSYM(37)
    static int32_t ANativeWindow_setProducerThrottlingEnabled(ANativeWindow* window,
            bool enabled) {
        return ndk.ANativeWindow_setProducerThrottlingEnabled ?
                ndk.ANativeWindow_setProducerThrottlingEnabled(window, enabled) : -1;
    }

    static int32_t ANativeWindow_isProducerThrottlingEnabled(ANativeWindow* window,
            bool* outEnabled) {
        return ndk.ANativeWindow_isProducerThrottlingEnabled ?
                ndk.ANativeWindow_isProducerThrottlingEnabled(window, outEnabled) : -1;
    }
#else
    static int32_t ANativeWindow_setProducerThrottlingEnabled(ANativeWindow* window,
            bool enabled) {
        return ::ANativeWindow_setProducerThrottlingEnabled(window, enabled);
    }

    static int32_t ANativeWindow_isProducerThrottlingEnabled(ANativeWindow* window,
            bool* outEnabled) {
        return ::ANativeWindow_isProducerThrottlingEnabled(window, outEnabled);
    }
#endif

    static bool isProducerThrottlingSupported() noexcept {
#if FILAMENT_USE_DLSYM(37)
        return ndk.ANativeWindow_setProducerThrottlingEnabled &&
                ndk.ANativeWindow_isProducerThrottlingEnabled;
#else
        return true;
#endif
    }

private:
#if FILAMENT_USE_DLSYM(37)
    static struct Ndk {
#if FILAMENT_USE_DLSYM(26)
        void (*AHardwareBuffer_acquire)(AHardwareBuffer*);
        void (*AHardwareBuffer_release)(AHardwareBuffer*);
        void (*AHardwareBuffer_describe)(AHardwareBuffer const*, AHardwareBuffer_Desc*);
#endif
#if FILAMENT_USE_DLSYM(30)
        int32_t (*ANativeWindow_setFrameRate)(ANativeWindow*, float, int8_t);
#endif
#if FILAMENT_USE_DLSYM(31)
        int32_t (*ANativeWindow_setFrameRateWithChangeStrategy)(
                ANativeWindow*, float, int8_t, int8_t);
#endif
#if FILAMENT_USE_DLSYM(37)
        int32_t (*ANativeWindow_setProducerThrottlingEnabled)(
                ANativeWindow*, bool);
        int32_t (*ANativeWindow_isProducerThrottlingEnabled)(
                ANativeWindow*, bool*);
#endif
    } ndk;
#endif
};

} // filament::backend

#endif //FILAMENT_BACKEND_ANDROIDNDK_H
