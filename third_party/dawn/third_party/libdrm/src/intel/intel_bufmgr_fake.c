/**************************************************************************
 * 
 * Copyright 2006 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/* Originally a fake version of the buffer manager so that we can
 * prototype the changes in a driver fairly quickly, has been fleshed
 * out to a fully functional interim solution.
 *
 * Basically wraps the old style memory management in the new
 * programming interface, but is more expressive and avoids many of
 * the bugs in the old texture manager.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <strings.h>
#include <xf86drm.h>
#include <pthread.h>
#include "intel_bufmgr.h"
#include "intel_bufmgr_priv.h"
#include "drm.h"
#include "i915_drm.h"
#include "mm.h"
#include "libdrm_macros.h"
#include "libdrm_lists.h"

#define DBG(...) do {					\
	if (bufmgr_fake->bufmgr.debug)			\
		drmMsg(__VA_ARGS__);			\
} while (0)

/* Internal flags:
 */
#define BM_NO_BACKING_STORE			0x00000001
#define BM_NO_FENCE_SUBDATA			0x00000002
#define BM_PINNED				0x00000004

/* Wrapper around mm.c's mem_block, which understands that you must
 * wait for fences to expire before memory can be freed.  This is
 * specific to our use of memcpy for uploads - an upload that was
 * processed through the command queue wouldn't need to care about
 * fences.
 */
#define MAX_RELOCS 4096

struct fake_buffer_reloc {
	/** Buffer object that the relocation points at. */
	drm_intel_bo *target_buf;
	/** Offset of the relocation entry within reloc_buf. */
	uint32_t offset;
	/**
	 * Cached value of the offset when we last performed this relocation.
	 */
	uint32_t last_target_offset;
	/** Value added to target_buf's offset to get the relocation entry. */
	uint32_t delta;
	/** Cache domains the target buffer is read into. */
	uint32_t read_domains;
	/** Cache domain the target buffer will have dirty cachelines in. */
	uint32_t write_domain;
};

struct block {
	struct block *next, *prev;
	struct mem_block *mem;	/* BM_MEM_AGP */

	/**
	 * Marks that the block is currently in the aperture and has yet to be
	 * fenced.
	 */
	unsigned on_hardware:1;
	/**
	 * Marks that the block is currently fenced (being used by rendering)
	 * and can't be freed until @fence is passed.
	 */
	unsigned fenced:1;

	/** Fence cookie for the block. */
	unsigned fence;		/* Split to read_fence, write_fence */

	drm_intel_bo *bo;
	void *virtual;
};

typedef struct _bufmgr_fake {
	drm_intel_bufmgr bufmgr;

	pthread_mutex_t lock;

	unsigned long low_offset;
	unsigned long size;
	void *virtual;

	struct mem_block *heap;

	unsigned buf_nr;	/* for generating ids */

	/**
	 * List of blocks which are currently in the GART but haven't been
	 * fenced yet.
	 */
	struct block on_hardware;
	/**
	 * List of blocks which are in the GART and have an active fence on
	 * them.
	 */
	struct block fenced;
	/**
	 * List of blocks which have an expired fence and are ready to be
	 * evicted.
	 */
	struct block lru;

	unsigned int last_fence;

	unsigned fail:1;
	unsigned need_fence:1;
	int thrashing;

	/**
	 * Driver callback to emit a fence, returning the cookie.
	 *
	 * This allows the driver to hook in a replacement for the DRM usage in
	 * bufmgr_fake.
	 *
	 * Currently, this also requires that a write flush be emitted before
	 * emitting the fence, but this should change.
	 */
	unsigned int (*fence_emit) (void *private);
	/** Driver callback to wait for a fence cookie to have passed. */
	void (*fence_wait) (unsigned int fence, void *private);
	void *fence_priv;

	/**
	 * Driver callback to execute a buffer.
	 *
	 * This allows the driver to hook in a replacement for the DRM usage in
	 * bufmgr_fake.
	 */
	int (*exec) (drm_intel_bo *bo, unsigned int used, void *priv);
	void *exec_priv;

	/** Driver-supplied argument to driver callbacks */
	void *driver_priv;
	/**
	 * Pointer to kernel-updated sarea data for the last completed user irq
	 */
	volatile int *last_dispatch;

	int fd;

	int debug;

	int performed_rendering;
} drm_intel_bufmgr_fake;

typedef struct _drm_intel_bo_fake {
	drm_intel_bo bo;

	unsigned id;		/* debug only */
	const char *name;

	unsigned dirty:1;
	/**
	 * has the card written to this buffer - we make need to copy it back
	 */
	unsigned card_dirty:1;
	unsigned int refcount;
	/* Flags may consist of any of the DRM_BO flags, plus
	 * DRM_BO_NO_BACKING_STORE and BM_NO_FENCE_SUBDATA, which are the
	 * first two driver private flags.
	 */
	uint64_t flags;
	/** Cache domains the target buffer is read into. */
	uint32_t read_domains;
	/** Cache domain the target buffer will have dirty cachelines in. */
	uint32_t write_domain;

	unsigned int alignment;
	int is_static, validated;
	unsigned int map_count;

	/** relocation list */
	struct fake_buffer_reloc *relocs;
	int nr_relocs;
	/**
	 * Total size of the target_bos of this buffer.
	 *
	 * Used for estimation in check_aperture.
	 */
	unsigned int child_size;

	struct block *block;
	void *backing_store;
	void (*invalidate_cb) (drm_intel_bo *bo, void *ptr);
	void *invalidate_ptr;
} drm_intel_bo_fake;

static int clear_fenced(drm_intel_bufmgr_fake *bufmgr_fake,
			unsigned int fence_cookie);

#define MAXFENCE 0x7fffffff

static int
FENCE_LTE(unsigned a, unsigned b)
{
	if (a == b)
		return 1;

	if (a < b && b - a < (1 << 24))
		return 1;

	if (a > b && MAXFENCE - a + b < (1 << 24))
		return 1;

	return 0;
}

drm_public void
drm_intel_bufmgr_fake_set_fence_callback(drm_intel_bufmgr *bufmgr,
					 unsigned int (*emit) (void *priv),
					 void (*wait) (unsigned int fence,
						       void *priv),
					 void *priv)
{
	drm_intel_bufmgr_fake *bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;

	bufmgr_fake->fence_emit = emit;
	bufmgr_fake->fence_wait = wait;
	bufmgr_fake->fence_priv = priv;
}

static unsigned int
_fence_emit_internal(drm_intel_bufmgr_fake *bufmgr_fake)
{
	struct drm_i915_irq_emit ie;
	int ret, seq = 1;

	if (bufmgr_fake->fence_emit != NULL) {
		seq = bufmgr_fake->fence_emit(bufmgr_fake->fence_priv);
		return seq;
	}

	ie.irq_seq = &seq;
	ret = drmCommandWriteRead(bufmgr_fake->fd, DRM_I915_IRQ_EMIT,
				  &ie, sizeof(ie));
	if (ret) {
		drmMsg("%s: drm_i915_irq_emit: %d\n", __func__, ret);
		abort();
	}

	DBG("emit 0x%08x\n", seq);
	return seq;
}

static void
_fence_wait_internal(drm_intel_bufmgr_fake *bufmgr_fake, int seq)
{
	struct drm_i915_irq_wait iw;
	int hw_seq, busy_count = 0;
	int ret;
	int kernel_lied;

	if (bufmgr_fake->fence_wait != NULL) {
		bufmgr_fake->fence_wait(seq, bufmgr_fake->fence_priv);
		clear_fenced(bufmgr_fake, seq);
		return;
	}

	iw.irq_seq = seq;

	DBG("wait 0x%08x\n", iw.irq_seq);

	/* The kernel IRQ_WAIT implementation is all sorts of broken.
	 * 1) It returns 1 to 0x7fffffff instead of using the full 32-bit
	 *    unsigned range.
	 * 2) It returns 0 if hw_seq >= seq, not seq - hw_seq < 0 on the 32-bit
	 *    signed range.
	 * 3) It waits if seq < hw_seq, not seq - hw_seq > 0 on the 32-bit
	 *    signed range.
	 * 4) It returns -EBUSY in 3 seconds even if the hardware is still
	 *    successfully chewing through buffers.
	 *
	 * Assume that in userland we treat sequence numbers as ints, which
	 * makes some of the comparisons convenient, since the sequence
	 * numbers are all positive signed integers.
	 *
	 * From this we get several cases we need to handle.  Here's a timeline.
	 * 0x2   0x7                                    0x7ffffff8   0x7ffffffd
	 *   |    |                                             |    |
	 * ------------------------------------------------------------
	 *
	 * A) Normal wait for hw to catch up
	 * hw_seq seq
	 *   |    |
	 * ------------------------------------------------------------
	 * seq - hw_seq = 5.  If we call IRQ_WAIT, it will wait for hw to
	 * catch up.
	 *
	 * B) Normal wait for a sequence number that's already passed.
	 * seq    hw_seq
	 *   |    |
	 * ------------------------------------------------------------
	 * seq - hw_seq = -5.  If we call IRQ_WAIT, it returns 0 quickly.
	 *
	 * C) Hardware has already wrapped around ahead of us
	 * hw_seq                                                    seq
	 *   |                                                       |
	 * ------------------------------------------------------------
	 * seq - hw_seq = 0x80000000 - 5.  If we called IRQ_WAIT, it would wait
	 * for hw_seq >= seq, which may never occur.  Thus, we want to catch
	 * this in userland and return 0.
	 *
	 * D) We've wrapped around ahead of the hardware.
	 * seq                                                      hw_seq
	 *   |                                                       |
	 * ------------------------------------------------------------
	 * seq - hw_seq = -(0x80000000 - 5).  If we called IRQ_WAIT, it would
	 * return 0 quickly because hw_seq >= seq, even though the hardware
	 * isn't caught up. Thus, we need to catch this early return in
	 * userland and bother the kernel until the hardware really does
	 * catch up.
	 *
	 * E) Hardware might wrap after we test in userland.
	 *                                                  hw_seq  seq
	 *                                                      |    |
	 * ------------------------------------------------------------
	 * seq - hw_seq = 5.  If we call IRQ_WAIT, it will likely see seq >=
	 * hw_seq and wait.  However, suppose hw_seq wraps before we make it
	 * into the kernel.  The kernel sees hw_seq >= seq and waits for 3
	 * seconds then returns -EBUSY.  This is case C).  We should catch
	 * this and then return successfully.
	 *
	 * F) Hardware might take a long time on a buffer.
	 * hw_seq seq
	 *   |    |
	 * -------------------------------------------------------------------
	 * seq - hw_seq = 5.  If we call IRQ_WAIT, if sequence 2 through 5
	 * take too long, it will return -EBUSY.  Batchbuffers in the
	 * gltestperf demo were seen to take up to 7 seconds.  We should
	 * catch early -EBUSY return and keep trying.
	 */

	do {
		/* Keep a copy of last_dispatch so that if the wait -EBUSYs
		 * because the hardware didn't catch up in 3 seconds, we can
		 * see if it at least made progress and retry.
		 */
		hw_seq = *bufmgr_fake->last_dispatch;

		/* Catch case C */
		if (seq - hw_seq > 0x40000000)
			return;

		ret = drmCommandWrite(bufmgr_fake->fd, DRM_I915_IRQ_WAIT,
				      &iw, sizeof(iw));
		/* Catch case D */
		kernel_lied = (ret == 0) && (seq - *bufmgr_fake->last_dispatch <
					     -0x40000000);

		/* Catch case E */
		if (ret == -EBUSY
		    && (seq - *bufmgr_fake->last_dispatch > 0x40000000))
			ret = 0;

		/* Catch case F: Allow up to 15 seconds chewing on one buffer. */
		if ((ret == -EBUSY) && (hw_seq != *bufmgr_fake->last_dispatch))
			busy_count = 0;
		else
			busy_count++;
	} while (kernel_lied || ret == -EAGAIN || ret == -EINTR ||
		 (ret == -EBUSY && busy_count < 5));

	if (ret != 0) {
		drmMsg("%s:%d: Error waiting for fence: %s.\n", __FILE__,
		       __LINE__, strerror(-ret));
		abort();
	}
	clear_fenced(bufmgr_fake, seq);
}

static int
_fence_test(drm_intel_bufmgr_fake *bufmgr_fake, unsigned fence)
{
	/* Slight problem with wrap-around:
	 */
	return fence == 0 || FENCE_LTE(fence, bufmgr_fake->last_fence);
}

/**
 * Allocate a memory manager block for the buffer.
 */
static int
alloc_block(drm_intel_bo *bo)
{
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	struct block *block = (struct block *)calloc(sizeof *block, 1);
	unsigned int align_log2 = ffs(bo_fake->alignment) - 1;
	unsigned int sz;

	if (!block)
		return 1;

	sz = (bo->size + bo_fake->alignment - 1) & ~(bo_fake->alignment - 1);

	block->mem = mmAllocMem(bufmgr_fake->heap, sz, align_log2, 0);
	if (!block->mem) {
		free(block);
		return 0;
	}

	DRMINITLISTHEAD(block);

	/* Insert at head or at tail??? */
	DRMLISTADDTAIL(block, &bufmgr_fake->lru);

	block->virtual = (uint8_t *) bufmgr_fake->virtual +
	    block->mem->ofs - bufmgr_fake->low_offset;
	block->bo = bo;

	bo_fake->block = block;

	return 1;
}

/* Release the card storage associated with buf:
 */
static void
free_block(drm_intel_bufmgr_fake *bufmgr_fake, struct block *block,
	   int skip_dirty_copy)
{
	drm_intel_bo_fake *bo_fake;
	DBG("free block %p %08x %d %d\n", block, block->mem->ofs,
	    block->on_hardware, block->fenced);

	if (!block)
		return;

	bo_fake = (drm_intel_bo_fake *) block->bo;

	if (bo_fake->flags & (BM_PINNED | BM_NO_BACKING_STORE))
		skip_dirty_copy = 1;

	if (!skip_dirty_copy && (bo_fake->card_dirty == 1)) {
		memcpy(bo_fake->backing_store, block->virtual, block->bo->size);
		bo_fake->card_dirty = 0;
		bo_fake->dirty = 1;
	}

	if (block->on_hardware) {
		block->bo = NULL;
	} else if (block->fenced) {
		block->bo = NULL;
	} else {
		DBG("    - free immediately\n");
		DRMLISTDEL(block);

		mmFreeMem(block->mem);
		free(block);
	}
}

static void
alloc_backing_store(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;
	assert(!bo_fake->backing_store);
	assert(!(bo_fake->flags & (BM_PINNED | BM_NO_BACKING_STORE)));

	bo_fake->backing_store = malloc(bo->size);

	DBG("alloc_backing - buf %d %p %lu\n", bo_fake->id,
	    bo_fake->backing_store, bo->size);
	assert(bo_fake->backing_store);
}

static void
free_backing_store(drm_intel_bo *bo)
{
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	if (bo_fake->backing_store) {
		assert(!(bo_fake->flags & (BM_PINNED | BM_NO_BACKING_STORE)));
		free(bo_fake->backing_store);
		bo_fake->backing_store = NULL;
	}
}

static void
set_dirty(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	if (bo_fake->flags & BM_NO_BACKING_STORE
	    && bo_fake->invalidate_cb != NULL)
		bo_fake->invalidate_cb(bo, bo_fake->invalidate_ptr);

	assert(!(bo_fake->flags & BM_PINNED));

	DBG("set_dirty - buf %d\n", bo_fake->id);
	bo_fake->dirty = 1;
}

static int
evict_lru(drm_intel_bufmgr_fake *bufmgr_fake, unsigned int max_fence)
{
	struct block *block, *tmp;

	DBG("%s\n", __func__);

	DRMLISTFOREACHSAFE(block, tmp, &bufmgr_fake->lru) {
		drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) block->bo;

		if (bo_fake != NULL && (bo_fake->flags & BM_NO_FENCE_SUBDATA))
			continue;

		if (block->fence && max_fence && !FENCE_LTE(block->fence,
							    max_fence))
			return 0;

		set_dirty(&bo_fake->bo);
		bo_fake->block = NULL;

		free_block(bufmgr_fake, block, 0);
		return 1;
	}

	return 0;
}

static int
evict_mru(drm_intel_bufmgr_fake *bufmgr_fake)
{
	struct block *block, *tmp;

	DBG("%s\n", __func__);

	DRMLISTFOREACHSAFEREVERSE(block, tmp, &bufmgr_fake->lru) {
		drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) block->bo;

		if (bo_fake && (bo_fake->flags & BM_NO_FENCE_SUBDATA))
			continue;

		set_dirty(&bo_fake->bo);
		bo_fake->block = NULL;

		free_block(bufmgr_fake, block, 0);
		return 1;
	}

	return 0;
}

/**
 * Removes all objects from the fenced list older than the given fence.
 */
static int
clear_fenced(drm_intel_bufmgr_fake *bufmgr_fake, unsigned int fence_cookie)
{
	struct block *block, *tmp;
	int ret = 0;

	bufmgr_fake->last_fence = fence_cookie;
	DRMLISTFOREACHSAFE(block, tmp, &bufmgr_fake->fenced) {
		assert(block->fenced);

		if (_fence_test(bufmgr_fake, block->fence)) {

			block->fenced = 0;

			if (!block->bo) {
				DBG("delayed free: offset %x sz %x\n",
				    block->mem->ofs, block->mem->size);
				DRMLISTDEL(block);
				mmFreeMem(block->mem);
				free(block);
			} else {
				DBG("return to lru: offset %x sz %x\n",
				    block->mem->ofs, block->mem->size);
				DRMLISTDEL(block);
				DRMLISTADDTAIL(block, &bufmgr_fake->lru);
			}

			ret = 1;
		} else {
			/* Blocks are ordered by fence, so if one fails, all
			 * from here will fail also:
			 */
			DBG("fence not passed: offset %x sz %x %d %d \n",
			    block->mem->ofs, block->mem->size, block->fence,
			    bufmgr_fake->last_fence);
			break;
		}
	}

	DBG("%s: %d\n", __func__, ret);
	return ret;
}

static void
fence_blocks(drm_intel_bufmgr_fake *bufmgr_fake, unsigned fence)
{
	struct block *block, *tmp;

	DRMLISTFOREACHSAFE(block, tmp, &bufmgr_fake->on_hardware) {
		DBG("Fence block %p (sz 0x%x ofs %x buf %p) with fence %d\n",
		    block, block->mem->size, block->mem->ofs, block->bo, fence);
		block->fence = fence;

		block->on_hardware = 0;
		block->fenced = 1;

		/* Move to tail of pending list here
		 */
		DRMLISTDEL(block);
		DRMLISTADDTAIL(block, &bufmgr_fake->fenced);
	}

	assert(DRMLISTEMPTY(&bufmgr_fake->on_hardware));
}

static int
evict_and_alloc_block(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	assert(bo_fake->block == NULL);

	/* Search for already free memory:
	 */
	if (alloc_block(bo))
		return 1;

	/* If we're not thrashing, allow lru eviction to dig deeper into
	 * recently used textures.  We'll probably be thrashing soon:
	 */
	if (!bufmgr_fake->thrashing) {
		while (evict_lru(bufmgr_fake, 0))
			if (alloc_block(bo))
				return 1;
	}

	/* Keep thrashing counter alive?
	 */
	if (bufmgr_fake->thrashing)
		bufmgr_fake->thrashing = 20;

	/* Wait on any already pending fences - here we are waiting for any
	 * freed memory that has been submitted to hardware and fenced to
	 * become available:
	 */
	while (!DRMLISTEMPTY(&bufmgr_fake->fenced)) {
		uint32_t fence = bufmgr_fake->fenced.next->fence;
		_fence_wait_internal(bufmgr_fake, fence);

		if (alloc_block(bo))
			return 1;
	}

	if (!DRMLISTEMPTY(&bufmgr_fake->on_hardware)) {
		while (!DRMLISTEMPTY(&bufmgr_fake->fenced)) {
			uint32_t fence = bufmgr_fake->fenced.next->fence;
			_fence_wait_internal(bufmgr_fake, fence);
		}

		if (!bufmgr_fake->thrashing) {
			DBG("thrashing\n");
		}
		bufmgr_fake->thrashing = 20;

		if (alloc_block(bo))
			return 1;
	}

	while (evict_mru(bufmgr_fake))
		if (alloc_block(bo))
			return 1;

	DBG("%s 0x%lx bytes failed\n", __func__, bo->size);

	return 0;
}

/***********************************************************************
 * Public functions
 */

/**
 * Wait for hardware idle by emitting a fence and waiting for it.
 */
static void
drm_intel_bufmgr_fake_wait_idle(drm_intel_bufmgr_fake *bufmgr_fake)
{
	unsigned int cookie;

	cookie = _fence_emit_internal(bufmgr_fake);
	_fence_wait_internal(bufmgr_fake, cookie);
}

/**
 * Wait for rendering to a buffer to complete.
 *
 * It is assumed that the batchbuffer which performed the rendering included
 * the necessary flushing.
 */
static void
drm_intel_fake_bo_wait_rendering_locked(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	if (bo_fake->block == NULL || !bo_fake->block->fenced)
		return;

	_fence_wait_internal(bufmgr_fake, bo_fake->block->fence);
}

static void
drm_intel_fake_bo_wait_rendering(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;

	pthread_mutex_lock(&bufmgr_fake->lock);
	drm_intel_fake_bo_wait_rendering_locked(bo);
	pthread_mutex_unlock(&bufmgr_fake->lock);
}

/* Specifically ignore texture memory sharing.
 *  -- just evict everything
 *  -- and wait for idle
 */
drm_public void
drm_intel_bufmgr_fake_contended_lock_take(drm_intel_bufmgr *bufmgr)
{
	drm_intel_bufmgr_fake *bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;
	struct block *block, *tmp;

	pthread_mutex_lock(&bufmgr_fake->lock);

	bufmgr_fake->need_fence = 1;
	bufmgr_fake->fail = 0;

	/* Wait for hardware idle.  We don't know where acceleration has been
	 * happening, so we'll need to wait anyway before letting anything get
	 * put on the card again.
	 */
	drm_intel_bufmgr_fake_wait_idle(bufmgr_fake);

	/* Check that we hadn't released the lock without having fenced the last
	 * set of buffers.
	 */
	assert(DRMLISTEMPTY(&bufmgr_fake->fenced));
	assert(DRMLISTEMPTY(&bufmgr_fake->on_hardware));

	DRMLISTFOREACHSAFE(block, tmp, &bufmgr_fake->lru) {
		assert(_fence_test(bufmgr_fake, block->fence));
		set_dirty(block->bo);
	}

	pthread_mutex_unlock(&bufmgr_fake->lock);
}

static drm_intel_bo *
drm_intel_fake_bo_alloc(drm_intel_bufmgr *bufmgr,
			const char *name,
			unsigned long size,
			unsigned int alignment)
{
	drm_intel_bufmgr_fake *bufmgr_fake;
	drm_intel_bo_fake *bo_fake;

	bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;

	assert(size != 0);

	bo_fake = calloc(1, sizeof(*bo_fake));
	if (!bo_fake)
		return NULL;

	bo_fake->bo.size = size;
	bo_fake->bo.offset = -1;
	bo_fake->bo.virtual = NULL;
	bo_fake->bo.bufmgr = bufmgr;
	bo_fake->refcount = 1;

	/* Alignment must be a power of two */
	assert((alignment & (alignment - 1)) == 0);
	if (alignment == 0)
		alignment = 1;
	bo_fake->alignment = alignment;
	bo_fake->id = ++bufmgr_fake->buf_nr;
	bo_fake->name = name;
	bo_fake->flags = 0;
	bo_fake->is_static = 0;

	DBG("drm_bo_alloc: (buf %d: %s, %lu kb)\n", bo_fake->id, bo_fake->name,
	    bo_fake->bo.size / 1024);

	return &bo_fake->bo;
}

static drm_intel_bo *
drm_intel_fake_bo_alloc_tiled(drm_intel_bufmgr * bufmgr,
			      const char *name,
			      int x, int y, int cpp,
			      uint32_t *tiling_mode,
			      unsigned long *pitch,
			      unsigned long flags)
{
	unsigned long stride, aligned_y;

	/* No runtime tiling support for fake. */
	*tiling_mode = I915_TILING_NONE;

	/* Align it for being a render target.  Shouldn't need anything else. */
	stride = x * cpp;
	stride = ROUND_UP_TO(stride, 64);

	/* 965 subspan loading alignment */
	aligned_y = ALIGN(y, 2);

	*pitch = stride;

	return drm_intel_fake_bo_alloc(bufmgr, name, stride * aligned_y,
				       4096);
}

drm_public drm_intel_bo *
drm_intel_bo_fake_alloc_static(drm_intel_bufmgr *bufmgr,
			       const char *name,
			       unsigned long offset,
			       unsigned long size, void *virtual)
{
	drm_intel_bufmgr_fake *bufmgr_fake;
	drm_intel_bo_fake *bo_fake;

	bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;

	assert(size != 0);

	bo_fake = calloc(1, sizeof(*bo_fake));
	if (!bo_fake)
		return NULL;

	bo_fake->bo.size = size;
	bo_fake->bo.offset = offset;
	bo_fake->bo.virtual = virtual;
	bo_fake->bo.bufmgr = bufmgr;
	bo_fake->refcount = 1;
	bo_fake->id = ++bufmgr_fake->buf_nr;
	bo_fake->name = name;
	bo_fake->flags = BM_PINNED;
	bo_fake->is_static = 1;

	DBG("drm_bo_alloc_static: (buf %d: %s, %lu kb)\n", bo_fake->id,
	    bo_fake->name, bo_fake->bo.size / 1024);

	return &bo_fake->bo;
}

static void
drm_intel_fake_bo_reference(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	pthread_mutex_lock(&bufmgr_fake->lock);
	bo_fake->refcount++;
	pthread_mutex_unlock(&bufmgr_fake->lock);
}

static void
drm_intel_fake_bo_reference_locked(drm_intel_bo *bo)
{
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	bo_fake->refcount++;
}

static void
drm_intel_fake_bo_unreference_locked(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;
	int i;

	if (--bo_fake->refcount == 0) {
		assert(bo_fake->map_count == 0);
		/* No remaining references, so free it */
		if (bo_fake->block)
			free_block(bufmgr_fake, bo_fake->block, 1);
		free_backing_store(bo);

		for (i = 0; i < bo_fake->nr_relocs; i++)
			drm_intel_fake_bo_unreference_locked(bo_fake->relocs[i].
							     target_buf);

		DBG("drm_bo_unreference: free buf %d %s\n", bo_fake->id,
		    bo_fake->name);

		free(bo_fake->relocs);
		free(bo);
	}
}

static void
drm_intel_fake_bo_unreference(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;

	pthread_mutex_lock(&bufmgr_fake->lock);
	drm_intel_fake_bo_unreference_locked(bo);
	pthread_mutex_unlock(&bufmgr_fake->lock);
}

/**
 * Set the buffer as not requiring backing store, and instead get the callback
 * invoked whenever it would be set dirty.
 */
drm_public void
drm_intel_bo_fake_disable_backing_store(drm_intel_bo *bo,
					void (*invalidate_cb) (drm_intel_bo *bo,
							       void *ptr),
					void *ptr)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	pthread_mutex_lock(&bufmgr_fake->lock);

	if (bo_fake->backing_store)
		free_backing_store(bo);

	bo_fake->flags |= BM_NO_BACKING_STORE;

	DBG("disable_backing_store set buf %d dirty\n", bo_fake->id);
	bo_fake->dirty = 1;
	bo_fake->invalidate_cb = invalidate_cb;
	bo_fake->invalidate_ptr = ptr;

	/* Note that it is invalid right from the start.  Also note
	 * invalidate_cb is called with the bufmgr locked, so cannot
	 * itself make bufmgr calls.
	 */
	if (invalidate_cb != NULL)
		invalidate_cb(bo, ptr);

	pthread_mutex_unlock(&bufmgr_fake->lock);
}

/**
 * Map a buffer into bo->virtual, allocating either card memory space (If
 * BM_NO_BACKING_STORE or BM_PINNED) or backing store, as necessary.
 */
static int
 drm_intel_fake_bo_map_locked(drm_intel_bo *bo, int write_enable)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	/* Static buffers are always mapped. */
	if (bo_fake->is_static) {
		if (bo_fake->card_dirty) {
			drm_intel_bufmgr_fake_wait_idle(bufmgr_fake);
			bo_fake->card_dirty = 0;
		}
		return 0;
	}

	/* Allow recursive mapping.  Mesa may recursively map buffers with
	 * nested display loops, and it is used internally in bufmgr_fake
	 * for relocation.
	 */
	if (bo_fake->map_count++ != 0)
		return 0;

	{
		DBG("drm_bo_map: (buf %d: %s, %lu kb)\n", bo_fake->id,
		    bo_fake->name, bo_fake->bo.size / 1024);

		if (bo->virtual != NULL) {
			drmMsg("%s: already mapped\n", __func__);
			abort();
		} else if (bo_fake->flags & (BM_NO_BACKING_STORE | BM_PINNED)) {

			if (!bo_fake->block && !evict_and_alloc_block(bo)) {
				DBG("%s: alloc failed\n", __func__);
				bufmgr_fake->fail = 1;
				return 1;
			} else {
				assert(bo_fake->block);
				bo_fake->dirty = 0;

				if (!(bo_fake->flags & BM_NO_FENCE_SUBDATA) &&
				    bo_fake->block->fenced) {
					drm_intel_fake_bo_wait_rendering_locked
					    (bo);
				}

				bo->virtual = bo_fake->block->virtual;
			}
		} else {
			if (write_enable)
				set_dirty(bo);

			if (bo_fake->backing_store == 0)
				alloc_backing_store(bo);

			if ((bo_fake->card_dirty == 1) && bo_fake->block) {
				if (bo_fake->block->fenced)
					drm_intel_fake_bo_wait_rendering_locked
					    (bo);

				memcpy(bo_fake->backing_store,
				       bo_fake->block->virtual,
				       bo_fake->block->bo->size);
				bo_fake->card_dirty = 0;
			}

			bo->virtual = bo_fake->backing_store;
		}
	}

	return 0;
}

static int
 drm_intel_fake_bo_map(drm_intel_bo *bo, int write_enable)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	int ret;

	pthread_mutex_lock(&bufmgr_fake->lock);
	ret = drm_intel_fake_bo_map_locked(bo, write_enable);
	pthread_mutex_unlock(&bufmgr_fake->lock);

	return ret;
}

static int
 drm_intel_fake_bo_unmap_locked(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	/* Static buffers are always mapped. */
	if (bo_fake->is_static)
		return 0;

	assert(bo_fake->map_count != 0);
	if (--bo_fake->map_count != 0)
		return 0;

	DBG("drm_bo_unmap: (buf %d: %s, %lu kb)\n", bo_fake->id, bo_fake->name,
	    bo_fake->bo.size / 1024);

	bo->virtual = NULL;

	return 0;
}

static int drm_intel_fake_bo_unmap(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	int ret;

	pthread_mutex_lock(&bufmgr_fake->lock);
	ret = drm_intel_fake_bo_unmap_locked(bo);
	pthread_mutex_unlock(&bufmgr_fake->lock);

	return ret;
}

static int
drm_intel_fake_bo_subdata(drm_intel_bo *bo, unsigned long offset,
			  unsigned long size, const void *data)
{
	int ret;

	if (size == 0 || data == NULL)
		return 0;

	ret = drm_intel_bo_map(bo, 1);
	if (ret)
		return ret;
	memcpy((unsigned char *)bo->virtual + offset, data, size);
	drm_intel_bo_unmap(bo);
	return 0;
}

static void
 drm_intel_fake_kick_all_locked(drm_intel_bufmgr_fake *bufmgr_fake)
{
	struct block *block, *tmp;

	bufmgr_fake->performed_rendering = 0;
	/* okay for ever BO that is on the HW kick it off.
	   seriously not afraid of the POLICE right now */
	DRMLISTFOREACHSAFE(block, tmp, &bufmgr_fake->on_hardware) {
		drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) block->bo;

		block->on_hardware = 0;
		free_block(bufmgr_fake, block, 0);
		bo_fake->block = NULL;
		bo_fake->validated = 0;
		if (!(bo_fake->flags & BM_NO_BACKING_STORE))
			bo_fake->dirty = 1;
	}

}

static int
 drm_intel_fake_bo_validate(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;

	bufmgr_fake = (drm_intel_bufmgr_fake *) bo->bufmgr;

	DBG("drm_bo_validate: (buf %d: %s, %lu kb)\n", bo_fake->id,
	    bo_fake->name, bo_fake->bo.size / 1024);

	/* Sanity check: Buffers should be unmapped before being validated.
	 * This is not so much of a problem for bufmgr_fake, but TTM refuses,
	 * and the problem is harder to debug there.
	 */
	assert(bo_fake->map_count == 0);

	if (bo_fake->is_static) {
		/* Add it to the needs-fence list */
		bufmgr_fake->need_fence = 1;
		return 0;
	}

	/* Allocate the card memory */
	if (!bo_fake->block && !evict_and_alloc_block(bo)) {
		bufmgr_fake->fail = 1;
		DBG("Failed to validate buf %d:%s\n", bo_fake->id,
		    bo_fake->name);
		return -1;
	}

	assert(bo_fake->block);
	assert(bo_fake->block->bo == &bo_fake->bo);

	bo->offset = bo_fake->block->mem->ofs;

	/* Upload the buffer contents if necessary */
	if (bo_fake->dirty) {
		DBG("Upload dirty buf %d:%s, sz %lu offset 0x%x\n", bo_fake->id,
		    bo_fake->name, bo->size, bo_fake->block->mem->ofs);

		assert(!(bo_fake->flags & (BM_NO_BACKING_STORE | BM_PINNED)));

		/* Actually, should be able to just wait for a fence on the
		 * memory, which we would be tracking when we free it. Waiting
		 * for idle is a sufficiently large hammer for now.
		 */
		drm_intel_bufmgr_fake_wait_idle(bufmgr_fake);

		/* we may never have mapped this BO so it might not have any
		 * backing store if this happens it should be rare, but 0 the
		 * card memory in any case */
		if (bo_fake->backing_store)
			memcpy(bo_fake->block->virtual, bo_fake->backing_store,
			       bo->size);
		else
			memset(bo_fake->block->virtual, 0, bo->size);

		bo_fake->dirty = 0;
	}

	bo_fake->block->fenced = 0;
	bo_fake->block->on_hardware = 1;
	DRMLISTDEL(bo_fake->block);
	DRMLISTADDTAIL(bo_fake->block, &bufmgr_fake->on_hardware);

	bo_fake->validated = 1;
	bufmgr_fake->need_fence = 1;

	return 0;
}

static void
drm_intel_fake_fence_validated(drm_intel_bufmgr *bufmgr)
{
	drm_intel_bufmgr_fake *bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;
	unsigned int cookie;

	cookie = _fence_emit_internal(bufmgr_fake);
	fence_blocks(bufmgr_fake, cookie);

	DBG("drm_fence_validated: 0x%08x cookie\n", cookie);
}

static void
drm_intel_fake_destroy(drm_intel_bufmgr *bufmgr)
{
	drm_intel_bufmgr_fake *bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;

	pthread_mutex_destroy(&bufmgr_fake->lock);
	mmDestroy(bufmgr_fake->heap);
	free(bufmgr);
}

static int
drm_intel_fake_emit_reloc(drm_intel_bo *bo, uint32_t offset,
			  drm_intel_bo *target_bo, uint32_t target_offset,
			  uint32_t read_domains, uint32_t write_domain)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	struct fake_buffer_reloc *r;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;
	drm_intel_bo_fake *target_fake = (drm_intel_bo_fake *) target_bo;
	int i;

	pthread_mutex_lock(&bufmgr_fake->lock);

	assert(bo);
	assert(target_bo);

	if (bo_fake->relocs == NULL) {
		bo_fake->relocs =
		    malloc(sizeof(struct fake_buffer_reloc) * MAX_RELOCS);
	}

	r = &bo_fake->relocs[bo_fake->nr_relocs++];

	assert(bo_fake->nr_relocs <= MAX_RELOCS);

	drm_intel_fake_bo_reference_locked(target_bo);

	if (!target_fake->is_static) {
		bo_fake->child_size +=
		    ALIGN(target_bo->size, target_fake->alignment);
		bo_fake->child_size += target_fake->child_size;
	}
	r->target_buf = target_bo;
	r->offset = offset;
	r->last_target_offset = target_bo->offset;
	r->delta = target_offset;
	r->read_domains = read_domains;
	r->write_domain = write_domain;

	if (bufmgr_fake->debug) {
		/* Check that a conflicting relocation hasn't already been
		 * emitted.
		 */
		for (i = 0; i < bo_fake->nr_relocs - 1; i++) {
			struct fake_buffer_reloc *r2 = &bo_fake->relocs[i];

			assert(r->offset != r2->offset);
		}
	}

	pthread_mutex_unlock(&bufmgr_fake->lock);

	return 0;
}

/**
 * Incorporates the validation flags associated with each relocation into
 * the combined validation flags for the buffer on this batchbuffer submission.
 */
static void
drm_intel_fake_calculate_domains(drm_intel_bo *bo)
{
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;
	int i;

	for (i = 0; i < bo_fake->nr_relocs; i++) {
		struct fake_buffer_reloc *r = &bo_fake->relocs[i];
		drm_intel_bo_fake *target_fake =
		    (drm_intel_bo_fake *) r->target_buf;

		/* Do the same for the tree of buffers we depend on */
		drm_intel_fake_calculate_domains(r->target_buf);

		target_fake->read_domains |= r->read_domains;
		target_fake->write_domain |= r->write_domain;
	}
}

static int
drm_intel_fake_reloc_and_validate_buffer(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;
	int i, ret;

	assert(bo_fake->map_count == 0);

	for (i = 0; i < bo_fake->nr_relocs; i++) {
		struct fake_buffer_reloc *r = &bo_fake->relocs[i];
		drm_intel_bo_fake *target_fake =
		    (drm_intel_bo_fake *) r->target_buf;
		uint32_t reloc_data;

		/* Validate the target buffer if that hasn't been done. */
		if (!target_fake->validated) {
			ret =
			    drm_intel_fake_reloc_and_validate_buffer(r->target_buf);
			if (ret != 0) {
				if (bo->virtual != NULL)
					drm_intel_fake_bo_unmap_locked(bo);
				return ret;
			}
		}

		/* Calculate the value of the relocation entry. */
		if (r->target_buf->offset != r->last_target_offset) {
			reloc_data = r->target_buf->offset + r->delta;

			if (bo->virtual == NULL)
				drm_intel_fake_bo_map_locked(bo, 1);

			*(uint32_t *) ((uint8_t *) bo->virtual + r->offset) =
			    reloc_data;

			r->last_target_offset = r->target_buf->offset;
		}
	}

	if (bo->virtual != NULL)
		drm_intel_fake_bo_unmap_locked(bo);

	if (bo_fake->write_domain != 0) {
		if (!(bo_fake->flags & (BM_NO_BACKING_STORE | BM_PINNED))) {
			if (bo_fake->backing_store == 0)
				alloc_backing_store(bo);
		}
		bo_fake->card_dirty = 1;
		bufmgr_fake->performed_rendering = 1;
	}

	return drm_intel_fake_bo_validate(bo);
}

static void
drm_intel_bo_fake_post_submit(drm_intel_bo *bo)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo;
	int i;

	for (i = 0; i < bo_fake->nr_relocs; i++) {
		struct fake_buffer_reloc *r = &bo_fake->relocs[i];
		drm_intel_bo_fake *target_fake =
		    (drm_intel_bo_fake *) r->target_buf;

		if (target_fake->validated)
			drm_intel_bo_fake_post_submit(r->target_buf);

		DBG("%s@0x%08x + 0x%08x -> %s@0x%08x + 0x%08x\n",
		    bo_fake->name, (uint32_t) bo->offset, r->offset,
		    target_fake->name, (uint32_t) r->target_buf->offset,
		    r->delta);
	}

	assert(bo_fake->map_count == 0);
	bo_fake->validated = 0;
	bo_fake->read_domains = 0;
	bo_fake->write_domain = 0;
}

drm_public void
drm_intel_bufmgr_fake_set_exec_callback(drm_intel_bufmgr *bufmgr,
					     int (*exec) (drm_intel_bo *bo,
							  unsigned int used,
							  void *priv),
					     void *priv)
{
	drm_intel_bufmgr_fake *bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;

	bufmgr_fake->exec = exec;
	bufmgr_fake->exec_priv = priv;
}

static int
drm_intel_fake_bo_exec(drm_intel_bo *bo, int used,
		       drm_clip_rect_t * cliprects, int num_cliprects, int DR4)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo->bufmgr;
	drm_intel_bo_fake *batch_fake = (drm_intel_bo_fake *) bo;
	struct drm_i915_batchbuffer batch;
	int ret;
	int retry_count = 0;

	pthread_mutex_lock(&bufmgr_fake->lock);

	bufmgr_fake->performed_rendering = 0;

	drm_intel_fake_calculate_domains(bo);

	batch_fake->read_domains = I915_GEM_DOMAIN_COMMAND;

	/* we've ran out of RAM so blow the whole lot away and retry */
restart:
	ret = drm_intel_fake_reloc_and_validate_buffer(bo);
	if (bufmgr_fake->fail == 1) {
		if (retry_count == 0) {
			retry_count++;
			drm_intel_fake_kick_all_locked(bufmgr_fake);
			bufmgr_fake->fail = 0;
			goto restart;
		} else		/* dump out the memory here */
			mmDumpMemInfo(bufmgr_fake->heap);
	}

	assert(ret == 0);

	if (bufmgr_fake->exec != NULL) {
		ret = bufmgr_fake->exec(bo, used, bufmgr_fake->exec_priv);
		if (ret != 0) {
			pthread_mutex_unlock(&bufmgr_fake->lock);
			return ret;
		}
	} else {
		batch.start = bo->offset;
		batch.used = used;
		batch.cliprects = cliprects;
		batch.num_cliprects = num_cliprects;
		batch.DR1 = 0;
		batch.DR4 = DR4;

		if (drmCommandWrite
		    (bufmgr_fake->fd, DRM_I915_BATCHBUFFER, &batch,
		     sizeof(batch))) {
			drmMsg("DRM_I915_BATCHBUFFER: %d\n", -errno);
			pthread_mutex_unlock(&bufmgr_fake->lock);
			return -errno;
		}
	}

	drm_intel_fake_fence_validated(bo->bufmgr);

	drm_intel_bo_fake_post_submit(bo);

	pthread_mutex_unlock(&bufmgr_fake->lock);

	return 0;
}

/**
 * Return an error if the list of BOs will exceed the aperture size.
 *
 * This is a rough guess and likely to fail, as during the validate sequence we
 * may place a buffer in an inopportune spot early on and then fail to fit
 * a set smaller than the aperture.
 */
static int
drm_intel_fake_check_aperture_space(drm_intel_bo ** bo_array, int count)
{
	drm_intel_bufmgr_fake *bufmgr_fake =
	    (drm_intel_bufmgr_fake *) bo_array[0]->bufmgr;
	unsigned int sz = 0;
	int i;

	for (i = 0; i < count; i++) {
		drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) bo_array[i];

		if (bo_fake == NULL)
			continue;

		if (!bo_fake->is_static)
			sz += ALIGN(bo_array[i]->size, bo_fake->alignment);
		sz += bo_fake->child_size;
	}

	if (sz > bufmgr_fake->size) {
		DBG("check_space: overflowed bufmgr size, %ukb vs %lukb\n",
		    sz / 1024, bufmgr_fake->size / 1024);
		return -1;
	}

	DBG("drm_check_space: sz %ukb vs bufgr %lukb\n", sz / 1024,
	    bufmgr_fake->size / 1024);
	return 0;
}

/**
 * Evicts all buffers, waiting for fences to pass and copying contents out
 * as necessary.
 *
 * Used by the X Server on LeaveVT, when the card memory is no longer our
 * own.
 */
drm_public void
drm_intel_bufmgr_fake_evict_all(drm_intel_bufmgr *bufmgr)
{
	drm_intel_bufmgr_fake *bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;
	struct block *block, *tmp;

	pthread_mutex_lock(&bufmgr_fake->lock);

	bufmgr_fake->need_fence = 1;
	bufmgr_fake->fail = 0;

	/* Wait for hardware idle.  We don't know where acceleration has been
	 * happening, so we'll need to wait anyway before letting anything get
	 * put on the card again.
	 */
	drm_intel_bufmgr_fake_wait_idle(bufmgr_fake);

	/* Check that we hadn't released the lock without having fenced the last
	 * set of buffers.
	 */
	assert(DRMLISTEMPTY(&bufmgr_fake->fenced));
	assert(DRMLISTEMPTY(&bufmgr_fake->on_hardware));

	DRMLISTFOREACHSAFE(block, tmp, &bufmgr_fake->lru) {
		drm_intel_bo_fake *bo_fake = (drm_intel_bo_fake *) block->bo;
		/* Releases the memory, and memcpys dirty contents out if
		 * necessary.
		 */
		free_block(bufmgr_fake, block, 0);
		bo_fake->block = NULL;
	}

	pthread_mutex_unlock(&bufmgr_fake->lock);
}

drm_public void
drm_intel_bufmgr_fake_set_last_dispatch(drm_intel_bufmgr *bufmgr,
					volatile unsigned int
					*last_dispatch)
{
	drm_intel_bufmgr_fake *bufmgr_fake = (drm_intel_bufmgr_fake *) bufmgr;

	bufmgr_fake->last_dispatch = (volatile int *)last_dispatch;
}

drm_public drm_intel_bufmgr *
drm_intel_bufmgr_fake_init(int fd, unsigned long low_offset,
			   void *low_virtual, unsigned long size,
			   volatile unsigned int *last_dispatch)
{
	drm_intel_bufmgr_fake *bufmgr_fake;

	bufmgr_fake = calloc(1, sizeof(*bufmgr_fake));

	if (pthread_mutex_init(&bufmgr_fake->lock, NULL) != 0) {
		free(bufmgr_fake);
		return NULL;
	}

	/* Initialize allocator */
	DRMINITLISTHEAD(&bufmgr_fake->fenced);
	DRMINITLISTHEAD(&bufmgr_fake->on_hardware);
	DRMINITLISTHEAD(&bufmgr_fake->lru);

	bufmgr_fake->low_offset = low_offset;
	bufmgr_fake->virtual = low_virtual;
	bufmgr_fake->size = size;
	bufmgr_fake->heap = mmInit(low_offset, size);

	/* Hook in methods */
	bufmgr_fake->bufmgr.bo_alloc = drm_intel_fake_bo_alloc;
	bufmgr_fake->bufmgr.bo_alloc_for_render = drm_intel_fake_bo_alloc;
	bufmgr_fake->bufmgr.bo_alloc_tiled = drm_intel_fake_bo_alloc_tiled;
	bufmgr_fake->bufmgr.bo_reference = drm_intel_fake_bo_reference;
	bufmgr_fake->bufmgr.bo_unreference = drm_intel_fake_bo_unreference;
	bufmgr_fake->bufmgr.bo_map = drm_intel_fake_bo_map;
	bufmgr_fake->bufmgr.bo_unmap = drm_intel_fake_bo_unmap;
	bufmgr_fake->bufmgr.bo_subdata = drm_intel_fake_bo_subdata;
	bufmgr_fake->bufmgr.bo_wait_rendering =
	    drm_intel_fake_bo_wait_rendering;
	bufmgr_fake->bufmgr.bo_emit_reloc = drm_intel_fake_emit_reloc;
	bufmgr_fake->bufmgr.destroy = drm_intel_fake_destroy;
	bufmgr_fake->bufmgr.bo_exec = drm_intel_fake_bo_exec;
	bufmgr_fake->bufmgr.check_aperture_space =
	    drm_intel_fake_check_aperture_space;
	bufmgr_fake->bufmgr.debug = 0;

	bufmgr_fake->fd = fd;
	bufmgr_fake->last_dispatch = (volatile int *)last_dispatch;

	return &bufmgr_fake->bufmgr;
}
