/*
 * Copyright (C) 2014-2015 Etnaviv Project
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
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "etnaviv_priv.h"

drm_public int etna_pipe_wait(struct etna_pipe *pipe, uint32_t timestamp, uint32_t ms)
{
	return etna_pipe_wait_ns(pipe, timestamp, ms * 1000000);
}

drm_public int etna_pipe_wait_ns(struct etna_pipe *pipe, uint32_t timestamp, uint64_t ns)
{
	struct etna_device *dev = pipe->gpu->dev;
	int ret;

	struct drm_etnaviv_wait_fence req = {
		.pipe = pipe->gpu->core,
		.fence = timestamp,
	};

	if (ns == 0)
		req.flags |= ETNA_WAIT_NONBLOCK;

	get_abs_timeout(&req.timeout, ns);

	ret = drmCommandWrite(dev->fd, DRM_ETNAVIV_WAIT_FENCE, &req, sizeof(req));
	if (ret) {
		ERROR_MSG("wait-fence failed! %d (%s)", ret, strerror(errno));
		return ret;
	}

	return 0;
}

drm_public void etna_pipe_del(struct etna_pipe *pipe)
{
	free(pipe);
}

drm_public struct etna_pipe *etna_pipe_new(struct etna_gpu *gpu, enum etna_pipe_id id)
{
	struct etna_pipe *pipe;

	pipe = calloc(1, sizeof(*pipe));
	if (!pipe) {
		ERROR_MSG("allocation failed");
		goto fail;
	}

	pipe->id = id;
	pipe->gpu = gpu;

	return pipe;
fail:
	return NULL;
}
