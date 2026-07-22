/*
 * TinyEXR - x86 SIMD kernels for the util module (integer<->float packs,
 * resize accumulation, 3x3 matrix apply).
 *
 * Function target attributes let the whole file build at baseline ISA; the
 * dispatcher in exr_cpu.c only calls a kernel when CPUID reports the feature.
 * Each kernel reproduces its scalar reference; the float->int kernels rely on
 * the default round-to-nearest-even of cvtps_epi32 (matches the scalar magic
 * trick), and the FMA-free mul+add ordering keeps axpy/mat3 close to scalar.
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

EXR_TARGET("sse4.1")
void exr_u8_to_f32_sse41(float *dst, const uint8_t *src, size_t n, float scale) {
    __m128 s = _mm_set1_ps(scale);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128i b = _mm_cvtsi32_si128((int)(*(const uint32_t *)(src + i)));
        __m128i w = _mm_cvtepu8_epi32(b);
        _mm_storeu_ps(dst + i, _mm_mul_ps(_mm_cvtepi32_ps(w), s));
    }
    for (; i < n; ++i) dst[i] = (float)src[i] * scale;
}

EXR_TARGET("sse4.1")
void exr_u16_to_f32_sse41(float *dst, const uint16_t *src, size_t n,
                          float scale) {
    __m128 s = _mm_set1_ps(scale);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128i h = _mm_loadl_epi64((const __m128i *)(src + i));
        __m128i w = _mm_cvtepu16_epi32(h);
        _mm_storeu_ps(dst + i, _mm_mul_ps(_mm_cvtepi32_ps(w), s));
    }
    for (; i < n; ++i) dst[i] = (float)src[i] * scale;
}

EXR_TARGET("sse4.1")
void exr_f32_to_u8_sse41(uint8_t *dst, const float *src, size_t n, float scale) {
    __m128 s = _mm_set1_ps(scale);
    __m128 lo = _mm_setzero_ps(), hi = _mm_set1_ps(255.0f);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 v = _mm_mul_ps(_mm_loadu_ps(src + i), s);
        v = _mm_min_ps(_mm_max_ps(v, lo), hi); /* max(NaN,0)->0 */
        {
            __m128i u = _mm_cvtps_epi32(v);          /* round to even */
            u = _mm_packus_epi32(u, u);              /* i32 -> u16 */
            u = _mm_packus_epi16(u, u);              /* u16 -> u8 */
            *(uint32_t *)(dst + i) = (uint32_t)_mm_cvtsi128_si32(u);
        }
    }
    for (; i < n; ++i) {
        float v = src[i] * scale;
        if (!(v > 0.0f)) v = 0.0f;
        if (v > 255.0f) v = 255.0f;
        v = (v + 12582912.0f) - 12582912.0f;
        dst[i] = (uint8_t)v;
    }
}

EXR_TARGET("sse4.1")
void exr_f32_to_u16_sse41(uint16_t *dst, const float *src, size_t n,
                          float scale) {
    __m128 s = _mm_set1_ps(scale);
    __m128 lo = _mm_setzero_ps(), hi = _mm_set1_ps(65535.0f);
    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 v = _mm_mul_ps(_mm_loadu_ps(src + i), s);
        v = _mm_min_ps(_mm_max_ps(v, lo), hi);
        {
            __m128i u = _mm_cvtps_epi32(v);
            u = _mm_packus_epi32(u, u); /* i32 -> u16 (unsigned saturate) */
            _mm_storel_epi64((__m128i *)(dst + i), u);
        }
    }
    for (; i < n; ++i) {
        float v = src[i] * scale;
        if (!(v > 0.0f)) v = 0.0f;
        if (v > 65535.0f) v = 65535.0f;
        v = (v + 12582912.0f) - 12582912.0f;
        dst[i] = (uint16_t)v;
    }
}

/* acc += w * x  (8-wide; mul+add, no FMA, to track the scalar reference). */
EXR_TARGET("avx2")
void exr_axpy_avx2(float *acc, const float *x, float w, size_t n) {
    __m256 wv = _mm256_set1_ps(w);
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 a = _mm256_loadu_ps(acc + i);
        __m256 p = _mm256_mul_ps(_mm256_loadu_ps(x + i), wv);
        _mm256_storeu_ps(acc + i, _mm256_add_ps(a, p));
    }
    for (; i < n; ++i) acc[i] += x[i] * w;
}

/* Interleaved 3x3 apply: out.rgb = M * in.rgb; 4th channel (alpha) passes
 * through. Processes one pixel per iteration but keeps the channel math in
 * vector lanes; the dominant win is for ch==3/4 over large images. */
EXR_TARGET("avx2")
void exr_mat3_avx2(float *dst, const float *src, size_t px, int ch,
                   const float *m) {
    size_t i;
    if (ch < 3) { /* nothing cross-channel to do */
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

#endif /* EXR_X86 */
