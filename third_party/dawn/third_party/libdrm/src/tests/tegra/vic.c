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

#include <errno.h>
#include <stdio.h> /* XXX remove */
#include <stdlib.h>

#include "util_math.h"

#include "tegra.h"
#include "host1x.h"
#include "vic.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

const struct vic_format_info *vic_format_get_info(unsigned int format)
{
    static const struct vic_format_info formats[] = {
        { .format = VIC_PIXEL_FORMAT_A8R8G8B8, .cpp = 4 },
    };
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(formats); i++) {
        if (formats[i].format == format)
            return &formats[i];
    }

    return 0;
}

int vic_image_new(struct vic *vic, unsigned int width, unsigned int height,
                  unsigned int format, unsigned int kind, uint32_t flags,
                  struct vic_image **imagep)
{
    const struct vic_format_info *info = vic_format_get_info(format);
    struct vic_image *image;
    int err;

    if (!info)
        return -EINVAL;

    image = calloc(1, sizeof(*image));
    if (!image)
        return -ENOMEM;

    if (kind == VIC_BLK_KIND_PITCH)
        image->align = 256;
    else
        image->align = 256; /* XXX */

    image->width = width;
    image->stride = ALIGN(width, image->align);
    image->pitch = image->stride * info->cpp;
    image->height = height;
    image->format = format;
    image->kind = kind;

    image->size = image->pitch * image->height;

    printf("image: %ux%u align: %zu stride: %u pitch: %u size: %zu\n",
           image->width, image->height, image->align, image->stride,
           image->pitch, image->size);

    err = drm_tegra_bo_new(vic->drm, 0, image->size, &image->bo);
    if (err < 0) {
        free(image);
        return err;
    }

    err = drm_tegra_channel_map(vic->channel, image->bo, flags, &image->map);
    if (err < 0) {
        drm_tegra_bo_unref(image->bo);
        free(image);
        return err;
    }

    *imagep = image;
    return 0;
}

void vic_image_free(struct vic_image *image)
{
    if (image) {
        drm_tegra_channel_unmap(image->map);
        drm_tegra_bo_unref(image->bo);
        free(image);
    }
}

void vic_image_dump(struct vic_image *image, FILE *fp)
{
    unsigned int i, j;
    void *ptr;
    int err;

    err = drm_tegra_bo_map(image->bo, &ptr);
    if (err < 0)
        return;

    for (j = 0; j < image->height; j++) {
        uint32_t *pixels = (uint32_t *)((unsigned long)ptr + j * image->pitch);

        printf("   ");

        for (i = 0; i < image->width; i++)
            printf(" %08x", pixels[i]);

        printf("\n");
    }

    drm_tegra_bo_unmap(image->bo);
}

/* from vic30.c */
int vic30_new(struct drm_tegra *drm, struct drm_tegra_channel *channel,
              struct vic **vicp);

/* from vic40.c */
int vic40_new(struct drm_tegra *drm, struct drm_tegra_channel *channel,
              struct vic **vicp);

/* from vic41.c */
int vic41_new(struct drm_tegra *drm, struct drm_tegra_channel *channel,
              struct vic **vicp);

/* from vic42.c */
int vic42_new(struct drm_tegra *drm, struct drm_tegra_channel *channel,
              struct vic **vicp);

int vic_new(struct drm_tegra *drm, struct drm_tegra_channel *channel,
            struct vic **vicp)
{
    unsigned int version;

    version = drm_tegra_channel_get_version(channel);

    switch (version) {
    case 0x40:
        return vic30_new(drm, channel, vicp);

    case 0x21:
        return vic40_new(drm, channel, vicp);

    case 0x18:
        return vic41_new(drm, channel, vicp);

    case 0x19:
        return vic42_new(drm, channel, vicp);
    }

    return -ENOTSUP;
}

void vic_free(struct vic *vic)
{
    if (vic)
        vic->ops->free(vic);
}

int vic_clear(struct vic *vic, struct vic_image *output, unsigned int alpha,
              unsigned int red, unsigned int green, unsigned int blue)
{
    return vic->ops->fill(vic, output, 0, 0, output->width - 1,
                          output->height - 1, alpha, red, green, blue);
}
