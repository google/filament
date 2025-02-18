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

#ifndef KGSL_PRIV_H_
#define KGSL_PRIV_H_

#include "freedreno_priv.h"
#include "msm_kgsl.h"
#include "kgsl_drm.h"

struct kgsl_device {
	struct fd_device base;
};

static inline struct kgsl_device * to_kgsl_device(struct fd_device *x)
{
	return (struct kgsl_device *)x;
}

struct kgsl_pipe {
	struct fd_pipe base;

	int fd;
	uint32_t drawctxt_id;

	/* device properties: */
	struct kgsl_version version;
	struct kgsl_devinfo devinfo;

	/* list of bo's that are referenced in ringbuffer but not
	 * submitted yet:
	 */
	struct list_head submit_list;

	/* list of bo's that have been submitted but timestamp has
	 * not passed yet (so still ref'd in active cmdstream)
	 */
	struct list_head pending_list;

	/* if we are the 2d pipe, and want to wait on a timestamp
	 * from 3d, we need to also internally open the 3d pipe:
	 */
	struct fd_pipe *p3d;
};

static inline struct kgsl_pipe * to_kgsl_pipe(struct fd_pipe *x)
{
	return (struct kgsl_pipe *)x;
}

drm_private int is_kgsl_pipe(struct fd_pipe *pipe);

struct kgsl_bo {
	struct fd_bo base;
	uint64_t offset;
	uint32_t gpuaddr;
	/* timestamp (per pipe) for bo's in a pipe's pending_list: */
	uint32_t timestamp[FD_PIPE_MAX];
	/* list-node for pipe's submit_list or pending_list */
	struct list_head list[FD_PIPE_MAX];
};

static inline struct kgsl_bo * to_kgsl_bo(struct fd_bo *x)
{
	return (struct kgsl_bo *)x;
}


drm_private struct fd_device * kgsl_device_new(int fd);

drm_private int kgsl_pipe_timestamp(struct kgsl_pipe *kgsl_pipe,
		uint32_t *timestamp);
drm_private void kgsl_pipe_add_submit(struct kgsl_pipe *pipe,
		struct kgsl_bo *bo);
drm_private void kgsl_pipe_pre_submit(struct kgsl_pipe *pipe);
drm_private void kgsl_pipe_post_submit(struct kgsl_pipe *pipe,
		uint32_t timestamp);
drm_private void kgsl_pipe_process_pending(struct kgsl_pipe *pipe,
		uint32_t timestamp);
drm_private struct fd_pipe * kgsl_pipe_new(struct fd_device *dev,
		enum fd_pipe_id id, uint32_t prio);

drm_private struct fd_ringbuffer * kgsl_ringbuffer_new(struct fd_pipe *pipe,
		uint32_t size, enum fd_ringbuffer_flags flags);

drm_private int kgsl_bo_new_handle(struct fd_device *dev,
		uint32_t size, uint32_t flags, uint32_t *handle);
drm_private struct fd_bo * kgsl_bo_from_handle(struct fd_device *dev,
		uint32_t size, uint32_t handle);

drm_private uint32_t kgsl_bo_gpuaddr(struct kgsl_bo *bo, uint32_t offset);
drm_private void kgsl_bo_set_timestamp(struct kgsl_bo *bo, uint32_t timestamp);
drm_private uint32_t kgsl_bo_get_timestamp(struct kgsl_bo *bo);

#endif /* KGSL_PRIV_H_ */
