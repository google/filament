/*
 * TinyEXR texcomp - BC5/BC4 encoder
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "texcomp.h"
#include "texcomp_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

void tc_encode_bc4_block(const uint8_t v[16], uint8_t out[8]) {
    uint8_t minv = 255, maxv = 0, palette[8];
    uint64_t bits = 0;
    uint32_t i;
    for (i = 0; i < 16u; ++i) {
        if (v[i] < minv) minv = v[i];
        if (v[i] > maxv) maxv = v[i];
    }
    out[0] = maxv;
    out[1] = minv;
    palette[0] = maxv;
    palette[1] = minv;
    for (i = 1; i <= 6u; ++i)
        palette[i + 1u] = (uint8_t)(((7u - i) * maxv + i * minv + 3u) / 7u);
    for (i = 0; i < 16u; ++i) {
        uint32_t j, best = 0, best_err = UINT_MAX;
        for (j = 0; j < 8u; ++j) {
            int d = (int)v[i] - (int)palette[j];
            uint32_t e = (uint32_t)(d * d);
            if (e < best_err) {
                best_err = e;
                best = j;
            }
        }
        bits |= (uint64_t)best << (3u * i);
    }
    for (i = 0; i < 6u; ++i) out[2u + i] = (uint8_t)(bits >> (8u * i));
}

static void tc_encode_bc5_block(const uint8_t block[16][2], uint8_t out[16]) {
    uint8_t r[16], g[16];
    uint32_t i;
    for (i = 0; i < 16u; ++i) {
        r[i] = block[i][0];
        g[i] = block[i][1];
    }
    tc_encode_bc4_block(r, out);
    tc_encode_bc4_block(g, out + 8);
}

tc_result tc_bc5_compress_rg8(const uint8_t *rg, uint32_t width,
                              uint32_t height, size_t stride,
                              const tc_bc5_options *opt, uint8_t *out_bc5,
                              size_t out_size) {
    uint32_t bx, by, x, y, xx, yy;
    uint8_t block[16][2];
    size_t need, off = 0;
    (void)opt;

    if (!rg || !out_bc5 || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 2u) return TC_ERROR_INVALID_ARGUMENT;
    need = tc_bc5_compressed_size(width, height);
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
                    src = rg + (size_t)y * stride + (size_t)x * 2u;
                    block[yy * 4u + xx][0] = src[0];
                    block[yy * 4u + xx][1] = src[1];
                }
            }
            tc_encode_bc5_block(block, out_bc5 + off);
            off += 16u;
        }
    }

    return TC_SUCCESS;
}

tc_result tc_bc5_compress_rgba8(const uint8_t *rgba, uint32_t width,
                                uint32_t height, size_t stride,
                                const tc_bc5_options *opt, uint8_t *out_bc5,
                                size_t out_size) {
    uint32_t bx, by, x, y, xx, yy;
    uint8_t block[16][2];
    size_t need, off = 0;
    (void)opt;

    if (!rgba || !out_bc5 || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u) return TC_ERROR_INVALID_ARGUMENT;
    need = tc_bc5_compressed_size(width, height);
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
                }
            }
            tc_encode_bc5_block(block, out_bc5 + off);
            off += 16u;
        }
    }

    return TC_SUCCESS;
}
