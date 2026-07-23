/*
 * TinyEXR texcomp - BC1 (DXT1) encoder
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "texcomp.h"
#include "texcomp_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* --- BC1 (DXT1) / BC3 (DXT5) color block ---------------------------------
 * BC1 and BC3 share the same 4-colour RGB565 colour block. BC1 stores it
 * directly (opaque, always the 4-colour interpolation mode, so colour0 must
 * be > colour1); BC3 pairs it with a BC4 alpha block and always decodes the
 * colours in 4-colour mode regardless of endpoint order. */
static uint16_t tc_rgb565_pack(int r, int g, int b) {
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;
    return (uint16_t)(((uint32_t)(r >> 3) << 11) | ((uint32_t)(g >> 2) << 5) |
                      (uint32_t)(b >> 3));
}

static void tc_rgb565_unpack(uint16_t c, int out[3]) {
    int r = (c >> 11) & 0x1f, g = (c >> 5) & 0x3f, b = c & 0x1f;
    out[0] = (r << 3) | (r >> 2);
    out[1] = (g << 2) | (g >> 4);
    out[2] = (b << 3) | (b >> 2);
}

/* Encode the RGB channels of a 4x4 block into an 8-byte DXT1 colour block.
 * Endpoints come from the RGB bounding box refined by two least-squares
 * passes over the assigned indices. `dxt1` selects standalone-BC1 behaviour
 * (enforce colour0 > colour1 and never emit the punch-through index 3 for a
 * single-colour block); when 0 the block is a BC3 colour sub-block. */
void tc_encode_bc1_color_block(const uint8_t px[16][4], int dxt1,
                                      uint8_t out[8]) {
    static const double af[4] = {1.0, 0.0, 2.0 / 3.0, 1.0 / 3.0};
    static const double bf[4] = {0.0, 1.0, 1.0 / 3.0, 2.0 / 3.0};
    double e0[3], e1[3];
    int bbmin[3] = {255, 255, 255}, bbmax[3] = {0, 0, 0};
    uint16_t c0 = 0, c1 = 0;
    uint8_t idx[16];
    int p0[3], p1[3], pal[4][3];
    int i, c, iter;

    for (i = 0; i < 16; ++i)
        for (c = 0; c < 3; ++c) {
            int v = px[i][c];
            if (v < bbmin[c]) bbmin[c] = v;
            if (v > bbmax[c]) bbmax[c] = v;
        }
    for (c = 0; c < 3; ++c) {
        e0[c] = bbmax[c];
        e1[c] = bbmin[c];
    }

    /* Refine endpoints: quantise, assign nearest indices, least-squares
     * re-fit the endpoints for those indices, repeat. */
    for (iter = 0; iter < 2; ++iter) {
        double A00 = 0.0, A01 = 0.0, A11 = 0.0, det;
        double B0[3] = {0, 0, 0}, B1[3] = {0, 0, 0};
        c0 = tc_rgb565_pack((int)(e0[0] + 0.5), (int)(e0[1] + 0.5),
                            (int)(e0[2] + 0.5));
        c1 = tc_rgb565_pack((int)(e1[0] + 0.5), (int)(e1[1] + 0.5),
                            (int)(e1[2] + 0.5));
        tc_rgb565_unpack(c0, p0);
        tc_rgb565_unpack(c1, p1);
        for (c = 0; c < 3; ++c) {
            pal[0][c] = p0[c];
            pal[1][c] = p1[c];
            pal[2][c] = (2 * p0[c] + p1[c] + 1) / 3;
            pal[3][c] = (p0[c] + 2 * p1[c] + 1) / 3;
        }
        for (i = 0; i < 16; ++i) {
            int best = 0, j;
            uint32_t bd = 0xffffffffu;
            for (j = 0; j < 4; ++j) {
                uint32_t d = 0;
                for (c = 0; c < 3; ++c) {
                    int e = (int)px[i][c] - pal[j][c];
                    d += (uint32_t)(e * e);
                }
                if (d < bd) {
                    bd = d;
                    best = j;
                }
            }
            idx[i] = (uint8_t)best;
            A00 += af[best] * af[best];
            A01 += af[best] * bf[best];
            A11 += bf[best] * bf[best];
            for (c = 0; c < 3; ++c) {
                B0[c] += af[best] * px[i][c];
                B1[c] += bf[best] * px[i][c];
            }
        }
        det = A00 * A11 - A01 * A01;
        if (det != 0.0) {
            double inv = 1.0 / det;
            for (c = 0; c < 3; ++c) {
                double n0 = (A11 * B0[c] - A01 * B1[c]) * inv;
                double n1 = (A00 * B1[c] - A01 * B0[c]) * inv;
                e0[c] = n0 < 0.0 ? 0.0 : (n0 > 255.0 ? 255.0 : n0);
                e1[c] = n1 < 0.0 ? 0.0 : (n1 > 255.0 ? 255.0 : n1);
            }
        }
    }

    c0 = tc_rgb565_pack((int)(e0[0] + 0.5), (int)(e0[1] + 0.5),
                        (int)(e0[2] + 0.5));
    c1 = tc_rgb565_pack((int)(e1[0] + 0.5), (int)(e1[1] + 0.5),
                        (int)(e1[2] + 0.5));
    /* 4-colour interpolation needs colour0 > colour1. Order them so the
     * palette below matches what the decoder builds. */
    if (c0 < c1) {
        uint16_t t = c0;
        c0 = c1;
        c1 = t;
    }
    tc_rgb565_unpack(c0, p0);
    tc_rgb565_unpack(c1, p1);
    for (c = 0; c < 3; ++c) {
        pal[0][c] = p0[c];
        pal[1][c] = p1[c];
        pal[2][c] = (2 * p0[c] + p1[c] + 1) / 3;
        pal[3][c] = (p0[c] + 2 * p1[c] + 1) / 3;
    }
    for (i = 0; i < 16; ++i) {
        int best = 0, j;
        uint32_t bd = 0xffffffffu;
        /* A single-colour BC1 block (c0 == c1) decodes in 3-colour mode where
         * index 3 is punch-through black, so restrict it to the flat indices. */
        int jmax = (dxt1 && c0 == c1) ? 3 : 4;
        for (j = 0; j < jmax; ++j) {
            uint32_t d = 0;
            for (c = 0; c < 3; ++c) {
                int e = (int)px[i][c] - pal[j][c];
                d += (uint32_t)(e * e);
            }
            if (d < bd) {
                bd = d;
                best = j;
            }
        }
        idx[i] = (uint8_t)best;
    }

    out[0] = (uint8_t)(c0 & 0xffu);
    out[1] = (uint8_t)(c0 >> 8);
    out[2] = (uint8_t)(c1 & 0xffu);
    out[3] = (uint8_t)(c1 >> 8);
    for (i = 0; i < 4; ++i) {
        out[4 + i] = (uint8_t)(idx[i * 4 + 0] | (idx[i * 4 + 1] << 2) |
                               (idx[i * 4 + 2] << 4) | (idx[i * 4 + 3] << 6));
    }
}

tc_result tc_bc1_compress_rgba8(const uint8_t *rgba, uint32_t width,
                                uint32_t height, size_t stride,
                                const tc_bc1_options *opt, uint8_t *out_bc1,
                                size_t out_size) {
    uint32_t bx, by, x, y, xx, yy;
    uint8_t block[16][4];
    size_t need, off = 0;
    (void)opt;

    if (!rgba || !out_bc1 || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u) return TC_ERROR_INVALID_ARGUMENT;
    need = tc_bc1_compressed_size(width, height);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    for (by = 0; by < height; by += 4) {
        for (bx = 0; bx < width; bx += 4) {
            for (yy = 0; yy < 4; ++yy) {
                y = by + yy;
                if (y >= height) y = height - 1u;
                for (xx = 0; xx < 4; ++xx) {
                    const uint8_t *src;
                    x = bx + xx;
                    if (x >= width) x = width - 1u;
                    src = rgba + (size_t)y * stride + (size_t)x * 4u;
                    block[yy * 4u + xx][0] = src[0];
                    block[yy * 4u + xx][1] = src[1];
                    block[yy * 4u + xx][2] = src[2];
                    block[yy * 4u + xx][3] = src[3];
                }
            }
            tc_encode_bc1_color_block(block, 1, out_bc1 + off);
            off += 8u;
        }
    }
    return TC_SUCCESS;
}
