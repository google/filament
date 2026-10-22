/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_BUFFERSTUFFINGDETECTOR_H
#define TNT_FILAMENT_DETAILS_BUFFERSTUFFINGDETECTOR_H

#include <filament/Renderer.h>

#include <utils/Slice.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

/**
 * BufferStuffingDetector monitors FrameInfo history to detect when completed frames are backing up
 * (buffer stuffing / queue buildup) before presentation.
 */
class BufferStuffingDetector {
public:
    BufferStuffingDetector() noexcept = default;

    /**
     * Evaluates historical frame telemetry to detect buffer stuffing.
     * Returns true if stuffing is detected (i.e. the frame should be skipped).
     * Automatically records currentFrameId as skipped when stuffing is detected.
     */
    bool shouldSkipFrame(utils::Slice<const Renderer::FrameInfo> history,
            uint32_t const currentFrameId) noexcept {
        for (size_t i = 0, c = history.size(); i < c; i++) {
            auto const& info = history[i];
            if (info.frameId <= mLastFrameId) {
                continue;
            }
            if (info.expectedPresentLatency < 0 || info.displayPresent < 0) {
                continue;
            }

            // this frame's presentation latency (time from vsync to display)
            auto const presentLatency = info.displayPresent - info.vsync;

            // The maximum latency we allow. we choose the expected presentation latency + one whole frame, this
            // is because by default the expected presentation latency is the shortest possible, and is usually almost
            // impossible to achieve, so we aim for an extra frame. The system is typically configured to allow this
            // (on Android), i.e. it has enough intermediary buffers.
            // TODO: the "expectedPresentLatency" should come from the user, and if they use the choreographer APIs
            //       on Android, it will be set to that. Of course, we need an abstraction. This code assumes
            //       the caller selected the default timeline.
            auto const maximumLatencyAllowed = info.expectedPresentLatency + info.displayPresentInterval;

            // if we took a whole extra frame more than the maximum latency allowed, we need to skip a frame.
            // we use a whole frame because in practice the presentLatency will straddle around the
            // maximumLatency, and the latency "unit" is displayPresentInterval.
            if (presentLatency - maximumLatencyAllowed >= info.displayPresentInterval) {
                return true;
            }
            break;
        }
        return false;
    }

    void setLastFrameId(uint32_t const frameId) noexcept {
        mLastFrameId = frameId;
    }

    uint32_t getLastFrameId() const noexcept {
        return mLastFrameId;
    }

private:
    uint32_t mLastFrameId = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_BUFFERSTUFFINGDETECTOR_H
