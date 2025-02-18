/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "msm_priv.h"

static void msm_device_destroy(struct fd_device *dev)
{
	struct msm_device *msm_dev = to_msm_device(dev);
	free(msm_dev);
}

static const struct fd_device_funcs funcs = {
		.bo_new_handle = msm_bo_new_handle,
		.bo_from_handle = msm_bo_from_handle,
		.pipe_new = msm_pipe_new,
		.destroy = msm_device_destroy,
};

drm_private struct fd_device * msm_device_new(int fd)
{
	struct msm_device *msm_dev;
	struct fd_device *dev;

	msm_dev = calloc(1, sizeof(*msm_dev));
	if (!msm_dev)
		return NULL;

	dev = &msm_dev->base;
	dev->funcs = &funcs;

	dev->bo_size = sizeof(struct msm_bo);

	return dev;
}
