/* xf86drmHash.c -- Small hash table support for integer -> integer mapping
 * Created: Sun Apr 18 09:35:45 1999 by faith@precisioninsight.com
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
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
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors: Rickard E. (Rik) Faith <faith@valinux.com>
 *
 * DESCRIPTION
 *
 * This file contains a straightforward implementation of a fixed-sized
 * hash table using self-organizing linked lists [Knuth73, pp. 398-399] for
 * collision resolution.  There are two potentially interesting things
 * about this implementation:
 *
 * 1) The table is power-of-two sized.  Prime sized tables are more
 * traditional, but do not have a significant advantage over power-of-two
 * sized table, especially when double hashing is not used for collision
 * resolution.
 *
 * 2) The hash computation uses a table of random integers [Hanson97,
 * pp. 39-41].
 *
 * FUTURE ENHANCEMENTS
 *
 * With a table size of 512, the current implementation is sufficient for a
 * few hundred keys.  Since this is well above the expected size of the
 * tables for which this implementation was designed, the implementation of
 * dynamic hash tables was postponed until the need arises.  A common (and
 * naive) approach to dynamic hash table implementation simply creates a
 * new hash table when necessary, rehashes all the data into the new table,
 * and destroys the old table.  The approach in [Larson88] is superior in
 * two ways: 1) only a portion of the table is expanded when needed,
 * distributing the expansion cost over several insertions, and 2) portions
 * of the table can be locked, enabling a scalable thread-safe
 * implementation.
 *
 * REFERENCES
 *
 * [Hanson97] David R. Hanson.  C Interfaces and Implementations:
 * Techniques for Creating Reusable Software.  Reading, Massachusetts:
 * Addison-Wesley, 1997.
 *
 * [Knuth73] Donald E. Knuth. The Art of Computer Programming.  Volume 3:
 * Sorting and Searching.  Reading, Massachusetts: Addison-Wesley, 1973.
 *
 * [Larson88] Per-Ake Larson. "Dynamic Hash Tables".  CACM 31(4), April
 * 1988, pp. 446-457.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "libdrm_macros.h"
#include "xf86drm.h"
#include "xf86drmHash.h"

#define HASH_MAGIC 0xdeadbeef

static unsigned long HashHash(unsigned long key)
{
    unsigned long        hash = 0;
    unsigned long        tmp  = key;
    static int           init = 0;
    static unsigned long scatter[256];
    int                  i;

    if (!init) {
	void *state;
	state = drmRandomCreate(37);
	for (i = 0; i < 256; i++) scatter[i] = drmRandom(state);
	drmRandomDestroy(state);
	++init;
    }

    while (tmp) {
	hash = (hash << 1) + scatter[tmp & 0xff];
	tmp >>= 8;
    }

    hash %= HASH_SIZE;
    return hash;
}

drm_public void *drmHashCreate(void)
{
    HashTablePtr table;

    table           = drmMalloc(sizeof(*table));
    if (!table) return NULL;
    table->magic    = HASH_MAGIC;

    return table;
}

drm_public int drmHashDestroy(void *t)
{
    HashTablePtr  table = (HashTablePtr)t;
    HashBucketPtr bucket;
    HashBucketPtr next;
    int           i;

    if (table->magic != HASH_MAGIC) return -1; /* Bad magic */

    for (i = 0; i < HASH_SIZE; i++) {
	for (bucket = table->buckets[i]; bucket;) {
	    next = bucket->next;
	    drmFree(bucket);
	    bucket = next;
	}
    }
    drmFree(table);
    return 0;
}

/* Find the bucket and organize the list so that this bucket is at the
   top. */

static HashBucketPtr HashFind(HashTablePtr table,
			      unsigned long key, unsigned long *h)
{
    unsigned long hash = HashHash(key);
    HashBucketPtr prev = NULL;
    HashBucketPtr bucket;

    if (h) *h = hash;

    for (bucket = table->buckets[hash]; bucket; bucket = bucket->next) {
	if (bucket->key == key) {
	    if (prev) {
				/* Organize */
		prev->next           = bucket->next;
		bucket->next         = table->buckets[hash];
		table->buckets[hash] = bucket;
		++table->partials;
	    } else {
		++table->hits;
	    }
	    return bucket;
	}
	prev = bucket;
    }
    ++table->misses;
    return NULL;
}

drm_public int drmHashLookup(void *t, unsigned long key, void **value)
{
    HashTablePtr  table = (HashTablePtr)t;
    HashBucketPtr bucket;

    if (!table || table->magic != HASH_MAGIC) return -1; /* Bad magic */

    bucket = HashFind(table, key, NULL);
    if (!bucket) return 1;	/* Not found */
    *value = bucket->value;
    return 0;			/* Found */
}

drm_public int drmHashInsert(void *t, unsigned long key, void *value)
{
    HashTablePtr  table = (HashTablePtr)t;
    HashBucketPtr bucket;
    unsigned long hash;

    if (table->magic != HASH_MAGIC) return -1; /* Bad magic */

    if (HashFind(table, key, &hash)) return 1; /* Already in table */

    bucket               = drmMalloc(sizeof(*bucket));
    if (!bucket) return -1;	/* Error */
    bucket->key          = key;
    bucket->value        = value;
    bucket->next         = table->buckets[hash];
    table->buckets[hash] = bucket;
    return 0;			/* Added to table */
}

drm_public int drmHashDelete(void *t, unsigned long key)
{
    HashTablePtr  table = (HashTablePtr)t;
    unsigned long hash;
    HashBucketPtr bucket;

    if (table->magic != HASH_MAGIC) return -1; /* Bad magic */

    bucket = HashFind(table, key, &hash);

    if (!bucket) return 1;	/* Not found */

    table->buckets[hash] = bucket->next;
    drmFree(bucket);
    return 0;
}

drm_public int drmHashNext(void *t, unsigned long *key, void **value)
{
    HashTablePtr  table = (HashTablePtr)t;

    while (table->p0 < HASH_SIZE) {
	if (table->p1) {
	    *key       = table->p1->key;
	    *value     = table->p1->value;
	    table->p1  = table->p1->next;
	    return 1;
	}
	table->p1 = table->buckets[table->p0];
	++table->p0;
    }
    return 0;
}

drm_public int drmHashFirst(void *t, unsigned long *key, void **value)
{
    HashTablePtr  table = (HashTablePtr)t;

    if (table->magic != HASH_MAGIC) return -1; /* Bad magic */

    table->p0 = 0;
    table->p1 = table->buckets[0];
    return drmHashNext(table, key, value);
}
