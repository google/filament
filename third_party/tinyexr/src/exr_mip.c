/*
 * TinyEXR - mipmap pyramid generation for tiled writing.
 *
 * Generates downsampled levels (2x2 box filter) from the level-0 image so the
 * writer can emit MIPMAP_LEVELS tiled files. Only x/y sampling == 1 is handled
 * (subsampled mipmaps are uncommon).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"


static int level_dim_step(int s, int up) {
    int n = up ? (s + 1) / 2 : s / 2;
    return n < 1 ? 1 : n;
}

/* Box downsample of one channel from (sw,sh) to (dw,dh) for the output rows
 * [y0,y1). Each destination pixel averages the source span
 * [x*sw/dw, (x+1)*sw/dw) x [y*sh/dh,...); handles 2x2 (mipmap) and separable
 * 2x1/1x2 (ripmap) uniformly. Output rows are disjoint, so the row range can be
 * split across threads. */
static void downsample_rows(const uint8_t *src, int sw, int sh,
                            exr_pixel_type pt, uint8_t *dst, int dw, int dh,
                            int y0, int y1) {
    int x, y, sx, sy;
    (void)dh;
    for (y = y0; y < y1; ++y) {
        int sy0 = (int)((int64_t)y * sh / dh);
        int sy1 = (int)((int64_t)(y + 1) * sh / dh);
        if (sy1 <= sy0) sy1 = sy0 + 1;
        if (sy1 > sh) sy1 = sh;
        for (x = 0; x < dw; ++x) {
            int sx0 = (int)((int64_t)x * sw / dw);
            int sx1 = (int)((int64_t)(x + 1) * sw / dw);
            size_t d = (size_t)y * dw + x;
            uint32_t count;
            if (sx1 <= sx0) sx1 = sx0 + 1;
            if (sx1 > sw) sx1 = sw;
            count = (uint32_t)(sx1 - sx0) * (uint32_t)(sy1 - sy0);
            if (pt == EXR_PIXEL_HALF) {
                const uint16_t *s = (const uint16_t *)src;
                float sum = 0.0f, avg;
                uint16_t r;
                for (sy = sy0; sy < sy1; ++sy)
                    for (sx = sx0; sx < sx1; ++sx) {
                        float f;
                        uint16_t hv = s[(size_t)sy * sw + sx];
                        exr_half_to_float(&hv, &f, 1);
                        sum += f;
                    }
                avg = sum / (float)count;
                exr_float_to_half(&avg, &r, 1);
                ((uint16_t *)dst)[d] = r;
            } else if (pt == EXR_PIXEL_FLOAT) {
                const float *s = (const float *)src;
                float sum = 0.0f;
                for (sy = sy0; sy < sy1; ++sy)
                    for (sx = sx0; sx < sx1; ++sx)
                        sum += s[(size_t)sy * sw + sx];
                ((float *)dst)[d] = sum / (float)count;
            } else { /* UINT */
                const uint32_t *s = (const uint32_t *)src;
                uint64_t sum = 0;
                for (sy = sy0; sy < sy1; ++sy)
                    for (sx = sx0; sx < sx1; ++sx)
                        sum += s[(size_t)sy * sw + sx];
                ((uint32_t *)dst)[d] = (uint32_t)((sum + count / 2) / count);
            }
        }
    }
}

/* Row-range job for the parallel downsample. */
typedef struct {
    const uint8_t *src;
    uint8_t *dst;
    int sw, sh, dw, dh;
    exr_pixel_type pt;
    int chunk; /* rows per job */
} ds_ctx;

static void ds_job(void *ctx, int job) {
    ds_ctx *d = (ds_ctx *)ctx;
    int y0 = job * d->chunk;
    int y1 = y0 + d->chunk;
    if (y1 > d->dh) y1 = d->dh;
    downsample_rows(d->src, d->sw, d->sh, d->pt, d->dst, d->dw, d->dh, y0, y1);
}

/* Box downsample one channel; rows are split across worker threads when the
 * caller has enabled them and the output is tall enough to be worth it. */
static void downsample(const uint8_t *src, int sw, int sh, exr_pixel_type pt,
                       uint8_t *dst, int dw, int dh) {
    int nt = exr_get_num_threads();
    if (nt > 1 && dh >= 64) {
        ds_ctx d;
        int chunk = (dh + nt * 4 - 1) / (nt * 4); /* ~4 chunks per worker */
        int njobs;
        if (chunk < 1) chunk = 1;
        njobs = (dh + chunk - 1) / chunk;
        d.src = src; d.dst = dst; d.sw = sw; d.sh = sh; d.dw = dw; d.dh = dh;
        d.pt = pt; d.chunk = chunk;
        exr_simd_init();                  /* warm half<->float before threads */
        exr_parallel_for(nt, njobs, ds_job, &d);
    } else {
        downsample_rows(src, sw, sh, pt, dst, dw, dh, 0, dh);
    }
}

exr_result exr_mip_generate(const exr_allocator *a, const exr_part *part,
                            int round_up, exr_mip_pyramid *out) {
    int w = part->width, h = part->height, nch = part->header.num_channels;
    int c, levels = 0, ww = w, hh = h, L;
    exr_result rc = EXR_SUCCESS;

    memset(out, 0, sizeof(*out));
    out->alloc = *a;
    out->num_channels = nch;

    for (;;) {
        levels++;
        if (ww <= 1 && hh <= 1) break;
        ww = level_dim_step(ww, round_up);
        hh = level_dim_step(hh, round_up);
    }
    out->num_levels = levels;
    out->lw = (int *)exr_calloc(a, (size_t)levels, sizeof(int));
    out->lh = (int *)exr_calloc(a, (size_t)levels, sizeof(int));
    out->img = (void ***)exr_calloc(a, (size_t)levels, sizeof(void **));
    if (!out->lw || !out->lh || !out->img) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }

    ww = w;
    hh = h;
    for (L = 0; L < levels; ++L) {
        out->lw[L] = ww;
        out->lh[L] = hh;
        out->img[L] = (void **)exr_calloc(a, nch ? (size_t)nch : 1, sizeof(void *));
        if (!out->img[L]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        for (c = 0; c < nch; ++c) {
            const exr_channel *ch = &part->header.channels[c];
            int xs = ch->x_sampling < 1 ? 1 : ch->x_sampling;
            int ys = ch->y_sampling < 1 ? 1 : ch->y_sampling;
            size_t ps = exr_pixel_size(ch->pixel_type);
            int cw = exr_num_samples(0, ww - 1, xs); /* channel-grid dims */
            int chh = exr_num_samples(0, hh - 1, ys);
            size_t bytes;
            if (cw < 0 || chh < 0 ||
                exr_mul_ovf((size_t)cw, (size_t)chh, &bytes) ||
                exr_mul_ovf(bytes, ps, &bytes)) {
                rc = EXR_ERROR_CORRUPT; goto fail;
            }
            out->img[L][c] = exr_malloc(a, bytes ? bytes : 1);
            if (!out->img[L][c]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
            if (L == 0) {
                memcpy(out->img[L][c], part->images[c], bytes);
            } else {
                int pcw = exr_num_samples(0, out->lw[L - 1] - 1, xs);
                int pch = exr_num_samples(0, out->lh[L - 1] - 1, ys);
                downsample((const uint8_t *)out->img[L - 1][c], pcw, pch,
                           ch->pixel_type, (uint8_t *)out->img[L][c], cw, chh);
            }
        }
        if (ww <= 1 && hh <= 1) break;
        ww = level_dim_step(ww, round_up);
        hh = level_dim_step(hh, round_up);
    }
    return EXR_SUCCESS;

fail:
    exr_mip_free(out);
    return rc;
}

void exr_mip_free(exr_mip_pyramid *pyr) {
    const exr_allocator *a;
    int L, c;
    if (!pyr || !pyr->img) {
        if (pyr) { exr_free(&pyr->alloc, pyr->lw); exr_free(&pyr->alloc, pyr->lh); }
        return;
    }
    a = &pyr->alloc;
    for (L = 0; L < pyr->num_levels; ++L) {
        if (pyr->img[L]) {
            for (c = 0; c < pyr->num_channels; ++c) exr_free(a, pyr->img[L][c]);
            exr_free(a, pyr->img[L]);
        }
    }
    exr_free(a, pyr->img);
    exr_free(a, pyr->lw);
    exr_free(a, pyr->lh);
    memset(pyr, 0, sizeof(*pyr));
}

static int dim_num_levels(int s, int up) {
    int n = 1;
    while (s > 1) { s = level_dim_step(s, up); n++; }
    return n;
}

exr_result exr_ripmap_generate(const exr_allocator *a, const exr_part *part,
                               int round_up, exr_ripmap_pyramid *out) {
    int w = part->width, h = part->height, nch = part->header.num_channels;
    int c, lx, ly, nxl, nyl, s;
    exr_result rc = EXR_SUCCESS;

    memset(out, 0, sizeof(*out));
    out->alloc = *a;
    out->num_channels = nch;

    nxl = dim_num_levels(w, round_up);
    nyl = dim_num_levels(h, round_up);
    out->num_x_levels = nxl;
    out->num_y_levels = nyl;
    out->lw = (int *)exr_calloc(a, (size_t)nxl, sizeof(int));
    out->lh = (int *)exr_calloc(a, (size_t)nyl, sizeof(int));
    out->img = (void ***)exr_calloc(a, (size_t)nxl * nyl, sizeof(void **));
    if (!out->lw || !out->lh || !out->img) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    for (lx = 0, s = w; lx < nxl; ++lx) { out->lw[lx] = s; s = level_dim_step(s, round_up); }
    for (ly = 0, s = h; ly < nyl; ++ly) { out->lh[ly] = s; s = level_dim_step(s, round_up); }

    /* Allocate every (lx,ly) level image. */
    for (lx = 0; lx < nxl; ++lx) {
        for (ly = 0; ly < nyl; ++ly) {
            void **slot = (void **)exr_calloc(a, nch ? (size_t)nch : 1, sizeof(void *));
            out->img[lx * nyl + ly] = slot;
            if (!slot) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
            for (c = 0; c < nch; ++c) {
                const exr_channel *ch = &part->header.channels[c];
                int xs = ch->x_sampling < 1 ? 1 : ch->x_sampling;
                int ys = ch->y_sampling < 1 ? 1 : ch->y_sampling;
                size_t ps = exr_pixel_size(ch->pixel_type);
                int rw = exr_num_samples(0, out->lw[lx] - 1, xs);
                int rh = exr_num_samples(0, out->lh[ly] - 1, ys);
                size_t bytes;
                if (rw < 0 || rh < 0 ||
                    exr_mul_ovf((size_t)rw, (size_t)rh, &bytes) ||
                    exr_mul_ovf(bytes, ps, &bytes)) {
                    rc = EXR_ERROR_CORRUPT; goto fail;
                }
                slot[c] = exr_malloc(a, bytes ? bytes : 1);
                if (!slot[c]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
            }
        }
    }
    /* Fill: (0,0) = level 0; x-chain at ly=0; then y-chain per lx. */
    for (c = 0; c < nch; ++c) {
        const exr_channel *ch = &part->header.channels[c];
        exr_pixel_type pt = ch->pixel_type;
        int xs = ch->x_sampling < 1 ? 1 : ch->x_sampling;
        int ys = ch->y_sampling < 1 ? 1 : ch->y_sampling;
        size_t ps = exr_pixel_size(pt);
#define GW(d) exr_num_samples(0, (d) - 1, xs)
#define GH(d) exr_num_samples(0, (d) - 1, ys)
        memcpy(out->img[0 * nyl + 0][c], part->images[c],
               (size_t)GW(w) * GH(h) * ps);
        for (lx = 1; lx < nxl; ++lx)
            downsample((const uint8_t *)out->img[(lx - 1) * nyl + 0][c],
                       GW(out->lw[lx - 1]), GH(out->lh[0]), pt,
                       (uint8_t *)out->img[lx * nyl + 0][c], GW(out->lw[lx]),
                       GH(out->lh[0]));
        for (lx = 0; lx < nxl; ++lx)
            for (ly = 1; ly < nyl; ++ly)
                downsample((const uint8_t *)out->img[lx * nyl + (ly - 1)][c],
                           GW(out->lw[lx]), GH(out->lh[ly - 1]), pt,
                           (uint8_t *)out->img[lx * nyl + ly][c], GW(out->lw[lx]),
                           GH(out->lh[ly]));
#undef GW
#undef GH
    }
    return EXR_SUCCESS;

fail:
    exr_ripmap_free(out);
    return rc;
}

void exr_ripmap_free(exr_ripmap_pyramid *pyr) {
    const exr_allocator *a;
    int i, c, n;
    if (!pyr || !pyr->img) {
        if (pyr) { exr_free(&pyr->alloc, pyr->lw); exr_free(&pyr->alloc, pyr->lh); }
        return;
    }
    a = &pyr->alloc;
    n = pyr->num_x_levels * pyr->num_y_levels;
    for (i = 0; i < n; ++i) {
        if (pyr->img[i]) {
            for (c = 0; c < pyr->num_channels; ++c) exr_free(a, pyr->img[i][c]);
            exr_free(a, pyr->img[i]);
        }
    }
    exr_free(a, pyr->img);
    exr_free(a, pyr->lw);
    exr_free(a, pyr->lh);
    memset(pyr, 0, sizeof(*pyr));
}
