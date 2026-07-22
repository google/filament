/*
 * tir - ARM SVE (1.0) kernels.
 *
 * This translation unit is the only one built with -march=...+sve (see the
 * resize Makefile rules); everything else stays at the baseline ISA so the
 * runtime HWCAP gate in tir_cpu.c is sound. Without SVE compiler support it
 * degrades to a stub exporting tir__have_sve_kernels = 0.
 *
 * Loop shape: bulk whole-vector iterations under an all-true predicate,
 * then exactly one predicated tail via whilelt - no scalar tails. Deployed
 * cores are mostly VL=128, so gains come from predication, not width; the
 * NEON kernels remain the AArch64 tuning baseline (and the only option on
 * Apple Silicon, which has no non-streaming SVE).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tir_internal.h"

#if defined(TIR_NEON) && defined(__ARM_FEATURE_SVE)

#include <arm_sve.h>

const int tir__have_sve_kernels = 1;

static void v_mul_sve(float *dst, const float *src, float w, size_t n) {
    size_t vl = svcntw(), i = 0;
    svbool_t pg = svptrue_b32();
    for (; i + vl <= n; i += vl)
        svst1_f32(pg, dst + i, svmul_n_f32_x(pg, svld1_f32(pg, src + i), w));
    if (i < n) {
        svbool_t pt = svwhilelt_b32((uint64_t)i, (uint64_t)n);
        svst1_f32(pt, dst + i,
                  svmul_n_f32_x(pt, svld1_f32(pt, src + i), w));
    }
}

static void v_fma_sve(float *dst, const float *src, float w, size_t n) {
    size_t vl = svcntw(), i = 0;
    svbool_t pg = svptrue_b32();
    for (; i + vl <= n; i += vl)
        svst1_f32(pg, dst + i,
                  svmla_n_f32_x(pg, svld1_f32(pg, dst + i),
                                svld1_f32(pg, src + i), w));
    if (i < n) {
        svbool_t pt = svwhilelt_b32((uint64_t)i, (uint64_t)n);
        svst1_f32(pt, dst + i,
                  svmla_n_f32_x(pt, svld1_f32(pt, dst + i),
                                svld1_f32(pt, src + i), w));
    }
}

static void v_fma4_sve(float *dst, const float *const src[4], const float *w,
                       size_t n) {
    size_t vl = svcntw(), i = 0;
    svbool_t pg = svptrue_b32();
    for (; i + vl <= n; i += vl) {
        svfloat32_t acc = svld1_f32(pg, dst + i);
        acc = svmla_n_f32_x(pg, acc, svld1_f32(pg, src[0] + i), w[0]);
        acc = svmla_n_f32_x(pg, acc, svld1_f32(pg, src[1] + i), w[1]);
        acc = svmla_n_f32_x(pg, acc, svld1_f32(pg, src[2] + i), w[2]);
        acc = svmla_n_f32_x(pg, acc, svld1_f32(pg, src[3] + i), w[3]);
        svst1_f32(pg, dst + i, acc);
    }
    if (i < n) {
        svbool_t pt = svwhilelt_b32((uint64_t)i, (uint64_t)n);
        svfloat32_t acc = svld1_f32(pt, dst + i);
        acc = svmla_n_f32_x(pt, acc, svld1_f32(pt, src[0] + i), w[0]);
        acc = svmla_n_f32_x(pt, acc, svld1_f32(pt, src[1] + i), w[1]);
        acc = svmla_n_f32_x(pt, acc, svld1_f32(pt, src[2] + i), w[2]);
        acc = svmla_n_f32_x(pt, acc, svld1_f32(pt, src[3] + i), w[3]);
        svst1_f32(pt, dst + i, acc);
    }
}

static void h_dot1_sve(float *dst, const float *src, const int32_t *start,
                       const int32_t *count, const float *w, int padded,
                       int x0, int x1) {
    size_t vl = svcntw();
    svbool_t pg = svptrue_b32();
    int x;
    for (x = x0; x < x1; ++x) {
        const float *wr = w + (size_t)x * (size_t)padded;
        const float *sp = src + (size_t)start[x];
        size_t cnt = (size_t)count[x], t = 0;
        svfloat32_t acc = svdup_n_f32(0.0f);
        for (; t + vl <= cnt; t += vl)
            acc = svmla_f32_x(pg, acc, svld1_f32(pg, wr + t),
                              svld1_f32(pg, sp + t));
        if (t < cnt) {
            svbool_t pt = svwhilelt_b32((uint64_t)t, (uint64_t)cnt);
            acc = svmla_f32_x(pg, acc,
                              svld1_f32(pt, wr + t) /* inactive -> 0 */,
                              svsel_f32(pt, svld1_f32(pt, sp + t),
                                        svdup_n_f32(0.0f)));
        }
        dst[x] = svaddv_f32(pg, acc);
    }
}

static void minmax_combine_sve(float *mn, float *mx, const float *rmn,
                               const float *rmx, size_t n) {
    size_t vl = svcntw(), i = 0;
    svbool_t pg = svptrue_b32();
    for (; i + vl <= n; i += vl) {
        svst1_f32(pg, mn + i,
                  svmin_f32_x(pg, svld1_f32(pg, mn + i),
                              svld1_f32(pg, rmn + i)));
        svst1_f32(pg, mx + i,
                  svmax_f32_x(pg, svld1_f32(pg, mx + i),
                              svld1_f32(pg, rmx + i)));
    }
    if (i < n) {
        svbool_t pt = svwhilelt_b32((uint64_t)i, (uint64_t)n);
        svst1_f32(pt, mn + i,
                  svmin_f32_x(pt, svld1_f32(pt, mn + i),
                              svld1_f32(pt, rmn + i)));
        svst1_f32(pt, mx + i,
                  svmax_f32_x(pt, svld1_f32(pt, mx + i),
                              svld1_f32(pt, rmx + i)));
    }
}

static void antiring_apply_sve(float *dst, const float *mn, const float *mx,
                               float s, size_t n) {
    size_t vl = svcntw(), i = 0;
    svbool_t pg = svptrue_b32();
    for (; i + vl <= n; i += vl) {
        svfloat32_t v = svld1_f32(pg, dst + i);
        svfloat32_t c = svmin_f32_x(
            pg, svmax_f32_x(pg, v, svld1_f32(pg, mn + i)),
            svld1_f32(pg, mx + i));
        svst1_f32(pg, dst + i,
                  svmla_n_f32_x(pg, v, svsub_f32_x(pg, c, v), s));
    }
    if (i < n) {
        svbool_t pt = svwhilelt_b32((uint64_t)i, (uint64_t)n);
        svfloat32_t v = svld1_f32(pt, dst + i);
        svfloat32_t c = svmin_f32_x(
            pt, svmax_f32_x(pt, v, svld1_f32(pt, mn + i)),
            svld1_f32(pt, mx + i));
        svst1_f32(pt, dst + i,
                  svmla_n_f32_x(pt, v, svsub_f32_x(pt, c, v), s));
    }
}

static void clamp_range_sve(float *dst, float lo, float hi, size_t n) {
    size_t vl = svcntw(), i = 0;
    svbool_t pg = svptrue_b32();
    for (; i + vl <= n; i += vl) {
        svfloat32_t v = svld1_f32(pg, dst + i);
        v = svmin_n_f32_x(pg, svmax_n_f32_x(pg, v, lo), hi);
        svst1_f32(pg, dst + i, v);
    }
    if (i < n) {
        svbool_t pt = svwhilelt_b32((uint64_t)i, (uint64_t)n);
        svfloat32_t v = svld1_f32(pt, dst + i);
        v = svmin_n_f32_x(pt, svmax_n_f32_x(pt, v, lo), hi);
        svst1_f32(pt, dst + i, v);
    }
}

void tir__kernels_set_sve(tir_kernels *k) {
    /* Overrides the contiguous-sweep kernels on top of the NEON set; the
     * channel-interleaved horizontal kernels stay NEON (structure loads
     * showed no win at VL=128). */
    k->v_mul = v_mul_sve;
    k->v_fma = v_fma_sve;
    k->v_fma4 = v_fma4_sve;
    k->h_dot[0] = h_dot1_sve;
    k->minmax_combine = minmax_combine_sve;
    k->antiring_apply = antiring_apply_sve;
    k->clamp_range = clamp_range_sve;
}

#elif defined(TIR_NEON)

const int tir__have_sve_kernels = 0;

void tir__kernels_set_sve(tir_kernels *k) { (void)k; }

#endif /* TIR_NEON && __ARM_FEATURE_SVE */
