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

#ifndef UTIL_PATTERN_H
#define UTIL_PATTERN_H

#include <drm_mode.h>

enum util_fill_pattern {
	UTIL_PATTERN_TILES,
	UTIL_PATTERN_PLAIN,
	UTIL_PATTERN_SMPTE,
	UTIL_PATTERN_GRADIENT,
};

void util_fill_pattern(uint32_t format, enum util_fill_pattern pattern,
		       void *planes[3], unsigned int width,
		       unsigned int height, unsigned int stride);

void util_smpte_fill_lut(unsigned int ncolors, struct drm_color_lut *lut);

enum util_fill_pattern util_pattern_enum(const char *name);

#endif /* UTIL_PATTERN_H */
