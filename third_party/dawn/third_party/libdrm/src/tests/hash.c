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

#include "xf86drm.h"
#include "xf86drmHash.h"

#define DIST_LIMIT 10
static int dist[DIST_LIMIT];

static void clear_dist(void) {
    int i;

    for (i = 0; i < DIST_LIMIT; i++)
        dist[i] = 0;
}

static int count_entries(HashBucketPtr bucket)
{
    int count = 0;

    for (; bucket; bucket = bucket->next)
        ++count;
    return count;
}

static void update_dist(int count)
{
    if (count >= DIST_LIMIT)
        ++dist[DIST_LIMIT-1];
    else
        ++dist[count];
}

static void compute_dist(HashTablePtr table)
{
    int           i;
    HashBucketPtr bucket;

    printf("Entries = %ld, hits = %ld, partials = %ld, misses = %ld\n",
          table->entries, table->hits, table->partials, table->misses);
    clear_dist();
    for (i = 0; i < HASH_SIZE; i++) {
        bucket = table->buckets[i];
        update_dist(count_entries(bucket));
    }
    for (i = 0; i < DIST_LIMIT; i++) {
        if (i != DIST_LIMIT-1)
            printf("%5d %10d\n", i, dist[i]);
        else
            printf("other %10d\n", dist[i]);
    }
}

static int check_table(HashTablePtr table,
                       unsigned long key, void * value)
{
    void *retval;
    int   retcode = drmHashLookup(table, key, &retval);

    switch (retcode) {
    case -1:
        printf("Bad magic = 0x%08lx:"
               " key = %lu, expected = %p, returned = %p\n",
               table->magic, key, value, retval);
        break;
    case 1:
        printf("Not found: key = %lu, expected = %p, returned = %p\n",
               key, value, retval);
        break;
    case 0:
        if (value != retval) {
            printf("Bad value: key = %lu, expected = %p, returned = %p\n",
                   key, value, retval);
            retcode = -1;
        }
        break;
    default:
        printf("Bad retcode = %d: key = %lu, expected = %p, returned = %p\n",
               retcode, key, value, retval);
        break;
    }
    return retcode;
}

int main(void)
{
    HashTablePtr  table;
    unsigned long i;
    int           ret = 0;

    printf("\n***** 256 consecutive integers ****\n");
    table = drmHashCreate();
    for (i = 0; i < 256; i++)
        drmHashInsert(table, i, (void *)(i << 16 | i));
    for (i = 0; i < 256; i++)
        ret |= check_table(table, i, (void *)(i << 16 | i));
    compute_dist(table);
    drmHashDestroy(table);

    printf("\n***** 1024 consecutive integers ****\n");
    table = drmHashCreate();
    for (i = 0; i < 1024; i++)
        drmHashInsert(table, i, (void *)(i << 16 | i));
    for (i = 0; i < 1024; i++)
        ret |= check_table(table, i, (void *)(i << 16 | i));
    compute_dist(table);
    drmHashDestroy(table);

    printf("\n***** 1024 consecutive page addresses (4k pages) ****\n");
    table = drmHashCreate();
    for (i = 0; i < 1024; i++)
        drmHashInsert(table, i*4096, (void *)(i << 16 | i));
    for (i = 0; i < 1024; i++)
        ret |= check_table(table, i*4096, (void *)(i << 16 | i));
    compute_dist(table);
    drmHashDestroy(table);

    printf("\n***** 1024 random integers ****\n");
    table = drmHashCreate();
    srandom(0xbeefbeef);
    for (i = 0; i < 1024; i++)
        drmHashInsert(table, random(), (void *)(i << 16 | i));
    srandom(0xbeefbeef);
    for (i = 0; i < 1024; i++)
        ret |= check_table(table, random(), (void *)(i << 16 | i));
    srandom(0xbeefbeef);
    for (i = 0; i < 1024; i++)
        ret |= check_table(table, random(), (void *)(i << 16 | i));
    compute_dist(table);
    drmHashDestroy(table);

    printf("\n***** 5000 random integers ****\n");
    table = drmHashCreate();
    srandom(0xbeefbeef);
    for (i = 0; i < 5000; i++)
        drmHashInsert(table, random(), (void *)(i << 16 | i));
    srandom(0xbeefbeef);
    for (i = 0; i < 5000; i++)
        ret |= check_table(table, random(), (void *)(i << 16 | i));
    srandom(0xbeefbeef);
    for (i = 0; i < 5000; i++)
        ret |= check_table(table, random(), (void *)(i << 16 | i));
    compute_dist(table);
    drmHashDestroy(table);

    return ret;
}
