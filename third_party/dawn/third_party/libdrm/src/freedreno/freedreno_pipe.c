/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#include "freedreno_drmif.h"
#include "freedreno_priv.h"

/**
 * priority of zero is highest priority, and higher numeric values are
 * lower priorities
 */
drm_public struct fd_pipe *
fd_pipe_new2(struct fd_device *dev, enum fd_pipe_id id, uint32_t prio)
{
	struct fd_pipe *pipe;
	uint64_t val;

	if (id > FD_PIPE_MAX) {
		ERROR_MSG("invalid pipe id: %d", id);
		return NULL;
	}

	if ((prio != 1) && (fd_device_version(dev) < FD_VERSION_SUBMIT_QUEUES)) {
		ERROR_MSG("invalid priority!");
		return NULL;
	}

	pipe = dev->funcs->pipe_new(dev, id, prio);
	if (!pipe) {
		ERROR_MSG("allocation failed");
		return NULL;
	}

	pipe->dev = dev;
	pipe->id = id;
	atomic_set(&pipe->refcnt, 1);

	fd_pipe_get_param(pipe, FD_GPU_ID, &val);
	pipe->gpu_id = val;

	return pipe;
}

drm_public struct fd_pipe *
fd_pipe_new(struct fd_device *dev, enum fd_pipe_id id)
{
	return fd_pipe_new2(dev, id, 1);
}

drm_public struct fd_pipe * fd_pipe_ref(struct fd_pipe *pipe)
{
	atomic_inc(&pipe->refcnt);
	return pipe;
}

drm_public void fd_pipe_del(struct fd_pipe *pipe)
{
	if (!atomic_dec_and_test(&pipe->refcnt))
		return;
	pipe->funcs->destroy(pipe);
}

drm_public int fd_pipe_get_param(struct fd_pipe *pipe,
				 enum fd_param_id param, uint64_t *value)
{
	return pipe->funcs->get_param(pipe, param, value);
}

drm_public int fd_pipe_wait(struct fd_pipe *pipe, uint32_t timestamp)
{
	return fd_pipe_wait_timeout(pipe, timestamp, ~0);
}

drm_public int fd_pipe_wait_timeout(struct fd_pipe *pipe, uint32_t timestamp,
		uint64_t timeout)
{
	return pipe->funcs->wait(pipe, timestamp, timeout);
}
