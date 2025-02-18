/*
 * Copyright 2011      Luc Verhaegen <libv@codethink.co.uk>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
/*
 * Quick 'n Dirty bitmap dumper.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "write_bmp.h"

#define FILENAME_SIZE 1024

struct bmp_header {
	unsigned short magic;
	unsigned int size;
	unsigned int unused;
	unsigned int start;
} __attribute__((__packed__));

struct dib_header {
	unsigned int size;
	unsigned int width;
	unsigned int height;
	unsigned short planes;
	unsigned short bpp;
	unsigned int compression;
	unsigned int data_size;
	unsigned int h_res;
	unsigned int v_res;
	unsigned int colours;
	unsigned int important_colours;
	unsigned int red_mask;
	unsigned int green_mask;
	unsigned int blue_mask;
	unsigned int alpha_mask;
	unsigned int colour_space;
	unsigned int unused[12];
} __attribute__((__packed__));

static void
bmp_header_write(int fd, int width, int height, int bgra, int noflip, int alpha)
{
	struct bmp_header bmp_header = {
		.magic = 0x4d42,
		.size = (width * height * 4) +
		sizeof(struct bmp_header) + sizeof(struct dib_header),
		.start = sizeof(struct bmp_header) + sizeof(struct dib_header),
	};
	struct dib_header dib_header = {
		.size = sizeof(struct dib_header),
		.width = width,
		.height = noflip ? -height : height,
		.planes = 1,
		.bpp = 32,
		.compression = 3,
		.data_size = 4 * width * height,
		.h_res = 0xB13,
		.v_res = 0xB13,
		.colours = 0,
		.important_colours = 0,
		.red_mask = 0x000000FF,
		.green_mask = 0x0000FF00,
		.blue_mask = 0x00FF0000,
		.alpha_mask = alpha ? 0xFF000000 : 0x00000000,
		.colour_space = 0x57696E20,
	};

	if (bgra) {
		dib_header.red_mask = 0x00FF0000;
		dib_header.blue_mask = 0x000000FF;
	}

	write(fd, &bmp_header, sizeof(struct bmp_header));
	write(fd, &dib_header, sizeof(struct dib_header));
}

void
bmp_dump32(char *buffer, unsigned width, unsigned height, bool bgra, const char *filename)
{
	int fd;

	fd = open(filename, O_WRONLY| O_TRUNC | O_CREAT, 0666);
	if (fd == -1) {
		printf("Failed to open %s: %s\n", filename, strerror(errno));
		return;
	}

	bmp_header_write(fd, width, height, bgra, false, true);

	write(fd, buffer, width * height * 4);
}

void
bmp_dump32_noflip(char *buffer, unsigned width, unsigned height, bool bgra, const char *filename)
{
	int fd;

	fd = open(filename, O_WRONLY| O_TRUNC | O_CREAT, 0666);
	if (fd == -1) {
		printf("Failed to open %s: %s\n", filename, strerror(errno));
		return;
	}

	bmp_header_write(fd, width, height, bgra, true, true);

	write(fd, buffer, width * height * 4);
}

void
bmp_dump32_ex(char *buffer, unsigned width, unsigned height, bool flip, bool bgra, bool alpha, const char *filename)
{
	int fd;

	fd = open(filename, O_WRONLY| O_TRUNC | O_CREAT, 0666);
	if (fd == -1) {
		printf("Failed to open %s: %s\n", filename, strerror(errno));
		return;
	}

	bmp_header_write(fd, width, height, bgra, flip, alpha);

	write(fd, buffer, width * height * 4);
}
