/*
 * tocio - internal shared declarations (not installed).
 *
 * Portions derived from OpenColorIO - Copyright Contributors to the OpenColorIO
 * Project (BSD-3-Clause).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TOCIO_INTERNAL_H_
#define TOCIO_INTERNAL_H_

#include "tocio.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Freestanding builds (TOC_FREESTANDING) must not pull hosted <string.h>. We
 * declare the handful of mem/str symbols we use (and that the compiler may
 * emit) and define them in toc_freestanding.c. Hosted builds use libc.
 */
#ifdef TOC_FREESTANDING
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
#else
#include <string.h>
#endif

/* ============================================================================
 * Allocator helpers
 * ========================================================================== */
void *toc_malloc(const toc_allocator *a, size_t size);
void *toc_calloc(const toc_allocator *a, size_t count, size_t size);
void toc_free(const toc_allocator *a, void *ptr);
char *toc_strndup(const toc_allocator *a, const char *s, size_t n);

static inline int toc_mul_ovf(size_t a, size_t b, size_t *out) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_mul_overflow(a, b, out);
#else
    if (a != 0 && b > SIZE_MAX / a) return 1;
    *out = a * b;
    return 0;
#endif
}

/* ============================================================================
 * Bump arena (freed in one pass; used for the YAML node tree + config strings)
 * ========================================================================== */
typedef struct toc_arena_block {
    struct toc_arena_block *next;
    size_t used, cap;
    /* data follows */
} toc_arena_block;

typedef struct toc_arena {
    toc_allocator alloc;
    toc_arena_block *head;
} toc_arena;

void toc_arena_init(toc_arena *ar, const toc_allocator *a);
void *toc_arena_alloc(toc_arena *ar, size_t size);
char *toc_arena_strndup(toc_arena *ar, const char *s, size_t n);
void toc_arena_free(toc_arena *ar);

/* ============================================================================
 * Growable string builder (allocator-backed; no sprintf)
 * ========================================================================== */
typedef struct toc_sb {
    toc_allocator alloc;
    char *buf;
    size_t len, cap;
    int oom; /* sticky out-of-memory flag */
} toc_sb;

void toc_sb_init(toc_sb *sb, const toc_allocator *a);
int toc_sb_putc(toc_sb *sb, char c);
int toc_sb_puts(toc_sb *sb, const char *s);
int toc_sb_putn(toc_sb *sb, const char *s, size_t n);
int toc_sb_int(toc_sb *sb, long v);
/* Round-trip-exact float as a C hex-float literal with trailing 'f'. */
int toc_sb_hexfloat(toc_sb *sb, float v);
/* Human-readable decimal float (for GLSL; always contains a '.'). */
int toc_sb_decfloat(toc_sb *sb, float v);
char *toc_sb_take(toc_sb *sb, size_t *out_len); /* hand off buffer ownership */
void toc_sb_free(toc_sb *sb);

/* ============================================================================
 * Vendored libm-free math (names dodge the freestanding exp|log|pow scan).
 * Accurate to ~1e-6 relative; verified vs libm in the host test.
 * ========================================================================== */
float toc_log2f(float x);
float toc_exp2f(float x);
float toc_powf(float x, float y);
float toc_sqrtf(float x);
/* base^x without naming pow/exp: = exp2(x*log2(base)). */
float toc_raisef(float base, float x);
/* Parse a float (libm-free, no strtod). Advances *pp; returns 1 on success. */
int toc_parse_float(const char **pp, const char *end, float *out);
int toc_parse_int(const char **pp, const char *end, long *out);
/* Invert a row-major 4x4 (returns 0 on singular). */
int toc_inv4x4(const float *m, float *out);
/* Apply a forward LUT3D op to one pixel in place (used by LUT3D inversion). */
void toc_lut3d_apply_pixel(const toc_op *op, float *px, int ch);

/* ============================================================================
 * Op-list builder helpers (used by the processor + builtins)
 * ========================================================================== */
toc_op *toc_op_list_push(toc_op_list *list, toc_op_kind kind);
/* Take ownership of a malloc'd LUT array so ops can borrow it safely. */
int toc_op_list_own(toc_op_list *list, float *data);

/* ============================================================================
 * YAML node tree (produced by toc_yaml.c, consumed by toc_config.c)
 * ========================================================================== */
typedef enum toc_node_kind {
    TOC_NODE_SCALAR = 0,
    TOC_NODE_SEQ,
    TOC_NODE_MAP
} toc_node_kind;

typedef struct toc_node {
    toc_node_kind kind;
    const char *tag;    /* e.g. "MatrixTransform" (without !<>), or NULL */
    const char *scalar; /* SCALAR: NUL-terminated value */
    struct toc_node **items;
    size_t n_items; /* SEQ */
    const char **keys;
    struct toc_node **vals;
    size_t n_pairs; /* MAP */
} toc_node;

/* Parse YAML text into a node tree allocated from `ar`. On error returns
 * TOC_ERROR_PARSE and (optionally) sets *err_line. */
toc_result toc_yaml_parse(const char *text, size_t len, toc_arena *ar,
                          toc_node **out_root, int *err_line);

/* Node-tree lookup helpers. */
const toc_node *toc_node_map_get(const toc_node *n, const char *key);
const char *toc_node_scalar(const toc_node *n);

/* ============================================================================
 * Interpreter SIMD dispatch (toc_interp.c + toc_interp_simd_x86.c)
 *
 * The interpreter runs op-major: each op is applied to the whole pixel buffer
 * via a batch kernel. matrix/range have SSE2/AVX2 variants; the scalar batch is
 * the bit-exact reference. Dispatched once via CPUID.
 * ========================================================================== */
/* Transcendental ops (exponent/log/log_camera/exp_linear/cdl) batch over the
 * whole buffer; the NEON tier vectorizes their pow/log/exp math 4 lanes per
 * pixel (one RGBA pixel per iteration) and is bit-exact with the scalar batch. */
typedef void (*toc_op_batch_fn)(const toc_op *op, float *rgba, size_t npix,
                                int ch);
typedef struct {
    void (*matrix)(const float *m, const float *off, float *rgba, size_t npix,
                   int ch);
    void (*range)(const float *scale, const float *offset, const float *vmin,
                  const float *vmax, int clamp_lo, int clamp_hi, float *rgba,
                  size_t npix, int ch);
    toc_op_batch_fn exponent;
    toc_op_batch_fn log_;
    toc_op_batch_fn log_camera;
    toc_op_batch_fn exp_linear;
    toc_op_batch_fn cdl;
} toc_simd_vtbl;
extern toc_simd_vtbl toc_simd;
void toc_simd_init(void);
/* Force a tier for parity tests: 0 scalar, 1 SSE2, 2 AVX2. */
void toc_simd_force(int level);

/* Apply a single op to one pixel (used by the batch fallback + the JIT). */
void toc_apply_op_pixel(const toc_op *op, float *px, int ch);

void toc_matrix_batch_scalar(const float *m, const float *off, float *rgba,
                             size_t npix, int ch);
void toc_range_batch_scalar(const float *scale, const float *offset,
                            const float *vmin, const float *vmax, int clamp_lo,
                            int clamp_hi, float *rgba, size_t npix, int ch);
/* Scalar batch references for the transcendental ops (vtbl defaults). */
void toc_exponent_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch);
void toc_log_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch);
void toc_logcam_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch);
void toc_explin_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch);
void toc_cdl_batch_scalar(const toc_op *op, float *rgba, size_t npix, int ch);

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) ||             \
    defined(_M_IX86)
#define TOC_X86 1
void toc_matrix_batch_sse2(const float *m, const float *off, float *rgba,
                           size_t npix, int ch);
void toc_range_batch_sse2(const float *scale, const float *offset,
                          const float *vmin, const float *vmax, int clamp_lo,
                          int clamp_hi, float *rgba, size_t npix, int ch);
void toc_range_batch_avx2(const float *scale, const float *offset,
                          const float *vmin, const float *vmax, int clamp_lo,
                          int clamp_hi, float *rgba, size_t npix, int ch);
unsigned toc_cpu_has_avx2(void);
#endif

#if defined(__aarch64__)
#define TOC_NEON 1
void toc_matrix_batch_neon(const float *m, const float *off, float *rgba,
                           size_t npix, int ch);
void toc_range_batch_neon(const float *scale, const float *offset,
                          const float *vmin, const float *vmax, int clamp_lo,
                          int clamp_hi, float *rgba, size_t npix, int ch);
/* NEON transcendental batch kernels (bit-exact with the *_scalar references). */
void toc_exponent_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch);
void toc_log_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch);
void toc_logcam_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch);
void toc_explin_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch);
void toc_cdl_batch_neon(const toc_op *op, float *rgba, size_t npix, int ch);
#endif

/* ============================================================================
 * Config model (opaque to the public API; defined here for config + processor)
 * ========================================================================== */
struct toc_config {
    toc_arena ar;        /* owns the node tree + all strings */
    toc_node *root;      /* top-level mapping */
    toc_file_reader reader;
    void *reader_user;
    toc_allocator alloc;
};

/* Resolve a role name to its colorspace name (or return `name` unchanged). */
const char *toc_cfg_resolve_role(const toc_config *cfg, const char *name);
/* Find a colorspace (or display_colorspace) node by name/alias. */
const toc_node *toc_cfg_find_colorspace(const toc_config *cfg, const char *name);
int toc_cfg_is_data(const toc_node *cs);
/* 1 if `name` is defined in the `display_colorspaces` section (display-referred),
 * 0 otherwise (scene-referred / not found). Role names are resolved. */
int toc_cfg_cs_in_display_section(const toc_config *cfg, const char *name);
/* Pick the transform for converting to/from the reference. `want_to_ref`
 * selects to_reference (1) vs from_reference (0). *out_invert is set when the
 * chosen node must be applied inverted (only the opposite direction existed).
 * Returns NULL if neither direction is defined (identity). */
const toc_node *toc_cfg_cs_transform(const toc_node *cs, int want_to_ref,
                                     int *out_invert);
/* Find a ViewTransform node by name. */
const toc_node *toc_cfg_find_view_transform(const toc_config *cfg, const char *name);
/* The default view transform (config `default_view_transform`, else the first),
 * used to bridge scene<->display references in colorspace conversions. */
const toc_node *toc_cfg_default_view_transform(const toc_config *cfg);
/* Find a Look node by name (from the `looks` top-level seq). */
const toc_node *toc_cfg_find_look(const toc_config *cfg, const char *name);
/* Parse a comma-separated `looks` list from a view node. Returns count written
 * to `names` (up to max), or 0 if absent/empty. Mutates the arena-owned string
 * in place (NUL-separating the entries). */
int toc_cfg_view_looks(const toc_node *vnode, const char **names, int max);
/* Public active-displays/views API is in tocio.h (toc_config_* names). */

/* ============================================================================
 * Processor internals (toc_processor.c) + LUT-file loaders (toc_lutfile.c)
 * ========================================================================== */
/* Lower a primitive transform node (MatrixTransform/RangeTransform/...) into an
 * op appended to `list`. `invert` requests the inverse. Group/ColorSpace/File/
 * Builtin are handled by the processor's recursive walk, not here. */
toc_result toc_lower_transform(const toc_config *cfg, toc_op_list *list,
                               const toc_node *node, int invert);
/* Invert an already-lowered op in place. `list` owns any new storage the inverse
 * needs (a rebuilt LUT1D); it may be NULL only if no op in play is a LUT. */
toc_result toc_invert_op(toc_op_list *list, toc_op *op);

/* Parse a LUT file blob by extension hint into a freshly-allocated op (LUT1D or
 * LUT3D). The sample array is added to `list->owned`. */
toc_result toc_load_lutfile(toc_op_list *list, const char *name,
                            const char *data, size_t len, int invert);
/* Parse a CLF (Common LUT Format) XML blob into a sequence of ops. */
toc_result toc_load_clf(toc_op_list *list, const char *data, size_t len);
/* Parse a CSP (ColorSpace Process) LUT blob into PRE_1D -> 3D -> POST_1D ops. */
toc_result toc_load_csp(toc_op_list *list, const char *data, size_t len);

/* Expand a BuiltinTransform style into primitive ops (toc_builtins.c). */
toc_result toc_builtin_expand(toc_op_list *list, const char *style, int invert);
/* Lower a FixedFunctionTransform node into a TOC_OP_FIXEDFUNC (toc_builtins.c). */
toc_result toc_lower_fixedfunc(toc_op_list *list, const toc_node *node,
                               int invert);

/* ============================================================================
 * ACES 2.0 output transform (toc_aces2.c)
 *
 * Reimplements the OCIO ACES2 fixed function: RGB(AP0) -> CAM16 JMh -> tonescale
 * on J -> chroma compression -> gamut compression (hue-indexed cusp/reach tables)
 * -> JMh -> RGB(limiting primaries). Constants/math ported verbatim from
 * OpenColorIO src/OpenColorIO/ops/fixedfunction/ACES2/{Common.h,Transform.cpp}.
 * ========================================================================== */
#define TOC_ACES2_NOMINAL 360
#define TOC_ACES2_TSIZE (TOC_ACES2_NOMINAL + 3) /* +1 lower wrap, +2 upper wrap */

/* JMh (CAM16) parameters for one RGB primaries set. Matrices are row-major 3x3
 * applied as M*v (matches OCIO mult_f3_f33). */
typedef struct toc_jmh {
    float rgb_to_cam[9], cam_to_rgb[9];
    float cone_to_aab[9], aab_to_cone[9];
    float F_L_n, cz, inv_cz, A_w_J, inv_A_w_J;
} toc_jmh;

typedef struct toc_aces2 {
    toc_jmh in;  /* AP0 (input) */
    toc_jmh out; /* limiting primaries (output) */
    /* tonescale */
    float ts_n, ts_n_r, ts_g, ts_t_1, ts_c_t, ts_s_2, ts_u_2, ts_m_2;
    float ts_fwd_limit, ts_inv_limit, ts_log_peak;
    /* chroma compress */
    float cc_sat, cc_sat_thr, cc_compr, cc_scale;
    /* shared compression */
    float limit_J_max, model_gamma_inv;
    /* gamut compress scalars */
    float mid_J, focus_dist, lower_hull_gamma_inv;
    int hue_search_lo, hue_search_hi;
    /* hue-indexed tables (total size includes wrap entries) */
    float reach_m[TOC_ACES2_TSIZE];
    float hue_table[TOC_ACES2_TSIZE];
    float cusp_J[TOC_ACES2_TSIZE], cusp_M[TOC_ACES2_TSIZE], cusp_g[TOC_ACES2_TSIZE];
    int inverse; /* 0 forward only (inverse deferred) */
} toc_aces2;

/* Build the parameter+table blob for `peak_luminance` (nits) and limiting
 * primaries (8 chromaticities: rx,ry,gx,gy,bx,by,wx,wy). Allocated with `a`;
 * register it in the op list's owned[] so it is freed with the list. */
toc_aces2 *toc_aces2_init(const toc_allocator *a, float peak_luminance,
                          const float lim_prims[8]);
/* Apply the forward output transform to one pixel (RGB in px[0..2]). */
void toc_aces2_apply_pixel(const toc_op *op, float *px, int ch);

#endif /* TOCIO_INTERNAL_H_ */
