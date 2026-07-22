/*
 * TinyEXR texpipe - octahedral fold-seam fixup.
 *
 * An octahedral env map folds the lower hemisphere across the square's outer
 * border: the top and bottom rows fold onto themselves reversed in u, the left
 * and right columns fold onto themselves reversed in v, and the four corners
 * meet at a single pole. Averaging these matched pairs per mip keeps the border
 * coherent across LODs (bilinear sampling and mip minification stop bleeding a
 * discontinuity), mirroring tp_cube_seam_fixup.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe_internal.h"

static float *octa_texel(tp_surface *s, int x, int y) {
    return (float *)((uint8_t *)s->data + (size_t)y * s->stride) +
           (size_t)x * (size_t)s->channels;
}

static void octa_avg(float *a, float *b, int ch) {
    int c;
    for (c = 0; c < ch; ++c) {
        float m = 0.5f * (a[c] + b[c]);
        a[c] = m;
        b[c] = m;
    }
}

tp_result tp_octa_seam_fixup(tp_surface *s, const tp_options *opt) {
    int W, H, ch, i, c;
    (void)opt;
    if (!s || !s->data) return TP_ERROR_INVALID_ARGUMENT;
    W = s->width;
    H = s->height;
    ch = s->channels;
    if (W < 2 || H < 2) return TP_SUCCESS;

    /* Top row (y=0) folds onto itself reversed in x. */
    for (i = 0; i * 2 < W; ++i) {
        int j = W - 1 - i;
        if (j > i) octa_avg(octa_texel(s, i, 0), octa_texel(s, j, 0), ch);
    }
    /* Bottom row (y=H-1) folds reversed in x. */
    for (i = 0; i * 2 < W; ++i) {
        int j = W - 1 - i;
        if (j > i) octa_avg(octa_texel(s, i, H - 1), octa_texel(s, j, H - 1), ch);
    }
    /* Left column (x=0) folds reversed in y. */
    for (i = 0; i * 2 < H; ++i) {
        int j = H - 1 - i;
        if (j > i) octa_avg(octa_texel(s, 0, i), octa_texel(s, 0, j), ch);
    }
    /* Right column (x=W-1) folds reversed in y. */
    for (i = 0; i * 2 < H; ++i) {
        int j = H - 1 - i;
        if (j > i) octa_avg(octa_texel(s, W - 1, i), octa_texel(s, W - 1, j), ch);
    }
    /* Four corners meet at the same pole direction: 3-way... actually 4-way. */
    {
        float *p[4];
        p[0] = octa_texel(s, 0, 0);
        p[1] = octa_texel(s, W - 1, 0);
        p[2] = octa_texel(s, 0, H - 1);
        p[3] = octa_texel(s, W - 1, H - 1);
        for (c = 0; c < ch; ++c) {
            float m = 0.25f * (p[0][c] + p[1][c] + p[2][c] + p[3][c]);
            p[0][c] = p[1][c] = p[2][c] = p[3][c] = m;
        }
    }
    return TP_SUCCESS;
}
