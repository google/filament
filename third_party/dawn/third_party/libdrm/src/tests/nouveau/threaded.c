/*
 * Copyright © 2015 Canonical Ltd. (Maarten Lankhorst)
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
 */

#include <sys/ioctl.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "xf86drm.h"
#include "nouveau.h"

static __typeof__(ioctl) *old_ioctl;
static int failed;

static int import_fd;

#if defined(__GLIBC__) || defined(__FreeBSD__)
int ioctl(int fd, unsigned long request, ...)
#else
int ioctl(int fd, int request, ...)
#endif
{
	va_list va;
	int ret;
	void *arg;

	va_start(va, request);
	arg = va_arg(va, void *);
	ret = old_ioctl(fd, request, arg);
	va_end(va);

	if (ret < 0 && request == DRM_IOCTL_GEM_CLOSE && errno == EINVAL)
		failed = 1;

	return ret;
}

static void *
openclose(void *dev)
{
	struct nouveau_device *nvdev = dev;
	struct nouveau_bo *bo = NULL;
	int i;

	for (i = 0; i < 100000; ++i) {
		if (!nouveau_bo_prime_handle_ref(nvdev, import_fd, &bo))
			nouveau_bo_ref(NULL, &bo);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	drmVersionPtr version;
	const char *device = NULL;
	int err, fd, fd2;
	struct nouveau_device *nvdev, *nvdev2;
	struct nouveau_bo *bo;
	pthread_t t1, t2;

	old_ioctl = dlsym(RTLD_NEXT, "ioctl");

	if (argc < 2) {
		fd = drmOpenWithType("nouveau", NULL, DRM_NODE_RENDER);
		if (fd >= 0)
			fd2 = drmOpenWithType("nouveau", NULL, DRM_NODE_RENDER);
	} else {
		device = argv[1];

		fd = open(device, O_RDWR);
		if (fd >= 0)
			fd2 = open(device, O_RDWR);
		else
			fd2 = fd = -errno;
	}

	if (fd < 0) {
		fprintf(stderr, "Opening nouveau render node failed with %i\n", fd);
		return device ? -fd : 77;
	}

	if (fd2 < 0) {
		fprintf(stderr, "Opening second nouveau render node failed with %i\n", -errno);
		return errno;
	}

	version = drmGetVersion(fd);
	if (version) {
		printf("Version: %d.%d.%d\n", version->version_major,
		       version->version_minor, version->version_patchlevel);
		printf("  Name: %s\n", version->name);
		printf("  Date: %s\n", version->date);
		printf("  Description: %s\n", version->desc);

		drmFreeVersion(version);
	}

	err = nouveau_device_wrap(fd, 0, &nvdev);
	if (!err)
		err = nouveau_device_wrap(fd2, 0, &nvdev2);
	if (err < 0)
		return 1;

	err = nouveau_bo_new(nvdev2, NOUVEAU_BO_GART, 0, 4096, NULL, &bo);
	if (!err)
		err = nouveau_bo_set_prime(bo, &import_fd);

	if (!err) {
		pthread_create(&t1, NULL, openclose, nvdev);
		pthread_create(&t2, NULL, openclose, nvdev);
	}

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	close(import_fd);
	nouveau_bo_ref(NULL, &bo);

	nouveau_device_del(&nvdev2);
	nouveau_device_del(&nvdev);
	if (device) {
		close(fd2);
		close(fd);
	} else {
		drmClose(fd2);
		drmClose(fd);
	}

	if (failed)
		fprintf(stderr, "DRM_IOCTL_GEM_CLOSE failed with EINVAL,\n"
				"race in opening/closing bo is likely.\n");

	return failed;
}
