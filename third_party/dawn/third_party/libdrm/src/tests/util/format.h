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

#ifndef UTIL_FORMAT_H
#define UTIL_FORMAT_H

struct util_color_component {
	unsigned int length;
	unsigned int offset;
};

struct util_rgb_info {
	struct util_color_component red;
	struct util_color_component green;
	struct util_color_component blue;
	struct util_color_component alpha;
};

enum util_yuv_order {
	YUV_YCbCr = 1,
	YUV_YCrCb = 2,
	YUV_YC = 4,
	YUV_CY = 8,
};

struct util_yuv_info {
	enum util_yuv_order order;
	unsigned int xsub;
	unsigned int ysub;
	unsigned int chroma_stride;
};

struct util_format_info {
	uint32_t format;
	const char *name;
	unsigned int ncolors;
	const struct util_rgb_info rgb;
	const struct util_yuv_info yuv;
};

uint32_t util_format_fourcc(const char *name);
const struct util_format_info *util_format_info_find(uint32_t format);

#endif /* UTIL_FORMAT_H */
