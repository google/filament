/*
 * Copyright © 2014 NVIDIA Corporation
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <errno.h>
#include <stdio.h>

#include "drm-test-tegra.h"
#include "tegra.h"

int drm_tegra_gr2d_open(struct drm_tegra *drm, struct drm_tegra_gr2d **gr2dp)
{
    struct drm_tegra_gr2d *gr2d;
    int err;

    gr2d = calloc(1, sizeof(*gr2d));
    if (!gr2d)
        return -ENOMEM;

    gr2d->drm = drm;

    err = drm_tegra_channel_open(drm, DRM_TEGRA_GR2D, &gr2d->channel);
    if (err < 0) {
        free(gr2d);
        return err;
    }

    *gr2dp = gr2d;

    return 0;
}

int drm_tegra_gr2d_close(struct drm_tegra_gr2d *gr2d)
{
    if (!gr2d)
        return -EINVAL;

    drm_tegra_channel_close(gr2d->channel);
    free(gr2d);

    return 0;
}

int drm_tegra_gr2d_fill(struct drm_tegra_gr2d *gr2d, struct drm_framebuffer *fb,
                        unsigned int x, unsigned int y, unsigned int width,
                        unsigned int height, uint32_t color)
{
    struct drm_tegra_bo *fbo = fb->data;
    struct drm_tegra_pushbuf *pushbuf;
    struct drm_tegra_mapping *map;
    struct drm_tegra_job *job;
    uint32_t *ptr;
    int err;

    err = drm_tegra_job_new(gr2d->channel, &job);
    if (err < 0)
        return err;

    err = drm_tegra_channel_map(gr2d->channel, fbo, 0, &map);
    if (err < 0)
        return err;

    err = drm_tegra_job_get_pushbuf(job, &pushbuf);
    if (err < 0)
        return err;

    err = drm_tegra_pushbuf_begin(pushbuf, 32, &ptr);
    if (err < 0)
        return err;

    *ptr++ = HOST1X_OPCODE_SETCL(0, HOST1X_CLASS_GR2D, 0);

    *ptr++ = HOST1X_OPCODE_MASK(0x9, 0x9);
    *ptr++ = 0x0000003a;
    *ptr++ = 0x00000000;

    *ptr++ = HOST1X_OPCODE_MASK(0x1e, 0x7);
    *ptr++ = 0x00000000;
    *ptr++ = (2 << 16) | (1 << 6) | (1 << 2);
    *ptr++ = 0x000000cc;

    *ptr++ = HOST1X_OPCODE_MASK(0x2b, 0x9);

    /* relocate destination buffer */
    err = drm_tegra_pushbuf_relocate(pushbuf, &ptr, map, 0, 0, 0);
    if (err < 0) {
        fprintf(stderr, "failed to relocate buffer object: %d\n", err);
        return err;
    }

    *ptr++ = fb->pitch;

    *ptr++ = HOST1X_OPCODE_NONINCR(0x35, 1);
    *ptr++ = color;

    *ptr++ = HOST1X_OPCODE_NONINCR(0x46, 1);
    *ptr++ = 0x00000000;

    *ptr++ = HOST1X_OPCODE_MASK(0x38, 0x5);
    *ptr++ = height << 16 | width;
    *ptr++ = y << 16 | x;

    err = drm_tegra_pushbuf_end(pushbuf, ptr);
    if (err < 0) {
        fprintf(stderr, "failed to update push buffer: %d\n", -err);
        return err;
    }

    err = drm_tegra_job_submit(job, NULL);
    if (err < 0) {
        fprintf(stderr, "failed to submit job: %d\n", err);
        return err;
    }

    err = drm_tegra_job_wait(job, 0);
    if (err < 0) {
        fprintf(stderr, "failed to wait for fence: %d\n", err);
        return err;
    }

    drm_tegra_channel_unmap(map);
    drm_tegra_job_free(job);

    return 0;
}
