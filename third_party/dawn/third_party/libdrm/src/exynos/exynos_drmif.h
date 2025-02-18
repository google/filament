/*
 * Copyright (C) 2012 Samsung Electronics Co., Ltd.
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
 *    Inki Dae <inki.dae@samsung.com>
 */

#ifndef EXYNOS_DRMIF_H_
#define EXYNOS_DRMIF_H_

#include <xf86drm.h>
#include <stdint.h>
#include "exynos_drm.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct exynos_device {
	int fd;
};

/*
 * Exynos Buffer Object structure.
 *
 * @dev: exynos device object allocated.
 * @handle: a gem handle to gem object created.
 * @flags: indicate memory allocation and cache attribute types.
 * @size: size to the buffer created.
 * @vaddr: user space address to a gem buffer mmapped.
 * @name: a gem global handle from flink request.
 */
struct exynos_bo {
	struct exynos_device	*dev;
	uint32_t		handle;
	uint32_t		flags;
	size_t			size;
	void			*vaddr;
	uint32_t		name;
};

#define EXYNOS_EVENT_CONTEXT_VERSION 1

/*
 * Exynos Event Context structure.
 *
 * @base: base context (for core events).
 * @version: version info similar to the one in 'drmEventContext'.
 * @g2d_event_handler: handler for G2D events.
 */
struct exynos_event_context {
	drmEventContext base;

	int version;

	void (*g2d_event_handler)(int fd, unsigned int cmdlist_no,
							  unsigned int tv_sec, unsigned int tv_usec,
							  void *user_data);
};

/*
 * device related functions:
 */
struct exynos_device * exynos_device_create(int fd);
void exynos_device_destroy(struct exynos_device *dev);

/*
 * buffer-object related functions:
 */
struct exynos_bo * exynos_bo_create(struct exynos_device *dev,
		size_t size, uint32_t flags);
int exynos_bo_get_info(struct exynos_device *dev, uint32_t handle,
			size_t *size, uint32_t *flags);
void exynos_bo_destroy(struct exynos_bo *bo);
struct exynos_bo * exynos_bo_from_name(struct exynos_device *dev, uint32_t name);
int exynos_bo_get_name(struct exynos_bo *bo, uint32_t *name);
uint32_t exynos_bo_handle(struct exynos_bo *bo);
void * exynos_bo_map(struct exynos_bo *bo);
int exynos_prime_handle_to_fd(struct exynos_device *dev, uint32_t handle,
					int *fd);
int exynos_prime_fd_to_handle(struct exynos_device *dev, int fd,
					uint32_t *handle);

/*
 * Virtual Display related functions:
 */
int exynos_vidi_connection(struct exynos_device *dev, uint32_t connect,
				uint32_t ext, void *edid);

/*
 * event handling related functions:
 */
int exynos_handle_event(struct exynos_device *dev,
				struct exynos_event_context *ctx);


#if defined(__cplusplus)
}
#endif

#endif /* EXYNOS_DRMIF_H_ */
