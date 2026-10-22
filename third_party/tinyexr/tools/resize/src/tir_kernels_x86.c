/*
 * tir - x86 SIMD kernels (SSE2 / SSE4.1 / AVX2+FMA+F16C).
 *
 * One translation unit compiled at the baseline ISA; each function carries a
 * target attribute and is reachable only after the matching CPUID check in
 * tir_cpu.c. Sums may be reassociated relative to the scalar reference
 * (multiple accumulators, FMA) within a small ULP budget; the type
 * converters are bit-exact.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tir_internal.h"

#if defined(TIR_X86)

#include <immintrin.h>

#if defined(__GNUC__) || defined(__clang__)
#define TIR_TGT(x) __attribute__((target(x)))
#else
#define TIR_TGT(x) /* MSVC compiles intrinsics without arch flags */
#endif

/* ===========================================================================
 * SSE2
 * ========================================================================= */

TIR_TGT("sse2")
static void v_mul_sse2(float *dst, const float *src, float w, size_t n) {
    __m128 wv = _mm_set1_ps(w);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        _mm_storeu_ps(dst + i, _mm_mul_ps(_mm_loadu_ps(src + i), wv));
        _mm_storeu_ps(dst + i + 4, _mm_mul_ps(_mm_loadu_ps(src + i + 4), wv));
    }
    for (; i < n; ++i) dst[i] = src[i] * w;
}

TIR_TGT("sse2")
static void v_fma_sse2(float *dst, const float *src, float w, size_t n) {
    __m128 wv = _mm_set1_ps(w);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m128 a = _mm_add_ps(_mm_loadu_ps(dst + i),
                              _mm_mul_ps(_mm_loadu_ps(src + i), wv));
        __m128 b = _mm_add_ps(_mm_loadu_ps(dst + i + 4),
                              _mm_mul_ps(_mm_loadu_ps(src + i + 4), wv));
        _mm_storeu_ps(dst + i, a);
        _mm_storeu_ps(dst + i + 4, b);
    }
    for (; i < n; ++i) dst[i] += src[i] * w;
}

TIR_TGT("sse2")
static void v_fma4_sse2(float *dst, const float *const src[4], const float *w,
                        size_t n) {
    __m128 w0 = _mm_set1_ps(w[0]), w1 = _mm_set1_ps(w[1]);
    __m128 w2 = _mm_set1_ps(w[2]), w3 = _mm_set1_ps(w[3]);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 acc = _mm_loadu_ps(dst + i);
        acc = _mm_add_ps(acc, _mm_mul_ps(_mm_loadu_ps(src[0] + i), w0));
        acc = _mm_add_ps(acc, _mm_mul_ps(_mm_loadu_ps(src[1] + i), w1));
        acc = _mm_add_ps(acc, _mm_mul_ps(_mm_loadu_ps(src[2] + i), w2));
        acc = _mm_add_ps(acc, _mm_mul_ps(_mm_loadu_ps(src[3] + i), w3));
        _mm_storeu_ps(dst + i, acc);
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

/* 1-channel horizontal: batch 4 outputs with independent accumulators and
 * reduce them with one 4x4 transpose instead of four per-output horizontal
 * sums. The source row carries `padded` zeroed slack, so the shared inner
 * loop can run to the batch's rounded max tap count. */
TIR_TGT("sse2")
static void h_dot1_sse2(float *dst, const float *src, const int32_t *start,
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
        __m128 a0 = _mm_setzero_ps(), a1 = _mm_setzero_ps();
        __m128 a2 = _mm_setzero_ps(), a3 = _mm_setzero_ps();
        int t;
        for (t = 0; t < padded; t += 4) {
            a0 = _mm_add_ps(a0,
                            _mm_mul_ps(_mm_loadu_ps(w0 + t), _mm_loadu_ps(s0 + t)));
            a1 = _mm_add_ps(a1,
                            _mm_mul_ps(_mm_loadu_ps(w1 + t), _mm_loadu_ps(s1 + t)));
            a2 = _mm_add_ps(a2,
                            _mm_mul_ps(_mm_loadu_ps(w2 + t), _mm_loadu_ps(s2 + t)));
            a3 = _mm_add_ps(a3,
                            _mm_mul_ps(_mm_loadu_ps(w3 + t), _mm_loadu_ps(s3 + t)));
        }
        _MM_TRANSPOSE4_PS(a0, a1, a2, a3);
        _mm_storeu_ps(dst + x,
                      _mm_add_ps(_mm_add_ps(a0, a1), _mm_add_ps(a2, a3)));
    }
    for (; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x];
        __m128 acc = _mm_setzero_ps();
        int t;
        for (t = 0; t < padded; t += 4)
            acc = _mm_add_ps(
                acc, _mm_mul_ps(_mm_loadu_ps(wr + t), _mm_loadu_ps(sp + t)));
        acc = _mm_add_ps(acc, _mm_movehl_ps(acc, acc));
        acc = _mm_add_ss(acc, _mm_shuffle_ps(acc, acc, 1));
        _mm_store_ss(dst + x, acc);
    }
}

TIR_TGT("sse2")
static void h_dot4_sse2(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 4;
        int nblk = (count[x] + 1) & ~1;
        int t;
        __m128 a0 = _mm_setzero_ps(), a1 = _mm_setzero_ps();
        for (t = 0; t < nblk; t += 2) {
            a0 = _mm_add_ps(a0, _mm_mul_ps(_mm_loadu_ps(sp + (size_t)t * 4),
                                           _mm_set1_ps(wr[t])));
            a1 = _mm_add_ps(a1,
                            _mm_mul_ps(_mm_loadu_ps(sp + (size_t)(t + 1) * 4),
                                       _mm_set1_ps(wr[t + 1])));
        }
        _mm_storeu_ps(dst + (size_t)x * 4, _mm_add_ps(a0, a1));
    }
}

TIR_TGT("sse2")
static void h_dot2_sse2(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 2;
        int nblk = (count[x] + 3) & ~3;
        int t;
        __m128 acc0 = _mm_setzero_ps(), acc1 = _mm_setzero_ps();
        for (t = 0; t < nblk; t += 4) {
            __m128 wv = _mm_loadu_ps(wr + t);
            __m128 w01 = _mm_unpacklo_ps(wv, wv);
            __m128 w23 = _mm_unpackhi_ps(wv, wv);
            __m128 s01 = _mm_loadu_ps(sp + (size_t)t * 2);
            __m128 s23 = _mm_loadu_ps(sp + (size_t)(t + 2) * 2);
            acc0 = _mm_add_ps(acc0, _mm_mul_ps(w01, s01));
            acc1 = _mm_add_ps(acc1, _mm_mul_ps(w23, s23));
        }
        {   /* sum: acc[0]+acc[2] = ch0, acc[1]+acc[3] = ch1 */
            __m128 s = _mm_add_ps(acc0, acc1);
            __m128 r = _mm_add_ps(s, _mm_shuffle_ps(s, s, _MM_SHUFFLE(1,0,3,2)));
            _mm_storel_pi((__m64 *)(dst + (size_t)x * 2), r);
        }
    }
}

TIR_TGT("sse2")
static void h_dot3_sse2(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 3;
        int nblk = (count[x] + 1) & ~1;
        int t;
        __m128 acc0 = _mm_setzero_ps(), acc1 = _mm_setzero_ps();
        for (t = 0; t < nblk; t += 2) {
            acc0 = _mm_add_ps(acc0, _mm_mul_ps(_mm_loadu_ps(sp + (size_t)t * 3),
                                               _mm_set1_ps(wr[t])));
            acc1 = _mm_add_ps(acc1, _mm_mul_ps(_mm_loadu_ps(sp + (size_t)(t + 1) * 3),
                                               _mm_set1_ps(wr[t + 1])));
        }
        /* writes 4 floats: +4 slack on staging/ring rows makes this safe */
        _mm_storeu_ps(dst + (size_t)x * 3, _mm_add_ps(acc0, acc1));
    }
}

TIR_TGT("sse2")
static void minmax_combine_sse2(float *mn, float *mx, const float *rmn,
                                const float *rmx, size_t n) {
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        _mm_storeu_ps(mn + i, _mm_min_ps(_mm_loadu_ps(mn + i),
                                         _mm_loadu_ps(rmn + i)));
        _mm_storeu_ps(mx + i, _mm_max_ps(_mm_loadu_ps(mx + i),
                                         _mm_loadu_ps(rmx + i)));
    }
    for (; i < n; ++i) {
        if (rmn[i] < mn[i]) mn[i] = rmn[i];
        if (rmx[i] > mx[i]) mx[i] = rmx[i];
    }
}

TIR_TGT("sse2")
static void antiring_apply_sse2(float *dst, const float *mn, const float *mx,
                                float s, size_t n) {
    __m128 sv = _mm_set1_ps(s);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 v = _mm_loadu_ps(dst + i);
        __m128 c = _mm_min_ps(_mm_max_ps(v, _mm_loadu_ps(mn + i)),
                              _mm_loadu_ps(mx + i));
        _mm_storeu_ps(dst + i,
                      _mm_add_ps(v, _mm_mul_ps(sv, _mm_sub_ps(c, v))));
    }
    for (; i < n; ++i) {
        float v = dst[i];
        float c = v < mn[i] ? mn[i] : (v > mx[i] ? mx[i] : v);
        dst[i] = v + s * (c - v);
    }
}

TIR_TGT("sse2")
static void clamp_range_sse2(float *dst, float lo, float hi, size_t n) {
    __m128 lov = _mm_set1_ps(lo), hiv = _mm_set1_ps(hi);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 v = _mm_loadu_ps(dst + i);
        _mm_storeu_ps(dst + i, _mm_min_ps(_mm_max_ps(v, lov), hiv));
    }
    for (; i < n; ++i) {
        float v = dst[i];
        if (v < lo) v = lo;
        if (v > hi) v = hi;
        dst[i] = v;
    }
}

TIR_TGT("sse2")
static void normalize3_sse2(float *xyz, float *len_out_or_null, size_t npix) {
    size_t i;
    for (i = 0; i < npix; ++i) {
        size_t off = (size_t)i * 3;
        float x = xyz[off], y = xyz[off + 1], z = xyz[off + 2];
        float l2 = x * x + y * y + z * z;
        if (len_out_or_null) len_out_or_null[i] = sqrtf(l2);
        if (l2 > 1e-8f) {
            float inv = 1.0f / sqrtf(l2);
            xyz[off] = x * inv;
            xyz[off + 1] = y * inv;
            xyz[off + 2] = z * inv;
        } else {
            xyz[off] = 0.0f;
            xyz[off + 1] = 0.0f;
            xyz[off + 2] = 1.0f;
        }
    }
}

TIR_TGT("sse2")
static void premult4_sse2(float *rgba, size_t npix) {
    size_t i;
    /* keep lane 3 (alpha) from the original pixel */
    const __m128 rgbmask =
        _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0));
    for (i = 0; i < npix; ++i) {
        __m128 p = _mm_loadu_ps(rgba + i * 4);
        __m128 a = _mm_shuffle_ps(p, p, _MM_SHUFFLE(3, 3, 3, 3));
        __m128 m = _mm_mul_ps(p, a);
        m = _mm_or_ps(_mm_and_ps(m, rgbmask), _mm_andnot_ps(rgbmask, p));
        _mm_storeu_ps(rgba + i * 4, m);
    }
}

TIR_TGT("sse2")
static void unpremult4_sse2(float *rgba, size_t npix) {
    size_t i;
    const __m128 one = _mm_set1_ps(1.0f);
    const __m128 rgbmask =
        _mm_castsi128_ps(_mm_setr_epi32(-1, -1, -1, 0));
    for (i = 0; i < npix; ++i) {
        __m128 p = _mm_loadu_ps(rgba + i * 4);
        __m128 a = _mm_shuffle_ps(p, p, _MM_SHUFFLE(3, 3, 3, 3));
        __m128 nz = _mm_cmpneq_ps(a, _mm_setzero_ps());
        __m128 inv = _mm_and_ps(_mm_div_ps(one, a), nz);
        __m128 scaled = _mm_mul_ps(p, inv);
        /* a==0: keep filtered RGB (matches scalar/OIIO), don't zero it */
        __m128 rgb = _mm_or_ps(_mm_and_ps(nz, scaled),
                               _mm_andnot_ps(nz, p));
        __m128 m = _mm_or_ps(_mm_and_ps(rgb, rgbmask),
                             _mm_andnot_ps(rgbmask, p));
        _mm_storeu_ps(rgba + i * 4, m);
    }
}

/* Anti-ringing per-tap min/max. The RGBA (ch==4) case is one vminps/vmaxps per
 * tap across the 4 channel lanes; other channel counts stay scalar (the taps
 * are not 16-byte aligned per lane, and a wide load would over-read the last
 * tap). min/max are exact, so this is within the ULP budget. This slot was
 * scalar on every ISA before -- the antiring HDR/RGBA path now vectorizes. */
TIR_TGT("sse2")
static void h_minmax_sse2(float *mn, float *mx, const float *src,
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
        __m128 lo = _mm_loadu_ps(sp);
        __m128 hi = lo;
        for (t = 1; t < n; ++t) {
            __m128 v = _mm_loadu_ps(sp + (size_t)t * 4);
            lo = _mm_min_ps(lo, v);
            hi = _mm_max_ps(hi, v);
        }
        _mm_storeu_ps(mn + (size_t)x * 4, lo);
        _mm_storeu_ps(mx + (size_t)x * 4, hi);
    }
}

void tir__kernels_set_sse2(tir_kernels *k) {
    k->v_mul = v_mul_sse2;
    k->v_fma = v_fma_sse2;
    k->v_fma4 = v_fma4_sse2;
    k->h_dot[0] = h_dot1_sse2;
    k->h_dot[1] = h_dot2_sse2;
    k->h_dot[2] = h_dot3_sse2;
    k->h_dot[3] = h_dot4_sse2;
    k->h_minmax = h_minmax_sse2;
    k->minmax_combine = minmax_combine_sse2;
    k->antiring_apply = antiring_apply_sse2;
    k->clamp_range = clamp_range_sse2;
    k->normalize3 = normalize3_sse2;
    k->premult4 = premult4_sse2;
    k->unpremult4 = unpremult4_sse2;
}

/* ===========================================================================
 * SSE4.1 (integer converters)
 * ========================================================================= */

TIR_TGT("sse4.1")
static void u8_to_f32_sse41(float *dst, const uint8_t *src, size_t n) {
    const __m128 k = _mm_set1_ps(1.0f / 255.0f);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m128i b = _mm_loadl_epi64((const __m128i *)(src + i));
        __m128i lo = _mm_cvtepu8_epi32(b);
        __m128i hi = _mm_cvtepu8_epi32(_mm_srli_epi64(b, 32));
        _mm_storeu_ps(dst + i, _mm_mul_ps(_mm_cvtepi32_ps(lo), k));
        _mm_storeu_ps(dst + i + 4, _mm_mul_ps(_mm_cvtepi32_ps(hi), k));
    }
    if (i < n) tir__u8_to_f32_sc(dst + i, src + i, n - i);
}

TIR_TGT("sse4.1")
static void u16_to_f32_sse41(float *dst, const uint16_t *src, size_t n) {
    const __m128 k = _mm_set1_ps(1.0f / 65535.0f);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m128i v = _mm_loadu_si128((const __m128i *)(src + i));
        __m128i lo = _mm_cvtepu16_epi32(v);
        __m128i hi = _mm_cvtepu16_epi32(_mm_srli_si128(v, 8));
        _mm_storeu_ps(dst + i, _mm_mul_ps(_mm_cvtepi32_ps(lo), k));
        _mm_storeu_ps(dst + i + 4, _mm_mul_ps(_mm_cvtepi32_ps(hi), k));
    }
    if (i < n) tir__u16_to_f32_sc(dst + i, src + i, n - i);
}

TIR_TGT("sse4.1")
static void f32_to_u8_sse41(uint8_t *dst, const float *src, size_t n) {
    const __m128 zero = _mm_setzero_ps(), one = _mm_set1_ps(1.0f);
    const __m128 k = _mm_set1_ps(255.0f);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m128 a = _mm_loadu_ps(src + i), b = _mm_loadu_ps(src + i + 4);
        __m128i ia, ib, w, bytes;
        a = _mm_min_ps(_mm_max_ps(a, zero), one); /* NaN -> 0 (max order) */
        b = _mm_min_ps(_mm_max_ps(b, zero), one);
        ia = _mm_cvtps_epi32(_mm_mul_ps(a, k)); /* RNE */
        ib = _mm_cvtps_epi32(_mm_mul_ps(b, k));
        w = _mm_packs_epi32(ia, ib);
        bytes = _mm_packus_epi16(w, w);
        _mm_storel_epi64((__m128i *)(dst + i), bytes);
    }
    if (i < n) tir__f32_to_u8_sc(dst + i, src + i, n - i);
}

TIR_TGT("sse4.1")
static void f32_to_u16_sse41(uint16_t *dst, const float *src, size_t n) {
    const __m128 zero = _mm_setzero_ps(), one = _mm_set1_ps(1.0f);
    const __m128 k = _mm_set1_ps(65535.0f);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m128 a = _mm_loadu_ps(src + i), b = _mm_loadu_ps(src + i + 4);
        __m128i ia, ib;
        a = _mm_min_ps(_mm_max_ps(a, zero), one);
        b = _mm_min_ps(_mm_max_ps(b, zero), one);
        ia = _mm_cvtps_epi32(_mm_mul_ps(a, k));
        ib = _mm_cvtps_epi32(_mm_mul_ps(b, k));
        _mm_storeu_si128((__m128i *)(dst + i), _mm_packus_epi32(ia, ib));
    }
    if (i < n) tir__f32_to_u16_sc(dst + i, src + i, n - i);
}

void tir__kernels_set_sse41(tir_kernels *k) {
    k->u8_to_f32 = u8_to_f32_sse41;
    k->u16_to_f32 = u16_to_f32_sse41;
    k->f32_to_u8 = f32_to_u8_sse41;
    k->f32_to_u16 = f32_to_u16_sse41;
}

/* ===========================================================================
 * AVX2 (+FMA +F16C)
 * ========================================================================= */

#define TIR_AVX2 "avx2,fma,f16c"

TIR_TGT(TIR_AVX2)
static void v_mul_avx2(float *dst, const float *src, float w, size_t n) {
    __m256 wv = _mm256_set1_ps(w);
    size_t i = 0;
    for (; i + 16 <= n; i += 16) {
        _mm256_storeu_ps(dst + i,
                         _mm256_mul_ps(_mm256_loadu_ps(src + i), wv));
        _mm256_storeu_ps(dst + i + 8,
                         _mm256_mul_ps(_mm256_loadu_ps(src + i + 8), wv));
    }
    for (; i + 8 <= n; i += 8)
        _mm256_storeu_ps(dst + i, _mm256_mul_ps(_mm256_loadu_ps(src + i), wv));
    for (; i < n; ++i) dst[i] = src[i] * w;
}

TIR_TGT(TIR_AVX2)
static void v_fma_avx2(float *dst, const float *src, float w, size_t n) {
    __m256 wv = _mm256_set1_ps(w);
    size_t i = 0;
    for (; i + 16 <= n; i += 16) {
        __m256 a = _mm256_fmadd_ps(_mm256_loadu_ps(src + i), wv,
                                   _mm256_loadu_ps(dst + i));
        __m256 b = _mm256_fmadd_ps(_mm256_loadu_ps(src + i + 8), wv,
                                   _mm256_loadu_ps(dst + i + 8));
        _mm256_storeu_ps(dst + i, a);
        _mm256_storeu_ps(dst + i + 8, b);
    }
    for (; i + 8 <= n; i += 8)
        _mm256_storeu_ps(dst + i, _mm256_fmadd_ps(_mm256_loadu_ps(src + i),
                                                  wv, _mm256_loadu_ps(dst + i)));
    for (; i < n; ++i) dst[i] += src[i] * w;
}

TIR_TGT(TIR_AVX2)
static void v_fma4_avx2(float *dst, const float *const src[4], const float *w,
                        size_t n) {
    __m256 w0 = _mm256_set1_ps(w[0]), w1 = _mm256_set1_ps(w[1]);
    __m256 w2 = _mm256_set1_ps(w[2]), w3 = _mm256_set1_ps(w[3]);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 acc = _mm256_loadu_ps(dst + i);
        acc = _mm256_fmadd_ps(_mm256_loadu_ps(src[0] + i), w0, acc);
        acc = _mm256_fmadd_ps(_mm256_loadu_ps(src[1] + i), w1, acc);
        acc = _mm256_fmadd_ps(_mm256_loadu_ps(src[2] + i), w2, acc);
        acc = _mm256_fmadd_ps(_mm256_loadu_ps(src[3] + i), w3, acc);
        _mm256_storeu_ps(dst + i, acc);
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
 * 4x4 transpose to reduce (vs four per-output horizontal sums). The 4-wide
 * inner loop wastes fewer lanes than an 8-wide single-output dot at the
 * small tap counts typical of resampling; wider 8-output batches lose to the
 * cross-lane transpose they need to reduce, so 4-wide is the sweet spot.
 * Weight rows are zero-padded to `padded` (a multiple of 4) so the loop runs
 * a fixed count with no per-output tap-count bookkeeping. */
TIR_TGT(TIR_AVX2)
static void h_dot1_avx2(float *dst, const float *src, const int32_t *start,
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
        __m128 a0 = _mm_setzero_ps(), a1 = _mm_setzero_ps();
        __m128 a2 = _mm_setzero_ps(), a3 = _mm_setzero_ps();
        int t;
        for (t = 0; t < padded; t += 4) {
            a0 = _mm_fmadd_ps(_mm_loadu_ps(w0 + t), _mm_loadu_ps(s0 + t), a0);
            a1 = _mm_fmadd_ps(_mm_loadu_ps(w1 + t), _mm_loadu_ps(s1 + t), a1);
            a2 = _mm_fmadd_ps(_mm_loadu_ps(w2 + t), _mm_loadu_ps(s2 + t), a2);
            a3 = _mm_fmadd_ps(_mm_loadu_ps(w3 + t), _mm_loadu_ps(s3 + t), a3);
        }
        _MM_TRANSPOSE4_PS(a0, a1, a2, a3);
        _mm_storeu_ps(dst + x,
                      _mm_add_ps(_mm_add_ps(a0, a1), _mm_add_ps(a2, a3)));
    }
    for (; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x];
        __m128 acc = _mm_setzero_ps();
        int t;
        for (t = 0; t < padded; t += 4)
            acc = _mm_fmadd_ps(_mm_loadu_ps(wr + t), _mm_loadu_ps(sp + t),
                               acc);
        acc = _mm_add_ps(acc, _mm_movehl_ps(acc, acc));
        acc = _mm_add_ss(acc, _mm_shuffle_ps(acc, acc, 1));
        _mm_store_ss(dst + x, acc);
    }
}

TIR_TGT(TIR_AVX2)
static void h_dot2_avx2(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    const __m256i dup = _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3);
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 2;
        int nblk = (count[x] + 3) & ~3;
        int t;
        __m256 acc = _mm256_setzero_ps();
        for (t = 0; t < nblk; t += 4) {
            __m256 wv = _mm256_permutevar8x32_ps(
                _mm256_castps128_ps256(_mm_loadu_ps(wr + t)), dup);
            acc = _mm256_fmadd_ps(_mm256_loadu_ps(sp + (size_t)t * 2), wv,
                                  acc);
        }
        {
            __m128 r = _mm_add_ps(_mm256_castps256_ps128(acc),
                                  _mm256_extractf128_ps(acc, 1));
            r = _mm_add_ps(r, _mm_movehl_ps(r, r));
            _mm_storel_pi((__m64 *)(dst + (size_t)x * 2), r);
        }
    }
}

TIR_TGT(TIR_AVX2)
static void h_dot3_avx2(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 3;
        int nblk = (count[x] + 1) & ~1;
        int t;
        __m128 a0 = _mm_setzero_ps(), a1 = _mm_setzero_ps();
        for (t = 0; t < nblk; t += 2) {
            a0 = _mm_fmadd_ps(_mm_loadu_ps(sp + (size_t)t * 3),
                              _mm_set1_ps(wr[t]), a0);
            a1 = _mm_fmadd_ps(_mm_loadu_ps(sp + (size_t)(t + 1) * 3),
                              _mm_set1_ps(wr[t + 1]), a1);
        }
        a0 = _mm_add_ps(a0, a1);
        /* writes 4 floats: +4 slack on staging/ring rows makes this safe */
        _mm_storeu_ps(dst + (size_t)x * 3, a0);
    }
}

/* 4-channel horizontal: one RGBA pixel is a 128-bit vector, so pack two
 * output pixels into a __m256 (low/high halves) and drive both with 256-bit
 * FMA. Each tap broadcasts its two coefficients into the matching halves;
 * the source is a loadu2 of the two pixels' windows. Two accumulators over
 * tap pairs hide FMA latency. The per-tap coefficient insertf128 only pays
 * off for narrow windows (up/mild-down); wide downscale windows keep the
 * single-pixel kernel whose coefficient broadcast is a plain set1. */
TIR_TGT(TIR_AVX2)
static void h_dot4_avx2(float *dst, const float *src, const int32_t *start,
                        const int32_t *count, const float *w, int padded,
                        int x0, int x1) {
    int x = x0;
    (void)count;
    if (padded <= 8)
    for (; x + 2 <= x1; x += 2) {
        const float *w0 = w + (size_t)(x + 0) * (size_t)padded;
        const float *w1 = w + (size_t)(x + 1) * (size_t)padded;
        const float *s0 = src + (size_t)start[x + 0] * 4;
        const float *s1 = src + (size_t)start[x + 1] * 4;
        __m256 acc0 = _mm256_setzero_ps(), acc1 = _mm256_setzero_ps();
        int t;
        for (t = 0; t < padded; t += 2) {
            __m256 c0 = _mm256_insertf128_ps(
                _mm256_castps128_ps256(_mm_set1_ps(w0[t])),
                _mm_set1_ps(w1[t]), 1);
            __m256 c1 = _mm256_insertf128_ps(
                _mm256_castps128_ps256(_mm_set1_ps(w0[t + 1])),
                _mm_set1_ps(w1[t + 1]), 1);
            __m256 v0 = _mm256_loadu2_m128(s1 + (size_t)t * 4,
                                           s0 + (size_t)t * 4);
            __m256 v1 = _mm256_loadu2_m128(s1 + (size_t)(t + 1) * 4,
                                           s0 + (size_t)(t + 1) * 4);
            acc0 = _mm256_fmadd_ps(c0, v0, acc0);
            acc1 = _mm256_fmadd_ps(c1, v1, acc1);
        }
        acc0 = _mm256_add_ps(acc0, acc1);
        _mm_storeu_ps(dst + (size_t)(x + 0) * 4,
                      _mm256_castps256_ps128(acc0));
        _mm_storeu_ps(dst + (size_t)(x + 1) * 4,
                      _mm256_extractf128_ps(acc0, 1));
    }
    for (; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x] * 4;
        __m128 a0 = _mm_setzero_ps(), a1 = _mm_setzero_ps();
        int t;
        for (t = 0; t < padded; t += 2) {
            a0 = _mm_fmadd_ps(_mm_loadu_ps(sp + (size_t)t * 4),
                              _mm_set1_ps(wr[t]), a0);
            a1 = _mm_fmadd_ps(_mm_loadu_ps(sp + (size_t)(t + 1) * 4),
                              _mm_set1_ps(wr[t + 1]), a1);
        }
        _mm_storeu_ps(dst + (size_t)x * 4, _mm_add_ps(a0, a1));
    }
}

TIR_TGT(TIR_AVX2)
static void minmax_combine_avx2(float *mn, float *mx, const float *rmn,
                                const float *rmx, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        _mm256_storeu_ps(mn + i, _mm256_min_ps(_mm256_loadu_ps(mn + i),
                                               _mm256_loadu_ps(rmn + i)));
        _mm256_storeu_ps(mx + i, _mm256_max_ps(_mm256_loadu_ps(mx + i),
                                               _mm256_loadu_ps(rmx + i)));
    }
    for (; i < n; ++i) {
        if (rmn[i] < mn[i]) mn[i] = rmn[i];
        if (rmx[i] > mx[i]) mx[i] = rmx[i];
    }
}

TIR_TGT(TIR_AVX2)
static void antiring_apply_avx2(float *dst, const float *mn, const float *mx,
                                float s, size_t n) {
    __m256 sv = _mm256_set1_ps(s);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 v = _mm256_loadu_ps(dst + i);
        __m256 c = _mm256_min_ps(_mm256_max_ps(v, _mm256_loadu_ps(mn + i)),
                                 _mm256_loadu_ps(mx + i));
        _mm256_storeu_ps(dst + i,
                         _mm256_fmadd_ps(sv, _mm256_sub_ps(c, v), v));
    }
    for (; i < n; ++i) {
        float v = dst[i];
        float c = v < mn[i] ? mn[i] : (v > mx[i] ? mx[i] : v);
        dst[i] = v + s * (c - v);
    }
}

TIR_TGT(TIR_AVX2)
static void clamp_range_avx2(float *dst, float lo, float hi, size_t n) {
    __m256 lov = _mm256_set1_ps(lo), hiv = _mm256_set1_ps(hi);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 v = _mm256_loadu_ps(dst + i);
        _mm256_storeu_ps(dst + i, _mm256_min_ps(_mm256_max_ps(v, lov), hiv));
    }
    for (; i < n; ++i) {
        float v = dst[i];
        if (v < lo) v = lo;
        if (v > hi) v = hi;
        dst[i] = v;
    }
}

TIR_TGT(TIR_AVX2)
static void f16_to_f32_f16c(float *dst, const uint16_t *src, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m128i h16 = _mm_loadu_si128((const __m128i *)(src + i));
        __m256 f = _mm256_cvtph_ps(h16);
        /* vcvtph2ps force-quiets signaling NaNs (sets the top mantissa bit);
         * the scalar path preserves the mantissa (sign | 0x7F800000 | man<<13),
         * so recompute the NaN lanes and blend them in to stay bit-identical.
         * inf/subnormal/finite lanes are already exact -- only NaN differs. */
        __m256i h = _mm256_cvtepu16_epi32(h16);
        __m256i man = _mm256_and_si256(h, _mm256_set1_epi32(0x03FF));
        __m256i is_exp31 = _mm256_cmpeq_epi32(
            _mm256_and_si256(h, _mm256_set1_epi32(0x7C00)),
            _mm256_set1_epi32(0x7C00));
        __m256i man_zero = _mm256_cmpeq_epi32(man, _mm256_setzero_si256());
        __m256i is_nan = _mm256_andnot_si256(man_zero, is_exp31);
        __m256i sign = _mm256_slli_epi32(
            _mm256_and_si256(h, _mm256_set1_epi32(0x8000)), 16);
        __m256i nan_bits = _mm256_or_si256(
            _mm256_or_si256(sign, _mm256_set1_epi32(0x7F800000)),
            _mm256_slli_epi32(man, 13));
        _mm256_storeu_ps(
            dst + i, _mm256_blendv_ps(f, _mm256_castsi256_ps(nan_bits),
                                      _mm256_castsi256_ps(is_nan)));
    }
    if (i < n) tir__f16_to_f32_sc(dst + i, src + i, n - i);
}

TIR_TGT(TIR_AVX2)
static void f32_to_f16_f16c(uint16_t *dst, const float *src, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8)
        _mm_storeu_si128(
            (__m128i *)(dst + i),
            _mm256_cvtps_ph(_mm256_loadu_ps(src + i),
                            _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
    if (i < n) tir__f32_to_f16_sc(dst + i, src + i, n - i);
}

TIR_TGT(TIR_AVX2)
static void u8_to_f32_avx2(float *dst, const uint8_t *src, size_t n) {
    const __m256 k = _mm256_set1_ps(1.0f / 255.0f);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256i v = _mm256_cvtepu8_epi32(
            _mm_loadl_epi64((const __m128i *)(src + i)));
        _mm256_storeu_ps(dst + i, _mm256_mul_ps(_mm256_cvtepi32_ps(v), k));
    }
    if (i < n) tir__u8_to_f32_sc(dst + i, src + i, n - i);
}

TIR_TGT(TIR_AVX2)
static void u16_to_f32_avx2(float *dst, const uint16_t *src, size_t n) {
    const __m256 k = _mm256_set1_ps(1.0f / 65535.0f);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256i v = _mm256_cvtepu16_epi32(
            _mm_loadu_si128((const __m128i *)(src + i)));
        _mm256_storeu_ps(dst + i, _mm256_mul_ps(_mm256_cvtepi32_ps(v), k));
    }
    if (i < n) tir__u16_to_f32_sc(dst + i, src + i, n - i);
}

void tir__kernels_set_avx2(tir_kernels *k) {
    k->v_mul = v_mul_avx2;
    k->v_fma = v_fma_avx2;
    k->v_fma4 = v_fma4_avx2;
    k->h_dot[0] = h_dot1_avx2;
    k->h_dot[1] = h_dot2_avx2;
    k->h_dot[2] = h_dot3_avx2;
    k->h_dot[3] = h_dot4_avx2;
    k->minmax_combine = minmax_combine_avx2;
    k->antiring_apply = antiring_apply_avx2;
    k->clamp_range = clamp_range_avx2;
    k->f16_to_f32 = f16_to_f32_f16c;
    k->f32_to_f16 = f32_to_f16_f16c;
    k->u8_to_f32 = u8_to_f32_avx2;
    k->u16_to_f32 = u16_to_f32_avx2;
}

#endif /* TIR_X86 */
