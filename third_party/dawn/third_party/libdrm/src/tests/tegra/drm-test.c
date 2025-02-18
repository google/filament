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

#include "drm-test.h"

static int drm_screen_probe_connector(struct drm_screen *screen,
                                      drmModeConnectorPtr connector)
{
    drmModeEncoderPtr encoder;
    drmModeCrtcPtr crtc;
    drmModeFBPtr fb;

    encoder = drmModeGetEncoder(screen->fd, connector->encoder_id);
    if (!encoder)
        return -ENODEV;

    crtc = drmModeGetCrtc(screen->fd, encoder->crtc_id);
    if (!crtc) {
        drmModeFreeEncoder(encoder);
        return -ENODEV;
    }

    screen->old_fb = crtc->buffer_id;

    fb = drmModeGetFB(screen->fd, crtc->buffer_id);
    if (!fb) {
        /* TODO: create new framebuffer */
        drmModeFreeEncoder(encoder);
        drmModeFreeCrtc(crtc);
        return -ENOSYS;
    }

    screen->connector = connector->connector_id;
    screen->old_fb = crtc->buffer_id;
    screen->crtc = encoder->crtc_id;
    /* TODO: check crtc->mode_valid */
    screen->mode = crtc->mode;

    screen->width = fb->width;
    screen->height = fb->height;
    screen->pitch = fb->pitch;
    screen->depth = fb->depth;
    screen->bpp = fb->bpp;

    drmModeFreeEncoder(encoder);
    drmModeFreeCrtc(crtc);
    drmModeFreeFB(fb);

    return 0;
}

int drm_screen_open(struct drm_screen **screenp, int fd)
{
    drmModeConnectorPtr connector;
    struct drm_screen *screen;
    bool found = false;
    drmModeResPtr res;
    unsigned int i;
    int err;

    if (!screenp || fd < 0)
        return -EINVAL;

    screen = calloc(1, sizeof(*screen));
    if (!screen)
        return -ENOMEM;

    screen->format = DRM_FORMAT_XRGB8888;
    screen->fd = fd;

    res = drmModeGetResources(fd);
    if (!res) {
        free(screen);
        return -ENOMEM;
    }

    for (i = 0; i < (unsigned int)res->count_connectors; i++) {
        connector = drmModeGetConnector(fd, res->connectors[i]);
        if (!connector)
            continue;

        if (connector->connection != DRM_MODE_CONNECTED) {
            drmModeFreeConnector(connector);
            continue;
        }

        err = drm_screen_probe_connector(screen, connector);
        if (err < 0) {
            drmModeFreeConnector(connector);
            continue;
        }

        drmModeFreeConnector(connector);
        found = true;
        break;
    }

    drmModeFreeResources(res);

    if (!found) {
        free(screen);
        return -ENODEV;
    }

    *screenp = screen;

    return 0;
}

int drm_screen_close(struct drm_screen *screen)
{
    int err;

    err = drmModeSetCrtc(screen->fd, screen->crtc, screen->old_fb, 0, 0,
                         &screen->connector, 1, &screen->mode);
    if (err < 0) {
        fprintf(stderr, "drmModeSetCrtc() failed: %m\n");
        return -errno;
    }

    free(screen);

    return 0;
}

int drm_framebuffer_new(struct drm_framebuffer **fbp,
                        struct drm_screen *screen, uint32_t handle,
                        unsigned int width, unsigned int height,
                        unsigned int pitch, uint32_t format,
                        void *data)
{
    struct drm_framebuffer *fb;
    uint32_t handles[4];
    uint32_t pitches[4];
    uint32_t offsets[4];
    int err;

    fb = calloc(1, sizeof(*fb));
    if (!fb)
        return -ENOMEM;

    fb->fd = screen->fd;
    fb->width = width;
    fb->height = height;
    fb->pitch = pitch;
    fb->format = format;
    fb->data = data;

    handles[0] = handle;
    pitches[0] = pitch;
    offsets[0] = 0;

    err = drmModeAddFB2(screen->fd, width, height, format, handles,
                        pitches, offsets, &fb->handle, 0);
    if (err < 0)
        return -errno;

    *fbp = fb;

    return 0;
}

int drm_framebuffer_free(struct drm_framebuffer *fb)
{
    int err;

    err = drmModeRmFB(fb->fd, fb->handle);
    if (err < 0)
        return -errno;

    free(fb);

    return 0;
}

int drm_screen_set_framebuffer(struct drm_screen *screen,
                               struct drm_framebuffer *fb)
{
    int err;

    err = drmModeSetCrtc(screen->fd, screen->crtc, fb->handle, 0, 0,
                         &screen->connector, 1, &screen->mode);
    if (err < 0)
        return -errno;

    return 0;
}

int drm_open(const char *path)
{
    int fd, err;

    fd = open(path, O_RDWR);
    if (fd < 0)
        return -errno;

    err = drmSetMaster(fd);
    if (err < 0) {
        close(fd);
        return -errno;
    }

    return fd;
}

void drm_close(int fd)
{
    drmDropMaster(fd);
    close(fd);
}
