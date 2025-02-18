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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#include <xf86drm.h>

#include <tegra_drm.h>

#include "private.h"

static void drm_tegra_bo_free(struct drm_tegra_bo *bo)
{
    struct drm_tegra *drm = bo->drm;

    if (bo->map)
        munmap(bo->map, bo->size);

    drmCloseBufferHandle(drm->fd, bo->handle);

    free(bo);
}

static int drm_tegra_wrap(struct drm_tegra **drmp, int fd, bool close)
{
    struct drm_tegra *drm;

    if (fd < 0 || !drmp)
        return -EINVAL;

    drm = calloc(1, sizeof(*drm));
    if (!drm)
        return -ENOMEM;

    drm->close = close;
    drm->fd = fd;

    *drmp = drm;

    return 0;
}

drm_public int drm_tegra_new(int fd, struct drm_tegra **drmp)
{
    bool supported = false;
    drmVersionPtr version;

    version = drmGetVersion(fd);
    if (!version)
        return -ENOMEM;

    if (!strncmp(version->name, "tegra", version->name_len))
        supported = true;

    drmFreeVersion(version);

    if (!supported)
        return -ENOTSUP;

    return drm_tegra_wrap(drmp, fd, false);
}

drm_public void drm_tegra_close(struct drm_tegra *drm)
{
    if (!drm)
        return;

    if (drm->close)
        close(drm->fd);

    free(drm);
}

static struct drm_tegra_bo *drm_tegra_bo_alloc(struct drm_tegra *drm,
                                               uint32_t handle,
                                               uint32_t flags,
                                               uint32_t size)
{
    struct drm_tegra_bo *bo;

    bo = calloc(1, sizeof(*bo));
    if (!bo)
        return NULL;

    atomic_set(&bo->ref, 1);
    bo->handle = handle;
    bo->flags = flags;
    bo->size = size;
    bo->drm = drm;

    return bo;
}

drm_public int
drm_tegra_bo_new(struct drm_tegra *drm, uint32_t flags, uint32_t size,
                 struct drm_tegra_bo **bop)
{
    struct drm_tegra_gem_create args;
    struct drm_tegra_bo *bo;
    int err;

    if (!drm || size == 0 || !bop)
        return -EINVAL;

    bo = drm_tegra_bo_alloc(drm, 0, flags, size);
    if (!bo)
        return -ENOMEM;

    memset(&args, 0, sizeof(args));
    args.flags = flags;
    args.size = size;

    err = drmCommandWriteRead(drm->fd, DRM_TEGRA_GEM_CREATE, &args,
                              sizeof(args));
    if (err < 0) {
        err = -errno;
        free(bo);
        return err;
    }

    bo->handle = args.handle;

    *bop = bo;

    return 0;
}

drm_public int
drm_tegra_bo_wrap(struct drm_tegra *drm, uint32_t handle, uint32_t flags,
                  uint32_t size, struct drm_tegra_bo **bop)
{
    struct drm_tegra_bo *bo;

    if (!drm || !bop)
        return -EINVAL;

    bo = drm_tegra_bo_alloc(drm, handle, flags, size);
    if (!bo)
        return -ENOMEM;

    *bop = bo;

    return 0;
}

drm_public struct drm_tegra_bo *drm_tegra_bo_ref(struct drm_tegra_bo *bo)
{
    if (bo)
        atomic_inc(&bo->ref);

    return bo;
}

drm_public void drm_tegra_bo_unref(struct drm_tegra_bo *bo)
{
    if (bo && atomic_dec_and_test(&bo->ref))
        drm_tegra_bo_free(bo);
}

drm_public int
drm_tegra_bo_get_handle(struct drm_tegra_bo *bo, uint32_t *handle)
{
    if (!bo || !handle)
        return -EINVAL;

    *handle = bo->handle;

    return 0;
}

drm_public int drm_tegra_bo_map(struct drm_tegra_bo *bo, void **ptr)
{
    struct drm_tegra *drm = bo->drm;

    if (!bo->map) {
        struct drm_tegra_gem_mmap args;
        int err;

        memset(&args, 0, sizeof(args));
        args.handle = bo->handle;

        err = drmCommandWriteRead(drm->fd, DRM_TEGRA_GEM_MMAP, &args,
                                  sizeof(args));
        if (err < 0)
            return -errno;

        bo->offset = args.offset;

        bo->map = drm_mmap(NULL, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                           drm->fd, bo->offset);
        if (bo->map == MAP_FAILED) {
            bo->map = NULL;
            return -errno;
        }
    }

    if (ptr)
        *ptr = bo->map;

    return 0;
}

drm_public int drm_tegra_bo_unmap(struct drm_tegra_bo *bo)
{
    if (!bo)
        return -EINVAL;

    if (!bo->map)
        return 0;

    if (munmap(bo->map, bo->size))
        return -errno;

    bo->map = NULL;

    return 0;
}

drm_public int drm_tegra_bo_get_name(struct drm_tegra_bo *bo, uint32_t *name)
{
    struct drm_tegra *drm = bo->drm;
    struct drm_gem_flink args;
    int err;

    memset(&args, 0, sizeof(args));
    args.handle = bo->handle;

    err = drmIoctl(drm->fd, DRM_IOCTL_GEM_FLINK, &args);
    if (err < 0)
        return err;

    if (name)
        *name = args.name;

    return 0;
}

drm_public int
drm_tegra_bo_open(struct drm_tegra *drm, uint32_t name, uint32_t flags,
                  struct drm_tegra_bo **bop)
{
    struct drm_gem_open args;
    struct drm_tegra_bo *bo;
    int err;

    bo = drm_tegra_bo_alloc(drm, 0, flags, 0);
    if (!bo)
        return -ENOMEM;

    memset(&args, 0, sizeof(args));
    args.name = name;

    err = drmIoctl(drm->fd, DRM_IOCTL_GEM_OPEN, &args);
    if (err < 0)
        goto free;

    bo->handle = args.handle;
    bo->size = args.size;

    *bop = bo;

    return 0;

free:
    free(bo);
    return err;
}

drm_public int drm_tegra_bo_export(struct drm_tegra_bo *bo, uint32_t flags)
{
    int fd, err;

    flags |= DRM_CLOEXEC;

    err = drmPrimeHandleToFD(bo->drm->fd, bo->handle, flags, &fd);
    if (err < 0)
        return err;

    return fd;
}

static ssize_t fd_get_size(int fd)
{
    ssize_t size, offset;
    int err;

    offset = lseek(fd, 0, SEEK_CUR);
    if (offset < 0)
        return -errno;

    size = lseek(fd, 0, SEEK_END);
    if (size < 0)
        return -errno;

    err = lseek(fd, offset, SEEK_SET);
    if (err < 0)
        return -errno;

    return size;
}

drm_public int
drm_tegra_bo_import(struct drm_tegra *drm, int fd, struct drm_tegra_bo **bop)
{
    struct drm_tegra_bo *bo;
    ssize_t size;
    int err;

    size = fd_get_size(fd);
    if (size < 0)
        return size;

    bo = drm_tegra_bo_alloc(drm, 0, 0, size);
    if (!bo)
        return -ENOMEM;

    err = drmPrimeFDToHandle(drm->fd, fd, &bo->handle);
    if (err < 0)
        goto free;

    *bop = bo;

    return 0;

free:
    free(bo);
    return err;
}
