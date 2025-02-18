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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>

#include <xf86drm.h>

#include "exynos_drm.h"
#include "exynos_drmif.h"
#include "exynos_fimg2d.h"

static int output_mathematica = 0;

static int fimg2d_perf_simple(struct exynos_bo *bo, struct g2d_context *ctx,
			unsigned buf_width, unsigned buf_height, unsigned iterations)
{
	struct timespec tspec = { 0 };
	struct g2d_image img = { 0 };

	unsigned long long g2d_time;
	unsigned i;
	int ret = 0;

	img.width = buf_width;
	img.height = buf_height;
	img.stride = buf_width * 4;
	img.color_mode = G2D_COLOR_FMT_ARGB8888 | G2D_ORDER_AXRGB;
	img.buf_type = G2D_IMGBUF_GEM;
	img.bo[0] = bo->handle;

	srand(time(NULL));

	printf("starting simple G2D performance test\n");
	printf("buffer width = %u, buffer height = %u, iterations = %u\n",
		buf_width, buf_height, iterations);

	if (output_mathematica)
		putchar('{');

	for (i = 0; i < iterations; ++i) {
		unsigned x, y, w, h;

		x = rand() % buf_width;
		y = rand() % buf_height;

		if (x == (buf_width - 1))
			x -= 1;
		if (y == (buf_height - 1))
			y -= 1;

		w = rand() % (buf_width - x);
		h = rand() % (buf_height - y);

		if (w == 0) w = 1;
		if (h == 0) h = 1;

		img.color = rand();

		ret = g2d_solid_fill(ctx, &img, x, y, w, h);

		clock_gettime(CLOCK_MONOTONIC, &tspec);

		if (ret == 0)
			ret = g2d_exec(ctx);

		if (ret != 0) {
			fprintf(stderr, "error: iteration %u failed (x = %u, y = %u, w = %u, h = %u)\n",
				i, x, y, w, h);
			break;
		} else {
			struct timespec end = { 0 };
			clock_gettime(CLOCK_MONOTONIC, &end);

			g2d_time = (end.tv_sec - tspec.tv_sec) * 1000000000ULL;
			g2d_time += (end.tv_nsec - tspec.tv_nsec);

			if (output_mathematica) {
				if (i != 0) putchar(',');
				printf("{%u,%llu}", w * h, g2d_time);
			} else {
				printf("num_pixels = %u, usecs = %llu\n", w * h, g2d_time);
			}
		}
	}

	if (output_mathematica)
		printf("}\n");

	return ret;
}

static int fimg2d_perf_multi(struct exynos_bo *bo, struct g2d_context *ctx,
			unsigned buf_width, unsigned buf_height, unsigned iterations, unsigned batch)
{
	struct timespec tspec = { 0 };
	struct g2d_image *images;

	unsigned long long g2d_time;
	unsigned i, j;
	int ret = 0;

	images = calloc(batch, sizeof(struct g2d_image));
	if (images == NULL) {
		fprintf(stderr, "error: failed to allocate G2D images.\n");
		return -ENOMEM;
	}

	for (i = 0; i < batch; ++i) {
		images[i].width = buf_width;
		images[i].height = buf_height;
		images[i].stride = buf_width * 4;
		images[i].color_mode = G2D_COLOR_FMT_ARGB8888 | G2D_ORDER_AXRGB;
		images[i].buf_type = G2D_IMGBUF_GEM;
		images[i].bo[0] = bo->handle;
	}

	srand(time(NULL));

	printf("starting multi G2D performance test (batch size = %u)\n", batch);
	printf("buffer width = %u, buffer height = %u, iterations = %u\n",
		buf_width, buf_height, iterations);

	if (output_mathematica)
		putchar('{');

	for (i = 0; i < iterations; ++i) {
		unsigned num_pixels = 0;

		for (j = 0; j < batch; ++j) {
			unsigned x, y, w, h;

			x = rand() % buf_width;
			y = rand() % buf_height;

			if (x == (buf_width - 1))
				x -= 1;
			if (y == (buf_height - 1))
				y -= 1;

			w = rand() % (buf_width - x);
			h = rand() % (buf_height - y);

			if (w == 0) w = 1;
			if (h == 0) h = 1;

			images[j].color = rand();

			num_pixels += w * h;

			ret = g2d_solid_fill(ctx, &images[j], x, y, w, h);
			if (ret != 0)
				break;
		}

		clock_gettime(CLOCK_MONOTONIC, &tspec);

		if (ret == 0)
			ret = g2d_exec(ctx);

		if (ret != 0) {
			fprintf(stderr, "error: iteration %u failed (num_pixels = %u)\n", i, num_pixels);
			break;
		} else {
			struct timespec end = { 0 };
			clock_gettime(CLOCK_MONOTONIC, &end);

			g2d_time = (end.tv_sec - tspec.tv_sec) * 1000000000ULL;
			g2d_time += (end.tv_nsec - tspec.tv_nsec);

			if (output_mathematica) {
				if (i != 0) putchar(',');
				printf("{%u,%llu}", num_pixels, g2d_time);
			} else {
				printf("num_pixels = %u, usecs = %llu\n", num_pixels, g2d_time);
			}
		}
	}

	if (output_mathematica)
		printf("}\n");

	free(images);

	return ret;
}

static void usage(const char *name)
{
	fprintf(stderr, "usage: %s [-ibwh]\n\n", name);

	fprintf(stderr, "\t-i <number of iterations>\n");
	fprintf(stderr, "\t-b <size of a batch> (default = 3)\n\n");

	fprintf(stderr, "\t-w <buffer width> (default = 4096)\n");
	fprintf(stderr, "\t-h <buffer height> (default = 4096)\n\n");

	fprintf(stderr, "\t-M <enable Mathematica styled output>\n");

	exit(0);
}

int main(int argc, char **argv)
{
	int fd, ret, c, parsefail;

	struct exynos_device *dev;
	struct g2d_context *ctx;
	struct exynos_bo *bo;

	unsigned int iters = 0, batch = 3;
	unsigned int bufw = 4096, bufh = 4096;

	ret = 0;
	parsefail = 0;

	while ((c = getopt(argc, argv, "i:b:w:h:M")) != -1) {
		switch (c) {
		case 'i':
			if (sscanf(optarg, "%u", &iters) != 1)
				parsefail = 1;
			break;
		case 'b':
			if (sscanf(optarg, "%u", &batch) != 1)
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
		case 'M':
			output_mathematica = 1;
			break;
		default:
			parsefail = 1;
			break;
		}
	}

	if (parsefail || (argc == 1) || (iters == 0))
		usage(argv[0]);

	if (bufw < 2 || bufw > 4096 || bufh < 2 || bufh > 4096) {
		fprintf(stderr, "error: buffer width/height should be in the range 2 to 4096.\n");
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

	ret = fimg2d_perf_simple(bo, ctx, bufw, bufh, iters);

	if (ret == 0)
		ret = fimg2d_perf_multi(bo, ctx, bufw, bufh, iters, batch);

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
