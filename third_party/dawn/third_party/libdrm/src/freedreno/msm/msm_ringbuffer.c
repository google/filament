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
#include <inttypes.h>

#include "xf86atomic.h"
#include "freedreno_ringbuffer.h"
#include "msm_priv.h"

/* represents a single cmd buffer in the submit ioctl.  Each cmd buffer has
 * a backing bo, and a reloc table.
 */
struct msm_cmd {
	struct list_head list;

	struct fd_ringbuffer *ring;
	struct fd_bo *ring_bo;

	/* reloc's table: */
	DECLARE_ARRAY(struct drm_msm_gem_submit_reloc, relocs);

	uint32_t size;

	/* has cmd already been added to parent rb's submit.cmds table? */
	int is_appended_to_submit;
};

struct msm_ringbuffer {
	struct fd_ringbuffer base;

	/* submit ioctl related tables:
	 * Note that bos and cmds are tracked by the parent ringbuffer, since
	 * that is global to the submit ioctl call.  The reloc's table is tracked
	 * per cmd-buffer.
	 */
	struct {
		/* bo's table: */
		DECLARE_ARRAY(struct drm_msm_gem_submit_bo, bos);

		/* cmd's table: */
		DECLARE_ARRAY(struct drm_msm_gem_submit_cmd, cmds);
	} submit;

	/* should have matching entries in submit.bos: */
	/* Note, only in parent ringbuffer */
	DECLARE_ARRAY(struct fd_bo *, bos);

	/* should have matching entries in submit.cmds: */
	DECLARE_ARRAY(struct msm_cmd *, cmds);

	/* List of physical cmdstream buffers (msm_cmd) associated with this
	 * logical fd_ringbuffer.
	 *
	 * Note that this is different from msm_ringbuffer::cmds (which
	 * shadows msm_ringbuffer::submit::cmds for tracking submit ioctl
	 * related stuff, and *only* is tracked in the parent ringbuffer.
	 * And only has "completed" cmd buffers (ie. we already know the
	 * size) added via get_cmd().
	 */
	struct list_head cmd_list;

	int is_growable;
	unsigned cmd_count;

	unsigned offset;    /* for sub-allocated stateobj rb's */

	unsigned seqno;

	/* maps fd_bo to idx: */
	void *bo_table;

	/* maps msm_cmd to drm_msm_gem_submit_cmd in parent rb.  Each rb has a
	 * list of msm_cmd's which correspond to each chunk of cmdstream in
	 * a 'growable' rb.  For each of those we need to create one
	 * drm_msm_gem_submit_cmd in the parent rb which collects the state
	 * for the submit ioctl.  Because we can have multiple IB's to the same
	 * target rb (for example, or same stateobj emit multiple times), and
	 * because in theory we can have multiple different rb's that have a
	 * reference to a given target, we need a hashtable to track this per
	 * rb.
	 */
	void *cmd_table;
};

static inline struct msm_ringbuffer * to_msm_ringbuffer(struct fd_ringbuffer *x)
{
	return (struct msm_ringbuffer *)x;
}

#define INIT_SIZE 0x1000

static pthread_mutex_t idx_lock = PTHREAD_MUTEX_INITIALIZER;

static struct msm_cmd *current_cmd(struct fd_ringbuffer *ring)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
	assert(!LIST_IS_EMPTY(&msm_ring->cmd_list));
	return LIST_LAST_ENTRY(&msm_ring->cmd_list, struct msm_cmd, list);
}

static void ring_cmd_del(struct msm_cmd *cmd)
{
	fd_bo_del(cmd->ring_bo);
	list_del(&cmd->list);
	to_msm_ringbuffer(cmd->ring)->cmd_count--;
	free(cmd->relocs);
	free(cmd);
}

static struct msm_cmd * ring_cmd_new(struct fd_ringbuffer *ring, uint32_t size,
		enum fd_ringbuffer_flags flags)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
	struct msm_cmd *cmd = calloc(1, sizeof(*cmd));

	if (!cmd)
		return NULL;

	cmd->ring = ring;

	/* TODO separate suballoc buffer for small non-streaming state, using
	 * smaller page-sized backing bo's.
	 */
	if (flags & FD_RINGBUFFER_STREAMING) {
		struct msm_pipe *msm_pipe = to_msm_pipe(ring->pipe);
		unsigned suballoc_offset = 0;
		struct fd_bo *suballoc_bo = NULL;

		if (msm_pipe->suballoc_ring) {
			struct msm_ringbuffer *suballoc_ring = to_msm_ringbuffer(msm_pipe->suballoc_ring);

			assert(msm_pipe->suballoc_ring->flags & FD_RINGBUFFER_OBJECT);
			assert(suballoc_ring->cmd_count == 1);

			suballoc_bo = current_cmd(msm_pipe->suballoc_ring)->ring_bo;

			suballoc_offset = fd_ringbuffer_size(msm_pipe->suballoc_ring) +
					suballoc_ring->offset;

			suballoc_offset = ALIGN(suballoc_offset, 0x10);

			if ((size + suballoc_offset) > suballoc_bo->size) {
				suballoc_bo = NULL;
			}
		}

		if (!suballoc_bo) {
			cmd->ring_bo = fd_bo_new_ring(ring->pipe->dev, 0x8000, 0);
			msm_ring->offset = 0;
		} else {
			cmd->ring_bo = fd_bo_ref(suballoc_bo);
			msm_ring->offset = suballoc_offset;
		}

		if (msm_pipe->suballoc_ring)
			fd_ringbuffer_del(msm_pipe->suballoc_ring);

		msm_pipe->suballoc_ring = fd_ringbuffer_ref(ring);
	} else {
		cmd->ring_bo = fd_bo_new_ring(ring->pipe->dev, size, 0);
	}
	if (!cmd->ring_bo)
		goto fail;

	list_addtail(&cmd->list, &msm_ring->cmd_list);
	msm_ring->cmd_count++;

	return cmd;

fail:
	ring_cmd_del(cmd);
	return NULL;
}

static uint32_t append_bo(struct fd_ringbuffer *ring, struct fd_bo *bo)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
	uint32_t idx;

	idx = APPEND(&msm_ring->submit, bos);
	idx = APPEND(msm_ring, bos);

	msm_ring->submit.bos[idx].flags = 0;
	msm_ring->submit.bos[idx].handle = bo->handle;
	msm_ring->submit.bos[idx].presumed = to_msm_bo(bo)->presumed;

	msm_ring->bos[idx] = fd_bo_ref(bo);

	return idx;
}

/* add (if needed) bo, return idx: */
static uint32_t bo2idx(struct fd_ringbuffer *ring, struct fd_bo *bo, uint32_t flags)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
	struct msm_bo *msm_bo = to_msm_bo(bo);
	uint32_t idx;
	pthread_mutex_lock(&idx_lock);
	if (msm_bo->current_ring_seqno == msm_ring->seqno) {
		idx = msm_bo->idx;
	} else {
		void *val;

		if (!msm_ring->bo_table)
			msm_ring->bo_table = drmHashCreate();

		if (!drmHashLookup(msm_ring->bo_table, bo->handle, &val)) {
			/* found */
			idx = (uint32_t)(uintptr_t)val;
		} else {
			idx = append_bo(ring, bo);
			val = (void *)(uintptr_t)idx;
			drmHashInsert(msm_ring->bo_table, bo->handle, val);
		}
		msm_bo->current_ring_seqno = msm_ring->seqno;
		msm_bo->idx = idx;
	}
	pthread_mutex_unlock(&idx_lock);
	if (flags & FD_RELOC_READ)
		msm_ring->submit.bos[idx].flags |= MSM_SUBMIT_BO_READ;
	if (flags & FD_RELOC_WRITE)
		msm_ring->submit.bos[idx].flags |= MSM_SUBMIT_BO_WRITE;
	return idx;
}

/* Ensure that submit has corresponding entry in cmds table for the
 * target cmdstream buffer:
 *
 * Returns TRUE if new cmd added (else FALSE if it was already in
 * the cmds table)
 */
static int get_cmd(struct fd_ringbuffer *ring, struct msm_cmd *target_cmd,
		uint32_t submit_offset, uint32_t size, uint32_t type)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
	struct drm_msm_gem_submit_cmd *cmd;
	uint32_t i;
	void *val;

	if (!msm_ring->cmd_table)
		msm_ring->cmd_table = drmHashCreate();

	/* figure out if we already have a cmd buf.. short-circuit hash
	 * lookup if:
	 *  - target cmd has never been added to submit.cmds
	 *  - target cmd is not a streaming stateobj (which unlike longer
	 *    lived CSO stateobj, is not expected to be reused with multiple
	 *    submits)
	 */
	if (target_cmd->is_appended_to_submit &&
			!(target_cmd->ring->flags & FD_RINGBUFFER_STREAMING) &&
			!drmHashLookup(msm_ring->cmd_table, (unsigned long)target_cmd, &val)) {
		i = VOID2U64(val);
		cmd = &msm_ring->submit.cmds[i];

		assert(cmd->submit_offset == submit_offset);
		assert(cmd->size == size);
		assert(cmd->type == type);
		assert(msm_ring->submit.bos[cmd->submit_idx].handle ==
				target_cmd->ring_bo->handle);

		return FALSE;
	}

	/* create cmd buf if not: */
	i = APPEND(&msm_ring->submit, cmds);
	APPEND(msm_ring, cmds);
	msm_ring->cmds[i] = target_cmd;
	cmd = &msm_ring->submit.cmds[i];
	cmd->type = type;
	cmd->submit_idx = bo2idx(ring, target_cmd->ring_bo, FD_RELOC_READ);
	cmd->submit_offset = submit_offset;
	cmd->size = size;
	cmd->pad = 0;

	target_cmd->is_appended_to_submit = TRUE;

	if (!(target_cmd->ring->flags & FD_RINGBUFFER_STREAMING)) {
		drmHashInsert(msm_ring->cmd_table, (unsigned long)target_cmd,
				U642VOID(i));
	}

	target_cmd->size = size;

	return TRUE;
}

static void * msm_ringbuffer_hostptr(struct fd_ringbuffer *ring)
{
	struct msm_cmd *cmd = current_cmd(ring);
	uint8_t *base = fd_bo_map(cmd->ring_bo);
	return base + to_msm_ringbuffer(ring)->offset;
}

static void delete_cmds(struct msm_ringbuffer *msm_ring)
{
	struct msm_cmd *cmd, *tmp;

	LIST_FOR_EACH_ENTRY_SAFE(cmd, tmp, &msm_ring->cmd_list, list) {
		ring_cmd_del(cmd);
	}
}

static void flush_reset(struct fd_ringbuffer *ring)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
	unsigned i;

	for (i = 0; i < msm_ring->nr_bos; i++) {
		struct msm_bo *msm_bo = to_msm_bo(msm_ring->bos[i]);
		if (!msm_bo)
			continue;
		msm_bo->current_ring_seqno = 0;
		fd_bo_del(&msm_bo->base);
	}

	for (i = 0; i < msm_ring->nr_cmds; i++) {
		struct msm_cmd *msm_cmd = msm_ring->cmds[i];

		if (msm_cmd->ring == ring)
			continue;

		if (msm_cmd->ring->flags & FD_RINGBUFFER_OBJECT)
			fd_ringbuffer_del(msm_cmd->ring);
	}

	msm_ring->submit.nr_cmds = 0;
	msm_ring->submit.nr_bos = 0;
	msm_ring->nr_cmds = 0;
	msm_ring->nr_bos = 0;

	if (msm_ring->bo_table) {
		drmHashDestroy(msm_ring->bo_table);
		msm_ring->bo_table = NULL;
	}

	if (msm_ring->cmd_table) {
		drmHashDestroy(msm_ring->cmd_table);
		msm_ring->cmd_table = NULL;
	}

	if (msm_ring->is_growable) {
		delete_cmds(msm_ring);
	} else {
		/* in old mode, just reset the # of relocs: */
		current_cmd(ring)->nr_relocs = 0;
	}
}

static void finalize_current_cmd(struct fd_ringbuffer *ring, uint32_t *last_start)
{
	uint32_t submit_offset, size, type;
	struct fd_ringbuffer *parent;

	if (ring->parent) {
		parent = ring->parent;
		type = MSM_SUBMIT_CMD_IB_TARGET_BUF;
	} else {
		parent = ring;
		type = MSM_SUBMIT_CMD_BUF;
	}

	submit_offset = offset_bytes(last_start, ring->start);
	size = offset_bytes(ring->cur, last_start);

	get_cmd(parent, current_cmd(ring), submit_offset, size, type);
}

static void dump_submit(struct msm_ringbuffer *msm_ring)
{
	uint32_t i, j;

	for (i = 0; i < msm_ring->submit.nr_bos; i++) {
		struct drm_msm_gem_submit_bo *bo = &msm_ring->submit.bos[i];
		ERROR_MSG("  bos[%d]: handle=%u, flags=%x", i, bo->handle, bo->flags);
	}
	for (i = 0; i < msm_ring->submit.nr_cmds; i++) {
		struct drm_msm_gem_submit_cmd *cmd = &msm_ring->submit.cmds[i];
		struct drm_msm_gem_submit_reloc *relocs = U642VOID(cmd->relocs);
		ERROR_MSG("  cmd[%d]: type=%u, submit_idx=%u, submit_offset=%u, size=%u",
				i, cmd->type, cmd->submit_idx, cmd->submit_offset, cmd->size);
		for (j = 0; j < cmd->nr_relocs; j++) {
			struct drm_msm_gem_submit_reloc *r = &relocs[j];
			ERROR_MSG("    reloc[%d]: submit_offset=%u, or=%08x, shift=%d, reloc_idx=%u"
					", reloc_offset=%"PRIu64, j, r->submit_offset, r->or, r->shift,
					r->reloc_idx, r->reloc_offset);
		}
	}
}

static struct drm_msm_gem_submit_reloc *
handle_stateobj_relocs(struct fd_ringbuffer *parent, struct fd_ringbuffer *stateobj,
		struct drm_msm_gem_submit_reloc *orig_relocs, unsigned nr_relocs)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(stateobj);
	struct drm_msm_gem_submit_reloc *relocs = malloc(nr_relocs * sizeof(*relocs));
	unsigned i;

	for (i = 0; i < nr_relocs; i++) {
		unsigned idx = orig_relocs[i].reloc_idx;
		struct fd_bo *bo = msm_ring->bos[idx];
		unsigned flags = 0;

		if (msm_ring->submit.bos[idx].flags & MSM_SUBMIT_BO_READ)
			flags |= FD_RELOC_READ;
		if (msm_ring->submit.bos[idx].flags & MSM_SUBMIT_BO_WRITE)
			flags |= FD_RELOC_WRITE;

		relocs[i] = orig_relocs[i];
		relocs[i].reloc_idx = bo2idx(parent, bo, flags);
	}

	/* stateobj rb's could have reloc's to other stateobj rb's which didn't
	 * get propagated to the parent rb at _emit_reloc_ring() time (because
	 * the parent wasn't known then), so fix that up now:
	 */
	for (i = 0; i < msm_ring->nr_cmds; i++) {
		struct msm_cmd *msm_cmd = msm_ring->cmds[i];
		struct drm_msm_gem_submit_cmd *cmd = &msm_ring->submit.cmds[i];

		if (msm_ring->cmds[i]->ring == stateobj)
			continue;

		assert(msm_cmd->ring->flags & FD_RINGBUFFER_OBJECT);

		if (get_cmd(parent, msm_cmd, cmd->submit_offset, cmd->size, cmd->type)) {
			fd_ringbuffer_ref(msm_cmd->ring);
		}
	}

	return relocs;
}

static int msm_ringbuffer_flush(struct fd_ringbuffer *ring, uint32_t *last_start,
		int in_fence_fd, int *out_fence_fd)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);
	struct msm_pipe *msm_pipe = to_msm_pipe(ring->pipe);
	struct drm_msm_gem_submit req = {
			.flags = msm_pipe->pipe,
			.queueid = msm_pipe->queue_id,
	};
	uint32_t i;
	int ret;

	assert(!ring->parent);

	if (in_fence_fd != -1) {
		req.flags |= MSM_SUBMIT_FENCE_FD_IN | MSM_SUBMIT_NO_IMPLICIT;
		req.fence_fd = in_fence_fd;
	}

	if (out_fence_fd) {
		req.flags |= MSM_SUBMIT_FENCE_FD_OUT;
	}

	finalize_current_cmd(ring, last_start);

	/* for each of the cmd's fix up their reloc's: */
	for (i = 0; i < msm_ring->submit.nr_cmds; i++) {
		struct msm_cmd *msm_cmd = msm_ring->cmds[i];
		struct drm_msm_gem_submit_reloc *relocs = msm_cmd->relocs;
		struct drm_msm_gem_submit_cmd *cmd;
		unsigned nr_relocs = msm_cmd->nr_relocs;

		/* for reusable stateobjs, the reloc table has reloc_idx that
		 * points into it's own private bos table, rather than the global
		 * bos table used for the submit, so we need to add the stateobj's
		 * bos to the global table and construct new relocs table with
		 * corresponding reloc_idx
		 */
		if (msm_cmd->ring->flags & FD_RINGBUFFER_OBJECT) {
			relocs = handle_stateobj_relocs(ring, msm_cmd->ring,
					relocs, nr_relocs);
		}

		cmd = &msm_ring->submit.cmds[i];
		cmd->relocs = VOID2U64(relocs);
		cmd->nr_relocs = nr_relocs;
	}

	/* needs to be after get_cmd() as that could create bos/cmds table: */
	req.bos = VOID2U64(msm_ring->submit.bos),
	req.nr_bos = msm_ring->submit.nr_bos;
	req.cmds = VOID2U64(msm_ring->submit.cmds),
	req.nr_cmds = msm_ring->submit.nr_cmds;

	DEBUG_MSG("nr_cmds=%u, nr_bos=%u", req.nr_cmds, req.nr_bos);

	ret = drmCommandWriteRead(ring->pipe->dev->fd, DRM_MSM_GEM_SUBMIT,
			&req, sizeof(req));
	if (ret) {
		ERROR_MSG("submit failed: %d (%s)", ret, strerror(errno));
		dump_submit(msm_ring);
	} else if (!ret) {
		/* update timestamp on all rings associated with submit: */
		for (i = 0; i < msm_ring->submit.nr_cmds; i++) {
			struct msm_cmd *msm_cmd = msm_ring->cmds[i];
			msm_cmd->ring->last_timestamp = req.fence;
		}

		if (out_fence_fd) {
			*out_fence_fd = req.fence_fd;
		}
	}

	/* free dynamically constructed stateobj relocs tables: */
	for (i = 0; i < msm_ring->submit.nr_cmds; i++) {
		struct drm_msm_gem_submit_cmd *cmd = &msm_ring->submit.cmds[i];
		struct msm_cmd *msm_cmd = msm_ring->cmds[i];
		if (msm_cmd->ring->flags & FD_RINGBUFFER_OBJECT) {
			free(U642VOID(cmd->relocs));
		}
	}

	flush_reset(ring);

	return ret;
}

static void msm_ringbuffer_grow(struct fd_ringbuffer *ring, uint32_t size)
{
	assert(to_msm_ringbuffer(ring)->is_growable);
	finalize_current_cmd(ring, ring->last_start);
	ring_cmd_new(ring, size, 0);
}

static void msm_ringbuffer_reset(struct fd_ringbuffer *ring)
{
	flush_reset(ring);
}

static void msm_ringbuffer_emit_reloc(struct fd_ringbuffer *ring,
		const struct fd_reloc *r)
{
	struct fd_ringbuffer *parent = ring->parent ? ring->parent : ring;
	struct msm_bo *msm_bo = to_msm_bo(r->bo);
	struct drm_msm_gem_submit_reloc *reloc;
	struct msm_cmd *cmd = current_cmd(ring);
	uint32_t idx = APPEND(cmd, relocs);
	uint32_t addr;

	reloc = &cmd->relocs[idx];

	reloc->reloc_idx = bo2idx(parent, r->bo, r->flags);
	reloc->reloc_offset = r->offset;
	reloc->or = r->or;
	reloc->shift = r->shift;
	reloc->submit_offset = offset_bytes(ring->cur, ring->start) +
			to_msm_ringbuffer(ring)->offset;

	addr = msm_bo->presumed;
	if (reloc->shift < 0)
		addr >>= -reloc->shift;
	else
		addr <<= reloc->shift;
	(*ring->cur++) = addr | r->or;

	if (ring->pipe->gpu_id >= 500) {
		struct drm_msm_gem_submit_reloc *reloc_hi;

		/* NOTE: grab reloc_idx *before* APPEND() since that could
		 * realloc() meaning that 'reloc' ptr is no longer valid:
		 */
		uint32_t reloc_idx = reloc->reloc_idx;

		idx = APPEND(cmd, relocs);

		reloc_hi = &cmd->relocs[idx];

		reloc_hi->reloc_idx = reloc_idx;
		reloc_hi->reloc_offset = r->offset;
		reloc_hi->or = r->orhi;
		reloc_hi->shift = r->shift - 32;
		reloc_hi->submit_offset = offset_bytes(ring->cur, ring->start) +
				to_msm_ringbuffer(ring)->offset;

		addr = msm_bo->presumed >> 32;
		if (reloc_hi->shift < 0)
			addr >>= -reloc_hi->shift;
		else
			addr <<= reloc_hi->shift;
		(*ring->cur++) = addr | r->orhi;
	}
}

static uint32_t msm_ringbuffer_emit_reloc_ring(struct fd_ringbuffer *ring,
		struct fd_ringbuffer *target, uint32_t cmd_idx)
{
	struct msm_cmd *cmd = NULL;
	struct msm_ringbuffer *msm_target = to_msm_ringbuffer(target);
	uint32_t idx = 0;
	int added_cmd = FALSE;
	uint32_t size;
	uint32_t submit_offset = msm_target->offset;

	LIST_FOR_EACH_ENTRY(cmd, &msm_target->cmd_list, list) {
		if (idx == cmd_idx)
			break;
		idx++;
	}

	assert(cmd && (idx == cmd_idx));

	if (idx < (msm_target->cmd_count - 1)) {
		/* All but the last cmd buffer is fully "baked" (ie. already has
		 * done get_cmd() to add it to the cmds table).  But in this case,
		 * the size we get is invalid (since it is calculated from the
		 * last cmd buffer):
		 */
		size = cmd->size;
	} else {
		struct fd_ringbuffer *parent = ring->parent ? ring->parent : ring;
		size = offset_bytes(target->cur, target->start);
		added_cmd = get_cmd(parent, cmd, submit_offset, size,
				MSM_SUBMIT_CMD_IB_TARGET_BUF);
	}

	msm_ringbuffer_emit_reloc(ring, &(struct fd_reloc){
		.bo = cmd->ring_bo,
		.flags = FD_RELOC_READ,
		.offset = submit_offset,
	});

	/* Unlike traditional ringbuffers which are deleted as a set (after
	 * being flushed), mesa can't really guarantee that a stateobj isn't
	 * destroyed after emitted but before flush, so we must hold a ref:
	 */
	if (added_cmd && (target->flags & FD_RINGBUFFER_OBJECT)) {
		fd_ringbuffer_ref(target);
	}

	return size;
}

static uint32_t msm_ringbuffer_cmd_count(struct fd_ringbuffer *ring)
{
	return to_msm_ringbuffer(ring)->cmd_count;
}

static void msm_ringbuffer_destroy(struct fd_ringbuffer *ring)
{
	struct msm_ringbuffer *msm_ring = to_msm_ringbuffer(ring);

	flush_reset(ring);
	delete_cmds(msm_ring);

	free(msm_ring->submit.cmds);
	free(msm_ring->submit.bos);
	free(msm_ring->bos);
	free(msm_ring->cmds);
	free(msm_ring);
}

static const struct fd_ringbuffer_funcs funcs = {
		.hostptr = msm_ringbuffer_hostptr,
		.flush = msm_ringbuffer_flush,
		.grow = msm_ringbuffer_grow,
		.reset = msm_ringbuffer_reset,
		.emit_reloc = msm_ringbuffer_emit_reloc,
		.emit_reloc_ring = msm_ringbuffer_emit_reloc_ring,
		.cmd_count = msm_ringbuffer_cmd_count,
		.destroy = msm_ringbuffer_destroy,
};

drm_private struct fd_ringbuffer * msm_ringbuffer_new(struct fd_pipe *pipe,
		uint32_t size, enum fd_ringbuffer_flags flags)
{
	struct msm_ringbuffer *msm_ring;
	struct fd_ringbuffer *ring;

	msm_ring = calloc(1, sizeof(*msm_ring));
	if (!msm_ring) {
		ERROR_MSG("allocation failed");
		return NULL;
	}

	if (size == 0) {
		assert(pipe->dev->version >= FD_VERSION_UNLIMITED_CMDS);
		size = INIT_SIZE;
		msm_ring->is_growable = TRUE;
	}

	list_inithead(&msm_ring->cmd_list);
	msm_ring->seqno = ++to_msm_device(pipe->dev)->ring_cnt;

	ring = &msm_ring->base;
	atomic_set(&ring->refcnt, 1);

	ring->funcs = &funcs;
	ring->size = size;
	ring->pipe = pipe;   /* needed in ring_cmd_new() */

	ring_cmd_new(ring, size, flags);

	return ring;
}
