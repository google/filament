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

#include "util_math.h"

#include "tegra.h"

#include "host1x.h"
#include "vic.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

int main(int argc, char *argv[])
{
    const unsigned int format = VIC_PIXEL_FORMAT_A8R8G8B8;
    const unsigned int kind = VIC_BLK_KIND_PITCH;
    const unsigned int width = 16, height = 16;
    const char *device = "/dev/dri/renderD128";
    struct drm_tegra_channel *channel;
    struct drm_tegra_pushbuf *pushbuf;
    struct drm_tegra_job *job;
    struct vic_image *output;
    struct drm_tegra *drm;
    unsigned int version;
    struct vic *vic;
    uint32_t *pb;
    int fd, err;
    void *ptr;

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

    err = drm_tegra_channel_open(drm, DRM_TEGRA_VIC, &channel);
    if (err < 0) {
        fprintf(stderr, "failed to open channel to VIC: %s\n", strerror(-err));
        return 1;
    }

    version = drm_tegra_channel_get_version(channel);
    printf("version: %08x\n", version);

    err = vic_new(drm, channel, &vic);
    if (err < 0) {
        fprintf(stderr, "failed to create VIC: %s\n", strerror(-err));
        return 1;
    }

    err = vic_image_new(vic, width, height, format, kind, DRM_TEGRA_CHANNEL_MAP_READ_WRITE,
                        &output);
    if (err < 0) {
        fprintf(stderr, "failed to create output image: %d\n", err);
        return 1;
    }

    printf("image: %zu bytes\n", output->size);

    err = drm_tegra_bo_map(output->bo, &ptr);
    if (err < 0) {
        fprintf(stderr, "failed to map output image: %d\n", err);
        return 1;
    }

    memset(ptr, 0xff, output->size);
    drm_tegra_bo_unmap(output->bo);

    printf("output: %ux%u\n", output->width, output->height);
    vic_image_dump(output, stdout);

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

    err = drm_tegra_pushbuf_begin(pushbuf, 32, &pb);
    if (err < 0) {
        fprintf(stderr, "failed to prepare push buffer: %s\n", strerror(-err));
        return 1;
    }

    err = vic_clear(vic, output, 1023, 0, 0, 1023);
    if (err < 0) {
        fprintf(stderr, "failed to clear surface: %s\n", strerror(-err));
        return err;
    }

    err = vic->ops->execute(vic, pushbuf, &pb, output, NULL, 0);
    if (err < 0) {
        fprintf(stderr, "failed to execute operation: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_pushbuf_sync_cond(pushbuf, &pb, vic->syncpt,
                                      DRM_TEGRA_SYNC_COND_OP_DONE);
    if (err < 0) {
        fprintf(stderr, "failed to push syncpoint: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_pushbuf_end(pushbuf, pb);
    if (err < 0) {
        fprintf(stderr, "failed to update push buffer: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_job_submit(job, NULL);
    if (err < 0) {
        fprintf(stderr, "failed to submit job: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_job_wait(job, 1000000000);
    if (err < 0) {
        fprintf(stderr, "failed to wait for job: %s\n", strerror(-err));
        return 1;
    }

    printf("output: %ux%u\n", output->width, output->height);
    vic_image_dump(output, stdout);

    drm_tegra_job_free(job);
    vic_image_free(output);
    vic_free(vic);
    drm_tegra_channel_close(channel);
    drm_tegra_close(drm);
    close(fd);

    return 0;
}
