/* xf86drmRandom.c -- "Minimal Standard" PRNG Implementation
 * Created: Mon Apr 19 08:28:13 1999 by faith@precisioninsight.com
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
 * This file contains a simple, straightforward implementation of the Park
 * & Miller "Minimal Standard" PRNG [PM88, PMS93], which is a Lehmer
 * multiplicative linear congruential generator (MLCG) with a period of
 * 2^31-1.
 *
 * This implementation is intended to provide a reliable, portable PRNG
 * that is suitable for testing a hash table implementation and for
 * implementing skip lists.
 *
 * FUTURE ENHANCEMENTS
 *
 * If initial seeds are not selected randomly, two instances of the PRNG
 * can be correlated.  [Knuth81, pp. 32-33] describes a shuffling technique
 * that can eliminate this problem.
 *
 * If PRNGs are used for simulation, the period of the current
 * implementation may be too short.  [LE88] discusses methods of combining
 * MLCGs to produce much longer periods, and suggests some alternative
 * values for A and M.  [LE90 and Sch92] also provide information on
 * long-period PRNGs.
 *
 * REFERENCES
 *
 * [Knuth81] Donald E. Knuth. The Art of Computer Programming.  Volume 2:
 * Seminumerical Algorithms.  Reading, Massachusetts: Addison-Wesley, 1981.
 *
 * [LE88] Pierre L'Ecuyer. "Efficient and Portable Combined Random Number
 * Generators".  CACM 31(6), June 1988, pp. 742-774.
 *
 * [LE90] Pierre L'Ecuyer. "Random Numbers for Simulation". CACM 33(10,
 * October 1990, pp. 85-97.
 *
 * [PM88] Stephen K. Park and Keith W. Miller. "Random Number Generators:
 * Good Ones are Hard to Find". CACM 31(10), October 1988, pp. 1192-1201.
 *
 * [Sch92] Bruce Schneier. "Pseudo-Ransom Sequence Generator for 32-Bit
 * CPUs".  Dr. Dobb's Journal 17(2), February 1992, pp. 34, 37-38, 40.
 *
 * [PMS93] Stephen K. Park, Keith W. Miller, and Paul K. Stockmeyer.  In
 * "Technical Correspondence: Remarks on Choosing and Implementing Random
 * Number Generators". CACM 36(7), July 1993, pp. 105-110.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "libdrm_macros.h"
#include "xf86drm.h"
#include "xf86drmRandom.h"

#define RANDOM_MAGIC 0xfeedbeef

drm_public void *drmRandomCreate(unsigned long seed)
{
    RandomState  *state;

    state           = drmMalloc(sizeof(*state));
    if (!state) return NULL;
    state->magic    = RANDOM_MAGIC;
#if 0
				/* Park & Miller, October 1988 */
    state->a        = 16807;
    state->m        = 2147483647;
    state->check    = 1043618065; /* After 10000 iterations */
#else
				/* Park, Miller, and Stockmeyer, July 1993 */
    state->a        = 48271;
    state->m        = 2147483647;
    state->check    = 399268537; /* After 10000 iterations */
#endif
    state->q        = state->m / state->a;
    state->r        = state->m % state->a;

    state->seed     = seed;
				/* Check for illegal boundary conditions,
                                   and choose closest legal value. */
    if (state->seed <= 0)        state->seed = 1;
    if (state->seed >= state->m) state->seed = state->m - 1;

    return state;
}

drm_public int drmRandomDestroy(void *state)
{
    drmFree(state);
    return 0;
}

drm_public unsigned long drmRandom(void *state)
{
    RandomState   *s = (RandomState *)state;
    unsigned long hi;
    unsigned long lo;

    hi      = s->seed / s->q;
    lo      = s->seed % s->q;
    s->seed = s->a * lo - s->r * hi;
    if ((s->a * lo) <= (s->r * hi)) s->seed += s->m;

    return s->seed;
}

drm_public double drmRandomDouble(void *state)
{
    RandomState *s = (RandomState *)state;
    
    return (double)drmRandom(state)/(double)s->m;
}
