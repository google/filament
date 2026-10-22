/*
 * TinyEXR - tonemapping operators (util module).
 *
 * Operate on interleaved linear-light float RGB; channels 0..min(ch,3)-1 are
 * tonemapped and any 4th (alpha) passes through. All curves are pure
 * arithmetic (no transcendentals) so they stay in the freestanding core.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

static float clamp01(float x) {
    if (!(x > 0.0f)) return 0.0f;
    return x > 1.0f ? 1.0f : x;
}

static float aces_narkowicz(float x) {
    const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    return clamp01((x * (a * x + b)) / (x * (c * x + d) + e));
}

static float hable_curve(float x, const exr_tonemap_params *p) {
    return ((x * (p->A * x + p->C * p->B) + p->D * p->E) /
            (x * (p->A * x + p->B) + p->D * p->F)) -
           p->E / p->F;
}

exr_result exr_tonemap_float(float *dst, const float *src, size_t pixel_count,
                             int channels, exr_tonemap_op op,
                             const exr_tonemap_params *params) {
    exr_tonemap_params p;
    int color_ch, c;
    size_t i;
    float exposure, lw2, hnorm = 1.0f;
    if (!dst || !src || channels < 1 || channels > 4)
        return EXR_ERROR_INVALID_ARGUMENT;

    /* Defaults. */
    memset(&p, 0, sizeof(p));
    if (params) p = *params;
    exposure = (p.exposure != 0.0f) ? p.exposure : 1.0f;
    {
        float lw = (p.white_point != 0.0f) ? p.white_point : 1.0f;
        lw2 = lw * lw;
    }
    if (op == EXR_TONEMAP_HABLE) {
        if (p.A == 0.0f && p.B == 0.0f && p.C == 0.0f && p.D == 0.0f &&
            p.E == 0.0f && p.F == 0.0f && p.W == 0.0f) {
            p.A = 0.15f; p.B = 0.50f; p.C = 0.10f; p.D = 0.20f;
            p.E = 0.02f; p.F = 0.30f; p.W = 11.2f;
        }
        hnorm = hable_curve(p.W, &p);
        if (hnorm == 0.0f) hnorm = 1.0f;
    }

    color_ch = channels < 3 ? channels : 3;
    for (i = 0; i < pixel_count; ++i) {
        const float *s = src + i * (size_t)channels;
        float *d = dst + i * (size_t)channels;
        for (c = 0; c < color_ch; ++c) {
            float x = s[c] * exposure;
            float y;
            switch (op) {
                case EXR_TONEMAP_REINHARD:
                    y = x / (1.0f + x);
                    break;
                case EXR_TONEMAP_REINHARD_EXT:
                    y = x * (1.0f + x / lw2) / (1.0f + x);
                    break;
                case EXR_TONEMAP_ACES:
                    y = aces_narkowicz(x);
                    break;
                default: /* HABLE */
                    y = hable_curve(x, &p) / hnorm;
                    break;
            }
            d[c] = y;
        }
        for (c = color_ch; c < channels; ++c) d[c] = s[c]; /* alpha passthrough */
    }
    return EXR_SUCCESS;
}
