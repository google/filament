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

#include <time.h>

#ifndef FILAMENT_LOG_PIPELINE_ESTIMATOR
#define FILAMENT_LOG_PIPELINE_ESTIMATOR 0
#endif

namespace filament {

namespace {
struct PipelineLogger {
    static inline void log(
            uint32_t count, uint32_t countGpu, double zScore,
            double meanMain, double meanApp, double meanRender, double meanBackend, double meanGpu,
            double standardDeviationMain, double standardDeviationRender, double standardDeviationBackend, double standardDeviationGpu,
            double effectiveMain, double effectiveBackend, double effectiveGpu,
            float idealFrameRate, uint32_t idealLatency, double safeDelayNs) noexcept {
#if FILAMENT_LOG_PIPELINE_ESTIMATOR
        LOG(INFO) << "FramePipelineEstimator: "
                << "history=" << count << " (gpu=" << countGpu << "), "
                << "Z=" << zScore << "\n"
                << "  mean   CPU=" << meanMain / 1e6 << "ms (App=" << meanApp / 1e6 << "ms, Render=" << meanRender / 1e6 << "ms), BE=" << meanBackend / 1e6 << "ms, GPU=" << meanGpu / 1e6 << "ms\n"
                << "  stddev CPU=" << standardDeviationMain / 1e6 << "ms (Render=" << standardDeviationRender / 1e6 << "ms), BE=" << standardDeviationBackend / 1e6 << "ms, GPU=" << standardDeviationGpu / 1e6 << "ms\n"
                << "  effect CPU=" << effectiveMain / 1e6 << "ms, BE=" << effectiveBackend / 1e6 << "ms, GPU=" << effectiveGpu / 1e6 << "ms\n"
                << "  ideal  fps=" << idealFrameRate << ", latency=" << idealLatency << ", safeDelay=" << safeDelayNs / 1e6 << "ms";
#endif
    }
};
} // namespace

double FramePipelineEstimator::getZScore(TargetPercentile const targetPercentile) noexcept {
    switch (targetPercentile) {
        case TargetPercentile::P50: return 0.0;
        case TargetPercentile::P90: return 1.282;
        case TargetPercentile::P95: return 1.645;
    }
    return 0.0;
}

FramePipelineEstimator::Estimation FramePipelineEstimator::estimate(FrameInfoHistory history,
        TargetPercentile const targetPercentile,
        std::chrono::nanoseconds targetVsyncInterval) noexcept {
    return estimate(history, getZScore(targetPercentile), targetVsyncInterval);
}

FramePipelineEstimator::Estimation FramePipelineEstimator::estimate(FrameInfoHistory history,
        double const zScore, std::chrono::nanoseconds targetVsyncInterval) noexcept {
    if (history.empty()) {
        return Estimation{};
    }

    uint32_t const count = static_cast<uint32_t>(history.size());
    uint32_t countGpu = 0;

    double sumApp = 0.0;
    double sumRender = 0.0;
    double sumBackend = 0.0;
    double sumGpu = 0.0;

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
    }

    double const meanApp = sumApp / count;
    double const meanRender = sumRender / count;
    double const meanMain = (sumApp + sumRender) / count; // total CPU (endFrame - frameScheduleTime)
    double const meanBackend = sumBackend / count;
    double const meanGpu = countGpu > 0 ? sumGpu / countGpu : 0.0;

    double varianceMain = 0.0;
    double varianceRender = 0.0;
    double varianceBackend = 0.0;
    double varianceGpu = 0.0;

    if (count > 1) {
        double squareSumMain = 0.0;
        double squareSumRender = 0.0;
        double squareSumBackend = 0.0;
        for (uint32_t i = 0; i < count; ++i) {
            auto const& frameInfo = history[i];
            double const durationMain = static_cast<double>(frameInfo.endFrame - getCpuStart(frameInfo));
            squareSumMain += (durationMain - meanMain) * (durationMain - meanMain);

            double const durationRender = static_cast<double>(frameInfo.endFrame - frameInfo.beginFrame);
            squareSumRender += (durationRender - meanRender) * (durationRender - meanRender);

            double const durationBackend = static_cast<double>(frameInfo.backendEndFrame - frameInfo.backendBeginFrame);
            squareSumBackend += (durationBackend - meanBackend) * (durationBackend - meanBackend);
        }
        varianceMain = squareSumMain / (count - 1);
        varianceRender = squareSumRender / (count - 1);
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
    double const standardDeviationRender = std::sqrt(varianceRender);
    double const standardDeviationBackend = std::sqrt(varianceBackend);
    double const standardDeviationGpu = std::sqrt(varianceGpu);

    // Reverted math: CPU total duration (response time) is used for throughput bottleneck.
    double const effectiveMain = std::max(0.0, meanMain + (zScore * standardDeviationMain));
    double const effectiveBackend = std::max(0.0, meanBackend + (zScore * standardDeviationBackend));
    double const effectiveGpu = std::max(0.0, meanGpu + (zScore * standardDeviationGpu));

    double idealFrameTimeNs = std::max({effectiveMain, effectiveBackend, effectiveGpu});
    if (idealFrameTimeNs <= 0.0) {
        idealFrameTimeNs = 16666666.0;
    }

    double const totalTransitTimeNs = effectiveMain + effectiveBackend + effectiveGpu;

    uint32_t idealLatency = static_cast<uint32_t>(std::ceil(totalTransitTimeNs / idealFrameTimeNs));
    if (idealLatency < 1) {
        idealLatency = 1;
    }

    float const idealFrameRate = static_cast<float>(1e9 / idealFrameTimeNs);

    double const budgetNs = idealLatency * static_cast<double>(targetVsyncInterval.count());
    double const safeDelayNs = budgetNs - totalTransitTimeNs;

    PipelineLogger::log(count, countGpu, zScore,
            meanMain, meanApp, meanRender, meanBackend, meanGpu,
            standardDeviationMain, standardDeviationRender, standardDeviationBackend, standardDeviationGpu,
            effectiveMain, effectiveBackend, effectiveGpu,
            idealFrameRate, idealLatency, safeDelayNs);

    Estimation estimation;
    estimation.idealFrameDuration = std::chrono::nanoseconds(static_cast<int64_t>(idealFrameTimeNs));
    estimation.idealFrameRate     = idealFrameRate;
    estimation.idealLatencyFrames = idealLatency;
    estimation.safeDelayDuration  = std::chrono::nanoseconds(
            static_cast<int64_t>(std::max(0.0, safeDelayNs)));

    return estimation;
}

} // namespace filament
