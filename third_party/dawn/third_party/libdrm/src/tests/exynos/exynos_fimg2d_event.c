/*
 * Copyright (C) 2015 - Tobias Jakobi
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
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <unistd.h>
#include <poll.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>

#include <pthread.h>

#include <xf86drm.h>

#include "exynos_drm.h"
#include "exynos_drmif.h"
#include "exynos_fimg2d.h"

struct g2d_job {
	unsigned int id;
	unsigned int busy;
};

struct exynos_evhandler {
	struct pollfd fds;
	struct exynos_event_context evctx;
};

struct threaddata {
	unsigned int stop;
	struct exynos_device *dev;
	struct exynos_evhandler evhandler;
};

static void g2d_event_handler(int fd, unsigned int cmdlist_no, unsigned int tv_sec,
							unsigned int tv_usec, void *user_data)
{
	struct g2d_job *job = user_data;

	fprintf(stderr, "info: g2d job (id = %u, cmdlist number = %u) finished!\n",
			job->id, cmdlist_no);

	job->busy = 0;
}

static void setup_g2d_event_handler(struct exynos_evhandler *evhandler, int fd)
{
	evhandler->fds.fd = fd;
	evhandler->fds.events = POLLIN;
	evhandler->evctx.base.version = 2;
	evhandler->evctx.version = 1;
	evhandler->evctx.g2d_event_handler = g2d_event_handler;
}

static void* threadfunc(void *arg) {
	const int timeout = 0;
	struct threaddata *data;

	data = arg;

	while (1) {
		if (data->stop) break;

		usleep(500);

		data->evhandler.fds.revents = 0;

		if (poll(&data->evhandler.fds, 1, timeout) < 0)
			continue;

		if (data->evhandler.fds.revents & (POLLHUP | POLLERR))
			continue;

		if (data->evhandler.fds.revents & POLLIN)
			exynos_handle_event(data->dev, &data->evhandler.evctx);
	}

	pthread_exit(0);
}

/*
 * We need to wait until all G2D jobs are finished, otherwise we
 * potentially remove a BO which the engine still operates on.
 * This results in the following kernel message:
 * [drm:exynos_drm_gem_put_dma_addr] *ERROR* failed to lookup gem object.
 * Also any subsequent BO allocations fail then with:
 * [drm:exynos_drm_alloc_buf] *ERROR* failed to allocate buffer.
 */
static void wait_all_jobs(struct g2d_job* jobs, unsigned num_jobs)
{
	unsigned i;

	for (i = 0; i < num_jobs; ++i) {
		while (jobs[i].busy)
			usleep(500);
	}

}

static struct g2d_job* free_job(struct g2d_job* jobs, unsigned num_jobs)
{
	unsigned i;

	for (i = 0; i < num_jobs; ++i) {
		if (jobs[i].busy == 0)
			return &jobs[i];
	}

	return NULL;
}

static int g2d_work(struct g2d_context *ctx, struct g2d_image *img,
					unsigned num_jobs, unsigned iterations)
{
	struct g2d_job *jobs = calloc(num_jobs, sizeof(struct g2d_job));
	int ret;
	unsigned i;

	/* setup job ids */
	for (i = 0; i < num_jobs; ++i)
		jobs[i].id = i;

	for (i = 0; i < iterations; ++i) {
		unsigned x, y, w, h;

		struct g2d_job *j = NULL;

		while (1) {
			j = free_job(jobs, num_jobs);

			if (j)
				break;
			else
				usleep(500);
		}

		x = rand() % img->width;
		y = rand() % img->height;

		if (x == (img->width - 1))
			x -= 1;
		if (y == (img->height - 1))
			y -= 1;

		w = rand() % (img->width - x);
		h = rand() % (img->height - y);

		if (w == 0) w = 1;
		if (h == 0) h = 1;

		img->color = rand();

		j->busy = 1;
		g2d_config_event(ctx, j);

		ret = g2d_solid_fill(ctx, img, x, y, w, h);

		if (ret == 0)
			g2d_exec(ctx);

		if (ret != 0) {
			fprintf(stderr, "error: iteration %u (x = %u, x = %u, x = %u, x = %u) failed\n",
					i, x, y, w, h);
			break;
		}
	}

	wait_all_jobs(jobs, num_jobs);
	free(jobs);

	return 0;
}

static void usage(const char *name)
{
	fprintf(stderr, "usage: %s [-ijwh]\n\n", name);

	fprintf(stderr, "\t-i <number of iterations>\n");
	fprintf(stderr, "\t-j <number of G2D jobs> (default = 4)\n\n");

	fprintf(stderr, "\t-w <buffer width> (default = 4096)\n");
	fprintf(stderr, "\t-h <buffer height> (default = 4096)\n");

	exit(0);
}

int main(int argc, char **argv)
{
	int fd, ret, c, parsefail;

	pthread_t event_thread;
	struct threaddata event_data = {0};

	struct exynos_device *dev;
	struct g2d_context *ctx;
	struct exynos_bo *bo;

	struct g2d_image img = {0};

	unsigned int iters = 0, njobs = 4;
	unsigned int bufw = 4096, bufh = 4096;

	ret = 0;
	parsefail = 0;

	while ((c = getopt(argc, argv, "i:j:w:h:")) != -1) {
		switch (c) {
		case 'i':
			if (sscanf(optarg, "%u", &iters) != 1)
				parsefail = 1;
			break;
		case 'j':
			if (sscanf(optarg, "%u", &njobs) != 1)
				parsefail = 1;
			break;
		case 'w':
			if (sscanf(optarg, "%u", &bufw) != 1)
				parsefail = 1;
			break;
		case 'h':
			if (sscanf(optarg, "%u", &bufh) != 1)
				parsefail = 1;
			break;
		default:
			parsefail = 1;
			break;
		}
	}

	if (parsefail || (argc == 1) || (iters == 0))
		usage(argv[0]);

	if (bufw > 4096 || bufh > 4096) {
		fprintf(stderr, "error: buffer width/height should be less than 4096.\n");
		ret = -1;

		goto out;
	}

	if (bufw == 0 || bufh == 0) {
		fprintf(stderr, "error: buffer width/height should be non-zero.\n");
		ret = -1;

		goto out;
	}

	fd = drmOpen("exynos", NULL);
	if (fd < 0) {
		fprintf(stderr, "error: failed to open drm\n");
		ret = -1;

		goto out;
	}

	dev = exynos_device_create(fd);
	if (dev == NULL) {
		fprintf(stderr, "error: failed to create device\n");
		ret = -2;

		goto fail;
	}

	ctx = g2d_init(fd);
	if (ctx == NULL) {
		fprintf(stderr, "error: failed to init G2D\n");
		ret = -3;

		goto g2d_fail;
	}

	bo = exynos_bo_create(dev, bufw * bufh * 4, 0);
	if (bo == NULL) {
		fprintf(stderr, "error: failed to create bo\n");
		ret = -4;

		goto bo_fail;
	}

	/* setup g2d image object */
	img.width = bufw;
	img.height = bufh;
	img.stride = bufw * 4;
	img.color_mode = G2D_COLOR_FMT_ARGB8888 | G2D_ORDER_AXRGB;
	img.buf_type = G2D_IMGBUF_GEM;
	img.bo[0] = bo->handle;

	event_data.dev = dev;
	setup_g2d_event_handler(&event_data.evhandler, fd);

	pthread_create(&event_thread, NULL, threadfunc, &event_data);

	ret = g2d_work(ctx, &img, njobs, iters);
	if (ret != 0)
		fprintf(stderr, "error: g2d_work failed\n");

	event_data.stop = 1;
	pthread_join(event_thread, NULL);

	exynos_bo_destroy(bo);

bo_fail:
	g2d_fini(ctx);

g2d_fail:
	exynos_device_destroy(dev);

fail:
	drmClose(fd);

out:
	return ret;
}
