/*
 * TinyEXR - SIMD kernels for the JPH (HTJ2K) codec.
 *
 * These are runtime-dispatched (via exr_cpu_caps()) accelerated variants of the
 * vectorizable JPH transform/pixel stages. The scalar implementations in
 * exr_jph.c are the source of truth; every kernel here must be bit-identical to
 * its `_scalar` counterpart (verified in test/unit/test_exr_v3.c). Everything
 * compiles at the C11 baseline; per-function `__attribute__((target(...)))`
 * upgrades the ISA so the baseline never requires SSE/AVX.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

#if defined(EXR_X86)
#include <immintrin.h>

#if defined(__GNUC__) || defined(__clang__)
#define EXR_TARGET(x) __attribute__((target(x)))
#else
#define EXR_TARGET(x)
#endif

/* ----------------------------------------------------------------------------
 * NLT type-3 (float non-linearity), the OpenEXR HTJ2K float transform. It is an
 * involution applied to int64 coefficient planes:  if (v < 0) v = -v - bias;
 * Fully element-independent -> a masked negate. Used by both decode (inverse)
 * and encode (forward), which are identical.
 * ------------------------------------------------------------------------- */

EXR_TARGET("sse2")
void jph_nlt_type3_i64_sse2(int64_t *data, size_t count, int64_t bias) {
    size_t i = 0;
    __m128i vbias = _mm_set1_epi64x(bias);
    __m128i zero = _mm_setzero_si128();
    for (; i + 2 <= count; i += 2) {
        __m128i v = _mm_loadu_si128((const __m128i *)(data + i));
        /* neg = (0 - v) - bias */
        __m128i neg = _mm_sub_epi64(_mm_sub_epi64(zero, v), vbias);
        /* mask = (v < 0) per 64-bit lane: broadcast the high-dword sign bit.
         * SSE2 has no 64-bit compare, so replicate each lane's high dword and
         * arithmetic-shift it right by 31 to fill the lane with its sign. */
        __m128i hi = _mm_shuffle_epi32(v, _MM_SHUFFLE(3, 3, 1, 1));
        __m128i mask = _mm_srai_epi32(hi, 31);
        /* result = (mask ? neg : v) */
        __m128i r = _mm_or_si128(_mm_and_si128(mask, neg),
                                 _mm_andnot_si128(mask, v));
        _mm_storeu_si128((__m128i *)(data + i), r);
    }
    for (; i < count; ++i)
        if (data[i] < 0) data[i] = -data[i] - bias;
}

EXR_TARGET("avx2")
void jph_nlt_type3_i64_avx2(int64_t *data, size_t count, int64_t bias) {
    size_t i = 0;
    __m256i vbias = _mm256_set1_epi64x(bias);
    __m256i zero = _mm256_setzero_si256();
    for (; i + 4 <= count; i += 4) {
        __m256i v = _mm256_loadu_si256((const __m256i *)(data + i));
        __m256i neg = _mm256_sub_epi64(_mm256_sub_epi64(zero, v), vbias);
        __m256i mask = _mm256_cmpgt_epi64(zero, v); /* v < 0 */
        __m256i r = _mm256_blendv_epi8(v, neg, mask);
        _mm256_storeu_si256((__m256i *)(data + i), r);
    }
    for (; i < count; ++i)
        if (data[i] < 0) data[i] = -data[i] - bias;
}

/* int32 NLT type-3 (bit_depth<=31): if v<0, v = ~v - biasm1 (== -v-bias, which
 * always fits int32 there). Pure int32, masked - no overflow, native compare. */

EXR_TARGET("sse2")
void jph_nlt_type3_i32_sse2(int32_t *data, size_t count, int32_t biasm1) {
    size_t i = 0;
    __m128i vb = _mm_set1_epi32(biasm1);
    __m128i ones = _mm_set1_epi32(-1);
    __m128i zero = _mm_setzero_si128();
    for (; i + 4 <= count; i += 4) {
        __m128i v = _mm_loadu_si128((const __m128i *)(data + i));
        __m128i neg = _mm_sub_epi32(_mm_xor_si128(v, ones), vb); /* ~v - biasm1 */
        __m128i mask = _mm_cmplt_epi32(v, zero);
        __m128i r = _mm_or_si128(_mm_and_si128(mask, neg),
                                 _mm_andnot_si128(mask, v));
        _mm_storeu_si128((__m128i *)(data + i), r);
    }
    for (; i < count; ++i)
        if (data[i] < 0) data[i] = ~data[i] - biasm1;
}

EXR_TARGET("avx2")
void jph_nlt_type3_i32_avx2(int32_t *data, size_t count, int32_t biasm1) {
    size_t i = 0;
    __m256i vb = _mm256_set1_epi32(biasm1);
    __m256i ones = _mm256_set1_epi32(-1);
    __m256i zero = _mm256_setzero_si256();
    for (; i + 8 <= count; i += 8) {
        __m256i v = _mm256_loadu_si256((const __m256i *)(data + i));
        __m256i neg = _mm256_sub_epi32(_mm256_xor_si256(v, ones), vb);
        __m256i mask = _mm256_cmpgt_epi32(zero, v); /* v < 0 */
        __m256i r = _mm256_blendv_epi8(v, neg, mask);
        _mm256_storeu_si256((__m256i *)(data + i), r);
    }
    for (; i < count; ++i)
        if (data[i] < 0) data[i] = ~data[i] - biasm1;
}

/* ----------------------------------------------------------------------------
 * Pixel pack: int32 plane sample -> little-endian uint16, by truncation (low 16
 * bits), the all-HALF decode store. A byte shuffle (vpshufb) gathers the low two
 * bytes of each int32 lane; truncation (not saturation) keeps it bit-identical
 * to the scalar (uint16_t)v store even for out-of-range/corrupt coefficients.
 * ------------------------------------------------------------------------- */

EXR_TARGET("sse4.1")
void jph_pack_i32_to_half_sse41(uint8_t *dst, const int32_t *src, size_t n) {
    size_t i = 0;
    const __m128i sh = _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13,
                                     -1, -1, -1, -1, -1, -1, -1, -1);
    for (; i + 4 <= n; i += 4) {
        __m128i v = _mm_loadu_si128((const __m128i *)(src + i));
        __m128i s = _mm_shuffle_epi8(v, sh);
        _mm_storel_epi64((__m128i *)(dst + 2u * i), s);
    }
    for (; i < n; ++i) {
        uint16_t b = (uint16_t)src[i];
        dst[2u * i] = (uint8_t)b;
        dst[2u * i + 1u] = (uint8_t)(b >> 8);
    }
}

EXR_TARGET("avx2")
void jph_pack_i32_to_half_avx2(uint8_t *dst, const int32_t *src, size_t n) {
    size_t i = 0;
    const __m256i sh = _mm256_setr_epi8(
        0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1,
        0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1);
    for (; i + 8 <= n; i += 8) {
        __m256i v = _mm256_loadu_si256((const __m256i *)(src + i));
        __m256i s = _mm256_shuffle_epi8(v, sh); /* per-128-lane gather */
        _mm_storel_epi64((__m128i *)(dst + 2u * i),
                         _mm256_castsi256_si128(s));
        _mm_storel_epi64((__m128i *)(dst + 2u * i + 8u),
                         _mm256_extracti128_si256(s, 1));
    }
    for (; i < n; ++i) {
        uint16_t b = (uint16_t)src[i];
        dst[2u * i] = (uint8_t)b;
        dst[2u * i + 1u] = (uint8_t)(b >> 8);
    }
}

/* ----------------------------------------------------------------------------
 * Inverse reversible 5/3 1D lifting (int32 fast path). Mirrors the scalar
 * exr_jph_inverse_53_i32 exactly: int64 intermediates, floor division by 2^s
 * (== arithmetic right shift), and EXR_ERROR_CORRUPT iff any reconstructed
 * sample leaves int32. Computes the even/odd sub-sequences into caller scratch
 * (ev/od) then narrows+interleaves into out, so there is no scatter.
 * ------------------------------------------------------------------------- */

/* floor(v / 2^s), identical to exr_jph.c's jph_floor_div_pow2 (scalar edges). */
static int64_t jph_sra64_ref(int64_t v, unsigned s) {
    int64_t d = (int64_t)1 << s;
    if (v >= 0) return v / d;
    return -(((-v) + d - 1) / d);
}

EXR_TARGET("avx2")
static __m256i jph_sra64x4(__m256i x, int s) {
    /* arithmetic >> s on 4x int64 (s in 1..2); == floor div by 2^s */
    __m256i sign = _mm256_srai_epi32(_mm256_shuffle_epi32(x, _MM_SHUFFLE(3, 3, 1, 1)), 31);
    __m256i lo = _mm256_srli_epi64(x, s);
    __m256i hi = _mm256_slli_epi64(sign, 64 - s);
    return _mm256_or_si256(lo, hi);
}

/* widen 4x int32 (low 128) -> 4x int64 */
EXR_TARGET("avx2")
static __m256i jph_widen_i32x4(const int32_t *p) {
    return _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)p));
}

/* narrow 4x int64 (in int32 range) -> 4x int32 (__m128i), by truncation */
EXR_TARGET("avx2")
static __m128i jph_narrow_i64x4(__m256i v) {
    const __m256i sh = _mm256_setr_epi8(
        0, 1, 2, 3, 8, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1, -1,
        0, 1, 2, 3, 8, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1, -1);
    __m256i s = _mm256_shuffle_epi8(v, sh);
    return _mm_unpacklo_epi64(_mm256_castsi256_si128(s),
                              _mm256_extracti128_si256(s, 1));
}

EXR_TARGET("avx2")
exr_result jph_inverse_53_i32_avx2(const int32_t *low, size_t low_count,
                                   const int32_t *high, size_t high_count,
                                   int32_t *out, size_t out_count,
                                   int64_t *ev, int64_t *od) {
    size_t i;
    size_t expected_low = (out_count + 1u) / 2u;
    size_t expected_high = out_count / 2u;
    __m256i two = _mm256_set1_epi64x(2);
    __m256i imin = _mm256_set1_epi64x(INT32_MIN);
    __m256i imax = _mm256_set1_epi64x(INT32_MAX);
    __m256i ovf = _mm256_setzero_si256();

    if (low_count != expected_low || high_count != expected_high)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (out_count == 0) return EXR_SUCCESS;
    if (high_count == 0) { out[0] = low[0]; return EXR_SUCCESS; }

    /* ---- predict: ev[i] = low[i] - floor((dl+dr+2)/2) ---- */
    ev[0] = (int64_t)low[0] -
            jph_sra64_ref((int64_t)high[0] + (int64_t)high[0] + 2, 2);
    i = 1;
    for (; i + 4 <= high_count; i += 4) {
        __m256i dl = jph_widen_i32x4(high + i - 1);
        __m256i dr = jph_widen_i32x4(high + i);
        __m256i lo = jph_widen_i32x4(low + i);
        __m256i q = jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(dl, dr), two), 2);
        _mm256_storeu_si256((__m256i *)(ev + i), _mm256_sub_epi64(lo, q));
    }
    for (; i < low_count; ++i) {
        int64_t dl = high[i - 1];
        int64_t dr = (i < high_count) ? high[i] : high[high_count - 1];
        ev[i] = (int64_t)low[i] - jph_sra64_ref(dl + dr + 2, 2);
    }

    /* ---- update: od[i] = high[i] + floor((ev[i]+ev[i+1])/2) ---- */
    i = 0;
    for (; i + 4 <= high_count && i + 4 < low_count; i += 4) {
        __m256i e0 = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i e1 = _mm256_loadu_si256((const __m256i *)(ev + i + 1));
        __m256i hi = jph_widen_i32x4(high + i);
        __m256i q = jph_sra64x4(_mm256_add_epi64(e0, e1), 1);
        _mm256_storeu_si256((__m256i *)(od + i), _mm256_add_epi64(hi, q));
    }
    for (; i < high_count; ++i) {
        int64_t e0 = ev[i];
        int64_t e1 = (i + 1u < low_count) ? ev[i + 1] : e0;
        od[i] = (int64_t)high[i] + jph_sra64_ref(e0 + e1, 1);
    }

    /* ---- narrow + interleave with int32-range check ---- */
    i = 0;
    for (; i + 4 <= high_count; i += 4) {
        __m256i e = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i o = _mm256_loadu_si256((const __m256i *)(od + i));
        __m128i e32, o32;
        ovf = _mm256_or_si256(ovf, _mm256_cmpgt_epi64(e, imax));
        ovf = _mm256_or_si256(ovf, _mm256_cmpgt_epi64(imin, e));
        ovf = _mm256_or_si256(ovf, _mm256_cmpgt_epi64(o, imax));
        ovf = _mm256_or_si256(ovf, _mm256_cmpgt_epi64(imin, o));
        e32 = jph_narrow_i64x4(e);
        o32 = jph_narrow_i64x4(o);
        _mm_storeu_si128((__m128i *)(out + 2u * i), _mm_unpacklo_epi32(e32, o32));
        _mm_storeu_si128((__m128i *)(out + 2u * i + 4u),
                         _mm_unpackhi_epi32(e32, o32));
    }
    if (!_mm256_testz_si256(ovf, ovf)) return EXR_ERROR_CORRUPT;
    for (; i < high_count; ++i) {
        if (ev[i] < INT32_MIN || ev[i] > INT32_MAX) return EXR_ERROR_CORRUPT;
        if (od[i] < INT32_MIN || od[i] > INT32_MAX) return EXR_ERROR_CORRUPT;
        out[2u * i] = (int32_t)ev[i];
        out[2u * i + 1u] = (int32_t)od[i];
    }
    if (low_count > high_count) { /* trailing even (odd out_count) */
        if (ev[high_count] < INT32_MIN || ev[high_count] > INT32_MAX)
            return EXR_ERROR_CORRUPT;
        out[2u * high_count] = (int32_t)ev[high_count];
    }
    return EXR_SUCCESS;
}

EXR_TARGET("avx2")
exr_result jph_inverse_53_i32_bounded_avx2(const int32_t *low,
                                           size_t low_count,
                                           const int32_t *high,
                                           size_t high_count, int32_t *out,
                                           size_t out_count, int64_t *ev,
                                           int64_t *od) {
    size_t i;
    size_t expected_low = (out_count + 1u) / 2u;
    size_t expected_high = out_count / 2u;
    __m256i two = _mm256_set1_epi64x(2);

    if (low_count != expected_low || high_count != expected_high)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (out_count == 0) return EXR_SUCCESS;
    if (high_count == 0) { out[0] = low[0]; return EXR_SUCCESS; }

    ev[0] = (int64_t)low[0] -
            jph_sra64_ref((int64_t)high[0] + (int64_t)high[0] + 2, 2);
    i = 1;
    for (; i + 4 <= high_count; i += 4) {
        __m256i dl = jph_widen_i32x4(high + i - 1);
        __m256i dr = jph_widen_i32x4(high + i);
        __m256i lo = jph_widen_i32x4(low + i);
        __m256i q = jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(dl, dr), two), 2);
        _mm256_storeu_si256((__m256i *)(ev + i), _mm256_sub_epi64(lo, q));
    }
    for (; i < low_count; ++i) {
        int64_t dl = high[i - 1];
        int64_t dr = (i < high_count) ? high[i] : high[high_count - 1];
        ev[i] = (int64_t)low[i] - jph_sra64_ref(dl + dr + 2, 2);
    }

    i = 0;
    for (; i + 4 <= high_count && i + 4 < low_count; i += 4) {
        __m256i e0 = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i e1 = _mm256_loadu_si256((const __m256i *)(ev + i + 1));
        __m256i hi = jph_widen_i32x4(high + i);
        __m256i q = jph_sra64x4(_mm256_add_epi64(e0, e1), 1);
        _mm256_storeu_si256((__m256i *)(od + i), _mm256_add_epi64(hi, q));
    }
    for (; i < high_count; ++i) {
        int64_t e0 = ev[i];
        int64_t e1 = (i + 1u < low_count) ? ev[i + 1] : e0;
        od[i] = (int64_t)high[i] + jph_sra64_ref(e0 + e1, 1);
    }

    i = 0;
    for (; i + 4 <= high_count; i += 4) {
        __m128i e32 = jph_narrow_i64x4(
            _mm256_loadu_si256((const __m256i *)(ev + i)));
        __m128i o32 = jph_narrow_i64x4(
            _mm256_loadu_si256((const __m256i *)(od + i)));
        _mm_storeu_si128((__m128i *)(out + 2u * i), _mm_unpacklo_epi32(e32, o32));
        _mm_storeu_si128((__m128i *)(out + 2u * i + 4u),
                         _mm_unpackhi_epi32(e32, o32));
    }
    for (; i < high_count; ++i) {
        out[2u * i] = (int32_t)ev[i];
        out[2u * i + 1u] = (int32_t)od[i];
    }
    if (low_count > high_count) out[2u * high_count] = (int32_t)ev[high_count];
    return EXR_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * Vertical (column) inverse reversible 5/3, int32, row-wise across all columns.
 * Bit-identical to the scalar exr_jph_inverse_53_vert_i32. `temp` holds lh
 * contiguous low-rows then hh high-rows (stride rw); writes rh = lh+hh rows
 * interleaved into `data` (stride width). Columns are the SIMD axis, so each
 * lifting step is a contiguous 4-lane int64 vector op over the row -- no gather
 * or scatter. Returns EXR_ERROR_CORRUPT iff any reconstructed sample leaves
 * int32. temp and data are distinct buffers; even rows (2i) and odd rows (2i+1)
 * are disjoint, so the phase-2 read-back of even rows is safe.
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
exr_result jph_inverse_53_vert_i32_avx2(const int32_t *temp, size_t rw,
                                        size_t lh, size_t hh,
                                        int32_t *data, size_t width) {
    size_t i, c;
    __m256i two = _mm256_set1_epi64x(2);
    __m256i imin = _mm256_set1_epi64x(INT32_MIN);
    __m256i imax = _mm256_set1_epi64x(INT32_MAX);
    __m256i ovf = _mm256_setzero_si256();
    int tail_ovf = 0;

    if (lh == 0u) return EXR_SUCCESS;
    if (hh == 0u) { /* rh == 1: copy low row 0 verbatim */
        for (c = 0u; c < rw; ++c) data[c] = temp[c];
        return EXR_SUCCESS;
    }

    /* ---- Phase 1: even rows -> data[2i] ---- */
    for (i = 0u; i < lh; ++i) {
        const int32_t *lo = temp + i * rw;
        const int32_t *hL = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
        const int32_t *hR = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
        int32_t *dst = data + (2u * i) * width;
        for (c = 0u; c + 4u <= rw; c += 4u) {
            __m256i dl = jph_widen_i32x4(hL + c);
            __m256i dr = jph_widen_i32x4(hR + c);
            __m256i lov = jph_widen_i32x4(lo + c);
            __m256i q =
                jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(dl, dr), two), 2);
            __m256i e = _mm256_sub_epi64(lov, q);
            ovf = _mm256_or_si256(ovf, _mm256_cmpgt_epi64(e, imax));
            ovf = _mm256_or_si256(ovf, _mm256_cmpgt_epi64(imin, e));
            _mm_storeu_si128((__m128i *)(dst + c), jph_narrow_i64x4(e));
        }
        for (; c < rw; ++c) {
            int64_t e = (int64_t)lo[c] -
                        jph_sra64_ref((int64_t)hL[c] + (int64_t)hR[c] + 2, 2);
            if (e < INT32_MIN || e > INT32_MAX) tail_ovf = 1;
            dst[c] = (int32_t)e;
        }
    }
    /* Gate: all evens of the plane are range-checked before any odd is read. */
    if (tail_ovf || !_mm256_testz_si256(ovf, ovf)) return EXR_ERROR_CORRUPT;

    /* ---- Phase 2: odd rows -> data[2i+1], reading even rows back ---- */
    ovf = _mm256_setzero_si256();
    for (i = 0u; i < hh; ++i) {
        const int32_t *hi = temp + (lh + i) * rw;
        const int32_t *e0p = data + (2u * i) * width;
        const int32_t *e1p = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        int32_t *dst = data + (2u * i + 1u) * width;
        for (c = 0u; c + 4u <= rw; c += 4u) {
            __m256i e0 = jph_widen_i32x4(e0p + c);
            __m256i e1 = jph_widen_i32x4(e1p + c);
            __m256i hv = jph_widen_i32x4(hi + c);
            __m256i q = jph_sra64x4(_mm256_add_epi64(e0, e1), 1);
            __m256i o = _mm256_add_epi64(hv, q);
            ovf = _mm256_or_si256(ovf, _mm256_cmpgt_epi64(o, imax));
            ovf = _mm256_or_si256(ovf, _mm256_cmpgt_epi64(imin, o));
            _mm_storeu_si128((__m128i *)(dst + c), jph_narrow_i64x4(o));
        }
        for (; c < rw; ++c) {
            int64_t o = (int64_t)hi[c] +
                        jph_sra64_ref((int64_t)e0p[c] + (int64_t)e1p[c], 1);
            if (o < INT32_MIN || o > INT32_MAX) tail_ovf = 1;
            dst[c] = (int32_t)o;
        }
    }
    if (tail_ovf || !_mm256_testz_si256(ovf, ovf)) return EXR_ERROR_CORRUPT;
    return EXR_SUCCESS;
}

EXR_TARGET("avx2")
exr_result jph_inverse_53_vert_i32_bounded_avx2(const int32_t *temp, size_t rw,
                                               size_t lh, size_t hh,
                                               int32_t *data, size_t width) {
    size_t i, c;
    __m256i two = _mm256_set1_epi32(2);

    if (lh == 0u) return EXR_SUCCESS;
    if (hh == 0u) {
        for (c = 0u; c < rw; ++c) data[c] = temp[c];
        return EXR_SUCCESS;
    }

    for (i = 0u; i < lh; ++i) {
        const int32_t *lo = temp + i * rw;
        const int32_t *hL = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
        const int32_t *hR = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
        int32_t *dst = data + (2u * i) * width;
        for (c = 0u; c + 8u <= rw; c += 8u) {
            __m256i dl = _mm256_loadu_si256((const __m256i *)(hL + c));
            __m256i dr = _mm256_loadu_si256((const __m256i *)(hR + c));
            __m256i lov = _mm256_loadu_si256((const __m256i *)(lo + c));
            __m256i q =
                _mm256_srai_epi32(_mm256_add_epi32(_mm256_add_epi32(dl, dr), two),
                                  2);
            __m256i e = _mm256_sub_epi32(lov, q);
            _mm256_storeu_si256((__m256i *)(dst + c), e);
        }
        for (; c < rw; ++c) {
            int64_t e = (int64_t)lo[c] -
                        jph_sra64_ref((int64_t)hL[c] + (int64_t)hR[c] + 2, 2);
            dst[c] = (int32_t)e;
        }
    }

    for (i = 0u; i < hh; ++i) {
        const int32_t *hi = temp + (lh + i) * rw;
        const int32_t *e0p = data + (2u * i) * width;
        const int32_t *e1p = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        int32_t *dst = data + (2u * i + 1u) * width;
        for (c = 0u; c + 8u <= rw; c += 8u) {
            __m256i e0 = _mm256_loadu_si256((const __m256i *)(e0p + c));
            __m256i e1 = _mm256_loadu_si256((const __m256i *)(e1p + c));
            __m256i hv = _mm256_loadu_si256((const __m256i *)(hi + c));
            __m256i q = _mm256_srai_epi32(_mm256_add_epi32(e0, e1), 1);
            __m256i o = _mm256_add_epi32(hv, q);
            _mm256_storeu_si256((__m256i *)(dst + c), o);
        }
        for (; c < rw; ++c) {
            int64_t o = (int64_t)hi[c] +
                        jph_sra64_ref((int64_t)e0p[c] + (int64_t)e1p[c], 1);
            dst[c] = (int32_t)o;
        }
    }
    return EXR_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * Inverse reversible 5/3 1D lifting (int64, the float/32-bit decode path).
 * Bit-identical to jph_inverse_53_i64. Pure int64 -- no narrowing or range
 * check (the final samples are narrowed to int32 at store). ev/od are caller
 * scratch of >= low_count / >= high_count int64 each.
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
EXR_NO_SANITIZE_SIO
exr_result jph_inverse_53_i64_avx2(const int64_t *low, size_t low_count,
                                   const int64_t *high, size_t high_count,
                                   int64_t *out, size_t out_count,
                                   int64_t *ev, int64_t *od) {
    size_t i;
    size_t expected_low = (out_count + 1u) / 2u;
    size_t expected_high = out_count / 2u;
    __m256i two = _mm256_set1_epi64x(2);
    if (low_count != expected_low || high_count != expected_high)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (out_count == 0) return EXR_SUCCESS;
    if (high_count == 0) { out[0] = low[0]; return EXR_SUCCESS; }

    /* predict: ev[i] = low[i] - floor((dl+dr+2)/4) */
    ev[0] = low[0] - jph_sra64_ref(high[0] + high[0] + 2, 2);
    i = 1;
    for (; i + 4 <= high_count; i += 4) {
        __m256i dl = _mm256_loadu_si256((const __m256i *)(high + i - 1));
        __m256i dr = _mm256_loadu_si256((const __m256i *)(high + i));
        __m256i lo = _mm256_loadu_si256((const __m256i *)(low + i));
        __m256i q = jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(dl, dr), two), 2);
        _mm256_storeu_si256((__m256i *)(ev + i), _mm256_sub_epi64(lo, q));
    }
    for (; i < low_count; ++i) {
        int64_t dl = high[i - 1];
        int64_t dr = (i < high_count) ? high[i] : high[high_count - 1];
        ev[i] = low[i] - jph_sra64_ref(dl + dr + 2, 2);
    }

    /* update: od[i] = high[i] + floor((ev[i]+ev[i+1])/2) */
    i = 0;
    for (; i + 4 <= high_count && i + 4 < low_count; i += 4) {
        __m256i e0 = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i e1 = _mm256_loadu_si256((const __m256i *)(ev + i + 1));
        __m256i hi = _mm256_loadu_si256((const __m256i *)(high + i));
        __m256i q = jph_sra64x4(_mm256_add_epi64(e0, e1), 1);
        _mm256_storeu_si256((__m256i *)(od + i), _mm256_add_epi64(hi, q));
    }
    for (; i < high_count; ++i) {
        int64_t e0 = ev[i];
        int64_t e1 = (i + 1u < low_count) ? ev[i + 1] : e0;
        od[i] = high[i] + jph_sra64_ref(e0 + e1, 1);
    }

    /* interleave ev/od -> out (no narrowing) */
    i = 0;
    for (; i + 4 <= high_count; i += 4) {
        __m256i e = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i o = _mm256_loadu_si256((const __m256i *)(od + i));
        __m256i lo = _mm256_unpacklo_epi64(e, o);
        __m256i hi = _mm256_unpackhi_epi64(e, o);
        _mm256_storeu_si256((__m256i *)(out + 2u * i),
                            _mm256_permute2x128_si256(lo, hi, 0x20));
        _mm256_storeu_si256((__m256i *)(out + 2u * i + 4u),
                            _mm256_permute2x128_si256(lo, hi, 0x31));
    }
    for (; i < high_count; ++i) { out[2u * i] = ev[i]; out[2u * i + 1u] = od[i]; }
    if (low_count > high_count) out[2u * high_count] = ev[high_count];
    return EXR_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * Vertical (column) inverse reversible 5/3, int64, row-wise across all columns.
 * Bit-identical to jph_inverse_53_vert_i64. Pure int64 (no range check). temp:
 * lh low-rows then hh high-rows (stride rw) -> rh interleaved rows in data
 * (stride width). temp and data are distinct; even/odd output rows are disjoint.
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
EXR_NO_SANITIZE_SIO
exr_result jph_inverse_53_vert_i64_avx2(const int64_t *temp, size_t rw,
                                        size_t lh, size_t hh,
                                        int64_t *data, size_t width) {
    size_t i, c;
    __m256i two = _mm256_set1_epi64x(2);
    if (lh == 0u) return EXR_SUCCESS;
    if (hh == 0u) { for (c = 0u; c < rw; ++c) data[c] = temp[c]; return EXR_SUCCESS; }

    /* Phase 1: even rows -> data[2i] */
    for (i = 0u; i < lh; ++i) {
        const int64_t *lo = temp + i * rw;
        const int64_t *hL = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
        const int64_t *hR = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
        int64_t *dst = data + (2u * i) * width;
        for (c = 0u; c + 4u <= rw; c += 4u) {
            __m256i l = _mm256_loadu_si256((const __m256i *)(lo + c));
            __m256i a = _mm256_loadu_si256((const __m256i *)(hL + c));
            __m256i b = _mm256_loadu_si256((const __m256i *)(hR + c));
            __m256i q =
                jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(a, b), two), 2);
            _mm256_storeu_si256((__m256i *)(dst + c), _mm256_sub_epi64(l, q));
        }
        for (; c < rw; ++c) dst[c] = lo[c] - jph_sra64_ref(hL[c] + hR[c] + 2, 2);
    }
    /* Phase 2: odd rows -> data[2i+1], reading even rows back from data */
    for (i = 0u; i < hh; ++i) {
        const int64_t *hi = temp + (lh + i) * rw;
        const int64_t *e0p = data + (2u * i) * width;
        const int64_t *e1p = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        int64_t *dst = data + (2u * i + 1u) * width;
        for (c = 0u; c + 4u <= rw; c += 4u) {
            __m256i h = _mm256_loadu_si256((const __m256i *)(hi + c));
            __m256i e0 = _mm256_loadu_si256((const __m256i *)(e0p + c));
            __m256i e1 = _mm256_loadu_si256((const __m256i *)(e1p + c));
            __m256i q = jph_sra64x4(_mm256_add_epi64(e0, e1), 1);
            _mm256_storeu_si256((__m256i *)(dst + c), _mm256_add_epi64(h, q));
        }
        for (; c < rw; ++c) dst[c] = hi[c] + jph_sra64_ref(e0p[c] + e1p[c], 1);
    }
    return EXR_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * Sign-magnitude -> signed int64. Converts a row of OpenJPH codeblock words
 * (bit 31 = sign, bits 0..30 = magnitude) to signed reversible-transform
 * coefficients: mag = (v & 0x7fffffff) >> shift; out = sign ? -mag : mag,
 * sign-extended to int64. Bit-identical to the scalar loop in jph_decode_block.
 * mag is always in [0, 2^31-1] so -mag never reaches INT32_MIN -- no overflow.
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
void jph_extract_signmag_i32_to_i64_avx2(int64_t *out, const uint32_t *buf,
                                         size_t n, unsigned shift) {
    size_t i = 0;
    const __m256i magmask = _mm256_set1_epi32(0x7fffffff);
    const __m256i zero = _mm256_setzero_si256();
    const __m128i sh = _mm_cvtsi32_si128((int)shift);
    for (; i + 8 <= n; i += 8) {
        __m256i v = _mm256_loadu_si256((const __m256i *)(buf + i));
        __m256i mag = _mm256_srl_epi32(_mm256_and_si256(v, magmask), sh);
        __m256i neg = _mm256_sub_epi32(zero, mag);
        __m256i smask = _mm256_srai_epi32(v, 31); /* all-ones where sign set */
        __m256i r = _mm256_blendv_epi8(mag, neg, smask); /* int32 result/lane */
        /* widen 8x int32 -> 2x 4x int64 (sign-extend) and store */
        _mm256_storeu_si256((__m256i *)(out + i),
                            _mm256_cvtepi32_epi64(_mm256_castsi256_si128(r)));
        _mm256_storeu_si256((__m256i *)(out + i + 4),
                            _mm256_cvtepi32_epi64(_mm256_extracti128_si256(r, 1)));
    }
    for (; i < n; ++i) {
        uint32_t v = buf[i];
        int32_t mag = (int32_t)((v & 0x7fffffffu) >> shift);
        out[i] = (v & 0x80000000u) ? -mag : mag;
    }
}

EXR_TARGET("avx2")
void jph_extract_signmag_i32_to_i32_avx2(int32_t *out, const uint32_t *buf,
                                         size_t n, unsigned shift) {
    size_t i = 0;
    const __m256i magmask = _mm256_set1_epi32(0x7fffffff);
    const __m256i zero = _mm256_setzero_si256();
    const __m128i sh = _mm_cvtsi32_si128((int)shift);
    for (; i + 8 <= n; i += 8) {
        __m256i v = _mm256_loadu_si256((const __m256i *)(buf + i));
        __m256i mag = _mm256_srl_epi32(_mm256_and_si256(v, magmask), sh);
        __m256i neg = _mm256_sub_epi32(zero, mag);
        __m256i smask = _mm256_srai_epi32(v, 31);
        __m256i r = _mm256_blendv_epi8(mag, neg, smask);
        _mm256_storeu_si256((__m256i *)(out + i), r);
    }
    for (; i < n; ++i) {
        uint32_t v = buf[i];
        int32_t mag = (int32_t)((v & 0x7fffffffu) >> shift);
        out[i] = (v & 0x80000000u) ? -mag : mag;
    }
}

static inline uint64_t jph_simd_fetch64_at(const uint64_t *buf,
                                           uint64_t real_bits, uint64_t c) {
    uint64_t v, avail;
    uint32_t off;
    if (c >= real_bits) return ~UINT64_C(0);
    off = (uint32_t)(c & 63u);
    v = buf[c >> 6u] >> off;
    if (off) v |= buf[(c >> 6u) + 1u] << (64u - off);
    avail = real_bits - c;
    if (avail < 64u) v |= ~UINT64_C(0) << avail;
    return v;
}

static inline __m128i jph_simd_fetch128_at(const uint64_t *buf,
                                           uint64_t real_bits, uint64_t c) {
    uint64_t lo = jph_simd_fetch64_at(buf, real_bits, c);
    uint64_t hi = jph_simd_fetch64_at(buf, real_bits, c + 64u);
    return _mm_set_epi64x((int64_t)hi, (int64_t)lo);
}

EXR_TARGET("avx2")
static void jph_frwd_read_ff_avx2(JphFrwdAvx2 *msp) {
    __m128i offset, val, validity, all_xff, ff_bytes;
    uint8_t tail[16];
    uint32_t flags, next_unstuff;
    int bytes, bits = 128;
    int cur_bytes, cur_bits, consumed_bits, upper;

    if (msp->size >= 16) {
        val = _mm_loadu_si128((const __m128i *)msp->data);
        bytes = 16;
    } else {
        bytes = msp->size > 0 ? msp->size : 0;
        memset(tail, 0xff, sizeof(tail));
        if (bytes) memcpy(tail, msp->data, (size_t)bytes);
        val = _mm_loadu_si128((const __m128i *)tail);
    }
    msp->data += bytes;
    msp->size -= bytes;

    offset = _mm_set_epi64x(INT64_C(0x0F0E0D0C0B0A0908),
                            INT64_C(0x0706050403020100));
    validity = _mm_cmpgt_epi8(_mm_set1_epi8((char)bytes), offset);
    all_xff = _mm_set1_epi8(-1);
    val = _mm_or_si128(_mm_xor_si128(validity, all_xff), val);

    ff_bytes = _mm_cmpeq_epi8(val, all_xff);
    ff_bytes = _mm_and_si128(ff_bytes, validity);
    flags = (uint32_t)_mm_movemask_epi8(ff_bytes);
    flags <<= 1;
    next_unstuff = flags >> 16u;
    flags = (flags | msp->unstuff) & 0xffffu;
    while (flags) {
        uint32_t loc = 31u - (uint32_t)__builtin_clz(flags);
        __m128i t, m, c;
        --bits;
        flags ^= 1u << loc;
        t = _mm_set1_epi8((char)loc);
        m = _mm_cmpgt_epi8(offset, t);
        t = _mm_and_si128(m, val);
        c = _mm_srli_epi64(t, 1);
        t = _mm_srli_si128(t, 8);
        t = _mm_slli_epi64(t, 63);
        t = _mm_or_si128(t, c);
        val = _mm_or_si128(t, _mm_andnot_si128(m, val));
    }

    cur_bytes = (int)(msp->bits >> 3u);
    cur_bits = (int)(msp->bits & 7u);
    {
        __m128i b1 = _mm_sll_epi64(val, _mm_set1_epi64x(cur_bits));
        __m128i b2 = _mm_slli_si128(val, 8);
        b2 = _mm_srl_epi64(b2, _mm_set1_epi64x(64 - cur_bits));
        b1 = _mm_or_si128(b1, b2);
        b2 = _mm_loadu_si128((const __m128i *)(msp->tmp + cur_bytes));
        b2 = _mm_or_si128(b1, b2);
        _mm_storeu_si128((__m128i *)(msp->tmp + cur_bytes), b2);
    }

    consumed_bits = bits < 128 - cur_bits ? bits : 128 - cur_bits;
    cur_bytes = (int)((msp->bits + (uint32_t)consumed_bits + 7u) >> 3u);
    upper = _mm_extract_epi16(val, 7);
    upper >>= consumed_bits - 112;
    msp->tmp[cur_bytes] = (uint8_t)upper;
    msp->bits += (uint32_t)bits;
    msp->unstuff = next_unstuff;
}

EXR_TARGET("avx2")
void jph_frwd_init_ff_avx2(JphFrwdAvx2 *msp, const uint8_t *data, int size) {
    msp->data = data;
    _mm_storeu_si128((__m128i *)msp->tmp, _mm_setzero_si128());
    _mm_storeu_si128((__m128i *)(msp->tmp + 16), _mm_setzero_si128());
    _mm_storeu_si128((__m128i *)(msp->tmp + 32), _mm_setzero_si128());
    msp->bits = 0;
    msp->unstuff = 0;
    msp->size = size;
    jph_frwd_read_ff_avx2(msp);
}

EXR_TARGET("avx2")
static __m128i jph_frwd_fetch_ff_avx2(JphFrwdAvx2 *msp) {
    if (msp->bits <= 128u) {
        jph_frwd_read_ff_avx2(msp);
        if (msp->bits <= 128u) jph_frwd_read_ff_avx2(msp);
    }
    return _mm_loadu_si128((const __m128i *)msp->tmp);
}

EXR_TARGET("avx2")
static void jph_frwd_advance_avx2(JphFrwdAvx2 *msp, uint32_t num_bits) {
    __m128i *p = (__m128i *)(msp->tmp + ((num_bits >> 3u) & 0x18u));
    __m128i v0, v1, c0, c1, t;
    msp->bits -= num_bits;
    num_bits &= 63u;

    v0 = _mm_loadu_si128(p);
    v1 = _mm_loadu_si128(p + 1);
    c0 = _mm_srl_epi64(v0, _mm_set1_epi64x((int)num_bits));
    t = _mm_srli_si128(v0, 8);
    t = _mm_sll_epi64(t, _mm_set1_epi64x((int)(64u - num_bits)));
    c0 = _mm_or_si128(c0, t);
    t = _mm_slli_si128(v1, 8);
    t = _mm_sll_epi64(t, _mm_set1_epi64x((int)(64u - num_bits)));
    c0 = _mm_or_si128(c0, t);
    _mm_storeu_si128((__m128i *)msp->tmp, c0);

    c1 = _mm_srl_epi64(v1, _mm_set1_epi64x((int)num_bits));
    t = _mm_srli_si128(v1, 8);
    t = _mm_sll_epi64(t, _mm_set1_epi64x((int)(64u - num_bits)));
    c1 = _mm_or_si128(c1, t);
    _mm_storeu_si128((__m128i *)(msp->tmp + 16), c1);
}

/* Four-quad MagSgn decode for the <=15-bit cleanup path. Derived from
 * OpenJPH's decode_four_quad16, adapted to TinyEXR's pre-unstuffed bit buffer.
 */
EXR_TARGET("avx2")
void jph_decode_four_quad16_avx2(uint32_t *row0, uint32_t *row1,
                                 uint16_t bottom_vn[8],
                                 const uint16_t *inf_uq,
                                 const uint32_t u_q[4],
                                 const uint64_t *bits, uint64_t real_bits,
                                 uint64_t *cursor, unsigned p16) {
    __m256i row = _mm256_setzero_si256();
    __m128i inf = _mm_loadu_si128((const __m128i *)inf_uq);
    __m128i Uq = _mm_setr_epi32((int)u_q[0], (int)u_q[1],
                                (int)u_q[2], (int)u_q[3]);
    __m128i ddd = _mm_shuffle_epi8(
        inf, _mm_set_epi16(0x0d0c, 0x0d0c, 0x0908, 0x0908,
                           0x0504, 0x0504, 0x0100, 0x0100));
    __m256i w0 = _mm256_permutevar8x32_epi32(
        _mm256_castsi128_si256(ddd),
        _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3));
    __m256i flags = _mm256_and_si256(
        w0, _mm256_set_epi16((int16_t)0x8880, 0x4440, 0x2220, 0x1110,
                             (int16_t)0x8880, 0x4440, 0x2220, 0x1110,
                             (int16_t)0x8880, 0x4440, 0x2220, 0x1110,
                             (int16_t)0x8880, 0x4440, 0x2220, 0x1110));
    __m256i insig = _mm256_cmpeq_epi16(flags, _mm256_setzero_si256());
    uint16_t vn_lane[16];

    memset(vn_lane, 0, sizeof(vn_lane));
    if ((uint32_t)_mm256_movemask_epi8(insig) != UINT32_MAX) {
        __m256i Uq_avx;
        __m256i m_n, inc_sum, ex_sum, ms_vec, byte_idx, bit_idx;
        __m256i d0, d1, bit_shift, shift, t0, t1, Uq0, Uq1, tvn;
        __m128i ms_vec0 = _mm_setzero_si128();
        __m128i ms_vec1 = _mm_setzero_si128();
        int total_mn1, total_mn2;
        const __m256i ones = _mm256_set1_epi16(1);
        const __m256i twos = _mm256_set1_epi16(2);

        ddd = _mm_or_si128(_mm_bslli_si128(Uq, 2), Uq);
        Uq_avx = _mm256_permutevar8x32_epi32(
            _mm256_castsi128_si256(ddd),
            _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3));
        flags = _mm256_mullo_epi16(
            flags, _mm256_set_epi16(1, 2, 4, 8, 1, 2, 4, 8,
                                    1, 2, 4, 8, 1, 2, 4, 8));

        w0 = _mm256_srli_epi16(flags, 15); /* e_k */
        m_n = _mm256_sub_epi16(Uq_avx, w0);
        m_n = _mm256_andnot_si256(insig, m_n);

        inc_sum = m_n;
        inc_sum = _mm256_add_epi16(inc_sum, _mm256_bslli_epi128(inc_sum, 2));
        inc_sum = _mm256_add_epi16(inc_sum, _mm256_bslli_epi128(inc_sum, 4));
        inc_sum = _mm256_add_epi16(inc_sum, _mm256_bslli_epi128(inc_sum, 8));
        total_mn1 = _mm256_extract_epi16(inc_sum, 7);
        total_mn2 = _mm256_extract_epi16(inc_sum, 15);
        ex_sum = _mm256_bslli_epi128(inc_sum, 2);

        if (total_mn1) {
            ms_vec0 = jph_simd_fetch128_at(bits, real_bits, *cursor);
            *cursor += (uint32_t)total_mn1;
        }
        if (total_mn2) {
            ms_vec1 = jph_simd_fetch128_at(bits, real_bits, *cursor);
            *cursor += (uint32_t)total_mn2;
        }
        ms_vec = _mm256_inserti128_si256(_mm256_castsi128_si256(ms_vec0),
                                         ms_vec1, 1);

        byte_idx = _mm256_srli_epi16(ex_sum, 3);
        bit_idx = _mm256_and_si256(ex_sum, _mm256_set1_epi16(7));
        byte_idx = _mm256_shuffle_epi8(
            byte_idx,
            _mm256_set_epi16(0x0E0E, 0x0C0C, 0x0A0A, 0x0808,
                             0x0606, 0x0404, 0x0202, 0x0000,
                             0x0E0E, 0x0C0C, 0x0A0A, 0x0808,
                             0x0606, 0x0404, 0x0202, 0x0000));
        byte_idx = _mm256_add_epi16(byte_idx, _mm256_set1_epi16(0x0100));
        d0 = _mm256_shuffle_epi8(ms_vec, byte_idx);
        byte_idx = _mm256_add_epi16(byte_idx, _mm256_set1_epi16(0x0101));
        d1 = _mm256_shuffle_epi8(ms_vec, byte_idx);

        bit_shift = _mm256_shuffle_epi8(
            _mm256_set_epi8(1, 3, 7, 15, 31, 63, 127, -1,
                            1, 3, 7, 15, 31, 63, 127, -1,
                            1, 3, 7, 15, 31, 63, 127, -1,
                            1, 3, 7, 15, 31, 63, 127, -1),
            bit_idx);
        bit_shift = _mm256_add_epi16(bit_shift, _mm256_set1_epi16(0x0101));
        d0 = _mm256_mullo_epi16(d0, bit_shift);
        d0 = _mm256_srli_epi16(d0, 8);
        d1 = _mm256_mullo_epi16(d1, bit_shift);
        d1 = _mm256_and_si256(d1, _mm256_set1_epi16((int16_t)0xFF00));
        d0 = _mm256_or_si256(d0, d1);

        {
            __m256i Uq_m1 = _mm256_sub_epi32(Uq_avx, ones);
            Uq0 = _mm256_and_si256(
                Uq_m1, _mm256_set_epi32(0, 0, 0, 0x1F, 0, 0, 0, 0x1F));
            Uq1 = _mm256_bsrli_epi128(Uq_m1, 14);
        }
        w0 = _mm256_sub_epi16(twos, w0);
        t0 = _mm256_and_si256(w0, _mm256_set_epi64x(0, -1, 0, -1));
        t1 = _mm256_and_si256(w0, _mm256_set_epi64x(-1, 0, -1, 0));
        {
            __m128i lo = _mm256_castsi256_si128(t0);
            __m128i hi = _mm256_extracti128_si256(t0, 1);
            lo = _mm_sll_epi16(lo, _mm256_castsi256_si128(Uq0));
            hi = _mm_sll_epi16(hi, _mm256_extracti128_si256(Uq0, 1));
            t0 = _mm256_inserti128_si256(_mm256_castsi128_si256(lo), hi, 1);

            lo = _mm256_castsi256_si128(t1);
            hi = _mm256_extracti128_si256(t1, 1);
            lo = _mm_sll_epi16(lo, _mm256_castsi256_si128(Uq1));
            hi = _mm_sll_epi16(hi, _mm256_extracti128_si256(Uq1, 1));
            t1 = _mm256_inserti128_si256(_mm256_castsi128_si256(lo), hi, 1);
        }
        shift = _mm256_or_si256(t0, t1);
        ms_vec = _mm256_and_si256(d0, _mm256_sub_epi16(shift, ones));

        w0 = _mm256_and_si256(flags, _mm256_set1_epi16(0x800));
        w0 = _mm256_cmpeq_epi16(w0, _mm256_setzero_si256());
        w0 = _mm256_andnot_si256(w0, shift);
        ms_vec = _mm256_or_si256(ms_vec, w0);
        w0 = _mm256_slli_epi16(ms_vec, 15);
        ms_vec = _mm256_or_si256(ms_vec, ones);
        tvn = _mm256_andnot_si256(insig, ms_vec);
        _mm256_storeu_si256((__m256i *)vn_lane, tvn);

        ms_vec = _mm256_add_epi16(ms_vec, twos);
        ms_vec = _mm256_slli_epi16(ms_vec, (int)p16 - 1);
        ms_vec = _mm256_or_si256(ms_vec, w0);
        row = _mm256_andnot_si256(insig, ms_vec);
    }

    {
        __m256i r0 = _mm256_shuffle_epi8(
            row, _mm256_set_epi16(0x0D0C, -1, 0x0908, -1,
                                  0x0504, -1, 0x0100, -1,
                                  0x0D0C, -1, 0x0908, -1,
                                  0x0504, -1, 0x0100, -1));
        __m256i r1 = _mm256_shuffle_epi8(
            row, _mm256_set_epi16(0x0F0E, -1, 0x0B0A, -1,
                                  0x0706, -1, 0x0302, -1,
                                  0x0F0E, -1, 0x0B0A, -1,
                                  0x0706, -1, 0x0302, -1));
        _mm256_storeu_si256((__m256i *)row0, r0);
        _mm256_storeu_si256((__m256i *)row1, r1);
    }

    bottom_vn[0] = vn_lane[1];
    bottom_vn[1] = vn_lane[3];
    bottom_vn[2] = vn_lane[5];
    bottom_vn[3] = vn_lane[7];
    bottom_vn[4] = vn_lane[9];
    bottom_vn[5] = vn_lane[11];
    bottom_vn[6] = vn_lane[13];
    bottom_vn[7] = vn_lane[15];
}

/* Same four-quad kernel, but using OpenJPH's rolling forward MagSgn reader to
 * avoid pre-unstuffing the cleanup bitstream into a separate random-access
 * buffer. */
EXR_TARGET("avx2")
void jph_decode_four_quad16_frwd_avx2(uint32_t *row0, uint32_t *row1,
                                      uint16_t bottom_vn[8],
                                      const uint16_t *inf_uq,
                                      const uint32_t u_q[4],
                                      JphFrwdAvx2 *magsgn,
                                      unsigned p16) {
    __m256i row = _mm256_setzero_si256();
    __m128i inf = _mm_loadu_si128((const __m128i *)inf_uq);
    __m128i Uq = _mm_setr_epi32((int)u_q[0], (int)u_q[1],
                                (int)u_q[2], (int)u_q[3]);
    __m128i ddd = _mm_shuffle_epi8(
        inf, _mm_set_epi16(0x0d0c, 0x0d0c, 0x0908, 0x0908,
                           0x0504, 0x0504, 0x0100, 0x0100));
    __m256i w0 = _mm256_permutevar8x32_epi32(
        _mm256_castsi128_si256(ddd),
        _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3));
    __m256i flags = _mm256_and_si256(
        w0, _mm256_set_epi16((int16_t)0x8880, 0x4440, 0x2220, 0x1110,
                             (int16_t)0x8880, 0x4440, 0x2220, 0x1110,
                             (int16_t)0x8880, 0x4440, 0x2220, 0x1110,
                             (int16_t)0x8880, 0x4440, 0x2220, 0x1110));
    __m256i insig = _mm256_cmpeq_epi16(flags, _mm256_setzero_si256());
    uint16_t vn_lane[16];

    memset(vn_lane, 0, sizeof(vn_lane));
    if ((uint32_t)_mm256_movemask_epi8(insig) != UINT32_MAX) {
        __m256i Uq_avx;
        __m256i m_n, inc_sum, ex_sum, ms_vec, byte_idx, bit_idx;
        __m256i d0, d1, bit_shift, shift, t0, t1, Uq0, Uq1, tvn;
        __m128i ms_vec0 = _mm_setzero_si128();
        __m128i ms_vec1 = _mm_setzero_si128();
        int total_mn1, total_mn2;
        const __m256i ones = _mm256_set1_epi16(1);
        const __m256i twos = _mm256_set1_epi16(2);

        ddd = _mm_or_si128(_mm_bslli_si128(Uq, 2), Uq);
        Uq_avx = _mm256_permutevar8x32_epi32(
            _mm256_castsi128_si256(ddd),
            _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3));
        flags = _mm256_mullo_epi16(
            flags, _mm256_set_epi16(1, 2, 4, 8, 1, 2, 4, 8,
                                    1, 2, 4, 8, 1, 2, 4, 8));

        w0 = _mm256_srli_epi16(flags, 15);
        m_n = _mm256_sub_epi16(Uq_avx, w0);
        m_n = _mm256_andnot_si256(insig, m_n);

        inc_sum = m_n;
        inc_sum = _mm256_add_epi16(inc_sum, _mm256_bslli_epi128(inc_sum, 2));
        inc_sum = _mm256_add_epi16(inc_sum, _mm256_bslli_epi128(inc_sum, 4));
        inc_sum = _mm256_add_epi16(inc_sum, _mm256_bslli_epi128(inc_sum, 8));
        total_mn1 = _mm256_extract_epi16(inc_sum, 7);
        total_mn2 = _mm256_extract_epi16(inc_sum, 15);
        ex_sum = _mm256_bslli_epi128(inc_sum, 2);

        if (total_mn1) {
            ms_vec0 = jph_frwd_fetch_ff_avx2(magsgn);
            jph_frwd_advance_avx2(magsgn, (uint32_t)total_mn1);
        }
        if (total_mn2) {
            ms_vec1 = jph_frwd_fetch_ff_avx2(magsgn);
            jph_frwd_advance_avx2(magsgn, (uint32_t)total_mn2);
        }
        ms_vec = _mm256_inserti128_si256(_mm256_castsi128_si256(ms_vec0),
                                         ms_vec1, 1);

        byte_idx = _mm256_srli_epi16(ex_sum, 3);
        bit_idx = _mm256_and_si256(ex_sum, _mm256_set1_epi16(7));
        byte_idx = _mm256_shuffle_epi8(
            byte_idx,
            _mm256_set_epi16(0x0E0E, 0x0C0C, 0x0A0A, 0x0808,
                             0x0606, 0x0404, 0x0202, 0x0000,
                             0x0E0E, 0x0C0C, 0x0A0A, 0x0808,
                             0x0606, 0x0404, 0x0202, 0x0000));
        byte_idx = _mm256_add_epi16(byte_idx, _mm256_set1_epi16(0x0100));
        d0 = _mm256_shuffle_epi8(ms_vec, byte_idx);
        byte_idx = _mm256_add_epi16(byte_idx, _mm256_set1_epi16(0x0101));
        d1 = _mm256_shuffle_epi8(ms_vec, byte_idx);

        bit_shift = _mm256_shuffle_epi8(
            _mm256_set_epi8(1, 3, 7, 15, 31, 63, 127, -1,
                            1, 3, 7, 15, 31, 63, 127, -1,
                            1, 3, 7, 15, 31, 63, 127, -1,
                            1, 3, 7, 15, 31, 63, 127, -1),
            bit_idx);
        bit_shift = _mm256_add_epi16(bit_shift, _mm256_set1_epi16(0x0101));
        d0 = _mm256_mullo_epi16(d0, bit_shift);
        d0 = _mm256_srli_epi16(d0, 8);
        d1 = _mm256_mullo_epi16(d1, bit_shift);
        d1 = _mm256_and_si256(d1, _mm256_set1_epi16((int16_t)0xFF00));
        d0 = _mm256_or_si256(d0, d1);

        {
            __m256i Uq_m1 = _mm256_sub_epi32(Uq_avx, ones);
            Uq0 = _mm256_and_si256(
                Uq_m1, _mm256_set_epi32(0, 0, 0, 0x1F, 0, 0, 0, 0x1F));
            Uq1 = _mm256_bsrli_epi128(Uq_m1, 14);
        }
        w0 = _mm256_sub_epi16(twos, w0);
        t0 = _mm256_and_si256(w0, _mm256_set_epi64x(0, -1, 0, -1));
        t1 = _mm256_and_si256(w0, _mm256_set_epi64x(-1, 0, -1, 0));
        {
            __m128i lo = _mm256_castsi256_si128(t0);
            __m128i hi = _mm256_extracti128_si256(t0, 1);
            lo = _mm_sll_epi16(lo, _mm256_castsi256_si128(Uq0));
            hi = _mm_sll_epi16(hi, _mm256_extracti128_si256(Uq0, 1));
            t0 = _mm256_inserti128_si256(_mm256_castsi128_si256(lo), hi, 1);

            lo = _mm256_castsi256_si128(t1);
            hi = _mm256_extracti128_si256(t1, 1);
            lo = _mm_sll_epi16(lo, _mm256_castsi256_si128(Uq1));
            hi = _mm_sll_epi16(hi, _mm256_extracti128_si256(Uq1, 1));
            t1 = _mm256_inserti128_si256(_mm256_castsi128_si256(lo), hi, 1);
        }
        shift = _mm256_or_si256(t0, t1);
        ms_vec = _mm256_and_si256(d0, _mm256_sub_epi16(shift, ones));

        w0 = _mm256_and_si256(flags, _mm256_set1_epi16(0x800));
        w0 = _mm256_cmpeq_epi16(w0, _mm256_setzero_si256());
        w0 = _mm256_andnot_si256(w0, shift);
        ms_vec = _mm256_or_si256(ms_vec, w0);
        w0 = _mm256_slli_epi16(ms_vec, 15);
        ms_vec = _mm256_or_si256(ms_vec, ones);
        tvn = _mm256_andnot_si256(insig, ms_vec);
        _mm256_storeu_si256((__m256i *)vn_lane, tvn);

        ms_vec = _mm256_add_epi16(ms_vec, twos);
        ms_vec = _mm256_slli_epi16(ms_vec, (int)p16 - 1);
        ms_vec = _mm256_or_si256(ms_vec, w0);
        row = _mm256_andnot_si256(insig, ms_vec);
    }

    {
        __m256i r0 = _mm256_shuffle_epi8(
            row, _mm256_set_epi16(0x0D0C, -1, 0x0908, -1,
                                  0x0504, -1, 0x0100, -1,
                                  0x0D0C, -1, 0x0908, -1,
                                  0x0504, -1, 0x0100, -1));
        __m256i r1 = _mm256_shuffle_epi8(
            row, _mm256_set_epi16(0x0F0E, -1, 0x0B0A, -1,
                                  0x0706, -1, 0x0302, -1,
                                  0x0F0E, -1, 0x0B0A, -1,
                                  0x0706, -1, 0x0302, -1));
        _mm256_storeu_si256((__m256i *)row0, r0);
        _mm256_storeu_si256((__m256i *)row1, r1);
    }

    bottom_vn[0] = vn_lane[1];
    bottom_vn[1] = vn_lane[3];
    bottom_vn[2] = vn_lane[5];
    bottom_vn[3] = vn_lane[7];
    bottom_vn[4] = vn_lane[9];
    bottom_vn[5] = vn_lane[11];
    bottom_vn[6] = vn_lane[13];
    bottom_vn[7] = vn_lane[15];
}

/* OpenJPH AVX2 32-bit cleanup path: two HT quads (4 columns x 2 rows). */
EXR_TARGET("avx2")
void jph_decode_two_quad32_frwd_avx2(uint32_t *row0, uint32_t *row1,
                                     uint32_t bottom_vn[4],
                                     const uint16_t *inf_uq,
                                     const uint32_t u_q[2],
                                     JphFrwdAvx2 *magsgn, unsigned p) {
    __m256i row = _mm256_setzero_si256();
    __m256i inf = _mm256_setr_epi32((int)inf_uq[0], (int)inf_uq[0],
                                    (int)inf_uq[0], (int)inf_uq[0],
                                    (int)inf_uq[2], (int)inf_uq[2],
                                    (int)inf_uq[2], (int)inf_uq[2]);
    __m256i Uq = _mm256_setr_epi32((int)u_q[0], (int)u_q[0],
                                   (int)u_q[0], (int)u_q[0],
                                   (int)u_q[1], (int)u_q[1],
                                   (int)u_q[1], (int)u_q[1]);
    __m256i flags = _mm256_and_si256(
        inf, _mm256_set_epi32(0x8880, 0x4440, 0x2220, 0x1110,
                              0x8880, 0x4440, 0x2220, 0x1110));
    __m256i insig = _mm256_cmpeq_epi32(flags, _mm256_setzero_si256());
    uint32_t vn_lane[8];

    bottom_vn[0] = 0u;
    bottom_vn[1] = 0u;
    bottom_vn[2] = 0u;
    bottom_vn[3] = 0u;
    if ((uint32_t)_mm256_movemask_epi8(insig) != UINT32_MAX) {
        __m256i m_n, inc_sum, ex_sum, ms_vec, byte_idx, bit_idx;
        __m256i d0, d1, bit_shift, shift, w0, tvn;
        __m128i ms_vec0 = _mm_setzero_si128();
        __m128i ms_vec1 = _mm_setzero_si128();
        int total_mn1, total_mn2;
        const __m256i ones = _mm256_set1_epi32(1);
        const __m256i twos = _mm256_set1_epi32(2);

        flags = _mm256_mullo_epi16(
            flags, _mm256_set_epi16(1, 1, 2, 2, 4, 4, 8, 8,
                                    1, 1, 2, 2, 4, 4, 8, 8));
        w0 = _mm256_srli_epi32(flags, 15);
        m_n = _mm256_sub_epi32(Uq, w0);
        m_n = _mm256_andnot_si256(insig, m_n);

        inc_sum = m_n;
        inc_sum = _mm256_add_epi32(inc_sum, _mm256_bslli_epi128(inc_sum, 4));
        inc_sum = _mm256_add_epi32(inc_sum, _mm256_bslli_epi128(inc_sum, 8));
        total_mn1 = _mm256_extract_epi16(inc_sum, 6);
        total_mn2 = _mm256_extract_epi16(inc_sum, 14);
        if (total_mn1) {
            ms_vec0 = jph_frwd_fetch_ff_avx2(magsgn);
            jph_frwd_advance_avx2(magsgn, (uint32_t)total_mn1);
        }
        if (total_mn2) {
            ms_vec1 = jph_frwd_fetch_ff_avx2(magsgn);
            jph_frwd_advance_avx2(magsgn, (uint32_t)total_mn2);
        }
        ms_vec = _mm256_inserti128_si256(_mm256_castsi128_si256(ms_vec0),
                                         ms_vec1, 1);
        ex_sum = _mm256_bslli_epi128(inc_sum, 4);

        byte_idx = _mm256_srli_epi32(ex_sum, 3);
        bit_idx = _mm256_and_si256(ex_sum, _mm256_set1_epi32(7));
        byte_idx = _mm256_shuffle_epi8(
            byte_idx,
            _mm256_set_epi32(0x0C0C0C0C, 0x08080808, 0x04040404, 0x00000000,
                             0x0C0C0C0C, 0x08080808, 0x04040404, 0x00000000));
        byte_idx = _mm256_add_epi32(byte_idx, _mm256_set1_epi32(0x03020100));
        d0 = _mm256_shuffle_epi8(ms_vec, byte_idx);
        byte_idx = _mm256_add_epi32(byte_idx, _mm256_set1_epi32(0x01010101));
        d1 = _mm256_shuffle_epi8(ms_vec, byte_idx);

        bit_idx = _mm256_or_si256(bit_idx, _mm256_slli_epi32(bit_idx, 16));
        {
            __m128i a = _mm_set_epi8(1, 3, 7, 15, 31, 63, 127, -1,
                                     1, 3, 7, 15, 31, 63, 127, -1);
            __m256i aa =
                _mm256_inserti128_si256(_mm256_castsi128_si256(a), a, 1);
            bit_shift = _mm256_shuffle_epi8(aa, bit_idx);
        }
        bit_shift = _mm256_add_epi16(bit_shift, _mm256_set1_epi16(0x0101));
        d0 = _mm256_mullo_epi16(d0, bit_shift);
        d0 = _mm256_srli_epi16(d0, 8);
        d1 = _mm256_mullo_epi16(d1, bit_shift);
        d1 = _mm256_and_si256(d1, _mm256_set1_epi32((int32_t)0xFF00FF00));
        d0 = _mm256_or_si256(d0, d1);

        {
            __m256i Uq_m1 = _mm256_sub_epi32(Uq, ones);
            Uq_m1 = _mm256_and_si256(
                Uq_m1, _mm256_set_epi32(0, 0, 0, 0x1F, 0, 0, 0, 0x1F));
            Uq_m1 = _mm256_shuffle_epi32(Uq_m1, 0);
            w0 = _mm256_sub_epi32(twos, w0);
            shift = _mm256_sllv_epi32(w0, Uq_m1);
        }
        ms_vec = _mm256_and_si256(d0, _mm256_sub_epi32(shift, ones));

        w0 = _mm256_and_si256(flags, _mm256_set1_epi32(0x800));
        w0 = _mm256_cmpeq_epi32(w0, _mm256_setzero_si256());
        w0 = _mm256_andnot_si256(w0, shift);
        ms_vec = _mm256_or_si256(ms_vec, w0);
        w0 = _mm256_slli_epi32(ms_vec, 31);
        ms_vec = _mm256_or_si256(ms_vec, ones);
        tvn = _mm256_andnot_si256(insig, ms_vec);
        _mm256_storeu_si256((__m256i *)vn_lane, tvn);
        bottom_vn[0] = vn_lane[1];
        bottom_vn[1] = vn_lane[3];
        bottom_vn[2] = vn_lane[5];
        bottom_vn[3] = vn_lane[7];

        ms_vec = _mm256_add_epi32(ms_vec, twos);
        ms_vec = _mm256_slli_epi32(ms_vec, (int)p - 1);
        ms_vec = _mm256_or_si256(ms_vec, w0);
        row = _mm256_andnot_si256(insig, ms_vec);
    }

    row = _mm256_permutevar8x32_epi32(
        row, _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7));
    _mm_storeu_si128((__m128i *)row0, _mm256_castsi256_si128(row));
    _mm_storeu_si128((__m128i *)row1, _mm256_extracti128_si256(row, 1));
}

/* ---------------------------------------------------------------------------
 * Forward reversible 5/3 1D lifting (int64). Mirrors the scalar
 * jph_forward_53_i64 exactly: deinterleave src into even/odd (ev/od scratch),
 * then predict (high) and update (low) with floor-div-by-2^s == arithmetic
 * shift. Output low[]/high[] are separate contiguous arrays (no scatter).
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
exr_result jph_forward_53_i64_avx2(const int64_t *src, size_t n, int64_t *low,
                                   size_t low_count, int64_t *high,
                                   size_t high_count, int64_t *ev, int64_t *od) {
    size_t nl = (n + 1u) / 2u, nh = n / 2u, i;
    __m256i two = _mm256_set1_epi64x(2);

    if (low_count != nl || high_count != nh) return EXR_ERROR_INVALID_ARGUMENT;
    if (n == 0) return EXR_SUCCESS;

    for (i = 0; i < nl; ++i) ev[i] = src[2u * i];
    for (i = 0; i < nh; ++i) od[i] = src[2u * i + 1u];

    /* predict: high[i] = od[i] - floor((ev[i] + ev[i+1]) / 2) */
    i = 0;
    for (; i + 4 <= nh && i + 4 < nl; i += 4) {
        __m256i e0 = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i e1 = _mm256_loadu_si256((const __m256i *)(ev + i + 1));
        __m256i o = _mm256_loadu_si256((const __m256i *)(od + i));
        __m256i q = jph_sra64x4(_mm256_add_epi64(e0, e1), 1);
        _mm256_storeu_si256((__m256i *)(high + i), _mm256_sub_epi64(o, q));
    }
    for (; i < nh; ++i) {
        int64_t e0 = ev[i];
        int64_t e1 = (i + 1u < nl) ? ev[i + 1] : e0;
        high[i] = od[i] - jph_sra64_ref(e0 + e1, 1);
    }

    /* update: low[i] = ev[i] + floor((high[i-1] + high[i] + 2) / 4) */
    if (nh == 0) {
        for (i = 0; i < nl; ++i) low[i] = ev[i]; /* +floor(2/4)=+0 */
        return EXR_SUCCESS;
    }
    low[0] = ev[0] + jph_sra64_ref((int64_t)high[0] + (int64_t)high[0] + 2, 2);
    i = 1;
    for (; i + 4 <= nh; i += 4) {
        __m256i hl = _mm256_loadu_si256((const __m256i *)(high + i - 1));
        __m256i hr = _mm256_loadu_si256((const __m256i *)(high + i));
        __m256i ee = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i q = jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(hl, hr), two), 2);
        _mm256_storeu_si256((__m256i *)(low + i), _mm256_add_epi64(ee, q));
    }
    for (; i < nl; ++i) {
        int64_t dl = high[i - 1];
        int64_t dr = (i < nh) ? high[i] : high[nh - 1];
        low[i] = ev[i] + jph_sra64_ref(dl + dr + 2, 2);
    }
    return EXR_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * Vertical (column) forward reversible 5/3, int64, row-wise across all columns.
 * Bit-identical to the scalar jph_forward_53_vert_i64. Reads data's interleaved
 * rows (stride width) -> subband layout in temp (stride rw): lh low-rows then hh
 * high-rows. Columns are the SIMD axis: each lifting step is a contiguous 4-lane
 * int64 vector op over the row -- no gather/scatter. data and temp are distinct.
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
EXR_NO_SANITIZE_SIO
exr_result jph_forward_53_vert_i64_avx2(const int64_t *data, size_t width,
                                        size_t rw, size_t lh, size_t hh,
                                        int64_t *temp) {
    size_t i, c;
    __m256i two = _mm256_set1_epi64x(2);

    /* ---- Phase 1: high rows -> temp[(lh+i)*rw] ---- */
    for (i = 0u; i < hh; ++i) {
        const int64_t *e0 = data + (2u * i) * width;
        const int64_t *e1 = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        const int64_t *od = data + (2u * i + 1u) * width;
        int64_t *hd = temp + (lh + i) * rw;
        for (c = 0u; c + 4u <= rw; c += 4u) {
            __m256i a = _mm256_loadu_si256((const __m256i *)(e0 + c));
            __m256i b = _mm256_loadu_si256((const __m256i *)(e1 + c));
            __m256i o = _mm256_loadu_si256((const __m256i *)(od + c));
            __m256i q = jph_sra64x4(_mm256_add_epi64(a, b), 1);
            _mm256_storeu_si256((__m256i *)(hd + c), _mm256_sub_epi64(o, q));
        }
        for (; c < rw; ++c)
            hd[c] = od[c] - jph_sra64_ref(e0[c] + e1[c], 1);
    }

    /* ---- Phase 2: low rows -> temp[i*rw] ---- */
    for (i = 0u; i < lh; ++i) {
        const int64_t *e0 = data + (2u * i) * width;
        int64_t *ld = temp + i * rw;
        if (hh == 0u) {
            for (c = 0u; c < rw; ++c) ld[c] = e0[c];
            continue;
        }
        {
            const int64_t *hl = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
            const int64_t *hr = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
            for (c = 0u; c + 4u <= rw; c += 4u) {
                __m256i a = _mm256_loadu_si256((const __m256i *)(e0 + c));
                __m256i l = _mm256_loadu_si256((const __m256i *)(hl + c));
                __m256i r = _mm256_loadu_si256((const __m256i *)(hr + c));
                __m256i q =
                    jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(l, r), two), 2);
                _mm256_storeu_si256((__m256i *)(ld + c), _mm256_add_epi64(a, q));
            }
            for (; c < rw; ++c)
                ld[c] = e0[c] + jph_sra64_ref(hl[c] + hr[c] + 2, 2);
        }
    }
    return EXR_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * Forward reversible 5/3 1D lifting (int32 -> int64 intermediates -> int32).
 * Bit-identical to jph_forward_53_i32. Deinterleaves int32 src into int64 ev/od,
 * then predict (high) and update (low) with floor-div-by-2^s. Outputs int32
 * low/high. ev/od scratch must be >= ceil(n/2) int64 each.
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
exr_result jph_forward_53_i32_avx2(const int32_t *src, size_t n, int32_t *low,
                                    size_t low_count, int32_t *high,
                                    size_t high_count, int64_t *ev, int64_t *od) {
    size_t nl = (n + 1u) / 2u, nh = n / 2u, i;
    __m256i two = _mm256_set1_epi64x(2);

    if (low_count != nl || high_count != nh) return EXR_ERROR_INVALID_ARGUMENT;
    if (n == 0) return EXR_SUCCESS;

    /* Deinterleave int32 src -> int64 ev/od.
     * src[2i..2i+7] = [ev0, od0, ev1, od1, ev2, od2, ev3, od3].
     * Load 8 int32, permute even lanes (0,2,4,6) to ev and odd (1,3,5,7) to od,
     * then sign-extend to int64. */
    for (i = 0; i + 4 <= nl; i += 4) {
        __m256i v = _mm256_loadu_si256((const __m256i *)(src + 2u * i));
        __m128i ev32 = _mm256_castsi256_si128(
            _mm256_permutevar8x32_epi32(v,
                _mm256_setr_epi32(0, 2, 4, 6, 0, 0, 0, 0)));
        _mm256_storeu_si256((__m256i *)(ev + i),
            _mm256_cvtepi32_epi64(ev32));
        if (i < nh) {
            __m128i od32 = _mm256_castsi256_si128(
                _mm256_permutevar8x32_epi32(v,
                    _mm256_setr_epi32(1, 3, 5, 7, 0, 0, 0, 0)));
            _mm256_storeu_si256((__m256i *)(od + i),
                _mm256_cvtepi32_epi64(od32));
        }
    }
    for (; i < nl; ++i) {
        ev[i] = (int64_t)src[2u * i];
        if (i < nh) od[i] = (int64_t)src[2u * i + 1u];
    }

    /* predict: high[i] = od[i] - floor((ev[i] + ev[i+1]) / 2) */
    i = 0;
    for (; i + 4 <= nh && i + 4 < nl; i += 4) {
        __m256i e0 = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i e1 = _mm256_loadu_si256((const __m256i *)(ev + i + 1));
        __m256i o = _mm256_loadu_si256((const __m256i *)(od + i));
        __m256i q = jph_sra64x4(_mm256_add_epi64(e0, e1), 1);
        __m256i hv = _mm256_sub_epi64(o, q);
        /* narrow int64 -> int32 and store */
        __m128i hv32 = jph_narrow_i64x4(hv);
        _mm_storeu_si128((__m128i *)(high + i), hv32);
    }
    for (; i < nh; ++i) {
        int64_t e0 = ev[i];
        int64_t e1 = (i + 1u < nl) ? ev[i + 1] : e0;
        high[i] = (int32_t)(od[i] - jph_sra64_ref(e0 + e1, 1));
    }

    /* update: low[i] = ev[i] + floor((high[i-1] + high[i] + 2) / 4) */
    if (nh == 0) {
        for (i = 0; i < nl; ++i) low[i] = (int32_t)ev[i];
        return EXR_SUCCESS;
    }
    low[0] = (int32_t)(ev[0] + jph_sra64_ref((int64_t)high[0] + (int64_t)high[0] + 2, 2));
    i = 1;
    for (; i + 4 <= nh; i += 4) {
        __m256i hl = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(high + i - 1)));
        __m256i hr = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(high + i)));
        __m256i ee = _mm256_loadu_si256((const __m256i *)(ev + i));
        __m256i q = jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(hl, hr), two), 2);
        __m256i lv = _mm256_add_epi64(ee, q);
        __m128i lv32 = jph_narrow_i64x4(lv);
        _mm_storeu_si128((__m128i *)(low + i), lv32);
    }
    for (; i < nl; ++i) {
        int64_t dl = (int64_t)high[i - 1];
        int64_t dr = (i < nh) ? (int64_t)high[i] : (int64_t)high[nh - 1];
        low[i] = (int32_t)(ev[i] + jph_sra64_ref(dl + dr + 2, 2));
    }
    return EXR_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * Vertical (column) forward reversible 5/3, int32, row-wise across all columns.
 * Bit-identical to jph_forward_53_vert_i32. Reads data's interleaved rows
 * (stride width) -> subband layout in temp (stride rw): lh low-rows then hh
 * high-rows. Columns are the SIMD axis: each lifting step loads contiguous
 * int32 data from even/odd rows and computes in int64, storing int32 results.
 * data and temp are distinct buffers.
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
exr_result jph_forward_53_vert_i32_avx2(const int32_t *data, size_t width,
                                        size_t rw, size_t lh, size_t hh,
                                        int32_t *temp) {
    size_t i, c;
    __m256i two = _mm256_set1_epi64x(2);

    /* ---- Phase 1: high rows -> temp[(lh+i)*rw] ---- */
    for (i = 0u; i < hh; ++i) {
        const int32_t *e0 = data + (2u * i) * width;
        const int32_t *e1 = data + (2u * (i + 1u < lh ? i + 1u : i)) * width;
        const int32_t *od = data + (2u * i + 1u) * width;
        int32_t *hd = temp + (lh + i) * rw;
        for (c = 0u; c + 8u <= rw; c += 8u) {
            __m256i a = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(e0 + c)));
            __m256i b = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(e1 + c)));
            __m256i o = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(od + c)));
            __m256i q = jph_sra64x4(_mm256_add_epi64(a, b), 1);
            __m256i hv = _mm256_sub_epi64(o, q);
            /* narrow first 4 -> store, then next 4 */
            __m128i lo32 = jph_narrow_i64x4(hv);
            /* second 4-wide int64 */
            __m256i a2 = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(e0 + c + 4)));
            __m256i b2 = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(e1 + c + 4)));
            __m256i o2 = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(od + c + 4)));
            __m256i q2 = jph_sra64x4(_mm256_add_epi64(a2, b2), 1);
            __m256i hv2 = _mm256_sub_epi64(o2, q2);
            __m128i hi32 = jph_narrow_i64x4(hv2);
            _mm_storeu_si128((__m128i *)(hd + c), lo32);
            _mm_storeu_si128((__m128i *)(hd + c + 4), hi32);
        }
        for (; c < rw; ++c)
            hd[c] = (int32_t)((int64_t)od[c] -
                              jph_sra64_ref((int64_t)e0[c] + (int64_t)e1[c], 1));
    }

    /* ---- Phase 2: low rows -> temp[i*rw] ---- */
    for (i = 0u; i < lh; ++i) {
        const int32_t *e0 = data + (2u * i) * width;
        int32_t *ld = temp + i * rw;
        if (hh == 0u) {
            for (c = 0u; c < rw; ++c) ld[c] = e0[c];
            continue;
        }
        {
            const int32_t *hl = temp + (lh + (i > 0u ? i - 1u : 0u)) * rw;
            const int32_t *hr = temp + (lh + (i < hh ? i : hh - 1u)) * rw;
            for (c = 0u; c + 8u <= rw; c += 8u) {
                __m256i a = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(e0 + c)));
                __m256i l = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(hl + c)));
                __m256i r = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(hr + c)));
                __m256i q = jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(l, r), two), 2);
                __m256i lv = _mm256_add_epi64(a, q);
                __m128i lo32 = jph_narrow_i64x4(lv);
                /* second 4-wide */
                __m256i a2 = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(e0 + c + 4)));
                __m256i l2 = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(hl + c + 4)));
                __m256i r2 = _mm256_cvtepi32_epi64(_mm_loadu_si128((const __m128i *)(hr + c + 4)));
                __m256i q2 = jph_sra64x4(_mm256_add_epi64(_mm256_add_epi64(l2, r2), two), 2);
                __m256i lv2 = _mm256_add_epi64(a2, q2);
                __m128i hi32 = jph_narrow_i64x4(lv2);
                _mm_storeu_si128((__m128i *)(ld + c), lo32);
                _mm_storeu_si128((__m128i *)(ld + c + 4), hi32);
            }
            for (; c < rw; ++c)
                ld[c] = (int32_t)((int64_t)e0[c] +
                                  jph_sra64_ref((int64_t)hl[c] + (int64_t)hr[c] + 2, 2));
        }
    }
    return EXR_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * SSE2 int32 quad sample preparation for the HT encode block.
 * Processes 4 samples of a 2x2 quad at once: load, abs, sign-magnitude shift,
 * val computation, float-based CLZ, e_q/s extraction.
 *
 * The 4 samples are at plane positions: (x, y), (x, y+1), (x+1, y), (x+1, y+1)
 * where (x,y) is the quad's top-left in codeblock-relative coordinates and
 * (x0,y0) is the codeblock's plane offset. plane_data holds int64 elements
 * whose low 32 bits are valid int32 sample values.
 * ------------------------------------------------------------------------- */
/* Shared body: given the quad's 4 samples in lane order [s0,s1,s2,s3]
 * (= (x,y),(x,y+1),(x+1,y),(x+1,y+1)), compute rho/e_q/s/e_qmax/max_val. The
 * int64- and int32-reading entry points below differ only in how they load and
 * assemble `samples`. */
EXR_TARGET("sse2")
static inline void jph_prep_quad_body_sse2(__m128i samples, uint32_t shift,
                                           uint32_t p, int *rho, int *e_qmax,
                                           int e_q[4], uint64_t s[4],
                                           uint64_t *max_val) {
    __m128i sign_mask, abs_v;
    __m128i sign32, shifted, t_val, val, val_m1, is_nz;
    __m128 fv;
    __m128i fv_bits, exp_bits, eq_simd, s_val_simd;
    __m128i bit_patterns, rho_bits, rho_sum;
    __m128i eq_swap, cmp, eq_pair, eq_pair_swap, eq_final;
    int eq_max_scalar;
    uint32_t s_tmp[4];
    uint64_t abs_scalar;
    int i;

    /* Track max absolute value (scalar extraction after SIMD compute) */
    sign_mask = _mm_srai_epi32(samples, 31);
    abs_v = _mm_sub_epi32(_mm_xor_si128(samples, sign_mask), sign_mask);
    _mm_storeu_si128((__m128i *)s_tmp, abs_v);
    if (max_val) {
        abs_scalar = (uint64_t)s_tmp[0];
        if (abs_scalar > *max_val) *max_val = abs_scalar;
        abs_scalar = (uint64_t)s_tmp[1];
        if (abs_scalar > *max_val) *max_val = abs_scalar;
        abs_scalar = (uint64_t)s_tmp[2];
        if (abs_scalar > *max_val) *max_val = abs_scalar;
        abs_scalar = (uint64_t)s_tmp[3];
        if (abs_scalar > *max_val) *max_val = abs_scalar;
    }

    /* Build sign-magnitude words:  sign32 = 0x80000000 if sample < 0 */
    sign32 = _mm_and_si128(sign_mask, _mm_set1_epi32((int)0x80000000u));
    shifted = _mm_slli_epi32(abs_v, (int)shift);
    t_val = _mm_or_si128(sign32, shifted);

    /* Compute encoded significance:  val = (t+t) >> p, mask LSB */
    val = _mm_add_epi32(t_val, t_val);
    val = _mm_srli_epi32(val, (int)p);
    val = _mm_and_si128(val, _mm_set1_epi32((int)~1u));

    /* Non-zero mask: -1 where val != 0 */
    is_nz = _mm_cmpeq_epi32(val, _mm_setzero_si128());
    is_nz = _mm_xor_si128(is_nz, _mm_set1_epi32(-1));

    /* CLZ via float exponent trick: eq = exponent - 126, zero for val==0 */
    val_m1 = _mm_sub_epi32(val, _mm_set1_epi32(1));
    fv = _mm_cvtepi32_ps(val_m1);
    fv_bits = _mm_castps_si128(fv);
    exp_bits = _mm_srli_epi32(fv_bits, 23);
    eq_simd = _mm_sub_epi32(exp_bits, _mm_set1_epi32(126));
    eq_simd = _mm_and_si128(eq_simd, is_nz);

    /* Store e_q[0..3] */
    _mm_storeu_si128((__m128i *)e_q, eq_simd);

    /* Compute s values:  s = (val - 2) + (t >> 31) */
    {
        __m128i t_sign_bit;
        __m128i s_base;
        t_sign_bit = _mm_srli_epi32(
            _mm_and_si128(sign_mask, _mm_set1_epi32((int)0x80000000u)), 31);
        s_base = _mm_sub_epi32(val_m1, _mm_set1_epi32(1));
        s_val_simd = _mm_add_epi32(s_base, t_sign_bit);
    }
    /* Zero s for insignificant lanes (val==0), matching the scalar prepare
     * (which sets s=0 when val==0). */
    s_val_simd = _mm_and_si128(s_val_simd, is_nz);
    _mm_storeu_si128((__m128i *)s_tmp, s_val_simd);
    for (i = 0; i < 4; ++i) s[i] = (uint64_t)s_tmp[i];

    /* Build rho bits: OR the per-sample bit flags (1,2,4,8) where not zero */
    bit_patterns = _mm_setr_epi32(1, 2, 4, 8);
    rho_bits = _mm_and_si128(bit_patterns, is_nz);
    rho_sum = _mm_add_epi32(rho_bits, _mm_srli_si128(rho_bits, 4));
    rho_sum = _mm_add_epi32(rho_sum, _mm_srli_si128(rho_sum, 8));
    *rho |= _mm_cvtsi128_si32(rho_sum);

    /* Update e_qmax: horizontal max of the 4 exponent values.
     * eq_pair = [max(eq0,eq2), max(eq1,eq3), max(eq0,eq2), max(eq1,eq3)].
     * The second shuffle must bring the *other* pair-max into lane 0 so the
     * final max combines both pairs; _MM_SHUFFLE(2,3,0,1) puts eq_pair[1] in
     * lane 0 (the previous (1,0,1,0) left eq_pair[0] there, dropping eq1/eq3). */
    eq_swap = _mm_shuffle_epi32(eq_simd, _MM_SHUFFLE(1, 0, 3, 2));
    cmp = _mm_cmpgt_epi32(eq_simd, eq_swap);
    eq_pair = _mm_add_epi32(
        _mm_and_si128(cmp, eq_simd),
        _mm_andnot_si128(cmp, eq_swap));
    eq_pair_swap = _mm_shuffle_epi32(eq_pair, _MM_SHUFFLE(2, 3, 0, 1));
    cmp = _mm_cmpgt_epi32(eq_pair, eq_pair_swap);
    eq_final = _mm_add_epi32(
        _mm_and_si128(cmp, eq_pair),
        _mm_andnot_si128(cmp, eq_pair_swap));
    eq_max_scalar = _mm_cvtsi128_si32(eq_final);
    if (eq_max_scalar > *e_qmax) *e_qmax = eq_max_scalar;
}

/* int64-plane entry: low 32 bits of each int64 element are the int32 sample. */
EXR_TARGET("sse2")
void jph_encode_prepare_quad_i32_sse2(const int64_t *plane_data,
                                       uint32_t stride, uint32_t x0,
                                       uint32_t y0, uint32_t x, uint32_t y,
                                       uint32_t shift, uint32_t p,
                                       int *rho, int *e_qmax, int e_q[4],
                                       uint64_t s[4], uint64_t *max_val) {
    __m128i row0, row1, r0_lo, r1_lo, combined, samples;
    /* Load 2 int64 each from rows y and y+1, extract low 32 bits */
    row0 = _mm_loadu_si128((const __m128i *)(plane_data + (y0 + y) * stride + (x0 + x)));
    row1 = _mm_loadu_si128((const __m128i *)(plane_data + (y0 + y + 1u) * stride + (x0 + x)));
    /* Gather the low 32 bits (the int32 sample) of each int64 into the low
     * lanes: _MM_SHUFFLE(2,0,2,0) -> lane0=row[0], lane1=row[2] = [s0,s2,..]. */
    r0_lo = _mm_shuffle_epi32(row0, _MM_SHUFFLE(2, 0, 2, 0));
    r1_lo = _mm_shuffle_epi32(row1, _MM_SHUFFLE(2, 0, 2, 0));
    combined = _mm_unpacklo_epi64(r0_lo, r1_lo);      /* [s0, s2, s1, s3] */
    samples = _mm_shuffle_epi32(combined, _MM_SHUFFLE(3, 1, 2, 0)); /* [s0,s1,s2,s3] */
    jph_prep_quad_body_sse2(samples, shift, p, rho, e_qmax, e_q, s, max_val);
}

/* int32-plane entry (all-HALF native path): each element is the int32 sample.
 * Loads 2 int32 from row y and 2 from row y+1, interleaving to [s0,s1,s2,s3]. */
EXR_TARGET("sse2")
void jph_encode_prepare_quad_from32_sse2(const int32_t *plane_data,
                                          uint32_t stride, uint32_t x0,
                                          uint32_t y0, uint32_t x, uint32_t y,
                                          uint32_t shift, uint32_t p,
                                          int *rho, int *e_qmax, int e_q[4],
                                          uint64_t s[4], uint64_t *max_val) {
    __m128i r0, r1, samples;
    /* r0 = [(x,y),(x+1,y)], r1 = [(x,y+1),(x+1,y+1)] in the low 64 bits */
    r0 = _mm_loadl_epi64((const __m128i *)(plane_data + (y0 + y) * stride + (x0 + x)));
    r1 = _mm_loadl_epi64((const __m128i *)(plane_data + (y0 + y + 1u) * stride + (x0 + x)));
    /* unpacklo_epi32 -> [(x,y),(x,y+1),(x+1,y),(x+1,y+1)] = [s0,s1,s2,s3] */
    samples = _mm_unpacklo_epi32(r0, r1);
    jph_prep_quad_body_sse2(samples, shift, p, rho, e_qmax, e_q, s, max_val);
}

/* ---------------------------------------------------------------------------
 * AVX2 8-quad sample preparation (proc_pixel) for the HT encode block.
 *
 * Computes the per-sample bookkeeping for 8 quads (16 columns x 2 rows) at once,
 * mirroring jph_prep_quad_body_sse2's exact math (shift-based sign-magnitude
 * build, (t+t)>>p significance, float-CLZ exponent, s = (val-2)+sign) but laid
 * out 8-wide. The caller passes a zero-padded contiguous tile so out-of-bounds
 * columns/rows become insignificant (val==0 -> e_q=0, s=0, rho bit clear),
 * exactly matching the scalar prepare's zeroing of edge samples.
 *
 *   tile[ 0..15] = row y    columns 0..15  (codeblock-relative)
 *   tile[16..31] = row y+1  columns 0..15
 *
 * Outputs (sample-major: index [k*8 + q] is sample k of quad q, q=0..7):
 *   eq[32]     e_q exponents          rho8[8]   per-quad significance bits
 *   sarr[32]   s (v_n) values         eqmax8[8] per-quad max exponent
 * and folds the maximum |coefficient| into *max_val. Sample order within a quad
 * is [s0,s1,s2,s3] = (col,row),(col,row+1),(col+1,row),(col+1,row+1).
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
void jph_enc_proc_pixel_8q_avx2(const int32_t *tile, uint32_t shift, uint32_t p,
                                int32_t eq[32], int32_t sarr[32],
                                int32_t rho8[8], int32_t eqmax8[8],
                                uint64_t *max_val) {
    const __m256i zero = _mm256_setzero_si256();
    const __m256i one = _mm256_set1_epi32(1);
    const __m256i signbit = _mm256_set1_epi32((int)0x80000000u);
    const __m256i notone = _mm256_set1_epi32((int)~1u);
    __m256i src[4], _eq[4], _s[4], _rho[4], absmax = zero;
    int i;

    /* src[0]=row0 cols0-7, src[1]=row1 cols0-7, src[2]=row0 cols8-15,
     * src[3]=row1 cols8-15 (matches OpenJPH proc_pixel src_vec layout). */
    src[0] = _mm256_loadu_si256((const __m256i *)(tile + 0));
    src[2] = _mm256_loadu_si256((const __m256i *)(tile + 8));
    src[1] = _mm256_loadu_si256((const __m256i *)(tile + 16));
    src[3] = _mm256_loadu_si256((const __m256i *)(tile + 24));

    for (i = 0; i < 4; ++i) {
        __m256i sm = _mm256_srai_epi32(src[i], 31);          /* -1 if neg */
        __m256i av = _mm256_sub_epi32(_mm256_xor_si256(src[i], sm), sm);
        __m256i s32 = _mm256_and_si256(sm, signbit);
        __m256i t = _mm256_or_si256(s32, _mm256_sllv_epi32(av,
                        _mm256_set1_epi32((int)shift)));
        __m256i val = _mm256_and_si256(
            _mm256_srlv_epi32(_mm256_add_epi32(t, t),
                              _mm256_set1_epi32((int)p)), notone);
        __m256i nz = _mm256_xor_si256(_mm256_cmpeq_epi32(val, zero),
                                      _mm256_set1_epi32(-1));
        __m256i vm1 = _mm256_sub_epi32(val, one);
        /* e_q = exponent(float(val-1)) - 126, zeroed where val==0 */
        __m256i fb = _mm256_castps_si256(_mm256_cvtepi32_ps(vm1));
        __m256i e = _mm256_sub_epi32(_mm256_srli_epi32(fb, 23),
                                     _mm256_set1_epi32(126));
        _eq[i] = _mm256_and_si256(e, nz);
        /* s = (val-2) + signbit, zeroed where val==0 */
        __m256i sv = _mm256_add_epi32(_mm256_sub_epi32(vm1, one),
                                      _mm256_srli_epi32(s32, 31));
        _s[i] = _mm256_and_si256(sv, nz);
        _rho[i] = _mm256_srli_epi32(nz, 31);                 /* 1 where nz */
        absmax = _mm256_max_epi32(absmax, av);
    }

    /* Quad-deinterleave: reorder even/odd columns into quad-major lanes so each
     * output vector holds one sample position across quads 0..7. */
    {
        const __m256i idx = _mm256_set_epi32(7, 5, 3, 1, 6, 4, 2, 0);
        __m256i t1, t2;
        for (i = 0; i < 2; ++i) {
            t1 = _mm256_permutevar8x32_epi32(_eq[0 + i], idx);
            t2 = _mm256_permutevar8x32_epi32(_eq[2 + i], idx);
            _mm256_storeu_si256((__m256i *)(eq + (0 + i) * 8),
                                _mm256_permute2x128_si256(t1, t2, 0x20));
            _mm256_storeu_si256((__m256i *)(eq + (2 + i) * 8),
                                _mm256_permute2x128_si256(t1, t2, 0x31));
            t1 = _mm256_permutevar8x32_epi32(_s[0 + i], idx);
            t2 = _mm256_permutevar8x32_epi32(_s[2 + i], idx);
            _mm256_storeu_si256((__m256i *)(sarr + (0 + i) * 8),
                                _mm256_permute2x128_si256(t1, t2, 0x20));
            _mm256_storeu_si256((__m256i *)(sarr + (2 + i) * 8),
                                _mm256_permute2x128_si256(t1, t2, 0x31));
            t1 = _mm256_permutevar8x32_epi32(_rho[0 + i], idx);
            t2 = _mm256_permutevar8x32_epi32(_rho[2 + i], idx);
            _rho[0 + i] = _mm256_permute2x128_si256(t1, t2, 0x20);
            _rho[2 + i] = _mm256_permute2x128_si256(t1, t2, 0x31);
        }
    }

    /* rho = bit0 | bit1<<1 | bit2<<2 | bit3<<3 ; e_qmax = max over 4 samples */
    {
        __m256i r = _mm256_or_si256(
            _mm256_or_si256(_rho[0], _mm256_slli_epi32(_rho[1], 1)),
            _mm256_or_si256(_mm256_slli_epi32(_rho[2], 2),
                            _mm256_slli_epi32(_rho[3], 3)));
        __m256i em = _mm256_loadu_si256((const __m256i *)(eq + 0));
        em = _mm256_max_epi32(em, _mm256_loadu_si256((const __m256i *)(eq + 8)));
        em = _mm256_max_epi32(em, _mm256_loadu_si256((const __m256i *)(eq + 16)));
        em = _mm256_max_epi32(em, _mm256_loadu_si256((const __m256i *)(eq + 24)));
        _mm256_storeu_si256((__m256i *)rho8, r);
        _mm256_storeu_si256((__m256i *)eqmax8, em);
    }

    /* Fold horizontal max of |coefficient| into *max_val. */
    if (max_val) {
        __m128i m = _mm_max_epi32(_mm256_castsi256_si128(absmax),
                                  _mm256_extracti128_si256(absmax, 1));
        m = _mm_max_epi32(m, _mm_shuffle_epi32(m, _MM_SHUFFLE(1, 0, 3, 2)));
        m = _mm_max_epi32(m, _mm_shuffle_epi32(m, _MM_SHUFFLE(2, 3, 0, 1)));
        uint64_t mv = (uint64_t)(uint32_t)_mm_cvtsi128_si32(m);
        if (mv > *max_val) *max_val = mv;
    }
}

/* ---------------------------------------------------------------------------
 * AVX2 8-quad context kernel for the HT encode block (Stage B).
 *
 * Vectorizes the per-quad bookkeeping that the scalar encoder threads serially:
 * the significance context c_q, the eps mask, kappa, U_q and u_q, plus the E
 * (e_line) and CX (cx_line) line-state updates. Computes all 8 quads of one
 * 16-column group at once and returns cq8/eps8/uq8/Uq8 for the serial emit.
 *
 * Line-state is a FLAT int32 array indexed by quad position (xv*8 + q): the
 * cross-vector neighbour reads needed by max_e and proc_cq2 are plain unaligned
 * loads (e_line[xv*8+1], cx_line[xv*8+1/+2]); only the single-element carries
 * (prev_cq/prev_e/prev_cx) cross the boundary. Read-before-write is preserved:
 * max_e and the proc_cq2 cx terms read the previous row's line-state before
 * update_lep / update_lcxp overwrite it.  initial!=0 selects the y=0 row
 * (proc_cq1, kappa==1); otherwise the main rows (proc_cq2, max_e-derived kappa).
 * Bit-identical to the scalar bookkeeping. e_line/cx_line must be zero-init at
 * codeblock start and sized for n_vec*8 + a few slack lanes.
 * ------------------------------------------------------------------------- */
EXR_TARGET("avx2")
void jph_enc_context_8q_avx2(int initial, const int32_t eq[32],
                             const int32_t rho8[8], const int32_t eqmax8[8],
                             int32_t *e_line, int32_t *cx_line, uint32_t xv,
                             int *prev_cq, int *prev_e, int *prev_cx,
                             int32_t cq8[8], int32_t eps8[8], int32_t uq8[8],
                             int32_t Uq8[8]) {
    const __m256i zero = _mm256_setzero_si256();
    const __m256i one = _mm256_set1_epi32(1);
    const __m256i left_shift = _mm256_set_epi32(6, 5, 4, 3, 2, 1, 0, 7);
    __m256i rho = _mm256_loadu_si256((const __m256i *)rho8);
    __m256i eqmax = _mm256_loadu_si256((const __m256i *)eqmax8);
    __m256i e1 = _mm256_loadu_si256((const __m256i *)(eq + 8));   /* sample 1 */
    __m256i e3 = _mm256_loadu_si256((const __m256i *)(eq + 24));  /* sample 3 */
    __m256i kappa, Uq, uq, tmp;

    /* kappa: 1 in the initial row; else (rho not a single bit) ? max(max_e,1):1,
     * max_e = max(e_line[P], e_line[P+1]) - 1 read from the previous row. */
    if (initial) {
        kappa = one;
    } else {
        __m256i v0 = _mm256_loadu_si256((const __m256i *)(e_line + xv * 8));
        __m256i v1 = _mm256_loadu_si256((const __m256i *)(e_line + xv * 8 + 1));
        __m256i maxe = _mm256_sub_epi32(_mm256_max_epi32(v0, v1), one);
        __m256i t = _mm256_and_si256(rho, _mm256_sub_epi32(rho, one));
        __m256i ispow2 = _mm256_cmpeq_epi32(t, zero);
        __m256i kelse = _mm256_max_epi32(maxe, one);
        kappa = _mm256_or_si256(_mm256_and_si256(ispow2, one),
                                _mm256_andnot_si256(ispow2, kelse));
    }
    Uq = _mm256_max_epi32(kappa, eqmax);
    uq = _mm256_sub_epi32(Uq, kappa);

    /* eps = (u_q>0) ? OR_k (e_q[k]==e_qmax)<<k : 0 */
    {
        __m256i uqmask = _mm256_cmpgt_epi32(uq, zero);
        __m256i e0 = _mm256_loadu_si256((const __m256i *)(eq + 0));
        __m256i e2 = _mm256_loadu_si256((const __m256i *)(eq + 16));
        __m256i eps = _mm256_srli_epi32(_mm256_cmpeq_epi32(e0, eqmax), 31);
        eps = _mm256_or_si256(eps, _mm256_slli_epi32(
            _mm256_srli_epi32(_mm256_cmpeq_epi32(e1, eqmax), 31), 1));
        eps = _mm256_or_si256(eps, _mm256_slli_epi32(
            _mm256_srli_epi32(_mm256_cmpeq_epi32(e2, eqmax), 31), 2));
        eps = _mm256_or_si256(eps, _mm256_slli_epi32(
            _mm256_srli_epi32(_mm256_cmpeq_epi32(e3, eqmax), 31), 3));
        eps = _mm256_and_si256(eps, uqmask);
        _mm256_storeu_si256((__m256i *)eps8, eps);
    }

    /* c_q: initial -> (rho>>1)|(rho&1) of the previous quad; main -> proc_cq2.
     * tmp[q] is the value that becomes c_q for quad q+1; shift right one lane and
     * insert prev_cq for lane 0. proc_cq2 reads the previous row's cx_line. */
    if (initial) {
        tmp = _mm256_or_si256(_mm256_srli_epi32(rho, 1),
                              _mm256_and_si256(rho, one));
    } else {
        __m256i c1 = _mm256_loadu_si256((const __m256i *)(cx_line + xv * 8 + 1));
        __m256i c2 = _mm256_loadu_si256((const __m256i *)(cx_line + xv * 8 + 2));
        __m256i base = _mm256_add_epi32(c1, _mm256_slli_epi32(c2, 2));
        __m256i rt = _mm256_or_si256(
            _mm256_srli_epi32(_mm256_and_si256(rho, _mm256_set1_epi32(4)), 1),
            _mm256_srli_epi32(_mm256_and_si256(rho, _mm256_set1_epi32(8)), 2));
        tmp = _mm256_or_si256(base, rt);
    }
    {
        __m256i cq = _mm256_permutevar8x32_epi32(tmp, left_shift);
        cq = _mm256_insert_epi32(cq, *prev_cq, 0);
        *prev_cq = _mm256_extract_epi32(tmp, 7);
        _mm256_storeu_si256((__m256i *)cq8, cq);
    }

    /* update_lep: e_line[P] = max(e_q1[q], e_q3[q-1]); carry prev_e = e_q3[7]. */
    {
        __m256i e3s = _mm256_permutevar8x32_epi32(e3, left_shift);
        e3s = _mm256_insert_epi32(e3s, *prev_e, 0);
        *prev_e = _mm256_extract_epi32(e3, 7);
        _mm256_storeu_si256((__m256i *)(e_line + xv * 8),
                            _mm256_max_epi32(e1, e3s));
    }
    /* update_lcxp: cx_line[P] = ((rho[q-1]&8)>>3) | ((rho[q]&2)>>1). */
    {
        __m256i rs = _mm256_permutevar8x32_epi32(rho, left_shift);
        rs = _mm256_insert_epi32(rs, *prev_cx, 0);
        *prev_cx = _mm256_extract_epi32(rho, 7);
        __m256i a = _mm256_srli_epi32(_mm256_and_si256(rs, _mm256_set1_epi32(8)),
                                      3);
        __m256i b = _mm256_srli_epi32(_mm256_and_si256(rho, _mm256_set1_epi32(2)),
                                      1);
        _mm256_storeu_si256((__m256i *)(cx_line + xv * 8),
                            _mm256_or_si256(a, b));
    }

    _mm256_storeu_si256((__m256i *)uq8, uq);
    _mm256_storeu_si256((__m256i *)Uq8, Uq);
}

/* ---------------------------------------------------------------------------
 * AVX2 8-quad MagSgn-prep kernel (Stage C).
 *
 * Vectorizes the magnitude-emit preparation that dominated the scalar encoder
 * (jph_encode_mag_bits_quad): for all 8 quads computes, per sample k (bit 1<<k),
 *   m_n = (rho & bit) ? U_q - ((tuple & bit) ? 1 : 0) : 0
 *   cwd_s = s & ((1u << m_n) - 1)
 * leaving only the (inherently serial) pair assembly + MagSgn bit-append to the
 * caller. Sample-major output: index [k*8 + q]. i32 path only (m_n <= kmax <= 30,
 * so the per-sample mask fits 32 bits). tuple8 holds the raw VLC table entries
 * (low 4 bits select the magnitude bits). Bit-identical to the scalar prep. */
EXR_TARGET("avx2")
void jph_enc_ms_prep_8q_avx2(const int32_t rho8[8], const int32_t Uq8[8],
                             const int32_t tuple8[8], const int32_t sarr[32],
                             int32_t m_n[32], int32_t cwd_s[32]) {
    const __m256i one = _mm256_set1_epi32(1);
    __m256i rho = _mm256_loadu_si256((const __m256i *)rho8);
    __m256i Uq = _mm256_loadu_si256((const __m256i *)Uq8);
    __m256i tuple = _mm256_loadu_si256((const __m256i *)tuple8);
    int k;
    for (k = 0; k < 4; ++k) {
        __m256i bit = _mm256_set1_epi32(1 << k);
        /* tb = (tuple & bit) ? 1 : 0 ; m = U_q - tb, then masked to 0 where the
         * rho bit is clear. */
        __m256i tb = _mm256_srli_epi32(_mm256_and_si256(tuple, bit), k);
        __m256i m = _mm256_sub_epi32(Uq, tb);
        __m256i notset = _mm256_cmpeq_epi32(_mm256_and_si256(rho, bit),
                                            _mm256_setzero_si256());
        m = _mm256_andnot_si256(notset, m);     /* keep m where rho bit set */
        _mm256_storeu_si256((__m256i *)(m_n + k * 8), m);
        /* cwd_s = s & ((1<<m) - 1); m in [0,30] so the shift is well defined. */
        __m256i mask = _mm256_sub_epi32(_mm256_sllv_epi32(one, m), one);
        __m256i s = _mm256_loadu_si256((const __m256i *)(sarr + k * 8));
        _mm256_storeu_si256((__m256i *)(cwd_s + k * 8),
                            _mm256_and_si256(s, mask));
    }
}

/* ---------------------------------------------------------------------------
 * Half-pixel deinterleave: bytes → int32 (SSE2/AVX2).
 * Reads `count` little-endian uint16 half-pixel values from `src` and writes
 * int32 to `dst`. The count is rounded down to the SIMD width.
 * ------------------------------------------------------------------------- */
EXR_TARGET("sse2")
size_t jph_deinterleave_half_sse2(const uint8_t *src, int32_t *dst,
                                   size_t count) {
    size_t i = 0, n8 = (count / 8u) * 8u;
    __m128i raw, lo, hi;
    for (; i < n8; i += 8) {
        raw = _mm_loadu_si128((const __m128i *)(src + i * 2u));
        lo = _mm_unpacklo_epi16(raw, raw);
        _mm_storeu_si128((__m128i *)(dst + i),
                         _mm_srai_epi32(lo, 16));
        hi = _mm_unpackhi_epi16(raw, raw);
        _mm_storeu_si128((__m128i *)(dst + i + 4),
                         _mm_srai_epi32(hi, 16));
    }
    for (; i < count; ++i)
        dst[i] = (int32_t)(int16_t)((uint16_t)src[i * 2u] |
                                     ((uint16_t)src[i * 2u + 1u] << 8));
    return i;
}

EXR_TARGET("avx2")
size_t jph_deinterleave_half_avx2(const uint8_t *src, int32_t *dst,
                                   size_t count) {
    size_t i = 0, n16 = (count / 16u) * 16u;
    __m256i raw;
    __m128i lo, hi;
    for (; i < n16; i += 16) {
        raw = _mm256_loadu_si256((const __m256i *)(src + i * 2u));
        lo = _mm256_castsi256_si128(raw);
        _mm_storeu_si128((__m128i *)(dst + i),
                         _mm_cvtepi16_epi32(lo));
        hi = _mm_srli_si128(lo, 8);
        _mm_storeu_si128((__m128i *)(dst + i + 4),
                         _mm_cvtepi16_epi32(hi));
        lo = _mm256_extracti128_si256(raw, 1);
        _mm_storeu_si128((__m128i *)(dst + i + 8),
                         _mm_cvtepi16_epi32(lo));
        hi = _mm_srli_si128(lo, 8);
        _mm_storeu_si128((__m128i *)(dst + i + 12),
                         _mm_cvtepi16_epi32(hi));
    }
    for (; i < count; ++i)
        dst[i] = (int32_t)(int16_t)((uint16_t)src[i * 2u] |
                                     ((uint16_t)src[i * 2u + 1u] << 8));
    return i;
}

/* ---------------------------------------------------------------------------
 * Reversible Color Transform (RCT), int32 (all-HALF path).
 *
 * Inverse  (Y,Db,Dr) -> (R,G,B): G = Y - ((Db+Dr) >> 2); R = Dr+G; B = Db+G.
 * Forward  (R,G,B) -> (Y,Db,Dr): Y = (R+B+2G) >> 2; Db = B-G; Dr = R-G.
 * (>> is the arithmetic floor-shift, matching jph_floor_div_pow2(.,2).)
 *
 * The scalar reference carries int64 intermediates and reports CORRUPT if any
 * output exceeds int32. These kernels process only vectors whose three inputs
 * are all within +/-2^28: there every intermediate and output fits int32 (so
 * the int32 SIMD math is exact AND the scalar path would not report CORRUPT),
 * making the SIMD result byte-identical. On the first out-of-range vector the
 * kernel stops and returns the count already processed; the caller finishes
 * (and performs the precise CORRUPT check) with the scalar loop. The arrays are
 * transformed in place, so the scalar tail reads still-original samples.
 * ------------------------------------------------------------------------- */
#define JPH_RCT_SAFE_BITS 28

EXR_TARGET("avx2")
size_t jph_inverse_rct_i32_avx2(int32_t *c0, int32_t *c1, int32_t *c2,
                                size_t count) {
    size_t i = 0, n8 = count & ~(size_t)7;
    const __m256i lim = _mm256_set1_epi32((1 << JPH_RCT_SAFE_BITS) - 1);
    for (; i < n8; i += 8) {
        __m256i y = _mm256_loadu_si256((const __m256i *)(c0 + i));
        __m256i db = _mm256_loadu_si256((const __m256i *)(c1 + i));
        __m256i dr = _mm256_loadu_si256((const __m256i *)(c2 + i));
        __m256i amax = _mm256_max_epi32(_mm256_abs_epi32(y),
                       _mm256_max_epi32(_mm256_abs_epi32(db),
                                        _mm256_abs_epi32(dr)));
        if (_mm256_movemask_epi8(_mm256_cmpgt_epi32(amax, lim))) break;
        __m256i g = _mm256_sub_epi32(y, _mm256_srai_epi32(
                                            _mm256_add_epi32(db, dr), 2));
        _mm256_storeu_si256((__m256i *)(c0 + i), _mm256_add_epi32(dr, g));
        _mm256_storeu_si256((__m256i *)(c1 + i), g);
        _mm256_storeu_si256((__m256i *)(c2 + i), _mm256_add_epi32(db, g));
    }
    return i;
}

EXR_TARGET("avx2")
size_t jph_forward_rct_i32_avx2(int32_t *c0, int32_t *c1, int32_t *c2,
                                size_t count) {
    size_t i = 0, n8 = count & ~(size_t)7;
    const __m256i lim = _mm256_set1_epi32((1 << JPH_RCT_SAFE_BITS) - 1);
    for (; i < n8; i += 8) {
        __m256i r = _mm256_loadu_si256((const __m256i *)(c0 + i));
        __m256i g = _mm256_loadu_si256((const __m256i *)(c1 + i));
        __m256i b = _mm256_loadu_si256((const __m256i *)(c2 + i));
        __m256i amax = _mm256_max_epi32(_mm256_abs_epi32(r),
                       _mm256_max_epi32(_mm256_abs_epi32(g),
                                        _mm256_abs_epi32(b)));
        if (_mm256_movemask_epi8(_mm256_cmpgt_epi32(amax, lim))) break;
        __m256i t = _mm256_add_epi32(_mm256_add_epi32(r, b),
                                     _mm256_slli_epi32(g, 1));
        _mm256_storeu_si256((__m256i *)(c0 + i), _mm256_srai_epi32(t, 2));
        _mm256_storeu_si256((__m256i *)(c1 + i), _mm256_sub_epi32(b, g));
        _mm256_storeu_si256((__m256i *)(c2 + i), _mm256_sub_epi32(r, g));
    }
    return i;
}

/* SSE2 lacks _mm_abs_epi32; compute abs via (x ^ (x>>31)) - (x>>31). */
EXR_TARGET("sse2")
static inline __m128i jph_abs_epi32_sse2(__m128i v) {
    __m128i m = _mm_srai_epi32(v, 31);
    return _mm_sub_epi32(_mm_xor_si128(v, m), m);
}

EXR_TARGET("sse2")
size_t jph_inverse_rct_i32_sse2(int32_t *c0, int32_t *c1, int32_t *c2,
                                size_t count) {
    size_t i = 0, n4 = count & ~(size_t)3;
    const __m128i lim = _mm_set1_epi32((1 << JPH_RCT_SAFE_BITS) - 1);
    for (; i < n4; i += 4) {
        __m128i y = _mm_loadu_si128((const __m128i *)(c0 + i));
        __m128i db = _mm_loadu_si128((const __m128i *)(c1 + i));
        __m128i dr = _mm_loadu_si128((const __m128i *)(c2 + i));
        if (_mm_movemask_epi8(_mm_or_si128(_mm_or_si128(
                _mm_cmpgt_epi32(jph_abs_epi32_sse2(y), lim),
                _mm_cmpgt_epi32(jph_abs_epi32_sse2(db), lim)),
                _mm_cmpgt_epi32(jph_abs_epi32_sse2(dr), lim))))
            break;
        __m128i g = _mm_sub_epi32(y, _mm_srai_epi32(_mm_add_epi32(db, dr), 2));
        _mm_storeu_si128((__m128i *)(c0 + i), _mm_add_epi32(dr, g));
        _mm_storeu_si128((__m128i *)(c1 + i), g);
        _mm_storeu_si128((__m128i *)(c2 + i), _mm_add_epi32(db, g));
    }
    return i;
}

EXR_TARGET("sse2")
size_t jph_forward_rct_i32_sse2(int32_t *c0, int32_t *c1, int32_t *c2,
                                size_t count) {
    size_t i = 0, n4 = count & ~(size_t)3;
    const __m128i lim = _mm_set1_epi32((1 << JPH_RCT_SAFE_BITS) - 1);
    for (; i < n4; i += 4) {
        __m128i r = _mm_loadu_si128((const __m128i *)(c0 + i));
        __m128i g = _mm_loadu_si128((const __m128i *)(c1 + i));
        __m128i b = _mm_loadu_si128((const __m128i *)(c2 + i));
        if (_mm_movemask_epi8(_mm_or_si128(_mm_or_si128(
                _mm_cmpgt_epi32(jph_abs_epi32_sse2(r), lim),
                _mm_cmpgt_epi32(jph_abs_epi32_sse2(g), lim)),
                _mm_cmpgt_epi32(jph_abs_epi32_sse2(b), lim))))
            break;
        __m128i t = _mm_add_epi32(_mm_add_epi32(r, b), _mm_slli_epi32(g, 1));
        _mm_storeu_si128((__m128i *)(c0 + i), _mm_srai_epi32(t, 2));
        _mm_storeu_si128((__m128i *)(c1 + i), _mm_sub_epi32(b, g));
        _mm_storeu_si128((__m128i *)(c2 + i), _mm_sub_epi32(r, g));
    }
    return i;
}

#endif /* EXR_X86 */
