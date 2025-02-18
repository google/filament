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
#include "vic30.h"

struct vic30 {
    struct vic base;

    struct {
        struct drm_tegra_mapping *map;
        struct drm_tegra_bo *bo;
    } config;

    struct {
        struct drm_tegra_mapping *map;
        struct drm_tegra_bo *bo;
    } filter;

    struct {
        struct drm_tegra_mapping *map;
        struct drm_tegra_bo *bo;
    } hist;
};

static int vic30_fill(struct vic *v, struct vic_image *output,
                      unsigned int left, unsigned int top,
                      unsigned int right, unsigned int bottom,
                      unsigned int alpha, unsigned int red,
                      unsigned int green, unsigned int blue)
{
    struct vic30 *vic = container_of(v, struct vic30, base);
    ConfigStruct *c;
    int err;

    err = drm_tegra_bo_map(vic->config.bo, (void **)&c);
    if (err < 0) {
        fprintf(stderr, "failed to map configuration structure: %s\n",
                strerror(-err));
        return err;
    }

    memset(c, 0, sizeof(*c));

    c->surfaceList0Struct.TargetRectLeft = left;
    c->surfaceList0Struct.TargetRectTop = top;
    c->surfaceList0Struct.TargetRectRight = right;
    c->surfaceList0Struct.TargetRectBottom = bottom;

    c->blending0Struct.PixelFormat = output->format;
    c->blending0Struct.BackgroundAlpha = alpha;
    c->blending0Struct.BackgroundR = red;
    c->blending0Struct.BackgroundG = green;
    c->blending0Struct.BackgroundB = blue;
    c->blending0Struct.LumaWidth = output->stride - 1;
    c->blending0Struct.LumaHeight = output->height - 1;
    c->blending0Struct.ChromaWidth = 16383;
    c->blending0Struct.ChromaWidth = 16383;
    c->blending0Struct.TargetRectLeft = left;
    c->blending0Struct.TargetRectTop = top;
    c->blending0Struct.TargetRectRight = right;
    c->blending0Struct.TargetRectBottom = bottom;
    c->blending0Struct.SurfaceWidth = output->width - 1;
    c->blending0Struct.SurfaceHeight = output->height - 1;
    c->blending0Struct.BlkKind = output->kind;
    c->blending0Struct.BlkHeight = 0;

    c->fetchControl0Struct.TargetRectLeft = left;
    c->fetchControl0Struct.TargetRectTop = top;
    c->fetchControl0Struct.TargetRectRight = right;
    c->fetchControl0Struct.TargetRectBottom = bottom;

    drm_tegra_bo_unmap(vic->config.bo);

    return 0;
}

static int vic30_blit(struct vic *v, struct vic_image *output,
                      struct vic_image *input)
{
    struct vic30 *vic = container_of(v, struct vic30, base);
    ColorConversionLumaAlphaStruct *ccla;
    ColorConversionMatrixStruct *ccm;
    ColorConversionClampStruct *ccc;
    SurfaceListSurfaceStruct *s;
    BlendingSurfaceStruct *b;
    SurfaceCache0Struct *sc;
    ConfigStruct *c;
    int err;

    err = drm_tegra_bo_map(vic->config.bo, (void **)&c);
    if (err < 0) {
        fprintf(stderr, "failed to map configuration structure: %s\n",
                strerror(-err));
        return err;
    }

    memset(c, 0, sizeof(*c));

    c->surfaceList0Struct.TargetRectLeft = 0;
    c->surfaceList0Struct.TargetRectTop = 0;
    c->surfaceList0Struct.TargetRectRight = output->width - 1;
    c->surfaceList0Struct.TargetRectBottom = output->height - 1;

    c->blending0Struct.PixelFormat = output->format;
    c->blending0Struct.BackgroundAlpha = 0;
    c->blending0Struct.BackgroundR = 0;
    c->blending0Struct.BackgroundG = 0;
    c->blending0Struct.BackgroundB = 0;
    c->blending0Struct.LumaWidth = output->stride - 1;
    c->blending0Struct.LumaHeight = output->height - 1;
    c->blending0Struct.ChromaWidth = 16383;
    c->blending0Struct.ChromaWidth = 16383;
    c->blending0Struct.TargetRectLeft = 0;
    c->blending0Struct.TargetRectTop = 0;
    c->blending0Struct.TargetRectRight = output->width - 1;
    c->blending0Struct.TargetRectBottom = output->height - 1;
    c->blending0Struct.SurfaceWidth = output->width - 1;
    c->blending0Struct.SurfaceHeight = output->height - 1;
    c->blending0Struct.BlkKind = output->kind;
    c->blending0Struct.BlkHeight = 0;

    c->fetchControl0Struct.TargetRectLeft = 0;
    c->fetchControl0Struct.TargetRectTop = 0;
    c->fetchControl0Struct.TargetRectRight = output->width - 1;
    c->fetchControl0Struct.TargetRectBottom = output->height - 1;

    /* setup fetch parameters for slot 0 */
    c->fetchControl0Struct.Enable0 = 0x1;
    c->fetchControl0Struct.Iir0 = 0x300;

    /* setup cache parameters for slot 0 */
    sc = &c->surfaceCache0Struct;
    sc->PixelFormat0 = input->format;

    /* setup surface configuration for slot 0 */
    s = &c->surfaceListSurfaceStruct[0];
    s->Enable = 1;
    s->FrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
    s->PixelFormat = input->format;
    s->SurfaceWidth = input->width - 1;
    s->SurfaceHeight = input->height - 1;
    s->LumaWidth = input->stride - 1;
    s->LumaHeight = input->height - 1;
    s->ChromaWidth = 16383;
    s->ChromaHeight = 16383;
    s->CacheWidth = VIC_CACHE_WIDTH_256Bx1; //VIC_CACHE_WIDTH_16Bx16;
    s->BlkKind = input->kind;
    s->BlkHeight = 0;
    s->DestRectLeft = 0;
    s->DestRectTop = 0;
    s->DestRectRight = output->width - 1;
    s->DestRectBottom = output->height - 1;
    s->SourceRectLeft = 0 << 16;
    s->SourceRectTop = 0 << 16;
    s->SourceRectRight = (input->width - 1) << 16;
    s->SourceRectBottom = (input->height - 1) << 16;

    /* setup color conversion for slot 0 */
    ccla = &c->colorConversionLumaAlphaStruct[0];
    ccla->PlanarAlpha = 1023;
    ccla->ConstantAlpha = 0;

    ccm = &c->colorConversionMatrixStruct[0];
    ccm->c00 = 1023;
    ccm->c11 = 1023;
    ccm->c22 = 1023;

    ccc = &c->colorConversionClampStruct[0];
    ccc->low = 0;
    ccc->high = 1023;

    /* setup blending for slot 0 */
    b = &c->blendingSurfaceStruct[0];
    b->AlphaK1 = 1023;
    b->SrcFactCMatchSelect = VIC_BLEND_SRCFACTC_K1;
    b->SrcFactAMatchSelect = VIC_BLEND_SRCFACTA_K1;
    b->DstFactCMatchSelect = VIC_BLEND_DSTFACTC_NEG_K1_TIMES_SRC;
    b->DstFactAMatchSelect = VIC_BLEND_DSTFACTA_NEG_K1_TIMES_SRC;

    drm_tegra_bo_unmap(vic->config.bo);

    return 0;
}

static int vic30_flip(struct vic *v, struct vic_image *output,
                      struct vic_image *input)
{
    struct vic30 *vic = container_of(v, struct vic30, base);
    ColorConversionLumaAlphaStruct *ccla;
    ColorConversionMatrixStruct *ccm;
    ColorConversionClampStruct *ccc;
    SurfaceListSurfaceStruct *s;
    BlendingSurfaceStruct *b;
    SurfaceCache0Struct *sc;
    ConfigStruct *c;
    int err;

    err = drm_tegra_bo_map(vic->config.bo, (void **)&c);
    if (err < 0) {
        fprintf(stderr, "failed to map configuration structure: %s\n",
                strerror(-err));
        return err;
    }

    memset(c, 0, sizeof(*c));

    c->surfaceList0Struct.TargetRectLeft = 0;
    c->surfaceList0Struct.TargetRectTop = 0;
    c->surfaceList0Struct.TargetRectRight = output->width - 1;
    c->surfaceList0Struct.TargetRectBottom = output->height - 1;

    c->blending0Struct.PixelFormat = output->format;
    c->blending0Struct.BackgroundAlpha = 0;
    c->blending0Struct.BackgroundR = 0;
    c->blending0Struct.BackgroundG = 0;
    c->blending0Struct.BackgroundB = 0;
    c->blending0Struct.LumaWidth = output->stride - 1;
    c->blending0Struct.LumaHeight = output->height - 1;
    c->blending0Struct.ChromaWidth = 16383;
    c->blending0Struct.ChromaWidth = 16383;
    c->blending0Struct.TargetRectLeft = 0;
    c->blending0Struct.TargetRectTop = 0;
    c->blending0Struct.TargetRectRight = output->width - 1;
    c->blending0Struct.TargetRectBottom = output->height - 1;
    c->blending0Struct.SurfaceWidth = output->width - 1;
    c->blending0Struct.SurfaceHeight = output->height - 1;
    c->blending0Struct.BlkKind = output->kind;
    c->blending0Struct.BlkHeight = 0;
    c->blending0Struct.OutputFlipY = 1;

    c->fetchControl0Struct.TargetRectLeft = 0;
    c->fetchControl0Struct.TargetRectTop = 0;
    c->fetchControl0Struct.TargetRectRight = output->width - 1;
    c->fetchControl0Struct.TargetRectBottom = output->height - 1;

    /* setup fetch parameters for slot 0 */
    c->fetchControl0Struct.Enable0 = 0x1;
    c->fetchControl0Struct.Iir0 = 0x300;

    /* setup cache parameters for slot 0 */
    sc = &c->surfaceCache0Struct;
    sc->PixelFormat0 = input->format;

    /* setup surface configuration for slot 0 */
    s = &c->surfaceListSurfaceStruct[0];
    s->Enable = 1;
    s->FrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
    s->PixelFormat = input->format;
    s->SurfaceWidth = input->width - 1;
    s->SurfaceHeight = input->height - 1;
    s->LumaWidth = input->stride - 1;
    s->LumaHeight = input->height - 1;
    s->ChromaWidth = 16383;
    s->ChromaHeight = 16383;
    s->CacheWidth = VIC_CACHE_WIDTH_256Bx1;
    s->BlkKind = input->kind;
    s->BlkHeight = 0;
    s->DestRectLeft = 0;
    s->DestRectTop = 0;
    s->DestRectRight = output->width - 1;
    s->DestRectBottom = output->height - 1;
    s->SourceRectLeft = 0 << 16;
    s->SourceRectTop = 0 << 16;
    s->SourceRectRight = (input->width - 1) << 16;
    s->SourceRectBottom = (input->height - 1) << 16;

    /* setup color conversion for slot 0 */
    ccla = &c->colorConversionLumaAlphaStruct[0];
    ccla->PlanarAlpha = 1023;
    ccla->ConstantAlpha = 0;

    ccm = &c->colorConversionMatrixStruct[0];
    ccm->c00 = 1023;
    ccm->c11 = 1023;
    ccm->c22 = 1023;

    ccc = &c->colorConversionClampStruct[0];
    ccc->low = 0;
    ccc->high = 1023;

    /* setup blending for slot 0 */
    b = &c->blendingSurfaceStruct[0];
    b->AlphaK1 = 1023;
    b->SrcFactCMatchSelect = VIC_BLEND_SRCFACTC_K1;
    b->SrcFactAMatchSelect = VIC_BLEND_SRCFACTA_K1;
    b->DstFactCMatchSelect = VIC_BLEND_DSTFACTC_NEG_K1_TIMES_SRC;
    b->DstFactAMatchSelect = VIC_BLEND_DSTFACTA_NEG_K1_TIMES_SRC;

    drm_tegra_bo_unmap(vic->config.bo);

    return 0;
}

static int vic30_execute(struct vic *v, struct drm_tegra_pushbuf *pushbuf,
                         uint32_t **ptrp, struct vic_image *output,
                         struct vic_image **inputs, unsigned int num_inputs)
{
    struct vic30 *vic = container_of(v, struct vic30, base);
    unsigned int i;

    if (num_inputs > 1)
        return -EINVAL;

    VIC_PUSH_METHOD(pushbuf, ptrp, NVA0B6_VIDEO_COMPOSITOR_SET_APPLICATION_ID, 1);
    VIC_PUSH_METHOD(pushbuf, ptrp, NVA0B6_VIDEO_COMPOSITOR_SET_CONTROL_PARAMS, (sizeof(ConfigStruct) / 16) << 16);
    VIC_PUSH_BUFFER(pushbuf, ptrp, NVA0B6_VIDEO_COMPOSITOR_SET_CONFIG_STRUCT_OFFSET, vic->config.map, 0, 0);
    VIC_PUSH_BUFFER(pushbuf, ptrp, NVA0B6_VIDEO_COMPOSITOR_SET_HIST_OFFSET, vic->hist.map, 0, 0);
    VIC_PUSH_BUFFER(pushbuf, ptrp, NVA0B6_VIDEO_COMPOSITOR_SET_OUTPUT_SURFACE_LUMA_OFFSET, output->map, 0, 0);

    for (i = 0; i < num_inputs; i++)
        VIC_PUSH_BUFFER(pushbuf, ptrp, NVA0B6_VIDEO_COMPOSITOR_SET_SURFACE0_SLOT0_LUMA_OFFSET, inputs[i]->map, 0, 0);

    VIC_PUSH_METHOD(pushbuf, ptrp, NVA0B6_VIDEO_COMPOSITOR_EXECUTE, 1 << 8);

    return 0;
}

static void vic30_free(struct vic *v)
{
    struct vic30 *vic = container_of(v, struct vic30, base);

    drm_tegra_channel_unmap(vic->hist.map);
    drm_tegra_bo_unref(vic->hist.bo);

    drm_tegra_channel_unmap(vic->filter.map);
    drm_tegra_bo_unref(vic->filter.bo);

    drm_tegra_channel_unmap(vic->config.map);
    drm_tegra_bo_unref(vic->config.bo);

    drm_tegra_syncpoint_free(v->syncpt);

    free(vic);
}

static const struct vic_ops vic30_ops = {
    .fill = vic30_fill,
    .blit = vic30_blit,
    .flip = vic30_flip,
    .execute = vic30_execute,
    .free = vic30_free,
};

int vic30_new(struct drm_tegra *drm, struct drm_tegra_channel *channel,
              struct vic **vicp)
{
    struct vic30 *vic;
    void *ptr;
    int err;

    vic = calloc(1, sizeof(*vic));
    if (!vic)
        return -ENOMEM;

    vic->base.drm = drm;
    vic->base.channel = channel;
    vic->base.ops = &vic30_ops;
    vic->base.version = 0x40;

    err = drm_tegra_syncpoint_new(drm, &vic->base.syncpt);
    if (err < 0) {
        fprintf(stderr, "failed to allocate syncpoint: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_bo_new(drm, 0, 16384, &vic->config.bo);
    if (err < 0) {
        fprintf(stderr, "failed to allocate configuration structure: %s\n",
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

    err = drm_tegra_bo_new(drm, 0, 4096, &vic->hist.bo);
    if (err < 0) {
        fprintf(stderr, "failed to allocate history buffer: %s\n",
                strerror(-err));
        return err;
    }

    err = drm_tegra_bo_map(vic->hist.bo, &ptr);
    if (err < 0) {
        fprintf(stderr, "failed to map history buffer: %s\n", strerror(-err));
        return err;
    }

    memset(ptr, 0, 4096);
    drm_tegra_bo_unmap(vic->hist.bo);

    err = drm_tegra_channel_map(channel, vic->hist.bo, DRM_TEGRA_CHANNEL_MAP_READ_WRITE,
                                &vic->hist.map);
    if (err < 0) {
        fprintf(stderr, "failed to map histogram buffer: %s\n",
                strerror(-err));
        return err;
    }

    if (vicp)
        *vicp = &vic->base;

    return 0;
}
