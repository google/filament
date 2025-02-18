/*
 * Copyright (C) 2012 Samsung Electronics Co., Ltd.
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
 *    Inki Dae <inki.dae@samsung.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/mman.h>

#include <xf86drm.h>

#include "libdrm_macros.h"
#include "exynos_drm.h"
#include "exynos_drmif.h"

#define U642VOID(x) ((void *)(unsigned long)(x))

/*
 * Create exynos drm device object.
 *
 * @fd: file descriptor to exynos drm driver opened.
 *
 * if true, return the device object else NULL.
 */
drm_public struct exynos_device * exynos_device_create(int fd)
{
	struct exynos_device *dev;

	dev = calloc(sizeof(*dev), 1);
	if (!dev) {
		fprintf(stderr, "failed to create device[%s].\n",
				strerror(errno));
		return NULL;
	}

	dev->fd = fd;

	return dev;
}

/*
 * Destroy exynos drm device object
 *
 * @dev: exynos drm device object.
 */
drm_public void exynos_device_destroy(struct exynos_device *dev)
{
	free(dev);
}

/*
 * Create a exynos buffer object to exynos drm device.
 *
 * @dev: exynos drm device object.
 * @size: user-desired size.
 * flags: user-desired memory type.
 *	user can set one or more types among several types to memory
 *	allocation and cache attribute types. and as default,
 *	EXYNOS_BO_NONCONTIG and EXYNOS-BO_NONCACHABLE types would
 *	be used.
 *
 * if true, return a exynos buffer object else NULL.
 */
drm_public struct exynos_bo * exynos_bo_create(struct exynos_device *dev,
                                               size_t size, uint32_t flags)
{
	struct exynos_bo *bo;
	struct drm_exynos_gem_create req = {
		.size = size,
		.flags = flags,
	};

	if (size == 0) {
		fprintf(stderr, "invalid size.\n");
		goto fail;
	}

	bo = calloc(sizeof(*bo), 1);
	if (!bo) {
		fprintf(stderr, "failed to create bo[%s].\n",
				strerror(errno));
		goto err_free_bo;
	}

	bo->dev = dev;

	if (drmIoctl(dev->fd, DRM_IOCTL_EXYNOS_GEM_CREATE, &req)){
		fprintf(stderr, "failed to create gem object[%s].\n",
				strerror(errno));
		goto err_free_bo;
	}

	bo->handle = req.handle;
	bo->size = size;
	bo->flags = flags;

	return bo;

err_free_bo:
	free(bo);
fail:
	return NULL;
}

/*
 * Get information to gem region allocated.
 *
 * @dev: exynos drm device object.
 * @handle: gem handle to request gem info.
 * @size: size to gem object and returned by kernel side.
 * @flags: gem flags to gem object and returned by kernel side.
 *
 * with this function call, you can get flags and size to gem handle
 * through bo object.
 *
 * if true, return 0 else negative.
 */
drm_public int exynos_bo_get_info(struct exynos_device *dev, uint32_t handle,
                                  size_t *size, uint32_t *flags)
{
	int ret;
	struct drm_exynos_gem_info req = {
		.handle = handle,
	};

	ret = drmIoctl(dev->fd, DRM_IOCTL_EXYNOS_GEM_GET, &req);
	if (ret < 0) {
		fprintf(stderr, "failed to get gem object information[%s].\n",
				strerror(errno));
		return ret;
	}

	*size = req.size;
	*flags = req.flags;

	return 0;
}

/*
 * Destroy a exynos buffer object.
 *
 * @bo: a exynos buffer object to be destroyed.
 */
drm_public void exynos_bo_destroy(struct exynos_bo *bo)
{
	if (!bo)
		return;

	if (bo->vaddr)
		munmap(bo->vaddr, bo->size);

	if (bo->handle) {
		drmCloseBufferHandle(bo->dev->fd, bo->handle);
	}

	free(bo);
}


/*
 * Get a exynos buffer object from a gem global object name.
 *
 * @dev: a exynos device object.
 * @name: a gem global object name exported by another process.
 *
 * this interface is used to get a exynos buffer object from a gem
 * global object name sent by another process for buffer sharing.
 *
 * if true, return a exynos buffer object else NULL.
 *
 */
drm_public struct exynos_bo *
exynos_bo_from_name(struct exynos_device *dev, uint32_t name)
{
	struct exynos_bo *bo;
	struct drm_gem_open req = {
		.name = name,
	};

	bo = calloc(sizeof(*bo), 1);
	if (!bo) {
		fprintf(stderr, "failed to allocate bo[%s].\n",
				strerror(errno));
		return NULL;
	}

	if (drmIoctl(dev->fd, DRM_IOCTL_GEM_OPEN, &req)) {
		fprintf(stderr, "failed to open gem object[%s].\n",
				strerror(errno));
		goto err_free_bo;
	}

	bo->dev = dev;
	bo->name = name;
	bo->handle = req.handle;

	return bo;

err_free_bo:
	free(bo);
	return NULL;
}

/*
 * Get a gem global object name from a gem object handle.
 *
 * @bo: a exynos buffer object including gem handle.
 * @name: a gem global object name to be got by kernel driver.
 *
 * this interface is used to get a gem global object name from a gem object
 * handle to a buffer that wants to share it with another process.
 *
 * if true, return 0 else negative.
 */
drm_public int exynos_bo_get_name(struct exynos_bo *bo, uint32_t *name)
{
	if (!bo->name) {
		struct drm_gem_flink req = {
			.handle = bo->handle,
		};
		int ret;

		ret = drmIoctl(bo->dev->fd, DRM_IOCTL_GEM_FLINK, &req);
		if (ret) {
			fprintf(stderr, "failed to get gem global name[%s].\n",
					strerror(errno));
			return ret;
		}

		bo->name = req.name;
	}

	*name = bo->name;

	return 0;
}

drm_public uint32_t exynos_bo_handle(struct exynos_bo *bo)
{
	return bo->handle;
}

/*
 * Mmap a buffer to user space.
 *
 * @bo: a exynos buffer object including a gem object handle to be mmapped
 *	to user space.
 *
 * if true, user pointer mmapped else NULL.
 */
drm_public void *exynos_bo_map(struct exynos_bo *bo)
{
	if (!bo->vaddr) {
		struct exynos_device *dev = bo->dev;
		struct drm_mode_map_dumb arg;
		void *map = NULL;
		int ret;

		memset(&arg, 0, sizeof(arg));
		arg.handle = bo->handle;

		ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
		if (ret) {
			fprintf(stderr, "failed to map dumb buffer[%s].\n",
				strerror(errno));
			return NULL;
		}

		map = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
				dev->fd, arg.offset);

		if (map != MAP_FAILED)
			bo->vaddr = map;
	}

	return bo->vaddr;
}

/*
 * Export gem object to dmabuf as file descriptor.
 *
 * @dev: exynos device object
 * @handle: gem handle to export as file descriptor of dmabuf
 * @fd: file descriptor returned from kernel
 *
 * @return: 0 on success, -1 on error, and errno will be set
 */
drm_public int
exynos_prime_handle_to_fd(struct exynos_device *dev, uint32_t handle, int *fd)
{
	return drmPrimeHandleToFD(dev->fd, handle, 0, fd);
}

/*
 * Import file descriptor into gem handle.
 *
 * @dev: exynos device object
 * @fd: file descriptor of dmabuf to import
 * @handle: gem handle returned from kernel
 *
 * @return: 0 on success, -1 on error, and errno will be set
 */
drm_public int
exynos_prime_fd_to_handle(struct exynos_device *dev, int fd, uint32_t *handle)
{
	return drmPrimeFDToHandle(dev->fd, fd, handle);
}



/*
 * Request Wireless Display connection or disconnection.
 *
 * @dev: a exynos device object.
 * @connect: indicate whether connectoin or disconnection request.
 * @ext: indicate whether edid data includes extensions data or not.
 * @edid: a pointer to edid data from Wireless Display device.
 *
 * this interface is used to request Virtual Display driver connection or
 * disconnection. for this, user should get a edid data from the Wireless
 * Display device and then send that data to kernel driver with connection
 * request
 *
 * if true, return 0 else negative.
 */
drm_public int
exynos_vidi_connection(struct exynos_device *dev, uint32_t connect,
		       uint32_t ext, void *edid)
{
	struct drm_exynos_vidi_connection req = {
		.connection	= connect,
		.extensions	= ext,
		.edid		= (uint64_t)(uintptr_t)edid,
	};
	int ret;

	ret = drmIoctl(dev->fd, DRM_IOCTL_EXYNOS_VIDI_CONNECTION, &req);
	if (ret) {
		fprintf(stderr, "failed to request vidi connection[%s].\n",
				strerror(errno));
		return ret;
	}

	return 0;
}

static void
exynos_handle_vendor(int fd, struct drm_event *e, void *ctx)
{
	struct drm_exynos_g2d_event *g2d;
	struct exynos_event_context *ectx = ctx;

	switch (e->type) {
		case DRM_EXYNOS_G2D_EVENT:
			if (ectx->version < 1 || ectx->g2d_event_handler == NULL)
				break;
			g2d = (struct drm_exynos_g2d_event *)e;
			ectx->g2d_event_handler(fd, g2d->cmdlist_no, g2d->tv_sec,
						g2d->tv_usec, U642VOID(g2d->user_data));
			break;

		default:
			break;
	}
}

drm_public int
exynos_handle_event(struct exynos_device *dev, struct exynos_event_context *ctx)
{
	char buffer[1024];
	int len, i;
	struct drm_event *e;
	struct drm_event_vblank *vblank;
	drmEventContextPtr evctx = &ctx->base;

	/* The DRM read semantics guarantees that we always get only
	 * complete events. */
	len = read(dev->fd, buffer, sizeof buffer);
	if (len == 0)
		return 0;
	if (len < (int)sizeof *e)
		return -1;

	i = 0;
	while (i < len) {
		e = (struct drm_event *)(buffer + i);
		switch (e->type) {
		case DRM_EVENT_VBLANK:
			if (evctx->version < 1 ||
			    evctx->vblank_handler == NULL)
				break;
			vblank = (struct drm_event_vblank *) e;
			evctx->vblank_handler(dev->fd,
					      vblank->sequence,
					      vblank->tv_sec,
					      vblank->tv_usec,
					      U642VOID (vblank->user_data));
			break;
		case DRM_EVENT_FLIP_COMPLETE:
			if (evctx->version < 2 ||
			    evctx->page_flip_handler == NULL)
				break;
			vblank = (struct drm_event_vblank *) e;
			evctx->page_flip_handler(dev->fd,
						 vblank->sequence,
						 vblank->tv_sec,
						 vblank->tv_usec,
						 U642VOID (vblank->user_data));
			break;
		default:
			exynos_handle_vendor(dev->fd, e, evctx);
			break;
		}
		i += e->length;
	}

	return 0;
}
