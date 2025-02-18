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

#ifndef ETNAVIV_PRIV_H_
#define ETNAVIV_PRIV_H_

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#include "libdrm_macros.h"
#include "xf86drm.h"
#include "xf86atomic.h"

#include "util_double_list.h"

#include "etnaviv_drmif.h"
#include "etnaviv_drm.h"

struct etna_bo_bucket {
	uint32_t size;
	struct list_head list;
};

struct etna_bo_cache {
	struct etna_bo_bucket cache_bucket[14 * 4];
	unsigned num_buckets;
	time_t time;
};

struct etna_device {
	int fd;
	atomic_t refcnt;

	/* tables to keep track of bo's, to avoid "evil-twin" etna_bo objects:
	 *
	 *   handle_table: maps handle to etna_bo
	 *   name_table: maps flink name to etna_bo
	 *
	 * We end up needing two tables, because DRM_IOCTL_GEM_OPEN always
	 * returns a new handle.  So we need to figure out if the bo is already
	 * open in the process first, before calling gem-open.
	 */
	void *handle_table, *name_table;

	struct etna_bo_cache bo_cache;

	int closefd;        /* call close(fd) upon destruction */
};

drm_private void etna_bo_cache_init(struct etna_bo_cache *cache);
drm_private void etna_bo_cache_cleanup(struct etna_bo_cache *cache, time_t time);
drm_private struct etna_bo *etna_bo_cache_alloc(struct etna_bo_cache *cache,
		uint32_t *size, uint32_t flags);
drm_private int etna_bo_cache_free(struct etna_bo_cache *cache, struct etna_bo *bo);

/* for where @table_lock is already held: */
drm_private void etna_device_del_locked(struct etna_device *dev);

/* a GEM buffer object allocated from the DRM device */
struct etna_bo {
	struct etna_device      *dev;
	void            *map;           /* userspace mmap'ing (if there is one) */
	uint32_t        size;
	uint32_t        handle;
	uint32_t        flags;
	uint32_t        name;           /* flink global handle (DRI2 name) */
	uint64_t        offset;         /* offset to mmap() */
	atomic_t        refcnt;

	/* in the common case, a bo won't be referenced by more than a single
	 * command stream.  So to avoid looping over all the bo's in the
	 * reloc table to find the idx of a bo that might already be in the
	 * table, we cache the idx in the bo.  But in order to detect the
	 * slow-path where bo is ref'd in multiple streams, we also must track
	 * the current_stream for which the idx is valid.  See bo2idx().
	 */
	struct etna_cmd_stream *current_stream;
	uint32_t idx;

	int reuse;
	struct list_head list;   /* bucket-list entry */
	time_t free_time;        /* time when added to bucket-list */
};

struct etna_gpu {
	struct etna_device *dev;
	uint32_t core;
	uint32_t model;
	uint32_t revision;
};

struct etna_pipe {
	enum etna_pipe_id id;
	struct etna_gpu *gpu;
};

struct etna_cmd_stream_priv {
	struct etna_cmd_stream base;
	struct etna_pipe *pipe;

	uint32_t last_timestamp;

	/* submit ioctl related tables: */
	struct {
		/* bo's table: */
		struct drm_etnaviv_gem_submit_bo *bos;
		uint32_t nr_bos, max_bos;

		/* reloc's table: */
		struct drm_etnaviv_gem_submit_reloc *relocs;
		uint32_t nr_relocs, max_relocs;

		/* perf's table: */
		struct drm_etnaviv_gem_submit_pmr *pmrs;
		uint32_t nr_pmrs, max_pmrs;
	} submit;

	/* should have matching entries in submit.bos: */
	struct etna_bo **bos;
	uint32_t nr_bos, max_bos;

	/* notify callback if buffer reset happened */
	void (*reset_notify)(struct etna_cmd_stream *stream, void *priv);
	void *reset_notify_priv;
};

struct etna_perfmon {
	struct list_head domains;
	struct etna_pipe *pipe;
};

struct etna_perfmon_domain
{
	struct list_head head;
	struct list_head signals;
	uint8_t id;
	char name[64];
};

struct etna_perfmon_signal
{
	struct list_head head;
	struct etna_perfmon_domain *domain;
	uint8_t signal;
	char name[64];
};

#define ALIGN(v,a) (((v) + (a) - 1) & ~((a) - 1))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define enable_debug 1  /* TODO make dynamic */

#define INFO_MSG(fmt, ...) \
		do { drmMsg("[I] "fmt " (%s:%d)\n", \
				##__VA_ARGS__, __FUNCTION__, __LINE__); } while (0)
#define DEBUG_MSG(fmt, ...) \
		do if (enable_debug) { drmMsg("[D] "fmt " (%s:%d)\n", \
				##__VA_ARGS__, __FUNCTION__, __LINE__); } while (0)
#define WARN_MSG(fmt, ...) \
		do { drmMsg("[W] "fmt " (%s:%d)\n", \
				##__VA_ARGS__, __FUNCTION__, __LINE__); } while (0)
#define ERROR_MSG(fmt, ...) \
		do { drmMsg("[E] " fmt " (%s:%d)\n", \
				##__VA_ARGS__, __FUNCTION__, __LINE__); } while (0)

#define VOID2U64(x) ((uint64_t)(unsigned long)(x))

static inline void get_abs_timeout(struct drm_etnaviv_timespec *tv, uint64_t ns)
{
	struct timespec t;
	uint32_t s = ns / 1000000000;
	clock_gettime(CLOCK_MONOTONIC, &t);
	tv->tv_sec = t.tv_sec + s;
	tv->tv_nsec = t.tv_nsec + ns - (s * 1000000000);
}

#endif /* ETNAVIV_PRIV_H_ */
