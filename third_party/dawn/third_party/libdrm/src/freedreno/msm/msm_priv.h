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

#ifndef MSM_PRIV_H_
#define MSM_PRIV_H_

#include "freedreno_priv.h"

#ifndef __user
#  define __user
#endif

#include "msm_drm.h"

struct msm_device {
	struct fd_device base;
	struct fd_bo_cache ring_cache;
	unsigned ring_cnt;
};

static inline struct msm_device * to_msm_device(struct fd_device *x)
{
	return (struct msm_device *)x;
}

drm_private struct fd_device * msm_device_new(int fd);

struct msm_pipe {
	struct fd_pipe base;
	uint32_t pipe;
	uint32_t gpu_id;
	uint32_t gmem;
	uint32_t chip_id;
	uint32_t queue_id;

	/* Allow for sub-allocation of stateobj ring buffers (ie. sharing
	 * the same underlying bo)..
	 *
	 * This takes advantage of each context having it's own fd_pipe,
	 * so we don't have to worry about access from multiple threads.
	 *
	 * We also rely on previous stateobj having been fully constructed
	 * so we can reclaim extra space at it's end.
	 */
	struct fd_ringbuffer *suballoc_ring;
};

static inline struct msm_pipe * to_msm_pipe(struct fd_pipe *x)
{
	return (struct msm_pipe *)x;
}

drm_private struct fd_pipe * msm_pipe_new(struct fd_device *dev,
		enum fd_pipe_id id, uint32_t prio);

drm_private struct fd_ringbuffer * msm_ringbuffer_new(struct fd_pipe *pipe,
		uint32_t size, enum fd_ringbuffer_flags flags);

struct msm_bo {
	struct fd_bo base;
	uint64_t offset;
	uint64_t presumed;
	/* to avoid excess hashtable lookups, cache the ring this bo was
	 * last emitted on (since that will probably also be the next ring
	 * it is emitted on)
	 */
	unsigned current_ring_seqno;
	uint32_t idx;
};

static inline struct msm_bo * to_msm_bo(struct fd_bo *x)
{
	return (struct msm_bo *)x;
}

drm_private int msm_bo_new_handle(struct fd_device *dev,
		uint32_t size, uint32_t flags, uint32_t *handle);
drm_private struct fd_bo * msm_bo_from_handle(struct fd_device *dev,
		uint32_t size, uint32_t handle);

static inline void get_abs_timeout(struct drm_msm_timespec *tv, uint64_t ns)
{
	struct timespec t;
	uint32_t s = ns / 1000000000;
	clock_gettime(CLOCK_MONOTONIC, &t);
	tv->tv_sec = t.tv_sec + s;
	tv->tv_nsec = t.tv_nsec + ns - (s * 1000000000);
}

/*
 * Stupid/simple growable array implementation:
 */

static inline void *
grow(void *ptr, uint32_t nr, uint32_t *max, uint32_t sz)
{
	if ((nr + 1) > *max) {
		if ((*max * 2) < (nr + 1))
			*max = nr + 5;
		else
			*max = *max * 2;
		ptr = realloc(ptr, *max * sz);
	}
	return ptr;
}

#define DECLARE_ARRAY(type, name) \
	unsigned nr_ ## name, max_ ## name; \
	type * name;

#define APPEND(x, name) ({ \
	(x)->name = grow((x)->name, (x)->nr_ ## name, &(x)->max_ ## name, sizeof((x)->name[0])); \
	(x)->nr_ ## name ++; \
})

#endif /* MSM_PRIV_H_ */
