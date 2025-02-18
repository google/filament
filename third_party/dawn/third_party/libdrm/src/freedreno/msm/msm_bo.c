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

#include "msm_priv.h"

static int bo_allocate(struct msm_bo *msm_bo)
{
	struct fd_bo *bo = &msm_bo->base;
	if (!msm_bo->offset) {
		struct drm_msm_gem_info req = {
				.handle = bo->handle,
		};
		int ret;

		/* if the buffer is already backed by pages then this
		 * doesn't actually do anything (other than giving us
		 * the offset)
		 */
		ret = drmCommandWriteRead(bo->dev->fd, DRM_MSM_GEM_INFO,
				&req, sizeof(req));
		if (ret) {
			ERROR_MSG("alloc failed: %s", strerror(errno));
			return ret;
		}

		msm_bo->offset = req.offset;
	}

	return 0;
}

static int msm_bo_offset(struct fd_bo *bo, uint64_t *offset)
{
	struct msm_bo *msm_bo = to_msm_bo(bo);
	int ret = bo_allocate(msm_bo);
	if (ret)
		return ret;
	*offset = msm_bo->offset;
	return 0;
}

static int msm_bo_cpu_prep(struct fd_bo *bo, struct fd_pipe *pipe, uint32_t op)
{
	struct drm_msm_gem_cpu_prep req = {
			.handle = bo->handle,
			.op = op,
	};

	get_abs_timeout(&req.timeout, 5000000000);

	return drmCommandWrite(bo->dev->fd, DRM_MSM_GEM_CPU_PREP, &req, sizeof(req));
}

static void msm_bo_cpu_fini(struct fd_bo *bo)
{
	struct drm_msm_gem_cpu_fini req = {
			.handle = bo->handle,
	};

	drmCommandWrite(bo->dev->fd, DRM_MSM_GEM_CPU_FINI, &req, sizeof(req));
}

static int msm_bo_madvise(struct fd_bo *bo, int willneed)
{
	struct drm_msm_gem_madvise req = {
			.handle = bo->handle,
			.madv = willneed ? MSM_MADV_WILLNEED : MSM_MADV_DONTNEED,
	};
	int ret;

	/* older kernels do not support this: */
	if (bo->dev->version < FD_VERSION_MADVISE)
		return willneed;

	ret = drmCommandWriteRead(bo->dev->fd, DRM_MSM_GEM_MADVISE, &req, sizeof(req));
	if (ret)
		return ret;

	return req.retained;
}

static uint64_t msm_bo_iova(struct fd_bo *bo)
{
	struct drm_msm_gem_info req = {
			.handle = bo->handle,
			.flags = MSM_INFO_IOVA,
	};

	drmCommandWriteRead(bo->dev->fd, DRM_MSM_GEM_INFO, &req, sizeof(req));

	return req.offset;
}

static void msm_bo_destroy(struct fd_bo *bo)
{
	struct msm_bo *msm_bo = to_msm_bo(bo);
	free(msm_bo);

}

static const struct fd_bo_funcs funcs = {
		.offset = msm_bo_offset,
		.cpu_prep = msm_bo_cpu_prep,
		.cpu_fini = msm_bo_cpu_fini,
		.madvise = msm_bo_madvise,
		.iova = msm_bo_iova,
		.destroy = msm_bo_destroy,
};

/* allocate a buffer handle: */
drm_private int msm_bo_new_handle(struct fd_device *dev,
		uint32_t size, uint32_t flags, uint32_t *handle)
{
	struct drm_msm_gem_new req = {
			.size = size,
			.flags = MSM_BO_WC,  // TODO figure out proper flags..
	};
	int ret;

	ret = drmCommandWriteRead(dev->fd, DRM_MSM_GEM_NEW,
			&req, sizeof(req));
	if (ret)
		return ret;

	*handle = req.handle;

	return 0;
}

/* allocate a new buffer object */
drm_private struct fd_bo * msm_bo_from_handle(struct fd_device *dev,
		uint32_t size, uint32_t handle)
{
	struct msm_bo *msm_bo;
	struct fd_bo *bo;

	msm_bo = calloc(1, sizeof(*msm_bo));
	if (!msm_bo)
		return NULL;

	bo = &msm_bo->base;
	bo->funcs = &funcs;

	return bo;
}
