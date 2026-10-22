/*
 * tocio - a tiny, self-contained, pure-C11 OpenColorIO config engine + codegen.
 *
 * Parses an OCIO config (a YAML subset), resolves a color transform into a flat
 * op list, and runs it three ways: a CPU interpreter (scalar + SSE2/AVX2), an
 * AOT C-source generator, and a GLSL/WebGL/Vulkan shader generator. Pure C11,
 * freestanding (no libc/libm beyond stdint/stddef; all heap via an allocator
 * hook), no external dependencies.
 *
 * Portions derived from OpenColorIO - Copyright Contributors to the OpenColorIO
 * Project (https://opencolorio.org, BSD-3-Clause). The OCIO config format, the
 * color-op math, and the GPU shader section model are reimplemented from it.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TOCIO_H_
#define TOCIO_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Result codes + allocator hook (mirror tinyexr conventions)
 * ========================================================================== */

typedef enum toc_result {
    TOC_SUCCESS = 0,
    TOC_ERROR_INVALID_ARGUMENT = -1,
    TOC_ERROR_PARSE = -2,         /* YAML / LUT-file syntax error */
    TOC_ERROR_UNSUPPORTED = -3,   /* feature/transform/builtin not implemented */
    TOC_ERROR_OUT_OF_MEMORY = -4,
    TOC_ERROR_IO = -5,
    TOC_ERROR_NOT_FOUND = -6,     /* role/colorspace/display/view missing */
    TOC_ERROR_NONINVERTIBLE = -7  /* op cannot be inverted (e.g. 3D LUT) */
} toc_result;

#define TOC_OK(r) ((int)(r) >= 0)

const char *toc_result_string(toc_result r);

typedef struct toc_allocator {
    void *user;
    void *(*alloc)(void *user, size_t size);
    void (*free)(void *user, void *ptr);
} toc_allocator;

/* NULL in freestanding builds; malloc/free wrapper otherwise. */
const toc_allocator *toc_default_allocator(void);

/* ============================================================================
 * Op IR - the single intermediate representation all backends consume.
 * Group/ColorSpace/File/Builtin transforms are flattened into these primitives
 * by the processor; backends never see them.
 * ========================================================================== */

typedef enum toc_op_kind {
    TOC_OP_MATRIX = 0, /* out = M(4x4 col-major) * rgba + offset[4] */
    TOC_OP_RANGE,      /* out = clamp(in*scale + offset, min, max) per channel */
    TOC_OP_EXPONENT,   /* out = pow(max(0,in), e) per channel */
    TOC_OP_EXP_LINEAR, /* ExponentWithLinear: piecewise linear/power */
    TOC_OP_LOG,        /* unified affine lin<->log (inverse flag picks dir) */
    TOC_OP_LOG_CAMERA, /* LogCamera: affine log + linear segment below break */
    TOC_OP_CDL,        /* (in*slope+offset)^power then saturation about luma */
    TOC_OP_LUT1D,      /* per-channel 1D interpolation over a domain */
    TOC_OP_LUT3D,      /* trilinear/tetrahedral over an NxNxN cube */
    TOC_OP_FIXEDFUNC,  /* ACES fixed function (style id + params) */
    TOC_OP_ACES_OUTPUT,/* ACES 2.0 output transform (CAM16 JMh tonescale+gamut) */
    TOC_OP_NOOP
} toc_op_kind;

typedef enum toc_interp {
    TOC_INTERP_NEAREST = 0,
    TOC_INTERP_LINEAR = 1,      /* 1D */
    TOC_INTERP_TRILINEAR = 1,   /* 3D (alias) */
    TOC_INTERP_TETRAHEDRAL = 2  /* 3D */
} toc_interp;

/* ACES FixedFunction style ids (subset). _INV variants are forward+1. */
typedef enum toc_ff_style {
    TOC_FF_ACES_GLOW10 = 0,
    TOC_FF_ACES_GLOW10_INV,
    TOC_FF_ACES_DARKTODIM10,
    TOC_FF_ACES_DARKTODIM10_INV,
    TOC_FF_ACES_GAMUTCOMP13,
    TOC_FF_ACES_GAMUTCOMP13_INV,
    TOC_FF_REC2100_SURROUND,
    TOC_FF_REC2100_SURROUND_INV,
    TOC_FF_ACES_GLOW03 = 8,
    TOC_FF_ACES_GLOW03_INV,
    TOC_FF_ACES_RED_MOD_03 = 10,
    TOC_FF_ACES_RED_MOD_03_INV,
    TOC_FF_ACES_RED_MOD_10 = 12,
    TOC_FF_ACES_RED_MOD_10_INV,
    TOC_FF_RGB_TO_HSV = 14,
    TOC_FF_HSV_TO_RGB = 15,
    TOC_FF_XYZ_TO_xyY = 16,
    TOC_FF_xyY_TO_XYZ = 17,
    TOC_FF_XYZ_TO_uvY = 18,
    TOC_FF_uvY_TO_XYZ = 19,
    TOC_FF_XYZ_TO_LUV = 20,
    TOC_FF_LUV_TO_XYZ = 21,
    /* HDR display transfer functions (per-channel). Forward = linear->encoded. */
    TOC_FF_LIN_TO_PQ = 22,  /* SMPTE ST 2084 (PQ) inverse-EOTF */
    TOC_FF_PQ_TO_LIN = 23,
    TOC_FF_LIN_TO_HLG = 24, /* Rec.2100 HLG OETF (per-channel; no OOTF) */
    TOC_FF_HLG_TO_LIN = 25
} toc_ff_style;

typedef struct toc_lut1d {
    int length;   /* samples per channel */
    int channels; /* 1 (shared R=G=B) or 3 */
    float domain_min, domain_max;
    const float *data; /* length*channels, interleaved when channels==3 */
    int interp;        /* toc_interp */
} toc_lut1d;

typedef struct toc_lut3d {
    int size; /* N; cube is N*N*N */
    float domain_min[3], domain_max[3];
    const float *data; /* N*N*N*3, R-fastest: ((b*N+g)*N+r)*3 */
    int interp;        /* toc_interp */
} toc_lut3d;

typedef struct toc_op {
    toc_op_kind kind;
    union {
        struct { float m[16]; float off[4]; } matrix; /* col-major */
        struct {
            float scale[4], offset[4], min[4], max[4];
            int clamp_lo, clamp_hi;
        } range;
        struct { float e[4]; int mirror; } exponent; /* mirror: sign(x)*pow(|x|,e) */
        /* MonCurve (ExponentWithLinear): forward is
         *   y = (x > breakpoint) ? pow(x*scale + offset, gamma) : x*slope
         * with all five params precomputed per channel from (gamma,offset).
         * Inverse is the analytic inverse of that (flag below). */
        struct {
            float scale[4], offset[4], gamma[4], breakpoint[4], slope[4];
            int inverse;
            int mirror; /* odd extension for x<0: sign(x)*curve(|x|) */
        } exp_linear;
        struct {
            /* out = (antilog/log)(in) using base, per-channel affine params */
            float log_slope[3], log_offset[3];
            float lin_slope[3], lin_offset[3];
            float base;    /* e.g. 2.0, 10.0 */
            int inverse;   /* 0: lin->log, 1: log->lin */
        } log;
        struct {
            float log_slope[3], log_offset[3];
            float lin_slope[3], lin_offset[3];
            float lin_break[3]; /* linear-side breakpoint */
            float linear_slope[3];
            float linear_offset[3];
            float base;
            int inverse;
        } logcam;
        struct {
            float slope[3], offset[3], power[3];
            float saturation;
            float luma[3];
            int clamp; /* ASC CDL clamps to [0,1] around power */
            int inverse;
        } cdl;
        toc_lut1d lut1d;
        toc_lut3d lut3d;
        struct { int style; float params[8]; int nparams; } fixedfunc;
        /* ACES 2.0 output transform. `t` points at a precomputed parameter+table
         * blob owned by the op list (toc_aces2, defined in toc_internal.h). */
        struct { const void *t; int inverse; } aces;
    } u;
} toc_op;

typedef struct toc_op_list {
    toc_op *ops;
    size_t count, cap;
    float **owned; /* LUT sample arrays the ops borrow; freed with the list */
    size_t owned_count, owned_cap;
    toc_allocator alloc;
} toc_op_list;

void toc_op_list_free(toc_op_list *list);

/* ============================================================================
 * Config (opaque) + file-reader hook for FileTransform LUTs
 * ========================================================================== */

typedef struct toc_config toc_config;

/* Reader the processor calls to load a FileTransform's LUT by (relative) name.
 * Returns the file bytes in *data (allocated with `a`, NUL-terminated not
 * required). Lets freestanding/WASM hosts inject I/O. */
typedef toc_result (*toc_file_reader)(void *user, const char *name,
                                      const toc_allocator *a, char **data,
                                      size_t *len);

toc_result toc_config_parse(const char *yaml, size_t len,
                            const toc_allocator *a, toc_config **out);
void toc_config_free(toc_config *cfg);
void toc_config_set_file_reader(toc_config *cfg, toc_file_reader fn, void *user);

/* Introspection (for viewer UIs). Pointers are owned by the config. */
int toc_config_num_colorspaces(const toc_config *cfg);
const char *toc_config_colorspace_name(const toc_config *cfg, int index);
const char *toc_config_role(const toc_config *cfg, const char *role);
int toc_config_num_displays(const toc_config *cfg);
const char *toc_config_display_name(const toc_config *cfg, int index);
int toc_config_num_views(const toc_config *cfg, const char *display);
const char *toc_config_view_name(const toc_config *cfg, const char *display,
                                 int index);
int toc_config_num_view_transforms(const toc_config *cfg);
const char *toc_config_view_transform_name(const toc_config *cfg, int index);
int toc_config_num_looks(const toc_config *cfg);
const char *toc_config_look_name(const toc_config *cfg, int index);
/* Returns -1 if not set (meaning all active), or count of active displays/views. */
int toc_config_num_active_displays(const toc_config *cfg);
const char *toc_config_active_display_name(const toc_config *cfg, int index);
int toc_config_num_active_views(const toc_config *cfg);
const char *toc_config_active_view_name(const toc_config *cfg, int index);

/* Hosted-only convenience (toc_stdio.c): load a config + default file reader. */
toc_result toc_config_load_file(const char *path, const toc_allocator *a,
                                toc_config **out);

/* ============================================================================
 * Processor build (config + endpoints -> flat op list)
 * ========================================================================== */

toc_result toc_processor_from_colorspaces(const toc_config *cfg, const char *src,
                                          const char *dst,
                                          const toc_allocator *a,
                                          toc_op_list **out);
toc_result toc_processor_from_display_view(const toc_config *cfg,
                                           const char *src_cs,
                                           const char *display, const char *view,
                                           const toc_allocator *a,
                                           toc_op_list **out);

/* ============================================================================
 * Backend 1: CPU interpreter (runtime SSE2/AVX2 dispatched)
 * ========================================================================== */

/* Apply the op list in place to interleaved float RGBA (channels 3 or 4). */
toc_result toc_apply(const toc_op_list *ops, float *rgba, size_t pixel_count,
                     int channels);

/* ============================================================================
 * Backend 2: AOT C-source codegen
 * ========================================================================== */

typedef struct toc_codegen_c_opts {
    const char *func_name; /* default "tocio_apply" */
    int simd;              /* 0 scalar, 1 SSE2, 2 AVX2 (matrix/range only) */
} toc_codegen_c_opts;

/* Emit a standalone .c source implementing the op chain. *out_src is allocated
 * with `a`; free with the allocator's free. */
toc_result toc_emit_c(const toc_op_list *ops, const toc_codegen_c_opts *opts,
                      const toc_allocator *a, char **out_src, size_t *out_len);

/* ============================================================================
 * Backend 3: GLSL/WebGL/Vulkan shader codegen
 * ========================================================================== */

typedef enum toc_glsl_target {
    TOC_GLSL_ES30 = 0,     /* "#version 300 es" (WebGL2) */
    TOC_GLSL_330,          /* "#version 330" (desktop GL) */
    TOC_GLSL_VULKAN450     /* "#version 450" (-> SPIR-V via glslang offline) */
} toc_glsl_target;

typedef enum toc_tex_dim {
    TOC_TEX_1D = 0,
    TOC_TEX_2D,
    TOC_TEX_3D
} toc_tex_dim;

typedef struct toc_texture_desc {
    char sampler_name[64];
    toc_tex_dim dim;
    int width, height, depth; /* 1D: width; 3D: N,N,N */
    int channels;             /* 3 or 4 */
    const float *data;        /* borrowed from the op list */
    int interp_linear;        /* GL_LINEAR (1) vs GL_NEAREST (0) */
} toc_texture_desc;

typedef struct toc_shader {
    char *source; /* full shader text (allocated) */
    toc_texture_desc *textures;
    size_t num_textures;
    toc_allocator alloc;
} toc_shader;

toc_result toc_emit_glsl(const toc_op_list *ops, toc_glsl_target target,
                         const toc_allocator *a, toc_shader *out);
/* Metal Shading Language (macOS/iOS). Emits a `float4 OCIOMain(float4, <LUT
 * textures>)` helper; LUT textures are appended as function arguments (named
 * per toc_texture_desc.sampler_name) and sampled with an in-shader sampler. */
toc_result toc_emit_metal(const toc_op_list *ops, const toc_allocator *a,
                          toc_shader *out);
void toc_shader_free(toc_shader *sh);

/* ============================================================================
 * Backend 4: x64 JIT (emit machine code to executable memory, run directly)
 *
 * Hosted-only (needs OS executable memory); x86-64 + SSE2. matrix/range are
 * inlined as SSE; transcendental/LUT ops call the C interpreter kernel. The
 * compiled function borrows the op list, which must outlive the jit. Returns
 * TOC_ERROR_UNSUPPORTED on non-x86-64 or if exec memory cannot be obtained.
 * ========================================================================== */
typedef struct toc_jit toc_jit;
typedef void (*toc_jit_fn)(float *rgba, size_t pixel_count);

/* `channels` (3 or 4) is baked into the compiled code. */
toc_result toc_jit_compile(const toc_op_list *ops, int channels,
                           const toc_allocator *a, toc_jit **out);
toc_jit_fn toc_jit_func(const toc_jit *j);
void toc_jit_destroy(toc_jit *j);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TOCIO_H_ */
