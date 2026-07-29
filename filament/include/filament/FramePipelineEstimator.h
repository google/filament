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

#ifndef TNT_FILAMENT_FRAMEPIPELINEESTIMATOR_H
#define TNT_FILAMENT_FRAMEPIPELINEESTIMATOR_H

#include <filament/Renderer.h>

#include <utils/Slice.h>

#include <algorithm>
#include <chrono>
#include <cmath>

#include <stddef.h>
#include <stdint.h>

namespace filament {

/**
 * FramePipelineEstimator calculates the ideal refresh rate (throughput) and ideal structural
 * latency (pipeline depth) based on historical Renderer::FrameInfo telemetry.
 *
 * It implements a probabilistic timing model over the three primary rendering stages (Main CPU,
 * Backend driver, GPU execution) using historical mean (mu) and standard deviation (sigma):
 *
 * 1. Determine Z-Score: Maps the target confidence interval (e.g. P90 = 1.282) to a normal Z-score.
 * 2. Effective Workload: Compute worst-case execution time per stage: mu + (Z * sigma).
 * 3. Ideal Throughput: Pipeline frame time is bounded entirely by the slowest individual stage.
 * 4. Ideal Structural Latency: Ceiling division of total transit time by bottleneck frame time.
 *
 * @code
 * auto const history = renderer->getFrameInfoHistory(renderer->getMaxFrameHistorySize());
 * auto const estimation = FramePipelineEstimator::estimate(history,
 *         FramePipelineEstimator::TargetPercentile::P90);
 *
 * renderer->setDesiredPresentationTime(now + estimation.idealFrameDuration);
 * @endcode
 */
class FramePipelineEstimator {
public:
    /**
     * TargetPercentile specifies the desired statistical confidence interval for workload estimation.
     */
    enum class TargetPercentile {
        P50, //!< 50th percentile (mean workload, Z = 0.0)
        P90, //!< 90th percentile (confidence interval, Z = 1.282)
        P95  //!< 95th percentile (high confidence interval, Z = 1.645)
    };

    /**
     * Estimation encapsulates the computed ideal throughput and pipeline latency recommendations.
     */
    struct Estimation {
        std::chrono::nanoseconds idealFrameDuration{16666666}; //!< Ideal bottleneck frame duration (Throughput)
        float idealFrameRate = 60.0f;                          //!< Ideal frame rate in Hz (1.0 / idealFrameDuration)
        uint32_t idealLatencyFrames = 2;                       //!< Ideal structural latency (Pipeline Depth in frames)
        std::chrono::nanoseconds safeDelayDuration{0};         //!< Maximum safe delay (slack) before starting CPU work (Touch latency optimization)
    };

    /**
     * FrameInfoHistory represents a contiguous slice of historical FrameInfo telemetry.
     */
    using FrameInfoHistory = utils::Slice<const Renderer::FrameInfo>;

    FramePipelineEstimator() = delete;

    /**
     * Converts a TargetPercentile enum to its corresponding normal distribution Z-score.
     *
     * @param targetPercentile Desired confidence interval percentile.
     * @return Standard normal distribution Z-score.
     */
    static double getZScore(TargetPercentile const targetPercentile) noexcept;

    /**
     * Evaluates historical frame telemetry to recommend ideal throughput and latency sizing.
     *
     * @param history Contiguous slice of past Renderer::FrameInfo records.
     * @param targetPercentile Statistical confidence interval (defaults to P90).
     * @param targetVsyncInterval Target VSYNC cadence frame duration (defaults to 16.67ms).
     * @return Recommended Estimation containing frame duration, refresh rate, structural latency, and safe delay.
     */
    static Estimation estimate(FrameInfoHistory history,
            TargetPercentile const targetPercentile = TargetPercentile::P90,
            std::chrono::nanoseconds targetVsyncInterval = std::chrono::nanoseconds(16666666)) noexcept;

    /**
     * Evaluates historical frame telemetry using a custom standard normal distribution Z-score.
     *
     * @param history Contiguous slice of past Renderer::FrameInfo records.
     * @param zScore Normal distribution Z-score multiplier for standard deviation.
     * @param targetVsyncInterval Target VSYNC cadence frame duration (defaults to 16.67ms).
     * @return Recommended Estimation containing frame duration, refresh rate, structural latency, and safe delay.
     */
    static Estimation estimate(FrameInfoHistory history, double const zScore,
            std::chrono::nanoseconds targetVsyncInterval = std::chrono::nanoseconds(16666666)) noexcept;
};


} // namespace filament

#endif // TNT_FILAMENT_FRAMEPIPELINEESTIMATOR_H
