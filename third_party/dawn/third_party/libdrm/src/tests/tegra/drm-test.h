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

#ifndef TEGRA_DRM_TEST_H
#define TEGRA_DRM_TEST_H

#include <stdint.h>
#include <stdlib.h>

#include "xf86drmMode.h"

struct drm_screen {
    int fd;

    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int depth;
    unsigned int bpp;

    drmModeModeInfo mode;
    uint32_t connector;
    uint32_t old_fb;
    uint32_t format;
    uint32_t crtc;
};

struct drm_framebuffer {
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    uint32_t format;
    uint32_t handle;
    void *data;
    int fd;
};

int drm_screen_open(struct drm_screen **screenp, int fd);
int drm_screen_close(struct drm_screen *screen);
int drm_screen_set_framebuffer(struct drm_screen *screen,
                               struct drm_framebuffer *fb);

int drm_framebuffer_new(struct drm_framebuffer **fbp,
                        struct drm_screen *screen, uint32_t handle,
                        unsigned int width, unsigned int height,
                        unsigned int pitch, uint32_t format,
                        void *data);
int drm_framebuffer_free(struct drm_framebuffer *fb);

int drm_open(const char *path);
void drm_close(int fd);

#endif
