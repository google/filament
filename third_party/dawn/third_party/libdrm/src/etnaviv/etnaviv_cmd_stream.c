/*
 * Copyright (C) 2014-2015 Etnaviv Project
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

#include <assert.h>

#include "etnaviv_drmif.h"
#include "etnaviv_priv.h"

static pthread_mutex_t idx_lock = PTHREAD_MUTEX_INITIALIZER;

static void *grow(void *ptr, uint32_t nr, uint32_t *max, uint32_t sz)
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

#define APPEND(x, name) ({ \
	(x)->name = grow((x)->name, (x)->nr_ ## name, &(x)->max_ ## name, sizeof((x)->name[0])); \
	(x)->nr_ ## name ++; \
})

static inline struct etna_cmd_stream_priv *
etna_cmd_stream_priv(struct etna_cmd_stream *stream)
{
    return (struct etna_cmd_stream_priv *)stream;
}

drm_public struct etna_cmd_stream *etna_cmd_stream_new(struct etna_pipe *pipe,
        uint32_t size,
		void (*reset_notify)(struct etna_cmd_stream *stream, void *priv),
		void *priv)
{
	struct etna_cmd_stream_priv *stream = NULL;

	if (size == 0) {
		ERROR_MSG("invalid size of 0");
		goto fail;
	}

	stream = calloc(1, sizeof(*stream));
	if (!stream) {
		ERROR_MSG("allocation failed");
		goto fail;
	}

	/* allocate even number of 32-bit words */
	size = ALIGN(size, 2);

	stream->base.buffer = malloc(size * sizeof(uint32_t));
	if (!stream->base.buffer) {
		ERROR_MSG("allocation failed");
		goto fail;
	}

	stream->base.size = size;
	stream->pipe = pipe;
	stream->reset_notify = reset_notify;
	stream->reset_notify_priv = priv;

	return &stream->base;

fail:
	if (stream)
		etna_cmd_stream_del(&stream->base);

	return NULL;
}

drm_public void etna_cmd_stream_del(struct etna_cmd_stream *stream)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);

	free(stream->buffer);
	free(priv->submit.relocs);
	free(priv->submit.pmrs);
	free(priv);
}

static void reset_buffer(struct etna_cmd_stream *stream)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);

	stream->offset = 0;
	priv->submit.nr_bos = 0;
	priv->submit.nr_relocs = 0;
	priv->submit.nr_pmrs = 0;
	priv->nr_bos = 0;

	if (priv->reset_notify)
		priv->reset_notify(stream, priv->reset_notify_priv);
}

drm_public uint32_t etna_cmd_stream_timestamp(struct etna_cmd_stream *stream)
{
	return etna_cmd_stream_priv(stream)->last_timestamp;
}

static uint32_t append_bo(struct etna_cmd_stream *stream, struct etna_bo *bo)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);
	uint32_t idx;

	idx = APPEND(&priv->submit, bos);
	idx = APPEND(priv, bos);

	priv->submit.bos[idx].flags = 0;
	priv->submit.bos[idx].handle = bo->handle;

	priv->bos[idx] = etna_bo_ref(bo);

	return idx;
}

/* add (if needed) bo, return idx: */
static uint32_t bo2idx(struct etna_cmd_stream *stream, struct etna_bo *bo,
		uint32_t flags)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);
	uint32_t idx;

	pthread_mutex_lock(&idx_lock);

	if (bo->current_stream == stream) {
		idx = bo->idx;
	} else {
		/* slow-path: */
		for (idx = 0; idx < priv->nr_bos; idx++)
			if (priv->bos[idx] == bo)
				break;
		if (idx == priv->nr_bos) {
			/* not found */
			idx = append_bo(stream, bo);
		}
		bo->current_stream = stream;
		bo->idx = idx;
	}
	pthread_mutex_unlock(&idx_lock);

	if (flags & ETNA_RELOC_READ)
		priv->submit.bos[idx].flags |= ETNA_SUBMIT_BO_READ;
	if (flags & ETNA_RELOC_WRITE)
		priv->submit.bos[idx].flags |= ETNA_SUBMIT_BO_WRITE;

	return idx;
}

static void flush(struct etna_cmd_stream *stream, int in_fence_fd,
		  int *out_fence_fd)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);
	int ret, id = priv->pipe->id;
	struct etna_gpu *gpu = priv->pipe->gpu;

	struct drm_etnaviv_gem_submit req = {
		.pipe = gpu->core,
		.exec_state = id,
		.bos = VOID2U64(priv->submit.bos),
		.nr_bos = priv->submit.nr_bos,
		.relocs = VOID2U64(priv->submit.relocs),
		.nr_relocs = priv->submit.nr_relocs,
		.pmrs = VOID2U64(priv->submit.pmrs),
		.nr_pmrs = priv->submit.nr_pmrs,
		.stream = VOID2U64(stream->buffer),
		.stream_size = stream->offset * 4, /* in bytes */
	};

	if (in_fence_fd != -1) {
		req.flags |= ETNA_SUBMIT_FENCE_FD_IN | ETNA_SUBMIT_NO_IMPLICIT;
		req.fence_fd = in_fence_fd;
	}

	if (out_fence_fd)
		req.flags |= ETNA_SUBMIT_FENCE_FD_OUT;

	ret = drmCommandWriteRead(gpu->dev->fd, DRM_ETNAVIV_GEM_SUBMIT,
			&req, sizeof(req));

	if (ret)
		ERROR_MSG("submit failed: %d (%s)", ret, strerror(errno));
	else
		priv->last_timestamp = req.fence;

	for (uint32_t i = 0; i < priv->nr_bos; i++) {
		struct etna_bo *bo = priv->bos[i];

		bo->current_stream = NULL;
		etna_bo_del(bo);
	}

	if (out_fence_fd)
		*out_fence_fd = req.fence_fd;
}

drm_public void etna_cmd_stream_flush(struct etna_cmd_stream *stream)
{
	flush(stream, -1, NULL);
	reset_buffer(stream);
}

drm_public void etna_cmd_stream_flush2(struct etna_cmd_stream *stream,
									   int in_fence_fd,
									   int *out_fence_fd)
{
	flush(stream, in_fence_fd, out_fence_fd);
	reset_buffer(stream);
}

drm_public void etna_cmd_stream_finish(struct etna_cmd_stream *stream)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);

	flush(stream, -1, NULL);
	etna_pipe_wait(priv->pipe, priv->last_timestamp, 5000);
	reset_buffer(stream);
}

drm_public void etna_cmd_stream_reloc(struct etna_cmd_stream *stream,
									  const struct etna_reloc *r)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);
	struct drm_etnaviv_gem_submit_reloc *reloc;
	uint32_t idx = APPEND(&priv->submit, relocs);
	uint32_t addr = 0;

	reloc = &priv->submit.relocs[idx];

	reloc->reloc_idx = bo2idx(stream, r->bo, r->flags);
	reloc->reloc_offset = r->offset;
	reloc->submit_offset = stream->offset * 4; /* in bytes */
	reloc->flags = 0;

	etna_cmd_stream_emit(stream, addr);
}

drm_public void etna_cmd_stream_perf(struct etna_cmd_stream *stream, const struct etna_perf *p)
{
	struct etna_cmd_stream_priv *priv = etna_cmd_stream_priv(stream);
	struct drm_etnaviv_gem_submit_pmr *pmr;
	uint32_t idx = APPEND(&priv->submit, pmrs);

	pmr = &priv->submit.pmrs[idx];

	pmr->flags = p->flags;
	pmr->sequence = p->sequence;
	pmr->read_offset = p->offset;
	pmr->read_idx = bo2idx(stream, p->bo, ETNA_SUBMIT_BO_READ | ETNA_SUBMIT_BO_WRITE);
	pmr->domain = p->signal->domain->id;
	pmr->signal = p->signal->signal;
}
