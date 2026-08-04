/*
 * TinyEXR texpipe - displacement + atlas sampling helpers.
 *
 *  - Min-max height pyramid: conservative (min,max) height bounds per mip for
 *    parallax-occlusion / relief / cone-step mapping.
 *  - Gutter dilation: flood valid texels into invalid ones so mips don't bleed
 *    background across atlas/lightmap chart borders.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "texpipe_internal.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------- min-max pyramid */

tp_result tp_build_minmax_pyramid(const tir_allocator *a,
                                  const tir_image_view *height, int channel,
                                  int max_levels, tp_mip_chain *out) {
    int num_levels, level, x, y;
    if (!height || !out || channel < 0 || channel >= height->channels)
        return TP_ERROR_INVALID_ARGUMENT;
    if (height->width < 1 || height->height < 1) return TP_ERROR_INVALID_ARGUMENT;
    num_levels = tp_level_count(height->width, height->height, max_levels);
    memset(out, 0, sizeof(*out));
    out->num_faces = 1;
    out->num_levels = num_levels;
    out->channels = 2;
    out->level = (tp_surface *)tp_alloc(a, (size_t)num_levels * sizeof(tp_surface));
    if (!out->level) return TP_ERROR_OUT_OF_MEMORY;
    memset(out->level, 0, (size_t)num_levels * sizeof(tp_surface));

    for (level = 0; level < num_levels; ++level) {
        tp_surface *s = &out->level[level];
        int w = tp_level_dim(height->width, level);
        int h = tp_level_dim(height->height, level);
        s->width = w; s->height = h; s->channels = 2;
        s->stride = (size_t)w * 2 * sizeof(float);
        s->data = (float *)tp_alloc(a, s->stride * (size_t)h);
        if (!s->data) { tp_mip_chain_free(a, out); return TP_ERROR_OUT_OF_MEMORY; }
        if (level == 0) {
            for (y = 0; y < h; ++y)
                for (x = 0; x < w; ++x) {
                    float hv = tp_read_view_sample(height, x, y, channel);
                    float *d = s->data + (y * w + x) * 2;
                    d[0] = hv; d[1] = hv;
                }
        } else {
            const tp_surface *p = &out->level[level - 1];
            int pw = p->width, ph = p->height, dx, dy;
            for (y = 0; y < h; ++y)
                for (x = 0; x < w; ++x) {
                    float mn = 1e30f, mx = -1e30f;
                    for (dy = 0; dy < 2; ++dy)
                        for (dx = 0; dx < 2; ++dx) {
                            int cx = 2 * x + dx, cy = 2 * y + dy;
                            const float *c;
                            if (cx >= pw) cx = pw - 1;
                            if (cy >= ph) cy = ph - 1;
                            c = p->data + (cy * pw + cx) * 2;
                            if (c[0] < mn) mn = c[0];
                            if (c[1] > mx) mx = c[1];
                        }
                    {
                        float *d = s->data + (y * w + x) * 2;
                        d[0] = mn; d[1] = mx;
                    }
                }
        }
    }
    return TP_SUCCESS;
}

/* ------------------------------------------------------- cone-step map */

tp_result tp_build_cone_map(const tir_allocator *a, const tir_image_view *height,
                            int channel, tp_surface *out) {
    int W, H, x, y, qx, qy;
    float *hf;
    if (!height || !out || channel < 0 || channel >= height->channels)
        return TP_ERROR_INVALID_ARGUMENT;
    W = height->width; H = height->height;
    if (W < 1 || H < 1) return TP_ERROR_INVALID_ARGUMENT;
    hf = (float *)tp_alloc(a, (size_t)W * H * sizeof(float));
    if (!hf) return TP_ERROR_OUT_OF_MEMORY;
    for (y = 0; y < H; ++y)
        for (x = 0; x < W; ++x)
            hf[y * W + x] = tp_read_view_sample(height, x, y, channel);

    out->width = W; out->height = H; out->channels = 1;
    out->stride = (size_t)W * sizeof(float);
    out->data = (float *)tp_alloc(a, (size_t)W * H * sizeof(float));
    if (!out->data) { tp_dealloc(a, hf); return TP_ERROR_OUT_OF_MEMORY; }

    for (y = 0; y < H; ++y)
        for (x = 0; x < W; ++x) {
            float hp = hf[y * W + x], cone = 1.0f;
            for (qy = 0; qy < H; ++qy)
                for (qx = 0; qx < W; ++qx) {
                    float dh = hf[qy * W + qx] - hp;
                    float dx, dy, dist, ratio;
                    if (dh <= 0.0f) continue;
                    dx = (float)(qx - x) / (float)W;
                    dy = (float)(qy - y) / (float)H;
                    dist = sqrtf(dx * dx + dy * dy);
                    ratio = dist / dh; /* 45-degree cone -> ratio 1 */
                    if (ratio < cone) cone = ratio;
                }
            if (cone < 0.0f) cone = 0.0f;
            if (cone > 1.0f) cone = 1.0f;
            out->data[y * W + x] = cone;
        }
    tp_dealloc(a, hf);
    return TP_SUCCESS;
}

/* ------------------------------------------------------- ripmap */

tp_result tp_build_ripmap(const tir_allocator *a, const tir_image_view *base,
                          const tp_options *opt, tp_mip_chain *out, int *out_nx,
                          int *out_ny) {
    int nx, ny, ix, jy, channels;
    tir_options topt;
    if (!base || !out) return TP_ERROR_INVALID_ARGUMENT;
    channels = base->channels;
    nx = tp_level_count(base->width, 1, 0);
    ny = tp_level_count(base->height, 1, 0);
    memset(out, 0, sizeof(*out));
    out->num_faces = 1;
    out->num_levels = nx * ny;
    out->channels = channels;
    out->level = (tp_surface *)tp_alloc(a, (size_t)nx * ny * sizeof(tp_surface));
    if (!out->level) return TP_ERROR_OUT_OF_MEMORY;
    memset(out->level, 0, (size_t)nx * ny * sizeof(tp_surface));
    tir_options_init(&topt);
    if (opt) { topt.filter_x = opt->filter; topt.filter_y = opt->filter; topt.edge_x = opt->edge_x; topt.edge_y = opt->edge_y; }

    for (jy = 0; jy < ny; ++jy)
        for (ix = 0; ix < nx; ++ix) {
            tp_surface *s = &out->level[jy * nx + ix];
            int w = tp_level_dim(base->width, ix);
            int h = tp_level_dim(base->height, jy);
            tir_image_view dstv;
            s->width = w; s->height = h; s->channels = channels;
            s->stride = (size_t)w * channels * sizeof(float);
            s->data = (float *)tp_alloc(a, s->stride * (size_t)h);
            if (!s->data) { tp_mip_chain_free(a, out); return TP_ERROR_OUT_OF_MEMORY; }
            dstv.data = s->data; dstv.width = w; dstv.height = h;
            dstv.channels = channels; dstv.type = TIR_F32; dstv.row_stride_bytes = s->stride;
            if (ix == 0 && jy == 0) {
                int xx, yy, c;
                for (yy = 0; yy < h; ++yy)
                    for (xx = 0; xx < w; ++xx)
                        for (c = 0; c < channels; ++c)
                            s->data[(yy * w + xx) * channels + c] = tp_read_view_sample(base, xx, yy, c);
            } else if (!TIR_OK(tir_resize(a, base, &dstv, &topt))) {
                tp_mip_chain_free(a, out);
                return TP_ERROR_UNSUPPORTED;
            }
        }
    if (out_nx) *out_nx = nx;
    if (out_ny) *out_ny = ny;
    return TP_SUCCESS;
}

/* ------------------------------------------------------- YCoCg */

void tp_rgb_to_ycocg(const float rgb[3], float ycocg[3]) {
    ycocg[0] = 0.25f * rgb[0] + 0.5f * rgb[1] + 0.25f * rgb[2];
    ycocg[1] = 0.5f * rgb[0] - 0.5f * rgb[2] + 0.5f;            /* Co + bias */
    ycocg[2] = -0.25f * rgb[0] + 0.5f * rgb[1] - 0.25f * rgb[2] + 0.5f; /* Cg */
}

void tp_ycocg_to_rgb(const float ycocg[3], float rgb[3]) {
    float y = ycocg[0], co = ycocg[1] - 0.5f, cg = ycocg[2] - 0.5f;
    rgb[0] = y + co - cg;
    rgb[1] = y + cg;
    rgb[2] = y - co - cg;
}

/* ------------------------------------------------------- gutter dilation */

tp_result tp_dilate(tp_surface *s, int vc, float threshold, int iters) {
    int W, H, ch, x, y, c, pass;
    uint8_t *valid, *nv;
    float *snap;
    if (!s || !s->data || vc < 0 || vc >= s->channels) return TP_ERROR_INVALID_ARGUMENT;
    W = s->width; H = s->height; ch = s->channels;
    if (iters < 1) return TP_SUCCESS;
    valid = (uint8_t *)tp_alloc(NULL, (size_t)W * H);
    nv = (uint8_t *)tp_alloc(NULL, (size_t)W * H);
    snap = (float *)tp_alloc(NULL, (size_t)W * H * ch * sizeof(float));
    if (!valid || !nv || !snap) { tp_dealloc(NULL, valid); tp_dealloc(NULL, nv); tp_dealloc(NULL, snap); return TP_ERROR_OUT_OF_MEMORY; }
    for (y = 0; y < H; ++y) {
        const float *row = (const float *)((const uint8_t *)s->data + (size_t)y * s->stride);
        for (x = 0; x < W; ++x)
            valid[y * W + x] = row[x * ch + vc] >= threshold ? 1u : 0u;
    }

    for (pass = 0; pass < iters; ++pass) {
        int changed = 0, i;
        /* Snapshot pixels; neighbour validity is frozen at the start of the pass
         * (fills only become valid next pass) so background can't propagate. */
        for (y = 0; y < H; ++y) {
            const float *row = (const float *)((const uint8_t *)s->data + (size_t)y * s->stride);
            memcpy(snap + (size_t)y * W * ch, row, (size_t)W * ch * sizeof(float));
        }
        memset(nv, 0, (size_t)W * H);
        for (y = 0; y < H; ++y) {
            float *row = (float *)((uint8_t *)s->data + (size_t)y * s->stride);
            for (x = 0; x < W; ++x) {
                int nx, ny, cnt = 0;
                float acc[4] = {0, 0, 0, 0};
                int off[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}, k;
                if (valid[y * W + x]) continue;
                for (k = 0; k < 4; ++k) {
                    nx = x + off[k][0]; ny = y + off[k][1];
                    if (nx < 0 || nx >= W || ny < 0 || ny >= H) continue;
                    if (!valid[ny * W + nx]) continue;
                    for (c = 0; c < ch && c < 4; ++c)
                        acc[c] += snap[((size_t)ny * W + nx) * ch + c];
                    ++cnt;
                }
                if (cnt > 0) {
                    for (c = 0; c < ch && c < 4; ++c)
                        if (c != vc) row[x * ch + c] = acc[c] / (float)cnt;
                    nv[y * W + x] = 1u;
                    changed = 1;
                }
            }
        }
        for (i = 0; i < W * H; ++i) if (nv[i]) valid[i] = 1u;
        if (!changed) break;
    }
    tp_dealloc(NULL, valid);
    tp_dealloc(NULL, nv);
    tp_dealloc(NULL, snap);
    return TP_SUCCESS;
}
