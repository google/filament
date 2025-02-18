/*
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
 *      Aapo Tahkola <aet@rasterburn.org>
 *      Nicolai Haehnle <prefect_@gmx.net>
 *      Jérôme Glisse <glisse@freedesktop.org>
 */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "radeon_cs.h"
#include "radeon_cs_int.h"
#include "radeon_bo_int.h"
#include "radeon_cs_gem.h"
#include "radeon_bo_gem.h"
#include "drm.h"
#include "libdrm_macros.h"
#include "xf86drm.h"
#include "xf86atomic.h"
#include "radeon_drm.h"

/* Add LIBDRM_RADEON_BOF_FILES to libdrm_radeon_la_SOURCES when building with BOF_DUMP */
#define CS_BOF_DUMP 0
#if CS_BOF_DUMP
#include "bof.h"
#endif

struct radeon_cs_manager_gem {
    struct radeon_cs_manager    base;
    uint32_t                    device_id;
    unsigned                    nbof;
};

#pragma pack(1)
struct cs_reloc_gem {
    uint32_t    handle;
    uint32_t    read_domain;
    uint32_t    write_domain;
    uint32_t    flags;
};

#pragma pack()
#define RELOC_SIZE (sizeof(struct cs_reloc_gem) / sizeof(uint32_t))

struct cs_gem {
    struct radeon_cs_int        base;
    struct drm_radeon_cs        cs;
    struct drm_radeon_cs_chunk  chunks[2];
    unsigned                    nrelocs;
    uint32_t                    *relocs;
    struct radeon_bo_int        **relocs_bo;
};

static pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t cs_id_source = 0;

/**
 * result is undefined if called with ~0
 */
static uint32_t get_first_zero(const uint32_t n)
{
    /* __builtin_ctz returns number of trailing zeros. */
    return 1 << __builtin_ctz(~n);
}

/**
 * Returns a free id for cs.
 * If there is no free id we return zero
 **/
static uint32_t generate_id(void)
{
    uint32_t r = 0;
    pthread_mutex_lock( &id_mutex );
    /* check for free ids */
    if (cs_id_source != ~r) {
        /* find first zero bit */
        r = get_first_zero(cs_id_source);

        /* set id as reserved */
        cs_id_source |= r;
    }
    pthread_mutex_unlock( &id_mutex );
    return r;
}

/**
 * Free the id for later reuse
 **/
static void free_id(uint32_t id)
{
    pthread_mutex_lock( &id_mutex );

    cs_id_source &= ~id;

    pthread_mutex_unlock( &id_mutex );
}

static struct radeon_cs_int *cs_gem_create(struct radeon_cs_manager *csm,
                                       uint32_t ndw)
{
    struct cs_gem *csg;

    /* max cmd buffer size is 64Kb */
    if (ndw > (64 * 1024 / 4)) {
        return NULL;
    }
    csg = (struct cs_gem*)calloc(1, sizeof(struct cs_gem));
    if (csg == NULL) {
        return NULL;
    }
    csg->base.csm = csm;
    csg->base.ndw = 64 * 1024 / 4;
    csg->base.packets = (uint32_t*)calloc(1, 64 * 1024);
    if (csg->base.packets == NULL) {
        free(csg);
        return NULL;
    }
    csg->base.relocs_total_size = 0;
    csg->base.crelocs = 0;
    csg->base.id = generate_id();
    csg->nrelocs = 4096 / (4 * 4) ;
    csg->relocs_bo = (struct radeon_bo_int**)calloc(1,
                                                csg->nrelocs*sizeof(void*));
    if (csg->relocs_bo == NULL) {
        free(csg->base.packets);
        free(csg);
        return NULL;
    }
    csg->base.relocs = csg->relocs = (uint32_t*)calloc(1, 4096);
    if (csg->relocs == NULL) {
        free(csg->relocs_bo);
        free(csg->base.packets);
        free(csg);
        return NULL;
    }
    csg->chunks[0].chunk_id = RADEON_CHUNK_ID_IB;
    csg->chunks[0].length_dw = 0;
    csg->chunks[0].chunk_data = (uint64_t)(uintptr_t)csg->base.packets;
    csg->chunks[1].chunk_id = RADEON_CHUNK_ID_RELOCS;
    csg->chunks[1].length_dw = 0;
    csg->chunks[1].chunk_data = (uint64_t)(uintptr_t)csg->relocs;
    return (struct radeon_cs_int*)csg;
}

static int cs_gem_write_reloc(struct radeon_cs_int *cs,
                              struct radeon_bo *bo,
                              uint32_t read_domain,
                              uint32_t write_domain,
                              uint32_t flags)
{
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    struct cs_gem *csg = (struct cs_gem*)cs;
    struct cs_reloc_gem *reloc;
    uint32_t idx;
    unsigned i;

    assert(boi->space_accounted);

    /* check domains */
    if ((read_domain && write_domain) || (!read_domain && !write_domain)) {
        /* in one CS a bo can only be in read or write domain but not
         * in read & write domain at the same time
         */
        return -EINVAL;
    }
    if (read_domain == RADEON_GEM_DOMAIN_CPU) {
        return -EINVAL;
    }
    if (write_domain == RADEON_GEM_DOMAIN_CPU) {
        return -EINVAL;
    }
    /* use bit field hash function to determine
       if this bo is for sure not in this cs.*/
    if ((atomic_read((atomic_t *)radeon_gem_get_reloc_in_cs(bo)) & cs->id)) {
        /* check if bo is already referenced.
         * Scanning from end to begin reduces cycles with mesa because
         * it often relocates same shared dma bo again. */
        for(i = cs->crelocs; i != 0;) {
            --i;
            idx = i * RELOC_SIZE;
            reloc = (struct cs_reloc_gem*)&csg->relocs[idx];
            if (reloc->handle == bo->handle) {
                /* Check domains must be in read or write. As we check already
                 * checked that in argument one of the read or write domain was
                 * set we only need to check that if previous reloc as the read
                 * domain set then the read_domain should also be set for this
                 * new relocation.
                 */
                /* the DDX expects to read and write from same pixmap */
                if (write_domain && (reloc->read_domain & write_domain)) {
                    reloc->read_domain = 0;
                    reloc->write_domain = write_domain;
                } else if (read_domain & reloc->write_domain) {
                    reloc->read_domain = 0;
                } else {
                    if (write_domain != reloc->write_domain)
                        return -EINVAL;
                    if (read_domain != reloc->read_domain)
                        return -EINVAL;
                }

                reloc->read_domain |= read_domain;
                reloc->write_domain |= write_domain;
                /* update flags */
                reloc->flags |= (flags & reloc->flags);
                /* write relocation packet */
                radeon_cs_write_dword((struct radeon_cs *)cs, 0xc0001000);
                radeon_cs_write_dword((struct radeon_cs *)cs, idx);
                return 0;
            }
        }
    }
    /* new relocation */
    if (csg->base.crelocs >= csg->nrelocs) {
        /* allocate more memory (TODO: should use a slab allocator maybe) */
        uint32_t *tmp, size;
        size = ((csg->nrelocs + 1) * sizeof(struct radeon_bo*));
        tmp = (uint32_t*)realloc(csg->relocs_bo, size);
        if (tmp == NULL) {
            return -ENOMEM;
        }
        csg->relocs_bo = (struct radeon_bo_int **)tmp;
        size = ((csg->nrelocs + 1) * RELOC_SIZE * 4);
        tmp = (uint32_t*)realloc(csg->relocs, size);
        if (tmp == NULL) {
            return -ENOMEM;
        }
        cs->relocs = csg->relocs = tmp;
        csg->nrelocs += 1;
        csg->chunks[1].chunk_data = (uint64_t)(uintptr_t)csg->relocs;
    }
    csg->relocs_bo[csg->base.crelocs] = boi;
    idx = (csg->base.crelocs++) * RELOC_SIZE;
    reloc = (struct cs_reloc_gem*)&csg->relocs[idx];
    reloc->handle = bo->handle;
    reloc->read_domain = read_domain;
    reloc->write_domain = write_domain;
    reloc->flags = flags;
    csg->chunks[1].length_dw += RELOC_SIZE;
    radeon_bo_ref(bo);
    /* bo might be referenced from another context so have to use atomic operations */
    atomic_add((atomic_t *)radeon_gem_get_reloc_in_cs(bo), cs->id);
    cs->relocs_total_size += boi->size;
    radeon_cs_write_dword((struct radeon_cs *)cs, 0xc0001000);
    radeon_cs_write_dword((struct radeon_cs *)cs, idx);
    return 0;
}

static int cs_gem_begin(struct radeon_cs_int *cs,
                        uint32_t ndw,
                        const char *file,
                        const char *func,
                        int line)
{

    if (cs->section_ndw) {
        fprintf(stderr, "CS already in a section(%s,%s,%d)\n",
                cs->section_file, cs->section_func, cs->section_line);
        fprintf(stderr, "CS can't start section(%s,%s,%d)\n",
                file, func, line);
        return -EPIPE;
    }
    cs->section_ndw = ndw;
    cs->section_cdw = 0;
    cs->section_file = file;
    cs->section_func = func;
    cs->section_line = line;

    if (cs->cdw + ndw > cs->ndw) {
        uint32_t tmp, *ptr;

        /* round up the required size to a multiple of 1024 */
        tmp = (cs->cdw + ndw + 0x3FF) & (~0x3FF);
        ptr = (uint32_t*)realloc(cs->packets, 4 * tmp);
        if (ptr == NULL) {
            return -ENOMEM;
        }
        cs->packets = ptr;
        cs->ndw = tmp;
    }
    return 0;
}

static int cs_gem_end(struct radeon_cs_int *cs,
                      const char *file,
                      const char *func,
                      int line)

{
    if (!cs->section_ndw) {
        fprintf(stderr, "CS no section to end at (%s,%s,%d)\n",
                file, func, line);
        return -EPIPE;
    }
    if (cs->section_ndw != cs->section_cdw) {
        fprintf(stderr, "CS section size mismatch start at (%s,%s,%d) %d vs %d\n",
                cs->section_file, cs->section_func, cs->section_line, cs->section_ndw, cs->section_cdw);
        fprintf(stderr, "CS section end at (%s,%s,%d)\n",
                file, func, line);

        /* We must reset the section even when there is error. */
        cs->section_ndw = 0;
        return -EPIPE;
    }
    cs->section_ndw = 0;
    return 0;
}

#if CS_BOF_DUMP
static void cs_gem_dump_bof(struct radeon_cs_int *cs)
{
    struct cs_gem *csg = (struct cs_gem*)cs;
    struct radeon_cs_manager_gem *csm;
    bof_t *bcs, *blob, *array, *bo, *size, *handle, *device_id, *root;
    char tmp[256];
    unsigned i;

    csm = (struct radeon_cs_manager_gem *)cs->csm;
    root = device_id = bcs = blob = array = bo = size = handle = NULL;
    root = bof_object();
    if (root == NULL)
        goto out_err;
    device_id = bof_int32(csm->device_id);
    if (device_id == NULL)
        return;
    if (bof_object_set(root, "device_id", device_id))
        goto out_err;
    bof_decref(device_id);
    device_id = NULL;
    /* dump relocs */
    blob = bof_blob(csg->nrelocs * 16, csg->relocs);
    if (blob == NULL)
        goto out_err;
    if (bof_object_set(root, "reloc", blob))
        goto out_err;
    bof_decref(blob);
    blob = NULL;
    /* dump cs */
    blob = bof_blob(cs->cdw * 4, cs->packets);
    if (blob == NULL)
        goto out_err;
    if (bof_object_set(root, "pm4", blob))
        goto out_err;
    bof_decref(blob);
    blob = NULL;
    /* dump bo */
    array = bof_array();
    if (array == NULL)
        goto out_err;
    for (i = 0; i < csg->base.crelocs; i++) {
        bo = bof_object();
        if (bo == NULL)
            goto out_err;
        size = bof_int32(csg->relocs_bo[i]->size);
        if (size == NULL)
            goto out_err;
        if (bof_object_set(bo, "size", size))
            goto out_err;
        bof_decref(size);
        size = NULL;
        handle = bof_int32(csg->relocs_bo[i]->handle);
        if (handle == NULL)
            goto out_err;
        if (bof_object_set(bo, "handle", handle))
            goto out_err;
        bof_decref(handle);
        handle = NULL;
        radeon_bo_map((struct radeon_bo*)csg->relocs_bo[i], 0);
        blob = bof_blob(csg->relocs_bo[i]->size, csg->relocs_bo[i]->ptr);
        radeon_bo_unmap((struct radeon_bo*)csg->relocs_bo[i]);
        if (blob == NULL)
            goto out_err;
        if (bof_object_set(bo, "data", blob))
            goto out_err;
        bof_decref(blob);
        blob = NULL;
        if (bof_array_append(array, bo))
            goto out_err;
        bof_decref(bo);
        bo = NULL;
    }
    if (bof_object_set(root, "bo", array))
        goto out_err;
    sprintf(tmp, "d-0x%04X-%08d.bof", csm->device_id, csm->nbof++);
    bof_dump_file(root, tmp);
out_err:
    bof_decref(blob);
    bof_decref(array);
    bof_decref(bo);
    bof_decref(size);
    bof_decref(handle);
    bof_decref(device_id);
    bof_decref(root);
}
#endif

static int cs_gem_emit(struct radeon_cs_int *cs)
{
    struct cs_gem *csg = (struct cs_gem*)cs;
    uint64_t chunk_array[2];
    unsigned i;
    int r;

    while (cs->cdw & 7)
	radeon_cs_write_dword((struct radeon_cs *)cs, 0x80000000);

#if CS_BOF_DUMP
    cs_gem_dump_bof(cs);
#endif
    csg->chunks[0].length_dw = cs->cdw;

    chunk_array[0] = (uint64_t)(uintptr_t)&csg->chunks[0];
    chunk_array[1] = (uint64_t)(uintptr_t)&csg->chunks[1];

    csg->cs.num_chunks = 2;
    csg->cs.chunks = (uint64_t)(uintptr_t)chunk_array;

    r = drmCommandWriteRead(cs->csm->fd, DRM_RADEON_CS,
                            &csg->cs, sizeof(struct drm_radeon_cs));
    for (i = 0; i < csg->base.crelocs; i++) {
        csg->relocs_bo[i]->space_accounted = 0;
        /* bo might be referenced from another context so have to use atomic operations */
        atomic_dec((atomic_t *)radeon_gem_get_reloc_in_cs((struct radeon_bo*)csg->relocs_bo[i]), cs->id);
        radeon_bo_unref((struct radeon_bo *)csg->relocs_bo[i]);
        csg->relocs_bo[i] = NULL;
    }

    cs->csm->read_used = 0;
    cs->csm->vram_write_used = 0;
    cs->csm->gart_write_used = 0;
    return r;
}

static int cs_gem_destroy(struct radeon_cs_int *cs)
{
    struct cs_gem *csg = (struct cs_gem*)cs;

    free_id(cs->id);
    free(csg->relocs_bo);
    free(cs->relocs);
    free(cs->packets);
    free(cs);
    return 0;
}

static int cs_gem_erase(struct radeon_cs_int *cs)
{
    struct cs_gem *csg = (struct cs_gem*)cs;
    unsigned i;

    if (csg->relocs_bo) {
        for (i = 0; i < csg->base.crelocs; i++) {
            if (csg->relocs_bo[i]) {
                /* bo might be referenced from another context so have to use atomic operations */
                atomic_dec((atomic_t *)radeon_gem_get_reloc_in_cs((struct radeon_bo*)csg->relocs_bo[i]), cs->id);
                radeon_bo_unref((struct radeon_bo *)csg->relocs_bo[i]);
                csg->relocs_bo[i] = NULL;
            }
        }
    }
    cs->relocs_total_size = 0;
    cs->cdw = 0;
    cs->section_ndw = 0;
    cs->crelocs = 0;
    csg->chunks[0].length_dw = 0;
    csg->chunks[1].length_dw = 0;
    return 0;
}

static int cs_gem_need_flush(struct radeon_cs_int *cs)
{
    return 0; //(cs->relocs_total_size > (32*1024*1024));
}

static void cs_gem_print(struct radeon_cs_int *cs, FILE *file)
{
    struct radeon_cs_manager_gem *csm;
    unsigned int i;

    csm = (struct radeon_cs_manager_gem *)cs->csm;
    fprintf(file, "VENDORID:DEVICEID 0x%04X:0x%04X\n", 0x1002, csm->device_id);
    for (i = 0; i < cs->cdw; i++) {
        fprintf(file, "0x%08X\n", cs->packets[i]);
    }
}

static const struct radeon_cs_funcs radeon_cs_gem_funcs = {
    .cs_create = cs_gem_create,
    .cs_write_reloc = cs_gem_write_reloc,
    .cs_begin = cs_gem_begin,
    .cs_end = cs_gem_end,
    .cs_emit = cs_gem_emit,
    .cs_destroy = cs_gem_destroy,
    .cs_erase = cs_gem_erase,
    .cs_need_flush = cs_gem_need_flush,
    .cs_print = cs_gem_print,
};

static int radeon_get_device_id(int fd, uint32_t *device_id)
{
    struct drm_radeon_info info = {};
    int r;

    *device_id = 0;
    info.request = RADEON_INFO_DEVICE_ID;
    info.value = (uintptr_t)device_id;
    r = drmCommandWriteRead(fd, DRM_RADEON_INFO, &info,
                            sizeof(struct drm_radeon_info));
    return r;
}

drm_public struct radeon_cs_manager *radeon_cs_manager_gem_ctor(int fd)
{
    struct radeon_cs_manager_gem *csm;

    csm = calloc(1, sizeof(struct radeon_cs_manager_gem));
    if (csm == NULL) {
        return NULL;
    }
    csm->base.funcs = &radeon_cs_gem_funcs;
    csm->base.fd = fd;
    radeon_get_device_id(fd, &csm->device_id);
    return &csm->base;
}

drm_public void radeon_cs_manager_gem_dtor(struct radeon_cs_manager *csm)
{
    free(csm);
}
