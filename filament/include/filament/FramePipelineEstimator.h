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
    static double getZScore(TargetPercentile const targetPercentile) noexcept {
        switch (targetPercentile) {
            case TargetPercentile::P50: return 0.0;
            case TargetPercentile::P90: return 1.282;
            case TargetPercentile::P95: return 1.645;
        }
        return 0.0;
    }

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
            std::chrono::nanoseconds targetVsyncInterval = std::chrono::nanoseconds(16666666)) noexcept {
        return estimate(history, getZScore(targetPercentile), targetVsyncInterval);
    }

    /**
     * Evaluates historical frame telemetry using a custom standard normal distribution Z-score.
     *
     * @param history Contiguous slice of past Renderer::FrameInfo records.
     * @param zScore Normal distribution Z-score multiplier for standard deviation.
     * @param targetVsyncInterval Target VSYNC cadence frame duration (defaults to 16.67ms).
     * @return Recommended Estimation containing frame duration, refresh rate, structural latency, and safe delay.
     */
    static Estimation estimate(FrameInfoHistory history, double const zScore,
            std::chrono::nanoseconds targetVsyncInterval = std::chrono::nanoseconds(16666666)) noexcept {
        if (history.empty()) {
            return Estimation{};
        }

        uint32_t const count = static_cast<uint32_t>(history.size());
        uint32_t countGpu = 0;

        double sumMain = 0.0;
        double sumBackend = 0.0;
        double sumGpu = 0.0;

        for (uint32_t i = 0; i < count; ++i) {
            auto const& frameInfo = history[i];
            sumMain += static_cast<double>(frameInfo.endFrame - frameInfo.vsync);
            sumBackend += static_cast<double>(frameInfo.backendEndFrame - frameInfo.backendBeginFrame);
            if (frameInfo.gpuFrameDuration > 0) {
                sumGpu += static_cast<double>(frameInfo.gpuFrameDuration);
                countGpu++;
            }
        }

        double const meanMain = sumMain / count;
        double const meanBackend = sumBackend / count;
        double const meanGpu = countGpu > 0 ? sumGpu / countGpu : 0.0;

        double varianceMain = 0.0;
        double varianceBackend = 0.0;
        double varianceGpu = 0.0;

        if (count > 1) {
            double squareSumMain = 0.0;
            double squareSumBackend = 0.0;
            for (uint32_t i = 0; i < count; ++i) {
                auto const& frameInfo = history[i];
                double const durationMain = static_cast<double>(frameInfo.endFrame - frameInfo.vsync);
                squareSumMain += (durationMain - meanMain) * (durationMain - meanMain);

                double const durationBackend = static_cast<double>(frameInfo.backendEndFrame - frameInfo.backendBeginFrame);
                squareSumBackend += (durationBackend - meanBackend) * (durationBackend - meanBackend);
            }
            varianceMain = squareSumMain / (count - 1);
            varianceBackend = squareSumBackend / (count - 1);
        }

        if (countGpu > 1) {
            double squareSumGpu = 0.0;
            for (uint32_t i = 0; i < count; ++i) {
                auto const& frameInfo = history[i];
                if (frameInfo.gpuFrameDuration > 0) {
                    double const durationGpu = static_cast<double>(frameInfo.gpuFrameDuration);
                    squareSumGpu += (durationGpu - meanGpu) * (durationGpu - meanGpu);
                }
            }
            varianceGpu = squareSumGpu / (countGpu - 1);
        }

        double const standardDeviationMain = std::sqrt(varianceMain);
        double const standardDeviationBackend = std::sqrt(varianceBackend);
        double const standardDeviationGpu = std::sqrt(varianceGpu);

        // Step B: Calculate Effective Workload per Stage
        // EffectiveWork = mean + (Z-Score * stdDev)
        double const effectiveMain = std::max(0.0, meanMain + (zScore * standardDeviationMain));
        double const effectiveBackend = std::max(0.0, meanBackend + (zScore * standardDeviationBackend));
        double const effectiveGpu = std::max(0.0, meanGpu + (zScore * standardDeviationGpu));

        // Step C: Calculate Ideal Throughput (Frame Time)
        // IdealFrameTime = max(EffectiveWork_main, EffectiveWork_backend, EffectiveWork_gpu)
        double idealFrameTimeNs = std::max({effectiveMain, effectiveBackend, effectiveGpu});
        if (idealFrameTimeNs <= 0.0) {
            idealFrameTimeNs = 16666666.0;
        }

        // Step D: Calculate Ideal Structural Latency (Pipeline Depth)
        // TotalTransitTime = EffectiveWork_main + EffectiveWork_backend + EffectiveWork_gpu
        double const totalTransitTimeNs = effectiveMain + effectiveBackend + effectiveGpu;

        // IdealLatencyFrames = ceil(TotalTransitTime / IdealFrameTime)
        uint32_t idealLatency = static_cast<uint32_t>(std::ceil(totalTransitTimeNs / idealFrameTimeNs));
        if (idealLatency < 1) {
            idealLatency = 1;
        }

        float const idealFrameRate = static_cast<float>(1e9 / idealFrameTimeNs);

        // Step E: Calculate Maximum Safe Delay (Slack time before CPU work starts)
        // safeDelay = (idealLatency * targetVsyncInterval) - totalTransitTime
        //
        // Note on Conservatism:
        // By summing the individual worst-case stage durations (totalTransitTimeNs) rather than the
        // statistically combined variance (which would use the root-sum-of-squares of their standard
        // deviations), we assume worst-case anomalies can happen in all stages simultaneously during
        // the same frame. This provides a highly robust safety margin, ensuring that delaying starting
        // the frame by this amount will not result in stutters under our target percentile.
        double const budgetNs = idealLatency * static_cast<double>(targetVsyncInterval.count());
        double const safeDelayNs = budgetNs - totalTransitTimeNs;

        Estimation estimation;
        estimation.idealFrameDuration = std::chrono::nanoseconds(static_cast<int64_t>(idealFrameTimeNs));
        estimation.idealFrameRate     = idealFrameRate;
        estimation.idealLatencyFrames = idealLatency;
        estimation.safeDelayDuration  = std::chrono::nanoseconds(
                static_cast<int64_t>(std::max(0.0, safeDelayNs)));

        return estimation;
    }
};


} // namespace filament

#endif // TNT_FILAMENT_FRAMEPIPELINEESTIMATOR_H
