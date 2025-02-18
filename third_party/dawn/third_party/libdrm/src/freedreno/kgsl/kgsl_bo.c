/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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

#include "kgsl_priv.h"

static int set_memtype(struct fd_device *dev, uint32_t handle, uint32_t flags)
{
	struct drm_kgsl_gem_memtype req = {
			.handle = handle,
			.type = flags & DRM_FREEDRENO_GEM_TYPE_MEM_MASK,
	};

	return drmCommandWrite(dev->fd, DRM_KGSL_GEM_SETMEMTYPE,
			&req, sizeof(req));
}

static int bo_alloc(struct kgsl_bo *kgsl_bo)
{
	struct fd_bo *bo = &kgsl_bo->base;
	if (!kgsl_bo->offset) {
		struct drm_kgsl_gem_alloc req = {
				.handle = bo->handle,
		};
		int ret;

		/* if the buffer is already backed by pages then this
		 * doesn't actually do anything (other than giving us
		 * the offset)
		 */
		ret = drmCommandWriteRead(bo->dev->fd, DRM_KGSL_GEM_ALLOC,
				&req, sizeof(req));
		if (ret) {
			ERROR_MSG("alloc failed: %s", strerror(errno));
			return ret;
		}

		kgsl_bo->offset = req.offset;
	}

	return 0;
}

static int kgsl_bo_offset(struct fd_bo *bo, uint64_t *offset)
{
	struct kgsl_bo *kgsl_bo = to_kgsl_bo(bo);
	int ret = bo_alloc(kgsl_bo);
	if (ret)
		return ret;
	*offset = kgsl_bo->offset;
	return 0;
}

static int kgsl_bo_cpu_prep(struct fd_bo *bo, struct fd_pipe *pipe, uint32_t op)
{
	uint32_t timestamp = kgsl_bo_get_timestamp(to_kgsl_bo(bo));

	if (op & DRM_FREEDRENO_PREP_NOSYNC) {
		uint32_t current;
		int ret;

		/* special case for is_idle().. we can't really handle that
		 * properly in kgsl (perhaps we need a way to just disable
		 * the bo-cache for kgsl?)
		 */
		if (!pipe)
			return -EBUSY;

		ret = kgsl_pipe_timestamp(to_kgsl_pipe(pipe), &current);
		if (ret)
			return ret;

		if (timestamp > current)
			return -EBUSY;

		return 0;
	}

	if (timestamp)
		fd_pipe_wait(pipe, timestamp);

	return 0;
}

static void kgsl_bo_cpu_fini(struct fd_bo *bo)
{
}

static int kgsl_bo_madvise(struct fd_bo *bo, int willneed)
{
	return willneed; /* not supported by kgsl */
}

static void kgsl_bo_destroy(struct fd_bo *bo)
{
	struct kgsl_bo *kgsl_bo = to_kgsl_bo(bo);
	free(kgsl_bo);

}

static const struct fd_bo_funcs funcs = {
		.offset = kgsl_bo_offset,
		.cpu_prep = kgsl_bo_cpu_prep,
		.cpu_fini = kgsl_bo_cpu_fini,
		.madvise = kgsl_bo_madvise,
		.destroy = kgsl_bo_destroy,
};

/* allocate a buffer handle: */
drm_private int kgsl_bo_new_handle(struct fd_device *dev,
		uint32_t size, uint32_t flags, uint32_t *handle)
{
	struct drm_kgsl_gem_create req = {
			.size = size,
	};
	int ret;

	ret = drmCommandWriteRead(dev->fd, DRM_KGSL_GEM_CREATE,
			&req, sizeof(req));
	if (ret)
		return ret;

	// TODO make flags match msm driver, since kgsl is legacy..
	// translate flags in kgsl..

	set_memtype(dev, req.handle, flags);

	*handle = req.handle;

	return 0;
}

/* allocate a new buffer object */
drm_private struct fd_bo * kgsl_bo_from_handle(struct fd_device *dev,
		uint32_t size, uint32_t handle)
{
	struct kgsl_bo *kgsl_bo;
	struct fd_bo *bo;
	unsigned i;

	kgsl_bo = calloc(1, sizeof(*kgsl_bo));
	if (!kgsl_bo)
		return NULL;

	bo = &kgsl_bo->base;
	bo->funcs = &funcs;

	for (i = 0; i < ARRAY_SIZE(kgsl_bo->list); i++)
		list_inithead(&kgsl_bo->list[i]);

	return bo;
}

drm_public struct fd_bo *
fd_bo_from_fbdev(struct fd_pipe *pipe, int fbfd, uint32_t size)
{
	struct fd_bo *bo;

	if (!is_kgsl_pipe(pipe))
		return NULL;

	bo = fd_bo_new(pipe->dev, 1, 0);

	/* this is fugly, but works around a bug in the kernel..
	 * priv->memdesc.size never gets set, so getbufinfo ioctl
	 * thinks the buffer hasn't be allocate and fails
	 */
	if (bo) {
		void *fbmem = drm_mmap(NULL, size, PROT_READ | PROT_WRITE,
				MAP_SHARED, fbfd, 0);
		struct kgsl_map_user_mem req = {
				.memtype = KGSL_USER_MEM_TYPE_ADDR,
				.len     = size,
				.offset  = 0,
				.hostptr = (unsigned long)fbmem,
		};
		struct kgsl_bo *kgsl_bo = to_kgsl_bo(bo);
		int ret;

		ret = ioctl(to_kgsl_pipe(pipe)->fd, IOCTL_KGSL_MAP_USER_MEM, &req);
		if (ret) {
			ERROR_MSG("mapping user mem failed: %s",
					strerror(errno));
			goto fail;
		}
		kgsl_bo->gpuaddr = req.gpuaddr;
		bo->map = fbmem;
	}

	return bo;
fail:
	if (bo)
		fd_bo_del(bo);
	return NULL;
}

drm_private uint32_t kgsl_bo_gpuaddr(struct kgsl_bo *kgsl_bo, uint32_t offset)
{
	struct fd_bo *bo = &kgsl_bo->base;
	if (!kgsl_bo->gpuaddr) {
		struct drm_kgsl_gem_bufinfo req = {
				.handle = bo->handle,
		};
		int ret;

		ret = bo_alloc(kgsl_bo);
		if (ret) {
			return ret;
		}

		ret = drmCommandWriteRead(bo->dev->fd, DRM_KGSL_GEM_GET_BUFINFO,
				&req, sizeof(req));
		if (ret) {
			ERROR_MSG("get bufinfo failed: %s", strerror(errno));
			return 0;
		}

		kgsl_bo->gpuaddr = req.gpuaddr[0];
	}
	return kgsl_bo->gpuaddr + offset;
}

/*
 * Super-cheezy way to synchronization between mesa and ddx..  the
 * SET_ACTIVE ioctl gives us a way to stash a 32b # w/ a GEM bo, and
 * GET_BUFINFO gives us a way to retrieve it.  We use this to stash
 * the timestamp of the last ISSUEIBCMDS on the buffer.
 *
 * To avoid an obscene amount of syscalls, we:
 *  1) Only set the timestamp for buffers w/ an flink name, ie.
 *     only buffers shared across processes.  This is enough to
 *     catch the DRI2 buffers.
 *  2) Only set the timestamp for buffers submitted to the 3d ring
 *     and only check the timestamps on buffers submitted to the
 *     2d ring.  This should be enough to handle synchronizing of
 *     presentation blit.  We could do synchronization in the other
 *     direction too, but that would be problematic if we are using
 *     the 3d ring from DDX, since client side wouldn't know this.
 *
 * The waiting on timestamp happens before flush, and setting of
 * timestamp happens after flush.  It is transparent to the user
 * of libdrm_freedreno as all the tracking of buffers happens via
 * _emit_reloc()..
 */

drm_private void kgsl_bo_set_timestamp(struct kgsl_bo *kgsl_bo,
		uint32_t timestamp)
{
	struct fd_bo *bo = &kgsl_bo->base;
	if (bo->name) {
		struct drm_kgsl_gem_active req = {
				.handle = bo->handle,
				.active = timestamp,
		};
		int ret;

		ret = drmCommandWrite(bo->dev->fd, DRM_KGSL_GEM_SET_ACTIVE,
				&req, sizeof(req));
		if (ret) {
			ERROR_MSG("set active failed: %s", strerror(errno));
		}
	}
}

drm_private uint32_t kgsl_bo_get_timestamp(struct kgsl_bo *kgsl_bo)
{
	struct fd_bo *bo = &kgsl_bo->base;
	uint32_t timestamp = 0;
	if (bo->name) {
		struct drm_kgsl_gem_bufinfo req = {
				.handle = bo->handle,
		};
		int ret;

		ret = drmCommandWriteRead(bo->dev->fd, DRM_KGSL_GEM_GET_BUFINFO,
				&req, sizeof(req));
		if (ret) {
			ERROR_MSG("get bufinfo failed: %s", strerror(errno));
			return 0;
		}

		timestamp = req.active;
	}
	return timestamp;
}
