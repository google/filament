/*
 * TinyEXR texpipe - alpha-coverage preservation across LODs.
 *
 * Downsampling shrinks the fraction of texels passing an alpha test (foliage
 * and cutouts thin out with distance). Following Castano, "Computing Alpha
 * Mipmaps" (2010), we rescale each mip's alpha by a scalar so its coverage
 * matches the base level's, found by binary search on the scale.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe_internal.h"

float tp_alpha_coverage(const tp_surface *s, float threshold) {
    size_t pass = 0, total;
    int x, y;
    if (!s || s->channels < 4 || s->width < 1 || s->height < 1) return 0.0f;
    total = (size_t)s->width * (size_t)s->height;
    for (y = 0; y < s->height; ++y) {
        const float *row = (const float *)((const uint8_t *)s->data +
                                           (size_t)y * s->stride);
        for (x = 0; x < s->width; ++x)
            if (row[x * s->channels + 3] >= threshold) ++pass;
    }
    return (float)pass / (float)total;
}

/* Coverage the surface would have if every alpha were multiplied by `scale`
 * (clamped to [0,1]) — cheaper than mutating then measuring. */
static float tp_coverage_scaled(const tp_surface *s, float threshold,
                                float scale) {
    size_t pass = 0, total = (size_t)s->width * (size_t)s->height;
    int x, y;
    for (y = 0; y < s->height; ++y) {
        const float *row = (const float *)((const uint8_t *)s->data +
                                           (size_t)y * s->stride);
        for (x = 0; x < s->width; ++x) {
            float a = row[x * s->channels + 3] * scale;
            if (a > 1.0f) a = 1.0f;
            if (a >= threshold) ++pass;
        }
    }
    return (float)pass / (float)total;
}

tp_result tp_alpha_scale_to_coverage(tp_surface *s, float target,
                                     float threshold) {
    float lo = 0.0f, hi = 4.0f, mid = 1.0f, best = 1.0f, best_err;
    int i, x, y;
    if (!s || s->channels < 4) return TP_ERROR_INVALID_ARGUMENT;
    if (s->width < 1 || s->height < 1) return TP_SUCCESS;

    best_err = tp_coverage_scaled(s, threshold, 1.0f) - target;
    if (best_err < 0.0f) best_err = -best_err;
    /* Binary search: coverage is monotonic non-decreasing in scale. */
    for (i = 0; i < 24; ++i) {
        float cov, err;
        mid = 0.5f * (lo + hi);
        cov = tp_coverage_scaled(s, threshold, mid);
        err = cov - target;
        if (err < 0.0f) err = -err;
        if (err < best_err) {
            best_err = err;
            best = mid;
        }
        if (cov < target)
            lo = mid; /* need more coverage -> larger scale */
        else
            hi = mid;
    }

    if (best == 1.0f) return TP_SUCCESS;
    for (y = 0; y < s->height; ++y) {
        float *row = (float *)((uint8_t *)s->data + (size_t)y * s->stride);
        for (x = 0; x < s->width; ++x) {
            float a = row[x * s->channels + 3] * best;
            if (a > 1.0f) a = 1.0f;
            if (a < 0.0f) a = 0.0f;
            row[x * s->channels + 3] = a;
        }
    }
    return TP_SUCCESS;
}
