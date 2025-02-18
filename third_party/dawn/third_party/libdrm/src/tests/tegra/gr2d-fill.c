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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "drm_fourcc.h"

#include "drm-test-tegra.h"
#include "tegra.h"

int main(int argc, char *argv[])
{
    uint32_t format = DRM_FORMAT_XRGB8888;
    struct drm_tegra_gr2d *gr2d;
    struct drm_framebuffer *fb;
    struct drm_screen *screen;
    unsigned int pitch, size;
    struct drm_tegra_bo *bo;
    struct drm_tegra *drm;
    uint32_t handle;
    int fd, err;
    void *ptr;

    fd = drm_open(argv[1]);
    if (fd < 0) {
        fprintf(stderr, "failed to open DRM device %s: %s\n", argv[1],
                strerror(errno));
        return 1;
    }

    err = drm_screen_open(&screen, fd);
    if (err < 0) {
        fprintf(stderr, "failed to open screen: %s\n", strerror(-err));
        return 1;
    }

    err = drm_tegra_new(fd, &drm);
    if (err < 0) {
        fprintf(stderr, "failed to create Tegra DRM context: %s\n",
                strerror(-err));
        return 1;
    }

    err = drm_tegra_gr2d_open(drm, &gr2d);
    if (err < 0) {
        fprintf(stderr, "failed to open gr2d channel: %s\n",
                strerror(-err));
        return 1;
    }

    pitch = screen->width * screen->bpp / 8;
    size = pitch * screen->height;

    err = drm_tegra_bo_new(drm, 0, size, &bo);
    if (err < 0) {
        fprintf(stderr, "failed to create buffer object: %s\n",
                strerror(-err));
        return 1;
    }

    err = drm_tegra_bo_get_handle(bo, &handle);
    if (err < 0) {
        fprintf(stderr, "failed to get handle to buffer object: %s\n",
                strerror(-err));
        return 1;
    }

    err = drm_tegra_bo_map(bo, &ptr);
    if (err < 0) {
        fprintf(stderr, "failed to map buffer object: %s\n",
                strerror(-err));
        return 1;
    }

    memset(ptr, 0xff, size);

    err = drm_framebuffer_new(&fb, screen, handle, screen->width,
                              screen->height, pitch, format, bo);
    if (err < 0) {
        fprintf(stderr, "failed to create framebuffer: %s\n",
                strerror(-err));
        return 1;
    }

    err = drm_screen_set_framebuffer(screen, fb);
    if (err < 0) {
        fprintf(stderr, "failed to display framebuffer: %s\n",
                strerror(-err));
        return 1;
    }

    sleep(1);

    err = drm_tegra_gr2d_fill(gr2d, fb, fb->width / 4, fb->height / 4,
                              fb->width / 2, fb->height / 2, 0x00000000);
    if (err < 0) {
        fprintf(stderr, "failed to fill rectangle: %s\n",
                strerror(-err));
        return 1;
    }

    sleep(1);

    drm_framebuffer_free(fb);
    drm_tegra_bo_unref(bo);
    drm_tegra_gr2d_close(gr2d);
    drm_tegra_close(drm);
    drm_screen_close(screen);
    drm_close(fd);

    return 0;
}
