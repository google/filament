/*
 * tir - streaming ring-buffer resample engine (horizontal-first).
 *
 * push_row: convert -> scrub -> decode -> premult -> compress -> horizontal
 * pass into the ring (plus reduced-footprint min/max rows for anti-ringing).
 * pull_row: vertical pass over resident ring rows -> anti-ring clamp (once,
 * after both passes; ring rows are never clamped) -> expand -> unpremult ->
 * renormalize -> output clamp -> encode.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tir_internal.h"

static float *ring_row(tir_sampler *s, float *base, int src_y) {
    return base + (size_t)(src_y % s->ring_cap) * s->ring_stride;
}

void tir__reset(tir_sampler *s) {
    int i;
    s->rows_pushed = 0;
    s->push_base = 0;
    s->cur_dst = 0;
    s->last_pending = 0;
    s->err_sticky = TIR_SUCCESS;
    for (i = 0; i < s->ring_cap; ++i) s->ring_srcy[i] = -1;
    /* zero the decode-row slack so padded horizontal taps read finite 0s */
    {
        size_t used = (size_t)s->sw * (size_t)s->wc;
        size_t total = ((size_t)s->sw + (size_t)s->fx.padded) * (size_t)s->wc;
        memset(s->decode_row + used, 0, (total - used) * sizeof(float));
        if (s->vstaging)
            memset(s->vstaging + used, 0, (total - used) * sizeof(float));
    }
}

/* ---- horizontal helpers ----------------------------------------------------- */

/* Wrap-border exception outputs: scalar gather over explicit indices. */
static void h_exc_dot(const tir__axis *ax, const float *src, float *dst,
                      int wc, int x) {
    int e = (x < ax->fast_lo) ? x : ax->fast_lo + (x - ax->fast_hi);
    const int32_t *idx = ax->exc_idx + (size_t)e * (size_t)ax->taps;
    const float *ew = ax->exc_w + (size_t)e * (size_t)ax->taps;
    int c, t, n = ax->count[x];
    for (c = 0; c < wc; ++c) {
        float acc = 0.0f;
        for (t = 0; t < n; ++t)
            acc += ew[t] * src[(size_t)idx[t] * (size_t)wc + c];
        dst[(size_t)x * (size_t)wc + c] = acc;
    }
}

/* Convert one raw source row into f32 at dst. */
static void convert_in(tir_sampler *s, float *dst, const void *src, size_t n) {
    switch (s->st) {
        case TIR_F32:
            memcpy(dst, src, n * sizeof(float));
            break;
        case TIR_F16:
            s->k.f16_to_f32(dst, (const uint16_t *)src, n);
            break;
        case TIR_U8:
            s->k.u8_to_f32(dst, (const uint8_t *)src, n);
            break;
        default:
            s->k.u16_to_f32(dst, (const uint16_t *)src, n);
            break;
    }
}

static void convert_out(tir_sampler *s, void *dst, const float *src, size_t n) {
    switch (s->dt) {
        case TIR_F32:
            memcpy(dst, src, n * sizeof(float));
            break;
        case TIR_F16:
            s->k.f32_to_f16((uint16_t *)dst, src, n);
            break;
        case TIR_U8:
            s->k.f32_to_u8((uint8_t *)dst, src, n);
            break;
        default:
            s->k.f32_to_u16((uint16_t *)dst, src, n);
            break;
    }
}

/* Horizontal pass: one filtering-domain source-width row -> dst-width row. */
static void h_pass(tir_sampler *s, const float *src, float *out) {
    int x;
    for (x = 0; x < s->fx.fast_lo; ++x) h_exc_dot(&s->fx, src, out, s->wc, x);
    s->k.h_dot[s->wc - 1](out, src, s->fx.start, s->fx.count, s->fx.w,
                          s->fx.padded, s->fx.fast_lo, s->fx.fast_hi);
    for (x = s->fx.fast_hi; x < s->dw; ++x)
        h_exc_dot(&s->fx, src, out, s->wc, x);
}

/* decode_row holds the final filtering-domain row: run the horizontal pass
 * into the ring slot for source row r (h-first), or park the raw row there
 * for the vertical-first pipeline. */
static void finalize_row(tir_sampler *s, int r) {
    int slot = r % s->ring_cap;
    float *out = s->ring + (size_t)slot * s->ring_stride;

    if (s->opt.mode == TIR_MODE_NORMAL_MAP)
        tir__normal_decode_row(s, s->decode_row);
    if (s->premult) s->k.premult4(s->decode_row, (size_t)s->sw);
    if (s->hicomp_ch)
        tir__rangecompress_row(s->decode_row, (size_t)s->sw, s->wc,
                               s->hicomp_ch);

    if (s->v_first) {
        memcpy(out, s->decode_row,
               (size_t)s->sw * (size_t)s->wc * sizeof(float));
    } else {
        h_pass(s, s->decode_row, out);
        if (s->ring_mn) {
            float *mn = s->ring_mn + (size_t)slot * s->ring_stride;
            float *mx = s->ring_mx + (size_t)slot * s->ring_stride;
            /* anti-ring runs are contiguous for every output, borders incl. */
            s->k.h_minmax(mn, mx, s->decode_row, s->fx.ar_start,
                          s->fx.ar_count, 0, s->dw, s->wc);
        }
    }
    s->ring_srcy[slot] = r;
}

/* ---- push -------------------------------------------------------------------- */

/* REPAIR path: rebuild decode_row for source row r from the raw 3-row window
 * held in repwin, then run the horizontal pass. */
static void finalize_from_repwin(tir_sampler *s, int r) {
    size_t nraw = (size_t)s->sw * (size_t)s->ch;
    const float *prev = s->repwin + (size_t)((r > 0 ? r - 1 : 0) % 3) * nraw;
    const float *cur = s->repwin + (size_t)(r % 3) * nraw;
    const float *next =
        s->repwin + (size_t)((r < s->sh - 1 ? r + 1 : r) % 3) * nraw;
    tir__nonfinite_repair(s->decode_row, prev, cur, next, s->sw, s->ch);
    finalize_row(s, r);
}

tir_result tir__push_row(tir_sampler *s, int src_y, const void *src_row) {
    int fin = -1, repair;
    size_t nraw;
    float *dec;

    if (!s || !src_row) return TIR_ERROR_INVALID_ARGUMENT;
    if (s->err_sticky < 0) return s->err_sticky;
    if (src_y < 0 || src_y >= s->sh) return TIR_ERROR_INVALID_ARGUMENT;
    if (src_y != s->rows_pushed) return TIR_ERROR_ORDER;
    repair = (s->opt.nonfinite == TIR_NONFINITE_REPAIR);
    nraw = (size_t)s->sw * (size_t)s->ch;

    /* REPAIR needs one row of lookahead: pushing y finalizes y-1; the very
     * last row is deferred until a pull needs it (its ring slot is then
     * guaranteed evictable, which is not true at push time). */
    fin = repair ? src_y - 1 : src_y;

    /* Refuse to evict a ring row the next pull still needs: the caller must
     * drain pull_row first. Impossible when the ring holds the full image
     * (wrap-y), where slots are unique. */
    if (fin >= 0 && s->ring_cap < s->sh && s->cur_dst < s->dh) {
        int old = s->ring_srcy[fin % s->ring_cap];
        if (old >= 0 && old >= s->fy.start[s->cur_dst])
            return TIR_WOULD_BLOCK;
    }

    dec = repair ? s->repwin + (size_t)(src_y % 3) * nraw : s->decode_row;
    convert_in(s, dec, src_row, nraw);

    if (s->opt.nonfinite == TIR_NONFINITE_ERROR) {
        if (tir__row_has_nonfinite(dec, nraw)) {
            s->err_sticky = TIR_ERROR_NONFINITE;
            return s->err_sticky;
        }
    } else if (s->opt.nonfinite == TIR_NONFINITE_ZERO) {
        tir__nonfinite_zero(dec, nraw);
    }

    if (fin >= 0) {
        if (repair) {
            /* band runs start mid-image: the row before push_base has no
             * predecessor in repwin (and is not needed) - skip it */
            if (fin == 0 || fin - 1 >= s->push_base)
                finalize_from_repwin(s, fin);
        } else {
            finalize_row(s, fin);
        }
    }
    if (repair && src_y == s->sh - 1) s->last_pending = 1;
    s->rows_pushed = src_y + 1;
    return TIR_SUCCESS;
}

/* ---- pull -------------------------------------------------------------------- */

/* A required source row is ready when its ring slot holds it; the deferred
 * REPAIR last row is finalized on demand (when it is part of the current
 * pull window its slot can only hold a row older than the window start, so
 * eviction is always safe here). */
static int row_ready(tir_sampler *s, int r) {
    if (s->ring_srcy[r % s->ring_cap] == r) return 1;
    if (s->last_pending && r == s->sh - 1) {
        finalize_from_repwin(s, r);
        s->last_pending = 0;
        return 1;
    }
    return 0;
}

/* Pre-normalization |N| for normal_length_out when renormalize is off. */
static void lengths_only(const float *xyz, float *len, size_t npix) {
    size_t i;
    for (i = 0; i < npix; ++i) {
        float x = xyz[i * 3 + 0], y = xyz[i * 3 + 1], z = xyz[i * 3 + 2];
        len[i] = sqrtf(x * x + y * y + z * z);
    }
}

tir_result tir__pull_row(tir_sampler *s, int *out_dst_y, void *dst_row) {
    int oy, t, count, is_fast;
    const int32_t *rows_idx = NULL;
    const float *wt;
    size_t rowf;

    if (!s || !out_dst_y) return TIR_ERROR_INVALID_ARGUMENT;
    if (s->err_sticky < 0) return s->err_sticky;
    oy = s->cur_dst;
    if (oy >= s->dh) {
        *out_dst_y = s->dh;
        return TIR_SUCCESS;
    }
    is_fast = (oy >= s->fy.fast_lo && oy < s->fy.fast_hi);
    count = s->fy.count[oy];
    if (is_fast) {
        int first = s->fy.start[oy];
        for (t = 0; t < count; ++t) {
            if (!row_ready(s, first + t)) {
                *out_dst_y = -1;
                return TIR_WOULD_BLOCK;
            }
        }
        wt = s->fy.w + (size_t)oy * (size_t)s->fy.padded;
    } else {
        int e = (oy < s->fy.fast_lo) ? oy
                                     : s->fy.fast_lo + (oy - s->fy.fast_hi);
        rows_idx = s->fy.exc_idx + (size_t)e * (size_t)s->fy.taps;
        wt = s->fy.exc_w + (size_t)e * (size_t)s->fy.taps;
        for (t = 0; t < count; ++t) {
            if (!row_ready(s, rows_idx[t])) {
                *out_dst_y = -1;
                return TIR_WOULD_BLOCK;
            }
        }
    }
    if (!dst_row) return TIR_ERROR_INVALID_ARGUMENT;

    rowf = (size_t)s->dw * (size_t)s->wc;
    {
        /* vertical blend target: dst-width staging (h-first) or the
         * src-width vstaging that still needs the horizontal pass */
        float *vdst = s->v_first ? s->vstaging : s->staging;
        size_t vn = s->v_first ? (size_t)s->sw * (size_t)s->wc : rowf;
        int first = is_fast ? s->fy.start[oy] : 0;
#define TIR__VROW(T) \
    ring_row(s, s->ring, is_fast ? first + (T) : rows_idx[T])
        s->k.v_mul(vdst, TIR__VROW(0), wt[0], vn);
        t = 1;
        while (t + 4 <= count) {
            const float *rows4[4];
            rows4[0] = TIR__VROW(t + 0);
            rows4[1] = TIR__VROW(t + 1);
            rows4[2] = TIR__VROW(t + 2);
            rows4[3] = TIR__VROW(t + 3);
            s->k.v_fma4(vdst, rows4, wt + t, vn);
            t += 4;
        }
        for (; t < count; ++t) s->k.v_fma(vdst, TIR__VROW(t), wt[t], vn);
#undef TIR__VROW
    }

    if (s->v_first) {
        h_pass(s, s->vstaging, s->staging);
        if (s->vs_mn && s->ar_strength > 0.0f) {
            /* min/max of the reduced 2D footprint, transposed order: raw
             * ring rows combine vertically, then reduce horizontally */
            int alo = s->fy.ar_start[oy], acnt = s->fy.ar_count[oy];
            size_t vn = (size_t)s->sw * (size_t)s->wc;
            memcpy(s->vs_mn, ring_row(s, s->ring, alo), vn * sizeof(float));
            memcpy(s->vs_mx, s->vs_mn, vn * sizeof(float));
            for (t = 1; t < acnt; ++t) {
                const float *r = ring_row(s, s->ring, alo + t);
                s->k.minmax_combine(s->vs_mn, s->vs_mx, r, r, vn);
            }
            s->k.h_minmax(s->stage_mn, s->stage_tmp, s->vs_mn,
                          s->fx.ar_start, s->fx.ar_count, 0, s->dw, s->wc);
            s->k.h_minmax(s->stage_tmp, s->stage_mx, s->vs_mx,
                          s->fx.ar_start, s->fx.ar_count, 0, s->dw, s->wc);
            s->k.antiring_apply(s->staging, s->stage_mn, s->stage_mx,
                                s->ar_strength, rowf);
        }
    } else if (s->ring_mn && s->ar_strength > 0.0f) {
        int alo = s->fy.ar_start[oy], acnt = s->fy.ar_count[oy];
        memcpy(s->stage_mn, ring_row(s, s->ring_mn, alo),
               rowf * sizeof(float));
        memcpy(s->stage_mx, ring_row(s, s->ring_mx, alo),
               rowf * sizeof(float));
        for (t = 1; t < acnt; ++t) {
            s->k.minmax_combine(s->stage_mn, s->stage_mx,
                                ring_row(s, s->ring_mn, alo + t),
                                ring_row(s, s->ring_mx, alo + t), rowf);
        }
        s->k.antiring_apply(s->staging, s->stage_mn, s->stage_mx,
                            s->ar_strength, rowf);
    }

    if (s->hicomp_ch)
        tir__rangeexpand_row(s->staging, (size_t)s->dw, s->wc, s->hicomp_ch);
    if (s->premult) s->k.unpremult4(s->staging, (size_t)s->dw);
    if (s->opt.mode == TIR_MODE_NORMAL_MAP) {
        if (s->opt.normal_renormalize)
            s->k.normalize3(s->staging,
                            s->nlen_out ? s->nlen_out + (size_t)oy * s->dw
                                        : NULL,
                            (size_t)s->dw);
        else if (s->nlen_out)
            lengths_only(s->staging, s->nlen_out + (size_t)oy * s->dw,
                         (size_t)s->dw);
        tir__normal_encode_row(s, s->staging);
    }
    {
        size_t nout = (size_t)s->dw * (size_t)s->ch;
        if (s->do_clamp)
            s->k.clamp_range(s->staging, s->clamp_lo, s->clamp_hi, nout);
        convert_out(s, dst_row, s->staging, nout);
    }
    *out_dst_y = oy;
    s->cur_dst = oy + 1;
    return TIR_SUCCESS;
}
