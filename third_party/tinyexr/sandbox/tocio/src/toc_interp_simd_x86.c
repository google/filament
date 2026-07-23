/*
 * tocio - x86 SIMD batch kernels for the interpreter (matrix, range). Each
 * reproduces its scalar reference using the same mul/add order (no FMA), so the
 * parity test matches. Function target attributes let the whole file build at
 * baseline ISA; the dispatcher only calls a kernel the CPU supports.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

#if defined(TOC_X86)

#include <immintrin.h>

#if defined(__GNUC__) || defined(__clang__)
#define TOC_TARGET(x) __attribute__((target(x)))
#include <cpuid.h>
#else
#define TOC_TARGET(x)
#endif

unsigned toc_cpu_has_avx2(void) {
#if defined(__GNUC__) || defined(__clang__)
    unsigned a, b, c, d;
    if (!__get_cpuid(1, &a, &b, &c, &d)) return 0;
    /* OSXSAVE + AVX */
    if (!(c & (1u << 27)) || !(c & (1u << 28))) return 0;
    if (!__get_cpuid_count(7, 0, &a, &b, &c, &d)) return 0;
    return (b & (1u << 5)) ? 1u : 0u; /* AVX2 = EBX bit 5 */
#else
    return 0;
#endif
}

/* matrix: m is col-major (m[0..3] = column 0). For ch==4 use SSE; ch==3 falls
 * back to scalar (no alpha lane). out_lane[r] = c0[r]*r + c1[r]*g + c2[r]*b +
 * c3[r]*a + off[r], same association as the scalar reference. */
TOC_TARGET("sse2")
void toc_matrix_batch_sse2(const float *m, const float *off, float *rgba,
                           size_t npix, int ch) {
    __m128 c0, c1, c2, c3, vo;
    size_t i;
    if (ch != 4) { toc_matrix_batch_scalar(m, off, rgba, npix, ch); return; }
    c0 = _mm_loadu_ps(m + 0);
    c1 = _mm_loadu_ps(m + 4);
    c2 = _mm_loadu_ps(m + 8);
    c3 = _mm_loadu_ps(m + 12);
    vo = _mm_loadu_ps(off);
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        __m128 r = _mm_set1_ps(px[0]);
        __m128 g = _mm_set1_ps(px[1]);
        __m128 b = _mm_set1_ps(px[2]);
        __m128 a = _mm_set1_ps(px[3]);
        __m128 v = _mm_add_ps(_mm_mul_ps(c0, r), _mm_mul_ps(c1, g));
        v = _mm_add_ps(v, _mm_mul_ps(c2, b));
        v = _mm_add_ps(v, _mm_mul_ps(c3, a));
        v = _mm_add_ps(v, vo);
        _mm_storeu_ps(px, v);
    }
}

TOC_TARGET("sse2")
void toc_range_batch_sse2(const float *scale, const float *offset,
                          const float *vmin, const float *vmax, int clamp_lo,
                          int clamp_hi, float *rgba, size_t npix, int ch) {
    __m128 vs, vof, lo, hi;
    size_t i;
    if (ch != 4) {
        toc_range_batch_scalar(scale, offset, vmin, vmax, clamp_lo, clamp_hi,
                               rgba, npix, ch);
        return;
    }
    vs = _mm_loadu_ps(scale);
    vof = _mm_loadu_ps(offset);
    lo = _mm_loadu_ps(vmin);
    hi = _mm_loadu_ps(vmax);
    for (i = 0; i < npix; ++i) {
        float *px = rgba + i * 4;
        __m128 v = _mm_add_ps(_mm_mul_ps(_mm_loadu_ps(px), vs), vof);
        if (clamp_lo) v = _mm_max_ps(v, lo);
        if (clamp_hi) v = _mm_min_ps(v, hi);
        _mm_storeu_ps(px, v);
    }
}

/* AVX2 range: process 2 RGBA pixels (8 floats) per iteration. */
TOC_TARGET("avx2")
void toc_range_batch_avx2(const float *scale, const float *offset,
                          const float *vmin, const float *vmax, int clamp_lo,
                          int clamp_hi, float *rgba, size_t npix, int ch) {
    __m256 vs, vof, lo, hi;
    size_t i = 0, n2;
    if (ch != 4) {
        toc_range_batch_scalar(scale, offset, vmin, vmax, clamp_lo, clamp_hi,
                               rgba, npix, ch);
        return;
    }
    vs = _mm256_set_m128(_mm_loadu_ps(scale), _mm_loadu_ps(scale));
    vof = _mm256_set_m128(_mm_loadu_ps(offset), _mm_loadu_ps(offset));
    lo = _mm256_set_m128(_mm_loadu_ps(vmin), _mm_loadu_ps(vmin));
    hi = _mm256_set_m128(_mm_loadu_ps(vmax), _mm_loadu_ps(vmax));
    n2 = npix & ~(size_t)1;
    for (; i < n2; i += 2) {
        float *px = rgba + i * 4;
        __m256 v = _mm256_add_ps(_mm256_mul_ps(_mm256_loadu_ps(px), vs), vof);
        if (clamp_lo) v = _mm256_max_ps(v, lo);
        if (clamp_hi) v = _mm256_min_ps(v, hi);
        _mm256_storeu_ps(px, v);
    }
    if (i < npix)
        toc_range_batch_sse2(scale, offset, vmin, vmax, clamp_lo, clamp_hi,
                             rgba + i * 4, npix - i, ch);
}

#endif /* TOC_X86 */
