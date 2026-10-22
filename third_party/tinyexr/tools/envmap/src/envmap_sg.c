/*
 * TinyEXR envmap - spherical gaussian (SG) fitting and evaluation.
 *
 * Fixed Fibonacci-distributed lobe axes with a shared sharpness (~N/2 so lobes
 * tile the sphere), amplitudes solved by weighted least squares (normal
 * equations + ridge, non-negativity clamp). A compact all-frequency-ish
 * environment representation for analytic lighting, after Wang et al.,
 * "All-Frequency Rendering of Dynamic, Spatially-Varying Reflectance"
 * (SIGGRAPH Asia 2009).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "envmap.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

void em_sg_eval(const em_sg_lobe *lobes, int num_lobes, const float dir[3],
                float rgb[3]) {
    int i;
    rgb[0] = rgb[1] = rgb[2] = 0.0f;
    for (i = 0; i < num_lobes; ++i) {
        const em_sg_lobe *L = &lobes[i];
        float d = L->axis[0] * dir[0] + L->axis[1] * dir[1] + L->axis[2] * dir[2];
        float g = expf(L->sharpness * (d - 1.0f));
        rgb[0] += L->amplitude[0] * g;
        rgb[1] += L->amplitude[1] * g;
        rgb[2] += L->amplitude[2] * g;
    }
}

typedef struct {
    int n;
    float sharpness;
    const float *axes; /* n*3 */
    double *gtg;       /* n*n */
    double *gtb;       /* n*3 */
    float *gbuf;       /* n scratch */
} sg_ctx;

static void sg_accum(const float dir[3], const float rgb[3], float sa,
                     void *user) {
    sg_ctx *c = (sg_ctx *)user;
    int i, j, n = c->n;
    for (i = 0; i < n; ++i) {
        const float *ax = c->axes + i * 3;
        float d = ax[0] * dir[0] + ax[1] * dir[1] + ax[2] * dir[2];
        c->gbuf[i] = expf(c->sharpness * (d - 1.0f));
    }
    for (i = 0; i < n; ++i) {
        double gi = (double)c->gbuf[i] * (double)sa;
        c->gtb[i * 3 + 0] += gi * rgb[0];
        c->gtb[i * 3 + 1] += gi * rgb[1];
        c->gtb[i * 3 + 2] += gi * rgb[2];
        for (j = 0; j < n; ++j) c->gtg[i * n + j] += gi * (double)c->gbuf[j];
    }
}

/* Solve A(n*n) X = B(n*rhs) in place (partial pivoting). Returns 0 on success. */
static int solve_linear(double *A, double *B, int n, int rhs) {
    int i, j, k;
    for (k = 0; k < n; ++k) {
        int piv = k;
        double best = fabs(A[k * n + k]);
        for (i = k + 1; i < n; ++i) {
            double v = fabs(A[i * n + k]);
            if (v > best) { best = v; piv = i; }
        }
        if (best < 1e-12) return 1;
        if (piv != k) {
            for (j = 0; j < n; ++j) { double t = A[k * n + j]; A[k * n + j] = A[piv * n + j]; A[piv * n + j] = t; }
            for (j = 0; j < rhs; ++j) { double t = B[k * rhs + j]; B[k * rhs + j] = B[piv * rhs + j]; B[piv * rhs + j] = t; }
        }
        for (i = k + 1; i < n; ++i) {
            double f = A[i * n + k] / A[k * n + k];
            for (j = k; j < n; ++j) A[i * n + j] -= f * A[k * n + j];
            for (j = 0; j < rhs; ++j) B[i * rhs + j] -= f * B[k * rhs + j];
        }
    }
    for (k = n - 1; k >= 0; --k) {
        for (j = 0; j < rhs; ++j) {
            double s = B[k * rhs + j];
            for (i = k + 1; i < n; ++i) s -= A[k * n + i] * B[i * rhs + j];
            B[k * rhs + j] = s / A[k * n + k];
        }
    }
    return 0;
}

em_result em_sg_fit(const tir_allocator *a, const em_image *src, int num_lobes,
                    int asg, em_sg_lobe *out_lobes) {
    int n = num_lobes, i;
    float *axes;
    double *gtg, *gtb;
    float *gbuf;
    sg_ctx ctx;
    const double golden = 2.39996322972865332; /* golden angle */
    (void)asg; /* isotropic SG for now */
    if (!src || !out_lobes || n < 1 || n > EM_SG_MAX_LOBES)
        return EM_ERROR_INVALID_ARGUMENT;

    axes = (float *)malloc((size_t)n * 3 * sizeof(float));
    gtg = (double *)malloc((size_t)n * n * sizeof(double));
    gtb = (double *)malloc((size_t)n * 3 * sizeof(double));
    gbuf = (float *)malloc((size_t)n * sizeof(float));
    if (!axes || !gtg || !gtb || !gbuf) {
        free(axes); free(gtg); free(gtb); free(gbuf);
        return EM_ERROR_OUT_OF_MEMORY;
    }
    (void)a;

    /* Fibonacci sphere axes (polar = +Y). */
    for (i = 0; i < n; ++i) {
        double y = 1.0 - (2.0 * i + 1.0) / (double)n;
        double r = sqrt(y * y < 1.0 ? 1.0 - y * y : 0.0);
        double phi = (double)i * golden;
        axes[i * 3 + 0] = (float)(r * cos(phi));
        axes[i * 3 + 1] = (float)y;
        axes[i * 3 + 2] = (float)(r * sin(phi));
    }

    ctx.n = n;
    /* Sharpness so Fibonacci-neighbour lobes overlap near half-max: neighbour
     * angular gap^2 ~ 4/n, half-max at lambda ~ 2 ln2 / gap^2 ~ 0.35 n. */
    ctx.sharpness = (float)n * 0.35f;
    if (ctx.sharpness < 1.0f) ctx.sharpness = 1.0f;
    ctx.axes = axes;
    ctx.gtg = gtg;
    ctx.gtb = gtb;
    ctx.gbuf = gbuf;
    memset(gtg, 0, (size_t)n * n * sizeof(double));
    memset(gtb, 0, (size_t)n * 3 * sizeof(double));

    em_foreach_texel(src, sg_accum, &ctx);

    /* Ridge for conditioning. */
    for (i = 0; i < n; ++i) gtg[i * n + i] += 1e-4;

    if (solve_linear(gtg, gtb, n, 3) != 0) {
        free(axes); free(gtg); free(gtb); free(gbuf);
        return EM_ERROR_UNSUPPORTED;
    }

    for (i = 0; i < n; ++i) {
        out_lobes[i].axis[0] = axes[i * 3 + 0];
        out_lobes[i].axis[1] = axes[i * 3 + 1];
        out_lobes[i].axis[2] = axes[i * 3 + 2];
        out_lobes[i].sharpness = ctx.sharpness;
        out_lobes[i].amplitude[0] = (float)(gtb[i * 3 + 0] > 0 ? gtb[i * 3 + 0] : 0);
        out_lobes[i].amplitude[1] = (float)(gtb[i * 3 + 1] > 0 ? gtb[i * 3 + 1] : 0);
        out_lobes[i].amplitude[2] = (float)(gtb[i * 3 + 2] > 0 ? gtb[i * 3 + 2] : 0);
    }
    free(axes); free(gtg); free(gtb); free(gbuf);
    return EM_SUCCESS;
}
