/*
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
 */

#define HASH_SIZE  512		/* Good for about 100 entries */
				/* If you change this value, you probably
                                   have to change the HashHash hashing
                                   function! */

typedef struct HashBucket {
    unsigned long     key;
    void              *value;
    struct HashBucket *next;
} HashBucket, *HashBucketPtr;

typedef struct HashTable {
    unsigned long    magic;
    unsigned long    entries;
    unsigned long    hits;	/* At top of linked list */
    unsigned long    partials;	/* Not at top of linked list */
    unsigned long    misses;	/* Not in table */
    HashBucketPtr    buckets[HASH_SIZE];
    int              p0;
    HashBucketPtr    p1;
} HashTable, *HashTablePtr;
