#ifndef __NOUVEAU_LIBDRM_PRIVATE_H__
#define __NOUVEAU_LIBDRM_PRIVATE_H__

#include <stdio.h>

#include <libdrm_macros.h>
#include <xf86drm.h>
#include <xf86atomic.h>
#include <pthread.h>
#include "nouveau_drm.h"

#include "nouveau.h"

/*
 * 0x00000001 dump all pushbuffers
 * 0x00000002 submit pushbuffers synchronously
 * 0x80000000 if compiled with SIMULATE return -EINVAL for all pb submissions
 */
drm_private extern uint32_t nouveau_debug;
drm_private extern FILE *nouveau_out;
#define dbg_on(lvl) (nouveau_debug & (1 << lvl))
#define dbg(lvl, fmt, args...) do {                                            \
	if (dbg_on((lvl)))                                                     \
		fprintf(nouveau_out, "nouveau: "fmt, ##args);                       \
} while(0)
#define err(fmt, args...) fprintf(nouveau_out, "nouveau: "fmt, ##args)

struct nouveau_client_kref {
	struct drm_nouveau_gem_pushbuf_bo *kref;
	struct nouveau_pushbuf *push;
};

struct nouveau_client_priv {
	struct nouveau_client base;
	struct nouveau_client_kref *kref;
	unsigned kref_nr;
};

static inline struct nouveau_client_priv *
nouveau_client(struct nouveau_client *client)
{
	return (struct nouveau_client_priv *)client;
}

static inline struct drm_nouveau_gem_pushbuf_bo *
cli_kref_get(struct nouveau_client *client, struct nouveau_bo *bo)
{
	struct nouveau_client_priv *pcli = nouveau_client(client);
	struct drm_nouveau_gem_pushbuf_bo *kref = NULL;
	if (pcli->kref_nr > bo->handle)
		kref = pcli->kref[bo->handle].kref;
	return kref;
}

static inline struct nouveau_pushbuf *
cli_push_get(struct nouveau_client *client, struct nouveau_bo *bo)
{
	struct nouveau_client_priv *pcli = nouveau_client(client);
	struct nouveau_pushbuf *push = NULL;
	if (pcli->kref_nr > bo->handle)
		push = pcli->kref[bo->handle].push;
	return push;
}

static inline void
cli_kref_set(struct nouveau_client *client, struct nouveau_bo *bo,
	     struct drm_nouveau_gem_pushbuf_bo *kref,
	     struct nouveau_pushbuf *push)
{
	struct nouveau_client_priv *pcli = nouveau_client(client);
	if (pcli->kref_nr <= bo->handle) {
		pcli->kref = realloc(pcli->kref,
				     sizeof(*pcli->kref) * bo->handle * 2);
		while (pcli->kref_nr < bo->handle * 2) {
			pcli->kref[pcli->kref_nr].kref = NULL;
			pcli->kref[pcli->kref_nr].push = NULL;
			pcli->kref_nr++;
		}
	}
	pcli->kref[bo->handle].kref = kref;
	pcli->kref[bo->handle].push = push;
}

struct nouveau_bo_priv {
	struct nouveau_bo base;
	struct nouveau_list head;
	atomic_t refcnt;
	uint64_t map_handle;
	uint32_t name;
	uint32_t access;
};

static inline struct nouveau_bo_priv *
nouveau_bo(struct nouveau_bo *bo)
{
	return (struct nouveau_bo_priv *)bo;
}

struct nouveau_device_priv {
	struct nouveau_device base;
	int close;
	pthread_mutex_t lock;
	struct nouveau_list bo_list;
	uint32_t *client;
	int nr_client;
	bool have_bo_usage;
	int gart_limit_percent, vram_limit_percent;
};

static inline struct nouveau_device_priv *
nouveau_device(struct nouveau_device *dev)
{
	return (struct nouveau_device_priv *)dev;
}

int
nouveau_device_open_existing(struct nouveau_device **, int, int, drm_context_t);

/* abi16.c */
drm_private bool abi16_object(struct nouveau_object *, int (**)(struct nouveau_object *));
drm_private void abi16_delete(struct nouveau_object *);
drm_private int  abi16_sclass(struct nouveau_object *, struct nouveau_sclass **);
drm_private void abi16_bo_info(struct nouveau_bo *, struct drm_nouveau_gem_info *);
drm_private int  abi16_bo_init(struct nouveau_bo *, uint32_t alignment,
			       union nouveau_bo_config *);

#endif
