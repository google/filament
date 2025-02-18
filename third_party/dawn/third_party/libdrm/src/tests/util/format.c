/*
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <drm_fourcc.h>

#include "common.h"
#include "format.h"

#define MAKE_RGB_INFO(rl, ro, gl, go, bl, bo, al, ao) \
	.rgb = { { (rl), (ro) }, { (gl), (go) }, { (bl), (bo) }, { (al), (ao) } }

#define MAKE_YUV_INFO(order, xsub, ysub, chroma_stride) \
	.yuv = { (order), (xsub), (ysub), (chroma_stride) }

static const struct util_format_info format_info[] = {
	/* Indexed */
	{ DRM_FORMAT_C1, "C1", .ncolors = 2 },
	{ DRM_FORMAT_C2, "C2", .ncolors = 4 },
	{ DRM_FORMAT_C4, "C4", .ncolors = 16 },
	{ DRM_FORMAT_C8, "C8", .ncolors = 256 },
	/* YUV packed */
	{ DRM_FORMAT_UYVY, "UYVY", MAKE_YUV_INFO(YUV_YCbCr | YUV_CY, 2, 2, 2) },
	{ DRM_FORMAT_VYUY, "VYUY", MAKE_YUV_INFO(YUV_YCrCb | YUV_CY, 2, 2, 2) },
	{ DRM_FORMAT_YUYV, "YUYV", MAKE_YUV_INFO(YUV_YCbCr | YUV_YC, 2, 2, 2) },
	{ DRM_FORMAT_YVYU, "YVYU", MAKE_YUV_INFO(YUV_YCrCb | YUV_YC, 2, 2, 2) },
	/* YUV semi-planar */
	{ DRM_FORMAT_NV12, "NV12", MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 2) },
	{ DRM_FORMAT_NV21, "NV21", MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 2) },
	{ DRM_FORMAT_NV16, "NV16", MAKE_YUV_INFO(YUV_YCbCr, 2, 1, 2) },
	{ DRM_FORMAT_NV61, "NV61", MAKE_YUV_INFO(YUV_YCrCb, 2, 1, 2) },
	{ DRM_FORMAT_NV24, "NV24", MAKE_YUV_INFO(YUV_YCbCr, 1, 1, 2) },
	{ DRM_FORMAT_NV42, "NV42", MAKE_YUV_INFO(YUV_YCrCb, 1, 1, 2) },
	{ DRM_FORMAT_NV15, "NV15", MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 2) },
	{ DRM_FORMAT_NV20, "NV20", MAKE_YUV_INFO(YUV_YCbCr, 2, 1, 2) },
	{ DRM_FORMAT_NV30, "NV30", MAKE_YUV_INFO(YUV_YCbCr, 1, 1, 2) },
	/* YUV planar */
	{ DRM_FORMAT_YUV420, "YU12", MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 1) },
	{ DRM_FORMAT_YVU420, "YV12", MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 1) },
	/* RGB16 */
	{ DRM_FORMAT_ARGB4444, "AR12", MAKE_RGB_INFO(4, 8, 4, 4, 4, 0, 4, 12) },
	{ DRM_FORMAT_XRGB4444, "XR12", MAKE_RGB_INFO(4, 8, 4, 4, 4, 0, 0, 0) },
	{ DRM_FORMAT_ABGR4444, "AB12", MAKE_RGB_INFO(4, 0, 4, 4, 4, 8, 4, 12) },
	{ DRM_FORMAT_XBGR4444, "XB12", MAKE_RGB_INFO(4, 0, 4, 4, 4, 8, 0, 0) },
	{ DRM_FORMAT_RGBA4444, "RA12", MAKE_RGB_INFO(4, 12, 4, 8, 4, 4, 4, 0) },
	{ DRM_FORMAT_RGBX4444, "RX12", MAKE_RGB_INFO(4, 12, 4, 8, 4, 4, 0, 0) },
	{ DRM_FORMAT_BGRA4444, "BA12", MAKE_RGB_INFO(4, 4, 4, 8, 4, 12, 4, 0) },
	{ DRM_FORMAT_BGRX4444, "BX12", MAKE_RGB_INFO(4, 4, 4, 8, 4, 12, 0, 0) },
	{ DRM_FORMAT_ARGB1555, "AR15", MAKE_RGB_INFO(5, 10, 5, 5, 5, 0, 1, 15) },
	{ DRM_FORMAT_XRGB1555, "XR15", MAKE_RGB_INFO(5, 10, 5, 5, 5, 0, 0, 0) },
	{ DRM_FORMAT_ABGR1555, "AB15", MAKE_RGB_INFO(5, 0, 5, 5, 5, 10, 1, 15) },
	{ DRM_FORMAT_XBGR1555, "XB15", MAKE_RGB_INFO(5, 0, 5, 5, 5, 10, 0, 0) },
	{ DRM_FORMAT_RGBA5551, "RA15", MAKE_RGB_INFO(5, 11, 5, 6, 5, 1, 1, 0) },
	{ DRM_FORMAT_RGBX5551, "RX15", MAKE_RGB_INFO(5, 11, 5, 6, 5, 1, 0, 0) },
	{ DRM_FORMAT_BGRA5551, "BA15", MAKE_RGB_INFO(5, 1, 5, 6, 5, 11, 1, 0) },
	{ DRM_FORMAT_BGRX5551, "BX15", MAKE_RGB_INFO(5, 1, 5, 6, 5, 11, 0, 0) },
	{ DRM_FORMAT_RGB565, "RG16", MAKE_RGB_INFO(5, 11, 6, 5, 5, 0, 0, 0) },
	{ DRM_FORMAT_BGR565, "BG16", MAKE_RGB_INFO(5, 0, 6, 5, 5, 11, 0, 0) },
	/* Big-endian RGB16 */
	{ DRM_FORMAT_XRGB1555 | DRM_FORMAT_BIG_ENDIAN, "XR15_BE", MAKE_RGB_INFO(5, 10, 5, 5, 5, 0, 0, 0) },
	{ DRM_FORMAT_RGB565 | DRM_FORMAT_BIG_ENDIAN, "RG16_BE", MAKE_RGB_INFO(5, 11, 6, 5, 5, 0, 0, 0) },
	/* RGB24 */
	{ DRM_FORMAT_BGR888, "BG24", MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 0, 0) },
	{ DRM_FORMAT_RGB888, "RG24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 0, 0) },
	/* RGB32 */
	{ DRM_FORMAT_ARGB8888, "AR24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 8, 24) },
	{ DRM_FORMAT_XRGB8888, "XR24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 0, 0) },
	{ DRM_FORMAT_ABGR8888, "AB24", MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 8, 24) },
	{ DRM_FORMAT_XBGR8888, "XB24", MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 0, 0) },
	{ DRM_FORMAT_RGBA8888, "RA24", MAKE_RGB_INFO(8, 24, 8, 16, 8, 8, 8, 0) },
	{ DRM_FORMAT_RGBX8888, "RX24", MAKE_RGB_INFO(8, 24, 8, 16, 8, 8, 0, 0) },
	{ DRM_FORMAT_BGRA8888, "BA24", MAKE_RGB_INFO(8, 8, 8, 16, 8, 24, 8, 0) },
	{ DRM_FORMAT_BGRX8888, "BX24", MAKE_RGB_INFO(8, 8, 8, 16, 8, 24, 0, 0) },
	{ DRM_FORMAT_ARGB2101010, "AR30", MAKE_RGB_INFO(10, 20, 10, 10, 10, 0, 2, 30) },
	{ DRM_FORMAT_XRGB2101010, "XR30", MAKE_RGB_INFO(10, 20, 10, 10, 10, 0, 0, 0) },
	{ DRM_FORMAT_ABGR2101010, "AB30", MAKE_RGB_INFO(10, 0, 10, 10, 10, 20, 2, 30) },
	{ DRM_FORMAT_XBGR2101010, "XB30", MAKE_RGB_INFO(10, 0, 10, 10, 10, 20, 0, 0) },
	{ DRM_FORMAT_RGBA1010102, "RA30", MAKE_RGB_INFO(10, 22, 10, 12, 10, 2, 2, 0) },
	{ DRM_FORMAT_RGBX1010102, "RX30", MAKE_RGB_INFO(10, 22, 10, 12, 10, 2, 0, 0) },
	{ DRM_FORMAT_BGRA1010102, "BA30", MAKE_RGB_INFO(10, 2, 10, 12, 10, 22, 2, 0) },
	{ DRM_FORMAT_BGRX1010102, "BX30", MAKE_RGB_INFO(10, 2, 10, 12, 10, 22, 0, 0) },
	{ DRM_FORMAT_XRGB16161616F, "XR4H", MAKE_RGB_INFO(16, 32, 16, 16, 16, 0, 0, 0) },
	{ DRM_FORMAT_XBGR16161616F, "XB4H", MAKE_RGB_INFO(16, 0, 16, 16, 16, 32, 0, 0) },
	{ DRM_FORMAT_ARGB16161616F, "AR4H", MAKE_RGB_INFO(16, 32, 16, 16, 16, 0, 16, 48) },
	{ DRM_FORMAT_ABGR16161616F, "AB4H", MAKE_RGB_INFO(16, 0, 16, 16, 16, 32, 16, 48) },

};

uint32_t util_format_fourcc(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(format_info); i++)
		if (!strcmp(format_info[i].name, name))
			return format_info[i].format;

	return 0;
}

const struct util_format_info *util_format_info_find(uint32_t format)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(format_info); i++)
		if (format_info[i].format == format)
			return &format_info[i];

	return NULL;
}
