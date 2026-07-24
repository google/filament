/*
 * tir - internal declarations shared across the implementation files.
 *
 * Not a public header. Everything here may change without notice.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TIR_INTERNAL_H_
#define TIR_INTERNAL_H_

#include <math.h>
#include <string.h>

#include "tir.h"

/* ===========================================================================
 * Architecture gates
 * ========================================================================= */
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || \
    defined(_M_IX86)
#define TIR_X86 1
#endif

#if (defined(__aarch64__) || defined(_M_ARM64)) && \
    (defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(_M_ARM64))
#define TIR_NEON 1
#endif

#define TIR_MAX_DIMENSION (1 << 24)

/* Weight rows are zero-padded to a multiple of TIR_PAD floats so the
 * horizontal kernels loop in whole blocks with no scalar tail. 4 keeps the
 * tables compact (better cache) for the common small-tap filters; every
 * kernel reads the weight table in <=4-wide blocks or predicated (SVE). */
#define TIR_PAD 4

/* ===========================================================================
 * Overflow-checked size arithmetic (all hostile-input math goes through it)
 * ========================================================================= */
static inline int tir__mul_ok(size_t a, size_t b, size_t *out) {
    if (b != 0 && a > (size_t)-1 / b) return 0;
    *out = a * b;
    return 1;
}
static inline int tir__add_ok(size_t a, size_t b, size_t *out) {
    if (a > (size_t)-1 - b) return 0;
    *out = a + b;
    return 1;
}
static inline size_t tir__align_up(size_t v, size_t a) {
    return (v + (a - 1)) & ~(a - 1);
}

const tir_allocator *tir__default_allocator(void);

/* ===========================================================================
 * Kernel dispatch table
 * ========================================================================= */

/* Horizontal coefficient-table dot product for a fixed channel count.
 * For each output x in [x0, x1): dst[x*ch+c] = sum_t w[x*padded+t] *
 * src[(start[x]+t)*ch+c]. Weight rows are zero-padded; kernels may read
 * (and multiply by zero) up to `padded` taps, so the source row carries
 * `padded` zeroed slack pixels at its end. */
typedef void (*tir__h_dot_fn)(float *dst, const float *src,
                              const int32_t *start, const int32_t *count,
                              const float *w, int padded, int x0, int x1);

typedef struct tir_kernels {
    /* vertical pass over n contiguous floats */
    void (*v_mul)(float *dst, const float *src, float w, size_t n);
    void (*v_fma)(float *dst, const float *src, float w, size_t n);
    void (*v_fma4)(float *dst, const float *const src[4], const float *w,
                   size_t n);
    /* horizontal pass, index = channels-1 */
    tir__h_dot_fn h_dot[4];
    /* anti-ringing */
    void (*h_minmax)(float *mn, float *mx, const float *src,
                     const int32_t *start, const int32_t *count, int x0,
                     int x1, int ch);
    void (*minmax_combine)(float *mn, float *mx, const float *rmn,
                           const float *rmx, size_t n);
    void (*antiring_apply)(float *dst, const float *mn, const float *mx,
                           float s, size_t n);
    void (*clamp_range)(float *dst, float lo, float hi, size_t n);
    /* scanline type converters (n = samples) */
    void (*u8_to_f32)(float *dst, const uint8_t *src, size_t n);
    void (*u16_to_f32)(float *dst, const uint16_t *src, size_t n);
    void (*f16_to_f32)(float *dst, const uint16_t *src, size_t n);
    void (*f32_to_u8)(uint8_t *dst, const float *src, size_t n);
    void (*f32_to_u16)(uint16_t *dst, const float *src, size_t n);
    void (*f32_to_f16)(uint16_t *dst, const float *src, size_t n);
    /* domain kernels */
    void (*normalize3)(float *xyz, float *len_out_or_null, size_t npix);
    void (*premult4)(float *rgba, size_t npix);
    void (*unpremult4)(float *rgba, size_t npix);
} tir_kernels;

/* Active global table (installed by tir__kernels_init). */
extern tir_kernels tir__k;

/* Lazily detect CPU caps and install the best kernel set (idempotent). */
void tir__kernels_init(void);
/* Fill `k` with the scalar reference kernels (bit-stable source of truth). */
void tir__kernels_set_scalar(tir_kernels *k);

/* Per-ISA overrides; each only touches what it implements. Definitions live
 * in the ISA translation units and are safe to call only after the matching
 * capability check. */
#if defined(TIR_X86)
void tir__kernels_set_sse2(tir_kernels *k);
void tir__kernels_set_sse41(tir_kernels *k);
void tir__kernels_set_avx2(tir_kernels *k);
#endif
#if defined(TIR_NEON)
void tir__kernels_set_neon(tir_kernels *k);
/* 1 when tir_kernels_sve.c was compiled with SVE support (data flag so the
 * baseline TU can test it without executing any SVE code); set_sve is a
 * no-op stub in the non-SVE build. Call set_sve only after the HWCAP gate. */
extern const int tir__have_sve_kernels;
void tir__kernels_set_sve(tir_kernels *k);
#endif

/* ===========================================================================
 * Per-axis coefficient tables
 * ========================================================================= */

/* Fast outputs [fast_lo, fast_hi) read a contiguous in-bounds source run
 * [start, start+count) with a zero-padded weight row. Outputs outside that
 * range (only possible with EDGE_WRAP borders) are "exceptions" resolved by
 * a scalar gather over explicit (index, weight) pairs. */
typedef struct tir__axis {
    int32_t *start;   /* [d] run start (valid for fast outputs) */
    int32_t *count;   /* [d] real tap count (<= taps) */
    float *w;         /* [d * padded] zero-padded weight rows */
    int taps;         /* max real count over all outputs */
    int padded;       /* taps rounded up to TIR_PAD */
    int fast_lo, fast_hi;
    int32_t *exc_idx; /* [n_exc * taps] mapped source indices */
    float *exc_w;     /* [n_exc * taps] */
    int n_exc;        /* fast_lo + (d - fast_hi) */
    /* anti-ringing reduced footprint: contiguous run per output, always a
     * subset of the filter run for fast outputs (ring residency). */
    int32_t *ar_start; /* [d] */
    int32_t *ar_count; /* [d] */
    int ar_taps;
    int is_down;
} tir__axis;

/* Sizes needed to carve a tir__axis (+ build scratch) out of the arena. */
typedef struct tir__axis_dims {
    int taps;
    int padded;
    int n_exc;
    int fast_lo, fast_hi;
    int ar_taps;
    int win_max; /* raw window width bound (build scratch elements) */
} tir__axis_dims;

typedef struct tir__axis_params {
    int s, d;
    tir_filter filter;
    tir_edge_mode edge;
    tir_registration reg;
    float filter_scale;
    float gaussian_sigma;
} tir__axis_params;

/* Measure pass: no allocation, computes the dims above. */
tir_result tir__axis_measure(const tir__axis_params *p, tir__axis_dims *out);
/* Fill pass: arrays already carved into `ax`; scratch holds 2 * win_max
 * doubles. Must be called with the same params as the measure pass. */
void tir__axis_fill(const tir__axis_params *p, const tir__axis_dims *dims,
                    tir__axis *ax, double *scratch);

/* ===========================================================================
 * Sampler
 * ========================================================================= */

struct tir_sampler {
    tir_allocator alloc;
    size_t arena_size;
    tir_options opt; /* resolved: no AUTO/negative-auto values remain */
    int sw, sh, dw, dh;
    int ch; /* io channels (1..4) */
    int wc; /* work channels (== ch, except NORMAL_RG: 2 -> 3) */
    tir_pixel_type st, dt;
    tir__axis fx, fy;
    tir_kernels k; /* kernel set bound at create (scalar if deterministic) */

    /* Pass order, chosen by the multiply-count cost model at create time.
     * h-first: the ring holds horizontally-resampled rows (dst width).
     * v-first: the ring holds raw decoded rows (src width); the vertical
     * blend lands in vstaging and the horizontal pass runs per output row. */
    int v_first;
    int ring_cap;
    size_t ring_stride;    /* floats per ring row */
    float *ring;           /* [ring_cap * ring_stride] */
    float *ring_mn;        /* h-first antiring only, else NULL */
    float *ring_mx;
    int32_t *ring_srcy;    /* source row resident per slot, -1 empty */

    float *decode_row;     /* [(sw + fx.padded) * wc], slack tail zeroed */
    float *staging;        /* [dw * wc + 4] */
    float *stage_mn;       /* antiring only, dst width */
    float *stage_mx;
    float *vstaging;       /* v-first only: [(sw + fx.padded) * wc], slack 0 */
    float *vs_mn;          /* v-first antiring only, src width */
    float *vs_mx;
    float *stage_tmp;      /* v-first antiring only, dst width scratch */
    float *repwin;         /* [3 * sw * ch] raw rows, REPAIR only */

    /* streaming state */
    int rows_pushed;
    int push_base;    /* first source row of this run (band runs start late) */
    int cur_dst;
    int last_pending; /* REPAIR: last source row decoded but not finalized */
    tir_result err_sticky;

    /* resolved domain params */
    float n_dec_a, n_dec_b; /* normal decode n = a*v + b (source type) */
    float n_enc_a, n_enc_b; /* normal encode v = (n - b) / a (dest type) */
    int premult;            /* premult -> filter -> unpremult */
    int already_premult;    /* filter as-is, no unpremult */
    int hicomp_ch;          /* leading channels to range-compress (0 = off) */
    float ar_strength;      /* 0 = off */
    float clamp_lo, clamp_hi;
    int do_clamp;
    float *nlen_out;
};

/* pipeline.c */
tir_result tir__push_row(tir_sampler *s, int src_y, const void *src_row);
tir_result tir__pull_row(tir_sampler *s, int *out_dst_y, void *dst_row);
void tir__reset(tir_sampler *s);

/* core.c: run destination rows [y0, y1) through a private sampler (used by
 * the whole-image path and by each worker band). */
tir_result tir__run_range(tir_sampler *s, const void *src,
                          size_t src_row_stride_bytes, void *dst,
                          size_t dst_row_stride_bytes, int y0, int y1);

#if defined(TIR_ENABLE_THREADS)
/* thread.c: band-parallel whole-image run (one clone sampler per worker). */
tir_result tir__run_threads(tir_sampler *s, const void *src,
                            size_t src_row_stride_bytes, void *dst,
                            size_t dst_row_stride_bytes, int nthreads);
#endif

/* domain.c */
float tir__rangecompress(float x);
float tir__rangeexpand(float y);
void tir__rangecompress_row(float *px, size_t npix, int wc, int nch);
void tir__rangeexpand_row(float *px, size_t npix, int wc, int nch);
/* Scrub NaN/Inf: ZERO in place; REPAIR averages finite 3x3 neighbors of the
 * raw window rows (prev/cur/next may alias cur at the borders). */
void tir__nonfinite_zero(float *row, size_t n);
int tir__row_has_nonfinite(const float *row, size_t n);
void tir__nonfinite_repair(float *dst, const float *prev, const float *cur,
                           const float *next, int w, int ch);
void tir__normal_decode_row(tir_sampler *s, float *row);
void tir__normal_encode_row(tir_sampler *s, float *row);

/* convert.c scalar reference converters */
void tir__u8_to_f32_sc(float *dst, const uint8_t *src, size_t n);
void tir__u16_to_f32_sc(float *dst, const uint16_t *src, size_t n);
void tir__f16_to_f32_sc(float *dst, const uint16_t *src, size_t n);
void tir__f32_to_u8_sc(uint8_t *dst, const float *src, size_t n);
void tir__f32_to_u16_sc(uint16_t *dst, const float *src, size_t n);
void tir__f32_to_f16_sc(uint16_t *dst, const float *src, size_t n);
size_t tir__pixel_size(tir_pixel_type t);

/* kernels_scalar.c */
void tir__v_mul_sc(float *dst, const float *src, float w, size_t n);
void tir__v_fma_sc(float *dst, const float *src, float w, size_t n);
void tir__v_fma4_sc(float *dst, const float *const src[4], const float *w,
                    size_t n);
void tir__h_dot1_sc(float *dst, const float *src, const int32_t *start,
                    const int32_t *count, const float *w, int padded, int x0,
                    int x1);
void tir__h_dot2_sc(float *dst, const float *src, const int32_t *start,
                    const int32_t *count, const float *w, int padded, int x0,
                    int x1);
void tir__h_dot3_sc(float *dst, const float *src, const int32_t *start,
                    const int32_t *count, const float *w, int padded, int x0,
                    int x1);
void tir__h_dot4_sc(float *dst, const float *src, const int32_t *start,
                    const int32_t *count, const float *w, int padded, int x0,
                    int x1);
void tir__h_minmax_sc(float *mn, float *mx, const float *src,
                      const int32_t *start, const int32_t *count, int x0,
                      int x1, int ch);
void tir__minmax_combine_sc(float *mn, float *mx, const float *rmn,
                            const float *rmx, size_t n);
void tir__antiring_apply_sc(float *dst, const float *mn, const float *mx,
                            float s, size_t n);
void tir__clamp_range_sc(float *dst, float lo, float hi, size_t n);
void tir__normalize3_sc(float *xyz, float *len_out_or_null, size_t npix);
void tir__premult4_sc(float *rgba, size_t npix);
void tir__unpremult4_sc(float *rgba, size_t npix);

#endif /* TIR_INTERNAL_H_ */
