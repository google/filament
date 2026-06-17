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

#include "details/FramePacer.h"

#include "details/Engine.h"
#include "details/Renderer.h"

#include <utils/Logger.h>
#include <utils/Panic.h>

#include <vector>

#if defined(__ANDROID__)
#include <android/choreographer.h>
#include <android/log.h>
#endif

namespace filament {

// ------------------------------------------------------------------------------------------------
// Concrete FFramePacer Implementation
// ------------------------------------------------------------------------------------------------

FFramePacer::FFramePacer(FEngine&, const Builder& builder)
    : mConfig(builder->mConfig) {
}

FFramePacer::~FFramePacer() = default;

void FFramePacer::terminate(FEngine&) noexcept {
}

void FFramePacer::configure(const Configuration& config) {
    FILAMENT_CHECK_PRECONDITION(config.targetFrameRate >= 0.0f)
            << "targetFrameRate must be non-negative";
    FILAMENT_CHECK_PRECONDITION(config.latencyFrames > 0)
            << "latencyFrames must be greater than 0";

    mConfig = config;
}

bool FFramePacer::setupFrame(const VsyncTick& tick) {
    // Update the active physical hardware refresh period from platform telemetry
    if (tick.vsyncPeriod > duration_t::zero()) {
        mHardwarePeriod = tick.vsyncPeriod;
        // If the physical display panel refresh rate changes (e.g. dynamic 60Hz -> 120Hz display mode switch),
        // instantly re-anchor our ideal phase to match the new incoming physical hardware Vsync pulse.
        if (std::chrono::abs(tick.vsyncPeriod - mActiveHardwarePeriod) > std::chrono::microseconds(100)) {
            mActiveHardwarePeriod = tick.vsyncPeriod;
            mNextIdealRenderTime  = tick.baseTime;
            mLastTargetPresentationTime = {};
        }
    }

    // 1. Calculate ideal frame step based on target FPS
    duration_t targetStep = mConfig.targetFrameRate > 0.0f ?
        std::chrono::duration_cast<duration_t>(std::chrono::duration<float>(1.0f / mConfig.targetFrameRate)) :
        mHardwarePeriod;

    bool exactAchieved = true;

    // Fuzzy Refresh Synchronization and Hardware Capping
    if (mConfig.targetFrameRate > 0.0f) {
        if (targetStep <= mHardwarePeriod) {
            // Cap exactly to the display panel's maximum physical refresh capability (e.g. 90 FPS requested on 60Hz)
            targetStep = mHardwarePeriod;
            exactAchieved = true;
        } else {
            // Fuzzy Refresh Synchronization: If the user's requested step is extremely close (within 3%) 
            // to an integer multiple of the physical hardware refresh period, snap exactly to the hardware cadence.
            int64_t const targetNanos   = targetStep.count();
            int64_t const hardwareNanos = mHardwarePeriod.count();
            int64_t const multiple = (targetNanos + (hardwareNanos / 2)) / hardwareNanos;
            if (multiple > 0) {
                int64_t const snappedNanos = multiple * hardwareNanos;
                int64_t const diffNanos    = std::abs(targetNanos - snappedNanos);
                if (diffNanos < (snappedNanos * 3 / 100)) {
                    targetStep = mHardwarePeriod * multiple;
                    exactAchieved = true;
                } else {
                    exactAchieved = false;
                }
            } else {
                exactAchieved = false;
            }
        }
    } else {
        // Explicitly assign native display tracking mode (0.0f)
        targetStep = mHardwarePeriod;
        exactAchieved = true;
    }

    mActiveTargetStep = targetStep;
    mExactFrameRateAchieved = exactAchieved;

    // Anomaly Rejection / Fast-Forward Recovery: If our ideal clock is uninitialized, vastly out of sync,
    // or has fallen behind by at least one full target frame step (e.g. because the GPU was busy or the app skipped frames),
    // instantly snap our ideal rendering anchor forward to match the live incoming physical Vsync tick.
    duration_t const anomalyThreshold = std::max(
            std::chrono::duration_cast<duration_t>(std::chrono::milliseconds(100)),
            targetStep * 2);

    duration_t const clockLag = tick.baseTime > mNextIdealRenderTime ?
        std::chrono::duration_cast<duration_t>(tick.baseTime - mNextIdealRenderTime) : duration_t::zero();

    // To prevent micro-jitter near the exact boundary from causing missed latching, we absorb up to 100 microseconds
    if (mNextIdealRenderTime == time_point_t() || 
        clockLag >= (targetStep - std::chrono::microseconds(100)) ||
        std::chrono::abs(tick.baseTime - mNextIdealRenderTime) > anomalyThreshold) {
        if (mNextIdealRenderTime == time_point_t() || std::chrono::abs(tick.baseTime - mNextIdealRenderTime) > anomalyThreshold) {
            mLastTargetPresentationTime = {};
        }
        mNextIdealRenderTime = tick.baseTime;
    }

    // 2. Evaluate if the incoming Vsync tick is the closest Vsync to our next ideal render time
    time_point_t const currentVsync = tick.baseTime;
    time_point_t const nextVsync    = currentVsync + mHardwarePeriod;

    duration_t const diffCurrent = std::chrono::abs(currentVsync - mNextIdealRenderTime);
    duration_t const diffNext    = std::chrono::abs(nextVsync - mNextIdealRenderTime);

    // If the next Vsync is strictly closer to our ideal render time, we skip this Vsync
    bool const skipped = (diffNext < diffCurrent);

    if constexpr (false) {
        LOG(INFO)   << "[FramePacerDebug] setupFrame: targetFps=" << mConfig.targetFrameRate
                    << ", targetStep=" << targetStep.count() / 1000000.0f
                    << "ms, activeHwPeriod=" << mActiveHardwarePeriod.count() / 1000000.0f
                    << "ms, vsyncPeriod=" << tick.vsyncPeriod.count() / 1000000.0f
                    << "ms, exactAchieved=" << (exactAchieved ? "1" : "0")
                    << ", diffCurrent=" << diffCurrent.count() / 1000000.0f
                    << "ms, diffNext=" << diffNext.count() / 1000000.0f
                    << "ms -> " << (skipped ? "SKIPPED" : "APPROVED");
    }

    if (skipped) {
        return false;
    }

    // Approved for rendering
    // 3. Compute the target presentation deadline based on our ideal cadence and pipeline depth
    time_point_t const projectedPresentation = mNextIdealRenderTime + (targetStep * mConfig.latencyFrames);

    // Advance our ideal render time for the upcoming frame cycle.
    // If exact display rate pacing was achieved (e.g. running at native refresh rate or exact sub-multiples),
    // re-anchor our ideal phase directly to the live incoming hardware VSYNC pulse to eliminate long-term open-loop drift.
    if (exactAchieved) {
        mNextIdealRenderTime = tick.baseTime + targetStep;
    } else {
        mNextIdealRenderTime += targetStep;
    }

    time_point_t candidatePresentation;
    time_point_t candidateDeadline;

    // 4. Match against hardware candidate presentation timelines (if available)
    if (!tick.timelines.empty()) {
        time_point_t bestMatch    = tick.timelines[0].expectedPresentationTime;
        time_point_t bestDeadline = tick.timelines[0].deadline;
        duration_t minDiff = duration_t::max();

        for (auto const [expectedPresentationTime, deadline] : tick.timelines) {
            time_point_t expected = expectedPresentationTime;
            duration_t diff = std::chrono::abs(expected - projectedPresentation);
            if (diff < minDiff) {
                minDiff = diff;
                bestMatch    = expected;
                bestDeadline = deadline;
            }
        }
        candidatePresentation = bestMatch;
        candidateDeadline     = bestDeadline;
    } else {
        candidatePresentation = projectedPresentation;
        candidateDeadline     = projectedPresentation - mHardwarePeriod;
    }

    // Monotonic Presentation Guard:
    // If the candidate presentation timestamp is less than or equal to the presentation time
    // of the most recently scheduled frame (e.g. due to user reducing latencyFrames or timeline aliasing),
    // skip scheduling this frame entirely to prevent non-monotonic presentation or out-of-order rendering.
    if (mLastTargetPresentationTime != time_point_t() && candidatePresentation <= mLastTargetPresentationTime) {
        return false;
    }

    mTargetPresentationTime     = candidatePresentation;
    mRenderingDeadline          = candidateDeadline;
    mLastTargetPresentationTime = candidatePresentation;

    return true;
}

bool FFramePacer::hasGpuFallenBehind(FRenderer* renderer) {
    if (renderer->hasGpuFallenBehind()) {
        mLastTargetPresentationTime = {};
        return true;
    }
    return false;
}

void FFramePacer::applyPresentationTime(FRenderer* renderer) const {
    // In order for Android's SurfaceFlinger to latch the buffer reliably for our exact target VSYNC deadline,
    // the presentation timestamp given to `setPresentationTime` must fall well within the preceding refresh period.
    // We adjust the target presentation timestamp backwards by half a hardware refresh period (mHardwarePeriod / 2).
    time_point_t const adjustedPresentation = mTargetPresentationTime - (mHardwarePeriod / 2);
    renderer->setPresentationTime(adjustedPresentation);
    renderer->setRenderingDeadline(mRenderingDeadline);
    renderer->setDesiredPresentationTime(mTargetPresentationTime);
}

FramePacer::time_point_t FFramePacer::getExpectedPresentationTime() const noexcept {
    return mTargetPresentationTime;
}

FramePacer::time_point_t FFramePacer::getRenderingDeadline() const noexcept {
    return mRenderingDeadline;
}

float FFramePacer::getSelectedFrameRate() const noexcept {
    if (mActiveTargetStep > duration_t::zero()) {
        float const seconds = std::chrono::duration<float>(mActiveTargetStep).count();
        return 1.0f / seconds;
    }
    return 0.0f;
}

bool FFramePacer::isExactFrameRateAchieved() const noexcept {
    return mExactFrameRateAchieved;
}

#if defined(__ANDROID__)
std::vector<FramePacer::HardwareTimeline>& FramePacer::extractTimelines(
        std::vector<FramePacer::HardwareTimeline>& out,
        const AChoreographerFrameCallbackData* data) noexcept {
    out.clear();

    if (data == nullptr) {
        return out;
    }

    if (__builtin_available(android 33, *)) {
        size_t count = AChoreographerFrameCallbackData_getFrameTimelinesLength(data);
        out.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            int64_t expectedNanos = AChoreographerFrameCallbackData_getFrameTimelineExpectedPresentationTimeNanos(data, i);
            int64_t deadlineNanos = AChoreographerFrameCallbackData_getFrameTimelineDeadlineNanos(data, i);

            FramePacer::HardwareTimeline timeline;
            timeline.expectedPresentationTime = time_point_t(std::chrono::nanoseconds(expectedNanos));
            timeline.deadline                 = time_point_t(std::chrono::nanoseconds(deadlineNanos));
            out.push_back(timeline);
        }
    }

    return out;
}
#endif

} // namespace filament
