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

#include <assert.h>

#include "xf86atomic.h"
#include "freedreno_ringbuffer.h"
#include "kgsl_priv.h"


/* because kgsl tries to validate the gpuaddr on kernel side in ISSUEIBCMDS,
 * we can't use normal gem bo's for ringbuffer..  someday the kernel part
 * needs to be reworked into a single sane drm driver :-/
 */
struct kgsl_rb_bo {
	struct kgsl_pipe *pipe;
	void    *hostptr;
	uint32_t gpuaddr;
	uint32_t size;
};

struct kgsl_ringbuffer {
	struct fd_ringbuffer base;
	struct kgsl_rb_bo *bo;
};

static inline struct kgsl_ringbuffer * to_kgsl_ringbuffer(struct fd_ringbuffer *x)
{
	return (struct kgsl_ringbuffer *)x;
}

static void kgsl_rb_bo_del(struct kgsl_rb_bo *bo)
{
	struct kgsl_sharedmem_free req = {
			.gpuaddr = bo->gpuaddr,
	};
	int ret;

	drm_munmap(bo->hostptr, bo->size);

	ret = ioctl(bo->pipe->fd, IOCTL_KGSL_SHAREDMEM_FREE, &req);
	if (ret) {
		ERROR_MSG("sharedmem free failed: %s", strerror(errno));
	}

	free(bo);
}

static struct kgsl_rb_bo * kgsl_rb_bo_new(struct kgsl_pipe *pipe, uint32_t size)
{
	struct kgsl_rb_bo *bo;
	struct kgsl_gpumem_alloc req = {
			.size = ALIGN(size, 4096),
			.flags = KGSL_MEMFLAGS_GPUREADONLY,
	};
	int ret;

	bo = calloc(1, sizeof(*bo));
	if (!bo) {
		ERROR_MSG("allocation failed");
		return NULL;
	}
	ret = ioctl(pipe->fd, IOCTL_KGSL_GPUMEM_ALLOC, &req);
	if (ret) {
		ERROR_MSG("gpumem allocation failed: %s", strerror(errno));
		goto fail;
	}

	bo->pipe = pipe;
	bo->gpuaddr = req.gpuaddr;
	bo->size = size;
	bo->hostptr = drm_mmap(NULL, size, PROT_WRITE|PROT_READ,
				MAP_SHARED, pipe->fd, req.gpuaddr);

	return bo;
fail:
	if (bo)
		kgsl_rb_bo_del(bo);
	return NULL;
}

static void * kgsl_ringbuffer_hostptr(struct fd_ringbuffer *ring)
{
	struct kgsl_ringbuffer *kgsl_ring = to_kgsl_ringbuffer(ring);
	return kgsl_ring->bo->hostptr;
}

static int kgsl_ringbuffer_flush(struct fd_ringbuffer *ring, uint32_t *last_start,
		int in_fence_fd, int *out_fence_fd)
{
	struct kgsl_ringbuffer *kgsl_ring = to_kgsl_ringbuffer(ring);
	struct kgsl_pipe *kgsl_pipe = to_kgsl_pipe(ring->pipe);
	uint32_t offset = (uint8_t *)last_start - (uint8_t *)ring->start;
	struct kgsl_ibdesc ibdesc = {
			.gpuaddr     = kgsl_ring->bo->gpuaddr + offset,
			.hostptr     = last_start,
			.sizedwords  = ring->cur - last_start,
	};
	struct kgsl_ringbuffer_issueibcmds req = {
			.drawctxt_id = kgsl_pipe->drawctxt_id,
			.ibdesc_addr = (unsigned long)&ibdesc,
			.numibs      = 1,
			.flags       = KGSL_CONTEXT_SUBMIT_IB_LIST,
	};
	int ret;

	assert(in_fence_fd == -1);
	assert(out_fence_fd == NULL);

	kgsl_pipe_pre_submit(kgsl_pipe);

	/* z180_cmdstream_issueibcmds() is made of fail: */
	if (ring->pipe->id == FD_PIPE_2D) {
		/* fix up size field in last cmd packet */
		uint32_t last_size = (uint32_t)(ring->cur - last_start);
		/* 5 is length of first packet, 2 for the two 7f000000's */
		last_start[2] = last_size - (5 + 2);
		ibdesc.gpuaddr = kgsl_ring->bo->gpuaddr;
		ibdesc.hostptr = kgsl_ring->bo->hostptr;
		ibdesc.sizedwords = 0x145;
		req.timestamp = (uintptr_t)kgsl_ring->bo->hostptr;
	}

	do {
		ret = ioctl(kgsl_pipe->fd, IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS, &req);
	} while ((ret == -1) && ((errno == EINTR) || (errno == EAGAIN)));
	if (ret)
		ERROR_MSG("issueibcmds failed!  %d (%s)", ret, strerror(errno));

	ring->last_timestamp = req.timestamp;
	ring->last_start = ring->cur;

	kgsl_pipe_post_submit(kgsl_pipe, req.timestamp);

	return ret;
}

static void kgsl_ringbuffer_emit_reloc(struct fd_ringbuffer *ring,
		const struct fd_reloc *r)
{
	struct kgsl_bo *kgsl_bo = to_kgsl_bo(r->bo);
	uint32_t addr = kgsl_bo_gpuaddr(kgsl_bo, r->offset);
	assert(addr);
	if (r->shift < 0)
		addr >>= -r->shift;
	else
		addr <<= r->shift;
	(*ring->cur++) = addr | r->or;
	kgsl_pipe_add_submit(to_kgsl_pipe(ring->pipe), kgsl_bo);
}

static uint32_t kgsl_ringbuffer_emit_reloc_ring(struct fd_ringbuffer *ring,
		struct fd_ringbuffer *target, uint32_t cmd_idx)
{
	struct kgsl_ringbuffer *target_ring = to_kgsl_ringbuffer(target);
	assert(cmd_idx == 0);
	(*ring->cur++) = target_ring->bo->gpuaddr;
	return 	offset_bytes(target->cur, target->start);
}

static void kgsl_ringbuffer_destroy(struct fd_ringbuffer *ring)
{
	struct kgsl_ringbuffer *kgsl_ring = to_kgsl_ringbuffer(ring);
	if (ring->last_timestamp)
		fd_pipe_wait(ring->pipe, ring->last_timestamp);
	if (kgsl_ring->bo)
		kgsl_rb_bo_del(kgsl_ring->bo);
	free(kgsl_ring);
}

static const struct fd_ringbuffer_funcs funcs = {
		.hostptr = kgsl_ringbuffer_hostptr,
		.flush = kgsl_ringbuffer_flush,
		.emit_reloc = kgsl_ringbuffer_emit_reloc,
		.emit_reloc_ring = kgsl_ringbuffer_emit_reloc_ring,
		.destroy = kgsl_ringbuffer_destroy,
};

drm_private struct fd_ringbuffer * kgsl_ringbuffer_new(struct fd_pipe *pipe,
		uint32_t size, enum fd_ringbuffer_flags flags)
{
	struct kgsl_ringbuffer *kgsl_ring;
	struct fd_ringbuffer *ring = NULL;

	assert(!flags);

	kgsl_ring = calloc(1, sizeof(*kgsl_ring));
	if (!kgsl_ring) {
		ERROR_MSG("allocation failed");
		goto fail;
	}

	ring = &kgsl_ring->base;
	atomic_set(&ring->refcnt, 1);

	ring->funcs = &funcs;
	ring->size = size;

	kgsl_ring->bo = kgsl_rb_bo_new(to_kgsl_pipe(pipe), size);
	if (!kgsl_ring->bo) {
		ERROR_MSG("ringbuffer allocation failed");
		goto fail;
	}

	return ring;
fail:
	if (ring)
		fd_ringbuffer_del(ring);
	return NULL;
}
