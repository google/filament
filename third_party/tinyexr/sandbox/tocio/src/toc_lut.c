/*
 * tocio - LUT1D / LUT3D per-pixel apply (trilinear + tetrahedral) and the
 * fixed-function dispatch stub. LUT math mirrors src/exr_color.c.
 *
 * Reimplemented from OpenColorIO (BSD-3-Clause).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

static float lerpf(float a, float b, float t) { return a + (b - a) * t; }

void toc_lut1d_apply_pixel(const toc_op *op, float *px, int ch) {
    const toc_lut1d *L = &op->u.lut1d;
    int N = L->length, c, n = ch < 3 ? ch : 3;
    float den = L->domain_max - L->domain_min;
    if (N < 2 || !L->data) return;
    for (c = 0; c < n; ++c) {
        float u = (den != 0.0f) ? (px[c] - L->domain_min) / den : 0.0f;
        float g, f, a, b;
        int i;
        if (!(u > 0.0f)) u = 0.0f;
        if (u > 1.0f) u = 1.0f;
        g = u * (float)(N - 1);
        i = (int)g;
        if (i >= N - 1) i = N - 2;
        f = g - (float)i;
        if (L->channels == 1) {
            a = L->data[i];
            b = L->data[i + 1];
        } else {
            a = L->data[(size_t)i * 3 + c];
            b = L->data[((size_t)i + 1) * 3 + c];
        }
        px[c] = (L->interp == TOC_INTERP_NEAREST) ? (f < 0.5f ? a : b)
                                                  : lerpf(a, b, f);
    }
}

static const float *cell(const toc_lut3d *L, int ir, int ig, int ib) {
    size_t idx = (((size_t)ib * L->size + ig) * L->size + ir) * 3;
    return L->data + idx;
}

void toc_lut3d_apply_pixel(const toc_op *op, float *px, int ch) {
    const toc_lut3d *L = &op->u.lut3d;
    int N = L->size, c;
    int ic[3];
    float f[3];
    if (N < 2 || !L->data) return;
    for (c = 0; c < 3; ++c) {
        float den = L->domain_max[c] - L->domain_min[c];
        float u = (den != 0.0f) ? (px[c] - L->domain_min[c]) / den : 0.0f;
        float g;
        if (!(u > 0.0f)) u = 0.0f;
        if (u > 1.0f) u = 1.0f;
        g = u * (float)(N - 1);
        ic[c] = (int)g;
        if (ic[c] >= N - 1) ic[c] = N - 2;
        f[c] = g - (float)ic[c];
    }
    if (op->u.lut3d.interp == TOC_INTERP_TETRAHEDRAL) {
        int ir = ic[0], ig = ic[1], ib = ic[2], k;
        float fr = f[0], fg = f[1], fb = f[2];
        const float *c000 = cell(L, ir, ig, ib);
        const float *c111 = cell(L, ir + 1, ig + 1, ib + 1);
        const float *p100 = cell(L, ir + 1, ig, ib);
        const float *p010 = cell(L, ir, ig + 1, ib);
        const float *p001 = cell(L, ir, ig, ib + 1);
        const float *p110 = cell(L, ir + 1, ig + 1, ib);
        const float *p101 = cell(L, ir + 1, ig, ib + 1);
        const float *p011 = cell(L, ir, ig + 1, ib + 1);
        for (k = 0; k < 3; ++k) {
            float v;
            if (fr >= fg && fg >= fb)
                v = c000[k] + fr * (p100[k] - c000[k]) +
                    fg * (p110[k] - p100[k]) + fb * (c111[k] - p110[k]);
            else if (fr >= fb && fb >= fg)
                v = c000[k] + fr * (p100[k] - c000[k]) +
                    fb * (p101[k] - p100[k]) + fg * (c111[k] - p101[k]);
            else if (fb >= fr && fr >= fg)
                v = c000[k] + fb * (p001[k] - c000[k]) +
                    fr * (p101[k] - p001[k]) + fg * (c111[k] - p101[k]);
            else if (fg >= fr && fr >= fb)
                v = c000[k] + fg * (p010[k] - c000[k]) +
                    fr * (p110[k] - p010[k]) + fb * (c111[k] - p110[k]);
            else if (fg >= fb && fb >= fr)
                v = c000[k] + fg * (p010[k] - c000[k]) +
                    fb * (p011[k] - p010[k]) + fr * (c111[k] - p011[k]);
            else
                v = c000[k] + fb * (p001[k] - c000[k]) +
                    fg * (p011[k] - p001[k]) + fr * (c111[k] - p011[k]);
            px[k] = v;
        }
    } else { /* trilinear */
        const float *c0[8];
        float c00[3], c01[3], c10[3], c11[3], a[3], bb[3];
        int k;
        c0[0] = cell(L, ic[0], ic[1], ic[2]);
        c0[1] = cell(L, ic[0] + 1, ic[1], ic[2]);
        c0[2] = cell(L, ic[0], ic[1] + 1, ic[2]);
        c0[3] = cell(L, ic[0] + 1, ic[1] + 1, ic[2]);
        c0[4] = cell(L, ic[0], ic[1], ic[2] + 1);
        c0[5] = cell(L, ic[0] + 1, ic[1], ic[2] + 1);
        c0[6] = cell(L, ic[0], ic[1] + 1, ic[2] + 1);
        c0[7] = cell(L, ic[0] + 1, ic[1] + 1, ic[2] + 1);
        for (k = 0; k < 3; ++k) {
            c00[k] = lerpf(c0[0][k], c0[1][k], f[0]);
            c01[k] = lerpf(c0[4][k], c0[5][k], f[0]);
            c10[k] = lerpf(c0[2][k], c0[3][k], f[0]);
            c11[k] = lerpf(c0[6][k], c0[7][k], f[0]);
            a[k] = lerpf(c00[k], c10[k], f[1]);
            bb[k] = lerpf(c01[k], c11[k], f[1]);
            px[k] = lerpf(a[k], bb[k], f[2]);
        }
    }
    (void)ch;
}

/* toc_fixedfunc_apply_pixel is defined in toc_builtins.c. */
