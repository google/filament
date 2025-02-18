/*
 * Copyright © 2018 NVIDIA Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef VIC_H
#define VIC_H

#include <stdio.h>

#include "host1x.h"

#define DXVAHD_FRAME_FORMAT_PROGRESSIVE 0
#define DXVAHD_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST 1
#define DXVAHD_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST 2
#define DXVAHD_FRAME_FORMAT_TOP_FIELD 3
#define DXVAHD_FRAME_FORMAT_BOTTOM_FIELD 4
#define DXVAHD_FRAME_FORMAT_SUBPIC_PROGRESSIVE 5
#define DXVAHD_FRAME_FORMAT_SUBPIC_INTERLACED_TOP_FIELD_FIRST 6
#define DXVAHD_FRAME_FORMAT_SUBPIC_INTERLACED_BOTTOM_FIELD_FIRST 7
#define DXVAHD_FRAME_FORMAT_SUBPIC_TOP_FIELD 8
#define DXVAHD_FRAME_FORMAT_SUBPIC_BOTTOM_FIELD 9
#define DXVAHD_FRAME_FORMAT_TOP_FIELD_CHROMA_BOTTOM 10
#define DXVAHD_FRAME_FORMAT_BOTTOM_FIELD_CHROMA_TOP 11
#define DXVAHD_FRAME_FORMAT_SUBPIC_TOP_FIELD_CHROMA_BOTTOM 12
#define DXVAHD_FRAME_FORMAT_SUBPIC_BOTTOM_FIELD_CHROMA_TOP 13

#define DXVAHD_ALPHA_FILL_MODE_OPAQUE 0
#define DXVAHD_ALPHA_FILL_MODE_BACKGROUND 1
#define DXVAHD_ALPHA_FILL_MODE_DESTINATION 2
#define DXVAHD_ALPHA_FILL_MODE_SOURCE_STREAM 3
#define DXVAHD_ALPHA_FILL_MODE_COMPOSITED 4
#define DXVAHD_ALPHA_FILL_MODE_SOURCE_ALPHA 5

#define VIC_BLEND_SRCFACTC_K1 0
#define VIC_BLEND_SRCFACTC_K1_TIMES_DST 1
#define VIC_BLEND_SRCFACTC_NEG_K1_TIMES_DST 2
#define VIC_BLEND_SRCFACTC_K1_TIMES_SRC 3
#define VIC_BLEND_SRCFACTC_ZERO 4

#define VIC_BLEND_DSTFACTC_K1 0
#define VIC_BLEND_DSTFACTC_K2 1
#define VIC_BLEND_DSTFACTC_K1_TIMES_DST 2
#define VIC_BLEND_DSTFACTC_NEG_K1_TIMES_DST 3
#define VIC_BLEND_DSTFACTC_NEG_K1_TIMES_SRC 4
#define VIC_BLEND_DSTFACTC_ZERO 5
#define VIC_BLEND_DSTFACTC_ONE 6

#define VIC_BLEND_SRCFACTA_K1 0
#define VIC_BLEND_SRCFACTA_K2 1
#define VIC_BLEND_SRCFACTA_NEG_K1_TIMES_DST 2
#define VIC_BLEND_SRCFACTA_ZERO 3

#define VIC_BLEND_DSTFACTA_K2 0
#define VIC_BLEND_DSTFACTA_NEG_K1_TIMES_SRC 1
#define VIC_BLEND_DSTFACTA_ZERO 2
#define VIC_BLEND_DSTFACTA_ONE 3

#define VIC_BLK_KIND_PITCH 0
#define VIC_BLK_KIND_GENERIC_16Bx2 1

#define VIC_PIXEL_FORMAT_L8 1
#define VIC_PIXEL_FORMAT_R8 4
#define VIC_PIXEL_FORMAT_A8R8G8B8 32
#define VIC_PIXEL_FORMAT_R8G8B8A8 34
#define VIC_PIXEL_FORMAT_Y8_U8V8_N420 67
#define VIC_PIXEL_FORMAT_Y8_V8U8_N420 68

#define VIC_CACHE_WIDTH_16Bx16 0 /* BL16Bx2 */
#define VIC_CACHE_WIDTH_32Bx8 1 /* BL16Bx2 */
#define VIC_CACHE_WIDTH_64Bx4 2 /* BL16Bx2, PL */
#define VIC_CACHE_WIDTH_128Bx2 3 /* BL16Bx2, PL */
#define VIC_CACHE_WIDTH_256Bx1 4 /* PL */

struct vic_format_info {
    unsigned int format;
    unsigned int cpp;
};


#define VIC_UCLASS_INCR_SYNCPT 0x00
#define VIC_UCLASS_METHOD_OFFSET 0x10
#define VIC_UCLASS_METHOD_DATA 0x11

static inline void VIC_PUSH_METHOD(struct drm_tegra_pushbuf *pushbuf,
                                   uint32_t **ptrp, uint32_t method,
                                   uint32_t value)
{
    *(*ptrp)++ = HOST1X_OPCODE_INCR(VIC_UCLASS_METHOD_OFFSET, 2);
    *(*ptrp)++ = method >> 2;
    *(*ptrp)++ = value;
}

static inline void VIC_PUSH_BUFFER(struct drm_tegra_pushbuf *pushbuf,
                                   uint32_t **ptrp, uint32_t method,
                                   struct drm_tegra_mapping *map,
                                   unsigned long offset, unsigned long flags)
{
    *(*ptrp)++ = HOST1X_OPCODE_INCR(VIC_UCLASS_METHOD_OFFSET, 2);
    *(*ptrp)++ = method >> 2;

    drm_tegra_pushbuf_relocate(pushbuf, ptrp, map, offset, 8, flags);
}

struct vic_image;
struct vic;

struct vic_ops {
    int (*fill)(struct vic *vic, struct vic_image *output,
                unsigned int left, unsigned int top,
                unsigned int right, unsigned int bottom,
                unsigned int alpha, unsigned red,
                unsigned int green, unsigned int blue);
    int (*blit)(struct vic *vic, struct vic_image *output,
                struct vic_image *input);
    int (*flip)(struct vic *vic, struct vic_image *output,
                struct vic_image *input);
    int (*execute)(struct vic *vic,
                   struct drm_tegra_pushbuf *pushbuf,
                   uint32_t **ptrp,
                   struct vic_image *output,
                   struct vic_image **inputs,
                   unsigned int num_inputs);
    void (*free)(struct vic *vic);
};

struct vic {
    struct drm_tegra *drm;
    struct drm_tegra_channel *channel;
    struct drm_tegra_syncpoint *syncpt;
    const struct vic_ops *ops;
    unsigned int version;
};

int vic_new(struct drm_tegra *drm, struct drm_tegra_channel *channel,
            struct vic **vicp);
void vic_free(struct vic *vic);

int vic_clear(struct vic *vic, struct vic_image *output, unsigned int alpha,
              unsigned int red, unsigned int green, unsigned int blue);

struct vic_image {
    struct drm_tegra_bo *bo;
    struct drm_tegra_mapping *map;
    unsigned int width;
    unsigned int stride;
    unsigned int pitch;
    unsigned int height;
    unsigned int format;
    unsigned int kind;

    size_t align;
    size_t size;
};

const struct vic_format_info *vic_format_get_info(unsigned int format);

int vic_image_new(struct vic *vic, unsigned int width, unsigned int height,
                  unsigned int format, unsigned int kind, uint32_t flags,
                  struct vic_image **imagep);
void vic_image_free(struct vic_image *image);
void vic_image_dump(struct vic_image *image, FILE *fp);

#endif
