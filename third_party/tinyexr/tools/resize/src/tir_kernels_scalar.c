/*
 * tir - scalar reference kernels.
 *
 * The scalar set is the semantic source of truth: SIMD kernels may
 * reassociate float sums (FMA, multiple accumulators) within a small ULP
 * budget, but must implement exactly these definitions. The deterministic
 * mode binds this table verbatim.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tir_internal.h"

/* ---- vertical pass -------------------------------------------------------- */

void tir__v_mul_sc(float *dst, const float *src, float w, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) dst[i] = src[i] * w;
}

void tir__v_fma_sc(float *dst, const float *src, float w, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) dst[i] += src[i] * w;
}

void tir__v_fma4_sc(float *dst, const float *const src[4], const float *w,
                    size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        float v = dst[i];
        v += src[0][i] * w[0];
        v += src[1][i] * w[1];
        v += src[2][i] * w[2];
        v += src[3][i] * w[3];
        dst[i] = v;
    }
}

/* ---- horizontal pass ------------------------------------------------------ */

#define TIR__H_DOT_BODY(CH)                                              \
    int x;                                                               \
    for (x = x0; x < x1; ++x) {                                          \
        const float *wr = w + (size_t)x * (size_t)padded;                \
        const float *sp = src + (size_t)start[x] * (CH);                 \
        int t, c;                                                        \
        float acc[CH];                                                   \
        int nblk = (count[x] + 3) & ~3;                                  \
        for (c = 0; c < (CH); ++c) acc[c] = 0.0f;                        \
        for (t = 0; t < nblk; ++t)                                       \
            for (c = 0; c < (CH); ++c)                                   \
                acc[c] += wr[t] * sp[(size_t)t * (CH) + c];              \
        for (c = 0; c < (CH); ++c) dst[(size_t)x * (CH) + c] = acc[c];   \
    }

void tir__h_dot1_sc(float *dst, const float *src, const int32_t *start,
                    const int32_t *count, const float *w, int padded, int x0,
                    int x1) {
    TIR__H_DOT_BODY(1)
}
void tir__h_dot2_sc(float *dst, const float *src, const int32_t *start,
                    const int32_t *count, const float *w, int padded, int x0,
                    int x1) {
    TIR__H_DOT_BODY(2)
}
void tir__h_dot3_sc(float *dst, const float *src, const int32_t *start,
                    const int32_t *count, const float *w, int padded, int x0,
                    int x1) {
    TIR__H_DOT_BODY(3)
}
void tir__h_dot4_sc(float *dst, const float *src, const int32_t *start,
                    const int32_t *count, const float *w, int padded, int x0,
                    int x1) {
    TIR__H_DOT_BODY(4)
}

/* ---- anti-ringing --------------------------------------------------------- */

void tir__h_minmax_sc(float *mn, float *mx, const float *src,
                      const int32_t *start, const int32_t *count, int x0,
                      int x1, int ch) {
    int x, t, c;
    for (x = x0; x < x1; ++x) {
        const float *sp = src + (size_t)start[x] * (size_t)ch;
        for (c = 0; c < ch; ++c) {
            float lo = sp[c], hi = sp[c];
            for (t = 1; t < count[x]; ++t) {
                float v = sp[(size_t)t * (size_t)ch + c];
                if (v < lo) lo = v;
                if (v > hi) hi = v;
            }
            mn[(size_t)x * (size_t)ch + c] = lo;
            mx[(size_t)x * (size_t)ch + c] = hi;
        }
    }
}

void tir__minmax_combine_sc(float *mn, float *mx, const float *rmn,
                            const float *rmx, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        if (rmn[i] < mn[i]) mn[i] = rmn[i];
        if (rmx[i] > mx[i]) mx[i] = rmx[i];
    }
}

void tir__antiring_apply_sc(float *dst, const float *mn, const float *mx,
                            float s, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        float v = dst[i];
        float c = v < mn[i] ? mn[i] : (v > mx[i] ? mx[i] : v);
        dst[i] = v + s * (c - v);
    }
}

void tir__clamp_range_sc(float *dst, float lo, float hi, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) {
        float v = dst[i];
        if (v < lo) v = lo;
        if (v > hi) v = hi;
        dst[i] = v;
    }
}

/* ---- domain kernels -------------------------------------------------------- */

void tir__normalize3_sc(float *xyz, float *len_out_or_null, size_t npix) {
    size_t i;
    for (i = 0; i < npix; ++i) {
        float x = xyz[i * 3 + 0], y = xyz[i * 3 + 1], z = xyz[i * 3 + 2];
        float l2 = x * x + y * y + z * z;
        if (len_out_or_null) len_out_or_null[i] = sqrtf(l2);
        if (l2 > 1e-8f) {
            float inv = 1.0f / sqrtf(l2);
            xyz[i * 3 + 0] = x * inv;
            xyz[i * 3 + 1] = y * inv;
            xyz[i * 3 + 2] = z * inv;
        } else { /* opposing normals cancelled out: tangent-space up */
            xyz[i * 3 + 0] = 0.0f;
            xyz[i * 3 + 1] = 0.0f;
            xyz[i * 3 + 2] = 1.0f;
        }
    }
}

void tir__premult4_sc(float *rgba, size_t npix) {
    size_t i;
    for (i = 0; i < npix; ++i) {
        float a = rgba[i * 4 + 3];
        rgba[i * 4 + 0] *= a;
        rgba[i * 4 + 1] *= a;
        rgba[i * 4 + 2] *= a;
    }
}

void tir__unpremult4_sc(float *rgba, size_t npix) {
    size_t i;
    for (i = 0; i < npix; ++i) {
        float a = rgba[i * 4 + 3];
        if (a != 0.0f) {
            float inv = 1.0f / a;
            rgba[i * 4 + 0] *= inv;
            rgba[i * 4 + 1] *= inv;
            rgba[i * 4 + 2] *= inv;
        } /* a == 0: keep filtered RGB (matches OIIO unpremult behavior) */
    }
}

/* ---- table binding --------------------------------------------------------- */

void tir__kernels_set_scalar(tir_kernels *k) {
    k->v_mul = tir__v_mul_sc;
    k->v_fma = tir__v_fma_sc;
    k->v_fma4 = tir__v_fma4_sc;
    k->h_dot[0] = tir__h_dot1_sc;
    k->h_dot[1] = tir__h_dot2_sc;
    k->h_dot[2] = tir__h_dot3_sc;
    k->h_dot[3] = tir__h_dot4_sc;
    k->h_minmax = tir__h_minmax_sc;
    k->minmax_combine = tir__minmax_combine_sc;
    k->antiring_apply = tir__antiring_apply_sc;
    k->clamp_range = tir__clamp_range_sc;
    k->u8_to_f32 = tir__u8_to_f32_sc;
    k->u16_to_f32 = tir__u16_to_f32_sc;
    k->f16_to_f32 = tir__f16_to_f32_sc;
    k->f32_to_u8 = tir__f32_to_u8_sc;
    k->f32_to_u16 = tir__f32_to_u16_sc;
    k->f32_to_f16 = tir__f32_to_f16_sc;
    k->normalize3 = tir__normalize3_sc;
    k->premult4 = tir__premult4_sc;
    k->unpremult4 = tir__unpremult4_sc;
}
