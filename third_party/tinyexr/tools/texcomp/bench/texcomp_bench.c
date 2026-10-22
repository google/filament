/*
 * TinyEXR texcomp benchmark.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "texcomp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now_sec(void) { return (double)clock() / (double)CLOCKS_PER_SEC; }

static void bench_astc_ise_pow2(void) {
    static const unsigned int qlevels[8] = {0, 2, 5, 8, 11, 14, 17, 20};
    static const unsigned int qcounts[8] = {2, 4, 8, 16, 32, 64, 128, 256};
    uint8_t values[64];
    uint8_t out[128];
    unsigned int qi, i;
    int iters = 200000;

    for (qi = 0; qi < 8u; ++qi) {
        double t0, t1, mvals;
        unsigned int q = qlevels[qi];
        unsigned int maxv = qcounts[qi] - 1u;
        for (i = 0; i < 64u; ++i) values[i] = (uint8_t)((i * 17u + qi) & maxv);

        t0 = now_sec();
        for (i = 0; i < (unsigned int)iters; ++i) {
            memset(out, 0, sizeof(out));
            if (tc_astc_ise_encode_bits(q, 64u, values, out, sizeof(out), 0) !=
                TC_SUCCESS) {
                return;
            }
        }
        t1 = now_sec();
        mvals = (64.0 * (double)iters) / ((t1 - t0) * 1000000.0);
        printf("texcomp astc ise quant%u: %.2f Mvals/s (64 x %d)\n", qcounts[qi],
               mvals, iters);
    }
}

static int bench_astc_4x4_backend(const char *name, uint32_t mask,
                                  const uint8_t *rgba, uint32_t w, uint32_t h,
                                  uint8_t *bc, size_t bc_size, int iters) {
    tc_astc_options astc_opt;
    size_t i, n = (size_t)w * (size_t)h;
    double t0, t1, mpix;
    tc_astc_options_init(&astc_opt);
    astc_opt.block_x = 4;
    astc_opt.block_y = 4;
    tc_backend_force_mask(mask);
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_astc_compress_rgba8(rgba, w, h, (size_t)w * 4u, &astc_opt, bc,
                                   bc_size) != TC_SUCCESS) {
            tc_backend_force_mask(TC_BACKEND_ALL);
            return 1;
        }
    }
    t1 = now_sec();
    tc_backend_force_mask(TC_BACKEND_ALL);
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp astc const 4x4 %s: %.2f MPix/s (%ux%u x %d)\n", name, mpix,
           w, h, iters);
    return 0;
}

int main(int argc, char **argv) {
    uint32_t w = 512, h = 512;
    int iters = 5;
    uint8_t *rgba, *rgba_solid, *bc;
    float *rgbf;
    size_t i, n, bc_size;
    double t0, t1, mpix;
    tc_bc7_options bc7_opt;
    tc_bc5_options bc5_opt;
    tc_bc6h_options bc6h_opt;
    tc_etc2_options etc2_opt;
    tc_astc_options astc_opt;
    (void)argc;
    (void)argv;
    n = (size_t)w * (size_t)h;
    rgba = (uint8_t *)malloc(n * 4u);
    rgba_solid = (uint8_t *)malloc(n * 4u);
    rgbf = (float *)malloc(n * 3u * sizeof(float));
    bc_size = tc_bc7_compressed_size(w, h);
    bc = (uint8_t *)malloc(bc_size);
    if (!rgba || !rgba_solid || !rgbf || !bc) {
        free(rgba);
        free(rgba_solid);
        free(rgbf);
        free(bc);
        return 1;
    }
    for (i = 0; i < n; ++i) {
        rgba[i * 4u + 0u] = (uint8_t)(i * 13u);
        rgba[i * 4u + 1u] = (uint8_t)(i * 7u);
        rgba[i * 4u + 2u] = (uint8_t)(i * 3u);
        rgba[i * 4u + 3u] = 255u;
        rgba_solid[i * 4u + 0u] = 48u;
        rgba_solid[i * 4u + 1u] = 96u;
        rgba_solid[i * 4u + 2u] = 144u;
        rgba_solid[i * 4u + 3u] = 255u;
        rgbf[i * 3u + 0u] = (float)rgba[i * 4u + 0u] / 255.0f;
        rgbf[i * 3u + 1u] = (float)rgba[i * 4u + 1u] / 255.0f;
        rgbf[i * 3u + 2u] = (float)rgba[i * 4u + 2u] / 255.0f;
    }
    tc_bc7_options_init(&bc7_opt);
    tc_bc5_options_init(&bc5_opt);
    tc_bc6h_options_init(&bc6h_opt);
    tc_etc2_options_init(&etc2_opt);
    tc_astc_options_init(&astc_opt);
    printf("texcomp backend: %s\n", tc_backend_name());
    bc7_opt.quick = 1;
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_bc7_compress_rgba8(rgba, w, h, (size_t)w * 4u, &bc7_opt, bc,
                                  bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp bc7 quick scalar: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h,
           iters);
    bc7_opt.quick = 2;
    iters = 5;
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_bc7_compress_rgba8(rgba, w, h, (size_t)w * 4u, &bc7_opt, bc,
                                  bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp bc7 medium scalar: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h,
           iters);
    bc7_opt.quick = 0;
    iters = 1;
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_bc7_compress_rgba8(rgba, w, h, (size_t)w * 4u, &bc7_opt, bc,
                                  bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp bc7 exhaustive scalar: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h,
           iters);
    iters = 10;
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_bc5_compress_rgba8(rgba, w, h, (size_t)w * 4u, &bc5_opt, bc,
                                  bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp bc5 scalar: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h, iters);
    iters = 10;
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_etc2_compress_rgba8(rgba, w, h, (size_t)w * 4u, &etc2_opt, bc,
                                   bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp etc2 rgba scalar: %.2f MPix/s (%ux%u x %d)\n", mpix, w,
           h, iters);
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_eac_compress_rgba8(rgba, w, h, (size_t)w * 4u, 1, bc,
                                  bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp eac rg11 scalar: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h,
           iters);
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_astc_compress_rgba8(rgba, w, h, (size_t)w * 4u, &astc_opt, bc,
                                   bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp astc ldr 6x6 normal: %.2f MPix/s (%ux%u x %d)\n", mpix, w,
           h, iters);
    astc_opt.quality = 1;
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_astc_compress_rgba8(rgba, w, h, (size_t)w * 4u, &astc_opt, bc,
                                   bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp astc ldr 6x6 medium: %.2f MPix/s (%ux%u x %d)\n", mpix, w,
           h, iters);
    astc_opt.quality = 0;
    t0 = now_sec();
    for (i = 0; i < (size_t)iters; ++i) {
        if (tc_astc_compress_rgba8(rgba, w, h, (size_t)w * 4u, &astc_opt, bc,
                                   bc_size) != TC_SUCCESS) {
            return 1;
        }
    }
    t1 = now_sec();
    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
    printf("texcomp astc ldr 6x6 fast: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h,
           iters);
    astc_opt.quality = 2;
    if (bench_astc_4x4_backend("scalar", TC_BACKEND_SCALAR, rgba_solid, w, h, bc,
                               bc_size, iters)) return 1;
    if (tc_backend_available_mask() & TC_BACKEND_SSE2) {
        if (bench_astc_4x4_backend("sse2", TC_BACKEND_SSE2, rgba_solid, w, h, bc,
                                   bc_size, iters)) return 1;
    }
    if (tc_backend_available_mask() & TC_BACKEND_SSE41) {
        if (bench_astc_4x4_backend("sse4.1", TC_BACKEND_SSE41, rgba_solid, w, h, bc,
                                   bc_size, iters)) return 1;
    }
    if (tc_backend_available_mask() & TC_BACKEND_AVX2) {
        if (bench_astc_4x4_backend("avx2", TC_BACKEND_AVX2, rgba_solid, w, h, bc,
                                   bc_size, iters)) return 1;
    }
    bench_astc_ise_pow2();

    {
        tc_bc1_options bc1_opt;
        tc_bc3_options bc3_opt;
        tc_astc_hdr_options hdr_opt;
        tc_bc1_options_init(&bc1_opt);
        tc_bc3_options_init(&bc3_opt);
        tc_astc_hdr_options_init(&hdr_opt);
        iters = 10;
        t0 = now_sec();
        for (i = 0; i < (size_t)iters; ++i)
            if (tc_bc1_compress_rgba8(rgba, w, h, (size_t)w * 4u, &bc1_opt, bc,
                                      bc_size) != TC_SUCCESS)
                return 1;
        t1 = now_sec();
        mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
        printf("texcomp bc1 scalar: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h,
               iters);
        t0 = now_sec();
        for (i = 0; i < (size_t)iters; ++i)
            if (tc_bc3_compress_rgba8(rgba, w, h, (size_t)w * 4u, &bc3_opt, bc,
                                      bc_size) != TC_SUCCESS)
                return 1;
        t1 = now_sec();
        mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
        printf("texcomp bc3 scalar: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h,
               iters);
        iters = 5;
        t0 = now_sec();
        for (i = 0; i < (size_t)iters; ++i)
            if (tc_astc_hdr_compress_rgbf(rgbf, w, h, (size_t)w * 3u * sizeof(float),
                                          &hdr_opt, bc, bc_size) != TC_SUCCESS)
                return 1;
        t1 = now_sec();
        mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
        printf("texcomp astc hdr 4x4: %.2f MPix/s (%ux%u x %d)\n", mpix, w, h,
               iters);
    }

    /* BC6H: benched per SIMD backend (its selector search is vectorized). */
    {
        static const uint32_t bmask[3] = {TC_BACKEND_SCALAR, TC_BACKEND_SSE41,
                                          TC_BACKEND_AVX2};
        static const char *const bname[3] = {"scalar", "sse4.1", "avx2"};
        uint32_t avail = tc_backend_available_mask(), sgn, mi;
        iters = 8;
        for (sgn = 0; sgn < 2u; ++sgn) {
            bc6h_opt.signed_float = (int)sgn;
            for (mi = 0; mi < 3u; ++mi) {
                if (mi > 0u && !(avail & bmask[mi])) continue;
                tc_backend_force_mask(bmask[mi]);
                t0 = now_sec();
                for (i = 0; i < (size_t)iters; ++i)
                    if (tc_bc6h_compress_rgb32f(rgbf, w, h,
                                                (size_t)w * 3u * sizeof(float),
                                                &bc6h_opt, bc, bc_size) !=
                        TC_SUCCESS)
                        return 1;
                t1 = now_sec();
                mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
                printf("texcomp bc6h%s mode11 %s: %.2f MPix/s (%ux%u x %d)\n",
                       sgn ? " signed" : "", bname[mi], mpix, w, h, iters);
            }
            if (sgn == 0u)
                for (i = 0; i < n * 3u; ++i)
                    if (i & 1u) rgbf[i] = -rgbf[i];
        }
        tc_backend_force_mask(TC_BACKEND_ALL);
    }
    {
        /* C7 batch pipeline benchmark: ASTC LDR at quality=1 (batch path) with
         * and without AVX2. Uses a varied 512x512 image so the partition search /
         * decimation / endpoint trial pipeline actually runs. */
        uint32_t avail = tc_backend_available_mask();
        static const uint32_t bmasks[3] = {TC_BACKEND_SSE2,
                                           TC_BACKEND_SSE2 | TC_BACKEND_SSE41,
                                           TC_BACKEND_SSE2 | TC_BACKEND_SSE41 | TC_BACKEND_AVX2};
        static const char *const bnames[3] = {"sse2", "sse41", "avx2"};
        int bi;
        iters = 8;
        for (i = 0; i < n; ++i) {
            uint32_t xi = (uint32_t)(i % w), yi = (uint32_t)(i / w);
            rgba[i * 4u + 0u] = (uint8_t)((xi ^ yi) * 7u);
            rgba[i * 4u + 1u] = (uint8_t)((xi + yi * 3u) * 5u);
            rgba[i * 4u + 2u] = (uint8_t)((xi * 13u + yi * 37u) & 255u);
            rgba[i * 4u + 3u] = (uint8_t)((xi * 3u + yi * 7u) & 255u);
        }
        tc_astc_options_init(&astc_opt);
        astc_opt.block_x = 4;
        astc_opt.block_y = 4;
        astc_opt.quality = 1;
        bc_size = tc_astc_compressed_size(w, h, &astc_opt);
        {
            uint8_t *abc = (uint8_t *)malloc(bc_size);
            if (abc) {
                for (bi = 0; bi < 3; ++bi) {
                    if (bi > 0 && !(avail & bmasks[bi])) continue;
                    tc_backend_force_mask(bmasks[bi]);
                    t0 = now_sec();
                    for (i = 0; i < (size_t)iters; ++i)
                        if (tc_astc_compress_rgba8(rgba, w, h,
                                                   (size_t)w * 4u, &astc_opt,
                                                   abc, bc_size) != TC_SUCCESS)
                            break;
                    t1 = now_sec();
                    mpix = ((double)n * (double)iters) / ((t1 - t0) * 1000000.0);
                    printf("texcomp astc batch 4x4 q1 %s: %.2f MPix/s (%ux%u x %d)\n",
                           bnames[bi], mpix, w, h, iters);
                }
                tc_backend_force_mask(TC_BACKEND_ALL);
                free(abc);
            }
        }
    }
    {
        /* Quality comparison: quick vs medium vs exhaustive.
         * Uses the asakusa EXR image loaded as uint8 via the CLI path, or a
         * smooth diagonal gradient for a meaningful PSNR comparison. */
        static const int qlevels[3] = {1, 2, 0};
        static const char *const qnames[3] = {"quick", "medium", "exhaustive"};
        uint8_t *qimg = (uint8_t *)malloc(n * 4u);
        uint8_t *dec = (uint8_t *)malloc(n * 4u);
        int qi;
        if (qimg && dec) {
            uint32_t xi, yi;
            for (yi = 0; yi < h; ++yi)
                for (xi = 0; xi < w; ++xi) {
                    size_t j = ((size_t)yi * w + xi);
                    uint8_t r = (uint8_t)((xi * 255u) / (w - 1u));
                    uint8_t g = (uint8_t)((yi * 255u) / (h - 1u));
                    uint8_t b = (uint8_t)(((xi + yi) * 255u) / (w + h - 2u));
                    qimg[j * 4u + 0u] = r;
                    qimg[j * 4u + 1u] = g;
                    qimg[j * 4u + 2u] = b;
                    qimg[j * 4u + 3u] = 255u;
                }
            for (qi = 0; qi < 3; ++qi) {
                double sse = 0.0, psnr_val;
                uint32_t pi;
                bc7_opt.quick = qlevels[qi];
                if (tc_bc7_compress_rgba8(qimg, w, h, (size_t)w * 4u,
                                          &bc7_opt, bc, bc_size) != TC_SUCCESS)
                    continue;
                if (tc_bc7_decompress_rgba8(bc, w, h, (size_t)w * 4u,
                                            dec, n * 4u) != TC_SUCCESS)
                    continue;
                for (pi = 0; pi < n * 4u; ++pi) {
                    int d = (int)qimg[pi] - (int)dec[pi];
                    sse += (double)(d * d);
                }
                psnr_val = (sse > 0.0)
                    ? 10.0 * log10((255.0 * 255.0 * (double)n * 4.0) / sse)
                    : 99.0;
                printf("texcomp bc7 %s psnr: %.2f dB\n", qnames[qi], psnr_val);
            }
            free(qimg);
            free(dec);
        }
    }
    free(rgba);
    free(rgba_solid);
    free(rgbf);
    free(bc);
    return 0;
}
