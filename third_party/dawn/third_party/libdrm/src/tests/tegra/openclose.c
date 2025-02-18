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

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "xf86drm.h"
#include "tegra.h"

static const char default_device[] = "/dev/dri/card0";

int main(int argc, char *argv[])
{
    struct drm_tegra *tegra;
    drmVersionPtr version;
    const char *device;
    int err, fd;

    if (argc < 2)
        device = default_device;
    else
        device = argv[1];

    fd = open(device, O_RDWR);
    if (fd < 0)
        return 1;

    version = drmGetVersion(fd);
    if (version) {
        printf("Version: %d.%d.%d\n", version->version_major,
               version->version_minor, version->version_patchlevel);
        printf("  Name: %s\n", version->name);
        printf("  Date: %s\n", version->date);
        printf("  Description: %s\n", version->desc);

        drmFreeVersion(version);
    }

    err = drm_tegra_new(fd, &tegra);
    if (err < 0)
        return 1;

    drm_tegra_close(tegra);
    close(fd);

    return 0;
}
