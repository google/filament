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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tegra.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static int channel_open(struct drm_tegra *drm,
                        struct drm_tegra_channel **channel)
{
    static const struct {
        enum drm_tegra_class class;
        const char *name;
    } classes[] = {
        { DRM_TEGRA_VIC,  "VIC"  },
        { DRM_TEGRA_GR2D, "GR2D" },
    };
    unsigned int i;
    int err;

    for (i = 0; i < ARRAY_SIZE(classes); i++) {
        err = drm_tegra_channel_open(drm, classes[i].class, channel);
        if (err < 0) {
            fprintf(stderr, "failed to open channel to %s: %s\n",
                    classes[i].name, strerror(-err));
            continue;
        }

        break;
    }

    return err;
}

int main(int argc, char *argv[])
{
    const char *device = "/dev/dri/renderD128";
    struct drm_tegra_syncpoint *syncpt;
    struct drm_tegra_channel *channel;
    struct drm_tegra_pushbuf *pushbuf;
    struct drm_tegra_job *job;
    struct drm_tegra *drm;
    uint32_t *ptr;
    int fd, err;

    if (argc > 1)
        device = argv[1];

    fd = open(device, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open() failed: %s\n", strerror(errno));
        return 1;
    }

    err = drm_tegra_new(fd, &drm);
    if (err < 0) {
        fprintf(stderr, "failed to open Tegra device: %s\n", strerror(-err));
        close(fd);
        return 1;
    }

    err = drm_tegra_syncpoint_new(drm, &syncpt);
    if (err < 0) {
        fprintf(stderr, "failed to allocate syncpoint: %s\n", strerror(-err));
        drm_tegra_close(drm);
        close(fd);
        return 1;
    }

    err = channel_open(drm, &channel);
    if (err < 0) {
        fprintf(stderr, "failed to open channel: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_job_new(channel, &job);
    if (err < 0) {
        fprintf(stderr, "failed to create job: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_job_get_pushbuf(job, &pushbuf);
    if (err < 0) {
        fprintf(stderr, "failed to create push buffer: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_pushbuf_begin(pushbuf, 8, &ptr);
    if (err < 0) {
        fprintf(stderr, "failed to prepare push buffer: %s\n", strerror(-err));
        return 1;
    }

    /*
     * Empty command streams will be rejected, so we use this as an easy way
     * to add something to the command stream. But this could be any other,
     * valid command stream.
     */
    err = drm_tegra_pushbuf_sync_cond(pushbuf, &ptr, syncpt,
                                      DRM_TEGRA_SYNC_COND_IMMEDIATE);
    if (err < 0) {
        fprintf(stderr, "failed to push syncpoint: %s\n", strerror(-err));
        return 1;
    }

    /* pretend that the syncpoint was incremented a second time */
    err = drm_tegra_pushbuf_sync(pushbuf, syncpt, 1);
    if (err < 0) {
        fprintf(stderr, "failed to push syncpoint: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_pushbuf_end(pushbuf, ptr);
    if (err < 0) {
        fprintf(stderr, "failed to update push buffer: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_job_submit(job, NULL);
    if (err < 0) {
        fprintf(stderr, "failed to submit job: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_job_wait(job, 250000);
    if (err < 0) {
        fprintf(stderr, "failed to wait for job: %s\n", strerror(-err));
        return 1;
    }

    drm_tegra_job_free(job);
    drm_tegra_channel_close(channel);
    drm_tegra_syncpoint_free(syncpt);
    drm_tegra_close(drm);
    close(fd);

    return 0;
}
