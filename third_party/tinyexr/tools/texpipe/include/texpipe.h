/*
 * TinyEXR texpipe - resize-aware texture compression.
 *
 * A small pure-C11 orchestration layer that ties together `tir` (content-aware
 * image resize) and `texcomp` (BC/ETC/ASTC block compression) to build
 * content-aware mip chains and serialize them into multi-mip GPU texture
 * containers (DDS, KTX2).
 *
 * Phase 0/1 (this file): mip-chain generation (resize-from-base), alpha-aware
 * premultiplied RGBA resize+compress, alpha-coverage preservation across LODs,
 * and multi-level DDS + KTX2 output. Normal/height LOD coherence and seam-free
 * cubemap LOD are declared here but implemented in later phases.
 *
 * Depends on tir.h (BSD-3-Clause) and texcomp.h (Apache-2.0).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef TINYEXR_TEXPIPE_H_
#define TINYEXR_TEXPIPE_H_

#include <stddef.h>
#include <stdint.h>

#include "tir.h"      /* tir_image_view, tir_pixel_type, tir_allocator, ... */
#include "texcomp.h"  /* tc_*_options codec knobs */

#ifdef __cplusplus
extern "C" {
#endif

#define TP_VERSION_MAJOR 0
#define TP_VERSION_MINOR 1
#define TP_VERSION_PATCH 0

typedef enum tp_result {
    TP_SUCCESS = 0,
    TP_ERROR_INVALID_ARGUMENT = -1,
    TP_ERROR_OUT_OF_MEMORY = -2,
    TP_ERROR_UNSUPPORTED = -3,
    TP_ERROR_IO = -4
} tp_result;

#define TP_OK(r) ((int)(r) == 0)

const char *tp_result_string(tp_result r);

/* Content class: selects the tir resize mode and which per-mip float passes
 * run. NORMAL/HEIGHT are wired in Phase 2 (currently treated as COLOR with a
 * TP_ERROR_UNSUPPORTED guard on the passes that are not yet implemented). */
typedef enum tp_content {
    TP_CONTENT_COLOR = 0,        /* tir GENERAL; premult/straight alpha        */
    TP_CONTENT_ALPHA_TESTED = 1, /* GENERAL + alpha-coverage preservation      */
    TP_CONTENT_NORMAL = 2,       /* NORMAL_MAP + renorm (+Toksvig) - Phase 2   */
    TP_CONTENT_HEIGHT = 3        /* HEIGHTMAP, mean-preserving       - Phase 2 */
} tp_content;

/* Block codec. LDR codecs consume 8-bit RGBA; HDR codecs (BC6H, ASTC_HDR)
 * consume float RGB. */
typedef enum tp_codec {
    TP_CODEC_BC1 = 0,
    TP_CODEC_BC3,
    TP_CODEC_BC5,
    TP_CODEC_BC7,
    TP_CODEC_BC6H,     /* HDR */
    TP_CODEC_ETC2_RGB,
    TP_CODEC_ETC2_RGBA,
    TP_CODEC_EAC_R11,
    TP_CODEC_EAC_RG11,
    TP_CODEC_ASTC,     /* LDR, arbitrary block */
    TP_CODEC_ASTC_HDR  /* HDR, 4x4 */
} tp_codec;

typedef enum tp_container {
    TP_CONTAINER_DDS = 0,  /* DX10 header; BC1/3/5/6H/7 */
    TP_CONTAINER_KTX2 = 1  /* Vulkan format; BC / ETC2 / EAC / ASTC */
} tp_container;

typedef enum tp_mip_source {
    TP_MIP_FROM_BASE = 0,     /* resample level 0 at each target size (default) */
    TP_MIP_FROM_PREVIOUS = 1  /* resample the previous level (lower quality)    */
} tp_mip_source;

/* Texture projection. TP_PROJ_OCTA runs an octahedral fold-seam fixup per mip
 * so the map's outer border stays coherent across LODs (like the cube fixup).
 * Cubemaps are selected by num_faces == 6, not here. */
typedef enum tp_projection {
    TP_PROJ_2D = 0,
    TP_PROJ_OCTA = 1
} tp_projection;

/* Per-channel downsample rule for packed material maps (ORM / masks). Applied
 * to a 4-channel COLOR surface so each packed channel minifies correctly. */
typedef enum tp_channel_op {
    TP_CH_LINEAR = 0,   /* filtered average (AO, linear data) — default */
    TP_CH_MAJORITY = 1, /* threshold to {0,1} at 0.5 (binary metallic / mask):
                         * keeps edges crisp instead of averaging to gray */
    TP_CH_ROUGHNESS = 2 /* variance-aware roughening: bump the channel by its
                         * footprint variance so minified roughness reduces
                         * specular aliasing (roughness = sqrt(mean^2 + var)) */
} tp_channel_op;

/* Cubemap face input layout (Phase 3). Kept here so the option struct is
 * stable; only TP_CUBE_SEPARATE is honored today. Face order is the KTX/D3D/GL
 * convention +X,-X,+Y,-Y,+Z,-Z. */
typedef enum tp_cube_layout {
    TP_CUBE_SEPARATE = 0, /* 6 independent face images                         */
    TP_CUBE_CROSS_H = 1,  /* horizontal cross                                  */
    TP_CUBE_CROSS_V = 2,  /* vertical cross                                    */
    TP_CUBE_STRIP_H = 3,  /* 6-wide strip                                      */
    TP_CUBE_STRIP_V = 4   /* 6-tall strip                                      */
} tp_cube_layout;

typedef struct tp_options {
    tp_content content;
    tp_codec codec;
    tp_container container;

    /* -- mip chain ------------------------------------------------------- */
    tp_mip_source mip_source;   /* FROM_BASE */
    int max_levels;             /* 0 = full chain down to 1x1 */
    tir_filter filter;          /* AUTO; forwarded to tir_options filter_x/y */
    tir_edge_mode edge_x;       /* CLAMP */
    tir_edge_mode edge_y;       /* CLAMP */

    /* -- color / alpha --------------------------------------------------- */
    int srgb;                   /* tag container sRGB */
    int srgb_aware;             /* COLOR: decode sRGB->linear, filter, re-encode
                                 * (correct albedo mips); implies srgb tag */
    tir_alpha_mode alpha;       /* PREMULTIPLY */
    float alpha_test_threshold; /* 0.5 */
    int preserve_alpha_coverage;/* auto-on for ALPHA_TESTED; -1 = auto */

    /* -- normal / height (Phase 2; declared now) ------------------------- */
    tir_normal_enc normal_encoding; /* SNORM */
    int renormalize;            /* 1 */
    int bake_toksvig_roughness; /* 0 */
    float base_roughness;       /* 0 */

    /* -- cube (Phase 3; declared now) ------------------------------------ */
    int is_cube;
    tp_cube_layout cube_layout;
    int cube_seam_fixup;        /* 1 */
    int cube_fixup_max_level;   /* -1 = all */

    /* -- octahedral ------------------------------------------------------ */
    tp_projection projection;   /* TP_PROJ_2D */
    int octa_seam_fixup;        /* 1; fold-edge fixup per mip when OCTA */

    /* -- packed material channels (ORM/mask) ----------------------------- */
    tp_channel_op channel_op[4];/* per-channel downsample rule; default LINEAR */
    int ycocg;                  /* COLOR: store RGB as YCoCg before compression
                                 * (shader inverts); better chroma at same size */

    /* -- codec knobs (forwarded verbatim to texcomp) --------------------- */
    tc_bc7_options bc7;
    tc_bc1_options bc1;
    tc_bc3_options bc3;
    tc_bc5_options bc5;
    tc_bc6h_options bc6h;
    tc_etc2_options etc2;
    tc_astc_options astc;
    tc_astc_hdr_options astc_hdr;

    int threads;
} tp_options;

/* Fill `opt` with defaults for the given content class + codec. */
void tp_options_init(tp_options *opt, tp_content content, tp_codec codec);

/* ===========================================================================
 * Intermediate structures (caller-visible so the staged API composes)
 * ========================================================================= */

/* One uncompressed float surface. `data` is `channels` interleaved F32 texels,
 * rows top to bottom, `stride` bytes per row. */
typedef struct tp_surface {
    int width;
    int height;
    int channels;
    float *data;
    size_t stride; /* bytes per row */
} tp_surface;

/* Content-aware mip pyramid. Surfaces are stored face-major:
 * level[face * num_levels + level], level 0 is the largest. num_faces is 1
 * (2D) or 6 (cube). */
typedef struct tp_mip_chain {
    int num_faces;
    int num_levels;
    int channels;
    tp_surface *level;    /* num_faces * num_levels surfaces */
    float **roughness;    /* optional per-surface Toksvig roughness, or NULL */
} tp_mip_chain;

/* Compressed block payloads, one per surface, same face-major order. */
typedef struct tp_block_level {
    uint8_t *data;
    size_t size;
    uint32_t width;
    uint32_t height;
} tp_block_level;

typedef struct tp_blocks {
    tp_codec codec;
    int num_faces;
    int num_levels;
    tp_block_level *blk;  /* num_faces * num_levels payloads */
} tp_blocks;

/* ===========================================================================
 * Staged pipeline
 * ========================================================================= */

/* Base surface(s) -> content-aware uncompressed float mip chain. Runs resize
 * (from base by default), then per-mip float passes (alpha-coverage for
 * ALPHA_TESTED). `faces` has `num_faces` entries (1 today; 6 with is_cube in
 * Phase 3). */
tp_result tp_build_mips(const tir_allocator *a, const tir_image_view *faces,
                        int num_faces, const tp_options *opt,
                        tp_mip_chain *out);
void tp_mip_chain_free(const tir_allocator *a, tp_mip_chain *c);

/* Compress every surface with the selected codec. */
tp_result tp_compress_chain(const tir_allocator *a, const tp_mip_chain *c,
                            const tp_options *opt, tp_blocks *out);
void tp_blocks_free(const tir_allocator *a, tp_blocks *b);

/* Serialize compressed blocks to a multi-mip (and, later, multi-face)
 * container. tp_container_size returns the exact byte count; tp_write_container
 * writes into a caller buffer of at least that size. */
size_t tp_container_size(const tp_blocks *b, const tp_options *opt);
tp_result tp_write_container(const tp_blocks *b, const tp_options *opt,
                             uint8_t *out, size_t out_size, size_t *written);

/* One-shot: base -> container bytes (caller frees *out with tp_free). */
tp_result tp_process(const tir_allocator *a, const tir_image_view *faces,
                     int num_faces, const tp_options *opt, uint8_t **out,
                     size_t *out_size);
void tp_free(const tir_allocator *a, void *ptr);

/* Texture arrays: write `num_layers` compressed chains (all same codec / faces /
 * levels / dimensions) into one KTX2 with layerCount = num_layers. Layers are
 * interleaved per level per the KTX2 order (layer, face, z). */
size_t tp_ktx2_array_size(const tp_blocks *layers, int num_layers,
                          const tp_options *opt);
tp_result tp_write_ktx2_array(const tp_blocks *layers, int num_layers,
                              const tp_options *opt, uint8_t *out,
                              size_t out_size, size_t *written);

/* ===========================================================================
 * Leaf helpers (also the Phase 1 test surface)
 * ========================================================================= */

/* Fraction of texels (in [0,1]) whose alpha channel is >= threshold. Requires
 * a 4-channel surface. */
float tp_alpha_coverage(const tp_surface *s, float threshold);

/* Binary-search a scalar alpha multiplier so tp_alpha_coverage(s,threshold)
 * matches `target`, then apply it (alpha clamped to [0,1]). No-op if the
 * surface has < 4 channels. */
tp_result tp_alpha_scale_to_coverage(tp_surface *s, float target,
                                     float threshold);

/* Min-max height pyramid (for parallax-occlusion / relief / cone-step mapping):
 * builds a 2-channel chain (R=min height, G=max height over the footprint) by
 * conservative min/max reduction from the previous level. `channel` selects the
 * height channel of the base. The bounds are conservative in float; if you then
 * block-compress, widen them (min down / max up) to stay conservative. Free
 * with tp_mip_chain_free. */
tp_result tp_build_minmax_pyramid(const tir_allocator *a,
                                  const tir_image_view *height, int channel,
                                  int max_levels, tp_mip_chain *out);

/* Gutter dilation: flood the colour channels of texels whose `valid_channel`
 * is below `threshold` from their valid neighbours, `iters` passes (each grows
 * the valid region by one texel). Run on the base before mipping so bilinear /
 * minification don't bleed background across atlas/lightmap chart borders. The
 * valid_channel itself (e.g. alpha) is left unchanged. */
tp_result tp_dilate(tp_surface *s, int valid_channel, float threshold,
                    int iters);

/* Cone-step ratio map (relaxed cone step / relief mapping): for each texel of
 * the height field (channel `channel` of `height`), the widest empty cone ratio
 * = min over higher texels of horizontal_dist / height_diff, in [0,1]. `out`
 * becomes a 1-channel surface (free out->data with the allocator). O(n^2 * n^2)
 * — use modest resolutions (<= ~128). */
tp_result tp_build_cone_map(const tir_allocator *a, const tir_image_view *height,
                            int channel, tp_surface *out);

/* Ripmap (anisotropic) grid: cell (ix,jy) has dimensions (w>>ix, h>>jy) and is
 * stored at out->level[jy*(*out_nx) + ix]. Reuses tp_mip_chain (num_faces = 1,
 * num_levels = nx*ny). Free with tp_mip_chain_free. */
tp_result tp_build_ripmap(const tir_allocator *a, const tir_image_view *base,
                          const tp_options *opt, tp_mip_chain *out, int *out_nx,
                          int *out_ny);

/* YCoCg decorrelation (float; Co,Cg biased by +0.5 into [0,1]). Storing colour
 * as YCoCg before BC/ASTC compression decorrelates chroma and improves quality;
 * the shader must invert with tp_ycocg_to_rgb after sampling. */
void tp_rgb_to_ycocg(const float rgb[3], float ycocg[3]);
void tp_ycocg_to_rgb(const float ycocg[3], float rgb[3]);

/* Seam-free cubemap fixup: average the shared edge and corner texels across
 * the 6 cube faces (order +X,-X,+Y,-Y,+Z,-Z) so borders become bit-identical
 * between adjacent faces. Operates in-place on the float surfaces; faces must
 * be square and equal-sized. Run per mip level.
 *
 * NOTE (honest limit): this makes borders identical BEFORE compression. Block
 * codecs compress each face independently, so a residual seam of up to one
 * quantization step can reappear after encoding even with identical float
 * borders. tp_build_mips applies this automatically for 6-face inputs. */
tp_result tp_cube_seam_fixup(tp_surface faces[6], const tp_options *opt);

/* Split one packed image into 6 face sub-rect views (no allocation; the views
 * point into src's pixels). Face order +X,-X,+Y,-Y,+Z,-Z. Cross/strip layouts
 * assume no per-face rotation. Returns TP_ERROR_INVALID_ARGUMENT if the image
 * geometry doesn't fit the layout (needs square, equal faces). */
tp_result tp_cube_split(const tir_image_view *src, tp_cube_layout layout,
                        tir_image_view out[6]);

/* Octahedral fold-seam fixup: mirror-average the square map's outer border
 * (top/bottom rows and left/right columns fold onto themselves reversed; the 4
 * corners meet at one pole) so borders are coherent across LODs. In-place per
 * mip. tp_build_mips applies this automatically when opt->projection ==
 * TP_PROJ_OCTA. Same post-compression seam caveat as the cube fixup. */
tp_result tp_octa_seam_fixup(tp_surface *s, const tp_options *opt);

/* Toksvig roughness (Toksvig 2005, "Mipmapping Normal Maps"): map per-texel
 * averaged-normal length |N| in (0,1] to an effective roughness, given a base
 * roughness, via a Beckmann power<->roughness conversion. |N| == 1 (no sub-
 * texel normal spread) yields base_roughness; shorter |N| yields a rougher
 * value. Output is never smoother than base_roughness. normal_len and out_rough
 * may alias (computed per texel). */
tp_result tp_toksvig_roughness(const float *normal_len, int count,
                               float base_roughness, float *out_rough);

/* Build a compressible roughness pyramid (4-channel float, roughness in R,
 * G=B=0, A=1) from a chain produced with content == TP_CONTENT_NORMAL and
 * bake_toksvig_roughness set. Returns TP_ERROR_UNSUPPORTED if the source chain
 * carries no roughness. Free the result with tp_mip_chain_free. */
tp_result tp_build_roughness_chain(const tir_allocator *a,
                                   const tp_mip_chain *src, tp_mip_chain *out);

#ifdef __cplusplus
}
#endif

#endif /* TINYEXR_TEXPIPE_H_ */
