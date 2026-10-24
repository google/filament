/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef IMAGE_COLORTRANSFORM_H_
#define IMAGE_COLORTRANSFORM_H_

#include <image/LinearImage.h>

#include <utils/compiler.h>

#include <math/half.h>
#include <math/scalar.h>
#include <math/vec3.h>
#include <math/vec4.h>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <type_traits>

namespace image {

namespace math = filament::math;

#if defined(__ARM_NEON)
namespace details {

UTILS_ALWAYS_INLINE
inline float32x4_t v_divq_f32(float32x4_t const a, float32x4_t const b) noexcept {
#if defined(__aarch64__)
    return vdivq_f32(a, b);
#else
    float32x4_t recip = vrecpeq_f32(b);
    recip = vmulq_f32(recip, vrecpsq_f32(b, recip));
    recip = vmulq_f32(recip, vrecpsq_f32(b, recip));
    return vmulq_f32(a, recip);
#endif
}

UTILS_ALWAYS_INLINE
inline float32x4_t v_sqrtq_f32(float32x4_t const x) noexcept {
#if defined(__aarch64__)
    return vsqrtq_f32(x);
#else
    float32x4_t rsqrt = vrsqrteq_f32(x);
    rsqrt = vmulq_f32(rsqrt, vrsqrtsq_f32(x, vmulq_f32(rsqrt, rsqrt)));
    rsqrt = vmulq_f32(rsqrt, vrsqrtsq_f32(x, vmulq_f32(rsqrt, rsqrt)));
    float32x4_t const res = vmulq_f32(x, rsqrt);
    uint32x4_t const cmp = vceqq_f32(x, vdupq_n_f32(0.0f));
    return vbslq_f32(cmp, vdupq_n_f32(0.0f), res);
#endif
}

UTILS_ALWAYS_INLINE
inline float32x4_t v_ceilq_f32(float32x4_t const x) noexcept {
#if defined(__aarch64__) || (defined(__ARM_ARCH) && __ARM_ARCH >= 8)
    return vrndpq_f32(x);
#else
    float32x4_t const trunc = vcvtq_f32_s32(vcvtq_s32_f32(x));
    uint32x4_t const mask = vcgtq_f32(x, trunc);
    return vaddq_f32(trunc, vbslq_f32(mask, vdupq_n_f32(1.0f), vdupq_n_f32(0.0f)));
#endif
}

UTILS_ALWAYS_INLINE
inline float32x4_t v_saturateq_f32(float32x4_t const x) noexcept {
    return vminq_f32(vmaxq_f32(x, vdupq_n_f32(0.0f)), vdupq_n_f32(1.0f));
}

UTILS_ALWAYS_INLINE
inline float32x4_t v_log2q_f32(float32x4_t const x) noexcept {
    uint32x4_t const ix = vreinterpretq_u32_f32(x);
    int32x4_t exp = vsubq_s32(vreinterpretq_s32_u32(vshrq_n_u32(ix, 23)), vdupq_n_s32(127));
    uint32x4_t mantissa = vorrq_u32(vbicq_u32(ix, vdupq_n_u32(0x7F800000u)), vdupq_n_u32(0x3F800000u));

    // Fold mantissa from [1, 2) to [sqrt(0.5), sqrt(2)) for ~1 ULP accuracy across the interval.
    uint32x4_t const fold = vcgtq_u32(mantissa, vdupq_n_u32(0x3FB504F3u));
    exp = vsubq_s32(exp, vreinterpretq_s32_u32(fold));
    mantissa = vsubq_u32(mantissa, vandq_u32(fold, vdupq_n_u32(0x00800000u)));

    float32x4_t const m = vreinterpretq_f32_u32(mantissa);
    float32x4_t const t = vsubq_f32(m, vdupq_n_f32(1.0f));

    float32x4_t p = vdupq_n_f32(-0.14275974f);
    p = vmlaq_f32(vdupq_n_f32(0.23265257f), p, t);
    p = vmlaq_f32(vdupq_n_f32(-0.24927182f), p, t);
    p = vmlaq_f32(vdupq_n_f32(0.28728887f), p, t);
    p = vmlaq_f32(vdupq_n_f32(-0.36022517f), p, t);
    p = vmlaq_f32(vdupq_n_f32(0.48091671f), p, t);
    p = vmlaq_f32(vdupq_n_f32(-0.72135293f), p, t);
    p = vmlaq_f32(vdupq_n_f32(1.44269502f), p, t);
    p = vmulq_f32(p, t);

    return vaddq_f32(vcvtq_f32_s32(exp), p);
}

UTILS_ALWAYS_INLINE
inline float32x4_t v_exp2q_f32(float32x4_t x) noexcept {
    x = vmaxq_f32(vminq_f32(x, vdupq_n_f32(126.0f)), vdupq_n_f32(-126.0f));
    int32x4_t i = vcvtq_s32_f32(x);
    float32x4_t const fi = vcvtq_f32_s32(i);
    float32x4_t f = vsubq_f32(x, fi);

    uint32x4_t const mask = vcltq_f32(f, vdupq_n_f32(0.0f));
    f = vaddq_f32(f, vbslq_f32(mask, vdupq_n_f32(1.0f), vdupq_n_f32(0.0f)));
    i = vaddq_s32(i, vreinterpretq_s32_u32(mask));

    float32x4_t p = vdupq_n_f32(0.00020818f);
    p = vmlaq_f32(vdupq_n_f32(0.00126922f), p, f);
    p = vmlaq_f32(vdupq_n_f32(0.00965220f), p, f);
    p = vmlaq_f32(vdupq_n_f32(0.05549602f), p, f);
    p = vmlaq_f32(vdupq_n_f32(0.24022721f), p, f);
    p = vmlaq_f32(vdupq_n_f32(0.69314718f), p, f);
    p = vmlaq_f32(vdupq_n_f32(1.0f), p, f);

    uint32x4_t const e = vshlq_n_u32(vreinterpretq_u32_s32(vaddq_s32(i, vdupq_n_s32(127))), 23);
    return vmulq_f32(p, vreinterpretq_f32_u32(e));
}

UTILS_ALWAYS_INLINE
inline float32x4_t v_powq_f32(float32x4_t const x, float const y) noexcept {
    uint32x4_t const mask = vcleq_f32(x, vdupq_n_f32(0.0f));
    float32x4_t const res = v_exp2q_f32(vmulq_n_f32(v_log2q_f32(x), y));
    return vbslq_f32(mask, vdupq_n_f32(0.0f), res);
}

UTILS_ALWAYS_INLINE
inline float32x4_t v_linearTosRGB(float32x4_t const x) noexcept {
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 1.0f / 2.4f;
    uint32x4_t const cond = vcleq_f32(x, vdupq_n_f32(0.0031308f));
    float32x4_t const lin = vmulq_n_f32(x, 12.92f);
    float32x4_t const pow = vsubq_f32(vmulq_n_f32(v_powq_f32(x, p), a1), vdupq_n_f32(a));
    return vbslq_f32(cond, lin, pow);
}

UTILS_ALWAYS_INLINE
inline float32x4_t v_sRGBToLinear(float32x4_t const x) noexcept {
    constexpr float a = 0.055f;
    constexpr float inv_a1 = 1.0f / 1.055f;
    constexpr float p = 2.4f;
    uint32x4_t const cond = vcleq_f32(x, vdupq_n_f32(0.04045f));
    float32x4_t const lin = vmulq_n_f32(x, 1.0f / 12.92f);
    float32x4_t const base = vmulq_n_f32(vaddq_f32(x, vdupq_n_f32(a)), inv_a1);
    float32x4_t const pow = v_powq_f32(base, p);
    return vbslq_f32(cond, lin, pow);
}

UTILS_ALWAYS_INLINE
inline void v_linearToRGBM(float32x4_t& r, float32x4_t& g, float32x4_t& b, float32x4_t& a) noexcept {
    r = vmulq_n_f32(v_sqrtq_f32(r), 1.0f / 16.0f);
    g = vmulq_n_f32(v_sqrtq_f32(g), 1.0f / 16.0f);
    b = vmulq_n_f32(v_sqrtq_f32(b), 1.0f / 16.0f);

    float32x4_t const maxComp = vmaxq_f32(vmaxq_f32(r, g), vmaxq_f32(b, vdupq_n_f32(1.0f / 16.0f)));
    a = vminq_f32(maxComp, vdupq_n_f32(1.0f));
    a = v_divq_f32(v_ceilq_f32(vmulq_n_f32(a, 255.0f)), vdupq_n_f32(255.0f));

    r = v_saturateq_f32(v_divq_f32(r, a));
    g = v_saturateq_f32(v_divq_f32(g, a));
    b = v_saturateq_f32(v_divq_f32(b, a));
}

template<unsigned E, unsigned M>
UTILS_ALWAYS_INLINE
inline uint32x4_t v_fp_fromf(float32x4_t const f) noexcept {
    constexpr uint32_t infinityBits = ((1u << E) - 1u) << 23u;
    constexpr uint32_t magicBits = ((1u << (E - 1u)) - 1u) << 23u;
    constexpr uint32_t roundBit = 1u << (22 - M);
    constexpr uint32_t eraseMask = roundBit - 1u;

    uint32x4_t const negMask = vcltq_f32(f, vdupq_n_f32(0.0f));
    uint32x4_t const bits = vbicq_u32(vreinterpretq_u32_f32(f), vdupq_n_u32(0x80000000u));
    uint32x4_t const expFF = vceqq_u32(vshrq_n_u32(bits, 23), vdupq_n_u32(0xFFu));

    uint32x4_t norm = vbicq_u32(bits, vdupq_n_u32(eraseMask));
    norm = vaddq_u32(norm, vdupq_n_u32(roundBit));
    float32x4_t const magic = vreinterpretq_f32_u32(vdupq_n_u32(magicBits));
    float32x4_t const normFp = vmulq_f32(vreinterpretq_f32_u32(norm), magic);
    norm = vminq_u32(vreinterpretq_u32_f32(normFp), vdupq_n_u32(infinityBits));
    norm = vshrq_n_u32(norm, 23 - M);

    uint32x4_t const mant = vandq_u32(bits, vdupq_n_u32(0x7FFFFFu));
    uint32x4_t const isNan = vcgtq_u32(mant, vdupq_n_u32(0u));
    uint32x4_t const infNan = vorrq_u32(
            vdupq_n_u32(((1u << E) - 1u) << M),
            vandq_u32(isNan, vdupq_n_u32(1u << (M - 1u))));

    uint32x4_t const out = vbslq_u32(expFF, infNan, norm);
    return vbicq_u32(out, negMask);
}

template<typename T>
UTILS_ALWAYS_INLINE
inline float32x4_t v_load4_f32(T const* p) noexcept {
    if constexpr (std::is_same_v<T, uint8_t>) {
        uint32_t v;
        memcpy(&v, p, sizeof(uint32_t));
        uint8x8_t const u8 = vreinterpret_u8_u32(vdup_n_u32(v));
        uint16x4_t const u16 = vget_low_u16(vmovl_u8(u8));
        return vcvtq_f32_u32(vmovl_u16(u16));
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        return vcvtq_f32_u32(vmovl_u16(vld1_u16(p)));
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return vcvtq_f32_u32(vld1q_u32(p));
    } else if constexpr (std::is_same_v<T, float>) {
        return vld1q_f32(p);
    } else {
        return float32x4_t{ float(p[0]), float(p[1]), float(p[2]), float(p[3]) };
    }
}

template<typename T, typename PROCESS>
UTILS_ALWAYS_INLINE
inline float32x4_t v_load4_proc_f32(T const* p, PROCESS& proc) {
    return float32x4_t{
        float(proc(p[0])),
        float(proc(p[1])),
        float(proc(p[2])),
        float(proc(p[3]))
    };
}

template<typename T>
UTILS_ALWAYS_INLINE
inline void v_load4_rgba_rgb_f32(T const* p,
        float32x4_t& r, float32x4_t& g, float32x4_t& b) noexcept {
    if constexpr (std::is_same_v<T, uint8_t>) {
        uint32x4_t const px = vreinterpretq_u32_u8(vld1q_u8(p));
        uint32x4_t const mask = vdupq_n_u32(0xFFu);
        r = vcvtq_f32_u32(vandq_u32(px, mask));
        g = vcvtq_f32_u32(vandq_u32(vshrq_n_u32(px, 8), mask));
        b = vcvtq_f32_u32(vandq_u32(vshrq_n_u32(px, 16), mask));
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        uint16x4x4_t const u16 = vld4_u16(p);
        r = vcvtq_f32_u32(vmovl_u16(u16.val[0]));
        g = vcvtq_f32_u32(vmovl_u16(u16.val[1]));
        b = vcvtq_f32_u32(vmovl_u16(u16.val[2]));
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        uint32x4x4_t const u32 = vld4q_u32(p);
        r = vcvtq_f32_u32(u32.val[0]);
        g = vcvtq_f32_u32(u32.val[1]);
        b = vcvtq_f32_u32(u32.val[2]);
    } else if constexpr (std::is_same_v<T, float>) {
        float32x4x4_t const f32 = vld4q_f32(p);
        r = f32.val[0];
        g = f32.val[1];
        b = f32.val[2];
    } else {
        r = float32x4_t{ float(p[0]), float(p[4]), float(p[8]), float(p[12]) };
        g = float32x4_t{ float(p[1]), float(p[5]), float(p[9]), float(p[13]) };
        b = float32x4_t{ float(p[2]), float(p[6]), float(p[10]), float(p[14]) };
    }
}

template<typename T>
UTILS_ALWAYS_INLINE
inline void v_store3(T* d, float32x4_t const v) noexcept {
    if constexpr (std::is_same_v<T, uint8_t>) {
        uint16x4_t const u16 = vmovn_u32(vcvtq_u32_f32(v));
        uint8x8_t const u8 = vmovn_u16(vcombine_u16(u16, u16));
        d[0] = vget_lane_u8(u8, 0);
        d[1] = vget_lane_u8(u8, 1);
        d[2] = vget_lane_u8(u8, 2);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        uint16x4_t const u16 = vmovn_u32(vcvtq_u32_f32(v));
        d[0] = vget_lane_u16(u16, 0);
        d[1] = vget_lane_u16(u16, 1);
        d[2] = vget_lane_u16(u16, 2);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        uint32x4_t const u32 = vcvtq_u32_f32(v);
        d[0] = vgetq_lane_u32(u32, 0);
        d[1] = vgetq_lane_u32(u32, 1);
        d[2] = vgetq_lane_u32(u32, 2);
    } else {
        alignas(16) float tmp[4];
        vst1q_f32(tmp, v);
        d[0] = T(tmp[0]);
        d[1] = T(tmp[1]);
        d[2] = T(tmp[2]);
    }
}

template<typename T>
UTILS_ALWAYS_INLINE
inline void v_store4(T* d, float32x4_t const v) noexcept {
    if constexpr (std::is_same_v<T, uint8_t>) {
        uint16x4_t const u16 = vmovn_u32(vcvtq_u32_f32(v));
        uint8x8_t const u8 = vmovn_u16(vcombine_u16(u16, u16));
        uint32_t const packed = vget_lane_u32(vreinterpret_u32_u8(u8), 0);
        memcpy(d, &packed, sizeof(uint32_t));
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        vst1_u16(d, vmovn_u32(vcvtq_u32_f32(v)));
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        vst1q_u32(d, vcvtq_u32_f32(v));
    } else {
        alignas(16) float tmp[4];
        vst1q_f32(tmp, v);
        d[0] = T(tmp[0]);
        d[1] = T(tmp[1]);
        d[2] = T(tmp[2]);
        d[3] = T(tmp[3]);
    }
}

template<typename T>
UTILS_ALWAYS_INLINE
inline void v_store4_rgb(T* d, float32x4_t const r, float32x4_t const g,
        float32x4_t const b) noexcept {
    if constexpr (std::is_same_v<T, uint8_t>) {
        uint8x8x3_t const rgb = {{
            vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(r)), vdup_n_u16(0))),
            vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(g)), vdup_n_u16(0))),
            vmovn_u16(vcombine_u16(vmovn_u32(vcvtq_u32_f32(b)), vdup_n_u16(0)))
        }};
        vst3_lane_u8(d + 0, rgb, 0);
        vst3_lane_u8(d + 3, rgb, 1);
        vst3_lane_u8(d + 6, rgb, 2);
        vst3_lane_u8(d + 9, rgb, 3);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        uint16x4x3_t const rgb = {{
            vmovn_u32(vcvtq_u32_f32(r)),
            vmovn_u32(vcvtq_u32_f32(g)),
            vmovn_u32(vcvtq_u32_f32(b))
        }};
        vst3_u16(d, rgb);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        uint32x4x3_t const rgb = {{
            vcvtq_u32_f32(r),
            vcvtq_u32_f32(g),
            vcvtq_u32_f32(b)
        }};
        vst3q_u32(d, rgb);
    } else {
        alignas(16) float tr[4], tg[4], tb[4];
        vst1q_f32(tr, r);
        vst1q_f32(tg, g);
        vst1q_f32(tb, b);
        for (size_t k = 0; k < 4; ++k) {
            d[k * 3 + 0] = T(tr[k]);
            d[k * 3 + 1] = T(tg[k]);
            d[k * 3 + 2] = T(tb[k]);
        }
    }
}

template<typename T>
UTILS_ALWAYS_INLINE
inline void v_store4_rgba(T* d, float32x4_t const r, float32x4_t const g,
        float32x4_t const b, float32x4_t const a) noexcept {
    if constexpr (std::is_same_v<T, uint8_t>) {
        uint32x4_t const ru = vcvtq_u32_f32(r);
        uint32x4_t const gu = vcvtq_u32_f32(g);
        uint32x4_t const bu = vcvtq_u32_f32(b);
        uint32x4_t const au = vcvtq_u32_f32(a);
        uint32x4_t const rg = vsliq_n_u32(ru, gu, 8);
        uint32x4_t const ba = vsliq_n_u32(bu, au, 8);
        uint32x4_t const rgba = vsliq_n_u32(rg, ba, 16);
        vst1q_u8(d, vreinterpretq_u8_u32(rgba));
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        uint16x4x4_t const rgba = {{
            vmovn_u32(vcvtq_u32_f32(r)),
            vmovn_u32(vcvtq_u32_f32(g)),
            vmovn_u32(vcvtq_u32_f32(b)),
            vmovn_u32(vcvtq_u32_f32(a))
        }};
        vst4_u16(d, rgba);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        uint32x4x4_t const rgba = {{
            vcvtq_u32_f32(r),
            vcvtq_u32_f32(g),
            vcvtq_u32_f32(b),
            vcvtq_u32_f32(a)
        }};
        vst4q_u32(d, rgba);
    } else {
        alignas(16) float tr[4], tg[4], tb[4], ta[4];
        vst1q_f32(tr, r);
        vst1q_f32(tg, g);
        vst1q_f32(tb, b);
        vst1q_f32(ta, a);
        for (size_t k = 0; k < 4; ++k) {
            d[k * 4 + 0] = T(tr[k]);
            d[k * 4 + 1] = T(tg[k]);
            d[k * 4 + 2] = T(tb[k]);
            d[k * 4 + 3] = T(ta[k]);
        }
    }
}

} // namespace details
#endif // defined(__ARM_NEON)

template <typename T>
uint32_t linearToRGB_10_11_11_REV(const T& linear) {
    using fp11 = math::fp<0, 5, 6>;
    using fp10 = math::fp<0, 5, 5>;
    // the max value for a RGB_11_11_10 is {65024, 65024, 64512} :  (2 - 2^-M) * 2^(E-1)
    // we clamp to the min of that
    const fp11 r = fp11::fromf(std::min(64512.0f, linear[0]));
    const fp11 g = fp11::fromf(std::min(64512.0f, linear[1]));
    const fp10 b = fp10::fromf(std::min(64512.0f, linear[2]));
    const uint32_t ir = r.bits & 0x7FF;
    const uint32_t ig = g.bits & 0x7FF;
    const uint32_t ib = b.bits & 0x3FF;
    return (ib << 22) | (ig << 11) | ir;
}

template <typename T>
math::float4 linearToRGBM(const T& linear) {
    using math::float4;

    float4 RGBM(linear[0], linear[1], linear[2], 1.0f);

    // Linear to gamma space
    RGBM.rgb = sqrt(RGBM.rgb);
    // Set the range
    RGBM.rgb /= 16.0f;

    float maxComponent = std::max(std::max(RGBM.r, RGBM.g), std::max(RGBM.b, 1e-6f));
    // Don't let M go below 1 in the [0..16] range
    RGBM.a =  math::clamp(maxComponent, 1.0f / 16.0f, 1.0f);
    RGBM.a = std::ceil(RGBM.a * 255.0f) / 255.0f;

    RGBM.rgb = saturate(RGBM.rgb / RGBM.a);

    return RGBM;
}

template <typename T>
math::float3 RGBMtoLinear(const T& rgbm) {
    using math::float3;

    float3 linear(rgbm[0], rgbm[1], rgbm[2]);
    linear *= rgbm.a * 16.0f;
    // Gamma to linear space
    return linear * linear;
}

template <typename T>
math::float3 linearTosRGB(const T& linear) {
    using math::float3;
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 1 / 2.4f;
    float3 sRGB;
    for (size_t i=0 ; i<3 ; i++) {
        if (linear[i] <= 0.0031308f) {
            sRGB[i] = linear[i] * 12.92f;
        } else {
            sRGB[i] = a1 * std::pow(linear[i], p) - a;
        }
    }
    return sRGB;
}

inline float linearTosRGB(float linear) {
    if (linear <= 0.0031308f) {
        return linear * 12.92f;
    }
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 1 / 2.4f;
    return a1 * std::pow(linear, p) - a;
}

template<typename T>
T sRGBToLinear(const T& sRGB);

template<>
inline math::float3 sRGBToLinear(const math::float3& sRGB) {
    using math::float3;
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 2.4f;
    float3 linear;
    for (size_t i=0 ; i<3 ; i++) {
        if (sRGB[i] <= 0.04045f) {
            linear[i] = sRGB[i] * (1.0f / 12.92f);
        } else {
            linear[i] = std::pow((sRGB[i] + a) / a1, p);
        }
    }
    return linear;
}

template<>
inline math::float4 sRGBToLinear(const math::float4& sRGB) {
    using math::float4;
    constexpr float a = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float p = 2.4f;
    float4 linear;
    for (size_t i=0 ; i<3 ; i++) {
        if (sRGB[i] <= 0.04045f) {
            linear[i] = sRGB[i] * (1.0f / 12.92f);
        } else {
            linear[i] = std::pow((sRGB[i] + a) / a1, p);
        }
    }
    linear[3] = sRGB[3];
    return linear;
}

template<typename T>
T linearToSRGB(const T& color);

template<>
inline math::float3 linearToSRGB(const math::float3& color) {
    using math::float3;
    float3 sRGBColor{color};
    UTILS_NOUNROLL
    for (size_t i = 0; i < sRGBColor.size(); i++) {
        sRGBColor[i] = sRGBColor[i] <= 0.0031308f ?
                sRGBColor[i] * 12.92f : powf(sRGBColor[i], 1.0f / 2.4f) * 1.055f - 0.055f;
    }
    return sRGBColor;
}

// Creates a N-channel sRGB image from a linear floating-point image.
// The source image can have more than N channels, but only the first 3 are converted to sRGB.
template<typename T, int N = 3>
std::unique_ptr<uint8_t[]> fromLinearTosRGB(const LinearImage& image) {
    const size_t w = image.getWidth();
    const size_t h = image.getHeight();
    const size_t nchan = image.getChannels();
    assert(nchan >= N);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * N * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
#if defined(__ARM_NEON)
    constexpr float maxVal = float(std::numeric_limits<T>::max());
    float32x4_t const half = vdupq_n_f32(0.5f);
    float const* p = image.getPixelRef();
    size_t const totalPixels = w * h;
    if constexpr (N <= 3) {
        if (nchan == N) {
            size_t const total = totalPixels * N;
            size_t i = 0;
            for (; i + 4 <= total; i += 4) {
                float32x4_t v = details::v_linearTosRGB(vld1q_f32(p + i));
                v = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(v), maxVal), half);
                details::v_store4(d + i, v);
            }
            if (i < total) {
                float32x4_t v = vdupq_n_f32(0.0f);
                v = vsetq_lane_f32(p[i], v, 0);
                if (i + 1 < total) v = vsetq_lane_f32(p[i + 1], v, 1);
                if (i + 2 < total) v = vsetq_lane_f32(p[i + 2], v, 2);
                v = details::v_linearTosRGB(v);
                v = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(v), maxVal), half);
                alignas(16) float tmp[4];
                vst1q_f32(tmp, v);
                for (size_t k = 0; i + k < total; ++k) {
                    d[i + k] = T(tmp[k]);
                }
            }
            return dst;
        } else if constexpr (N == 1) {
            size_t i = 0;
            for (; i + 4 <= totalPixels; i += 4, p += 4 * nchan, d += 4) {
                float32x4_t r;
                if (nchan == 3) {
                    r = vld3q_f32(p).val[0];
                } else if (nchan == 4) {
                    r = vld4q_f32(p).val[0];
                } else {
                    r = float32x4_t{ p[0], p[nchan], p[2 * nchan], p[3 * nchan] };
                }
                r = details::v_linearTosRGB(r);
                r = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(r), maxVal), half);
                details::v_store4(d, r);
            }
            if (i < totalPixels) {
                float32x4_t r = vdupq_n_f32(0.0f);
                r = vsetq_lane_f32(p[0], r, 0);
                if (i + 1 < totalPixels) r = vsetq_lane_f32(p[nchan], r, 1);
                if (i + 2 < totalPixels) r = vsetq_lane_f32(p[2 * nchan], r, 2);
                r = details::v_linearTosRGB(r);
                r = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(r), maxVal), half);
                alignas(16) float tmp[4];
                vst1q_f32(tmp, r);
                for (size_t k = 0; i + k < totalPixels; ++k) {
                    d[k] = T(tmp[k]);
                }
            }
            return dst;
        } else if constexpr (N == 3) {
            size_t i = 0;
            if (nchan == 4) {
                for (; i + 4 <= totalPixels; i += 4, p += 16, d += 12) {
                    float32x4x4_t const rgba = vld4q_f32(p);
                    float32x4_t r = details::v_linearTosRGB(rgba.val[0]);
                    float32x4_t g = details::v_linearTosRGB(rgba.val[1]);
                    float32x4_t b = details::v_linearTosRGB(rgba.val[2]);
                    r = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(r), maxVal), half);
                    g = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(g), maxVal), half);
                    b = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(b), maxVal), half);
                    details::v_store4_rgb(d, r, g, b);
                }
                for (; i < totalPixels; ++i, p += 4, d += 3) {
                    float32x4_t v = details::v_linearTosRGB(vld1q_f32(p));
                    v = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(v), maxVal), half);
                    details::v_store3(d, v);
                }
                return dst;
            }
        }
    } else if constexpr (N == 4) {
        size_t i = 0;
        if (nchan == 4) {
            for (; i + 4 <= totalPixels; i += 4, p += 16, d += 16) {
                float32x4x4_t const rgba = vld4q_f32(p);
                float32x4_t r = details::v_linearTosRGB(rgba.val[0]);
                float32x4_t g = details::v_linearTosRGB(rgba.val[1]);
                float32x4_t b = details::v_linearTosRGB(rgba.val[2]);
                r = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(r), maxVal), half);
                g = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(g), maxVal), half);
                b = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(b), maxVal), half);
                float32x4_t const a = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(rgba.val[3]), maxVal), half);
                details::v_store4_rgba(d, r, g, b, a);
            }
        }
        for (; i < totalPixels; ++i, p += nchan, d += 4) {
            float32x4_t const v = vld1q_f32(p);
            float32x4_t srgb = details::v_linearTosRGB(v);
            srgb = vsetq_lane_f32(vgetq_lane_f32(v, 3), srgb, 3);
            srgb = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(srgb), maxVal), half);
            details::v_store4(d, srgb);
        }
        return dst;
    }
    for (size_t i = 0; i < totalPixels; ++i, p += nchan, d += N) {
        float32x4_t v = vdupq_n_f32(0.0f);
        if constexpr (N >= 1) v = vsetq_lane_f32(p[0], v, 0);
        if constexpr (N >= 2) v = vsetq_lane_f32(p[1], v, 1);
        if constexpr (N >= 3) v = vsetq_lane_f32(p[2], v, 2);
        v = details::v_linearTosRGB(v);
        v = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(v), maxVal), half);
        alignas(16) float tmp[4];
        vst1q_f32(tmp, v);
        for (int n = 0; n < N; ++n) {
            if (n < 3) {
                d[n] = T(tmp[n]);
            } else {
                float const target = math::saturate(p[n]) * maxVal + 0.5f;
                d[n] = T(target);
            }
        }
    }
#else
    for (size_t y = 0; y < h; ++y) {
        float const* p = image.getPixelRef(0, y);
        for (size_t x = 0; x < w; ++x, p += nchan, d += N) {
            for (int n = 0; n < N; n++) {
                const float source = n < 3 ? linearTosRGB(p[n]) : p[n];
                float target =  math::saturate(source) * std::numeric_limits<T>::max() + 0.5f;
                d[n] = T(target);
            }
        }
    }
#endif
    return dst;
}

// Creates a N-channel RGB u8 image from a f32 image.
template<typename T, int N = 3>
std::unique_ptr<uint8_t[]> fromLinearToRGB(const LinearImage& image) {
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    size_t channels = image.getChannels();
    assert(channels >= N);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * N * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
#if defined(__ARM_NEON)
    constexpr float maxVal = float(std::numeric_limits<T>::max());
    float32x4_t const half = vdupq_n_f32(0.5f);
    float const* p = image.getPixelRef();
    size_t const totalPixels = w * h;
    if (channels == N) {
        size_t const total = totalPixels * N;
        size_t i = 0;
        for (; i + 4 <= total; i += 4) {
            float32x4_t v = vld1q_f32(p + i);
            v = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(v), maxVal), half);
            details::v_store4(d + i, v);
        }
        for (; i < total; ++i) {
            float const target = math::saturate(p[i]) * maxVal + 0.5f;
            d[i] = T(target);
        }
        return dst;
    }
    if constexpr (N == 4) {
        for (size_t i = 0; i < totalPixels; ++i, p += channels, d += 4) {
            float32x4_t v = vld1q_f32(p);
            v = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(v), maxVal), half);
            details::v_store4(d, v);
        }
        return dst;
    } else if constexpr (N == 3) {
        size_t i = 0;
        if (channels == 4) {
            for (; i + 4 <= totalPixels; i += 4, p += 16, d += 12) {
                float32x4x4_t const rgba = vld4q_f32(p);
                float32x4_t const r = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(rgba.val[0]), maxVal), half);
                float32x4_t const g = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(rgba.val[1]), maxVal), half);
                float32x4_t const b = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(rgba.val[2]), maxVal), half);
                details::v_store4_rgb(d, r, g, b);
            }
        }
        for (; i < totalPixels; ++i, p += channels, d += 3) {
            float32x4_t v = vld1q_f32(p);
            v = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(v), maxVal), half);
            details::v_store3(d, v);
        }
        return dst;
    } else if constexpr (N == 1) {
        size_t i = 0;
        for (; i + 4 <= totalPixels; i += 4, p += 4 * channels, d += 4) {
            float32x4_t r;
            if (channels == 3) {
                r = vld3q_f32(p).val[0];
            } else if (channels == 4) {
                r = vld4q_f32(p).val[0];
            } else {
                r = float32x4_t{ p[0], p[channels], p[2 * channels], p[3 * channels] };
            }
            r = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(r), maxVal), half);
            details::v_store4(d, r);
        }
        for (; i < totalPixels; ++i, p += channels, ++d) {
            float const target = math::saturate(p[0]) * maxVal + 0.5f;
            d[0] = T(target);
        }
        return dst;
    }
    for (size_t i = 0; i < totalPixels; ++i, p += channels, d += N) {
        for (int n = 0; n < N; n++) {
            float const target = math::saturate(p[n]) * maxVal + 0.5f;
            d[n] = T(target);
        }
    }
#else
    for (size_t y = 0; y < h; ++y) {
        float const* p = image.getPixelRef(0, y);
        for (size_t x = 0; x < w; ++x, p += channels, d += N) {
            for (int n = 0; n < N; n++) {
                float target =  math::saturate(p[n]) * std::numeric_limits<T>::max() + 0.5f;
                d[n] = T(target);
            }
        }
    }
#endif
    return dst;
}

// Creates a 4-channel RGBM u8 image from a f32 image.
// The source image can have three or more channels, but only the first three are honored.
template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToRGBM(const LinearImage& image) {
    using namespace filament::math;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    UTILS_UNUSED_IN_RELEASE size_t channels = image.getChannels();
    assert(channels >= 3);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * 4 * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
#if defined(__ARM_NEON)
    constexpr float maxVal = float(std::numeric_limits<T>::max());
    float32x4_t const half = vdupq_n_f32(0.5f);
    float const* p = image.getPixelRef();
    size_t const total = w * h;
    size_t i = 0;
    for (; i + 4 <= total; i += 4, p += 4 * channels, d += 16) {
        float32x4_t r, g, b, a;
        if (channels == 3) {
            float32x4x3_t const rgb = vld3q_f32(p);
            r = rgb.val[0];
            g = rgb.val[1];
            b = rgb.val[2];
        } else if (channels == 4) {
            float32x4x4_t const rgba = vld4q_f32(p);
            r = rgba.val[0];
            g = rgba.val[1];
            b = rgba.val[2];
        } else {
            r = float32x4_t{ p[0], p[channels], p[2 * channels], p[3 * channels] };
            g = float32x4_t{ p[1], p[channels + 1], p[2 * channels + 1], p[3 * channels + 1] };
            b = float32x4_t{ p[2], p[channels + 2], p[2 * channels + 2], p[3 * channels + 2] };
        }
        details::v_linearToRGBM(r, g, b, a);
        r = vaddq_f32(vmulq_n_f32(r, maxVal), half);
        g = vaddq_f32(vmulq_n_f32(g, maxVal), half);
        b = vaddq_f32(vmulq_n_f32(b, maxVal), half);
        a = vaddq_f32(vmulq_n_f32(a, maxVal), half);
        details::v_store4_rgba(d, r, g, b, a);
    }
    for (; i < total; ++i, p += channels, d += 4) {
        float4 const l(linearToRGBM(*reinterpret_cast<float3 const*>(p)) * maxVal + 0.5f);
        for (size_t c = 0; c < 4; c++) {
            d[c] = T(l[c]);
        }
    }
#else
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x, d += 4) {
            auto src = image.get<float3>(uint32_t(x), uint32_t(y));
            float4 l(linearToRGBM(*src) * std::numeric_limits<T>::max() + 0.5f);
            for (size_t i = 0; i < 4; i++) {
                d[i] = T(l[i]);
            }
        }
    }
#endif
    return dst;
}

// Creates a 3-channel RGB_10_11_11_REV image from a f32 image.
// The source image can have three or more channels, but only the first three are honored.
inline std::unique_ptr<uint8_t[]> fromLinearToRGB_10_11_11_REV(const LinearImage& image) {
    using namespace filament::math;
    size_t w = image.getWidth();
    size_t h = image.getHeight();
    assert(image.getChannels() >= 3);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * sizeof(uint32_t)]);
    uint8_t* d = dst.get();
#if defined(__ARM_NEON)
    size_t const channels = image.getChannels();
    float const* p = image.getPixelRef();
    size_t const total = w * h;
    size_t i = 0;
    float32x4_t const maxVal = vdupq_n_f32(64512.0f);
    for (; i + 4 <= total; i += 4, p += 4 * channels, d += 4 * sizeof(uint32_t)) {
        float32x4_t r, g, b;
        if (channels == 3) {
            float32x4x3_t const rgb = vld3q_f32(p);
            r = rgb.val[0];
            g = rgb.val[1];
            b = rgb.val[2];
        } else if (channels == 4) {
            float32x4x4_t const rgba = vld4q_f32(p);
            r = rgba.val[0];
            g = rgba.val[1];
            b = rgba.val[2];
        } else {
            r = float32x4_t{ p[0], p[channels], p[2 * channels], p[3 * channels] };
            g = float32x4_t{ p[1], p[channels + 1], p[2 * channels + 1], p[3 * channels + 1] };
            b = float32x4_t{ p[2], p[channels + 2], p[2 * channels + 2], p[3 * channels + 2] };
        }
        uint32x4_t const ir = vandq_u32(details::v_fp_fromf<5, 6>(vminq_f32(r, maxVal)), vdupq_n_u32(0x7FFu));
        uint32x4_t const ig = vandq_u32(details::v_fp_fromf<5, 6>(vminq_f32(g, maxVal)), vdupq_n_u32(0x7FFu));
        uint32x4_t const ib = vandq_u32(details::v_fp_fromf<5, 5>(vminq_f32(b, maxVal)), vdupq_n_u32(0x3FFu));
        uint32x4_t const packed = vorrq_u32(vorrq_u32(vshlq_n_u32(ib, 22), vshlq_n_u32(ig, 11)), ir);
        vst1q_u32(reinterpret_cast<uint32_t*>(d), packed);
    }
    for (; i < total; ++i, p += channels, d += sizeof(uint32_t)) {
        uint32_t const v = linearToRGB_10_11_11_REV(*reinterpret_cast<float3 const*>(p));
        *reinterpret_cast<uint32_t*>(d) = v;
    }
#else
    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x, d += sizeof(uint32_t)) {
            auto src = image.get<float3>(uint32_t(x), uint32_t(y));
            uint32_t v = linearToRGB_10_11_11_REV(*src);
            *reinterpret_cast<uint32_t*>(d) = v;
        }
    }
#endif
    return dst;
}

// Creates a packed single-channel integer-based image from a floating-point image.
// For example if T is uint8_t, then this performs a transformation from [0,1] to [0,255].
template <typename T>
std::unique_ptr<uint8_t[]> fromLinearToGrayscale(const LinearImage& image) {
    const size_t w = image.getWidth();
    const size_t h = image.getHeight();
    assert(image.getChannels() == 1);
    std::unique_ptr<uint8_t[]> dst(new uint8_t[w * h * sizeof(T)]);
    T* d = reinterpret_cast<T*>(dst.get());
#if defined(__ARM_NEON)
    constexpr float maxVal = float(std::numeric_limits<T>::max());
    float32x4_t const half = vdupq_n_f32(0.5f);
    float const* p = image.getPixelRef();
    size_t const total = w * h;
    size_t i = 0;
    for (; i + 4 <= total; i += 4) {
        float32x4_t v = vld1q_f32(p + i);
        v = vaddq_f32(vmulq_n_f32(details::v_saturateq_f32(v), maxVal), half);
        details::v_store4(d + i, v);
    }
    for (; i < total; ++i) {
        const float gray = math::saturate(p[i]) * maxVal + 0.5f;
        d[i] = T(gray);
    }
#else
    for (size_t y = 0; y < h; ++y) {
        float const* p = image.getPixelRef(0, y);
        for (size_t x = 0; x < w; ++x, ++p, ++d) {
            const float gray =  math::saturate(*p) * std::numeric_limits<T>::max() + 0.5f;
            d[0] = T(gray);
        }
    }
#endif
    return dst;
}

// Constructs a 3-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// The "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinear(size_t w, size_t h, size_t bpr,
            const uint8_t* src, PROCESS proc, TRANSFORM transform) {
    LinearImage result(uint32_t(w), uint32_t(h), 3);
    auto d = result.get< math::float3>();
#if defined(__ARM_NEON)
    constexpr float invMax = 1.0f / float(std::numeric_limits<T>::max());
    bool isSRGB = false;
    if constexpr (std::is_convertible_v<TRANSFORM, math::float3 (*)(const math::float3&)>) {
        math::float3 (*fn)(const math::float3&) = transform;
        isSRGB = (fn == &sRGBToLinear<math::float3>);
    }
    if (isSRGB) {
        size_t const count = w * 3;
        for (size_t y = 0; y < h; ++y, d += w) {
            T const* p = reinterpret_cast<T const*>(src + y * bpr);
            float* out = reinterpret_cast<float*>(d);
            size_t i = 0;
            for (; i + 4 <= count; i += 4) {
                float32x4_t const v = vmulq_n_f32(details::v_load4_proc_f32(p + i, proc), invMax);
                vst1q_f32(out + i, details::v_sRGBToLinear(v));
            }
            if (i < count) {
                float32x4_t v = vdupq_n_f32(0.0f);
                v = vsetq_lane_f32(float(proc(p[i])), v, 0);
                if (i + 1 < count) v = vsetq_lane_f32(float(proc(p[i + 1])), v, 1);
                if (i + 2 < count) v = vsetq_lane_f32(float(proc(p[i + 2])), v, 2);
                v = details::v_sRGBToLinear(vmulq_n_f32(v, invMax));
                out[i] = vgetq_lane_f32(v, 0);
                if (i + 1 < count) out[i + 1] = vgetq_lane_f32(v, 1);
                if (i + 2 < count) out[i + 2] = vgetq_lane_f32(v, 2);
            }
        }
        return result;
    }
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        size_t x = 0;
        for (; x + 4 <= w; x += 4, p += 12, d += 4) {
            float32x4_t const v0 = vmulq_n_f32(details::v_load4_proc_f32(p, proc), invMax);
            float32x4_t const v1 = vmulq_n_f32(details::v_load4_proc_f32(p + 4, proc), invMax);
            float32x4_t const v2 = vmulq_n_f32(details::v_load4_proc_f32(p + 8, proc), invMax);
            alignas(16) float tmp[12];
            vst1q_f32(tmp, v0);
            vst1q_f32(tmp + 4, v1);
            vst1q_f32(tmp + 8, v2);
            d[0] = transform(math::float3{ tmp[0], tmp[1], tmp[2] });
            d[1] = transform(math::float3{ tmp[3], tmp[4], tmp[5] });
            d[2] = transform(math::float3{ tmp[6], tmp[7], tmp[8] });
            d[3] = transform(math::float3{ tmp[9], tmp[10], tmp[11] });
        }
        for (; x < w; ++x, p += 3) {
            math::float3 sRGB(proc(p[0]), proc(p[1]), proc(p[2]));
            sRGB *= invMax;
            *d++ = transform(sRGB);
        }
    }
#else
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
             math::float3 sRGB(proc(p[0]), proc(p[1]), proc(p[2]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = transform(sRGB);
        }
    }
#endif
    return result;
}

// Constructs a 3-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// The "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinear(size_t w, size_t h, size_t bpr,
        const std::unique_ptr<uint8_t[]>& src, PROCESS proc, TRANSFORM transform) {
    return toLinear<T>(w, h, bpr, src.get(), proc, transform);
}

// Constructs a 4-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// the "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinearWithAlpha(size_t w, size_t h, size_t bpr,
        const uint8_t* src, PROCESS proc, TRANSFORM transform) {
    LinearImage result(uint32_t(w), uint32_t(h), 4);
    auto d = result.get< math::float4>();
#if defined(__ARM_NEON)
    constexpr float invMax = 1.0f / float(std::numeric_limits<T>::max());
    bool isSRGB = false;
    if constexpr (std::is_convertible_v<TRANSFORM, math::float4 (*)(const math::float4&)>) {
        math::float4 (*fn)(const math::float4&) = transform;
        isSRGB = (fn == &sRGBToLinear<math::float4>);
    }
    if (isSRGB) {
        for (size_t y = 0; y < h; ++y) {
            T const* p = reinterpret_cast<T const*>(src + y * bpr);
            size_t x = 0;
            for (; x + 4 <= w; x += 4, p += 16, d += 4) {
                float32x4_t const v0 = vmulq_n_f32(details::v_load4_proc_f32(p, proc), invMax);
                float32x4_t const v1 = vmulq_n_f32(details::v_load4_proc_f32(p + 4, proc), invMax);
                float32x4_t const v2 = vmulq_n_f32(details::v_load4_proc_f32(p + 8, proc), invMax);
                float32x4_t const v3 = vmulq_n_f32(details::v_load4_proc_f32(p + 12, proc), invMax);
                float32x4x2_t const t01 = vtrnq_f32(v0, v1);
                float32x4x2_t const t23 = vtrnq_f32(v2, v3);
                float32x4_t r = vcombine_f32(vget_low_f32(t01.val[0]), vget_low_f32(t23.val[0]));
                float32x4_t g = vcombine_f32(vget_low_f32(t01.val[1]), vget_low_f32(t23.val[1]));
                float32x4_t b = vcombine_f32(vget_high_f32(t01.val[0]), vget_high_f32(t23.val[0]));
                float32x4_t const a = vcombine_f32(vget_high_f32(t01.val[1]), vget_high_f32(t23.val[1]));
                r = details::v_sRGBToLinear(r);
                g = details::v_sRGBToLinear(g);
                b = details::v_sRGBToLinear(b);
                float32x4x4_t const rgba = {{ r, g, b, a }};
                vst4q_f32(reinterpret_cast<float*>(d), rgba);
            }
            for (; x < w; ++x, p += 4, ++d) {
                float32x4_t const sRGB = vmulq_n_f32(details::v_load4_proc_f32(p, proc), invMax);
                float32x4_t lin = details::v_sRGBToLinear(sRGB);
                lin = vsetq_lane_f32(vgetq_lane_f32(sRGB, 3), lin, 3);
                vst1q_f32(reinterpret_cast<float*>(d), lin);
            }
        }
        return result;
    }
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        size_t x = 0;
        for (; x + 4 <= w; x += 4, p += 16, d += 4) {
            float32x4_t const v0 = vmulq_n_f32(details::v_load4_proc_f32(p, proc), invMax);
            float32x4_t const v1 = vmulq_n_f32(details::v_load4_proc_f32(p + 4, proc), invMax);
            float32x4_t const v2 = vmulq_n_f32(details::v_load4_proc_f32(p + 8, proc), invMax);
            float32x4_t const v3 = vmulq_n_f32(details::v_load4_proc_f32(p + 12, proc), invMax);
            math::float4 s0, s1, s2, s3;
            vst1q_f32(&s0.v[0], v0);
            vst1q_f32(&s1.v[0], v1);
            vst1q_f32(&s2.v[0], v2);
            vst1q_f32(&s3.v[0], v3);
            d[0] = transform(s0);
            d[1] = transform(s1);
            d[2] = transform(s2);
            d[3] = transform(s3);
        }
        for (; x < w; ++x, p += 4, ++d) {
            float32x4_t const v = vmulq_n_f32(details::v_load4_proc_f32(p, proc), invMax);
            math::float4 sRGB;
            vst1q_f32(&sRGB.v[0], v);
            *d = transform(sRGB);
        }
    }
#else
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
             math::float4 sRGB(proc(p[0]), proc(p[1]), proc(p[2]), proc(p[3]));
            sRGB /= std::numeric_limits<T>::max();
            *d++ = transform(sRGB);
        }
    }
#endif
    return result;
}

// Constructs a 4-channel LinearImage from an untyped data blob.
// The "proc" lambda converts a single color component into a float.
// the "transform" lambda performs an arbitrary float-to-float transformation.
template<typename T, typename PROCESS, typename TRANSFORM>
static LinearImage toLinearWithAlpha(size_t w, size_t h, size_t bpr,
        const std::unique_ptr<uint8_t[]>& src, PROCESS proc, TRANSFORM transform) {
    return toLinearWithAlpha<T>(w, h, bpr, src.get(), proc, transform);
}

// Constructs a 3-channel LinearImage from RGBM data.
inline LinearImage toLinearFromRGBM( math::float4 const* src, uint32_t w, uint32_t h) {
    LinearImage result(w, h, 3);
    auto dst = result.get< math::float3>();
#if defined(__ARM_NEON)
    size_t const total = size_t(w) * h;
    size_t i = 0;
    for (; i + 4 <= total; i += 4, src += 4, dst += 4) {
        float32x4x4_t const rgbm = vld4q_f32(reinterpret_cast<float const*>(src));
        float32x4_t const s = vmulq_n_f32(rgbm.val[3], 16.0f);
        float32x4_t r = vmulq_f32(rgbm.val[0], s);
        float32x4_t g = vmulq_f32(rgbm.val[1], s);
        float32x4_t b = vmulq_f32(rgbm.val[2], s);
        r = vmulq_f32(r, r);
        g = vmulq_f32(g, g);
        b = vmulq_f32(b, b);
        float32x4x3_t const lin = {{ r, g, b }};
        vst3q_f32(reinterpret_cast<float*>(dst), lin);
    }
    for (; i < total; ++i, ++src, ++dst) {
        *dst = RGBMtoLinear(*src);
    }
#else
    for (uint32_t row = 0; row < h; ++row) {
        for (uint32_t col = 0; col < w; ++col, ++src, ++dst) {
            *dst = RGBMtoLinear(*src);
        }
    }
#endif
    return result;
}

inline LinearImage fromLinearToRGBM(const LinearImage& image) {
    assert(image.getChannels() == 3);
    const uint32_t w = image.getWidth(), h = image.getHeight();
    LinearImage result(w, h, 4);
    auto src = image.get< math::float3>();
    auto dst = result.get< math::float4>();
#if defined(__ARM_NEON)
    size_t const total = size_t(w) * h;
    size_t i = 0;
    for (; i + 4 <= total; i += 4, src += 4, dst += 4) {
        float32x4x3_t const rgb = vld3q_f32(reinterpret_cast<float const*>(src));
        float32x4_t r = rgb.val[0];
        float32x4_t g = rgb.val[1];
        float32x4_t b = rgb.val[2];
        float32x4_t a;
        details::v_linearToRGBM(r, g, b, a);
        float32x4x4_t const rgba = {{ r, g, b, a }};
        vst4q_f32(reinterpret_cast<float*>(dst), rgba);
    }
    for (; i < total; ++i, ++src, ++dst) {
        *dst = linearToRGBM(*src);
    }
#else
    for (uint32_t row = 0; row < h; ++row) {
        for (uint32_t col = 0; col < w; ++col, ++src, ++dst) {
            *dst = linearToRGBM(*src);
        }
    }
#endif
    return result;
}

template<typename T>
static LinearImage toLinearWithAlpha(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(uint32_t(w), uint32_t(h), 4);
    auto d = reinterpret_cast<math::float4*>(result.getPixelRef(0, 0));
#if defined(__ARM_NEON)
    constexpr float invMax = 1.0f / float(std::numeric_limits<T>::max());
    float32x4_t const one = vdupq_n_f32(1.0f);
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        size_t x = 0;
        for (; x + 4 <= w; x += 4, p += 16, d += 4) {
            float32x4_t r, g, b;
            details::v_load4_rgba_rgb_f32(p, r, g, b);
            r = details::v_sRGBToLinear(vmulq_n_f32(r, invMax));
            g = details::v_sRGBToLinear(vmulq_n_f32(g, invMax));
            b = details::v_sRGBToLinear(vmulq_n_f32(b, invMax));
            float32x4x4_t const rgba = {{ r, g, b, one }};
            vst4q_f32(reinterpret_cast<float*>(d), rgba);
        }
        for (; x < w; ++x, p += 4, ++d) {
            float32x4_t const sRGB = vmulq_n_f32(details::v_load4_f32(p), invMax);
            float32x4_t lin = details::v_sRGBToLinear(sRGB);
            lin = vsetq_lane_f32(1.0f, lin, 3);
            vst1q_f32(reinterpret_cast<float*>(d), lin);
        }
    }
#else
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 4) {
            math::float3 sRGB(p[0], p[1], p[2]);
            sRGB /= std::numeric_limits<T>::max();
            *d++ = math::float4(sRGBToLinear(sRGB), 1.0f);
        }
    }
#endif
    return result;
}

template<typename T>
static LinearImage toLinear(size_t w, size_t h, size_t bpr, const uint8_t* src) {
    LinearImage result(uint32_t(w), uint32_t(h), 3);
    auto d = reinterpret_cast<math::float3*>(result.getPixelRef(0, 0));
#if defined(__ARM_NEON)
    constexpr float invMax = 1.0f / float(std::numeric_limits<T>::max());
    size_t const count = w * 3;
    for (size_t y = 0; y < h; ++y, d += w) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        float* out = reinterpret_cast<float*>(d);
        size_t i = 0;
        for (; i + 4 <= count; i += 4) {
            float32x4_t const sRGB = vmulq_n_f32(details::v_load4_f32(p + i), invMax);
            vst1q_f32(out + i, details::v_sRGBToLinear(sRGB));
        }
        if (i < count) {
            float32x4_t v = vdupq_n_f32(0.0f);
            v = vsetq_lane_f32(float(p[i]), v, 0);
            if (i + 1 < count) v = vsetq_lane_f32(float(p[i + 1]), v, 1);
            if (i + 2 < count) v = vsetq_lane_f32(float(p[i + 2]), v, 2);
            v = details::v_sRGBToLinear(vmulq_n_f32(v, invMax));
            out[i] = vgetq_lane_f32(v, 0);
            if (i + 1 < count) out[i + 1] = vgetq_lane_f32(v, 1);
            if (i + 2 < count) out[i + 2] = vgetq_lane_f32(v, 2);
        }
    }
#else
    for (size_t y = 0; y < h; ++y) {
        T const* p = reinterpret_cast<T const*>(src + y * bpr);
        for (size_t x = 0; x < w; ++x, p += 3) {
            math::float3 sRGB(p[0], p[1], p[2]);
            sRGB /= std::numeric_limits<T>::max();
            *d++ = sRGBToLinear(sRGB);
        }
    }
#endif
    return result;
}

}  // namespace image

#endif // IMAGE_COLORTRANSFORM_H_
