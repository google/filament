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

#ifndef FREEDRENO_DRMIF_H_
#define FREEDRENO_DRMIF_H_

#include <xf86drm.h>
#include <stdint.h>

#if defined(__GNUC__)
#  define drm_deprecated __attribute__((__deprecated__))
#else
#  define drm_deprecated
#endif

/* an empty marker for things that will be deprecated in the future: */
#define will_be_deprecated

struct fd_bo;
struct fd_pipe;
struct fd_device;

enum fd_pipe_id {
	FD_PIPE_3D = 1,
	FD_PIPE_2D = 2,
	/* some devices have two 2d blocks.. not really sure how to
	 * use that yet, so just ignoring the 2nd 2d pipe for now
	 */
	FD_PIPE_MAX
};

enum fd_param_id {
	FD_DEVICE_ID,
	FD_GMEM_SIZE,
	FD_GPU_ID,
	FD_CHIP_ID,
	FD_MAX_FREQ,
	FD_TIMESTAMP,
	FD_NR_RINGS,      /* # of rings == # of distinct priority levels */
};

/* bo flags: */
#define DRM_FREEDRENO_GEM_TYPE_SMI        0x00000001
#define DRM_FREEDRENO_GEM_TYPE_KMEM       0x00000002
#define DRM_FREEDRENO_GEM_TYPE_MEM_MASK   0x0000000f
#define DRM_FREEDRENO_GEM_CACHE_NONE      0x00000000
#define DRM_FREEDRENO_GEM_CACHE_WCOMBINE  0x00100000
#define DRM_FREEDRENO_GEM_CACHE_WTHROUGH  0x00200000
#define DRM_FREEDRENO_GEM_CACHE_WBACK     0x00400000
#define DRM_FREEDRENO_GEM_CACHE_WBACKWA   0x00800000
#define DRM_FREEDRENO_GEM_CACHE_MASK      0x00f00000
#define DRM_FREEDRENO_GEM_GPUREADONLY     0x01000000

/* bo access flags: (keep aligned to MSM_PREP_x) */
#define DRM_FREEDRENO_PREP_READ           0x01
#define DRM_FREEDRENO_PREP_WRITE          0x02
#define DRM_FREEDRENO_PREP_NOSYNC         0x04

/* device functions:
 */

struct fd_device * fd_device_new(int fd);
struct fd_device * fd_device_new_dup(int fd);
struct fd_device * fd_device_ref(struct fd_device *dev);
void fd_device_del(struct fd_device *dev);
int fd_device_fd(struct fd_device *dev);

enum fd_version {
	FD_VERSION_MADVISE = 1,            /* kernel supports madvise */
	FD_VERSION_UNLIMITED_CMDS = 1,     /* submits w/ >4 cmd buffers (growable ringbuffer) */
	FD_VERSION_FENCE_FD = 2,           /* submit command supports in/out fences */
	FD_VERSION_SUBMIT_QUEUES = 3,      /* submit queues and multiple priority levels */
	FD_VERSION_BO_IOVA = 3,            /* supports fd_bo_get/put_iova() */
};
enum fd_version fd_device_version(struct fd_device *dev);

/* pipe functions:
 */

struct fd_pipe * fd_pipe_new(struct fd_device *dev, enum fd_pipe_id id);
struct fd_pipe * fd_pipe_new2(struct fd_device *dev, enum fd_pipe_id id, uint32_t prio);
struct fd_pipe * fd_pipe_ref(struct fd_pipe *pipe);
void fd_pipe_del(struct fd_pipe *pipe);
int fd_pipe_get_param(struct fd_pipe *pipe, enum fd_param_id param,
		uint64_t *value);
int fd_pipe_wait(struct fd_pipe *pipe, uint32_t timestamp);
/* timeout in nanosec */
int fd_pipe_wait_timeout(struct fd_pipe *pipe, uint32_t timestamp,
		uint64_t timeout);


/* buffer-object functions:
 */

struct fd_bo * fd_bo_new(struct fd_device *dev,
		uint32_t size, uint32_t flags);
struct fd_bo * fd_bo_from_fbdev(struct fd_pipe *pipe,
		int fbfd, uint32_t size);
struct fd_bo *fd_bo_from_handle(struct fd_device *dev,
		uint32_t handle, uint32_t size);
struct fd_bo * fd_bo_from_name(struct fd_device *dev, uint32_t name);
struct fd_bo * fd_bo_from_dmabuf(struct fd_device *dev, int fd);
uint64_t fd_bo_get_iova(struct fd_bo *bo);
void fd_bo_put_iova(struct fd_bo *bo);
struct fd_bo * fd_bo_ref(struct fd_bo *bo);
void fd_bo_del(struct fd_bo *bo);
int fd_bo_get_name(struct fd_bo *bo, uint32_t *name);
uint32_t fd_bo_handle(struct fd_bo *bo);
int fd_bo_dmabuf(struct fd_bo *bo);
uint32_t fd_bo_size(struct fd_bo *bo);
void * fd_bo_map(struct fd_bo *bo);
int fd_bo_cpu_prep(struct fd_bo *bo, struct fd_pipe *pipe, uint32_t op);
void fd_bo_cpu_fini(struct fd_bo *bo);

#endif /* FREEDRENO_DRMIF_H_ */
