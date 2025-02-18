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

#ifndef FREEDRENO_RINGBUFFER_H_
#define FREEDRENO_RINGBUFFER_H_

#include <freedreno_drmif.h>

/* the ringbuffer object is not opaque so that OUT_RING() type stuff
 * can be inlined.  Note that users should not make assumptions about
 * the size of this struct.
 */

struct fd_ringbuffer_funcs;

enum fd_ringbuffer_flags {

	/* Ringbuffer is a "state object", which is potentially reused
	 * many times, rather than being used in one-shot mode linked
	 * to a parent ringbuffer.
	 */
	FD_RINGBUFFER_OBJECT = 0x1,

	/* Hint that the stateobj will be used for streaming state
	 * that is used once or a few times and then discarded.
	 *
	 * For sub-allocation, non streaming stateobj's should be
	 * sub-allocated from a page size buffer, so one long lived
	 * state obj doesn't prevent other pages from being freed.
	 * (Ie. it would be no worse than allocating a page sized
	 * bo for each small non-streaming stateobj).
	 *
	 * But streaming stateobj's could be sub-allocated from a
	 * larger buffer to reduce the alloc/del overhead.
	 */
	FD_RINGBUFFER_STREAMING = 0x2,
};

struct fd_ringbuffer {
	int size;
	uint32_t *cur, *end, *start, *last_start;
	struct fd_pipe *pipe;
	const struct fd_ringbuffer_funcs *funcs;
	uint32_t last_timestamp;
	struct fd_ringbuffer *parent;

	/* for users of fd_ringbuffer to store their own private per-
	 * ringbuffer data
	 */
	void *user;

	enum fd_ringbuffer_flags flags;

	/* This is a bit gross, but we can't use atomic_t in exported
	 * headers.  OTOH, we don't need the refcnt to be publicly
	 * visible.  The only reason that this struct is exported is
	 * because fd_ringbuffer_emit needs to be something that can
	 * be inlined for performance reasons.
	 */
	union {
#ifdef HAS_ATOMIC_OPS
		atomic_t refcnt;
#endif
		uint64_t __pad;
	};
};

struct fd_ringbuffer * fd_ringbuffer_new(struct fd_pipe *pipe,
		uint32_t size);
will_be_deprecated
struct fd_ringbuffer * fd_ringbuffer_new_object(struct fd_pipe *pipe,
		uint32_t size);
struct fd_ringbuffer * fd_ringbuffer_new_flags(struct fd_pipe *pipe,
		uint32_t size, enum fd_ringbuffer_flags flags);

struct fd_ringbuffer *fd_ringbuffer_ref(struct fd_ringbuffer *ring);
void fd_ringbuffer_del(struct fd_ringbuffer *ring);
void fd_ringbuffer_set_parent(struct fd_ringbuffer *ring,
		struct fd_ringbuffer *parent);
will_be_deprecated
void fd_ringbuffer_reset(struct fd_ringbuffer *ring);
int fd_ringbuffer_flush(struct fd_ringbuffer *ring);
/* in_fence_fd: -1 for no in-fence, else fence fd
 * out_fence_fd: NULL for no output-fence requested, else ptr to return out-fence
 */
int fd_ringbuffer_flush2(struct fd_ringbuffer *ring, int in_fence_fd,
		int *out_fence_fd);
void fd_ringbuffer_grow(struct fd_ringbuffer *ring, uint32_t ndwords);
uint32_t fd_ringbuffer_timestamp(struct fd_ringbuffer *ring);

static inline void fd_ringbuffer_emit(struct fd_ringbuffer *ring,
		uint32_t data)
{
	(*ring->cur++) = data;
}

struct fd_reloc {
	struct fd_bo *bo;
#define FD_RELOC_READ             0x0001
#define FD_RELOC_WRITE            0x0002
	uint32_t flags;
	uint32_t offset;
	uint32_t or;
	int32_t  shift;
	uint32_t orhi;      /* used for a5xx+ */
};

/* NOTE: relocs are 2 dwords on a5xx+ */

void fd_ringbuffer_reloc2(struct fd_ringbuffer *ring, const struct fd_reloc *reloc);
will_be_deprecated void fd_ringbuffer_reloc(struct fd_ringbuffer *ring, const struct fd_reloc *reloc);
uint32_t fd_ringbuffer_cmd_count(struct fd_ringbuffer *ring);
uint32_t fd_ringbuffer_emit_reloc_ring_full(struct fd_ringbuffer *ring,
		struct fd_ringbuffer *target, uint32_t cmd_idx);
uint32_t fd_ringbuffer_size(struct fd_ringbuffer *ring);

#endif /* FREEDRENO_RINGBUFFER_H_ */
