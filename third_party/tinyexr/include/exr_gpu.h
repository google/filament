/*
 * TinyEXR - optional CUDA GPU backend (public API).
 *
 * This header is ALWAYS includable and its declarations are unconditional, so
 * callers compile whether or not the library was built with CUDA. The symbols
 * resolve to real implementations only when the library is built with
 * -DEXR_USE_CUDA; otherwise every entry returns EXR_ERROR_UNSUPPORTED (and
 * exr_gpu_available() returns 0). Even with CUDA support compiled in, the
 * backend probes for a usable device + NVRTC at runtime (via cuew/dlopen) and
 * degrades gracefully when none is present — so the honest pattern is:
 *
 *     if (exr_gpu_available()) { ... exr_gpu_* ... } else { ... CPU API ... }
 *
 * The split is HYBRID: the bit-serial EXR entropy stages (DEFLATE/PIZ/RLE/...)
 * run on the CPU; the GPU runs the parallel reconstruction passes (byte
 * predictor, even/odd deinterleave, half<->float, channel scatter/gather) plus
 * all image processing (resize, extract/combine channel, color, tonemap,
 * transfer, format conversion). Kernels are compiled at runtime with NVRTC, so
 * no CUDA SDK is required at build time.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TINYEXR_EXR_GPU_H_
#define TINYEXR_EXR_GPU_H_

#include "exr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Availability & device query
 * ========================================================================== */

/* 1 if the library was built with CUDA support AND a usable device + NVRTC are
 * present at runtime; 0 otherwise. Never allocates; safe to call any time. */
int exr_gpu_available(void);

/* Number of CUDA devices visible to the driver (0 if unavailable). */
int exr_gpu_device_count(void);

/* Copy device `device`'s name into buf (always NUL-terminated when buf_size>0).
 * EXR_ERROR_UNSUPPORTED if no CUDA, EXR_ERROR_INVALID_ARGUMENT on bad args. */
exr_result exr_gpu_device_name(int device, char *buf, size_t buf_size);

/* ============================================================================
 * Context lifecycle
 * ========================================================================== */

typedef struct exr_gpu_context exr_gpu_context;

typedef struct exr_gpu_options {
    int    device;            /* CUDA device ordinal; -1 = default (0). */
    int    num_streams;       /* streaming ring depth; 0 -> default (3). */
    size_t max_device_bytes;  /* soft VRAM working-set cap; 0 -> auto (~70%). */
    int    verbose;           /* 0..3 diagnostic chatter to stderr. */
} exr_gpu_options;

/* Create a context: select device, create a CUDA context + stream ring, and
 * NVRTC-compile/cache the kernel module (CUBIN preferred, PTX fallback). Returns
 * EXR_ERROR_UNSUPPORTED when CUDA/NVRTC/device is unavailable. `opts` may be
 * NULL for defaults. `alloc` is used for host-side allocations (NULL = default).
 */
exr_result exr_gpu_context_create(const exr_allocator *alloc,
                                  const exr_gpu_options *opts,
                                  exr_gpu_context **out);
void exr_gpu_context_destroy(exr_gpu_context *ctx);

/* ============================================================================
 * Hybrid decode / encode (high level)
 *
 * Same semantics and output as the CPU exr_load / exr_save calls: the GPU
 * accelerates the reconstruction/conversion passes and falls back to the CPU
 * path for deep parts or unsupported cases. Output images are owned by the
 * caller (release with exr_image_free).
 * ========================================================================== */

exr_result exr_gpu_load_from_file(exr_gpu_context *ctx, const char *path,
                                  const exr_allocator *alloc, exr_image *out);
exr_result exr_gpu_load_from_memory(exr_gpu_context *ctx, const void *data,
                                    size_t size, const exr_allocator *alloc,
                                    exr_image *out);
exr_result exr_gpu_save_to_file(exr_gpu_context *ctx, const char *path,
                                const exr_image *img, exr_compression comp);
exr_result exr_gpu_save_to_memory(exr_gpu_context *ctx, void **out_data,
                                  size_t *out_size, const exr_allocator *alloc,
                                  const exr_image *img, exr_compression comp);

/* ============================================================================
 * Image processing (GPU twins of the exr.h utilities; same arg semantics)
 * ========================================================================== */

exr_result exr_gpu_resize_float(exr_gpu_context *ctx, const float *src,
                                int src_w, int src_h, size_t src_row_stride,
                                float *dst, int dst_w, int dst_h,
                                size_t dst_row_stride, int channels,
                                exr_resize_filter filter, exr_edge_mode edge,
                                int alpha_channel);

exr_result exr_gpu_convert_pixels(exr_gpu_context *ctx, void *dst,
                                  exr_pixel_type dst_type, const void *src,
                                  exr_pixel_type src_type, size_t count,
                                  exr_convert_mode mode);

exr_result exr_gpu_color_apply_matrix(exr_gpu_context *ctx, float *dst,
                                      const float *src, size_t pixel_count,
                                      int channels, const float m[9]);

exr_result exr_gpu_tonemap_float(exr_gpu_context *ctx, float *dst,
                                 const float *src, size_t pixel_count,
                                 int channels, exr_tonemap_op op,
                                 const exr_tonemap_params *params);

exr_result exr_gpu_encode_transfer(exr_gpu_context *ctx, float *dst,
                                   const float *src, size_t count,
                                   exr_transfer tf);
exr_result exr_gpu_decode_transfer(exr_gpu_context *ctx, float *dst,
                                   const float *src, size_t count,
                                   exr_transfer tf);

exr_result exr_gpu_lut3d_apply(exr_gpu_context *ctx, float *dst,
                               const float *src, size_t pixel_count,
                               int channels, const exr_lut3d *lut,
                               exr_lut_interp interp);

/* ============================================================================
 * Channel scatter / gather (extract / combine)
 * ========================================================================== */

/* GPU twin of exr_part_to_rgba_float: gather a part's channels (sorted order,
 * subsampling expanded, any pixel type widened to float) into a freshly
 * allocated interleaved RGBA float buffer. *out freed with the same allocator. */
exr_result exr_gpu_part_to_rgba_float(exr_gpu_context *ctx,
                                      const exr_allocator *a,
                                      const exr_part *part, float **out,
                                      int *out_width, int *out_height,
                                      int *out_channels);

/* GPU twin of exr_rgba_float_to_part: scatter interleaved float into freshly
 * allocated planar channels of dst_type (named R,G,B,A). Release with
 * exr_part_free. */
exr_result exr_gpu_rgba_float_to_part(exr_gpu_context *ctx,
                                      const exr_allocator *a, const float *rgba,
                                      int width, int height, int channels,
                                      exr_pixel_type dst_type, exr_part *out);

/* ---- HTJ2K HT block coder on GPU ----------------------------------------
 * Decode every i32-eligible code-block of a collected plan (exr_jph_cb_plan,
 * defined in the internal header) in one kernel launch — one GPU thread per
 * code-block. out_coeffs is a host buffer of out_count int32 sign/magnitude
 * coefficients; eligible record i writes its tile at tile_offsets[i] with row
 * stride (width+7)&~7. Mainly for the hybrid HTJ2K decode path and bit-exact
 * testing against the CPU coder. Returns EXR_ERROR_UNSUPPORTED without CUDA. */
struct exr_jph_cb_plan;
exr_result exr_gpu_jph_decode_plan(exr_gpu_context *ctx,
                                   const struct exr_jph_cb_plan *plan,
                                   const size_t *tile_offsets, size_t out_count,
                                   int32_t *out_coeffs);

/* Encode every i32-eligible code-block of an encode plan (exr_jph_enc_plan) in
 * one launch (one thread per block). Block i writes its cleanup-pass bytes at
 * out_bytes + i*out_stride; per-block missing_msbs / length0 / size land in the
 * out arrays (size 0 => all-zero/empty block). EXR_ERROR_UNSUPPORTED w/o CUDA. */
struct exr_jph_enc_plan;
exr_result exr_gpu_jph_encode_plan(exr_gpu_context *ctx,
                                   const struct exr_jph_enc_plan *plan,
                                   unsigned char *out_bytes,
                                   unsigned int out_stride,
                                   unsigned int *out_missing,
                                   unsigned int *out_len0,
                                   unsigned int *out_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TINYEXR_EXR_GPU_H_ */
