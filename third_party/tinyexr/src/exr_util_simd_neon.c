/*
 * TinyEXR - ARM NEON kernels for the util module (integer<->float packs,
 * resize accumulation, 3x3 matrix apply). Compile-time selected on ARM; each
 * kernel reproduces its scalar reference (round-to-nearest-even via vcvtnq,
 * NaN scrubbed to 0, mul+add ordering for axpy).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#if defined(EXR_NEON)

#include <arm_neon.h>

void exr_u8_to_f32_neon(float *dst, const uint8_t *src, size_t n, float scale) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        uint16x8_t w16 = vmovl_u8(vld1_u8(src + i));
        uint32x4_t lo = vmovl_u16(vget_low_u16(w16));
        uint32x4_t hi = vmovl_u16(vget_high_u16(w16));
        vst1q_f32(dst + i, vmulq_n_f32(vcvtq_f32_u32(lo), scale));
        vst1q_f32(dst + i + 4, vmulq_n_f32(vcvtq_f32_u32(hi), scale));
    }
    for (; i < n; ++i) dst[i] = (float)src[i] * scale;
}

void exr_u16_to_f32_neon(float *dst, const uint16_t *src, size_t n,
                         float scale) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        uint32x4_t w = vmovl_u16(vld1_u16(src + i));
        vst1q_f32(dst + i, vmulq_n_f32(vcvtq_f32_u32(w), scale));
    }
    for (; i < n; ++i) dst[i] = (float)src[i] * scale;
}

static float32x4_t neon_prep(float32x4_t v, float32x4_t maxv) {
    uint32x4_t isnum = vceqq_f32(v, v); /* 0 where NaN */
    v = vbslq_f32(isnum, v, vdupq_n_f32(0.0f));
    v = vmaxq_f32(v, vdupq_n_f32(0.0f));
    return vminq_f32(v, maxv);
}

void exr_f32_to_u8_neon(uint8_t *dst, const float *src, size_t n, float scale) {
    float32x4_t maxv = vdupq_n_f32(255.0f);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        float32x4_t v = vmulq_n_f32(vld1q_f32(src + i), scale);
        uint32x4_t u = vcvtnq_u32_f32(neon_prep(v, maxv));
        uint16x4_t u16 = vqmovn_u32(u);
        uint8x8_t u8 = vqmovn_u16(vcombine_u16(u16, u16));
        dst[i + 0] = vget_lane_u8(u8, 0);
        dst[i + 1] = vget_lane_u8(u8, 1);
        dst[i + 2] = vget_lane_u8(u8, 2);
        dst[i + 3] = vget_lane_u8(u8, 3);
    }
    for (; i < n; ++i) {
        float v = src[i] * scale;
        if (!(v > 0.0f)) v = 0.0f;
        if (v > 255.0f) v = 255.0f;
        v = (v + 12582912.0f) - 12582912.0f;
        dst[i] = (uint8_t)v;
    }
}

void exr_f32_to_u16_neon(uint16_t *dst, const float *src, size_t n, float scale) {
    float32x4_t maxv = vdupq_n_f32(65535.0f);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        float32x4_t v = vmulq_n_f32(vld1q_f32(src + i), scale);
        uint32x4_t u = vcvtnq_u32_f32(neon_prep(v, maxv));
        vst1_u16(dst + i, vqmovn_u32(u));
    }
    for (; i < n; ++i) {
        float v = src[i] * scale;
        if (!(v > 0.0f)) v = 0.0f;
        if (v > 65535.0f) v = 65535.0f;
        v = (v + 12582912.0f) - 12582912.0f;
        dst[i] = (uint16_t)v;
    }
}

void exr_axpy_neon(float *acc, const float *x, float w, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        float32x4_t a = vld1q_f32(acc + i);
        float32x4_t p = vmulq_n_f32(vld1q_f32(x + i), w);
        vst1q_f32(acc + i, vaddq_f32(a, p));
    }
    for (; i < n; ++i) acc[i] += x[i] * w;
}

void exr_mat3_neon(float *dst, const float *src, size_t px, int ch,
                   const float *m) {
    size_t i;
    if (ch < 3) {
        exr_mat3_scalar(dst, src, px, ch, m);
        return;
    }
    for (i = 0; i < px; ++i) {
        const float *s = src + i * (size_t)ch;
        float *d = dst + i * (size_t)ch;
        float r = s[0], g = s[1], b = s[2];
        d[0] = m[0] * r + m[1] * g + m[2] * b;
        d[1] = m[3] * r + m[4] * g + m[5] * b;
        d[2] = m[6] * r + m[7] * g + m[8] * b;
        if (ch == 4) d[3] = s[3];
    }
}

#endif /* EXR_NEON */
