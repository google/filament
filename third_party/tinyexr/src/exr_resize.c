/*
 * TinyEXR - separable HDR resize / resampling (util module).
 *
 * Linear-light float resampling with box / triangle / cubic kernels. The core
 * is a streaming resizer holding only a sliding window of horizontally-resampled
 * source rows (O(filter_support * dst_w) memory, independent of height). The
 * whole-image entry point wraps the same streaming core so the two paths are
 * bit-identical. The hot vertical accumulation uses the dispatched axpy kernel.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* Scalar reference for the dispatched vertical-accumulation kernel. */
void exr_axpy_scalar(float *acc, const float *x, float w, size_t n) {
    size_t i;
    for (i = 0; i < n; ++i) acc[i] += x[i] * w;
}

/* ---- libm-free floor/ceil to int ----------------------------------------- */
static int ifloor(float x) {
    int i = (int)x;
    return (x < 0.0f && (float)i != x) ? i - 1 : i;
}
static int iceil(float x) {
    int i = (int)x;
    return (x > 0.0f && (float)i != x) ? i + 1 : i;
}

/* ---- filter kernels (pure arithmetic, no transcendentals) ---------------- */
static float kern_cubic(float a, float B, float C) {
    if (a < 1.0f)
        return ((12.0f - 9.0f * B - 6.0f * C) * a * a * a +
                (-18.0f + 12.0f * B + 6.0f * C) * a * a + (6.0f - 2.0f * B)) /
               6.0f;
    if (a < 2.0f)
        return ((-B - 6.0f * C) * a * a * a + (6.0f * B + 30.0f * C) * a * a +
                (-12.0f * B - 48.0f * C) * a + (8.0f * B + 24.0f * C)) /
               6.0f;
    return 0.0f;
}

static float kern_weight(exr_resize_filter f, float t) {
    float a = t < 0.0f ? -t : t;
    switch (f) {
        case EXR_RESIZE_BOX:
            return a <= 0.5f ? 1.0f : 0.0f;
        case EXR_RESIZE_TRIANGLE:
            return a < 1.0f ? 1.0f - a : 0.0f;
        case EXR_RESIZE_CATMULL_ROM:
            return kern_cubic(a, 0.0f, 0.5f);
        default: /* EXR_RESIZE_MITCHELL */
            return kern_cubic(a, 1.0f / 3.0f, 1.0f / 3.0f);
    }
}

static float filter_radius(exr_resize_filter f) {
    if (f == EXR_RESIZE_BOX) return 0.5f;
    if (f == EXR_RESIZE_TRIANGLE) return 1.0f;
    return 2.0f;
}

static int edge_idx(int j, int n, exr_edge_mode m) {
    if (n == 1) return 0;
    if (j >= 0 && j < n) return j;
    if (m == EXR_EDGE_CLAMP) return j < 0 ? 0 : n - 1;
    if (m == EXR_EDGE_WRAP) {
        j %= n;
        if (j < 0) j += n;
        return j;
    }
    { /* reflect without repeating the edge sample (period 2n-2) */
        int p = 2 * n - 2;
        j %= p;
        if (j < 0) j += p;
        return j < n ? j : p - j;
    }
}

/* ---- per-axis contributor table (contiguous mapped runs) ----------------- */
typedef struct {
    int *first;   /* [dst] first (lowest) mapped source index */
    int *n;       /* [dst] contiguous run length */
    size_t *woff; /* [dst] offset into w */
    float *w;     /* packed run weights */
    int dst;
    int max_n;
} axisfilter;

static void axisfilter_free(const exr_allocator *a, axisfilter *f) {
    exr_free(a, f->first);
    exr_free(a, f->n);
    exr_free(a, f->woff);
    exr_free(a, f->w);
    memset(f, 0, sizeof(*f));
}

/* Compute the contiguous mapped run [first, first+n) for output index i. */
static void window_for(int s, int d, int i, exr_resize_filter filt,
                       exr_edge_mode edge, float support, float invscale,
                       int *out_first, int *out_n) {
    float center = ((float)i + 0.5f) * ((float)s / (float)d) - 0.5f;
    int lo = iceil(center - support), hi = ifloor(center + support);
    int lo_m = s, hi_m = -1, t;
    if (hi < lo) hi = lo;
    (void)filt;
    (void)invscale;
    for (t = lo; t <= hi; ++t) {
        int mi = edge_idx(t, s, edge);
        if (mi < lo_m) lo_m = mi;
        if (mi > hi_m) hi_m = mi;
    }
    if (hi_m < lo_m) { lo_m = 0; hi_m = 0; }
    *out_first = lo_m;
    *out_n = hi_m - lo_m + 1;
}

static exr_result axisfilter_build(const exr_allocator *a, int s, int d,
                                   exr_resize_filter filt, exr_edge_mode edge,
                                   axisfilter *out) {
    float fscale = (d < s) ? (float)s / (float)d : 1.0f;
    float support = filter_radius(filt) * fscale;
    float invscale = 1.0f / fscale;
    int i;
    size_t total = 0;
    memset(out, 0, sizeof(*out));
    out->dst = d;
    out->first = (int *)exr_calloc(a, (size_t)d, sizeof(int));
    out->n = (int *)exr_calloc(a, (size_t)d, sizeof(int));
    out->woff = (size_t *)exr_calloc(a, (size_t)d, sizeof(size_t));
    if (!out->first || !out->n || !out->woff) goto oom;

    for (i = 0; i < d; ++i) {
        int first, n;
        window_for(s, d, i, filt, edge, support, invscale, &first, &n);
        out->first[i] = first;
        out->n[i] = n;
        out->woff[i] = total;
        if (n > out->max_n) out->max_n = n;
        total += (size_t)n;
    }
    out->w = (float *)exr_calloc(a, total ? total : 1, sizeof(float));
    if (!out->w) goto oom;

    for (i = 0; i < d; ++i) {
        float center = ((float)i + 0.5f) * ((float)s / (float)d) - 0.5f;
        int lo = iceil(center - support), hi = ifloor(center + support), t;
        float *w = out->w + out->woff[i];
        int first = out->first[i], n = out->n[i];
        float sum = 0.0f;
        if (hi < lo) hi = lo;
        for (t = lo; t <= hi; ++t) {
            int mi = edge_idx(t, s, edge);
            float wt = kern_weight(filt, (center - (float)t) * invscale);
            if (mi >= first && mi < first + n) w[mi - first] += wt;
        }
        for (t = 0; t < n; ++t) sum += w[t];
        if (sum == 0.0f) {
            w[0] = 1.0f;
        } else {
            float inv = 1.0f / sum;
            for (t = 0; t < n; ++t) w[t] *= inv;
        }
    }
    return EXR_SUCCESS;
oom:
    axisfilter_free(a, out);
    return EXR_ERROR_OUT_OF_MEMORY;
}

/* ---- pixel widen / narrow for the streaming io_type ---------------------- */
static void widen_row(float *dst, const void *src, size_t count,
                      exr_pixel_type t) {
    if (t == EXR_PIXEL_FLOAT) {
        memcpy(dst, src, count * sizeof(float));
    } else if (t == EXR_PIXEL_HALF) {
        exr_half_to_float((const uint16_t *)src, dst, count);
    } else {
        size_t i;
        const uint32_t *u = (const uint32_t *)src;
        for (i = 0; i < count; ++i) dst[i] = (float)u[i];
    }
}
static void narrow_row(void *dst, const float *src, size_t count,
                       exr_pixel_type t) {
    if (t == EXR_PIXEL_FLOAT) {
        memcpy(dst, src, count * sizeof(float));
    } else if (t == EXR_PIXEL_HALF) {
        exr_float_to_half(src, (uint16_t *)dst, count);
    } else {
        size_t i;
        uint32_t *u = (uint32_t *)dst;
        for (i = 0; i < count; ++i) {
            double v = (double)src[i];
            if (!(v > 0.0)) v = 0.0;
            if (v > 4294967295.0) v = 4294967295.0;
            {
                uint32_t k = (uint32_t)v;
                double f = v - (double)k;
                if (f > 0.5 || (f == 0.5 && (k & 1u))) k += 1u;
                u[i] = k;
            }
        }
    }
}

/* ============================================================================
 * Streaming resizer
 * ========================================================================== */
struct exr_resizer {
    exr_allocator alloc;
    int sw, sh, dw, dh, channels;
    exr_pixel_type io_type;
    axisfilter fx, fy;
    /* ring of horizontally-resampled source rows (dw*channels floats each) */
    int ring_cap;
    float **ring;   /* [ring_cap] */
    int *ring_srcy; /* [ring_cap] source row resident in each slot, -1 empty */
    float *srcf;    /* sw*channels widen scratch */
    float *accum;   /* dw*channels vertical scratch */
    int cur_dst;
};

void exr_resizer_destroy(exr_resizer *r) {
    const exr_allocator *a;
    int i;
    if (!r) return;
    a = &r->alloc;
    axisfilter_free(a, &r->fx);
    axisfilter_free(a, &r->fy);
    if (r->ring) {
        for (i = 0; i < r->ring_cap; ++i) exr_free(a, r->ring[i]);
        exr_free(a, r->ring);
    }
    exr_free(a, r->ring_srcy);
    exr_free(a, r->srcf);
    exr_free(a, r->accum);
    exr_free(a, r);
}

exr_result exr_resizer_create(const exr_allocator *a, int sw, int sh, int dw,
                              int dh, int channels, exr_pixel_type io_type,
                              exr_resize_filter filter, exr_edge_mode edge,
                              exr_resizer **out) {
    exr_resizer *r;
    exr_result rc;
    int i;
    size_t rowf;
    if (out) *out = NULL;
    if (!a) a = exr_default_allocator();
    if (!out || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0 || channels < 1 ||
        channels > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    if (sw > EXR_MAX_DIMENSION || sh > EXR_MAX_DIMENSION ||
        dw > EXR_MAX_DIMENSION || dh > EXR_MAX_DIMENSION)
        return EXR_ERROR_INVALID_ARGUMENT;

    exr_simd_init();
    r = (exr_resizer *)exr_calloc(a, 1, sizeof(*r));
    if (!r) return EXR_ERROR_OUT_OF_MEMORY;
    r->alloc = *a;
    r->sw = sw; r->sh = sh; r->dw = dw; r->dh = dh;
    r->channels = channels;
    r->io_type = io_type;
    r->cur_dst = 0;

    rc = axisfilter_build(a, sw, dw, filter, edge, &r->fx);
    if (!EXR_OK(rc)) goto fail;
    rc = axisfilter_build(a, sh, dh, filter, edge, &r->fy);
    if (!EXR_OK(rc)) goto fail;

    r->ring_cap = r->fy.max_n;
    if (r->ring_cap < 1) r->ring_cap = 1;
    rowf = (size_t)dw * (size_t)channels;
    r->ring = (float **)exr_calloc(a, (size_t)r->ring_cap, sizeof(float *));
    r->ring_srcy = (int *)exr_malloc(a, (size_t)r->ring_cap * sizeof(int));
    r->srcf = (float *)exr_malloc(a, (size_t)sw * channels * sizeof(float));
    r->accum = (float *)exr_malloc(a, rowf * sizeof(float));
    if (!r->ring || !r->ring_srcy || !r->srcf || !r->accum) {
        rc = EXR_ERROR_OUT_OF_MEMORY;
        goto fail;
    }
    for (i = 0; i < r->ring_cap; ++i) {
        r->ring[i] = (float *)exr_malloc(a, rowf * sizeof(float));
        if (!r->ring[i]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        r->ring_srcy[i] = -1;
    }
    *out = r;
    return EXR_SUCCESS;
fail:
    exr_resizer_destroy(r);
    return rc;
}

/* Horizontal-resample one widened source row into ring slot for src_y. */
static void hresample(exr_resizer *r, int src_y, const float *srcf) {
    int ch = r->channels, ox, c, t;
    int slot = src_y % r->ring_cap;
    float *row = r->ring[slot];
    for (ox = 0; ox < r->dw; ++ox) {
        const float *w = r->fx.w + r->fx.woff[ox];
        int first = r->fx.first[ox], n = r->fx.n[ox];
        for (c = 0; c < ch; ++c) {
            float acc = 0.0f;
            for (t = 0; t < n; ++t)
                acc += w[t] * srcf[(size_t)(first + t) * ch + c];
            row[(size_t)ox * ch + c] = acc;
        }
    }
    r->ring_srcy[slot] = src_y;
}

exr_result exr_resizer_push_row(exr_resizer *r, int src_y, const void *src_row) {
    if (!r || !src_row || src_y < 0 || src_y >= r->sh)
        return EXR_ERROR_INVALID_ARGUMENT;
    widen_row(r->srcf, src_row, (size_t)r->sw * r->channels, r->io_type);
    hresample(r, src_y, r->srcf);
    return EXR_SUCCESS;
}

exr_result exr_resizer_pull_row(exr_resizer *r, int *out_dst_y, void *dst_row) {
    int first, n, t, oy;
    size_t rowf;
    if (!r || !out_dst_y) return EXR_ERROR_INVALID_ARGUMENT;
    oy = r->cur_dst;
    if (oy >= r->dh) { *out_dst_y = r->dh; return EXR_SUCCESS; }
    first = r->fy.first[oy];
    n = r->fy.n[oy];
    /* All required source rows must be resident; else ask for more. */
    for (t = 0; t < n; ++t) {
        int sy = first + t;
        if (r->ring_srcy[sy % r->ring_cap] != sy) {
            *out_dst_y = -1;
            return EXR_WOULD_BLOCK;
        }
    }
    if (!dst_row) return EXR_ERROR_INVALID_ARGUMENT;
    rowf = (size_t)r->dw * r->channels;
    memset(r->accum, 0, rowf * sizeof(float));
    {
        const float *w = r->fy.w + r->fy.woff[oy];
        for (t = 0; t < n; ++t) {
            float *src = r->ring[(first + t) % r->ring_cap];
            exr_simd.axpy(r->accum, src, w[t], rowf);
        }
    }
    narrow_row(dst_row, r->accum, rowf, r->io_type);
    *out_dst_y = oy;
    r->cur_dst = oy + 1;
    return EXR_SUCCESS;
}

/* ============================================================================
 * Whole-image entry (wraps the streaming core; handles strides + premult)
 * ========================================================================== */
exr_result exr_resize_float(const exr_allocator *a, const float *src, int src_w,
                            int src_h, size_t src_row_stride, float *dst,
                            int dst_w, int dst_h, size_t dst_row_stride,
                            int channels, exr_resize_filter filter,
                            exr_edge_mode edge, int alpha_channel) {
    exr_resizer *r = NULL;
    exr_result rc;
    int sy = 0, ch = channels, premult = (alpha_channel >= 0 &&
                                          alpha_channel < channels);
    float *rowbuf = NULL;
    size_t tightS, tightD;
    if (!a) a = exr_default_allocator();
    if (!src || !dst || channels < 1 || channels > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    tightS = (size_t)src_w * ch;
    tightD = (size_t)dst_w * ch;
    if (src_row_stride == 0) src_row_stride = tightS;
    if (dst_row_stride == 0) dst_row_stride = tightD;

    rc = exr_resizer_create(a, src_w, src_h, dst_w, dst_h, channels,
                            EXR_PIXEL_FLOAT, filter, edge, &r);
    if (!EXR_OK(rc)) return rc;

    /* scratch row big enough for either a src or dst tight row */
    rowbuf = (float *)exr_malloc(a, (tightS > tightD ? tightS : tightD) *
                                        sizeof(float));
    if (!rowbuf) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }

    for (;;) {
        int dy;
        rc = exr_resizer_pull_row(r, &dy, rowbuf);
        if (rc == EXR_WOULD_BLOCK) {
            const float *srow;
            int x, c;
            if (sy >= src_h) { rc = EXR_ERROR_CORRUPT; goto done; } /* unreachable */
            srow = src + (size_t)sy * src_row_stride;
            if (premult) {
                for (x = 0; x < src_w; ++x) {
                    float al = srow[(size_t)x * ch + alpha_channel];
                    for (c = 0; c < ch; ++c)
                        rowbuf[(size_t)x * ch + c] =
                            (c == alpha_channel) ? al
                                                 : srow[(size_t)x * ch + c] * al;
                }
                rc = exr_resizer_push_row(r, sy, rowbuf);
            } else {
                rc = exr_resizer_push_row(r, sy, srow);
            }
            if (!EXR_OK(rc)) goto done;
            sy++;
            continue;
        }
        if (!EXR_OK(rc)) goto done;
        if (dy >= dst_h) break; /* finished */
        /* rowbuf holds the dst row (tight); un-premultiply + scatter. */
        {
            float *drow = dst + (size_t)dy * dst_row_stride;
            int x, c;
            for (x = 0; x < dst_w; ++x) {
                if (premult) {
                    float al = rowbuf[(size_t)x * ch + alpha_channel];
                    float inv = (al != 0.0f) ? 1.0f / al : 0.0f;
                    for (c = 0; c < ch; ++c)
                        drow[(size_t)x * ch + c] =
                            (c == alpha_channel)
                                ? al
                                : rowbuf[(size_t)x * ch + c] * inv;
                } else {
                    for (c = 0; c < ch; ++c)
                        drow[(size_t)x * ch + c] = rowbuf[(size_t)x * ch + c];
                }
            }
        }
    }
    rc = EXR_SUCCESS;
done:
    exr_free(a, rowbuf);
    exr_resizer_destroy(r);
    return rc;
}
