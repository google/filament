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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "libdrm_macros.h"
#include "xf86drm.h"
#include "xf86atomic.h"
#include "drm.h"
#include "radeon_drm.h"
#include "radeon_bo.h"
#include "radeon_bo_int.h"
#include "radeon_bo_gem.h"
#include <fcntl.h>
struct radeon_bo_gem {
    struct radeon_bo_int    base;
    uint32_t                name;
    int                     map_count;
    atomic_t                reloc_in_cs;
    void                    *priv_ptr;
};

struct bo_manager_gem {
    struct radeon_bo_manager    base;
};

static int bo_wait(struct radeon_bo_int *boi);
    
static struct radeon_bo *bo_open(struct radeon_bo_manager *bom,
                                 uint32_t handle,
                                 uint32_t size,
                                 uint32_t alignment,
                                 uint32_t domains,
                                 uint32_t flags)
{
    struct radeon_bo_gem *bo;
    int r;

    bo = (struct radeon_bo_gem*)calloc(1, sizeof(struct radeon_bo_gem));
    if (bo == NULL) {
        return NULL;
    }

    bo->base.bom = bom;
    bo->base.handle = 0;
    bo->base.size = size;
    bo->base.alignment = alignment;
    bo->base.domains = domains;
    bo->base.flags = flags;
    bo->base.ptr = NULL;
    atomic_set(&bo->reloc_in_cs, 0);
    bo->map_count = 0;
    if (handle) {
        struct drm_gem_open open_arg;

        memset(&open_arg, 0, sizeof(open_arg));
        open_arg.name = handle;
        r = drmIoctl(bom->fd, DRM_IOCTL_GEM_OPEN, &open_arg);
        if (r != 0) {
            free(bo);
            return NULL;
        }
        bo->base.handle = open_arg.handle;
        bo->base.size = open_arg.size;
        bo->name = handle;
    } else {
        struct drm_radeon_gem_create args;

        args.size = size;
        args.alignment = alignment;
        args.initial_domain = bo->base.domains;
        args.flags = flags;
        args.handle = 0;
        r = drmCommandWriteRead(bom->fd, DRM_RADEON_GEM_CREATE,
                                &args, sizeof(args));
        bo->base.handle = args.handle;
        if (r) {
            fprintf(stderr, "Failed to allocate :\n");
            fprintf(stderr, "   size      : %d bytes\n", size);
            fprintf(stderr, "   alignment : %d bytes\n", alignment);
            fprintf(stderr, "   domains   : %d\n", bo->base.domains);
            free(bo);
            return NULL;
        }
    }
    radeon_bo_ref((struct radeon_bo*)bo);
    return (struct radeon_bo*)bo;
}

static void bo_ref(struct radeon_bo_int *boi)
{
}

static struct radeon_bo *bo_unref(struct radeon_bo_int *boi)
{
    struct radeon_bo_gem *bo_gem = (struct radeon_bo_gem*)boi;

    if (boi->cref) {
        return (struct radeon_bo *)boi;
    }
    if (bo_gem->priv_ptr) {
        drm_munmap(bo_gem->priv_ptr, boi->size);
    }

    /* close object */
    drmCloseBufferHandle(boi->bom->fd, boi->handle);
    memset(bo_gem, 0, sizeof(struct radeon_bo_gem));
    free(bo_gem);
    return NULL;
}

static int bo_map(struct radeon_bo_int *boi, int write)
{
    struct radeon_bo_gem *bo_gem = (struct radeon_bo_gem*)boi;
    struct drm_radeon_gem_mmap args;
    int r;
    void *ptr;

    if (bo_gem->map_count++ != 0) {
        return 0;
    }
    if (bo_gem->priv_ptr) {
        goto wait;
    }

    boi->ptr = NULL;

    /* Zero out args to make valgrind happy */
    memset(&args, 0, sizeof(args));
    args.handle = boi->handle;
    args.offset = 0;
    args.size = (uint64_t)boi->size;
    r = drmCommandWriteRead(boi->bom->fd,
                            DRM_RADEON_GEM_MMAP,
                            &args,
                            sizeof(args));
    if (r) {
        fprintf(stderr, "error mapping %p 0x%08X (error = %d)\n",
                boi, boi->handle, r);
        return r;
    }
    ptr = drm_mmap(0, args.size, PROT_READ|PROT_WRITE, MAP_SHARED, boi->bom->fd, args.addr_ptr);
    if (ptr == MAP_FAILED)
        return -errno;
    bo_gem->priv_ptr = ptr;
wait:
    boi->ptr = bo_gem->priv_ptr;
    r = bo_wait(boi);
    if (r)
        return r;
    return 0;
}

static int bo_unmap(struct radeon_bo_int *boi)
{
    struct radeon_bo_gem *bo_gem = (struct radeon_bo_gem*)boi;

    if (--bo_gem->map_count > 0) {
        return 0;
    }
    //drm_munmap(bo->ptr, bo->size);
    boi->ptr = NULL;
    return 0;
}

static int bo_wait(struct radeon_bo_int *boi)
{
    struct drm_radeon_gem_wait_idle args;
    int ret;

    /* Zero out args to make valgrind happy */
    memset(&args, 0, sizeof(args));
    args.handle = boi->handle;
    do {
        ret = drmCommandWrite(boi->bom->fd, DRM_RADEON_GEM_WAIT_IDLE,
			      &args, sizeof(args));
    } while (ret == -EBUSY);
    return ret;
}

static int bo_is_busy(struct radeon_bo_int *boi, uint32_t *domain)
{
    struct drm_radeon_gem_busy args;
    int ret;

    args.handle = boi->handle;
    args.domain = 0;

    ret = drmCommandWriteRead(boi->bom->fd, DRM_RADEON_GEM_BUSY,
                              &args, sizeof(args));

    *domain = args.domain;
    return ret;
}

static int bo_set_tiling(struct radeon_bo_int *boi, uint32_t tiling_flags,
                         uint32_t pitch)
{
    struct drm_radeon_gem_set_tiling args;
    int r;

    args.handle = boi->handle;
    args.tiling_flags = tiling_flags;
    args.pitch = pitch;

    r = drmCommandWriteRead(boi->bom->fd,
                            DRM_RADEON_GEM_SET_TILING,
                            &args,
                            sizeof(args));
    return r;
}

static int bo_get_tiling(struct radeon_bo_int *boi, uint32_t *tiling_flags,
                         uint32_t *pitch)
{
    struct drm_radeon_gem_set_tiling args = {};
    int r;

    args.handle = boi->handle;

    r = drmCommandWriteRead(boi->bom->fd,
                            DRM_RADEON_GEM_GET_TILING,
                            &args,
                            sizeof(args));

    if (r)
        return r;

    *tiling_flags = args.tiling_flags;
    *pitch = args.pitch;
    return r;
}

static const struct radeon_bo_funcs bo_gem_funcs = {
    .bo_open = bo_open,
    .bo_ref = bo_ref,
    .bo_unref = bo_unref,
    .bo_map = bo_map,
    .bo_unmap = bo_unmap,
    .bo_wait = bo_wait,
    .bo_is_static = NULL,
    .bo_set_tiling = bo_set_tiling,
    .bo_get_tiling = bo_get_tiling,
    .bo_is_busy = bo_is_busy,
    .bo_is_referenced_by_cs = NULL,
};

drm_public struct radeon_bo_manager *radeon_bo_manager_gem_ctor(int fd)
{
    struct bo_manager_gem *bomg;

    bomg = (struct bo_manager_gem*)calloc(1, sizeof(struct bo_manager_gem));
    if (bomg == NULL) {
        return NULL;
    }
    bomg->base.funcs = &bo_gem_funcs;
    bomg->base.fd = fd;
    return (struct radeon_bo_manager*)bomg;
}

drm_public void radeon_bo_manager_gem_dtor(struct radeon_bo_manager *bom)
{
    struct bo_manager_gem *bomg = (struct bo_manager_gem*)bom;

    if (bom == NULL) {
        return;
    }
    free(bomg);
}

drm_public uint32_t
radeon_gem_name_bo(struct radeon_bo *bo)
{
    struct radeon_bo_gem *bo_gem = (struct radeon_bo_gem*)bo;
    return bo_gem->name;
}

drm_public void *
radeon_gem_get_reloc_in_cs(struct radeon_bo *bo)
{
    struct radeon_bo_gem *bo_gem = (struct radeon_bo_gem*)bo;
    return &bo_gem->reloc_in_cs;
}

drm_public int
radeon_gem_get_kernel_name(struct radeon_bo *bo, uint32_t *name)
{
    struct radeon_bo_gem *bo_gem = (struct radeon_bo_gem*)bo;
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    struct drm_gem_flink flink;
    int r;

    if (bo_gem->name) {
        *name = bo_gem->name;
        return 0;
    }
    flink.handle = bo->handle;
    r = drmIoctl(boi->bom->fd, DRM_IOCTL_GEM_FLINK, &flink);
    if (r) {
        return r;
    }
    bo_gem->name = flink.name;
    *name = flink.name;
    return 0;
}

drm_public int
radeon_gem_set_domain(struct radeon_bo *bo, uint32_t read_domains, uint32_t write_domain)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    struct drm_radeon_gem_set_domain args;
    int r;

    args.handle = bo->handle;
    args.read_domains = read_domains;
    args.write_domain = write_domain;

    r = drmCommandWriteRead(boi->bom->fd,
                            DRM_RADEON_GEM_SET_DOMAIN,
                            &args,
                            sizeof(args));
    return r;
}

drm_public int radeon_gem_prime_share_bo(struct radeon_bo *bo, int *handle)
{
    struct radeon_bo_gem *bo_gem = (struct radeon_bo_gem*)bo;
    int ret;

    ret = drmPrimeHandleToFD(bo_gem->base.bom->fd, bo->handle, DRM_CLOEXEC, handle);
    return ret;
}

drm_public struct radeon_bo *
radeon_gem_bo_open_prime(struct radeon_bo_manager *bom, int fd_handle, uint32_t size)
{
    struct radeon_bo_gem *bo;
    int r;
    uint32_t handle;

    bo = (struct radeon_bo_gem*)calloc(1, sizeof(struct radeon_bo_gem));
    if (bo == NULL) {
        return NULL;
    }

    bo->base.bom = bom;
    bo->base.handle = 0;
    bo->base.size = size;
    bo->base.alignment = 0;
    bo->base.domains = RADEON_GEM_DOMAIN_GTT;
    bo->base.flags = 0;
    bo->base.ptr = NULL;
    atomic_set(&bo->reloc_in_cs, 0);
    bo->map_count = 0;

    r = drmPrimeFDToHandle(bom->fd, fd_handle, &handle);
    if (r != 0) {
	free(bo);
	return NULL;
    }

    bo->base.handle = handle;
    bo->name = handle;

    radeon_bo_ref((struct radeon_bo *)bo);
    return (struct radeon_bo *)bo;

}
