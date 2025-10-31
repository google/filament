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

#include <android/native_window.h>

#include <utils/compiler.h>
#include <utils/Logger.h>

#include <cstdint>
#include <cerrno>
#include <utility>

#include <dlfcn.h>

namespace filament::backend {

std::pair<int, bool> NativeWindow::isValid(ANativeWindow* const anw) noexcept {
    if (__builtin_available(android 28, *)) {
        // this a proxy for is_valid()
        auto const result = ANativeWindow_getBuffersDataSpace(anw);
        return { result, result >= 0 };
    }
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

int NativeWindow::enableFrameTimestamps(ANativeWindow* anw, bool enable) {
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    return pWindow->perform(anw, ENABLE_FRAME_TIMESTAMPS, enable);
}

int NativeWindow::getCompositorTiming(ANativeWindow* anw,
        int64_t* compositeDeadline, int64_t* compositeInterval,
        int64_t* compositeToPresentLatency) {
    NativeWindow const* pWindow = reinterpret_cast<NativeWindow const*>(anw);
    return pWindow->perform(anw, GET_COMPOSITOR_TIMING,
            compositeDeadline, compositeInterval, compositeToPresentLatency);
}

int NativeWindow::getFrameTimestamps(ANativeWindow* anw,
        uint64_t frameId,
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

AndroidProducerThrottling::AndroidProducerThrottling() {
    // note: we don't need to dlclose() mNativeWindowLib here, because the library will be cleaned
    // when the process ends and dlopen() are ref-counted. dlclose() NDK documentation documents
    // not to call dlclose().
    void* nativeWindowLibHandle = dlopen("libnativewindow.so", RTLD_LOCAL | RTLD_NOW);
    if (nativeWindowLibHandle) {
        mSetProducerThrottlingEnabled =
                (int32_t(*)(ANativeWindow*, bool)) dlsym(nativeWindowLibHandle,
                        "ANativeWindow_setProducerThrottlingEnabled");

        mIsProducerThrottlingEnabled =
                (int32_t(*)(ANativeWindow*, bool*)) dlsym(nativeWindowLibHandle,
                        "ANativeWindow_isProducerThrottlingEnabled");

        if (mSetProducerThrottlingEnabled && mIsProducerThrottlingEnabled) {
            LOG(INFO) << "Producer Throttling API available";
        }
    }
}

int32_t AndroidProducerThrottling::setProducerThrottlingEnabled(
        ANativeWindow* window, bool enabled) const {
    if (mSetProducerThrottlingEnabled) {
        return mSetProducerThrottlingEnabled(window, enabled);
    }
    return -1;
}

int32_t AndroidProducerThrottling::isProducerThrottlingEnabled(
        ANativeWindow* window, bool* outEnabled) const {
    if (mIsProducerThrottlingEnabled) {
        return mIsProducerThrottlingEnabled(window, outEnabled);
    }
    return -1;
}

bool AndroidProducerThrottling::isSupported() const noexcept {
    return mSetProducerThrottlingEnabled && mIsProducerThrottlingEnabled;
}

} // namespace filament::backend
