/*
 * TinyEXR - ARM NEON kernels (byte interleave + predictor prefix-sum/delta).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#if defined(EXR_NEON)

#include <arm_neon.h>

/* half <-> float via the AArch64 baseline FCVTL/FCVTN convert instructions
 * (vcvt_f32_f16 / vcvt_f16_f32), 8 elements per iteration with a 1-wide tail.
 * This mirrors the x86 F16C kernels (exr_half_to_float_f16c et al.): like F16C,
 * the hardware convert is bit-identical to the integer scalar path for every
 * finite / zero / inf / subnormal value, and quiets signalling NaNs (sets the
 * MSB of the payload) where the integer scalar preserves them. float->half is
 * round-to-nearest-even and bit-exact with the scalar (incl. NaN payloads). */
void exr_half_to_float_neon(const uint16_t *src, float *dst, size_t count) {
    size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        uint16x8_t h = vld1q_u16(src + i);
        vst1q_f32(dst + i, vcvt_f32_f16(vreinterpret_f16_u16(vget_low_u16(h))));
        vst1q_f32(dst + i + 4,
                  vcvt_f32_f16(vreinterpret_f16_u16(vget_high_u16(h))));
    }
    for (; i < count; ++i) {
        uint16x4_t t = vdup_n_u16(src[i]);
        dst[i] = vgetq_lane_f32(vcvt_f32_f16(vreinterpret_f16_u16(t)), 0);
    }
}

void exr_float_to_half_neon(const float *src, uint16_t *dst, size_t count) {
    size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        float16x4_t lo = vcvt_f16_f32(vld1q_f32(src + i));
        float16x4_t hi = vcvt_f16_f32(vld1q_f32(src + i + 4));
        vst1q_u16(dst + i,
                  vcombine_u16(vreinterpret_u16_f16(lo), vreinterpret_u16_f16(hi)));
    }
    for (; i < count; ++i) {
        float16x4_t h = vcvt_f16_f32(vdupq_n_f32(src[i]));
        dst[i] = vget_lane_u16(vreinterpret_u16_f16(h), 0);
    }
}

void exr_interleave_neon(const uint8_t *src, uint8_t *dst, size_t n) {
    size_t half = (n + 1) / 2, n2 = n / 2, i = 0;
    const uint8_t *t1 = src, *t2 = src + half;
    for (; i + 16 <= n2; i += 16) {
        uint8x16x2_t v;
        v.val[0] = vld1q_u8(t1 + i);
        v.val[1] = vld1q_u8(t2 + i);
        vst2q_u8(dst + 2 * i, v);
    }
    for (; i < n2; ++i) {
        dst[2 * i] = t1[i];
        dst[2 * i + 1] = t2[i];
    }
    if (n & 1) dst[n - 1] = t1[n2];
}

/* Predictor decode: byte prefix-sum mod 256 with a -128 bias per step,
 * bit-identical to exr_predictor_decode_scalar. Within each 16-byte chunk the
 * prefix sum is built by log2(16) lane shifts (vextq from a zero vector);
 * `running` carries the cumulative total across chunks. */
void exr_predictor_decode_neon(uint8_t *p, size_t n) {
    const uint8x16_t bias = vdupq_n_u8(128);
    const uint8x16_t z = vdupq_n_u8(0);
    uint8_t running;
    size_t i;
    if (n == 0) return;
    running = p[0];
    i = 1;
    for (; i + 16 <= n; i += 16) {
        uint8x16_t x = vld1q_u8(p + i);
        x = vsubq_u8(x, bias);               /* b = p - 128 */
        x = vaddq_u8(x, vextq_u8(z, x, 15)); /* << 1 byte */
        x = vaddq_u8(x, vextq_u8(z, x, 14)); /* << 2 */
        x = vaddq_u8(x, vextq_u8(z, x, 12)); /* << 4 */
        x = vaddq_u8(x, vextq_u8(z, x, 8));  /* << 8 -> full 16-lane prefix sum */
        x = vaddq_u8(x, vdupq_n_u8(running));
        vst1q_u8(p + i, x);
        running = p[i + 15];
    }
    for (; i < n; ++i) {
        int d = (int)running + (int)p[i] - 128;
        p[i] = (uint8_t)d;
        running = p[i];
    }
}

/* Predictor encode: per-byte delta (cur - prev + 128) mod 256, in place,
 * bit-identical to exr_predictor_encode_scalar. `prev` is the original previous
 * byte, saved before the store overwrites it. */
void exr_predictor_encode_neon(uint8_t *p, size_t n) {
    const uint8x16_t bias = vdupq_n_u8(128);
    const uint8x16_t z = vdupq_n_u8(0);
    uint8_t prev;
    size_t i;
    if (n == 0) return;
    prev = p[0];
    i = 1;
    for (; i + 16 <= n; i += 16) {
        uint8x16_t cur = vld1q_u8(p + i);     /* original p[i..i+15] */
        uint8_t nextprev = p[i + 15];         /* save before overwrite */
        uint8x16_t sh = vextq_u8(z, cur, 15); /* lane0=0, lanes1..15=p[i..i+14] */
        uint8x16_t out;
        sh = vsetq_lane_u8(prev, sh, 0);      /* lane0 = prev */
        out = vaddq_u8(vsubq_u8(cur, sh), bias);
        vst1q_u8(p + i, out);
        prev = nextprev;
    }
    for (; i < n; ++i) {
        int cur = p[i];
        p[i] = (uint8_t)(cur - prev + (128 + 256));
        prev = cur;
    }
}

#endif /* EXR_NEON */
