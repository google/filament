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

#include "util_math.h"
#include "private.h"

#define HOST1X_OPCODE_NONINCR(offset, count) \
    ((0x2 << 28) | (((offset) & 0xfff) << 16) | ((count) & 0xffff))

static inline unsigned int
drm_tegra_pushbuf_get_offset(struct drm_tegra_pushbuf *pushbuf, uint32_t *ptr)
{
    return ptr - pushbuf->start;
}

void drm_tegra_pushbuf_free(struct drm_tegra_pushbuf *pushbuf)
{
    if (pushbuf->start)
        free(pushbuf->start);

    free(pushbuf);
}

/**
 * drm_tegra_pushbuf_begin() - prepare push buffer for a series of pushes
 * @pushbuf: push buffer
 * @words: maximum number of words in series of pushes to follow
 */
drm_public int
drm_tegra_pushbuf_begin(struct drm_tegra_pushbuf *pushbuf,
                        unsigned int words, uint32_t **ptrp)
{
    struct drm_tegra_job *job = pushbuf->job;
    unsigned long offset;
    size_t size;
    void *ptr;

    if (pushbuf->ptr + words >= pushbuf->end) {
        words = pushbuf->end - pushbuf->start + words;
        size = ALIGN(words * 4, job->page_size);
        offset = pushbuf->ptr - pushbuf->start;

        ptr = realloc(pushbuf->start, size);
        if (!ptr)
            return -ENOMEM;

        pushbuf->start = ptr;
        pushbuf->end = pushbuf->start + size / 4;
        pushbuf->ptr = pushbuf->start + offset;
    }

    if (ptrp)
        *ptrp = pushbuf->ptr;

    return 0;
}

drm_public int
drm_tegra_pushbuf_end(struct drm_tegra_pushbuf *pushbuf, uint32_t *ptr)
{
    struct drm_tegra_submit_cmd *command;

    command = drm_tegra_job_add_command(pushbuf->job,
                                        DRM_TEGRA_SUBMIT_CMD_GATHER_UPTR,
                                        0);
    if (!command)
        return -ENOMEM;

    command->gather_uptr.words = ptr - pushbuf->start;
    pushbuf->ptr = ptr;

    return 0;
}

drm_public int
drm_tegra_pushbuf_wait(struct drm_tegra_pushbuf *pushbuf,
                       struct drm_tegra_syncpoint *syncpt,
                       uint32_t value)
{
    struct drm_tegra_submit_cmd *command;

    command = drm_tegra_job_add_command(pushbuf->job,
                                        DRM_TEGRA_SUBMIT_CMD_WAIT_SYNCPT,
                                        0);
    if (!command)
        return -ENOMEM;

    command->wait_syncpt.id = syncpt->id;
    command->wait_syncpt.value = value;

    return 0;
}

drm_public int
drm_tegra_pushbuf_relocate(struct drm_tegra_pushbuf *pushbuf, uint32_t **ptrp,
                           struct drm_tegra_mapping *target,
                           unsigned long offset, unsigned int shift,
                           uint32_t flags)
{
    struct drm_tegra_submit_buf *buffers, *buffer;
    struct drm_tegra_job *job = pushbuf->job;
    size_t size;

    size = (job->num_buffers + 1) * sizeof(*buffer);

    buffers = realloc(job->buffers, size);
    if (!buffers)
        return -ENOMEM;

    buffer = &buffers[job->num_buffers];

    memset(buffer, 0, sizeof(*buffer));
    buffer->mapping = target->id;
    buffer->flags = flags;
    buffer->reloc.target_offset = offset;
    buffer->reloc.gather_offset_words = drm_tegra_pushbuf_get_offset(pushbuf,
                                                                     *ptrp);
    buffer->reloc.shift = shift;

    *(*ptrp)++ = 0xdeadbeef;

    job->buffers = buffers;
    job->num_buffers++;

    return 0;
}

drm_public int
drm_tegra_pushbuf_sync(struct drm_tegra_pushbuf *pushbuf,
                       struct drm_tegra_syncpoint *syncpt,
                       unsigned int count)
{
    struct drm_tegra_job *job = pushbuf->job;

    job->syncpt.increments += count;
    job->syncpt.id = syncpt->id;

    return 0;
}

drm_public int
drm_tegra_pushbuf_sync_cond(struct drm_tegra_pushbuf *pushbuf, uint32_t **ptrp,
                            struct drm_tegra_syncpoint *syncpt,
                            enum drm_tegra_sync_cond cond)
{
    struct drm_tegra_channel *channel = pushbuf->job->channel;

    if (cond >= DRM_TEGRA_SYNC_COND_MAX)
        return -EINVAL;

    *(*ptrp)++ = HOST1X_OPCODE_NONINCR(0x0, 0x1);
    *(*ptrp)++ = cond << channel->cond_shift | syncpt->id;

    return drm_tegra_pushbuf_sync(pushbuf, syncpt, 1);
}
