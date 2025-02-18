/*
 * Copyright © 2012, 2013 Thierry Reding
 * Copyright © 2013 Erik Faye-Lund
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <poll.h>

#include "private.h"

struct drm_tegra_submit_cmd *
drm_tegra_job_add_command(struct drm_tegra_job *job, uint32_t type,
                          uint32_t flags)
{
    struct drm_tegra_submit_cmd *commands, *command;
    size_t size;

    size = (job->num_commands + 1) * sizeof(*commands);

    commands = realloc(job->commands, size);
    if (!commands)
        return NULL;

    command = &commands[job->num_commands];
    memset(command, 0, sizeof(*command));
    command->type = type;
    command->flags = flags;

    job->commands = commands;
    job->num_commands++;

    return command;
}

drm_public int
drm_tegra_job_new(struct drm_tegra_channel *channel,
                  struct drm_tegra_job **jobp)
{
    struct drm_tegra_job *job;

    job = calloc(1, sizeof(*job));
    if (!job)
        return -ENOMEM;

    job->page_size = sysconf(_SC_PAGESIZE);
    job->channel = channel;

    *jobp = job;

    return 0;
}

drm_public int drm_tegra_job_free(struct drm_tegra_job *job)
{
    if (!job)
        return -EINVAL;

    if (job->pushbuf)
        drm_tegra_pushbuf_free(job->pushbuf);

    if (job->commands)
        free(job->commands);

    if (job->buffers)
        free(job->buffers);

    free(job);

    return 0;
}

drm_public int
drm_tegra_job_get_pushbuf(struct drm_tegra_job *job,
                          struct drm_tegra_pushbuf **pushbufp)
{
    struct drm_tegra_pushbuf *pushbuf;

    if (!job->pushbuf) {
        pushbuf = calloc(1, sizeof(*pushbuf));
        if (!pushbuf)
            return -ENOMEM;

        pushbuf->job = job;

        pushbuf->start = calloc(1, job->page_size);
        if (!pushbuf->start) {
            free(pushbuf);
            return -ENOMEM;
        }

        pushbuf->end = pushbuf->start + job->page_size / 4;
        pushbuf->ptr = pushbuf->start;

        job->pushbuf = pushbuf;
    }

    *pushbufp = job->pushbuf;

    return 0;
}

drm_public int
drm_tegra_job_submit(struct drm_tegra_job *job, struct drm_tegra_fence *fence)
{
    struct drm_tegra_channel *channel = job->channel;
    struct drm_tegra *drm = channel->drm;
    struct drm_tegra_channel_submit args;
    int err;

    memset(&args, 0, sizeof(args));
    args.context = channel->context;
    args.num_bufs = job->num_buffers;
    args.num_cmds = job->num_commands;
    args.gather_data_words = job->pushbuf->ptr - job->pushbuf->start;
    args.syncpt.id = job->syncpt.id;
    args.syncpt.increments = job->syncpt.increments;

    args.bufs_ptr = (uintptr_t)job->buffers;
    args.cmds_ptr = (uintptr_t)job->commands;
    args.gather_data_ptr = (uintptr_t)job->pushbuf->start;

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_CHANNEL_SUBMIT, &args);
    if (err < 0)
        return -errno;

    job->syncpt.fence = args.syncpt.value;

    if (fence) {
        fence->drm = drm;
        fence->syncpt = job->syncpt.id;
        fence->value = job->syncpt.fence;
    }

    return 0;
}

drm_public int
drm_tegra_job_wait(struct drm_tegra_job *job, unsigned long timeout)
{
    struct drm_tegra_channel *channel = job->channel;
    struct drm_tegra *drm = channel->drm;
    struct drm_tegra_syncpoint_wait args;
    struct timespec ts;
    int err;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    memset(&args, 0, sizeof(args));
    args.timeout_ns = ts.tv_sec * 1000000000 + ts.tv_nsec + timeout;
    args.id = job->syncpt.id;
    args.threshold = job->syncpt.fence;

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_SYNCPOINT_WAIT, &args);
    if (err < 0)
        return -errno;

    return 0;
}
