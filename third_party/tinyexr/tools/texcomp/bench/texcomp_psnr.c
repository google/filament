/*
 * TinyEXR texcomp ASTC quality harness.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 *
 * Encodes synthetic images with the ASTC encoder, decodes them with the
 * reference decoder from test/astc_ref_decode.h, and prints a PSNR table.
 * Used to track encoder quality across changes; the unit tests enforce
 * per-configuration PSNR floors derived from this table.
 */

#include "texcomp.h"

#include "astc_ref_decode.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define IMG_W 128u
#define IMG_H 128u

static uint32_t psnr_rng_state;

static uint32_t psnr_rng(void) {
    psnr_rng_state = psnr_rng_state * 1664525u + 1013904223u;
    return psnr_rng_state >> 8;
}

/* Deterministic value-noise octaves, roughly photo-like statistics. */
static uint32_t psnr_lattice(uint32_t xi, uint32_t yi, uint32_t seed) {
    uint32_t h = xi * 374761393u + yi * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return (h ^ (h >> 16)) & 255u;
}

static uint32_t psnr_value_noise(uint32_t x, uint32_t y, uint32_t seed) {
    uint32_t total = 0, weight = 0, oct;
    for (oct = 0; oct < 4u; ++oct) {
        uint32_t cell = 32u >> oct;
        uint32_t xi = x / cell, yi = y / cell;
        uint32_t fx = (x % cell) * 256u / cell;
        uint32_t fy = (y % cell) * 256u / cell;
        uint32_t v00 = psnr_lattice(xi, yi, seed + oct);
        uint32_t v10 = psnr_lattice(xi + 1u, yi, seed + oct);
        uint32_t v01 = psnr_lattice(xi, yi + 1u, seed + oct);
        uint32_t v11 = psnr_lattice(xi + 1u, yi + 1u, seed + oct);
        uint32_t top = v00 * (256u - fx) + v10 * fx;
        uint32_t bot = v01 * (256u - fx) + v11 * fx;
        uint32_t v = (top * (256u - fy) + bot * fy) >> 16;
        uint32_t w = 8u >> oct;
        total += v * w;
        weight += w;
    }
    return total / weight;
}

typedef void (*fill_fn)(uint8_t *img, uint32_t w, uint32_t h);

static void fill_gradient(uint8_t *img, uint32_t w, uint32_t h) {
    uint32_t x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            uint8_t *p = img + ((size_t)y * w + x) * 4u;
            p[0] = (uint8_t)(x * 255u / (w - 1u));
            p[1] = (uint8_t)(y * 255u / (h - 1u));
            p[2] = (uint8_t)((x + y) * 255u / (w + h - 2u));
            p[3] = 255u;
        }
    }
}

static void fill_noise(uint8_t *img, uint32_t w, uint32_t h) {
    uint32_t x, y;
    psnr_rng_state = 12345u;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            uint8_t *p = img + ((size_t)y * w + x) * 4u;
            p[0] = (uint8_t)psnr_rng();
            p[1] = (uint8_t)psnr_rng();
            p[2] = (uint8_t)psnr_rng();
            p[3] = 255u;
        }
    }
}

static void fill_photo(uint8_t *img, uint32_t w, uint32_t h) {
    uint32_t x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            uint8_t *p = img + ((size_t)y * w + x) * 4u;
            p[0] = (uint8_t)psnr_value_noise(x, y, 1u);
            p[1] = (uint8_t)psnr_value_noise(x, y, 7u);
            p[2] = (uint8_t)psnr_value_noise(x, y, 13u);
            p[3] = 255u;
        }
    }
}

static void fill_clusters(uint8_t *img, uint32_t w, uint32_t h) {
    uint32_t x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            uint8_t *p = img + ((size_t)y * w + x) * 4u;
            if (((x / 3u) + (y / 3u)) & 1u) {
                p[0] = 220u;
                p[1] = 40u;
                p[2] = 30u;
            } else {
                p[0] = 20u;
                p[1] = 60u;
                p[2] = 200u;
            }
            p[3] = 255u;
        }
    }
}

static void fill_alpha_ramp(uint8_t *img, uint32_t w, uint32_t h) {
    uint32_t x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            uint8_t *p = img + ((size_t)y * w + x) * 4u;
            p[0] = (uint8_t)psnr_value_noise(x, y, 21u);
            p[1] = (uint8_t)psnr_value_noise(x, y, 33u);
            p[2] = (uint8_t)(x * 255u / (w - 1u));
            p[3] = (uint8_t)(y * 255u / (h - 1u));
        }
    }
}

int main(void) {
    static const struct {
        const char *name;
        fill_fn fill;
    } images[5] = {{"gradient", fill_gradient},
                   {"noise", fill_noise},
                   {"photo", fill_photo},
                   {"clusters", fill_clusters},
                   {"alpharamp", fill_alpha_ramp}};
    static const uint32_t bs[4][2] = {{4, 4}, {6, 6}, {8, 8}, {12, 12}};
    uint8_t *img = (uint8_t *)malloc((size_t)IMG_W * IMG_H * 4u);
    uint8_t *dec = (uint8_t *)malloc((size_t)IMG_W * IMG_H * 4u);
    uint8_t *blocks = NULL;
    size_t blocks_cap = 0;
    unsigned int ii, bi;
    int q, failures = 0;

    if (!img || !dec) {
        free(img);
        free(dec);
        return 1;
    }
    printf("texcomp astc quality (%ux%u, PSNR dB, reference decoder)\n", IMG_W,
           IMG_H);
    printf("%-10s %-6s %8s %8s %8s\n", "image", "block", "fast", "medium",
           "normal");
    for (ii = 0; ii < 5u; ++ii) {
        images[ii].fill(img, IMG_W, IMG_H);
        for (bi = 0; bi < 4u; ++bi) {
            double psnr[3] = {0.0, 0.0, 0.0};
            for (q = 0; q <= 2; ++q) {
                tc_astc_options opt;
                size_t need;
                uint64_t sse = 0;
                size_t k, n = (size_t)IMG_W * IMG_H * 4u;
                double mse;
                tc_astc_options_init(&opt);
                opt.block_x = bs[bi][0];
                opt.block_y = bs[bi][1];
                opt.quality = q;
                need = tc_astc_compressed_size(IMG_W, IMG_H, &opt);
                if (need > blocks_cap) {
                    free(blocks);
                    blocks = (uint8_t *)malloc(need);
                    blocks_cap = need;
                    if (!blocks) return 1;
                }
                if (tc_astc_compress_rgba8(img, IMG_W, IMG_H,
                                           (size_t)IMG_W * 4u, &opt, blocks,
                                           need) != TC_SUCCESS) {
                    printf("%-10s %ux%-4u q%d ENCODE FAIL\n", images[ii].name,
                           bs[bi][0], bs[bi][1], q);
                    ++failures;
                    continue;
                }
                if (!aref_decode_image(blocks, IMG_W, IMG_H, opt.block_x,
                                       opt.block_y, dec)) {
                    printf("%-10s %ux%-4u q%d DECODE FAIL\n", images[ii].name,
                           bs[bi][0], bs[bi][1], q);
                    ++failures;
                    continue;
                }
                for (k = 0; k < n; ++k) {
                    int d = (int)img[k] - (int)dec[k];
                    sse += (uint64_t)(d * d);
                }
                mse = (double)sse / (double)n;
                psnr[q] = mse > 0.0 ? 10.0 * log10(65025.0 / mse) : 99.0;
            }
            printf("%-10s %ux%-4u %8.2f %8.2f %8.2f\n", images[ii].name,
                   bs[bi][0], bs[bi][1], psnr[0], psnr[1], psnr[2]);
        }
    }
    free(img);
    free(dec);
    free(blocks);
    return failures ? 1 : 0;
}
