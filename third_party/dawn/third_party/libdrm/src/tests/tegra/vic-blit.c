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

#include "host1x.h"
#include "vic.h"

/* clear output image to red */
static int clear(struct vic *vic, struct drm_tegra_channel *channel,
                 struct vic_image *output)
{
    struct drm_tegra_pushbuf *pushbuf;
    struct drm_tegra_job *job;
    uint32_t *ptr;
    int err;

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

    err = vic_clear(vic, output, 1023, 1023, 0, 0);
    if (err < 0) {
        fprintf(stderr, "failed to clear surface: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_pushbuf_begin(pushbuf, 32, &ptr);
    if (err < 0) {
        fprintf(stderr, "failed to prepare push buffer: %s\n", strerror(-err));
        return err;
    }

    err = vic->ops->execute(vic, pushbuf, &ptr, output, NULL, 0);
    if (err < 0) {
        fprintf(stderr, "failed to execute operation: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_pushbuf_sync_cond(pushbuf, &ptr, vic->syncpt,
                                      DRM_TEGRA_SYNC_COND_OP_DONE);
    if (err < 0) {
        fprintf(stderr, "failed to push syncpoint: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_pushbuf_end(pushbuf, ptr);
    if (err < 0) {
        fprintf(stderr, "failed to update push buffer: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_job_submit(job, NULL);
    if (err < 0) {
        fprintf(stderr, "failed to submit job: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_job_wait(job, 1000000000);
    if (err < 0) {
        fprintf(stderr, "failed to wait for job: %s\n", strerror(-err));
        return err;
    }

    drm_tegra_job_free(job);

    return 0;
}

/* fill bottom half of image to blue */
static int fill(struct vic *vic, struct drm_tegra_channel *channel,
                struct vic_image *output)
{
    struct drm_tegra_pushbuf *pushbuf;
    struct drm_tegra_job *job;
    uint32_t *ptr;
    int err;

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

    err = drm_tegra_pushbuf_begin(pushbuf, 32, &ptr);
    if (err < 0) {
        fprintf(stderr, "failed to prepare push buffer: %s\n", strerror(-err));
        return err;
    }

    err = vic->ops->fill(vic, output, 0, output->height / 2, output->width - 1,
                         output->height -1, 1023, 0, 0, 1023);
    if (err < 0) {
        fprintf(stderr, "failed to fill surface: %s\n", strerror(-err));
        return err;
    }

    err = vic->ops->execute(vic, pushbuf, &ptr, output, NULL, 0);
    if (err < 0) {
        fprintf(stderr, "failed to execute operation: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_pushbuf_sync_cond(pushbuf, &ptr, vic->syncpt,
                                      DRM_TEGRA_SYNC_COND_OP_DONE);
    if (err < 0) {
        fprintf(stderr, "failed to push syncpoint: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_pushbuf_end(pushbuf, ptr);
    if (err < 0) {
        fprintf(stderr, "failed to update push buffer: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_job_submit(job, NULL);
    if (err < 0) {
        fprintf(stderr, "failed to submit job: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_job_wait(job, 1000000000);
    if (err < 0) {
        fprintf(stderr, "failed to wait for job: %s\n", strerror(-err));
        return err;
    }

    drm_tegra_job_free(job);

    return 0;
}

/* blit image */
static int blit(struct vic *vic, struct drm_tegra_channel *channel,
                struct vic_image *output, struct vic_image *input)
{
    struct drm_tegra_pushbuf *pushbuf;
    struct drm_tegra_job *job;
    uint32_t *ptr;
    int err;

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

    err = drm_tegra_pushbuf_begin(pushbuf, 32, &ptr);
    if (err < 0) {
        fprintf(stderr, "failed to prepare push buffer: %s\n", strerror(-err));
        return err;
    }

    err = vic->ops->blit(vic, output, input);
    if (err < 0) {
        fprintf(stderr, "failed to blit surface: %s\n", strerror(-err));
        return err;
    }

    err = vic->ops->execute(vic, pushbuf, &ptr, output, &input, 1);
    if (err < 0) {
        fprintf(stderr, "failed to execute operation: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_pushbuf_sync_cond(pushbuf, &ptr, vic->syncpt,
                                      DRM_TEGRA_SYNC_COND_OP_DONE);
    if (err < 0) {
        fprintf(stderr, "failed to push syncpoint: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_pushbuf_end(pushbuf, ptr);
    if (err < 0) {
        fprintf(stderr, "failed to update push buffer: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_job_submit(job, NULL);
    if (err < 0) {
        fprintf(stderr, "failed to submit job: %s\n", strerror(-err));
        return err;
    }

    err = drm_tegra_job_wait(job, 1000000000);
    if (err < 0) {
        fprintf(stderr, "failed to wait for job: %s\n", strerror(-err));
        return err;
    }

    drm_tegra_job_free(job);

    return 0;
}

int main(int argc, char *argv[])
{
    const unsigned int format = VIC_PIXEL_FORMAT_A8R8G8B8;
    const unsigned int kind = VIC_BLK_KIND_PITCH;
    const unsigned int width = 16, height = 16;
    const char *device = "/dev/dri/renderD128";
    struct drm_tegra_channel *channel;
    struct vic_image *input, *output;
    struct drm_tegra *drm;
    unsigned int version;
    struct vic *vic;
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
                        &input);
    if (err < 0) {
        fprintf(stderr, "failed to create input image: %d\n", err);
        return 1;
    }

    err = vic_image_new(vic, width, height, format, kind, DRM_TEGRA_CHANNEL_MAP_READ_WRITE,
                        &output);
    if (err < 0) {
        fprintf(stderr, "failed to create output image: %d\n", err);
        return 1;
    }

    err = clear(vic, channel, input);
    if (err < 0) {
        fprintf(stderr, "failed to clear image: %s\n", strerror(-err));
        return 1;
    }

    err = fill(vic, channel, input);
    if (err < 0) {
        fprintf(stderr, "failed to fill rectangle: %s\n", strerror(-err));
        return 1;
    }

    err = blit(vic, channel, output, input);
    if (err < 0) {
        fprintf(stderr, "failed to blit image: %s\n", strerror(-err));
        return 1;
    }

    printf("input: %ux%u\n", input->width, input->height);
    vic_image_dump(input, stdout);

    printf("output: %ux%u\n", output->width, output->height);
    vic_image_dump(output, stdout);

    vic_image_free(output);
    vic_image_free(input);

    vic_free(vic);
    drm_tegra_channel_close(channel);
    drm_tegra_close(drm);
    close(fd);

    return 0;
}
