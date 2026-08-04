/*
 * TinyEXR envmap - real spherical harmonics (projection / evaluation).
 *
 * Polar axis = +Y (matching the Y-up env convention). The basis is orthonormal
 * over the sphere; project and eval use the same convention so round-trips are
 * self-consistent. Supports order up to EM_SH_MAX_ORDER.
 *
 * Real-SH formulation follows Ramamoorthi & Hanrahan, "An Efficient
 * Representation for Irradiance Environment Maps" (SIGGRAPH 2001), and Sloan,
 * "Stupid Spherical Harmonics (SH) Tricks" (GDC 2008).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "envmap.h"

#include <math.h>

#ifndef EM_PI
#define EM_PI 3.14159265358979323846
#endif

int em_sh_num_coeffs(int order) { return (order + 1) * (order + 1); }

/* Associated Legendre polynomial P_l^m(x), m >= 0, standard recurrence. */
static double alp(int l, int m, double x) {
    double pmm = 1.0;
    if (m > 0) {
        double somx2 = sqrt((1.0 - x) * (1.0 + x));
        double fact = 1.0;
        int i;
        for (i = 1; i <= m; ++i) {
            pmm *= -fact * somx2;
            fact += 2.0;
        }
    }
    if (l == m) return pmm;
    {
        double pmmp1 = x * (2.0 * m + 1.0) * pmm;
        if (l == m + 1) return pmmp1;
        {
            double pll = 0.0;
            int ll;
            for (ll = m + 2; ll <= l; ++ll) {
                pll = ((2.0 * ll - 1.0) * x * pmmp1 - (ll + m - 1.0) * pmm) /
                      (ll - m);
                pmm = pmmp1;
                pmmp1 = pll;
            }
            return pll;
        }
    }
}

static double factorial(int n) {
    double f = 1.0;
    int i;
    for (i = 2; i <= n; ++i) f *= (double)i;
    return f;
}

/* Normalization K_l^m (m >= 0). */
static double sh_K(int l, int m) {
    double num = (2.0 * l + 1.0) * factorial(l - m);
    double den = 4.0 * EM_PI * factorial(l + m);
    return sqrt(num / den);
}

void em_sh_eval_basis(int order, const float dir[3], float *basis) {
    /* polar axis = y; azimuth in the x-z plane. */
    double z = dir[1]; /* cos(theta) */
    double phi = atan2((double)dir[2], (double)dir[0]);
    int l, m;
    const double sqrt2 = 1.4142135623730951;
    if (z < -1.0) z = -1.0;
    if (z > 1.0) z = 1.0;
    for (l = 0; l <= order; ++l) {
        for (m = -l; m <= l; ++m) {
            int idx = l * (l + 1) + m;
            double y;
            if (m == 0) {
                y = sh_K(l, 0) * alp(l, 0, z);
            } else if (m > 0) {
                y = sqrt2 * sh_K(l, m) * cos(m * phi) * alp(l, m, z);
            } else {
                y = sqrt2 * sh_K(l, -m) * sin(-m * phi) * alp(l, -m, z);
            }
            basis[idx] = (float)y;
        }
    }
}

typedef struct {
    int order;
    int ncoeff;
    float *coeffs;
    float *basis;
} sh_ctx;

static void sh_accum(const float dir[3], const float rgb[3], float sa,
                     void *user) {
    sh_ctx *c = (sh_ctx *)user;
    int i;
    em_sh_eval_basis(c->order, dir, c->basis);
    for (i = 0; i < c->ncoeff; ++i) {
        float w = c->basis[i] * sa;
        c->coeffs[i * 3 + 0] += rgb[0] * w;
        c->coeffs[i * 3 + 1] += rgb[1] * w;
        c->coeffs[i * 3 + 2] += rgb[2] * w;
    }
}

em_result em_sh_project(const em_image *src, int order, float *coeffs) {
    sh_ctx ctx;
    float basis[EM_SH_MAX_ORDER * EM_SH_MAX_ORDER + 2 * EM_SH_MAX_ORDER + 1];
    int i, ncoeff;
    if (!src || !coeffs || order < 0 || order > EM_SH_MAX_ORDER)
        return EM_ERROR_INVALID_ARGUMENT;
    ncoeff = em_sh_num_coeffs(order);
    for (i = 0; i < ncoeff * 3; ++i) coeffs[i] = 0.0f;
    ctx.order = order;
    ctx.ncoeff = ncoeff;
    ctx.coeffs = coeffs;
    ctx.basis = basis;
    em_foreach_texel(src, sh_accum, &ctx);
    return EM_SUCCESS;
}

void em_sh_eval(int order, const float *coeffs, const float dir[3],
                float rgb[3]) {
    float basis[EM_SH_MAX_ORDER * EM_SH_MAX_ORDER + 2 * EM_SH_MAX_ORDER + 1];
    int i, ncoeff = em_sh_num_coeffs(order);
    rgb[0] = rgb[1] = rgb[2] = 0.0f;
    em_sh_eval_basis(order, dir, basis);
    for (i = 0; i < ncoeff; ++i) {
        rgb[0] += coeffs[i * 3 + 0] * basis[i];
        rgb[1] += coeffs[i * 3 + 1] * basis[i];
        rgb[2] += coeffs[i * 3 + 2] * basis[i];
    }
}

void em_sh_window(int order, float *coeffs, float window_width) {
    int l, m;
    if (window_width <= 0.0f) return;
    for (l = 0; l <= order; ++l) {
        /* Hanning window over bands; band l attenuated as it approaches the
         * window edge, killing Gibbs ringing from a truncated series. */
        double w = (l <= window_width)
                       ? 0.5 * (1.0 + cos(EM_PI * (double)l / (double)window_width))
                       : 0.0;
        for (m = -l; m <= l; ++m) {
            int idx = l * (l + 1) + m;
            coeffs[idx * 3 + 0] *= (float)w;
            coeffs[idx * 3 + 1] *= (float)w;
            coeffs[idx * 3 + 2] *= (float)w;
        }
    }
}
