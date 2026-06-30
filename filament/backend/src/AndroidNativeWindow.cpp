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

#include "AndroidNativeWindow.h"

#include <backend/platforms/AndroidNdk.h>

#include <utils/compiler.h>
#include <utils/Logger.h>

#include <android/api-level.h>
#include <android/native_window.h>

#include <cerrno>
#include <cstdint>
#include <utility>

#include <dlfcn.h>

namespace filament::backend {

bool NativeWindow::queuesToWindowComposer(ANativeWindow* const anw) noexcept {
    if (!anw) {
        return false;
    }
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    if (UTILS_LIKELY(pWindow->query)) {
        int value = 0;
        int const err = pWindow->query(anw, QUEUES_TO_WINDOW_COMPOSER, &value);
        return err == 0 && value != 0;
    }
    return false;
}

std::pair<int, bool> NativeWindow::isValid(ANativeWindow* const anw) noexcept {
#if __ANDROID_API__ >= 26
    // libnativewindow.so is not available before API level 26, this means we can't call
    // any method above 25 (even protected by __builtin_available()).
    if (__builtin_available(android 28, *)) {
        // this a proxy for is_valid()
        auto const result = ANativeWindow_getBuffersDataSpace(anw);
        return { result, result >= 0 };
    }
#endif

    // fallback on using private APIs
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    if (UTILS_LIKELY(pWindow->query)) {
        int isValid = 0;
        int const err = pWindow->query(anw, IS_VALID, &isValid);
        return { err, bool(isValid) };
    }
    return { -EINVAL, false };
}

int NativeWindow::getNextFrameId(ANativeWindow* anw, uint64_t* frameId) {
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    return pWindow->perform(anw, GET_NEXT_FRAME_ID, frameId);
}

int NativeWindow::enableFrameTimestamps(ANativeWindow* anw, bool const enable) {
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    return pWindow->perform(anw, ENABLE_FRAME_TIMESTAMPS, enable);
}

int NativeWindow::frameTimestampsSupportsPresent(ANativeWindow* anw, bool* outSupportsPresent) {
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    int value = 0;
    bool const success = pWindow->perform(anw, FRAME_TIMESTAMPS_SUPPORTS_PRESENT, &value);
    if (success) {
        *outSupportsPresent = bool(value);
    }
    return success;
}

int NativeWindow::getCompositorTiming(ANativeWindow* anw,
        int64_t* compositeDeadline, int64_t* compositeInterval,
        int64_t* compositeToPresent) {
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    return pWindow->perform(anw, GET_COMPOSITOR_TIMING,
            compositeDeadline, compositeInterval, compositeToPresent);
}

int NativeWindow::getFrameTimestamps(ANativeWindow* anw,
        uint64_t const frameId,
        int64_t* outRequestedPresentTime, int64_t* outAcquireTime,
        int64_t* outLatchTime, int64_t* outFirstRefreshStartTime,
        int64_t* outLastRefreshStartTime, int64_t* outGpuCompositionDoneTime,
        int64_t* outDisplayPresentTime, int64_t* outDequeueReadyTime,
        int64_t* outReleaseTime) {
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    return pWindow->perform(anw, GET_FRAME_TIMESTAMPS,
            frameId, outRequestedPresentTime, outAcquireTime, outLatchTime,
            outFirstRefreshStartTime, outLastRefreshStartTime,
            outGpuCompositionDoneTime, outDisplayPresentTime,
            outDequeueReadyTime, outReleaseTime);
}

bool NativeWindow::isProducerThrottlingSupported() noexcept {
    return AndroidNdk::isProducerThrottlingSupported();
}

int32_t NativeWindow::setProducerThrottlingEnabled(ANativeWindow* const anw,
        bool const enabled) noexcept {
    return AndroidNdk::ANativeWindow_setProducerThrottlingEnabled(anw, enabled);
}

utils::tribool NativeWindow::isFrameRateChangeSupported(ANativeWindow* const anw) noexcept {
    if (!anw) return utils::tribool::kFalse;

    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    if (UTILS_LIKELY(pWindow->query)) {
        int value = 0;
        int const err = pWindow->query(anw, QUEUES_TO_WINDOW_COMPOSER, &value);
        if (err == 0) {
            if (value != 0) {
                if (__builtin_available(android 30, *)) {
                    return utils::tribool::kTrue;
                }
            }
            return utils::tribool::kFalse;
        }
        return utils::tribool::kIndeterminate;
    }
    return utils::tribool::kFalse;
}

int NativeWindow::setFrameRate(ANativeWindow* const anw, float const frameRate,
        Platform::FrameRateCompatibility const compatibility,
        Platform::ChangeFrameRateStrategy const strategy) noexcept {
    if (queuesToWindowComposer(anw)) {
        if (__builtin_available(android 31, *)) {
            return AndroidNdk::ANativeWindow_setFrameRateWithChangeStrategy(anw, frameRate,
                    static_cast<int8_t>(compatibility), static_cast<int8_t>(strategy));
        }
        if (__builtin_available(android 30, *)) {
            return AndroidNdk::ANativeWindow_setFrameRate(
                    anw, frameRate, static_cast<int8_t>(compatibility));
        }
    }
    return -ENOSYS;
}

} // namespace filament::backend

