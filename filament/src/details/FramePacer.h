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

#ifndef TNT_FILAMENT_DETAILS_FRAMEPACER_H
#define TNT_FILAMENT_DETAILS_FRAMEPACER_H

#include "downcast.h"

#include <filament/FramePacer.h>

#include <chrono>

namespace filament {

class FEngine;
class FRenderer;

struct FramePacer::BuilderDetails {
    Configuration mConfig;
};

class FFramePacer : public FramePacer {
public:
    FFramePacer(FEngine& engine, Builder const& builder);
    ~FFramePacer();

    void terminate(FEngine& engine) noexcept;

    void configure(Configuration const& config);
    Configuration const& getConfiguration() const noexcept { return mConfig; }
    FrameStatus setupFrame(VsyncTick const& tick);
    bool hasGpuFallenBehind(FRenderer const* renderer);
    void applyPresentationTime(FRenderer* renderer) const;
    time_point_t getExpectedPresentationTime() const noexcept;
    time_point_t getRenderingDeadline() const noexcept;
    std::chrono::nanoseconds getEffectiveLatency() const noexcept;

    float getSelectedFrameRate() const noexcept;
    bool isExactFrameRateAchieved() const noexcept;

private:
    using TargetStepResult = std::pair<duration_t, bool>;
    using HardwareTimelineResult = std::tuple<time_point_t, time_point_t, time_point_t>;

    Configuration mConfig;
    duration_t mHardwarePeriod = std::chrono::duration_cast<duration_t>(
            std::chrono::duration<float>(1.0f / 60.0f));
    duration_t mActiveTargetStep = mHardwarePeriod;
    time_point_t mExpectedBaseTime;
    time_point_t mCurrentFrameBaseTime;
    time_point_t mTargetPresentationTime;
    time_point_t mRenderingDeadline;
    time_point_t mAdjustedPresentation;
    time_point_t mLastTargetPresentationTime;
    time_point_t mFrameScheduleTime;
    bool mExactFrameRateAchieved = true;
    bool mConfigLatencyChanged = false;

    // Returns { targetStep, exactAchieved }
    static TargetStepResult calculateTargetStep(float targetFrameRate, duration_t hardwarePeriod);
    void syncExpectedBaseTimeWithVsync(time_point_t baseTime, duration_t targetStep);
    bool shouldSkipVsync(time_point_t baseTime) const;

    // Returns { candidatePresentation, candidateDeadline, candidateAdjustedPresentation }
    static HardwareTimelineResult matchHardwareTimeline(VsyncTick const& tick, time_point_t projectedPresentation, duration_t hardwarePeriod);
};
FILAMENT_DOWNCAST(FramePacer)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_FRAMEPACER_H
