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

#include <filament/FramePipelineEstimator.h>

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

FFramePacer::FFramePacer(FEngine&, Builder const& builder)
    : mConfig(builder->mConfig) {
}

FFramePacer::~FFramePacer() = default;

void FFramePacer::terminate(FEngine&) noexcept {
}

void FFramePacer::configure(Configuration const& config) {
    FILAMENT_CHECK_PRECONDITION(config.targetFrameRate >= 0.0f)
            << "targetFrameRate must be non-negative";
    FILAMENT_CHECK_PRECONDITION(config.latency > std::chrono::nanoseconds::zero())
            << "latency must be greater than 0";

    if (mConfig.latency != config.latency) {
        mConfigLatencyChanged = true;
    }

    if (mConfig.targetFrameRate != config.targetFrameRate) {
        mLastTargetPresentationTime = {};
    }

    mConfig = config;
}


FFramePacer::TargetStepResult FFramePacer::calculateTargetStep(
        float const targetFrameRate, duration_t const hardwarePeriod) {
    // Calculate ideal frame step based on target FPS

    duration_t targetStep;
    bool exactAchieved = false;

    // Fuzzy Refresh Synchronization and Hardware Capping
    if (targetFrameRate > 0.0f) {
        targetStep = std::chrono::duration_cast<duration_t>(std::chrono::duration<float>(1.0f / targetFrameRate));
        if (targetStep <= hardwarePeriod) {
            // Cap exactly to the display panel's maximum physical refresh capability (e.g. 90 FPS requested on 60Hz)
            targetStep = hardwarePeriod;
            exactAchieved = true;
        } else {
            // Fuzzy Refresh Synchronization: If the user's requested step is extremely close (within 3%) 
            // to an integer multiple of the physical hardware refresh period, snap exactly to the hardware cadence.
            int64_t const targetNanos   = targetStep.count();
            int64_t const hardwareNanos = hardwarePeriod.count();
            int64_t const multiple = (targetNanos + (hardwareNanos / 2)) / hardwareNanos;
            if (multiple > 0) {
                int64_t const snappedNanos = multiple * hardwareNanos;
                int64_t const diffNanos    = std::abs(targetNanos - snappedNanos);
                if (diffNanos < (snappedNanos * 3 / 100)) {
                    targetStep = hardwarePeriod * multiple;
                    exactAchieved = true;
                }
            }
        }
    } else {
        // Explicitly assign native display tracking mode (0.0f)
        targetStep = hardwarePeriod;
        exactAchieved = true;
    }

    return { targetStep, exactAchieved };
}

void FFramePacer::syncExpectedBaseTimeWithVsync(time_point_t const baseTime, duration_t const targetStep) {
    // Anomaly Rejection / Fast-Forward Recovery: If our ideal clock is uninitialized, vastly out of sync,
    // or has fallen behind by at least one full target frame step (e.g. because the GPU was busy or the app skipped frames),
    // instantly snap our ideal rendering anchor forward to match the live incoming physical Vsync tick.
    duration_t const anomalyThreshold = std::max(
            std::chrono::duration_cast<duration_t>(std::chrono::milliseconds(100)),
            targetStep * 2);

    duration_t const clockDiff = baseTime - mExpectedBaseTime;
    duration_t const absClockDiff = std::chrono::abs(clockDiff);

    // Warmup or severe OS stall: Reset history and rigidly re-anchor target base time
    if (UTILS_UNLIKELY(mExpectedBaseTime == time_point_t() || absClockDiff > anomalyThreshold)) {
        mLastTargetPresentationTime = {};
        mExpectedBaseTime = baseTime;
    }
    // Normal CPU lag catch-up: Shift rendering clock forward to sync with the current Vsync pulse
    else if (UTILS_UNLIKELY(clockDiff >= (targetStep - std::chrono::microseconds(100)))) {
        mExpectedBaseTime = baseTime;
    }

    mCurrentFrameBaseTime = mExpectedBaseTime;
}

bool FFramePacer::shouldSkipVsync(time_point_t const baseTime) const {
    /*
     * Vsync skipping logic chooses the Vsync tick closest to our next ideal render time.
     * Example: 30 FPS target (33.3ms step) on 60Hz display (16.6ms period).
     *
     * Vsync Timeline:
     * Vsync 0 (0ms)        Vsync 1 (16.6ms)       Vsync 2 (33.3ms)
     *   |----------------------|----------------------|
     *   ^                                             ^
     *   mExpectedBaseTime=0ms                         mExpectedBaseTime=33.3ms
     *
     * Evaluation when Vsync 1 arrives:
     *                          Vsync 1 (16.6ms)
     *                          |
     *                          |-- diffCurrent -----| (16.6ms)
     *                          v                    v
     *                                               mExpectedBaseTime (33.3ms)
     *                          |-- diffNext -| (0ms)
     *                          v             v
     *                          Vsync 2 (33.3ms)
     *
     * Result: diffNext < diffCurrent (0ms < 16.6ms) is true. Skip Vsync 1!
     */

    // Evaluate if the incoming Vsync tick is the closest Vsync to our next ideal render time
    time_point_t const currentVsync = baseTime;
    time_point_t const nextVsync    = currentVsync + mHardwarePeriod;

    duration_t const diffCurrent = std::chrono::abs(currentVsync - mExpectedBaseTime);
    duration_t const diffNext    = std::chrono::abs(nextVsync - mExpectedBaseTime);

    // If the next Vsync is strictly closer to our ideal render time, we skip this Vsync
    bool const skipped = (diffNext < diffCurrent);

    return skipped;
}

FFramePacer::HardwareTimelineResult FFramePacer::matchHardwareTimeline(
        VsyncTick const& tick, time_point_t projectedPresentation, duration_t const hardwarePeriod) {

    if (UTILS_LIKELY(!tick.timelines.empty())) {
        size_t const lastIdx = tick.timelines.size() - 1;
        time_point_t bestMatch    = tick.timelines[lastIdx].expectedPresentationTime;
        time_point_t bestDeadline = tick.timelines[lastIdx].deadline;
        time_point_t bestAdjusted;
        if (lastIdx > 0) {
            time_point_t prevExpected = tick.timelines[lastIdx - 1].expectedPresentationTime;
            bestAdjusted = prevExpected + (bestMatch - prevExpected) / 2;
        } else {
            bestAdjusted = bestMatch - (hardwarePeriod / 2);
        }

        duration_t minDiff = duration_t::max();

        for (size_t i = 0; i < tick.timelines.size(); ++i) {
            auto const [expectedPresentationTime, deadline] = tick.timelines[i];
            // Filter out timelines whose deadlines have already passed relative to our schedule time
            if (UTILS_UNLIKELY(deadline <= tick.frameScheduleTime)) {
                continue;
            }

            time_point_t expected = expectedPresentationTime;
            duration_t diff = std::chrono::abs(expected - projectedPresentation);
            if (diff < minDiff) {
                minDiff = diff;
                bestMatch    = expected;
                bestDeadline = deadline;
                if (i > 0) {
                    time_point_t prevExpected = tick.timelines[i - 1].expectedPresentationTime;
                    bestAdjusted = prevExpected + (expected - prevExpected) / 2;
                } else {
                    bestAdjusted = expected - (hardwarePeriod / 2);
                }
            }
        }
        return { bestMatch, bestDeadline, bestAdjusted };
    }

    return {
        projectedPresentation,
        projectedPresentation - hardwarePeriod,
        projectedPresentation - (hardwarePeriod / 2)
    };
}

FramePacer::FrameStatus FFramePacer::setupFrame(VsyncTick const& tick) {
    mFrameScheduleTime = tick.frameScheduleTime;
    if (UTILS_LIKELY(tick.vsyncPeriod > duration_t::zero())) {
        mHardwarePeriod = tick.vsyncPeriod;
    }

    time_point_t const syntheticBaseTime = tick.baseTime;

    auto const [targetStep, exactAchieved] = calculateTargetStep(mConfig.targetFrameRate, mHardwarePeriod);

    // If the target pacing step changes significantly (e.g., dynamic display refresh rate change or
    // configuration update), we must immediately flush historical presentation tracking and reset the
    // ideal base clock alignment. Setting mConfigLatencyChanged enforces rigid anchoring on the next frame
    // to establish a clean phase alignment matching the new refresh cadence.
    if (UTILS_UNLIKELY(std::chrono::abs(targetStep - mActiveTargetStep) > std::chrono::microseconds(100))) {
        mExpectedBaseTime = syntheticBaseTime;
        mLastTargetPresentationTime = {};
        mConfigLatencyChanged = true;
    }

    mActiveTargetStep = targetStep;
    mExactFrameRateAchieved = exactAchieved;

    syncExpectedBaseTimeWithVsync(syntheticBaseTime, targetStep);

    if (UTILS_UNLIKELY(shouldSkipVsync(syntheticBaseTime))) {
        return FrameStatus::SKIPPED_SPURIOUS;
    }

    // Approved for rendering
    time_point_t projectedPresentation;

    // Relative Presentation Pacing (Shock Absorption):
    // If we've already warmed up and achieved an exact target step, we accumulate presentation
    // iteratively. This compresses the queue depth dynamically to absorb CPU scheduling delays.
    if (UTILS_LIKELY(exactAchieved && !mConfigLatencyChanged &&
            mLastTargetPresentationTime.time_since_epoch().count() > 0)) {
        projectedPresentation = mLastTargetPresentationTime + targetStep;
    } else {
        // Rigid Anchoring:
        // Used for initial warmup, inexact framerates (to prevent quantization drag), or 
        // after explicit resets from Config/Underflow.
        projectedPresentation = mExpectedBaseTime + mConfig.latency;
        mConfigLatencyChanged = false;
    }

    auto [candidatePresentation, candidateDeadline, candidateAdjusted] =
            matchHardwareTimeline(tick, projectedPresentation, mHardwarePeriod);

    // Buffer Underflow Detection:
    // If the matched timeline's deadline is in the past, it's physically impossible to hit.
    // This means the CPU starved the pipeline and completely drained the buffer queue.
    if (UTILS_UNLIKELY(candidateDeadline <= tick.frameScheduleTime)) {
        // Abandon Relative Presentation Pacing and rigidly reset the queue depth
        mLastTargetPresentationTime = {};
        projectedPresentation = mExpectedBaseTime + mConfig.latency;

        // Re-match with the rigidly reset anchor
        std::tie(candidatePresentation, candidateDeadline, candidateAdjusted) =
                matchHardwareTimeline(tick, projectedPresentation, mHardwarePeriod);
    }

    // Calculate the drift between our projected timeline and the actual matched timeline
    duration_t const snapOffset = candidatePresentation - projectedPresentation;

    // Advance our ideal render time for the upcoming frame cycle.
    if (UTILS_LIKELY(exactAchieved)) {
        mExpectedBaseTime = syntheticBaseTime + targetStep;
    } else {
        mExpectedBaseTime += targetStep;
    }

    // Apply the timeline snap to our internal clock to permanently prevent drift
    mExpectedBaseTime += snapOffset;

    // Monotonic Presentation Guard:
    // If the candidate presentation timestamp is less than or equal to the presentation time
    // of the most recently scheduled frame (e.g. due to user reducing latency or timeline aliasing),
    // skip scheduling this frame entirely to prevent non-monotonic presentation or out-of-order rendering.
    if (UTILS_UNLIKELY(mLastTargetPresentationTime != time_point_t() &&
            candidatePresentation <= mLastTargetPresentationTime)) {
        return FrameStatus::SKIPPED_STALE;
    }

    mTargetPresentationTime     = candidatePresentation;
    mRenderingDeadline          = candidateDeadline;
    mAdjustedPresentation       = candidateAdjusted;
    mLastTargetPresentationTime = candidatePresentation;

    return FrameStatus::ACCEPTED;
}

bool FFramePacer::hasGpuFallenBehind(FRenderer const* renderer) {
    if (UTILS_UNLIKELY(renderer->hasGpuFallenBehind())) {
        mLastTargetPresentationTime = {};
        return true;
    }
    return false;
}

void FFramePacer::applyPresentationTime(FRenderer* renderer) const {
    // In order for Android's SurfaceFlinger to latch the buffer reliably for our exact target VSYNC deadline,
    // the presentation timestamp given to `setPresentationTime` must fall well within the preceding refresh period.
    // We aim for the middle of the chosen timeline and the one that presents immediately before it.
    renderer->setPresentationTime(mAdjustedPresentation);
    renderer->setRenderingDeadline(mRenderingDeadline);
    renderer->setDesiredPresentationTime(mTargetPresentationTime);
    renderer->setFrameScheduleTime(mFrameScheduleTime);

    if constexpr (false) {
        LOG(INFO) << "Expected latency: "
                << std::chrono::duration<float, std::milli>(mTargetPresentationTime - mCurrentFrameBaseTime).count()
                << " ms, config latency: "
                << std::chrono::duration<float, std::milli>(mConfig.latency).count()
                << " ms";
    }

    if constexpr (false) {
        auto estimation = FramePipelineEstimator::estimate(
                renderer->getFrameInfoHistory(renderer->getMaxFrameHistorySize()),
                FramePipelineEstimator::TargetPercentile::P90,
                mActiveTargetStep);

        LOG(INFO) << "ideal fps = " << 1000000000.0 / estimation.idealFrameDuration.count()
                << ", " << estimation.idealLatencyFrames << "-frame latency, safe delay = "
                << estimation.safeDelayDuration.count() / 1000000.0 << " ms";
    }
}

FramePacer::time_point_t FFramePacer::getExpectedPresentationTime() const noexcept {
    return mTargetPresentationTime;
}

FramePacer::time_point_t FFramePacer::getRenderingDeadline() const noexcept {
    return mRenderingDeadline;
}

std::chrono::nanoseconds FFramePacer::getEffectiveLatency() const noexcept {
    if (mTargetPresentationTime == time_point_t()) {
        return mConfig.latency;
    }
    return mTargetPresentationTime - mCurrentFrameBaseTime;
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

    if (UTILS_UNLIKELY(data == nullptr)) {
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
