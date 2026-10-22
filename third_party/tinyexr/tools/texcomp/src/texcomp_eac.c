/*
 * TinyEXR texcomp - EAC encoder
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "texcomp.h"
#include "texcomp_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

static const int32_t tc_eac_alpha[16][8] = {
    {-3, -6, -9, -15, 2, 5, 8, 14}, {-3, -7, -10, -13, 2, 6, 9, 12},
    {-2, -5, -8, -13, 1, 4, 7, 12}, {-2, -4, -6, -13, 1, 3, 5, 12},
    {-3, -6, -8, -12, 2, 5, 7, 11}, {-3, -7, -9, -11, 2, 6, 8, 10},
    {-4, -7, -8, -11, 3, 6, 7, 10}, {-3, -5, -8, -11, 2, 4, 7, 10},
    {-2, -6, -8, -10, 1, 5, 7, 9},  {-2, -5, -8, -10, 1, 4, 7, 9},
    {-2, -4, -8, -10, 1, 3, 7, 9},  {-2, -5, -7, -10, 1, 4, 6, 9},
    {-3, -4, -7, -10, 2, 3, 6, 9},  {-1, -2, -3, -10, 0, 1, 2, 9},
    {-4, -6, -8, -9, 3, 5, 7, 8},   {-3, -5, -7, -9, 2, 4, 6, 8}};

static const int32_t tc_eac_alpha_range[16] = {
    0x100FF / (1 + 14 - -15), 0x100FF / (1 + 12 - -13),
    0x100FF / (1 + 12 - -13), 0x100FF / (1 + 12 - -13),
    0x100FF / (1 + 11 - -12), 0x100FF / (1 + 10 - -11),
    0x100FF / (1 + 10 - -11), 0x100FF / (1 + 10 - -11),
    0x100FF / (1 + 9 - -10),  0x100FF / (1 + 9 - -10),
    0x100FF / (1 + 9 - -10),  0x100FF / (1 + 9 - -10),
    0x100FF / (1 + 9 - -10),  0x100FF / (1 + 9 - -10),
    0x100FF / (1 + 8 - -9),   0x100FF / (1 + 8 - -9)};

uint64_t tc_encode_eac_alpha(const uint8_t alpha[16]) {
    uint32_t i, j, tab;
    uint8_t minv = 255, maxv = 0, mid;
    int32_t range, best_err = INT_MAX, best_tab = 0, best_mul = 0;
    uint8_t best_sel[16];

    for (i = 0; i < 16u; ++i) {
        if (alpha[i] < minv) minv = alpha[i];
        if (alpha[i] > maxv) maxv = alpha[i];
    }
    if (minv == maxv) return minv;

    mid = (uint8_t)((minv + maxv) >> 1);
    range = (int32_t)maxv - (int32_t)minv;
    memset(best_sel, 0, sizeof(best_sel));
    for (tab = 0; tab < 16u; ++tab) {
        int32_t mul = ((range * tc_eac_alpha_range[tab]) >> 16) + 1;
        int32_t err = 0;
        uint8_t sel[16];
        for (i = 0; i < 16u; ++i) {
            int32_t local_best = INT_MAX;
            uint32_t local_sel = 0;
            for (j = 0; j < 8u; ++j) {
                int32_t rec = tc_clamp_i32((int32_t)mid + tc_eac_alpha[tab][j] * mul,
                                           0, 255);
                int32_t diff = (int32_t)alpha[i] - rec;
                int32_t e = diff * diff;
                if (e < local_best) {
                    local_best = e;
                    local_sel = j;
                }
            }
            sel[i] = (uint8_t)local_sel;
            err += local_best;
        }
        if (err < best_err) {
            best_err = err;
            best_tab = (int32_t)tab;
            best_mul = mul;
            memcpy(best_sel, sel, sizeof(best_sel));
            if (err == 0) break;
        }
    }

    {
        uint64_t d = ((uint64_t)mid << 56) | ((uint64_t)(best_mul & 15) << 52) |
                     ((uint64_t)(best_tab & 15) << 48);
        int shift = 45;
        for (i = 0; i < 16u; ++i) {
            d |= (uint64_t)best_sel[i] << shift;
            shift -= 3;
        }
        return tc_bswap64(d);
    }
}

tc_result tc_eac_compress_rgba8(const uint8_t *rgba, uint32_t width,
                                uint32_t height, size_t stride,
                                int rg11, uint8_t *out_eac, size_t out_size) {
    uint32_t bx, by, x, y, xx, yy;
    uint8_t red[16], green[16];
    size_t need, off = 0;

    if (!rgba || !out_eac || !width || !height) return TC_ERROR_INVALID_ARGUMENT;
    if (stride < (size_t)width * 4u) return TC_ERROR_INVALID_ARGUMENT;
    need = rg11 ? tc_eac_rg11_compressed_size(width, height)
                : tc_eac_r11_compressed_size(width, height);
    if (!need || out_size < need) return TC_ERROR_INVALID_ARGUMENT;

    for (by = 0; by < height; by += 4u) {
        for (bx = 0; bx < width; bx += 4u) {
            for (yy = 0; yy < 4u; ++yy) {
                y = by + yy;
                if (y >= height) y = height - 1u;
                for (xx = 0; xx < 4u; ++xx) {
                    const uint8_t *src;
                    x = bx + xx;
                    if (x >= width) x = width - 1u;
                    src = rgba + (size_t)y * stride + (size_t)x * 4u;
                    red[yy * 4u + xx] = src[0];
                    green[yy * 4u + xx] = src[1];
                }
            }
            tc_wr_u64(out_eac + off, tc_encode_eac_alpha(red));
            off += 8u;
            if (rg11) {
                tc_wr_u64(out_eac + off, tc_encode_eac_alpha(green));
                off += 8u;
            }
        }
    }

    return TC_SUCCESS;
}
