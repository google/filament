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

#include <math/scalar.h>
#include <math/vec4.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>

namespace filament::tools::taa {

using namespace filament::math;
using namespace image;

float haltonSample(unsigned int i, unsigned int const b) noexcept {
    i += 409u;
    float f = 1.0f;
    float r = 0.0f;
    while (i > 0u) {
        f /= float(b);
        r += f * float(i % b);
        i /= b;
    }
    return r;
}

float filterRadius(ReferenceFilter const filter) noexcept {
    return filter == ReferenceFilter::BOX ? 0.5f : 2.0f;
}

float sinc(float const x) noexcept {
    if (x == 0.0f) {
        return 1.0f;
    }
    float const px = float(F_PI) * x;
    return std::sin(px) / px;
}

float lanczos2(float const x) noexcept {
    return std::abs(x) >= 2.0f ? 0.0f : sinc(x) * sinc(x * 0.5f);
}

float lanczos2Filament(float const x) noexcept {
    if (std::abs(x) >= 2.0f) {
        return 0.0f;
    }
    float const x2 = x * x;
    float const wA = x2 - 4.0f;
    float const wA2 = wA * wA;
    float const wC = x2 - 1.0f;
    float const wD = -0.354377f * x2 + 1.0f;
    float const wE = wC * wD;
    return wA2 * wE * (-1.0f / 16.0f);
}

float filterWeight(float2 const offset, ReferenceFilter const filter) noexcept {
    switch (filter) {
        case ReferenceFilter::BOX:
            return 1.0f;
        case ReferenceFilter::LANCZOS:
            return lanczos2(offset.x) * lanczos2(offset.y);
        case ReferenceFilter::LANCZOS_FILAMENT:
            return lanczos2Filament(offset.x) * lanczos2Filament(offset.y);
    }
    return 1.0f;
}

float2 haltonJitter(size_t const index, ReferenceFilter const filter) noexcept {
    float const scale = 2.0f * filterRadius(filter);
    return { (haltonSample(unsigned(index), 2u) - 0.5f) * scale,
             (haltonSample(unsigned(index), 3u) - 0.5f) * scale };
}

char const* referenceFilterName(ReferenceFilter const filter) noexcept {
    switch (filter) {
        case ReferenceFilter::BOX: return "box";
        case ReferenceFilter::LANCZOS: return "lanczos";
        case ReferenceFilter::LANCZOS_FILAMENT: return "lanczos-filament";
    }
    return "box";
}

float srgbToLinear(float const c) noexcept {
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

LinearImage decode(uint8_t const* rgba, uint32_t const w, uint32_t const h) {
    LinearImage out(w, h, 3);
    float* dst = out.getPixelRef();
    for (uint32_t y = 0; y < h; y++) {
        uint8_t const* src = rgba + size_t(h - 1 - y) * w * 4;
        for (uint32_t x = 0; x < w; x++) {
            for (uint32_t c = 0; c < 3; c++) {
                dst[(size_t(y) * w + x) * 3 + c] =
                        srgbToLinear(float(src[x * 4 + c]) * (1.0f / 255.0f));
            }
        }
    }
    return out;
}

float luma(float const* p) noexcept {
    return 0.2126f * p[0] + 0.7152f * p[1] + 0.0722f * p[2];
}

double sampleLumaBilinear(LinearImage const& img, double const sx, double const sy) noexcept {
    uint32_t const w = img.getWidth();
    uint32_t const h = img.getHeight();
    if (w == 0 || h == 0) {
        return 0.0;
    }
    double const cx = std::clamp(sx, 0.0, double(w - 1));
    double const cy = std::clamp(sy, 0.0, double(h - 1));
    uint32_t const x0 = uint32_t(std::floor(cx));
    uint32_t const y0 = uint32_t(std::floor(cy));
    uint32_t const x1 = std::min(w - 1, x0 + 1);
    uint32_t const y1 = std::min(h - 1, y0 + 1);
    double const fx = cx - double(x0);
    double const fy = cy - double(y0);
    double const l00 = double(luma(img.getPixelRef(x0, y0)));
    double const l10 = double(luma(img.getPixelRef(x1, y0)));
    double const l01 = double(luma(img.getPixelRef(x0, y1)));
    double const l11 = double(luma(img.getPixelRef(x1, y1)));
    double const top = l00 * (1.0 - fx) + l10 * fx;
    double const bot = l01 * (1.0 - fx) + l11 * fx;
    return top * (1.0 - fy) + bot * fy;
}

std::vector<uint8_t> edgeMask(LinearImage const& reference, float const threshold) {
    uint32_t const w = reference.getWidth();
    uint32_t const h = reference.getHeight();
    std::vector<uint8_t> raw(size_t(w) * h, 0);
    for (uint32_t y = 1; y + 1 < h; y++) {
        for (uint32_t x = 1; x + 1 < w; x++) {
            float const l  = luma(reference.getPixelRef(x, y));
            float const lx = luma(reference.getPixelRef(x + 1, y));
            float const ly = luma(reference.getPixelRef(x, y + 1));
            float const gradient = std::abs(lx - l) + std::abs(ly - l);
            raw[size_t(y) * w + x] = gradient > threshold ? 1 : 0;
        }
    }
    std::vector<uint8_t> mask(size_t(w) * h, 0);
    for (uint32_t y = 1; y + 1 < h; y++) {
        for (uint32_t x = 1; x + 1 < w; x++) {
            uint8_t v = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    v |= raw[size_t(y + dy) * w + (x + dx)];
                }
            }
            mask[size_t(y) * w + x] = v;
        }
    }
    return mask;
}

std::vector<uint8_t> foregroundMask(LinearImage const& reference,
        std::vector<uint8_t> const& silhouetteMask) {
    uint32_t const w = reference.getWidth();
    uint32_t const h = reference.getHeight();
    std::vector<uint8_t> fg(size_t(w) * h, 0);
    if (w < 3 || h < 3) {
        std::fill(fg.begin(), fg.end(), uint8_t(1));
        return fg;
    }
    size_t activeCount = 0;
    double const invWm1 = 1.0 / double(std::max<uint32_t>(1u, w - 1));
    for (uint32_t y = 0; y < h; y++) {
        float const* leftBg = reference.getPixelRef(0, y);
        float const* rightBg = reference.getPixelRef(w - 1, y);
        for (uint32_t x = 0; x < w; x++) {
            size_t const idx = size_t(y) * w + x;
            if (idx < silhouetteMask.size() && silhouetteMask[idx]) {
                fg[idx] = 1;
                activeCount++;
                continue;
            }
            float const* p = reference.getPixelRef(x, y);
            double const t = double(x) * invWm1;
            double rgbDiffSq = 0.0;
            for (int c = 0; c < 3; c++) {
                double const bg = double(leftBg[c]) * (1.0 - t) + double(rightBg[c]) * t;
                double const d = double(p[c]) - bg;
                rgbDiffSq += d * d;
            }
            bool isFg = std::sqrt(rgbDiffSq * (1.0 / 3.0)) > 0.025;
            if (!isFg && x > 0 && x + 1 < w && y > 0 && y + 1 < h) {
                float const l = luma(p);
                float const gx = std::abs(luma(reference.getPixelRef(x + 1, y)) - l);
                float const gy = std::abs(luma(reference.getPixelRef(x, y + 1)) - l);
                isFg = (gx + gy) > 0.01f;
            }
            if (isFg) {
                fg[idx] = 1;
                activeCount++;
            }
        }
    }
    if (activeCount < 64) {
        std::fill(fg.begin(), fg.end(), uint8_t(1));
    }
    return fg;
}

LinearImage maskImage(std::vector<uint8_t> const& mask, uint32_t const w, uint32_t const h) {
    LinearImage out(w, h, 3);
    float* dst = out.getPixelRef();
    for (size_t i = 0; i < size_t(w) * h; i++) {
        float const v = mask[i] ? 1.0f : 0.0f;
        dst[i * 3 + 0] = dst[i * 3 + 1] = dst[i * 3 + 2] = v;
    }
    return out;
}

LinearImage differenceImage(LinearImage const& a, LinearImage const& b, float const gain) {
    uint32_t const w = a.getWidth();
    uint32_t const h = a.getHeight();
    LinearImage out(w, h, 3);
    float* dst = out.getPixelRef();
    float const* pa = a.getPixelRef();
    float const* pb = b.getPixelRef();
    for (size_t i = 0; i < size_t(w) * h * 3; i++) {
        dst[i] = std::min(1.0f, std::abs(pa[i] - pb[i]) * gain);
    }
    return out;
}

double mseToDb(double const mse) noexcept {
    return -10.0 * std::log10(std::max(mse, 1e-12));
}

double sortedPercentile(std::vector<double> const& sorted, double const q) noexcept {
    if (sorted.empty()) {
        return 0.0;
    }
    if (sorted.size() == 1) {
        return sorted.front();
    }
    double const pos = std::clamp(q, 0.0, 1.0) * double(sorted.size() - 1);
    size_t const lo = size_t(std::floor(pos));
    size_t const hi = std::min(sorted.size() - 1, lo + 1);
    double const frac = pos - double(lo);
    return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
}

void computeDistributionStats(std::vector<double> values, double* outP50, double* outP95,
        double* outIqr, double* outMean, double* outStddev) {
    if (values.empty()) {
        *outP50 = 0.0;
        *outP95 = 0.0;
        *outIqr = 0.0;
        *outMean = 0.0;
        *outStddev = 0.0;
        return;
    }
    std::sort(values.begin(), values.end());
    *outP50 = sortedPercentile(values, 0.50);
    *outP95 = sortedPercentile(values, 0.95);
    *outIqr = sortedPercentile(values, 0.75) - sortedPercentile(values, 0.25);
    double sum = 0.0;
    for (double const v : values) {
        sum += v;
    }
    double const mean = sum / double(values.size());
    *outMean = mean;
    double varSum = 0.0;
    for (double const v : values) {
        double const d = v - mean;
        varSum += d * d;
    }
    *outStddev = std::sqrt(varSum / double(values.size()));
}

double rmse(LinearImage const& a, LinearImage const& b,
        std::vector<uint8_t> const* mask) noexcept {
    uint32_t const w = a.getWidth();
    uint32_t const h = a.getHeight();
    double sum = 0.0;
    size_t count = 0;
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            if (mask && !(*mask)[size_t(y) * w + x]) {
                continue;
            }
            float const* pa = a.getPixelRef(x, y);
            float const* pb = b.getPixelRef(x, y);
            for (int c = 0; c < 3; c++) {
                double const d = double(pa[c]) - double(pb[c]);
                sum += d * d;
            }
            count += 3;
        }
    }
    return count ? std::sqrt(sum / double(count)) : 0.0;
}

double shiftedRmse(LinearImage const& a, LinearImage const& b, std::vector<uint8_t> const& mask,
        double const dx, double const dy) noexcept {
    uint32_t const w = a.getWidth();
    uint32_t const h = a.getHeight();
    double sum = 0.0;
    size_t count = 0;
    for (uint32_t y = 1; y + 2 < h; y++) {
        for (uint32_t x = 1; x + 2 < w; x++) {
            if (!mask[size_t(y) * w + x]) {
                continue;
            }
            double const sx = double(x) + dx;
            double const sy = double(y) + dy;
            auto const x0 = uint32_t(std::floor(sx));
            auto const y0 = uint32_t(std::floor(sy));
            if (x0 < 1 || y0 < 1 || x0 + 2 >= w || y0 + 2 >= h) {
                continue;
            }
            double const fx = sx - double(x0);
            double const fy = sy - double(y0);
            float const* pa = a.getPixelRef(x, y);
            float const* p00 = b.getPixelRef(x0, y0);
            float const* p10 = b.getPixelRef(x0 + 1, y0);
            float const* p01 = b.getPixelRef(x0, y0 + 1);
            float const* p11 = b.getPixelRef(x0 + 1, y0 + 1);
            for (int c = 0; c < 3; c++) {
                double const top = double(p00[c]) * (1.0 - fx) + double(p10[c]) * fx;
                double const bottom = double(p01[c]) * (1.0 - fx) + double(p11[c]) * fx;
                double const d = double(pa[c]) - (top * (1.0 - fy) + bottom * fy);
                sum += d * d;
            }
            count += 3;
        }
    }
    return count ? std::sqrt(sum / double(count)) : 0.0;
}

double3 referenceAlignment(LinearImage const& capture, LinearImage const& reference,
        std::vector<uint8_t> const& mask) noexcept {
    double2 offset = {};
    double best = std::numeric_limits<double>::max();
    for (double const step: { 0.25, 0.05 }) {
        double2 const centre = offset;
        for (int iy = -6; iy <= 6; iy++) {
            for (int ix = -6; ix <= 6; ix++) {
                double2 const candidate = { centre.x + step * ix, centre.y + step * iy };
                double const e = shiftedRmse(capture, reference, mask, candidate.x, candidate.y);
                if (e < best) {
                    best = e;
                    offset = candidate;
                }
            }
        }
    }
    double const unaligned = shiftedRmse(capture, reference, mask, 0.0, 0.0);
    double const gain = unaligned > 0.0 ? 1.0 - best / unaligned : 0.0;
    return { offset.x, offset.y, gain };
}

double highFrequencyEnergy(LinearImage const& img, std::vector<uint8_t> const* mask) noexcept {
    uint32_t const w = img.getWidth();
    uint32_t const h = img.getHeight();
    double sum = 0.0;
    size_t count = 0;
    for (uint32_t y = 1; y + 1 < h; y++) {
        for (uint32_t x = 1; x + 1 < w; x++) {
            if (mask && !(*mask)[size_t(y) * w + x]) {
                continue;
            }
            double const l = 4.0 * luma(img.getPixelRef(x, y))
                    - luma(img.getPixelRef(x - 1, y)) - luma(img.getPixelRef(x + 1, y))
                    - luma(img.getPixelRef(x, y - 1)) - luma(img.getPixelRef(x, y + 1));
            sum += l * l;
            count++;
        }
    }
    return count ? sum / double(count) : 0.0;
}

LaplacianComparison compareLaplacians(LinearImage const& capture, LinearImage const& reference,
        std::vector<uint8_t> const* mask) noexcept {
    uint32_t const w = capture.getWidth();
    uint32_t const h = capture.getHeight();
    double capSum = 0.0;
    double refSum = 0.0;
    double dotSum = 0.0;
    size_t count = 0;
    for (uint32_t y = 1; y + 1 < h; y++) {
        for (uint32_t x = 1; x + 1 < w; x++) {
            if (mask && !(*mask)[size_t(y) * w + x]) {
                continue;
            }
            double const lc = 4.0 * luma(capture.getPixelRef(x, y))
                    - luma(capture.getPixelRef(x - 1, y)) - luma(capture.getPixelRef(x + 1, y))
                    - luma(capture.getPixelRef(x, y - 1)) - luma(capture.getPixelRef(x, y + 1));
            double const lr = 4.0 * luma(reference.getPixelRef(x, y))
                    - luma(reference.getPixelRef(x - 1, y)) - luma(reference.getPixelRef(x + 1, y))
                    - luma(reference.getPixelRef(x, y - 1)) - luma(reference.getPixelRef(x, y + 1));
            capSum += lc * lc;
            refSum += lr * lr;
            dotSum += lc * lr;
            count++;
        }
    }
    if (count == 0 || refSum <= 0.0) {
        return {};
    }
    double const alpha = dotSum / refSum;
    double const sharpnessRatio = capSum / refSum;
    double const orthoEnergyRatio = std::max(0.0, sharpnessRatio - alpha * alpha);
    double const overshoot = std::max(0.0, alpha - 1.0);
    return {
        .detailTransfer = alpha,
        .spuriousHfRms = std::sqrt(orthoEnergyRatio + overshoot * overshoot),
    };
}

double computeGmsd(LinearImage const& capture, LinearImage const& reference,
        std::vector<uint8_t> const* mask) {
    uint32_t const w = capture.getWidth();
    uint32_t const h = capture.getHeight();
    if (w < 3 || h < 3) {
        return 0.0;
    }
    constexpr double kStabilityC = 0.0026;
    auto prewittMag = [](LinearImage const& img, uint32_t const x, uint32_t const y) -> double {
        double const tl = double(luma(img.getPixelRef(x - 1, y - 1)));
        double const tc = double(luma(img.getPixelRef(x,     y - 1)));
        double const tr = double(luma(img.getPixelRef(x + 1, y - 1)));
        double const ml = double(luma(img.getPixelRef(x - 1, y)));
        double const mr = double(luma(img.getPixelRef(x + 1, y)));
        double const bl = double(luma(img.getPixelRef(x - 1, y + 1)));
        double const bc = double(luma(img.getPixelRef(x,     y + 1)));
        double const br = double(luma(img.getPixelRef(x + 1, y + 1)));
        double const gx = ((tr + mr + br) - (tl + ml + bl)) * (1.0 / 3.0);
        double const gy = ((bl + bc + br) - (tl + tc + tr)) * (1.0 / 3.0);
        return std::sqrt(gx * gx + gy * gy);
    };

    std::vector<double> gmsValues;
    gmsValues.reserve(size_t(w - 2) * (h - 2));
    double sum = 0.0;
    for (uint32_t y = 1; y + 1 < h; y++) {
        for (uint32_t x = 1; x + 1 < w; x++) {
            if (mask && !(*mask)[size_t(y) * w + x]) {
                continue;
            }
            double const gRef = prewittMag(reference, x, y);
            double const gCap = prewittMag(capture, x, y);
            double const gDiff = gRef - gCap;
            // 1 - GMS(x, y) = (gRef - gCap)^2 / (gRef^2 + gCap^2 + c).
            // Since stddev(GMS) == stddev(1 - GMS) identically, evaluating 1 - GMS avoids
            // catastrophic floating-point cancellation near 1.0 when gRef ~= gCap and guarantees
            // exact 0.0 on identical images.
            double const gmsLoss = (gDiff * gDiff) / (gRef * gRef + gCap * gCap + kStabilityC);
            gmsValues.push_back(gmsLoss);
            sum += gmsLoss;
        }
    }
    if (gmsValues.size() < 2) {
        return 0.0;
    }
    double const mean = sum / double(gmsValues.size());
    double varSum = 0.0;
    for (double const v : gmsValues) {
        double const d = v - mean;
        varSum += d * d;
    }
    return std::sqrt(varSum / double(gmsValues.size()));
}

HotspotMetrics computeInnerGridHotspots(LinearImage const& capture,
        LinearImage const& reference, std::vector<uint8_t> const& fgMask) {
    uint32_t const w = capture.getWidth();
    uint32_t const h = capture.getHeight();
    if (w < 10 || h < 10) {
        return {};
    }
    std::vector<double> activeCellP95;
    std::vector<double> allCellP95;
    activeCellP95.reserve(64);
    allCellP95.reserve(64);
    std::vector<double> cellErrors;

    for (uint32_t r = 1; r <= 8; r++) {
        uint32_t const y0 = (r * h) / 10;
        uint32_t const y1 = ((r + 1) * h) / 10;
        for (uint32_t c = 1; c <= 8; c++) {
            uint32_t const x0 = (c * w) / 10;
            uint32_t const x1 = ((c + 1) * w) / 10;
            if (y1 <= y0 || x1 <= x0) {
                continue;
            }
            cellErrors.clear();
            cellErrors.reserve(size_t(y1 - y0) * (x1 - x0));
            size_t fgCount = 0;
            for (uint32_t y = y0; y < y1; y++) {
                for (uint32_t x = x0; x < x1; x++) {
                    size_t const idx = size_t(y) * w + x;
                    if (idx < fgMask.size() && fgMask[idx]) {
                        fgCount++;
                    }
                    float const* pa = capture.getPixelRef(x, y);
                    float const* pb = reference.getPixelRef(x, y);
                    double const d0 = double(pa[0]) - double(pb[0]);
                    double const d1 = double(pa[1]) - double(pb[1]);
                    double const d2 = double(pa[2]) - double(pb[2]);
                    cellErrors.push_back(std::sqrt((d0 * d0 + d1 * d1 + d2 * d2) * (1.0 / 3.0)));
                }
            }
            if (cellErrors.empty()) {
                continue;
            }
            std::sort(cellErrors.begin(), cellErrors.end());
            double const p95 = sortedPercentile(cellErrors, 0.95);
            allCellP95.push_back(p95);
            if (fgCount >= 10) {
                activeCellP95.push_back(p95);
            }
        }
    }

    std::vector<double>& pool = activeCellP95.empty() ? allCellP95 : activeCellP95;
    if (pool.empty()) {
        return {};
    }
    std::sort(pool.begin(), pool.end(), std::greater<double>());
    size_t const topK = std::min<size_t>(10, pool.size());
    double topSum = 0.0;
    for (size_t i = 0; i < topK; i++) {
        topSum += pool[i];
    }
    return {
        .top10Rmse = topSum / double(topK),
        .peakRmse = pool.front(),
    };
}

double temporalVariance(std::vector<LinearImage> const& frames,
        std::vector<uint8_t> const* mask) noexcept {
    if (frames.size() < 2) {
        return 0.0;
    }
    uint32_t const w = frames[0].getWidth();
    uint32_t const h = frames[0].getHeight();
    double total = 0.0;
    size_t count = 0;
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            if (mask && !(*mask)[size_t(y) * w + x]) {
                continue;
            }
            for (int c = 0; c < 3; c++) {
                double mean = 0.0;
                for (auto const& f: frames) {
                    mean += double(f.getPixelRef(x, y)[c]);
                }
                mean /= double(frames.size());
                double variance = 0.0;
                for (auto const& f: frames) {
                    double const d = double(f.getPixelRef(x, y)[c]) - mean;
                    variance += d * d;
                }
                total += variance / double(frames.size());
                count++;
            }
        }
    }
    return count ? total / double(count) : 0.0;
}

double topFractionTemporalRms(std::vector<LinearImage> const& captures,
        std::vector<LinearImage> const* references, double const fraction) {
    if (captures.size() < 2) {
        return 0.0;
    }
    uint32_t const w = captures[0].getWidth();
    uint32_t const h = captures[0].getHeight();
    size_t const pixelCount = size_t(w) * h;
    std::vector<double> pixelVariances(pixelCount, 0.0);
    double const invN = 1.0 / double(captures.size());

    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            double channelVarSum = 0.0;
            for (int c = 0; c < 3; c++) {
                double mean = 0.0;
                for (size_t k = 0; k < captures.size(); k++) {
                    double v = double(captures[k].getPixelRef(x, y)[c]);
                    if (references && !references->empty()) {
                        size_t const rk = std::min(k, references->size() - 1);
                        v -= double((*references)[rk].getPixelRef(x, y)[c]);
                    }
                    mean += v;
                }
                mean *= invN;
                double var = 0.0;
                for (size_t k = 0; k < captures.size(); k++) {
                    double v = double(captures[k].getPixelRef(x, y)[c]);
                    if (references && !references->empty()) {
                        size_t const rk = std::min(k, references->size() - 1);
                        v -= double((*references)[rk].getPixelRef(x, y)[c]);
                    }
                    double const d = v - mean;
                    var += d * d;
                }
                channelVarSum += var * invN;
            }
            pixelVariances[size_t(y) * w + x] = channelVarSum / 3.0;
        }
    }

    size_t const topCount = std::clamp<size_t>(
            size_t(std::ceil(double(pixelCount) * fraction)), 1, pixelCount);
    auto pivot = pixelVariances.end() - ptrdiff_t(topCount);
    std::nth_element(pixelVariances.begin(), pivot, pixelVariances.end());
    double sum = 0.0;
    for (auto it = pivot; it != pixelVariances.end(); ++it) {
        sum += *it;
    }
    return std::sqrt(sum / double(topCount));
}

SubpixelRingMetrics subpixelRingFlicker(std::vector<LinearImage> const& captures) noexcept {
    SubpixelRingMetrics out = {};
    if (captures.size() < 2) {
        return out;
    }
    uint32_t const w = captures[0].getWidth();
    uint32_t const h = captures[0].getHeight();
    double const scale = double(h) / 512.0;
    double const cx = 256.0 * scale;
    double const cy = 275.0 * scale;
    constexpr double kRingBounds[4][2] = {
        { 14.0, 22.0 },
        { 22.0, 34.0 },
        { 34.0, 52.0 },
        { 52.0, 78.0 },
    };
    double ringSum[4] = {};
    size_t ringCount[4] = {};
    double const invN = 1.0 / double(captures.size());

    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            double const r = std::hypot(double(x) - cx, double(y) - cy);
            int ringIdx = -1;
            for (int b = 0; b < 4; b++) {
                if (r >= kRingBounds[b][0] * scale && r < kRingBounds[b][1] * scale) {
                    ringIdx = b;
                    break;
                }
            }
            if (ringIdx < 0) {
                continue;
            }
            double mean = 0.0;
            for (auto const& f: captures) {
                mean += double(luma(f.getPixelRef(x, y)));
            }
            mean *= invN;
            double var = 0.0;
            for (auto const& f: captures) {
                double const d = double(luma(f.getPixelRef(x, y))) - mean;
                var += d * d;
            }
            ringSum[ringIdx] += std::sqrt(var * invN);
            ringCount[ringIdx]++;
        }
    }
    double rmsAccum = 0.0;
    int validRings = 0;
    for (int b = 0; b < 4; b++) {
        if (ringCount[b] > 0) {
            out.ringStd[b] = ringSum[b] / double(ringCount[b]);
            rmsAccum += out.ringStd[b] * out.ringStd[b];
            validRings++;
        }
    }
    out.rms = validRings > 0 ? std::sqrt(rmsAccum / double(validRings)) : 0.0;
    return out;
}

MotionCurveMetrics computeMotionCurveMetrics(std::vector<double> const& edgeErrors,
        size_t const motionFrames) noexcept {
    if (edgeErrors.empty()) {
        return {};
    }
    double const floor = edgeErrors.front();
    double const peak = *std::max_element(edgeErrors.begin(), edgeErrors.end());
    double recovery = 0.0;
    if (peak - floor > 0.0) {
        double const threshold = floor + 0.25 * (peak - floor);
        recovery = -1.0;
        for (size_t k = motionFrames; k < edgeErrors.size(); k++) {
            if (edgeErrors[k] <= threshold) {
                recovery = double(k - motionFrames);
                break;
            }
        }
    }
    return {
        .edgeGhostPeak = peak - floor,
        .edgeTailError = std::max(0.0, edgeErrors.back() - floor),
        .edgeEndRmse = edgeErrors.back(),
        .edgeRecoveryFrames = recovery,
    };
}

double computeWakeGhostingFrame(bool const isStatic, bool const isSpinObject,
        CapturePose const* poseCurr, CapturePose const* posePrev,
        LinearImage const& capture, LinearImage const& refCurr, LinearImage const& refPrev,
        std::vector<uint8_t> const& currEdgeMask,
        std::vector<uint8_t> const& prevEdgeMask) noexcept {
    if (isStatic) {
        return 0.0;
    }
    uint32_t const w = capture.getWidth();
    uint32_t const h = capture.getHeight();
    if (w < 16 || h < 16) {
        return 0.0;
    }

    bool const hasPoses = poseCurr != nullptr && posePrev != nullptr &&
            poseCurr->valid && posePrev->valid;

    auto patchSad = [&](uint32_t const x, uint32_t const y, int const dx, int const dy) -> double {
        int const px = int(x) - dx;
        int const py = int(y) - dy;
        if (px < 2 || py < 2 || px + 2 >= int(w) || py + 2 >= int(h)) {
            return std::numeric_limits<double>::max();
        }
        double sad = 0.0;
        for (int wy = -2; wy <= 2; wy++) {
            for (int wx = -2; wx <= 2; wx++) {
                double const lc = double(luma(refCurr.getPixelRef(
                        uint32_t(int(x) + wx), uint32_t(int(y) + wy))));
                double const lp = double(luma(refPrev.getPixelRef(
                        uint32_t(px + wx), uint32_t(py + wy))));
                sad += std::abs(lc - lp);
            }
        }
        return sad;
    };

    auto scanlineBgDist = [&](LinearImage const& img, double const sx, double const sy) -> double {
        uint32_t const row = std::min(h - 1, uint32_t(std::clamp(std::round(sy), 0.0, double(h - 1))));
        float const* leftBg = img.getPixelRef(0, row);
        float const* rightBg = img.getPixelRef(w - 1, row);
        double const t = std::clamp(sx / double(std::max<uint32_t>(1u, w - 1)), 0.0, 1.0);
        double const bgLuma = double(luma(leftBg)) * (1.0 - t) + double(luma(rightBg)) * t;
        double const pxLuma = sampleLumaBilinear(img, sx, sy);
        return std::abs(pxLuma - bgLuma);
    };

    double ghostSqSum = 0.0;
    size_t ghostCount = 0;

    for (uint32_t y = 8; y + 8 < h; y++) {
        for (uint32_t x = 8; x + 8 < w; x++) {
            size_t const idx = size_t(y) * w + x;
            if (!currEdgeMask[idx]) {
                continue;
            }
            double const lRefCurr = double(luma(refCurr.getPixelRef(x, y)));

            double vx = 0.0;
            double vy = 0.0;
            if (hasPoses) {
                double const ndcX = (2.0 * (double(x) + 0.5)) / double(w) - 1.0;
                double const ndcY = 1.0 - (2.0 * (double(y) + 0.5)) / double(h);
                double4 const c0 = poseCurr->invViewProj * double4{ ndcX, ndcY, -0.5, 1.0 };
                double4 const c1 = poseCurr->invViewProj * double4{ ndcX, ndcY,  0.5, 1.0 };
                if (std::abs(c0.w) > 1e-9 && std::abs(c1.w) > 1e-9) {
                    double3 const p0 = c0.xyz / c0.w;
                    double3 const p1 = c1.xyz / c1.w;
                    double3 const viewDir = normalize(poseCurr->target - poseCurr->eye);
                    double3 rayDir = normalize(p1 - p0);
                    if (dot(rayDir, viewDir) < 0.0) {
                        rayDir = -rayDir;
                    }
                    double const denom = dot(rayDir, viewDir);
                    double3 worldCurr = poseCurr->target;
                    if (std::abs(denom) > 1e-6) {
                        double const sHit = dot(poseCurr->target - p0, viewDir) / denom;
                        worldCurr = p0 + rayDir * sHit;
                    }
                    if (isSpinObject) {
                        double3 const offset = worldCurr - poseCurr->target;
                        double const rEff = 0.55 * poseCurr->radius;
                        double const dPerpSq = dot(offset, offset);
                        double const zFront = std::sqrt(
                                std::max(0.0, rEff * rEff - std::min(dPerpSq, rEff * rEff)));
                        double const pullBack = std::max(zFront, 0.25 * poseCurr->radius);
                        double const maxPull = std::max(0.0,
                                dot(worldCurr - poseCurr->eye, rayDir) - 0.05 * poseCurr->radius);
                        worldCurr -= rayDir * std::min(pullBack, maxPull);
                    }
                    double4 const localPt =
                            poseCurr->invObjectTransform * double4{ worldCurr, 1.0 };
                    double4 const worldPrev = posePrev->objectTransform * localPt;
                    double4 const clipPrev = posePrev->viewProj * worldPrev;
                    if (std::abs(clipPrev.w) > 1e-6) {
                        double const prevNdcX = clipPrev.x / clipPrev.w;
                        double const prevNdcY = clipPrev.y / clipPrev.w;
                        double const prevScreenX = 0.5 * (prevNdcX + 1.0) * double(w) - 0.5;
                        double const prevScreenY = 0.5 * (1.0 - prevNdcY) * double(h) - 0.5;
                        vx = double(x) - prevScreenX;
                        vy = double(y) - prevScreenY;
                    }
                }
            }

            if (std::hypot(vx, vy) < 0.5) {
                // Local SAD block-matching fallback for un-vectored deformation (e.g. skinning).
                double const zeroSad = patchSad(x, y, 0, 0);
                double bestSad = zeroSad;
                int bestDx = 0;
                int bestDy = 0;
                for (int dy = -12; dy <= 12; dy++) {
                    for (int dx = -12; dx <= 12; dx++) {
                        if (dx == 0 && dy == 0) {
                            continue;
                        }
                        double const sad = patchSad(x, y, dx, dy);
                        if (sad < bestSad) {
                            bestSad = sad;
                            bestDx = dx;
                            bestDy = dy;
                        }
                    }
                }
                if (bestSad < 0.85 * zeroSad) {
                    vx = double(bestDx);
                    vy = double(bestDy);
                }
            }

            double const vMag = std::hypot(vx, vy);
            if (vMag < 0.5) {
                continue;
            }
            double const ux = vx / vMag;
            double const uy = vy / vMag;

            // Verify `(x, y)` sits on the moving foreground side of a trailing disoccluded edge:
            //  1. Stepping forward (+1.5 * v_hat) stays on the moving surface while stepping
            //     backward (-2.0 * v_hat) crosses the silhouette step into a disoccluded wake.
            //  2. The backward wake sample changed between frame k-1 and k, and `(x, y)` lies on
            //     the foreground side rather than static background ahead of a leading edge.
            double const ax = std::clamp(double(x) + 1.5 * ux, 1.0, double(w - 2));
            double const ay = std::clamp(double(y) + 1.5 * uy, 1.0, double(h - 2));
            double const aheadDiff = std::abs(sampleLumaBilinear(refCurr, ax, ay) - lRefCurr);
            double const selfBgDist = scanlineBgDist(refCurr, double(x), double(y));

            bool isTrailingWake = false;
            for (double const probeD : { 2.0, 4.0 }) {
                double const px = double(x) - probeD * ux;
                double const py = double(y) - probeD * uy;
                if (px < 1.0 || py < 1.0 || px >= double(w - 2) || py >= double(h - 2)) {
                    continue;
                }
                double const wakeRefCurr = sampleLumaBilinear(refCurr, px, py);
                double const wakeRefPrev = sampleLumaBilinear(refPrev, px, py);
                double const stepContrast = std::abs(wakeRefCurr - lRefCurr);
                double const wakeBgDist = scanlineBgDist(refCurr, px, py);
                if (std::abs(wakeRefCurr - wakeRefPrev) > 0.015 &&
                        stepContrast > 0.02 &&
                        aheadDiff < 0.5 * stepContrast &&
                        selfBgDist + 0.01 >= wakeBgDist) {
                    isTrailingWake = true;
                    break;
                }
            }
            if (!isTrailingWake) {
                continue;
            }

            double wakeSum = 0.0;
            int validD = 0;
            for (double const d : { 2.0, 4.0, 6.0, 8.0 }) {
                if (d > vMag + 1.0) {
                    break;
                }
                double const sx = double(x) - d * ux;
                double const sy = double(y) - d * uy;
                if (sx < 1.0 || sy < 1.0 || sx >= double(w - 2) || sy >= double(h - 2)) {
                    continue;
                }
                double const refWakeCurr = sampleLumaBilinear(refCurr, sx, sy);
                double const refWakePrev = sampleLumaBilinear(refPrev, sx, sy);
                double const histDelta = refWakePrev - refWakeCurr;
                if (std::abs(histDelta) <= 0.015) {
                    continue;
                }
                double const capWake = sampleLumaBilinear(capture, sx, sy);
                double const capDelta = capWake - refWakeCurr;
                double const signHist = histDelta >= 0.0 ? 1.0 : -1.0;
                wakeSum += std::max(0.0, signHist * capDelta);
                validD++;
            }
            if (validD > 0) {
                double const g = wakeSum / double(validD);
                ghostSqSum += g * g;
                ghostCount++;
            }
        }
    }

    (void) prevEdgeMask;
    return ghostCount > 0 ? std::sqrt(ghostSqSum / double(ghostCount)) : 0.0;
}

std::string sha256Hex(std::string_view const input) {
    static constexpr uint32_t kK[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
        0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
        0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
        0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
        0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
        0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
        0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
        0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
        0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
    };

    auto rotr = [](uint32_t const x, uint32_t const n) -> uint32_t {
        return (x >> n) | (x << (32u - n));
    };

    uint32_t h[8] = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    };

    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t const bitLen = uint64_t(msg.size()) * 8ull;
    msg.push_back(0x80u);
    while ((msg.size() % 64u) != 56u) {
        msg.push_back(0x00u);
    }
    for (int i = 7; i >= 0; i--) {
        msg.push_back(uint8_t((bitLen >> (i * 8)) & 0xffu));
    }

    for (size_t offset = 0; offset < msg.size(); offset += 64u) {
        uint32_t w[64];
        for (size_t i = 0; i < 16u; i++) {
            size_t const j = offset + i * 4u;
            w[i] = (uint32_t(msg[j]) << 24u)
                    | (uint32_t(msg[j + 1]) << 16u)
                    | (uint32_t(msg[j + 2]) << 8u)
                    | uint32_t(msg[j + 3]);
        }
        for (size_t i = 16u; i < 64u; i++) {
            uint32_t const s0 = rotr(w[i - 15], 7u) ^ rotr(w[i - 15], 18u) ^ (w[i - 15] >> 3u);
            uint32_t const s1 = rotr(w[i - 2], 17u) ^ rotr(w[i - 2], 19u) ^ (w[i - 2] >> 10u);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        for (size_t i = 0; i < 64u; i++) {
            uint32_t const S1 = rotr(e, 6u) ^ rotr(e, 11u) ^ rotr(e, 25u);
            uint32_t const ch = (e & f) ^ ((~e) & g);
            uint32_t const temp1 = hh + S1 + ch + kK[i] + w[i];
            uint32_t const S0 = rotr(a, 2u) ^ rotr(a, 13u) ^ rotr(a, 22u);
            uint32_t const maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t const temp2 = S0 + maj;

            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }

    char hex[65];
    for (int i = 0; i < 8; i++) {
        snprintf(hex + i * 8, 9, "%08x", h[i]);
    }
    hex[64] = '\0';
    return std::string(hex);
}

} // namespace filament::tools::taa
