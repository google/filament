/*
 * Copyright © 2012, 2013 Thierry Reding
 * Copyright © 2013 Erik Faye-Lund
 * Copyright © 2014-2021 NVIDIA Corporation
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
drm_tegra_channel_open(struct drm_tegra *drm,
                       enum drm_tegra_class client,
                       struct drm_tegra_channel **channelp)
{
    struct drm_tegra_channel_open args;
    struct drm_tegra_channel *channel;
    enum host1x_class class;
    int err;

    switch (client) {
    case DRM_TEGRA_HOST1X:
        class = HOST1X_CLASS_HOST1X;
        break;

    case DRM_TEGRA_GR2D:
        class = HOST1X_CLASS_GR2D;
        break;

    case DRM_TEGRA_GR3D:
        class = HOST1X_CLASS_GR3D;
        break;

    case DRM_TEGRA_VIC:
        class = HOST1X_CLASS_VIC;
        break;

    default:
        return -EINVAL;
    }

    channel = calloc(1, sizeof(*channel));
    if (!channel)
        return -ENOMEM;

    channel->drm = drm;

    memset(&args, 0, sizeof(args));
    args.host1x_class = class;

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_CHANNEL_OPEN, &args);
    if (err < 0) {
        free(channel);
        return -errno;
    }

    channel->context = args.context;
    channel->version = args.version;
    channel->capabilities = args.capabilities;
    channel->class = class;

    switch (channel->version) {
    case 0x20:
    case 0x30:
    case 0x35:
    case 0x40:
    case 0x21:
        channel->cond_shift = 8;
        break;

    case 0x18:
    case 0x19:
        channel->cond_shift = 10;
        break;

    default:
        return -ENOTSUP;
    }

    *channelp = channel;

    return 0;
}

drm_public int drm_tegra_channel_close(struct drm_tegra_channel *channel)
{
    struct drm_tegra_channel_close args;
    struct drm_tegra *drm;
    int err;

    if (!channel)
        return -EINVAL;

    drm = channel->drm;

    memset(&args, 0, sizeof(args));
    args.context = channel->context;

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_CHANNEL_CLOSE, &args);
    if (err < 0)
        return -errno;

    free(channel);

    return 0;
}

drm_public unsigned int
drm_tegra_channel_get_version(struct drm_tegra_channel *channel)
{
    return channel->version;
}

drm_public int
drm_tegra_channel_map(struct drm_tegra_channel *channel,
                      struct drm_tegra_bo *bo, uint32_t flags,
                      struct drm_tegra_mapping **mapp)
{
    struct drm_tegra *drm = channel->drm;
    struct drm_tegra_channel_map args;
    struct drm_tegra_mapping *map;
    int err;

    if (!drm || !bo || !mapp)
        return -EINVAL;

    map = calloc(1, sizeof(*map));
    if (!map)
        return -ENOMEM;

    memset(&args, 0, sizeof(args));
    args.context = channel->context;
    args.handle = bo->handle;
    args.flags = flags;

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_CHANNEL_MAP, &args);
    if (err < 0) {
        free(map);
        return -errno;
    }

    map->channel = channel;
    map->id = args.mapping;
    *mapp = map;

    return 0;
}

drm_public int
drm_tegra_channel_unmap(struct drm_tegra_mapping *map)
{
    struct drm_tegra_channel *channel = map->channel;
    struct drm_tegra *drm = channel->drm;
    struct drm_tegra_channel_unmap args;
    int err;

    if (!channel || !map)
        return -EINVAL;

    memset(&args, 0, sizeof(args));
    args.context = channel->context;
    args.mapping = map->id;

    err = ioctl(drm->fd, DRM_IOCTL_TEGRA_CHANNEL_UNMAP, &args);
    if (err < 0)
        return -errno;

    free(map);
    return 0;
}
