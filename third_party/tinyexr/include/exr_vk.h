/*
 * TinyEXR - optional Vulkan GPU backend (public API).
 *
 * Mirrors include/exr_gpu.h (the CUDA backend) but uses Vulkan compute via the
 * vkew loader: at runtime it dlopen()s libvulkan and resolves entry points, so
 * no Vulkan SDK is required at build time and we link only -ldl. Compute
 * shaders are precompiled SPIR-V embedded in the library. The header is always
 * includable; the symbols are real only when built with -DEXR_USE_VULKAN,
 * otherwise they are inert stubs that fall back to the CPU API. Even when built
 * in, exr_vk_available() probes for a usable device at runtime.
 *
 * Split matches the CUDA backend: CPU does the bit-serial entropy stages, the
 * GPU runs the parallel reconstruction passes (byte predictor, deinterleave),
 * channel split/gather, and all image processing (resize, convert, color,
 * tonemap, transfer, LUT). Non-scanline / deep / subsampled inputs fall back to
 * the CPU codec.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TINYEXR_EXR_VK_H_
#define TINYEXR_EXR_VK_H_

#include "exr.h"

#ifdef __cplusplus
extern "C" {
#endif

int exr_vk_available(void);
int exr_vk_device_count(void);
exr_result exr_vk_device_name(int device, char *buf, size_t buf_size);

typedef struct exr_vk_context exr_vk_context;

typedef struct exr_vk_options {
    int device;     /* physical device ordinal; -1 = default (0) */
    int verbose;    /* 0..3 diagnostic chatter to stderr */
} exr_vk_options;

exr_result exr_vk_context_create(const exr_allocator *alloc,
                                 const exr_vk_options *opts,
                                 exr_vk_context **out);
void exr_vk_context_destroy(exr_vk_context *ctx);

/* Hybrid decode / encode (same semantics and output as the CPU load/save). */
exr_result exr_vk_load_from_file(exr_vk_context *ctx, const char *path,
                                 const exr_allocator *alloc, exr_image *out);
exr_result exr_vk_load_from_memory(exr_vk_context *ctx, const void *data,
                                   size_t size, const exr_allocator *alloc,
                                   exr_image *out);
exr_result exr_vk_save_to_file(exr_vk_context *ctx, const char *path,
                               const exr_image *img, exr_compression comp);
exr_result exr_vk_save_to_memory(exr_vk_context *ctx, void **out_data,
                                 size_t *out_size, const exr_allocator *alloc,
                                 const exr_image *img, exr_compression comp);

/* Image processing (GPU twins of the exr.h utilities; same arg semantics). */
exr_result exr_vk_resize_float(exr_vk_context *ctx, const float *src, int src_w,
                               int src_h, size_t src_row_stride, float *dst,
                               int dst_w, int dst_h, size_t dst_row_stride,
                               int channels, exr_resize_filter filter,
                               exr_edge_mode edge, int alpha_channel);
exr_result exr_vk_convert_pixels(exr_vk_context *ctx, void *dst,
                                 exr_pixel_type dst_type, const void *src,
                                 exr_pixel_type src_type, size_t count,
                                 exr_convert_mode mode);
exr_result exr_vk_color_apply_matrix(exr_vk_context *ctx, float *dst,
                                     const float *src, size_t pixel_count,
                                     int channels, const float m[9]);
exr_result exr_vk_tonemap_float(exr_vk_context *ctx, float *dst, const float *src,
                                size_t pixel_count, int channels,
                                exr_tonemap_op op,
                                const exr_tonemap_params *params);
exr_result exr_vk_encode_transfer(exr_vk_context *ctx, float *dst,
                                  const float *src, size_t count,
                                  exr_transfer tf);
exr_result exr_vk_decode_transfer(exr_vk_context *ctx, float *dst,
                                  const float *src, size_t count,
                                  exr_transfer tf);
exr_result exr_vk_lut3d_apply(exr_vk_context *ctx, float *dst, const float *src,
                              size_t pixel_count, int channels,
                              const exr_lut3d *lut, exr_lut_interp interp);

/* Channel scatter / gather (extract / combine). */
exr_result exr_vk_part_to_rgba_float(exr_vk_context *ctx, const exr_allocator *a,
                                     const exr_part *part, float **out,
                                     int *out_width, int *out_height,
                                     int *out_channels);
exr_result exr_vk_rgba_float_to_part(exr_vk_context *ctx, const exr_allocator *a,
                                     const float *rgba, int width, int height,
                                     int channels, exr_pixel_type dst_type,
                                     exr_part *out);

#ifdef __cplusplus
}
#endif

#endif /* TINYEXR_EXR_VK_H_ */
