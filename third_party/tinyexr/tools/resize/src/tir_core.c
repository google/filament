/*
 * tir - public API: validation, option resolution, sampler lifecycle.
 *
 * A sampler performs exactly one allocation: an arena holding the sampler
 * struct, both coefficient tables, the ring buffers and every scratch row,
 * with all sizes computed through overflow-checked arithmetic.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include "tir_internal.h"

/* ---- result strings / allocator --------------------------------------------- */

const char *tir_result_string(tir_result r) {
    switch (r) {
        case TIR_SUCCESS:
            return "success";
        case TIR_WOULD_BLOCK:
            return "would block";
        case TIR_ERROR_INVALID_ARGUMENT:
            return "invalid argument";
        case TIR_ERROR_OUT_OF_MEMORY:
            return "out of memory";
        case TIR_ERROR_UNSUPPORTED:
            return "unsupported";
        case TIR_ERROR_TOO_LARGE:
            return "size overflow";
        case TIR_ERROR_NONFINITE:
            return "nonfinite pixels";
        case TIR_ERROR_ORDER:
            return "rows pushed out of order";
        default:
            return "unknown";
    }
}

static void *tir__malloc_shim(void *user, size_t size) {
    (void)user;
    return malloc(size);
}
static void tir__free_shim(void *user, void *ptr) {
    (void)user;
    free(ptr);
}

const tir_allocator *tir__default_allocator(void) {
    static const tir_allocator a = {NULL, tir__malloc_shim, tir__free_shim};
    return &a;
}

/* ---- options ------------------------------------------------------------------ */

void tir_options_init(tir_options *opt) {
    if (!opt) return;
    memset(opt, 0, sizeof(*opt));
    opt->filter_x = TIR_FILTER_AUTO;
    opt->filter_y = TIR_FILTER_AUTO;
    opt->edge_x = TIR_EDGE_CLAMP;
    opt->edge_y = TIR_EDGE_CLAMP;
    opt->filter_scale = 1.0f;
    opt->gaussian_sigma = 0.5f;
    opt->mode = TIR_MODE_GENERAL;
    opt->antiring = -1.0f; /* auto */
    opt->clamp_min = -INFINITY;
    opt->clamp_max = INFINITY;
    opt->nonfinite = TIR_NONFINITE_KEEP;
    opt->alpha = TIR_ALPHA_PREMULTIPLY;
    opt->normal_encoding = TIR_NORMAL_SNORM;
    opt->normal_renormalize = 1;
    opt->registration = TIR_REG_CELL_CENTERED;
}

static int filter_has_negative_lobes(tir_filter f) {
    return f == TIR_FILTER_MITCHELL || f == TIR_FILTER_CATMULL_ROM ||
           f == TIR_FILTER_LANCZOS2 || f == TIR_FILTER_LANCZOS3 ||
           f == TIR_FILTER_KAISER;
}

static tir_filter resolve_filter(tir_filter f, tir_mode mode, int is_down) {
    if (f != TIR_FILTER_AUTO) return f;
    switch (mode) {
        case TIR_MODE_NORMAL_MAP:
            return TIR_FILTER_TRIANGLE;
        case TIR_MODE_HEIGHTMAP:
            return is_down ? TIR_FILTER_BOX : TIR_FILTER_TRIANGLE;
        default:
            return TIR_FILTER_MITCHELL;
    }
}

/* Linear map from stored value to signed normal component: n = a*v + b. */
static void normal_map_coeffs(tir_normal_enc enc, tir_pixel_type t, float *a,
                              float *b) {
    *a = 1.0f;
    *b = 0.0f;
    switch (enc) {
        case TIR_NORMAL_SNORM:
            break;
        case TIR_NORMAL_UNORM:
            *a = 2.0f;
            *b = -1.0f;
            break;
        case TIR_NORMAL_UNORM_C127:
            *a = 2.0f;
            *b = (t == TIR_U8)    ? -254.0f / 255.0f
                 : (t == TIR_U16) ? -65534.0f / 65535.0f
                                  : -1.0f;
            break;
        case TIR_NORMAL_RG:
            if (t == TIR_U8 || t == TIR_U16) {
                *a = 2.0f;
                *b = -1.0f;
            }
            break;
    }
}

static tir_result validate_options(const tir_options *o) {
    if ((int)o->filter_x < 0 || o->filter_x > TIR_FILTER_KAISER ||
        (int)o->filter_y < 0 || o->filter_y > TIR_FILTER_KAISER)
        return TIR_ERROR_INVALID_ARGUMENT;
    if ((int)o->edge_x < 0 || o->edge_x > TIR_EDGE_WRAP || (int)o->edge_y < 0 ||
        o->edge_y > TIR_EDGE_WRAP)
        return TIR_ERROR_INVALID_ARGUMENT;
    if ((int)o->mode < 0 || o->mode > TIR_MODE_HEIGHTMAP)
        return TIR_ERROR_INVALID_ARGUMENT;
    if ((int)o->nonfinite < 0 || o->nonfinite > TIR_NONFINITE_ERROR)
        return TIR_ERROR_INVALID_ARGUMENT;
    if ((int)o->alpha < 0 || o->alpha > TIR_ALPHA_STRAIGHT)
        return TIR_ERROR_INVALID_ARGUMENT;
    if ((int)o->normal_encoding < 0 || o->normal_encoding > TIR_NORMAL_RG)
        return TIR_ERROR_INVALID_ARGUMENT;
    if ((int)o->registration < 0 || o->registration > TIR_REG_GRID_VERTEX)
        return TIR_ERROR_INVALID_ARGUMENT;
    if (!(o->filter_scale > 0.0f) || !isfinite(o->filter_scale) ||
        o->filter_scale > 64.0f)
        return TIR_ERROR_INVALID_ARGUMENT;
    if (!(o->gaussian_sigma > 0.0f) || !isfinite(o->gaussian_sigma) ||
        o->gaussian_sigma > 32.0f)
        return TIR_ERROR_INVALID_ARGUMENT;
    if (isnan(o->antiring) || o->antiring > 1.0f)
        return TIR_ERROR_INVALID_ARGUMENT;
    if (isnan(o->clamp_min) || isnan(o->clamp_max) ||
        o->clamp_min > o->clamp_max)
        return TIR_ERROR_INVALID_ARGUMENT;
    return TIR_SUCCESS;
}

/* ---- arena layout ---------------------------------------------------------------- */

typedef struct arena_builder {
    size_t total;
    int ok;
} arena_builder;

static size_t arena_push(arena_builder *b, size_t count, size_t elem) {
    size_t bytes, off, end;
    if (!b->ok) return 0;
    if (!tir__mul_ok(count, elem, &bytes)) {
        b->ok = 0;
        return 0;
    }
    off = tir__align_up(b->total, 64);
    if (off < b->total || !tir__add_ok(off, bytes, &end)) {
        b->ok = 0;
        return 0;
    }
    b->total = end;
    return off;
}

/* Push (a * c + add) elements of `elem` bytes, with every multiply/add
 * overflow-checked. The plain arena_push only guards count*elem, but most
 * callers pass a product as `count`; on an ILP32 build (32-bit size_t) that
 * product can wrap to a small value and under-size the arena, so route the
 * factors through here instead of multiplying them at the call site. */
static size_t arena_pushm(arena_builder *b, size_t a, size_t c, size_t add,
                          size_t elem) {
    size_t count;
    if (!b->ok) return 0;
    if (!tir__mul_ok(a, c, &count) || !tir__add_ok(count, add, &count)) {
        b->ok = 0;
        return 0;
    }
    return arena_push(b, count, elem);
}

typedef struct axis_offs {
    size_t start, count, w, exc_idx, exc_w, ar_start, ar_count;
} axis_offs;

static void axis_layout(arena_builder *b, int d, const tir__axis_dims *dm,
                        axis_offs *o) {
    o->start = arena_push(b, (size_t)d, sizeof(int32_t));
    o->count = arena_push(b, (size_t)d, sizeof(int32_t));
    o->w = arena_pushm(b, (size_t)d, (size_t)dm->padded, 0, sizeof(float));
    o->exc_idx = arena_pushm(b, (size_t)dm->n_exc, (size_t)dm->taps, 0,
                             sizeof(int32_t));
    o->exc_w = arena_pushm(b, (size_t)dm->n_exc, (size_t)dm->taps, 0,
                           sizeof(float));
    o->ar_start = arena_push(b, (size_t)d, sizeof(int32_t));
    o->ar_count = arena_push(b, (size_t)d, sizeof(int32_t));
}

static void axis_bind(char *base, const axis_offs *o, tir__axis *ax, int n_exc) {
    ax->start = (int32_t *)(base + o->start);
    ax->count = (int32_t *)(base + o->count);
    ax->w = (float *)(base + o->w);
    ax->exc_idx = n_exc ? (int32_t *)(base + o->exc_idx) : NULL;
    ax->exc_w = n_exc ? (float *)(base + o->exc_w) : NULL;
    ax->ar_start = (int32_t *)(base + o->ar_start);
    ax->ar_count = (int32_t *)(base + o->ar_count);
}

/* ---- sampler create / destroy ------------------------------------------------------ */

void tir_sampler_destroy(tir_sampler *s) {
    if (!s) return;
    s->alloc.free(s->alloc.user, s);
}

tir_result tir_sampler_create(const tir_allocator *a, int src_w, int src_h,
                              int dst_w, int dst_h, int channels,
                              tir_pixel_type src_type, tir_pixel_type dst_type,
                              const tir_options *opt, tir_sampler **out) {
    tir_options o;
    tir__axis_params px, py;
    tir__axis_dims dx, dy;
    tir_result rc;
    int wc, premult, hicomp_ch, ring_cap, do_ar, v_first;
    float ar_strength;
    size_t ring_stride;

    if (out) *out = NULL;
    if (!out) return TIR_ERROR_INVALID_ARGUMENT;
    if (!a) a = tir__default_allocator();
    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0)
        return TIR_ERROR_INVALID_ARGUMENT;
    if (src_w > TIR_MAX_DIMENSION || src_h > TIR_MAX_DIMENSION ||
        dst_w > TIR_MAX_DIMENSION || dst_h > TIR_MAX_DIMENSION)
        return TIR_ERROR_INVALID_ARGUMENT;
    if (channels < 1 || channels > 4) return TIR_ERROR_INVALID_ARGUMENT;
    if ((int)src_type < 0 || src_type > TIR_U16 || (int)dst_type < 0 ||
        dst_type > TIR_U16)
        return TIR_ERROR_INVALID_ARGUMENT;

    if (opt)
        o = *opt;
    else
        tir_options_init(&o);
    rc = validate_options(&o);
    if (!TIR_OK(rc)) return rc;

    /* -- resolve mode-dependent settings ---------------------------------- */
    wc = channels;
    if (o.mode == TIR_MODE_NORMAL_MAP) {
        if (o.normal_encoding == TIR_NORMAL_RG) {
            if (channels != 2) return TIR_ERROR_UNSUPPORTED;
            wc = 3;
        } else if (channels != 3) {
            return TIR_ERROR_UNSUPPORTED;
        }
    }
    o.filter_x = resolve_filter(o.filter_x, o.mode, dst_w < src_w);
    o.filter_y = resolve_filter(o.filter_y, o.mode, dst_h < src_h);
    if (o.antiring < 0.0f) {
        ar_strength = (o.mode == TIR_MODE_HEIGHTMAP &&
                       (filter_has_negative_lobes(o.filter_x) ||
                        filter_has_negative_lobes(o.filter_y)))
                          ? 1.0f
                          : 0.0f;
    } else {
        ar_strength = o.antiring;
    }
    o.antiring = ar_strength;
    do_ar = ar_strength > 0.0f;
    premult = (o.mode == TIR_MODE_GENERAL && channels == 4 &&
               o.alpha == TIR_ALPHA_PREMULTIPLY);
    hicomp_ch = 0;
    if (o.hicomp && o.mode != TIR_MODE_NORMAL_MAP)
        hicomp_ch = (channels == 4) ? 3 : wc;

    /* -- coefficient tables ------------------------------------------------ */
    px.s = src_w;
    px.d = dst_w;
    px.filter = o.filter_x;
    px.edge = o.edge_x;
    px.reg = o.registration;
    px.filter_scale = o.filter_scale;
    px.gaussian_sigma = o.gaussian_sigma;
    py = px;
    py.s = src_h;
    py.d = dst_h;
    py.filter = o.filter_y;
    py.edge = o.edge_y;
    rc = tir__axis_measure(&px, &dx);
    if (!TIR_OK(rc)) return rc;
    rc = tir__axis_measure(&py, &dy);
    if (!TIR_OK(rc)) return rc;

    ring_cap = dy.n_exc > 0 ? src_h : dy.taps;
    if (ring_cap < 1) ring_cap = 1;
    /* Pass order by multiply count: horizontal-first filters src_h rows in x
     * then dst rows in y; vertical-first blends dst_h source-width rows in y
     * then filters them in x. Worth 20-40% typically, ~3x when the resize is
     * strongly anisotropic. */
    {
        double h_cost = (double)src_h * dst_w * dx.taps +
                        (double)dst_h * dst_w * dy.taps;
        double v_cost = (double)dst_h * src_w * dy.taps +
                        (double)dst_h * dst_w * dx.taps;
        v_first = v_cost < h_cost;
    }
    /* +4 write slack for the 3-channel SIMD horizontal kernels; padded to
     * dodge 4K cache aliasing between ring entries */
    ring_stride = tir__align_up(
        (size_t)(v_first ? src_w : dst_w) * (size_t)wc + 4, 16);
    if ((ring_stride * sizeof(float)) % 4096 == 0) ring_stride += 16;

    /* -- arena layout -------------------------------------------------------- */
    {
        arena_builder b;
        axis_offs ox, oy;
        size_t off_ring, off_mn = 0, off_mx = 0, off_srcy, off_dec, off_stg;
        size_t off_smn = 0, off_smx = 0, off_rep = 0, off_scratch;
        size_t off_vstg = 0, off_vmn = 0, off_vmx = 0, off_stmp = 0;
        size_t win_max, nrep, dec_w = 0;
        tir_sampler *s;
        char *base;

        b.total = sizeof(tir_sampler);
        b.ok = 1;
        /* decode/vstaging width = src_w + horizontal padding slack (elems/ch) */
        if (!tir__add_ok((size_t)src_w, (size_t)dx.padded, &dec_w)) b.ok = 0;
        axis_layout(&b, dst_w, &dx, &ox);
        axis_layout(&b, dst_h, &dy, &oy);
        off_ring = arena_pushm(&b, (size_t)ring_cap, ring_stride, 0,
                               sizeof(float));
        if (do_ar && !v_first) {
            off_mn = arena_pushm(&b, (size_t)ring_cap, ring_stride, 0,
                                 sizeof(float));
            off_mx = arena_pushm(&b, (size_t)ring_cap, ring_stride, 0,
                                 sizeof(float));
        }
        off_srcy = arena_push(&b, (size_t)ring_cap, sizeof(int32_t));
        off_dec = arena_pushm(&b, dec_w, (size_t)wc, 0, sizeof(float));
        off_stg = arena_pushm(&b, (size_t)dst_w, (size_t)wc, 4, sizeof(float));
        if (do_ar) {
            off_smn =
                arena_pushm(&b, (size_t)dst_w, (size_t)wc, 4, sizeof(float));
            off_smx =
                arena_pushm(&b, (size_t)dst_w, (size_t)wc, 4, sizeof(float));
        }
        if (v_first) {
            off_vstg = arena_pushm(&b, dec_w, (size_t)wc, 0, sizeof(float));
            if (do_ar) {
                off_vmn = arena_pushm(&b, (size_t)src_w, (size_t)wc, 0,
                                      sizeof(float));
                off_vmx = arena_pushm(&b, (size_t)src_w, (size_t)wc, 0,
                                      sizeof(float));
                off_stmp = arena_pushm(&b, (size_t)dst_w, (size_t)wc, 4,
                                       sizeof(float));
            }
        }
        nrep = 0u;
        if (o.nonfinite == TIR_NONFINITE_REPAIR) {
            /* 3 source rows of src_w*channels for the NaN repair window */
            if (!tir__mul_ok((size_t)src_w, (size_t)channels, &nrep) ||
                !tir__mul_ok(nrep, 3u, &nrep))
                b.ok = 0;
        }
        if (nrep) off_rep = arena_push(&b, nrep, sizeof(float));
        win_max = (size_t)(dx.win_max > dy.win_max ? dx.win_max : dy.win_max);
        off_scratch = arena_pushm(&b, 2u, win_max, 0, sizeof(double));
        if (!b.ok) return TIR_ERROR_TOO_LARGE;

        base = (char *)a->alloc(a->user, b.total);
        if (!base) return TIR_ERROR_OUT_OF_MEMORY;
        s = (tir_sampler *)base;
        memset(s, 0, sizeof(*s));
        s->alloc = *a;
        s->arena_size = b.total;
        s->opt = o;
        s->sw = src_w;
        s->sh = src_h;
        s->dw = dst_w;
        s->dh = dst_h;
        s->ch = channels;
        s->wc = wc;
        s->st = src_type;
        s->dt = dst_type;
        axis_bind(base, &ox, &s->fx, dx.n_exc);
        axis_bind(base, &oy, &s->fy, dy.n_exc);
        s->v_first = v_first;
        s->ring_cap = ring_cap;
        s->ring_stride = ring_stride;
        s->ring = (float *)(base + off_ring);
        s->ring_mn = (do_ar && !v_first) ? (float *)(base + off_mn) : NULL;
        s->ring_mx = (do_ar && !v_first) ? (float *)(base + off_mx) : NULL;
        s->ring_srcy = (int32_t *)(base + off_srcy);
        s->decode_row = (float *)(base + off_dec);
        s->staging = (float *)(base + off_stg);
        s->stage_mn = do_ar ? (float *)(base + off_smn) : NULL;
        s->stage_mx = do_ar ? (float *)(base + off_smx) : NULL;
        s->vstaging = v_first ? (float *)(base + off_vstg) : NULL;
        s->vs_mn = (v_first && do_ar) ? (float *)(base + off_vmn) : NULL;
        s->vs_mx = (v_first && do_ar) ? (float *)(base + off_vmx) : NULL;
        s->stage_tmp = (v_first && do_ar) ? (float *)(base + off_stmp) : NULL;
        s->repwin = nrep ? (float *)(base + off_rep) : NULL;

        s->premult = premult;
        s->already_premult = (o.alpha == TIR_ALPHA_PREMULTIPLIED);
        s->hicomp_ch = hicomp_ch;
        s->ar_strength = ar_strength;
        /* highlight compression clamps negatives to 0 after expand -- but only
     * when it is actually applied (hicomp_ch != 0). Guarding on hicomp_ch
     * rather than the raw flag avoids clamping legitimately-signed data (e.g.
     * normal-map X/Y components, where hicomp is disabled). */
    s->clamp_lo = hicomp_ch && o.clamp_min < 0.0f ? 0.0f : o.clamp_min;
        s->clamp_hi = o.clamp_max;
        s->do_clamp = (s->clamp_lo > -INFINITY) || (s->clamp_hi < INFINITY);
        s->nlen_out = o.normal_length_out;
        normal_map_coeffs(o.normal_encoding, src_type, &s->n_dec_a,
                          &s->n_dec_b);
        normal_map_coeffs(o.normal_encoding, dst_type, &s->n_enc_a,
                          &s->n_enc_b);

        /* kernels: deterministic pins the scalar reference set */
        if (o.deterministic) {
            tir__kernels_set_scalar(&s->k);
        } else {
            tir__kernels_init();
            s->k = tir__k;
        }

        tir__axis_fill(&px, &dx, &s->fx, (double *)(base + off_scratch));
        tir__axis_fill(&py, &dy, &s->fy, (double *)(base + off_scratch));
        tir__reset(s);
        *out = s;
    }
    return TIR_SUCCESS;
}

/* ---- streaming wrappers -------------------------------------------------------- */

tir_result tir_sampler_reset(tir_sampler *s) {
    if (!s) return TIR_ERROR_INVALID_ARGUMENT;
    tir__reset(s);
    return TIR_SUCCESS;
}

tir_result tir_sampler_push_row(tir_sampler *s, int src_y,
                                const void *src_row) {
    return tir__push_row(s, src_y, src_row);
}

tir_result tir_sampler_pull_row(tir_sampler *s, int *out_dst_y,
                                void *dst_row) {
    return tir__pull_row(s, out_dst_y, dst_row);
}

/* ---- whole image ------------------------------------------------------------------ */

/* Produce destination rows [y0, y1). Band runs (y0 > 0) fast-forward the
 * push cursor to the first source row the band's window needs. */
tir_result tir__run_range(tir_sampler *s, const void *src,
                          size_t src_row_stride_bytes, void *dst,
                          size_t dst_row_stride_bytes, int y0, int y1) {
    tir__reset(s);
    s->cur_dst = y0;
    if (y0 > 0) {
        /* Fast-forward the push cursor only when the whole band lies inside the
         * contiguous "fast" range. Any exception output in the band (a WRAP or
         * REFLECT border) may reference an arbitrary source row -- e.g. a WRAP
         * bottom output wraps back to row 0 -- so those rows must be pushed;
         * start from row 0 in that case. (Bands with exceptions already buffer
         * the whole image, since n_exc>0 forces ring_cap = src_h.) */
        int band_fast = (y0 >= s->fy.fast_lo && y1 <= s->fy.fast_hi);
        int first = 0;
        if (band_fast) {
            /* start[] is NOT monotonic: an output whose center lands exactly
             * on an integer source row collapses to a single-tap window (the
             * off-integer taps of a windowed-sinc are exactly zero), spiking
             * its start above its neighbours'. Fast-forward to the minimum
             * start over the whole band so no row a later output needs is
             * skipped -- otherwise the band deadlocks (pull waits on a row the
             * push cursor stepped past). */
            int q;
            first = s->fy.start[y0];
            for (q = y0 + 1; q < y1; ++q)
                if (s->fy.start[q] < first) first = s->fy.start[q];
        }
        if (s->opt.nonfinite == TIR_NONFINITE_REPAIR && first > 0)
            first--; /* one extra row so the repair window is complete */
        s->rows_pushed = first;
        s->push_base = first;
    }
    while (s->cur_dst < y1) {
        int dy;
        char *drow = (char *)dst + (size_t)s->cur_dst * dst_row_stride_bytes;
        tir_result rc = tir__pull_row(s, &dy, drow);
        if (rc == TIR_WOULD_BLOCK) {
            const char *srow =
                (const char *)src +
                (size_t)s->rows_pushed * src_row_stride_bytes;
            if (s->rows_pushed >= s->sh)
                return TIR_ERROR_INVALID_ARGUMENT; /* unreachable */
            rc = tir__push_row(s, s->rows_pushed, srow);
            if (!TIR_OK(rc)) return rc;
            continue;
        }
        if (!TIR_OK(rc)) return rc;
        if (dy >= s->dh) break;
    }
    return TIR_SUCCESS;
}

tir_result tir_sampler_run(tir_sampler *s, const void *src,
                           size_t src_row_stride_bytes, void *dst,
                           size_t dst_row_stride_bytes) {
    if (!s || !src || !dst) return TIR_ERROR_INVALID_ARGUMENT;
    if (src_row_stride_bytes == 0)
        src_row_stride_bytes =
            (size_t)s->sw * (size_t)s->ch * tir__pixel_size(s->st);
    if (dst_row_stride_bytes == 0)
        dst_row_stride_bytes =
            (size_t)s->dw * (size_t)s->ch * tir__pixel_size(s->dt);
#if defined(TIR_ENABLE_THREADS)
    if (s->opt.num_threads > 1)
        return tir__run_threads(s, src, src_row_stride_bytes, dst,
                                dst_row_stride_bytes, s->opt.num_threads);
#endif
    return tir__run_range(s, src, src_row_stride_bytes, dst,
                          dst_row_stride_bytes, 0, s->dh);
}

tir_result tir_resize(const tir_allocator *a, const tir_image_view *src,
                      const tir_image_view *dst, const tir_options *opt) {
    tir_sampler *s = NULL;
    tir_result rc;
    if (!src || !dst || !src->data || !dst->data)
        return TIR_ERROR_INVALID_ARGUMENT;
    if (src->channels != dst->channels) return TIR_ERROR_INVALID_ARGUMENT;
    rc = tir_sampler_create(a, src->width, src->height, dst->width,
                            dst->height, src->channels, src->type, dst->type,
                            opt, &s);
    if (!TIR_OK(rc)) return rc;
    rc = tir_sampler_run(s, src->data, src->row_stride_bytes, dst->data,
                         dst->row_stride_bytes);
    tir_sampler_destroy(s);
    return rc;
}
