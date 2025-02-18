/*
 * Copyright (C) 2016 Etnaviv Project
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

#undef NDEBUG
#include <assert.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "xf86drm.h"
#include "etnaviv_drmif.h"
#include "etnaviv_drm.h"

static void test_cache(struct etna_device *dev)
{
	struct etna_bo *bo, *tmp;

	/* allocate and free some bo's with same size - we must
	 * get the same bo over and over. */
	printf("testing bo cache ... ");

	bo = tmp = etna_bo_new(dev, 0x100, ETNA_BO_UNCACHED);
	assert(bo);
	etna_bo_del(bo);

	for (unsigned i = 0; i < 100; i++) {
		tmp = etna_bo_new(dev, 0x100, ETNA_BO_UNCACHED);
		etna_bo_del(tmp);
		assert(tmp == bo);
	}

	printf("ok\n");
}

static void test_size_rounding(struct etna_device *dev)
{
	struct etna_bo *bo;

	printf("testing size rounding ... ");

	bo = etna_bo_new(dev, 15, ETNA_BO_UNCACHED);
	assert(etna_bo_size(bo) == 4096);
	etna_bo_del(bo);

	bo = etna_bo_new(dev, 4096, ETNA_BO_UNCACHED);
	assert(etna_bo_size(bo) == 4096);
	etna_bo_del(bo);

	bo = etna_bo_new(dev, 4100, ETNA_BO_UNCACHED);
	assert(etna_bo_size(bo) == 8192);
	etna_bo_del(bo);

	printf("ok\n");
}

int main(int argc, char *argv[])
{
	struct etna_device *dev;

	drmVersionPtr version;
	int fd, ret = 0;

	fd = open(argv[1], O_RDWR);
	if (fd < 0)
		return 1;

	version = drmGetVersion(fd);
	if (version) {
		printf("Version: %d.%d.%d\n", version->version_major,
		       version->version_minor, version->version_patchlevel);
		printf("  Name: %s\n", version->name);
		printf("  Date: %s\n", version->date);
		printf("  Description: %s\n", version->desc);
		drmFreeVersion(version);
	}

	dev = etna_device_new(fd);
	if (!dev) {
		ret = 2;
		goto out;
	}

	test_cache(dev);
	test_size_rounding(dev);

	etna_device_del(dev);

out:
	close(fd);

	return ret;
}
