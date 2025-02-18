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
#include <string.h>

#include "private.h"
#include "tegra.h"
#include "vic.h"
#include "vic40.h"

struct vic40 {
    struct vic base;

    struct {
        struct drm_tegra_mapping *map;
        struct drm_tegra_bo *bo;
    } config;

    struct {
        struct drm_tegra_mapping *map;
        struct drm_tegra_bo *bo;
    } filter;
};

static int vic40_fill(struct vic *v, struct vic_image *output,
                      unsigned int left, unsigned int top,
                      unsigned int right, unsigned int bottom,
                      unsigned int alpha, unsigned int red,
                      unsigned int green, unsigned int blue)
{
    struct vic40 *vic = container_of(v, struct vic40, base);
    ConfigStruct *c;
    int err;

    err = drm_tegra_bo_map(vic->config.bo, (void **)&c);
    if (err < 0) {
        fprintf(stderr, "failed to map configuration structure: %s\n",
                strerror(-err));
        return err;
    }

    memset(c, 0, sizeof(*c));

    c->outputConfig.TargetRectTop = top;
    c->outputConfig.TargetRectLeft = left;
    c->outputConfig.TargetRectRight = right;
    c->outputConfig.TargetRectBottom = bottom;
    c->outputConfig.BackgroundAlpha = alpha;
    c->outputConfig.BackgroundR = red;
    c->outputConfig.BackgroundG = green;
    c->outputConfig.BackgroundB = blue;

    c->outputSurfaceConfig.OutPixelFormat = output->format;
    c->outputSurfaceConfig.OutBlkKind = output->kind;
    c->outputSurfaceConfig.OutBlkHeight = 0;
    c->outputSurfaceConfig.OutSurfaceWidth = output->width - 1;
    c->outputSurfaceConfig.OutSurfaceHeight = output->height - 1;
    c->outputSurfaceConfig.OutLumaWidth = output->stride - 1;
    c->outputSurfaceConfig.OutLumaHeight = output->height - 1;
    c->outputSurfaceConfig.OutChromaWidth = 16383;
    c->outputSurfaceConfig.OutChromaHeight = 16383;

    drm_tegra_bo_unmap(vic->config.bo);

    return 0;
}

static int vic40_blit(struct vic *v, struct vic_image *output,
                      struct vic_image *input)
{
    struct vic40 *vic = container_of(v, struct vic40, base);
    SlotSurfaceConfig *surface;
    SlotConfig *slot;
    ConfigStruct *c;
    int err;

    err = drm_tegra_bo_map(vic->config.bo, (void **)&c);
    if (err < 0) {
        fprintf(stderr, "failed to map configuration structure: %s\n",
                strerror(-err));
        return err;
    }

    memset(c, 0, sizeof(*c));

    c->outputConfig.TargetRectTop = 0;
    c->outputConfig.TargetRectLeft = 0;
    c->outputConfig.TargetRectRight = output->width - 1;
    c->outputConfig.TargetRectBottom = output->height - 1;
    c->outputConfig.BackgroundAlpha = 1023;
    c->outputConfig.BackgroundR = 1023;
    c->outputConfig.BackgroundG = 1023;
    c->outputConfig.BackgroundB = 1023;

    c->outputSurfaceConfig.OutPixelFormat = output->format;
    c->outputSurfaceConfig.OutBlkKind = output->kind;
    c->outputSurfaceConfig.OutBlkHeight = 0;
    c->outputSurfaceConfig.OutSurfaceWidth = output->width - 1;
    c->outputSurfaceConfig.OutSurfaceHeight = output->height - 1;
    c->outputSurfaceConfig.OutLumaWidth = output->stride - 1;
    c->outputSurfaceConfig.OutLumaHeight = output->height - 1;
    c->outputSurfaceConfig.OutChromaWidth = 16383;
    c->outputSurfaceConfig.OutChromaHeight = 16383;

    slot = &c->slotStruct[0].slotConfig;
    slot->SlotEnable = 1;
    slot->CurrentFieldEnable = 1;
    slot->PlanarAlpha = 1023;
    slot->ConstantAlpha = 1;
    slot->SourceRectLeft = 0 << 16;
    slot->SourceRectRight = (input->width - 1) << 16;
    slot->SourceRectTop = 0 << 16;
    slot->SourceRectBottom = (input->height - 1) << 16;
    slot->DestRectLeft = 0;
    slot->DestRectRight = output->width - 1;
    slot->DestRectTop = 0;
    slot->DestRectBottom = output->height - 1;
    slot->SoftClampHigh = 1023;

    surface = &c->slotStruct[0].slotSurfaceConfig;
    surface->SlotPixelFormat = input->format;
    surface->SlotBlkKind = input->kind;
    surface->SlotBlkHeight = 0; /* XXX */
    surface->SlotCacheWidth = VIC_CACHE_WIDTH_64Bx4; /* XXX */
    surface->SlotSurfaceWidth = input->width - 1;
    surface->SlotSurfaceHeight = input->height - 1;
    surface->SlotLumaWidth = input->stride - 1;
    surface->SlotLumaHeight = input->height - 1;
    surface->SlotChromaWidth = 16383;
    surface->SlotChromaHeight = 16383;

    drm_tegra_bo_unmap(vic->config.bo);

    return 0;
}

static int vic40_flip(struct vic *v, struct vic_image *output,
                      struct vic_image *input)
{
    struct vic40 *vic = container_of(v, struct vic40, base);
    SlotSurfaceConfig *surface;
    SlotConfig *slot;
    ConfigStruct *c;
    int err;

    err = drm_tegra_bo_map(vic->config.bo, (void **)&c);
    if (err < 0) {
        fprintf(stderr, "failed to map configuration structure: %s\n",
                strerror(-err));
        return err;
    }

    memset(c, 0, sizeof(*c));

    c->outputConfig.TargetRectTop = 0;
    c->outputConfig.TargetRectLeft = 0;
    c->outputConfig.TargetRectRight = output->width - 1;
    c->outputConfig.TargetRectBottom = output->height - 1;
    c->outputConfig.BackgroundAlpha = 1023;
    c->outputConfig.BackgroundR = 1023;
    c->outputConfig.BackgroundG = 1023;
    c->outputConfig.BackgroundB = 1023;
    c->outputConfig.OutputFlipY = 1;

    c->outputSurfaceConfig.OutPixelFormat = output->format;
    c->outputSurfaceConfig.OutBlkKind = output->kind;
    c->outputSurfaceConfig.OutBlkHeight = 0;
    c->outputSurfaceConfig.OutSurfaceWidth = output->width - 1;
    c->outputSurfaceConfig.OutSurfaceHeight = output->height - 1;
    c->outputSurfaceConfig.OutLumaWidth = output->stride - 1;
    c->outputSurfaceConfig.OutLumaHeight = output->height - 1;
    c->outputSurfaceConfig.OutChromaWidth = 16383;
    c->outputSurfaceConfig.OutChromaHeight = 16383;

    slot = &c->slotStruct[0].slotConfig;
    slot->SlotEnable = 1;
    slot->CurrentFieldEnable = 1;
    slot->PlanarAlpha = 1023;
    slot->ConstantAlpha = 1;
    slot->SourceRectLeft = 0 << 16;
    slot->SourceRectRight = (input->width - 1) << 16;
    slot->SourceRectTop = 0 << 16;
    slot->SourceRectBottom = (input->height - 1) << 16;
    slot->DestRectLeft = 0;
    slot->DestRectRight = output->width - 1;
    slot->DestRectTop = 0;
    slot->DestRectBottom = output->height - 1;
    slot->SoftClampHigh = 1023;

    surface = &c->slotStruct[0].slotSurfaceConfig;
    surface->SlotPixelFormat = input->format;
    surface->SlotBlkKind = input->kind;
    surface->SlotBlkHeight = 0; /* XXX */
    surface->SlotCacheWidth = VIC_CACHE_WIDTH_64Bx4; /* XXX */
    surface->SlotSurfaceWidth = input->width - 1;
    surface->SlotSurfaceHeight = input->height - 1;
    surface->SlotLumaWidth = input->stride - 1;
    surface->SlotLumaHeight = input->height - 1;
    surface->SlotChromaWidth = 16383;
    surface->SlotChromaHeight = 16383;

    drm_tegra_bo_unmap(vic->config.bo);

    return 0;
}

static int vic40_execute(struct vic *v, struct drm_tegra_pushbuf *pushbuf,
                         uint32_t **ptrp, struct vic_image *output,
                         struct vic_image **inputs, unsigned int num_inputs)
{
    struct vic40 *vic = container_of(v, struct vic40, base);
    unsigned int i;

    if (num_inputs > 1)
        return -EINVAL;

    VIC_PUSH_METHOD(pushbuf, ptrp, NVB0B6_VIDEO_COMPOSITOR_SET_APPLICATION_ID, 1);
    VIC_PUSH_METHOD(pushbuf, ptrp, NVB0B6_VIDEO_COMPOSITOR_SET_CONTROL_PARAMS, (sizeof(ConfigStruct) / 16) << 16);
    VIC_PUSH_BUFFER(pushbuf, ptrp, NVB0B6_VIDEO_COMPOSITOR_SET_CONFIG_STRUCT_OFFSET, vic->config.map, 0, 0);
    VIC_PUSH_BUFFER(pushbuf, ptrp, NVB0B6_VIDEO_COMPOSITOR_SET_OUTPUT_SURFACE_LUMA_OFFSET, output->map, 0, 0);

    for (i = 0; i < num_inputs; i++)
        VIC_PUSH_BUFFER(pushbuf, ptrp, NVB0B6_VIDEO_COMPOSITOR_SET_SURFACE0_SLOT0_LUMA_OFFSET, inputs[i]->map, 0, 0);

    VIC_PUSH_METHOD(pushbuf, ptrp, NVB0B6_VIDEO_COMPOSITOR_EXECUTE, 1 << 8);

    return 0;
}

static void vic40_free(struct vic *v)
{
    struct vic40 *vic = container_of(v, struct vic40, base);

    drm_tegra_channel_unmap(vic->filter.map);
    drm_tegra_bo_unref(vic->filter.bo);

    drm_tegra_channel_unmap(vic->config.map);
    drm_tegra_bo_unref(vic->config.bo);

    drm_tegra_syncpoint_free(v->syncpt);

    free(vic);
}

static const struct vic_ops vic40_ops = {
    .fill = vic40_fill,
    .blit = vic40_blit,
    .flip = vic40_flip,
    .execute = vic40_execute,
    .free = vic40_free,
};

int vic40_new(struct drm_tegra *drm, struct drm_tegra_channel *channel,
              struct vic **vicp)
{
    struct vic40 *vic;
    void *ptr;
    int err;

    vic = calloc(1, sizeof(*vic));
    if (!vic)
        return -ENOMEM;

    vic->base.drm = drm;
    vic->base.channel = channel;
    vic->base.ops = &vic40_ops;
    vic->base.version = 0x21;

    err = drm_tegra_syncpoint_new(drm, &vic->base.syncpt);
    if (err < 0) {
        fprintf(stderr, "failed to allocate syncpoint: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_bo_new(drm, 0, 16384, &vic->config.bo);
    if (err < 0) {
        fprintf(stderr, "failed to allocate configuration structurer: %s\n",
                strerror(-err));
        return err;
    }

    err = drm_tegra_channel_map(channel, vic->config.bo, DRM_TEGRA_CHANNEL_MAP_READ,
                                &vic->config.map);
    if (err < 0) {
        fprintf(stderr, "failed to map configuration structure: %s\n",
                strerror(-err));
        return err;
    }

    err = drm_tegra_bo_new(drm, 0, 16384, &vic->filter.bo);
    if (err < 0) {
        fprintf(stderr, "failed to allocate filter buffer: %s\n",
                strerror(-err));
        return err;
    }

    err = drm_tegra_bo_map(vic->filter.bo, &ptr);
    if (err < 0) {
        fprintf(stderr, "failed to map filter buffer: %s\n", strerror(-err));
        return err;
    }

    memset(ptr, 0, 16384);
    drm_tegra_bo_unmap(vic->filter.bo);

    err = drm_tegra_channel_map(channel, vic->filter.bo, DRM_TEGRA_CHANNEL_MAP_READ,
                                &vic->filter.map);
    if (err < 0) {
        fprintf(stderr, "failed to map filter buffer: %s\n",
                strerror(-err));
        return err;
    }

    if (vicp)
        *vicp = &vic->base;

    return 0;
}
