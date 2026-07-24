/*
 * tir - tiny image resize: a small, fast, content-aware image resizer.
 *
 * Standalone pure-C11 library (no dependency on the TinyEXR core; the header
 * is safe to include from C++). Separable two-pass resampling over float32
 * with precomputed coefficient tables and SIMD kernels (SSE2/SSE4.1/AVX2 via
 * runtime dispatch, NEON at compile time, SVE behind a HWCAP runtime gate).
 *
 * Content-aware modes:
 *   - HDR: values are never clamped by default; optional anti-ringing clamp
 *     (applied once, after both passes) and output clamp give a hard
 *     "no negative pixels" guarantee for negative-lobe filters.
 *   - Normal maps: decode -> filter 3 components -> renormalize once.
 *   - Height/displacement maps: mean-preserving defaults, grid-vertex
 *     (endpoint-exact) registration for 2^n+1 terrains.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TIR_H_
#define TIR_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIR_VERSION_MAJOR 0
#define TIR_VERSION_MINOR 1
#define TIR_VERSION_PATCH 0

/* ===========================================================================
 * Results / allocator
 * ========================================================================= */

typedef enum tir_result {
    TIR_SUCCESS = 0,
    TIR_WOULD_BLOCK = 1,            /* streaming: push more source rows first */
    TIR_ERROR_INVALID_ARGUMENT = -1,
    TIR_ERROR_OUT_OF_MEMORY = -2,
    TIR_ERROR_UNSUPPORTED = -3,
    TIR_ERROR_TOO_LARGE = -4,       /* size arithmetic would overflow */
    TIR_ERROR_NONFINITE = -5,       /* TIR_NONFINITE_ERROR policy hit NaN/Inf */
    TIR_ERROR_ORDER = -6            /* source rows pushed out of order */
} tir_result;

#define TIR_OK(r) ((int)(r) >= 0)

const char *tir_result_string(tir_result r);

/* Custom allocator; pass NULL anywhere a tir_allocator* is taken to use
 * malloc/free. A sampler performs exactly one allocation. */
typedef struct tir_allocator {
    void *user;
    void *(*alloc)(void *user, size_t size);
    void (*free)(void *user, void *ptr);
} tir_allocator;

/* ===========================================================================
 * SIMD control
 * ========================================================================= */

typedef enum tir_simd_level {
    TIR_SIMD_SCALAR = 0,
    TIR_SIMD_SSE2 = 1,
    TIR_SIMD_SSE41 = 2,
    TIR_SIMD_AVX2 = 3,   /* implies FMA + F16C */
    TIR_SIMD_NEON = 4,
    TIR_SIMD_SVE = 5
} tir_simd_level;

/* Bitmask of (1u << tir_simd_level) usable on this machine (scalar always). */
uint32_t tir_simd_available(void);

/* Cap the active kernel set at `level` (bench/parity hook). Rejects levels
 * not present in tir_simd_available(). Not thread-safe against running
 * samplers; call before creating them. */
tir_result tir_simd_force(tir_simd_level level);

/* Human-readable active kernel set, e.g. "avx2". */
const char *tir_simd_info(void);

/* ===========================================================================
 * Images
 * ========================================================================= */

typedef enum tir_pixel_type {
    TIR_F32 = 0,  /* 32-bit float */
    TIR_F16 = 1,  /* IEEE 754 binary16 in a uint16_t */
    TIR_U8 = 2,   /* unorm: 0..255 maps to 0..1 */
    TIR_U16 = 3   /* unorm: 0..65535 maps to 0..1 */
} tir_pixel_type;

/* Interleaved pixels, rows top to bottom. row_stride_bytes == 0 means tight
 * (width * channels * bytes-per-sample). `data` is not written through when
 * the view describes a source image. */
typedef struct tir_image_view {
    void *data;
    int width;
    int height;
    int channels;               /* 1..4, interleaved */
    tir_pixel_type type;
    size_t row_stride_bytes;    /* 0 = tight */
} tir_image_view;

/* ===========================================================================
 * Options
 * ========================================================================= */

typedef enum tir_filter {
    TIR_FILTER_AUTO = 0,        /* mode/direction dependent, see tir_options */
    /* non-negative kernels: convex weights, cannot ring or overshoot */
    TIR_FILTER_BOX = 1,         /* area average; mean-preserving downscale */
    TIR_FILTER_TRIANGLE = 2,    /* linear / bilinear */
    TIR_FILTER_BSPLINE = 3,     /* cubic B-spline (B=1,C=0): smooth, soft */
    TIR_FILTER_GAUSSIAN = 4,    /* sigma configurable, radius 2*sigma*fscale */
    /* negative-lobe kernels: sharper, ring on hard edges (step over/undershoot
     * Mitchell +-3.6%, Catmull-Rom +-7.4%, Lanczos2 +-8.5%, Lanczos3 +-11.8%) */
    TIR_FILTER_MITCHELL = 5,    /* cubic B=C=1/3; good general default */
    TIR_FILTER_CATMULL_ROM = 6, /* interpolating cubic (B=0,C=0.5) */
    TIR_FILTER_LANCZOS2 = 7,
    TIR_FILTER_LANCZOS3 = 8,
    TIR_FILTER_KAISER = 9       /* Kaiser-windowed sinc (radius 3, beta 8): sharp
                                 * with lower ringing than Lanczos3 */
} tir_filter;

typedef enum tir_edge_mode {
    TIR_EDGE_CLAMP = 0,
    TIR_EDGE_REFLECT = 1,       /* mirror without repeating the edge sample */
    TIR_EDGE_WRAP = 2           /* tile; y-wrap buffers the whole image */
} tir_edge_mode;

typedef enum tir_mode {
    TIR_MODE_GENERAL = 0,
    TIR_MODE_NORMAL_MAP = 1,    /* decode -> filter vectors -> renormalize */
    TIR_MODE_HEIGHTMAP = 2      /* scalar data: mean-preserving, registration */
} tir_mode;

/* What to do about NaN/Inf in the source. One nonfinite sample poisons every
 * output pixel whose filter footprint touches it, so scrubbing happens before
 * filtering. */
typedef enum tir_nonfinite {
    TIR_NONFINITE_KEEP = 0,     /* pass through (default; fastest) */
    TIR_NONFINITE_ZERO = 1,     /* replace with 0 */
    TIR_NONFINITE_REPAIR = 2,   /* replace with the average of finite 3x3
                                 * neighbors (0 if none); one row of latency */
    TIR_NONFINITE_ERROR = 3     /* fail with TIR_ERROR_NONFINITE */
} tir_nonfinite;

/* RGBA handling (channels == 4 only; ignored otherwise). Filtering straight
 * (unassociated) alpha bleeds the RGB of transparent texels into neighbors. */
typedef enum tir_alpha_mode {
    TIR_ALPHA_PREMULTIPLY = 0,  /* premult -> filter -> unpremult (RGB kept
                                 * where the filtered alpha is exactly 0) */
    TIR_ALPHA_PREMULTIPLIED = 1,/* input is already associated; filter as-is */
    TIR_ALPHA_STRAIGHT = 2      /* filter all channels independently */
} tir_alpha_mode;

/* Normal-map decode. Filtering happens on the decoded [-1,1] vectors; the
 * result is re-encoded to the destination type. */
typedef enum tir_normal_enc {
    TIR_NORMAL_SNORM = 0,       /* components already signed in [-1,1] */
    TIR_NORMAL_UNORM = 1,       /* n = 2x - 1 (u8/u16/float in [0,1]) */
    TIR_NORMAL_UNORM_C127 = 2,  /* NVTT center convention: zero at code 127
                                 * (u8: n = 2x - 254/255; u16: code 32767) */
    TIR_NORMAL_RG = 3           /* 2-channel X,Y; z = sqrt(max(0,1-x^2-y^2))
                                 * reconstructed BEFORE filtering */
} tir_normal_enc;

/* Sample-position convention. Cell-centered is the texture standard.
 * Grid-vertex treats texels as grid points (GIS "point" / terrain 2^n+1
 * convention): first and last samples map exactly, no half-texel shift. */
typedef enum tir_registration {
    TIR_REG_CELL_CENTERED = 0,  /* src = (dst + 0.5) * ratio - 0.5 */
    TIR_REG_GRID_VERTEX = 1     /* src = dst * (src_n-1) / (dst_n-1) */
} tir_registration;

typedef struct tir_options {
    /* -- filtering ------------------------------------------------------- */
    tir_filter filter_x;        /* AUTO: GENERAL->Mitchell; NORMAL->triangle;
                                 * HEIGHTMAP->box (downscale) / triangle (up) */
    tir_filter filter_y;
    tir_edge_mode edge_x;       /* CLAMP */
    tir_edge_mode edge_y;
    float filter_scale;         /* 1.0; >1 widens the kernel (blurs). Must be
                                 * in (0, 64]; outside that resize() returns
                                 * TIR_ERROR_INVALID_ARGUMENT. */
    float gaussian_sigma;       /* 0.5; must be in (0, 32] (else
                                 * TIR_ERROR_INVALID_ARGUMENT). */

    /* -- content mode ---------------------------------------------------- */
    tir_mode mode;              /* GENERAL */

    /* -- HDR safety ------------------------------------------------------ */
    /* Anti-ringing strength in [0,1]: blends the result toward its clamp
     * against the min/max of the reduced source footprint (nearest 2 samples
     * per axis upscaling, trunc(ratio)+1 downscaling). Applied once after
     * both passes. < 0 = auto (0.0; 1.0 in heightmap mode with a
     * negative-lobe filter). 1.0 guarantees output within local source range. */
    float antiring;
    float clamp_min;            /* final output clamp; -INFINITY = off */
    float clamp_max;            /* +INFINITY = off; (0, +INF) = no negatives */
    int hicomp;                 /* 0; 1 = filter under highlight compression
                                 * (log-domain, OIIO/SPI curve) and clamp the
                                 * expanded result non-negative */
    tir_nonfinite nonfinite;    /* KEEP */

    /* -- alpha ------------------------------------------------------------ */
    tir_alpha_mode alpha;       /* PREMULTIPLY */

    /* -- normal-map mode --------------------------------------------------- */
    tir_normal_enc normal_encoding; /* SNORM */
    int normal_renormalize;     /* 1; 0 keeps the filtered (shortened) vector */
    float *normal_length_out;   /* optional dst_w*dst_h floats receiving the
                                 * pre-renormalize |N| (Toksvig/roughness
                                 * baking), or NULL */

    /* -- heightmap mode ---------------------------------------------------- */
    tir_registration registration; /* CELL_CENTERED */

    /* -- execution --------------------------------------------------------- */
    int deterministic;          /* 0; 1 = scalar kernels, fixed accumulation
                                 * order: bit-identical across platforms,
                                 * SIMD levels and thread counts */
    int num_threads;            /* 0 = serial. Effective only when built with
                                 * -DTIR_ENABLE_THREADS and for whole-image
                                 * entry points (not the streaming rows API). */
} tir_options;

/* Fill `opt` with the defaults documented above. */
void tir_options_init(tir_options *opt);

/* ===========================================================================
 * One-shot resize
 * ========================================================================= */

/* Resize src into dst (both fully described views; dst buffer caller-owned).
 * Source and destination types/channels may differ only in type; channel
 * counts must match. NULL `opt` means tir_options_init defaults. */
tir_result tir_resize(const tir_allocator *a, const tir_image_view *src,
                      const tir_image_view *dst, const tir_options *opt);

/* ===========================================================================
 * Reusable sampler + streaming rows
 * ========================================================================= */

/* A sampler pre-builds coefficient tables and scratch for one resize
 * geometry; reuse it across frames (video / mip chains). One allocation. */
typedef struct tir_sampler tir_sampler;

tir_result tir_sampler_create(const tir_allocator *a, int src_w, int src_h,
                              int dst_w, int dst_h, int channels,
                              tir_pixel_type src_type, tir_pixel_type dst_type,
                              const tir_options *opt, tir_sampler **out);

/* Whole images through a prebuilt sampler (strides in bytes, 0 = tight). */
tir_result tir_sampler_run(tir_sampler *s, const void *src,
                           size_t src_row_stride_bytes, void *dst,
                           size_t dst_row_stride_bytes);

/* Streaming: push source rows top to bottom, pull destination rows.
 * Memory is O(filter_support * dst_w) (except edge_y == WRAP, which must
 * buffer the whole image). Pull returns TIR_WOULD_BLOCK while more source
 * rows are needed, else TIR_SUCCESS; *out_dst_y == dst_h signals completion.
 * The streaming path is horizontal-first and serial by definition. */
tir_result tir_sampler_reset(tir_sampler *s);
tir_result tir_sampler_push_row(tir_sampler *s, int src_y, const void *src_row);
tir_result tir_sampler_pull_row(tir_sampler *s, int *out_dst_y, void *dst_row);

void tir_sampler_destroy(tir_sampler *s);

#ifdef __cplusplus
}
#endif

#endif /* TIR_H_ */
