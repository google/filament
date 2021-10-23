/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include <stdio.h>

#include "SDL.h"

/*
  Absolutely basic tests just to see if we get the expected value
  after calling each function.
*/

static
char *
tf(SDL_bool tf)
{
    static char *t = "TRUE";
    static char *f = "FALSE";

    if (tf)
    {
       return t;
    }

    return f;
}

static
void RunBasicTest()
{
    int value;
    SDL_SpinLock lock = 0;

    SDL_atomic_t v;
    SDL_bool tfret = SDL_FALSE;

    SDL_Log("\nspin lock---------------------------------------\n\n");

    SDL_AtomicLock(&lock);
    SDL_Log("AtomicLock                   lock=%d\n", lock);
    SDL_AtomicUnlock(&lock);
    SDL_Log("AtomicUnlock                 lock=%d\n", lock);

    SDL_Log("\natomic -----------------------------------------\n\n");

    SDL_AtomicSet(&v, 0);
    tfret = SDL_AtomicSet(&v, 10) == 0 ? SDL_TRUE : SDL_FALSE;
    SDL_Log("AtomicSet(10)        tfret=%s val=%d\n", tf(tfret), SDL_AtomicGet(&v));
    tfret = SDL_AtomicAdd(&v, 10) == 10 ? SDL_TRUE : SDL_FALSE;
    SDL_Log("AtomicAdd(10)        tfret=%s val=%d\n", tf(tfret), SDL_AtomicGet(&v));

    SDL_AtomicSet(&v, 0);
    SDL_AtomicIncRef(&v);
    tfret = (SDL_AtomicGet(&v) == 1) ? SDL_TRUE : SDL_FALSE;
    SDL_Log("AtomicIncRef()       tfret=%s val=%d\n", tf(tfret), SDL_AtomicGet(&v));
    SDL_AtomicIncRef(&v);
    tfret = (SDL_AtomicGet(&v) == 2) ? SDL_TRUE : SDL_FALSE;
    SDL_Log("AtomicIncRef()       tfret=%s val=%d\n", tf(tfret), SDL_AtomicGet(&v));
    tfret = (SDL_AtomicDecRef(&v) == SDL_FALSE) ? SDL_TRUE : SDL_FALSE;
    SDL_Log("AtomicDecRef()       tfret=%s val=%d\n", tf(tfret), SDL_AtomicGet(&v));
    tfret = (SDL_AtomicDecRef(&v) == SDL_TRUE) ? SDL_TRUE : SDL_FALSE;
    SDL_Log("AtomicDecRef()       tfret=%s val=%d\n", tf(tfret), SDL_AtomicGet(&v));

    SDL_AtomicSet(&v, 10);
    tfret = (SDL_AtomicCAS(&v, 0, 20) == SDL_FALSE) ? SDL_TRUE : SDL_FALSE;
    SDL_Log("AtomicCAS()          tfret=%s val=%d\n", tf(tfret), SDL_AtomicGet(&v));
    value = SDL_AtomicGet(&v);
    tfret = (SDL_AtomicCAS(&v, value, 20) == SDL_TRUE) ? SDL_TRUE : SDL_FALSE;
    SDL_Log("AtomicCAS()          tfret=%s val=%d\n", tf(tfret), SDL_AtomicGet(&v));
}

/**************************************************************************/
/* Atomic operation test
 * Adapted with permission from code by Michael Davidsaver at:
 *  http://bazaar.launchpad.net/~mdavidsaver/epics-base/atomic/revision/12105#src/libCom/test/epicsAtomicTest.c
 * Original copyright 2010 Brookhaven Science Associates as operator of Brookhaven National Lab
 * http://www.aps.anl.gov/epics/license/open.php
 */

/* Tests semantics of atomic operations.  Also a stress test
 * to see if they are really atomic.
 *
 * Several threads adding to the same variable.
 * at the end the value is compared with the expected
 * and with a non-atomic counter.
 */

/* Number of concurrent incrementers */
#define NThreads 2
#define CountInc 100
#define VALBITS (sizeof(atomicValue)*8)

#define atomicValue int
#define CountTo ((atomicValue)((unsigned int)(1<<(VALBITS-1))-1))
#define NInter (CountTo/CountInc/NThreads)
#define Expect (CountTo-NInter*CountInc*NThreads)

enum {
   CountTo_GreaterThanZero = CountTo > 0,
};
SDL_COMPILE_TIME_ASSERT(size, CountTo_GreaterThanZero); /* check for rollover */

static SDL_atomic_t good = { 42 };

static atomicValue bad = 42;

static SDL_atomic_t threadsRunning;

static SDL_sem *threadDone;

static
int SDLCALL adder(void* junk)
{
    unsigned long N=NInter;
    SDL_Log("Thread subtracting %d %lu times\n",CountInc,N);
    while (N--) {
        SDL_AtomicAdd(&good, -CountInc);
        bad-=CountInc;
    }
    SDL_AtomicAdd(&threadsRunning, -1);
    SDL_SemPost(threadDone);
    return 0;
}

static
void runAdder(void)
{
    Uint32 start, end;
    int T=NThreads;

    start = SDL_GetTicks();

    threadDone = SDL_CreateSemaphore(0);

    SDL_AtomicSet(&threadsRunning, NThreads);

    while (T--)
        SDL_CreateThread(adder, "Adder", NULL);

    while (SDL_AtomicGet(&threadsRunning) > 0)
        SDL_SemWait(threadDone);

    SDL_DestroySemaphore(threadDone);

    end = SDL_GetTicks();

    SDL_Log("Finished in %f sec\n", (end - start) / 1000.f);
}

static
void RunEpicTest()
{
    int b;
    atomicValue v;

    SDL_Log("\nepic test---------------------------------------\n\n");

    SDL_Log("Size asserted to be >= 32-bit\n");
    SDL_assert(sizeof(atomicValue)>=4);

    SDL_Log("Check static initializer\n");
    v=SDL_AtomicGet(&good);
    SDL_assert(v==42);

    SDL_assert(bad==42);

    SDL_Log("Test negative values\n");
    SDL_AtomicSet(&good, -5);
    v=SDL_AtomicGet(&good);
    SDL_assert(v==-5);

    SDL_Log("Verify maximum value\n");
    SDL_AtomicSet(&good, CountTo);
    v=SDL_AtomicGet(&good);
    SDL_assert(v==CountTo);

    SDL_Log("Test compare and exchange\n");

    b=SDL_AtomicCAS(&good, 500, 43);
    SDL_assert(!b); /* no swap since CountTo!=500 */
    v=SDL_AtomicGet(&good);
    SDL_assert(v==CountTo); /* ensure no swap */

    b=SDL_AtomicCAS(&good, CountTo, 44);
    SDL_assert(!!b); /* will swap */
    v=SDL_AtomicGet(&good);
    SDL_assert(v==44);

    SDL_Log("Test Add\n");

    v=SDL_AtomicAdd(&good, 1);
    SDL_assert(v==44);
    v=SDL_AtomicGet(&good);
    SDL_assert(v==45);

    v=SDL_AtomicAdd(&good, 10);
    SDL_assert(v==45);
    v=SDL_AtomicGet(&good);
    SDL_assert(v==55);

    SDL_Log("Test Add (Negative values)\n");

    v=SDL_AtomicAdd(&good, -20);
    SDL_assert(v==55);
    v=SDL_AtomicGet(&good);
    SDL_assert(v==35);

    v=SDL_AtomicAdd(&good, -50); /* crossing zero down */
    SDL_assert(v==35);
    v=SDL_AtomicGet(&good);
    SDL_assert(v==-15);

    v=SDL_AtomicAdd(&good, 30); /* crossing zero up */
    SDL_assert(v==-15);
    v=SDL_AtomicGet(&good);
    SDL_assert(v==15);

    SDL_Log("Reset before count down test\n");
    SDL_AtomicSet(&good, CountTo);
    v=SDL_AtomicGet(&good);
    SDL_assert(v==CountTo);

    bad=CountTo;
    SDL_assert(bad==CountTo);

    SDL_Log("Counting down from %d, Expect %d remaining\n",CountTo,Expect);
    runAdder();

    v=SDL_AtomicGet(&good);
    SDL_Log("Atomic %d Non-Atomic %d\n",v,bad);
    SDL_assert(v==Expect);
    SDL_assert(bad!=Expect);
}

/* End atomic operation test */
/**************************************************************************/

/**************************************************************************/
/* Lock-free FIFO test */

/* This is useful to test the impact of another thread locking the queue
   entirely for heavy-weight manipulation.
 */
#define TEST_SPINLOCK_FIFO

#define NUM_READERS 4
#define NUM_WRITERS 4
#define EVENTS_PER_WRITER   1000000

/* The number of entries must be a power of 2 */
#define MAX_ENTRIES 256
#define WRAP_MASK   (MAX_ENTRIES-1)

typedef struct
{
    SDL_atomic_t sequence;
    SDL_Event event;
} SDL_EventQueueEntry;

typedef struct
{
    SDL_EventQueueEntry entries[MAX_ENTRIES];

    char cache_pad1[SDL_CACHELINE_SIZE-((sizeof(SDL_EventQueueEntry)*MAX_ENTRIES)%SDL_CACHELINE_SIZE)];

    SDL_atomic_t enqueue_pos;

    char cache_pad2[SDL_CACHELINE_SIZE-sizeof(SDL_atomic_t)];

    SDL_atomic_t dequeue_pos;

    char cache_pad3[SDL_CACHELINE_SIZE-sizeof(SDL_atomic_t)];

#ifdef TEST_SPINLOCK_FIFO
    SDL_SpinLock lock;
    SDL_atomic_t rwcount;
    SDL_atomic_t watcher;

    char cache_pad4[SDL_CACHELINE_SIZE-sizeof(SDL_SpinLock)-2*sizeof(SDL_atomic_t)];
#endif

    SDL_atomic_t active;

    /* Only needed for the mutex test */
    SDL_mutex *mutex;

} SDL_EventQueue;

static void InitEventQueue(SDL_EventQueue *queue)
{
    int i;

    for (i = 0; i < MAX_ENTRIES; ++i) {
        SDL_AtomicSet(&queue->entries[i].sequence, i);
    }
    SDL_AtomicSet(&queue->enqueue_pos, 0);
    SDL_AtomicSet(&queue->dequeue_pos, 0);
#ifdef TEST_SPINLOCK_FIFO
    queue->lock = 0;
    SDL_AtomicSet(&queue->rwcount, 0);
    SDL_AtomicSet(&queue->watcher, 0);
#endif
    SDL_AtomicSet(&queue->active, 1);
}

static SDL_bool EnqueueEvent_LockFree(SDL_EventQueue *queue, const SDL_Event *event)
{
    SDL_EventQueueEntry *entry;
    unsigned queue_pos;
    unsigned entry_seq;
    int delta;
    SDL_bool status;

#ifdef TEST_SPINLOCK_FIFO
    /* This is a gate so an external thread can lock the queue */
    SDL_AtomicLock(&queue->lock);
    SDL_assert(SDL_AtomicGet(&queue->watcher) == 0);
    SDL_AtomicIncRef(&queue->rwcount);
    SDL_AtomicUnlock(&queue->lock);
#endif

    queue_pos = (unsigned)SDL_AtomicGet(&queue->enqueue_pos);
    for ( ; ; ) {
        entry = &queue->entries[queue_pos & WRAP_MASK];
        entry_seq = (unsigned)SDL_AtomicGet(&entry->sequence);

        delta = (int)(entry_seq - queue_pos);
        if (delta == 0) {
            /* The entry and the queue position match, try to increment the queue position */
            if (SDL_AtomicCAS(&queue->enqueue_pos, (int)queue_pos, (int)(queue_pos+1))) {
                /* We own the object, fill it! */
                entry->event = *event;
                SDL_AtomicSet(&entry->sequence, (int)(queue_pos + 1));
                status = SDL_TRUE;
                break;
            }
        } else if (delta < 0) {
            /* We ran into an old queue entry, which means it still needs to be dequeued */
            status = SDL_FALSE;
            break;
        } else {
            /* We ran into a new queue entry, get the new queue position */
            queue_pos = (unsigned)SDL_AtomicGet(&queue->enqueue_pos);
        }
    }

#ifdef TEST_SPINLOCK_FIFO
    (void) SDL_AtomicDecRef(&queue->rwcount);
#endif
    return status;
}

static SDL_bool DequeueEvent_LockFree(SDL_EventQueue *queue, SDL_Event *event)
{
    SDL_EventQueueEntry *entry;
    unsigned queue_pos;
    unsigned entry_seq;
    int delta;
    SDL_bool status;

#ifdef TEST_SPINLOCK_FIFO
    /* This is a gate so an external thread can lock the queue */
    SDL_AtomicLock(&queue->lock);
    SDL_assert(SDL_AtomicGet(&queue->watcher) == 0);
    SDL_AtomicIncRef(&queue->rwcount);
    SDL_AtomicUnlock(&queue->lock);
#endif

    queue_pos = (unsigned)SDL_AtomicGet(&queue->dequeue_pos);
    for ( ; ; ) {
        entry = &queue->entries[queue_pos & WRAP_MASK];
        entry_seq = (unsigned)SDL_AtomicGet(&entry->sequence);

        delta = (int)(entry_seq - (queue_pos + 1));
        if (delta == 0) {
            /* The entry and the queue position match, try to increment the queue position */
            if (SDL_AtomicCAS(&queue->dequeue_pos, (int)queue_pos, (int)(queue_pos+1))) {
                /* We own the object, fill it! */
                *event = entry->event;
                SDL_AtomicSet(&entry->sequence, (int)(queue_pos+MAX_ENTRIES));
                status = SDL_TRUE;
                break;
            }
        } else if (delta < 0) {
            /* We ran into an old queue entry, which means we've hit empty */
            status = SDL_FALSE;
            break;
        } else {
            /* We ran into a new queue entry, get the new queue position */
            queue_pos = (unsigned)SDL_AtomicGet(&queue->dequeue_pos);
        }
    }

#ifdef TEST_SPINLOCK_FIFO
    (void) SDL_AtomicDecRef(&queue->rwcount);
#endif
    return status;
}

static SDL_bool EnqueueEvent_Mutex(SDL_EventQueue *queue, const SDL_Event *event)
{
    SDL_EventQueueEntry *entry;
    unsigned queue_pos;
    unsigned entry_seq;
    int delta;
    SDL_bool status = SDL_FALSE;

    SDL_LockMutex(queue->mutex);

    queue_pos = (unsigned)queue->enqueue_pos.value;
    entry = &queue->entries[queue_pos & WRAP_MASK];
    entry_seq = (unsigned)entry->sequence.value;

    delta = (int)(entry_seq - queue_pos);
    if (delta == 0) {
        ++queue->enqueue_pos.value;

        /* We own the object, fill it! */
        entry->event = *event;
        entry->sequence.value = (int)(queue_pos + 1);
        status = SDL_TRUE;
    } else if (delta < 0) {
        /* We ran into an old queue entry, which means it still needs to be dequeued */
    } else {
        SDL_Log("ERROR: mutex failed!\n");
    }

    SDL_UnlockMutex(queue->mutex);

    return status;
}

static SDL_bool DequeueEvent_Mutex(SDL_EventQueue *queue, SDL_Event *event)
{
    SDL_EventQueueEntry *entry;
    unsigned queue_pos;
    unsigned entry_seq;
    int delta;
    SDL_bool status = SDL_FALSE;

    SDL_LockMutex(queue->mutex);

    queue_pos = (unsigned)queue->dequeue_pos.value;
    entry = &queue->entries[queue_pos & WRAP_MASK];
    entry_seq = (unsigned)entry->sequence.value;

    delta = (int)(entry_seq - (queue_pos + 1));
    if (delta == 0) {
        ++queue->dequeue_pos.value;

        /* We own the object, fill it! */
        *event = entry->event;
        entry->sequence.value = (int)(queue_pos + MAX_ENTRIES);
        status = SDL_TRUE;
    } else if (delta < 0) {
        /* We ran into an old queue entry, which means we've hit empty */
    } else {
        SDL_Log("ERROR: mutex failed!\n");
    }

    SDL_UnlockMutex(queue->mutex);

    return status;
}

static SDL_sem *writersDone;
static SDL_sem *readersDone;

typedef struct
{
    SDL_EventQueue *queue;
    int index;
    char padding1[SDL_CACHELINE_SIZE-(sizeof(SDL_EventQueue*)+sizeof(int))%SDL_CACHELINE_SIZE];
    int waits;
    SDL_bool lock_free;
    char padding2[SDL_CACHELINE_SIZE-sizeof(int)-sizeof(SDL_bool)];
} WriterData;

typedef struct
{
    SDL_EventQueue *queue;
    int counters[NUM_WRITERS];
    int waits;
    SDL_bool lock_free;
    char padding[SDL_CACHELINE_SIZE-(sizeof(SDL_EventQueue*)+sizeof(int)*NUM_WRITERS+sizeof(int)+sizeof(SDL_bool))%SDL_CACHELINE_SIZE];
} ReaderData;

static int SDLCALL FIFO_Writer(void* _data)
{
    WriterData *data = (WriterData *)_data;
    SDL_EventQueue *queue = data->queue;
    int i;
    SDL_Event event;

    event.type = SDL_USEREVENT;
    event.user.windowID = 0;
    event.user.code = 0;
    event.user.data1 = data;
    event.user.data2 = NULL;

    if (data->lock_free) {
        for (i = 0; i < EVENTS_PER_WRITER; ++i) {
            event.user.code = i;
            while (!EnqueueEvent_LockFree(queue, &event)) {
                ++data->waits;
                SDL_Delay(0);
            }
        }
    } else {
        for (i = 0; i < EVENTS_PER_WRITER; ++i) {
            event.user.code = i;
            while (!EnqueueEvent_Mutex(queue, &event)) {
                ++data->waits;
                SDL_Delay(0);
            }
        }
    }
    SDL_SemPost(writersDone);
    return 0;
}

static int SDLCALL FIFO_Reader(void* _data)
{
    ReaderData *data = (ReaderData *)_data;
    SDL_EventQueue *queue = data->queue;
    SDL_Event event;

    if (data->lock_free) {
        for ( ; ; ) {
            if (DequeueEvent_LockFree(queue, &event)) {
                WriterData *writer = (WriterData*)event.user.data1;
                ++data->counters[writer->index];
            } else if (SDL_AtomicGet(&queue->active)) {
                ++data->waits;
                SDL_Delay(0);
            } else {
                /* We drained the queue, we're done! */
                break;
            }
        }
    } else {
        for ( ; ; ) {
            if (DequeueEvent_Mutex(queue, &event)) {
                WriterData *writer = (WriterData*)event.user.data1;
                ++data->counters[writer->index];
            } else if (SDL_AtomicGet(&queue->active)) {
                ++data->waits;
                SDL_Delay(0);
            } else {
                /* We drained the queue, we're done! */
                break;
            }
        }
    }
    SDL_SemPost(readersDone);
    return 0;
}

#ifdef TEST_SPINLOCK_FIFO
/* This thread periodically locks the queue for no particular reason */
static int SDLCALL FIFO_Watcher(void* _data)
{
    SDL_EventQueue *queue = (SDL_EventQueue *)_data;

    while (SDL_AtomicGet(&queue->active)) {
        SDL_AtomicLock(&queue->lock);
        SDL_AtomicIncRef(&queue->watcher);
        while (SDL_AtomicGet(&queue->rwcount) > 0) {
            SDL_Delay(0);
        }
        /* Do queue manipulation here... */
        (void) SDL_AtomicDecRef(&queue->watcher);
        SDL_AtomicUnlock(&queue->lock);

        /* Wait a bit... */
        SDL_Delay(1);
    }
    return 0;
}
#endif /* TEST_SPINLOCK_FIFO */

static void RunFIFOTest(SDL_bool lock_free)
{
    SDL_EventQueue queue;
    WriterData writerData[NUM_WRITERS];
    ReaderData readerData[NUM_READERS];
    Uint32 start, end;
    int i, j;
    int grand_total;
    char textBuffer[1024];
    size_t len;

    SDL_Log("\nFIFO test---------------------------------------\n\n");
    SDL_Log("Mode: %s\n", lock_free ? "LockFree" : "Mutex");

    readersDone = SDL_CreateSemaphore(0);
    writersDone = SDL_CreateSemaphore(0);

    SDL_memset(&queue, 0xff, sizeof(queue));

    InitEventQueue(&queue);
    if (!lock_free) {
        queue.mutex = SDL_CreateMutex();
    }

    start = SDL_GetTicks();

#ifdef TEST_SPINLOCK_FIFO
    /* Start a monitoring thread */
    if (lock_free) {
        SDL_CreateThread(FIFO_Watcher, "FIFOWatcher", &queue);
    }
#endif

    /* Start the readers first */
    SDL_Log("Starting %d readers\n", NUM_READERS);
    SDL_zeroa(readerData);
    for (i = 0; i < NUM_READERS; ++i) {
        char name[64];
        SDL_snprintf(name, sizeof (name), "FIFOReader%d", i);
        readerData[i].queue = &queue;
        readerData[i].lock_free = lock_free;
        SDL_CreateThread(FIFO_Reader, name, &readerData[i]);
    }

    /* Start up the writers */
    SDL_Log("Starting %d writers\n", NUM_WRITERS);
    SDL_zeroa(writerData);
    for (i = 0; i < NUM_WRITERS; ++i) {
        char name[64];
        SDL_snprintf(name, sizeof (name), "FIFOWriter%d", i);
        writerData[i].queue = &queue;
        writerData[i].index = i;
        writerData[i].lock_free = lock_free;
        SDL_CreateThread(FIFO_Writer, name, &writerData[i]);
    }

    /* Wait for the writers */
    for (i = 0; i < NUM_WRITERS; ++i) {
        SDL_SemWait(writersDone);
    }

    /* Shut down the queue so readers exit */
    SDL_AtomicSet(&queue.active, 0);

    /* Wait for the readers */
    for (i = 0; i < NUM_READERS; ++i) {
        SDL_SemWait(readersDone);
    }

    end = SDL_GetTicks();

    SDL_DestroySemaphore(readersDone);
    SDL_DestroySemaphore(writersDone);

    if (!lock_free) {
        SDL_DestroyMutex(queue.mutex);
    }

    SDL_Log("Finished in %f sec\n", (end - start) / 1000.f);

    SDL_Log("\n");
    for (i = 0; i < NUM_WRITERS; ++i) {
        SDL_Log("Writer %d wrote %d events, had %d waits\n", i, EVENTS_PER_WRITER, writerData[i].waits);
    }
    SDL_Log("Writers wrote %d total events\n", NUM_WRITERS*EVENTS_PER_WRITER);

    /* Print a breakdown of which readers read messages from which writer */
    SDL_Log("\n");
    grand_total = 0;
    for (i = 0; i < NUM_READERS; ++i) {
        int total = 0;
        for (j = 0; j < NUM_WRITERS; ++j) {
            total += readerData[i].counters[j];
        }
        grand_total += total;
        SDL_Log("Reader %d read %d events, had %d waits\n", i, total, readerData[i].waits);
        SDL_snprintf(textBuffer, sizeof(textBuffer), "  { ");
        for (j = 0; j < NUM_WRITERS; ++j) {
            if (j > 0) {
                len = SDL_strlen(textBuffer);
                SDL_snprintf(textBuffer + len, sizeof(textBuffer) - len, ", ");
            }
            len = SDL_strlen(textBuffer);
            SDL_snprintf(textBuffer + len, sizeof(textBuffer) - len, "%d", readerData[i].counters[j]);
        }
        len = SDL_strlen(textBuffer);
        SDL_snprintf(textBuffer + len, sizeof(textBuffer) - len, " }\n");
        SDL_Log("%s", textBuffer);
    }
    SDL_Log("Readers read %d total events\n", grand_total);
}

/* End FIFO test */
/**************************************************************************/

int
main(int argc, char *argv[])
{
    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    RunBasicTest();
    RunEpicTest();
/* This test is really slow, so don't run it by default */
#if 0
    RunFIFOTest(SDL_FALSE);
#endif
    RunFIFOTest(SDL_TRUE);
    return 0;
}

/* vi: set ts=4 sw=4 expandtab: */
