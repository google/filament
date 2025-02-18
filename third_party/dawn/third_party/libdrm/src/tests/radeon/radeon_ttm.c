/*
 * Copyright © 2011 Red Hat
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
 *    Jerome Glisse <j.glisse@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rbo.h"
#include "xf86drm.h"

/* allocate as many single page bo to try to starve the kernel
 * memory zone (below highmem)
 */
static void ttm_starve_kernel_private_memory(int fd)
{
    struct list_head list;
    struct rbo *bo, *tmp;
    unsigned nbo = 0;

    printf("\n[%s]\n", __func__);
    list_inithead(&list);
    while (1) {
        bo = rbo(fd, 0, 4096, 0, NULL);
        if (bo == NULL) {
            printf("failing after %d bo\n", nbo);
            break;
        }
        nbo++;
        list_add(&bo->list, &list);
    }
    LIST_FOR_EACH_ENTRY_SAFE(bo, tmp, &list, list) {
        list_del(&bo->list);
        rbo_decref(bo);
    }
}

static int radeon_open_fd(void)
{
    return drmOpen("radeon", NULL);
}

int main(void)
{
    int radeonfd;

    radeonfd = radeon_open_fd();
    if (radeonfd < 0) {
        fprintf(stderr, "failed to open radeon fd\n");
        return -1;
    }

    ttm_starve_kernel_private_memory(radeonfd);

    close(radeonfd);
    return 0;
}
