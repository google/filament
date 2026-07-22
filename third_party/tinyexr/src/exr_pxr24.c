/*
 * TinyEXR - PXR24 codec (lossy 24-bit float): zlib inflate + byte-plane delta.
 *
 * Each channel-scanline is stored as byte planes with a running delta across
 * the row; FLOAT keeps the top 3 bytes (low mantissa byte dropped), HALF 2
 * bytes, UINT 4 bytes.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

static size_t pxr_bpc(exr_pixel_type t) {
    switch (t) {
    case EXR_PIXEL_UINT: return 4;
    case EXR_PIXEL_HALF: return 2;
    case EXR_PIXEL_FLOAT: return 3;
    }
    return 2;
}

exr_result exr_pxr24_decompress(const exr_codec_ctx *ctx, const uint8_t *src,
                                size_t src_size, uint8_t *dst, size_t dst_size) {
    const exr_allocator *a = ctx->alloc;
    int xmin = ctx->x, xmax = ctx->x + ctx->width - 1;
    int line, c;
    size_t inter = 0;
    uint8_t *buf;
    const uint8_t *in;
    uint8_t *out, *out_end;
    exr_result rc = EXR_SUCCESS;

    /* Pass 1: intermediate (planar, delta-coded) size. */
    for (line = 0; line < ctx->num_lines; ++line) {
        int yy = ctx->y + line;
        for (c = 0; c < ctx->num_channels; ++c) {
            int ys = ctx->channels[c].y_sampling, xs = ctx->channels[c].x_sampling;
            int w;
            size_t add;
            if (ys <= 0 || xs <= 0) return EXR_ERROR_CORRUPT;
            if ((yy % ys) != 0) continue;
            w = exr_num_samples(xmin, xmax, xs);
            if (w < 0) w = 0;
            if (exr_mul_ovf((size_t)w, pxr_bpc(ctx->channels[c].pixel_type), &add))
                return EXR_ERROR_CORRUPT;
            if (exr_add_ovf(inter, add, &inter)) return EXR_ERROR_CORRUPT;
        }
    }

    buf = (uint8_t *)exr_malloc(a, inter ? inter : 1);
    if (!buf) return EXR_ERROR_OUT_OF_MEMORY;

    if (src_size == inter) {
        memcpy(buf, src, inter);
    } else {
        size_t got = 0;
        rc = exr_zlib_inflate(src, src_size, buf, inter, &got);
        if (EXR_OK(rc) && got != inter) rc = EXR_ERROR_CORRUPT;
        if (!EXR_OK(rc)) {
            exr_free(a, buf);
            return rc;
        }
    }

    /* Pass 2: reconstruct pixels into the canonical block layout. */
    in = buf;
    out = dst;
    out_end = dst + dst_size;
    for (line = 0; line < ctx->num_lines; ++line) {
        int yy = ctx->y + line;
        for (c = 0; c < ctx->num_channels; ++c) {
            int ys = ctx->channels[c].y_sampling, xs = ctx->channels[c].x_sampling;
            int w, x;
            uint32_t pixel = 0;
            if ((yy % ys) != 0) continue;
            w = exr_num_samples(xmin, xmax, xs);
            if (w < 0) w = 0;
            switch (ctx->channels[c].pixel_type) {
            case EXR_PIXEL_UINT: {
                const uint8_t *p0 = in, *p1 = in + w, *p2 = in + 2 * w,
                              *p3 = in + 3 * w;
                in += (size_t)w * 4;
                for (x = 0; x < w; ++x) {
                    pixel += ((uint32_t)p0[x] << 24) | ((uint32_t)p1[x] << 16) |
                             ((uint32_t)p2[x] << 8) | (uint32_t)p3[x];
                    if (out + 4 > out_end) { rc = EXR_ERROR_CORRUPT; goto done; }
                    memcpy(out, &pixel, 4);
                    out += 4;
                }
                break;
            }
            case EXR_PIXEL_HALF: {
                const uint8_t *p0 = in, *p1 = in + w;
                in += (size_t)w * 2;
                for (x = 0; x < w; ++x) {
                    uint16_t h;
                    pixel += ((uint32_t)p0[x] << 8) | (uint32_t)p1[x];
                    h = (uint16_t)pixel;
                    if (out + 2 > out_end) { rc = EXR_ERROR_CORRUPT; goto done; }
                    memcpy(out, &h, 2);
                    out += 2;
                }
                break;
            }
            case EXR_PIXEL_FLOAT: {
                const uint8_t *p0 = in, *p1 = in + w, *p2 = in + 2 * w;
                in += (size_t)w * 3;
                for (x = 0; x < w; ++x) {
                    pixel += ((uint32_t)p0[x] << 24) | ((uint32_t)p1[x] << 16) |
                             ((uint32_t)p2[x] << 8);
                    if (out + 4 > out_end) { rc = EXR_ERROR_CORRUPT; goto done; }
                    memcpy(out, &pixel, 4);
                    out += 4;
                }
                break;
            }
            }
        }
    }

    if ((size_t)(out - dst) != dst_size) rc = EXR_ERROR_CORRUPT;

done:
    exr_free(a, buf);
    return rc;
}

/* float -> 24-bit (top 3 bytes, round-to-nearest). Inverse of the decode's
 * (p0<<24|p1<<16|p2<<8) reconstruction. */
static uint32_t float_to_float24(float f) {
    union { float f; uint32_t i; } u;
    uint32_t s, e, m, i;
    u.f = f;
    s = u.i & 0x80000000u;
    e = u.i & 0x7f800000u;
    m = u.i & 0x007fffffu;
    if (e == 0x7f800000u) {
        if (m) {
            m >>= 8;
            return (s >> 8) | (e >> 8) | m | (m == 0 ? 1u : 0u);
        }
        return (s >> 8) | (e >> 8);
    }
    i = ((e | m) + (m & 0x00000080u)) >> 8;
    if (i >= 0x7f8000u) i = (e | m) >> 8;
    return (s >> 8) | i;
}

exr_result exr_pxr24_compress(const exr_codec_ctx *ctx, const uint8_t *block,
                              size_t n, uint8_t **out_data, size_t *out_size) {
    const exr_allocator *a = ctx->alloc;
    int xmin = ctx->x, xmax = ctx->x + ctx->width - 1;
    int line, c;
    size_t inter = 0;
    uint8_t *buf, *comp = NULL;
    const uint8_t *in;
    uint8_t *op;
    size_t clen = 0;
    exr_result rc;

    *out_data = NULL;
    *out_size = 0;

    for (line = 0; line < ctx->num_lines; ++line) {
        int yy = ctx->y + line;
        for (c = 0; c < ctx->num_channels; ++c) {
            int ys = ctx->channels[c].y_sampling, xs = ctx->channels[c].x_sampling;
            int w;
            if (ys <= 0 || xs <= 0) return EXR_ERROR_CORRUPT;
            if ((yy % ys) != 0) continue;
            w = exr_num_samples(xmin, xmax, xs);
            if (w < 0) w = 0;
            {
                size_t add;
                if (exr_mul_ovf((size_t)w, pxr_bpc(ctx->channels[c].pixel_type),
                                &add) ||
                    exr_add_ovf(inter, add, &inter))
                    return EXR_ERROR_CORRUPT;
            }
        }
    }
    buf = (uint8_t *)exr_malloc(a, inter ? inter : 1);
    if (!buf) return EXR_ERROR_OUT_OF_MEMORY;

    in = block;
    op = buf;
    for (line = 0; line < ctx->num_lines; ++line) {
        int yy = ctx->y + line;
        for (c = 0; c < ctx->num_channels; ++c) {
            int ys = ctx->channels[c].y_sampling, xs = ctx->channels[c].x_sampling;
            int w, x;
            uint32_t prev = 0;
            if ((yy % ys) != 0) continue;
            w = exr_num_samples(xmin, xmax, xs);
            switch (ctx->channels[c].pixel_type) {
            case EXR_PIXEL_UINT: {
                uint8_t *p0 = op, *p1 = op + w, *p2 = op + 2 * w, *p3 = op + 3 * w;
                op += (size_t)w * 4;
                for (x = 0; x < w; ++x) {
                    uint32_t px = exr_rd_u32(in), d;
                    in += 4;
                    d = px - prev;
                    prev = px;
                    p0[x] = (uint8_t)(d >> 24);
                    p1[x] = (uint8_t)(d >> 16);
                    p2[x] = (uint8_t)(d >> 8);
                    p3[x] = (uint8_t)d;
                }
                break;
            }
            case EXR_PIXEL_HALF: {
                uint8_t *p0 = op, *p1 = op + w;
                op += (size_t)w * 2;
                for (x = 0; x < w; ++x) {
                    uint32_t px = exr_rd_u16(in), d;
                    in += 2;
                    d = px - prev;
                    prev = px;
                    p0[x] = (uint8_t)(d >> 8);
                    p1[x] = (uint8_t)d;
                }
                break;
            }
            case EXR_PIXEL_FLOAT: {
                uint8_t *p0 = op, *p1 = op + w, *p2 = op + 2 * w;
                op += (size_t)w * 3;
                for (x = 0; x < w; ++x) {
                    uint32_t px = float_to_float24(exr_rd_f32(in)), d;
                    in += 4;
                    d = px - prev;
                    prev = px;
                    p0[x] = (uint8_t)(d >> 16);
                    p1[x] = (uint8_t)(d >> 8);
                    p2[x] = (uint8_t)d;
                }
                break;
            }
            }
        }
    }

    rc = exr_zlib_deflate(a, buf, inter, &comp, &clen);
    exr_free(a, buf);
    if (!EXR_OK(rc) || clen >= n) { /* store the canonical block raw */
        exr_free(a, comp);
        *out_data = (uint8_t *)exr_malloc(a, n ? n : 1);
        if (!*out_data) return EXR_ERROR_OUT_OF_MEMORY;
        memcpy(*out_data, block, n);
        *out_size = n;
        return EXR_SUCCESS;
    }
    *out_data = comp;
    *out_size = clen;
    return EXR_SUCCESS;
}
