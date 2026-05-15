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

#ifndef TNT_FILAMENT_DETAILS_COLORGRADINGNEON_H
#define TNT_FILAMENT_DETAILS_COLORGRADINGNEON_H

#if defined(__ARM_NEON)

#include <utils/compiler.h>

#include <arm_neon.h>

namespace filament {

/**
 * Computes the base-2 logarithm for four 32-bit floating point values simultaneously.
 * Uses IEEE-754 bit manipulation to extract the exponent and mantissa, followed by a
 * degree-4 Remez minimax polynomial approximation on the interval [1, 2).
 * 
 * @param x Input vector of positive floating-point numbers.
 * @return float32x4_t Vector containing log2(x).
 */
UTILS_ALWAYS_INLINE
inline float32x4_t v_log2q_f32(float32x4_t const x) {
    uint32x4_t const ix = vreinterpretq_u32_f32(x);
    int32x4_t const exp = vsubq_s32(vreinterpretq_s32_u32(vshrq_n_u32(ix, 23)), vdupq_n_s32(127));
    uint32x4_t mantissa = vbicq_u32(ix, vdupq_n_u32(0x7F800000));
    mantissa = vorrq_u32(mantissa, vdupq_n_u32(0x3F800000));
    float32x4_t const m = vreinterpretq_f32_u32(mantissa);
    float32x4_t const t = vsubq_f32(m, vdupq_n_f32(1.0f));

    float32x4_t p = vdupq_n_f32(-0.07267851f);
    p = vmlaq_f32(vdupq_n_f32(0.21985102f), p, t);
    p = vmlaq_f32(vdupq_n_f32(-0.34730548f), p, t);
    p = vmlaq_f32(vdupq_n_f32(0.47868481f), p, t);
    p = vmlaq_f32(vdupq_n_f32(-0.72116596f), p, t);
    p = vmlaq_f32(vdupq_n_f32(1.44268988f), p, t);
    p = vmulq_f32(p, t);

    return vaddq_f32(vcvtq_f32_s32(exp), p);
}

/**
 * Computes the base-2 exponential for four 32-bit floating point values simultaneously.
 * Clamps the input to prevent overflow/underflow, decomposes x into integer and fractional
 * components, and evaluates a polynomial approximation on the fractional part.
 * 
 * @param x Input vector of exponents.
 * @return float32x4_t Vector containing 2^x.
 */
UTILS_ALWAYS_INLINE
inline float32x4_t v_exp2q_f32(float32x4_t x) {
    x = vmaxq_f32(vminq_f32(x, vdupq_n_f32(126.0f)), vdupq_n_f32(-126.0f));
    int32x4_t i = vcvtq_s32_f32(x);
    float32x4_t const fi = vcvtq_f32_s32(i);
    float32x4_t f = vsubq_f32(x, fi);

    uint32x4_t const mask = vcltq_f32(f, vdupq_n_f32(0.0f));
    f = vaddq_f32(f, vbslq_f32(mask, vdupq_n_f32(1.0f), vdupq_n_f32(0.0f)));
    i = vaddq_s32(i, vreinterpretq_s32_u32(mask));

    float32x4_t p = vdupq_n_f32(0.00015355f);
    p = vmlaq_f32(vdupq_n_f32(0.00133596f), p, f);
    p = vmlaq_f32(vdupq_n_f32(0.00961413f), p, f);
    p = vmlaq_f32(vdupq_n_f32(0.05550872f), p, f);
    p = vmlaq_f32(vdupq_n_f32(0.24022242f), p, f);
    p = vmlaq_f32(vdupq_n_f32(0.69314705f), p, f);
    p = vmlaq_f32(vdupq_n_f32(1.0f), p, f);

    uint32x4_t const e = vshlq_n_u32(vreinterpretq_u32_s32(vaddq_s32(i, vdupq_n_s32(127))), 23);
    return vmulq_f32(p, vreinterpretq_f32_u32(e));
}

/**
 * Computes x^y for four floating point values simultaneously, where y is a uniform scalar.
 * Evaluated efficiently as 2^(y * log2(x)). Clamps negative inputs to 0.0f.
 * 
 * @param x Vector of base values.
 * @param y Scalar exponent.
 * @return float32x4_t Vector containing x^y.
 */
UTILS_ALWAYS_INLINE
inline float32x4_t v_powq_f32(float32x4_t const x, float const y) {
    uint32x4_t const mask = vcleq_f32(x, vdupq_n_f32(0.0f));
    float32x4_t const res = v_exp2q_f32(vmulq_n_f32(v_log2q_f32(x), y));
    return vbslq_f32(mask, vdupq_n_f32(0.0f), res);
}

/**
 * Computes the four-quadrant arctangent atan2(y, x) for four pairs of floating point values.
 * Uses range reduction to [0, 1] and evaluates a Remez minimax polynomial approximation.
 * 
 * @param y Vector of y coordinates.
 * @param x Vector of x coordinates.
 * @return float32x4_t Vector containing angles in radians [-pi, pi].
 */
UTILS_ALWAYS_INLINE
inline float32x4_t v_atan2q_f32(float32x4_t const y, float32x4_t const x) {
    float32x4_t const abs_y = vabsq_f32(y);
    float32x4_t const abs_x = vabsq_f32(x);
    uint32x4_t const x_lt_y = vcltq_f32(abs_x, abs_y);
    float32x4_t const min_xy = vbslq_f32(x_lt_y, abs_x, abs_y);
    float32x4_t max_xy = vbslq_f32(x_lt_y, abs_y, abs_x);

    uint32x4_t const z_mask = vceqq_f32(max_xy, vdupq_n_f32(0.0f));
    max_xy = vbslq_f32(z_mask, vdupq_n_f32(1.0f), max_xy);

    float32x4_t const a = vdivq_f32(min_xy, max_xy);

    float32x4_t const a2 = vmulq_f32(a, a);
    float32x4_t p = vdupq_n_f32(0.020835f);
    p = vmlaq_f32(vdupq_n_f32(-0.085133f), p, a2);
    p = vmlaq_f32(vdupq_n_f32(0.180141f), p, a2);
    p = vmlaq_f32(vdupq_n_f32(-0.330299f), p, a2);
    p = vmlaq_f32(vdupq_n_f32(0.999866f), p, a2);
    p = vmulq_f32(p, a);

    p = vbslq_f32(x_lt_y, vsubq_f32(vdupq_n_f32(1.57079632679f), p), p);

    uint32x4_t const x_sign = vcltq_f32(x, vdupq_n_f32(0.0f));
    p = vbslq_f32(x_sign, vsubq_f32(vdupq_n_f32(3.14159265359f), p), p);

    uint32x4_t const y_sign = vcltq_f32(y, vdupq_n_f32(0.0f));
    p = vbslq_f32(y_sign, vnegq_f32(p), p);

    return p;
}


/**
 * Vectorized sRGB Opto-Electronic Transfer Function (OETF).
 * Evaluates the piece-wise linear/exponential sRGB transfer function on four RGB color
 * values simultaneously. Evaluated branchlessly using vector selects.
 * 
 * @param cg_r Reference to vector of Red channels, modified in-place.
 * @param cg_g Reference to vector of Green channels, modified in-place.
 * @param cg_b Reference to vector of Blue channels, modified in-place.
 */
UTILS_ALWAYS_INLINE
inline void v_oetf_sRGB(float32x4_t& cg_r, float32x4_t& cg_g, float32x4_t& cg_b) {
    float32x4_t const r_cond = vcleq_f32(cg_r, vdupq_n_f32(0.0031308f));
    float32x4_t const g_cond = vcleq_f32(cg_g, vdupq_n_f32(0.0031308f));
    float32x4_t const b_cond = vcleq_f32(cg_b, vdupq_n_f32(0.0031308f));

    float32x4_t const r_lin = vmulq_n_f32(cg_r, 12.92f);
    float32x4_t const g_lin = vmulq_n_f32(cg_g, 12.92f);
    float32x4_t const b_lin = vmulq_n_f32(cg_b, 12.92f);

    float32x4_t const r_pow = vsubq_f32(vmulq_n_f32(v_powq_f32(cg_r, 1.0f / 2.4f), 1.055f), vdupq_n_f32(0.055f));
    float32x4_t const g_pow = vsubq_f32(vmulq_n_f32(v_powq_f32(cg_g, 1.0f / 2.4f), 1.055f), vdupq_n_f32(0.055f));
    float32x4_t const b_pow = vsubq_f32(vmulq_n_f32(v_powq_f32(cg_b, 1.0f / 2.4f), 1.055f), vdupq_n_f32(0.055f));

    cg_r = vbslq_f32(r_cond, r_lin, r_pow);
    cg_g = vbslq_f32(g_cond, g_lin, g_pow);
    cg_b = vbslq_f32(b_cond, b_lin, b_pow);
}

/**
 * Vectorized Channel Mixer.
 */
UTILS_ALWAYS_INLINE
inline void v_channelMixer(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        float3 const& outRed, float3 const& outGreen, float3 const& outBlue) {
    float32x4_t const m_r = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vr, outRed.r), vg, outRed.g), vb, outRed.b);
    float32x4_t const m_g = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vr, outGreen.r), vg, outGreen.g), vb, outGreen.b);
    float32x4_t const m_b = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vr, outBlue.r), vg, outBlue.g), vb, outBlue.b);
    vr = m_r;
    vg = m_g;
    vb = m_b;
}

/**
 * Vectorized Tonal Ranges.
 */
UTILS_ALWAYS_INLINE
inline void v_tonalRanges(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        float3 const& luminance, float4 const& tonalRanges,
        float3 const& shadows, float3 const& midtones, float3 const& highlights) {
    float32x4_t const m_r = vr;
    float32x4_t const m_g = vg;
    float32x4_t const m_b = vb;

    float32x4_t const y = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(m_r, luminance.r), m_g, luminance.g), m_b, luminance.b);

    float32x4_t ts = vmulq_n_f32(vsubq_f32(y, vdupq_n_f32(tonalRanges.x)), 1.0f / (tonalRanges.y - tonalRanges.x));
    ts = vmaxq_f32(vminq_f32(ts, vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));
    float32x4_t const s_step = vsubq_f32(vdupq_n_f32(1.0f), vmulq_f32(vmulq_f32(ts, ts), vsubq_f32(vdupq_n_f32(3.0f), vmulq_n_f32(ts, 2.0f))));

    float32x4_t th = vmulq_n_f32(vsubq_f32(y, vdupq_n_f32(tonalRanges.z)), 1.0f / (tonalRanges.w - tonalRanges.z));
    th = vmaxq_f32(vminq_f32(th, vdupq_n_f32(1.0f)), vdupq_n_f32(0.0f));
    float32x4_t const h_step = vmulq_f32(vmulq_f32(th, th), vsubq_f32(vdupq_n_f32(3.0f), vmulq_n_f32(th, 2.0f)));

    float32x4_t const m_step = vsubq_f32(vsubq_f32(vdupq_n_f32(1.0f), s_step), h_step);

    vr = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vmulq_f32(m_r, s_step), shadows.r), vmulq_f32(m_r, m_step), midtones.r), vmulq_f32(m_r, h_step), highlights.r);
    vg = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vmulq_f32(m_g, s_step), shadows.g), vmulq_f32(m_g, m_step), midtones.g), vmulq_f32(m_g, h_step), highlights.g);
    vb = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vmulq_f32(m_b, s_step), shadows.b), vmulq_f32(m_b, m_step), midtones.b), vmulq_f32(m_b, h_step), highlights.b);
}

/**
 * Vectorized Linear to LogC.
 */
UTILS_ALWAYS_INLINE
inline float32x4_t v_toLogC(float32x4_t const x) {
    float32x4_t const ax_b = vmlaq_n_f32(vdupq_n_f32(0.047996f), x, 5.555556f);
    float32x4_t const l2 = v_log2q_f32(ax_b);
    return vmlaq_n_f32(vdupq_n_f32(0.386036f), l2, 0.244161f * 0.30102999566f);
}

/**
 * Vectorized ASC CDL.
 */
UTILS_ALWAYS_INLINE
inline float32x4_t v_cdl(float32x4_t const x, float const slope, float const offset, float const power) {
    float32x4_t const v = vmlaq_n_f32(vdupq_n_f32(offset), x, slope);
    float32x4_t const pv = v_powq_f32(v, power);
    uint32x4_t const cmp = vcleq_f32(v, vdupq_n_f32(0.0f));
    return vbslq_f32(cmp, v, pv);
}

/**
 * Vectorized Contrast.
 */
UTILS_ALWAYS_INLINE
inline void v_contrast(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        float const contrast, float const contrastOffset) {
    vr = vmlaq_n_f32(vdupq_n_f32(contrastOffset), vr, contrast);
    vg = vmlaq_n_f32(vdupq_n_f32(contrastOffset), vg, contrast);
    vb = vmlaq_n_f32(vdupq_n_f32(contrastOffset), vb, contrast);
}

/**
 * Vectorized LogC to Linear.
 */
UTILS_ALWAYS_INLINE
inline float32x4_t v_toLinear(float32x4_t const x) {
    float32x4_t const y = vmulq_n_f32(vsubq_f32(x, vdupq_n_f32(0.386036f)), 4.095658f * 3.32192809489f);
    float32x4_t const p10 = v_exp2q_f32(y);
    return vmulq_n_f32(vsubq_f32(p10, vdupq_n_f32(0.047996f)), 0.180000f);
}

/**
 * Vectorized Vibrance.
 */
UTILS_ALWAYS_INLINE
inline void v_vibrance(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        float const vibrance, float3 const& luminance) {
    float32x4_t const lin_r = vr;
    float32x4_t const lin_g = vg;
    float32x4_t const lin_b = vb;

    float32x4_t const max_gb = vmaxq_f32(lin_g, lin_b);
    float32x4_t const r_vib = vsubq_f32(lin_r, max_gb);
    float32x4_t const exp_arg = vmulq_n_f32(r_vib, -3.0f * 1.44269504089f);
    float32x4_t const ex = v_exp2q_f32(exp_arg);
    float32x4_t const denom = vaddq_f32(vdupq_n_f32(1.0f), ex);
    float32x4_t recip = vrecpeq_f32(denom);
    recip = vmulq_f32(recip, vrecpsq_f32(denom, recip));
    float32x4_t const s_vib = vmlaq_n_f32(vdupq_n_f32(1.0f), recip, vibrance - 1.0f);

    float32x4_t const one_minus_s = vsubq_f32(vdupq_n_f32(1.0f), s_vib);
    float32x4_t const l_r = vmulq_n_f32(one_minus_s, luminance.r);
    float32x4_t const l_g = vmulq_n_f32(one_minus_s, luminance.g);
    float32x4_t const l_b = vmulq_n_f32(one_minus_s, luminance.b);
    float32x4_t const dot_vl = vmlaq_f32(vmlaq_f32(vmulq_f32(lin_r, l_r), lin_g, l_g), lin_b, l_b);

    vr = vmlaq_f32(dot_vl, lin_r, s_vib);
    vg = vmlaq_f32(dot_vl, lin_g, s_vib);
    vb = vmlaq_f32(dot_vl, lin_b, s_vib);
}

/**
 * Vectorized Saturation.
 */
UTILS_ALWAYS_INLINE
inline void v_saturation(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        float const saturation, float3 const& luminance) {
    float32x4_t const y_sat = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(vr, luminance.r), vg, luminance.g), vb, luminance.b);
    float32x4_t const sat_r = vmlaq_n_f32(y_sat, vsubq_f32(vr, y_sat), saturation);
    float32x4_t const sat_g = vmlaq_n_f32(y_sat, vsubq_f32(vg, y_sat), saturation);
    float32x4_t const sat_b = vmlaq_n_f32(y_sat, vsubq_f32(vb, y_sat), saturation);

    vr = vmaxq_f32(sat_r, vdupq_n_f32(0.0f));
    vg = vmaxq_f32(sat_g, vdupq_n_f32(0.0f));
    vb = vmaxq_f32(sat_b, vdupq_n_f32(0.0f));
}

/**
 * Vectorized Curves.
 */
UTILS_ALWAYS_INLINE
inline void v_curves(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        float3 const& shadowGamma, float3 const& midPoint, float3 const& highlightScale, float3 const& curvesDenominator) {
    auto curve_ch = [](float32x4_t const v, float const gamma, float const mid, float const scale, float const d) {
        float32x4_t const dark = vmulq_n_f32(v_powq_f32(v, gamma), d);
        float32x4_t const light = vmlaq_n_f32(vdupq_n_f32(mid), vsubq_f32(v, vdupq_n_f32(mid)), scale);
        uint32x4_t const cmp = vcleq_f32(v, vdupq_n_f32(mid));
        return vbslq_f32(cmp, dark, light);
    };
    vr = curve_ch(vr, shadowGamma.r, midPoint.r, highlightScale.r, curvesDenominator.r);
    vg = curve_ch(vg, shadowGamma.g, midPoint.g, highlightScale.g, curvesDenominator.g);
    vb = curve_ch(vb, shadowGamma.b, midPoint.b, highlightScale.b, curvesDenominator.b);
}

/**
 * Vectorized Scotopic Adaptation (Purkinje shift).
 */
UTILS_ALWAYS_INLINE
inline void v_scotopicAdaptation(float32x4_t& vr, float32x4_t& vg, float32x4_t& vb,
        float const nightAdaptation) {
    constexpr float3 L{7.696847f, 18.424824f,  2.068096f};
    constexpr float3 M{2.431137f, 18.697937f,  3.012463f};
    constexpr float3 S{0.289117f,  1.401833f, 13.792292f};
    constexpr float3 R{0.466386f, 15.564362f, 10.059963f};

    constexpr mat3f LMS_to_RGB = inverse(transpose(mat3f{L, M, S}));

    constexpr float3 m{0.63721f, 0.39242f, 1.6064f};
    constexpr float3 k{0.2f, 0.2f, 0.3f};

    constexpr mat3f opponent_to_LMS{
        -0.5f, 0.5f, 0.0f,
         0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 1.0f
    };

    constexpr float K_ = 45.0f;
    constexpr float S_ = 10.0f;
    constexpr float k3 = 0.6f;
    constexpr float rw = 0.139f;
    constexpr float p  = 0.6189f;

    constexpr mat3f weightedRodResponse = (K_ / S_) * (mat3f{
       -(k3 + rw),       p * k3,          p * S_,
        1.0f + k3 * rw, (1.0f - p) * k3, (1.0f - p) * S_,
        0.0f,            1.0f,            0.0f
    } * mat3f{k} * inverse(mat3f{m}));

    constexpr float logExposure = 380.0f;

    float32x4_t const ev_r = vmulq_n_f32(vr, logExposure);
    float32x4_t const ev_g = vmulq_n_f32(vg, logExposure);
    float32x4_t const ev_b = vmulq_n_f32(vb, logExposure);

    float32x4_t const q_L = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(ev_r, L.r), ev_g, L.g), ev_b, L.b);
    float32x4_t const q_M = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(ev_r, M.r), ev_g, M.g), ev_b, M.b);
    float32x4_t const q_S = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(ev_r, S.r), ev_g, S.g), ev_b, S.b);
    float32x4_t const q_R = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(ev_r, R.r), ev_g, R.g), ev_b, R.b);

    auto calc_g = [](float32x4_t const q_ch, float32x4_t const rod, float const k_val, float const m_val) {
        float32x4_t arg = vmlaq_n_f32(q_ch, rod, k_val);
        arg = vmulq_n_f32(arg, 0.33f / m_val);
        arg = vmaxq_f32(arg, vdupq_n_f32(0.0f));
        arg = vaddq_f32(vdupq_n_f32(1.0f), arg);
        float32x4_t rsqrt = vrsqrteq_f32(arg);
        rsqrt = vmulq_f32(rsqrt, vrsqrtsq_f32(arg, vmulq_f32(rsqrt, rsqrt)));
        return rsqrt;
    };

    float32x4_t const g_L = calc_g(q_L, q_R, k.r, m.r);
    float32x4_t const g_M = calc_g(q_M, q_R, k.g, m.g);
    float32x4_t const g_S = calc_g(q_S, q_R, k.b, m.b);

    float32x4_t const r_factor = vmulq_n_f32(q_R, nightAdaptation);
    float32x4_t const f_L = vmulq_f32(g_L, r_factor);
    float32x4_t const f_M = vmulq_f32(g_M, r_factor);
    float32x4_t const f_S = vmulq_f32(g_S, r_factor);

    // deltaOpponent = weightedRodResponse * f
    float32x4_t const d_r = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(f_L, weightedRodResponse[0][0]), f_M, weightedRodResponse[1][0]), f_S, weightedRodResponse[2][0]);
    float32x4_t const d_g = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(f_L, weightedRodResponse[0][1]), f_M, weightedRodResponse[1][1]), f_S, weightedRodResponse[2][1]);
    float32x4_t const d_b = vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(f_L, weightedRodResponse[0][2]), f_M, weightedRodResponse[1][2]), f_S, weightedRodResponse[2][2]);

    // qHat = q.rgb + opponent_to_LMS * deltaOpponent
    float32x4_t const qh_L = vaddq_f32(q_L, vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(d_r, opponent_to_LMS[0][0]), d_g, opponent_to_LMS[1][0]), d_b, opponent_to_LMS[2][0]));
    float32x4_t const qh_M = vaddq_f32(q_M, vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(d_r, opponent_to_LMS[0][1]), d_g, opponent_to_LMS[1][1]), d_b, opponent_to_LMS[2][1]));
    float32x4_t const qh_S = vaddq_f32(q_S, vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(d_r, opponent_to_LMS[0][2]), d_g, opponent_to_LMS[1][2]), d_b, opponent_to_LMS[2][2]));

    // (LMS_to_RGB * qHat) / logExposure
    constexpr float invLogExposure = 1.0f / logExposure;
    vr = vmulq_n_f32(vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(qh_L, LMS_to_RGB[0][0]), qh_M, LMS_to_RGB[1][0]), qh_S, LMS_to_RGB[2][0]), invLogExposure);
    vg = vmulq_n_f32(vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(qh_L, LMS_to_RGB[0][1]), qh_M, LMS_to_RGB[1][1]), qh_S, LMS_to_RGB[2][1]), invLogExposure);
    vb = vmulq_n_f32(vmlaq_n_f32(vmlaq_n_f32(vmulq_n_f32(qh_L, LMS_to_RGB[0][2]), qh_M, LMS_to_RGB[1][2]), qh_S, LMS_to_RGB[2][2]), invLogExposure);
}

} // namespace filament

#endif // defined(__ARM_NEON)

#endif // TNT_FILAMENT_DETAILS_COLORGRADINGNEON_H
