/*
 * tir - filter kernels + per-axis coefficient table builder.
 *
 * Tables are built once per sampler in double precision: per output index a
 * contiguous in-bounds source run with normalized, zero-padded weights.
 * Edge behavior is folded into the weights (accumulated at the mapped
 * indices) so the hot loops never branch on borders. EDGE_WRAP windows that
 * straddle the axis boundary cannot be made contiguous and become explicit
 * (index, weight) exception entries handled by a scalar gather.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include "tir_internal.h"

#define TIR_PI 3.14159265358979323846

/* ---- filter kernels (double precision, build time only) ------------------ */

static double kd_cubic(double a, double B, double C) {
    if (a < 1.0)
        return ((12.0 - 9.0 * B - 6.0 * C) * a * a * a +
                (-18.0 + 12.0 * B + 6.0 * C) * a * a + (6.0 - 2.0 * B)) /
               6.0;
    if (a < 2.0)
        return ((-B - 6.0 * C) * a * a * a + (6.0 * B + 30.0 * C) * a * a +
                (-12.0 * B - 48.0 * C) * a + (8.0 * B + 24.0 * C)) /
               6.0;
    return 0.0;
}

static double kd_sinc(double x) {
    if (x == 0.0) return 1.0;
    x *= TIR_PI;
    return sin(x) / x;
}

static double kd_lanczos(double a, double lobes) {
    if (a >= lobes) return 0.0;
    return kd_sinc(a) * kd_sinc(a / lobes);
}

/* Modified Bessel function of the first kind, order 0 (series). */
static double kd_bessel_i0(double x) {
    double sum = 1.0, term = 1.0, xx = 0.25 * x * x;
    int k;
    for (k = 1; k < 30; ++k) {
        term *= xx / ((double)k * (double)k);
        sum += term;
        if (term < 1e-12 * sum) break;
    }
    return sum;
}

/* Kaiser-windowed sinc of half-width `radius`. */
static double kd_kaiser(double a, double radius, double beta) {
    double t;
    if (a >= radius) return 0.0;
    t = a / radius;
    return kd_sinc(a) * kd_bessel_i0(beta * sqrt(1.0 - t * t)) /
           kd_bessel_i0(beta);
}

static double kd_weight(tir_filter f, double t, double sigma) {
    double a = t < 0.0 ? -t : t;
    switch (f) {
        case TIR_FILTER_TRIANGLE:
            return a < 1.0 ? 1.0 - a : 0.0;
        case TIR_FILTER_BSPLINE:
            return kd_cubic(a, 1.0, 0.0);
        case TIR_FILTER_GAUSSIAN:
            return exp(-(a * a) / (2.0 * sigma * sigma));
        case TIR_FILTER_MITCHELL:
            return kd_cubic(a, 1.0 / 3.0, 1.0 / 3.0);
        case TIR_FILTER_CATMULL_ROM:
            return kd_cubic(a, 0.0, 0.5);
        case TIR_FILTER_LANCZOS2:
            return kd_lanczos(a, 2.0);
        case TIR_FILTER_LANCZOS3:
            return kd_lanczos(a, 3.0);
        case TIR_FILTER_KAISER:
            return kd_kaiser(a, 3.0, 8.0);
        default: /* box is special-cased (exact interval overlap) */
            return a <= 0.5 ? 1.0 : 0.0;
    }
}

static double filter_radius(tir_filter f, double sigma) {
    switch (f) {
        case TIR_FILTER_BOX:
            return 0.5;
        case TIR_FILTER_TRIANGLE:
            return 1.0;
        case TIR_FILTER_GAUSSIAN:
            /* truncate at 3 sigma (<= 0.3% tail, renormalized away) */
            return 3.0 * sigma < 0.5 ? 0.5 : 3.0 * sigma;
        case TIR_FILTER_LANCZOS3:
        case TIR_FILTER_KAISER:
            return 3.0;
        default: /* cubics, lanczos2 */
            return 2.0;
    }
}

/* ---- mapping -------------------------------------------------------------- */

static int edge_map(int j, int n, tir_edge_mode m) {
    if (n == 1) return 0;
    if (j >= 0 && j < n) return j;
    if (m == TIR_EDGE_CLAMP) return j < 0 ? 0 : n - 1;
    if (m == TIR_EDGE_WRAP) {
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

static int ifloor_div(int a, int b) { /* b > 0 */
    int q = a / b;
    return (a % b != 0 && (a < 0)) ? q - 1 : q;
}

typedef struct axis_geom {
    double ratio;     /* source samples per destination sample */
    double support;   /* window half-width in source samples */
    double kscale;    /* kernel argument divisor */
    double box_sb;    /* box half-width (exact overlap) */
    int is_down;
    int use_vertex;   /* grid-vertex mapping active */
} axis_geom;

static void axis_geom_init(const tir__axis_params *p, axis_geom *g) {
    double ratio;
    g->use_vertex = (p->reg == TIR_REG_GRID_VERTEX && p->d > 1 && p->s > 1);
    if (g->use_vertex)
        ratio = (double)(p->s - 1) / (double)(p->d - 1);
    else
        ratio = (double)p->s / (double)p->d;
    g->ratio = ratio;
    g->is_down = ratio > 1.0;
    {
        double fscale = (ratio > 1.0 ? ratio : 1.0) * (double)p->filter_scale;
        if (fscale < 1.0) fscale = 1.0;
        g->kscale = fscale;
        g->support = filter_radius(p->filter, p->gaussian_sigma) * fscale;
        g->box_sb = 0.5 * fscale;
    }
}

static double axis_center(const axis_geom *g, const tir__axis_params *p,
                          int i) {
    if (g->use_vertex)
        return (double)i * (double)(p->s - 1) / (double)(p->d - 1);
    return ((double)i + 0.5) * ((double)p->s / (double)p->d) - 0.5;
}

/* Raw (unmapped) window for output i: taps [lo, lo+n) with trimmed,
 * normalized weights in w. Returns n (>= 1). */
static int window_weights(const tir__axis_params *p, const axis_geom *g,
                          int i, int *out_lo, double *w, int win_max) {
    double center = axis_center(g, p, i);
    int lo, hi, n, t;
    if (p->filter == TIR_FILTER_BOX) {
        double a = center - g->box_sb, b = center + g->box_sb;
        lo = (int)ceil(a - 0.5);
        hi = (int)floor(b + 0.5);
        if (hi < lo) hi = lo;
        if (hi - lo + 1 > win_max) hi = lo + win_max - 1;
        for (t = lo; t <= hi; ++t) {
            double l = (double)t - 0.5, r = (double)t + 0.5;
            double ov = (r < b ? r : b) - (l > a ? l : a);
            w[t - lo] = ov > 0.0 ? ov : 0.0;
        }
    } else {
        lo = (int)ceil(center - g->support);
        hi = (int)floor(center + g->support);
        if (hi < lo) hi = lo;
        if (hi - lo + 1 > win_max) hi = lo + win_max - 1;
        for (t = lo; t <= hi; ++t)
            w[t - lo] = kd_weight(p->filter, (center - (double)t) / g->kscale,
                                  p->gaussian_sigma);
    }
    n = hi - lo + 1;
    /* trim zero-weight taps at both ends */
    while (n > 1 && fabs(w[0]) <= 1e-10) {
        memmove(w, w + 1, (size_t)(n - 1) * sizeof(double));
        lo++;
        n--;
    }
    while (n > 1 && fabs(w[n - 1]) <= 1e-10) n--;
    /* normalize (double) */
    {
        double sum = 0.0;
        for (t = 0; t < n; ++t) sum += w[t];
        if (sum == 0.0) {
            int c = (int)floor(center + 0.5);
            lo = c;
            n = 1;
            w[0] = 1.0;
        } else {
            double inv = 1.0 / sum;
            for (t = 0; t < n; ++t) w[t] *= inv;
        }
    }
    *out_lo = lo;
    return n;
}

/* Fold analysis: is the mapped window a contiguous in-bounds run?
 * Returns 1 (fast) with [*run_lo, *run_hi] or 0 (wrap exception). */
static int window_fold(const tir__axis_params *p, int lo, int n, int *run_lo,
                       int *run_hi) {
    int hi = lo + n - 1;
    if (lo >= 0 && hi < p->s) {
        *run_lo = lo;
        *run_hi = hi;
        return 1;
    }
    if (p->edge == TIR_EDGE_WRAP) {
        if (n >= p->s) { /* window covers the whole axis: dense run */
            *run_lo = 0;
            *run_hi = p->s - 1;
            return 1;
        }
        if (ifloor_div(lo, p->s) != ifloor_div(hi, p->s)) return 0;
        {
            int base = ifloor_div(lo, p->s) * p->s;
            *run_lo = lo - base;
            *run_hi = hi - base;
            return 1;
        }
    }
    { /* CLAMP / REFLECT: run of the mapped indices (always contiguous) */
        int mn = p->s, mx = -1, t;
        for (t = lo; t <= hi; ++t) {
            int m = edge_map(t, p->s, p->edge);
            if (m < mn) mn = m;
            if (m > mx) mx = m;
        }
        *run_lo = mn;
        *run_hi = mx;
        return 1;
    }
}

/* Anti-ringing reduced footprint (contiguous mapped run, before the
 * intersection with the filter run). */
static void ar_window(const tir__axis_params *p, const axis_geom *g, int i,
                      int *out_lo, int *out_hi) {
    double center = axis_center(g, p, i);
    int k = g->is_down ? (int)g->ratio + 1 : 2;
    int lo = (int)floor(center - 0.5 * (double)(k - 1));
    int hi = lo + k - 1;
    int mn = p->s, mx = -1, t;
    for (t = lo; t <= hi; ++t) {
        int m = edge_map(t, p->s, p->edge);
        if (m < mn) mn = m;
        if (m > mx) mx = m;
    }
    if (mx < mn) { mn = 0; mx = 0; }
    *out_lo = mn;
    *out_hi = mx;
}

/* ---- measure --------------------------------------------------------------
 * Window width bound: full raw window + 2 slack. Also bounds fold runs
 * (clamp/reflect runs and wrap dense runs never exceed the raw width). */
static int win_bound(const axis_geom *g, const tir__axis_params *p) {
    double w = (p->filter == TIR_FILTER_BOX) ? (2.0 * g->box_sb + 3.0)
                                             : (2.0 * g->support + 3.0);
    if (w < 4.0) w = 4.0;
    if (w > 2147483000.0) return -1;
    return (int)w;
}

tir_result tir__axis_measure(const tir__axis_params *p, tir__axis_dims *out) {
    axis_geom g;
    int i, taps = 1, ar_taps = 1;
    int fast_lo = 0, fast_hi = p->d, seen_fast = 0;
    double *ws;
    axis_geom_init(p, &g);
    memset(out, 0, sizeof(*out));
    out->win_max = win_bound(&g, p);
    if (out->win_max < 0) return TIR_ERROR_TOO_LARGE;

    /* Classify with the SAME trimmed window that fill will build. An untrimmed
     * window can be a dense wrap run (fast) yet, once its ~0 end taps are
     * trimmed, become a sparse period-straddling window (exception) -- and a
     * measure/fill disagreement corrupts the arena (fill's fast branch would
     * then run with an unset run_lo/run_hi). window_weights trims exactly as
     * fill does; the temporary win_max buffer is no larger than the
     * coefficient scratch tir_sampler_create allocates anyway. */
    ws = (double *)malloc((size_t)out->win_max * sizeof(double));
    if (!ws) return TIR_ERROR_OUT_OF_MEMORY;

    for (i = 0; i < p->d; ++i) {
        int lo, n, run_lo, run_hi, fast;
        n = window_weights(p, &g, i, &lo, ws, out->win_max);
        fast = window_fold(p, lo, n, &run_lo, &run_hi);
        if (fast) {
            int cnt = run_hi - run_lo + 1;
            if (cnt > taps) taps = cnt;
            seen_fast = 1;
            if (i >= fast_hi) { /* fast after a suffix exception: unsupported */
                free(ws);
                return TIR_ERROR_INVALID_ARGUMENT;
            }
        } else {
            if (n > taps) taps = n; /* exceptions share the taps bound */
            if (!seen_fast)
                fast_lo = i + 1;
            else if (fast_hi == p->d)
                fast_hi = i;
        }
        {
            int alo, ahi, acnt;
            ar_window(p, &g, i, &alo, &ahi);
            acnt = ahi - alo + 1;
            if (acnt > ar_taps) ar_taps = acnt;
        }
    }
    free(ws);
    if (!seen_fast) fast_hi = fast_lo; /* everything exceptional (tiny wrap) */
    out->taps = taps;
    out->padded = (int)tir__align_up((size_t)taps, TIR_PAD);
    out->fast_lo = fast_lo;
    out->fast_hi = fast_hi;
    out->n_exc = fast_lo + (p->d - fast_hi);
    out->ar_taps = ar_taps;
    return TIR_SUCCESS;
}

/* ---- fill ------------------------------------------------------------------ */

void tir__axis_fill(const tir__axis_params *p, const tir__axis_dims *dims,
                    tir__axis *ax, double *scratch) {
    axis_geom g;
    double *raw = scratch;                    /* [win_max] */
    double *fold = scratch + dims->win_max;   /* [win_max] */
    int i;
    axis_geom_init(p, &g);
    ax->taps = dims->taps;
    ax->padded = dims->padded;
    ax->fast_lo = dims->fast_lo;
    ax->fast_hi = dims->fast_hi;
    ax->n_exc = dims->n_exc;
    ax->ar_taps = dims->ar_taps;
    ax->is_down = g.is_down;

    for (i = 0; i < p->d; ++i) {
        int lo, n, run_lo, run_hi, t;
        n = window_weights(p, &g, i, &lo, raw, dims->win_max);
        if (i >= ax->fast_lo && i < ax->fast_hi) {
            int cnt;
            float *wrow = ax->w + (size_t)i * (size_t)ax->padded;
            window_fold(p, lo, n, &run_lo, &run_hi);
            cnt = run_hi - run_lo + 1;
            for (t = 0; t < cnt; ++t) fold[t] = 0.0;
            for (t = 0; t < n; ++t) {
                int m = edge_map(lo + t, p->s, p->edge);
                fold[m - run_lo] += raw[t];
            }
            ax->start[i] = run_lo;
            ax->count[i] = cnt;
            for (t = 0; t < cnt; ++t) wrow[t] = (float)fold[t];
            for (; t < ax->padded; ++t) wrow[t] = 0.0f;
        } else {
            /* wrap-border exception: explicit gather entry */
            int e = (i < ax->fast_lo) ? i : ax->fast_lo + (i - ax->fast_hi);
            int32_t *idx = ax->exc_idx + (size_t)e * (size_t)ax->taps;
            float *ew = ax->exc_w + (size_t)e * (size_t)ax->taps;
            ax->start[i] = 0;
            ax->count[i] = n;
            for (t = 0; t < n; ++t) {
                idx[t] = (int32_t)edge_map(lo + t, p->s, p->edge);
                ew[t] = (float)raw[t];
            }
            for (; t < ax->taps; ++t) {
                idx[t] = 0;
                ew[t] = 0.0f;
            }
        }
        /* anti-ringing footprint: mapped run intersected with the filter run
         * (fast outputs) so the vertical pass only touches resident rows. */
        {
            int alo, ahi;
            ar_window(p, &g, i, &alo, &ahi);
            if (i >= ax->fast_lo && i < ax->fast_hi) {
                int flo = ax->start[i], fhi = ax->start[i] + ax->count[i] - 1;
                if (alo < flo) alo = flo;
                if (ahi > fhi) ahi = fhi;
                if (ahi < alo) { /* disjoint (degenerate): nearest run tap */
                    alo = flo;
                    ahi = flo;
                }
            }
            ax->ar_start[i] = alo;
            ax->ar_count[i] = ahi - alo + 1;
        }
    }
}
