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

#include <utils/compiler.h>

#include <chrono>

namespace filament {

class FEngine;
class FRenderer;

struct FramePacer::BuilderDetails {
    FramePacer::Configuration mConfig;
};

class FFramePacer : public FramePacer {
public:
    FFramePacer(FEngine& engine, const Builder& builder);
    ~FFramePacer();

    void terminate(FEngine& engine) noexcept;

    void configure(const Configuration& config);
    bool setupFrame(const VsyncTick& tick);
    bool hasGpuFallenBehind(FRenderer* renderer);
    void applyPresentationTime(FRenderer* renderer) const;
    time_point_t getExpectedPresentationTime() const noexcept;
    time_point_t getRenderingDeadline() const noexcept;

    float getSelectedFrameRate() const noexcept;
    bool isExactFrameRateAchieved() const noexcept;

private:
    FramePacer::Configuration mConfig;
    duration_t mHardwarePeriod = std::chrono::duration_cast<duration_t>(
            std::chrono::duration<float>(1.0f / 60.0f));
    duration_t mActiveHardwarePeriod = mHardwarePeriod;
    duration_t mActiveTargetStep = mHardwarePeriod;
    time_point_t mNextIdealRenderTime;
    time_point_t mTargetPresentationTime;
    time_point_t mRenderingDeadline;
    time_point_t mLastTargetPresentationTime;
    bool mExactFrameRateAchieved = true;
};

FILAMENT_DOWNCAST(FramePacer)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_FRAMEPACER_H
