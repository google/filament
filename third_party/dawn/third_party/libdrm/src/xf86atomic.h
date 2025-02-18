/*
 * Copyright © 2009 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

/**
 * @file xf86atomics.h
 *
 * Private definitions for atomic operations
 */

#ifndef LIBDRM_ATOMICS_H
#define LIBDRM_ATOMICS_H

#if HAVE_LIBDRM_ATOMIC_PRIMITIVES

#define HAS_ATOMIC_OPS 1

typedef struct {
	int atomic;
} atomic_t;

# define atomic_read(x) ((x)->atomic)
# define atomic_set(x, val) ((x)->atomic = (val))
# define atomic_inc(x) ((void) __sync_fetch_and_add (&(x)->atomic, 1))
# define atomic_inc_return(x) (__sync_add_and_fetch (&(x)->atomic, 1))
# define atomic_dec_and_test(x) (__sync_add_and_fetch (&(x)->atomic, -1) == 0)
# define atomic_add(x, v) ((void) __sync_add_and_fetch(&(x)->atomic, (v)))
# define atomic_dec(x, v) ((void) __sync_sub_and_fetch(&(x)->atomic, (v)))
# define atomic_cmpxchg(x, oldv, newv) __sync_val_compare_and_swap (&(x)->atomic, oldv, newv)

#endif

#if HAVE_LIB_ATOMIC_OPS
#define AO_REQUIRE_CAS
#include <atomic_ops.h>

#define HAS_ATOMIC_OPS 1

typedef struct {
	AO_t atomic;
} atomic_t;

# define atomic_read(x) AO_load_full(&(x)->atomic)
# define atomic_set(x, val) AO_store_full(&(x)->atomic, (val))
# define atomic_inc(x) ((void) AO_fetch_and_add1_full(&(x)->atomic))
# define atomic_inc_return(x) (AO_fetch_and_add1_full(&(x)->atomic) + 1)
# define atomic_add(x, v) ((void) AO_fetch_and_add_full(&(x)->atomic, (v)))
# define atomic_dec(x, v) ((void) AO_fetch_and_add_full(&(x)->atomic, -(v)))
# define atomic_dec_and_test(x) (AO_fetch_and_sub1_full(&(x)->atomic) == 1)
# define atomic_cmpxchg(x, oldv, newv) AO_compare_and_swap_full(&(x)->atomic, oldv, newv)

#endif

#if (defined(__sun) || defined(__NetBSD__)) && !defined(HAS_ATOMIC_OPS)  /* Solaris & OpenSolaris & NetBSD */

#include <sys/atomic.h>
#define HAS_ATOMIC_OPS 1

#if defined(__NetBSD__)
#define LIBDRM_ATOMIC_TYPE int
#else
#define LIBDRM_ATOMIC_TYPE uint_t
#endif

typedef struct { LIBDRM_ATOMIC_TYPE atomic; } atomic_t;

# define atomic_read(x) (int) ((x)->atomic)
# define atomic_set(x, val) ((x)->atomic = (LIBDRM_ATOMIC_TYPE)(val))
# define atomic_inc(x) (atomic_inc_uint (&(x)->atomic))
# define atomic_inc_return(x) (atomic_inc_uint_nv(&(x)->atomic))
# define atomic_dec_and_test(x) (atomic_dec_uint_nv(&(x)->atomic) == 0)
# define atomic_add(x, v) (atomic_add_int(&(x)->atomic, (v)))
# define atomic_dec(x, v) (atomic_add_int(&(x)->atomic, -(v)))
# define atomic_cmpxchg(x, oldv, newv) atomic_cas_uint (&(x)->atomic, oldv, newv)

#endif

#if !defined(HAS_ATOMIC_OPS)
#error libdrm requires atomic operations, please define them for your CPU/compiler.
#endif

static inline int atomic_add_unless(atomic_t *v, int add, int unless)
{
	int c, old;
	c = atomic_read(v);
	while (c != unless && (old = atomic_cmpxchg(v, c, c + add)) != c)
		c = old;
	return c != unless;
}

#endif
