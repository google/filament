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

// Note: The overhead of SYSTRACE_TAG_JOBSYSTEM is not negligible especially with parallel_for().
#ifndef SYSTRACE_TAG
//#define SYSTRACE_TAG SYSTRACE_TAG_JOBSYSTEM
#define SYSTRACE_TAG SYSTRACE_TAG_NEVER
#endif

// when SYSTRACE_TAG_JOBSYSTEM is used, enables even heavier systraces
#define HEAVY_SYSTRACE  0

// enable for catching hangs waiting on a job to finish
static constexpr bool DEBUG_FINISH_HANGS = false;

#include <utils/JobSystem.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Log.h>
#include <utils/ostream.h>
#include <utils/Panic.h>
#include <utils/Systrace.h>

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
#    include <string>
# else
#    include <pthread.h>
#endif

#ifdef __ANDROID__
    // see https://developer.android.com/topic/performance/threads#priority
#    include <sys/time.h>
#    include <sys/resource.h>
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

#if HEAVY_SYSTRACE
#   define HEAVY_SYSTRACE_CALL()            SYSTRACE_CALL()
#   define HEAVY_SYSTRACE_NAME(name)        SYSTRACE_NAME(name)
#   define HEAVY_SYSTRACE_VALUE32(name, v)  SYSTRACE_VALUE32(name, v)
#else
#   define HEAVY_SYSTRACE_CALL()
#   define HEAVY_SYSTRACE_NAME(name)
#   define HEAVY_SYSTRACE_VALUE32(name, v)
#endif

namespace utils {

void JobSystem::setThreadName(const char* name) noexcept {
#if defined(__linux__)
    pthread_setname_np(pthread_self(), name);
#elif defined(__APPLE__)
    pthread_setname_np(name);
#elif defined(WIN32)
    std::string_view u8name(name);
    size_t size = MultiByteToWideChar(CP_UTF8, 0, u8name.data(), u8name.size(), nullptr, 0);

    std::wstring u16name;
    u16name.resize(size);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, u8name.data(), u8name.size(), u16name.data(), u16name.size());
    
    SetThreadDescription(GetCurrentThread(), u16name.data());
#endif
}

void JobSystem::setThreadPriority(Priority priority) noexcept {
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

JobSystem::JobSystem(const size_t userThreadCount, const size_t adoptableThreadsCount) noexcept
    : mJobPool("JobSystem Job pool", MAX_JOB_COUNT * sizeof(Job)),
      mJobStorageBase(static_cast<Job *>(mJobPool.getAllocator().getCurrent()))
{
    SYSTRACE_ENABLE();

    unsigned int threadPoolCount = userThreadCount;
    if (threadPoolCount == 0) {
        // default value, system dependant
        unsigned int hwThreads = std::thread::hardware_concurrency();
        if (UTILS_HAS_HYPER_THREADING) {
            // For now we avoid using HT, this simplifies profiling.
            // TODO: figure-out what to do with Hyper-threading
            // since we assumed HT, always round-up to an even number of cores (to play it safe)
            hwThreads = (hwThreads + 1) / 2;
        }
        // one of the thread will be the user thread
        threadPoolCount = hwThreads - 1;
    }
    // make sure we have at least one thread in the thread pool
    threadPoolCount = std::max(1u, threadPoolCount);
    // and also limit the pool to 32 threads
    threadPoolCount = std::min(UTILS_HAS_THREADING ? 32u : 0u, threadPoolCount);

    mThreadStates = aligned_vector<ThreadState>(threadPoolCount + adoptableThreadsCount);
    mThreadCount = uint16_t(threadPoolCount);
    mParallelSplitCount = (uint8_t)std::ceil((std::log2f(threadPoolCount + adoptableThreadsCount)));

    static_assert(std::atomic<bool>::is_always_lock_free);
    static_assert(std::atomic<uint16_t>::is_always_lock_free);

    std::random_device rd;
    const size_t hardwareThreadCount = mThreadCount;
    auto& states = mThreadStates;

    #pragma nounroll
    for (size_t i = 0, n = states.size(); i < n; i++) {
        auto& state = states[i];
        state.rndGen = default_random_engine(rd());
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
    job->refCount.fetch_add(1, std::memory_order_relaxed);
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
    auto c = job->refCount.fetch_sub(1, std::memory_order_acq_rel);
    assert(c > 0);
    if (c == 1) {
        // This was the last reference, it's safe to destroy the job.
        mJobPool.destroy(job);
    }
}

void JobSystem::requestExit() noexcept {
    mExitRequested.store(true);
    std::lock_guard<Mutex> const lock(mWaiterLock);
    mWaiterCondition.notify_all();
}

inline bool JobSystem::exitRequested() const noexcept {
    // memory_order_relaxed is safe because the only action taken is to exit the thread
    return mExitRequested.load(std::memory_order_relaxed);
}

inline bool JobSystem::hasActiveJobs() const noexcept {
    return mActiveJobs.load(std::memory_order_relaxed) > 0;
}

inline bool JobSystem::hasJobCompleted(JobSystem::Job const* job) noexcept {
    return job->runningJobCount.load(std::memory_order_acquire) <= 0;
}

void JobSystem::wait(std::unique_lock<Mutex>& lock, Job* job) noexcept {
    HEAVY_SYSTRACE_CALL();
    if constexpr (!DEBUG_FINISH_HANGS) {
        mWaiterCondition.wait(lock);
    } else {
        do {
            // we use a pretty long timeout (4s) so we're very confident that the system is hung
            // and nothing else is happening.
            std::cv_status status = mWaiterCondition.wait_for(lock,
                    std::chrono::milliseconds(4000));
            if (status == std::cv_status::no_timeout) {
                break;
            }

            // hang debugging...

            // We check of we had active jobs or if the job we're waiting on had completed already.
            // There is the possibility of a race condition, but our long timeout gives us some
            // confidence that we're in an incorrect state.

            size_t const id = std::distance(mThreadStates.data(), &getState());
            auto activeJobs = mActiveJobs.load();

            if (job) {
                auto runningJobCount = job->runningJobCount.load();
                FILAMENT_CHECK_POSTCONDITION(runningJobCount > 0)
                        << "JobSystem(" << this << ", " << unsigned(id) << "): waiting while job "
                        << job << " has completed and " << activeJobs << " jobs are active!";
            }

            FILAMENT_CHECK_POSTCONDITION(activeJobs <= 0)
                    << "JobSystem(" << this << ", " << unsigned(id) << "): waiting while "
                    << activeJobs << " jobs are active!";

        } while (true);
    }
}

void JobSystem::wakeAll() noexcept {
    // wakeAll() is called when a job finishes (to wake up any thread that might be waiting on it)
    HEAVY_SYSTRACE_CALL();
    mWaiterLock.lock();
    // this empty critical section is needed -- it guarantees that notify_all() happens
    // after the condition's variables are set.
    mWaiterLock.unlock();
    // notify_all() can be pretty slow, and it doesn't need to be inside the lock.
    mWaiterCondition.notify_all();
}

void JobSystem::wakeOne() noexcept {
    // wakeOne() is called when a new job is added to a queue
    HEAVY_SYSTRACE_CALL();
    mWaiterLock.lock();
    // this empty critical section is needed -- it guarantees that notify_one() happens
    // after the condition's variables are set.
    mWaiterLock.unlock();
    // notify_one() can be pretty slow, and it doesn't need to be inside the lock.
    mWaiterCondition.notify_one();
}

inline JobSystem::ThreadState& JobSystem::getState() noexcept {
    std::lock_guard<Mutex> const lock(mThreadMapLock);
    auto iter = mThreadMap.find(std::this_thread::get_id());
    FILAMENT_CHECK_PRECONDITION(iter != mThreadMap.end()) << "This thread has not been adopted.";
    return *iter->second;
}

JobSystem::Job* JobSystem::allocateJob() noexcept {
    return mJobPool.make<Job>();
}

void JobSystem::put(WorkQueue& workQueue, Job* job) noexcept {
    assert(job);
    size_t const index = job - mJobStorageBase;
    assert(index >= 0 && index < MAX_JOB_COUNT);

    // put the job into the queue first
    workQueue.push(uint16_t(index + 1));
    // then increase our active job count
    int32_t const oldActiveJobs = mActiveJobs.fetch_add(1, std::memory_order_relaxed);
    // But it's possible that the job has already been picked-up, so oldActiveJobs could be
    // negative for instance. We signal only if that's not the case.
    if (oldActiveJobs >= 0) {
        wakeOne(); // wake-up a thread if needed...
    }
}

JobSystem::Job* JobSystem::pop(WorkQueue& workQueue) noexcept {
    // decrement mActiveJobs first, this is to ensure that if there is only a single job left
    // (and we're about to pick it up), other threads don't loop trying to do the same.
    mActiveJobs.fetch_sub(1, std::memory_order_relaxed);

    size_t const index = workQueue.pop();
    assert(index <= MAX_JOB_COUNT);
    Job* const job = !index ? nullptr : &mJobStorageBase[index - 1];

    // If our guess was wrong, i.e. we couldn't pick up a job (b/c our queue was empty), we
    // need to correct mActiveJobs.
    if (!job) {
        // no need to wake someone else up because, we will go into job-stealing mode
        // immediately after this
        mActiveJobs.fetch_add(1, std::memory_order_relaxed);
    }
    return job;
}

JobSystem::Job* JobSystem::steal(WorkQueue& workQueue) noexcept {
    // decrement mActiveJobs first, this is to ensure that if there is only a single job left
    // (and we're about to pick it up), other threads don't loop trying to do the same.
    mActiveJobs.fetch_sub(1, std::memory_order_relaxed);

    size_t const index = workQueue.steal();
    assert_invariant(index <= MAX_JOB_COUNT);
    Job* const job = !index ? nullptr : &mJobStorageBase[index - 1];

    if (!job) {
        // If we failed taking a job, we need to correct mActiveJobs.
        mActiveJobs.fetch_add(1, std::memory_order_relaxed);
    }

    return job;
}

inline JobSystem::ThreadState* JobSystem::getStateToStealFrom(JobSystem::ThreadState& state) noexcept {
    auto& threadStates = mThreadStates;
    // memory_order_relaxed is okay because we don't take any action that has data dependency
    // on this value (in particular mThreadStates, is always initialized properly).
    uint16_t const adopted = mAdoptedThreads.load(std::memory_order_relaxed);
    uint16_t const threadCount = mThreadCount + adopted;

    JobSystem::ThreadState* stateToStealFrom = nullptr;

    // don't try to steal from someone else if we're the only thread (infinite loop)
    if (threadCount >= 2) {
        do {
            // This is biased, but frankly, we don't care. It's fast.
            uint16_t const index = uint16_t(state.rndGen() % threadCount);
            assert(index < threadStates.size());
            stateToStealFrom = &threadStates[index];
            // don't steal from our own queue
        } while (stateToStealFrom == &state);
    }
    return stateToStealFrom;
}

JobSystem::Job* JobSystem::steal(JobSystem::ThreadState& state) noexcept {
    HEAVY_SYSTRACE_CALL();
    Job* job = nullptr;
    do {
        ThreadState* const stateToStealFrom = getStateToStealFrom(state);
        if (UTILS_LIKELY(stateToStealFrom)) {
            job = steal(stateToStealFrom->workQueue);
        }
        // nullptr -> nothing to steal in that queue either, if there are active jobs,
        // continue to try stealing one.
    } while (!job && hasActiveJobs());
    return job;
}

bool JobSystem::execute(JobSystem::ThreadState& state) noexcept {
    HEAVY_SYSTRACE_CALL();

    Job* job = pop(state.workQueue);
    if (UTILS_UNLIKELY(job == nullptr)) {
        // our queue is empty, try to steal a job
        job = steal(state);
    }

    if (job) {
        assert(job->runningJobCount.load(std::memory_order_relaxed) >= 1);

        if (UTILS_LIKELY(job->function)) {
            HEAVY_SYSTRACE_NAME("job->function");
            job->id = std::distance(mThreadStates.data(), &state);
            job->function(job->storage, *this, job);
            job->id = invalidThreadId;
        }
        finish(job);
    }
    return job != nullptr;
}

void JobSystem::loop(ThreadState* state) noexcept {
    setThreadName("JobSystem::loop");
    setThreadPriority(Priority::DISPLAY);

    // record our work queue
    std::unique_lock<Mutex> lock(mThreadMapLock);
    bool const inserted = mThreadMap.emplace(std::this_thread::get_id(), state).second;
    lock.unlock();

    FILAMENT_CHECK_PRECONDITION(inserted) << "This thread is already in a loop.";

    // run our main loop...
    do {
        if (!execute(*state)) {
            std::unique_lock<Mutex> lock(mWaiterLock);
            while (!exitRequested() && !hasActiveJobs()) {
                wait(lock);
            }
        }
    } while (!exitRequested());
}

UTILS_NOINLINE
void JobSystem::finish(Job* job) noexcept {
    HEAVY_SYSTRACE_CALL();

    bool notify = false;

    // terminate this job and notify its parent
    Job* const storage = mJobStorageBase;
    do {
        // std::memory_order_release here is needed to synchronize with JobSystem::wait()
        // which needs to "see" all changes that happened before the job terminated.
        auto runningJobCount = job->runningJobCount.fetch_sub(1, std::memory_order_acq_rel);
        assert(runningJobCount > 0);
        if (runningJobCount == 1) {
            // no more work, destroy this job and notify its parent
            notify = true;
            Job* const parent = job->parent == 0x7FFF ? nullptr : &storage[job->parent];
            decRef(job);
            job = parent;
        } else {
            // there is still work (e.g.: children), we're done.
            break;
        }
    } while (job);

    // wake-up all threads that could potentially be waiting on this job finishing
    if (notify) {
        wakeAll();
    }
}

// -----------------------------------------------------------------------------------------------
// public API...


JobSystem::Job* JobSystem::create(JobSystem::Job* parent, JobFunc func) noexcept {
    parent = (parent == nullptr) ? mRootJob : parent;
    Job* const job = allocateJob();
    if (UTILS_LIKELY(job)) {
        size_t index = 0x7FFF;
        if (parent) {
            // add a reference to the parent to make sure it can't be terminated.
            // memory_order_relaxed is safe because no action is taken at this point
            // (the job is not started yet).
            auto parentJobCount = parent->runningJobCount.fetch_add(1, std::memory_order_relaxed);

            // can't create a child job of a terminated parent
            assert(parentJobCount > 0);

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

JobSystem::Job* JobSystem::retain(JobSystem::Job* job) noexcept {
    JobSystem::Job* retained = job;
    incRef(retained);
    return retained;
}

void JobSystem::release(JobSystem::Job*& job) noexcept {
    decRef(job);
    job = nullptr;
}

void JobSystem::run(Job*& job) noexcept {
    HEAVY_SYSTRACE_CALL();

    ThreadState& state(getState());

    put(state.workQueue, job);

    // after run() returns, the job is virtually invalid (it'll die on its own)
    job = nullptr;
}

void JobSystem::run(Job*& job, uint8_t id) noexcept {
    HEAVY_SYSTRACE_CALL();

    ThreadState& state = mThreadStates[id];
    assert_invariant(&state == &getState());

    put(state.workQueue, job);

    // after run() returns, the job is virtually invalid (it'll die on its own)
    job = nullptr;
}

JobSystem::Job* JobSystem::runAndRetain(Job* job) noexcept {
    JobSystem::Job* retained = retain(job);
    run(job);
    return retained;
}

void JobSystem::waitAndRelease(Job*& job) noexcept {
    SYSTRACE_CALL();

    assert(job);
    assert(job->refCount.load(std::memory_order_relaxed) >= 1);

    ThreadState& state(getState());
    do {
        if (!execute(state)) {
            // test if job has completed first, to possibly avoid taking the lock
            if (hasJobCompleted(job)) {
                break;
            }

            // the only way we can be here is if the job we're waiting on it being handled
            // by another thread:
            //    - we returned from execute() which means all queues are empty
            //    - yet our job hasn't completed yet
            //    ergo, it's being run in another thread
            //
            // this could take time however, so we will wait with a condition, and
            // continue to handle more jobs, as they get added.

            std::unique_lock<Mutex> lock(mWaiterLock);
            if (!hasJobCompleted(job) && !hasActiveJobs() && !exitRequested()) {
                wait(lock, job);
            }
        }
    } while (!hasJobCompleted(job) && !exitRequested());

    if (job == mRootJob) {
        mRootJob = nullptr;
    }

    release(job);
}

void JobSystem::runAndWait(JobSystem::Job*& job) noexcept {
    SYSTRACE_CALL();
    runAndRetain(job);
    waitAndRelease(job);
}

void JobSystem::adopt() {
    const auto tid = std::this_thread::get_id();

    std::unique_lock<Mutex> lock(mThreadMapLock);
    auto iter = mThreadMap.find(tid);
    ThreadState* const state = iter ==  mThreadMap.end() ? nullptr : iter->second;
    lock.unlock();

    if (state) {
        // we're already part of a JobSystem, do nothing.
        FILAMENT_CHECK_PRECONDITION(this == state->js)
                << "Called adopt on a thread owned by another JobSystem (" << state->js
                << "), this=" << this << "!";
        return;
    }

    // memory_order_relaxed is safe because we don't take action on this value.
    uint16_t const adopted = mAdoptedThreads.fetch_add(1, std::memory_order_relaxed);
    size_t const index = mThreadCount + adopted;

    FILAMENT_CHECK_POSTCONDITION(index < mThreadStates.size())
            << "Too many calls to adopt(). No more adoptable threads!";

    // all threads adopted by the JobSystem need to run at the same priority
    JobSystem::setThreadPriority(Priority::DISPLAY);

    // This thread's queue will be selectable immediately (i.e.: before we set its TLS)
    // however, it's not a problem since mThreadState is pre-initialized and valid
    // (e.g.: the queue is empty).

    lock.lock();
    mThreadMap[tid] = &mThreadStates[index];
}

void JobSystem::emancipate() {
    const auto tid = std::this_thread::get_id();
    std::unique_lock<Mutex> const lock(mThreadMapLock);
    auto iter = mThreadMap.find(tid);
    ThreadState* const state = iter ==  mThreadMap.end() ? nullptr : iter->second;
    FILAMENT_CHECK_PRECONDITION(state) << "this thread is not an adopted thread";
    FILAMENT_CHECK_PRECONDITION(state->js == this) << "this thread is not adopted by us";
    mThreadMap.erase(iter);
}

io::ostream& operator<<(io::ostream& out, JobSystem const& js) {
    for (auto const& item : js.mThreadStates) {
        size_t const id = std::distance(js.mThreadStates.data(), &item);
        out << id << ": " << item.workQueue.getCount() << io::endl;
    }
    return out;
}

} // namespace utils
