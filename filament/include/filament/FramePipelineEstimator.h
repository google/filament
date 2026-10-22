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

#include <chrono>

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
 *
 * // 1. Estimate unthrottled timing bottleneck (throughput workload limit)
 * auto const workload = FramePipelineEstimator::estimateWorkload(history,
 *         FramePipelineEstimator::TargetPercentile::P90);
 *
 * // 2. Size latency and safe delay for vsync-locked 60 FPS pacing cadence
 * auto const pacingPeriod = std::chrono::nanoseconds(16666666); // 60Hz pacing step
 * auto const sizing = FramePipelineEstimator::estimatePacing(history,
 *         pacingPeriod, FramePipelineEstimator::TargetPercentile::P90);
 *
 * renderer->setDesiredPresentationTime(now + workload.idealFrameDuration);
 * pacer->configure(sizing.latencyFrames, sizing.safeDelayDuration);
 * @endcode
 */
class FramePipelineEstimator {
public:
    /**
     * TargetPercentile specifies the desired statistical confidence interval for workload estimation.
     *
     * The percentile P (e.g., P90 = 90%) represents the probability that a frame's processing
     * time will fall within the estimated workload budget. Conversely, 1 - P represents the
     * theoretical probability of exceeding the budget and missing a frame deadline (jank/stutter):
     *
     * - P50 (Z = 0.0): 50% theoretical miss rate. Expect constant judder (30 drops/sec at 60Hz).
     * - P90 (Z = 1.282): 10% theoretical miss rate. Expect a drop every 10 frames (~6 drops/sec at 60Hz).
     * - P95 (Z = 1.645): 5% theoretical miss rate. Expect a drop every 20 frames (~3 drops/sec at 60Hz).
     *
     * Note: In practice, actual visual stutters are significantly lower because:
     * 1. Frame workloads have temporal coherence (spikes are clustered rather than independent).
     * 2. The FramePacer's structural latency (queue depth) acts as a shock absorber.
     */
    enum class TargetPercentile {
        P50, //!< 50th percentile (mean workload, Z = 0.0)
        P90, //!< 90th percentile (confidence interval, Z = 1.282)
        P95  //!< 95th percentile (high confidence interval, Z = 1.645)
    };

    /**
     * Workload encapsulates the computed ideal throughput recommendation (raw bottleneck).
     */
    struct Workload {
        std::chrono::nanoseconds idealFrameDuration{16666666}; //!< Ideal bottleneck frame duration (Throughput)
        float idealFrameRate = 60.0f;                          //!< Ideal frame rate in Hz (1.0 / idealFrameDuration)
    };

    /**
     * PacingSizing encapsulates the structural latency and CPU delay recommendations relative to a pacing period.
     */
    struct PacingSizing {
        uint32_t latencyFrames = 2;                            //!< Recommended structural latency (Pipeline Depth in frames)
        std::chrono::nanoseconds safeDelayDuration{0};         //!< Maximum safe delay (slack) before starting CPU work
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
    static double getZScore(TargetPercentile targetPercentile) noexcept;

    /**
     * Evaluates historical frame telemetry to estimate raw unthrottled throughput limit.
     *
     * @param history Contiguous slice of past Renderer::FrameInfo records.
     * @param targetPercentile Statistical confidence interval (defaults to P90).
     * @return Estimated Workload.
     */
    static Workload estimateWorkload(FrameInfoHistory history,
            TargetPercentile targetPercentile = TargetPercentile::P90) noexcept;

    /**
     * Evaluates historical frame telemetry to estimate raw unthrottled throughput limit using a custom Z-score.
     *
     * @param history Contiguous slice of past Renderer::FrameInfo records.
     * @param zScore Normal distribution Z-score multiplier for standard deviation.
     * @return Estimated Workload.
     */
    static Workload estimateWorkload(FrameInfoHistory history, double zScore) noexcept;

    /**
     * Evaluates historical frame telemetry to size latency and safe delay for a given pacing period.
     *
     * @param history Contiguous slice of past Renderer::FrameInfo records.
     * @param pacingPeriod Frame interval step at which the pipeline is throttled (e.g. 16.67ms VSYNC period).
     * @param targetPercentile Statistical confidence interval (defaults to P90).
     * @return Estimated latency and safe delay pacing configurations.
     */
    static PacingSizing estimatePacing(FrameInfoHistory history,
            std::chrono::nanoseconds pacingPeriod,
            TargetPercentile targetPercentile = TargetPercentile::P90) noexcept;

    /**
     * Evaluates historical frame telemetry to size latency and safe delay for a given pacing period using a custom Z-score.
     *
     * @param history Contiguous slice of past Renderer::FrameInfo records.
     * @param pacingPeriod Frame interval step at which the pipeline is throttled.
     * @param zScore Normal distribution Z-score multiplier for standard deviation.
     * @return Estimated latency and safe delay pacing configurations.
     */
    static PacingSizing estimatePacing(FrameInfoHistory history,
            std::chrono::nanoseconds pacingPeriod,
            double zScore) noexcept;
};


} // namespace filament

#endif // TNT_FILAMENT_FRAMEPIPELINEESTIMATOR_H
