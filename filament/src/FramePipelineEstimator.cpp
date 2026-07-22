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

#include <filament/FramePipelineEstimator.h>

#include <utils/Log.h>

#include <algorithm>
#include <cmath>

#ifndef FILAMENT_LOG_PIPELINE_ESTIMATOR
#define FILAMENT_LOG_PIPELINE_ESTIMATOR 0
#endif

namespace filament {

namespace {
struct TelemetryStats {
    uint32_t count = 0;
    uint32_t countGpu = 0;
    uint32_t countMargin = 0;

    double meanMain = 0.0;
    double meanBackend = 0.0;
    double meanGpu = 0.0;
    double meanMargin = 0.0;

    double stdDevMain = 0.0;
    double stdDevBackend = 0.0;
    double stdDevGpu = 0.0;
    double stdDevMargin = 0.0;

    double effectiveMain = 0.0;
    double effectiveBackend = 0.0;
    double effectiveGpu = 0.0;
    double totalTransitTimeNs = 0.0;
    double effectiveCompositionMarginNs = 0.0;
};

struct PipelineLogger {
    static void log(
            TelemetryStats const& stats, double zScore,
            double pacingIntervalNs, uint32_t idealLatency, double safeDelayNs) noexcept {
#if FILAMENT_LOG_PIPELINE_ESTIMATOR
        LOG(INFO) << "FramePipelineEstimator: "
                << "history=" << stats.count
                << " (gpu=" << stats.countGpu
                << ", margin=" << stats.countMargin << "), "
                << "Z=" << zScore << "\n"
                << "  mean   CPU=" << stats.meanMain / 1e6 << "ms, BE=" << stats.meanBackend / 1e6 << "ms, GPU=" << stats.meanGpu / 1e6 << "ms, Margin=" << stats.meanMargin / 1e6 << "ms\n"
                << "  stddev CPU=" << stats.stdDevMain / 1e6 << "ms, BE=" << stats.stdDevBackend / 1e6 << "ms, GPU=" << stats.stdDevGpu / 1e6 << "ms, Margin=" << stats.stdDevMargin / 1e6 << "ms\n"
                << "  effect CPU=" << stats.effectiveMain / 1e6 << "ms, BE=" << stats.effectiveBackend / 1e6 << "ms, GPU=" << stats.effectiveGpu / 1e6 << "ms, Margin=" << stats.effectiveCompositionMarginNs / 1e6 << "ms\n"
                << "  pacing interval=" << pacingIntervalNs / 1e6 << "ms, latency=" << idealLatency << " frames, safeDelay=" << safeDelayNs / 1e6 << "ms";
#endif
    }
};

TelemetryStats computeStats(FramePipelineEstimator::FrameInfoHistory history, double const zScore) noexcept {
    if (history.empty()) {
        return {};
    }

    uint32_t const count = static_cast<uint32_t>(history.size());
    uint32_t countGpu = 0;
    uint32_t countMargin = 0;

    double sumApp = 0.0;
    double sumRender = 0.0;
    double sumBackend = 0.0;
    double sumGpu = 0.0;
    double sumMargin = 0.0;

    auto const getCpuStart = [](Renderer::FrameInfo const& f) {
        return (f.frameScheduleTime != Renderer::FrameInfo::INVALID && f.frameScheduleTime != 0) ?
                f.frameScheduleTime : f.vsync;
    };

    for (uint32_t i = 0; i < count; ++i) {
        auto const& frameInfo = history[i];
        int64_t const cpuStart = getCpuStart(frameInfo);
        sumApp += static_cast<double>(frameInfo.beginFrame - cpuStart);
        sumRender += static_cast<double>(frameInfo.endFrame - frameInfo.beginFrame);
        sumBackend += static_cast<double>(frameInfo.backendEndFrame - frameInfo.backendBeginFrame);
        if (frameInfo.gpuFrameDuration > 0) {
            sumGpu += static_cast<double>(frameInfo.gpuFrameDuration);
            countGpu++;
        }
        if (frameInfo.displayPresent > 0 && frameInfo.presentDeadline > 0 &&
                frameInfo.displayPresent != Renderer::FrameInfo::INVALID &&
                frameInfo.presentDeadline != Renderer::FrameInfo::INVALID &&
                frameInfo.displayPresent > frameInfo.presentDeadline) {
            sumMargin += static_cast<double>(frameInfo.displayPresent - frameInfo.presentDeadline);
            countMargin++;
        }
    }

    double const meanMain = (sumApp + sumRender) / count;
    double const meanBackend = sumBackend / count;
    double const meanGpu = countGpu > 0 ? sumGpu / countGpu : 0.0;
    double const meanMargin = countMargin > 0 ? sumMargin / countMargin : 0.0;

    double varianceMain = 0.0;
    double varianceBackend = 0.0;
    double varianceGpu = 0.0;
    double varianceMargin = 0.0;

    if (count > 1) {
        double squareSumMain = 0.0;
        double squareSumBackend = 0.0;
        for (uint32_t i = 0; i < count; ++i) {
            auto const& frameInfo = history[i];
            double const durationMain = static_cast<double>(frameInfo.endFrame - getCpuStart(frameInfo));
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

    if (countMargin > 1) {
        double squareSumMargin = 0.0;
        for (uint32_t i = 0; i < count; ++i) {
            auto const& frameInfo = history[i];
            if (frameInfo.displayPresent > 0 && frameInfo.presentDeadline > 0 &&
                    frameInfo.displayPresent != Renderer::FrameInfo::INVALID &&
                    frameInfo.presentDeadline != Renderer::FrameInfo::INVALID &&
                    frameInfo.displayPresent > frameInfo.presentDeadline) {
                double const margin = static_cast<double>(frameInfo.displayPresent - frameInfo.presentDeadline);
                squareSumMargin += (margin - meanMargin) * (margin - meanMargin);
            }
        }
        varianceMargin = squareSumMargin / (countMargin - 1);
    }

    double const standardDeviationMain = std::sqrt(varianceMain);
    double const standardDeviationBackend = std::sqrt(varianceBackend);
    double const standardDeviationGpu = std::sqrt(varianceGpu);
    double const standardDeviationMargin = std::sqrt(varianceMargin);

    double const effectiveMain = std::max(0.0, meanMain + (zScore * standardDeviationMain));
    double const effectiveBackend = std::max(0.0, meanBackend + (zScore * standardDeviationBackend));
    double const effectiveGpu = std::max(0.0, meanGpu + (zScore * standardDeviationGpu));
    double const totalTransitTimeNs = effectiveMain + effectiveBackend + effectiveGpu;
    double const effectiveCompositionMargin = std::max(0.0, meanMargin + (zScore * standardDeviationMargin));

    return {
        count,
        countGpu,
        countMargin,
        meanMain,
        meanBackend,
        meanGpu,
        meanMargin,
        standardDeviationMain,
        standardDeviationBackend,
        standardDeviationGpu,
        standardDeviationMargin,
        effectiveMain,
        effectiveBackend,
        effectiveGpu,
        totalTransitTimeNs,
        effectiveCompositionMargin
    };
}
} // namespace

double FramePipelineEstimator::getZScore(TargetPercentile const targetPercentile) noexcept {
    switch (targetPercentile) {
        case TargetPercentile::P50: return 0.0;
        case TargetPercentile::P90: return 1.282;
        case TargetPercentile::P95: return 1.645;
    }
    return 0.0;
}

FramePipelineEstimator::Workload FramePipelineEstimator::estimateWorkload(FrameInfoHistory history,
        TargetPercentile const targetPercentile) noexcept {
    return estimateWorkload(history, getZScore(targetPercentile));
}

FramePipelineEstimator::Workload FramePipelineEstimator::estimateWorkload(FrameInfoHistory history,
        double const zScore) noexcept {
    if (history.empty()) {
        return Workload{};
    }

    TelemetryStats const stats = computeStats(history, zScore);
    double idealFrameTimeNs = std::max({stats.effectiveMain, stats.effectiveBackend, stats.effectiveGpu});
    if (idealFrameTimeNs <= 0.0) {
        idealFrameTimeNs = 16666666.0;
    }

    Workload workload;
    workload.idealFrameDuration = std::chrono::nanoseconds(static_cast<int64_t>(idealFrameTimeNs));
    workload.idealFrameRate = static_cast<float>(1e9 / idealFrameTimeNs);
    return workload;
}

FramePipelineEstimator::PacingSizing FramePipelineEstimator::estimatePacing(FrameInfoHistory history,
        std::chrono::nanoseconds pacingPeriod,
        TargetPercentile const targetPercentile) noexcept {
    return estimatePacing(history, pacingPeriod, getZScore(targetPercentile));
}

FramePipelineEstimator::PacingSizing FramePipelineEstimator::estimatePacing(FrameInfoHistory history,
        std::chrono::nanoseconds pacingPeriod,
        double const zScore) noexcept {
    if (history.empty() || pacingPeriod.count() <= 0) {
        return PacingSizing{};
    }

    TelemetryStats const stats = computeStats(history, zScore);

    double const pacingIntervalNs = static_cast<double>(pacingPeriod.count());
    uint32_t idealLatency = static_cast<uint32_t>(
            std::ceil((stats.totalTransitTimeNs + stats.effectiveCompositionMarginNs) / pacingIntervalNs));
    if (idealLatency < 1) {
        idealLatency = 1;
    }

    double const budgetNs = idealLatency * pacingIntervalNs - stats.effectiveCompositionMarginNs;
    double const safeDelayNs = budgetNs - stats.totalTransitTimeNs;

#if FILAMENT_LOG_PIPELINE_ESTIMATOR
    PipelineLogger::log(stats, zScore, pacingIntervalNs, idealLatency, safeDelayNs);
#endif

    PacingSizing sizing;
    sizing.latencyFrames = idealLatency;
    sizing.safeDelayDuration = std::chrono::nanoseconds(
            static_cast<int64_t>(std::max(0.0, safeDelayNs)));
    return sizing;
}

} // namespace filament
