/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

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

#ifndef __OMAP_DRM_H__
#define __OMAP_DRM_H__

#include <stdint.h>
#include <drm.h>

/* Please note that modifications to all structs defined here are
 * subject to backwards-compatibility constraints.
 */

#define OMAP_PARAM_CHIPSET_ID	1	/* ie. 0x3430, 0x4430, etc */

struct drm_omap_param {
	uint64_t param;			/* in */
	uint64_t value;			/* in (set_param), out (get_param) */
};

struct drm_omap_get_base {
	char plugin_name[64];		/* in */
	uint32_t ioctl_base;		/* out */
	uint32_t __pad;
};

#define OMAP_BO_SCANOUT		0x00000001	/* scanout capable (phys contiguous) */
#define OMAP_BO_CACHE_MASK	0x00000006	/* cache type mask, see cache modes */
#define OMAP_BO_TILED_MASK	0x00000f00	/* tiled mapping mask, see tiled modes */

/* cache modes */
#define OMAP_BO_CACHED		0x00000000	/* default */
#define OMAP_BO_WC		0x00000002	/* write-combine */
#define OMAP_BO_UNCACHED	0x00000004	/* strongly-ordered (uncached) */

/* tiled modes */
#define OMAP_BO_TILED_8		0x00000100
#define OMAP_BO_TILED_16	0x00000200
#define OMAP_BO_TILED_32	0x00000300
#define OMAP_BO_TILED		(OMAP_BO_TILED_8 | OMAP_BO_TILED_16 | OMAP_BO_TILED_32)

union omap_gem_size {
	uint32_t bytes;		/* (for non-tiled formats) */
	struct {
		uint16_t width;
		uint16_t height;
	} tiled;		/* (for tiled formats) */
};

struct drm_omap_gem_new {
	union omap_gem_size size;	/* in */
	uint32_t flags;			/* in */
	uint32_t handle;		/* out */
	uint32_t __pad;
};

/* mask of operations: */
enum omap_gem_op {
	OMAP_GEM_READ = 0x01,
	OMAP_GEM_WRITE = 0x02,
};

struct drm_omap_gem_cpu_prep {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t op;			/* mask of omap_gem_op (in) */
};

struct drm_omap_gem_cpu_fini {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t op;			/* mask of omap_gem_op (in) */
	/* TODO maybe here we pass down info about what regions are touched
	 * by sw so we can be clever about cache ops?  For now a placeholder,
	 * set to zero and we just do full buffer flush..
	 */
	uint32_t nregions;
	uint32_t __pad;
};

struct drm_omap_gem_info {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t pad;
	uint64_t offset;		/* mmap offset (out) */
	/* note: in case of tiled buffers, the user virtual size can be
	 * different from the physical size (ie. how many pages are needed
	 * to back the object) which is returned in DRM_IOCTL_GEM_OPEN..
	 * This size here is the one that should be used if you want to
	 * mmap() the buffer:
	 */
	uint32_t size;			/* virtual size for mmap'ing (out) */
	uint32_t __pad;
};

#define DRM_OMAP_GET_PARAM		0x00
#define DRM_OMAP_SET_PARAM		0x01
#define DRM_OMAP_GET_BASE		0x02
#define DRM_OMAP_GEM_NEW		0x03
#define DRM_OMAP_GEM_CPU_PREP		0x04
#define DRM_OMAP_GEM_CPU_FINI		0x05
#define DRM_OMAP_GEM_INFO		0x06
#define DRM_OMAP_NUM_IOCTLS		0x07

#define DRM_IOCTL_OMAP_GET_PARAM	DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GET_PARAM, struct drm_omap_param)
#define DRM_IOCTL_OMAP_SET_PARAM	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_SET_PARAM, struct drm_omap_param)
#define DRM_IOCTL_OMAP_GET_BASE		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GET_BASE, struct drm_omap_get_base)
#define DRM_IOCTL_OMAP_GEM_NEW		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GEM_NEW, struct drm_omap_gem_new)
#define DRM_IOCTL_OMAP_GEM_CPU_PREP	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_GEM_CPU_PREP, struct drm_omap_gem_cpu_prep)
#define DRM_IOCTL_OMAP_GEM_CPU_FINI	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_GEM_CPU_FINI, struct drm_omap_gem_cpu_fini)
#define DRM_IOCTL_OMAP_GEM_INFO		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GEM_INFO, struct drm_omap_gem_info)

#endif /* __OMAP_DRM_H__ */
