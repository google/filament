/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>

#include <xf86drm.h>
#include <xf86atomic.h>
#include "libdrm_lists.h"
#include "nouveau_drm.h"

#include "nouveau.h"
#include "private.h"

struct nouveau_pushbuf_krec {
	struct nouveau_pushbuf_krec *next;
	struct drm_nouveau_gem_pushbuf_bo buffer[NOUVEAU_GEM_MAX_BUFFERS];
	struct drm_nouveau_gem_pushbuf_reloc reloc[NOUVEAU_GEM_MAX_RELOCS];
	struct drm_nouveau_gem_pushbuf_push push[NOUVEAU_GEM_MAX_PUSH];
	int nr_buffer;
	int nr_reloc;
	int nr_push;
	uint64_t vram_used;
	uint64_t gart_used;
};

struct nouveau_pushbuf_priv {
	struct nouveau_pushbuf base;
	struct nouveau_pushbuf_krec *list;
	struct nouveau_pushbuf_krec *krec;
	struct nouveau_list bctx_list;
	struct nouveau_bo *bo;
	uint32_t type;
	uint32_t suffix0;
	uint32_t suffix1;
	uint32_t *ptr;
	uint32_t *bgn;
	int bo_next;
	int bo_nr;
	struct nouveau_bo *bos[];
};

static inline struct nouveau_pushbuf_priv *
nouveau_pushbuf(struct nouveau_pushbuf *push)
{
	return (struct nouveau_pushbuf_priv *)push;
}

static int pushbuf_validate(struct nouveau_pushbuf *, bool);
static int pushbuf_flush(struct nouveau_pushbuf *);

static bool
pushbuf_kref_fits(struct nouveau_pushbuf *push, struct nouveau_bo *bo,
		  uint32_t *domains)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct nouveau_device *dev = push->client->device;
	struct nouveau_bo *kbo;
	struct drm_nouveau_gem_pushbuf_bo *kref;
	int i;

	/* VRAM is the only valid domain.  GART and VRAM|GART buffers
	 * are all accounted to GART, so if this doesn't fit in VRAM
	 * straight up, a flush is needed.
	 */
	if (*domains == NOUVEAU_GEM_DOMAIN_VRAM) {
		if (krec->vram_used + bo->size > dev->vram_limit)
			return false;
		krec->vram_used += bo->size;
		return true;
	}

	/* GART or VRAM|GART buffer.  Account both of these buffer types
	 * to GART only for the moment, which simplifies things.  If the
	 * buffer can fit already, we're done here.
	 */
	if (krec->gart_used + bo->size <= dev->gart_limit) {
		krec->gart_used += bo->size;
		return true;
	}

	/* Ran out of GART space, if it's a VRAM|GART buffer and it'll
	 * fit into available VRAM, turn it into a VRAM buffer
	 */
	if ((*domains & NOUVEAU_GEM_DOMAIN_VRAM) &&
	    krec->vram_used + bo->size <= dev->vram_limit) {
		*domains &= NOUVEAU_GEM_DOMAIN_VRAM;
		krec->vram_used += bo->size;
		return true;
	}

	/* Still couldn't fit the buffer in anywhere, so as a last resort;
	 * scan the buffer list for VRAM|GART buffers and turn them into
	 * VRAM buffers until we have enough space in GART for this one
	 */
	kref = krec->buffer;
	for (i = 0; i < krec->nr_buffer; i++, kref++) {
		if (!(kref->valid_domains & NOUVEAU_GEM_DOMAIN_GART))
			continue;

		kbo = (void *)(unsigned long)kref->user_priv;
		if (!(kref->valid_domains & NOUVEAU_GEM_DOMAIN_VRAM) ||
		    krec->vram_used + kbo->size > dev->vram_limit)
			continue;

		kref->valid_domains &= NOUVEAU_GEM_DOMAIN_VRAM;
		krec->gart_used -= kbo->size;
		krec->vram_used += kbo->size;
		if (krec->gart_used + bo->size <= dev->gart_limit) {
			krec->gart_used += bo->size;
			return true;
		}
	}

	/* Couldn't resolve a placement, need to force a flush */
	return false;
}

static struct drm_nouveau_gem_pushbuf_bo *
pushbuf_kref(struct nouveau_pushbuf *push, struct nouveau_bo *bo,
	     uint32_t flags)
{
	struct nouveau_device *dev = push->client->device;
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct nouveau_pushbuf *fpush;
	struct drm_nouveau_gem_pushbuf_bo *kref;
	uint32_t domains, domains_wr, domains_rd;

	domains = 0;
	if (flags & NOUVEAU_BO_VRAM)
		domains |= NOUVEAU_GEM_DOMAIN_VRAM;
	if (flags & NOUVEAU_BO_GART)
		domains |= NOUVEAU_GEM_DOMAIN_GART;
	domains_wr = domains * !!(flags & NOUVEAU_BO_WR);
	domains_rd = domains * !!(flags & NOUVEAU_BO_RD);

	/* if buffer is referenced on another pushbuf that is owned by the
	 * same client, we need to flush the other pushbuf first to ensure
	 * the correct ordering of commands
	 */
	fpush = cli_push_get(push->client, bo);
	if (fpush && fpush != push)
		pushbuf_flush(fpush);

	kref = cli_kref_get(push->client, bo);
	if (kref) {
		/* possible conflict in memory types - flush and retry */
		if (!(kref->valid_domains & domains))
			return NULL;

		/* VRAM|GART buffer turning into a VRAM buffer.  Make sure
		 * it'll fit in VRAM and force a flush if not.
		 */
		if ((kref->valid_domains  & NOUVEAU_GEM_DOMAIN_GART) &&
		    (            domains == NOUVEAU_GEM_DOMAIN_VRAM)) {
			if (krec->vram_used + bo->size > dev->vram_limit)
				return NULL;
			krec->vram_used += bo->size;
			krec->gart_used -= bo->size;
		}

		kref->valid_domains &= domains;
		kref->write_domains |= domains_wr;
		kref->read_domains  |= domains_rd;
	} else {
		if (krec->nr_buffer == NOUVEAU_GEM_MAX_BUFFERS ||
		    !pushbuf_kref_fits(push, bo, &domains))
			return NULL;

		kref = &krec->buffer[krec->nr_buffer++];
		kref->user_priv = (unsigned long)bo;
		kref->handle = bo->handle;
		kref->valid_domains = domains;
		kref->write_domains = domains_wr;
		kref->read_domains = domains_rd;
		kref->presumed.valid = 1;
		kref->presumed.offset = bo->offset;
		if (bo->flags & NOUVEAU_BO_VRAM)
			kref->presumed.domain = NOUVEAU_GEM_DOMAIN_VRAM;
		else
			kref->presumed.domain = NOUVEAU_GEM_DOMAIN_GART;

		cli_kref_set(push->client, bo, kref, push);
		atomic_inc(&nouveau_bo(bo)->refcnt);
	}

	return kref;
}

static uint32_t
pushbuf_krel(struct nouveau_pushbuf *push, struct nouveau_bo *bo,
	     uint32_t data, uint32_t flags, uint32_t vor, uint32_t tor)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct drm_nouveau_gem_pushbuf_reloc *krel;
	struct drm_nouveau_gem_pushbuf_bo *pkref;
	struct drm_nouveau_gem_pushbuf_bo *bkref;
	uint32_t reloc = data;

	pkref = cli_kref_get(push->client, nvpb->bo);
	bkref = cli_kref_get(push->client, bo);
	krel  = &krec->reloc[krec->nr_reloc++];

	assert(pkref);
	assert(bkref);
	krel->reloc_bo_index = pkref - krec->buffer;
	krel->reloc_bo_offset = (push->cur - nvpb->ptr) * 4;
	krel->bo_index = bkref - krec->buffer;
	krel->flags = 0;
	krel->data = data;
	krel->vor = vor;
	krel->tor = tor;

	if (flags & NOUVEAU_BO_LOW) {
		reloc = (bkref->presumed.offset + data);
		krel->flags |= NOUVEAU_GEM_RELOC_LOW;
	} else
	if (flags & NOUVEAU_BO_HIGH) {
		reloc = (bkref->presumed.offset + data) >> 32;
		krel->flags |= NOUVEAU_GEM_RELOC_HIGH;
	}
	if (flags & NOUVEAU_BO_OR) {
		if (bkref->presumed.domain & NOUVEAU_GEM_DOMAIN_VRAM)
			reloc |= vor;
		else
			reloc |= tor;
		krel->flags |= NOUVEAU_GEM_RELOC_OR;
	}

	return reloc;
}

static void
pushbuf_dump(struct nouveau_pushbuf_krec *krec, int krec_id, int chid)
{
	struct drm_nouveau_gem_pushbuf_reloc *krel;
	struct drm_nouveau_gem_pushbuf_push *kpsh;
	struct drm_nouveau_gem_pushbuf_bo *kref;
	struct nouveau_bo *bo;
	uint32_t *bgn, *end;
	int i;

	err("ch%d: krec %d pushes %d bufs %d relocs %d\n", chid,
	    krec_id, krec->nr_push, krec->nr_buffer, krec->nr_reloc);

	kref = krec->buffer;
	for (i = 0; i < krec->nr_buffer; i++, kref++) {
		bo = (void *)(uintptr_t)kref->user_priv;
		err("ch%d: buf %08x %08x %08x %08x %08x %p 0x%"PRIx64" 0x%"PRIx64"\n", chid, i,
		    kref->handle, kref->valid_domains,
		    kref->read_domains, kref->write_domains, bo->map, bo->offset, bo->size);
	}

	krel = krec->reloc;
	for (i = 0; i < krec->nr_reloc; i++, krel++) {
		err("ch%d: rel %08x %08x %08x %08x %08x %08x %08x\n",
		    chid, krel->reloc_bo_index, krel->reloc_bo_offset,
		    krel->bo_index, krel->flags, krel->data,
		    krel->vor, krel->tor);
	}

	kpsh = krec->push;
	for (i = 0; i < krec->nr_push; i++, kpsh++) {
		kref = krec->buffer + kpsh->bo_index;
		bo = (void *)(unsigned long)kref->user_priv;
		bgn = (uint32_t *)((char *)bo->map + kpsh->offset);
		end = bgn + ((kpsh->length & 0x7fffff) /4);

		err("ch%d: psh %s%08x %010llx %010llx\n", chid,
		    bo->map ? "" : "(unmapped) ", kpsh->bo_index,
		    (unsigned long long)kpsh->offset,
		    (unsigned long long)(kpsh->offset + kpsh->length));
		if (!bo->map)
			continue;
		while (bgn < end)
			err("\t0x%08x\n", *bgn++);
	}
}

static int
pushbuf_submit(struct nouveau_pushbuf *push, struct nouveau_object *chan)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->list;
	struct nouveau_device *dev = push->client->device;
	struct nouveau_drm *drm = nouveau_drm(&dev->object);
	struct drm_nouveau_gem_pushbuf_bo_presumed *info;
	struct drm_nouveau_gem_pushbuf_bo *kref;
	struct drm_nouveau_gem_pushbuf req;
	struct nouveau_fifo *fifo = chan->data;
	struct nouveau_bo *bo;
	int krec_id = 0;
	int ret = 0, i;

	if (chan->oclass != NOUVEAU_FIFO_CHANNEL_CLASS)
		return -EINVAL;

	if (push->kick_notify)
		push->kick_notify(push);

	nouveau_pushbuf_data(push, NULL, 0, 0);

	while (krec && krec->nr_push) {
		req.channel = fifo->channel;
		req.nr_buffers = krec->nr_buffer;
		req.buffers = (uint64_t)(unsigned long)krec->buffer;
		req.nr_relocs = krec->nr_reloc;
		req.nr_push = krec->nr_push;
		req.relocs = (uint64_t)(unsigned long)krec->reloc;
		req.push = (uint64_t)(unsigned long)krec->push;
		req.suffix0 = nvpb->suffix0;
		req.suffix1 = nvpb->suffix1;
		req.vram_available = 0; /* for valgrind */
		if (dbg_on(1))
			req.vram_available |= NOUVEAU_GEM_PUSHBUF_SYNC;
		req.gart_available = 0;

		if (dbg_on(0))
			pushbuf_dump(krec, krec_id++, fifo->channel);

#ifndef SIMULATE
		ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_GEM_PUSHBUF,
					  &req, sizeof(req));
		nvpb->suffix0 = req.suffix0;
		nvpb->suffix1 = req.suffix1;
		dev->vram_limit = (req.vram_available *
				nouveau_device(dev)->vram_limit_percent) / 100;
		dev->gart_limit = (req.gart_available *
				nouveau_device(dev)->gart_limit_percent) / 100;
#else
		if (dbg_on(31))
			ret = -EINVAL;
#endif

		if (ret) {
			err("kernel rejected pushbuf: %s\n", strerror(-ret));
			pushbuf_dump(krec, krec_id++, fifo->channel);
			break;
		}

		kref = krec->buffer;
		for (i = 0; i < krec->nr_buffer; i++, kref++) {
			bo = (void *)(unsigned long)kref->user_priv;

			info = &kref->presumed;
			if (!info->valid) {
				bo->flags &= ~NOUVEAU_BO_APER;
				if (info->domain == NOUVEAU_GEM_DOMAIN_VRAM)
					bo->flags |= NOUVEAU_BO_VRAM;
				else
					bo->flags |= NOUVEAU_BO_GART;
				bo->offset = info->offset;
			}

			if (kref->write_domains)
				nouveau_bo(bo)->access |= NOUVEAU_BO_WR;
			if (kref->read_domains)
				nouveau_bo(bo)->access |= NOUVEAU_BO_RD;
		}

		krec = krec->next;
	}

	return ret;
}

static int
pushbuf_flush(struct nouveau_pushbuf *push)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct drm_nouveau_gem_pushbuf_bo *kref;
	struct nouveau_bufctx *bctx, *btmp;
	struct nouveau_bo *bo;
	int ret = 0, i;

	if (push->channel) {
		ret = pushbuf_submit(push, push->channel);
	} else {
		nouveau_pushbuf_data(push, NULL, 0, 0);
		krec->next = malloc(sizeof(*krec));
		nvpb->krec = krec->next;
	}

	kref = krec->buffer;
	for (i = 0; i < krec->nr_buffer; i++, kref++) {
		bo = (void *)(unsigned long)kref->user_priv;
		cli_kref_set(push->client, bo, NULL, NULL);
		if (push->channel)
			nouveau_bo_ref(NULL, &bo);
	}

	krec = nvpb->krec;
	krec->vram_used = 0;
	krec->gart_used = 0;
	krec->nr_buffer = 0;
	krec->nr_reloc = 0;
	krec->nr_push = 0;

	DRMLISTFOREACHENTRYSAFE(bctx, btmp, &nvpb->bctx_list, head) {
		DRMLISTJOIN(&bctx->current, &bctx->pending);
		DRMINITLISTHEAD(&bctx->current);
		DRMLISTDELINIT(&bctx->head);
	}

	return ret;
}

static void
pushbuf_refn_fail(struct nouveau_pushbuf *push, int sref, int srel)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct drm_nouveau_gem_pushbuf_bo *kref;

	kref = krec->buffer + sref;
	while (krec->nr_buffer-- > sref) {
		struct nouveau_bo *bo = (void *)(unsigned long)kref->user_priv;
		cli_kref_set(push->client, bo, NULL, NULL);
		nouveau_bo_ref(NULL, &bo);
		kref++;
	}
	krec->nr_buffer = sref;
	krec->nr_reloc = srel;
}

static int
pushbuf_refn(struct nouveau_pushbuf *push, bool retry,
	     struct nouveau_pushbuf_refn *refs, int nr)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct drm_nouveau_gem_pushbuf_bo *kref;
	int sref = krec->nr_buffer;
	int ret = 0, i;

	for (i = 0; i < nr; i++) {
		kref = pushbuf_kref(push, refs[i].bo, refs[i].flags);
		if (!kref) {
			ret = -ENOSPC;
			break;
		}
	}

	if (ret) {
		pushbuf_refn_fail(push, sref, krec->nr_reloc);
		if (retry) {
			pushbuf_flush(push);
			nouveau_pushbuf_space(push, 0, 0, 0);
			return pushbuf_refn(push, false, refs, nr);
		}
	}

	return ret;
}

static int
pushbuf_validate(struct nouveau_pushbuf *push, bool retry)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct drm_nouveau_gem_pushbuf_bo *kref;
	struct nouveau_bufctx *bctx = push->bufctx;
	struct nouveau_bufref *bref;
	int relocs = bctx ? bctx->relocs * 2: 0;
	int sref, srel, ret;

	ret = nouveau_pushbuf_space(push, relocs, relocs, 0);
	if (ret || bctx == NULL)
		return ret;

	sref = krec->nr_buffer;
	srel = krec->nr_reloc;

	DRMLISTDEL(&bctx->head);
	DRMLISTADD(&bctx->head, &nvpb->bctx_list);

	DRMLISTFOREACHENTRY(bref, &bctx->pending, thead) {
		kref = pushbuf_kref(push, bref->bo, bref->flags);
		if (!kref) {
			ret = -ENOSPC;
			break;
		}

		if (bref->packet) {
			pushbuf_krel(push, bref->bo, bref->packet, 0, 0, 0);
			*push->cur++ = 0;
			pushbuf_krel(push, bref->bo, bref->data, bref->flags,
					   bref->vor, bref->tor);
			*push->cur++ = 0;
		}
	}

	DRMLISTJOIN(&bctx->pending, &bctx->current);
	DRMINITLISTHEAD(&bctx->pending);

	if (ret) {
		pushbuf_refn_fail(push, sref, srel);
		if (retry) {
			pushbuf_flush(push);
			return pushbuf_validate(push, false);
		}
	}

	return ret;
}

drm_public int
nouveau_pushbuf_new(struct nouveau_client *client, struct nouveau_object *chan,
		    int nr, uint32_t size, bool immediate,
		    struct nouveau_pushbuf **ppush)
{
	struct nouveau_drm *drm = nouveau_drm(&client->device->object);
	struct nouveau_fifo *fifo = chan->data;
	struct nouveau_pushbuf_priv *nvpb;
	struct nouveau_pushbuf *push;
	struct drm_nouveau_gem_pushbuf req = {};
	int ret;

	if (chan->oclass != NOUVEAU_FIFO_CHANNEL_CLASS)
		return -EINVAL;

	/* nop pushbuf call, to get the current "return to main" sequence
	 * we need to append to the pushbuf on early chipsets
	 */
	req.channel = fifo->channel;
	req.nr_push = 0;
	ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_GEM_PUSHBUF,
				  &req, sizeof(req));
	if (ret)
		return ret;

	nvpb = calloc(1, sizeof(*nvpb) + nr * sizeof(*nvpb->bos));
	if (!nvpb)
		return -ENOMEM;

#ifndef SIMULATE
	nvpb->suffix0 = req.suffix0;
	nvpb->suffix1 = req.suffix1;
#else
	nvpb->suffix0 = 0xffffffff;
	nvpb->suffix1 = 0xffffffff;
#endif
	nvpb->krec = calloc(1, sizeof(*nvpb->krec));
	nvpb->list = nvpb->krec;
	if (!nvpb->krec) {
		free(nvpb);
		return -ENOMEM;
	}

	push = &nvpb->base;
	push->client = client;
	push->channel = immediate ? chan : NULL;
	push->flags = NOUVEAU_BO_RD;
	if (fifo->pushbuf & NOUVEAU_GEM_DOMAIN_GART) {
		push->flags |= NOUVEAU_BO_GART;
		nvpb->type   = NOUVEAU_BO_GART;
	} else
	if (fifo->pushbuf & NOUVEAU_GEM_DOMAIN_VRAM) {
		push->flags |= NOUVEAU_BO_VRAM;
		nvpb->type   = NOUVEAU_BO_VRAM;
	}
	nvpb->type |= NOUVEAU_BO_MAP;

	for (nvpb->bo_nr = 0; nvpb->bo_nr < nr; nvpb->bo_nr++) {
		ret = nouveau_bo_new(client->device, nvpb->type, 0, size,
				     NULL, &nvpb->bos[nvpb->bo_nr]);
		if (ret) {
			nouveau_pushbuf_del(&push);
			return ret;
		}
	}

	DRMINITLISTHEAD(&nvpb->bctx_list);
	*ppush = push;
	return 0;
}

drm_public void
nouveau_pushbuf_del(struct nouveau_pushbuf **ppush)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(*ppush);
	if (nvpb) {
		struct drm_nouveau_gem_pushbuf_bo *kref;
		struct nouveau_pushbuf_krec *krec;
		while ((krec = nvpb->list)) {
			kref = krec->buffer;
			while (krec->nr_buffer--) {
				unsigned long priv = kref++->user_priv;
				struct nouveau_bo *bo = (void *)priv;
				cli_kref_set(nvpb->base.client, bo, NULL, NULL);
				nouveau_bo_ref(NULL, &bo);
			}
			nvpb->list = krec->next;
			free(krec);
		}
		while (nvpb->bo_nr--)
			nouveau_bo_ref(NULL, &nvpb->bos[nvpb->bo_nr]);
		nouveau_bo_ref(NULL, &nvpb->bo);
		free(nvpb);
	}
	*ppush = NULL;
}

drm_public struct nouveau_bufctx *
nouveau_pushbuf_bufctx(struct nouveau_pushbuf *push, struct nouveau_bufctx *ctx)
{
	struct nouveau_bufctx *prev = push->bufctx;
	push->bufctx = ctx;
	return prev;
}

drm_public int
nouveau_pushbuf_space(struct nouveau_pushbuf *push,
		      uint32_t dwords, uint32_t relocs, uint32_t pushes)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct nouveau_client *client = push->client;
	struct nouveau_bo *bo = NULL;
	bool flushed = false;
	int ret = 0;

	/* switch to next buffer if insufficient space in the current one */
	if (push->cur + dwords >= push->end) {
		if (nvpb->bo_next < nvpb->bo_nr) {
			nouveau_bo_ref(nvpb->bos[nvpb->bo_next++], &bo);
			if (nvpb->bo_next == nvpb->bo_nr && push->channel)
				nvpb->bo_next = 0;
		} else {
			ret = nouveau_bo_new(client->device, nvpb->type, 0,
					     nvpb->bos[0]->size, NULL, &bo);
			if (ret)
				return ret;
		}
	}

	/* make sure there's always enough space to queue up the pending
	 * data in the pushbuf proper
	 */
	pushes++;

	/* need to flush if we've run out of space on an immediate pushbuf,
	 * if the new buffer won't fit, or if the kernel push/reloc limits
	 * have been hit
	 */
	if ((bo && ( push->channel ||
		    !pushbuf_kref(push, bo, push->flags))) ||
	    krec->nr_reloc + relocs >= NOUVEAU_GEM_MAX_RELOCS ||
	    krec->nr_push + pushes >= NOUVEAU_GEM_MAX_PUSH) {
		if (nvpb->bo && krec->nr_buffer)
			pushbuf_flush(push);
		flushed = true;
	}

	/* if necessary, switch to new buffer */
	if (bo) {
		ret = nouveau_bo_map(bo, NOUVEAU_BO_WR, push->client);
		if (ret)
			return ret;

		nouveau_pushbuf_data(push, NULL, 0, 0);
		nouveau_bo_ref(bo, &nvpb->bo);
		nouveau_bo_ref(NULL, &bo);

		nvpb->bgn = nvpb->bo->map;
		nvpb->ptr = nvpb->bgn;
		push->cur = nvpb->bgn;
		push->end = push->cur + (nvpb->bo->size / 4);
		push->end -= 2 + push->rsvd_kick; /* space for suffix */
	}

	pushbuf_kref(push, nvpb->bo, push->flags);
	return flushed ? pushbuf_validate(push, false) : 0;
}

drm_public void
nouveau_pushbuf_data(struct nouveau_pushbuf *push, struct nouveau_bo *bo,
		     uint64_t offset, uint64_t length)
{
	struct nouveau_pushbuf_priv *nvpb = nouveau_pushbuf(push);
	struct nouveau_pushbuf_krec *krec = nvpb->krec;
	struct drm_nouveau_gem_pushbuf_push *kpsh;
	struct drm_nouveau_gem_pushbuf_bo *kref;

	if (bo != nvpb->bo && nvpb->bgn != push->cur) {
		if (nvpb->suffix0 || nvpb->suffix1) {
			*push->cur++ = nvpb->suffix0;
			*push->cur++ = nvpb->suffix1;
		}

		nouveau_pushbuf_data(push, nvpb->bo,
				     (nvpb->bgn - nvpb->ptr) * 4,
				     (push->cur - nvpb->bgn) * 4);
		nvpb->bgn = push->cur;
	}

	if (bo) {
		kref = cli_kref_get(push->client, bo);
		assert(kref);
		kpsh = &krec->push[krec->nr_push++];
		kpsh->bo_index = kref - krec->buffer;
		kpsh->offset   = offset;
		kpsh->length   = length;
	}
}

drm_public int
nouveau_pushbuf_refn(struct nouveau_pushbuf *push,
		     struct nouveau_pushbuf_refn *refs, int nr)
{
	return pushbuf_refn(push, true, refs, nr);
}

drm_public void
nouveau_pushbuf_reloc(struct nouveau_pushbuf *push, struct nouveau_bo *bo,
		      uint32_t data, uint32_t flags, uint32_t vor, uint32_t tor)
{
	*push->cur = pushbuf_krel(push, bo, data, flags, vor, tor);
	push->cur++;
}

drm_public int
nouveau_pushbuf_validate(struct nouveau_pushbuf *push)
{
	return pushbuf_validate(push, true);
}

drm_public uint32_t
nouveau_pushbuf_refd(struct nouveau_pushbuf *push, struct nouveau_bo *bo)
{
	struct drm_nouveau_gem_pushbuf_bo *kref;
	uint32_t flags = 0;

	if (cli_push_get(push->client, bo) == push) {
		kref = cli_kref_get(push->client, bo);
		assert(kref);
		if (kref->read_domains)
			flags |= NOUVEAU_BO_RD;
		if (kref->write_domains)
			flags |= NOUVEAU_BO_WR;
	}

	return flags;
}

drm_public int
nouveau_pushbuf_kick(struct nouveau_pushbuf *push, struct nouveau_object *chan)
{
	if (!push->channel)
		return pushbuf_submit(push, chan);
	pushbuf_flush(push);
	return pushbuf_validate(push, false);
}

drm_public bool
nouveau_check_dead_channel(struct nouveau_drm *drm, struct nouveau_object *chan)
{
	struct drm_nouveau_gem_pushbuf req = {};
	struct nouveau_fifo *fifo = chan->data;
	int ret;

	req.channel = fifo->channel;
	req.nr_push = 0;

	ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_GEM_PUSHBUF,
				  &req, sizeof(req));
	/* nouveau returns ENODEV once the channel was killed */
	return ret == -ENODEV;
}
