/* drmsl.c -- Skip list test
 * Created: Mon May 10 09:28:13 1999 by faith@precisioninsight.com
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
 * This file contains a straightforward skip list implementation.n
 *
 * FUTURE ENHANCEMENTS
 *
 * REFERENCES
 *
 * [Pugh90] William Pugh.  Skip Lists: A Probabilistic Alternative to
 * Balanced Trees. CACM 33(6), June 1990, pp. 668-676.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "xf86drm.h"

static void print(void* list)
{
    unsigned long key;
    void          *value;

    if (drmSLFirst(list, &key, &value)) {
	do {
	    printf("key = %5lu, value = %p\n", key, value);
	} while (drmSLNext(list, &key, &value));
    }
}

static double do_time(int size, int iter)
{
    void           *list;
    int            i, j;
    unsigned long  keys[1000000];
    unsigned long  previous;
    unsigned long  key;
    void           *value;
    struct timeval start, stop;
    double         usec;
    void           *ranstate;

    list = drmSLCreate();
    ranstate = drmRandomCreate(12345);

    for (i = 0; i < size; i++) {
	keys[i] = drmRandom(ranstate);
	drmSLInsert(list, keys[i], NULL);
    }

    previous = 0;
    if (drmSLFirst(list, &key, &value)) {
	do {
	    if (key <= previous) {
		printf( "%lu !< %lu\n", previous, key);
	    }
	    previous = key;
	} while (drmSLNext(list, &key, &value));
    }

    gettimeofday(&start, NULL);
    for (j = 0; j < iter; j++) {
	for (i = 0; i < size; i++) {
	    if (drmSLLookup(list, keys[i], &value))
		printf("Error %lu %d\n", keys[i], i);
	}
    }
    gettimeofday(&stop, NULL);

    usec = (double)(stop.tv_sec * 1000000 + stop.tv_usec
		    - start.tv_sec * 1000000 - start.tv_usec) / (size * iter);

    printf("%0.2f microseconds for list length %d\n", usec, size);

    drmRandomDouble(ranstate);
    drmSLDestroy(list);

    return usec;
}

static void print_neighbors(void *list, unsigned long key,
                            unsigned long expected_prev,
                            unsigned long expected_next)
{
    unsigned long prev_key = 0;
    unsigned long next_key = 0;
    void          *prev_value;
    void          *next_value;
    int           retval;

    retval = drmSLLookupNeighbors(list, key,
				  &prev_key, &prev_value,
				  &next_key, &next_value);
    printf("Neighbors of %5lu: %d %5lu %5lu\n",
	   key, retval, prev_key, next_key);
    if (prev_key != expected_prev) {
        fprintf(stderr, "Unexpected neighbor: %5lu. Expected: %5lu\n",
                prev_key, expected_prev);
	exit(1);
    }
    if (next_key != expected_next) {
        fprintf(stderr, "Unexpected neighbor: %5lu. Expected: %5lu\n",
                next_key, expected_next);
	exit(1);
    }
}

int main(void)
{
    void*    list;
    double   usec, usec2, usec3, usec4;

    list = drmSLCreate();
    printf( "list at %p\n", list);

    print(list);
    printf("\n==============================\n\n");

    drmSLInsert(list, 123, NULL);
    drmSLInsert(list, 213, NULL);
    drmSLInsert(list, 50, NULL);
    print(list);
    printf("\n==============================\n\n");

    print_neighbors(list, 0, 0, 50);
    print_neighbors(list, 50, 0, 50);
    print_neighbors(list, 51, 50, 123);
    print_neighbors(list, 123, 50, 123);
    print_neighbors(list, 200, 123, 213);
    print_neighbors(list, 213, 123, 213);
    print_neighbors(list, 256, 213, 256);
    printf("\n==============================\n\n");

    drmSLDelete(list, 50);
    print(list);
    printf("\n==============================\n\n");

    drmSLDump(list);
    drmSLDestroy(list);
    printf("\n==============================\n\n");

    usec  = do_time(100, 10000);
    usec2 = do_time(1000, 500);
    printf("Table size increased by %0.2f, search time increased by %0.2f\n",
	   1000.0/100.0, usec2 / usec);

    usec3 = do_time(10000, 50);
    printf("Table size increased by %0.2f, search time increased by %0.2f\n",
	   10000.0/100.0, usec3 / usec);

    usec4 = do_time(100000, 4);
    printf("Table size increased by %0.2f, search time increased by %0.2f\n",
	   100000.0/100.0, usec4 / usec);

    return 0;
}
