/*
 * Copyright © 2008 Dave Airlie
 * Copyright © 2008 Jérôme Glisse
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS, AUTHORS
 * AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 */
/*
 * Authors:
 *      Dave Airlie
 *      Jérôme Glisse <glisse@freedesktop.org>
 */
#include <libdrm_macros.h>
#include <radeon_bo.h>
#include <radeon_bo_int.h>

drm_public void radeon_bo_debug(struct radeon_bo *bo, const char *op)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;

    fprintf(stderr, "%s %p 0x%08X 0x%08X 0x%08X\n",
            op, bo, bo->handle, boi->size, boi->cref);
}

drm_public struct radeon_bo *
radeon_bo_open(struct radeon_bo_manager *bom, uint32_t handle, uint32_t size,
	       uint32_t alignment, uint32_t domains, uint32_t flags)
{
    struct radeon_bo *bo;
    bo = bom->funcs->bo_open(bom, handle, size, alignment, domains, flags);
    return bo;
}

drm_public void radeon_bo_ref(struct radeon_bo *bo)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    boi->cref++;
    boi->bom->funcs->bo_ref(boi);
}

drm_public struct radeon_bo *radeon_bo_unref(struct radeon_bo *bo)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    if (bo == NULL)
        return NULL;

    boi->cref--;
    return boi->bom->funcs->bo_unref(boi);
}

drm_public int radeon_bo_map(struct radeon_bo *bo, int write)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    return boi->bom->funcs->bo_map(boi, write);
}

drm_public int radeon_bo_unmap(struct radeon_bo *bo)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    return boi->bom->funcs->bo_unmap(boi);
}

drm_public int radeon_bo_wait(struct radeon_bo *bo)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    if (!boi->bom->funcs->bo_wait)
        return 0;
    return boi->bom->funcs->bo_wait(boi);
}

drm_public int radeon_bo_is_busy(struct radeon_bo *bo, uint32_t *domain)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    return boi->bom->funcs->bo_is_busy(boi, domain);
}

drm_public int
radeon_bo_set_tiling(struct radeon_bo *bo,
                     uint32_t tiling_flags, uint32_t pitch)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    return boi->bom->funcs->bo_set_tiling(boi, tiling_flags, pitch);
}

drm_public int
radeon_bo_get_tiling(struct radeon_bo *bo,
                     uint32_t *tiling_flags, uint32_t *pitch)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    return boi->bom->funcs->bo_get_tiling(boi, tiling_flags, pitch);
}

drm_public int radeon_bo_is_static(struct radeon_bo *bo)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    if (boi->bom->funcs->bo_is_static)
        return boi->bom->funcs->bo_is_static(boi);
    return 0;
}

drm_public int
radeon_bo_is_referenced_by_cs(struct radeon_bo *bo, struct radeon_cs *cs)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    return boi->cref > 1;
}

drm_public uint32_t radeon_bo_get_handle(struct radeon_bo *bo)
{
    return bo->handle;
}

drm_public uint32_t radeon_bo_get_src_domain(struct radeon_bo *bo)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    uint32_t src_domain;

    src_domain = boi->space_accounted & 0xffff;
    if (!src_domain)
        src_domain = boi->space_accounted >> 16;

    return src_domain;
}
