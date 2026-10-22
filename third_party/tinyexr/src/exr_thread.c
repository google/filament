/*
 * TinyEXR - optional C11-threads worker pool for per-block parallel codec work.
 *
 * Gated by EXR_USE_THREADS (default off). When enabled, exr_parallel_for spawns
 * an ephemeral set of workers that pull jobs from a shared, mutex-protected
 * counter; the calling thread participates too. The workers come from C11
 * <threads.h> where available, or Grand Central Dispatch on macOS/Apple
 * platforms (which do not ship <threads.h>). When disabled (the default, and
 * always for freestanding builds) it runs the jobs serially and pulls in no
 * threading headers.
 *
 * The thread-count setter/getter are always defined so the public API exists in
 * every build; they simply have no effect when threads are compiled out.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_internal.h"

/* Process-global worker count (always present; >1 only matters with threads). */
static int g_num_threads = 1;

void exr_set_num_threads(int n) { g_num_threads = (n < 1) ? 1 : n; }
int exr_get_num_threads(void) { return g_num_threads; }

#if defined(EXR_USE_THREADS) && defined(__APPLE__)

/* Apple platforms do not ship C11 <threads.h>; use Grand Central Dispatch and
 * os_unfair_lock, both native (in libSystem, no extra link flags). We spawn
 * nthreads-1 GCD workers via dispatch_group_async_f (a C function pointer, so
 * no Objective-C blocks / -fblocks needed); the calling thread is the nth
 * worker. All of them drain the same shared, lock-guarded job counter, exactly
 * like the C11 path below. */
#include <dispatch/dispatch.h>
#include <os/lock.h>

typedef struct {
    exr_par_fn fn;
    void *ctx;
    int njobs;
    int next; /* next unclaimed job index, guarded by lock */
    os_unfair_lock lock;
} par_state;

static void par_worker(void *arg) {
    par_state *st = (par_state *)arg;
    for (;;) {
        int job;
        os_unfair_lock_lock(&st->lock);
        job = (st->next < st->njobs) ? st->next++ : -1;
        os_unfair_lock_unlock(&st->lock);
        if (job < 0) break;
        st->fn(st->ctx, job);
    }
}

void exr_parallel_for(int nthreads, int njobs, exr_par_fn fn, void *ctx) {
    par_state st;
    dispatch_queue_t q;
    dispatch_group_t grp;
    int i;

    if (njobs <= 0) return;
    if (nthreads > njobs) nthreads = njobs;
    if (nthreads <= 1) { /* serial */
        for (i = 0; i < njobs; ++i) fn(ctx, i);
        return;
    }

    st.fn = fn;
    st.ctx = ctx;
    st.njobs = njobs;
    st.next = 0;
    st.lock = (os_unfair_lock)OS_UNFAIR_LOCK_INIT;

    /* st lives on this stack frame; dispatch_group_wait below keeps it alive
     * for the whole worker lifetime, so capturing &st is safe. */
    q = dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0);
    grp = dispatch_group_create();
    for (i = 0; i < nthreads - 1; ++i)
        dispatch_group_async_f(grp, q, &st, par_worker);

    par_worker(&st); /* calling thread participates */

    dispatch_group_wait(grp, DISPATCH_TIME_FOREVER);
    dispatch_release(grp);
}

#elif defined(EXR_USE_THREADS)

#include <threads.h>

/* Upper bound on workers, so the thread handle array can live on the stack and
 * a pathological count cannot exhaust resources. */
#define EXR_THREAD_CAP 64

typedef struct {
    exr_par_fn fn;
    void *ctx;
    int njobs;
    int next; /* next unclaimed job index, guarded by lock */
    mtx_t lock;
} par_state;

static int par_worker(void *arg) {
    par_state *st = (par_state *)arg;
    for (;;) {
        int job;
        mtx_lock(&st->lock);
        job = (st->next < st->njobs) ? st->next++ : -1;
        mtx_unlock(&st->lock);
        if (job < 0) break;
        st->fn(st->ctx, job);
    }
    return 0;
}

void exr_parallel_for(int nthreads, int njobs, exr_par_fn fn, void *ctx) {
    par_state st;
    thrd_t threads[EXR_THREAD_CAP];
    int spawned = 0, i;

    if (njobs <= 0) return;
    if (nthreads > njobs) nthreads = njobs;
    if (nthreads > EXR_THREAD_CAP) nthreads = EXR_THREAD_CAP;
    if (nthreads <= 1) { /* serial */
        for (i = 0; i < njobs; ++i) fn(ctx, i);
        return;
    }

    st.fn = fn;
    st.ctx = ctx;
    st.njobs = njobs;
    st.next = 0;
    if (mtx_init(&st.lock, mtx_plain) != thrd_success) {
        for (i = 0; i < njobs; ++i) fn(ctx, i); /* fallback: serial */
        return;
    }

    /* Spawn up to nthreads-1 helpers; the calling thread is the nth worker.
     * If a spawn fails we simply have fewer workers - correctness is unaffected
     * because every worker drains the same shared job counter. */
    for (i = 0; i < nthreads - 1; ++i) {
        if (thrd_create(&threads[spawned], par_worker, &st) == thrd_success)
            ++spawned;
    }

    par_worker(&st); /* calling thread participates */

    for (i = 0; i < spawned; ++i) thrd_join(threads[i], NULL);
    mtx_destroy(&st.lock);
}

#else /* !EXR_USE_THREADS : serial, no <threads.h> (freestanding-safe) */

void exr_parallel_for(int nthreads, int njobs, exr_par_fn fn, void *ctx) {
    int i;
    (void)nthreads;
    for (i = 0; i < njobs; ++i) fn(ctx, i);
}

#endif /* EXR_USE_THREADS */
