/*
 * Copyright (C) 2014 Etnaviv Project
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
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "etnaviv_priv.h"
#include "etnaviv_drmif.h"

drm_private pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
drm_private void bo_del(struct etna_bo *bo);

/* set buffer name, and add to table, call w/ table_lock held: */
static void set_name(struct etna_bo *bo, uint32_t name)
{
	bo->name = name;
	/* add ourself into the name table: */
	drmHashInsert(bo->dev->name_table, name, bo);
}

/* Called under table_lock */
drm_private void bo_del(struct etna_bo *bo)
{
	if (bo->map)
		drm_munmap(bo->map, bo->size);

	if (bo->name)
		drmHashDelete(bo->dev->name_table, bo->name);

	if (bo->handle) {
		drmHashDelete(bo->dev->handle_table, bo->handle);
		drmCloseBufferHandle(bo->dev->fd, bo->handle);
	}

	free(bo);
}

/* lookup a buffer from it's handle, call w/ table_lock held: */
static struct etna_bo *lookup_bo(void *tbl, uint32_t handle)
{
	struct etna_bo *bo = NULL;

	if (!drmHashLookup(tbl, handle, (void **)&bo)) {
		/* found, incr refcnt and return: */
		bo = etna_bo_ref(bo);

		/* don't break the bucket if this bo was found in one */
		list_delinit(&bo->list);
	}

	return bo;
}

/* allocate a new buffer object, call w/ table_lock held */
static struct etna_bo *bo_from_handle(struct etna_device *dev,
		uint32_t size, uint32_t handle, uint32_t flags)
{
	struct etna_bo *bo = calloc(sizeof(*bo), 1);

	if (!bo) {
		drmCloseBufferHandle(dev->fd, handle);
		return NULL;
	}

	bo->dev = etna_device_ref(dev);
	bo->size = size;
	bo->handle = handle;
	bo->flags = flags;
	atomic_set(&bo->refcnt, 1);
	list_inithead(&bo->list);
	/* add ourselves to the handle table: */
	drmHashInsert(dev->handle_table, handle, bo);

	return bo;
}

/* allocate a new (un-tiled) buffer object */
drm_public struct etna_bo *etna_bo_new(struct etna_device *dev, uint32_t size,
		uint32_t flags)
{
	struct etna_bo *bo;
	int ret;
	struct drm_etnaviv_gem_new req = {
			.flags = flags,
	};

	bo = etna_bo_cache_alloc(&dev->bo_cache, &size, flags);
	if (bo)
		return bo;

	req.size = size;
	ret = drmCommandWriteRead(dev->fd, DRM_ETNAVIV_GEM_NEW,
			&req, sizeof(req));
	if (ret)
		return NULL;

	pthread_mutex_lock(&table_lock);
	bo = bo_from_handle(dev, size, req.handle, flags);
	bo->reuse = 1;
	pthread_mutex_unlock(&table_lock);

	return bo;
}

drm_public struct etna_bo *etna_bo_ref(struct etna_bo *bo)
{
	atomic_inc(&bo->refcnt);

	return bo;
}

/* get buffer info */
static int get_buffer_info(struct etna_bo *bo)
{
	int ret;
	struct drm_etnaviv_gem_info req = {
		.handle = bo->handle,
	};

	ret = drmCommandWriteRead(bo->dev->fd, DRM_ETNAVIV_GEM_INFO,
			&req, sizeof(req));
	if (ret) {
		return ret;
	}

	/* really all we need for now is mmap offset */
	bo->offset = req.offset;

	return 0;
}

/* import a buffer object from DRI2 name */
drm_public struct etna_bo *etna_bo_from_name(struct etna_device *dev,
		uint32_t name)
{
	struct etna_bo *bo;
	struct drm_gem_open req = {
		.name = name,
	};

	pthread_mutex_lock(&table_lock);

	/* check name table first, to see if bo is already open: */
	bo = lookup_bo(dev->name_table, name);
	if (bo)
		goto out_unlock;

	if (drmIoctl(dev->fd, DRM_IOCTL_GEM_OPEN, &req)) {
		ERROR_MSG("gem-open failed: %s", strerror(errno));
		goto out_unlock;
	}

	bo = lookup_bo(dev->handle_table, req.handle);
	if (bo)
		goto out_unlock;

	bo = bo_from_handle(dev, req.size, req.handle, 0);
	if (bo)
		set_name(bo, name);

out_unlock:
	pthread_mutex_unlock(&table_lock);

	return bo;
}

/* import a buffer from dmabuf fd, does not take ownership of the
 * fd so caller should close() the fd when it is otherwise done
 * with it (even if it is still using the 'struct etna_bo *')
 */
drm_public struct etna_bo *etna_bo_from_dmabuf(struct etna_device *dev, int fd)
{
	struct etna_bo *bo;
	int ret, size;
	uint32_t handle;

	/* take the lock before calling drmPrimeFDToHandle to avoid
	 * racing against etna_bo_del, which might invalidate the
	 * returned handle.
	 */
	pthread_mutex_lock(&table_lock);

	ret = drmPrimeFDToHandle(dev->fd, fd, &handle);
	if (ret) {
		pthread_mutex_unlock(&table_lock);
		return NULL;
	}

	bo = lookup_bo(dev->handle_table, handle);
	if (bo)
		goto out_unlock;

	/* lseek() to get bo size */
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_CUR);

	bo = bo_from_handle(dev, size, handle, 0);

out_unlock:
	pthread_mutex_unlock(&table_lock);

	return bo;
}

/* destroy a buffer object */
drm_public void etna_bo_del(struct etna_bo *bo)
{
	struct etna_device *dev = bo->dev;

	if (!bo)
		return;

	if (!atomic_dec_and_test(&bo->refcnt))
		return;

	pthread_mutex_lock(&table_lock);

	if (bo->reuse && (etna_bo_cache_free(&dev->bo_cache, bo) == 0))
		goto out;

	bo_del(bo);
	etna_device_del_locked(dev);
out:
	pthread_mutex_unlock(&table_lock);
}

/* get the global flink/DRI2 buffer name */
drm_public int etna_bo_get_name(struct etna_bo *bo, uint32_t *name)
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

		pthread_mutex_lock(&table_lock);
		set_name(bo, req.name);
		pthread_mutex_unlock(&table_lock);
		bo->reuse = 0;
	}

	*name = bo->name;

	return 0;
}

drm_public uint32_t etna_bo_handle(struct etna_bo *bo)
{
	return bo->handle;
}

/* caller owns the dmabuf fd that is returned and is responsible
 * to close() it when done
 */
drm_public int etna_bo_dmabuf(struct etna_bo *bo)
{
	int ret, prime_fd;

	ret = drmPrimeHandleToFD(bo->dev->fd, bo->handle, DRM_CLOEXEC,
				&prime_fd);
	if (ret) {
		ERROR_MSG("failed to get dmabuf fd: %d", ret);
		return ret;
	}

	bo->reuse = 0;

	return prime_fd;
}

drm_public uint32_t etna_bo_size(struct etna_bo *bo)
{
	return bo->size;
}

drm_public void *etna_bo_map(struct etna_bo *bo)
{
	if (!bo->map) {
		if (!bo->offset) {
			get_buffer_info(bo);
		}

		bo->map = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE,
				MAP_SHARED, bo->dev->fd, bo->offset);
		if (bo->map == MAP_FAILED) {
			ERROR_MSG("mmap failed: %s", strerror(errno));
			bo->map = NULL;
		}
	}

	return bo->map;
}

drm_public int etna_bo_cpu_prep(struct etna_bo *bo, uint32_t op)
{
	struct drm_etnaviv_gem_cpu_prep req = {
		.handle = bo->handle,
		.op = op,
	};

	get_abs_timeout(&req.timeout, 5000000000);

	return drmCommandWrite(bo->dev->fd, DRM_ETNAVIV_GEM_CPU_PREP,
			&req, sizeof(req));
}

drm_public void etna_bo_cpu_fini(struct etna_bo *bo)
{
	struct drm_etnaviv_gem_cpu_fini req = {
		.handle = bo->handle,
	};

	drmCommandWrite(bo->dev->fd, DRM_ETNAVIV_GEM_CPU_FINI,
			&req, sizeof(req));
}
