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

#ifndef FILAMENT_BACKEND_ANDROIDNATIVEWINDOW_H
#define FILAMENT_BACKEND_ANDROIDNATIVEWINDOW_H

#include <android/native_window.h>

#include <cstdint>
#include <utility>

namespace filament::backend {

struct NativeWindow {
    // this class can't be instantiated. It's used by casting ANativeWindow* to it.
    NativeWindow() noexcept = delete;
    ~NativeWindow() noexcept = delete;

    // is valid query enum value
    enum {
        IS_VALID                = 17,
        GET_NEXT_FRAME_ID       = 24,
        ENABLE_FRAME_TIMESTAMPS = 25,
        GET_COMPOSITOR_TIMING   = 26,
        GET_FRAME_TIMESTAMPS    = 27,
    };

#if defined(__LP64__)
    uint64_t pad[18];
#else
    uint32_t pad[21];
#endif
    int (*query)(ANativeWindow const*, int, int*);
    int (*perform)(ANativeWindow*, int, ...);

    // return whether the ANativeWindow is valid
    static std::pair<int, bool> isValid(ANativeWindow* anw) noexcept;

    static int getNextFrameId(ANativeWindow* anw, uint64_t* frameId);
    static int enableFrameTimestamps(ANativeWindow* anw, bool enable);
    static int getCompositorTiming(ANativeWindow* anw,
            int64_t* compositeDeadline, int64_t* compositeInterval,
            int64_t* compositeToPresentLatency);
    static int getFrameTimestamps(ANativeWindow* anw,
            uint64_t frameId,
            int64_t* outRequestedPresentTime, int64_t* outAcquireTime,
            int64_t* outLatchTime, int64_t* outFirstRefreshStartTime,
            int64_t* outLastRefreshStartTime, int64_t* outGpuCompositionDoneTime,
            int64_t* outDisplayPresentTime, int64_t* outDequeueReadyTime,
            int64_t* outReleaseTime);
};

struct AndroidProducerThrottling {
    AndroidProducerThrottling();
    int32_t setProducerThrottlingEnabled(ANativeWindow* window, bool enabled) const;
    int32_t isProducerThrottlingEnabled(ANativeWindow* window, bool* outEnabled) const;
    bool isSupported() const noexcept;
private:
    int32_t (*mSetProducerThrottlingEnabled)(ANativeWindow* window, bool enabled) = nullptr;
    int32_t (*mIsProducerThrottlingEnabled)(ANativeWindow* window, bool* outEnabled) = nullptr;
};


} // namespace filament::backend

#endif //FILAMENT_BACKEND_ANDROIDNATIVEWINDOW_H
