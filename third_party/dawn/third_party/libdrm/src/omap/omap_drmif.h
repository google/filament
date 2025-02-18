/*
 * Copyright (C) 2011 Texas Instruments, Inc
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
 *    Rob Clark <rob@ti.com>
 */

#ifndef OMAP_DRMIF_H_
#define OMAP_DRMIF_H_

#include <xf86drm.h>
#include <stdint.h>
#include <omap_drm.h>

struct omap_bo;
struct omap_device;

/* device related functions:
 */

struct omap_device * omap_device_new(int fd);
struct omap_device * omap_device_ref(struct omap_device *dev);
void omap_device_del(struct omap_device *dev);
int omap_get_param(struct omap_device *dev, uint64_t param, uint64_t *value);
int omap_set_param(struct omap_device *dev, uint64_t param, uint64_t value);

/* buffer-object related functions:
 */

struct omap_bo * omap_bo_new(struct omap_device *dev,
		uint32_t size, uint32_t flags);
struct omap_bo * omap_bo_new_tiled(struct omap_device *dev,
		uint32_t width, uint32_t height, uint32_t flags);
struct omap_bo * omap_bo_ref(struct omap_bo *bo);
struct omap_bo * omap_bo_from_name(struct omap_device *dev, uint32_t name);
struct omap_bo * omap_bo_from_dmabuf(struct omap_device *dev, int fd);
void omap_bo_del(struct omap_bo *bo);
int omap_bo_get_name(struct omap_bo *bo, uint32_t *name);
uint32_t omap_bo_handle(struct omap_bo *bo);
int omap_bo_dmabuf(struct omap_bo *bo);
uint32_t omap_bo_size(struct omap_bo *bo);
void * omap_bo_map(struct omap_bo *bo);
int omap_bo_cpu_prep(struct omap_bo *bo, enum omap_gem_op op);
int omap_bo_cpu_fini(struct omap_bo *bo, enum omap_gem_op op);

#endif /* OMAP_DRMIF_H_ */
