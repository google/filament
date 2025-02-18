/*
 * Copyright (C) 2015 Etnaviv Project
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

#undef NDEBUG
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "etnaviv_drmif.h"

static void test_avail()
{
	struct etna_cmd_stream *stream;

	printf("testing etna_cmd_stream_avail ... ");

	/* invalid size */
	stream = etna_cmd_stream_new(NULL, 0, NULL, NULL);
	assert(stream == NULL);

	stream = etna_cmd_stream_new(NULL, 4, NULL, NULL);
	assert(stream);
	assert(etna_cmd_stream_avail(stream) == 2);
	etna_cmd_stream_del(stream);

	stream = etna_cmd_stream_new(NULL, 20, NULL, NULL);
	assert(stream);
	assert(etna_cmd_stream_avail(stream) == 18);
	etna_cmd_stream_del(stream);

	/* odd number of 32 bit words */
	stream = etna_cmd_stream_new(NULL, 1, NULL, NULL);
	assert(stream);
	assert(etna_cmd_stream_avail(stream) == 0);
	etna_cmd_stream_del(stream);

	stream = etna_cmd_stream_new(NULL, 23, NULL, NULL);
	assert(stream);
	assert(etna_cmd_stream_avail(stream) == 22);
	etna_cmd_stream_del(stream);

	printf("ok\n");
}

static void test_emit()
{
	struct etna_cmd_stream *stream;

	printf("testing etna_cmd_stream_emit ... ");

	stream = etna_cmd_stream_new(NULL, 6, NULL, NULL);
	assert(stream);
	assert(etna_cmd_stream_avail(stream) == 4);

	etna_cmd_stream_emit(stream, 0x1);
	assert(etna_cmd_stream_avail(stream) == 3);

	etna_cmd_stream_emit(stream, 0x2);
	assert(etna_cmd_stream_avail(stream) == 2);

	etna_cmd_stream_emit(stream, 0x3);
	assert(etna_cmd_stream_avail(stream) == 1);

	etna_cmd_stream_del(stream);

	printf("ok\n");
}

static void test_offset()
{
	struct etna_cmd_stream *stream;

	printf("testing etna_cmd_stream_offset ... ");

	stream = etna_cmd_stream_new(NULL, 6, NULL, NULL);
	assert(etna_cmd_stream_offset(stream) == 0);

	etna_cmd_stream_emit(stream, 0x1);
	assert(etna_cmd_stream_offset(stream) == 1);

	etna_cmd_stream_emit(stream, 0x2);
	assert(etna_cmd_stream_offset(stream) == 2);

	etna_cmd_stream_emit(stream, 0x3);
	etna_cmd_stream_emit(stream, 0x4);
	assert(etna_cmd_stream_offset(stream) == 4);

	etna_cmd_stream_del(stream);

	printf("ok\n");
}

int main(int argc, char *argv[])
{
	test_avail();
	test_emit();
	test_offset();

	return 0;
}
