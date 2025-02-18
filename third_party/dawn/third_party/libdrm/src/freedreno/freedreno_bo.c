/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "freedreno_drmif.h"
#include "freedreno_priv.h"

drm_private pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
drm_private void bo_del(struct fd_bo *bo);

/* set buffer name, and add to table, call w/ table_lock held: */
static void set_name(struct fd_bo *bo, uint32_t name)
{
	bo->name = name;
	/* add ourself into the handle table: */
	drmHashInsert(bo->dev->name_table, name, bo);
}

/* lookup a buffer, call w/ table_lock held: */
static struct fd_bo * lookup_bo(void *tbl, uint32_t key)
{
	struct fd_bo *bo = NULL;
	if (!drmHashLookup(tbl, key, (void **)&bo)) {
		/* found, incr refcnt and return: */
		bo = fd_bo_ref(bo);

		/* don't break the bucket if this bo was found in one */
		list_delinit(&bo->list);
	}
	return bo;
}

/* allocate a new buffer object, call w/ table_lock held */
static struct fd_bo * bo_from_handle(struct fd_device *dev,
		uint32_t size, uint32_t handle)
{
	struct fd_bo *bo;

	bo = dev->funcs->bo_from_handle(dev, size, handle);
	if (!bo) {
		drmCloseBufferHandle(dev->fd, handle);
		return NULL;
	}
	bo->dev = fd_device_ref(dev);
	bo->size = size;
	bo->handle = handle;
	atomic_set(&bo->refcnt, 1);
	list_inithead(&bo->list);
	/* add ourself into the handle table: */
	drmHashInsert(dev->handle_table, handle, bo);
	return bo;
}

static struct fd_bo *
bo_new(struct fd_device *dev, uint32_t size, uint32_t flags,
		struct fd_bo_cache *cache)
{
	struct fd_bo *bo = NULL;
	uint32_t handle;
	int ret;

	bo = fd_bo_cache_alloc(cache, &size, flags);
	if (bo)
		return bo;

	ret = dev->funcs->bo_new_handle(dev, size, flags, &handle);
	if (ret)
		return NULL;

	pthread_mutex_lock(&table_lock);
	bo = bo_from_handle(dev, size, handle);
	pthread_mutex_unlock(&table_lock);

	VG_BO_ALLOC(bo);

	return bo;
}

drm_public struct fd_bo *
fd_bo_new(struct fd_device *dev, uint32_t size, uint32_t flags)
{
	struct fd_bo *bo = bo_new(dev, size, flags, &dev->bo_cache);
	if (bo)
		bo->bo_reuse = BO_CACHE;
	return bo;
}

/* internal function to allocate bo's that use the ringbuffer cache
 * instead of the normal bo_cache.  The purpose is, because cmdstream
 * bo's get vmap'd on the kernel side, and that is expensive, we want
 * to re-use cmdstream bo's for cmdstream and not unrelated purposes.
 */
drm_private struct fd_bo *
fd_bo_new_ring(struct fd_device *dev, uint32_t size, uint32_t flags)
{
	struct fd_bo *bo = bo_new(dev, size, flags, &dev->ring_cache);
	if (bo)
		bo->bo_reuse = RING_CACHE;
	return bo;
}

drm_public struct fd_bo *
fd_bo_from_handle(struct fd_device *dev, uint32_t handle, uint32_t size)
{
	struct fd_bo *bo = NULL;

	pthread_mutex_lock(&table_lock);

	bo = lookup_bo(dev->handle_table, handle);
	if (bo)
		goto out_unlock;

	bo = bo_from_handle(dev, size, handle);

	VG_BO_ALLOC(bo);

out_unlock:
	pthread_mutex_unlock(&table_lock);

	return bo;
}

drm_public struct fd_bo *
fd_bo_from_dmabuf(struct fd_device *dev, int fd)
{
	int ret, size;
	uint32_t handle;
	struct fd_bo *bo;

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

	bo = bo_from_handle(dev, size, handle);

	VG_BO_ALLOC(bo);

out_unlock:
	pthread_mutex_unlock(&table_lock);

	return bo;
}

drm_public struct fd_bo * fd_bo_from_name(struct fd_device *dev, uint32_t name)
{
	struct drm_gem_open req = {
			.name = name,
	};
	struct fd_bo *bo;

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

	bo = bo_from_handle(dev, req.size, req.handle);
	if (bo) {
		set_name(bo, name);
		VG_BO_ALLOC(bo);
	}

out_unlock:
	pthread_mutex_unlock(&table_lock);

	return bo;
}

drm_public uint64_t fd_bo_get_iova(struct fd_bo *bo)
{
	return bo->funcs->iova(bo);
}

drm_public void fd_bo_put_iova(struct fd_bo *bo)
{
	/* currently a no-op */
}

drm_public struct fd_bo * fd_bo_ref(struct fd_bo *bo)
{
	atomic_inc(&bo->refcnt);
	return bo;
}

drm_public void fd_bo_del(struct fd_bo *bo)
{
	struct fd_device *dev = bo->dev;

	if (!atomic_dec_and_test(&bo->refcnt))
		return;

	pthread_mutex_lock(&table_lock);

	if ((bo->bo_reuse == BO_CACHE) && (fd_bo_cache_free(&dev->bo_cache, bo) == 0))
		goto out;
	if ((bo->bo_reuse == RING_CACHE) && (fd_bo_cache_free(&dev->ring_cache, bo) == 0))
		goto out;

	bo_del(bo);
	fd_device_del_locked(dev);
out:
	pthread_mutex_unlock(&table_lock);
}

/* Called under table_lock */
drm_private void bo_del(struct fd_bo *bo)
{
	VG_BO_FREE(bo);

	if (bo->map)
		drm_munmap(bo->map, bo->size);

	/* TODO probably bo's in bucket list get removed from
	 * handle table??
	 */

	if (bo->handle) {
		drmHashDelete(bo->dev->handle_table, bo->handle);
		if (bo->name)
			drmHashDelete(bo->dev->name_table, bo->name);
		drmCloseBufferHandle(bo->dev->fd, bo->handle);
	}

	bo->funcs->destroy(bo);
}

drm_public int fd_bo_get_name(struct fd_bo *bo, uint32_t *name)
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
		bo->bo_reuse = NO_CACHE;
	}

	*name = bo->name;

	return 0;
}

drm_public uint32_t fd_bo_handle(struct fd_bo *bo)
{
	return bo->handle;
}

drm_public int fd_bo_dmabuf(struct fd_bo *bo)
{
	int ret, prime_fd;

	ret = drmPrimeHandleToFD(bo->dev->fd, bo->handle, DRM_CLOEXEC,
			&prime_fd);
	if (ret) {
		ERROR_MSG("failed to get dmabuf fd: %d", ret);
		return ret;
	}

	bo->bo_reuse = NO_CACHE;

	return prime_fd;
}

drm_public uint32_t fd_bo_size(struct fd_bo *bo)
{
	return bo->size;
}

drm_public void * fd_bo_map(struct fd_bo *bo)
{
	if (!bo->map) {
		uint64_t offset;
		int ret;

		ret = bo->funcs->offset(bo, &offset);
		if (ret) {
			return NULL;
		}

		bo->map = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
				bo->dev->fd, offset);
		if (bo->map == MAP_FAILED) {
			ERROR_MSG("mmap failed: %s", strerror(errno));
			bo->map = NULL;
		}
	}
	return bo->map;
}

/* a bit odd to take the pipe as an arg, but it's a, umm, quirk of kgsl.. */
drm_public int fd_bo_cpu_prep(struct fd_bo *bo, struct fd_pipe *pipe, uint32_t op)
{
	return bo->funcs->cpu_prep(bo, pipe, op);
}

drm_public void fd_bo_cpu_fini(struct fd_bo *bo)
{
	bo->funcs->cpu_fini(bo);
}

#if !HAVE_FREEDRENO_KGSL
drm_public struct fd_bo * fd_bo_from_fbdev(struct fd_pipe *pipe, int fbfd, uint32_t size)
{
    return NULL;
}
#endif
