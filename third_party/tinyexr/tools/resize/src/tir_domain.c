/*
 * tir - content-aware domain operations.
 *
 * Highlight range compression (the Sony Pictures Imageworks curve used by
 * OpenImageIO's --hicomp), NaN/Inf scrubbing, and normal-map decode/encode.
 * These run at scanline granularity around the filtering core.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tir_internal.h"

/* ---- highlight compression -------------------------------------------------
 * Identity below the knee, logarithmic above; sign-preserving; exact inverse.
 * Constants from OpenImageIO imagebufalgo_pixelmath.cpp (SPI curve): the
 * identity and log branches meet at x1 = 0.18. */
#define TIR__HC_X1 0.18f
#define TIR__HC_A -0.54576885700225830078f
#define TIR__HC_B 0.18351669609546661377f
#define TIR__HC_C 284.3577880859375f

float tir__rangecompress(float x) {
    float ax = x < 0.0f ? -x : x;
    float y;
    if (ax <= TIR__HC_X1) return x;
    y = TIR__HC_A + TIR__HC_B * logf(TIR__HC_C * ax + 1.0f);
    return x < 0.0f ? -y : y;
}

float tir__rangeexpand(float y) {
    float ay = y < 0.0f ? -y : y;
    float x;
    if (ay <= TIR__HC_X1) return y;
    x = (expf((ay - TIR__HC_A) / TIR__HC_B) - 1.0f) / TIR__HC_C;
    return y < 0.0f ? -x : x;
}

void tir__rangecompress_row(float *px, size_t npix, int wc, int nch) {
    size_t i;
    int c;
    for (i = 0; i < npix; ++i)
        for (c = 0; c < nch; ++c)
            px[i * (size_t)wc + c] = tir__rangecompress(px[i * (size_t)wc + c]);
}

void tir__rangeexpand_row(float *px, size_t npix, int wc, int nch) {
    size_t i;
    int c;
    for (i = 0; i < npix; ++i)
        for (c = 0; c < nch; ++c)
            px[i * (size_t)wc + c] = tir__rangeexpand(px[i * (size_t)wc + c]);
}

/* ---- NaN / Inf -------------------------------------------------------------- */

int tir__row_has_nonfinite(const float *row, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i)
        if (!isfinite(row[i])) return 1;
    return 0;
}

void tir__nonfinite_zero(float *row, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i)
        if (!isfinite(row[i])) row[i] = 0.0f;
}

/* Replace each nonfinite sample with the average of the finite samples in
 * its 3x3 neighborhood (0 if none) - OIIO NONFINITE_BOX3 policy. prev/next
 * may alias cur at the image borders. */
void tir__nonfinite_repair(float *dst, const float *prev, const float *cur,
                           const float *next, int w, int ch) {
    int x, c;
    const float *rows[3];
    rows[0] = prev;
    rows[1] = cur;
    rows[2] = next;
    for (x = 0; x < w; ++x) {
        for (c = 0; c < ch; ++c) {
            float v = cur[(size_t)x * (size_t)ch + c];
            if (isfinite(v)) {
                dst[(size_t)x * (size_t)ch + c] = v;
            } else {
                float sum = 0.0f;
                int cnt = 0, r, dx;
                for (r = 0; r < 3; ++r) {
                    for (dx = -1; dx <= 1; ++dx) {
                        int xx = x + dx;
                        float nv;
                        if (xx < 0) xx = 0;
                        if (xx >= w) xx = w - 1;
                        nv = rows[r][(size_t)xx * (size_t)ch + c];
                        if (isfinite(nv)) {
                            sum += nv;
                            cnt++;
                        }
                    }
                }
                dst[(size_t)x * (size_t)ch + c] = cnt ? sum / (float)cnt : 0.0f;
            }
        }
    }
}

/* ---- normal maps -------------------------------------------------------------
 * Decode runs on the converted f32 scanline in decode_row BEFORE filtering;
 * RG input (ch == 2) is expanded in place to 3 work channels with
 * z = sqrt(max(0, 1 - x^2 - y^2)) reconstructed per input texel. */

void tir__normal_decode_row(tir_sampler *s, float *row) {
    const float a = s->n_dec_a, b = s->n_dec_b;
    size_t x, w = (size_t)s->sw;
    if (s->opt.normal_encoding == TIR_NORMAL_RG) {
        /* backward in-place 2ch -> 3ch expansion */
        for (x = w; x-- > 0;) {
            float nx = a * row[x * 2 + 0] + b;
            float ny = a * row[x * 2 + 1] + b;
            float t = 1.0f - nx * nx - ny * ny;
            row[x * 3 + 0] = nx;
            row[x * 3 + 1] = ny;
            row[x * 3 + 2] = t > 0.0f ? sqrtf(t) : 0.0f;
        }
    } else if (a != 1.0f || b != 0.0f) {
        size_t n = w * (size_t)s->wc, i;
        for (i = 0; i < n; ++i) row[i] = a * row[i] + b;
    }
}

/* Encode runs on the staging row AFTER renormalization; RG output packs
 * x,y forward in place (3ch -> 2ch), dropping the reconstructed z. */
void tir__normal_encode_row(tir_sampler *s, float *row) {
    const float a = s->n_enc_a, b = s->n_enc_b;
    size_t x, w = (size_t)s->dw;
    if (s->opt.normal_encoding == TIR_NORMAL_RG) {
        for (x = 0; x < w; ++x) {
            row[x * 2 + 0] = (row[x * 3 + 0] - b) / a;
            row[x * 2 + 1] = (row[x * 3 + 1] - b) / a;
        }
    } else if (a != 1.0f || b != 0.0f) {
        size_t n = w * (size_t)s->wc, i;
        for (i = 0; i < n; ++i) row[i] = (row[i] - b) / a;
    }
}
