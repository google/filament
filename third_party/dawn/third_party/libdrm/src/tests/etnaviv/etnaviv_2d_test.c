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

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "xf86drm.h"
#include "etnaviv_drmif.h"
#include "etnaviv_drm.h"

#include "state.xml.h"
#include "state_2d.xml.h"
#include "cmdstream.xml.h"

#include "write_bmp.h"

static inline void etna_emit_load_state(struct etna_cmd_stream *stream,
		const uint16_t offset, const uint16_t count)
{
	uint32_t v;

	v = 	(VIV_FE_LOAD_STATE_HEADER_OP_LOAD_STATE | VIV_FE_LOAD_STATE_HEADER_OFFSET(offset) |
			(VIV_FE_LOAD_STATE_HEADER_COUNT(count) & VIV_FE_LOAD_STATE_HEADER_COUNT__MASK));

	etna_cmd_stream_emit(stream, v);
}

static inline void etna_set_state(struct etna_cmd_stream *stream, uint32_t address, uint32_t value)
{
	etna_cmd_stream_reserve(stream, 2);
	etna_emit_load_state(stream, address >> 2, 1);
	etna_cmd_stream_emit(stream, value);
}

static inline void etna_set_state_from_bo(struct etna_cmd_stream *stream,
		uint32_t address, struct etna_bo *bo)
{
	etna_cmd_stream_reserve(stream, 2);
	etna_emit_load_state(stream, address >> 2, 1);

	etna_cmd_stream_reloc(stream, &(struct etna_reloc){
		.bo = bo,
		.flags = ETNA_RELOC_READ,
		.offset = 0,
	});
}

static void gen_cmd_stream(struct etna_cmd_stream *stream, struct etna_bo *bmp, const int width, const int height)
{
	int rec;
	static int num_rects = 256;

	etna_set_state(stream, VIVS_DE_SRC_STRIDE, 0);
	etna_set_state(stream, VIVS_DE_SRC_ROTATION_CONFIG, 0);
	etna_set_state(stream, VIVS_DE_SRC_CONFIG, 0);
	etna_set_state(stream, VIVS_DE_SRC_ORIGIN, 0);
	etna_set_state(stream, VIVS_DE_SRC_SIZE, 0);
	etna_set_state(stream, VIVS_DE_SRC_COLOR_BG, 0);
	etna_set_state(stream, VIVS_DE_SRC_COLOR_FG, 0);
	etna_set_state(stream, VIVS_DE_STRETCH_FACTOR_LOW, 0);
	etna_set_state(stream, VIVS_DE_STRETCH_FACTOR_HIGH, 0);
	etna_set_state_from_bo(stream, VIVS_DE_DEST_ADDRESS, bmp);
	etna_set_state(stream, VIVS_DE_DEST_STRIDE, width*4);
	etna_set_state(stream, VIVS_DE_DEST_ROTATION_CONFIG, 0);
	etna_set_state(stream, VIVS_DE_DEST_CONFIG,
			VIVS_DE_DEST_CONFIG_FORMAT(DE_FORMAT_A8R8G8B8) |
			VIVS_DE_DEST_CONFIG_COMMAND_CLEAR |
			VIVS_DE_DEST_CONFIG_SWIZZLE(DE_SWIZZLE_ARGB) |
			VIVS_DE_DEST_CONFIG_TILED_DISABLE |
			VIVS_DE_DEST_CONFIG_MINOR_TILED_DISABLE
			);
	etna_set_state(stream, VIVS_DE_ROP,
			VIVS_DE_ROP_ROP_FG(0xcc) | VIVS_DE_ROP_ROP_BG(0xcc) | VIVS_DE_ROP_TYPE_ROP4);
	etna_set_state(stream, VIVS_DE_CLIP_TOP_LEFT,
			VIVS_DE_CLIP_TOP_LEFT_X(0) |
			VIVS_DE_CLIP_TOP_LEFT_Y(0)
			);
	etna_set_state(stream, VIVS_DE_CLIP_BOTTOM_RIGHT,
			VIVS_DE_CLIP_BOTTOM_RIGHT_X(width) |
			VIVS_DE_CLIP_BOTTOM_RIGHT_Y(height)
			);
	etna_set_state(stream, VIVS_DE_CONFIG, 0); /* TODO */
	etna_set_state(stream, VIVS_DE_SRC_ORIGIN_FRACTION, 0);
	etna_set_state(stream, VIVS_DE_ALPHA_CONTROL, 0);
	etna_set_state(stream, VIVS_DE_ALPHA_MODES, 0);
	etna_set_state(stream, VIVS_DE_DEST_ROTATION_HEIGHT, 0);
	etna_set_state(stream, VIVS_DE_SRC_ROTATION_HEIGHT, 0);
	etna_set_state(stream, VIVS_DE_ROT_ANGLE, 0);

	/* Clear color PE20 */
	etna_set_state(stream, VIVS_DE_CLEAR_PIXEL_VALUE32, 0xff40ff40);
	/* Clear color PE10 */
	etna_set_state(stream, VIVS_DE_CLEAR_BYTE_MASK, 0xff);
	etna_set_state(stream, VIVS_DE_CLEAR_PIXEL_VALUE_LOW, 0xff40ff40);
	etna_set_state(stream, VIVS_DE_CLEAR_PIXEL_VALUE_HIGH, 0xff40ff40);

	etna_set_state(stream, VIVS_DE_DEST_COLOR_KEY, 0);
	etna_set_state(stream, VIVS_DE_GLOBAL_SRC_COLOR, 0);
	etna_set_state(stream, VIVS_DE_GLOBAL_DEST_COLOR, 0);
	etna_set_state(stream, VIVS_DE_COLOR_MULTIPLY_MODES, 0);
	etna_set_state(stream, VIVS_DE_PE_TRANSPARENCY, 0);
	etna_set_state(stream, VIVS_DE_PE_CONTROL, 0);
	etna_set_state(stream, VIVS_DE_PE_DITHER_LOW, 0xffffffff);
	etna_set_state(stream, VIVS_DE_PE_DITHER_HIGH, 0xffffffff);

	/* Queue DE command */
	etna_cmd_stream_emit(stream,
			VIV_FE_DRAW_2D_HEADER_OP_DRAW_2D | VIV_FE_DRAW_2D_HEADER_COUNT(num_rects) /* render one rectangle */
		);
	etna_cmd_stream_emit(stream, 0x0); /* rectangles start aligned */

	for(rec=0; rec < num_rects; ++rec) {
		int x = rec%16;
		int y = rec/16;
		etna_cmd_stream_emit(stream, VIV_FE_DRAW_2D_TOP_LEFT_X(x*8) | VIV_FE_DRAW_2D_TOP_LEFT_Y(y*8));
		etna_cmd_stream_emit(stream, VIV_FE_DRAW_2D_BOTTOM_RIGHT_X(x*8+4) | VIV_FE_DRAW_2D_BOTTOM_RIGHT_Y(y*8+4));
	}
	etna_set_state(stream, 1, 0);
	etna_set_state(stream, 1, 0);
	etna_set_state(stream, 1, 0);

	etna_set_state(stream, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_PE2D);
}

int etna_check_image(uint32_t *p, int width, int height)
{
	int i;
	uint32_t expected;

	for (i = 0; i < width * height; i++) {
		if (i%8 < 4 && i%(width*8) < width*4 && i%width < 8*16 && i < width*8*16)
			expected = 0xff40ff40;
		else
			expected = 0x00000000;

		if (p[i] != expected) {
			fprintf(stderr, "Offset %d: expected: 0x%08x, got: 0x%08x\n",
				i, expected, p[i]);
			return -1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	const int width = 256;
	const int height = 256;
	const size_t bmp_size = width * height * 4;

	struct etna_device *dev;
	struct etna_gpu *gpu;
	struct etna_pipe *pipe;
	struct etna_bo *bmp;
	struct etna_cmd_stream *stream;

	drmVersionPtr version;
	int fd, ret = 0;
	uint64_t feat;
	int core = 0;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s /dev/dri/<device> [<etna.bmp>]\n", argv[0]);
		return 1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		perror(argv[1]);
		return 1;
	}

	version = drmGetVersion(fd);
	if (version) {
		printf("Version: %d.%d.%d\n", version->version_major,
		       version->version_minor, version->version_patchlevel);
		printf("  Name: %s\n", version->name);
		printf("  Date: %s\n", version->date);
		printf("  Description: %s\n", version->desc);
		drmFreeVersion(version);
	}

	dev = etna_device_new(fd);
	if (!dev) {
		perror("etna_device_new");
		ret = 2;
		goto out;
	}

	do {
		gpu = etna_gpu_new(dev, core);
		if (!gpu) {
			perror("etna_gpu_new");
			ret = 3;
			goto out_device;
		}

		if (etna_gpu_get_param(gpu, ETNA_GPU_FEATURES_0, &feat)) {
			perror("etna_gpu_get_param");
			ret = 4;
			goto out_device;
		}

		if ((feat & (1 << 9)) == 0) {
			/* GPU not 2D capable. */
			etna_gpu_del(gpu);
			gpu = NULL;
		}

		core++;
	} while (!gpu);

	pipe = etna_pipe_new(gpu, ETNA_PIPE_2D);
	if (!pipe) {
		perror("etna_pipe_new");
		ret = 4;
		goto out_gpu;
	}

	bmp = etna_bo_new(dev, bmp_size, ETNA_BO_UNCACHED);
	if (!bmp) {
		perror("etna_bo_new");
		ret = 5;
		goto out_pipe;
	}
	memset(etna_bo_map(bmp), 0, bmp_size);

	stream = etna_cmd_stream_new(pipe, 0x300, NULL, NULL);
	if (!stream) {
		perror("etna_cmd_stream_new");
		ret = 6;
		goto out_bo;
	}

	/* generate command sequence */
	gen_cmd_stream(stream, bmp, width, height);

	etna_cmd_stream_finish(stream);

	if (argc > 2)
		bmp_dump32(etna_bo_map(bmp), width, height, false, argv[2]);

	if (etna_check_image(etna_bo_map(bmp), width, height))
		ret = 7;

	etna_cmd_stream_del(stream);

out_bo:
    etna_bo_del(bmp);

out_pipe:
	etna_pipe_del(pipe);

out_gpu:
	etna_gpu_del(gpu);

out_device:
	etna_device_del(dev);

out:
	close(fd);

	return ret;
}
