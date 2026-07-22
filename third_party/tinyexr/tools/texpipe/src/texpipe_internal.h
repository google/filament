/*
 * TinyEXR texpipe - internal shared declarations.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef TINYEXR_TEXPIPE_INTERNAL_H_
#define TINYEXR_TEXPIPE_INTERNAL_H_

#include "texpipe.h"

/* Allocator shims: NULL allocator -> malloc/free. */
void *tp_alloc(const tir_allocator *a, size_t size);
void tp_dealloc(const tir_allocator *a, void *ptr);

/* Static description of a codec: block geometry, input domain and the
 * container format codes. Retrieved via tp_codec_describe(). */
typedef struct tp_codec_desc {
    const char *name;
    int is_hdr;          /* 1 = consumes float RGB, 0 = 8-bit RGBA */
    int block_w;         /* texels per block (4 for BC/ETC; ASTC uses opt) */
    int block_h;
    int block_bytes;     /* bytes per block */
    int channels_in;     /* 3 (HDR RGB) or 4 (LDR RGBA) */
    uint32_t dxgi_format;/* DXGI_FORMAT for DDS (0 = not DDS-representable) */
    uint32_t vk_format;  /* VkFormat for KTX2  (0 = not KTX2-representable) */
} tp_codec_desc;

/* Fill `d` for `codec`, honoring sRGB / signed / block-size options. Returns
 * TP_ERROR_UNSUPPORTED for an unknown codec. */
tp_result tp_codec_describe(tp_codec codec, const tp_options *opt, tp_codec_desc *d);

/* Byte size of the compressed payload for one w*h surface with `codec`. */
size_t tp_codec_block_size(tp_codec codec, const tp_options *opt, uint32_t w,
                           uint32_t h);

/* Compress one float surface `s` into `out` (>= tp_codec_block_size bytes),
 * converting to the codec's input domain as needed. */
tp_result tp_codec_compress(tp_codec codec, const tp_options *opt,
                            const tir_allocator *a, const tp_surface *s,
                            uint8_t *out, size_t out_size);

/* Number of mip levels for a w*h base capped by opt->max_levels (0 = full). */
int tp_level_count(int w, int h, int max_levels);

/* Level dimension: max(1, base >> level). */
int tp_level_dim(int base, int level);

/* Multi-level container writers (texpipe_container.c). */
size_t tp_dds_size(const tp_blocks *b, const tp_options *opt);
tp_result tp_dds_write(const tp_blocks *b, const tp_options *opt, uint8_t *out,
                       size_t out_size, size_t *written);
size_t tp_ktx2_size(const tp_blocks *b, const tp_options *opt);
tp_result tp_ktx2_write(const tp_blocks *b, const tp_options *opt, uint8_t *out,
                        size_t out_size, size_t *written);

/* Little-endian store helpers. */
void tp_wr_u32(uint8_t *p, uint32_t v);
void tp_wr_u64(uint8_t *p, uint64_t v);

/* Read a sample from any-type view as float (handles tight stride). */
float tp_read_view_sample(const tir_image_view *v, int x, int y, int c);

#endif /* TINYEXR_TEXPIPE_INTERNAL_H_ */
