/*
 * Copyright (C) 2016 Etnaviv Project
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

#include "etnaviv_priv.h"
#include "etnaviv_drmif.h"

drm_private void bo_del(struct etna_bo *bo);
drm_private extern pthread_mutex_t table_lock;

static void add_bucket(struct etna_bo_cache *cache, int size)
{
	unsigned i = cache->num_buckets;

	assert(i < ARRAY_SIZE(cache->cache_bucket));

	list_inithead(&cache->cache_bucket[i].list);
	cache->cache_bucket[i].size = size;
	cache->num_buckets++;
}

drm_private void etna_bo_cache_init(struct etna_bo_cache *cache)
{
	unsigned long size, cache_max_size = 64 * 1024 * 1024;

	/* OK, so power of two buckets was too wasteful of memory.
	 * Give 3 other sizes between each power of two, to hopefully
	 * cover things accurately enough.  (The alternative is
	 * probably to just go for exact matching of sizes, and assume
	 * that for things like composited window resize the tiled
	 * width/height alignment and rounding of sizes to pages will
	 * get us useful cache hit rates anyway)
	 */
	add_bucket(cache, 4096);
	add_bucket(cache, 4096 * 2);
	add_bucket(cache, 4096 * 3);

	/* Initialize the linked lists for BO reuse cache. */
	for (size = 4 * 4096; size <= cache_max_size; size *= 2) {
		add_bucket(cache, size);
		add_bucket(cache, size + size * 1 / 4);
		add_bucket(cache, size + size * 2 / 4);
		add_bucket(cache, size + size * 3 / 4);
	}
}

/* Frees older cached buffers.  Called under table_lock */
drm_private void etna_bo_cache_cleanup(struct etna_bo_cache *cache, time_t time)
{
	unsigned i;

	if (cache->time == time)
		return;

	for (i = 0; i < cache->num_buckets; i++) {
		struct etna_bo_bucket *bucket = &cache->cache_bucket[i];
		struct etna_bo *bo;

		while (!LIST_IS_EMPTY(&bucket->list)) {
			bo = LIST_ENTRY(struct etna_bo, bucket->list.next, list);

			/* keep things in cache for at least 1 second: */
			if (time && ((time - bo->free_time) <= 1))
				break;

			list_del(&bo->list);
			bo_del(bo);
		}
	}

	cache->time = time;
}

static struct etna_bo_bucket *get_bucket(struct etna_bo_cache *cache, uint32_t size)
{
	unsigned i;

	/* hmm, this is what intel does, but I suppose we could calculate our
	 * way to the correct bucket size rather than looping..
	 */
	for (i = 0; i < cache->num_buckets; i++) {
		struct etna_bo_bucket *bucket = &cache->cache_bucket[i];
		if (bucket->size >= size) {
			return bucket;
		}
	}

	return NULL;
}

static int is_idle(struct etna_bo *bo)
{
	return etna_bo_cpu_prep(bo,
			DRM_ETNA_PREP_READ |
			DRM_ETNA_PREP_WRITE |
			DRM_ETNA_PREP_NOSYNC) == 0;
}

static struct etna_bo *find_in_bucket(struct etna_bo_bucket *bucket, uint32_t flags)
{
	struct etna_bo *bo = NULL, *tmp;

	pthread_mutex_lock(&table_lock);

	if (LIST_IS_EMPTY(&bucket->list))
		goto out_unlock;

	LIST_FOR_EACH_ENTRY_SAFE(bo, tmp, &bucket->list, list) {
		/* skip BOs with different flags */
		if (bo->flags != flags)
			continue;

		/* check if the first BO with matching flags is idle */
		if (is_idle(bo)) {
			list_delinit(&bo->list);
			goto out_unlock;
		}

		/* If the oldest BO is still busy, don't try younger ones */
		break;
	}

	/* There was no matching buffer found */
	bo = NULL;

out_unlock:
	pthread_mutex_unlock(&table_lock);

	return bo;
}

/* allocate a new (un-tiled) buffer object
 *
 * NOTE: size is potentially rounded up to bucket size
 */
drm_private struct etna_bo *etna_bo_cache_alloc(struct etna_bo_cache *cache, uint32_t *size,
    uint32_t flags)
{
	struct etna_bo *bo;
	struct etna_bo_bucket *bucket;

	*size = ALIGN(*size, 4096);
	bucket = get_bucket(cache, *size);

	/* see if we can be green and recycle: */
	if (bucket) {
		*size = bucket->size;
		bo = find_in_bucket(bucket, flags);
		if (bo) {
			atomic_set(&bo->refcnt, 1);
			etna_device_ref(bo->dev);
			return bo;
		}
	}

	return NULL;
}

drm_private int etna_bo_cache_free(struct etna_bo_cache *cache, struct etna_bo *bo)
{
	struct etna_bo_bucket *bucket = get_bucket(cache, bo->size);

	/* see if we can be green and recycle: */
	if (bucket) {
		struct timespec time;

		clock_gettime(CLOCK_MONOTONIC, &time);

		bo->free_time = time.tv_sec;
		list_addtail(&bo->list, &bucket->list);
		etna_bo_cache_cleanup(cache, time.tv_sec);

		/* bo's in the bucket cache don't have a ref and
		 * don't hold a ref to the dev:
		 */
		etna_device_del_locked(bo->dev);

		return 0;
	}

	return -1;
}
