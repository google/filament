/*
 * DRM based mode setting test program
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#include "xf86drm.h"
#include "xf86drmMode.h"

#include "util/common.h"

#include "buffers.h"
#include "cursor.h"

struct cursor {
	int fd;
	uint32_t bo_handle;
	uint32_t crtc_id;
	uint32_t crtc_w, crtc_h;
	uint32_t w, h;

	/* current state */
	uint32_t enabled, x, y;
	int32_t dx, dy;
};

#define MAX_CURSORS 8
static struct cursor cursors[MAX_CURSORS];
static int ncursors;

static pthread_t cursor_thread;
static int cursor_running;

/*
 * Timer driven program loops through these steps to move/enable/disable
 * the cursor
 */

struct cursor_step {
	void (*run)(struct cursor *cursor, const struct cursor_step *step);
	uint32_t msec;
	uint32_t repeat;
	int arg;
};

static uint32_t indx, count;

static void set_cursor(struct cursor *cursor, const struct cursor_step *step)
{
	int enabled = (step->arg ^ count) & 0x1;
	uint32_t handle = 0;

	if (enabled)
		handle = cursor->bo_handle;

	cursor->enabled = enabled;

	drmModeSetCursor(cursor->fd, cursor->crtc_id, handle, cursor->w, cursor->h);
}

static void move_cursor(struct cursor *cursor, const struct cursor_step *step)
{
	int x = cursor->x;
	int y = cursor->y;

	if (!cursor->enabled)
		drmModeSetCursor(cursor->fd, cursor->crtc_id,
				cursor->bo_handle, cursor->w, cursor->h);

	/* calculate new cursor position: */
	x += cursor->dx * step->arg;
	y += cursor->dy * step->arg;

	if (x < 0) {
		x = 0;
		cursor->dx = 1;
	} else if (x > (int)cursor->crtc_w) {
		x = cursor->crtc_w - 1;
		cursor->dx = -1;
	}

	if (y < 0) {
		y = 0;
		cursor->dy = 1;
	} else if (y > (int)cursor->crtc_h) {
		y = cursor->crtc_h - 1;
		cursor->dy = -1;
	}

	cursor->x = x;
	cursor->y = y;

	drmModeMoveCursor(cursor->fd, cursor->crtc_id, x, y);
}

static const struct cursor_step steps[] = {
		{  set_cursor, 10,   0,  1 },  /* enable */
		{ move_cursor,  1, 100,  1 },
		{ move_cursor,  1,  10, 10 },
		{  set_cursor,  1, 100,  0 },  /* disable/enable loop */
		{ move_cursor,  1,  10, 10 },
		{ move_cursor,  9, 100,  1 },
		{ move_cursor, 11, 100,  5 },
		{  set_cursor, 17,  10,  0 },  /* disable/enable loop */
		{ move_cursor,  9, 100,  1 },
		{  set_cursor, 13,  10,  0 },  /* disable/enable loop */
		{ move_cursor,  9, 100,  1 },
		{  set_cursor, 13,  10,  0 },  /* disable/enable loop */
		{  set_cursor, 10,   0,  0 },  /* disable */
};

static void *cursor_thread_func(void *data)
{
	while (cursor_running) {
		const struct cursor_step *step = &steps[indx % ARRAY_SIZE(steps)];
		int i;

		for (i = 0; i < ncursors; i++) {
			struct cursor *cursor = &cursors[i];
			step->run(cursor, step);
		}

		/* iterate to next count/step: */
		if (count < step->repeat) {
			count++;
		} else {
			count = 0;
			indx++;
		}

		usleep(1000 * step->msec);
	}

	return NULL;
}

int cursor_init(int fd, uint32_t bo_handle, uint32_t crtc_id,
		uint32_t crtc_w, uint32_t crtc_h, uint32_t w, uint32_t h)
{
	struct cursor *cursor = &cursors[ncursors];

	assert(ncursors < MAX_CURSORS);

	cursor->fd = fd;
	cursor->bo_handle = bo_handle;
	cursor->crtc_id = crtc_id;
	cursor->crtc_w = crtc_w;
	cursor->crtc_h = crtc_h;
	cursor->w = w;
	cursor->h = h;

	cursor->enabled = 0;
	cursor->x = w/2;
	cursor->y = h/2;
	cursor->dx = 1;
	cursor->dy = 1;

	ncursors++;

	return 0;
}

int cursor_start(void)
{
	cursor_running = 1;
	pthread_create(&cursor_thread, NULL, cursor_thread_func, NULL);
	printf("starting cursor\n");
	return 0;
}

int cursor_stop(void)
{
	cursor_running = 0;
	pthread_join(cursor_thread, NULL);
	printf("cursor stopped\n");
	return 0;
}
