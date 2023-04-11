/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "ColorSpaceUtils.h"

namespace filament {

using namespace math;

/*
 * The section below is subject to the following license.
 * Source: https://bottosson.github.io/posts/gamutclipping/
 *
 * Copyright (c) 2021 Björn Ottosson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// TODO: Replace compute_max_saturation() with solution from
//       https://simonstechblog.blogspot.com/2021/06/implementing-gamut-mapping.html to
//       target arbitrary output gamuts. The solution below works only for sRGB/Rec.709

// Finds the maximum saturation possible for a given hue that fits in sRGB
// Saturation here is defined as S = C/L
// a and b must be normalized so a^2 + b^2 == 1
static float compute_max_saturation(float a, float b) noexcept {
    // Max saturation will be when one of r, g or b goes below zero.

    // Select different coefficients depending on which component goes below zero first
    float k0, k1, k2, k3, k4, wl, wm, ws;

    if (-1.88170328f * a - 0.80936493f * b > 1) {
        // Red component
        k0 = +1.19086277f;
        k1 = +1.76576728f;
        k2 = +0.59662641f;
        k3 = +0.75515197f;
        k4 = +0.56771245f;
        wl = +4.0767416621f;
        wm = -3.3077115913f;
        ws = +0.2309699292f;
    } else if (1.81444104f * a - 1.19445276f * b > 1) {
        // Green component
        k0 = +0.73956515f;
        k1 = -0.45954404f;
        k2 = +0.08285427f;
        k3 = +0.12541070f;
        k4 = +0.14503204f;
        wl = -1.2681437731f;
        wm = +2.6097574011f;
        ws = -0.3413193965f;
    } else {
        // Blue component
        k0 = +1.35733652f;
        k1 = -0.00915799f;
        k2 = -1.15130210f;
        k3 = -0.50559606f;
        k4 = +0.00692167f;
        wl = -0.0041960863f;
        wm = -0.7034186147f;
        ws = +1.7076147010f;
    }

    // Approximate max saturation using a polynomial:
    float S = k0 + k1 * a + k2 * b + k3 * a * a + k4 * a * b;

    // Do one step Halley's method to get closer
    // this gives an error less than 10e6, except for some blue hues where the dS/dh
    // is close to infinite
    // this should be sufficient for most applications, otherwise do two/three steps

    float const k_l = +0.3963377774f * a + 0.2158037573f * b;
    float const k_m = -0.1055613458f * a - 0.0638541728f * b;
    float const k_s = -0.0894841775f * a - 1.2914855480f * b;

    {
        float const l_ = 1.f + S * k_l;
        float const m_ = 1.f + S * k_m;
        float const s_ = 1.f + S * k_s;

        float const l = l_ * l_ * l_;
        float const m = m_ * m_ * m_;
        float const s = s_ * s_ * s_;

        float const l_dS = 3.f * k_l * l_ * l_;
        float const m_dS = 3.f * k_m * m_ * m_;
        float const s_dS = 3.f * k_s * s_ * s_;

        float const l_dS2 = 6.f * k_l * k_l * l_;
        float const m_dS2 = 6.f * k_m * k_m * m_;
        float const s_dS2 = 6.f * k_s * k_s * s_;

        float const f = wl * l + wm * m + ws * s;
        float const f1 = wl * l_dS + wm * m_dS + ws * s_dS;
        float const f2 = wl * l_dS2 + wm * m_dS2 + ws * s_dS2;

        S = S - f * f1 / (f1 * f1 - 0.5f * f * f2);
    }

    return S;
}

// finds L_cusp and C_cusp for a given hue
// a and b must be normalized so a^2 + b^2 == 1
static float2 find_cusp(float a, float b) noexcept {
    // First, find the maximum saturation (saturation S = C/L)
    float const S_cusp = compute_max_saturation(a, b);

    // Convert to linear sRGB to find the first point where at least one of r,g or b >= 1:
    float3 const rgb_at_max = OkLab_to_sRGB({1.0f, S_cusp * a, S_cusp * b});
    float const L_cusp = std::cbrt(1.0f / max(rgb_at_max));
    float const C_cusp = L_cusp * S_cusp;

    return { L_cusp, C_cusp };
}

// Finds intersection of the line defined by
// L = L0 * (1 - t) + t * L1;
// C = t * C1;
// a and b must be normalized so a^2 + b^2 == 1
static float find_gamut_intersection(float a, float b, float L1, float C1, float L0) noexcept {
    // Find the cusp of the gamut triangle
    float2 const cusp = find_cusp(a, b);

    // Find the intersection for upper and lower half separately
    float t;
    if (((L1 - L0) * cusp.y - (cusp.x - L0) * C1) <= 0.f) {
        // Lower half
        t = cusp.y * L0 / (C1 * cusp.x + cusp.y * (L0 - L1));
    } else {
        // Upper half

        // First intersect with triangle
        t = cusp.y * (L0 - 1.f) / (C1 * (cusp.x - 1.f) + cusp.y * (L0 - L1));

        // Then one step Halley's method
        {
            float const dL = L1 - L0;
            float const dC = C1;

            float const k_l = +0.3963377774f * a + 0.2158037573f * b;
            float const k_m = -0.1055613458f * a - 0.0638541728f * b;
            float const k_s = -0.0894841775f * a - 1.2914855480f * b;

            float const l_dt = dL + dC * k_l;
            float const m_dt = dL + dC * k_m;
            float const s_dt = dL + dC * k_s;


            // If higher accuracy is required, 2 or 3 iterations of the
            // following block can be used:
            {
                float const L = L0 * (1.f - t) + t * L1;
                float const C = t * C1;

                float const l_ = L + C * k_l;
                float const m_ = L + C * k_m;
                float const s_ = L + C * k_s;

                float const l = l_ * l_ * l_;
                float const m = m_ * m_ * m_;
                float const s = s_ * s_ * s_;

                float const ldt = 3 * l_dt * l_ * l_;
                float const mdt = 3 * m_dt * m_ * m_;
                float const sdt = 3 * s_dt * s_ * s_;

                float const ldt2 = 6 * l_dt * l_dt * l_;
                float const mdt2 = 6 * m_dt * m_dt * m_;
                float const sdt2 = 6 * s_dt * s_dt * s_;

                float const r = 4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s - 1;
                float const r1 = 4.0767416621f * ldt - 3.3077115913f * mdt + 0.2309699292f * sdt;
                float const r2 = 4.0767416621f * ldt2 - 3.3077115913f * mdt2 + 0.2309699292f * sdt2;

                float const u_r = r1 / (r1 * r1 - 0.5f * r * r2);
                float t_r = -r * u_r;

                float const g = -1.2681437731f * l + 2.6097574011f * m - 0.3413193965f * s - 1;
                float const g1 = -1.2681437731f * ldt + 2.6097574011f * mdt - 0.3413193965f * sdt;
                float const g2 = -1.2681437731f * ldt2 + 2.6097574011f * mdt2 - 0.3413193965f * sdt2;

                float const u_g = g1 / (g1 * g1 - 0.5f * g * g2);
                float t_g = -g * u_g;

                float const b0 = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s - 1;
                float const b1 = -0.0041960863f * ldt - 0.7034186147f * mdt + 1.7076147010f * sdt;
                float const b2 = -0.0041960863f * ldt2 - 0.7034186147f * mdt2 + 1.7076147010f * sdt2;

                float const u_b = b1 / (b1 * b1 - 0.5f * b0 * b2);
                float t_b = -b0 * u_b;

                t_r = u_r >= 0.f ? t_r : std::numeric_limits<float>::max();
                t_g = u_g >= 0.f ? t_g : std::numeric_limits<float>::max();
                t_b = u_b >= 0.f ? t_b : std::numeric_limits<float>::max();

                t += min(t_r, min(t_g, t_b));
            }
        }
    }

    return t;
}

constexpr float sgn(float x) noexcept {
    return (float) (0.f < x) - (float) (x < 0.f);
}

// Adaptive L0 = 0.5, with alpha set to 0.05 by default
// The threshold parameters defines a flexible range above 1.0 and below 0.0 where out-of-gamut
// values are still considered in-gamut. This helps control for inaccuracies in the previous
// color grading steps that may slightly deviate from in-gamut values when they shouldn't.
inline float3 gamut_clip_adaptive_L0_0_5(float3 rgb,
        float alpha = 0.05f, float threshold = 0.03f) noexcept {

    if (all(lessThanEqual(rgb, float3{1.0f + threshold})) &&
            all(greaterThanEqual(rgb, float3{-threshold}))) {
        return rgb;
    }

    float3 const lab = sRGB_to_OkLab(rgb);

    float const L = lab.x;
    float const eps = 0.00001f;
    float const C = max(eps, std::sqrt(lab.y * lab.y + lab.z * lab.z));
    float const a_ = lab.y / C;
    float const b_ = lab.z / C;

    float const Ld = L - 0.5f;
    float const e1 = 0.5f + std::abs(Ld) + alpha * C;
    float const L0 = 0.5f * (1.0f + sgn(Ld) * (e1 - std::sqrt(e1 * e1 - 2.0f * std::abs(Ld))));

    float const t = find_gamut_intersection(a_, b_, L, C, L0);
    float const L_clipped = L0 * (1.f - t) + t * L;
    float const C_clipped = t * C;

    return OkLab_to_sRGB({L_clipped, C_clipped * a_, C_clipped * b_});
}

float3 gamutMapping_sRGB(float3 rgb) noexcept {
    return gamut_clip_adaptive_L0_0_5(rgb);
}

/*
 * End source: https://bottosson.github.io/posts/gamutclipping/
 */

} //namespace filament
