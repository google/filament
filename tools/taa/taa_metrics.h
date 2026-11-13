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

#ifndef TNT_TAA_METRICS_H
#define TNT_TAA_METRICS_H

#include <image/LinearImage.h>

#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace filament::tools::taa {

constexpr char const* kBenchmarkProtocolVersion = "2.1";

enum class ReferenceFilter : uint8_t {
    BOX,
    LANCZOS,
    LANCZOS_FILAMENT,
};

struct CapturePose {
    math::mat4 viewProj;
    math::mat4 invViewProj;
    math::mat4 objectTransform;
    math::mat4 invObjectTransform;
    math::double3 eye = {};
    math::double3 target = {};
    double radius = 1.0;
    double distance = 1.0;
    bool valid = false;
};

struct LaplacianComparison {
    double detailTransfer = 0.0;
    double spuriousHfRms = 0.0;
};

struct HotspotMetrics {
    double top10Rmse = 0.0;
    double peakRmse = 0.0;
};

struct SubpixelRingMetrics {
    double ringStd[4] = {};
    double rms = 0.0;
};

struct MotionCurveMetrics {
    double edgeGhostPeak = 0.0;
    double edgeTailError = 0.0;
    double edgeEndRmse = 0.0;
    double edgeRecoveryFrames = 0.0;
};

// Jitter & reference reconstruction filter kernels.
float haltonSample(unsigned int i, unsigned int b) noexcept;
float filterRadius(ReferenceFilter filter) noexcept;
float sinc(float x) noexcept;
float lanczos2(float x) noexcept;
float lanczos2Filament(float x) noexcept;
float filterWeight(math::float2 offset, ReferenceFilter filter) noexcept;
math::float2 haltonJitter(size_t index, ReferenceFilter filter) noexcept;
char const* referenceFilterName(ReferenceFilter filter) noexcept;

// Image decoding, luminance, and masks.
float srgbToLinear(float c) noexcept;
image::LinearImage decode(uint8_t const* rgba, uint32_t w, uint32_t h);
float luma(float const* p) noexcept;
double sampleLumaBilinear(image::LinearImage const& img, double sx, double sy) noexcept;
std::vector<uint8_t> edgeMask(image::LinearImage const& reference, float threshold);
std::vector<uint8_t> foregroundMask(image::LinearImage const& reference,
        std::vector<uint8_t> const& silhouetteMask);
image::LinearImage maskImage(std::vector<uint8_t> const& mask, uint32_t w, uint32_t h);
image::LinearImage differenceImage(image::LinearImage const& a, image::LinearImage const& b,
        float gain);

// Spatial accuracy, alignment, detail, gradient, and hotspot metrics.
double mseToDb(double mse) noexcept;
double sortedPercentile(std::vector<double> const& sorted, double q) noexcept;
void computeDistributionStats(std::vector<double> values, double* outP50, double* outP95,
        double* outIqr, double* outMean, double* outStddev);
double rmse(image::LinearImage const& a, image::LinearImage const& b,
        std::vector<uint8_t> const* mask = nullptr) noexcept;
double shiftedRmse(image::LinearImage const& a, image::LinearImage const& b,
        std::vector<uint8_t> const& mask, double dx, double dy) noexcept;
math::double3 referenceAlignment(image::LinearImage const& capture,
        image::LinearImage const& reference, std::vector<uint8_t> const& mask) noexcept;
double highFrequencyEnergy(image::LinearImage const& img,
        std::vector<uint8_t> const* mask = nullptr) noexcept;
LaplacianComparison compareLaplacians(image::LinearImage const& capture,
        image::LinearImage const& reference, std::vector<uint8_t> const* mask = nullptr) noexcept;
double computeGmsd(image::LinearImage const& capture, image::LinearImage const& reference,
        std::vector<uint8_t> const* mask = nullptr);
HotspotMetrics computeInnerGridHotspots(image::LinearImage const& capture,
        image::LinearImage const& reference, std::vector<uint8_t> const& fgMask);

// Temporal stability metrics.
double temporalVariance(std::vector<image::LinearImage> const& frames,
        std::vector<uint8_t> const* mask = nullptr) noexcept;
double topFractionTemporalRms(std::vector<image::LinearImage> const& captures,
        std::vector<image::LinearImage> const* references, double fraction = 0.01);
SubpixelRingMetrics subpixelRingFlicker(std::vector<image::LinearImage> const& captures) noexcept;

// Motion transient, recovery curve, and analytical backward-motion wake ghosting metrics.
MotionCurveMetrics computeMotionCurveMetrics(std::vector<double> const& edgeErrors,
        size_t motionFrames) noexcept;
double computeWakeGhostingFrame(bool isStatic, bool isSpinObject,
        CapturePose const* poseCurr, CapturePose const* posePrev,
        image::LinearImage const& capture, image::LinearImage const& refCurr,
        image::LinearImage const& refPrev, std::vector<uint8_t> const& currEdgeMask,
        std::vector<uint8_t> const& prevEdgeMask) noexcept;

// Normative protocol SHA-256 hashing.
std::string sha256Hex(std::string_view input);

} // namespace filament::tools::taa

#endif // TNT_TAA_METRICS_H
