/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2011 Texas Instruments, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <rob@ti.com>
 */

#include <stdlib.h>
#include <linux/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <libdrm_macros.h>
#include <xf86drm.h>
#include <xf86atomic.h>

#include "omap_drm.h"
#include "omap_drmif.h"

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define PAGE_SIZE 4096

static pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
static void * dev_table;

struct omap_device {
	int fd;
	atomic_t refcnt;

	/* The handle_table is used to track GEM bo handles associated w/
	 * this fd.  This is needed, in particular, when importing
	 * dmabuf's because we don't want multiple 'struct omap_bo's
	 * floating around with the same handle.  Otherwise, when the
	 * first one is omap_bo_del()'d the handle becomes no longer
	 * valid, and the remaining 'struct omap_bo's are left pointing
	 * to an invalid handle (and possible a GEM bo that is already
	 * free'd).
	 */
	void *handle_table;
};

/* a GEM buffer object allocated from the DRM device */
struct omap_bo {
	struct omap_device	*dev;
	void		*map;		/* userspace mmap'ing (if there is one) */
	uint32_t	size;
	uint32_t	handle;
	uint32_t	name;		/* flink global handle (DRI2 name) */
	uint64_t	offset;		/* offset to mmap() */
	int		fd;		/* dmabuf handle */
	atomic_t	refcnt;
};

static struct omap_device * omap_device_new_impl(int fd)
{
	struct omap_device *dev = calloc(sizeof(*dev), 1);
	if (!dev)
		return NULL;
	dev->fd = fd;
	atomic_set(&dev->refcnt, 1);
	dev->handle_table = drmHashCreate();
	return dev;
}

drm_public struct omap_device * omap_device_new(int fd)
{
	struct omap_device *dev = NULL;

	pthread_mutex_lock(&table_lock);

	if (!dev_table)
		dev_table = drmHashCreate();

	if (drmHashLookup(dev_table, fd, (void **)&dev)) {
		/* not found, create new device */
		dev = omap_device_new_impl(fd);
		drmHashInsert(dev_table, fd, dev);
	} else {
		/* found, just incr refcnt */
		dev = omap_device_ref(dev);
	}

	pthread_mutex_unlock(&table_lock);

	return dev;
}

drm_public struct omap_device * omap_device_ref(struct omap_device *dev)
{
	atomic_inc(&dev->refcnt);
	return dev;
}

drm_public void omap_device_del(struct omap_device *dev)
{
	if (!atomic_dec_and_test(&dev->refcnt))
		return;
	pthread_mutex_lock(&table_lock);
	drmHashDestroy(dev->handle_table);
	drmHashDelete(dev_table, dev->fd);
	pthread_mutex_unlock(&table_lock);
	free(dev);
}

drm_public int
omap_get_param(struct omap_device *dev, uint64_t param, uint64_t *value)
{
	struct drm_omap_param req = {
			.param = param,
	};
	int ret;

	ret = drmCommandWriteRead(dev->fd, DRM_OMAP_GET_PARAM, &req, sizeof(req));
	if (ret) {
		return ret;
	}

	*value = req.value;

	return 0;
}

drm_public int
omap_set_param(struct omap_device *dev, uint64_t param, uint64_t value)
{
	struct drm_omap_param req = {
			.param = param,
			.value = value,
	};
	return drmCommandWrite(dev->fd, DRM_OMAP_SET_PARAM, &req, sizeof(req));
}

/* lookup a buffer from it's handle, call w/ table_lock held: */
static struct omap_bo * lookup_bo(struct omap_device *dev,
		uint32_t handle)
{
	struct omap_bo *bo = NULL;
	if (!drmHashLookup(dev->handle_table, handle, (void **)&bo)) {
		/* found, incr refcnt and return: */
		bo = omap_bo_ref(bo);
	}
	return bo;
}

/* allocate a new buffer object, call w/ table_lock held */
static struct omap_bo * bo_from_handle(struct omap_device *dev,
		uint32_t handle)
{
	struct omap_bo *bo = calloc(sizeof(*bo), 1);
	if (!bo) {
		drmCloseBufferHandle(dev->fd, handle);
		return NULL;
	}
	bo->dev = omap_device_ref(dev);
	bo->handle = handle;
	bo->fd = -1;
	atomic_set(&bo->refcnt, 1);
	/* add ourselves to the handle table: */
	drmHashInsert(dev->handle_table, handle, bo);
	return bo;
}

/* allocate a new buffer object */
static struct omap_bo * omap_bo_new_impl(struct omap_device *dev,
		union omap_gem_size size, uint32_t flags)
{
	struct omap_bo *bo = NULL;
	struct drm_omap_gem_new req = {
			.size = size,
			.flags = flags,
	};

	if (size.bytes == 0) {
		goto fail;
	}

	if (drmCommandWriteRead(dev->fd, DRM_OMAP_GEM_NEW, &req, sizeof(req))) {
		goto fail;
	}

	pthread_mutex_lock(&table_lock);
	bo = bo_from_handle(dev, req.handle);
	pthread_mutex_unlock(&table_lock);

	if (flags & OMAP_BO_TILED) {
		bo->size = round_up(size.tiled.width, PAGE_SIZE) * size.tiled.height;
	} else {
		bo->size = size.bytes;
	}

	return bo;

fail:
	free(bo);
	return NULL;
}


/* allocate a new (un-tiled) buffer object */
drm_public struct omap_bo *
omap_bo_new(struct omap_device *dev, uint32_t size, uint32_t flags)
{
	union omap_gem_size gsize = {
			.bytes = size,
	};
	if (flags & OMAP_BO_TILED) {
		return NULL;
	}
	return omap_bo_new_impl(dev, gsize, flags);
}

/* allocate a new buffer object */
drm_public struct omap_bo *
omap_bo_new_tiled(struct omap_device *dev, uint32_t width,
		  uint32_t height, uint32_t flags)
{
	union omap_gem_size gsize = {
			.tiled = {
				.width = width,
				.height = height,
			},
	};
	if (!(flags & OMAP_BO_TILED)) {
		return NULL;
	}
	return omap_bo_new_impl(dev, gsize, flags);
}

drm_public struct omap_bo *omap_bo_ref(struct omap_bo *bo)
{
	atomic_inc(&bo->refcnt);
	return bo;
}

/* get buffer info */
static int get_buffer_info(struct omap_bo *bo)
{
	struct drm_omap_gem_info req = {
			.handle = bo->handle,
	};
	int ret = drmCommandWriteRead(bo->dev->fd, DRM_OMAP_GEM_INFO,
			&req, sizeof(req));
	if (ret) {
		return ret;
	}

	/* really all we need for now is mmap offset */
	bo->offset = req.offset;
	bo->size = req.size;

	return 0;
}

/* import a buffer object from DRI2 name */
drm_public struct omap_bo *
omap_bo_from_name(struct omap_device *dev, uint32_t name)
{
	struct omap_bo *bo = NULL;
	struct drm_gem_open req = {
			.name = name,
	};

	pthread_mutex_lock(&table_lock);

	if (drmIoctl(dev->fd, DRM_IOCTL_GEM_OPEN, &req)) {
		goto fail;
	}

	bo = lookup_bo(dev, req.handle);
	if (!bo) {
		bo = bo_from_handle(dev, req.handle);
		bo->name = name;
	}

	pthread_mutex_unlock(&table_lock);

	return bo;

fail:
	pthread_mutex_unlock(&table_lock);
	free(bo);
	return NULL;
}

/* import a buffer from dmabuf fd, does not take ownership of the
 * fd so caller should close() the fd when it is otherwise done
 * with it (even if it is still using the 'struct omap_bo *')
 */
drm_public struct omap_bo *
omap_bo_from_dmabuf(struct omap_device *dev, int fd)
{
	struct omap_bo *bo = NULL;
	struct drm_prime_handle req = {
			.fd = fd,
	};
	int ret;

	pthread_mutex_lock(&table_lock);

	ret = drmIoctl(dev->fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &req);
	if (ret) {
		goto fail;
	}

	bo = lookup_bo(dev, req.handle);
	if (!bo) {
		bo = bo_from_handle(dev, req.handle);
	}

	pthread_mutex_unlock(&table_lock);

	return bo;

fail:
	pthread_mutex_unlock(&table_lock);
	free(bo);
	return NULL;
}

/* destroy a buffer object */
drm_public void omap_bo_del(struct omap_bo *bo)
{
	if (!bo) {
		return;
	}

	if (!atomic_dec_and_test(&bo->refcnt))
		return;

	if (bo->map) {
		munmap(bo->map, bo->size);
	}

	if (bo->fd >= 0) {
		close(bo->fd);
	}

	if (bo->handle) {
		pthread_mutex_lock(&table_lock);
		drmHashDelete(bo->dev->handle_table, bo->handle);
		drmCloseBufferHandle(bo->dev->fd, bo->handle);
		pthread_mutex_unlock(&table_lock);
	}

	omap_device_del(bo->dev);

	free(bo);
}

/* get the global flink/DRI2 buffer name */
drm_public int omap_bo_get_name(struct omap_bo *bo, uint32_t *name)
{
	if (!bo->name) {
		struct drm_gem_flink req = {
				.handle = bo->handle,
		};
		int ret;

		ret = drmIoctl(bo->dev->fd, DRM_IOCTL_GEM_FLINK, &req);
		if (ret) {
			return ret;
		}

		bo->name = req.name;
	}

	*name = bo->name;

	return 0;
}

drm_public uint32_t omap_bo_handle(struct omap_bo *bo)
{
	return bo->handle;
}

/* caller owns the dmabuf fd that is returned and is responsible
 * to close() it when done
 */
drm_public int omap_bo_dmabuf(struct omap_bo *bo)
{
	if (bo->fd < 0) {
		struct drm_prime_handle req = {
				.handle = bo->handle,
				.flags = DRM_CLOEXEC | DRM_RDWR,
		};
		int ret;

		ret = drmIoctl(bo->dev->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &req);
		if (ret) {
			return ret;
		}

		bo->fd = req.fd;
	}
	return dup(bo->fd);
}

drm_public uint32_t omap_bo_size(struct omap_bo *bo)
{
	if (!bo->size) {
		get_buffer_info(bo);
	}
	return bo->size;
}

drm_public void *omap_bo_map(struct omap_bo *bo)
{
	if (!bo->map) {
		if (!bo->offset) {
			get_buffer_info(bo);
		}

		bo->map = mmap(0, bo->size, PROT_READ | PROT_WRITE,
				MAP_SHARED, bo->dev->fd, bo->offset);
		if (bo->map == MAP_FAILED) {
			bo->map = NULL;
		}
	}
	return bo->map;
}

drm_public int omap_bo_cpu_prep(struct omap_bo *bo, enum omap_gem_op op)
{
	struct drm_omap_gem_cpu_prep req = {
			.handle = bo->handle,
			.op = op,
	};
	return drmCommandWrite(bo->dev->fd,
			DRM_OMAP_GEM_CPU_PREP, &req, sizeof(req));
}

drm_public int omap_bo_cpu_fini(struct omap_bo *bo, enum omap_gem_op op)
{
	struct drm_omap_gem_cpu_fini req = {
			.handle = bo->handle,
			.op = op,
			.nregions = 0,
	};
	return drmCommandWrite(bo->dev->fd,
			DRM_OMAP_GEM_CPU_FINI, &req, sizeof(req));
}
