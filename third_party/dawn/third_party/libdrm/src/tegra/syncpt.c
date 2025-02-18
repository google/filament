/*
 * Copyright © 2021 NVIDIA Corporation
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
#include <string.h>

#include <sys/ioctl.h>

#include "private.h"

drm_public int
drm_tegra_syncpoint_new(struct drm_tegra *drm,
                        struct drm_tegra_syncpoint **syncptp)
{
    struct drm_tegra_syncpoint_allocate args;
    struct drm_tegra_syncpoint *syncpt;
    int err;

    syncpt = calloc(1, sizeof(*syncpt));
    if (!syncpt)
        return -ENOMEM;

    memset(&args, 0, sizeof(args));

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_SYNCPOINT_ALLOCATE, &args);
    if (err < 0) {
        free(syncpt);
        return -errno;
    }

    syncpt->drm = drm;
    syncpt->id = args.id;

    *syncptp = syncpt;

    return 0;
}

drm_public int
drm_tegra_syncpoint_free(struct drm_tegra_syncpoint *syncpt)
{
    struct drm_tegra_syncpoint_free args;
    struct drm_tegra *drm = syncpt->drm;
    int err;

    if (!syncpt)
        return -EINVAL;

    memset(&args, 0, sizeof(args));
    args.id = syncpt->id;

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_SYNCPOINT_FREE, &args);
    if (err < 0)
        return -errno;

    free(syncpt);

    return 0;
}

drm_public int
drm_tegra_fence_wait(struct drm_tegra_fence *fence, unsigned long timeout)
{
    struct drm_tegra_syncpoint_wait args;
    struct drm_tegra *drm = fence->drm;
    int err;

    memset(&args, 0, sizeof(args));
    args.timeout_ns = 0;
    args.id = fence->syncpt;
    args.threshold = fence->value;

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_SYNCPOINT_WAIT, &args);
    if (err < 0)
        return -errno;

    return 0;
}
