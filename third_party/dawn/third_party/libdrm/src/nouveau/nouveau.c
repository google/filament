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
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>

#include <xf86drm.h>
#include <xf86atomic.h>
#include "libdrm_macros.h"
#include "libdrm_lists.h"
#include "nouveau_drm.h"

#include "nouveau.h"
#include "private.h"

#include "nvif/class.h"
#include "nvif/cl0080.h"
#include "nvif/ioctl.h"
#include "nvif/unpack.h"

drm_private FILE *nouveau_out = NULL;
drm_private uint32_t nouveau_debug = 0;

static void
debug_init(void)
{
	static bool once = false;
	char *debug, *out;

	if (once)
		return;
	once = true;

	debug = getenv("NOUVEAU_LIBDRM_DEBUG");
	if (debug) {
		int n = strtol(debug, NULL, 0);
		if (n >= 0)
			nouveau_debug = n;

	}

	nouveau_out = stderr;
	out = getenv("NOUVEAU_LIBDRM_OUT");
	if (out) {
		FILE *fout = fopen(out, "w");
		if (fout)
			nouveau_out = fout;
	}
}

static int
nouveau_object_ioctl(struct nouveau_object *obj, void *data, uint32_t size)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	union {
		struct nvif_ioctl_v0 v0;
	} *args = data;
	uint32_t argc = size;
	int ret = -ENOSYS;

	if (!(ret = nvif_unpack(ret, &data, &size, args->v0, 0, 0, true))) {
		if (!obj->length) {
			if (obj != &drm->client)
				args->v0.object = (unsigned long)(void *)obj;
			else
				args->v0.object = 0;
			args->v0.owner = NVIF_IOCTL_V0_OWNER_ANY;
			args->v0.route = 0x00;
		} else {
			args->v0.route = 0xff;
			args->v0.token = obj->handle;
		}
	} else
		return ret;

	return drmCommandWriteRead(drm->fd, DRM_NOUVEAU_NVIF, args, argc);
}

drm_public int
nouveau_object_mthd(struct nouveau_object *obj,
		    uint32_t mthd, void *data, uint32_t size)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	struct {
		struct nvif_ioctl_v0 ioctl;
		struct nvif_ioctl_mthd_v0 mthd;
	} *args;
	uint32_t argc = sizeof(*args) + size;
	uint8_t stack[128];
	int ret;

	if (!drm->nvif)
		return -ENOSYS;

	if (argc > sizeof(stack)) {
		if (!(args = malloc(argc)))
			return -ENOMEM;
	} else {
		args = (void *)stack;
	}
	args->ioctl.version = 0;
	args->ioctl.type = NVIF_IOCTL_V0_MTHD;
	args->mthd.version = 0;
	args->mthd.method = mthd;

	memcpy(args->mthd.data, data, size);
	ret = nouveau_object_ioctl(obj, args, argc);
	memcpy(data, args->mthd.data, size);
	if (args != (void *)stack)
		free(args);
	return ret;
}

drm_public void
nouveau_object_sclass_put(struct nouveau_sclass **psclass)
{
	free(*psclass);
	*psclass = NULL;
}

drm_public int
nouveau_object_sclass_get(struct nouveau_object *obj,
			  struct nouveau_sclass **psclass)
{
	struct nouveau_drm *drm = nouveau_drm(obj);
	struct {
		struct nvif_ioctl_v0 ioctl;
		struct nvif_ioctl_sclass_v0 sclass;
	} *args = NULL;
	struct nouveau_sclass *sclass;
	int ret, cnt = 0, i;
	uint32_t size;

	if (!drm->nvif)
		return abi16_sclass(obj, psclass);

	while (1) {
		size = sizeof(*args) + cnt * sizeof(args->sclass.oclass[0]);
		if (!(args = malloc(size)))
			return -ENOMEM;
		args->ioctl.version = 0;
		args->ioctl.type = NVIF_IOCTL_V0_SCLASS;
		args->sclass.version = 0;
		args->sclass.count = cnt;

		ret = nouveau_object_ioctl(obj, args, size);
		if (ret == 0 && args->sclass.count <= cnt)
			break;
		cnt = args->sclass.count;
		free(args);
		if (ret != 0)
			return ret;
	}

	if ((sclass = calloc(args->sclass.count, sizeof(*sclass)))) {
		for (i = 0; i < args->sclass.count; i++) {
			sclass[i].oclass = args->sclass.oclass[i].oclass;
			sclass[i].minver = args->sclass.oclass[i].minver;
			sclass[i].maxver = args->sclass.oclass[i].maxver;
		}
		*psclass = sclass;
		ret = args->sclass.count;
	} else {
		ret = -ENOMEM;
	}

	free(args);
	return ret;
}

drm_public int
nouveau_object_mclass(struct nouveau_object *obj,
		      const struct nouveau_mclass *mclass)
{
	struct nouveau_sclass *sclass;
	int ret = -ENODEV;
	int cnt, i, j;

	cnt = nouveau_object_sclass_get(obj, &sclass);
	if (cnt < 0)
		return cnt;

	for (i = 0; ret < 0 && mclass[i].oclass; i++) {
		for (j = 0; j < cnt; j++) {
			if (mclass[i].oclass  == sclass[j].oclass &&
			    mclass[i].version >= sclass[j].minver &&
			    mclass[i].version <= sclass[j].maxver) {
				ret = i;
				break;
			}
		}
	}

	nouveau_object_sclass_put(&sclass);
	return ret;
}

static void
nouveau_object_fini(struct nouveau_object *obj)
{
	struct {
		struct nvif_ioctl_v0 ioctl;
		struct nvif_ioctl_del del;
	} args = {
		.ioctl.type = NVIF_IOCTL_V0_DEL,
	};

	if (obj->data) {
		abi16_delete(obj);
		free(obj->data);
		obj->data = NULL;
		return;
	}

	nouveau_object_ioctl(obj, &args, sizeof(args));
}

static int
nouveau_object_init(struct nouveau_object *parent, uint32_t handle,
		    int32_t oclass, void *data, uint32_t size,
		    struct nouveau_object *obj)
{
	struct nouveau_drm *drm = nouveau_drm(parent);
	struct {
		struct nvif_ioctl_v0 ioctl;
		struct nvif_ioctl_new_v0 new;
	} *args;
	uint32_t argc = sizeof(*args) + size;
	int (*func)(struct nouveau_object *);
	int ret = -ENOSYS;

	obj->parent = parent;
	obj->handle = handle;
	obj->oclass = oclass;
	obj->length = 0;
	obj->data = NULL;

	if (!abi16_object(obj, &func) && drm->nvif) {
		if (!(args = malloc(argc)))
			return -ENOMEM;
		args->ioctl.version = 0;
		args->ioctl.type = NVIF_IOCTL_V0_NEW;
		args->new.version = 0;
		args->new.route = NVIF_IOCTL_V0_ROUTE_NVIF;
		args->new.token = (unsigned long)(void *)obj;
		args->new.object = (unsigned long)(void *)obj;
		args->new.handle = handle;
		args->new.oclass = oclass;
		memcpy(args->new.data, data, size);
		ret = nouveau_object_ioctl(parent, args, argc);
		memcpy(data, args->new.data, size);
		free(args);
	} else
	if (func) {
		obj->length = size ? size : sizeof(struct nouveau_object *);
		if (!(obj->data = malloc(obj->length)))
			return -ENOMEM;
		if (data)
			memcpy(obj->data, data, obj->length);
		*(struct nouveau_object **)obj->data = obj;

		ret = func(obj);
	}

	if (ret) {
		nouveau_object_fini(obj);
		return ret;
	}

	return 0;
}

drm_public int
nouveau_object_new(struct nouveau_object *parent, uint64_t handle,
		   uint32_t oclass, void *data, uint32_t length,
		   struct nouveau_object **pobj)
{
	struct nouveau_object *obj;
	int ret;

	if (!(obj = malloc(sizeof(*obj))))
		return -ENOMEM;

	ret = nouveau_object_init(parent, handle, oclass, data, length, obj);
	if (ret) {
		free(obj);
		return ret;
	}

	*pobj = obj;
	return 0;
}

drm_public void
nouveau_object_del(struct nouveau_object **pobj)
{
	struct nouveau_object *obj = *pobj;
	if (obj) {
		nouveau_object_fini(obj);
		free(obj);
		*pobj = NULL;
	}
}

drm_public void
nouveau_drm_del(struct nouveau_drm **pdrm)
{
	free(*pdrm);
	*pdrm = NULL;
}

drm_public int
nouveau_drm_new(int fd, struct nouveau_drm **pdrm)
{
	struct nouveau_drm *drm;
	drmVersionPtr ver;

	debug_init();

	if (!(drm = calloc(1, sizeof(*drm))))
		return -ENOMEM;
	drm->fd = fd;

	if (!(ver = drmGetVersion(fd))) {
		nouveau_drm_del(&drm);
		return -EINVAL;
	}
	*pdrm = drm;

	drm->version = (ver->version_major << 24) |
		       (ver->version_minor << 8) |
		        ver->version_patchlevel;
	drm->nvif = (drm->version >= 0x01000301);
	drmFreeVersion(ver);
	return 0;
}

/* this is the old libdrm's version of nouveau_device_wrap(), the symbol
 * is kept here to prevent AIGLX from crashing if the DDX is linked against
 * the new libdrm, but the DRI driver against the old
 */
drm_public int
nouveau_device_open_existing(struct nouveau_device **pdev, int close, int fd,
			     drm_context_t ctx)
{
	return -EACCES;
}

drm_public int
nouveau_device_new(struct nouveau_object *parent, int32_t oclass,
		   void *data, uint32_t size, struct nouveau_device **pdev)
{
	struct nv_device_info_v0 info = {};
	union {
		struct nv_device_v0 v0;
	} *args = data;
	uint32_t argc = size;
	struct nouveau_drm *drm = nouveau_drm(parent);
	struct nouveau_device_priv *nvdev;
	struct nouveau_device *dev;
	uint64_t v;
	char *tmp;
	int ret = -ENOSYS;

	if (oclass != NV_DEVICE ||
	    nvif_unpack(ret, &data, &size, args->v0, 0, 0, false))
		return ret;

	if (!(nvdev = calloc(1, sizeof(*nvdev))))
		return -ENOMEM;
	dev = *pdev = &nvdev->base;
	dev->fd = -1;

	if (drm->nvif) {
		ret = nouveau_object_init(parent, 0, oclass, args, argc,
					  &dev->object);
		if (ret)
			goto done;

		info.version = 0;

		ret = nouveau_object_mthd(&dev->object, NV_DEVICE_V0_INFO,
					  &info, sizeof(info));
		if (ret)
			goto done;

		nvdev->base.chipset = info.chipset;
		nvdev->have_bo_usage = true;
	} else
	if (args->v0.device == ~0ULL) {
		nvdev->base.object.parent = &drm->client;
		nvdev->base.object.handle = ~0ULL;
		nvdev->base.object.oclass = NOUVEAU_DEVICE_CLASS;
		nvdev->base.object.length = ~0;

		ret = nouveau_getparam(dev, NOUVEAU_GETPARAM_CHIPSET_ID, &v);
		if (ret)
			goto done;
		nvdev->base.chipset = v;

		ret = nouveau_getparam(dev, NOUVEAU_GETPARAM_HAS_BO_USAGE, &v);
		if (ret == 0)
			nvdev->have_bo_usage = (v != 0);
	} else
		return -ENOSYS;

	ret = nouveau_getparam(dev, NOUVEAU_GETPARAM_FB_SIZE, &v);
	if (ret)
		goto done;
	nvdev->base.vram_size = v;

	ret = nouveau_getparam(dev, NOUVEAU_GETPARAM_AGP_SIZE, &v);
	if (ret)
		goto done;
	nvdev->base.gart_size = v;

	tmp = getenv("NOUVEAU_LIBDRM_VRAM_LIMIT_PERCENT");
	if (tmp)
		nvdev->vram_limit_percent = atoi(tmp);
	else
		nvdev->vram_limit_percent = 80;

	nvdev->base.vram_limit =
		(nvdev->base.vram_size * nvdev->vram_limit_percent) / 100;

	tmp = getenv("NOUVEAU_LIBDRM_GART_LIMIT_PERCENT");
	if (tmp)
		nvdev->gart_limit_percent = atoi(tmp);
	else
		nvdev->gart_limit_percent = 80;

	nvdev->base.gart_limit =
		(nvdev->base.gart_size * nvdev->gart_limit_percent) / 100;

	ret = pthread_mutex_init(&nvdev->lock, NULL);
	DRMINITLISTHEAD(&nvdev->bo_list);
done:
	if (ret)
		nouveau_device_del(pdev);
	return ret;
}

drm_public int
nouveau_device_wrap(int fd, int close, struct nouveau_device **pdev)
{
	struct nouveau_drm *drm;
	struct nouveau_device_priv *nvdev;
	int ret;

	ret = nouveau_drm_new(fd, &drm);
	if (ret)
		return ret;
	drm->nvif = false;

	ret = nouveau_device_new(&drm->client, NV_DEVICE,
				 &(struct nv_device_v0) {
					.device = ~0ULL,
				 }, sizeof(struct nv_device_v0), pdev);
	if (ret) {
		nouveau_drm_del(&drm);
		return ret;
	}

	nvdev = nouveau_device(*pdev);
	nvdev->base.fd = drm->fd;
	nvdev->base.drm_version = drm->version;
	nvdev->close = close;
	return 0;
}

drm_public int
nouveau_device_open(const char *busid, struct nouveau_device **pdev)
{
	int ret = -ENODEV, fd = drmOpen("nouveau", busid);
	if (fd >= 0) {
		ret = nouveau_device_wrap(fd, 1, pdev);
		if (ret)
			drmClose(fd);
	}
	return ret;
}

drm_public void
nouveau_device_del(struct nouveau_device **pdev)
{
	struct nouveau_device_priv *nvdev = nouveau_device(*pdev);
	if (nvdev) {
		free(nvdev->client);
		pthread_mutex_destroy(&nvdev->lock);
		if (nvdev->base.fd >= 0) {
			struct nouveau_drm *drm =
				nouveau_drm(&nvdev->base.object);
			nouveau_drm_del(&drm);
			if (nvdev->close)
				drmClose(nvdev->base.fd);
		}
		free(nvdev);
		*pdev = NULL;
	}
}

drm_public int
nouveau_getparam(struct nouveau_device *dev, uint64_t param, uint64_t *value)
{
	struct nouveau_drm *drm = nouveau_drm(&dev->object);
	struct drm_nouveau_getparam r = { .param = param };
	int fd = drm->fd, ret =
		drmCommandWriteRead(fd, DRM_NOUVEAU_GETPARAM, &r, sizeof(r));
	*value = r.value;
	return ret;
}

drm_public int
nouveau_setparam(struct nouveau_device *dev, uint64_t param, uint64_t value)
{
	struct nouveau_drm *drm = nouveau_drm(&dev->object);
	struct drm_nouveau_setparam r = { .param = param, .value = value };
	return drmCommandWrite(drm->fd, DRM_NOUVEAU_SETPARAM, &r, sizeof(r));
}

drm_public int
nouveau_client_new(struct nouveau_device *dev, struct nouveau_client **pclient)
{
	struct nouveau_device_priv *nvdev = nouveau_device(dev);
	struct nouveau_client_priv *pcli;
	int id = 0, i, ret = -ENOMEM;
	uint32_t *clients;

	pthread_mutex_lock(&nvdev->lock);

	for (i = 0; i < nvdev->nr_client; i++) {
		id = ffs(nvdev->client[i]) - 1;
		if (id >= 0)
			goto out;
	}

	clients = realloc(nvdev->client, sizeof(uint32_t) * (i + 1));
	if (!clients)
		goto unlock;
	nvdev->client = clients;
	nvdev->client[i] = 0;
	nvdev->nr_client++;

out:
	pcli = calloc(1, sizeof(*pcli));
	if (pcli) {
		nvdev->client[i] |= (1 << id);
		pcli->base.device = dev;
		pcli->base.id = (i * 32) + id;
		ret = 0;
	}

	*pclient = &pcli->base;

unlock:
	pthread_mutex_unlock(&nvdev->lock);
	return ret;
}

drm_public void
nouveau_client_del(struct nouveau_client **pclient)
{
	struct nouveau_client_priv *pcli = nouveau_client(*pclient);
	struct nouveau_device_priv *nvdev;
	if (pcli) {
		int id = pcli->base.id;
		nvdev = nouveau_device(pcli->base.device);
		pthread_mutex_lock(&nvdev->lock);
		nvdev->client[id / 32] &= ~(1 << (id % 32));
		pthread_mutex_unlock(&nvdev->lock);
		free(pcli->kref);
		free(pcli);
	}
}

static void
nouveau_bo_del(struct nouveau_bo *bo)
{
	struct nouveau_drm *drm = nouveau_drm(&bo->device->object);
	struct nouveau_device_priv *nvdev = nouveau_device(bo->device);
	struct nouveau_bo_priv *nvbo = nouveau_bo(bo);

	if (nvbo->head.next) {
		pthread_mutex_lock(&nvdev->lock);
		if (atomic_read(&nvbo->refcnt) == 0) {
			DRMLISTDEL(&nvbo->head);
			/*
			 * This bo has to be closed with the lock held because
			 * gem handles are not refcounted. If a shared bo is
			 * closed and re-opened in another thread a race
			 * against DRM_IOCTL_GEM_OPEN or drmPrimeFDToHandle
			 * might cause the bo to be closed accidentally while
			 * re-importing.
			 */
			drmCloseBufferHandle(drm->fd, bo->handle);
		}
		pthread_mutex_unlock(&nvdev->lock);
	} else {
		drmCloseBufferHandle(drm->fd, bo->handle);
	}
	if (bo->map)
		drm_munmap(bo->map, bo->size);
	free(nvbo);
}

drm_public int
nouveau_bo_new(struct nouveau_device *dev, uint32_t flags, uint32_t align,
	       uint64_t size, union nouveau_bo_config *config,
	       struct nouveau_bo **pbo)
{
	struct nouveau_bo_priv *nvbo = calloc(1, sizeof(*nvbo));
	struct nouveau_bo *bo = &nvbo->base;
	int ret;

	if (!nvbo)
		return -ENOMEM;
	atomic_set(&nvbo->refcnt, 1);
	bo->device = dev;
	bo->flags = flags;
	bo->size = size;

	ret = abi16_bo_init(bo, align, config);
	if (ret) {
		free(nvbo);
		return ret;
	}

	*pbo = bo;
	return 0;
}

static int
nouveau_bo_wrap_locked(struct nouveau_device *dev, uint32_t handle,
		       struct nouveau_bo **pbo, int name)
{
	struct nouveau_drm *drm = nouveau_drm(&dev->object);
	struct nouveau_device_priv *nvdev = nouveau_device(dev);
	struct drm_nouveau_gem_info req = { .handle = handle };
	struct nouveau_bo_priv *nvbo;
	int ret;

	DRMLISTFOREACHENTRY(nvbo, &nvdev->bo_list, head) {
		if (nvbo->base.handle == handle) {
			if (atomic_inc_return(&nvbo->refcnt) == 1) {
				/*
				 * Uh oh, this bo is dead and someone else
				 * will free it, but because refcnt is
				 * now non-zero fortunately they won't
				 * call the ioctl to close the bo.
				 *
				 * Remove this bo from the list so other
				 * calls to nouveau_bo_wrap_locked will
				 * see our replacement nvbo.
				 */
				DRMLISTDEL(&nvbo->head);
				if (!name)
					name = nvbo->name;
				break;
			}

			*pbo = &nvbo->base;
			return 0;
		}
	}

	ret = drmCommandWriteRead(drm->fd, DRM_NOUVEAU_GEM_INFO,
				  &req, sizeof(req));
	if (ret)
		return ret;

	nvbo = calloc(1, sizeof(*nvbo));
	if (nvbo) {
		atomic_set(&nvbo->refcnt, 1);
		nvbo->base.device = dev;
		abi16_bo_info(&nvbo->base, &req);
		nvbo->name = name;
		DRMLISTADD(&nvbo->head, &nvdev->bo_list);
		*pbo = &nvbo->base;
		return 0;
	}

	return -ENOMEM;
}

static void
nouveau_nvbo_make_global(struct nouveau_bo_priv *nvbo)
{
	if (!nvbo->head.next) {
		struct nouveau_device_priv *nvdev = nouveau_device(nvbo->base.device);
		pthread_mutex_lock(&nvdev->lock);
		if (!nvbo->head.next)
			DRMLISTADD(&nvbo->head, &nvdev->bo_list);
		pthread_mutex_unlock(&nvdev->lock);
	}
}

drm_public void
nouveau_bo_make_global(struct nouveau_bo *bo)
{
    struct nouveau_bo_priv *nvbo = nouveau_bo(bo);

    nouveau_nvbo_make_global(nvbo);
}

drm_public int
nouveau_bo_wrap(struct nouveau_device *dev, uint32_t handle,
		struct nouveau_bo **pbo)
{
	struct nouveau_device_priv *nvdev = nouveau_device(dev);
	int ret;
	pthread_mutex_lock(&nvdev->lock);
	ret = nouveau_bo_wrap_locked(dev, handle, pbo, 0);
	pthread_mutex_unlock(&nvdev->lock);
	return ret;
}

drm_public int
nouveau_bo_name_ref(struct nouveau_device *dev, uint32_t name,
		    struct nouveau_bo **pbo)
{
	struct nouveau_drm *drm = nouveau_drm(&dev->object);
	struct nouveau_device_priv *nvdev = nouveau_device(dev);
	struct nouveau_bo_priv *nvbo;
	struct drm_gem_open req = { .name = name };
	int ret;

	pthread_mutex_lock(&nvdev->lock);
	DRMLISTFOREACHENTRY(nvbo, &nvdev->bo_list, head) {
		if (nvbo->name == name) {
			ret = nouveau_bo_wrap_locked(dev, nvbo->base.handle,
						     pbo, name);
			pthread_mutex_unlock(&nvdev->lock);
			return ret;
		}
	}

	ret = drmIoctl(drm->fd, DRM_IOCTL_GEM_OPEN, &req);
	if (ret == 0) {
		ret = nouveau_bo_wrap_locked(dev, req.handle, pbo, name);
	}

	pthread_mutex_unlock(&nvdev->lock);
	return ret;
}

drm_public int
nouveau_bo_name_get(struct nouveau_bo *bo, uint32_t *name)
{
	struct drm_gem_flink req = { .handle = bo->handle };
	struct nouveau_drm *drm = nouveau_drm(&bo->device->object);
	struct nouveau_bo_priv *nvbo = nouveau_bo(bo);

	*name = nvbo->name;
	if (!*name) {
		int ret = drmIoctl(drm->fd, DRM_IOCTL_GEM_FLINK, &req);

		if (ret) {
			*name = 0;
			return ret;
		}
		nvbo->name = *name = req.name;

		nouveau_nvbo_make_global(nvbo);
	}
	return 0;
}

drm_public void
nouveau_bo_ref(struct nouveau_bo *bo, struct nouveau_bo **pref)
{
	struct nouveau_bo *ref = *pref;
	if (bo) {
		atomic_inc(&nouveau_bo(bo)->refcnt);
	}
	if (ref) {
		if (atomic_dec_and_test(&nouveau_bo(ref)->refcnt))
			nouveau_bo_del(ref);
	}
	*pref = bo;
}

drm_public int
nouveau_bo_prime_handle_ref(struct nouveau_device *dev, int prime_fd,
			    struct nouveau_bo **bo)
{
	struct nouveau_drm *drm = nouveau_drm(&dev->object);
	struct nouveau_device_priv *nvdev = nouveau_device(dev);
	int ret;
	unsigned int handle;

	nouveau_bo_ref(NULL, bo);

	pthread_mutex_lock(&nvdev->lock);
	ret = drmPrimeFDToHandle(drm->fd, prime_fd, &handle);
	if (ret == 0) {
		ret = nouveau_bo_wrap_locked(dev, handle, bo, 0);
	}
	pthread_mutex_unlock(&nvdev->lock);
	return ret;
}

drm_public int
nouveau_bo_set_prime(struct nouveau_bo *bo, int *prime_fd)
{
	struct nouveau_drm *drm = nouveau_drm(&bo->device->object);
	struct nouveau_bo_priv *nvbo = nouveau_bo(bo);
	int ret;

	ret = drmPrimeHandleToFD(drm->fd, nvbo->base.handle, DRM_CLOEXEC, prime_fd);
	if (ret)
		return ret;

	nouveau_nvbo_make_global(nvbo);
	return 0;
}

drm_public int
nouveau_bo_wait(struct nouveau_bo *bo, uint32_t access,
		struct nouveau_client *client)
{
	struct nouveau_drm *drm = nouveau_drm(&bo->device->object);
	struct nouveau_bo_priv *nvbo = nouveau_bo(bo);
	struct drm_nouveau_gem_cpu_prep req;
	struct nouveau_pushbuf *push;
	int ret = 0;

	if (!(access & NOUVEAU_BO_RDWR))
		return 0;

	push = cli_push_get(client, bo);
	if (push && push->channel)
		nouveau_pushbuf_kick(push, push->channel);

	if (!nvbo->head.next && !(nvbo->access & NOUVEAU_BO_WR) &&
				!(access & NOUVEAU_BO_WR))
		return 0;

	req.handle = bo->handle;
	req.flags = 0;
	if (access & NOUVEAU_BO_WR)
		req.flags |= NOUVEAU_GEM_CPU_PREP_WRITE;
	if (access & NOUVEAU_BO_NOBLOCK)
		req.flags |= NOUVEAU_GEM_CPU_PREP_NOWAIT;

	ret = drmCommandWrite(drm->fd, DRM_NOUVEAU_GEM_CPU_PREP,
			      &req, sizeof(req));
	if (ret == 0)
		nvbo->access = 0;
	return ret;
}

drm_public int
nouveau_bo_map(struct nouveau_bo *bo, uint32_t access,
	       struct nouveau_client *client)
{
	struct nouveau_drm *drm = nouveau_drm(&bo->device->object);
	struct nouveau_bo_priv *nvbo = nouveau_bo(bo);
	if (bo->map == NULL) {
		bo->map = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE,
			       MAP_SHARED, drm->fd, nvbo->map_handle);
		if (bo->map == MAP_FAILED) {
			bo->map = NULL;
			return -errno;
		}
	}
	return nouveau_bo_wait(bo, access, client);
}
