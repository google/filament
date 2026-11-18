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

#include "taa_metrics.h"

#include <image/LinearImage.h>

#include <math/mat4.h>
#include <math/scalar.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace {

using namespace filament::math;
using namespace filament::tools::taa;
using image::LinearImage;

// Note: LinearImage's copy constructor shares pixel memory; `cloneImage` performs a deep copy so
// synthetic perturbations never mutate the reference image in place.
LinearImage cloneImage(LinearImage const& src) {
    LinearImage dst(src.getWidth(), src.getHeight(), src.getChannels());
    size_t const bytes =
            size_t(src.getWidth()) * src.getHeight() * src.getChannels() * sizeof(float);
    std::memcpy(dst.getPixelRef(), src.getPixelRef(), bytes);
    return dst;
}

LinearImage makeSolidImage(uint32_t const w, uint32_t const h, float3 const rgb) {
    LinearImage img(w, h, 3);
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            float* p = img.getPixelRef(x, y);
            p[0] = rgb.x;
            p[1] = rgb.y;
            p[2] = rgb.z;
        }
    }
    return img;
}

void fillRect(LinearImage& img, uint32_t const x0, uint32_t const y0, uint32_t const x1,
        uint32_t const y1, float3 const rgb) {
    uint32_t const w = img.getWidth();
    uint32_t const h = img.getHeight();
    for (uint32_t y = y0; y < std::min(y1, h); y++) {
        for (uint32_t x = x0; x < std::min(x1, w); x++) {
            float* p = img.getPixelRef(x, y);
            p[0] = rgb.x;
            p[1] = rgb.y;
            p[2] = rgb.z;
        }
    }
}

LinearImage boxBlur3x3(LinearImage const& src, uint32_t const rx0, uint32_t const ry0,
        uint32_t const rx1, uint32_t const ry1) {
    uint32_t const w = src.getWidth();
    uint32_t const h = src.getHeight();
    LinearImage dst = cloneImage(src);
    for (uint32_t y = std::max(1u, ry0); y < std::min(h - 1, ry1); y++) {
        for (uint32_t x = std::max(1u, rx0); x < std::min(w - 1, rx1); x++) {
            float sum[3] = { 0.0f, 0.0f, 0.0f };
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    float const* p = src.getPixelRef(uint32_t(int(x) + dx), uint32_t(int(y) + dy));
                    sum[0] += p[0];
                    sum[1] += p[1];
                    sum[2] += p[2];
                }
            }
            float* out = dst.getPixelRef(x, y);
            out[0] = sum[0] / 9.0f;
            out[1] = sum[1] / 9.0f;
            out[2] = sum[2] / 9.0f;
        }
    }
    return dst;
}

CapturePose makeOrthoPose(double const halfExtent, double3 const eye, double3 const target,
        mat4 const& objTransform = mat4()) {
    // Note: mat4::lookAt returns the camera model matrix; inverse() gives the view matrix.
    mat4 const view = inverse(mat4::lookAt(eye, target, double3{ 0.0, 1.0, 0.0 }));
    mat4 const proj = mat4::ortho(-halfExtent, halfExtent, -halfExtent, halfExtent, 0.1, 100.0);
    mat4 const vp = proj * view;
    CapturePose pose;
    pose.viewProj = vp;
    pose.invViewProj = inverse(vp);
    pose.objectTransform = objTransform;
    pose.invObjectTransform = inverse(objTransform);
    pose.eye = eye;
    pose.target = target;
    pose.radius = halfExtent * 0.5;
    pose.distance = length(target - eye);
    pose.valid = true;
    return pose;
}

CapturePose makePerspectivePose(double const fovDeg, double3 const eye, double3 const target,
        mat4 const& objTransform = mat4()) {
    mat4 const view = inverse(mat4::lookAt(eye, target, double3{ 0.0, 1.0, 0.0 }));
    mat4 const proj = mat4::perspective(fovDeg, 1.0, 0.1, 100.0);
    mat4 const vp = proj * view;
    CapturePose pose;
    pose.viewProj = vp;
    pose.invViewProj = inverse(vp);
    pose.objectTransform = objTransform;
    pose.invObjectTransform = inverse(objTransform);
    pose.eye = eye;
    pose.target = target;
    pose.radius = 1.0;
    pose.distance = length(target - eye);
    pose.valid = true;
    return pose;
}

// ================================================================================================
// 1. Reference Filters, Halton Jitter, sRGB Decode & Luminance
// ================================================================================================

TEST(TaaMetricsTest, ReferenceFiltersAndHaltonJitterProperties) {
    EXPECT_FLOAT_EQ(filterRadius(ReferenceFilter::BOX), 0.5f);
    EXPECT_FLOAT_EQ(filterRadius(ReferenceFilter::LANCZOS), 2.0f);
    EXPECT_FLOAT_EQ(filterRadius(ReferenceFilter::LANCZOS_FILAMENT), 2.0f);

    // Exact cardinal points and symmetry for both Lanczos-2 variants.
    EXPECT_FLOAT_EQ(lanczos2(0.0f), 1.0f);
    EXPECT_FLOAT_EQ(lanczos2Filament(0.0f), 1.0f);
    EXPECT_NEAR(lanczos2(1.0f), 0.0f, 1e-6f);
    EXPECT_NEAR(lanczos2Filament(1.0f), 0.0f, 1e-6f);
    EXPECT_FLOAT_EQ(lanczos2(2.0f), 0.0f);
    EXPECT_FLOAT_EQ(lanczos2Filament(2.0f), 0.0f);
    EXPECT_FLOAT_EQ(lanczos2(2.5f), 0.0f);
    EXPECT_FLOAT_EQ(lanczos2Filament(2.5f), 0.0f);

    // Negative lobes in (1, 2) and even symmetry.
    EXPECT_LT(lanczos2(1.5f), 0.0f);
    EXPECT_LT(lanczos2Filament(1.5f), 0.0f);
    EXPECT_FLOAT_EQ(lanczos2(-1.3f), lanczos2(1.3f));
    EXPECT_FLOAT_EQ(lanczos2Filament(-1.3f), lanczos2Filament(1.3f));

    // Separability of 2D filterWeight.
    float2 const off{ 0.4f, -1.2f };
    EXPECT_NEAR(filterWeight(off, ReferenceFilter::LANCZOS),
            lanczos2(off.x) * lanczos2(off.y), 1e-6f);
    EXPECT_NEAR(filterWeight(off, ReferenceFilter::LANCZOS_FILAMENT),
            lanczos2Filament(off.x) * lanczos2Filament(off.y), 1e-6f);

    // Halton jitter must stay within [-radius, +radius] and be zero-mean centered over 256 samples.
    double2 meanBox = {};
    for (size_t i = 0; i < 256; i++) {
        float2 const j = haltonJitter(i, ReferenceFilter::BOX);
        EXPECT_GE(j.x, -0.5f);
        EXPECT_LE(j.x, 0.5f);
        EXPECT_GE(j.y, -0.5f);
        EXPECT_LE(j.y, 0.5f);
        meanBox.x += j.x;
        meanBox.y += j.y;
    }
    meanBox /= 256.0;
    EXPECT_NEAR(meanBox.x, 0.0, 0.01);
    EXPECT_NEAR(meanBox.y, 0.0, 0.01);
}

TEST(TaaMetricsTest, DecodeFlipsBottomUpAndAppliesExactSrgbTransfer) {
    // 2x2 bottom-up RGBA buffer:
    // Row 0 in memory = bottom of screen -> decoded y = 1.
    // Row 1 in memory = top of screen    -> decoded y = 0.
    uint8_t const rgba[2 * 2 * 4] = {
        // Memory row 0 (bottom-left: black, bottom-right: linear toe 10/255)
        0, 0, 0, 255,       10, 10, 10, 128,
        // Memory row 1 (top-left: mid-grey 128/255, top-right: white 255/255)
        128, 0, 0, 0,       255, 255, 255, 64,
    };

    LinearImage const img = decode(rgba, 2, 2);
    ASSERT_EQ(img.getWidth(), 2u);
    ASSERT_EQ(img.getHeight(), 2u);
    ASSERT_EQ(img.getChannels(), 3u);

    // Top-left (0, 0) comes from memory row 1, col 0 (R = 128/255, G = 0, B = 0).
    float const expectedMid = std::pow(((128.0f / 255.0f) + 0.055f) / 1.055f, 2.4f);
    EXPECT_NEAR(img.getPixelRef(0, 0)[0], expectedMid, 1e-6f);
    EXPECT_FLOAT_EQ(img.getPixelRef(0, 0)[1], 0.0f);
    EXPECT_FLOAT_EQ(img.getPixelRef(0, 0)[2], 0.0f);

    // Top-right (1, 0) comes from memory row 1, col 1 (white).
    EXPECT_NEAR(img.getPixelRef(1, 0)[0], 1.0f, 1e-6f);
    EXPECT_NEAR(img.getPixelRef(1, 0)[1], 1.0f, 1e-6f);
    EXPECT_NEAR(img.getPixelRef(1, 0)[2], 1.0f, 1e-6f);

    // Bottom-left (0, 1) comes from memory row 0, col 0 (black).
    EXPECT_FLOAT_EQ(img.getPixelRef(0, 1)[0], 0.0f);

    // Bottom-right (1, 1) comes from memory row 0, col 1 (10/255 <= 0.04045 linear toe).
    float const expectedToe = (10.0f / 255.0f) / 12.92f;
    EXPECT_NEAR(img.getPixelRef(1, 1)[0], expectedToe, 1e-6f);
}

TEST(TaaMetricsTest, LumaAndBilinearSampling) {
    float const rgb[3] = { 1.0f, 0.5f, 0.25f };
    EXPECT_NEAR(luma(rgb), 0.2126f * 1.0f + 0.7152f * 0.5f + 0.0722f * 0.25f, 1e-6f);

    LinearImage img = makeSolidImage(4, 4, { 0.0f, 0.0f, 0.0f });
    fillRect(img, 1, 1, 2, 2, { 1.0f, 1.0f, 1.0f }); // (1,1) has luma 1.0; neighbours 0.0
    EXPECT_NEAR(sampleLumaBilinear(img, 1.0, 1.0), 1.0, 1e-6);
    EXPECT_NEAR(sampleLumaBilinear(img, 0.5, 1.0), 0.5, 1e-6);
    EXPECT_NEAR(sampleLumaBilinear(img, 0.5, 0.5), 0.25, 1e-6);
}

// ================================================================================================
// 2. Silhouette Mask, Foreground Mask & Diff Images
// ================================================================================================

TEST(TaaMetricsTest, EdgeMaskDetectsAndDilatesVerticalStepByOneTexel) {
    // Vertical step between x = 15 (0.1) and x = 16 (0.9) on a 32x32 image.
    LinearImage ref = makeSolidImage(32, 32, { 0.1f, 0.1f, 0.1f });
    fillRect(ref, 16, 0, 32, 32, { 0.9f, 0.9f, 0.9f });

    std::vector<uint8_t> const mask = edgeMask(ref, 0.08f);
    ASSERT_EQ(mask.size(), 32u * 32u);

    // Raw forward-difference gradient |l(x+1,y) - l(x,y)| > 0.08 fires at x = 15.
    // 3x3 dilation expands x = 15 to {14, 15, 16} for interior rows 2..29.
    for (uint32_t y = 2; y < 30; y++) {
        EXPECT_EQ(mask[y * 32 + 13], 0u);
        EXPECT_EQ(mask[y * 32 + 14], 1u);
        EXPECT_EQ(mask[y * 32 + 15], 1u);
        EXPECT_EQ(mask[y * 32 + 16], 1u);
        EXPECT_EQ(mask[y * 32 + 17], 0u);
    }
}

TEST(TaaMetricsTest, ForegroundMaskExcludesSkyboxRampAndIncludesObjectInterior) {
    // Smooth horizontal skybox ramp from (0.1, 0.2, 0.3) at x=0 to (0.3, 0.4, 0.5) at x=31.
    LinearImage ref(32, 32, 3);
    for (uint32_t y = 0; y < 32; y++) {
        for (uint32_t x = 0; x < 32; x++) {
            float const t = float(x) / 31.0f;
            float* p = ref.getPixelRef(x, y);
            p[0] = 0.1f * (1.0f - t) + 0.3f * t;
            p[1] = 0.2f * (1.0f - t) + 0.4f * t;
            p[2] = 0.3f * (1.0f - t) + 0.5f * t;
        }
    }
    // Place a solid object at [10..22) x [10..22) with distinct albedo (0.8, 0.5, 0.1).
    fillRect(ref, 10, 10, 22, 22, { 0.8f, 0.5f, 0.1f });

    std::vector<uint8_t> const sil = edgeMask(ref, 0.08f);
    std::vector<uint8_t> const fg = foregroundMask(ref, sil);

    // Skybox background corners/margins are excluded (0).
    EXPECT_EQ(fg[5 * 32 + 5], 0u);
    EXPECT_EQ(fg[5 * 32 + 27], 0u);
    // Flat interior pixel (16, 16) inside the object is included (1).
    EXPECT_EQ(fg[16 * 32 + 16], 1u);
}

// ================================================================================================
// 3. Spatial RMSE, Reference Alignment Self-Check & Percentile Stats
// ================================================================================================

TEST(TaaMetricsTest, RmseIdentityClosedFormAndMasking) {
    LinearImage const a = makeSolidImage(16, 16, { 0.2f, 0.4f, 0.6f });
    EXPECT_DOUBLE_EQ(rmse(a, a, nullptr), 0.0);

    LinearImage b = makeSolidImage(16, 16, { 0.3f, 0.6f, 0.9f }); // deltas: 0.1, 0.2, 0.3
    double const expected = std::sqrt((0.01 + 0.04 + 0.09) / 3.0);
    EXPECT_NEAR(rmse(a, b, nullptr), expected, 1e-6);

    // Mask selecting only the top-left quadrant where we reset b == a -> masked RMSE == 0.
    std::vector<uint8_t> mask(16 * 16, 0);
    for (uint32_t y = 0; y < 8; y++) {
        for (uint32_t x = 0; x < 8; x++) {
            mask[y * 16 + x] = 1;
            float* pb = b.getPixelRef(x, y);
            pb[0] = 0.2f;
            pb[1] = 0.4f;
            pb[2] = 0.6f;
        }
    }
    EXPECT_DOUBLE_EQ(rmse(a, b, &mask), 0.0);
}

TEST(TaaMetricsTest, MseToDbAndDistributionStatsClosedForm) {
    EXPECT_NEAR(mseToDb(1.0), 0.0, 1e-9);
    EXPECT_NEAR(mseToDb(1e-2), 20.0, 1e-9);
    EXPECT_NEAR(mseToDb(1e-4), 40.0, 1e-9);
    EXPECT_NEAR(mseToDb(0.0), 120.0, 1e-9); // clamped at 1e-12

    std::vector<double> const vals = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    double p50 = 0, p95 = 0, iqr = 0, mean = 0, stddev = 0;
    computeDistributionStats(vals, &p50, &p95, &iqr, &mean, &stddev);
    EXPECT_DOUBLE_EQ(p50, 3.0);
    EXPECT_NEAR(p95, 4.8, 1e-9);
    EXPECT_DOUBLE_EQ(iqr, 2.0);
    EXPECT_DOUBLE_EQ(mean, 3.0);
    EXPECT_NEAR(stddev, std::sqrt(2.0), 1e-9);
}

TEST(TaaMetricsTest, ReferenceAlignmentDetectsSubtexelShift) {
    // Smooth 2D Gaussian blob so bilinear resampling is exact and free of step-aliasing.
    uint32_t const w = 48, h = 48;
    auto evalGaussian = [](double const x, double const y) -> float {
        double const dx = x - 24.0;
        double const dy = y - 24.0;
        return float(std::exp(-(dx * dx + dy * dy) / (2.0 * 4.5 * 4.5)));
    };

    LinearImage ref(w, h, 3);
    LinearImage capAligned(w, h, 3);
    LinearImage capShifted(w, h, 3);
    constexpr double kShiftX = 0.35;
    constexpr double kShiftY = -0.25;
    std::vector<uint8_t> mask(w * h, 1);

    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            float const vr = evalGaussian(double(x), double(y));
            // `shiftedRmse` evaluates b(x + dx, y + dy) against a(x, y): if capture(x, y) =
            // ref(x + kShiftX, y + kShiftY), best alignment offset is (kShiftX, kShiftY).
            float const vs = evalGaussian(double(x) + kShiftX, double(y) + kShiftY);
            for (int c = 0; c < 3; c++) {
                ref.getPixelRef(x, y)[c] = vr;
                capAligned.getPixelRef(x, y)[c] = vr;
                capShifted.getPixelRef(x, y)[c] = vs;
            }
        }
    }

    double3 const aligned = referenceAlignment(capAligned, ref, mask);
    EXPECT_NEAR(aligned.z, 0.0, 1e-6);

    double3 const misaligned = referenceAlignment(capShifted, ref, mask);
    EXPECT_NEAR(misaligned.x, kShiftX, 0.06);
    EXPECT_NEAR(misaligned.y, kShiftY, 0.06);
    EXPECT_GT(misaligned.z, 0.85);
}

// ================================================================================================
// 4. Laplacian Energy, Phase-Correlated Detail Transfer & Spurious HF
// ================================================================================================

TEST(TaaMetricsTest, HighFrequencyEnergyZeroOnLinearPlaneAndExactOnCheckerboard) {
    // A linear ramp f(x, y) = 0.02*x + 0.01*y + 0.1 has zero second derivative everywhere.
    LinearImage plane(16, 16, 3);
    for (uint32_t y = 0; y < 16; y++) {
        for (uint32_t x = 0; x < 16; x++) {
            float const v = 0.1f + 0.02f * float(x) + 0.01f * float(y);
            float* p = plane.getPixelRef(x, y);
            p[0] = p[1] = p[2] = v;
        }
    }
    EXPECT_NEAR(highFrequencyEnergy(plane), 0.0, 1e-10);

    // Checkerboard in {0, 1}: at every interior pixel, |4*1 - 0 - 0 - 0 - 0| = 4 -> L^2 = 16.
    LinearImage checker(16, 16, 3);
    for (uint32_t y = 0; y < 16; y++) {
        for (uint32_t x = 0; x < 16; x++) {
            float const v = ((x + y) & 1u) ? 1.0f : 0.0f;
            float* p = checker.getPixelRef(x, y);
            p[0] = p[1] = p[2] = v;
        }
    }
    EXPECT_NEAR(highFrequencyEnergy(checker), 16.0, 1e-5);
}

TEST(TaaMetricsTest, CompareLaplaciansSeparatesTrueDetailBlurOvershootAndGhostEdges) {
    // Reference has an isolated vertical bar at x = 10 on a 32x32 canvas.
    LinearImage ref = makeSolidImage(32, 32, { 0.2f, 0.2f, 0.2f });
    fillRect(ref, 10, 4, 11, 28, { 0.8f, 0.8f, 0.8f }); // delta = +0.6

    // 1. Identity -> detailTransfer = 1.0, spuriousHfRms = 0.0.
    LaplacianComparison const id = compareLaplacians(ref, ref);
    EXPECT_NEAR(id.detailTransfer, 1.0, 1e-6);
    EXPECT_NEAR(id.spuriousHfRms, 0.0, 1e-6);

    // 2. 50% attenuated amplitude (half-contrast blur) -> detailTransfer = 0.5, spurious = 0.0.
    LinearImage halfDetail = makeSolidImage(32, 32, { 0.2f, 0.2f, 0.2f });
    fillRect(halfDetail, 10, 4, 11, 28, { 0.5f, 0.5f, 0.5f }); // delta = +0.3
    LaplacianComparison const atten = compareLaplacians(halfDetail, ref);
    EXPECT_NEAR(atten.detailTransfer, 0.5, 1e-6);
    EXPECT_NEAR(atten.spuriousHfRms, 0.0, 1e-6);

    // 3. 130% oversharpened amplitude -> detailTransfer = 1.3, spuriousHfRms = 0.3 (overshoot).
    LinearImage overSharp = makeSolidImage(32, 32, { 0.2f, 0.2f, 0.2f });
    fillRect(overSharp, 10, 4, 11, 28, { 0.98f, 0.98f, 0.98f }); // delta = +0.78 = 1.3 * 0.6
    LaplacianComparison const over = compareLaplacians(overSharp, ref);
    EXPECT_NEAR(over.detailTransfer, 1.3, 1e-5);
    EXPECT_NEAR(over.spuriousHfRms, 0.3, 1e-5);

    // 4. Displaced ghost double-edge: 50% at true position x = 10 + 50% ghost copy at x = 20.
    //    Unaligned Laplacian energy increases, but phase-aligned `detailTransfer` stays at 0.5
    //    and `spuriousHfRms` isolates the orthogonal 0.5 ghost residual!
    LinearImage ghosted = cloneImage(halfDetail);
    fillRect(ghosted, 20, 4, 21, 28, { 0.5f, 0.5f, 0.5f });
    LaplacianComparison const ghost = compareLaplacians(ghosted, ref);
    EXPECT_NEAR(ghost.detailTransfer, 0.5, 1e-6);
    EXPECT_NEAR(ghost.spuriousHfRms, 0.5, 1e-6);

    // 5. Inverted phase -> detailTransfer = -1.0.
    LinearImage inverted = makeSolidImage(32, 32, { 0.8f, 0.8f, 0.8f });
    fillRect(inverted, 10, 4, 11, 28, { 0.2f, 0.2f, 0.2f });
    LaplacianComparison const inv = compareLaplacians(inverted, ref);
    EXPECT_NEAR(inv.detailTransfer, -1.0, 1e-5);
}

// ================================================================================================
// 5. Gradient Magnitude Similarity Deviation (GMSD)
// ================================================================================================

TEST(TaaMetricsTest, GmsdZeroOnIdenticalAndMonotonicWithInteriorTextureSmoothing) {
    // 64x64 scene: dark background (0.05) with a [12..52)x[12..52) bright foreground square (0.80)
    // containing gentle interior texture (+/- 0.08) inside [18..46)x[18..46), leaving the outer
    // silhouette [12..18) untouched.
    LinearImage ref = makeSolidImage(64, 64, { 0.05f, 0.05f, 0.05f });
    fillRect(ref, 12, 12, 52, 52, { 0.80f, 0.80f, 0.80f });
    for (uint32_t y = 18; y < 46; y++) {
        for (uint32_t x = 18; x < 46; x++) {
            float const tex = 0.80f + 0.08f * std::sin(float(x) * 1.3f) * std::cos(float(y) * 1.1f);
            float* p = ref.getPixelRef(x, y);
            p[0] = p[1] = p[2] = tex;
        }
    }

    // Outer step silhouette mask (threshold 0.50 > max interior gradient ~0.22, < outer step 0.75).
    std::vector<uint8_t> const outerSil = edgeMask(ref, 0.50f);
    std::vector<uint8_t> const fg = foregroundMask(ref, outerSil);

    EXPECT_DOUBLE_EQ(computeGmsd(ref, ref, &fg), 0.0);

    // Smooth ONLY the deep interior [20..44)x[20..44) so the outer silhouette is 100% untouched.
    LinearImage const mildBlur = boxBlur3x3(ref, 20, 20, 44, 44);
    LinearImage const heavyBlur = boxBlur3x3(boxBlur3x3(mildBlur, 20, 20, 44, 44), 20, 20, 44, 44);

    // Outer silhouette RMSE is completely blind to this interior texture loss (0.0),
    // whereas foreground-masked GMSD strictly increases with blur severity!
    EXPECT_DOUBLE_EQ(rmse(mildBlur, ref, &outerSil), 0.0);
    EXPECT_DOUBLE_EQ(rmse(heavyBlur, ref, &outerSil), 0.0);

    double const gmsdMild = computeGmsd(mildBlur, ref, &fg);
    double const gmsdHeavy = computeGmsd(heavyBlur, ref, &fg);
    EXPECT_GT(gmsdMild, 0.002);
    EXPECT_GT(gmsdHeavy, gmsdMild);

    // Corrupting pixels strictly outside `fg` must not affect masked GMSD.
    LinearImage bgCorrupted = cloneImage(ref);
    fillRect(bgCorrupted, 1, 1, 8, 8, { 1.0f, 0.0f, 1.0f });
    EXPECT_DOUBLE_EQ(computeGmsd(bgCorrupted, ref, &fg), 0.0);
}

// ================================================================================================
// 6. 10x10 Inner-Grid (8x8 Core) Hotspot Pooling
// ================================================================================================

TEST(TaaMetricsTest, HotspotsIgnoreOuterBorderRingAndGateInactiveCells) {
    // 100x100 viewport -> each cell in the 10x10 grid is an exact 10x10 pixel block.
    LinearImage const ref = makeSolidImage(100, 100, { 0.2f, 0.2f, 0.2f });
    LinearImage cap = cloneImage(ref);
    std::vector<uint8_t> fg(100 * 100, 1);

    // Inject huge error (0.8) across ALL 36 outer-ring border cells (r in {0,9} or c in {0,9}).
    for (uint32_t y = 0; y < 100; y++) {
        for (uint32_t x = 0; x < 100; x++) {
            bool const inBorder = (x < 10 || x >= 90 || y < 10 || y >= 90);
            if (inBorder) {
                float* p = cap.getPixelRef(x, y);
                p[0] = p[1] = p[2] = 1.0f;
            }
        }
    }
    HotspotMetrics const borderOnly = computeInnerGridHotspots(cap, ref, fg);
    EXPECT_DOUBLE_EQ(borderOnly.top10Rmse, 0.0);
    EXPECT_DOUBLE_EQ(borderOnly.peakRmse, 0.0);

    // Now test active-cell gating (>= 10 foreground pixels required):
    // Clear fg everywhere, mark 10 inner cells active (fgCount = 100) with error = 0.10, and mark
    // cell (r=5, c=5) inactive with only 9 foreground pixels and huge error = 0.75.
    cap = cloneImage(ref);
    std::fill(fg.begin(), fg.end(), uint8_t(0));
    for (uint32_t c = 1; c <= 8; c++) {
        for (uint32_t y = 10; y < 20; y++) {
            for (uint32_t x = c * 10; x < (c + 1) * 10; x++) {
                fg[y * 100 + x] = 1;
                float* p = cap.getPixelRef(x, y);
                p[0] = p[1] = p[2] = 0.3f; // error = 0.10
            }
        }
    }
    for (uint32_t c = 1; c <= 2; c++) {
        for (uint32_t y = 20; y < 30; y++) {
            for (uint32_t x = c * 10; x < (c + 1) * 10; x++) {
                fg[y * 100 + x] = 1;
                float* p = cap.getPixelRef(x, y);
                p[0] = p[1] = p[2] = 0.3f; // error = 0.10 (total 10 active cells)
            }
        }
    }
    // Inactive cell (r=5, c=5) -> [50..60)x[50..60): 9 fg pixels (< 10), error = 0.75.
    for (uint32_t i = 0; i < 9; i++) {
        fg[(50 + i / 3) * 100 + (50 + i % 3)] = 1;
    }
    fillRect(cap, 50, 50, 60, 60, { 0.95f, 0.95f, 0.95f });

    HotspotMetrics const gated = computeInnerGridHotspots(cap, ref, fg);
    EXPECT_NEAR(gated.peakRmse, 0.10, 1e-6);
    EXPECT_NEAR(gated.top10Rmse, 0.10, 1e-6);

    // Adding the 10th foreground pixel in cell (5, 5) activates it -> becomes the new peak!
    fg[53 * 100 + 50] = 1;
    HotspotMetrics const activated = computeInnerGridHotspots(cap, ref, fg);
    EXPECT_NEAR(activated.peakRmse, 0.75, 1e-6);
    // Top-10 mean is now (0.75 + 9 * 0.10) / 10 = 0.165.
    EXPECT_NEAR(activated.top10Rmse, 0.165, 1e-6);
}

TEST(TaaMetricsTest, HotspotsExactCellP95AndTop10Ranking) {
    LinearImage const ref = makeSolidImage(100, 100, { 0.0f, 0.0f, 0.0f });
    LinearImage cap = cloneImage(ref);
    std::vector<uint8_t> const fg(100 * 100, 1);

    // Populate 12 inner cells with uniform errors 0.01, 0.02, ..., 0.12.
    // Top-10 cells are 0.03..0.12 -> mean = (0.03 + 0.12) * 5 / 10 = 0.075, peak = 0.12.
    for (uint32_t idx = 0; idx < 12; idx++) {
        uint32_t const r = 1 + idx / 8;
        uint32_t const c = 1 + idx % 8;
        float const err = float(idx + 1) * 0.01f;
        fillRect(cap, c * 10, r * 10, (c + 1) * 10, (r + 1) * 10, { err, err, err });
    }
    HotspotMetrics const m = computeInnerGridHotspots(cap, ref, fg);
    EXPECT_NEAR(m.peakRmse, 0.12, 1e-6);
    EXPECT_NEAR(m.top10Rmse, 0.075, 1e-6);
}

// ================================================================================================
// 7. Temporal Stability: Whole-Frame Variance, Top-1% RMS, Residual Drift RMS & SubpixelStar Rings
// ================================================================================================

TEST(TaaMetricsTest, TemporalVarianceAndTop1PctIsolationAndDriftCancellation) {
    // 100x100 = 10,000 pixels over 16 frames.
    // Top 1% = 100 pixels oscillate by +-0.20 around 0.50; remaining 9,900 pixels sit at 0.50.
    std::vector<LinearImage> captures;
    for (size_t k = 0; k < 16; k++) {
        LinearImage f = makeSolidImage(100, 100, { 0.5f, 0.5f, 0.5f });
        float const delta = (k % 2 == 0) ? 0.20f : -0.20f;
        fillRect(f, 10, 10, 20, 20, { 0.5f + delta, 0.5f + delta, 0.5f + delta }); // 10x10 = 100 px
        captures.push_back(std::move(f));
    }

    // Whole-frame variance is 0.01 * (0.20^2) = 0.0004 (diluted 100x in variance, 10x in RMS).
    EXPECT_NEAR(temporalVariance(captures, nullptr), 0.0004, 1e-7);
    // Top-1% RMS recovers the exact undiluted 0.20 shimmer amplitude!
    EXPECT_NEAR(topFractionTemporalRms(captures, nullptr, 0.01), 0.20, 1e-6);

    // Deterministic drift test: translating edge across 16 frames where capture[k] == reference[k].
    std::vector<LinearImage> driftRefs;
    std::vector<LinearImage> driftCapsClean;
    std::vector<LinearImage> driftCapsShimmer;
    for (size_t k = 0; k < 16; k++) {
        LinearImage ref = makeSolidImage(100, 100, { 0.1f, 0.1f, 0.1f });
        fillRect(ref, uint32_t(20 + k), 10, uint32_t(50 + k), 90, { 0.9f, 0.9f, 0.9f });
        LinearImage capShimmer = cloneImage(ref);
        float const d = (k % 2 == 0) ? 0.04f : -0.04f;
        fillRect(capShimmer, 0, 0, 10, 10, { 0.1f + d, 0.1f + d, 0.1f + d }); // 100 px = 1%
        driftCapsClean.push_back(cloneImage(ref));
        driftRefs.push_back(std::move(ref));
        driftCapsShimmer.push_back(std::move(capShimmer));
    }

    // Raw top-1% RMS sees the moving edge (> 0.3), while residual RMS cancels ground-truth motion!
    EXPECT_GT(topFractionTemporalRms(driftCapsClean, nullptr, 0.01), 0.30);
    EXPECT_NEAR(topFractionTemporalRms(driftCapsClean, &driftRefs, 0.01), 0.0, 1e-7);
    EXPECT_NEAR(topFractionTemporalRms(driftCapsShimmer, &driftRefs, 0.01), 0.04, 1e-6);
}

TEST(TaaMetricsTest, SubpixelStarRingsIsolateConcentricAnnuli) {
    constexpr double kRingMidRadius[4] = { 18.0, 28.0, 43.0, 65.0 };
    constexpr double kRingLo[4] = { 14.0, 22.0, 34.0, 52.0 };
    constexpr double kRingHi[4] = { 22.0, 34.0, 52.0, 78.0 };

    for (int targetRing = 0; targetRing < 4; targetRing++) {
        float const amp = 0.05f * float(targetRing + 1);
        std::vector<LinearImage> frames;
        for (size_t k = 0; k < 8; k++) {
            LinearImage img = makeSolidImage(512, 512, { 0.5f, 0.5f, 0.5f });
            float const v = 0.5f + ((k % 2 == 0) ? amp : -amp);
            for (uint32_t y = 190; y < 360; y++) {
                for (uint32_t x = 170; x < 345; x++) {
                    double const r = std::hypot(double(x) - 256.0, double(y) - 275.0);
                    if (r >= kRingLo[targetRing] && r < kRingHi[targetRing]) {
                        float* p = img.getPixelRef(x, y);
                        p[0] = p[1] = p[2] = v;
                    }
                }
            }
            (void) kRingMidRadius;
            frames.push_back(std::move(img));
        }

        SubpixelRingMetrics const res = subpixelRingFlicker(frames);
        for (int b = 0; b < 4; b++) {
            if (b == targetRing) {
                EXPECT_NEAR(res.ringStd[b], double(amp), 1e-5);
            } else {
                EXPECT_NEAR(res.ringStd[b], 0.0, 1e-7);
            }
        }
        // RMS across 4 rings when 1 ring has `amp` and 3 have 0 is sqrt(amp^2 / 4) = amp / 2.
        EXPECT_NEAR(res.rms, double(amp) * 0.5, 1e-5);
    }
}

// ================================================================================================
// 8. Motion Ghost Peak, Tail Error & Recovery Curve
// ================================================================================================

TEST(TaaMetricsTest, MotionCurveMetricsFloorSubtractionRecoveryAndFailure) {
    // 16-frame series, motion stops at frame 8.
    // Floor = 0.02, Peak = 0.10 -> transient = 0.08, 25% recovery threshold = 0.02 + 0.02 = 0.04.
    std::vector<double> const recoveredSeries = {
        0.02, 0.06, 0.09, 0.10, 0.09, 0.08, 0.08, 0.08, // moving 0..7
        0.07, 0.055, 0.045, 0.038, 0.025, 0.022, 0.021, 0.025 // static hold 8..15 (hits <= 0.04 at k=11 -> 3 frames)
    };
    MotionCurveMetrics const m = computeMotionCurveMetrics(recoveredSeries, 8);
    EXPECT_NEAR(m.edgeGhostPeak, 0.08, 1e-9);
    EXPECT_NEAR(m.edgeTailError, 0.005, 1e-9);
    EXPECT_NEAR(m.edgeEndRmse, 0.025, 1e-9);
    EXPECT_DOUBLE_EQ(m.edgeRecoveryFrames, 3.0);

    // Unrecovered series (stays at 0.05 > 0.04 threshold through frame 15) -> returns -1.0.
    std::vector<double> unrecovered = recoveredSeries;
    for (size_t k = 8; k < unrecovered.size(); k++) {
        unrecovered[k] = 0.05;
    }
    MotionCurveMetrics const unrec = computeMotionCurveMetrics(unrecovered, 8);
    EXPECT_DOUBLE_EQ(unrec.edgeRecoveryFrames, -1.0);
    EXPECT_NEAR(unrec.edgeTailError, 0.03, 1e-9);
}

// ================================================================================================
// 9. Analytical Backward-Motion Wake Ghosting (Horizontal, Vertical NDC Sign & Fallback)
// ================================================================================================

TEST(TaaMetricsTest, WakeGhostingHorizontalTrailingWakeVsLeadingBlur) {
    // 64x64 viewport with orthographic camera spanning [-32, +32]^2 -> 1 world unit == 1 pixel!
    // Object translates +10 units in +X (screen right, +vx = +10 px) between k-1 and k:
    //   refPrev: bright square [0.9] at x in [16..32), y in [20..44) on [0.1] background.
    //   refCurr: bright square [0.9] at x in [26..42), y in [20..44) on [0.1] background.
    uint32_t const w = 64, h = 64;
    LinearImage refPrev = makeSolidImage(w, h, { 0.1f, 0.1f, 0.1f });
    fillRect(refPrev, 16, 20, 32, 44, { 0.9f, 0.9f, 0.9f });

    LinearImage refCurr = makeSolidImage(w, h, { 0.1f, 0.1f, 0.1f });
    fillRect(refCurr, 26, 20, 42, 44, { 0.9f, 0.9f, 0.9f });

    std::vector<uint8_t> const prevMask = edgeMask(refPrev, 0.08f);
    std::vector<uint8_t> const currMask = edgeMask(refCurr, 0.08f);

    CapturePose const posePrev = makeOrthoPose(32.0, { 0.0, 0.0, 10.0 }, { 0.0, 0.0, 0.0 },
            mat4::translation(double3{ 0.0, 0.0, 0.0 }));
    CapturePose const poseCurr = makeOrthoPose(32.0, { 0.0, 0.0, 10.0 }, { 0.0, 0.0, 0.0 },
            mat4::translation(double3{ 10.0, 0.0, 0.0 }));

    // 1. Clean capture == refCurr -> zero wake ghosting.
    EXPECT_DOUBLE_EQ(computeWakeGhostingFrame(false, false, &poseCurr, &posePrev,
            refCurr, refCurr, refPrev, currMask, prevMask), 0.0);

    // 2. Trailing ghost wake behind -vx (left of x = 26, in the newly disoccluded [16..26) region):
    //    Inject ghost intensity 0.5 in [16..26) x [20..44).
    //    Analytical wake residual: capWake (0.5) - refWakeCurr (0.1) = +0.40 everywhere in wake!
    LinearImage capTrailingGhost = cloneImage(refCurr);
    fillRect(capTrailingGhost, 16, 20, 26, 44, { 0.5f, 0.5f, 0.5f });
    double const trailingScore = computeWakeGhostingFrame(false, false, &poseCurr, &posePrev,
            capTrailingGhost, refCurr, refPrev, currMask, prevMask);
    EXPECT_NEAR(trailingScore, 0.40, 1e-5);

    // 3a. Leading-edge halo/blur ahead of +vx (right of x = 42, in [42..52) x [20..44)):
    //     Must NOT be conflated with trailing wake ghosting -> score == 0.0!
    LinearImage capLeadingHalo = cloneImage(refCurr);
    fillRect(capLeadingHalo, 42, 20, 52, 44, { 0.5f, 0.5f, 0.5f });
    double const leadingScore = computeWakeGhostingFrame(false, false, &poseCurr, &posePrev,
            capLeadingHalo, refCurr, refPrev, currMask, prevMask);
    EXPECT_DOUBLE_EQ(leadingScore, 0.0);

    // 3b. Leading-edge inward blur (softening [38..42) inside the arriving right edge to 0.5):
    //     Must ALSO return 0.0 (not a trailing disoccluded wake)!
    LinearImage capLeadingInwardBlur = cloneImage(refCurr);
    fillRect(capLeadingInwardBlur, 38, 20, 42, 44, { 0.5f, 0.5f, 0.5f });
    EXPECT_DOUBLE_EQ(computeWakeGhostingFrame(false, false, &poseCurr, &posePrev,
            capLeadingInwardBlur, refCurr, refPrev, currMask, prevMask), 0.0);

    // 4. Static regime flag always returns 0.0.
    EXPECT_DOUBLE_EQ(computeWakeGhostingFrame(true, false, &poseCurr, &posePrev,
            capTrailingGhost, refCurr, refPrev, currMask, prevMask), 0.0);

    // 5. Un-vectored fallback (poseCurr == nullptr): SAD block-matching recovers +vx and detects
    //    the trailing ghost while still rejecting the leading halo!
    double const fallbackTrailing = computeWakeGhostingFrame(false, false, nullptr, nullptr,
            capTrailingGhost, refCurr, refPrev, currMask, prevMask);
    double const fallbackLeading = computeWakeGhostingFrame(false, false, nullptr, nullptr,
            capLeadingHalo, refCurr, refPrev, currMask, prevMask);
    EXPECT_GT(fallbackTrailing, 0.25);
    EXPECT_DOUBLE_EQ(fallbackLeading, 0.0);
}

TEST(TaaMetricsTest, WakeGhostingVerticalPerspectiveMotionVerifiesNdcYConvention) {
    // Regression guard for clip-space NDC (+Y up) vs top-down LinearImage (+y down):
    // Move object in world -Y -> moves DOWNWARD on screen (+vy > 0, increasing row index y).
    //   refPrev: bright square at y in [16..32), x in [20..44)
    //   refCurr: bright square at y in [26..42), x in [20..44)
    // Therefore:
    //   - TOP edge (y = 26) is the TRAILING disoccluded edge; wake is ABOVE it (y in [16..26)).
    //   - BOTTOM edge (y = 42) is the LEADING edge; region BELOW it (y in [42..52)) is ahead of +v.
    uint32_t const w = 64, h = 64;
    LinearImage refPrev = makeSolidImage(w, h, { 0.1f, 0.1f, 0.1f });
    fillRect(refPrev, 20, 16, 44, 32, { 0.9f, 0.9f, 0.9f });

    LinearImage refCurr = makeSolidImage(w, h, { 0.1f, 0.1f, 0.1f });
    fillRect(refCurr, 20, 26, 44, 42, { 0.9f, 0.9f, 0.9f });

    std::vector<uint8_t> const prevMask = edgeMask(refPrev, 0.08f);
    std::vector<uint8_t> const currMask = edgeMask(refCurr, 0.08f);

    // Perspective camera at z = +10 with 90 deg FOV (at z = 0, frustum spans [-10, +10]^2, so
    // moving -3.125 world units in Y moves +10 pixels in top-down screen y).
    CapturePose const posePrev = makePerspectivePose(90.0, { 0.0, 0.0, 10.0 }, { 0.0, 0.0, 0.0 },
            mat4::translation(double3{ 0.0, 0.0, 0.0 }));
    CapturePose const poseCurr = makePerspectivePose(90.0, { 0.0, 0.0, 10.0 }, { 0.0, 0.0, 0.0 },
            mat4::translation(double3{ 0.0, -3.125, 0.0 }));

    // Ghost wake ABOVE top trailing edge (y in [16..26)) MUST be detected:
    LinearImage capTopTrailingGhost = cloneImage(refCurr);
    fillRect(capTopTrailingGhost, 20, 16, 44, 26, { 0.5f, 0.5f, 0.5f });
    EXPECT_GT(computeWakeGhostingFrame(false, false, &poseCurr, &posePrev,
            capTopTrailingGhost, refCurr, refPrev, currMask, prevMask), 0.25);

    // Halo BELOW bottom leading edge (y in [42..52)) MUST return 0.0 (would fire if NDC Y flipped!):
    LinearImage capBottomLeadingHalo = cloneImage(refCurr);
    fillRect(capBottomLeadingHalo, 20, 42, 44, 52, { 0.5f, 0.5f, 0.5f });
    EXPECT_DOUBLE_EQ(computeWakeGhostingFrame(false, false, &poseCurr, &posePrev,
            capBottomLeadingHalo, refCurr, refPrev, currMask, prevMask), 0.0);
}

// ================================================================================================
// 10. Normative Protocol SHA-256 Digest (NIST FIPS 180-4 Test Vectors)
// ================================================================================================

TEST(TaaMetricsTest, Sha256MatchesNistFips180TestVectors) {
    // 1. Empty string
    EXPECT_EQ(sha256Hex(""),
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    // 2. "abc"
    EXPECT_EQ(sha256Hex("abc"),
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    // 3. 56-byte two-block padding boundary vector from NIST FIPS 180-4
    EXPECT_EQ(sha256Hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
            "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");

    // 4. 112-byte multi-block NIST vector
    EXPECT_EQ(sha256Hex("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"
                        "ijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"),
            "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1");
}

} // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
