/*
 * TinyEXR - ARM NEON kernels for the JPH (HTJ2K) codec.
 *
 * NEON counterparts of the x86 kernels in exr_jph_simd.c. The scalar functions
 * in exr_jph.c are the source of truth; every kernel here is bit-identical to
 * its `_scalar` counterpart (verified in test/unit/test_exr_v3.c).
 *
 * AArch64 NEON has native signed 64-bit arithmetic shift (vshrq_n_s64 == floor
 * division by 2^s for any sign), native 64-bit compares (vcltq/vcgtq_s64) and
 * interleaving stores (vst2), so these are simpler than the SSE2/AVX2 variants,
 * which had to synthesize all three. int64 lanes are processed 2-wide, int32
 * lanes 4-wide.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#if defined(EXR_NEON)

#include <arm_neon.h>

/* ----------------------------------------------------------------------------
 * NLT type-3 (float non-linearity): involution `if (v < 0) v = -v - bias`.
 * Element-independent masked negate; used by both encode and decode.
 * ------------------------------------------------------------------------- */

void jph_nlt_type3_i64_neon(int64_t *data, size_t count, int64_t bias) {
    size_t i = 0;
    int64x2_t vbias = vdupq_n_s64(bias);
    int64x2_t zero = vdupq_n_s64(0);
    for (; i + 2 <= count; i += 2) {
        int64x2_t v = vld1q_s64(data + i);
        int64x2_t neg = vsubq_s64(vsubq_s64(zero, v), vbias); /* (0-v)-bias */
        uint64x2_t mask = vcltq_s64(v, zero);                 /* v < 0 */
        vst1q_s64(data + i, vbslq_s64(mask, neg, v));
    }
    for (; i < count; ++i)
        if (data[i] < 0) data[i] = -data[i] - bias;
}

/* int32 NLT (bit_depth<=31): if v<0, v = ~v - biasm1 (== -v-bias, fits int32). */
void jph_nlt_type3_i32_neon(int32_t *data, size_t count, int32_t biasm1) {
    size_t i = 0;
    int32x4_t vb = vdupq_n_s32(biasm1);
    int32x4_t zero = vdupq_n_s32(0);
    for (; i + 4 <= count; i += 4) {
        int32x4_t v = vld1q_s32(data + i);
        int32x4_t neg = vsubq_s32(vmvnq_s32(v), vb); /* ~v - biasm1 */
        uint32x4_t mask = vcltq_s32(v, zero);
        vst1q_s32(data + i, vbslq_s32(mask, neg, v));
    }
    for (; i < count; ++i)
        if (data[i] < 0) data[i] = ~data[i] - biasm1;
}

/* ----------------------------------------------------------------------------
 * Pixel pack: int32 plane sample -> little-endian uint16 by truncation (low 16
 * bits). vmovn_u32 narrows (truncates) each 32-bit lane to 16 bits; the byte
 * reinterpret + vst1_u8 writes little-endian, matching the scalar store.
 * ------------------------------------------------------------------------- */
void jph_pack_i32_to_half_neon(uint8_t *dst, const int32_t *src, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        uint32x4_t v = vreinterpretq_u32_s32(vld1q_s32(src + i));
        uint16x4_t s = vmovn_u32(v); /* low 16 bits of each lane */
        vst1_u8(dst + 2u * i, vreinterpret_u8_u16(s));
    }
    for (; i < n; ++i) {
        uint16_t b = (uint16_t)src[i];
        dst[2u * i] = (uint8_t)b;
        dst[2u * i + 1u] = (uint8_t)(b >> 8);
    }
}

/* ----------------------------------------------------------------------------
 * Sign-magnitude -> signed int64. word: bit 31 = sign, bits 0..30 = magnitude;
 * mag = (v & 0x7fffffff) >> shift; out = sign ? -mag : mag (sign-extended to
 * int64). mag in [0, 2^31-1] so -mag never reaches INT32_MIN.
 * ------------------------------------------------------------------------- */
void jph_extract_signmag_i32_to_i64_neon(int64_t *out, const uint32_t *buf,
                                         size_t n, unsigned shift) {
    size_t i = 0;
    uint32x4_t magmask = vdupq_n_u32(0x7fffffffu);
    uint32x4_t signbit = vdupq_n_u32(0x80000000u);
    int32x4_t nsh = vdupq_n_s32(-(int32_t)shift); /* vshlq with neg = >> shift */
    for (; i + 4 <= n; i += 4) {
        uint32x4_t v = vld1q_u32(buf + i);
        int32x4_t mag = vreinterpretq_s32_u32(vshlq_u32(vandq_u32(v, magmask), nsh));
        int32x4_t neg = vnegq_s32(mag);
        uint32x4_t smask = vcgeq_u32(v, signbit); /* sign bit set */
        int32x4_t r = vbslq_s32(smask, neg, mag);
        vst1q_s64(out + i, vmovl_s32(vget_low_s32(r)));      /* sign-extend lo */
        vst1q_s64(out + i + 2, vmovl_s32(vget_high_s32(r))); /* sign-extend hi */
    }
    for (; i < n; ++i) {
        uint32_t v = buf[i];
        int32_t mag = (int32_t)((v & 0x7fffffffu) >> shift);
        out[i] = (v & 0x80000000u) ? -mag : mag;
    }
}

/* ----------------------------------------------------------------------------
 * Reversible 5/3 lifting. floor(v / 2^s) is just an arithmetic right shift on
 * AArch64 (vshrq_n_s64 / the scalar helper below round toward -inf for any
 * sign), so the NEON kernels mirror the AVX2 ones with native int64 lanes (2
 * wide). `ovf_any` ORs the two 64-bit lanes of the saturating-compare mask.
 * ------------------------------------------------------------------------- */

/* floor(v / 2^s); identical to exr_jph.c's jph_floor_div_pow2 / the AVX2 ref. */
static int64_t jph_sra64_ref(int64_t v, unsigned s) {
    int64_t d = (int64_t)1 << s;
    if (v >= 0) return v / d;
    return -(((-v) + d - 1) / d);
}

static int ovf_any(uint64x2_t m) {
    return (vgetq_lane_u64(m, 0) | vgetq_lane_u64(m, 1)) != 0u;
}

/* widen 2x int32 -> 2x int64 (sign-extend). */
static int64x2_t jph_widen_i32x2(const int32_t *p) {
    return vmovl_s32(vld1_s32(p));
}

/* Inverse reversible 5/3 1D lifting (int32 fast path), bit-identical to
 * exr_jph_inverse_53_i32: int64 intermediates, floor-div by 2^s, and
 * EXR_ERROR_CORRUPT iff a reconstructed sample leaves int32. */
exr_result jph_inverse_53_i32_neon(const int32_t *low, size_t low_count,
                                   const int32_t *high, size_t high_count,
                                   int32_t *out, size_t out_count,
                                   int64_t *ev, int64_t *od) {
    size_t i;
    size_t expected_low = (out_count + 1u) / 2u;
    size_t expected_high = out_count / 2u;
    int64x2_t two = vdupq_n_s64(2);
    int64x2_t imin = vdupq_n_s64(INT32_MIN);
    int64x2_t imax = vdupq_n_s64(INT32_MAX);
    uint64x2_t ovf = vdupq_n_u64(0);

    if (low_count != expected_low || high_count != expected_high)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (out_count == 0) return EXR_SUCCESS;
    if (high_count == 0) { out[0] = low[0]; return EXR_SUCCESS; }

    /* predict: ev[i] = low[i] - floor((dl+dr+2)/4) */
    ev[0] = (int64_t)low[0] -
            jph_sra64_ref((int64_t)high[0] + (int64_t)high[0] + 2, 2);
    i = 1;
    for (; i + 2 <= high_count; i += 2) {
        int64x2_t dl = jph_widen_i32x2(high + i - 1);
        int64x2_t dr = jph_widen_i32x2(high + i);
        int64x2_t lo = jph_widen_i32x2(low + i);
        int64x2_t q = vshrq_n_s64(vaddq_s64(vaddq_s64(dl, dr), two), 2);
        vst1q_s64(ev + i, vsubq_s64(lo, q));
    }
    for (; i < low_count; ++i) {
        int64_t dl = high[i - 1];
        int64_t dr = (i < high_count) ? high[i] : high[high_count - 1];
        ev[i] = (int64_t)low[i] - jph_sra64_ref(dl + dr + 2, 2);
    }

    /* update: od[i] = high[i] + floor((ev[i]+ev[i+1])/2) */
    i = 0;
    for (; i + 2 <= high_count && i + 2 < low_count; i += 2) {
        int64x2_t e0 = vld1q_s64(ev + i);
        int64x2_t e1 = vld1q_s64(ev + i + 1);
        int64x2_t hi = jph_widen_i32x2(high + i);
        int64x2_t q = vshrq_n_s64(vaddq_s64(e0, e1), 1);
        vst1q_s64(od + i, vaddq_s64(hi, q));
    }
    for (; i < high_count; ++i) {
        int64_t e0 = ev[i];
        int64_t e1 = (i + 1u < low_count) ? ev[i + 1] : e0;
        od[i] = (int64_t)high[i] + jph_sra64_ref(e0 + e1, 1);
    }

    /* narrow + interleave with int32-range check */
    i = 0;
    for (; i + 2 <= high_count; i += 2) {
        int64x2_t e = vld1q_s64(ev + i);
        int64x2_t o = vld1q_s64(od + i);
        int32x2x2_t st;
        ovf = vorrq_u64(ovf, vcgtq_s64(e, imax));
        ovf = vorrq_u64(ovf, vcgtq_s64(imin, e));
        ovf = vorrq_u64(ovf, vcgtq_s64(o, imax));
        ovf = vorrq_u64(ovf, vcgtq_s64(imin, o));
        st.val[0] = vmovn_s64(e);
        st.val[1] = vmovn_s64(o);
        vst2_s32(out + 2u * i, st);
    }
    if (ovf_any(ovf)) return EXR_ERROR_CORRUPT;
    for (; i < high_count; ++i) {
        if (ev[i] < INT32_MIN || ev[i] > INT32_MAX) return EXR_ERROR_CORRUPT;
        if (od[i] < INT32_MIN || od[i] > INT32_MAX) return EXR_ERROR_CORRUPT;
        out[2u * i] = (int32_t)ev[i];
        out[2u * i + 1u] = (int32_t)od[i];
    }
    if (low_count > high_count) {
        if (ev[high_count] < INT32_MIN || ev[high_count] > INT32_MAX)
            return EXR_ERROR_CORRUPT;
        out[2u * high_count] = (int32_t)ev[high_count];
    }
    return EXR_SUCCESS;
}

/* Vertical (column) inverse 5/3, int32; bit-identical to
 * exr_jph_inverse_53_vert_i32. temp: lh low-rows then hh high-rows (stride rw)
 * -> rh interleaved rows in data (stride width). Columns are the SIMD axis. */
exr_result jph_inverse_53_vert_i32_neon(const int32_t *temp, size_t rw,
                                        size_t lh, size_t hh,
                                        int32_t *data, size_t width) {
    size_t i, c;
    int64x2_t two = vdupq_n_s64(2);
    int64x2_t imin = vdupq_n_s64(INT32_MIN);
    int64x2_t imax = vdupq_n_s64(INT32_MAX);
    uint64x2_t ovf = vdupq_n_u64(0);
    int tail_ovf = 0;

    if (lh == 0u) return EXR_SUCCESS;
    if (hh == 0u) { for (c = 0u; c < rw; ++c) data[c] = temp[c]; return EXR_SUCCESS; }

    /* Phase 1: even rows -> data[2i] */
    for (i = 0u; i < lh; ++i) {
        const int32_t *lo = temp + i * rw;
        const int32_t *hL = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
        const int32_t *hR = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
        int32_t *dst = data + (2u * i) * width;
        for (c = 0u; c + 2u <= rw; c += 2u) {
            int64x2_t dl = jph_widen_i32x2(hL + c);
            int64x2_t dr = jph_widen_i32x2(hR + c);
            int64x2_t lov = jph_widen_i32x2(lo + c);
            int64x2_t e =
                vsubq_s64(lov, vshrq_n_s64(vaddq_s64(vaddq_s64(dl, dr), two), 2));
            ovf = vorrq_u64(ovf, vcgtq_s64(e, imax));
            ovf = vorrq_u64(ovf, vcgtq_s64(imin, e));
            vst1_s32(dst + c, vmovn_s64(e));
        }
        for (; c < rw; ++c) {
            int64_t e = (int64_t)lo[c] -
                        jph_sra64_ref((int64_t)hL[c] + (int64_t)hR[c] + 2, 2);
            if (e < INT32_MIN || e > INT32_MAX) tail_ovf = 1;
            dst[c] = (int32_t)e;
        }
    }
    if (tail_ovf || ovf_any(ovf)) return EXR_ERROR_CORRUPT;

    /* Phase 2: odd rows -> data[2i+1], reading even rows back */
    ovf = vdupq_n_u64(0);
    for (i = 0u; i < hh; ++i) {
        const int32_t *hi = temp + (lh + i) * rw;
        const int32_t *e0p = data + (2u * i) * width;
        const int32_t *e1p = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        int32_t *dst = data + (2u * i + 1u) * width;
        for (c = 0u; c + 2u <= rw; c += 2u) {
            int64x2_t e0 = jph_widen_i32x2(e0p + c);
            int64x2_t e1 = jph_widen_i32x2(e1p + c);
            int64x2_t hv = jph_widen_i32x2(hi + c);
            int64x2_t o = vaddq_s64(hv, vshrq_n_s64(vaddq_s64(e0, e1), 1));
            ovf = vorrq_u64(ovf, vcgtq_s64(o, imax));
            ovf = vorrq_u64(ovf, vcgtq_s64(imin, o));
            vst1_s32(dst + c, vmovn_s64(o));
        }
        for (; c < rw; ++c) {
            int64_t o = (int64_t)hi[c] +
                        jph_sra64_ref((int64_t)e0p[c] + (int64_t)e1p[c], 1);
            if (o < INT32_MIN || o > INT32_MAX) tail_ovf = 1;
            dst[c] = (int32_t)o;
        }
    }
    if (tail_ovf || ovf_any(ovf)) return EXR_ERROR_CORRUPT;
    return EXR_SUCCESS;
}

/* Inverse reversible 5/3 1D lifting (int64, float/32-bit decode path);
 * bit-identical to jph_inverse_53_i64. Pure int64, no range check. */
exr_result jph_inverse_53_i64_neon(const int64_t *low, size_t low_count,
                                   const int64_t *high, size_t high_count,
                                   int64_t *out, size_t out_count,
                                   int64_t *ev, int64_t *od) {
    size_t i;
    size_t expected_low = (out_count + 1u) / 2u;
    size_t expected_high = out_count / 2u;
    int64x2_t two = vdupq_n_s64(2);
    if (low_count != expected_low || high_count != expected_high)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (out_count == 0) return EXR_SUCCESS;
    if (high_count == 0) { out[0] = low[0]; return EXR_SUCCESS; }

    ev[0] = low[0] - jph_sra64_ref(high[0] + high[0] + 2, 2);
    i = 1;
    for (; i + 2 <= high_count; i += 2) {
        int64x2_t dl = vld1q_s64(high + i - 1);
        int64x2_t dr = vld1q_s64(high + i);
        int64x2_t lo = vld1q_s64(low + i);
        int64x2_t q = vshrq_n_s64(vaddq_s64(vaddq_s64(dl, dr), two), 2);
        vst1q_s64(ev + i, vsubq_s64(lo, q));
    }
    for (; i < low_count; ++i) {
        int64_t dl = high[i - 1];
        int64_t dr = (i < high_count) ? high[i] : high[high_count - 1];
        ev[i] = low[i] - jph_sra64_ref(dl + dr + 2, 2);
    }

    i = 0;
    for (; i + 2 <= high_count && i + 2 < low_count; i += 2) {
        int64x2_t e0 = vld1q_s64(ev + i);
        int64x2_t e1 = vld1q_s64(ev + i + 1);
        int64x2_t hi = vld1q_s64(high + i);
        int64x2_t q = vshrq_n_s64(vaddq_s64(e0, e1), 1);
        vst1q_s64(od + i, vaddq_s64(hi, q));
    }
    for (; i < high_count; ++i) {
        int64_t e0 = ev[i];
        int64_t e1 = (i + 1u < low_count) ? ev[i + 1] : e0;
        od[i] = high[i] + jph_sra64_ref(e0 + e1, 1);
    }

    i = 0;
    for (; i + 2 <= high_count; i += 2) {
        int64x2x2_t st;
        st.val[0] = vld1q_s64(ev + i);
        st.val[1] = vld1q_s64(od + i);
        vst2q_s64(out + 2u * i, st);
    }
    for (; i < high_count; ++i) { out[2u * i] = ev[i]; out[2u * i + 1u] = od[i]; }
    if (low_count > high_count) out[2u * high_count] = ev[high_count];
    return EXR_SUCCESS;
}

/* Vertical (column) inverse 5/3, int64; bit-identical to
 * jph_inverse_53_vert_i64. temp: lh low-rows, hh high-rows -> rh rows in data. */
exr_result jph_inverse_53_vert_i64_neon(const int64_t *temp, size_t rw,
                                        size_t lh, size_t hh,
                                        int64_t *data, size_t width) {
    size_t i, c;
    int64x2_t two = vdupq_n_s64(2);
    if (lh == 0u) return EXR_SUCCESS;
    if (hh == 0u) { for (c = 0u; c < rw; ++c) data[c] = temp[c]; return EXR_SUCCESS; }

    for (i = 0u; i < lh; ++i) {
        const int64_t *lo = temp + i * rw;
        const int64_t *hL = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
        const int64_t *hR = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
        int64_t *dst = data + (2u * i) * width;
        for (c = 0u; c + 2u <= rw; c += 2u) {
            int64x2_t l = vld1q_s64(lo + c);
            int64x2_t a = vld1q_s64(hL + c);
            int64x2_t b = vld1q_s64(hR + c);
            int64x2_t q = vshrq_n_s64(vaddq_s64(vaddq_s64(a, b), two), 2);
            vst1q_s64(dst + c, vsubq_s64(l, q));
        }
        for (; c < rw; ++c) dst[c] = lo[c] - jph_sra64_ref(hL[c] + hR[c] + 2, 2);
    }
    for (i = 0u; i < hh; ++i) {
        const int64_t *hi = temp + (lh + i) * rw;
        const int64_t *e0p = data + (2u * i) * width;
        const int64_t *e1p = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        int64_t *dst = data + (2u * i + 1u) * width;
        for (c = 0u; c + 2u <= rw; c += 2u) {
            int64x2_t h = vld1q_s64(hi + c);
            int64x2_t e0 = vld1q_s64(e0p + c);
            int64x2_t e1 = vld1q_s64(e1p + c);
            int64x2_t q = vshrq_n_s64(vaddq_s64(e0, e1), 1);
            vst1q_s64(dst + c, vaddq_s64(h, q));
        }
        for (; c < rw; ++c) dst[c] = hi[c] + jph_sra64_ref(e0p[c] + e1p[c], 1);
    }
    return EXR_SUCCESS;
}

/* Forward reversible 5/3 1D lifting (int64); bit-identical to
 * jph_forward_53_i64. ev/od are caller scratch of >= ceil(n/2) int64 each. */
exr_result jph_forward_53_i64_neon(const int64_t *src, size_t n, int64_t *low,
                                   size_t low_count, int64_t *high,
                                   size_t high_count, int64_t *ev, int64_t *od) {
    size_t nl = (n + 1u) / 2u, nh = n / 2u, i;
    int64x2_t two = vdupq_n_s64(2);

    if (low_count != nl || high_count != nh) return EXR_ERROR_INVALID_ARGUMENT;
    if (n == 0) return EXR_SUCCESS;

    for (i = 0; i < nl; ++i) ev[i] = src[2u * i];
    for (i = 0; i < nh; ++i) od[i] = src[2u * i + 1u];

    /* predict: high[i] = od[i] - floor((ev[i] + ev[i+1]) / 2) */
    i = 0;
    for (; i + 2 <= nh && i + 2 < nl; i += 2) {
        int64x2_t e0 = vld1q_s64(ev + i);
        int64x2_t e1 = vld1q_s64(ev + i + 1);
        int64x2_t o = vld1q_s64(od + i);
        int64x2_t q = vshrq_n_s64(vaddq_s64(e0, e1), 1);
        vst1q_s64(high + i, vsubq_s64(o, q));
    }
    for (; i < nh; ++i) {
        int64_t e0 = ev[i];
        int64_t e1 = (i + 1u < nl) ? ev[i + 1] : e0;
        high[i] = od[i] - jph_sra64_ref(e0 + e1, 1);
    }

    /* update: low[i] = ev[i] + floor((high[i-1] + high[i] + 2) / 4) */
    if (nh == 0) { for (i = 0; i < nl; ++i) low[i] = ev[i]; return EXR_SUCCESS; }
    low[0] = ev[0] + jph_sra64_ref((int64_t)high[0] + (int64_t)high[0] + 2, 2);
    i = 1;
    for (; i + 2 <= nh; i += 2) {
        int64x2_t hl = vld1q_s64(high + i - 1);
        int64x2_t hr = vld1q_s64(high + i);
        int64x2_t ee = vld1q_s64(ev + i);
        int64x2_t q = vshrq_n_s64(vaddq_s64(vaddq_s64(hl, hr), two), 2);
        vst1q_s64(low + i, vaddq_s64(ee, q));
    }
    for (; i < nl; ++i) {
        int64_t dl = high[i - 1];
        int64_t dr = (i < nh) ? high[i] : high[nh - 1];
        low[i] = ev[i] + jph_sra64_ref(dl + dr + 2, 2);
    }
    return EXR_SUCCESS;
}

/* Vertical (column) forward reversible 5/3, int64; bit-identical to
 * jph_forward_53_vert_i64. data's interleaved rows (stride width) -> subband
 * layout in temp (stride rw): lh low-rows then hh high-rows. */
exr_result jph_forward_53_vert_i64_neon(const int64_t *data, size_t width,
                                        size_t rw, size_t lh, size_t hh,
                                        int64_t *temp) {
    size_t i, c;
    int64x2_t two = vdupq_n_s64(2);

    /* Phase 1: high rows -> temp[(lh+i)*rw] */
    for (i = 0u; i < hh; ++i) {
        const int64_t *e0 = data + (2u * i) * width;
        const int64_t *e1 = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        const int64_t *od = data + (2u * i + 1u) * width;
        int64_t *hd = temp + (lh + i) * rw;
        for (c = 0u; c + 2u <= rw; c += 2u) {
            int64x2_t a = vld1q_s64(e0 + c);
            int64x2_t b = vld1q_s64(e1 + c);
            int64x2_t o = vld1q_s64(od + c);
            int64x2_t q = vshrq_n_s64(vaddq_s64(a, b), 1);
            vst1q_s64(hd + c, vsubq_s64(o, q));
        }
        for (; c < rw; ++c) hd[c] = od[c] - jph_sra64_ref(e0[c] + e1[c], 1);
    }

    /* Phase 2: low rows -> temp[i*rw] */
    for (i = 0u; i < lh; ++i) {
        const int64_t *e0 = data + (2u * i) * width;
        int64_t *ld = temp + i * rw;
        if (hh == 0u) { for (c = 0u; c < rw; ++c) ld[c] = e0[c]; continue; }
        {
            const int64_t *hl = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
            const int64_t *hr = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
            for (c = 0u; c + 2u <= rw; c += 2u) {
                int64x2_t a = vld1q_s64(e0 + c);
                int64x2_t l = vld1q_s64(hl + c);
                int64x2_t r = vld1q_s64(hr + c);
                int64x2_t q = vshrq_n_s64(vaddq_s64(vaddq_s64(l, r), two), 2);
                vst1q_s64(ld + c, vaddq_s64(a, q));
            }
            for (; c < rw; ++c) ld[c] = e0[c] + jph_sra64_ref(hl[c] + hr[c] + 2, 2);
        }
    }
    return EXR_SUCCESS;
}

#endif /* EXR_NEON */
