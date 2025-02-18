/*
 * Copyright © 2014 NVIDIA Corporation
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

#ifndef TEGRA_DRM_TEST_TEGRA_H
#define TEGRA_DRM_TEST_TEGRA_H

#include "drm-test.h"
#include "tegra.h"

#define HOST1X_OPCODE_SETCL(offset, classid, mask) \
    ((0x0 << 28) | (((offset) & 0xfff) << 16) | (((classid) & 0x3ff) << 6) | ((mask) & 0x3f))
#define HOST1X_OPCODE_INCR(offset, count) \
    ((0x1 << 28) | (((offset) & 0xfff) << 16) | ((count) & 0xffff))
#define HOST1X_OPCODE_NONINCR(offset, count) \
    ((0x2 << 28) | (((offset) & 0xfff) << 16) | ((count) & 0xffff))
#define HOST1X_OPCODE_MASK(offset, mask) \
    ((0x3 << 28) | (((offset) & 0xfff) << 16) | ((mask) & 0xffff))
#define HOST1X_OPCODE_IMM(offset, data) \
    ((0x4 << 28) | (((offset) & 0xfff) << 16) | ((data) & 0xffff))
#define HOST1X_OPCODE_EXTEND(subop, value) \
    ((0xe << 28) | (((subop) & 0xf) << 24) | ((value) & 0xffffff))

#define HOST1X_CLASS_GR2D 0x51

struct drm_tegra_gr2d {
    struct drm_tegra *drm;
    struct drm_tegra_channel *channel;
};

int drm_tegra_gr2d_open(struct drm_tegra *drm, struct drm_tegra_gr2d **gr2dp);
int drm_tegra_gr2d_close(struct drm_tegra_gr2d *gr2d);
int drm_tegra_gr2d_fill(struct drm_tegra_gr2d *gr2d, struct drm_framebuffer *fb,
                        unsigned int x, unsigned int y, unsigned int width,
                        unsigned int height, uint32_t color);

#endif
