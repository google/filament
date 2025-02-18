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

#ifndef __DRM_TEGRA_PRIVATE_H__
#define __DRM_TEGRA_PRIVATE_H__ 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libdrm_macros.h>
#include <xf86atomic.h>

#include "tegra_drm.h"
#include "tegra.h"

#define container_of(ptr, type, member) ({                      \
        const __typeof__(((type *)0)->member) *__mptr = (ptr);  \
        (type *)((char *)__mptr - offsetof(type, member));      \
    })

enum host1x_class {
    HOST1X_CLASS_HOST1X = 0x01,
    HOST1X_CLASS_GR2D = 0x51,
    HOST1X_CLASS_GR2D_SB = 0x52,
    HOST1X_CLASS_VIC = 0x5d,
    HOST1X_CLASS_GR3D = 0x60,
};

struct drm_tegra {
    bool close;
    int fd;
};

struct drm_tegra_bo {
    struct drm_tegra *drm;
    uint32_t handle;
    uint64_t offset;
    uint32_t flags;
    uint32_t size;
    atomic_t ref;
    void *map;
};

struct drm_tegra_channel {
    struct drm_tegra *drm;
    enum host1x_class class;
    uint32_t capabilities;
    unsigned int version;
    uint64_t context;

    unsigned int cond_shift;
};

struct drm_tegra_mapping {
    struct drm_tegra_channel *channel;
    uint32_t id;
};

struct drm_tegra_pushbuf {
    struct drm_tegra_job *job;

    uint32_t *start;
    uint32_t *end;
    uint32_t *ptr;
};

void drm_tegra_pushbuf_free(struct drm_tegra_pushbuf *pushbuf);

struct drm_tegra_job {
    struct drm_tegra_channel *channel;
    struct drm_tegra_pushbuf *pushbuf;
    size_t page_size;

    struct drm_tegra_submit_cmd *commands;
    unsigned int num_commands;

    struct drm_tegra_submit_buf *buffers;
    unsigned int num_buffers;

    struct {
        uint32_t id;
        uint32_t increments;
        uint32_t fence;
    } syncpt;
};

struct drm_tegra_submit_cmd *
drm_tegra_job_add_command(struct drm_tegra_job *job, uint32_t type,
                          uint32_t flags);

struct drm_tegra_syncpoint {
    struct drm_tegra *drm;
    uint32_t id;
};

#endif /* __DRM_TEGRA_PRIVATE_H__ */
