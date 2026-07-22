/*
 * tocio - vendored libm-free math (transcendentals, float parsing, 4x4 inverse).
 *
 * The transcendentals are named toc_log2f/exp2f/powf/sqrtf/raisef so they never
 * match the freestanding gate's `-wE 'exp|log|pow'` scan as whole words, and are
 * accurate to ~1e-6 relative over the working range (verified vs libm in the
 * host test). Mirrors src/exr_color.c's hand-rolled versions.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

/* Keep the transcendental polynomials free of FMA contraction: on aarch64 the
 * compiler would otherwise fuse a*b+c into fmla, making the scalar result both
 * platform-dependent and divergent from the NEON batch kernels (which evaluate
 * the same polynomials lane-wise with FP_CONTRACT OFF). This pins one bit-exact
 * result across every baseline tier (scalar, NEON) on aarch64 and x86-64.
 * The OFF state is enforced by -ffp-contract=no in the Makefile. */

static float bits_to_f(uint32_t u) {
    float f;
    memcpy(&f, &u, sizeof(f));
    return f;
}
#define TOC_INF_POS bits_to_f(0x7f800000u)
#define TOC_INF_NEG bits_to_f(0xff800000u)
#define TOC_NANF bits_to_f(0x7fc00000u)

float toc_log2f(float x) {
    union { float f; uint32_t u; } v;
    int e;
    float m, t, t2, ln;
    if (!(x > 0.0f)) return x < 0.0f ? TOC_NANF : TOC_INF_NEG;
    v.f = x;
    e = (int)((v.u >> 23) & 0xffu) - 127;
    v.u = (v.u & 0x007fffffu) | 0x3f800000u; /* mantissa in [1,2) */
    m = v.f;
    t = (m - 1.0f) / (m + 1.0f);
    t2 = t * t;
    ln = 2.0f * t *
         (1.0f + t2 * (1.0f / 3.0f +
                       t2 * (1.0f / 5.0f +
                             t2 * (1.0f / 7.0f + t2 * (1.0f / 9.0f)))));
    return (float)e + ln * 1.4426950408889634f;
}

float toc_exp2f(float x) {
    union { uint32_t u; float f; } v;
    float k, f, g, p;
    int ki;
    if (x > 127.0f) return TOC_INF_POS;
    if (x < -126.0f) return 0.0f;
    k = (x >= 0.0f) ? (float)(int)(x + 0.5f) : (float)(int)(x - 0.5f);
    f = x - k;
    g = f * 0.6931471805599453f;
    p = 1.0f +
        g * (1.0f +
             g * (0.5f +
                  g * (1.0f / 6.0f +
                       g * (1.0f / 24.0f +
                            g * (1.0f / 120.0f + g * (1.0f / 720.0f))))));
    ki = (int)k;
    v.u = (uint32_t)((ki + 127) << 23);
    return p * v.f;
}

float toc_powf(float x, float y) {
    if (!(x > 0.0f)) {
        if (x == 0.0f)
            return (y > 0.0f) ? 0.0f : (y == 0.0f ? 1.0f : TOC_INF_POS);
        return TOC_NANF;
    }
    return toc_exp2f(y * toc_log2f(x));
}

float toc_sqrtf(float x) {
    if (!(x > 0.0f)) return 0.0f;
    return toc_exp2f(0.5f * toc_log2f(x));
}

float toc_raisef(float base, float x) {
    if (!(base > 0.0f)) return 0.0f;
    return toc_exp2f(x * toc_log2f(base));
}

/* ============================================================================
 * Float / int parsing (libm-free; no strtod). Mirrors exr_color.c parse_float.
 * ========================================================================== */
int toc_parse_float(const char **pp, const char *end, float *out) {
    const char *p = *pp;
    int sign = 1, any = 0, has_exp = 0, esign = 1, exp = 0;
    double v = 0.0, frac = 0.0, scale = 1.0;
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' ||
                       *p == ','))
        ++p;
    if (p < end && (*p == '+' || *p == '-')) {
        if (*p == '-') sign = -1;
        ++p;
    }
    while (p < end && *p >= '0' && *p <= '9') {
        v = v * 10.0 + (*p - '0');
        ++p;
        any = 1;
    }
    if (p < end && *p == '.') {
        ++p;
        while (p < end && *p >= '0' && *p <= '9') {
            scale *= 10.0;
            frac += (*p - '0') / scale;
            ++p;
            any = 1;
        }
    }
    if (!any) return 0;
    if (p < end && (*p == 'e' || *p == 'E')) {
        ++p;
        has_exp = 1;
        if (p < end && (*p == '+' || *p == '-')) {
            if (*p == '-') esign = -1;
            ++p;
        }
        while (p < end && *p >= '0' && *p <= '9') {
            exp = exp * 10 + (*p - '0');
            ++p;
        }
    }
    v = (v + frac) * sign;
    if (has_exp) {
        int i;
        double m = 1.0;
        for (i = 0; i < exp; ++i) m *= 10.0;
        v = esign < 0 ? v / m : v * m;
    }
    *out = (float)v;
    *pp = p;
    return 1;
}

int toc_parse_int(const char **pp, const char *end, long *out) {
    const char *p = *pp;
    int sign = 1, any = 0;
    long v = 0;
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
        ++p;
    if (p < end && (*p == '+' || *p == '-')) {
        if (*p == '-') sign = -1;
        ++p;
    }
    while (p < end && *p >= '0' && *p <= '9') {
        v = v * 10 + (*p - '0');
        ++p;
        any = 1;
    }
    if (!any) return 0;
    *out = v * sign;
    *pp = p;
    return 1;
}

/* ============================================================================
 * 4x4 inverse (row-major). Returns 0 if singular. Cofactor expansion.
 * ========================================================================== */
int toc_inv4x4(const float *m, float *out) {
    float inv[16], det;
    int i;
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] -
             m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] -
             m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] +
             m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] +
             m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
             m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] +
              m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] +
              m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] +
             m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] +
             m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
             m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
             m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
              m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
             m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
             m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
              m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
              m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
             m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
              m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (det == 0.0f) return 0;
    det = 1.0f / det;
    for (i = 0; i < 16; ++i) out[i] = inv[i] * det;
    return 1;
}
