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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "freedreno_drmif.h"
#include "freedreno_priv.h"

static pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;

struct fd_device * kgsl_device_new(int fd);
struct fd_device * msm_device_new(int fd);

drm_public struct fd_device * fd_device_new(int fd)
{
	struct fd_device *dev;
	drmVersionPtr version;

	/* figure out if we are kgsl or msm drm driver: */
	version = drmGetVersion(fd);
	if (!version) {
		ERROR_MSG("cannot get version: %s", strerror(errno));
		return NULL;
	}

	if (!strcmp(version->name, "msm")) {
		DEBUG_MSG("msm DRM device");
		if (version->version_major != 1) {
			ERROR_MSG("unsupported version: %u.%u.%u", version->version_major,
				version->version_minor, version->version_patchlevel);
			dev = NULL;
			goto out;
		}

		dev = msm_device_new(fd);
		dev->version = version->version_minor;
#if HAVE_FREEDRENO_KGSL
	} else if (!strcmp(version->name, "kgsl")) {
		DEBUG_MSG("kgsl DRM device");
		dev = kgsl_device_new(fd);
#endif
	} else {
		ERROR_MSG("unknown device: %s", version->name);
		dev = NULL;
	}

out:
	drmFreeVersion(version);

	if (!dev)
		return NULL;

	atomic_set(&dev->refcnt, 1);
	dev->fd = fd;
	dev->handle_table = drmHashCreate();
	dev->name_table = drmHashCreate();
	fd_bo_cache_init(&dev->bo_cache, FALSE);
	fd_bo_cache_init(&dev->ring_cache, TRUE);

	return dev;
}

/* like fd_device_new() but creates it's own private dup() of the fd
 * which is close()d when the device is finalized.
 */
drm_public struct fd_device * fd_device_new_dup(int fd)
{
	int dup_fd = dup(fd);
	struct fd_device *dev = fd_device_new(dup_fd);
	if (dev)
		dev->closefd = 1;
	else
		close(dup_fd);
	return dev;
}

drm_public struct fd_device * fd_device_ref(struct fd_device *dev)
{
	atomic_inc(&dev->refcnt);
	return dev;
}

static void fd_device_del_impl(struct fd_device *dev)
{
	int close_fd = dev->closefd ? dev->fd : -1;
	fd_bo_cache_cleanup(&dev->bo_cache, 0);
	drmHashDestroy(dev->handle_table);
	drmHashDestroy(dev->name_table);
	dev->funcs->destroy(dev);
	if (close_fd >= 0)
		close(close_fd);
}

drm_private void fd_device_del_locked(struct fd_device *dev)
{
	if (!atomic_dec_and_test(&dev->refcnt))
		return;
	fd_device_del_impl(dev);
}

drm_public void fd_device_del(struct fd_device *dev)
{
	if (!atomic_dec_and_test(&dev->refcnt))
		return;
	pthread_mutex_lock(&table_lock);
	fd_device_del_impl(dev);
	pthread_mutex_unlock(&table_lock);
}

drm_public int fd_device_fd(struct fd_device *dev)
{
	return dev->fd;
}

drm_public enum fd_version fd_device_version(struct fd_device *dev)
{
	return dev->version;
}
