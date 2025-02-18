/*
 * Copyright © 2009 Red Hat Inc.
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
 */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "libdrm_macros.h"
#include "radeon_cs.h"
#include "radeon_bo_int.h"
#include "radeon_cs_int.h"

struct rad_sizes {
    int32_t op_read;
    int32_t op_gart_write;
    int32_t op_vram_write;
};

static inline int radeon_cs_setup_bo(struct radeon_cs_space_check *sc, struct rad_sizes *sizes)
{
    uint32_t read_domains, write_domain;
    struct radeon_bo_int *bo;

    bo = sc->bo;
    sc->new_accounted = 0;
    read_domains = sc->read_domains;
    write_domain = sc->write_domain;

    /* legacy needs a static check */
    if (radeon_bo_is_static((struct radeon_bo *)sc->bo)) {
        bo->space_accounted = sc->new_accounted = (read_domains << 16) | write_domain;
        return 0;
    }

    /* already accounted this bo */
    if (write_domain && (write_domain == bo->space_accounted)) {
        sc->new_accounted = bo->space_accounted;
        return 0;
    }
    if (read_domains && ((read_domains << 16) == bo->space_accounted)) {
        sc->new_accounted = bo->space_accounted;
        return 0;
    }

    if (bo->space_accounted == 0) {
        if (write_domain) {
            if (write_domain == RADEON_GEM_DOMAIN_VRAM)
                sizes->op_vram_write += bo->size;
            else if (write_domain == RADEON_GEM_DOMAIN_GTT)
                sizes->op_gart_write += bo->size;
            sc->new_accounted = write_domain;
        } else {
            sizes->op_read += bo->size;
            sc->new_accounted = read_domains << 16;
        }
    } else {
        uint16_t old_read, old_write;

        old_read = bo->space_accounted >> 16;
        old_write = bo->space_accounted & 0xffff;

        if (write_domain && (old_read & write_domain)) {
            sc->new_accounted = write_domain;
            /* moving from read to a write domain */
            if (write_domain == RADEON_GEM_DOMAIN_VRAM) {
                sizes->op_read -= bo->size;
                sizes->op_vram_write += bo->size;
            } else if (write_domain == RADEON_GEM_DOMAIN_GTT) {
                sizes->op_read -= bo->size;
                sizes->op_gart_write += bo->size;
            }
        } else if (read_domains & old_write) {
            sc->new_accounted = bo->space_accounted & 0xffff;
        } else {
            /* rewrite the domains */
            if (write_domain != old_write)
                fprintf(stderr,"WRITE DOMAIN RELOC FAILURE 0x%x %d %d\n", bo->handle, write_domain, old_write);
            if (read_domains != old_read)
               fprintf(stderr,"READ DOMAIN RELOC FAILURE 0x%x %d %d\n", bo->handle, read_domains, old_read);
            return RADEON_CS_SPACE_FLUSH;
        }
    }
    return 0;
}

static int radeon_cs_do_space_check(struct radeon_cs_int *cs, struct radeon_cs_space_check *new_tmp)
{
    struct radeon_cs_manager *csm = cs->csm;
    int i;
    struct radeon_bo_int *bo;
    struct rad_sizes sizes;
    int ret;

    /* check the totals for this operation */

    if (cs->bo_count == 0 && !new_tmp)
        return 0;

    memset(&sizes, 0, sizeof(struct rad_sizes));

    /* prepare */
    for (i = 0; i < cs->bo_count; i++) {
        ret = radeon_cs_setup_bo(&cs->bos[i], &sizes);
        if (ret)
            return ret;
    }

    if (new_tmp) {
        ret = radeon_cs_setup_bo(new_tmp, &sizes);
        if (ret)
            return ret;
    }

    if (sizes.op_read < 0)
        sizes.op_read = 0;

    /* check sizes - operation first */
    if ((sizes.op_read + sizes.op_gart_write > csm->gart_limit) ||
        (sizes.op_vram_write > csm->vram_limit)) {
        return RADEON_CS_SPACE_OP_TO_BIG;
    }

    if (((csm->vram_write_used + sizes.op_vram_write) > csm->vram_limit) ||
        ((csm->read_used + csm->gart_write_used + sizes.op_gart_write + sizes.op_read) > csm->gart_limit)) {
        return RADEON_CS_SPACE_FLUSH;
    }

    csm->gart_write_used += sizes.op_gart_write;
    csm->vram_write_used += sizes.op_vram_write;
    csm->read_used += sizes.op_read;
    /* commit */
    for (i = 0; i < cs->bo_count; i++) {
        bo = cs->bos[i].bo;
        bo->space_accounted = cs->bos[i].new_accounted;
    }
    if (new_tmp)
        new_tmp->bo->space_accounted = new_tmp->new_accounted;

    return RADEON_CS_SPACE_OK;
}

drm_public void
radeon_cs_space_add_persistent_bo(struct radeon_cs *cs, struct radeon_bo *bo,
                                  uint32_t read_domains, uint32_t write_domain)
{
    struct radeon_cs_int *csi = (struct radeon_cs_int *)cs;
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    int i;
    for (i = 0; i < csi->bo_count; i++) {
        if (csi->bos[i].bo == boi &&
            csi->bos[i].read_domains == read_domains &&
            csi->bos[i].write_domain == write_domain)
            return;
    }
    radeon_bo_ref(bo);
    i = csi->bo_count;
    csi->bos[i].bo = boi;
    csi->bos[i].read_domains = read_domains;
    csi->bos[i].write_domain = write_domain;
    csi->bos[i].new_accounted = 0;
    csi->bo_count++;

    assert(csi->bo_count < MAX_SPACE_BOS);
}

static int radeon_cs_check_space_internal(struct radeon_cs_int *cs,
                      struct radeon_cs_space_check *tmp_bo)
{
    int ret;
    int flushed = 0;

again:
    ret = radeon_cs_do_space_check(cs, tmp_bo);
    if (ret == RADEON_CS_SPACE_OP_TO_BIG)
        return -1;
    if (ret == RADEON_CS_SPACE_FLUSH) {
        (*cs->space_flush_fn)(cs->space_flush_data);
        if (flushed)
            return -1;
        flushed = 1;
        goto again;
    }
    return 0;
}

drm_public int
radeon_cs_space_check_with_bo(struct radeon_cs *cs, struct radeon_bo *bo,
                              uint32_t read_domains, uint32_t write_domain)
{
    struct radeon_cs_int *csi = (struct radeon_cs_int *)cs;
    struct radeon_bo_int *boi = (struct radeon_bo_int *)bo;
    struct radeon_cs_space_check temp_bo;

    int ret = 0;

    if (bo) {
        temp_bo.bo = boi;
        temp_bo.read_domains = read_domains;
        temp_bo.write_domain = write_domain;
        temp_bo.new_accounted = 0;
    }

    ret = radeon_cs_check_space_internal(csi, bo ? &temp_bo : NULL);
    return ret;
}

drm_public int radeon_cs_space_check(struct radeon_cs *cs)
{
    struct radeon_cs_int *csi = (struct radeon_cs_int *)cs;
    return radeon_cs_check_space_internal(csi, NULL);
}

drm_public void radeon_cs_space_reset_bos(struct radeon_cs *cs)
{
    struct radeon_cs_int *csi = (struct radeon_cs_int *)cs;
    int i;
    for (i = 0; i < csi->bo_count; i++) {
        radeon_bo_unref((struct radeon_bo *)csi->bos[i].bo);
        csi->bos[i].bo = NULL;
        csi->bos[i].read_domains = 0;
        csi->bos[i].write_domain = 0;
        csi->bos[i].new_accounted = 0;
    }
    csi->bo_count = 0;
}
