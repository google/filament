/*
 * Copyright © 2011 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>

#include "libdrm_macros.h"
#include "intel_bufmgr.h"
#include "intel_chipset.h"

#define HW_OFFSET 0x12300000

static void
usage(void)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "  test_decode <batch>\n");
	fprintf(stderr, "  test_decode <batch> -dump\n");
	exit(1);
}

static void
read_file(const char *filename, void **ptr, size_t *size)
{
	int fd, ret;
	struct stat st;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		errx(1, "couldn't open `%s'", filename);

	ret = fstat(fd, &st);
	if (ret)
		errx(1, "couldn't stat `%s'", filename);

	*size = st.st_size;
	*ptr = drm_mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (*ptr == MAP_FAILED)
		errx(1, "couldn't map `%s'", filename);

	close(fd);
}

static void
dump_batch(struct drm_intel_decode *ctx, const char *batch_filename)
{
	void *batch_ptr;
	size_t batch_size;

	read_file(batch_filename, &batch_ptr, &batch_size);

	drm_intel_decode_set_batch_pointer(ctx, batch_ptr, HW_OFFSET,
					   batch_size / 4);
	drm_intel_decode_set_output_file(ctx, stdout);

	drm_intel_decode(ctx);
}

static void
compare_batch(struct drm_intel_decode *ctx, const char *batch_filename)
{
	FILE *out = NULL;
	char *ptr;
	void *ref_ptr, *batch_ptr;
#if HAVE_OPEN_MEMSTREAM
	size_t size;
#endif
	size_t ref_size, batch_size;
	const char *ref_suffix = "-ref.txt";
	char *ref_filename;

	ref_filename = malloc(strlen(batch_filename) + strlen(ref_suffix) + 1);
	sprintf(ref_filename, "%s%s", batch_filename, ref_suffix);

	/* Read the batch and reference. */
	read_file(batch_filename, &batch_ptr, &batch_size);
	read_file(ref_filename, &ref_ptr, &ref_size);

	/* Set up our decode output in memory, because I don't want to
	 * figure out how to output to a file in a safe and sane way
	 * inside of an automake project's test infrastructure.
	 */
#if HAVE_OPEN_MEMSTREAM
	out = open_memstream(&ptr, &size);
#else
	fprintf(stderr, "platform lacks open_memstream, skipping.\n");
	exit(77);
#endif

	drm_intel_decode_set_batch_pointer(ctx, batch_ptr, HW_OFFSET,
					   batch_size / 4);
	drm_intel_decode_set_output_file(ctx, out);

	drm_intel_decode(ctx);

	if (strcmp(ref_ptr, ptr) != 0) {
		fprintf(stderr, "Decode mismatch with reference `%s'.\n",
			ref_filename);
		fprintf(stderr, "You can dump the new output using:\n");
		fprintf(stderr, "  test_decode \"%s\" -dump\n", batch_filename);
		exit(1);
	}

	fclose(out);
	free(ref_filename);
	free(ptr);
}

static uint16_t
infer_devid(const char *batch_filename)
{
	struct {
		const char *name;
		uint16_t devid;
	} chipsets[] = {
		{ "830",  0x3577},
		{ "855",  0x3582},
		{ "945",  0x2772},
		{ "gen4", 0x2a02 },
		{ "gm45", 0x2a42 },
		{ "gen5", PCI_CHIP_ILD_G },
		{ "gen6", PCI_CHIP_SANDYBRIDGE_GT2 },
		{ "gen7", PCI_CHIP_IVYBRIDGE_GT2 },
		{ "gen8", 0x1616 },
		{ NULL, 0 },
	};
	int i;

	for (i = 0; chipsets[i].name != NULL; i++) {
		if (strstr(batch_filename, chipsets[i].name))
			return chipsets[i].devid;
	}

	fprintf(stderr, "Couldn't guess chipset id from batch filename `%s'.\n",
		batch_filename);
	fprintf(stderr, "Must be contain one of:\n");
	for (i = 0; chipsets[i].name != NULL; i++) {
		fprintf(stderr, "  %s\n", chipsets[i].name);
	}
	exit(1);
}

int
main(int argc, char **argv)
{
	uint16_t devid;
	struct drm_intel_decode *ctx;

	if (argc < 2)
		usage();


	devid = infer_devid(argv[1]);

	ctx = drm_intel_decode_context_alloc(devid);

	if (argc == 3) {
		if (strcmp(argv[2], "-dump") == 0)
			dump_batch(ctx, argv[1]);
		else
			usage();
	} else {
		compare_batch(ctx, argv[1]);
	}

	drm_intel_decode_context_free(ctx);

	return 0;
}
