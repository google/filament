/*
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "amdgpu.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"
#include "util_math.h"

drm_public int amdgpu_va_range_query(amdgpu_device_handle dev,
				     enum amdgpu_gpu_va_range type,
				     uint64_t *start, uint64_t *end)
{
	if (type != amdgpu_gpu_va_range_general)
		return -EINVAL;

	*start = dev->dev_info.virtual_address_offset;
	*end = dev->dev_info.virtual_address_max;
	return 0;
}

drm_private void amdgpu_vamgr_init(struct amdgpu_bo_va_mgr *mgr, uint64_t start,
				   uint64_t max, uint64_t alignment)
{
	struct amdgpu_bo_va_hole *n;

	mgr->va_max = max;
	mgr->va_alignment = alignment;

	list_inithead(&mgr->va_holes);
	pthread_mutex_init(&mgr->bo_va_mutex, NULL);
	pthread_mutex_lock(&mgr->bo_va_mutex);
	n = calloc(1, sizeof(struct amdgpu_bo_va_hole));
	n->size = mgr->va_max - start;
	n->offset = start;
	list_add(&n->list, &mgr->va_holes);
	pthread_mutex_unlock(&mgr->bo_va_mutex);
}

drm_private void amdgpu_vamgr_deinit(struct amdgpu_bo_va_mgr *mgr)
{
	struct amdgpu_bo_va_hole *hole, *tmp;
	LIST_FOR_EACH_ENTRY_SAFE(hole, tmp, &mgr->va_holes, list) {
		list_del(&hole->list);
		free(hole);
	}
	pthread_mutex_destroy(&mgr->bo_va_mutex);
}

static drm_private int
amdgpu_vamgr_subtract_hole(struct amdgpu_bo_va_hole *hole, uint64_t start_va,
			   uint64_t end_va)
{
	if (start_va > hole->offset && end_va - hole->offset < hole->size) {
		struct amdgpu_bo_va_hole *n = calloc(1, sizeof(struct amdgpu_bo_va_hole));
		if (!n)
			return -ENOMEM;

		n->size = start_va - hole->offset;
		n->offset = hole->offset;
		list_add(&n->list, &hole->list);

		hole->size -= (end_va - hole->offset);
		hole->offset = end_va;
	} else if (start_va > hole->offset) {
		hole->size = start_va - hole->offset;
	} else if (end_va - hole->offset < hole->size) {
		hole->size -= (end_va - hole->offset);
		hole->offset = end_va;
	} else {
		list_del(&hole->list);
		free(hole);
	}

	return 0;
}

static drm_private int
amdgpu_vamgr_find_va(struct amdgpu_bo_va_mgr *mgr, uint64_t size,
		     uint64_t alignment, uint64_t base_required,
		     bool search_from_top, uint64_t *va_out)
{
	struct amdgpu_bo_va_hole *hole, *n;
	uint64_t offset = 0;
	int ret;


	alignment = MAX2(alignment, mgr->va_alignment);
	size = ALIGN(size, mgr->va_alignment);

	if (base_required % alignment)
		return -EINVAL;

	pthread_mutex_lock(&mgr->bo_va_mutex);
	if (!search_from_top) {
		LIST_FOR_EACH_ENTRY_SAFE_REV(hole, n, &mgr->va_holes, list) {
			if (base_required) {
				if (hole->offset > base_required ||
				   (hole->offset + hole->size) < (base_required + size))
					continue;
				offset = base_required;
			} else {
				uint64_t waste = hole->offset % alignment;
				waste = waste ? alignment - waste : 0;
				offset = hole->offset + waste;
				if (offset >= (hole->offset + hole->size) ||
				    size > (hole->offset + hole->size) - offset) {
					continue;
				}
			}
			ret = amdgpu_vamgr_subtract_hole(hole, offset, offset + size);
			pthread_mutex_unlock(&mgr->bo_va_mutex);
			*va_out = offset;
			return ret;
		}
	} else {
		LIST_FOR_EACH_ENTRY_SAFE(hole, n, &mgr->va_holes, list) {
			if (base_required) {
				if (hole->offset > base_required ||
				   (hole->offset + hole->size) < (base_required + size))
					continue;
				offset = base_required;
			} else {
				if (size > hole->size)
					continue;

				offset = hole->offset + hole->size - size;
				offset -= offset % alignment;
				if (offset < hole->offset) {
					continue;
				}
			}

			ret = amdgpu_vamgr_subtract_hole(hole, offset, offset + size);
			pthread_mutex_unlock(&mgr->bo_va_mutex);
			*va_out = offset;
			return ret;
		}
	}

	pthread_mutex_unlock(&mgr->bo_va_mutex);
	return -ENOMEM;
}

static drm_private void
amdgpu_vamgr_free_va(struct amdgpu_bo_va_mgr *mgr, uint64_t va, uint64_t size)
{
	struct amdgpu_bo_va_hole *hole, *next;

	if (va == AMDGPU_INVALID_VA_ADDRESS)
		return;

	size = ALIGN(size, mgr->va_alignment);

	pthread_mutex_lock(&mgr->bo_va_mutex);
	hole = container_of(&mgr->va_holes, hole, list);
	LIST_FOR_EACH_ENTRY(next, &mgr->va_holes, list) {
		if (next->offset < va)
			break;
		hole = next;
	}

	if (&hole->list != &mgr->va_holes) {
		/* Grow upper hole if it's adjacent */
		if (hole->offset == (va + size)) {
			hole->offset = va;
			hole->size += size;
			/* Merge lower hole if it's adjacent */
			if (next != hole &&
			    &next->list != &mgr->va_holes &&
			    (next->offset + next->size) == va) {
				next->size += hole->size;
				list_del(&hole->list);
				free(hole);
			}
			goto out;
		}
	}

	/* Grow lower hole if it's adjacent */
	if (next != hole && &next->list != &mgr->va_holes &&
	    (next->offset + next->size) == va) {
		next->size += size;
		goto out;
	}

	/* FIXME on allocation failure we just lose virtual address space
	 * maybe print a warning
	 */
	next = calloc(1, sizeof(struct amdgpu_bo_va_hole));
	if (next) {
		next->size = size;
		next->offset = va;
		list_add(&next->list, &hole->list);
	}

out:
	pthread_mutex_unlock(&mgr->bo_va_mutex);
}

drm_public int amdgpu_va_range_alloc(amdgpu_device_handle dev,
				     enum amdgpu_gpu_va_range va_range_type,
				     uint64_t size,
				     uint64_t va_base_alignment,
				     uint64_t va_base_required,
				     uint64_t *va_base_allocated,
				     amdgpu_va_handle *va_range_handle,
				     uint64_t flags)
{
	return amdgpu_va_range_alloc2(&dev->va_mgr, va_range_type, size,
				      va_base_alignment, va_base_required,
				      va_base_allocated, va_range_handle,
				      flags);
}

drm_public int amdgpu_va_range_alloc2(amdgpu_va_manager_handle va_mgr,
				      enum amdgpu_gpu_va_range va_range_type,
				      uint64_t size,
				      uint64_t va_base_alignment,
				      uint64_t va_base_required,
				      uint64_t *va_base_allocated,
				      amdgpu_va_handle *va_range_handle,
				      uint64_t flags)
{
	struct amdgpu_bo_va_mgr *vamgr;
	bool search_from_top = !!(flags & AMDGPU_VA_RANGE_REPLAYABLE);
	int ret;

	/* Clear the flag when the high VA manager is not initialized */
	if (flags & AMDGPU_VA_RANGE_HIGH && !va_mgr->vamgr_high_32.va_max)
		flags &= ~AMDGPU_VA_RANGE_HIGH;

	if (flags & AMDGPU_VA_RANGE_HIGH) {
		if (flags & AMDGPU_VA_RANGE_32_BIT)
			vamgr = &va_mgr->vamgr_high_32;
		else
			vamgr = &va_mgr->vamgr_high;
	} else {
		if (flags & AMDGPU_VA_RANGE_32_BIT)
			vamgr = &va_mgr->vamgr_32;
		else
			vamgr = &va_mgr->vamgr_low;
	}

	va_base_alignment = MAX2(va_base_alignment, vamgr->va_alignment);
	size = ALIGN(size, vamgr->va_alignment);

	ret = amdgpu_vamgr_find_va(vamgr, size,
				   va_base_alignment, va_base_required,
				   search_from_top, va_base_allocated);

	if (!(flags & AMDGPU_VA_RANGE_32_BIT) && ret) {
		/* fallback to 32bit address */
		if (flags & AMDGPU_VA_RANGE_HIGH)
			vamgr = &va_mgr->vamgr_high_32;
		else
			vamgr = &va_mgr->vamgr_32;
		ret = amdgpu_vamgr_find_va(vamgr, size,
					   va_base_alignment, va_base_required,
					   search_from_top, va_base_allocated);
	}

	if (!ret) {
		struct amdgpu_va* va;
		va = calloc(1, sizeof(struct amdgpu_va));
		if(!va){
			amdgpu_vamgr_free_va(vamgr, *va_base_allocated, size);
			return -ENOMEM;
		}
		va->address = *va_base_allocated;
		va->size = size;
		va->range = va_range_type;
		va->vamgr = vamgr;
		*va_range_handle = va;
	}

	return ret;
}

drm_public int amdgpu_va_range_free(amdgpu_va_handle va_range_handle)
{
	if(!va_range_handle || !va_range_handle->address)
		return 0;

	amdgpu_vamgr_free_va(va_range_handle->vamgr,
			va_range_handle->address,
			va_range_handle->size);
	free(va_range_handle);
	return 0;
}

drm_public uint64_t amdgpu_va_get_start_addr(amdgpu_va_handle va_handle)
{
   return va_handle->address;
}

drm_public amdgpu_va_manager_handle amdgpu_va_manager_alloc(void)
{
	amdgpu_va_manager_handle r = calloc(1, sizeof(struct amdgpu_va_manager));
	return r;
}

drm_public void amdgpu_va_manager_init(struct amdgpu_va_manager *va_mgr,
					uint64_t low_va_offset, uint64_t low_va_max,
					uint64_t high_va_offset, uint64_t high_va_max,
					uint32_t virtual_address_alignment)
{
	uint64_t start, max;

	start = low_va_offset;
	max = MIN2(low_va_max, 0x100000000ULL);
	amdgpu_vamgr_init(&va_mgr->vamgr_32, start, max,
			  virtual_address_alignment);

	start = max;
	max = MAX2(low_va_max, 0x100000000ULL);
	amdgpu_vamgr_init(&va_mgr->vamgr_low, start, max,
			  virtual_address_alignment);

	start = high_va_offset;
	max = MIN2(high_va_max, (start & ~0xffffffffULL) + 0x100000000ULL);
	amdgpu_vamgr_init(&va_mgr->vamgr_high_32, start, max,
			  virtual_address_alignment);

	start = max;
	max = MAX2(high_va_max, (start & ~0xffffffffULL) + 0x100000000ULL);
	amdgpu_vamgr_init(&va_mgr->vamgr_high, start, max,
			  virtual_address_alignment);
}

drm_public void amdgpu_va_manager_deinit(struct amdgpu_va_manager *va_mgr)
{
	amdgpu_vamgr_deinit(&va_mgr->vamgr_32);
	amdgpu_vamgr_deinit(&va_mgr->vamgr_low);
	amdgpu_vamgr_deinit(&va_mgr->vamgr_high_32);
	amdgpu_vamgr_deinit(&va_mgr->vamgr_high);
}
