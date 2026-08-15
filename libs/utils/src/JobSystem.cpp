/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// TODO: Clean-up. We shouldn't need this #ifndef here, but a client has requested that perfetto be
// disabled due to size increase.  In their case, this flag would be defined across targets. Hence
// we guard below with an #ifndef.
#include <cstring>
#ifndef FILAMENT_TRACING_ENABLED
// Note: The overhead of TRACING is not negligible especially with parallel_for().
#define FILAMENT_TRACING_ENABLED false
#endif

// when FILAMENT_TRACING_ENABLED is true, enables even heavier tracing
#define HEAVY_TRACING  0

#include <utils/JobSystem.h>

#include <private/utils/ThreadMap.h>
#include <private/utils/Tracing.h>

#include <utils/algorithm.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Log.h>
#include <utils/Logger.h>
#include <utils/ostream.h>
#include <utils/Panic.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <iterator>
#include <mutex>
#include <random>
#include <thread>

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>


#if defined(WIN32)
#    define NOMINMAX
#    include <windows.h>
# else
#    include <pthread.h>
#endif

#ifdef __ANDROID__
    // see https://developer.android.com/topic/performance/threads#priority
#    include <sys/time.h>
#    include <sys/resource.h>
#    include <unistd.h>
#    ifndef ANDROID_PRIORITY_URGENT_DISPLAY
#        define ANDROID_PRIORITY_URGENT_DISPLAY (-8)
#    endif
#    ifndef ANDROID_PRIORITY_DISPLAY
#        define ANDROID_PRIORITY_DISPLAY (-4)
#    endif
#    ifndef ANDROID_PRIORITY_NORMAL
#        define ANDROID_PRIORITY_NORMAL (0)
#    endif
#    ifndef ANDROID_PRIORITY_BACKGROUND
#        define ANDROID_PRIORITY_BACKGROUND (10)
#    endif
#elif defined(__linux__)
// There is no glibc wrapper for gettid on linux so we need to syscall it.
#    include <unistd.h>
#    include <sys/syscall.h>
#    define gettid() syscall(SYS_gettid)
#endif

#if HEAVY_TRACING
#   define HEAVY_FILAMENT_TRACING_CALL(tag)              FILAMENT_TRACING_CALL(tag)
#   define HEAVY_FILAMENT_TRACING_NAME(tag, name)        FILAMENT_TRACING_NAME(tag, name)
#   define HEAVY_FILAMENT_TRACING_VALUE(tag, name, v)    FILAMENT_TRACING_VALUE(tag, name, v)
#else
#   define HEAVY_FILAMENT_TRACING_CALL(tag)
#   define HEAVY_FILAMENT_TRACING_NAME(tag, name)
#   define HEAVY_FILAMENT_TRACING_VALUE(tag, name, v)
#endif

namespace utils {

void JobSystem::setThreadName(const char* threadName) noexcept {
#if defined(__linux__)
    constexpr size_t MAX_PTHREAD_NAME_LEN = 16;
    char buf[MAX_PTHREAD_NAME_LEN];
    strncpy(buf, threadName, MAX_PTHREAD_NAME_LEN - 1);
    buf[MAX_PTHREAD_NAME_LEN - 1] = '\0';
    pthread_setname_np(pthread_self(), buf);
#elif defined(__APPLE__)
    pthread_setname_np(threadName);
#elif defined(WIN32)
    std::string_view u8name(threadName);
    size_t size = MultiByteToWideChar(CP_UTF8, 0, u8name.data(), u8name.size(), nullptr, 0);

    std::wstring u16name;
    u16name.resize(size);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, u8name.data(), u8name.size(), u16name.data(), u16name.size());
    
    SetThreadDescription(GetCurrentThread(), u16name.data());
#endif
}

void JobSystem::setThreadPriority(Priority const priority) noexcept {
#ifdef __ANDROID__
    int androidPriority = 0;
    switch (priority) {
        case Priority::BACKGROUND:
            androidPriority = ANDROID_PRIORITY_BACKGROUND;
            break;
        case Priority::NORMAL:
            androidPriority = ANDROID_PRIORITY_NORMAL;
            break;
        case Priority::DISPLAY:
            androidPriority = ANDROID_PRIORITY_DISPLAY;
            break;
        case Priority::URGENT_DISPLAY:
            androidPriority = ANDROID_PRIORITY_URGENT_DISPLAY;
            break;
    }
    errno = 0;
    UTILS_UNUSED_IN_RELEASE int error;
    error = setpriority(PRIO_PROCESS, 0, androidPriority);
#ifndef NDEBUG
    if (UTILS_UNLIKELY(error)) {
        slog.w << "setpriority failed: " << strerror(errno) << io::endl;
    }
#endif
#elif defined(__APPLE__)
    qos_class_t qosClass = QOS_CLASS_DEFAULT;
    switch (priority) {
        case Priority::BACKGROUND:
            qosClass = QOS_CLASS_BACKGROUND;
            break;
        case Priority::NORMAL:
            qosClass = QOS_CLASS_DEFAULT;
            break;
        case Priority::DISPLAY:
            qosClass = QOS_CLASS_USER_INTERACTIVE;
            break;
        case Priority::URGENT_DISPLAY:
            qosClass = QOS_CLASS_USER_INTERACTIVE;
            break;
    }
    errno = 0;
    UTILS_UNUSED_IN_RELEASE int error;
    error = pthread_set_qos_class_self_np(qosClass, 0);
#ifndef NDEBUG
    if (UTILS_UNLIKELY(error)) {
        LOG(WARNING) << "pthread_set_qos_class_self_np failed: " << strerror(errno);
    }
#endif
#endif
}

void JobSystem::setThreadAffinityById(size_t id) noexcept {
#if defined(__linux__)
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(id, &set);
    sched_setaffinity(gettid(), sizeof(set), &set);
#endif
}

UTILS_NOINLINE
JobSystem::JobSystem(uint32_t const userThreadCount, uint32_t adoptableThreadsCount) noexcept
    : mJobPool("JobSystem Job pool", MAX_JOB_COUNT * sizeof(Job)),
      mJobStorageBase(static_cast<Job *>(mJobPool.getAllocator().getBase()))
{
    FILAMENT_TRACING_ENABLE(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);

    unsigned int threadPoolCount = userThreadCount;

    if (userThreadCount != SINGLE_THREADED) {
        if (threadPoolCount == 0) {
            // default value, system dependant
            unsigned int hwThreads = std::thread::hardware_concurrency();
            if constexpr (UTILS_HAS_HYPER_THREADING) {
                // For now we avoid using HT, this simplifies profiling.
                // TODO: figure-out what to do with Hyper-threading
                // since we assumed HT, always round-up to an even number of cores (to play it safe)
                hwThreads = (hwThreads + 1) / 2;
            }
            // one of the thread will be the user thread
            threadPoolCount = hwThreads - 1;
        }
        // clamp adoptableThreadsCount so it does not exceed MAX_THREADS
        adoptableThreadsCount = std::min(adoptableThreadsCount, MAX_THREADS);
        // limit the pool such that threadPoolCount + adoptableThreadsCount <= MAX_THREADS
        const uint32_t maxThreadPoolCount = MAX_THREADS - adoptableThreadsCount;
        // make sure we have at least one thread in the thread pool if capacity permits
        threadPoolCount = std::max(maxThreadPoolCount > 0 ? 1u : 0u, threadPoolCount);
        threadPoolCount = std::min(UTILS_HAS_THREADING ? maxThreadPoolCount : 0u, threadPoolCount);
    } else {
        threadPoolCount = 0;
        adoptableThreadsCount = 1;
    }

    mThreadStates = aligned_vector<ThreadState>(threadPoolCount + adoptableThreadsCount);
    mThreadMap = std::make_unique<ThreadMap<ThreadState*>>(threadPoolCount + adoptableThreadsCount);
    mThreadCount = uint16_t(threadPoolCount);
    mParallelSplitCount = uint8_t(std::ceil((std::log2f(threadPoolCount + adoptableThreadsCount))));
    mActiveThreadCount.store(mThreadCount, std::memory_order_relaxed);

    uint32_t adoptableMask = 0;
    for (uint32_t i = 0; i < adoptableThreadsCount; ++i) {
        adoptableMask |= (1u << (threadPoolCount + i));
    }
    mAdoptableSlotsMask.store(adoptableMask, std::memory_order_relaxed);

    static_assert(std::atomic<bool>::is_always_lock_free);
    static_assert(std::atomic<uint32_t>::is_always_lock_free);

    const size_t hardwareThreadCount = mThreadCount;
    auto& states = mThreadStates;

    #pragma nounroll
    for (size_t i = 0, n = states.size(); i < n; i++) {
        // don't use std::random_device because it forces the use of std::string
        constexpr uint32_t base_seed = 1337;
        uint32_t seed = base_seed + static_cast<uint32_t>(i);
        seed = (seed ^ 61) ^ (seed >> 16);
        seed = seed + (seed << 3);
        seed = seed ^ (seed >> 4);
        seed = seed * 0x27d4eb2d;
        seed = seed ^ (seed >> 15);

        auto& state = states[i];
        state.rndGen = default_random_engine(seed);
        state.js = this;
        if (i < hardwareThreadCount) {
            // don't start a thread of adoptable thread slots
            state.thread = std::thread(&JobSystem::loop, this, &state);
        }
    }
}

JobSystem::~JobSystem() {
    requestExit();

    #pragma nounroll
    for (auto &state : mThreadStates) {
        // adopted threads are not joinable
        if (state.thread.joinable()) {
            state.thread.join();
        }
    }
}

inline void JobSystem::incRef(Job const* job) noexcept {
    // no action is taken when incrementing the reference counter, therefore we can safely use
    // memory_order_relaxed.
    UTILS_UNUSED_IN_RELEASE auto const c = job->refCount.fetch_add(1, std::memory_order_relaxed);
    assert_invariant(c < 255);
}

UTILS_NOINLINE
void JobSystem::decRef(Job const* job) noexcept {

    // We must ensure that accesses from other threads happen before deleting the Job.
    // To accomplish this, we need to guarantee that no read/writes are reordered after the
    // dec-ref, because ANOTHER thread could hold the last reference (after us) and that thread
    // needs to see all accesses completed before it deletes the object. This is done
    // with memory_order_release.
    // Similarly, we need to guarantee that no read/write are reordered before the last decref,
    // or some other thread could see a destroyed object before the ref-count is 0. This is done
    // with memory_order_acquire.
    auto const c = job->refCount.fetch_sub(1, std::memory_order_acq_rel);
    assert(c > 0);
    if (c == 1) {
        // This was the last reference, it's safe to destroy the job.
        mJobPool.destroy(job);
    }
}
void JobSystem::requestExit() noexcept {
    mExitRequested.store(true);
    LockGuard const lock(mWaiterLock);
    mWorkCondition.notify_all();
    mJobCondition.notify_all();
}

inline bool JobSystem::exitRequested() const noexcept {
    // memory_order_relaxed is safe because the only action taken is to exit the thread
    return mExitRequested.load(std::memory_order_relaxed);
}

inline bool JobSystem::hasActiveJobs() const noexcept {
    return mActiveJobs.load(std::memory_order_relaxed) > 0;
}

inline bool JobSystem::hasJobCompleted(Job const* job) noexcept {
    return (job->runningJobCount.load(std::memory_order_acquire) & JOB_COUNT_MASK) == 0;
}

inline void JobSystem::waitForWork(UniqueLock& lock) noexcept {
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);
    mSleepingCounts.fetch_add(SLEEPING_WORKER_ONE, std::memory_order_relaxed);
    mWorkCondition.wait(lock);
    mSleepingCounts.fetch_sub(SLEEPING_WORKER_ONE, std::memory_order_relaxed);
}

inline void JobSystem::waitForJob(UniqueLock& lock) noexcept {
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);
    mSleepingCounts.fetch_add(SLEEPING_WAITER_ONE, std::memory_order_relaxed);
    mJobCondition.wait(lock);
    mSleepingCounts.fetch_sub(SLEEPING_WAITER_ONE, std::memory_order_relaxed);
}

inline uint32_t JobSystem::wait(UniqueLock& lock, Job* const job) noexcept {
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);
    // signal we are waiting

    // Note: Do NOT check hasActiveJobs() here to abort waiting. The caller has already
    // executed execute() and found no work to run. If we returned immediately because
    // unrelated active jobs exist in other queues, the caller would loop in a 100% CPU
    // busy-spin without ever sleeping on the condition variable.
    if (exitRequested()) {
        return job->runningJobCount.load(std::memory_order_acquire);
    }

    uint32_t runningJobCount =
            job->runningJobCount.fetch_add(1 << WAITER_COUNT_SHIFT, std::memory_order_relaxed);

    if (runningJobCount & JOB_COUNT_MASK) {
        waitForJob(lock);
    }

    runningJobCount =
            job->runningJobCount.fetch_sub(1 << WAITER_COUNT_SHIFT, std::memory_order_acquire);

    assert_invariant((runningJobCount >> WAITER_COUNT_SHIFT) >= 1);

    return runningJobCount;
}

UTILS_NOINLINE
void JobSystem::wakeWaiters() noexcept {
    // wakeWaiters() is called when a job finishes (to wake up any thread that might be waiting on it)
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);
    mWaiterLock.lock();
    // this empty critical section is needed -- it guarantees that notify_all() happens
    // either before the condition is checked, or after the condition variable sleeps.
    mWaiterLock.unlock();
    // notify_all() can be pretty slow, and it doesn't need to be inside the lock.
    mJobCondition.notify_all();
}

void JobSystem::wakeOne() noexcept {
    // wakeOne() is called when a new job is added to a queue
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);
    mWaiterLock.lock();
    // this empty critical section is needed -- it guarantees that notify_one() happens
    // either before the condition is checked, or after the condition variable sleeps.
    mWaiterLock.unlock();
    // notify_one() can be pretty slow, and it doesn't need to be inside the lock.
    uint32_t const sleeping = mSleepingCounts.load(std::memory_order_relaxed);
    if (sleeping & SLEEPING_WORKER_MASK) {
        mWorkCondition.notify_one();
    } else if (sleeping & SLEEPING_WAITER_MASK) {
        mJobCondition.notify_one();
    }
}

inline JobSystem::ThreadState& JobSystem::getState() {
    ThreadState* const state = mThreadMap->get();
    FILAMENT_CHECK_PRECONDITION(state) << "This thread has not been adopted.";
    return *state;
}

JobSystem::Job* JobSystem::allocateJob() noexcept {
    return mJobPool.make<Job>();
}

void JobSystem::put(ThreadState& state, Job const* job) noexcept {
    assert(job);
    assert(job >= mJobStorageBase && job < mJobStorageBase + MAX_JOB_COUNT);

    uint16_t const index = uint16_t(job - mJobStorageBase + 1);

    // Try to put into the fast 1-slot nextJob cache
    uint16_t const prev = state.nextJob.exchange(index, std::memory_order_release);
    if (UTILS_UNLIKELY(prev != 0)) {
        // If nextJob was already occupied, push the older job into the workQueue
        state.workQueue.push(prev);
    }

    // Increase our active job count. We MUST use std::memory_order_release here.
    // If we used relaxed, the compiler or CPU could reorder this increment to happen 
    // BEFORE nextJob.exchange() or workQueue.push(). If a preemption happened right there,
    // other threads would see mActiveJobs > 0, enter the steal() loop, fail to find the job,
    // and spin infinitely, recreating the exact same priority inversion lockup on the push side!
    mActiveJobs.fetch_add(1, std::memory_order_release);

    // Only wake a sleeping thread if there is at least one thread sleeping (worker or waiter).
    // If all threads are already awake and running, wakeOne() and its internal mWaiterLock are completely bypassed.
    if (UTILS_UNLIKELY(mSleepingCounts.load(std::memory_order_relaxed) > 0)) {
        wakeOne();
    }
}

JobSystem::Job* JobSystem::pop(ThreadState& state) noexcept {
    // 1. First check our 1-slot nextJob continuation bypass
    uint16_t const nextIdx = state.nextJob.exchange(0, std::memory_order_acquire);
    if (UTILS_LIKELY(nextIdx != 0)) {
        mActiveJobs.fetch_sub(1, std::memory_order_relaxed);
        return &mJobStorageBase[nextIdx - 1];
    }

    // 2. Fall back to popping from our workQueue
    // By design, the owner thread must always attempt to pop from its own queue first without
    // inspecting or decrementing mActiveJobs. Popping from our own queue is wait-free and
    // contention-free. Gating pop() on global active job counters would cause owner threads to
    // starve and falsely skip their own runnable jobs if a concurrent stealer has speculatively
    // decremented mActiveJobs.
    size_t const index = state.workQueue.pop();
    assert(index <= MAX_JOB_COUNT);
    Job* const job = !index ? nullptr : &mJobStorageBase[index - 1];
    if (UTILS_LIKELY(job)) {
        mActiveJobs.fetch_sub(1, std::memory_order_relaxed);
    }
    return job;
}

JobSystem::Job* JobSystem::stealFrom(ThreadState& victim) noexcept {
    // 1. Try to steal from victim's workQueue
    size_t const index = victim.workQueue.steal();
    if (index != 0) {
        assert_invariant(index <= MAX_JOB_COUNT);
        return &mJobStorageBase[index - 1];
    }

    // 2. If workQueue is empty, try to steal victim's nextJob
    uint16_t victimNext = victim.nextJob.load(std::memory_order_relaxed);
    if (victimNext != 0) {
        if (victim.nextJob.compare_exchange_strong(victimNext, 0,
                std::memory_order_acquire, std::memory_order_relaxed)) {
            return &mJobStorageBase[victimNext - 1];
        }
    }

    return nullptr;
}

inline JobSystem::ThreadState* JobSystem::getStateToStealFrom(ThreadState& state) noexcept {
    auto& threadStates = mThreadStates;
    uint16_t const threadCount = mActiveThreadCount.load(std::memory_order_relaxed);

    ThreadState* stateToStealFrom = nullptr;

    // don't try to steal from someone else if we're the only thread (infinite loop)
    if (threadCount >= 2) {
        do {
            // This is biased, but frankly, we don't care. It's fast.
            uint16_t const index = uint16_t(state.rndGen() % threadCount);
            assert_invariant(index < threadStates.size());
            stateToStealFrom = &threadStates[index];
            // don't steal from our own queue
        } while (stateToStealFrom == &state);
    }
    return stateToStealFrom;
}

JobSystem::Job* JobSystem::steal(ThreadState& state) noexcept {
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);

    // Fast check: if no jobs are active, avoid attempting to steal or touch atomics.
    if (UTILS_UNLIKELY(!hasActiveJobs())) {
        return nullptr;
    }

    // Speculatively claim one job count before searching across queues.
    // If preempted while stealing or executing, mActiveJobs is already decremented,
    // allowing other threads to sleep rather than spin (preventing priority inversions).
    int32_t const prevActiveJobs = mActiveJobs.fetch_sub(1, std::memory_order_acquire);
    if (UTILS_UNLIKELY(prevActiveJobs <= 0)) {
        // Another thread took the last available job; restore count and exit immediately.
        mActiveJobs.fetch_add(1, std::memory_order_relaxed);
        return nullptr;
    }

    Job* job = nullptr;
    uint16_t const activeThreads = mActiveThreadCount.load(std::memory_order_relaxed);

    // Search queues for the job we claimed, bounded strictly to activeThreads attempts.
    // Note: It is incorrect to check hasActiveJobs() in this loop because:
    // 1) mActiveJobs was already speculatively decremented above (so it may read 0 even if
    //    our claimed job is waiting in another queue).
    // 2) Checking hasActiveJobs() would turn this into an unbounded busy loop whenever jobs
    //    remain in other queues.
    // Bounding strictly to activeThreads guarantees prompt termination while giving a high
    // probability of finding available work.
    size_t attempts = 0;
    do {
        if (ThreadState* const stateToStealFrom = getStateToStealFrom(state)) {
            job = stealFrom(*stateToStealFrom);
        } else {
            break;
        }
        attempts++;
    } while (!job && attempts < activeThreads);

    if (UTILS_UNLIKELY(!job)) {
        // If we couldn't find a job (e.g. consumed concurrently), restore the count.
        mActiveJobs.fetch_add(1, std::memory_order_relaxed);
    }

    return job;
}

bool JobSystem::execute(ThreadState& state) noexcept {
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);

    Job* job = pop(state);

    // It is beneficial for some benchmarks to poll on steal() for a bit, because going back to
    // sleep and waking up is pretty expensive. However, it is unclear it helps in practice with
    // larger jobs or when parallel_for is used.
    constexpr size_t STEAL_TRY_COUNT = 1;
    for (size_t i = 0; UTILS_UNLIKELY(!job && i < STEAL_TRY_COUNT); i++) {
        // our queue is empty, try to steal a job
        job = steal(state);
    }

    if (UTILS_LIKELY(job)) {
        assert((job->runningJobCount.load(std::memory_order_relaxed) & JOB_COUNT_MASK) >= 1);
        if (UTILS_LIKELY(job->function)) {
            HEAVY_FILAMENT_TRACING_NAME(FILAMENT_TRACING_CATEGORY_JOBSYSTEM, "job->function");
            job->id = std::distance(mThreadStates.data(), &state);
            job->function(job->storage, *this, job);
            job->id = invalidThreadId;
        }
        finish(job);
    }
    return job != nullptr;
}

void JobSystem::loop(ThreadState* state) {
    setThreadName("JobSystem::loop");
    setThreadPriority(Priority::DISPLAY);

    // record our work queue
    size_t const index = std::distance(mThreadStates.data(), state);
    mThreadMap->set(uint32_t(index), state);

    // run our main loop...
    do {
        if (!execute(*state)) {
            // Note: Do NOT check hasActiveJobs() here before taking the lock to "double-check".
            // If execute() failed to acquire a job (e.g. random steal probe missed or the owner
            // is descheduled), immediately continuing on hasActiveJobs() creates a tight 100% CPU
            // user-space busy-spin loop. This starves the thread holding the active job, turning
            // transient contention into priority inversion livelocks. Instead, we proceed to
            // take mWaiterLock and sleep in wait(lock).
            UniqueLock lock(mWaiterLock);
            while (!exitRequested() && !hasActiveJobs()) {
                waitForWork(lock);
            }
        }
    } while (!exitRequested());
}

UTILS_NOINLINE
void JobSystem::finish(Job* job) noexcept {
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);

    bool notify = false;

    // terminate this job and notify its parent
    Job* const storage = mJobStorageBase;
    do {
        // std::memory_order_release here is needed to synchronize with JobSystem::wait()
        // which needs to "see" all changes that happened before the job terminated.
        uint32_t const v = job->runningJobCount.fetch_sub(1, std::memory_order_acq_rel);
        uint32_t const runningJobCount = v & JOB_COUNT_MASK;
        assert(runningJobCount > 0);

        if (runningJobCount == 1) {
            // no more work, destroy this job and notify its parent
            uint32_t const waiters = v >> WAITER_COUNT_SHIFT;
            if (waiters) {
                notify = true;
            }
            Job* const parent = job->parent == 0x7FFF ? nullptr : &storage[job->parent];
            decRef(job);
            job = parent;
        } else {
            // there is still work (e.g.: children), we're done.
            break;
        }
    } while (job);

    // wake-up all threads that could potentially be waiting on this job finishing
    if (UTILS_UNLIKELY(notify)) {
        // but avoid calling notify_all() at all cost, because it's always expensive
        wakeWaiters();
    }
}

// -----------------------------------------------------------------------------------------------
// public API...


JobSystem::Job* JobSystem::create(Job* parent, JobFunc const func) noexcept {
    parent = (parent == nullptr) ? mRootJob : parent;
    Job* const job = allocateJob();
    if (UTILS_LIKELY(job)) {
        size_t index = 0x7FFF;
        if (parent) {
            // add a reference to the parent to make sure it can't be terminated.
            // memory_order_relaxed is safe because no action is taken at this point
            // (the job is not started yet).
            UTILS_UNUSED_IN_RELEASE auto const parentJobCount =
                    parent->runningJobCount.fetch_add(1, std::memory_order_relaxed);

            // can't create a child job of a terminated parent
            assert((parentJobCount & JOB_COUNT_MASK) > 0);

            index = parent - mJobStorageBase;
            assert(index < MAX_JOB_COUNT);
        }
        job->function = func;
        job->parent = uint16_t(index);
    }
    return job;
}

void JobSystem::cancel(Job*& job) noexcept {
    finish(job);
    job = nullptr;
}

JobSystem::Job* JobSystem::retain(Job* job) noexcept {
    Job* retained = job;
    incRef(retained);
    return retained;
}

void JobSystem::release(Job*& job) noexcept {
    decRef(job);
    job = nullptr;
}

void JobSystem::run(Job*& job) noexcept {
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);

    ThreadState& state(getState());

    put(state, job);

    // after run() returns, the job is virtually invalid (it'll die on its own)
    job = nullptr;
}

void JobSystem::run(Job*& job, uint8_t const id) noexcept {
    HEAVY_FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);

    ThreadState& state = mThreadStates[id];
    assert_invariant(&state == &getState());

    put(state, job);

    // after run() returns, the job is virtually invalid (it'll die on its own)
    job = nullptr;
}

JobSystem::Job* JobSystem::runAndRetain(Job* job) noexcept {
    Job* retained = retain(job);
    run(job);
    return retained;
}

void JobSystem::waitAndRelease(Job*& job) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);

    assert(job);
    assert(job->refCount.load(std::memory_order_relaxed) >= 1);

    ThreadState& state(getState());
    do {
        if (UTILS_UNLIKELY(!execute(state))) {
            // test if job has completed or exit was requested first, to possibly avoid taking the lock
            if (hasJobCompleted(job) || exitRequested()) {
                break;
            }

            // Note: Do NOT check hasActiveJobs() here to loop again. If execute() found no work,
            // we must proceed to wait on the job's condition variable rather than
            // busy-spinning at 100% CPU.

            // the only way we can be here is if the job we're waiting on it being handled
            // by another thread:
            //    - we returned from execute() which means all queues are empty
            //    - yet our job hasn't completed yet
            //    ergo, it's being run in another thread
            //
            // this could take time however, so we will wait with a condition, and
            // continue to handle more jobs, as they get added.

            UniqueLock lock(mWaiterLock);
            uint32_t const runningJobCount = wait(lock, job);
            // we could be waking up because either:
            // - the job we're waiting on has completed
            // - more jobs where added to the JobSystem
            // - we're asked to exit
            if ((runningJobCount & JOB_COUNT_MASK) == 0 || exitRequested()) {
                break;
            }

            // if we get here, it means that
            // - the job we're waiting on is still running, and
            // - we're not asked to exit, and
            // - there were some active jobs
            // So we try to handle one.
            continue;
        }
    } while (UTILS_LIKELY(!hasJobCompleted(job) && !exitRequested()));

    if (job == mRootJob) {
        mRootJob = nullptr;
    }

    release(job);
}

void JobSystem::runAndWait(Job*& job) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_JOBSYSTEM);
    runAndRetain(job);
    waitAndRelease(job);
}

void JobSystem::adopt() {
    ThreadState const* const state = mThreadMap->get();
    if (state) {
        // we're already part of a JobSystem, do nothing.
        FILAMENT_CHECK_PRECONDITION(this == state->js)
                << "Called adopt on a thread owned by another JobSystem (" << state->js
                << "), this=" << this << "!";
        return;
    }

    uint32_t mask = mAdoptableSlotsMask.load(std::memory_order_relaxed);
    uint32_t bit = 0;
    do {
        FILAMENT_CHECK_POSTCONDITION(mask != 0)
                << "Too many calls to adopt(). No more adoptable threads!";
        bit = 1u << utils::ctz(mask);
    } while (!mAdoptableSlotsMask.compare_exchange_weak(mask, mask & ~bit,
            std::memory_order_acquire, std::memory_order_relaxed));

    size_t const index = utils::ctz(bit);
    FILAMENT_CHECK_POSTCONDITION(index < mThreadStates.size())
            << "Too many calls to adopt(). No more adoptable threads!";

    uint16_t const targetCount = uint16_t(index + 1);
    uint16_t current = mActiveThreadCount.load(std::memory_order_relaxed);
    while (targetCount > current && !mActiveThreadCount.compare_exchange_weak(current, targetCount,
            std::memory_order_relaxed, std::memory_order_relaxed)) {}

    // all threads adopted by the JobSystem need to run at the same priority
    setThreadPriority(Priority::DISPLAY);

    // This thread's queue will be selectable immediately (i.e.: before we set its TLS)
    // however, it's not a problem since mThreadState is pre-initialized and valid
    // (e.g.: the queue is empty).

    mThreadMap->set(uint32_t(index), &mThreadStates[index]);
}

void JobSystem::emancipate() {
    ThreadState const* const state = mThreadMap->get();
    FILAMENT_CHECK_PRECONDITION(state) << "this thread is not an adopted thread";
    FILAMENT_CHECK_PRECONDITION(state->js == this) << "this thread is not adopted by us";
    size_t const index = std::distance(mThreadStates.data(), const_cast<ThreadState*>(state));
    mThreadMap->erase();
    mAdoptableSlotsMask.fetch_or(1u << index, std::memory_order_release);
}

io::ostream& operator<<(io::ostream& out, JobSystem const& js) {
    for (auto const& item : js.mThreadStates) {
        size_t const id = std::distance(js.mThreadStates.data(), &item);
        out << id << ": " << item.workQueue.getCount() << io::endl;
    }
    return out;
}

} // namespace utils
