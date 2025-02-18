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

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "private.h"

#include "nvif/class.h"

static int
abi16_chan_nv04(struct nouveau_object *obj)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	struct nv04_fifo *nv04 = obj->data;
	struct drm_nouveau_channel_alloc req = {
		.fb_ctxdma_handle = nv04->vram,
		.tt_ctxdma_handle = nv04->gart
	};
	int ret;

	ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_CHANNEL_ALLOC,
				  &req, sizeof(req));
	if (ret)
		return ret;

	nv04->base.channel = req.channel;
	nv04->base.pushbuf = req.pushbuf_domains;
	nv04->notify = req.notifier_handle;
	nv04->base.object->handle = req.channel;
	nv04->base.object->length = sizeof(*nv04);
	return 0;
}

static int
abi16_chan_nvc0(struct nouveau_object *obj)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	struct drm_nouveau_channel_alloc req = {};
	struct nvc0_fifo *nvc0 = obj->data;
	int ret;

	ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_CHANNEL_ALLOC,
				  &req, sizeof(req));
	if (ret)
		return ret;

	nvc0->base.channel = req.channel;
	nvc0->base.pushbuf = req.pushbuf_domains;
	nvc0->notify = req.notifier_handle;
	nvc0->base.object->handle = req.channel;
	nvc0->base.object->length = sizeof(*nvc0);
	return 0;
}

static int
abi16_chan_nve0(struct nouveau_object *obj)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	struct drm_nouveau_channel_alloc req = {};
	struct nve0_fifo *nve0 = obj->data;
	int ret;

	if (obj->length > offsetof(struct nve0_fifo, engine)) {
		req.fb_ctxdma_handle = 0xffffffff;
		req.tt_ctxdma_handle = nve0->engine;
	}

	ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_CHANNEL_ALLOC,
				  &req, sizeof(req));
	if (ret)
		return ret;

	nve0->base.channel = req.channel;
	nve0->base.pushbuf = req.pushbuf_domains;
	nve0->notify = req.notifier_handle;
	nve0->base.object->handle = req.channel;
	nve0->base.object->length = sizeof(*nve0);
	return 0;
}

static int
abi16_engobj(struct nouveau_object *obj)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	struct drm_nouveau_grobj_alloc req = {
		.channel = obj->parent->handle,
		.handle = obj->handle,
		.class = obj->oclass,
	};
	int ret;

	/* Older kernel versions did not have the concept of nouveau-
	 * specific classes and abused some NVIDIA-assigned ones for
	 * a SW class.  The ABI16 layer has compatibility in place to
	 * translate these older identifiers to the newer ones.
	 *
	 * Clients that have been updated to use NVIF are required to
	 * use the newer class identifiers, which means that they'll
	 * break if running on an older kernel.
	 *
	 * To handle this case, when using ABI16, we translate to the
	 * older values which work on any kernel.
	 */
	switch (req.class) {
	case NVIF_CLASS_SW_NV04 : req.class = 0x006e; break;
	case NVIF_CLASS_SW_NV10 : req.class = 0x016e; break;
	case NVIF_CLASS_SW_NV50 : req.class = 0x506e; break;
	case NVIF_CLASS_SW_GF100: req.class = 0x906e; break;
	default:
		break;
	}

	ret = drmCommandWrite(drm->fd, DRM_NOUVEAU_GROBJ_ALLOC,
			      &req, sizeof(req));
	if (ret)
		return ret;

	obj->length = sizeof(struct nouveau_object *);
	return 0;
}

static int
abi16_ntfy(struct nouveau_object *obj)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	struct nv04_notify *ntfy = obj->data;
	struct drm_nouveau_notifierobj_alloc req = {
		.channel = obj->parent->handle,
		.handle = ntfy->object->handle,
		.size = ntfy->length,
	};
	int ret;

	ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_NOTIFIEROBJ_ALLOC,
				  &req, sizeof(req));
	if (ret)
		return ret;

	ntfy->offset = req.offset;
	ntfy->object->length = sizeof(*ntfy);
	return 0;
}

drm_private int
abi16_sclass(struct nouveau_object *obj, struct nouveau_sclass **psclass)
{
	struct nouveau_sclass *sclass;
	struct nouveau_device *dev;

	if (!(sclass = calloc(8, sizeof(*sclass))))
		return -ENOMEM;
	*psclass = sclass;

	switch (obj->oclass) {
	case NOUVEAU_FIFO_CHANNEL_CLASS:
		/* Older kernel versions were exposing the wrong video engine
		 * classes on certain G98:GF100 boards.  This has since been
		 * corrected, but ABI16 has compatibility in place to avoid
		 * breaking older userspace.
		 *
		 * Clients that have been updated to use NVIF are required to
		 * use the correct classes, which means that they'll break if
		 * running on an older kernel.
		 *
		 * To handle this issue, if using the older kernel interfaces,
		 * we'll magic up a list containing the vdec classes that the
		 * kernel will accept for these boards.  Clients should make
		 * use of this information instead of hardcoding classes for
		 * specific chipsets.
		 */
		dev = (struct nouveau_device *)obj->parent;
		if (dev->chipset >= 0x98 &&
		    dev->chipset != 0xa0 &&
		    dev->chipset <  0xc0) {
			*sclass++ = (struct nouveau_sclass){
				GT212_MSVLD, -1, -1
			};
			*sclass++ = (struct nouveau_sclass){
				GT212_MSPDEC, -1, -1
			};
			*sclass++ = (struct nouveau_sclass){
				GT212_MSPPP, -1, -1
			};
		}
		break;
	default:
		break;
	}

	return sclass - *psclass;
}

drm_private void
abi16_delete(struct nouveau_object *obj)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	if (obj->oclass == NOUVEAU_FIFO_CHANNEL_CLASS) {
		struct drm_nouveau_channel_free req;
		req.channel = obj->handle;
		drmCommandWrite(drm->fd, DRM_NOUVEAU_CHANNEL_FREE,
				&req, sizeof(req));
	} else {
		struct drm_nouveau_gpuobj_free req;
		req.channel = obj->parent->handle;
		req.handle  = obj->handle;
		drmCommandWrite(drm->fd, DRM_NOUVEAU_GPUOBJ_FREE,
				&req, sizeof(req));
	}
}

drm_private bool
abi16_object(struct nouveau_object *obj, int (**func)(struct nouveau_object *))
{
	struct nouveau_object *parent = obj->parent;

	/* nouveau_object::length is (ab)used to determine whether the
	 * object is a legacy object (!=0), or a real NVIF object.
	 */
	if ((parent->length != 0 && parent->oclass == NOUVEAU_DEVICE_CLASS) ||
	    (parent->length == 0 && parent->oclass == NV_DEVICE)) {
		if (obj->oclass == NOUVEAU_FIFO_CHANNEL_CLASS) {
			struct nouveau_device *dev = (void *)parent;
			if (dev->chipset < 0xc0)
				*func = abi16_chan_nv04;
			else
			if (dev->chipset < 0xe0)
				*func = abi16_chan_nvc0;
			else
				*func = abi16_chan_nve0;
			return true;
		}
	} else
	if ((parent->length != 0 &&
	     parent->oclass == NOUVEAU_FIFO_CHANNEL_CLASS)) {
		if (obj->oclass == NOUVEAU_NOTIFIER_CLASS) {
			*func = abi16_ntfy;
			return true;
		}

		*func = abi16_engobj;
		return false; /* try NVIF, if supported, before calling func */
	}

	*func = NULL;
	return false;
}

drm_private void
abi16_bo_info(struct nouveau_bo *bo, struct drm_nouveau_gem_info *info)
{
	struct nouveau_bo_priv *nvbo = nouveau_bo(bo);

	nvbo->map_handle = info->map_handle;
	bo->handle = info->handle;
	bo->size = info->size;
	bo->offset = info->offset;

	bo->flags = 0;
	if (info->domain & NOUVEAU_GEM_DOMAIN_VRAM)
		bo->flags |= NOUVEAU_BO_VRAM;
	if (info->domain & NOUVEAU_GEM_DOMAIN_GART)
		bo->flags |= NOUVEAU_BO_GART;
	if (!(info->tile_flags & NOUVEAU_GEM_TILE_NONCONTIG))
		bo->flags |= NOUVEAU_BO_CONTIG;
	if (nvbo->map_handle)
		bo->flags |= NOUVEAU_BO_MAP;

	if (bo->device->chipset >= 0xc0) {
		bo->config.nvc0.memtype   = (info->tile_flags & 0xff00) >> 8;
		bo->config.nvc0.tile_mode = info->tile_mode;
	} else
	if (bo->device->chipset >= 0x80 || bo->device->chipset == 0x50) {
		bo->config.nv50.memtype   = (info->tile_flags & 0x07f00) >> 8 |
					    (info->tile_flags & 0x30000) >> 9;
		bo->config.nv50.tile_mode = info->tile_mode << 4;
	} else {
		bo->config.nv04.surf_flags = info->tile_flags & 7;
		bo->config.nv04.surf_pitch = info->tile_mode;
	}
}

drm_private int
abi16_bo_init(struct nouveau_bo *bo, uint32_t alignment,
	      union nouveau_bo_config *config)
{
	struct nouveau_device *dev = bo->device;
	struct nouveau_drm *drm = nouveau_drm(&dev->object);
	struct drm_nouveau_gem_new req = {};
	struct drm_nouveau_gem_info *info = &req.info;
	int ret;

	if (bo->flags & NOUVEAU_BO_VRAM)
		info->domain |= NOUVEAU_GEM_DOMAIN_VRAM;
	if (bo->flags & NOUVEAU_BO_GART)
		info->domain |= NOUVEAU_GEM_DOMAIN_GART;
	if (!info->domain)
		info->domain |= NOUVEAU_GEM_DOMAIN_VRAM |
				NOUVEAU_GEM_DOMAIN_GART;

	if (bo->flags & NOUVEAU_BO_MAP)
		info->domain |= NOUVEAU_GEM_DOMAIN_MAPPABLE;

	if (bo->flags & NOUVEAU_BO_COHERENT)
		info->domain |= NOUVEAU_GEM_DOMAIN_COHERENT;

	if (!(bo->flags & NOUVEAU_BO_CONTIG))
		info->tile_flags = NOUVEAU_GEM_TILE_NONCONTIG;

	info->size = bo->size;
	req.align = alignment;

	if (config) {
		if (dev->chipset >= 0xc0) {
			info->tile_flags = (config->nvc0.memtype & 0xff) << 8;
			info->tile_mode  = config->nvc0.tile_mode;
		} else
		if (dev->chipset >= 0x80 || dev->chipset == 0x50) {
			info->tile_flags = (config->nv50.memtype & 0x07f) << 8 |
					   (config->nv50.memtype & 0x180) << 9;
			info->tile_mode  = config->nv50.tile_mode >> 4;
		} else {
			info->tile_flags = config->nv04.surf_flags & 7;
			info->tile_mode  = config->nv04.surf_pitch;
		}
	}

	if (!nouveau_device(dev)->have_bo_usage)
		info->tile_flags &= 0x0000ff00;

	ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_GEM_NEW,
				  &req, sizeof(req));
	if (ret == 0)
		abi16_bo_info(bo, &req.info);
	return ret;
}
