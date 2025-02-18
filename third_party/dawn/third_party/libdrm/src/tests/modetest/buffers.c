/*
 * DRM based mode setting test program
 * Copyright 2008 Tungsten Graphics
 *   Jakob Bornecrantz <jakob@tungstengraphics.com>
 * Copyright 2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>

#include "drm.h"
#include "drm_fourcc.h"

#include "libdrm_macros.h"
#include "xf86drm.h"
#include "xf86drmMode.h"

#include "buffers.h"

struct bo
{
	int fd;
	void *ptr;
	uint64_t size;
	uint32_t pitch;
	uint32_t handle;
};

/* -----------------------------------------------------------------------------
 * Buffers management
 */

static struct bo *
bo_create_dumb(int fd, unsigned int width, unsigned int height, unsigned int bpp)
{
	struct bo *bo;
	int ret;

	bo = calloc(1, sizeof(*bo));
	if (bo == NULL) {
		fprintf(stderr, "failed to allocate buffer object\n");
		return NULL;
	}

	ret = drmModeCreateDumbBuffer(fd, width, height, bpp, 0, &bo->handle,
				      &bo->pitch, &bo->size);
	if (ret) {
		fprintf(stderr, "failed to create dumb buffer: %s\n",
			strerror(errno));
		free(bo);
		return NULL;
	}

	bo->fd = fd;

	return bo;
}

static int bo_map(struct bo *bo, void **out)
{
	void *map;
	int ret;
	uint64_t offset;

	ret = drmModeMapDumbBuffer(bo->fd, bo->handle, &offset);
	if (ret)
		return ret;

	map = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		       bo->fd, offset);
	if (map == MAP_FAILED)
		return -EINVAL;

	bo->ptr = map;
	*out = map;

	return 0;
}

static void bo_unmap(struct bo *bo)
{
	if (!bo->ptr)
		return;

	drm_munmap(bo->ptr, bo->size);
	bo->ptr = NULL;
}

struct bo *
bo_create(int fd, unsigned int format,
	  unsigned int width, unsigned int height,
	  unsigned int handles[4], unsigned int pitches[4],
	  unsigned int offsets[4], enum util_fill_pattern pattern)
{
	unsigned int virtual_height;
	struct bo *bo;
	unsigned int bpp;
	void *planes[3] = { 0, };
	void *virtual;
	int ret;

	switch (format) {
	case DRM_FORMAT_C1:
		bpp = 1;
		break;

	case DRM_FORMAT_C2:
		bpp = 2;
		break;

	case DRM_FORMAT_C4:
		bpp = 4;
		break;

	case DRM_FORMAT_C8:
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_NV24:
	case DRM_FORMAT_NV42:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		bpp = 8;
		break;

	case DRM_FORMAT_NV15:
	case DRM_FORMAT_NV20:
	case DRM_FORMAT_NV30:
		bpp = 10;
		break;

	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_XRGB1555 | DRM_FORMAT_BIG_ENDIAN:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_RGB565 | DRM_FORMAT_BIG_ENDIAN:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		bpp = 16;
		break;

	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
		bpp = 24;
		break;

	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
		bpp = 32;
		break;

	case DRM_FORMAT_XRGB16161616F:
	case DRM_FORMAT_XBGR16161616F:
	case DRM_FORMAT_ARGB16161616F:
	case DRM_FORMAT_ABGR16161616F:
		bpp = 64;
		break;

	default:
		fprintf(stderr, "unsupported format 0x%08x\n",  format);
		return NULL;
	}

	switch (format) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV15:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		virtual_height = height * 3 / 2;
		break;

	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_NV20:
		virtual_height = height * 2;
		break;

	case DRM_FORMAT_NV24:
	case DRM_FORMAT_NV42:
	case DRM_FORMAT_NV30:
		virtual_height = height * 3;
		break;

	default:
		virtual_height = height;
		break;
	}

	bo = bo_create_dumb(fd, width, virtual_height, bpp);
	if (!bo)
		return NULL;

	ret = bo_map(bo, &virtual);
	if (ret) {
		fprintf(stderr, "failed to map buffer: %s\n",
			strerror(-errno));
		bo_destroy(bo);
		return NULL;
	}

	/* just testing a limited # of formats to test single
	 * and multi-planar path.. would be nice to add more..
	 */
	switch (format) {
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;

		planes[0] = virtual;
		break;

	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_NV15:
	case DRM_FORMAT_NV20:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0];
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		break;

	case DRM_FORMAT_NV24:
	case DRM_FORMAT_NV42:
	case DRM_FORMAT_NV30:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0] * 2;
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		break;

	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;
		pitches[1] = pitches[0] / 2;
		offsets[1] = pitches[0] * height;
		handles[1] = bo->handle;
		pitches[2] = pitches[1];
		offsets[2] = offsets[1] + pitches[1] * height / 2;
		handles[2] = bo->handle;

		planes[0] = virtual;
		planes[1] = virtual + offsets[1];
		planes[2] = virtual + offsets[2];
		break;

	case DRM_FORMAT_C1:
	case DRM_FORMAT_C2:
	case DRM_FORMAT_C4:
	case DRM_FORMAT_C8:
	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_XRGB1555 | DRM_FORMAT_BIG_ENDIAN:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_RGB565 | DRM_FORMAT_BIG_ENDIAN:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_XBGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_RGBX1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_BGRX1010102:
	case DRM_FORMAT_XRGB16161616F:
	case DRM_FORMAT_XBGR16161616F:
	case DRM_FORMAT_ARGB16161616F:
	case DRM_FORMAT_ABGR16161616F:
		offsets[0] = 0;
		handles[0] = bo->handle;
		pitches[0] = bo->pitch;

		planes[0] = virtual;
		break;
	}

	util_fill_pattern(format, pattern, planes, width, height, pitches[0]);
	bo_unmap(bo);

	return bo;
}

void bo_destroy(struct bo *bo)
{
	int ret;

	ret = drmModeDestroyDumbBuffer(bo->fd, bo->handle);
	if (ret)
		fprintf(stderr, "failed to destroy dumb buffer: %s\n",
			strerror(errno));

	free(bo);
}

void bo_dump(struct bo *bo, const char *filename)
{
	FILE *fp;

	if (!bo || !filename)
		return;

	fp = fopen(filename, "wb");
	if (fp) {
		void *addr;

		bo_map(bo, &addr);
		printf("Dumping buffer %p to file %s.\n", bo->ptr, filename);
		fwrite(bo->ptr, 1, bo->size, fp);
		bo_unmap(bo);
		fclose(fp);
	}
}
