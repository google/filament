/*
 * Copyright © 2007 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <drm.h>
#include <i915_drm.h>
#ifndef __ANDROID__
#include <pciaccess.h>
#endif
#include "libdrm_macros.h"
#include "intel_bufmgr.h"
#include "intel_bufmgr_priv.h"
#include "xf86drm.h"

/** @file intel_bufmgr.c
 *
 * Convenience functions for buffer management methods.
 */

drm_public drm_intel_bo *
drm_intel_bo_alloc(drm_intel_bufmgr *bufmgr, const char *name,
		   unsigned long size, unsigned int alignment)
{
	return bufmgr->bo_alloc(bufmgr, name, size, alignment);
}

drm_public drm_intel_bo *
drm_intel_bo_alloc_for_render(drm_intel_bufmgr *bufmgr, const char *name,
			      unsigned long size, unsigned int alignment)
{
	return bufmgr->bo_alloc_for_render(bufmgr, name, size, alignment);
}

drm_public drm_intel_bo *
drm_intel_bo_alloc_userptr(drm_intel_bufmgr *bufmgr,
			   const char *name, void *addr,
			   uint32_t tiling_mode,
			   uint32_t stride,
			   unsigned long size,
			   unsigned long flags)
{
	if (bufmgr->bo_alloc_userptr)
		return bufmgr->bo_alloc_userptr(bufmgr, name, addr, tiling_mode,
						stride, size, flags);
	return NULL;
}

drm_public drm_intel_bo *
drm_intel_bo_alloc_tiled(drm_intel_bufmgr *bufmgr, const char *name,
                        int x, int y, int cpp, uint32_t *tiling_mode,
                        unsigned long *pitch, unsigned long flags)
{
	return bufmgr->bo_alloc_tiled(bufmgr, name, x, y, cpp,
				      tiling_mode, pitch, flags);
}

drm_public void
drm_intel_bo_reference(drm_intel_bo *bo)
{
	bo->bufmgr->bo_reference(bo);
}

drm_public void
drm_intel_bo_unreference(drm_intel_bo *bo)
{
	if (bo == NULL)
		return;

	bo->bufmgr->bo_unreference(bo);
}

drm_public int
drm_intel_bo_map(drm_intel_bo *buf, int write_enable)
{
	return buf->bufmgr->bo_map(buf, write_enable);
}

drm_public int
drm_intel_bo_unmap(drm_intel_bo *buf)
{
	return buf->bufmgr->bo_unmap(buf);
}

drm_public int
drm_intel_bo_subdata(drm_intel_bo *bo, unsigned long offset,
		     unsigned long size, const void *data)
{
	return bo->bufmgr->bo_subdata(bo, offset, size, data);
}

drm_public int
drm_intel_bo_get_subdata(drm_intel_bo *bo, unsigned long offset,
			 unsigned long size, void *data)
{
	int ret;
	if (bo->bufmgr->bo_get_subdata)
		return bo->bufmgr->bo_get_subdata(bo, offset, size, data);

	if (size == 0 || data == NULL)
		return 0;

	ret = drm_intel_bo_map(bo, 0);
	if (ret)
		return ret;
	memcpy(data, (unsigned char *)bo->virtual + offset, size);
	drm_intel_bo_unmap(bo);
	return 0;
}

drm_public void
drm_intel_bo_wait_rendering(drm_intel_bo *bo)
{
	bo->bufmgr->bo_wait_rendering(bo);
}

drm_public void
drm_intel_bufmgr_destroy(drm_intel_bufmgr *bufmgr)
{
	bufmgr->destroy(bufmgr);
}

drm_public int
drm_intel_bo_exec(drm_intel_bo *bo, int used,
		  drm_clip_rect_t * cliprects, int num_cliprects, int DR4)
{
	return bo->bufmgr->bo_exec(bo, used, cliprects, num_cliprects, DR4);
}

drm_public int
drm_intel_bo_mrb_exec(drm_intel_bo *bo, int used,
		drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
		unsigned int rings)
{
	if (bo->bufmgr->bo_mrb_exec)
		return bo->bufmgr->bo_mrb_exec(bo, used,
					cliprects, num_cliprects, DR4,
					rings);

	switch (rings) {
	case I915_EXEC_DEFAULT:
	case I915_EXEC_RENDER:
		return bo->bufmgr->bo_exec(bo, used,
					   cliprects, num_cliprects, DR4);
	default:
		return -ENODEV;
	}
}

drm_public void
drm_intel_bufmgr_set_debug(drm_intel_bufmgr *bufmgr, int enable_debug)
{
	bufmgr->debug = enable_debug;
}

drm_public int
drm_intel_bufmgr_check_aperture_space(drm_intel_bo ** bo_array, int count)
{
	return bo_array[0]->bufmgr->check_aperture_space(bo_array, count);
}

drm_public int
drm_intel_bo_flink(drm_intel_bo *bo, uint32_t * name)
{
	if (bo->bufmgr->bo_flink)
		return bo->bufmgr->bo_flink(bo, name);

	return -ENODEV;
}

drm_public int
drm_intel_bo_emit_reloc(drm_intel_bo *bo, uint32_t offset,
			drm_intel_bo *target_bo, uint32_t target_offset,
			uint32_t read_domains, uint32_t write_domain)
{
	return bo->bufmgr->bo_emit_reloc(bo, offset,
					 target_bo, target_offset,
					 read_domains, write_domain);
}

/* For fence registers, not GL fences */
drm_public int
drm_intel_bo_emit_reloc_fence(drm_intel_bo *bo, uint32_t offset,
			      drm_intel_bo *target_bo, uint32_t target_offset,
			      uint32_t read_domains, uint32_t write_domain)
{
	return bo->bufmgr->bo_emit_reloc_fence(bo, offset,
					       target_bo, target_offset,
					       read_domains, write_domain);
}


drm_public int
drm_intel_bo_pin(drm_intel_bo *bo, uint32_t alignment)
{
	if (bo->bufmgr->bo_pin)
		return bo->bufmgr->bo_pin(bo, alignment);

	return -ENODEV;
}

drm_public int
drm_intel_bo_unpin(drm_intel_bo *bo)
{
	if (bo->bufmgr->bo_unpin)
		return bo->bufmgr->bo_unpin(bo);

	return -ENODEV;
}

drm_public int
drm_intel_bo_set_tiling(drm_intel_bo *bo, uint32_t * tiling_mode,
			uint32_t stride)
{
	if (bo->bufmgr->bo_set_tiling)
		return bo->bufmgr->bo_set_tiling(bo, tiling_mode, stride);

	*tiling_mode = I915_TILING_NONE;
	return 0;
}

drm_public int
drm_intel_bo_get_tiling(drm_intel_bo *bo, uint32_t * tiling_mode,
			uint32_t * swizzle_mode)
{
	if (bo->bufmgr->bo_get_tiling)
		return bo->bufmgr->bo_get_tiling(bo, tiling_mode, swizzle_mode);

	*tiling_mode = I915_TILING_NONE;
	*swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
	return 0;
}

drm_public int
drm_intel_bo_set_softpin_offset(drm_intel_bo *bo, uint64_t offset)
{
	if (bo->bufmgr->bo_set_softpin_offset)
		return bo->bufmgr->bo_set_softpin_offset(bo, offset);

	return -ENODEV;
}

drm_public int
drm_intel_bo_disable_reuse(drm_intel_bo *bo)
{
	if (bo->bufmgr->bo_disable_reuse)
		return bo->bufmgr->bo_disable_reuse(bo);
	return 0;
}

drm_public int
drm_intel_bo_is_reusable(drm_intel_bo *bo)
{
	if (bo->bufmgr->bo_is_reusable)
		return bo->bufmgr->bo_is_reusable(bo);
	return 0;
}

drm_public int
drm_intel_bo_busy(drm_intel_bo *bo)
{
	if (bo->bufmgr->bo_busy)
		return bo->bufmgr->bo_busy(bo);
	return 0;
}

drm_public int
drm_intel_bo_madvise(drm_intel_bo *bo, int madv)
{
	if (bo->bufmgr->bo_madvise)
		return bo->bufmgr->bo_madvise(bo, madv);
	return -1;
}

drm_public int
drm_intel_bo_use_48b_address_range(drm_intel_bo *bo, uint32_t enable)
{
	if (bo->bufmgr->bo_use_48b_address_range) {
		bo->bufmgr->bo_use_48b_address_range(bo, enable);
		return 0;
	}

	return -ENODEV;
}

drm_public int
drm_intel_bo_references(drm_intel_bo *bo, drm_intel_bo *target_bo)
{
	return bo->bufmgr->bo_references(bo, target_bo);
}

drm_public int
drm_intel_get_pipe_from_crtc_id(drm_intel_bufmgr *bufmgr, int crtc_id)
{
	if (bufmgr->get_pipe_from_crtc_id)
		return bufmgr->get_pipe_from_crtc_id(bufmgr, crtc_id);
	return -1;
}

#ifndef __ANDROID__
static size_t
drm_intel_probe_agp_aperture_size(int fd)
{
	struct pci_device *pci_dev;
	size_t size = 0;
	int ret;

	ret = pci_system_init();
	if (ret)
		goto err;

	/* XXX handle multiple adaptors? */
	pci_dev = pci_device_find_by_slot(0, 0, 2, 0);
	if (pci_dev == NULL)
		goto err;

	ret = pci_device_probe(pci_dev);
	if (ret)
		goto err;

	size = pci_dev->regions[2].size;
err:
	pci_system_cleanup ();
	return size;
}
#else
static size_t
drm_intel_probe_agp_aperture_size(int fd)
{
	/* Nothing seems to rely on this value on Android anyway... */
	fprintf(stderr, "%s: Mappable aperture size hardcoded to 64MiB\n", __func__);
	return 64 * 1024 * 1024;
}
#endif

drm_public int
drm_intel_get_aperture_sizes(int fd, size_t *mappable, size_t *total)
{

	struct drm_i915_gem_get_aperture aperture;
	int ret;

	ret = drmIoctl(fd, DRM_IOCTL_I915_GEM_GET_APERTURE, &aperture);
	if (ret)
		return ret;

	*mappable = 0;
	/* XXX add a query for the kernel value? */
	if (*mappable == 0)
		*mappable = drm_intel_probe_agp_aperture_size(fd);
	if (*mappable == 0)
		*mappable = 64 * 1024 * 1024; /* minimum possible value */
	*total = aperture.aper_size;
	return 0;
}
