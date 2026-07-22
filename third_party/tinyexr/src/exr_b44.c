/*
 * TinyEXR - B44 / B44A codec (lossy 4x4 block, HALF channels only).
 *
 * The 4x4 block unpack and the exp/log perceptual tables follow the
 * well-tested decoder in the legacy single-header tinyexr (which matches
 * OpenEXR). Output is emitted in the canonical scanline-block layout so the
 * shared scatter step can place it.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* ---- perceptual tables (computed once, on first use) -----------------------
 * The two half-indexed tables (convertFromLinear / convertToLinear) are built
 * at runtime into .bss so no large precomputed array is baked into the object.
 * To keep the core freestanding (no <math.h>), exp/log are implemented here in
 * double precision; they reproduce libm's results bit-for-bit after the f2h
 * quantization for the entire half domain (verified vs libm over all 65536
 * entries; see the b44 table test in test_exr_v3.c).
 * ------------------------------------------------------------------------- */

static uint16_t g_b44_exp_table[65536];
static uint16_t g_b44_log_table[65536];
static int g_b44_tables_ready = 0;

static float b44_h2f(uint16_t h) {
    union { uint32_t i; float f; } u;
    int s = (h >> 15) & 1, e = (h >> 10) & 0x1f, m = h & 0x3ff;
    if (e == 0) {
        if (m == 0) { u.i = (uint32_t)s << 31; return u.f; }
        {
            float f = (float)m / 1024.0f * (1.0f / 16384.0f);
            return s ? -f : f;
        }
    } else if (e == 31) {
        u.i = ((uint32_t)s << 31) | 0x7f800000u | ((uint32_t)m << 13);
        return u.f;
    }
    u.i = ((uint32_t)s << 31) | ((uint32_t)(e + 112) << 23) | ((uint32_t)m << 13);
    return u.f;
}

static uint16_t b44_f2h(float f) {
    union { uint32_t i; float f; } u;
    int s, e, m;
    u.f = f;
    s = (int)((u.i >> 31) & 1);
    e = (int)((u.i >> 23) & 0xff);
    m = (int)(u.i & 0x7fffff);
    if (e == 0) return (uint16_t)(s << 15);
    if (e == 255) return (uint16_t)((s << 15) | 0x7c00 | (m >> 13));
    if (e < 113) {
        if (e < 103) return (uint16_t)(s << 15);
        m = (m | 0x800000) >> (114 - e);
        return (uint16_t)((s << 15) | (m >> 13));
    }
    if (e > 142) return (uint16_t)((s << 15) | 0x7c00);
    return (uint16_t)((s << 15) | ((e - 112) << 10) | (m >> 13));
}

/* 2^k as a double (normal + subnormal range). The B44 table domain keeps k well
 * inside [-1022,1023]; the clamps just keep the bit math defined at the edges. */
static double b44_scale2(int k) {
    union { uint64_t b; double d; } u;
    if (k > 1023) k = 1023;
    if (k < -1074) return 0.0;
    if (k >= -1022) { u.b = (uint64_t)(k + 1023) << 52; return u.d; }
    u.b = (uint64_t)(k + 54 + 1023) << 52;
    return u.d * 0x1p-54;
}

/* exp(x): range-reduce x = k*ln2 + r, Taylor on r in [-ln2/2, ln2/2]. */
static double b44_exp(double x) {
    const double INV_LN2 = 1.4426950408889634;
    const double LN2_HI = 6.93145751953125e-1, LN2_LO = 1.42860682030941723212e-6;
    int k;
    double r, r2, p;
    if (x != x) return x; /* NaN */
    k = (int)(x * INV_LN2 + (x < 0 ? -0.5 : 0.5));
    r = (x - (double)k * LN2_HI) - (double)k * LN2_LO;
    r2 = r * r;
    p = r + r2 * (0.5 + r * (1.0 / 6 + r * (1.0 / 24 + r * (1.0 / 120 +
        r * (1.0 / 720 + r * (1.0 / 5040 + r * (1.0 / 40320 + r * (1.0 / 362880))))))));
    return (1.0 + p) * b44_scale2(k);
}

/* log(x), x > 0: x = m*2^e with m in [1,2); log via atanh series of (m-1)/(m+1). */
static double b44_log(double x) {
    union { uint64_t b; double d; } u;
    int e;
    double m, f, f2, s;
    const double LN2 = 0.6931471805599453;
    if (x <= 0.0) return -1.0e308; /* unreachable: caller guards x>0 */
    u.d = x;
    if (((u.b >> 52) & 0x7ff) == 0) { u.d = x * 0x1p54; e = -54; } else { e = 0; }
    e += (int)((u.b >> 52) & 0x7ff) - 1023;
    u.b = (u.b & 0x000fffffffffffffULL) | 0x3ff0000000000000ULL;
    m = u.d;
    if (m > 1.4142135623730951) { m *= 0.5; e++; }
    f = (m - 1.0) / (m + 1.0);
    f2 = f * f;
    s = f * (2.0 + f2 * (2.0 / 3 + f2 * (2.0 / 5 + f2 * (2.0 / 7 + f2 * (2.0 / 9 +
        f2 * (2.0 / 11 + f2 * (2.0 / 13)))))));
    return (double)e * LN2 + s;
}

static void b44_init_tables(void) {
    int i;
    if (g_b44_tables_ready) return;
    for (i = 0; i < 65536; ++i) {
        uint16_t x = (uint16_t)i;
        /* expTable: convertFromLinear (decode for p_linear channels) */
        if ((x & 0x7c00) == 0x7c00)
            g_b44_exp_table[i] = 0;
        else if (x >= 0x558c && x < 0x8000)
            g_b44_exp_table[i] = 0x7bff;
        else
            g_b44_exp_table[i] = b44_f2h((float)b44_exp((double)b44_h2f(x) / 8.0));
        /* logTable: convertToLinear */
        if ((x & 0x7c00) == 0x7c00) {
            g_b44_log_table[i] = 0;
        } else if (x > 0x8000) {
            g_b44_log_table[i] = 0;
        } else {
            float ff = b44_h2f(x);
            if (ff <= 0.0f)
                g_b44_log_table[i] = 0;
            else
                g_b44_log_table[i] = b44_f2h((float)(8.0 * b44_log((double)ff)));
        }
    }
    g_b44_tables_ready = 1;
}

/* Internal hook for the table-correctness test (forces init, returns tables). */
void exr_b44_debug_tables(const uint16_t **exp_tbl, const uint16_t **log_tbl) {
    b44_init_tables();
    if (exp_tbl) *exp_tbl = g_b44_exp_table;
    if (log_tbl) *log_tbl = g_b44_log_table;
}

/* ---- block unpack (matches OpenEXR unpack14 / unpack3) --------------------- */

static void b44_unpack14(uint16_t dst[16], const uint8_t s[14]) {
    uint16_t s0 = (uint16_t)(((uint16_t)s[0] << 8) | s[1]);
    unsigned shift = s[2] >> 2;
    unsigned bias = 0x20u << shift;
    uint16_t v[16];
#define ACC(prev, six) \
    (uint16_t)((uint32_t)(prev) + (uint32_t)((six) & 0x3fu) * (1u << shift) - bias)
    v[0] = s0;
    v[4] = ACC(v[0], (((uint32_t)s[2] << 4) | ((uint32_t)s[3] >> 4)));
    v[8] = ACC(v[4], (((uint32_t)s[3] << 2) | ((uint32_t)s[4] >> 6)));
    v[12] = ACC(v[8], (uint32_t)s[4]);
    v[1] = ACC(v[0], (uint32_t)(s[5] >> 2));
    v[5] = ACC(v[4], (((uint32_t)s[5] << 4) | ((uint32_t)s[6] >> 4)));
    v[9] = ACC(v[8], (((uint32_t)s[6] << 2) | ((uint32_t)s[7] >> 6)));
    v[13] = ACC(v[12], (uint32_t)s[7]);
    v[2] = ACC(v[1], (uint32_t)(s[8] >> 2));
    v[6] = ACC(v[5], (((uint32_t)s[8] << 4) | ((uint32_t)s[9] >> 4)));
    v[10] = ACC(v[9], (((uint32_t)s[9] << 2) | ((uint32_t)s[10] >> 6)));
    v[14] = ACC(v[13], (uint32_t)s[10]);
    v[3] = ACC(v[2], (uint32_t)(s[11] >> 2));
    v[7] = ACC(v[6], (((uint32_t)s[11] << 4) | ((uint32_t)s[12] >> 4)));
    v[11] = ACC(v[10], (((uint32_t)s[12] << 2) | ((uint32_t)s[13] >> 6)));
    v[15] = ACC(v[14], (uint32_t)s[13]);
#undef ACC
    {
        int i;
        for (i = 0; i < 16; ++i) {
            if (v[i] & 0x8000)
                dst[i] = (uint16_t)(v[i] & 0x7fff);
            else
                dst[i] = (uint16_t)(~v[i]);
        }
    }
}

static void b44_unpack3(uint16_t dst[16], const uint8_t s[3]) {
    uint16_t t = (uint16_t)(((uint16_t)s[0] << 8) | s[1]);
    uint16_t h = (t & 0x8000) ? (uint16_t)(t & 0x7fff) : (uint16_t)(~t);
    int i;
    for (i = 0; i < 16; ++i) dst[i] = h;
}

/* ---- decode --------------------------------------------------------------- */

exr_result exr_b44_decompress(const exr_codec_ctx *ctx, const uint8_t *src,
                              size_t src_size, uint8_t *dst, size_t dst_size,
                              int optimize_flat) {
    const exr_allocator *a = ctx->alloc;
    int xmin = ctx->x, xmax = ctx->x + ctx->width - 1;
    const uint8_t *in = src, *in_end = src + src_size;
    uint16_t **half = NULL;        /* per-channel decoded scratch (HALF) */
    const uint8_t **nonhalf = NULL; /* per-channel raw ptr (UINT/FLOAT) */
    int *cw = NULL, *ch = NULL, *cpw = NULL;
    int *emitted = NULL;
    int c, line, need_tables = 0;
    exr_result rc = EXR_SUCCESS;
    uint8_t *out, *out_end;

    (void)optimize_flat; /* flat blocks are detected per-block by shift value */

    for (c = 0; c < ctx->num_channels; ++c)
        if (ctx->channels[c].p_linear) need_tables = 1;
    if (need_tables) b44_init_tables();

    half = (uint16_t **)exr_calloc(a, (size_t)ctx->num_channels, sizeof(*half));
    nonhalf = (const uint8_t **)exr_calloc(a, (size_t)ctx->num_channels,
                                           sizeof(*nonhalf));
    cw = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    ch = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    cpw = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    emitted = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    if (!half || !nonhalf || !cw || !ch || !cpw || !emitted) {
        rc = EXR_ERROR_OUT_OF_MEMORY;
        goto done;
    }

    /* Pass A: decode each channel from the stream into scratch. */
    for (c = 0; c < ctx->num_channels; ++c) {
        int xs = ctx->channels[c].x_sampling, ys = ctx->channels[c].y_sampling;
        int width, height, pw, ph, bx, by, nbx, nby;
        if (xs <= 0 || ys <= 0) { rc = EXR_ERROR_CORRUPT; goto done; }
        width = exr_num_samples(xmin, xmax, xs);
        height = exr_num_samples(ctx->y, ctx->y + ctx->num_lines - 1, ys);
        if (width < 0) width = 0;
        if (height < 0) height = 0;
        cw[c] = width;
        ch[c] = height;

        if (ctx->channels[c].pixel_type != EXR_PIXEL_HALF) {
            size_t bytes = (size_t)width * (size_t)height * 4u;
            if ((size_t)(in_end - in) < bytes) { rc = EXR_ERROR_CORRUPT; goto done; }
            nonhalf[c] = in;
            in += bytes;
            continue;
        }

        pw = ((width + 3) / 4) * 4;
        ph = ((height + 3) / 4) * 4;
        cpw[c] = pw;
        nbx = pw / 4;
        nby = ph / 4;
        if (pw > 0 && ph > 0) {
            size_t n;
            if (exr_mul_ovf((size_t)pw, (size_t)ph, &n)) { rc = EXR_ERROR_CORRUPT; goto done; }
            half[c] = (uint16_t *)exr_calloc(a, n, sizeof(uint16_t));
            if (!half[c]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        }
        for (by = 0; by < nby; ++by) {
            for (bx = 0; bx < nbx; ++bx) {
                uint16_t block[16];
                int dy;
                if (in + 3 > in_end) { rc = EXR_ERROR_CORRUPT; goto done; }
                if (in[2] >= (13 << 2)) {
                    b44_unpack3(block, in);
                    in += 3;
                } else {
                    if (in + 14 > in_end) { rc = EXR_ERROR_CORRUPT; goto done; }
                    b44_unpack14(block, in);
                    in += 14;
                }
                if (ctx->channels[c].p_linear) {
                    int i;
                    for (i = 0; i < 16; ++i) block[i] = g_b44_exp_table[block[i]];
                }
                for (dy = 0; dy < 4; ++dy) {
                    int dx;
                    for (dx = 0; dx < 4; ++dx)
                        half[c][(size_t)(by * 4 + dy) * pw + (bx * 4 + dx)] =
                            block[dy * 4 + dx];
                }
            }
        }
    }

    /* Pass B: emit canonical block layout (per line, per sampled channel). */
    out = dst;
    out_end = dst + dst_size;
    for (line = 0; line < ctx->num_lines; ++line) {
        int yy = ctx->y + line;
        for (c = 0; c < ctx->num_channels; ++c) {
            int ys = ctx->channels[c].y_sampling;
            int row, x, width = cw[c];
            if ((yy % ys) != 0) continue;
            row = emitted[c]++;
            if (row >= ch[c]) { rc = EXR_ERROR_CORRUPT; goto done; }
            if (ctx->channels[c].pixel_type == EXR_PIXEL_HALF) {
                const uint16_t *r = half[c] + (size_t)row * cpw[c];
                if (out + (size_t)width * 2 > out_end) { rc = EXR_ERROR_CORRUPT; goto done; }
                for (x = 0; x < width; ++x) {
                    out[0] = (uint8_t)(r[x] & 0xff);
                    out[1] = (uint8_t)(r[x] >> 8);
                    out += 2;
                }
            } else {
                size_t bytes = (size_t)width * 4u;
                const uint8_t *r = nonhalf[c] + (size_t)row * bytes;
                if (out + bytes > out_end) { rc = EXR_ERROR_CORRUPT; goto done; }
                memcpy(out, r, bytes);
                out += bytes;
            }
        }
    }
    if ((size_t)(out - dst) != dst_size) rc = EXR_ERROR_CORRUPT;

done:
    if (half) {
        for (c = 0; c < ctx->num_channels; ++c) exr_free(a, half[c]);
        exr_free(a, half);
    }
    exr_free(a, (void *)nonhalf);
    exr_free(a, cw);
    exr_free(a, ch);
    exr_free(a, cpw);
    exr_free(a, emitted);
    return rc;
}

/* ---- encode --------------------------------------------------------------- */

static int b44_shift_round(int x, int shift) {
    int a, b;
    x <<= 1;
    a = (1 << shift) - 1;
    shift += 1;
    b = (x >> shift) & 1;
    return (x + a + b) >> shift;
}

/* Pack a 4x4 block of HALF values into 14 bytes (or 3 for a flat block). */
static int b44_pack(uint8_t *out, const uint16_t *blk, int flatfields) {
    int d[16], r[15], rMin, rMax, shift = -1, i;
    uint16_t t[16], tMax = 0;
    const int bias = 0x20;
    for (i = 0; i < 16; ++i) {
        if ((blk[i] & 0x7c00) == 0x7c00) t[i] = 0x8000;
        else if (blk[i] & 0x8000) t[i] = (uint16_t)~blk[i];
        else t[i] = (uint16_t)(blk[i] | 0x8000);
    }
    for (i = 0; i < 16; ++i) if (tMax < t[i]) tMax = t[i];
    do {
        shift += 1;
        for (i = 0; i < 16; ++i) d[i] = b44_shift_round(tMax - t[i], shift);
        r[0] = d[0] - d[4] + bias;   r[1] = d[4] - d[8] + bias;
        r[2] = d[8] - d[12] + bias;  r[3] = d[0] - d[1] + bias;
        r[4] = d[4] - d[5] + bias;   r[5] = d[8] - d[9] + bias;
        r[6] = d[12] - d[13] + bias; r[7] = d[1] - d[2] + bias;
        r[8] = d[5] - d[6] + bias;   r[9] = d[9] - d[10] + bias;
        r[10] = d[13] - d[14] + bias; r[11] = d[2] - d[3] + bias;
        r[12] = d[6] - d[7] + bias;  r[13] = d[10] - d[11] + bias;
        r[14] = d[14] - d[15] + bias;
        rMin = rMax = r[0];
        for (i = 1; i < 15; ++i) { if (rMin > r[i]) rMin = r[i]; if (rMax < r[i]) rMax = r[i]; }
    } while (rMin < 0 || rMax > 0x3f);

    if (rMin == bias && rMax == bias && flatfields) {
        out[0] = (uint8_t)(t[0] >> 8);
        out[1] = (uint8_t)t[0];
        out[2] = 0xfc;
        return 3;
    }
    t[0] = (uint16_t)(tMax - (uint16_t)(d[0] << shift)); /* exactmax */
    out[0] = (uint8_t)(t[0] >> 8);
    out[1] = (uint8_t)t[0];
    out[2] = (uint8_t)((shift << 2) | (r[0] >> 4));
    out[3] = (uint8_t)((r[0] << 4) | (r[1] >> 2));
    out[4] = (uint8_t)((r[1] << 6) | r[2]);
    out[5] = (uint8_t)((r[3] << 2) | (r[4] >> 4));
    out[6] = (uint8_t)((r[4] << 4) | (r[5] >> 2));
    out[7] = (uint8_t)((r[5] << 6) | r[6]);
    out[8] = (uint8_t)((r[7] << 2) | (r[8] >> 4));
    out[9] = (uint8_t)((r[8] << 4) | (r[9] >> 2));
    out[10] = (uint8_t)((r[9] << 6) | r[10]);
    out[11] = (uint8_t)((r[11] << 2) | (r[12] >> 4));
    out[12] = (uint8_t)((r[12] << 4) | (r[13] >> 2));
    out[13] = (uint8_t)((r[13] << 6) | r[14]);
    return 14;
}

exr_result exr_b44_compress(const exr_codec_ctx *ctx, const uint8_t *block,
                            size_t n, uint8_t **out_data, size_t *out_size,
                            int optimize_flat) {
    const exr_allocator *a = ctx->alloc;
    int xmin = ctx->x, xmax = ctx->x + ctx->width - 1;
    uint16_t **hp = NULL;
    uint8_t **np = NULL;
    int *cw = NULL, *ch = NULL, *emit = NULL;
    int c, line, need_tables = 0;
    size_t maxout = 0, pos = 0;
    uint8_t *out = NULL;
    const uint8_t *bp;
    exr_result rc = EXR_SUCCESS;

    *out_data = NULL;
    *out_size = 0;
    for (c = 0; c < ctx->num_channels; ++c)
        if (ctx->channels[c].p_linear) need_tables = 1;
    if (need_tables) b44_init_tables();

    hp = (uint16_t **)exr_calloc(a, (size_t)ctx->num_channels, sizeof(*hp));
    np = (uint8_t **)exr_calloc(a, (size_t)ctx->num_channels, sizeof(*np));
    cw = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    ch = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    emit = (int *)exr_calloc(a, (size_t)ctx->num_channels, sizeof(int));
    if (!hp || !np || !cw || !ch || !emit) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

    for (c = 0; c < ctx->num_channels; ++c) {
        int xs = ctx->channels[c].x_sampling, ys = ctx->channels[c].y_sampling;
        int w, h;
        if (xs <= 0 || ys <= 0) { rc = EXR_ERROR_CORRUPT; goto done; }
        w = exr_num_samples(xmin, xmax, xs);
        h = exr_num_samples(ctx->y, ctx->y + ctx->num_lines - 1, ys);
        if (w < 0) w = 0;
        if (h < 0) h = 0;
        cw[c] = w;
        ch[c] = h;
        if (ctx->channels[c].pixel_type == EXR_PIXEL_HALF) {
            int nbx = (w + 3) / 4, nby = (h + 3) / 4;
            maxout += (size_t)nbx * nby * 14;
            hp[c] = (uint16_t *)exr_calloc(a, (size_t)(w ? w : 1) * (h ? h : 1),
                                           sizeof(uint16_t));
            if (!hp[c]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        } else {
            maxout += (size_t)w * h * 4;
            np[c] = (uint8_t *)exr_calloc(a, (size_t)(w ? w : 1) * (h ? h : 1) * 4, 1);
            if (!np[c]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        }
    }

    /* reorganize canonical block (dense per line/channel) -> channel planar */
    bp = block;
    for (line = 0; line < ctx->num_lines; ++line) {
        int yy = ctx->y + line;
        for (c = 0; c < ctx->num_channels; ++c) {
            int ys = ctx->channels[c].y_sampling, x, row, w = cw[c];
            if ((yy % ys) != 0) continue;
            row = emit[c]++;
            if (ctx->channels[c].pixel_type == EXR_PIXEL_HALF) {
                uint16_t *dst = hp[c] + (size_t)row * w;
                for (x = 0; x < w; ++x) { dst[x] = exr_rd_u16(bp); bp += 2; }
            } else {
                uint8_t *dst = np[c] + (size_t)row * w * 4;
                memcpy(dst, bp, (size_t)w * 4);
                bp += (size_t)w * 4;
            }
        }
    }

    out = (uint8_t *)exr_malloc(a, maxout ? maxout : 1);
    if (!out) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

    for (c = 0; c < ctx->num_channels; ++c) {
        int w = cw[c], h = ch[c];
        if (ctx->channels[c].pixel_type == EXR_PIXEL_HALF) {
            int nbx = (w + 3) / 4, nby = (h + 3) / 4, by, bx;
            int plinear = ctx->channels[c].p_linear;
            for (by = 0; by < nby; ++by) {
                for (bx = 0; bx < nbx; ++bx) {
                    uint16_t blk[16];
                    int dy;
                    for (dy = 0; dy < 4; ++dy) {
                        int yy = by * 4 + dy, sy = (yy >= h) ? (h - 1) : yy, dx;
                        for (dx = 0; dx < 4; ++dx) {
                            int xx = bx * 4 + dx, sx = (xx >= w) ? (w - 1) : xx;
                            uint16_t v = (w > 0 && h > 0) ? hp[c][(size_t)sy * w + sx] : 0;
                            if (plinear) v = g_b44_log_table[v];
                            blk[dy * 4 + dx] = v;
                        }
                    }
                    pos += (size_t)b44_pack(out + pos, blk, optimize_flat);
                }
            }
        } else {
            size_t cb = (size_t)w * h * 4;
            memcpy(out + pos, np[c], cb);
            pos += cb;
        }
    }

    if (pos >= n) { /* store the canonical block raw */
        exr_free(a, out);
        out = (uint8_t *)exr_malloc(a, n ? n : 1);
        if (!out) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
        memcpy(out, block, n);
        *out_data = out;
        *out_size = n;
        out = NULL;
        goto done;
    }
    *out_data = out;
    *out_size = pos;
    out = NULL;

done:
    if (hp) { for (c = 0; c < ctx->num_channels; ++c) exr_free(a, hp[c]); exr_free(a, hp); }
    if (np) { for (c = 0; c < ctx->num_channels; ++c) exr_free(a, np[c]); exr_free(a, np); }
    exr_free(a, cw);
    exr_free(a, ch);
    exr_free(a, emit);
    exr_free(a, out);
    return rc;
}
