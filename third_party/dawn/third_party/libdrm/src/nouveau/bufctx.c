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
#include <assert.h>
#include <errno.h>

#include "libdrm_lists.h"

#include "nouveau.h"
#include "private.h"

struct nouveau_bufref_priv {
	struct nouveau_bufref base;
	struct nouveau_bufref_priv *next;
	struct nouveau_bufctx *bufctx;
};

struct nouveau_bufbin_priv {
	struct nouveau_bufref_priv *list;
	int relocs;
};

struct nouveau_bufctx_priv {
	struct nouveau_bufctx base;
	struct nouveau_bufref_priv *free;
	int nr_bins;
	struct nouveau_bufbin_priv bins[];
};

static inline struct nouveau_bufctx_priv *
nouveau_bufctx(struct nouveau_bufctx *bctx)
{
	return (struct nouveau_bufctx_priv *)bctx;
}

drm_public int
nouveau_bufctx_new(struct nouveau_client *client, int bins,
		   struct nouveau_bufctx **pbctx)
{
	struct nouveau_bufctx_priv *priv;

	priv = calloc(1, sizeof(*priv) + sizeof(priv->bins[0]) * bins);
	if (priv) {
		DRMINITLISTHEAD(&priv->base.head);
		DRMINITLISTHEAD(&priv->base.pending);
		DRMINITLISTHEAD(&priv->base.current);
		priv->base.client = client;
		priv->nr_bins = bins;
		*pbctx = &priv->base;
		return 0;
	}

	return -ENOMEM;
}

drm_public void
nouveau_bufctx_del(struct nouveau_bufctx **pbctx)
{
	struct nouveau_bufctx_priv *pctx = nouveau_bufctx(*pbctx);
	struct nouveau_bufref_priv *pref;
	if (pctx) {
		while (pctx->nr_bins--)
			nouveau_bufctx_reset(&pctx->base, pctx->nr_bins);
		while ((pref = pctx->free)) {
			pctx->free = pref->next;
			free(pref);
		}
		free(pctx);
		*pbctx = NULL;
	}
}

drm_public void
nouveau_bufctx_reset(struct nouveau_bufctx *bctx, int bin)
{
	struct nouveau_bufctx_priv *pctx = nouveau_bufctx(bctx);
	struct nouveau_bufbin_priv *pbin = &pctx->bins[bin];
	struct nouveau_bufref_priv *pref;

	while ((pref = pbin->list)) {
		DRMLISTDELINIT(&pref->base.thead);
		pbin->list = pref->next;
		pref->next = pctx->free;
		pctx->free = pref;
	}

	bctx->relocs -= pbin->relocs;
	pbin->relocs  = 0;
}

drm_public struct nouveau_bufref *
nouveau_bufctx_refn(struct nouveau_bufctx *bctx, int bin,
		    struct nouveau_bo *bo, uint32_t flags)
{
	struct nouveau_bufctx_priv *pctx = nouveau_bufctx(bctx);
	struct nouveau_bufbin_priv *pbin = &pctx->bins[bin];
	struct nouveau_bufref_priv *pref = pctx->free;

	if (!pref)
		pref = malloc(sizeof(*pref));
	else
		pctx->free = pref->next;

	if (pref) {
		pref->base.bo = bo;
		pref->base.flags = flags;
		pref->base.packet = 0;

		DRMLISTADDTAIL(&pref->base.thead, &bctx->pending);
		pref->bufctx = bctx;
		pref->next = pbin->list;
		pbin->list = pref;
	}

	return &pref->base;
}

drm_public struct nouveau_bufref *
nouveau_bufctx_mthd(struct nouveau_bufctx *bctx, int bin, uint32_t packet,
		    struct nouveau_bo *bo, uint64_t data, uint32_t flags,
		    uint32_t vor, uint32_t tor)
{
	struct nouveau_bufctx_priv *pctx = nouveau_bufctx(bctx);
	struct nouveau_bufbin_priv *pbin = &pctx->bins[bin];
	struct nouveau_bufref *bref = nouveau_bufctx_refn(bctx, bin, bo, flags);
	if (bref) {
		bref->packet = packet;
		bref->data = data;
		bref->vor = vor;
		bref->tor = tor;
		pbin->relocs++;
		bctx->relocs++;
	}
	return bref;
}
