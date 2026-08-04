/*
 * tir - AArch64 NEON kernels (compile-time selected).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tir_internal.h"

#if defined(TIR_NEON)

#include <arm_neon.h>

static void v_mul_neon(float *dst, const float *src, float w, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        vst1q_f32(dst + i, vmulq_n_f32(vld1q_f32(src + i), w));
        vst1q_f32(dst + i + 4, vmulq_n_f32(vld1q_f32(src + i + 4), w));
    }
    for (; i < n; ++i) dst[i] = src[i] * w;
}

static void v_fma_neon(float *dst, const float *src, float w, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        vst1q_f32(dst + i,
                  vfmaq_n_f32(vld1q_f32(dst + i), vld1q_f32(src + i), w));
        vst1q_f32(dst + i + 4, vfmaq_n_f32(vld1q_f32(dst + i + 4),
                                           vld1q_f32(src + i + 4), w));
    }
    for (; i < n; ++i) dst[i] += src[i] * w;
}

static void v_fma4_neon(float *dst, const float *const src[4], const float *w,
                        size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        float32x4_t acc = vld1q_f32(dst + i);
        acc = vfmaq_n_f32(acc, vld1q_f32(src[0] + i), w[0]);
        acc = vfmaq_n_f32(acc, vld1q_f32(src[1] + i), w[1]);
        acc = vfmaq_n_f32(acc, vld1q_f32(src[2] + i), w[2]);
        acc = vfmaq_n_f32(acc, vld1q_f32(src[3] + i), w[3]);
        vst1q_f32(dst + i, acc);
    }
    for (; i < n; ++i) {
        float v = dst[i];
        v += src[0][i] * w[0];
        v += src[1][i] * w[1];
        v += src[2][i] * w[2];
        v += src[3][i] * w[3];
        dst[i] = v;
    }
}

/* 1-channel horizontal: batch 4 outputs, four independent FMA chains, one
 * 4x4 transpose to reduce (instead of four vaddvq horizontal sums). */
static void h_dot1_neon(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x = x0;
    (void)count;
    for (; x + 4 <= x1; x += 4) {
        const float *w0 = w + (size_t)(x + 0) * (size_t)padded;
        const float *w1 = w + (size_t)(x + 1) * (size_t)padded;
        const float *w2 = w + (size_t)(x + 2) * (size_t)padded;
        const float *w3 = w + (size_t)(x + 3) * (size_t)padded;
        const float *s0 = src + (size_t)start[x + 0];
        const float *s1 = src + (size_t)start[x + 1];
        const float *s2 = src + (size_t)start[x + 2];
        const float *s3 = src + (size_t)start[x + 3];
        float32x4_t a0 = vdupq_n_f32(0.0f), a1 = vdupq_n_f32(0.0f);
        float32x4_t a2 = vdupq_n_f32(0.0f), a3 = vdupq_n_f32(0.0f);
        float32x4x2_t p01, p23;
        int t;
        for (t = 0; t < padded; t += 4) {
            a0 = vfmaq_f32(a0, vld1q_f32(w0 + t), vld1q_f32(s0 + t));
            a1 = vfmaq_f32(a1, vld1q_f32(w1 + t), vld1q_f32(s1 + t));
            a2 = vfmaq_f32(a2, vld1q_f32(w2 + t), vld1q_f32(s2 + t));
            a3 = vfmaq_f32(a3, vld1q_f32(w3 + t), vld1q_f32(s3 + t));
        }
        /* 4x4 transpose then column sum: lane i = sum of a_i's lanes */
        p01 = vtrnq_f32(a0, a1);
        p23 = vtrnq_f32(a2, a3);
        a0 = vcombine_f32(vget_low_f32(p01.val[0]), vget_low_f32(p23.val[0]));
        a1 = vcombine_f32(vget_low_f32(p01.val[1]), vget_low_f32(p23.val[1]));
        a2 = vcombine_f32(vget_high_f32(p01.val[0]), vget_high_f32(p23.val[0]));
        a3 = vcombine_f32(vget_high_f32(p01.val[1]), vget_high_f32(p23.val[1]));
        vst1q_f32(dst + x, vaddq_f32(vaddq_f32(a0, a1), vaddq_f32(a2, a3)));
    }
    for (; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x];
        float32x4_t acc = vdupq_n_f32(0.0f);
        int t;
        for (t = 0; t < padded; t += 4)
            acc = vfmaq_f32(acc, vld1q_f32(wr + t), vld1q_f32(sp + t));
        dst[x] = vaddvq_f32(acc);
    }
}

static void h_dot2_neon(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 2;
        int nblk = (count[x] + 1) & ~1;
        int t;
        float32x2_t a0 = vdup_n_f32(0.0f), a1 = vdup_n_f32(0.0f);
        for (t = 0; t < nblk; t += 2) {
            a0 = vfma_n_f32(a0, vld1_f32(sp + (size_t)t * 2), wr[t]);
            a1 = vfma_n_f32(a1, vld1_f32(sp + (size_t)(t + 1) * 2),
                            wr[t + 1]);
        }
        vst1_f32(dst + (size_t)x * 2, vadd_f32(a0, a1));
    }
}

static void h_dot3_neon(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 3;
        int nblk = (count[x] + 1) & ~1;
        int t;
        float32x4_t a0 = vdupq_n_f32(0.0f), a1 = vdupq_n_f32(0.0f);
        for (t = 0; t < nblk; t += 2) {
            a0 = vfmaq_n_f32(a0, vld1q_f32(sp + (size_t)t * 3), wr[t]);
            a1 = vfmaq_n_f32(a1, vld1q_f32(sp + (size_t)(t + 1) * 3),
                             wr[t + 1]);
        }
        /* writes 4 floats: +4 slack on staging/ring rows makes this safe */
        vst1q_f32(dst + (size_t)x * 3, vaddq_f32(a0, a1));
    }
}

static void h_dot4_neon(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 4;
        int nblk = (count[x] + 3) & ~3;
        int t;
        float32x4_t a0 = vdupq_n_f32(0.0f), a1 = vdupq_n_f32(0.0f);
        float32x4_t a2 = vdupq_n_f32(0.0f), a3 = vdupq_n_f32(0.0f);
        for (t = 0; t < nblk; t += 4) {
            a0 = vfmaq_n_f32(a0, vld1q_f32(sp + (size_t)t * 4), wr[t]);
            a1 = vfmaq_n_f32(a1, vld1q_f32(sp + (size_t)(t + 1) * 4),
                             wr[t + 1]);
            a2 = vfmaq_n_f32(a2, vld1q_f32(sp + (size_t)(t + 2) * 4),
                             wr[t + 2]);
            a3 = vfmaq_n_f32(a3, vld1q_f32(sp + (size_t)(t + 3) * 4),
                             wr[t + 3]);
        }
        vst1q_f32(dst + (size_t)x * 4,
                  vaddq_f32(vaddq_f32(a0, a1), vaddq_f32(a2, a3)));
    }
}

static void minmax_combine_neon(float *mn, float *mx, const float *rmn,
                                const float *rmx, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        vst1q_f32(mn + i, vminq_f32(vld1q_f32(mn + i), vld1q_f32(rmn + i)));
        vst1q_f32(mx + i, vmaxq_f32(vld1q_f32(mx + i), vld1q_f32(rmx + i)));
    }
    for (; i < n; ++i) {
        if (rmn[i] < mn[i]) mn[i] = rmn[i];
        if (rmx[i] > mx[i]) mx[i] = rmx[i];
    }
}

static void antiring_apply_neon(float *dst, const float *mn, const float *mx,
                                float s, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        float32x4_t v = vld1q_f32(dst + i);
        float32x4_t c =
            vminq_f32(vmaxq_f32(v, vld1q_f32(mn + i)), vld1q_f32(mx + i));
        vst1q_f32(dst + i, vfmaq_n_f32(v, vsubq_f32(c, v), s));
    }
    for (; i < n; ++i) {
        float v = dst[i];
        float c = v < mn[i] ? mn[i] : (v > mx[i] ? mx[i] : v);
        dst[i] = v + s * (c - v);
    }
}

static void clamp_range_neon(float *dst, float lo, float hi, size_t n) {
    float32x4_t lov = vdupq_n_f32(lo), hiv = vdupq_n_f32(hi);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        float32x4_t v = vld1q_f32(dst + i);
        vst1q_f32(dst + i, vminq_f32(vmaxq_f32(v, lov), hiv));
    }
    for (; i < n; ++i) {
        float v = dst[i];
        if (v < lo) v = lo;
        if (v > hi) v = hi;
        dst[i] = v;
    }
}

/* ---- converters -------------------------------------------------------------- */

static void u8_to_f32_neon(float *dst, const uint8_t *src, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        uint16x8_t w = vmovl_u8(vld1_u8(src + i));
        float32x4_t lo = vcvtq_f32_u32(vmovl_u16(vget_low_u16(w)));
        float32x4_t hi = vcvtq_f32_u32(vmovl_u16(vget_high_u16(w)));
        vst1q_f32(dst + i, vmulq_n_f32(lo, 1.0f / 255.0f));
        vst1q_f32(dst + i + 4, vmulq_n_f32(hi, 1.0f / 255.0f));
    }
    if (i < n) tir__u8_to_f32_sc(dst + i, src + i, n - i);
}

static void u16_to_f32_neon(float *dst, const uint16_t *src, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        uint16x8_t w = vld1q_u16(src + i);
        float32x4_t lo = vcvtq_f32_u32(vmovl_u16(vget_low_u16(w)));
        float32x4_t hi = vcvtq_f32_u32(vmovl_u16(vget_high_u16(w)));
        vst1q_f32(dst + i, vmulq_n_f32(lo, 1.0f / 65535.0f));
        vst1q_f32(dst + i + 4, vmulq_n_f32(hi, 1.0f / 65535.0f));
    }
    if (i < n) tir__u16_to_f32_sc(dst + i, src + i, n - i);
}

static void f32_to_u8_neon(uint8_t *dst, const float *src, size_t n) {
    const float32x4_t zero = vdupq_n_f32(0.0f), one = vdupq_n_f32(1.0f);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        float32x4_t a = vld1q_f32(src + i), b = vld1q_f32(src + i + 4);
        int32x4_t ia, ib;
        uint16x8_t w;
        a = vminq_f32(vmaxq_f32(a, zero), one); /* NaN -> 0 (vmaxq scrubs) */
        b = vminq_f32(vmaxq_f32(b, zero), one);
        ia = vcvtnq_s32_f32(vmulq_n_f32(a, 255.0f)); /* RNE */
        ib = vcvtnq_s32_f32(vmulq_n_f32(b, 255.0f));
        w = vcombine_u16(vqmovun_s32(ia), vqmovun_s32(ib));
        vst1_u8(dst + i, vqmovn_u16(w));
    }
    if (i < n) tir__f32_to_u8_sc(dst + i, src + i, n - i);
}

static void f32_to_u16_neon(uint16_t *dst, const float *src, size_t n) {
    const float32x4_t zero = vdupq_n_f32(0.0f), one = vdupq_n_f32(1.0f);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        float32x4_t a = vld1q_f32(src + i), b = vld1q_f32(src + i + 4);
        int32x4_t ia, ib;
        a = vminq_f32(vmaxq_f32(a, zero), one);
        b = vminq_f32(vmaxq_f32(b, zero), one);
        ia = vcvtnq_s32_f32(vmulq_n_f32(a, 65535.0f));
        ib = vcvtnq_s32_f32(vmulq_n_f32(b, 65535.0f));
        vst1q_u16(dst + i,
                  vcombine_u16(vqmovun_s32(ia), vqmovun_s32(ib)));
    }
    if (i < n) tir__f32_to_u16_sc(dst + i, src + i, n - i);
}

#if defined(__ARM_FP16_FORMAT_IEEE)
static void f16_to_f32_neon(float *dst, const uint16_t *src, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        uint16x4_t h16 = vld1_u16(src + i);
        float32x4_t f = vcvt_f32_f16(vreinterpret_f16_u16(h16));
        /* FCVT force-quiets signaling NaNs (sets the top mantissa bit); the
         * scalar path preserves the mantissa (sign | 0x7F800000 | man<<13), so
         * recompute the NaN lanes and blend them in to stay bit-identical.
         * inf/subnormal/finite lanes are already exact -- only NaN differs. */
        uint32x4_t h = vmovl_u16(h16);
        uint32x4_t man = vandq_u32(h, vdupq_n_u32(0x03FF));
        uint32x4_t is_exp31 = vceqq_u32(
            vandq_u32(h, vdupq_n_u32(0x7C00)), vdupq_n_u32(0x7C00));
        uint32x4_t is_nan = vandq_u32(is_exp31, vtstq_u32(man, man));
        uint32x4_t sign = vshlq_n_u32(vandq_u32(h, vdupq_n_u32(0x8000)), 16);
        uint32x4_t nan_bits = vorrq_u32(
            vorrq_u32(sign, vdupq_n_u32(0x7F800000)), vshlq_n_u32(man, 13));
        vst1q_f32(dst + i,
                  vreinterpretq_f32_u32(vbslq_u32(
                      is_nan, nan_bits, vreinterpretq_u32_f32(f))));
    }
    if (i < n) tir__f16_to_f32_sc(dst + i, src + i, n - i);
}

static void f32_to_f16_neon(uint16_t *dst, const float *src, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4)
        vst1_u16(dst + i,
                 vreinterpret_u16_f16(vcvt_f16_f32(vld1q_f32(src + i))));
    if (i < n) tir__f32_to_f16_sc(dst + i, src + i, n - i);
}
#endif

/* Anti-ringing per-tap min/max, RGBA (ch==4) vectorized (exact); other channel
 * counts stay scalar to avoid over-reading the last tap. Scalar on every ISA
 * before this. */
static void h_minmax_neon(float *mn, float *mx, const float *src,
                          const int32_t *start, const int32_t *count, int x0,
                          int x1, int ch) {
    int x;
    if (ch != 4) {
        tir__h_minmax_sc(mn, mx, src, start, count, x0, x1, ch);
        return;
    }
    for (x = x0; x < x1; ++x) {
        const float *sp = src + (size_t)start[x] * 4;
        int n = count[x], t;
        float32x4_t lo = vld1q_f32(sp), hi = lo;
        for (t = 1; t < n; ++t) {
            float32x4_t v = vld1q_f32(sp + (size_t)t * 4);
            lo = vminq_f32(lo, v);
            hi = vmaxq_f32(hi, v);
        }
        vst1q_f32(mn + (size_t)x * 4, lo);
        vst1q_f32(mx + (size_t)x * 4, hi);
    }
}

static void normalize3_neon(float *xyz, float *len_out_or_null, size_t npix) {
    size_t i;
    for (i = 0; i < npix; ++i) {
        size_t off = (size_t)i * 3;
        float32x4_t v = vld1q_f32(xyz + off);
        float32x4_t sq = vmulq_f32(v, v);
        float l2 = vgetq_lane_f32(sq, 0) + vgetq_lane_f32(sq, 1) + vgetq_lane_f32(sq, 2);
        if (len_out_or_null) len_out_or_null[i] = sqrtf(l2);
        if (l2 > 1e-8f) {
            float inv = 1.0f / sqrtf(l2);
            xyz[off] = vgetq_lane_f32(v, 0) * inv;
            xyz[off + 1] = vgetq_lane_f32(v, 1) * inv;
            xyz[off + 2] = vgetq_lane_f32(v, 2) * inv;
        } else {
            xyz[off] = 0.0f;
            xyz[off + 1] = 0.0f;
            xyz[off + 2] = 1.0f;
        }
    }
}

/* RGBA premultiply / un-premultiply (NEON lacked these -> was scalar on ARM).
 * Alpha (lane 3) is preserved; a==0 keeps the filtered RGB (matches scalar). */
static void premult4_neon(float *rgba, size_t npix) {
    size_t i;
    for (i = 0; i < npix; ++i) {
        float32x4_t p = vld1q_f32(rgba + i * 4);
        float a = vgetq_lane_f32(p, 3);
        float32x4_t m = vmulq_n_f32(p, a);
        vst1q_f32(rgba + i * 4, vsetq_lane_f32(a, m, 3));
    }
}

static void unpremult4_neon(float *rgba, size_t npix) {
    size_t i;
    for (i = 0; i < npix; ++i) {
        float32x4_t p = vld1q_f32(rgba + i * 4);
        float a = vgetq_lane_f32(p, 3);
        if (a != 0.0f) {
            float32x4_t m = vmulq_n_f32(p, 1.0f / a);
            vst1q_f32(rgba + i * 4, vsetq_lane_f32(a, m, 3));
        } /* a==0: keep filtered RGB untouched */
    }
}

void tir__kernels_set_neon(tir_kernels *k) {
    k->v_mul = v_mul_neon;
    k->v_fma = v_fma_neon;
    k->v_fma4 = v_fma4_neon;
    k->h_dot[0] = h_dot1_neon;
    k->h_dot[1] = h_dot2_neon;
    k->h_dot[2] = h_dot3_neon;
    k->h_dot[3] = h_dot4_neon;
    k->h_minmax = h_minmax_neon;
    k->minmax_combine = minmax_combine_neon;
    k->antiring_apply = antiring_apply_neon;
    k->clamp_range = clamp_range_neon;
    k->normalize3 = normalize3_neon;
    k->premult4 = premult4_neon;
    k->unpremult4 = unpremult4_neon;
    k->u8_to_f32 = u8_to_f32_neon;
    k->u16_to_f32 = u16_to_f32_neon;
    k->f32_to_u8 = f32_to_u8_neon;
    k->f32_to_u16 = f32_to_u16_neon;
#if defined(__ARM_FP16_FORMAT_IEEE)
    k->f16_to_f32 = f16_to_f32_neon;
    k->f32_to_f16 = f32_to_f16_neon;
#endif
}

#endif /* TIR_NEON */
